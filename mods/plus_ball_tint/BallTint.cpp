// Ball Tint mod for Hamsterball Plus API
// Adds RGB sliders in the options menu to customize player ball colors.
// Sliders: P1 R/G/B, P2 R/G/B, P3 R/G/B, P4 R/G/B (0.0-1.0 each)
// Uses onBallUpdate to write colors to board+0x3AB0 every frame.
// Ported from bass.dll proxy ball_tint v5 (mods/ball_tint/).

#include "HamsterballAPI.h"
#include <windows.h>

// Board color table offsets (from original ball_tint mod)
static constexpr DWORD BOARD_COLOR_BASE   = 0x3AB0;
static constexpr DWORD BOARD_COLOR_STRIDE = 0x14;   // 20 bytes per player entry

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

    void onBallUpdate(Ball* ball) override {
        if (!api || !ball) return;

        // Get board from ball->scene (ball+0x14 = Scene*, scene is the board)
        // This is the same pointer the bass.dll version finds via App->Profile->Board
        DWORD board = (DWORD)ball->scene;
        if (!board || board < 0x10000) return;
        if (IsBadReadPtr((void*)board, BOARD_COLOR_BASE + 0x40)) return;

        // Apply all 4 player colors every frame
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
