// ============================================================
// Hamsterball Pinball Mode - bass.dll proxy (v6)
//
// COMPLETELY NEW APPROACH: No code hooks at all!
//
// v3-v5 all failed because hooks inside Ball_Update (0x407300, 0x407CE0)
// NEVER FIRE for the player in race mode. The player path goes through
// Ball_AI_ChaseNearest ELSE branch → Ball_ApplyForceWithMultipliers,
// completely bypassing Ball_Update's collision processing.
//
// v6 uses a background thread that:
// 1. Polls ball position (+0x164/+0x168/+0x16C) every ~16ms
// 2. Computes velocity = current_pos - last_pos
// 3. If velocity direction reversed (dot < 0 = collision happened):
//    - Amplifies position by adding velocity * (bounce_mult - 1)
//    - This pushes the ball further in the bounce direction
// 4. Works regardless of which code path processes physics
//
// F8: Toggle pinball mode on/off (silent)
// F9: Cycle bounce multiplier 2x -> 3x -> 5x -> 10x -> 2x (silent)
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
    if (slash) {
        strcpy(slash + 1, "bass_real.dll");
    } else {
        strcpy(path, "bass_real.dll");
    }
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
 * Pinball Mode - Background Thread Approach
 *
 * Ball struct offsets:
 *   +0x164/+0x168/+0x16C = position X/Y/Z (float)
 *
 * App global: 0x5341E0
 * App+0x178 = Scene pointer
 * Scene+0x29D4 = Ball AthenaList
 * Ball list first entry = *(Scene+0x2DE0) → first ball ptr
 *
 * Algorithm:
 *   Every ~16ms:
 *   1. Read ball position
 *   2. velocity = pos - last_pos
 *   3. If dot(velocity, last_velocity) < 0 (direction reversed):
 *        collision detected → amplify bounce
 *        extra = velocity * (bounce_mult - 1)
 *        new_pos = pos + extra
 *        write new_pos
 *   4. Store velocity for next frame
 * ════════════════════════════════════════════════════════════════════ */

static const float BOUNCE_MULTS[] = {2.0f, 3.0f, 5.0f, 10.0f};
static const int NUM_MULTS = 4;

static volatile LONG g_pinball_enabled = 0;
static volatile float g_bounce_mult = 3.0f;
static volatile int g_mult_index = 1;

/* Ball position offsets */
#define BALL_POS_X  0x164
#define BALL_POS_Y  0x168
#define BALL_POS_Z  0x16C

/* Safe memory read */
static float SafeReadFloat(void* addr) {
    if (IsBadReadPtr(addr, 4)) return 0.0f;
    return *(float*)addr;
}

static void SafeWriteFloat(void* addr, float val) {
    if (IsBadReadPtr(addr, 4)) return;
    *(float*)addr = val;
}

/* Get player ball pointer (ball 0 in the scene's ball list) */
static DWORD GetPlayerBall(void) {
    DWORD app = *(DWORD*)0x5341E0;
    if (!app || IsBadReadPtr((void*)(app + 0x178), 4)) return 0;
    DWORD scene = *(DWORD*)(app + 0x178);
    if (!scene || IsBadReadPtr((void*)(scene + 0x2DE0), 4)) return 0;
    DWORD ball_list_ptr = *(DWORD*)(scene + 0x2DE0);
    if (!ball_list_ptr || IsBadReadPtr((void*)ball_list_ptr, 4)) return 0;
    DWORD ball = *(DWORD*)ball_list_ptr;
    return ball;
}

#define DIK_F8  0x42
#define DIK_F9  0x43

static DWORD WINAPI PinballThread(LPVOID param) {
    /* Wait for game to start */
    int wait = 0;
    while (!(*(DWORD*)0x5341E0) && wait < 200) {
        Sleep(100);
        wait++;
    }
    if (!(*(DWORD*)0x5341E0)) return 0;
    Sleep(2000); /* Wait for game to fully initialize */

    float last_pos[3] = {0, 0, 0};
    float last_vel[3] = {0, 0, 0};
    int have_last_pos = 0;
    int have_last_vel = 0;

    BYTE prev_f8 = 0, prev_f9 = 0;

    while (1) {
        Sleep(16); /* ~60fps polling */

        /* Key polling */
        DWORD app = *(DWORD*)0x5341E0;
        if (app && !IsBadReadPtr((void*)(app + 0x180), 4)) {
            DWORD input_handler = *(DWORD*)(app + 0x180);
            if (input_handler && !IsBadReadPtr((void*)(input_handler + 0x434), 4)) {
                DWORD kb_dev = *(DWORD*)(input_handler + 0x434);
                if (kb_dev && !IsBadReadPtr((void*)(kb_dev + 0xC), 256)) {
                    BYTE* keys = (BYTE*)(kb_dev + 0xC);

                    BYTE f8_down = keys[DIK_F8] & 0x80;
                    if (f8_down && !prev_f8) {
                        InterlockedExchange(&g_pinball_enabled, !g_pinball_enabled);
                    }
                    prev_f8 = f8_down;

                    BYTE f9_down = keys[DIK_F9] & 0x80;
                    if (f9_down && !prev_f9) {
                        g_mult_index = (g_mult_index + 1) % NUM_MULTS;
                        g_bounce_mult = BOUNCE_MULTS[g_mult_index];
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
        if (!ball || IsBadReadPtr((void*)(ball + BALL_POS_X), 12)) {
            have_last_pos = 0;
            have_last_vel = 0;
            continue;
        }

        /* Read current position */
        float pos[3];
        pos[0] = *(float*)(ball + BALL_POS_X);
        pos[1] = *(float*)(ball + BALL_POS_Y);
        pos[2] = *(float*)(ball + BALL_POS_Z);

        if (!have_last_pos) {
            last_pos[0] = pos[0];
            last_pos[1] = pos[1];
            last_pos[2] = pos[2];
            have_last_pos = 1;
            continue;
        }

        /* Compute velocity = current_pos - last_pos */
        float vel[3];
        vel[0] = pos[0] - last_pos[0];
        vel[1] = pos[1] - last_pos[1];
        vel[2] = pos[2] - last_pos[2];

        /* Check if we have last velocity for collision detection */
        if (have_last_vel) {
            /* Dot product of current velocity and last velocity */
            float dot = vel[0] * last_vel[0] + vel[1] * last_vel[1] + vel[2] * last_vel[2];

            /* If dot < 0, direction reversed = collision happened */
            /* Also check that velocity magnitude is significant (not just noise) */
            float vel_mag = vel[0]*vel[0] + vel[1]*vel[1] + vel[2]*vel[2];

            if (dot < 0.0f && vel_mag > 0.01f) {
                /* COLLISION DETECTED! Amplify the bounce. */
                float extra = g_bounce_mult - 1.0f;

                /* Push ball further in the bounce direction */
                float new_pos[3];
                new_pos[0] = pos[0] + vel[0] * extra;
                new_pos[1] = pos[1] + vel[1] * extra;
                new_pos[2] = pos[2] + vel[2] * extra;

                SafeWriteFloat((void*)(ball + BALL_POS_X), new_pos[0]);
                SafeWriteFloat((void*)(ball + BALL_POS_Y), new_pos[1]);
                SafeWriteFloat((void*)(ball + BALL_POS_Z), new_pos[2]);

                /* Update last_pos to the amplified position */
                last_pos[0] = new_pos[0];
                last_pos[1] = new_pos[1];
                last_pos[2] = new_pos[2];

                /* Update velocity to the amplified velocity */
                last_vel[0] = vel[0] * g_bounce_mult;
                last_vel[1] = vel[1] * g_bounce_mult;
                last_vel[2] = vel[2] * g_bounce_mult;
                have_last_vel = 1;
                continue;
            }
        }

        /* Store for next frame */
        last_pos[0] = pos[0];
        last_pos[1] = pos[1];
        last_pos[2] = pos[2];
        last_vel[0] = vel[0];
        last_vel[1] = vel[1];
        last_vel[2] = vel[2];
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
