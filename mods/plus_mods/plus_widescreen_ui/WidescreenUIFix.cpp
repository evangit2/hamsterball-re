#define _CRT_SECURE_NO_WARNINGS
#include "HamsterballAPI.h"
#include <math.h>
#include <string.h>

class WidescreenUIFix : public HamsterballAPI {
private:
	IModAPI* api = nullptr;
	static inline bool g_enabled = true;

	static inline bool g_inSceneRender = false;
	static inline int g_viewportCallCount = 0;

	static inline bool g_scaleModified = false;
	static inline float g_origScaleX = 0.0f;
	static inline DWORD g_scaleXAddr = 0;

	typedef void(__fastcall *SetViewport_t)(void*, void*, int, int);
	static inline SetViewport_t orig_SetViewport = nullptr;

	typedef void(__fastcall *SceneRender_t)(void*, void*, void*);
	static inline SceneRender_t orig_SceneRender = nullptr;

	// UI coordinate system:
	//   Gfx_TransformY = pixel_x * scaleX + offsetX
	//   Gfx_TransformZ = pixel_y * scaleY + offsetY
	// where scaleX is at config+0x1f8, scaleY at config+0x1fc
	// (config = *(gfx+0x5c))
	//
	// On 16:9, the X/Y scale ratio is wrong for 4:3 UI.
	// Fix: read both scales, set scaleX = scaleY * (3/4) so the
	// X/Y ratio matches 4:3. This works regardless of what the
	// original scale values actually are.
	static void fixUIScale(void* gfx) {
		DWORD gfxAddr = (DWORD)gfx;
		if (IsBadReadPtr(gfx, 0x800)) return;

		DWORD config = *(DWORD*)(gfxAddr + 0x5c);
		if (!config || IsBadReadPtr((void*)config, 0x200)) return;

		DWORD bbWidth = *(DWORD*)(config + 0x15c);
		DWORD bbHeight = *(DWORD*)(config + 0x160);
		if (bbWidth <= 0 || bbHeight <= 0) return;

		// Already 4:3 or taller — no fix needed
		float currentAspect = (float)bbWidth / (float)bbHeight;
		if (currentAspect <= 1.34f) return;

		float scaleX = *(float*)(config + 0x1f8);
		float scaleY = *(float*)(config + 0x1fc);

		// Set scaleX so X/Y ratio = 3/4 (4:3 proportions)
		float newScaleX = scaleY * (3.0f / 4.0f);

		// Save and apply
		g_origScaleX = scaleX;
		g_scaleXAddr = config + 0x1f8;
		g_scaleModified = true;

		*(float*)(config + 0x1f8) = newScaleX;
	}

	static void restoreUIScale() {
		if (g_scaleModified && g_scaleXAddr) {
			*(float*)g_scaleXAddr = g_origScaleX;
			g_scaleModified = false;
			g_scaleXAddr = 0;
		}
	}

	static void __fastcall hook_SetViewport(void* gfx, void* edx, int param1, int param2) {
		orig_SetViewport(gfx, edx, param1, param2);

		if (!g_enabled || !g_inSceneRender) return;
		if (param1 != 0 || param2 != 0) return;

		g_viewportCallCount++;

		// 1st (0,0) = 3D pass → leave alone
		// 2nd (0,0) = UI pass → fix scale
		if (g_viewportCallCount == 2) {
			fixUIScale(gfx);
		}
	}

	static void __fastcall hook_SceneRender(void* this_ptr, void* edx, void* param1) {
		restoreUIScale();
		g_inSceneRender = true;
		g_viewportCallCount = 0;
		orig_SceneRender(this_ptr, edx, param1);
		g_inSceneRender = false;
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
