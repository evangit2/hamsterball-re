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

	// UI coordinate system (from Ghidra decompilation):
	//   Gfx_TransformY(pixel_x) = pixel_x * scaleX + offsetX
	//   Gfx_TransformZ(pixel_y) = pixel_y * scaleY + offsetY
	//
	// Where:
	//   scaleX = *(float*)(config + 0x1f8)   (config = *(gfx+0x5c))
	//   scaleY = *(float*)(config + 0x1fc)
	//   offsetX = *(int*)(gfx + 0x798)  (set to 0 for full-screen)
	//   offsetY = *(int*)(gfx + 0x79c)  (set to 0 for full-screen)
	//   bbWidth = *(int*)(config + 0x15c)
	//   bbHeight = *(int*)(config + 0x160)
	//
	// The game's UI pixel coordinate system is CENTERED AT 0:
	//   pixel 0 → NDC 0 → screen center (bbWidth/2)
	//   pixel range: [-bbHeight/2, +bbHeight/2] maps to NDC [-1, +1]
	//
	// The game sets scaleX = scaleY = 2/bbHeight (calibrated for height).
	// On 4:3 this gives correct square pixels.
	// On 16:9 the same scaleX stretches X by the aspect ratio.
	//
	// Fix: shrink scaleX so X/Y ratio = 4:3.
	//   newScaleX = scaleX * (4/3) / screenAspect
	//   = scaleX * (4.0 * bbHeight) / (3.0 * bbWidth)
	//
	// Do NOT modify offsetX — it must stay 0 for centering.
	// The pixel coordinate system is symmetric around 0, so changing
	// only scaleX automatically pillarboxes (compresses X range)
	// while keeping everything centered.
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

		// Shrink scaleX to achieve 4:3 X/Y proportions.
		// Screen aspect = bbWidth/bbHeight. On 16:9 = 1.778.
		// 4:3 ratio = 1.333. Scale factor = 1.333/1.778 = 0.75.
		// This compresses the X coordinate range so UI elements
		// render at correct 4:3 proportions, automatically centered
		// (because offsetX stays 0 and the coordinate system is
		// symmetric around 0).
		float aspectRatio = (float)bbWidth / (float)bbHeight;
		float ratio43 = 4.0f / 3.0f;
		float newScaleX = scaleX * ratio43 / aspectRatio;

		// Save and apply (only scaleX, NOT offsetX)
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
