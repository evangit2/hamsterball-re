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
	static inline bool g_inSceneRender = false;
	static inline bool g_seen3DFull = false;
	static inline bool g_hadSplit = false;

	typedef void(__fastcall *SetViewport_t)(void*, void*, int, int);
	static inline SetViewport_t orig_SetViewport = nullptr;

	typedef void(__fastcall *SceneRender_t)(void*, void*, void*);
	static inline SceneRender_t orig_SceneRender = nullptr;

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

		float aspect = 4.0f / 3.0f;
		int vpWidth = (int)((float)bbHeight * aspect);
		if (vpWidth > bbWidth) vpWidth = bbWidth;
		int vpX = (bbWidth - vpWidth) / 2;

		float nearPlane = *(float*)(gfxAddr + 0x790);
		float farPlane = *(float*)(gfxAddr + 0x794);
		if (farPlane <= nearPlane) return;

		float halfFov = 0.7853982f * 0.5f;
		float s = sinf(halfFov);
		if (s == 0.0f) return;
		float cot = cosf(halfFov) / s;

		float matrix[16];
		memset(matrix, 0, sizeof(matrix));
		matrix[0] = cot / aspect;
		matrix[5] = cot;
		matrix[10] = farPlane / (farPlane - nearPlane);
		matrix[11] = 1.0f;
		matrix[14] = -(matrix[10] * nearPlane);

		typedef long(__stdcall *SetTransform_t)(void*, DWORD, float*);
		((SetTransform_t)(vtable[37]))((void*)device, 3, matrix);

		D3DVIEWPORT8 vp = { (DWORD)vpX, 0, (DWORD)vpWidth, (DWORD)bbHeight, 0.0f, 1.0f };
		typedef long(__stdcall *SetViewportD3D_t)(void*, D3DVIEWPORT8*);
		((SetViewportD3D_t)(vtable[40]))((void*)device, &vp);
	}

	static void __fastcall hook_SetViewport(void* gfx, void* edx, int param1, int param2) {
		orig_SetViewport(gfx, edx, param1, param2);
		if (!g_enabled || !g_inSceneRender) return;

		if (param1 != 0 || param2 != 0) {
			g_hadSplit = true;
			return;
		}

		bool isUI;
		if (g_hadSplit) {
			isUI = true;
		} else if (!g_seen3DFull) {
			g_seen3DFull = true;
			isUI = false;
		} else {
			isUI = true;
		}

		if (isUI) overrideTo43(gfx);
	}

	static void __fastcall hook_SceneRender(void* this_ptr, void* edx, void* param1) {
		g_inSceneRender = true;
		g_seen3DFull = false;
		g_hadSplit = false;
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
