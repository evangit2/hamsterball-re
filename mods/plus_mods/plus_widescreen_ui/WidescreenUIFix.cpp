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

	// Saved original values (captured on first valid call)
	static inline float g_origScaleX = 0.0f;
	static inline int g_origOffsetX = 0;
	static inline bool g_haveOriginals = false;

	// Global margin value for DrawScreenRect code caves
	static inline float g_margin = 0.0f;
	static inline bool g_rectsPatched = false;

	// Addresses of the 4 FMUL [ECX+0x1f8] instructions in DrawScreenRect
	static constexpr DWORD FMUL_ADDR_1 = 0x55dc8;
	static constexpr DWORD FMUL_ADDR_2 = 0x55e1b;
	static constexpr DWORD FMUL_ADDR_3 = 0x55e43;
	static constexpr DWORD FMUL_ADDR_4 = 0x55e75;

	static void patchRects() {
		if (g_rectsPatched) return;

		DWORD base = (DWORD)GetModuleHandle(NULL);

		BYTE* cave = (BYTE*)VirtualAlloc(NULL, 256, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
		if (!cave) return;

		DWORD fmulAddrs[] = {FMUL_ADDR_1, FMUL_ADDR_2, FMUL_ADDR_3, FMUL_ADDR_4};
		DWORD caveOffset = 0;

		for (int i = 0; i < 4; i++) {
			DWORD patchAddr = base + fmulAddrs[i];
			DWORD retAddr = patchAddr + 6;

			// Cave: FMUL [ECX+0x1f8] + FADD [g_margin] + JMP back
			// D8 89 = FMUL m32fp [ECX+disp32]  (32-bit float multiply)
			// D8 05 = FADD m32fp [imm32]       (32-bit float add — NOT DC=double!)
			cave[caveOffset + 0] = 0xD8;
			cave[caveOffset + 1] = 0x89;
			*(DWORD*)(cave + caveOffset + 2) = 0x1f8;

			cave[caveOffset + 6] = 0xD8;
			cave[caveOffset + 7] = 0x05;
			*(DWORD*)(cave + caveOffset + 8) = (DWORD)&g_margin;

			cave[caveOffset + 12] = 0xE9;
			DWORD jmpTarget = retAddr - ((DWORD)(cave + caveOffset + 17));
			*(DWORD*)(cave + caveOffset + 13) = jmpTarget;

			DWORD oldProtect;
			VirtualProtect((void*)patchAddr, 6, PAGE_EXECUTE_READWRITE, &oldProtect);
			*(BYTE*)(patchAddr + 0) = 0xE9;
			DWORD caveTarget = (DWORD)(cave + caveOffset) - (patchAddr + 5);
			*(DWORD*)(patchAddr + 1) = caveTarget;
			*(BYTE*)(patchAddr + 5) = 0x90;
			VirtualProtect((void*)patchAddr, 6, oldProtect, &oldProtect);
			FlushInstructionCache(GetCurrentProcess(), (void*)patchAddr, 6);

			caveOffset += 17;
		}

		g_rectsPatched = true;
		printf("[WSUI] DrawScreenRect patched: cave=%p margin_addr=%p\n", cave, &g_margin);
	}

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
		int curOffsetX = *pOffsetX;

		float ratio43 = 4.0f / 3.0f;
		float scaleFactor = ratio43 / aspect;
		float margin = ((float)bbWidth - (float)bbHeight * ratio43) / 2.0f;
		g_margin = margin;

		// On first call, save the original values
		if (!g_haveOriginals) {
			if ((curScaleX == curScaleX) && curScaleX > 0.0f) {
				g_origScaleX = curScaleX;
				g_origOffsetX = curOffsetX;
				g_haveOriginals = true;
			} else {
				return orig_TransformX(gfx, edx, pixel_x);
			}
		}

		// ALWAYS restore originals before calling orig
		// This prevents exponential multiplication when SetViewport
		// resets offsetX but leaves our modified scaleX
		*pScaleX = g_origScaleX;
		*pOffsetX = g_origOffsetX;

		// Call orig with ORIGINAL values → correct result
		float result = orig_TransformX(gfx, edx, pixel_x);

		// Transform return value (computed from original values)
		float transformed = result * scaleFactor + margin;

		// Write modified values for DrawScreenRect
		*pScaleX = g_origScaleX * scaleFactor;
		*pOffsetX = (int)((float)g_origOffsetX * scaleFactor + margin);

		g_callCount++;
		if (g_callCount <= 10) {
			printf("[WSUI] call %d: pixel_x=%f origScale=%f origOffset=%d curScale=%f curOffset=%d newScale=%f newOffset=%d margin=%f result=%f transformed=%f\n",
				g_callCount, pixel_x, g_origScaleX, g_origOffsetX, curScaleX, curOffsetX,
				*pScaleX, *pOffsetX, margin, result, transformed);
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
		patchRects();
	}

	void onButtonToggle(const char* id, bool state) override {
		if (strcmp(id, "ws_ui_fix") == 0) {
			g_enabled = state;
			if (!state && g_haveOriginals) {
				// Restore originals when disabled
				App* app = api->GetApp();
				if (app) {
					DWORD gfx = *(DWORD*)((DWORD)app + 0x174);
					if (gfx && !IsBadReadPtr((void*)gfx, 0x800)) {
						DWORD config = *(DWORD*)((DWORD)gfx + 0x5c);
						if (config && !IsBadReadPtr((void*)config, 0x200)) {
							*((float*)(config + 0x1f8)) = g_origScaleX;
							*((int*)((DWORD)gfx + 0x798)) = g_origOffsetX;
						}
					}
				}
			}
		}
	}
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
	return new WidescreenUIFix();
}
