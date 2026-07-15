/*
 * custom_entities.c — Hamsterball Custom Entities Mod v7
 *
 * bass.dll proxy mod. Simplified GRID system:
 *   Scans S1 ref points for (GRIDxx) suffixes.
 *   Makes all GRID objects visible (posScale = 1.0).
 *   Hides all GRID objects EXCEPT GRID02 (test mode).
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll custom_entities.c \
 *     -I../shared -lwinmm -Wl,--enable-stdcall-fixup -O2 -static \
 *     -static-libgcc -Wl,--add-stdcall-alias -msse2 -mfpmath=sse -lshlwapi
 */

#include "bass_proxy.h"
#include <shlwapi.h>
#include <ctype.h>
#include <stdio.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * Structure offsets
 * ═══════════════════════════════════════════════════════════════════════════ */

#define BOARD_LEVEL             0x8AC
#define LEVEL_SCENEOBJECT       0x480
#define SCENEOBJ_S1_LIST        0x894

#define S1ENTRY_NAME             0x00
#define S1ENTRY_POS_X            0x04
#define S1ENTRY_POS_Y            0x08
#define S1ENTRY_POS_Z            0x0C

/* S1 entry has embedded EntityTransform starting at +0x2C (vtable=0x4D8E68).
 * The EntityTransform's posScale is at ET+0x20 = S1+0x4C.
 * But the S1 entry also has scale values at +0x1C/+0x20/+0x24 (all 1.0).
 * We write to BOTH locations to maximize chance of affecting visibility. */

#define S1_SCALE_X               0x1C
#define S1_SCALE_Y               0x20
#define S1_SCALE_Z               0x24
#define S1_ET_BASE               0x2C
#define S1_ET_POSSCALE           0x4C  /* ET+0x20 */

/* ═══════════════════════════════════════════════════════════════════════════
 * State
 * ═══════════════════════════════════════════════════════════════════════════ */

static HANDLE g_thread = NULL;
static volatile int g_running = 1;
static char g_game_dir[MAX_PATH] = {0};
static DWORD g_last_board = 0;

/* ═══════════════════════════════════════════════════════════════════════════
 * Helpers
 * ═══════════════════════════════════════════════════════════════════════════ */

/* get_board is provided by bass_proxy.h — it returns the App pointer */

static DWORD get_level(DWORD app) {
    if (!app) return 0;
    if (IsBadReadPtr((void*)(app + BOARD_LEVEL), 4)) return 0;
    DWORD level = *(DWORD*)(app + BOARD_LEVEL);
    if (!level || level < 0x10000) return 0;
    return level;
}

static DWORD get_sceneobj(DWORD app) {
    DWORD level = get_level(app);
    if (!level) return 0;
    if (IsBadReadPtr((void*)(level + LEVEL_SCENEOBJECT), 4)) return 0;
    DWORD sceneobj = *(DWORD*)(level + LEVEL_SCENEOBJECT);
    if (!sceneobj || sceneobj < 0x10000) return 0;
    return sceneobj;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * GRID: Parse (GRIDxx) from name
 * ═══════════════════════════════════════════════════════════════════════════ */

static int parse_grid_flag(const char* name) {
    const char* p = name;
    while ((p = strstr(p, "(GRID")) != NULL) {
        const char* digits = p + 5;
        if (isdigit((unsigned char)digits[0]) && isdigit((unsigned char)digits[1]) &&
            digits[2] == ')') {
            int num = (digits[0] - '0') * 10 + (digits[1] - '0');
            if (num >= 1 && num <= 99) return num;
        }
        if (isdigit((unsigned char)digits[0]) && digits[1] == ')') {
            int num = digits[0] - '0';
            if (num >= 1 && num <= 9) return num;
        }
        p++;
    }
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * GRID: Scan S1 ref points and apply visibility
 *
 * For each S1 ref point with (GRIDxx):
 *   - If grid_num == 2: make VISIBLE (scale = 1.0)
 *   - If grid_num != 2: make HIDDEN (scale = 0.0)
 *
 * We write to S1+0x1C/+0x20/+0x24 (scale XYZ) AND S1+0x4C (ET posScale).
 * ═══════════════════════════════════════════════════════════════════════════ */

static void apply_grid_visibility(DWORD app, FILE* logf) {
    DWORD level = get_level(app);
    if (!level) {
        if (logf) fprintf(logf, "  GRID: no level\n");
        return;
    }

    /* Try level+0x480 for sceneobj */
    DWORD sceneobj = 0;
    if (!IsBadReadPtr((void*)(level + LEVEL_SCENEOBJECT), 4)) {
        sceneobj = *(DWORD*)(level + LEVEL_SCENEOBJECT);
    }
    
    if (!sceneobj || sceneobj < 0x10000) {
        if (logf) {
            fprintf(logf, "  GRID: level+0x480 sceneobj invalid (0x%08X), scanning level struct for heap ptrs...\n", sceneobj);
            /* Dump level struct to find sceneobj pointer */
            int off;
            for (off = 0; off < 0x600; off += 4) {
                if (IsBadReadPtr((void*)(level + off), 4)) continue;
                DWORD val = *(DWORD*)(level + off);
                if (val > 0x00100000 && val < 0x10000000) {
                    /* Check if this pointer leads to something with an S1 list */
                    fprintf(logf, "    level+0x%03X: 0x%08X", off, val);
                    /* Try reading val+0x894 (S1 list count) to see if it's a sceneobj */
                    if (!IsBadReadPtr((void*)(val + 0x898), 4)) {
                        int s1cnt = *(int*)(val + 0x898);
                        if (s1cnt > 0 && s1cnt < 10000) {
                            fprintf(logf, " -> +0x898 has S1 count=%d!", s1cnt);
                        }
                    }
                    fprintf(logf, "\n");
                }
            }
        }
        return;
    }

    /* S1 list is embedded AthenaList at sceneobj+0x894 */
    DWORD s1_list = sceneobj + SCENEOBJ_S1_LIST;
    if (IsBadReadPtr((void*)(s1_list + 0x04), 4)) return;
    int s1_count = *(int*)(s1_list + 0x04);
    if (s1_count < 1 || s1_count > 10000) return;

    if (IsBadReadPtr((void*)(s1_list + 0x40C), 4)) return;
    DWORD* s1_array = *(DWORD**)(s1_list + 0x40C);
    if (!s1_array || IsBadReadPtr(s1_array, s1_count * 4)) return;

    if (logf) fprintf(logf, "  GRID: scanning %d S1 ref points\n", s1_count);

    int found = 0;
    int i;
    for (i = 0; i < s1_count; i++) {
        DWORD entry = s1_array[i];
        if (!entry || entry < 0x10000) continue;
        if (IsBadReadPtr((void*)entry, 0x50)) continue;

        char* name = *(char**)(entry + S1ENTRY_NAME);
        if (!name || IsBadReadPtr(name, 4)) continue;

        int grid_num = parse_grid_flag(name);
        if (grid_num == 0) continue;

        found++;

        /* GRID02 = visible, all others = hidden */
        float scale = (grid_num == 2) ? 1.0f : 0.0f;

        /* Write scale values at S1+0x1C/+0x20/+0x24 */
        if (!IsBadWritePtr((void*)(entry + S1_SCALE_X), 4))
            *(float*)(entry + S1_SCALE_X) = scale;
        if (!IsBadWritePtr((void*)(entry + S1_SCALE_Y), 4))
            *(float*)(entry + S1_SCALE_Y) = scale;
        if (!IsBadWritePtr((void*)(entry + S1_SCALE_Z), 4))
            *(float*)(entry + S1_SCALE_Z) = scale;

        /* Also write ET posScale at S1+0x4C */
        if (!IsBadWritePtr((void*)(entry + S1_ET_POSSCALE), 4))
            *(float*)(entry + S1_ET_POSSCALE) = scale;

        if (logf) {
            float px = *(float*)(entry + S1ENTRY_POS_X);
            float py = *(float*)(entry + S1ENTRY_POS_Y);
            float pz = *(float*)(entry + S1ENTRY_POS_Z);
            fprintf(logf, "  GRID: S1[%d] name='%s' grid=%d pos=(%.1f,%.1f,%.1f) -> %s (scale=%.1f)\n",
                    i, name, grid_num, px, py, pz,
                    (grid_num == 2) ? "VISIBLE" : "HIDDEN", scale);
        }
    }

    if (logf) fprintf(logf, "  GRID: found %d GRID ref points\n", found);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Main mod thread
 * ═══════════════════════════════════════════════════════════════════════════ */

static void init_game_dir(void) {
    char path[MAX_PATH];
    HMODULE hSelf = NULL;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                      | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                      (LPCSTR)&init_game_dir, &hSelf);
    if (hSelf && GetModuleFileNameA(hSelf, path, MAX_PATH) > 0) {
        char *p = strrchr(path, '\\');
        if (p) { *p = '\0'; strcpy(g_game_dir, path); return; }
    }
    if (GetCurrentDirectoryA(MAX_PATH, path) > 0) {
        strcpy(g_game_dir, path);
    }
}

static DWORD WINAPI mod_thread(LPVOID param) {
    Sleep(3000);

    {
        char log_path[MAX_PATH];
        snprintf(log_path, MAX_PATH, "%s\\custom_entities.log", g_game_dir);
        FILE* f = NULL;
        fopen_s(&f, log_path, "a");
        if (f) {
            fprintf(f, "=== Custom Entities Mod v7 Started ===\n");
            fprintf(f, "Game dir: %s\n", g_game_dir);
            fclose(f);
        }
    }

    while (g_running) {
        DWORD app = *(DWORD*)0x005341E0;
        if (!app || app < 0x10000 || IsBadReadPtr((void*)app, 0x1000)) {
            Sleep(100);
            continue;
        }

        DWORD level = get_level(app);
        if (level != g_last_board) {
            g_last_board = level;

            if (level) {
                /* Poll until sceneobj is ready (up to 10 seconds) */
                int poll;
                for (poll = 0; poll < 200; poll++) {
                    DWORD so = get_sceneobj(app);
                    if (so) break;
                    Sleep(50);
                }

                char log_path[MAX_PATH];
                snprintf(log_path, MAX_PATH, "%s\\custom_entities.log", g_game_dir);
                FILE* logf = NULL;
                fopen_s(&logf, log_path, "a");

                if (logf) fprintf(logf, "\n--- Level loaded (app=0x%08X, level=0x%08X, after %dms poll) ---\n", app, level, poll * 50);

                /* Apply GRID visibility: GRID02 visible, others hidden */
                apply_grid_visibility(app, logf);

                if (logf) {
                    fprintf(logf, "Done.\n\n");
                    fclose(logf);
                }
            }
        }

        /* Re-apply every 2 seconds (in case game resets values) */
        Sleep(2000);
        if (g_last_board) {
            DWORD app2 = *(DWORD*)0x005341E0;
            if (app2 && app2 >= 0x10000 && !IsBadReadPtr((void*)app2, 0x1000)) {
                /* Only apply if sceneobj is valid */
                if (get_sceneobj(app2)) {
                    apply_grid_visibility(app2, NULL);
                }
            }
        }
    }

    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DllMain
 * ═══════════════════════════════════════════════════════════════════════════ */

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        load_real_bass();
        init_game_dir();
        g_thread = CreateThread(NULL, 0, mod_thread, NULL, 0, NULL);
    }
    else if (reason == DLL_PROCESS_DETACH) {
        g_running = 0;
        if (g_thread) {
            WaitForSingleObject(g_thread, 2000);
            CloseHandle(g_thread);
        }
    }
    return TRUE;
}
