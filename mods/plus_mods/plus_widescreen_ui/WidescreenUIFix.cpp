#define _CRT_SECURE_NO_WARNINGS
#include "HamsterballAPI.h"
#include <math.h>
#include <string.h>

class WidescreenUIFix : public HamsterballAPI {
private:
	IModAPI* api = nullptr;
	static inline bool g_enabled = true;

	typedef float(__fastcall *TransformX_t)(void*, void*, float);
	static inline TransformX_t orig_TransformX = nullptr;

	// Track what we last wrote to detect frame boundaries
	static inline float g_lastModifiedScaleX = -1.0f;
	static inline float g_origScaleX = 0.0f;
	static inline int g_origOffsetX = 0;

	static float __fastcall hook_TransformX(void* gfx, void* edx, float pixel_x) {
		// Call original FIRST — before any memory modification
		float result = orig_TransformX(gfx, edx, pixel_x);

		if (!g_enabled) return result;

		DWORD gfxAddr = (DWORD)gfx;
		if (IsBadReadPtr(gfx, 0x800)) return result;
		DWORD config = *(DWORD*)(gfxAddr + 0x5c);
		if (!config || IsBadReadPtr((void*)config, 0x200)) return result;
		DWORD bbWidth = *(DWORD*)(config + 0x15c);
		DWORD bbHeight = *(DWORD*)(config + 0x160);
		if (bbWidth <= 0 || bbHeight <= 0) return result;

		float aspect = (float)bbWidth / (float)bbHeight;
		if (aspect <= 1.34f) return result;

		float* pScaleX = (float*)(config + 0x1f8);
		int* pOffsetX = (int*)(gfxAddr + 0x798);

		float curScaleX = *pScaleX;
		if (!(curScaleX == curScaleX) || curScaleX <= 0.0f || curScaleX > 1.0f) return result;

		float ratio43 = 4.0f / 3.0f;
		float scaleFactor = ratio43 / aspect;
		float margin = ((float)bbWidth - (float)bbHeight * ratio43) / 2.0f;

		// Check if memory still has our modified values from last call
		// (same frame) or has been reset by the game (new frame)
		if (curScaleX == g_lastModifiedScaleX) {
			// Memory has our modified values — orig_TransformX already
			// used them, so result is already correct. No transform needed.
			return result;
		}

		// New frame (or first call) — game set fresh original values
		g_origScaleX = curScaleX;
		g_origOffsetX = *pOffsetX;

		// Modify global memory so DrawScreenRect also gets the fix
		float newScaleX = g_origScaleX * scaleFactor;
		int newOffsetX = (int)((float)g_origOffsetX * scaleFactor + margin);
		*pScaleX = newScaleX;
		*pOffsetX = newOffsetX;
		g_lastModifiedScaleX = newScaleX;

		// result was computed with ORIGINAL values (before our modification)
		// so we need to transform it
		return result * scaleFactor + margin;
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
