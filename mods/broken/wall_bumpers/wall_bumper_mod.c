/*
 * wall_bumper_mod.c — ALL WALLS ARE BUMPERS (v3)
 *
 * v1 FAILED: hooked Ball_FallUpdate (0x408830) — DEAD CODE, never called for player
 * v2 FAILED: hooked Ball_AI_ChaseNearest (0x408390) — hook fires but collision
 *            entries are EMPTY for player in race mode (Ball_Update never called)
 *            ALSO: BASS proxy used void WINAPI name() with NO params → stack corruption
 * v3 FIXES:
 *   a) BASS proxy uses correct typed signatures for all 10 game imports
 *   b) Uses velocity-reversal detection (position delta between frames) instead
 *      of collision entry scanning — works in ALL game modes (race + arena)
 *   c) Also scans collision entries as secondary method (works in arena mode)
 *   d) Logs everything to bumper_mod.log in the game directory
 *
 * HOOK: Ball_AI_ChaseNearest (0x00408390) — ball vtable[4]
 *   Called every frame for ALL balls (including player in race mode).
 *   For the player in race mode, the original function is a no-op (JZ to RET),
 *   but our hook still fires — giving us a per-frame callback.
 *
 * DETECTION: Two methods:
 *   1. VELOCITY REVERSAL (primary, works in all modes):
 *      Track ball position each frame. Compute velocity = pos - last_pos.
 *      If dot(current_vel, last_vel) < 0 → ball hit something (velocity reversed).
 *      Amplify the new velocity direction (the bounce direction).
 *   2. COLLISION ENTRIES (secondary, arena mode only):
 *      Scan PhysicsObject collision list for type==2 (wall) entries.
 *
 * BUILD:
 *   i686-w64-mingw32-gcc -shared -o bass.dll wall_bumper_mod.c \
 *       -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *       -Wl,--add-stdcall-alias
 *
 * INSTALL:
 *   1. Rename original bass.dll → bass_real.dll in your Hamsterball folder
 *   2. Copy this bass.dll into the game folder
 *   3. Launch the game — all walls now act as bumpers
 *
 * CONTROLS:
 *   F8 — Toggle mod on/off (default: ON)
 *   F9 — Cycle bumper force (40 → 60 → 80 → 120 → 200)
 *
 * LOG:
 *   bumper_mod.log in the game directory — hook status, ball state, bumper hits
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdarg.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * BASS Proxy Exports — correct typed signatures (v3 fix)
 *
 * The game imports exactly 10 functions from BASS.dll. Each MUST be forwarded
 * with the correct __stdcall parameter count, otherwise the callee cleans the
 * wrong number of bytes from the stack → stack corruption → crash.
 * ═══════════════════════════════════════════════════════════════════════════ */

static HMODULE g_hRealBass = NULL;

/* BASS_Init: 5 params, 20 bytes */
typedef int  (__stdcall *BASS_Init_t)(int, DWORD, DWORD, HWND, void*);
static BASS_Init_t real_BASS_Init = NULL;
__declspec(dllexport) int __stdcall BASS_Init(int a, DWORD b, DWORD c, HWND d, void* e) {
    if (!real_BASS_Init) {
        char path[MAX_PATH]; GetModuleFileNameA(NULL, path, MAX_PATH);
        char* s = strrchr(path, '\\'); if (s) strcpy(s+1, "bass_real.dll"); else strcpy(path, "bass_real.dll");
        HMODULE h = LoadLibraryA(path);
        if (h) real_BASS_Init = (BASS_Init_t)GetProcAddress(h, "BASS_Init");
    }
    if (real_BASS_Init) return real_BASS_Init(a, b, c, d, e);
    return 0;
}

/* BASS_Free: 0 params */
typedef void (__stdcall *BASS_Free_t)(void);
static BASS_Free_t real_BASS_Free = NULL;
__declspec(dllexport) void __stdcall BASS_Free(void) {
    if (!real_BASS_Free) {
        char path[MAX_PATH]; GetModuleFileNameA(NULL, path, MAX_PATH);
        char* s = strrchr(path, '\\'); if (s) strcpy(s+1, "bass_real.dll"); else strcpy(path, "bass_real.dll");
        HMODULE h = LoadLibraryA(path);
        if (h) real_BASS_Free = (BASS_Free_t)GetProcAddress(h, "BASS_Free");
    }
    if (real_BASS_Free) real_BASS_Free();
}

/* BASS_Start: 0 params */
typedef int  (__stdcall *BASS_Start_t)(void);
static BASS_Start_t real_BASS_Start = NULL;
__declspec(dllexport) int __stdcall BASS_Start(void) {
    if (!real_BASS_Start) {
        char path[MAX_PATH]; GetModuleFileNameA(NULL, path, MAX_PATH);
        char* s = strrchr(path, '\\'); if (s) strcpy(s+1, "bass_real.dll"); else strcpy(path, "bass_real.dll");
        HMODULE h = LoadLibraryA(path);
        if (h) real_BASS_Start = (BASS_Start_t)GetProcAddress(h, "BASS_Start");
    }
    if (real_BASS_Start) return real_BASS_Start();
    return 0;
}

/* BASS_Stop: 0 params */
typedef int  (__stdcall *BASS_Stop_t)(void);
static BASS_Stop_t real_BASS_Stop = NULL;
__declspec(dllexport) int __stdcall BASS_Stop(void) {
    if (!real_BASS_Stop) {
        char path[MAX_PATH]; GetModuleFileNameA(NULL, path, MAX_PATH);
        char* s = strrchr(path, '\\'); if (s) strcpy(s+1, "bass_real.dll"); else strcpy(path, "bass_real.dll");
        HMODULE h = LoadLibraryA(path);
        if (h) real_BASS_Stop = (BASS_Stop_t)GetProcAddress(h, "BASS_Stop");
    }
    if (real_BASS_Stop) return real_BASS_Stop();
    return 0;
}

/* BASS_SetConfig: 2 params, 8 bytes */
typedef int  (__stdcall *BASS_SetConfig_t)(DWORD, DWORD);
static BASS_SetConfig_t real_BASS_SetConfig = NULL;
__declspec(dllexport) int __stdcall BASS_SetConfig(DWORD a, DWORD b) {
    if (!real_BASS_SetConfig) {
        char path[MAX_PATH]; GetModuleFileNameA(NULL, path, MAX_PATH);
        char* s = strrchr(path, '\\'); if (s) strcpy(s+1, "bass_real.dll"); else strcpy(path, "bass_real.dll");
        HMODULE h = LoadLibraryA(path);
        if (h) real_BASS_SetConfig = (BASS_SetConfig_t)GetProcAddress(h, "BASS_SetConfig");
    }
    if (real_BASS_SetConfig) return real_BASS_SetConfig(a, b);
    return 0;
}

/* BASS_ErrorGetCode: 0 params */
typedef int  (__stdcall *BASS_ErrorGetCode_t)(void);
static BASS_ErrorGetCode_t real_BASS_ErrorGetCode = NULL;
__declspec(dllexport) int __stdcall BASS_ErrorGetCode(void) {
    if (!real_BASS_ErrorGetCode) {
        char path[MAX_PATH]; GetModuleFileNameA(NULL, path, MAX_PATH);
        char* s = strrchr(path, '\\'); if (s) strcpy(s+1, "bass_real.dll"); else strcpy(path, "bass_real.dll");
        HMODULE h = LoadLibraryA(path);
        if (h) real_BASS_ErrorGetCode = (BASS_ErrorGetCode_t)GetProcAddress(h, "BASS_ErrorGetCode");
    }
    if (real_BASS_ErrorGetCode) return real_BASS_ErrorGetCode();
    return 0;
}

/* BASS_MusicLoad: 6 params, 28 bytes (QWORD offset = 8 bytes on 32-bit) */
typedef int  (__stdcall *BASS_MusicLoad_t)(BOOL, const void*, unsigned long long, DWORD, DWORD, DWORD);
static BASS_MusicLoad_t real_BASS_MusicLoad = NULL;
__declspec(dllexport) int __stdcall BASS_MusicLoad(BOOL a, const void* b, unsigned long long c, DWORD d, DWORD e, DWORD f) {
    if (!real_BASS_MusicLoad) {
        char path[MAX_PATH]; GetModuleFileNameA(NULL, path, MAX_PATH);
        char* s = strrchr(path, '\\'); if (s) strcpy(s+1, "bass_real.dll"); else strcpy(path, "bass_real.dll");
        HMODULE h = LoadLibraryA(path);
        if (h) real_BASS_MusicLoad = (BASS_MusicLoad_t)GetProcAddress(h, "BASS_MusicLoad");
    }
    if (real_BASS_MusicLoad) return real_BASS_MusicLoad(a, b, c, d, e, f);
    return 0;
}

/* BASS_MusicPlayEx: 3 params, 12 bytes */
typedef int  (__stdcall *BASS_MusicPlayEx_t)(DWORD, DWORD, DWORD);
static BASS_MusicPlayEx_t real_BASS_MusicPlayEx = NULL;
__declspec(dllexport) int __stdcall BASS_MusicPlayEx(DWORD a, DWORD b, DWORD c) {
    if (!real_BASS_MusicPlayEx) {
        char path[MAX_PATH]; GetModuleFileNameA(NULL, path, MAX_PATH);
        char* s = strrchr(path, '\\'); if (s) strcpy(s+1, "bass_real.dll"); else strcpy(path, "bass_real.dll");
        HMODULE h = LoadLibraryA(path);
        if (h) real_BASS_MusicPlayEx = (BASS_MusicPlayEx_t)GetProcAddress(h, "BASS_MusicPlayEx");
    }
    if (real_BASS_MusicPlayEx) return real_BASS_MusicPlayEx(a, b, c);
    return 0;
}

/* BASS_MusicFree: 1 param, 4 bytes */
typedef int  (__stdcall *BASS_MusicFree_t)(DWORD);
static BASS_MusicFree_t real_BASS_MusicFree = NULL;
__declspec(dllexport) int __stdcall BASS_MusicFree(DWORD a) {
    if (!real_BASS_MusicFree) {
        char path[MAX_PATH]; GetModuleFileNameA(NULL, path, MAX_PATH);
        char* s = strrchr(path, '\\'); if (s) strcpy(s+1, "bass_real.dll"); else strcpy(path, "bass_real.dll");
        HMODULE h = LoadLibraryA(path);
        if (h) real_BASS_MusicFree = (BASS_MusicFree_t)GetProcAddress(h, "BASS_MusicFree");
    }
    if (real_BASS_MusicFree) return real_BASS_MusicFree(a);
    return 0;
}

/* BASS_ChannelSetAttributes: 4 params, 16 bytes */
typedef int  (__stdcall *BASS_ChannelSetAttributes_t)(DWORD, float, int, int);
static BASS_ChannelSetAttributes_t real_BASS_ChannelSetAttributes = NULL;
__declspec(dllexport) int __stdcall BASS_ChannelSetAttributes(DWORD a, float b, int c, int d) {
    if (!real_BASS_ChannelSetAttributes) {
        char path[MAX_PATH]; GetModuleFileNameA(NULL, path, MAX_PATH);
        char* s = strrchr(path, '\\'); if (s) strcpy(s+1, "bass_real.dll"); else strcpy(path, "bass_real.dll");
        HMODULE h = LoadLibraryA(path);
        if (h) real_BASS_ChannelSetAttributes = (BASS_ChannelSetAttributes_t)GetProcAddress(h, "BASS_ChannelSetAttributes");
    }
    if (real_BASS_ChannelSetAttributes) return real_BASS_ChannelSetAttributes(a, b, c, d);
    return 0;
}

/* BASS_ChannelStop: 1 param, 4 bytes */
typedef int  (__stdcall *BASS_ChannelStop_t)(DWORD);
static BASS_ChannelStop_t real_BASS_ChannelStop = NULL;
__declspec(dllexport) int __stdcall BASS_ChannelStop(DWORD a) {
    if (!real_BASS_ChannelStop) {
        char path[MAX_PATH]; GetModuleFileNameA(NULL, path, MAX_PATH);
        char* s = strrchr(path, '\\'); if (s) strcpy(s+1, "bass_real.dll"); else strcpy(path, "bass_real.dll");
        HMODULE h = LoadLibraryA(path);
        if (h) real_BASS_ChannelStop = (BASS_ChannelStop_t)GetProcAddress(h, "BASS_ChannelStop");
    }
    if (real_BASS_ChannelStop) return real_BASS_ChannelStop(a);
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Game Constants & Struct Offsets
 * ═══════════════════════════════════════════════════════════════════════════ */

#define APP_PTR                0x005341E0
#define HOOK_TARGET            0x00408390
#define HOOK_SIZE              6
#define TRAMPOLINE_SIZE        (HOOK_SIZE + 5)

/* Ball struct offsets */
#define BALL_PHYSICS           0x1A4   /* void* PhysicsObject */
#define BALL_POS_X             0x164   /* float */
#define BALL_POS_Y             0x168   /* float */
#define BALL_POS_Z             0x16C   /* float */
#define BALL_SPEED_BOOST       0x808   /* int: speed_boost counter */
#define BALL_FALLING_FLAG      0x2F9   /* byte: 1 when falling/despawning */
#define BALL_PLAYER_INDEX      0x18    /* int32: -1 = NPC, 0-3 = Player 1-4 */

/* PhysicsObject struct offsets */
#define PHYS_COLLISION_COUNT   0x1C    /* int */
#define PHYS_COLLISION_ARRAY   0x424   /* void**: array of CollisionEntry* */
#define PHYS_SPEED             0xC64   /* float: speed magnitude */
#define PHYS_VEL_X             0xC98   /* float: velocity X */
#define PHYS_VEL_Y             0xC9C   /* float: velocity Y */
#define PHYS_VEL_Z             0xCA0   /* float: velocity Z */

/* CollisionEntry struct offsets */
#define ENTRY_TYPE             0x00    /* int32: 1=ball-ball, 2=wall, 5=floor */
#define ENTRY_NORMAL_X         0x20    /* float */
#define ENTRY_NORMAL_Y         0x24
#define ENTRY_NORMAL_Z         0x28

/* Bumper force presets */
static const float BUMPER_FORCES[] = { 40.0f, 60.0f, 80.0f, 120.0f, 200.0f };
#define NUM_FORCES (sizeof(BUMPER_FORCES) / sizeof(BUMPER_FORCES[0]))

/* Collision detection thresholds */
#define MIN_SPEED_FOR_BOUNCE   0.5f    /* ignore micro-movements */
#define REVERSAL_THRESHOLD     -0.3f   /* dot product < this = collision */
#define COOLDOWN_FRAMES        5       /* frames between bumper hits */

/* ═══════════════════════════════════════════════════════════════════════════
 * Globals
 * ═══════════════════════════════════════════════════════════════════════════ */

static int g_mod_enabled = 1;
static int g_force_index = 2;  /* default: 80 */
static void *g_trampoline = NULL;
static int g_hook_installed = 0;

/* Keyboard state (Windows VK codes for GetAsyncKeyState) */
#define VK_F8  0x77
#define VK_F9  0x78

/* Log file path */
static char g_log_path[MAX_PATH] = {0};
static CRITICAL_SECTION g_log_cs;
static int g_log_initialized = 0;

/* Velocity tracking state (per-ball, but we only track player) */
static float g_last_pos_x = 0.0f, g_last_pos_y = 0.0f, g_last_pos_z = 0.0f;
static float g_last_vel_x = 0.0f, g_last_vel_y = 0.0f, g_last_vel_z = 0.0f;
static int g_have_last_pos = 0;
static int g_cooldown = 0;
static int g_frame_count = 0;
static int g_bumper_hits = 0;

/* ═══════════════════════════════════════════════════════════════════════════
 * Safe Memory Access
 * ═══════════════════════════════════════════════════════════════════════════ */

static int safe_read(void *addr, void *dest, int size) {
    if (!addr || IsBadReadPtr(addr, size)) return 0;
    memcpy(dest, addr, size);
    return 1;
}

static int safe_read_int(void *addr) {
    int val = 0;
    safe_read(addr, &val, sizeof(val));
    return val;
}

static float safe_read_float(void *addr) {
    float val = 0.0f;
    safe_read(addr, &val, sizeof(val));
    return val;
}

/* Fast inverse square root */
static float fast_inv_sqrt(float x) {
    union { float f; DWORD d; } inv;
    DWORD i = *(DWORD*)&x;
    i = 0x5f3759df - (i >> 1);
    inv.d = i;
    inv.f *= (1.5f - 0.5f * x * inv.f * inv.f);
    return inv.f;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Logging
 * ═══════════════════════════════════════════════════════════════════════════ */

static void log_init(void) {
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    char* slash = strrchr(path, '\\');
    if (slash) {
        strcpy(slash + 1, "bumper_mod.log");
    } else {
        strcpy(path, "bumper_mod.log");
    }
    strncpy(g_log_path, path, MAX_PATH - 1);

    FILE* f = NULL;
    if (fopen_s(&f, g_log_path, "w") == 0 && f) {
        fprintf(f, "Hamsterball Wall Bumper Mod v3\n");
        fprintf(f, "=============================\n");
        fprintf(f, "Log started.\n\n");
        fclose(f);
    }
    g_log_initialized = 1;
}

static void log_write(const char* fmt, ...) {
    if (!g_log_initialized || g_log_path[0] == 0) return;

    EnterCriticalSection(&g_log_cs);

    FILE* f = NULL;
    if (fopen_s(&f, g_log_path, "a") != 0 || !f) {
        LeaveCriticalSection(&g_log_cs);
        return;
    }

    va_list args;
    va_start(args, fmt);
    vfprintf(f, fmt, args);
    va_end(args);
    fprintf(f, "\n");

    fclose(f);
    LeaveCriticalSection(&g_log_cs);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Hook: Ball_AI_ChaseNearest (0x408390)
 *
 * Called every frame for ALL balls (including player in race mode).
 * For the player in race mode, the original is a no-op (JZ to RET),
 * but our hook still fires — giving us a per-frame callback.
 *
 * We detect wall collisions by tracking position changes between frames
 * and checking for velocity reversal (dot product of consecutive velocities < 0).
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef void (__fastcall *BallAIChaseNearest_t)(void *ball, void *edx_dummy);
static BallAIChaseNearest_t g_orig_BallAIChaseNearest = NULL;

static void __fastcall hook_BallAIChaseNearest(void *ball, void *edx_dummy) {
    /* Call original function first */
    if (g_orig_BallAIChaseNearest)
        g_orig_BallAIChaseNearest(ball, edx_dummy);

    g_frame_count++;

    if (!g_mod_enabled) return;
    if (!ball || IsBadReadPtr(ball, 0x200)) return;

    /* Don't process when ball is falling/despawning */
    if (safe_read_int((char*)ball + BALL_FALLING_FLAG))
        return;

    /* Read ball position */
    float pos_x = safe_read_float((char*)ball + BALL_POS_X);
    float pos_y = safe_read_float((char*)ball + BALL_POS_Y);
    float pos_z = safe_read_float((char*)ball + BALL_POS_Z);

    /* Get PhysicsObject */
    void *physics = NULL;
    if (!safe_read((char*)ball + BALL_PHYSICS, &physics, sizeof(physics)) || !physics)
        return;
    if (IsBadReadPtr(physics, 0xCA4)) return;

    /* Read physics velocity + speed */
    float phys_speed = safe_read_float((char*)physics + PHYS_SPEED);
    float phys_vel_x = safe_read_float((char*)physics + PHYS_VEL_X);
    float phys_vel_y = safe_read_float((char*)physics + PHYS_VEL_Y);
    float phys_vel_z = safe_read_float((char*)physics + PHYS_VEL_Z);

    /* Log first 10 frames to confirm hook is working */
    if (g_frame_count <= 10) {
        int player_idx = safe_read_int((char*)ball + BALL_PLAYER_INDEX);
        int col_count = safe_read_int((char*)physics + PHYS_COLLISION_COUNT);
        log_write("[Frame %d] ball=%p player=%d pos=(%.1f,%.1f,%.1f) speed=%.2f vel=(%.2f,%.2f,%.2f) col_count=%d",
                  g_frame_count, ball, player_idx, pos_x, pos_y, pos_z,
                  phys_speed, phys_vel_x, phys_vel_y, phys_vel_z, col_count);
    }

    /* Decrement cooldown */
    if (g_cooldown > 0) g_cooldown--;

    /* ── Method 1: Velocity Reversal Detection (primary, all modes) ── */

    /* Compute frame-to-frame velocity from position delta */
    float cur_vel_x = pos_x - g_last_pos_x;
    float cur_vel_y = pos_y - g_last_pos_y;
    float cur_vel_z = pos_z - g_last_pos_z;

    if (g_have_last_pos && g_cooldown == 0) {
        float last_speed_sq = g_last_vel_x*g_last_vel_x + g_last_vel_y*g_last_vel_y + g_last_vel_z*g_last_vel_z;
        float cur_speed_sq = cur_vel_x*cur_vel_x + cur_vel_y*cur_vel_y + cur_vel_z*cur_vel_z;

        if (last_speed_sq > MIN_SPEED_FOR_BOUNCE * MIN_SPEED_FOR_BOUNCE &&
            cur_speed_sq > MIN_SPEED_FOR_BOUNCE * MIN_SPEED_FOR_BOUNCE) {

            /* Dot product: if < 0, velocity reversed → collision */
            float dot = cur_vel_x*g_last_vel_x + cur_vel_y*g_last_vel_y + cur_vel_z*g_last_vel_z;
            float last_speed = sqrtf(last_speed_sq);
            float cur_speed = sqrtf(cur_speed_sq);
            float cos_angle = dot / (last_speed * cur_speed);

            if (cos_angle < REVERSAL_THRESHOLD) {
                /* Collision detected! Amplify the bounce direction. */
                float force = BUMPER_FORCES[g_force_index];

                /* Normalize current velocity (bounce direction) */
                float inv = fast_inv_sqrt(cur_speed_sq);
                float dir_x = cur_vel_x * inv;
                float dir_y = cur_vel_y * inv;
                float dir_z = cur_vel_z * inv;

                log_write("[BUMPER HIT #%d] vel reversed! cos=%.2f last_vel=(%.2f,%.2f,%.2f) cur_vel=(%.2f,%.2f,%.2f) force=%.0f dir=(%.2f,%.2f,%.2f)",
                          ++g_bumper_hits, cos_angle,
                          g_last_vel_x, g_last_vel_y, g_last_vel_z,
                          cur_vel_x, cur_vel_y, cur_vel_z,
                          force, dir_x, dir_y, dir_z);

                /* Push ball position along bounce direction */
                *(float*)((char*)ball + BALL_POS_X) += dir_x * force;
                *(float*)((char*)ball + BALL_POS_Y) += dir_y * force;
                *(float*)((char*)ball + BALL_POS_Z) += dir_z * force;

                /* Overwrite physics velocity (amplified bounce) */
                *(float*)((char*)physics + PHYS_VEL_X) = dir_x * force;
                *(float*)((char*)physics + PHYS_VEL_Y) = dir_y * force;
                *(float*)((char*)physics + PHYS_VEL_Z) = dir_z * force;
                *(float*)((char*)physics + PHYS_SPEED) = force;

                /* Set speed_boost to briefly disable input (like real bumpers) */
                *(int*)((char*)ball + BALL_SPEED_BOOST) = 3;

                g_cooldown = COOLDOWN_FRAMES;

                /* After applying force, update last_pos so we don't re-detect */
                g_last_pos_x = pos_x + dir_x * force;
                g_last_pos_y = pos_y + dir_y * force;
                g_last_pos_z = pos_z + dir_z * force;
                g_last_vel_x = dir_x * force;
                g_last_vel_y = dir_y * force;
                g_last_vel_z = dir_z * force;
                return;
            }
        }
    }

    /* ── Method 2: Collision Entry Scanning (secondary, arena mode) ── */

    int col_count = safe_read_int((char*)physics + PHYS_COLLISION_COUNT);
    if (col_count > 0 && col_count < 200 && g_cooldown == 0) {
        void **entries = NULL;
        if (safe_read((char*)physics + PHYS_COLLISION_ARRAY, &entries, sizeof(entries)) && entries) {
            if (!IsBadReadPtr(entries, col_count * sizeof(void*))) {
                int wall_hits = 0;
                float push_x = 0.0f, push_y = 0.0f, push_z = 0.0f;

                for (int i = 0; i < col_count; i++) {
                    void *entry = NULL;
                    if (!safe_read(&entries[i], &entry, sizeof(entry)) || !entry)
                        continue;
                    if (IsBadReadPtr(entry, 0x30))
                        continue;

                    int type = safe_read_int(entry);
                    if (type == 2) {  /* Wall collision */
                        float nx = safe_read_float((char*)entry + ENTRY_NORMAL_X);
                        float ny = safe_read_float((char*)entry + ENTRY_NORMAL_Y);
                        float nz = safe_read_float((char*)entry + ENTRY_NORMAL_Z);
                        push_x += nx;
                        push_y += ny;
                        push_z += nz;
                        wall_hits++;
                    }
                }

                if (wall_hits > 0) {
                    float force = BUMPER_FORCES[g_force_index];

                    /* Normalize push direction */
                    float mag = push_x*push_x + push_y*push_y + push_z*push_z;
                    if (mag > 0.0001f) {
                        float inv = fast_inv_sqrt(mag);
                        push_x *= inv;
                        push_y *= inv;
                        push_z *= inv;
                    }

                    log_write("[BUMPER HIT #%d] (collision entry) wall_hits=%d normal=(%.2f,%.2f,%.2f) force=%.0f",
                              ++g_bumper_hits, wall_hits, push_x, push_y, push_z, force);

                    *(float*)((char*)ball + BALL_POS_X) += push_x * force;
                    *(float*)((char*)ball + BALL_POS_Y) += push_y * force;
                    *(float*)((char*)ball + BALL_POS_Z) += push_z * force;

                    *(float*)((char*)physics + PHYS_VEL_X) = push_x * force;
                    *(float*)((char*)physics + PHYS_VEL_Y) = push_y * force;
                    *(float*)((char*)physics + PHYS_VEL_Z) = push_z * force;
                    *(float*)((char*)physics + PHYS_SPEED) = force;

                    *(int*)((char*)ball + BALL_SPEED_BOOST) = 3;
                    g_cooldown = COOLDOWN_FRAMES;

                    g_last_pos_x = pos_x + push_x * force;
                    g_last_pos_y = pos_y + push_y * force;
                    g_last_pos_z = pos_z + push_z * force;
                    g_last_vel_x = push_x * force;
                    g_last_vel_y = push_y * force;
                    g_last_vel_z = push_z * force;
                    return;
                }
            }
        }
    }

    /* Update tracking state for next frame */
    g_last_pos_x = pos_x;
    g_last_pos_y = pos_y;
    g_last_pos_z = pos_z;
    g_last_vel_x = cur_vel_x;
    g_last_vel_y = cur_vel_y;
    g_last_vel_z = cur_vel_z;
    g_have_last_pos = 1;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Detour Installation (6-byte: 5-byte JMP + 1 NOP)
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Expected bytes at 0x408390: 83 EC 30 56 8B F1 */
static const unsigned char EXPECTED_BYTES[6] = { 0x83, 0xEC, 0x30, 0x56, 0x8B, 0xF1 };

static int install_detour(void) {
    DWORD old_protect;
    BYTE *target = (BYTE*)HOOK_TARGET;

    /* Verify target bytes match expected */
    if (IsBadReadPtr(target, HOOK_SIZE)) {
        log_write("ERROR: Cannot read hook target at 0x%08X", HOOK_TARGET);
        return 0;
    }

    if (memcmp(target, EXPECTED_BYTES, HOOK_SIZE) != 0) {
        log_write("ERROR: Byte mismatch at 0x%08X! Expected: 83 EC 30 56 8B F1", HOOK_TARGET);
        log_write("  Got: %02X %02X %02X %02X %02X %02X",
                  target[0], target[1], target[2], target[3], target[4], target[5]);
        return 0;
    }

    log_write("Target bytes verified at 0x%08X: 83 EC 30 56 8B F1", HOOK_TARGET);

    /* Allocate trampoline */
    g_trampoline = VirtualAlloc(NULL, TRAMPOLINE_SIZE + 16,
                                 MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!g_trampoline) {
        log_write("ERROR: VirtualAlloc failed for trampoline");
        return 0;
    }

    BYTE *tramp = (BYTE*)g_trampoline;

    /* Copy original bytes to trampoline */
    memcpy(tramp, target, HOOK_SIZE);

    /* Add JMP from trampoline back to target+HOOK_SIZE */
    tramp[HOOK_SIZE] = 0xE9;
    DWORD rel_addr = (DWORD)target + HOOK_SIZE - ((DWORD)tramp + HOOK_SIZE + 5);
    *(DWORD*)(tramp + HOOK_SIZE + 1) = rel_addr;

    /* Set up function pointer to trampoline */
    g_orig_BallAIChaseNearest = (BallAIChaseNearest_t)tramp;

    /* Write JMP from target to hook function */
    VirtualProtect(target, HOOK_SIZE, PAGE_EXECUTE_READWRITE, &old_protect);

    BYTE jump_instr[5] = { 0xE9, 0, 0, 0, 0 };
    DWORD hook_addr = (DWORD)hook_BallAIChaseNearest;
    rel_addr = hook_addr - ((DWORD)target + 5);
    jump_instr[1] = rel_addr & 0xFF;
    jump_instr[2] = (rel_addr >> 8) & 0xFF;
    jump_instr[3] = (rel_addr >> 16) & 0xFF;
    jump_instr[4] = (rel_addr >> 24) & 0xFF;

    memcpy(target, jump_instr, 5);
    target[5] = 0x90;  /* NOP */

    VirtualProtect(target, HOOK_SIZE, old_protect, &old_protect);
    FlushInstructionCache(GetCurrentProcess(), target, HOOK_SIZE);

    log_write("Hook installed successfully! target=0x%08X → hook=0x%08X trampoline=0x%08X",
              HOOK_TARGET, hook_addr, (DWORD)tramp);

    return 1;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Keyboard Handling
 * ═══════════════════════════════════════════════════════════════════════════ */

static void handle_keyboard(void) {
    /* F8: toggle */
    if (GetAsyncKeyState(VK_F8) & 0x8000) {
        g_mod_enabled = !g_mod_enabled;
        log_write("Mod %s (F8)", g_mod_enabled ? "ENABLED" : "DISABLED");
        Sleep(300);
    }

    /* F9: cycle force */
    if (GetAsyncKeyState(VK_F9) & 0x8000) {
        g_force_index = (g_force_index + 1) % NUM_FORCES;
        log_write("Bumper force set to %.0f (F9)", BUMPER_FORCES[g_force_index]);
        Sleep(300);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Main Thread
 * ═══════════════════════════════════════════════════════════════════════════ */

static DWORD WINAPI main_thread(LPVOID param) {
    (void)param;

    InitializeCriticalSection(&g_log_cs);
    log_init();

    log_write("Thread started. Waiting for game to load...");

    /* Wait for App global to be allocated */
    int wait = 0;
    while (!(*(DWORD*)APP_PTR) && wait < 100) {
        Sleep(100);
        wait++;
    }
    if (!(*(DWORD*)APP_PTR)) {
        log_write("ERROR: App pointer at 0x%08X never became non-null after 10s", APP_PTR);
        MessageBoxA(NULL, "Bumper Mod: App pointer never initialized!\nMod will not work.",
                    "Bumper Mod Error", MB_OK | MB_ICONERROR);
        return 0;
    }
    log_write("App pointer ready at 0x%08X (waited %d ms)", APP_PTR, wait * 100);

    Sleep(500);  /* extra safety margin */

    /* Install detour */
    if (install_detour()) {
        g_hook_installed = 1;
        MessageBoxA(NULL,
                    "Wall Bumper Mod v3 installed!\n\n"
                    "Hook: Ball_AI_ChaseNearest (0x408390)\n"
                    "Log: bumper_mod.log in game folder\n\n"
                    "F8 = toggle on/off\n"
                    "F9 = cycle bumper force",
                    "Bumper Mod", MB_OK | MB_ICONINFORMATION);
    } else {
        MessageBoxA(NULL, "Bumper Mod: Failed to install hook!\nCheck bumper_mod.log for details.",
                    "Bumper Mod Error", MB_OK | MB_ICONERROR);
        return 0;
    }

    /* Main loop — handle keyboard */
    while (1) {
        handle_keyboard();
        Sleep(50);
    }

    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DllMain
 * ═══════════════════════════════════════════════════════════════════════════ */

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, LPVOID reserved) {
    (void)hinst;
    (void)reserved;

    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hinst);
        CreateThread(NULL, 0, main_thread, NULL, 0, NULL);
    }
    return TRUE;
}
