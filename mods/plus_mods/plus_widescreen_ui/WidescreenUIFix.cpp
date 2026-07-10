#define _CRT_SECURE_NO_WARNINGS
#include "HamsterballAPI.h"
#include <math.h>
#include <string.h>
#include <stdio.h>

class WidescreenUIFix : public HamsterballAPI {
private:
	IModAPI* api = nullptr;
	static inline bool g_enabled = true;
	static inline int g_callCount = 0;

	typedef float(__fastcall *TransformX_t)(void*, void*, float);
	static inline TransformX_t orig_TransformX = nullptr;

	// One-time-per-frame memory modification
	static inline float g_lastModifiedScaleX = -999.0f;
	static inline float g_origScaleX = 0.0f;
	static inline int g_origOffsetX = 0;

	static float __fastcall hook_TransformX(void* gfx, void* edx, float pixel_x) {
		if (!g_enabled) return orig_TransformX(gfx, edx, pixel_x);

		DWORD gfxAddr = (DWORD)gfx;
		if (IsBadReadPtr(gfx, 0x800)) return orig_TransformX(gfx, edx, pixel_x);
		DWORD config = *(DWORD*)(gfxAddr + 0x5c);
		if (!config || IsBadReadPtr((void*)config, 0x200)) return orig_TransformX(gfx, edx, pixel_x);
		DWORD bbWidth = *(DWORD*)(config + 0x15c);
		DWORD bbHeight = *(DWORD*)(config + 0x160);
		if (bbWidth <= 0 || bbHeight <= 0) return orig_TransformX(gfx, edx, pixel_x);

		float aspect = (float)bbWidth / (float)bbHeight;
		if (aspect <= 1.34f) return orig_TransformX(gfx, edx, pixel_x);

		float* pScaleX = (float*)(config + 0x1f8);
		int* pOffsetX = (int*)(gfxAddr + 0x798);

		float curScaleX = *pScaleX;
		float ratio43 = 4.0f / 3.0f;
		float scaleFactor = ratio43 / aspect;
		float margin = ((float)bbWidth - (float)bbHeight * ratio43) / 2.0f;

		// Check if this is the first call this frame
		// (game hasn't reset scaleX, so it still has our modified value from last frame)
		if (curScaleX == g_lastModifiedScaleX && g_lastModifiedScaleX != -999.0f) {
			// Same frame — memory already modified, orig already computed correct result
			float result = orig_TransformX(gfx, edx, pixel_x);

			g_callCount++;
			if (g_callCount <= 10) {
				printf("[WSUI] sub call %d: pixel_x=%f scaleX=%f offsetX=%d result=%f (no transform)\n",
					g_callCount, pixel_x, curScaleX, *pOffsetX, result);
			}

			return result;
		}

		// First call this frame (or first ever call)
		// Save originals before modifying
		g_origScaleX = curScaleX;
		g_origOffsetX = *pOffsetX;

		// Call orig FIRST (reads original values)
		float result = orig_TransformX(gfx, edx, pixel_x);

		// Transform return value (computed from original values — no double-app)
		float transformed = result * scaleFactor + margin;

		// Modify memory ONCE for DrawScreenRect
		float newScaleX = g_origScaleX * scaleFactor;
		int newOffsetX = (int)((float)g_origOffsetX * scaleFactor + margin);
		*pScaleX = newScaleX;
		*pOffsetX = newOffsetX;
		g_lastModifiedScaleX = newScaleX;

		g_callCount++;
		if (g_callCount <= 10) {
			printf("[WSUI] first call %d: pixel_x=%f origScale=%f origOffset=%d newScale=%f newOffset=%d result=%f transformed=%f\n",
				g_callCount, pixel_x, g_origScaleX, g_origOffsetX, newScaleX, newOffsetX, result, transformed);
		}

		return transformed;
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
		api->RegisterCustomHook(0x453e90, (void*)hook_TransformX, (void**)&orig_TransformX);
	}

	void onButtonToggle(const char* id, bool state) override {
		if (strcmp(id, "ws_ui_fix") == 0) g_enabled = state;
	}
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
	return new WidescreenUIFix();
}
