/*
 * custom_entities.c — Hamsterball Custom Entities Mod v15
 *
 * bass.dll proxy mod. Spawns testcube meshes at S1 GRID reference points.
 *
 * v13 REWRITE: Uses the proven CEA spawning pattern instead of direct
 * MeshBuffer injection. Loads testcube.MESHWORLD via the game's own
 * MeshWorld_ctor, then creates PopCylinder objects that reference it.
 * This is the same pattern used by XRow's "Press S to spawn red bridge"
 * CEA script — load mesh + create object + register in board lists.
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
 * Game function pointers
 * ═══════════════════════════════════════════════════════════════════════════ */

/* operator_new — game's C++ allocator (malloc wrapper) */
typedef void* (__cdecl *operator_new_t)(SIZE_T);
static operator_new_t pfn_operator_new = (operator_new_t)0x004BA57B;

/* MeshWorld_ctor — loads a .MESHWORLD file into a mesh object
 * __thiscall(this, gfx_device, mesh_path_string)
 * mesh_path_string = e.g. "levels\\testcube" (without .MESHWORLD extension)
 * Returns the mesh object pointer (same as this) */
typedef void* (__thiscall *MeshWorld_ctor_t)(void* this_, void* gfx_device, const char* mesh_path);
static MeshWorld_ctor_t pfn_MeshWorld_ctor = (MeshWorld_ctor_t)0x00461510;

/* PopCylinder_ctor — creates a PopCylinder (bumper) object
 * __thiscall(this, board, posX, posY, posZ, mesh)
 * Stationary object, no path/spline needed */
typedef void* (__thiscall *PopCylinder_ctor_t)(void* this_, void* board, float posX, float posY, float posZ, void* mesh);
static PopCylinder_ctor_t pfn_PopCylinder_ctor = (PopCylinder_ctor_t)0x00436EE0;

/* AthenaList_Append — adds item to an AthenaList
 * __thiscall(list, item) */
/* AthenaList_Append — declared in bass_proxy.h, reusing that typedef */
static AthenaList_Append_t pfn_AthenaList_Append = (AthenaList_Append_t)0x00453810;

/* AthenaList_RemoveByValue — removes item from list by pointer value
 * __thiscall(list, item_value) — at 0x004534D0 */
typedef void (__thiscall *AthenaList_Remove_t)(DWORD* list, int item);
static AthenaList_Remove_t pfn_AthenaList_Remove = (AthenaList_Remove_t)0x004534D0;

/* SpatialTree_CloneToLevel / SpatialTree_Cleanup */
typedef void (__thiscall *SpatialTree_CloneToLevel_t)(void* this_);
static SpatialTree_CloneToLevel_t pfn_SpatialTree_CloneToLevel = (SpatialTree_CloneToLevel_t)0x00457AD0;
typedef void (__thiscall *SpatialTree_Cleanup_t)(void* this_);
static SpatialTree_Cleanup_t pfn_SpatialTree_Cleanup = (SpatialTree_Cleanup_t)0x00457A40;

/* Board layout */
#define BOARD_APP               0x878
#define BOARD_LEVEL             0x8AC
#define BOARD_UPDATE_LIST       0x2578
#define BOARD_RENDER_LIST       0xCD4
#define BOARD_COLLISION_LIST   0x10EC
#define BOARD_SCENE_OBJ         0x8B0

/* App layout */
#define APP_GFX_DEVICE          0x174

/* Level/SceneObject layout */
#define LEVEL_SCENEOBJECT       0x480

/* PopCylinder layout */
#define PC_COLLISION_OBJ        0x10E0

/* S1 entry layout */
#define S1ENTRY_NAME             0x00
#define S1ENTRY_POS_X            0x04
#define S1ENTRY_POS_Y            0x08
#define S1ENTRY_POS_Z            0x0C

/* Object size constants */
#define MESHWORLD_SIZE          0x10D0
#define POPCYLINDER_SIZE        0x10D0
#define SPATIALTREE_SIZE        68

/* ═══════════════════════════════════════════════════════════════════════════
 * State
 * ═══════════════════════════════════════════════════════════════════════════ */

static HANDLE g_thread = NULL;
static volatile int g_running = 1;
static char g_game_dir[MAX_PATH] = {0};

/* Track spawned objects so we can despawn them individually */
#define MAX_SPAWNED 16
static DWORD g_spawned_objs[MAX_SPAWNED];
static char  g_spawned_names[MAX_SPAWNED][32];
static int   g_spawned_count = 0;
static DWORD g_spawned_board = 0;

/* Mesh path string — we copy testcube.MESHWORLD to levels\ at startup */
static char g_mesh_path[] = "levels\\testcube";

/* ═══════════════════════════════════════════════════════════════════════════
 * Helpers
 * ═══════════════════════════════════════════════════════════════════════════ */

/* get_board is declared in bass_proxy.h */

static DWORD get_level(DWORD board) {
    if (!board) return 0;
    if (IsBadReadPtr((void*)(board + BOARD_LEVEL), 4)) return 0;
    return *(DWORD*)(board + BOARD_LEVEL);
}

/* Get the SceneObject from the level */
static DWORD get_sceneobj(DWORD board) {
    DWORD level = get_level(board);
    if (!level) return 0;
    if (IsBadReadPtr((void*)(level + LEVEL_SCENEOBJECT), 4)) return 0;
    return *(DWORD*)(level + LEVEL_SCENEOBJECT);
}

/* Find S1 reference points by scanning the sceneobj's S1 list */
static int find_grid_points(DWORD board, float* out_x, float* out_y, float* out_z, int max_points, FILE* logf) {
    DWORD sceneobj = get_sceneobj(board);
    if (!sceneobj) {
        if (logf) fprintf(logf, "  GRID: sceneobj=NULL\n");
        return 0;
    }

    /* S1 AthenaList is at sceneobj+0x894 */
    DWORD s1_list = sceneobj + 0x894;
    if (IsBadReadPtr((void*)(s1_list + 0x04), 4)) {
        if (logf) fprintf(logf, "  GRID: can't read S1 list count (sceneobj=0x%08X)\n", sceneobj);
        return 0;
    }
    int s1_count = *(int*)(s1_list + 0x04);
    if (logf) fprintf(logf, "  GRID: sceneobj=0x%08X, S1 count=%d\n", sceneobj, s1_count);
    if (s1_count <= 0 || s1_count > 1000) {
        return 0;
    }

    /* S1 data pointer is at s1_list+0x40C (AthenaList data) */
    if (IsBadReadPtr((void*)(s1_list + 0x40C), 4)) {
        if (logf) fprintf(logf, "  GRID: can't read S1 data ptr\n");
        return 0;
    }
    DWORD* s1_data = *(DWORD**)(s1_list + 0x40C);
    if (!s1_data || IsBadReadPtr(s1_data, s1_count * 4)) {
        if (logf) fprintf(logf, "  GRID: S1 data ptr invalid (0x%08X)\n", (DWORD)s1_data);
        return 0;
    }

    int found = 0;
    int i;
    for (i = 0; i < s1_count && found < max_points; i++) {
        DWORD entry = s1_data[i];
        if (!entry || entry < 0x10000) continue;
        if (IsBadReadPtr((void*)entry, 16)) continue;

        /* Try reading name as char* pointer at offset 0x00 */
        char* name = *(char**)(entry + S1ENTRY_NAME);
        if (name && !IsBadReadPtr(name, 5)) {
            if (logf && i < 10) fprintf(logf, "  GRID: S1[%d] name='%s'\n", i, name);
            /* Use strstr — name may be "testcube(GRID01)" not starting with GRID */
            if (strstr(name, "GRID") != NULL) {
                float x = *(float*)(entry + S1ENTRY_POS_X);
                float y = *(float*)(entry + S1ENTRY_POS_Y);
                float z = *(float*)(entry + S1ENTRY_POS_Z);
                out_x[found] = x;
                out_y[found] = y;
                out_z[found] = z;
                found++;
                if (logf) fprintf(logf, "  GRID: found %s at (%.1f, %.1f, %.1f)\n", name, x, y, z);
            }
        } else {
            /* Name might be inline char array, not a pointer */
            if (logf && i < 10) {
                fprintf(logf, "  GRID: S1[%d] entry=0x%08X (bad nameptr=0x%08X) raw: %02X%02X%02X%02X\n",
                    i, entry, (DWORD)name,
                    *(BYTE*)entry, *((BYTE*)entry+1), *((BYTE*)entry+2), *((BYTE*)entry+3));
            }
            char* inline_name = (char*)entry;
            if (strstr(inline_name, "GRID") != NULL) {
                float x = *(float*)(entry + S1ENTRY_POS_X);
                float y = *(float*)(entry + S1ENTRY_POS_Y);
                float z = *(float*)(entry + S1ENTRY_POS_Z);
                out_x[found] = x;
                out_y[found] = y;
                out_z[found] = z;
                found++;
                if (logf) fprintf(logf, "  GRID: found (inline) %s at (%.1f, %.1f, %.1f)\n", inline_name, x, y, z);
            }
        }
    }

    return found;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * CEA spawning pattern — load mesh + create object + register
 * Based on XRow's "Press S to spawn red bridge GLOBALLY" CEA script
 * ═══════════════════════════════════════════════════════════════════════════ */

static void spawn_testcube_at(DWORD board, float px, float py, float pz, int grid_num, FILE* logf) {
    if (!board) return;

    /* 1. Get gfx_device from App */
    DWORD app = *(DWORD*)(board + BOARD_APP);
    if (!app || IsBadReadPtr((void*)app, 4)) {
        if (logf) fprintf(logf, "  GRID: app=NULL\n");
        return;
    }
    DWORD gfx_device = *(DWORD*)(app + APP_GFX_DEVICE);
    if (!gfx_device || IsBadReadPtr((void*)gfx_device, 4)) {
        if (logf) fprintf(logf, "  GRID: gfx_device=NULL\n");
        return;
    }

    /* 2. Load mesh via MeshWorld_ctor */
    void* mesh = pfn_operator_new(MESHWORLD_SIZE);
    if (!mesh) {
        if (logf) fprintf(logf, "  GRID: failed to allocate mesh\n");
        return;
    }
    memset(mesh, 0, MESHWORLD_SIZE);

    void* loaded_mesh = pfn_MeshWorld_ctor(mesh, (void*)gfx_device, g_mesh_path);
    if (!loaded_mesh) {
        if (logf) fprintf(logf, "  GRID: MeshWorld_ctor failed for '%s'\n", g_mesh_path);
        return;
    }

    /* 3. Allocate PopCylinder object */
    void* obj = pfn_operator_new(POPCYLINDER_SIZE);
    if (!obj) {
        if (logf) fprintf(logf, "  GRID: failed to allocate PopCylinder\n");
        return;
    }
    memset(obj, 0, POPCYLINDER_SIZE);

    /* 4. Call PopCylinder_ctor(this, board, X, Y, Z, mesh) */
    void* result = pfn_PopCylinder_ctor(obj, (void*)board, px, py, pz, mesh);
    if (!result) {
        if (logf) fprintf(logf, "  GRID: PopCylinder_ctor failed\n");
        return;
    }

    /* 5. Add to board+0x2578 (update list) */
    pfn_AthenaList_Append((DWORD*)(board + BOARD_UPDATE_LIST), obj);

    /* 6. Add to board+0xCD4 (render list) */
    pfn_AthenaList_Append((DWORD*)(board + BOARD_RENDER_LIST), obj);

    /* 7. Add collision object to board+0x10EC */
    DWORD col_obj = *(DWORD*)((char*)obj + PC_COLLISION_OBJ);
    if (col_obj) {
        pfn_AthenaList_Append((DWORD*)(board + BOARD_COLLISION_LIST), (void*)col_obj);

        /* Also add to board+0x8B0+0x18 (scene collision) */
        DWORD scene_col = *(DWORD*)(board + BOARD_SCENE_OBJ);
        if (scene_col) {
            pfn_AthenaList_Append((DWORD*)(scene_col + 0x18), (void*)col_obj);
        }
    }

    /* 8. Add to scene spatial tree (board+0x8AC+0x480+0x1C) */
    DWORD level = get_level(board);
    if (level) {
        DWORD sceneobj = *(DWORD*)(level + LEVEL_SCENEOBJECT);
        if (sceneobj) {
            pfn_AthenaList_Append((DWORD*)(sceneobj + 0x1C), obj);
        }
    }

    if (logf) {
        fprintf(logf, "  GRID: spawned testcube(GRID%02d) at (%.1f,%.1f,%.1f) obj=0x%08X\n",
                grid_num, px, py, pz, (DWORD)obj);
        fflush(logf);
    }

    /* Track spawned object for later despawn */
    if (g_spawned_count < MAX_SPAWNED) {
        g_spawned_objs[g_spawned_count] = (DWORD)obj;
        snprintf(g_spawned_names[g_spawned_count], 32, "testcube(GRID%02d)", grid_num);
        g_spawned_count++;
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Despawn a specific testcube by GRID name
 * Based on Rotator_RemoveAndFree (0x436FC0) pattern:
 *   1. Remove collision obj from board+0x8B0+0x18 and board+0x10EC
 *   2. Remove obj from board+0x2578 (update) and board+0xCD4 (render)
 *   3. Remove obj from sceneobj+0x1C (scene tree)
 *   4. Call collision obj destructor
 *   5. Set flags to prevent double-free
 * ═══════════════════════════════════════════════════════════════════════════ */

static void despawn_object(DWORD board, DWORD obj, FILE* logf) {
    if (!board || !obj) return;
    if (IsBadReadPtr((void*)obj, 0x10D0)) return;

    char* name = "(unknown)";
    if (logf) fprintf(logf, "  DESPAWN: removing obj=0x%08X\n", obj);

    /* 1. Mark as removed (prevents update/render from touching it) */
    *(BYTE*)((char*)obj + 0x10E5) = 1;  /* removed flag */
    *(BYTE*)((char*)obj + 0x10E4) = 1;  /* inactive flag */

    /* 2. Remove collision object from board+0x10EC */
    DWORD col_obj = *(DWORD*)((char*)obj + PC_COLLISION_OBJ);
    if (col_obj) {
        /* Remove from board+0x10EC (collision list) */
        pfn_AthenaList_Remove((DWORD*)(board + BOARD_COLLISION_LIST), (int)col_obj);

        /* Remove from board+0x8B0+0x18 (scene collision) */
        DWORD scene_col = *(DWORD*)(board + BOARD_SCENE_OBJ);
        if (scene_col) {
            pfn_AthenaList_Remove((DWORD*)(scene_col + 0x18), (int)col_obj);
        }

        /* Call collision object's destructor (vtable[0] with arg=1 to free) */
        if (!IsBadReadPtr((void*)col_obj, 4)) {
            DWORD col_vtable = *(DWORD*)col_obj;
            if (col_vtable && !IsBadReadPtr((void*)col_vtable, 4)) {
                DWORD dtor = *(DWORD*)col_vtable;
                if (dtor && dtor > 0x400000) {
                    typedef void (__thiscall *dtor_t)(void* this_, int free_mem);
                    ((dtor_t)dtor)((void*)col_obj, 1);
                }
            }
        }
        *(DWORD*)((char*)obj + PC_COLLISION_OBJ) = 0;
    }

    /* 3. Remove obj from board+0x2578 (update list) */
    pfn_AthenaList_Remove((DWORD*)(board + BOARD_UPDATE_LIST), (int)obj);

    /* 4. Remove obj from board+0xCD4 (render list) */
    pfn_AthenaList_Remove((DWORD*)(board + BOARD_RENDER_LIST), (int)obj);

    /* 5. Remove obj from sceneobj+0x1C (scene tree) */
    DWORD level = get_level(board);
    if (level) {
        DWORD sceneobj = *(DWORD*)(level + LEVEL_SCENEOBJECT);
        if (sceneobj) {
            pfn_AthenaList_Remove((DWORD*)(sceneobj + 0x1C), (int)obj);
        }
    }

    if (logf) {
        fprintf(logf, "  DESPAWN: obj=0x%08X removed from all lists\n", obj);
        fflush(logf);
    }
}

static void despawn_by_name(const char* target_name, DWORD board, FILE* logf) {
    int i;
    for (i = 0; i < g_spawned_count; i++) {
        if (strstr(g_spawned_names[i], target_name) != NULL) {
            if (logf) fprintf(logf, "  DESPAWN: found '%s' at index %d, obj=0x%08X\n",
                    g_spawned_names[i], i, g_spawned_objs[i]);
            despawn_object(board, g_spawned_objs[i], logf);
            /* Shift array down */
            int j;
            for (j = i; j < g_spawned_count - 1; j++) {
                g_spawned_objs[j] = g_spawned_objs[j + 1];
                strcpy(g_spawned_names[j], g_spawned_names[j + 1]);
            }
            g_spawned_count--;
            return;
        }
    }
    if (logf) fprintf(logf, "  DESPAWN: '%s' not found in spawned objects\n", target_name);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Init: copy testcube.MESHWORLD to levels\ directory
 * ═══════════════════════════════════════════════════════════════════════════ */

static void init_game_dir(void) {
    char dll_path[MAX_PATH] = {0};
    HMODULE hMod = NULL;

    /* Find our own module handle via VirtualQuery */
    VirtualQuery((void*)&init_game_dir, (PMEMORY_BASIC_INFORMATION)&hMod, sizeof(hMod));
    GetModuleFileNameA(hMod, dll_path, MAX_PATH);

    /* Extract directory from DLL path */
    char* p = strrchr(dll_path, '\\');
    if (p) *p = 0;
    strncpy(g_game_dir, dll_path, MAX_PATH - 1);

    /* Copy testcube.MESHWORLD to levels\ directory */
    char src[MAX_PATH], dst[MAX_PATH];
    snprintf(src, MAX_PATH, "%s\\testcube.MESHWORLD", g_game_dir);
    snprintf(dst, MAX_PATH, "%s\\levels\\testcube.MESHWORLD", g_game_dir);

    /* Create levels\ dir if it doesn't exist */
    char levels_dir[MAX_PATH];
    snprintf(levels_dir, MAX_PATH, "%s\\levels", g_game_dir);
    CreateDirectoryA(levels_dir, NULL);

    /* Only copy if destination doesn't exist or source is newer */
    if (!PathFileExistsA(dst) || GetFileAttributesA(src) != INVALID_FILE_ATTRIBUTES) {
        CopyFileA(src, dst, FALSE);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Main thread — hooks Ball_Update, spawns at GRID positions
 * ═══════════════════════════════════════════════════════════════════════════ */

static DWORD WINAPI entity_thread(LPVOID param) {
    init_game_dir();

    /* Open log file */
    char log_path[MAX_PATH];
    snprintf(log_path, MAX_PATH, "%s\\custom_entities.log", g_game_dir);
    FILE* logf = NULL;
    fopen_s(&logf, log_path, "a");
    if (logf) {
        fprintf(logf, "=== Custom Entities Mod v15 Started ===\n");
        fprintf(logf, "Game dir: %s\n", g_game_dir);
        fprintf(logf, "Mesh path: %s\n", g_mesh_path);
        fclose(logf);
    }

    /* Wait for game to fully load */
    Sleep(3000);

    while (g_running) {
        Sleep(100);

        DWORD board = get_board();
        if (!board) continue;

        /* Check if board changed (new level loaded) */
        if (board == g_spawned_board) continue;

        /* New board — wait for level to finish loading */
        Sleep(500);

        /* Re-read board in case it changed during sleep */
        board = get_board();
        if (!board) continue;

        /* Verify level is loaded */
        DWORD level = get_level(board);
        if (!level) continue;

        logf = NULL;
        fopen_s(&logf, log_path, "a");
        if (logf) {
            fprintf(logf, "\n--- Level loaded (board=0x%08X, level=0x%08X) ---\n", board, level);
        }

        /* Find GRID reference points */
        float grid_x[32], grid_y[32], grid_z[32];
        int grid_count = find_grid_points(board, grid_x, grid_y, grid_z, 32, logf);

        if (grid_count > 0) {
            int i;
            /* Find GRID02's position */
            int grid02_idx = -1;
            for (i = 0; i < grid_count; i++) {
                if (i + 1 == 2) {  /* GRID02 = grid_num 2 */
                    grid02_idx = i;
                    break;
                }
            }

            if (grid02_idx >= 0) {
                /* Spawn only GRID02 */
                spawn_testcube_at(board, grid_x[grid02_idx], grid_y[grid02_idx], grid_z[grid02_idx], 2, logf);
                g_spawned_board = board;
                if (logf) fprintf(logf, "  Spawned GRID02, starting respawn loop\n");

                /* Respawn loop: despawn → wait → respawn → wait → repeat */
                while (g_running && board == get_board()) {
                    Sleep(3000);
                    if (!g_running || board != get_board()) break;

                    logf = NULL;
                    fopen_s(&logf, log_path, "a");

                    /* Despawn */
                    despawn_by_name("GRID02", board, logf);

                    Sleep(3000);
                    if (!g_running || board != get_board()) {
                        if (logf) { fflush(logf); fclose(logf); }
                        break;
                    }

                    /* Respawn */
                    spawn_testcube_at(board, grid_x[grid02_idx], grid_y[grid02_idx], grid_z[grid02_idx], 2, logf);
                    if (logf) {
                        fprintf(logf, "  GRID02 respawned\n");
                        fflush(logf);
                        fclose(logf);
                    }
                }
            } else {
                /* No GRID02 — spawn all and mark done */
                for (i = 0; i < grid_count; i++) {
                    spawn_testcube_at(board, grid_x[i], grid_y[i], grid_z[i], i + 1, logf);
                }
                g_spawned_board = board;
                if (logf) fprintf(logf, "  Spawned %d testcubes (no GRID02 found)\n", grid_count);
            }
        } else {
            /* No GRID points — still mark board as processed */
            g_spawned_board = board;
            if (logf) fprintf(logf, "  No GRID points found\n");
        }

        if (logf) {
            fflush(logf);
            fclose(logf);
        }
    }

    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DLL entry
 * ═══════════════════════════════════════════════════════════════════════════ */

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        g_thread = CreateThread(NULL, 0, entity_thread, NULL, 0, NULL);
    } else if (reason == DLL_PROCESS_DETACH) {
        g_running = 0;
    }
    return TRUE;
}
