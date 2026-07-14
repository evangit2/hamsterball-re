/*
 * custom_entities.c — Hamsterball Custom Entities Mod v5
 *
 * bass.dll proxy mod. Two features:
 *
 * 1. GRID SYSTEM (NEW)
 *    Scans MeshBuffer names for (GRIDxx) suffixes (GRID01-GRID99).
 *    Cycles visibility: shows all meshes matching current grid_counter,
 *    hides all others. Advances counter every grid_speed ticks.
 *    Loops back to 1 when counter exceeds highest GRID number in level.
 *
 * 2. LEGACY CE ENTITY SYSTEM (preserved from v4)
 *    Scans S1 ref points for entity names containing "CE" (uppercase).
 *    Loads behavior DLLs from Levels/CustomEntities/.
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

#define MESHWORLD_OFFSET            0x08
#define MESHWORLD_RENDERCTX_PTR     0x28

/* AthenaList is EMBEDDED at MeshWorld+0x2C (not a pointer — inline struct).
 * AthenaList+0x04 = count  → MeshWorld+0x30
 * AthenaList+0x40C = data   → MeshWorld+0x438
 * Verified from Level_LoadMeshes decompilation:
 *   iVar5 = MeshWorld*
 *   count = *(int*)(iVar5 + 0x30)
 *   data  = *(int**)(iVar5 + 0x438)
 */
#define MESHWORLD_MB_COUNT          0x30
#define MESHWORLD_MB_DATA           0x438

#define MESHBUFFER_NAME             0x864
#define MESHBUFFER_CTX_INDEX        0x04

#define BOARD_SCENE                 0x878
#define BOARD_LEVEL                 0x8AC

#define LEVEL_SCENEOBJECT           0x480
#define SCENEOBJ_S1_LIST            0x894

#define S1ENTRY_NAME                0x00
#define S1ENTRY_POS_X               0x04
#define S1ENTRY_POS_Y               0x08
#define S1ENTRY_POS_Z               0x0C
#define S1ENTRY_ROT_X               0x10
#define S1ENTRY_ROT_Y               0x14
#define S1ENTRY_ROT_Z               0x18
#define S1ENTRY_SIZE                0x1C

/* EntityTransform is 0x50 bytes per MeshBuffer, stored at MeshWorld+0x28 */
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
    int grid_num;                /* GRID number (1-99) from name */
} GridMesh;

static GridMesh g_grid_meshes[MAX_GRID_MESHES];
static int g_grid_mesh_count = 0;
static int g_grid_counter = 1;
static int g_grid_max = 0;           /* highest GRID number found in level */
static int g_grid_tick_counter = 0;
static float g_grid_speed = 10.0f;   /* ticks between grid advances */

/* ═══════════════════════════════════════════════════════════════════════════
 * LEGACY CE ENTITY SYSTEM (from v4)
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef void (__cdecl *Behavior_Init_t)(EntityTransform*, void*);
typedef void (__cdecl *Behavior_Update_t)(EntityTransform*, void*);
typedef void (__cdecl *Behavior_Shutdown_t)(void);

#define MAX_CUSTOM_ENTITIES 64

typedef struct {
    EntityTransform*  transform;
    HMODULE            behavior_dll;
    Behavior_Init_t    init_fn;
    Behavior_Update_t  update_fn;
    Behavior_Shutdown_t shutdown_fn;
    char               entity_name[256];
    int                initialized;
    float              posX, posY, posZ;
    float              rotX, rotY, rotZ;
} CustomEntity;

static CustomEntity g_entities[MAX_CUSTOM_ENTITIES];
static int g_entity_count = 0;

/* ═══════════════════════════════════════════════════════════════════════════
 * Common state
 * ═══════════════════════════════════════════════════════════════════════════ */

static HANDLE g_thread = NULL;
static volatile int g_running = 1;
static char g_game_dir[MAX_PATH] = {0};
static char g_entities_dir[MAX_PATH] = {0};
static char g_levels_dir[MAX_PATH] = {0};
static DWORD g_last_board = 0;

/* ═══════════════════════════════════════════════════════════════════════════
 * Pointer chain helpers
 * ═══════════════════════════════════════════════════════════════════════════ */

static DWORD get_scene(DWORD board) {
    if (!board) return 0;
    if (IsBadReadPtr((void*)(board + BOARD_SCENE), 4)) return 0;
    DWORD scene = *(DWORD*)(board + BOARD_SCENE);
    if (!scene || scene < 0x10000) return 0;
    return scene;
}

static DWORD get_meshworld(DWORD scene) {
    if (!scene) return 0;
    if (IsBadReadPtr((void*)(scene + MESHWORLD_OFFSET), 4)) return 0;
    DWORD mw = *(DWORD*)(scene + MESHWORLD_OFFSET);
    if (!mw || mw < 0x10000) return 0;
    return mw;
}

static void init_game_dir(void) {
    char path[MAX_PATH];
    HMODULE hSelf = NULL;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                      | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                      (LPCSTR)&init_game_dir, &hSelf);
    if (hSelf && GetModuleFileNameA(hSelf, path, MAX_PATH) > 0) {
        char *p = strrchr(path, '\\');
        if (p) {
            *p = '\0';
            strcpy(g_game_dir, path);
            snprintf(g_levels_dir, MAX_PATH, "%s\\Levels", path);
            snprintf(g_entities_dir, MAX_PATH, "%s\\Levels\\CustomEntities", path);
            return;
        }
    }
    if (GetCurrentDirectoryA(MAX_PATH, path) > 0) {
        strcpy(g_game_dir, path);
        snprintf(g_levels_dir, MAX_PATH, "%s\\Levels", path);
        snprintf(g_entities_dir, MAX_PATH, "%s\\Levels\\CustomEntities", path);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Config loading
 * ═══════════════════════════════════════════════════════════════════════════ */

static void load_config(void) {
    char path[MAX_PATH];
    snprintf(path, MAX_PATH, "%s\\custom_entities.txt", g_game_dir);

    /* Defaults */
    g_grid_speed = 10.0f;

    FILE* f = NULL;
    if (fopen_s(&f, path, "r") != 0 || !f) {
        /* Auto-generate config with defaults */
        FILE* gen = NULL;
        if (fopen_s(&gen, path, "w") == 0 && gen) {
            fprintf(gen, "# Custom Entities Mod Configuration\n");
            fprintf(gen, "\n");
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
 * GRID: Parse (GRIDxx) from a MeshBuffer name
 * Returns grid number (1-99) or 0 if not found
 * ═══════════════════════════════════════════════════════════════════════════ */

static int parse_grid_flag(const char* name) {
    const char* p = name;
    while ((p = strstr(p, "(GRID")) != NULL) {
        /* p points at "(GRID" — check for digits after */
        const char* digits = p + 5;  /* skip "(GRID" */
        if (isdigit((unsigned char)digits[0]) && isdigit((unsigned char)digits[1]) &&
            digits[2] == ')') {
            int num = (digits[0] - '0') * 10 + (digits[1] - '0');
            if (num >= 1 && num <= 99) return num;
        }
        /* Also accept single digit: (GRID1) through (GRID9) */
        if (isdigit((unsigned char)digits[0]) && digits[1] == ')') {
            int num = digits[0] - '0';
            if (num >= 1 && num <= 9) return num;
        }
        p++;  /* continue searching past this match */
    }
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * GRID: Scan MeshBuffers for (GRIDxx) flags
 * ═══════════════════════════════════════════════════════════════════════════ */

static void scan_grid_meshes(DWORD board, FILE* logf) {
    g_grid_mesh_count = 0;
    g_grid_counter = 1;
    g_grid_max = 0;
    g_grid_tick_counter = 0;

    if (logf) fprintf(logf, "  GRID: scan start, board=0x%08X\n", board);

    DWORD scene = get_scene(board);
    if (!scene) { if (logf) fprintf(logf, "  GRID: get_scene returned NULL\n"); return; }
    if (logf) fprintf(logf, "  GRID: scene=0x%08X\n", scene);

    DWORD mw = get_meshworld(scene);
    if (!mw) { if (logf) fprintf(logf, "  GRID: get_meshworld returned NULL\n"); return; }
    if (logf) fprintf(logf, "  GRID: meshworld=0x%08X\n", mw);

    /* Dump raw bytes at key offsets for debugging */
    if (logf) {
        if (!IsBadReadPtr((void*)(mw + 0x24), 4)) {
            int raw_count = *(int*)(mw + 0x24);
            fprintf(logf, "  GRID: MeshWorld+0x24 (raw mb count from file) = %d\n", raw_count);
        }
        if (!IsBadReadPtr((void*)(mw + 0x28), 4)) {
            DWORD raw_xform = *(DWORD*)(mw + 0x28);
            fprintf(logf, "  GRID: MeshWorld+0x28 (EntityTransform array ptr) = 0x%08X\n", raw_xform);
        }
        if (!IsBadReadPtr((void*)(mw + 0x2C), 16)) {
            fprintf(logf, "  GRID: MeshWorld+0x2C raw bytes: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n",
                ((BYTE*)mw)[0x2C], ((BYTE*)mw)[0x2D], ((BYTE*)mw)[0x2E], ((BYTE*)mw)[0x2F],
                ((BYTE*)mw)[0x30], ((BYTE*)mw)[0x31], ((BYTE*)mw)[0x32], ((BYTE*)mw)[0x33],
                ((BYTE*)mw)[0x34], ((BYTE*)mw)[0x35], ((BYTE*)mw)[0x36], ((BYTE*)mw)[0x37],
                ((BYTE*)mw)[0x38], ((BYTE*)mw)[0x39], ((BYTE*)mw)[0x3A], ((BYTE*)mw)[0x3B]);
        }
    }

    /* Get MeshBuffer count and data array directly from MeshWorld (AthenaList is embedded) */
    if (IsBadReadPtr((void*)(mw + MESHWORLD_MB_COUNT), 4)) {
        if (logf) fprintf(logf, "  GRID: IsBadReadPtr at mw+0x%02X (MB_COUNT)\n", MESHWORLD_MB_COUNT);
        return;
    }
    int mb_count = *(int*)(mw + MESHWORLD_MB_COUNT);
    if (logf) fprintf(logf, "  GRID: mb_count=%d (at mw+0x%02X)\n", mb_count, MESHWORLD_MB_COUNT);
    if (mb_count < 1 || mb_count > 10000) {
        if (logf) fprintf(logf, "  GRID: mb_count out of range, aborting\n");
        return;
    }

    if (IsBadReadPtr((void*)(mw + MESHWORLD_MB_DATA), 4)) {
        if (logf) fprintf(logf, "  GRID: IsBadReadPtr at mw+0x%02X (MB_DATA)\n", MESHWORLD_MB_DATA);
        return;
    }
    DWORD* mb_array = *(DWORD**)(mw + MESHWORLD_MB_DATA);
    if (logf) fprintf(logf, "  GRID: mb_array=0x%08X\n", (DWORD)mb_array);
    if (!mb_array || IsBadReadPtr(mb_array, mb_count * 4)) {
        if (logf) fprintf(logf, "  GRID: mb_array invalid\n");
        return;
    }

    /* Get EntityTransform array */
    if (IsBadReadPtr((void*)(mw + MESHWORLD_RENDERCTX_PTR), 4)) {
        if (logf) fprintf(logf, "  GRID: IsBadReadPtr at mw+0x%02X (RENDERCTX_PTR)\n", MESHWORLD_RENDERCTX_PTR);
        return;
    }
    EntityTransform* transforms = *(EntityTransform**)(mw + MESHWORLD_RENDERCTX_PTR);
    if (logf) fprintf(logf, "  GRID: transforms=0x%08X\n", (DWORD)transforms);
    if (!transforms) {
        if (logf) fprintf(logf, "  GRID: transforms NULL\n");
        return;
    }

    int i;
    for (i = 0; i < mb_count && g_grid_mesh_count < MAX_GRID_MESHES; i++) {
        DWORD mb = mb_array[i];
        if (!mb || mb < 0x10000) {
            if (logf) fprintf(logf, "  GRID: mb[%d] skipped (ptr=0x%08X)\n", i, mb);
            continue;
        }
        if (IsBadReadPtr((void*)mb, 0x900)) {
            if (logf) fprintf(logf, "  GRID: mb[%d] skipped (bad read at 0x%08X)\n", i, mb);
            continue;
        }

        /* Read name */
        if (IsBadReadPtr((void*)(mb + MESHBUFFER_NAME), 4)) {
            if (logf) fprintf(logf, "  GRID: mb[%d] skipped (bad name ptr)\n", i);
            continue;
        }
        char* name = *(char**)(mb + MESHBUFFER_NAME);
        if (!name || IsBadReadPtr(name, 4)) {
            if (logf) fprintf(logf, "  GRID: mb[%d] skipped (name invalid, ptr=0x%08X)\n", i, (DWORD)name);
            continue;
        }

        if (logf) fprintf(logf, "  GRID: mb[%d] name='%s'\n", i, name);

        int grid_num = parse_grid_flag(name);
        if (grid_num == 0) continue;

        /* Get EntityTransform for this MeshBuffer */
        if (IsBadReadPtr((void*)(mb + MESHBUFFER_CTX_INDEX), 4)) continue;
        DWORD ctx_idx = *(DWORD*)(mb + MESHBUFFER_CTX_INDEX);
        if (ctx_idx > 10000) continue;

        EntityTransform* t = &transforms[ctx_idx];
        if (IsBadReadPtr(t, sizeof(EntityTransform))) continue;

        /* Store in grid list */
        g_grid_meshes[g_grid_mesh_count].transform = t;
        g_grid_meshes[g_grid_mesh_count].grid_num = grid_num;
        g_grid_mesh_count++;

        if (grid_num > g_grid_max) g_grid_max = grid_num;

        if (logf) fprintf(logf, "  GRID: *** FOUND mesh[%d] name='%s' grid=%d pos=(%.1f,%.1f,%.1f) posScale=%.2f\n",
                i, name, grid_num, t->posX, t->posY, t->posZ, t->posScale);
    }

    if (logf) {
        fprintf(logf, "  GRID: Scanned %d MeshBuffers, found %d grid meshes, max grid = %d, speed = %.1f ticks\n",
                mb_count, g_grid_mesh_count, g_grid_max, g_grid_speed);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * GRID: Apply visibility — show meshes matching current counter, hide others
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

/* ═══════════════════════════════════════════════════════════════════════════
 * GRID: Advance counter
 * ═══════════════════════════════════════════════════════════════════════════ */

static void grid_advance(void) {
    g_grid_tick_counter++;
    if ((float)g_grid_tick_counter >= g_grid_speed) {
        g_grid_tick_counter = 0;
        g_grid_counter++;
        if (g_grid_counter > g_grid_max || g_grid_max == 0) {
            g_grid_counter = 1;
        }
        grid_apply_visibility();
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * LEGACY CE ENTITY SYSTEM (from v4)
 * ═══════════════════════════════════════════════════════════════════════════ */

static int is_custom_entity(const char* name) {
    return (strstr(name, "CE") != NULL);
}

static int load_behavior_dll(CustomEntity* ent) {
    char dll_path[MAX_PATH];
    snprintf(dll_path, MAX_PATH, "%s\\%s.dll", g_entities_dir, ent->entity_name);

    ent->behavior_dll = LoadLibraryA(dll_path);
    if (!ent->behavior_dll) {
        char lower_name[256];
        int i;
        for (i = 0; ent->entity_name[i] && i < 255; i++)
            lower_name[i] = (char)tolower((unsigned char)ent->entity_name[i]);
        lower_name[i] = '\0';
        snprintf(dll_path, MAX_PATH, "%s\\%s.dll", g_entities_dir, lower_name);
        ent->behavior_dll = LoadLibraryA(dll_path);
    }

    if (!ent->behavior_dll) return 0;

    ent->init_fn = (Behavior_Init_t)GetProcAddress(ent->behavior_dll, "Behavior_Init");
    ent->update_fn = (Behavior_Update_t)GetProcAddress(ent->behavior_dll, "Behavior_Update");
    ent->shutdown_fn = (Behavior_Shutdown_t)GetProcAddress(ent->behavior_dll, "Behavior_Shutdown");

    if (!ent->update_fn) {
        FreeLibrary(ent->behavior_dll);
        ent->behavior_dll = NULL;
        return 0;
    }
    return 1;
}

static void shutdown_all_entities(void) {
    int i;
    for (i = 0; i < g_entity_count; i++) {
        CustomEntity* ent = &g_entities[i];
        if (ent->initialized && ent->shutdown_fn) {
            ent->shutdown_fn();
        }
        if (ent->behavior_dll) {
            FreeLibrary(ent->behavior_dll);
            ent->behavior_dll = NULL;
        }
        ent->initialized = 0;
        ent->transform = NULL;
        ent->init_fn = NULL;
        ent->update_fn = NULL;
        ent->shutdown_fn = NULL;
    }
    g_entity_count = 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * CE Entity: Scan S1 ref points
 * ═══════════════════════════════════════════════════════════════════════════ */

static void scan_s1_ref_points(DWORD board, FILE* logf) {
    if (!board) return;

    if (IsBadReadPtr((void*)(board + BOARD_LEVEL), 4)) return;
    DWORD level = *(DWORD*)(board + BOARD_LEVEL);
    if (!level || level < 0x10000) return;

    if (IsBadReadPtr((void*)(level + LEVEL_SCENEOBJECT), 4)) return;
    DWORD sceneobj = *(DWORD*)(level + LEVEL_SCENEOBJECT);
    if (!sceneobj || sceneobj < 0x10000) return;

    /* S1 list is an embedded AthenaList at sceneobj+0x894.
     * AthenaList+0x04 = count, AthenaList+0x40C = data ptr.
     * So count is at sceneobj+0x898, data at sceneobj+0xCA0. */
    DWORD s1_list = sceneobj + SCENEOBJ_S1_LIST;
    if (s1_list < 0x10000) return;

    if (IsBadReadPtr((void*)(s1_list + 0x04), 4)) return;
    int s1_count = *(int*)(s1_list + 0x04);
    if (s1_count < 1 || s1_count > 10000) return;

    if (IsBadReadPtr((void*)(s1_list + 0x40C), 4)) return;
    DWORD* s1_array = *(DWORD**)(s1_list + 0x40C);
    if (!s1_array || IsBadReadPtr(s1_array, s1_count * 4)) return;

    if (logf) fprintf(logf, "  S1 ref point scan: %d entries\n", s1_count);

    int i;
    for (i = 0; i < s1_count && g_entity_count < MAX_CUSTOM_ENTITIES; i++) {
        DWORD entry = s1_array[i];
        if (!entry || entry < 0x10000) continue;
        if (IsBadReadPtr((void*)entry, S1ENTRY_SIZE)) continue;

        if (IsBadReadPtr((void*)(entry + S1ENTRY_NAME), 4)) continue;
        char* name = *(char**)(entry + S1ENTRY_NAME);
        if (!name || IsBadReadPtr(name, 3)) continue;

        if (!is_custom_entity(name)) continue;

        int k;
        for (k = 0; k < g_entity_count; k++) {
            if (_stricmp(g_entities[k].entity_name, name) == 0)
                break;
        }
        if (k < g_entity_count) continue;

        float posX = *(float*)(entry + S1ENTRY_POS_X);
        float posY = *(float*)(entry + S1ENTRY_POS_Y);
        float posZ = *(float*)(entry + S1ENTRY_POS_Z);
        float rotX = *(float*)(entry + S1ENTRY_ROT_X);
        float rotY = *(float*)(entry + S1ENTRY_ROT_Y);
        float rotZ = *(float*)(entry + S1ENTRY_ROT_Z);

        if (logf) fprintf(logf, "  S1[%d]: name='%s' pos=(%.1f,%.1f,%.1f) rot=(%.3f,%.3f,%.3f)\n",
                i, name, posX, posY, posZ, rotX, rotY, rotZ);

        /* Find the game's EntityTransform by matching position */
        EntityTransform* game_transform = NULL;

        DWORD scene = get_scene(board);
        if (scene) {
            DWORD mw = get_meshworld(scene);
            if (mw) {
                if (!IsBadReadPtr((void*)(mw + MESHWORLD_MB_COUNT), 4)) {
                    int mb_count = *(int*)(mw + MESHWORLD_MB_COUNT);
                    if (mb_count >= 1 && mb_count <= 10000) {
                        if (!IsBadReadPtr((void*)(mw + MESHWORLD_MB_DATA), 4)) {
                            DWORD* mb_array = *(DWORD**)(mw + MESHWORLD_MB_DATA);
                            if (mb_array && !IsBadReadPtr(mb_array, mb_count * 4)) {
                                if (!IsBadReadPtr((void*)(mw + MESHWORLD_RENDERCTX_PTR), 4)) {
                                    EntityTransform* transforms = *(EntityTransform**)(mw + MESHWORLD_RENDERCTX_PTR);
                                    if (transforms) {
                                        int mb_i;
                                        for (mb_i = 0; mb_i < mb_count; mb_i++) {
                                            DWORD mb = mb_array[mb_i];
                                            if (!mb || mb < 0x10000) continue;
                                            if (IsBadReadPtr((void*)mb, 0x900)) continue;
                                            if (IsBadReadPtr((void*)(mb + MESHBUFFER_CTX_INDEX), 4)) continue;
                                            DWORD ctx_idx = *(DWORD*)(mb + MESHBUFFER_CTX_INDEX);
                                            if (ctx_idx > 10000) continue;
                                            EntityTransform* t = &transforms[ctx_idx];
                                            if (IsBadReadPtr(t, sizeof(EntityTransform))) continue;

                                            float dx = t->posX - posX;
                                            float dy = t->posY - posY;
                                            float dz = t->posZ - posZ;
                                            if (dx < 0) dx = -dx;
                                            if (dy < 0) dy = -dy;
                                            if (dz < 0) dz = -dz;
                                            if (dx < 50.0f && dy < 50.0f && dz < 50.0f) {
                                                game_transform = t;
                                                if (logf) fprintf(logf, "    -> Found EntityTransform by pos match: mb[%d] ctx=%d\n",
                                                        mb_i, ctx_idx);
                                                break;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        CustomEntity* ent = &g_entities[g_entity_count];
        memset(ent, 0, sizeof(CustomEntity));
        strncpy(ent->entity_name, name, 255);
        ent->posX = posX;
        ent->posY = posY;
        ent->posZ = posZ;
        ent->rotX = rotX;
        ent->rotY = rotY;
        ent->rotZ = rotZ;

        if (game_transform) {
            ent->transform = game_transform;
            if (logf) fprintf(logf, "    -> Using GAME EntityTransform\n");
        } else {
            ent->transform = (EntityTransform*)LocalAlloc(LPTR, sizeof(EntityTransform));
            if (!ent->transform) continue;
            if (logf) fprintf(logf, "    -> WARNING: No matching EntityTransform found (disconnected)\n");
        }

        ent->transform->posX = posX;
        ent->transform->posY = posY;
        ent->transform->posZ = posZ;
        ent->transform->rotX = rotX;
        ent->transform->rotY = rotY;
        ent->transform->rotZ = rotZ;
        ent->transform->rotScale = 1.0f;
        ent->transform->posScale = 1.0f;

        if (!load_behavior_dll(ent)) {
            if (logf) fprintf(logf, "    -> Could not load %s.dll, skipping\n", name);
            if ((DWORD)ent->transform > 0x10000000)
                LocalFree(ent->transform);
            ent->transform = NULL;
            continue;
        }

        if (logf) fprintf(logf, "    -> Loaded %s.dll successfully!\n", name);

        if (ent->init_fn) {
            ent->init_fn(ent->transform, (void*)board);
        }
        ent->initialized = 1;
        g_entity_count++;
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * CE Entity: Hide original entity mesh
 * ═══════════════════════════════════════════════════════════════════════════ */

static void hide_original_entity_mesh(DWORD board, FILE* logf) {
    DWORD scene = get_scene(board);
    if (!scene) return;
    DWORD mw = get_meshworld(scene);
    if (!mw) return;

    if (IsBadReadPtr((void*)(mw + MESHWORLD_MB_COUNT), 4)) return;
    int mb_count = *(int*)(mw + MESHWORLD_MB_COUNT);
    if (mb_count < 1 || mb_count > 10000) return;

    if (IsBadReadPtr((void*)(mw + MESHWORLD_MB_DATA), 4)) return;
    DWORD* mb_array = *(DWORD**)(mw + MESHWORLD_MB_DATA);
    if (!mb_array || IsBadReadPtr(mb_array, mb_count * 4)) return;

    if (IsBadReadPtr((void*)(mw + MESHWORLD_RENDERCTX_PTR), 4)) return;
    EntityTransform* transforms = *(EntityTransform**)(mw + MESHWORLD_RENDERCTX_PTR);
    if (!transforms) return;

    int i;
    for (i = 0; i < mb_count; i++) {
        DWORD mb = mb_array[i];
        if (!mb || mb < 0x10000) continue;
        if (IsBadReadPtr((void*)mb, 0x900)) continue;
        if (IsBadReadPtr((void*)(mb + MESHBUFFER_NAME), 4)) continue;
        char* name = *(char**)(mb + MESHBUFFER_NAME);
        if (!name || IsBadReadPtr(name, 3)) continue;

        int j;
        for (j = 0; j < g_entity_count; j++) {
            if (!g_entities[j].initialized) continue;
            const char* ent_name = g_entities[j].entity_name;

            if (_stricmp(name, ent_name) == 0) {
                DWORD ctx_idx = *(DWORD*)(mb + MESHBUFFER_CTX_INDEX);
                if (ctx_idx <= 10000) {
                    EntityTransform* t = &transforms[ctx_idx];
                    if (!IsBadReadPtr(t, sizeof(EntityTransform))) {
                        t->posScale = 0.0f;
                        if (logf) fprintf(logf, "  Hidden original mesh '%s' (mb[%d] ctx=%d) by setting posScale=0\n",
                                name, i, ctx_idx);
                    }
                }
                break;
            }
        }
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
            fprintf(f, "=== Custom Entities Mod v5 Started ===\n");
            fprintf(f, "Game dir: %s\n", g_game_dir);
            fprintf(f, "Grid speed: %.1f ticks\n", g_grid_speed);
            fclose(f);
        }
    }

    while (g_running) {
        DWORD board = get_board();

        if (board != g_last_board) {
            if (g_last_board != 0) {
                shutdown_all_entities();
                g_grid_mesh_count = 0;
                g_grid_counter = 1;
                g_grid_max = 0;
                g_grid_tick_counter = 0;
            }
            g_last_board = board;

            if (board) {
                Sleep(500);
                board = get_board();
                if (board) {
                    char log_path[MAX_PATH];
                    snprintf(log_path, MAX_PATH, "%s\\custom_entities.log", g_game_dir);
                    FILE* logf = NULL;
                    fopen_s(&logf, log_path, "a");

                    if (logf) fprintf(logf, "\n--- Level loaded, scanning... ---\n");

                    /* GRID scan */
                    scan_grid_meshes(board, logf);

                    /* Apply initial grid visibility */
                    if (g_grid_mesh_count > 0) {
                        grid_apply_visibility();
                    }

                    /* CE entity scan */
                    scan_s1_ref_points(board, logf);
                    hide_original_entity_mesh(board, logf);

                    if (logf) {
                        fprintf(logf, "Scan complete: %d grid meshes, %d custom entities\n\n",
                                g_grid_mesh_count, g_entity_count);
                        fclose(logf);
                    }
                }
            }
        }

        /* GRID: advance counter */
        if (g_grid_mesh_count > 0 && board) {
            grid_advance();
        }

        /* CE: Call Behavior_Update for all active entities */
        if (g_entity_count > 0 && board) {
            int i;
            for (i = 0; i < g_entity_count; i++) {
                CustomEntity* ent = &g_entities[i];
                if (ent->initialized && ent->update_fn && ent->transform) {
                    if (!IsBadReadPtr(ent->transform, sizeof(EntityTransform))) {
                        ent->update_fn(ent->transform, (void*)board);
                    }
                }
            }
        }

        Sleep(16);
    }

    shutdown_all_entities();
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
