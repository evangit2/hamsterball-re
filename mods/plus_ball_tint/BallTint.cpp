// Ball Tint mod for Hamsterball Plus API
// Adds RGB sliders in the options menu to customize player ball colors.
// Sliders: P1 R/G/B, P2 R/G/B (0.0-1.0 each)
// Uses onGameUpdate to write colors to board+0x3AB0 every frame.
// Ported from bass.dll proxy ball_tint v4 (mods/ball_tint/).

#include "HamsterballAPI.h"
#include <windows.h>

// Board color table offsets (from original ball_tint mod)
static constexpr DWORD BOARD_COLOR_BASE   = 0x3AB0;
static constexpr DWORD BOARD_COLOR_STRIDE = 0x14;   // 20 bytes per player entry

class BallTintMod : public HamsterballAPI {
private:
    IModAPI* api = nullptr;

public:
    const char* GetModName() override    { return "Ball Tint"; }
    const char* GetAuthorName() override { return "Hamsterbot"; }
    const char* GetContributors() override { return "ported from bass.dll ball_tint v4"; }
    int GetApiVersion() override         { return HAMSTERBALL_API_VERSION; }

    void Initialize(IModAPI* modApi) override {
        api = modApi;

        // Player 1 color sliders
        CustomSlider p1r("TINT_P1_R", "P1 Red",   1.0f);
        p1r.lowerBound = 0.0f; p1r.upperBound = 1.0f; p1r.stepSize = 0.05f;
        p1r.decimalPlaces = 2;
        api->CreateSlider(p1r, this);

        CustomSlider p1g("TINT_P1_G", "P1 Green",  1.0f);
        p1g.lowerBound = 0.0f; p1g.upperBound = 1.0f; p1g.stepSize = 0.05f;
        p1g.decimalPlaces = 2;
        api->CreateSlider(p1g, this);

        CustomSlider p1b("TINT_P1_B", "P1 Blue",   1.0f);
        p1b.lowerBound = 0.0f; p1b.upperBound = 1.0f; p1b.stepSize = 0.05f;
        p1b.decimalPlaces = 2;
        api->CreateSlider(p1b, this);

        // Player 2 color sliders
        CustomSlider p2r("TINT_P2_R", "P2 Red",   0.0f);
        p2r.lowerBound = 0.0f; p2r.upperBound = 1.0f; p2r.stepSize = 0.05f;
        p2r.decimalPlaces = 2;
        api->CreateSlider(p2r, this);

        CustomSlider p2g("TINT_P2_G", "P2 Green",  0.5f);
        p2g.lowerBound = 0.0f; p2g.upperBound = 1.0f; p2g.stepSize = 0.05f;
        p2g.decimalPlaces = 2;
        api->CreateSlider(p2g, this);

        CustomSlider p2b("TINT_P2_B", "P2 Blue",   1.0f);
        p2b.lowerBound = 0.0f; p2b.upperBound = 1.0f; p2b.stepSize = 0.05f;
        p2b.decimalPlaces = 2;
        api->CreateSlider(p2b, this);
    }

    void onGameUpdate() override {
        if (!api) return;

        App* app = api->GetApp();
        if (!app) return;

        // Find board via App -> PlayerProfile(+0x220) -> Board(+0x0C)
        DWORD appAddr = (DWORD)app;
        DWORD profile = *(DWORD*)(appAddr + 0x220);
        if (!profile || profile < 0x10000) return;
        if (IsBadReadPtr((void*)(profile + 0x0C), 4)) return;

        DWORD board = *(DWORD*)(profile + 0x0C);
        if (!board || board < 0x10000) return;
        if (IsBadReadPtr((void*)board, BOARD_COLOR_BASE + 0x40)) return;

        // Read slider values
        float p1r = api->GetSliderState("TINT_P1_R");
        float p1g = api->GetSliderState("TINT_P1_G");
        float p1b = api->GetSliderState("TINT_P1_B");

        float p2r = api->GetSliderState("TINT_P2_R");
        float p2g = api->GetSliderState("TINT_P2_G");
        float p2b = api->GetSliderState("TINT_P2_B");

        // Write P1 color (board+0x3AB0 = 4 floats: R, G, B, A)
        DWORD p1Addr = board + BOARD_COLOR_BASE;
        if (!IsBadWritePtr((void*)p1Addr, 16)) {
            *(float*)(p1Addr + 0x00) = p1r;
            *(float*)(p1Addr + 0x04) = p1g;
            *(float*)(p1Addr + 0x08) = p1b;
            *(float*)(p1Addr + 0x0C) = 1.0f;
        }

        // Write P2 color (board+0x3AB0 + 0x14 = board+0x3AC4)
        DWORD p2Addr = board + BOARD_COLOR_BASE + BOARD_COLOR_STRIDE;
        if (!IsBadWritePtr((void*)p2Addr, 16)) {
            *(float*)(p2Addr + 0x00) = p2r;
            *(float*)(p2Addr + 0x04) = p2g;
            *(float*)(p2Addr + 0x08) = p2b;
            *(float*)(p2Addr + 0x0C) = 1.0f;
        }
    }
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    return new BallTintMod();
}
