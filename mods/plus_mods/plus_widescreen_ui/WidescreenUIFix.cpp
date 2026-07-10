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

	// Whether we've already patched scaleX/offsetX this frame
	static inline bool g_patchedThisFrame = false;
	static inline float g_origScaleX = 0.0f;
	static inline int g_origOffsetX = 0;

	static float __fastcall hook_TransformX(void* gfx, void* edx, float pixel_x) {
		// Call original first — it reads scaleX/offsetX and computes the result
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

		// Validate scaleX
		if (!(curScaleX == curScaleX) || curScaleX <= 0.0f || curScaleX > 1.0f) return result;

		float ratio43 = 4.0f / 3.0f;
		float scaleFactor = ratio43 / aspect;
		float margin = ((float)bbWidth - (float)bbHeight * ratio43) / 2.0f;

		// If not yet patched this frame, save originals and apply
		if (!g_patchedThisFrame) {
			g_origScaleX = curScaleX;
			g_origOffsetX = *pOffsetX;
			g_patchedThisFrame = true;
		}

		// Apply widescreen correction to global memory
		*pScaleX = g_origScaleX * scaleFactor;
		*pOffsetX = (int)((float)g_origOffsetX * scaleFactor + margin);

		// The original already computed result with the OLD scaleX/offsetX.
		// Re-compute with the NEW values to get the correct result.
		result = pixel_x * (*pScaleX) + (float)(*pOffsetX);

		return result;
	}

public:
	const char* GetModName() override { return "Widescreen UI Fix"; }
	const char* GetAuthorName() override { return "BookwormKevin"; }
	int GetApiVersion() override { return HAMSTERBALL_API_VERSION; }

	void onGameUpdate() override {
		// Reset the per-frame flag so the first Gfx_TransformX call next frame
		// saves fresh originals
		g_patchedThisFrame = false;

		// If disabled, restore originals on next frame's first call
	}

	void onButtonToggle(const char* id, bool state) override {
		if (strcmp(id, "ws_ui_fix") == 0) g_enabled = state;
	}
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
	return new WidescreenUIFix();
}
