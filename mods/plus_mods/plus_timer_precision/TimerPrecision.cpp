#define _CRT_SECURE_NO_WARNINGS
#include "HamsterballAPI.h"
#include <string.h>

// ============================================================================
// Timer Precision Mod — increases arena timer from 1 to 2 decimal places
//
// Game timer: 100 ticks = 1 second
// Original:  integer=timer/100, decimal=(timer/10)%10  →  "12.3"
// Patched:   integer=timer/100, decimal=timer%100        →  "12.34"
//
// The arena timer (ArenaBoard_Render @ 0x421910) is the in-race HUD timer
// shown during Rodent Rumble / arena mode. It reads timer from board+0x47AC.
// ============================================================================

class TimerPrecision : public HamsterballAPI {
private:
    IModAPI* api = nullptr;
    static inline bool g_enabled = true;

    // Our custom format string in DLL memory
    static const char* s_decimalFmt2;

    static void applyPatches(IModAPI* api) {
        DWORD base = (DWORD)GetModuleHandle(NULL);

        // --- Patch 1: Format string reference ---
        // At 0x421BB3: PUSH 0x4d03f0 (".%.1d")
        // Replace with PUSH <our ".%.2d">
        DWORD fmtAddr = (DWORD)s_decimalFmt2;
        unsigned char fmtPush[5] = { 0x68, 0, 0, 0, 0 };
        memcpy(&fmtPush[1], &fmtAddr, 4);
        api->PatchMemory(0x421BB3, (const char*)fmtPush, 5);

        // --- Patch 2: Decimal computation ---
        // At 0x421B8C..0x421BA5 (26 bytes), the code computes (timer/10)%10:
        //   B8 67 66 66 66   MOV EAX,0x66666667   ; magic multiplier for /10
        //   F7 E9            IMUL ECX             ; EDX:EAX = ECX * 0x66666667
        //   89 D0            MOV EAX,EDX          ; take high dword
        //   C1 F8 02         SAR EAX,2             ; shift
        //   89 C1            MOV ECX,EAX
        //   D1 E9            SHR ECX,1FH          ; sign fixup
        //   01 C8            ADD EAX,ECX          ; EAX = timer/10
        //   99               CDQ
        //   B9 0A 00 00 00   MOV ECX,10
        //   F7 F1            IDIV ECX             ; EDX = (timer/10)%10
        //
        // At entry to this block, ECX = timer value (loaded at 0x421B86)
        // We replace with: timer % 100
        //   8B C1            MOV EAX,ECX          ; EAX = timer
        //   99               CDQ                  ; sign extend for IDIV
        //   B9 64 00 00 00   MOV ECX,100
        //   F7 F1            IDIV ECX             ; EDX = timer % 100
        //   90 * 16          NOP (fill to 26 bytes)
        unsigned char compute[27] = {
            0x8B, 0xC1,                         // MOV EAX,ECX
            0x99,                                // CDQ
            0xB9, 0x64, 0x00, 0x00, 0x00,        // MOV ECX,100
            0xF7, 0xF1,                          // IDIV ECX (EDX=timer%100)
            0x90, 0x90, 0x90, 0x90, 0x90, 0x90,  // NOP *17
            0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
            0x90, 0x90, 0x90, 0x90, 0x90
        };
        api->PatchMemory(0x421B8C, (const char*)compute, 27);
    }

public:
    const char* GetModName() override { return "Timer Precision"; }
    const char* GetAuthorName() override { return "BookwormKevin"; }
    int GetApiVersion() override { return HAMSTERBALL_API_VERSION; }

    void Initialize(IModAPI* modApi) override {
        api = modApi;
        CustomButton btn;
        btn.id = "timer_precision";
        btn.displayText = "Timer Precision (2 decimals)";
        btn.defaultState = true;
        api->CreateToggleButton(btn, this);
        applyPatches(api);
    }

    void onButtonToggle(const char* id, bool state) override {
        // Re-patching on toggle is complex (need to restore originals).
        // For now, patches are applied at init and stay.
        // Toggle just controls whether future patches apply.
    }
};

const char* TimerPrecision::s_decimalFmt2 = ".%.2d";

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    return new TimerPrecision();
}
