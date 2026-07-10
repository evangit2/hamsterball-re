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

	// Save original values once, reuse every call
	static inline float g_savedScaleX = -1.0f;
	static inline int g_savedOffsetX = 0;

	static float __fastcall hook_TransformX(void* gfx, void* edx, float pixel_x) {
		// Call original first — uses current scaleX/offsetX
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
		float ratio43 = 4.0f / 3.0f;
		float scaleFactor = ratio43 / aspect;
		float margin = ((float)bbWidth - (float)bbHeight * ratio43) / 2.0f;

		// Save originals if this is the first call or if the game
		// reset scaleX to a new value (different from our last write)
		// We detect "game reset" by checking if curScaleX is NOT our modified value
		float expectedModified = g_savedScaleX * scaleFactor;

		if (g_savedScaleX <= 0.0f ||
			(curScaleX != expectedModified && curScaleX != g_savedScaleX)) {
			// curScaleX is neither our saved original nor our modified value
			// → game set a new original, save it
			if ((curScaleX == curScaleX) && curScaleX > 0.0f && curScaleX <= 1.0f) {
				g_savedScaleX = curScaleX;
				g_savedOffsetX = *pOffsetX;
			}
		}

		// Always transform return value (v17 approach — known working)
		float transformed = result * scaleFactor + margin;

		// Modify global memory for DrawScreenRect using SAVED originals
		if (g_savedScaleX > 0.0f) {
			*pScaleX = g_savedScaleX * scaleFactor;
			*pOffsetX = (int)((float)g_savedOffsetX * scaleFactor + margin);
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
		if (strcmp(id, "ws_ui_fix") == 0) {
			g_enabled = state;
			if (!state) g_savedScaleX = -1.0f; // reset on disable
		}
	}
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
	return new WidescreenUIFix();
}
