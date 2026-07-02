// Ball Tint mod for Hamsterball Plus API
// Adds RGB sliders in the options menu to customize player ball colors.
// Sliders: P1 R/G/B, P2 R/G/B, P3 R/G/B, P4 R/G/B (0.0-1.0 each)
// Uses onTextRenderLoop to write colors AFTER all game logic.
// Ported from bass.dll proxy ball_tint v5 (mods/ball_tint/).
//
// v2 fix: Uses GetApp() + App->+0x220->+0x0C board-finding path
// (identical to the working bass.dll version) instead of GetScene().
// GetScene() returns g_Scene which is only set during Ball_Update
// and is cleared on SceneDtor — it can be null during transitions.
// GetApp() returns the global App pointer which is always valid.

#include "HamsterballAPI.h"
#include <windows.h>

// Board color table offsets (from original ball_tint mod)
static constexpr DWORD BOARD_COLOR_BASE   = 0x3AB0;
static constexpr DWORD BOARD_COLOR_STRIDE = 0x14;   // 20 bytes per player entry

// App struct offsets (proven path from bass.dll version)
static constexpr DWORD APP_PROFILE_OFFSET  = 0x220;  // App+0x220 = PlayerProfile*
static constexpr DWORD PROFILE_BOARD_OFFSET = 0x0C; // profile+0x0C = Board*

// Board vtable range for validation (covers all board types including Arena)
static constexpr DWORD BOARD_VTABLE_MIN = 0x4D0000;
static constexpr DWORD BOARD_VTABLE_MAX = 0x4D2000;

class BallTintMod : public HamsterballAPI {
private:
    IModAPI* api = nullptr;

    void createColorSlider(const char* id, const char* label, float defaultVal) {
        CustomSlider s(id, label, defaultVal);
        s.lowerBound = 0.0f;
        s.upperBound = 1.0f;
        s.stepSize = 0.05f;
        s.decimalPlaces = 2;
        api->CreateSlider(s, this);
    }

    void applyColor(DWORD board, int playerIndex, float r, float g, float b) {
        DWORD addr = board + BOARD_COLOR_BASE + (playerIndex * BOARD_COLOR_STRIDE);
        if (IsBadWritePtr((void*)addr, 16)) return;
        *(float*)(addr + 0x00) = r;
        *(float*)(addr + 0x04) = g;
        *(float*)(addr + 0x08) = b;
        *(float*)(addr + 0x0C) = 1.0f;
    }

    bool validateBoard(DWORD board) {
        if (!board || board < 0x10000) return false;
        if (IsBadReadPtr((void*)board, 4)) return false;
        DWORD vtable = *(DWORD*)board;
        if (vtable < BOARD_VTABLE_MIN || vtable > BOARD_VTABLE_MAX) return false;
        return true;
    }

    // Find board via the proven App->+0x220->+0x0C path.
    // This is the EXACT same method used by the working bass.dll version.
    DWORD findBoard() {
        if (!api) return 0;

        App* app = api->GetApp();
        if (!app) return 0;
        DWORD appAddr = (DWORD)app;
        if (appAddr < 0x10000) return 0;
        if (IsBadReadPtr((void*)(appAddr + APP_PROFILE_OFFSET), 4)) return 0;

        DWORD profile = *(DWORD*)(appAddr + APP_PROFILE_OFFSET);
        if (!profile || profile < 0x10000) return 0;
        if (IsBadReadPtr((void*)(profile + PROFILE_BOARD_OFFSET), 4)) return 0;

        DWORD board = *(DWORD*)(profile + PROFILE_BOARD_OFFSET);
        if (!board || board < 0x10000) return 0;
        if (!validateBoard(board)) return 0;
        if (IsBadReadPtr((void*)board, BOARD_COLOR_BASE + 0x40)) return 0;

        return board;
    }

public:
    const char* GetModName() override    { return "Ball Tint"; }
    const char* GetAuthorName() override { return "Hamsterbot"; }
    const char* GetContributors() override { return "ported from bass.dll ball_tint v5"; }
    int GetApiVersion() override         { return HAMSTERBALL_API_VERSION; }

    void Initialize(IModAPI* modApi) override {
        api = modApi;

        // Player 1 (default: white)
        createColorSlider("TINT_P1_R", "P1 Red",   1.0f);
        createColorSlider("TINT_P1_G", "P1 Green", 1.0f);
        createColorSlider("TINT_P1_B", "P1 Blue",  1.0f);

        // Player 2 (default: blue)
        createColorSlider("TINT_P2_R", "P2 Red",   0.0f);
        createColorSlider("TINT_P2_G", "P2 Green", 0.5f);
        createColorSlider("TINT_P2_B", "P2 Blue",  1.0f);

        // Player 3 (default: salmon)
        createColorSlider("TINT_P3_R", "P3 Red",   1.0f);
        createColorSlider("TINT_P3_G", "P3 Green", 0.25f);
        createColorSlider("TINT_P3_B", "P3 Blue",  0.25f);

        // Player 4 (default: yellow)
        createColorSlider("TINT_P4_R", "P4 Red",   1.0f);
        createColorSlider("TINT_P4_G", "P4 Green", 1.0f);
        createColorSlider("TINT_P4_B", "P4 Blue",  0.0f);
    }

    // Use onTextRenderLoop — runs after all game logic, right before render.
    void onTextRenderLoop() override {
        if (!api) return;

        DWORD board = findBoard();
        if (!board) return;

        // Apply all 4 player colors
        applyColor(board, 0,
            api->GetSliderState("TINT_P1_R"),
            api->GetSliderState("TINT_P1_G"),
            api->GetSliderState("TINT_P1_B"));

        applyColor(board, 1,
            api->GetSliderState("TINT_P2_R"),
            api->GetSliderState("TINT_P2_G"),
            api->GetSliderState("TINT_P2_B"));

        applyColor(board, 2,
            api->GetSliderState("TINT_P3_R"),
            api->GetSliderState("TINT_P3_G"),
            api->GetSliderState("TINT_P3_B"));

        applyColor(board, 3,
            api->GetSliderState("TINT_P4_R"),
            api->GetSliderState("TINT_P4_G"),
            api->GetSliderState("TINT_P4_B"));
    }
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    return new BallTintMod();
}
