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

	typedef void(__fastcall *SetViewport_t)(void*, void*, int, int);
	static inline SetViewport_t orig_SetViewport = nullptr;

	static void overrideTo43(void* gfx) {
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

		// Already 4:3 or taller — no pillarbox needed
		float currentAspect = (float)bbWidth / (float)bbHeight;
		if (currentAspect <= 1.34f) return;

		// Compute centered 4:3 viewport
		float targetAspect = 4.0f / 3.0f;
		int vpWidth = (int)((float)bbHeight * targetAspect);
		if (vpWidth > bbWidth) vpWidth = bbWidth;
		int vpX = (bbWidth - vpWidth) / 2;

		float nearPlane = *(float*)(gfxAddr + 0x790);
		float farPlane = *(float*)(gfxAddr + 0x794);
		if (farPlane <= nearPlane) return;

		// Build 4:3 perspective matrix (same formula as game's Matrix_BuildPerspectiveFOV)
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

		// Override D3D projection matrix to 4:3
		typedef long(__stdcall *SetTransform_t)(void*, DWORD, float*);
		((SetTransform_t)(vtable[37]))((void*)device, 3, matrix);

		// Override D3D viewport to centered 4:3 rectangle (pillarbox)
		D3DVIEWPORT8 vp = { (DWORD)vpX, 0, (DWORD)vpWidth, (DWORD)bbHeight, 0.0f, 1.0f };
		typedef long(__stdcall *SetViewportD3D_t)(void*, D3DVIEWPORT8*);
		((SetViewportD3D_t)(vtable[40]))((void*)device, &vp);

		// Also update the gfx struct's stored dimensions so the game's
		// own rendering code uses the pillarboxed dimensions
		*(int*)(gfxAddr + 0x798) = vpX;           // viewport X offset
		*(float*)(gfxAddr + 0x7a0) = (float)vpWidth;  // render width
		*(float*)(gfxAddr + 0x7a4) = (float)bbHeight; // render height (unchanged)
	}

	static void __fastcall hook_SetViewport(void* gfx, void* edx, int param1, int param2) {
		// Always call original first — it sets up internal state
		orig_SetViewport(gfx, edx, param1, param2);

		if (!g_enabled) return;

		// Only override on full-screen (0,0) calls — leave split-screen alone
		if (param1 != 0 || param2 != 0) return;

		overrideTo43(gfx);
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
	}

	void onButtonToggle(const char* id, bool state) override {
		if (strcmp(id, "ws_ui_fix") == 0) g_enabled = state;
	}
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
	return new WidescreenUIFix();
}
