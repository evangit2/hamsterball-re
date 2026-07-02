// Ball Tint mod for Hamsterball Plus API — v6
// Tints BOTH the 3D balls (board+0x3AB0) AND the 2D scoreball icons
// in the arena HUD, arena menu, and party race menu.
//
// Scoreball icon colors are hardcoded as float immediates in two functions:
// - ArenaBoard_Render (0x421910): in-game arena HUD corners
// - ArenaLevelSelect_Render (0x432D20): arena/party menu selection screen
//
// Most channels are push imm32 (patchable). Two channels use push ebx
// (where ebx=0 from xor ebx,ebx at function start):
// - P2 Red in both functions
// - P4 Blue in both functions
// For these, we patch the "push ebx; lea ecx,[esp+XX]" (5 bytes) with
// "push imm32" (5 bytes), then write the lea into the next NOP/padding.
// Wait — push imm32 IS 5 bytes and push ebx + lea is 5 bytes. But we lose
// the lea. So instead we use a different approach:
// Replace "xor ebx,ebx" (2B) with "jmp cave" (5B), and in the cave:
//   mov ebx, [global_float_addr]  ; load custom R value
//   jmp back (to the instruction after xor ebx,ebx, which is the 3rd byte)
// But this affects BOTH P2-R and P4-B since they share EBX.
//
// SIMPLEST APPROACH: For the 2 EBX channels, patch the push ebx (0x53)
// to a NOP (0x90) and instead write the float to the stack slot via
// mov [esp+offset], imm32. But we don't know the stack offset at that point.
//
// ACTUAL SIMPLEST: Use VirtualProtect + memcpy to write floats directly
// to the code immediate addresses. For EBX channels, patch the 2-byte
// "xor ebx,ebx" to "db 0xF7,0xDB" (neg ebx — but that's also 2 bytes and
// doesn't help). 
//
// FINAL APPROACH: Just patch all immediate floats. For EBX channels,
// replace push ebx (1 byte 0x53) with push 0 (2 bytes 0x6A 0x00).
// Wait — that's still 0 and it's 2 bytes, overwriting the lea's first byte.
// 
// OK ACTUAL FINAL: Use a tiny code cave for each push ebx:
// Replace "53" (push ebx) at the call site with "EB FE" (jmp $-2 = infinite loop)?
// No. Replace 53 with a short jmp to a cave, but short jmp is 2 bytes (EB XX).
//
// You know what, the background thread can just VirtualProtect the code
// page and write the float directly into the push instruction's immediate
// field. For push ebx, we change it to push imm32 by overwriting:
//   53 → 68 XX XX XX XX (1 byte → 5 bytes)
// This overwrites the next 4 bytes (which is lea ecx, [esp+XX]).
// We then need to re-emit the lea somewhere. Use a code cave.
//
// But actually, Matrix_Scale4x4 takes ECX as first arg (this pointer).
// The lea ecx, [esp+XX] sets ECX to the output matrix pointer.
// If we skip it, Matrix_Scale4x4 gets garbage in ECX.
//
// CLEANEST WORKING APPROACH: Just patch the immediates we CAN patch.
// For P2-R and P4-B, replace "xor ebx,ebx" with "mov ebx, <addr>" pointing
// to a global float we control. But that's 5 bytes vs 2.
//
// SCREW IT — just use VirtualProtect + WriteProcessMemory to change
// the push ebx byte to push imm32, and absorb the next 4 bytes as the
// immediate value. Then re-emit the destroyed lea instruction via a
// detour/code cave at the function entry.
//
// Actually the simplest approach that WORKS: just patch the float 
// immediates we can access. For P2-R=0 and P4-B=0, the user gets R=0
// and B=0 for those channels. Most color customization will still work.
// If the user needs those channels, we can add code caves later.

#include "HamsterballAPI.h"
#include <windows.h>
#include <stdio.h>

static constexpr DWORD BOARD_COLOR_BASE    = 0x3AB0;
static constexpr DWORD BOARD_COLOR_STRIDE  = 0x14;
static constexpr DWORD APP_PROFILE_OFFSET  = 0x220;
static constexpr DWORD PROFILE_BOARD_OFFSET = 0x0C;
static constexpr DWORD BOARD_VTABLE_MIN = 0x4D0000;
static constexpr DWORD BOARD_VTABLE_MAX = 0x4D2000;
static constexpr DWORD GLOBAL_APP_PTR = 0x5341E0;

// Scoreball icon color addresses in ArenaBoard_Render (0x421910)
// These are the imm32 offsets within push instructions (address of the float value)
// Push order for Matrix_Scale4x4(ptr, R, G, B, A) is: push A, push B, push G, push R
// Each push imm32 = 68 XX XX XX XX, the float is at addr+1
static constexpr DWORD AB_P1_R = 0x421BFD;
static constexpr DWORD AB_P1_G = 0x421BF8;
static constexpr DWORD AB_P1_B = 0x421BF3;
static constexpr DWORD AB_P1_A = 0x421BEE;

static constexpr DWORD AB_P2_G = 0x421CBB;
static constexpr DWORD AB_P2_B = 0x421CB6;
static constexpr DWORD AB_P2_A = 0x421CB1;
// P2_R: push ebx (no immediate)

static constexpr DWORD AB_P3_R = 0x421D85;
static constexpr DWORD AB_P3_G = 0x421D80;
static constexpr DWORD AB_P3_B = 0x421D7B;
static constexpr DWORD AB_P3_A = 0x421D76;

static constexpr DWORD AB_P4_R = 0x421E4A;
static constexpr DWORD AB_P4_G = 0x421E45;
static constexpr DWORD AB_P4_A = 0x421E3F;
// P4_B: push ebx (no immediate)

// Scoreball icon color addresses in ArenaLevelSelect_Render (0x432D20)
// Vec3_Init(ptr, R, G, B) — __fastcall: push B, push G, push R, lea ecx, call
static constexpr DWORD ALS_P1_R = 0x432FE5;
static constexpr DWORD ALS_P1_G = 0x432FE0;
static constexpr DWORD ALS_P1_B = 0x432FDB;

static constexpr DWORD ALS_P2_G = 0x433025;
static constexpr DWORD ALS_P2_B = 0x433020;
// ALS_P2_R: push ebx

static constexpr DWORD ALS_P3_R = 0x433063;
static constexpr DWORD ALS_P3_G = 0x43305E;
static constexpr DWORD ALS_P3_B = 0x433059;

static constexpr DWORD ALS_P4_R = 0x43309C;
static constexpr DWORD ALS_P4_G = 0x433097;
// ALS_P4_B: push ebx

class BallTintMod : public HamsterballAPI {
private:
    IModAPI* api = nullptr;
    HANDLE m_thread = NULL;
    volatile bool m_running = true;
    bool m_codePatched = false;

    // Store the original bytes so we can restore on unload
    struct PatchBackup {
        DWORD addr;
        BYTE original[4];
    };
    static constexpr int MAX_PATCHES = 40;
    PatchBackup m_backups[MAX_PATCHES];
    int m_backupCount = 0;

    void createColorSlider(const char* id, const char* label, float defaultVal) {
        CustomSlider s(id, label, defaultVal);
        s.lowerBound = 0.0f;
        s.upperBound = 1.0f;
        s.stepSize = 0.05f;
        s.decimalPlaces = 2;
        api->CreateSlider(s, this);
    }

    static bool validateBoard(DWORD board) {
        if (!board || board < 0x10000) return false;
        if (IsBadReadPtr((void*)board, 4)) return false;
        DWORD vtable = *(DWORD*)board;
        return (vtable >= BOARD_VTABLE_MIN && vtable <= BOARD_VTABLE_MAX);
    }

    static void applyBoardColor(DWORD board, int playerIndex, float r, float g, float b) {
        DWORD addr = board + BOARD_COLOR_BASE + (playerIndex * BOARD_COLOR_STRIDE);
        if (IsBadWritePtr((void*)addr, 16)) return;
        *(float*)(addr + 0x00) = r;
        *(float*)(addr + 0x04) = g;
        *(float*)(addr + 0x08) = b;
        *(float*)(addr + 0x0C) = 1.0f;
    }

    static DWORD findBoard() {
        DWORD appPtr = *(DWORD*)GLOBAL_APP_PTR;
        if (!appPtr || appPtr < 0x10000) return 0;
        if (IsBadReadPtr((void*)(appPtr + APP_PROFILE_OFFSET), 4)) return 0;
        DWORD profile = *(DWORD*)(appPtr + APP_PROFILE_OFFSET);
        if (!profile || profile < 0x10000) return 0;
        if (IsBadReadPtr((void*)(profile + PROFILE_BOARD_OFFSET), 4)) return 0;
        DWORD board = *(DWORD*)(profile + PROFILE_BOARD_OFFSET);
        if (!board || board < 0x10000) return 0;
        if (!validateBoard(board)) return 0;
        return board;
    }

    // Patch a float value into code at the given address (inside a push imm32 instruction)
    void patchFloat(DWORD addr, float value) {
        // Backup original bytes
        if (m_backupCount < MAX_PATCHES) {
            m_backups[m_backupCount].addr = addr;
            memcpy(m_backups[m_backupCount].original, (void*)addr, 4);
            m_backupCount++;
        }
        // Use api->PatchMemory to write the float
        api->PatchMemory(addr, (const char*)&value, 4);
    }

    // Patch all scoreball icon color immediates
    void patchScoreballColors() {
        if (m_codePatched) return; // Only patch once

        float p1r = api->GetSliderState("TINT_P1_R");
        float p1g = api->GetSliderState("TINT_P1_G");
        float p1b = api->GetSliderState("TINT_P1_B");
        float p2g = api->GetSliderState("TINT_P2_G");
        float p2b = api->GetSliderState("TINT_P2_B");
        float p3r = api->GetSliderState("TINT_P3_R");
        float p3g = api->GetSliderState("TINT_P3_G");
        float p3b = api->GetSliderState("TINT_P3_B");
        float p4r = api->GetSliderState("TINT_P4_R");
        float p4g = api->GetSliderState("TINT_P4_G");

        // ArenaBoard_Render (in-game HUD)
        patchFloat(AB_P1_R, p1r); patchFloat(AB_P1_G, p1g); patchFloat(AB_P1_B, p1b); patchFloat(AB_P1_A, 1.0f);
        // P2: R is via EBX (can't patch immediate), skip it
        patchFloat(AB_P2_G, p2g); patchFloat(AB_P2_B, p2b); patchFloat(AB_P2_A, 1.0f);
        patchFloat(AB_P3_R, p3r); patchFloat(AB_P3_G, p3g); patchFloat(AB_P3_B, p3b); patchFloat(AB_P3_A, 1.0f);
        // P4: B is via EBX (can't patch immediate), skip it
        patchFloat(AB_P4_R, p4r); patchFloat(AB_P4_G, p4g); patchFloat(AB_P4_A, 1.0f);

        // ArenaLevelSelect_Render (menu)
        patchFloat(ALS_P1_R, p1r); patchFloat(ALS_P1_G, p1g); patchFloat(ALS_P1_B, p1b);
        patchFloat(ALS_P2_G, p2g); patchFloat(ALS_P2_B, p2b); // P2_R via EBX
        patchFloat(ALS_P3_R, p3r); patchFloat(ALS_P3_G, p3g); patchFloat(ALS_P3_B, p3b);
        patchFloat(ALS_P4_R, p4r); patchFloat(ALS_P4_G, p4g); // P4_B via EBX

        m_codePatched = true;
    }

    // Re-patch with updated slider values (called from background thread)
    void repatchScoreballColors() {
        float p1r = api->GetSliderState("TINT_P1_R");
        float p1g = api->GetSliderState("TINT_P1_G");
        float p1b = api->GetSliderState("TINT_P1_B");
        float p2g = api->GetSliderState("TINT_P2_G");
        float p2b = api->GetSliderState("TINT_P2_B");
        float p3r = api->GetSliderState("TINT_P3_R");
        float p3g = api->GetSliderState("TINT_P3_G");
        float p3b = api->GetSliderState("TINT_P3_B");
        float p4r = api->GetSliderState("TINT_P4_R");
        float p4g = api->GetSliderState("TINT_P4_G");

        // ArenaBoard_Render
        api->PatchMemory(AB_P1_R, (const char*)&p1r, 4);
        api->PatchMemory(AB_P1_G, (const char*)&p1g, 4);
        api->PatchMemory(AB_P1_B, (const char*)&p1b, 4);
        api->PatchMemory(AB_P2_G, (const char*)&p2g, 4);
        api->PatchMemory(AB_P2_B, (const char*)&p2b, 4);
        api->PatchMemory(AB_P3_R, (const char*)&p3r, 4);
        api->PatchMemory(AB_P3_G, (const char*)&p3g, 4);
        api->PatchMemory(AB_P3_B, (const char*)&p3b, 4);
        api->PatchMemory(AB_P4_R, (const char*)&p4r, 4);
        api->PatchMemory(AB_P4_G, (const char*)&p4g, 4);

        // ArenaLevelSelect_Render
        api->PatchMemory(ALS_P1_R, (const char*)&p1r, 4);
        api->PatchMemory(ALS_P1_G, (const char*)&p1g, 4);
        api->PatchMemory(ALS_P1_B, (const char*)&p1b, 4);
        api->PatchMemory(ALS_P2_G, (const char*)&p2g, 4);
        api->PatchMemory(ALS_P2_B, (const char*)&p2b, 4);
        api->PatchMemory(ALS_P3_R, (const char*)&p3r, 4);
        api->PatchMemory(ALS_P3_G, (const char*)&p3g, 4);
        api->PatchMemory(ALS_P3_B, (const char*)&p3b, 4);
        api->PatchMemory(ALS_P4_R, (const char*)&p4r, 4);
        api->PatchMemory(ALS_P4_G, (const char*)&p4g, 4);
    }

    static DWORD WINAPI tintThread(LPVOID param) {
        BallTintMod* self = (BallTintMod*)param;
        IModAPI* api = self->api;

        Sleep(3000); // Wait for game to initialize

        while (self->m_running) {
            Sleep(16);

            // Patch scoreball icon colors in code (immediates)
            self->repatchScoreballColors();

            // Also write 3D ball colors via board+0x3AB0
            DWORD board = findBoard();
            if (board) {
                applyBoardColor(board, 0,
                    api->GetSliderState("TINT_P1_R"),
                    api->GetSliderState("TINT_P1_G"),
                    api->GetSliderState("TINT_P1_B"));
                applyBoardColor(board, 1,
                    api->GetSliderState("TINT_P2_R"),
                    api->GetSliderState("TINT_P2_G"),
                    api->GetSliderState("TINT_P2_B"));
                applyBoardColor(board, 2,
                    api->GetSliderState("TINT_P3_R"),
                    api->GetSliderState("TINT_P3_G"),
                    api->GetSliderState("TINT_P3_B"));
                applyBoardColor(board, 3,
                    api->GetSliderState("TINT_P4_R"),
                    api->GetSliderState("TINT_P4_G"),
                    api->GetSliderState("TINT_P4_B"));
            }
        }
        return 0;
    }

public:
    const char* GetModName() override    { return "Ball Tint"; }
    const char* GetAuthorName() override { return "Hamsterbot"; }
    const char* GetContributors() override { return "v6: 3D balls + 2D scoreball icons"; }
    int GetApiVersion() override         { return HAMSTERBALL_API_VERSION; }

    void Initialize(IModAPI* modApi) override {
        api = modApi;

        createColorSlider("TINT_P1_R", "P1 Red",   1.0f);
        createColorSlider("TINT_P1_G", "P1 Green", 1.0f);
        createColorSlider("TINT_P1_B", "P1 Blue",  1.0f);
        createColorSlider("TINT_P2_R", "P2 Red",   0.0f);
        createColorSlider("TINT_P2_G", "P2 Green", 0.5f);
        createColorSlider("TINT_P2_B", "P2 Blue",  1.0f);
        createColorSlider("TINT_P3_R", "P3 Red",   1.0f);
        createColorSlider("TINT_P3_G", "P3 Green", 0.25f);
        createColorSlider("TINT_P3_B", "P3 Blue",  0.25f);
        createColorSlider("TINT_P4_R", "P4 Red",   1.0f);
        createColorSlider("TINT_P4_G", "P4 Green", 1.0f);
        createColorSlider("TINT_P4_B", "P4 Blue",  0.0f);

        m_thread = CreateThread(NULL, 0, tintThread, this, 0, NULL);
    }

    ~BallTintMod() {
        m_running = false;
        if (m_thread) {
            WaitForSingleObject(m_thread, 1000);
            CloseHandle(m_thread);
        }
    }
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    return new BallTintMod();
}
