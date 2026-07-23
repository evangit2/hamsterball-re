/*
 * magnet_mod.c — Magnet zones for Hamsterball
 *
 * S1 ref points named MAGNET(P) attract the ball.
 * S1 ref points named MAGNET(N) repel the ball.
 *
 * Force is applied by directly modifying the ball's position each frame.
 * This is the proven approach for player-affecting mods (see wall_bumpers
 * Option D) since Ball_Update is not called for the player in race mode.
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll magnet_mod.c \
 *     -I../_WIP\ 🔨\ │\ shared -lwinmm -Wl,--enable-stdcall-fixup -O2 \
 *     -static -static-libgcc -Wl,--add-stdcall-alias -msse2 -mfpmath=sse
 *
 * Config: magnet_config.txt next to bass.dll
 *   range = 300.0      (how far magnets reach)
 *   strength = 0.5     (force magnitude)
 *   max_force = 3.0    (cap per-frame displacement)
 */

#include "bass_proxy.h"
#include <math.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * Magnet State
 * ═══════════════════════════════════════════════════════════════════════════ */

#define MAX_MAGNETS 64

typedef struct {
    float x, y, z;
    int   is_positive;  /* 1 = attract (P), 0 = repel (N) */
} MagnetPoint;

static MagnetPoint g_magnets[MAX_MAGNETS];
static int   g_magnet_count = 0;
static DWORD g_last_board   = 0;

/* Config */
static float g_range     = 300.0f;
static float g_strength   = 0.5f;
static float g_max_force = 3.0f;

/* ═══════════════════════════════════════════════════════════════════════════
 * Game Accessors (App+0x178 chain — works in all modes incl. Time Trial)
 * ═══════════════════════════════════════════════════════════════════════════ */

static DWORD get_board_v2(void) {
    if (IsBadReadPtr((void*)GLOBAL_APP_PTR, 4)) return 0;
    DWORD app = *(DWORD*)GLOBAL_APP_PTR;
    if (!app || app < 0x10000) return 0;
    if (IsBadReadPtr((void*)(app + 0x178), 4)) return 0;
    DWORD board = *(DWORD*)(app + 0x178);
    if (!board || board < 0x10000) return 0;
    return board;
}

static DWORD get_player_ball_v2(void) {
    if (IsBadReadPtr((void*)GLOBAL_APP_PTR, 4)) return 0;
    DWORD app = *(DWORD*)GLOBAL_APP_PTR;
    if (!app || app < 0x10000) return 0;
    if (IsBadReadPtr((void*)(app + 0x5DC), 4)) return 0;
    DWORD ball = *(DWORD*)(app + 0x5DC);
    if (!ball || ball < 0x10000) return 0;
    if (IsBadReadPtr((void*)ball, 4)) return 0;
    return ball;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * S1 Ref Point Scanner
 *
 * Chain: board+0x8AC → Level → Level+0x480 → SceneObject
 *        SceneObject+0x898 = count (AthenaList+0x04)
 *        SceneObject+0xCA0 = items array ptr (AthenaList+0x40C)
 *
 * Each item: [0]=name_ptr, [1]=X(float), [2]=Y(float), [3]=Z(float), [5]=scale
 * ═══════════════════════════════════════════════════════════════════════════ */

static void scan_magnets(DWORD board) {
    g_magnet_count = 0;
    g_last_board   = board;

    if (IsBadReadPtr((void*)(board + 0x8AC), 4)) return;
    DWORD level = *(DWORD*)(board + 0x8AC);
    if (!level || level < 0x10000) return;
    if (IsBadReadPtr((void*)(level + 0x480), 4)) return;
    DWORD sceneobj = *(DWORD*)(level + 0x480);
    if (!sceneobj || sceneobj < 0x10000) return;

    if (IsBadReadPtr((void*)(sceneobj + 0x898), 4)) return;
    int count = *(int*)(sceneobj + 0x898);
    if (count <= 0 || count > 2000) return;

    if (IsBadReadPtr((void*)(sceneobj + 0xCA0), 4)) return;
    DWORD items_array = *(DWORD*)(sceneobj + 0xCA0);
    if (!items_array || items_array < 0x10000) return;

    for (int i = 0; i < count && g_magnet_count < MAX_MAGNETS; i++) {
        if (IsBadReadPtr((void*)(items_array + i * 4), 4)) break;
        DWORD item = *(DWORD*)(items_array + i * 4);
        if (!item || item < 0x10000) continue;
        if (IsBadReadPtr((void*)item, 16)) continue;

        char *name = *(char **)item;   /* item[0] = name string pointer */
        if (!name || IsBadReadPtr(name, 10)) continue;

        /* Match "MAGNET(P)" or "MAGNET(N)" (case-insensitive prefix) */
        if (_strnicmp(name, "MAGNET(", 7) != 0) continue;

        char type = name[7];
        if (type != 'P' && type != 'p' && type != 'N' && type != 'n') continue;

        MagnetPoint *m   = &g_magnets[g_magnet_count++];
        m->x             = *(float *)(item + 4);   /* item[1] = X */
        m->y             = *(float *)(item + 8);   /* item[2] = Y */
        m->z             = *(float *)(item + 12);  /* item[3] = Z */
        m->is_positive   = (type == 'P' || type == 'p') ? 1 : 0;
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Config Reader
 * ═══════════════════════════════════════════════════════════════════════════ */

static void read_config(void) {
    char path[MAX_PATH];
    HMODULE hSelf = NULL;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                          | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                        (LPCSTR)&read_config, &hSelf);
    GetModuleFileNameA(hSelf, path, MAX_PATH);
    char *p = strrchr(path, '\\');
    if (p) strcpy(p + 1, "magnet_config.txt");
    else   strcat(path, "\\magnet_config.txt");

    FILE *f = fopen(path, "r");
    if (!f) {
        /* Auto-generate config with defaults */
        f = fopen(path, "w");
        if (f) {
            fprintf(f, "# Magnet Mod Config\n");
            fprintf(f, "# S1 points: MAGNET(P)=attract, MAGNET(N)=repel\n\n");
            fprintf(f, "range = 300.0\n");
            fprintf(f, "strength = 0.5\n");
            fprintf(f, "max_force = 3.0\n");
            fclose(f);
        }
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        char *s = line;
        while (*s == ' ' || *s == '\t') s++;
        if (*s == '#' || *s == '\n' || *s == '\r' || *s == '\0') continue;

        char key[64];
        float val;
        if (sscanf(s, "%63[^=] = %f", key, &val) == 2) {
            /* Trim whitespace from key */
            char *k = key;
            while (*k == ' ') k++;
            char *end = k + strlen(k) - 1;
            while (end > k && (*end == ' ' || *end == '\t')) *end-- = '\0';

            if (_stricmp(k, "range")     == 0) g_range     = val;
            else if (_stricmp(k, "strength") == 0) g_strength   = val;
            else if (_stricmp(k, "max_force")== 0) g_max_force = val;
        }
    }
    fclose(f);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Magnet Update Thread
 *
 * Runs at ~60 fps. Reads ball position, computes total magnet force,
 * applies by modifying ball position directly (ball+0x164/0x168/0x16C).
 * Also writes to PhysicsObject velocity fields for momentum continuity.
 * ═══════════════════════════════════════════════════════════════════════════ */

static DWORD WINAPI MagnetThread(LPVOID param) {
    /* Wait for App global to be allocated */
    int wait = 0;
    while (!(*(DWORD *)GLOBAL_APP_PTR) && wait < 100) {
        Sleep(100); wait++;
    }
    if (!(*(DWORD *)GLOBAL_APP_PTR)) return 0;
    Sleep(1000);   /* Let the game stabilize */

    read_config();

    while (1) {
        Sleep(16);   /* ~60 fps */

        DWORD board = get_board_v2();
        if (!board) continue;

        /* Re-scan magnets when board changes (new level loaded) */
        if (board != g_last_board) {
            scan_magnets(board);
        }

        if (g_magnet_count == 0) continue;

        DWORD ball = get_player_ball_v2();
        if (!ball) continue;

        /* Read ball position */
        if (IsBadReadPtr((void *)(ball + 0x164), 12)) continue;
        float bx = *(float *)(ball + 0x164);
        float by = *(float *)(ball + 0x168);
        float bz = *(float *)(ball + 0x16C);

        /* Skip if ball position not yet initialized */
        if (bx == 0.0f && by == 0.0f && bz == 0.0f) continue;

        /* Compute total magnet force */
        float total_fx = 0.0f, total_fy = 0.0f, total_fz = 0.0f;

        for (int i = 0; i < g_magnet_count; i++) {
            MagnetPoint *m = &g_magnets[i];

            float dx = m->x - bx;
            float dy = m->y - by;
            float dz = m->z - bz;
            float dist_sq = dx * dx + dy * dy + dz * dz;

            if (dist_sq > g_range * g_range) continue;
            if (dist_sq < 1.0f) continue;   /* too close, skip */

            float dist = sqrtf(dist_sq);

            /* Linear falloff: full force at dist=0, zero at dist=range */
            float falloff = 1.0f - (dist / g_range);
            if (falloff <= 0.0f) continue;

            float force = g_strength * falloff;

            /* Normalize direction */
            float inv_dist = 1.0f / dist;
            float nx = dx * inv_dist;
            float ny = dy * inv_dist;
            float nz = dz * inv_dist;

            if (m->is_positive) {
                /* Attract: pull ball toward magnet */
                total_fx += nx * force;
                total_fy += ny * force;
                total_fz += nz * force;
            } else {
                /* Repel: push ball away from magnet */
                total_fx -= nx * force;
                total_fy -= ny * force;
                total_fz -= nz * force;
            }
        }

        /* Clamp total force to max_force */
        float total_mag = sqrtf(total_fx * total_fx
                              + total_fy * total_fy
                              + total_fz * total_fz);
        if (total_mag > g_max_force) {
            float scale = g_max_force / total_mag;
            total_fx *= scale;
            total_fy *= scale;
            total_fz *= scale;
        }

        /* Apply force by modifying ball position directly.
         * This is the proven approach for player-affecting mods
         * (Ball_Update is not called for the player in race mode,
         *  so force accumulators at ball+0x170 are never consumed). */
        if (total_fx != 0.0f || total_fy != 0.0f || total_fz != 0.0f) {
            *(float *)(ball + 0x164) = bx + total_fx;
            *(float *)(ball + 0x168) = by + total_fy;
            *(float *)(ball + 0x16C) = bz + total_fz;

            /* Also write to PhysicsObject velocity for momentum continuity.
             * ball+0x1A4 = pointer to PhysicsObject/collision_node.
             * Velocity at +0xC98(X), +0xC9C(Y), +0xCA0(Z), speed at +0xC64. */
            if (!IsBadReadPtr((void *)(ball + 0x1A4), 4)) {
                DWORD physics = *(DWORD *)(ball + 0x1A4);
                if (physics && physics > 0x10000
                 && !IsBadReadPtr((void *)(physics + 0xCA0), 4)) {
                    *(float *)(physics + 0xC98) += total_fx;
                    *(float *)(physics + 0xC9C) += total_fy;
                    *(float *)(physics + 0xCA0) += total_fz;

                    /* Recompute speed magnitude */
                    float vx = *(float *)(physics + 0xC98);
                    float vy = *(float *)(physics + 0xC9C);
                    float vz = *(float *)(physics + 0xCA0);
                    *(float *)(physics + 0xC64) = sqrtf(vx * vx
                                                      + vy * vy
                                                      + vz * vz);
                }
            }
        }
    }
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DLL Entry Point
 * ═══════════════════════════════════════════════════════════════════════════ */

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        load_real_bass();
        CreateThread(NULL, 0, MagnetThread, NULL, 0, NULL);
    }
    return TRUE;
}
