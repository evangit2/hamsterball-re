/*
 * JumpMod.cpp — Jump Mod for HB+ v2.1
 *
 * Port of jump_mod_raycast v22 (bass.dll proxy) to HB+ API.
 *
 * Simplifications vs bass.dll version:
 * - Input thread polling DirectInput → WasKeyPressed(DIK_SPACE) in onBallUpdate
 * - Raycast via Mesh_FindClosestCollision → LevelRaycastHit (HB+ API wrapper)
 * - Phase 15 code cave (asm FLD/FADD/FSTP) → direct write to ball+0x174
 * - BASS proxy shell removed entirely
 *
 * Features (same as v22):
 * - Press SPACE to jump (Player 1 only)
 * - Raycast ground detection (straight down, slope-aware threshold)
 * - Countdown gating (no jumping during Ready/Set/Go)
 * - Race-end gating (no jumping after touching goal)
 * - Rising-edge detection (one jump per keypress)
 *
 * Author: RodentRacer / Hamsterbot
 */
#include "HamsterballAPI.h"
#include <windows.h>
#include <math.h>

/* DirectInput keycodes */
#define DIK_SPACE 0x39

/* ═══════════════════════════════════════════════════════════════════════════
 * Constants
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Ball struct offsets */
#define BALL_POS_Y              0x168
#define BALL_PHYS_PTR           0x1A4
#define BALL_RADIUS             0x284
#define BALL_FORCE_Y            0x174   /* Y force accumulator (impulse target) */
#define BALL_FREEZE_FLAG        0x14C   /* BYTE: set when race ended */

/* Scene offset */
#define SCENE_COUNTDOWN_DONE    0x3A4C  /* BYTE: 1 when countdown finished */

/* Slope-aware threshold: covers slopes up to 45deg (cos(45)≈0.707, r/0.707≈1.414r) */
static constexpr float GROUND_SLOPE_FACTOR = 1.45f;

/* ═══════════════════════════════════════════════════════════════════════════
 * Config
 * ═══════════════════════════════════════════════════════════════════════════ */

static float g_jump_impulse = 20.0f;
static bool  g_enabled = true;
static bool  g_prev_space = false;

/* ═══════════════════════════════════════════════════════════════════════════
 * Mod class
 * ═══════════════════════════════════════════════════════════════════════════ */

class JumpMod : public HamsterballAPI {
    IModAPI* api = nullptr;

public:
    const char* GetModName() override      { return "Jump Mod"; }
    const char* GetAuthorName() override   { return "RodentRacer"; }
    const char* GetContributors() override  { return "Hamsterbot"; }
    int GetApiVersion() override           { return HAMSTERBALL_API_VERSION; }

    void Initialize(IModAPI* modApi) override {
        api = modApi;

        CustomButton btn("JUMP_MOD", "Jump (Space)");
        btn.defaultState = true;
        modApi->CreateToggleButton(btn, this);

        CustomSlider slider("JUMP_IMPULSE", "Jump Force", g_jump_impulse);
        slider.lowerBound = 5.0f;
        slider.upperBound = 50.0f;
        slider.stepSize = 1.0f;
        slider.decimalPlaces = 1;
        modApi->CreateSlider(slider, this);
    }

    void onButtonToggle(const char* buttonId, bool newState) override {
        if (strcmp(buttonId, "JUMP_MOD") == 0) {
            g_enabled = newState;
        }
    }

    void onSliderChange(const char* sliderId, float value) override {
        if (strcmp(sliderId, "JUMP_IMPULSE") == 0) {
            g_jump_impulse = value;
        }
    }

    void onBallUpdate(Ball* ball) override {
        if (!g_enabled || !ball || !api) return;

        /* Only Player 1 */
        if (ball->playerID != 0) return;

        /* Check if spacebar is pressed */
        bool space_down = api->WasKeyPressed(DIK_SPACE);
        if (!space_down || g_prev_space) {
            g_prev_space = space_down;
            return;
        }
        g_prev_space = space_down;

        /* Gate 1: Countdown — Scene+0x3A4C must be 1 */
        DWORD ballAddr = (DWORD)ball;
        DWORD scene = *(DWORD*)(ballAddr + 0x14);
        if (!scene) return;
        if (!IsBadReadPtr((void*)(scene + SCENE_COUNTDOWN_DONE), 1)) {
            if (!*(BYTE*)(scene + SCENE_COUNTDOWN_DONE)) return;
        }

        /* Gate 2: Race end — ball+0x14C (freeze flag) */
        if (*(BYTE*)(ballAddr + BALL_FREEZE_FLAG)) return;

        /* Gate 3: App+0x5D6 (goal flag) */
        DWORD app = *(DWORD*)0x005341E0;
        if (app && !IsBadReadPtr((void*)(app + 0x5D6), 1)) {
            if (*(BYTE*)(app + 0x5D6)) return;
        }

        /* Raycast ground check: straight down from ball position */
        Vec3 pos(ball->position.x, ball->position.y, ball->position.z);
        Vec3 down(0.0f, -1.0f, 0.0f);
        float radius = ball->radius;
        float max_dist = radius * GROUND_SLOPE_FACTOR;

        bool grounded = api->LevelRaycastHit(pos, down, 1.0f, max_dist);
        if (!grounded) return;

        /* Apply jump impulse to Y force accumulator */
        *(float*)(ballAddr + BALL_FORCE_Y) += g_jump_impulse;
    }

    void onSceneEnd() override {
        g_prev_space = false;
    }

    void onLevelStart() override {
        g_prev_space = false;
    }
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    return new JumpMod();
}
