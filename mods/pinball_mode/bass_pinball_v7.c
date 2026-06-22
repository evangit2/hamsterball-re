// ============================================================
// Hamsterball Pinball Mode - bass.dll proxy (v7)
//
// v6 failed because:
// 1. Wrong ball pointer: used Scene+0x2DE0 but AthenaList is at Scene+0x29D4
// 2. Writing position (+0x164) is useless - FallUpdate overwrites it next
//    frame from internal velocity (+0xC98/+0xC9C/+0xCA0)
//
// v7 fixes:
// 1. Correct ball pointer resolution via Scene+0x29D4 AthenaList
//    Also has fallback: ball+0x14 → board → scene chain
// 2. Modify VELOCITY (+0xC98/+0xC9C/+0xCA0) not position
//    After detecting direction reversal (collision), multiply the
//    velocity vector by bounce_mult. This persists across frames
//    because FallUpdate uses +0xC98 as current velocity.
// 3. MessageBoxA debug on first successful ball read + first collision
//    so user can verify it's working.
//
// F8: Toggle pinball mode on/off
// F9: Cycle bounce multiplier 2x -> 3x -> 5x -> 10x -> 2x
// ============================================================

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>

/* ---- BASS Proxy: Lazy load bass_real.dll on first call ---- */

static HMODULE g_hRealBass = NULL;
static int g_bass_tried_load = 0;

static void load_real_bass(void) {
    if (g_bass_tried_load) return;
    g_bass_tried_load = 1;
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    char* slash = strrchr(path, '\\');
    if (slash) strcpy(slash + 1, "bass_real.dll");
    else strcpy(path, "bass_real.dll");
    g_hRealBass = LoadLibraryA(path);
}

#define DEFINE_BASS_FORWARDED(name, ret_type, params, args, stub_ret) \
    typedef ret_type (__stdcall *name##_t) params; \
    static name##_t real_##name = NULL; \
    __declspec(dllexport) ret_type __stdcall name params { \
        if (!g_bass_tried_load) load_real_bass(); \
        if (g_hRealBass && !real_##name) \
            real_##name = (name##_t)GetProcAddress(g_hRealBass, #name); \
        if (real_##name) return real_##name args; \
        return stub_ret; \
    }

DEFINE_BASS_FORWARDED(BASS_Init, int, (int a, DWORD b, DWORD c, HWND d, void* e), (a, b, c, d, e), 1)
DEFINE_BASS_FORWARDED(BASS_Free, void, (void), (), )
DEFINE_BASS_FORWARDED(BASS_Start, int, (void), (), 1)
DEFINE_BASS_FORWARDED(BASS_Stop, int, (void), (), 0)
DEFINE_BASS_FORWARDED(BASS_SetConfig, int, (DWORD a, DWORD b), (a, b), 1)
DEFINE_BASS_FORWARDED(BASS_ErrorGetCode, int, (void), (), 0)
DEFINE_BASS_FORWARDED(BASS_MusicLoad, int, (BOOL a, const void* b, unsigned long long c, DWORD d, DWORD e, DWORD f), (a, b, c, d, e, f), 0)
DEFINE_BASS_FORWARDED(BASS_MusicPlayEx, int, (DWORD a, DWORD b, BOOL c), (a, b, c), 1)
DEFINE_BASS_FORWARDED(BASS_MusicFree, int, (DWORD a), (a), 0)
DEFINE_BASS_FORWARDED(BASS_ChannelSetAttributes, int, (DWORD a, float b, int c, int d), (a, b, c, d), 1)
DEFINE_BASS_FORWARDED(BASS_ChannelStop, int, (DWORD a), (a), 1)

/* ════════════════════════════════════════════════════════════════════
 * Pinball Mode v7 - Background Thread, Velocity Modification
 * ════════════════════════════════════════════════════════════════════ */

static const float BOUNCE_MULTS[] = {2.0f, 3.0f, 5.0f, 10.0f};
static const int NUM_MULTS = 4;

static volatile LONG g_pinball_enabled = 0;
static volatile float g_bounce_mult = 3.0f;
static volatile int g_mult_index = 1;

/* Ball struct offsets */
#define BALL_POS_X     0x164
#define BALL_POS_Y     0x168
#define BALL_POS_Z     0x16C
#define BALL_VEL_X     0xC98   /* persistent velocity (used by FallUpdate) */
#define BALL_VEL_Y     0xC9C
#define BALL_VEL_Z     0xCA0

/* App/Scene offsets */
#define APP_PTR        0x5341E0
#define APP_SCENE      0x178   /* App+0x178 = Scene pointer */
#define SCENE_BALLLIST 0x29D4  /* Scene+0x29D4 = Ball AthenaList */

#define DIK_F8  0x42
#define DIK_F9  0x43

static int SafeReadFloats(DWORD addr, float* out, int count) {
    if (!addr || IsBadReadPtr((void*)addr, count * 4)) return 0;
    for (int i = 0; i < count; i++) out[i] = *(float*)(addr + i * 4);
    return 1;
}

static void SafeWriteFloats(DWORD addr, float* vals, int count) {
    if (!addr || IsBadReadPtr((void*)addr, count * 4)) return;
    for (int i = 0; i < count; i++) *(float*)(addr + i * 4) = vals[i];
}

/* Get player ball pointer - try multiple methods */
static DWORD GetPlayerBall(void) {
    DWORD app = *(DWORD*)APP_PTR;
    if (!app || IsBadReadPtr((void*)(app + APP_SCENE), 4)) return 0;
    DWORD scene = *(DWORD*)(app + APP_SCENE);
    if (!scene) return 0;

    /* Method 1: Scene+0x29D4 = AthenaList
     * AthenaList struct: [count(4)] [items_ptr(4)] or [count(4)] [capacity(4)] [items_ptr(4)]
     * Try reading at Scene+0x29D4 as count, then items at +4 and +8 */
    if (!IsBadReadPtr((void*)(scene + SCENE_BALLLIST), 12)) {
        DWORD count = *(DWORD*)(scene + SCENE_BALLLIST);
        /* If count looks reasonable (1-16 balls) */
        if (count > 0 && count < 32) {
            /* Try items at +4 (count, items_ptr layout) */
            DWORD items_ptr = *(DWORD*)(scene + SCENE_BALLLIST + 4);
            if (items_ptr && !IsBadReadPtr((void*)items_ptr, 4)) {
                DWORD ball = *(DWORD*)items_ptr;
                if (ball && !IsBadReadPtr((void*)(ball + BALL_POS_X), 4))
                    return ball;
            }
            /* Try items at +8 (count, capacity, items_ptr layout) */
            items_ptr = *(DWORD*)(scene + SCENE_BALLLIST + 8);
            if (items_ptr && !IsBadReadPtr((void*)items_ptr, 4)) {
                DWORD ball = *(DWORD*)items_ptr;
                if (ball && !IsBadReadPtr((void*)(ball + BALL_POS_X), 4))
                    return ball;
            }
        }
    }

    /* Method 2: Scene+0x29D4 might BE the items pointer directly */
    if (!IsBadReadPtr((void*)(scene + SCENE_BALLLIST), 4)) {
        DWORD ptr = *(DWORD*)(scene + SCENE_BALLLIST);
        if (ptr && !IsBadReadPtr((void*)ptr, 4)) {
            DWORD ball = *(DWORD*)ptr;
            if (ball && !IsBadReadPtr((void*)(ball + BALL_POS_X), 4))
                return ball;
            /* Maybe ptr IS the ball directly */
            if (!IsBadReadPtr((void*)(ptr + BALL_POS_X), 4))
                return ptr;
        }
    }

    /* Method 3: Try ball+0x14 chain from Scene */
    /* Scene might have a direct ball pointer at various offsets */
    DWORD offsets_to_try[] = {0x29D8, 0x29DC, 0x2DE0, 0x2DEC, 0x29E0, 0x29E4};
    for (int i = 0; i < 6; i++) {
        if (!IsBadReadPtr((void*)(scene + offsets_to_try[i]), 4)) {
            DWORD ptr = *(DWORD*)(scene + offsets_to_try[i]);
            if (ptr && !IsBadReadPtr((void*)ptr, 4)) {
                DWORD ball = *(DWORD*)ptr;
                if (ball && !IsBadReadPtr((void*)(ball + BALL_POS_X), 4))
                    return ball;
            }
        }
    }

    return 0;
}

static DWORD WINAPI PinballThread(LPVOID param) {
    /* Wait for game to start */
    int wait = 0;
    while (!(*(DWORD*)APP_PTR) && wait < 200) {
        Sleep(100);
        wait++;
    }
    if (!(*(DWORD*)APP_PTR)) return 0;
    Sleep(3000); /* Wait for game to fully initialize */

    float last_pos[3] = {0, 0, 0};
    float last_vel[3] = {0, 0, 0};
    int have_last_pos = 0;
    int have_last_vel = 0;
    int debug_ball_found = 0;
    int debug_collision_shown = 0;

    BYTE prev_f8 = 0, prev_f9 = 0;

    while (1) {
        Sleep(16); /* ~60fps polling */

        /* Key polling */
        DWORD app = *(DWORD*)APP_PTR;
        if (app && !IsBadReadPtr((void*)(app + 0x180), 4)) {
            DWORD input_handler = *(DWORD*)(app + 0x180);
            if (input_handler && !IsBadReadPtr((void*)(input_handler + 0x434), 4)) {
                DWORD kb_dev = *(DWORD*)(input_handler + 0x434);
                if (kb_dev && !IsBadReadPtr((void*)(kb_dev + 0xC), 256)) {
                    BYTE* keys = (BYTE*)(kb_dev + 0xC);

                    BYTE f8_down = keys[DIK_F8] & 0x80;
                    if (f8_down && !prev_f8) {
                        InterlockedExchange(&g_pinball_enabled, !g_pinball_enabled);
                        char msg[128];
                        wsprintfA(msg, "Pinball Mode: %s (mult: %.1fx)",
                            g_pinball_enabled ? "ON" : "OFF", g_bounce_mult);
                        MessageBoxA(NULL, msg, "Pinball v7", MB_OK | MB_TOPMOST);
                    }
                    prev_f8 = f8_down;

                    BYTE f9_down = keys[DIK_F9] & 0x80;
                    if (f9_down && !prev_f9) {
                        g_mult_index = (g_mult_index + 1) % NUM_MULTS;
                        g_bounce_mult = BOUNCE_MULTS[g_mult_index];
                        char msg[64];
                        wsprintfA(msg, "Bounce: %.1fx", g_bounce_mult);
                        MessageBoxA(NULL, msg, "Pinball v7", MB_OK | MB_TOPMOST);
                    }
                    prev_f9 = f9_down;
                }
            }
        }

        if (!g_pinball_enabled) {
            have_last_pos = 0;
            have_last_vel = 0;
            continue;
        }

        /* Get player ball */
        DWORD ball = GetPlayerBall();
        if (!ball) continue;

        /* Debug: first successful ball read */
        if (!debug_ball_found) {
            float pos[3];
            if (SafeReadFloats(ball + BALL_POS_X, pos, 3)) {
                if (pos[0] != 0.0f || pos[1] != 0.0f || pos[2] != 0.0f) {
                    debug_ball_found = 1;
                    char msg[256];
                    wsprintfA(msg,
                        "Ball found at 0x%08X\nPos: (%.1f, %.1f, %.1f)",
                        ball, pos[0], pos[1], pos[2]);
                    MessageBoxA(NULL, msg, "Pinball v7 - Ball Found", MB_OK | MB_TOPMOST);
                }
            }
        }

        /* Read current position */
        float pos[3];
        if (!SafeReadFloats(ball + BALL_POS_X, pos, 3)) continue;

        if (!have_last_pos) {
            last_pos[0] = pos[0]; last_pos[1] = pos[1]; last_pos[2] = pos[2];
            have_last_pos = 1;
            continue;
        }

        /* Compute frame velocity = current_pos - last_pos */
        float fvel[3];
        fvel[0] = pos[0] - last_pos[0];
        fvel[1] = pos[1] - last_pos[1];
        fvel[2] = pos[2] - last_pos[2];

        /* Check for direction reversal (collision) */
        if (have_last_vel) {
            float dot = fvel[0]*last_vel[0] + fvel[1]*last_vel[1] + fvel[2]*last_vel[2];
            float vel_mag = fvel[0]*fvel[0] + fvel[1]*fvel[1] + fvel[2]*fvel[2];

            if (dot < 0.0f && vel_mag > 0.01f) {
                /* COLLISION DETECTED! */
                if (!debug_collision_shown) {
                    debug_collision_shown = 1;
                    char msg[256];
                    wsprintfA(msg,
                        "Collision detected!\nPos: (%.1f, %.1f, %.1f)\nVel: (%.1f, %.1f, %.1f)\nMult: %.1fx",
                        pos[0], pos[1], pos[2], fvel[0], fvel[1], fvel[2], g_bounce_mult);
                    MessageBoxA(NULL, msg, "Pinball v7 - BOUNCE!", MB_OK | MB_TOPMOST);
                }

                /* Read the game's internal velocity at +0xC98 */
                float gvel[3];
                if (SafeReadFloats(ball + BALL_VEL_X, gvel, 3)) {
                    /* Multiply the game velocity by bounce_mult */
                    float new_vel[3];
                    new_vel[0] = gvel[0] * g_bounce_mult;
                    new_vel[1] = gvel[1] * g_bounce_mult;
                    new_vel[2] = gvel[2] * g_bounce_mult;
                    SafeWriteFloats(ball + BALL_VEL_X, new_vel, 3);

                    /* Also write to velocity accumulators (+0x170) for good measure */
                    SafeWriteFloats(ball + 0x170, new_vel, 3);
                }

                /* Update tracking with amplified velocity */
                last_pos[0] = pos[0]; last_pos[1] = pos[1]; last_pos[2] = pos[2];
                last_vel[0] = fvel[0] * g_bounce_mult;
                last_vel[1] = fvel[1] * g_bounce_mult;
                last_vel[2] = fvel[2] * g_bounce_mult;
                have_last_vel = 1;
                continue;
            }
        }

        /* Store for next frame */
        last_pos[0] = pos[0]; last_pos[1] = pos[1]; last_pos[2] = pos[2];
        last_vel[0] = fvel[0]; last_vel[1] = fvel[1]; last_vel[2] = fvel[2];
        have_last_vel = 1;
    }
    return 0;
}

BOOL APIENTRY DllMain(HINSTANCE hInst, DWORD reason, LPVOID reserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hInst);
        CreateThread(NULL, 0, PinballThread, NULL, 0, NULL);
    }
    return TRUE;
}
