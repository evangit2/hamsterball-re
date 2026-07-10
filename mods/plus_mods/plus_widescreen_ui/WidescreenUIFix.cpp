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

	static float __fastcall hook_TransformX(void* gfx, void* edx, float pixel_x) {
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

		float ratio43 = 4.0f / 3.0f;
		float scaleFactor = ratio43 / aspect;
		float margin = ((float)bbWidth - (float)bbHeight * ratio43) / 2.0f;

		// Also modify memory for DrawScreenRect
		float curScaleX = *pScaleX;
		int curOffsetX = *pOffsetX;
		*pScaleX = curScaleX * scaleFactor;
		*pOffsetX = (int)((float)curOffsetX * scaleFactor + margin);

		g_callCount++;
		if (g_callCount <= 5) {
			printf("[WSUI] call %d: pixel_x=%f scaleX=%f offsetX=%d bbW=%d bbH=%d aspect=%f scaleFactor=%f margin=%f result=%f transformed=%f\n",
				g_callCount, pixel_x, curScaleX, curOffsetX, bbWidth, bbHeight, aspect, scaleFactor, margin,
				result, result * scaleFactor + margin);
		}

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
