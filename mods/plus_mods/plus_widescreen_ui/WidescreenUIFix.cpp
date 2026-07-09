#define _CRT_SECURE_NO_WARNINGS
#include "HamsterballAPI.h"
#include <math.h>
#include <string.h>

class WidescreenUIFix : public HamsterballAPI {
private:
	IModAPI* api = nullptr;
	static inline bool g_enabled = true;
	static inline int g_debugCount = 0;

	typedef float(__fastcall *TransformX_t)(void*, void*, float);
	static inline TransformX_t orig_TransformX = nullptr;

	typedef void(__fastcall *DrawScreenRect_t)(void*, void*, int, int, int, int);
	static inline DrawScreenRect_t orig_DrawScreenRect = nullptr;

	static bool getWidescreenParams(void* gfx, float* outScale, float* outMargin) {
		DWORD gfxAddr = (DWORD)gfx;
		if (IsBadReadPtr(gfx, 0x800)) return false;
		DWORD config = *(DWORD*)(gfxAddr + 0x5c);
		if (!config || IsBadReadPtr((void*)config, 0x200)) return false;
		DWORD bbWidth = *(DWORD*)(config + 0x15c);
		DWORD bbHeight = *(DWORD*)(config + 0x160);
		if (bbWidth <= 0 || bbHeight <= 0) return false;
		float aspect = (float)bbWidth / (float)bbHeight;
		if (aspect <= 1.34f) return false;
		float ratio43 = 4.0f / 3.0f;
		*outScale = ratio43 / aspect;
		*outMargin = ((float)bbWidth - (float)bbHeight * ratio43) / 2.0f;
		return true;
	}

	static float __fastcall hook_TransformX(void* gfx, void* edx, float pixel_x) {
		float result = orig_TransformX(gfx, edx, pixel_x);
		if (!g_enabled) return result;
		float scaleFactor, margin;
		if (!getWidescreenParams(gfx, &scaleFactor, &margin)) return result;
		return result * scaleFactor + margin;
	}

	static void __fastcall hook_DrawScreenRect(void* gfx, void* edx, int x, int y, int w, int h) {
		if (!g_enabled) { orig_DrawScreenRect(gfx, edx, x, y, w, h); return; }

		float scaleFactor, margin;
		if (!getWidescreenParams(gfx, &scaleFactor, &margin)) {
			orig_DrawScreenRect(gfx, edx, x, y, w, h);
			return;
		}

		DWORD gfxAddr = (DWORD)gfx;
		DWORD config = *(DWORD*)(gfxAddr + 0x5c);

		float* pScaleX = (float*)(config + 0x1f8);
		float origScaleX = *pScaleX;

		if (origScaleX <= 0.0f || origScaleX > 1.0f) {
			orig_DrawScreenRect(gfx, edx, x, y, w, h);
			return;
		}

		float newScaleX = origScaleX * scaleFactor;

		g_debugCount++;
		if (g_debugCount <= 5) {
			printf("[WSUI] rect #%d: x=%d y=%d w=%d h=%d scaleX=%f newScale=%f margin=%f offset=%d\n",
				g_debugCount, x, y, w, h, origScaleX, newScaleX, margin, (int)(margin / newScaleX));
		}

		*pScaleX = newScaleX;
		int newX = x + (int)(margin / newScaleX);
		orig_DrawScreenRect(gfx, edx, newX, y, w, h);
		*pScaleX = origScaleX;
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
		api->RegisterCustomHook(0x455d60, (void*)hook_DrawScreenRect, (void**)&orig_DrawScreenRect);
	}

	void onButtonToggle(const char* id, bool state) override {
		if (strcmp(id, "ws_ui_fix") == 0) g_enabled = state;
	}
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
	return new WidescreenUIFix();
}
