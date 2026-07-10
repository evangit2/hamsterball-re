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

	static float __fastcall hook_TransformX(void* gfx, void* edx, float pixel_x) {
		// Global scaleX/offsetX modification in onGameUpdate handles the transform.
		// This hook is pass-through — don't double-apply.
		return orig_TransformX(gfx, edx, pixel_x);
	}

	// Graphics_DrawScreenRect (0x455d60) can't be hooked via MinHook
	// because its SEH prologue (push -1; push handler; push FS:[0])
	// gets corrupted by the trampoline. Instead, modify scaleX and
	// offsetX globally from onGameUpdate so both Gfx_TransformX and
	// Graphics_DrawScreenRect produce correct results.
	//
	// Gfx_TransformX:       result = pixel_x * scaleX + offsetX
	// Graphics_DrawScreenRect: vertex = pixel_x * scaleX (no offset)
	//
	// Our Gfx_TransformX hook transforms the RETURN value:
	//   new_result = (pixel_x * scaleX + offsetX) * scaleFactor + margin
	//
	// For Graphics_DrawScreenRect to match, we need:
	//   pixel_x * new_scaleX = (pixel_x * scaleX) * scaleFactor + margin
	//   new_scaleX = scaleX * scaleFactor + margin/pixel_x
	//
	// But margin/pixel_x varies per call, so we can't use a single scaleX.
	// Instead, we modify scaleX AND offsetX so that Gfx_TransformX's
	// UNMODIFIED result already includes the transform:
	//   new_scaleX = scaleX * scaleFactor
	//   new_offsetX = offsetX * scaleFactor + margin
	//
	// Then Gfx_TransformX returns: pixel_x * (scaleX*scaleFactor) + (offsetX*scaleFactor + margin)
	//   = (pixel_x * scaleX + offsetX) * scaleFactor + margin  ✓
	//
	// And Graphics_DrawScreenRect returns: pixel_x * (scaleX*scaleFactor)
	//   = (pixel_x * scaleX) * scaleFactor  (no margin, but compressed)  ~OK
	//
	// The rectangles won't have the margin offset, but they'll be compressed
	// to the right width. The Gfx_TransformX hook then becomes a no-op
	// (or we remove it entirely since the global values do the work).

	static inline float g_origScaleX = 0.0f;
	static inline int g_origOffsetX = 0;
	static inline bool g_patched = false;

	static void applyGlobalScaleFix(void* gfx) {
		if (!g_enabled) {
			if (g_patched) {
				DWORD gfxAddr = (DWORD)gfx;
				if (!IsBadReadPtr(gfx, 0x800)) {
					DWORD config = *(DWORD*)(gfxAddr + 0x5c);
					if (config && !IsBadReadPtr((void*)config, 0x200)) {
						float* pScaleX = (float*)(config + 0x1f8);
						int* pOffsetX = (int*)(gfxAddr + 0x798);
						*pScaleX = g_origScaleX;
						*pOffsetX = g_origOffsetX;
					}
				}
				g_patched = false;
			}
			return;
		}

		DWORD gfxAddr = (DWORD)gfx;
		if (IsBadReadPtr(gfx, 0x800)) return;
		DWORD config = *(DWORD*)(gfxAddr + 0x5c);
		if (!config || IsBadReadPtr((void*)config, 0x200)) return;
		DWORD bbWidth = *(DWORD*)(config + 0x15c);
		DWORD bbHeight = *(DWORD*)(config + 0x160);
		if (bbWidth <= 0 || bbHeight <= 0) return;

		float aspect = (float)bbWidth / (float)bbHeight;
		if (aspect <= 1.34f) {
			if (g_patched) {
				float* pScaleX = (float*)(config + 0x1f8);
				int* pOffsetX = (int*)(gfxAddr + 0x798);
				*pScaleX = g_origScaleX;
				*pOffsetX = g_origOffsetX;
				g_patched = false;
			}
			return;
		}

		float* pScaleX = (float*)(config + 0x1f8);
		int* pOffsetX = (int*)(gfxAddr + 0x798);

		float curScaleX = *pScaleX;
		if (!(curScaleX == curScaleX) || curScaleX <= 0.0f || curScaleX > 1.0f) return;

		if (!g_patched) {
			g_origScaleX = curScaleX;
			g_origOffsetX = *pOffsetX;
			g_patched = true;
		}

		float ratio43 = 4.0f / 3.0f;
		float scaleFactor = ratio43 / aspect;
		float margin = ((float)bbWidth - (float)bbHeight * ratio43) / 2.0f;

		*pScaleX = g_origScaleX * scaleFactor;
		*pOffsetX = (int)((float)g_origOffsetX * scaleFactor + margin);
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

	void onGameUpdate() override {
		App* app = api->GetApp();
		if (!app) return;
		DWORD gfx = *(DWORD*)((DWORD)app + 0x174);
		if (!gfx) return;
		applyGlobalScaleFix((void*)gfx);
	}

	void onButtonToggle(const char* id, bool state) override {
		if (strcmp(id, "ws_ui_fix") == 0) g_enabled = state;
	}
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
	return new WidescreenUIFix();
}
