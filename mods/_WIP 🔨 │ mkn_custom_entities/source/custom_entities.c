/*
 * custom_entities.c — Hamsterball Custom Entities Mod v6
 *
 * bass.dll proxy mod. GRID system:
 *   Scans S1 ref points for (GRIDxx) suffixes in their names.
 *   Matches ref points to loaded MeshBuffers by position.
 *   Cycles visibility: shows meshes matching current grid_counter, hides others.
 *
 * Config (next to bass.dll):
 *   grid_speed = 10.0    (ticks between grid advances, default 10)
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
 * Structure offsets (from Ghidra decompilation)
 * ═══════════════════════════════════════════════════════════════════════════ */

#define BOARD_LEVEL             0x8AC
#define LEVEL_SCENEOBJECT       0x480

#define MESHWORLD_OFFSET         0x08
#define MESHWORLD_MB_COUNT       0x30  /* AthenaList+0x04 (embedded at MW+0x2C) */
#define MESHWORLD_MB_DATA        0x438 /* AthenaList+0x40C */
#define MESHWORLD_RENDERCTX_PTR  0x28

#define MESHBUFFER_NAME          0x864
#define MESHBUFFER_CTX_INDEX     0x04

#define SCENEOBJ_S1_LIST         0x894

#define S1ENTRY_NAME             0x00
#define S1ENTRY_POS_X            0x04
#define S1ENTRY_POS_Y            0x08
#define S1ENTRY_POS_Z            0x0C
#define S1ENTRY_ROT_X            0x10
#define S1ENTRY_ROT_Y            0x14
#define S1ENTRY_ROT_Z            0x18
#define S1ENTRY_SIZE             0x1C

/* EntityTransform is 0x50 bytes, stored at MeshWorld+0x28 + ctx_idx * 0x50 */
typedef struct {
    DWORD  vtable;      /* +0x00 */
    float  rotX;        /* +0x04 */
    float  rotY;        /* +0x08 */
    float  rotZ;        /* +0x0C */
    float  rotScale;    /* +0x10 */
    float  posX;        /* +0x14 */
    float  posY;        /* +0x18 */
    float  posZ;        /* +0x1C */
    float  posScale;    /* +0x20 */
    float  extra[12];   /* +0x24 - +0x4F */
} EntityTransform;

/* ═══════════════════════════════════════════════════════════════════════════
 * GRID SYSTEM
 * ═══════════════════════════════════════════════════════════════════════════ */

#define MAX_GRID_MESHES 256

typedef struct {
    EntityTransform* transform;  /* pointer to the mesh's render transform */
    int grid_num;                /* GRID number (1-99) from S1 ref point name */
    float src_x, src_y, src_z;   /* S1 ref point position (for logging) */
} GridMesh;

static GridMesh g_grid_meshes[MAX_GRID_MESHES];
static int g_grid_mesh_count = 0;
static int g_grid_counter = 1;
static int g_grid_max = 0;
static int g_grid_tick_counter = 0;
static float g_grid_speed = 10.0f;

/* ═══════════════════════════════════════════════════════════════════════════
 * Common state
 * ═══════════════════════════════════════════════════════════════════════════ */

static HANDLE g_thread = NULL;
static volatile int g_running = 1;
static char g_game_dir[MAX_PATH] = {0};
static DWORD g_last_board = 0;

/* ═══════════════════════════════════════════════════════════════════════════
 * Pointer chain helpers
 * ═══════════════════════════════════════════════════════════════════════════ */

static DWORD get_level(DWORD board) {
    if (!board) return 0;
    if (IsBadReadPtr((void*)(board + BOARD_LEVEL), 4)) return 0;
    DWORD level = *(DWORD*)(board + BOARD_LEVEL);
    if (!level || level < 0x10000) return 0;
    return level;
}

static DWORD get_sceneobj(DWORD board) {
    DWORD level = get_level(board);
    if (!level) return 0;
    if (IsBadReadPtr((void*)(level + LEVEL_SCENEOBJECT), 4)) return 0;
    DWORD sceneobj = *(DWORD*)(level + LEVEL_SCENEOBJECT);
    if (!sceneobj || sceneobj < 0x10000) return 0;
    return sceneobj;
}

static DWORD get_meshworld(DWORD board) {
    DWORD sceneobj = get_sceneobj(board);
    if (!sceneobj) return 0;
    if (IsBadReadPtr((void*)(sceneobj + MESHWORLD_OFFSET), 4)) return 0;
    DWORD mw = *(DWORD*)(sceneobj + MESHWORLD_OFFSET);
    if (!mw || mw < 0x10000) return 0;
    return mw;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Init / config
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

static void load_config(void) {
    char path[MAX_PATH];
    snprintf(path, MAX_PATH, "%s\\custom_entities.txt", g_game_dir);
    g_grid_speed = 10.0f;
    FILE* f = NULL;
    if (fopen_s(&f, path, "r") != 0 || !f) {
        FILE* gen = NULL;
        if (fopen_s(&gen, path, "w") == 0 && gen) {
            fprintf(gen, "# Custom Entities Mod Configuration\n\n");
            fprintf(gen, "# Ticks between grid advances (1 tick = ~16ms)\n");
            fprintf(gen, "grid_speed = 10.0\n");
            fclose(gen);
        }
        return;
    }
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        char key[128], val[128];
        if (sscanf(line, "%127[^=]=%127s", key, val) == 2) {
            char* k = key;
            while (*k == ' ' || *k == '\t') k++;
            char* end = k + strlen(k) - 1;
            while (end > k && (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n'))
                *end-- = 0;
            if (_stricmp(k, "grid_speed") == 0) {
                g_grid_speed = (float)atof(val);
                if (g_grid_speed < 1.0f) g_grid_speed = 1.0f;
            }
        }
    }
    fclose(f);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * GRID: Parse (GRIDxx) from an S1 ref point name
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
 * GRID: Scan S1 ref points for (GRIDxx) flags, then match to MeshBuffers
 *
 * The GRID flag is in the S1 ref point name (e.g. "testcube(GRID01)").
 * The game loads the referenced .MESHWORLD and creates MeshBuffers named
 * after the mesh file (e.g. "testcube"), NOT after the ref point name.
 * So we must:
 *   1. Scan S1 ref points for GRID flags → record grid_num + position
 *   2. Scan MeshBuffers → match by position to find EntityTransforms
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Step 1: S1 ref point data (name has GRID flag, position for matching) */
typedef struct {
    int grid_num;
    float posX, posY, posZ;
} S1GridRef;

static S1GridRef g_s1_grid_refs[64];
static int g_s1_grid_ref_count = 0;

static void scan_s1_for_grid(DWORD board, FILE* logf) {
    g_s1_grid_ref_count = 0;

    DWORD level = get_level(board);
    if (!level) return;
    if (IsBadReadPtr((void*)(level + LEVEL_SCENEOBJECT), 4)) return;
    DWORD sceneobj = *(DWORD*)(level + LEVEL_SCENEOBJECT);
    if (!sceneobj || sceneobj < 0x10000) return;

    /* S1 list is embedded AthenaList at sceneobj+0x894 */
    DWORD s1_list = sceneobj + SCENEOBJ_S1_LIST;
    if (IsBadReadPtr((void*)(s1_list + 0x04), 4)) return;
    int s1_count = *(int*)(s1_list + 0x04);
    if (s1_count < 1 || s1_count > 10000) return;

    if (IsBadReadPtr((void*)(s1_list + 0x40C), 4)) return;
    DWORD* s1_array = *(DWORD**)(s1_list + 0x40C);
    if (!s1_array || IsBadReadPtr(s1_array, s1_count * 4)) return;

    if (logf) fprintf(logf, "  GRID: S1 scan: %d ref points\n", s1_count);

    int i;
    for (i = 0; i < s1_count && g_s1_grid_ref_count < 64; i++) {
        DWORD entry = s1_array[i];
        if (!entry || entry < 0x10000) continue;
        if (IsBadReadPtr((void*)entry, S1ENTRY_SIZE)) continue;
        if (IsBadReadPtr((void*)(entry + S1ENTRY_NAME), 4)) continue;
        char* name = *(char**)(entry + S1ENTRY_NAME);
        if (!name || IsBadReadPtr(name, 4)) continue;

        int grid_num = parse_grid_flag(name);
        if (grid_num == 0) continue;

        float px = *(float*)(entry + S1ENTRY_POS_X);
        float py = *(float*)(entry + S1ENTRY_POS_Y);
        float pz = *(float*)(entry + S1ENTRY_POS_Z);

        g_s1_grid_refs[g_s1_grid_ref_count].grid_num = grid_num;
        g_s1_grid_refs[g_s1_grid_ref_count].posX = px;
        g_s1_grid_refs[g_s1_grid_ref_count].posY = py;
        g_s1_grid_refs[g_s1_grid_ref_count].posZ = pz;
        g_s1_grid_ref_count++;

        if (grid_num > g_grid_max) g_grid_max = grid_num;

        if (logf) fprintf(logf, "  GRID: S1[%d] name='%s' grid=%d pos=(%.1f,%.1f,%.1f)\n",
                i, name, grid_num, px, py, pz);
    }

    if (logf) fprintf(logf, "  GRID: Found %d GRID ref points, max=%d\n",
            g_s1_grid_ref_count, g_grid_max);
}

/* Step 2: Match S1 grid refs to MeshBuffers by position */
static void match_meshbuffers_to_grid(DWORD board, FILE* logf) {
    g_grid_mesh_count = 0;
    if (g_s1_grid_ref_count == 0) return;

    DWORD mw = get_meshworld(board);
    if (!mw) {
        if (logf) fprintf(logf, "  GRID: get_meshworld returned NULL\n");
        return;
    }
    if (logf) fprintf(logf, "  GRID: meshworld=0x%08X\n", mw);

    if (IsBadReadPtr((void*)(mw + MESHWORLD_MB_COUNT), 4)) return;
    int mb_count = *(int*)(mw + MESHWORLD_MB_COUNT);
    if (logf) fprintf(logf, "  GRID: mb_count=%d\n", mb_count);
    if (mb_count < 1 || mb_count > 10000) return;

    if (IsBadReadPtr((void*)(mw + MESHWORLD_MB_DATA), 4)) return;
    DWORD* mb_array = *(DWORD**)(mw + MESHWORLD_MB_DATA);
    if (!mb_array || IsBadReadPtr(mb_array, mb_count * 4)) return;

    if (IsBadReadPtr((void*)(mw + MESHWORLD_RENDERCTX_PTR), 4)) return;
    EntityTransform* transforms = *(EntityTransform**)(mw + MESHWORLD_RENDERCTX_PTR);
    if (!transforms) {
        if (logf) fprintf(logf, "  GRID: transforms NULL\n");
        return;
    }

    int i, j;
    for (i = 0; i < mb_count && g_grid_mesh_count < MAX_GRID_MESHES; i++) {
        DWORD mb = mb_array[i];
        if (!mb || mb < 0x10000) continue;
        if (IsBadReadPtr((void*)mb, 0x900)) continue;
        if (IsBadReadPtr((void*)(mb + MESHBUFFER_CTX_INDEX), 4)) continue;
        DWORD ctx_idx = *(DWORD*)(mb + MESHBUFFER_CTX_INDEX);
        if (ctx_idx > 10000) continue;

        EntityTransform* t = &transforms[ctx_idx];
        if (IsBadReadPtr(t, sizeof(EntityTransform))) continue;

        /* Read MeshBuffer name for logging */
        char* mb_name = NULL;
        if (!IsBadReadPtr((void*)(mb + MESHBUFFER_NAME), 4)) {
            mb_name = *(char**)(mb + MESHBUFFER_NAME);
            if (mb_name && IsBadReadPtr(mb_name, 4)) mb_name = NULL;
        }

        /* Match by position: find S1 ref with closest position */
        float best_dist = 1e9f;
        int best_ref = -1;
        for (j = 0; j < g_s1_grid_ref_count; j++) {
            float dx = t->posX - g_s1_grid_refs[j].posX;
            float dy = t->posY - g_s1_grid_refs[j].posY;
            float dz = t->posZ - g_s1_grid_refs[j].posZ;
            if (dx < 0) dx = -dx;
            if (dy < 0) dy = -dy;
            if (dz < 0) dz = -dz;
            float dist = dx + dy + dz;
            if (dist < best_dist) {
                best_dist = dist;
                best_ref = j;
            }
        }

        /* Accept match if within 50 units */
        if (best_ref >= 0 && best_dist < 150.0f) {
            g_grid_meshes[g_grid_mesh_count].transform = t;
            g_grid_meshes[g_grid_mesh_count].grid_num = g_s1_grid_refs[best_ref].grid_num;
            g_grid_meshes[g_grid_mesh_count].src_x = g_s1_grid_refs[best_ref].posX;
            g_grid_meshes[g_grid_mesh_count].src_y = g_s1_grid_refs[best_ref].posY;
            g_grid_meshes[g_grid_mesh_count].src_z = g_s1_grid_refs[best_ref].posZ;
            g_grid_mesh_count++;

            if (logf) fprintf(logf, "  GRID: MATCH mb[%d] name='%s' ctx=%d pos=(%.1f,%.1f,%.1f) -> grid=%d (dist=%.1f)\n",
                    i, mb_name ? mb_name : "?", ctx_idx, t->posX, t->posY, t->posZ,
                    g_s1_grid_refs[best_ref].grid_num, best_dist);
        } else if (logf && mb_name) {
            fprintf(logf, "  GRID: mb[%d] name='%s' pos=(%.1f,%.1f,%.1f) no match (best_dist=%.1f)\n",
                    i, mb_name, t->posX, t->posY, t->posZ, best_dist);
        }
    }

    if (logf) fprintf(logf, "  GRID: Matched %d grid meshes\n", g_grid_mesh_count);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * GRID: Apply visibility
 * ═══════════════════════════════════════════════════════════════════════════ */

static void grid_apply_visibility(void) {
    int i;
    for (i = 0; i < g_grid_mesh_count; i++) {
        GridMesh* gm = &g_grid_meshes[i];
        if (!gm->transform || IsBadWritePtr(gm->transform, sizeof(EntityTransform)))
            continue;
        gm->transform->posScale = 0.0f;  /* TEST: hide ALL grid meshes */
    }
}

static void grid_advance(void) {
    g_grid_tick_counter++;
    if ((float)g_grid_tick_counter >= g_grid_speed) {
        g_grid_tick_counter = 0;
        g_grid_counter++;
        if (g_grid_counter > g_grid_max || g_grid_max == 0) {
            g_grid_counter = 1;
        }
        /* Don't apply during test mode */
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Main mod thread
 * ═══════════════════════════════════════════════════════════════════════════ */

static DWORD WINAPI mod_thread(LPVOID param) {
    Sleep(3000);
    load_config();

    {
        char log_path[MAX_PATH];
        snprintf(log_path, MAX_PATH, "%s\\custom_entities.log", g_game_dir);
        FILE* f = NULL;
        fopen_s(&f, log_path, "a");
        if (f) {
            fprintf(f, "=== Custom Entities Mod v6 Started ===\n");
            fprintf(f, "Game dir: %s\n", g_game_dir);
            fprintf(f, "Grid speed: %.1f ticks\n", g_grid_speed);
            fclose(f);
        }
    }

    while (g_running) {
        DWORD board = get_board();

        if (board != g_last_board) {
            if (g_last_board != 0) {
                g_grid_mesh_count = 0;
                g_s1_grid_ref_count = 0;
                g_grid_counter = 1;
                g_grid_max = 0;
                g_grid_tick_counter = 0;
            }
            g_last_board = board;

            if (board) {
                /* Poll until MeshWorld is ready */
                int poll;
                for (poll = 0; poll < 120; poll++) {
                    DWORD mw = get_meshworld(board);
                    if (mw) {
                        if (!IsBadReadPtr((void*)(mw + MESHWORLD_RENDERCTX_PTR), 4)) {
                            DWORD xform_ptr = *(DWORD*)(mw + MESHWORLD_RENDERCTX_PTR);
                            if (xform_ptr != 0) break;
                        }
                    }
                    Sleep(50);
                }

                char log_path[MAX_PATH];
                snprintf(log_path, MAX_PATH, "%s\\custom_entities.log", g_game_dir);
                FILE* logf = NULL;
                fopen_s(&logf, log_path, "a");

                if (logf) fprintf(logf, "\n--- Level loaded, scanning (after %dms poll) ---\n", poll * 50);

                /* Step 1: Scan S1 ref points for GRID flags */
                scan_s1_for_grid(board, logf);

                /* Step 2: Match MeshBuffers to S1 refs by position */
                match_meshbuffers_to_grid(board, logf);

                /* Step 3: Apply initial visibility */
                if (g_grid_mesh_count > 0) {
                    grid_apply_visibility();
                }

                if (logf) {
                    fprintf(logf, "Scan complete: %d grid meshes\n\n", g_grid_mesh_count);
                    fclose(logf);
                }
            }
        }

        Sleep(16);
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
