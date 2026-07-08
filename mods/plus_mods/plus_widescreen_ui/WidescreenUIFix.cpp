#define _CRT_SECURE_NO_WARNINGS
#include "HamsterballAPI.h"
#include <math.h>
#include <string.h>

#pragma pack(push, 1)
struct D3DVIEWPORT8 {
	DWORD X, Y, Width, Height;
	float MinZ, MaxZ;
};
#pragma pack(pop)

class WidescreenUIFix : public HamsterballAPI {
private:
	IModAPI* api = nullptr;
	static inline bool g_enabled = true;

	// Render phase tracking
	static inline bool g_inSceneRender = false;
	static inline int g_viewportCallCount = 0;  // counts (0,0) calls within current Scene_Render

	typedef void(__fastcall *SetViewport_t)(void*, void*, int, int);
	static inline SetViewport_t orig_SetViewport = nullptr;

	typedef void(__fastcall *SceneRender_t)(void*, void*, void*);
	static inline SceneRender_t orig_SceneRender = nullptr;

	// Override the projection matrix to 4:3 aspect ratio
	// This makes UI elements render at correct proportions without stretching
	static void overrideProjectionTo43(void* gfx) {
		DWORD gfxAddr = (DWORD)gfx;
		if (IsBadReadPtr(gfx, 0x800)) return;

		DWORD device = *(DWORD*)(gfxAddr + 0x154);
		if (!device || IsBadReadPtr((void*)device, 4)) return;
		DWORD* vtable = *(DWORD**)device;

		DWORD presentParams = *(DWORD*)(gfxAddr + 0x5c);
		if (!presentParams || IsBadReadPtr((void*)presentParams, 0x200)) return;
		int bbWidth = *(int*)(presentParams + 0x15c);
		int bbHeight = *(int*)(presentParams + 0x160);
		if (bbWidth <= 0 || bbHeight <= 0) return;

		// Already 4:3 or taller — no fix needed
		float currentAspect = (float)bbWidth / (float)bbHeight;
		if (currentAspect <= 1.34f) return;

		float targetAspect = 4.0f / 3.0f;

		float nearPlane = *(float*)(gfxAddr + 0x790);
		float farPlane = *(float*)(gfxAddr + 0x794);
		if (farPlane <= nearPlane) return;

		// Build 4:3 perspective matrix using the same formula as Matrix_BuildPerspectiveFOV
		float halfFov = 0.7853982f * 0.5f;  // PI/4 * 0.5
		float s = sinf(halfFov);
		if (s == 0.0f) return;
		float cot = cosf(halfFov) / s;

		float matrix[16];
		memset(matrix, 0, sizeof(matrix));
		matrix[0] = cot / targetAspect;
		matrix[5] = cot;
		matrix[10] = farPlane / (farPlane - nearPlane);
		matrix[11] = 1.0f;
		matrix[14] = -(matrix[10] * nearPlane);

		// Override D3D projection matrix to 4:3 aspect
		typedef long(__stdcall *SetTransform_t)(void*, DWORD, float*);
		((SetTransform_t)(vtable[37]))((void*)device, 3, matrix);

		// Update gfx struct's stored projection matrix (gfx+0x2a4, 16 floats = 64 bytes)
		// so the game's own code uses the corrected matrix
		memcpy((void*)(gfxAddr + 0x2a4), matrix, 64);

		// Update the stored aspect in gfx struct so Gfx_TransformY/Z use 4:3 dimensions
		// gfx+0x7a0 = render width, gfx+0x7a4 = render height
		// Set render width to what it would be at 4:3 — this fixes the UI scale factors
		int vpWidth = (int)((float)bbHeight * targetAspect);
		if (vpWidth > bbWidth) vpWidth = bbWidth;
		*(float*)(gfxAddr + 0x7a0) = (float)vpWidth;
		// Keep height the same, but set the X offset for centering
		*(int*)(gfxAddr + 0x798) = (bbWidth - vpWidth) / 2;
	}

	static void __fastcall hook_SetViewport(void* gfx, void* edx, int param1, int param2) {
		// Always call original first
		orig_SetViewport(gfx, edx, param1, param2);

		if (!g_enabled || !g_inSceneRender) return;

		// Only override on full-screen (0,0) calls — leave split-screen alone
		if (param1 != 0 || param2 != 0) return;

		g_viewportCallCount++;

		// In Scene_Render, each mode (0 players, 1 player, 2 player) follows the same pattern:
		//   Graphics_SetViewport(0,0)  ← 3D pass (first call)
		//   vtable[0x60/0x64/0x68](gfx)  ← 3D world rendering
		//   Graphics_SetViewport(0,0)  ← UI pass (second call)
		//   vtable[0x70/0x6c](gfx)  ← UI overlay rendering
		//
		// The second (0,0) call is always the UI pass.
		// For 2-player split-screen: the loop uses non-zero params for 3D,
		// then the final (0,0) is the UI pass.
		if (g_viewportCallCount == 2) {
			overrideProjectionTo43(gfx);
		}
	}

	static void __fastcall hook_SceneRender(void* this_ptr, void* edx, void* param1) {
		g_inSceneRender = true;
		g_viewportCallCount = 0;
		orig_SceneRender(this_ptr, edx, param1);
		g_inSceneRender = false;
	}

public:
	const char* GetModName() override { return "Widescreen UI Fix"; }
	const char* GetAuthorName() override { return "BookwormKevin"; }
	int GetApiVersion() override { return HAMSTERBALL_API_VERSION; }

	void Initialize(IModAPI* modApi) override {
		api = modApi;
		CustomButton btn;
		btn.id = "ws_ui_fix";
		btn.displayText = "Widescreen UI Fix";
		btn.defaultState = true;
		api->CreateToggleButton(btn, this);
		api->RegisterCustomHook(0x454f10, (void*)hook_SetViewport, (void**)&orig_SetViewport);
		api->RegisterCustomHook(0x41a2e0, (void*)hook_SceneRender, (void**)&orig_SceneRender);
	}

	void onButtonToggle(const char* id, bool state) override {
		if (strcmp(id, "ws_ui_fix") == 0) g_enabled = state;
	}
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
	return new WidescreenUIFix();
}
