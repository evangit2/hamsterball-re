// Ball Tint mod for Hamsterball Plus API — v4 DEBUG
// Fixes v3's broken frame counter (chicken-and-egg: tryApplyColors never ran).
// Now: ALL callbacks ALWAYS apply colors. Debug prints use separate counter.
// Console output: first-fire confirmation + periodic state dump.

#include "HamsterballAPI.h"
#include <windows.h>
#include <stdio.h>

static constexpr DWORD BOARD_COLOR_BASE   = 0x3AB0;
static constexpr DWORD BOARD_COLOR_STRIDE = 0x14;
static constexpr DWORD APP_PROFILE_OFFSET  = 0x220;
static constexpr DWORD PROFILE_BOARD_OFFSET = 0x0C;
static constexpr DWORD BOARD_VTABLE_MIN = 0x4D0000;
static constexpr DWORD BOARD_VTABLE_MAX = 0x4D2000;
static constexpr DWORD GLOBAL_APP_PTR = 0x5341E0;

class BallTintMod : public HamsterballAPI {
private:
    IModAPI* api = nullptr;
    int m_printCounter = 0;
    bool m_seenBallUpdate = false;
    bool m_seenGameUpdate = false;
    bool m_seenTextRender = false;

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

    DWORD findBoard() {
        if (!api) return 0;

        // Path 1: GetApp() → +0x220 → +0x0C
        App* app = api->GetApp();
        if (app) {
            DWORD appAddr = (DWORD)app;
            if (appAddr >= 0x10000 && !IsBadReadPtr((void*)(appAddr + APP_PROFILE_OFFSET), 4)) {
                DWORD profile = *(DWORD*)(appAddr + APP_PROFILE_OFFSET);
                if (profile && profile >= 0x10000 && !IsBadReadPtr((void*)(profile + PROFILE_BOARD_OFFSET), 4)) {
                    DWORD board = *(DWORD*)(profile + PROFILE_BOARD_OFFSET);
                    if (board && board >= 0x10000 && validateBoard(board)) return board;
                }
            }
        }

        // Path 2: GetScene()
        Scene* scene = api->GetScene();
        if (scene) {
            DWORD board = (DWORD)scene;
            if (validateBoard(board)) return board;
        }

        // Path 3: Direct global pointer (bass.dll method)
        DWORD globalApp = *(DWORD*)GLOBAL_APP_PTR;
        if (globalApp && globalApp >= 0x10000 && !IsBadReadPtr((void*)(globalApp + APP_PROFILE_OFFSET), 4)) {
            DWORD profile = *(DWORD*)(globalApp + APP_PROFILE_OFFSET);
            if (profile && profile >= 0x10000 && !IsBadReadPtr((void*)(profile + PROFILE_BOARD_OFFSET), 4)) {
                DWORD board = *(DWORD*)(profile + PROFILE_BOARD_OFFSET);
                if (board && board >= 0x10000 && validateBoard(board)) return board;
            }
        }

        return 0;
    }

    void applyAllColors(DWORD board) {
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

    void doDebugDump(const char* callbackName) {
        m_printCounter++;

        // Print first-fire confirmation for each callback
        bool isFirst = false;
        if (strcmp(callbackName, "BallUpdate") == 0 && !m_seenBallUpdate) {
            m_seenBallUpdate = true;
            isFirst = true;
        } else if (strcmp(callbackName, "GameUpdate") == 0 && !m_seenGameUpdate) {
            m_seenGameUpdate = true;
            isFirst = true;
        } else if (strcmp(callbackName, "TextRender") == 0 && !m_seenTextRender) {
            m_seenTextRender = true;
            isFirst = true;
        }
        if (isFirst) {
            printf("[BallTint] *** %s first fire! ***\n", callbackName);
        }

        // Every 120 frames, dump state
        if (m_printCounter % 120 != 0) return;

        App* app = api->GetApp();
        Scene* scene = api->GetScene();
        DWORD globalApp = *(DWORD*)GLOBAL_APP_PTR;

        printf("[BallTint] %s dump #%d\n", callbackName, m_printCounter);
        printf("  GetApp=%p GetScene=%p GlobalPtr=%08lX\n",
            (void*)app, (void*)scene, (unsigned long)globalApp);

        // Try to trace App → Profile → Board
        DWORD appAddr = 0;
        if (app) appAddr = (DWORD)app;
        else if (globalApp) appAddr = globalApp;

        if (appAddr && !IsBadReadPtr((void*)(appAddr + APP_PROFILE_OFFSET), 4)) {
            DWORD profile = *(DWORD*)(appAddr + APP_PROFILE_OFFSET);
            printf("  App=%08lX +0x220 -> profile=%08lX\n",
                (unsigned long)appAddr, (unsigned long)profile);
            if (profile && profile >= 0x10000 &&
                !IsBadReadPtr((void*)(profile + PROFILE_BOARD_OFFSET), 4)) {
                DWORD board = *(DWORD*)(profile + PROFILE_BOARD_OFFSET);
                printf("  profile+0xC -> board=%08lX\n", (unsigned long)board);
                if (board && board >= 0x10000 && !IsBadReadPtr((void*)board, 4)) {
                    DWORD vt = *(DWORD*)board;
                    printf("  board vtable=%08lX (valid=%s)\n",
                        (unsigned long)vt,
                        (vt >= BOARD_VTABLE_MIN && vt <= BOARD_VTABLE_MAX) ? "YES" : "NO");
                }
            }
        }

        DWORD foundBoard = findBoard();
        if (foundBoard) {
            printf("  findBoard() -> %08lX SUCCESS\n", (unsigned long)foundBoard);
            float p1r = api->GetSliderState("TINT_P1_R");
            float p1g = api->GetSliderState("TINT_P1_G");
            float p1b = api->GetSliderState("TINT_P1_B");
            printf("  P1 sliders: R=%.2f G=%.2f B=%.2f\n", p1r, p1g, p1b);
        } else {
            printf("  findBoard() -> FAILED\n");
        }
    }

public:
    const char* GetModName() override    { return "Ball Tint (Debug v4)"; }
    const char* GetAuthorName() override { return "Hamsterbot"; }
    const char* GetContributors() override { return "v4: fixed frame counter"; }
    int GetApiVersion() override         { return HAMSTERBALL_API_VERSION; }

    void Initialize(IModAPI* modApi) override {
        api = modApi;
        printf("[BallTint] Initialize() api=%p GetApp=%p GetScene=%p\n",
            (void*)modApi, (void*)modApi->GetApp(), (void*)modApi->GetScene());

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
    }

    void onLevelStart() override {
        printf("[BallTint] onLevelStart()\n");
    }

    void onBallUpdate(Ball* ball) override {
        doDebugDump("BallUpdate");
        DWORD board = findBoard();
        if (board) applyAllColors(board);
    }

    void onGameUpdate() override {
        doDebugDump("GameUpdate");
        DWORD board = findBoard();
        if (board) applyAllColors(board);
    }

    void onTextRenderLoop() override {
        doDebugDump("TextRender");
        DWORD board = findBoard();
        if (board) applyAllColors(board);
    }
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    return new BallTintMod();
}
