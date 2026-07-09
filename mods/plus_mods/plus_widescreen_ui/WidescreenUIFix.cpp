#define _CRT_SECURE_NO_WARNINGS
#include "HamsterballAPI.h"
#include <math.h>
#include <string.h>

class WidescreenUIFix : public HamsterballAPI {
private:
	IModAPI* api = nullptr;
	static inline bool g_enabled = true;

	// Render phase tracking
	static inline bool g_inSceneRender = false;
	static inline int g_viewportCallCount = 0;

	// Saved state for restore after UI pass
	static inline bool g_scaleModified = false;
	static inline float g_origScaleX = 0.0f;
	static inline DWORD g_origScaleXAddr = 0;

	typedef void(__fastcall *SetViewport_t)(void*, void*, int, int);
	static inline SetViewport_t orig_SetViewport = nullptr;

	typedef void(__fastcall *SceneRender_t)(void*, void*, void*);
	static inline SceneRender_t orig_SceneRender = nullptr;

	// On the UI pass, modify the X scale factor so UI elements render at 4:3
	// proportions instead of being stretched to 16:9.
	//
	// The UI uses D3DFVF_XYZRHW (transformed) vertices. Gfx_TransformY converts
	// pixel X to NDC via: result = pixel * scaleX + offsetX
	// where scaleX is at presentParams+0x1f8.
	//
	// The UI coordinate system is centered at origin (0 = screen center).
	// scaleX = 2/bbWidth, scaleY = 2/bbHeight.
	// On 16:9, the X/Y ratio is bbHeight/bbWidth (0.5625) instead of 3/4 (0.75).
	// Fix: set scaleX = 2/(bbHeight * 4/3) so X/Y ratio = 3/4 (4:3).
	// This compresses UI horizontally to correct proportions, centered on screen.
	static void fixUIScale(void* gfx) {
		DWORD gfxAddr = (DWORD)gfx;
		if (IsBadReadPtr(gfx, 0x800)) return;

		DWORD presentParams = *(DWORD*)(gfxAddr + 0x5c);
		if (!presentParams || IsBadReadPtr((void*)presentParams, 0x200)) return;
		int bbWidth = *(int*)(presentParams + 0x15c);
		int bbHeight = *(int*)(presentParams + 0x160);
		if (bbWidth <= 0 || bbHeight <= 0) return;

		// Already 4:3 or taller — no fix needed
		float currentAspect = (float)bbWidth / (float)bbHeight;
		if (currentAspect <= 1.34f) return;

		// Fix scaleX to 4:3 proportions
		DWORD scaleXAddr = presentParams + 0x1f8;
		float origScaleX = *(float*)scaleXAddr;
		float newScaleX = 2.0f / ((float)bbHeight * (4.0f / 3.0f));

		// Save original for restore
		g_origScaleX = origScaleX;
		g_origScaleXAddr = scaleXAddr;
		g_scaleModified = true;

		// Apply 4:3 scale
		*(float*)scaleXAddr = newScaleX;
	}

	static void restoreUIScale() {
		if (g_scaleModified && g_origScaleXAddr) {
			*(float*)g_origScaleXAddr = g_origScaleX;
			g_scaleModified = false;
			g_origScaleXAddr = 0;
		}
	}

	static void __fastcall hook_SetViewport(void* gfx, void* edx, int param1, int param2) {
		// Always call original first
		orig_SetViewport(gfx, edx, param1, param2);

		if (!g_enabled || !g_inSceneRender) return;

		// Only override on full-screen (0,0) calls — leave split-screen alone
		if (param1 != 0 || param2 != 0) return;

		g_viewportCallCount++;

		// In Scene_Render, each mode follows the same pattern:
		//   1st (0,0) call = 3D pass → leave alone (widescreen 3D is fine)
		//   2nd (0,0) call = UI pass → fix the X scale factor to 4:3
		if (g_viewportCallCount == 2) {
			fixUIScale(gfx);
		}
	}

	static void __fastcall hook_SceneRender(void* this_ptr, void* edx, void* param1) {
		// Restore scale from previous frame (in case it wasn't restored)
		restoreUIScale();

		g_inSceneRender = true;
		g_viewportCallCount = 0;
		orig_SceneRender(this_ptr, edx, param1);
		g_inSceneRender = false;

		// Restore scale after Scene_Render completes (3D pass of next frame
		// needs the original widescreen scale)
		restoreUIScale();
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
