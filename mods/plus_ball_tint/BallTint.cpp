// Ball Tint mod for Hamsterball Plus API — DEBUG VERSION
// Adds RGB sliders + tries every callback + prints debug info to console.
//
// Enable console via ModConfig.ini: [Config] ShowConsole=1
//
// v3: Heavy debug logging. Tries onBallUpdate, onGameUpdate, AND onTextRenderLoop.
//     Tries both GetApp() path AND GetScene() path. Prints what works.

#include "HamsterballAPI.h"
#include <windows.h>
#include <stdio.h>

static constexpr DWORD BOARD_COLOR_BASE   = 0x3AB0;
static constexpr DWORD BOARD_COLOR_STRIDE = 0x14;
static constexpr DWORD APP_PROFILE_OFFSET  = 0x220;
static constexpr DWORD PROFILE_BOARD_OFFSET = 0x0C;
static constexpr DWORD BOARD_VTABLE_MIN = 0x4D0000;
static constexpr DWORD BOARD_VTABLE_MAX = 0x4D2000;

// The real global App pointer address (bass.dll reads from here)
static constexpr DWORD GLOBAL_APP_PTR = 0x5341E0;

static int g_frame_count = 0;

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

    // Path 1: GetApp() → +0x220 → +0x0C
    DWORD findBoardViaApp() {
        if (!api) return 0;
        App* app = api->GetApp();
        if (!app) {
            return 0;
        }
        DWORD appAddr = (DWORD)app;
        if (appAddr < 0x10000) return 0;
        if (IsBadReadPtr((void*)(appAddr + APP_PROFILE_OFFSET), 4)) return 0;
        DWORD profile = *(DWORD*)(appAddr + APP_PROFILE_OFFSET);
        if (!profile || profile < 0x10000) return 0;
        if (IsBadReadPtr((void*)(profile + PROFILE_BOARD_OFFSET), 4)) return 0;
        DWORD board = *(DWORD*)(profile + PROFILE_BOARD_OFFSET);
        if (!board || board < 0x10000) return 0;
        if (!validateBoard(board)) return 0;
        return board;
    }

    // Path 2: GetScene() (g_Scene from ball->scene)
    DWORD findBoardViaScene() {
        if (!api) return 0;
        Scene* scene = api->GetScene();
        if (!scene) return 0;
        DWORD board = (DWORD)scene;
        if (!validateBoard(board)) return 0;
        return board;
    }

    // Path 3: Direct global pointer at 0x5341E0 (bass.dll method)
    DWORD findBoardViaGlobal() {
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

    void tryApplyColors(const char* callbackName) {
        if (!api) return;
        g_frame_count++;

        // Try all three paths
        DWORD board = findBoardViaApp();
        const char* method = "GetApp";
        if (!board) {
            board = findBoardViaScene();
            method = "GetScene";
        }
        if (!board) {
            board = findBoardViaGlobal();
            method = "Global";
        }

        // Print debug info every 60 frames (~1 second)
        if (g_frame_count % 60 == 1) {
            App* app = api->GetApp();
            Scene* scene = api->GetScene();
            DWORD globalApp = *(DWORD*)GLOBAL_APP_PTR;

            printf("[BallTint] %s frame=%d\n", callbackName, g_frame_count);
            printf("  GetApp()=%p  GetScene()=%p  GlobalApp(0x5341E0)=%08X\n",
                (void*)app, (void*)scene, globalApp);

            if (app) {
                DWORD appAddr = (DWORD)app;
                if (!IsBadReadPtr((void*)(appAddr + APP_PROFILE_OFFSET), 4)) {
                    DWORD profile = *(DWORD*)(appAddr + APP_PROFILE_OFFSET);
                    printf("  App+0x220(profile)=%08X", profile);
                    if (profile && profile > 0x10000 &&
                        !IsBadReadPtr((void*)(profile + PROFILE_BOARD_OFFSET), 4)) {
                        DWORD b = *(DWORD*)(profile + PROFILE_BOARD_OFFSET);
                        printf(" profile+0xC(board)=%08X", b);
                        if (b && b > 0x10000 && !IsBadReadPtr((void*)b, 4)) {
                            printf(" board_vtable=%08X", *(DWORD*)b);
                        }
                    }
                    printf("\n");
                } else {
                    printf("  App+0x220 unreadable\n");
                }
            }

            if (globalApp) {
                if (!IsBadReadPtr((void*)(globalApp + APP_PROFILE_OFFSET), 4)) {
                    DWORD profile = *(DWORD*)(globalApp + APP_PROFILE_OFFSET);
                    DWORD b = 0;
                    if (profile && profile > 0x10000 &&
                        !IsBadReadPtr((void*)(profile + PROFILE_BOARD_OFFSET), 4)) {
                        b = *(DWORD*)(profile + PROFILE_BOARD_OFFSET);
                    }
                    printf("  GlobalApp: profile=%08X board=%08X\n", profile, b);
                }
            }

            if (board) {
                printf("  FOUND board=%08X via %s — applying colors\n", board, method);
            } else {
                printf("  NO BOARD FOUND via any method\n");
            }
        }

        if (!board) return;

        // Read current slider values
        float p1r = api->GetSliderState("TINT_P1_R");
        float p1g = api->GetSliderState("TINT_P1_G");
        float p1b = api->GetSliderState("TINT_P1_B");

        // Print slider values every 120 frames
        if (g_frame_count % 120 == 1) {
            printf("  Sliders: P1(%.2f,%.2f,%.2f)\n", p1r, p1g, p1b);
        }

        applyColor(board, 0, p1r, p1g, p1b);
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

public:
    const char* GetModName() override    { return "Ball Tint (Debug)"; }
    const char* GetAuthorName() override { return "Hamsterbot"; }
    const char* GetContributors() override { return "v3 debug build"; }
    int GetApiVersion() override         { return HAMSTERBALL_API_VERSION; }

    void Initialize(IModAPI* modApi) override {
        api = modApi;
        printf("[BallTint] Initialize() called, api=%p\n", (void*)api);

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

        printf("[BallTint] Sliders registered. GetApp()=%p GetScene()=%p\n",
            (void*)api->GetApp(), (void*)api->GetScene());
    }

    void onLevelStart() override {
        printf("[BallTint] onLevelStart() called\n");
    }

    void onBallUpdate(Ball* ball) override {
        // Only log occasionally — this fires every frame
        if (g_frame_count % 60 == 1) {
            tryApplyColors("onBallUpdate");
        } else {
            // Still apply colors silently
            DWORD board = findBoardViaApp();
            if (!board) board = findBoardViaScene();
            if (!board) board = findBoardViaGlobal();
            if (board) {
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
        }
    }

    void onGameUpdate() override {
        if (g_frame_count % 60 == 1) {
            tryApplyColors("onGameUpdate");
        }
    }

    void onTextRenderLoop() override {
        if (g_frame_count % 60 == 1) {
            tryApplyColors("onTextRenderLoop");
        }
    }
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    return new BallTintMod();
}
