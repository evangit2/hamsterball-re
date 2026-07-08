/*
 * custom_entities.c — Hamsterball Custom Entities Mod v4
 *
 * Main mod DLL (bass.dll proxy). Scans S1 ref points for entity names
 * containing "CE" (uppercase). When found, loads the matching behavior DLL
 * from Levels/CustomEntities/ and calls its update function every frame.
 *
 * v4 changes:
 *   - Removed CE: prefix system. Now uses plain entity names with "CE" check.
 *   - CustomEntities folder moved inside Levels/ (Levels/CustomEntities/)
 *   - Removed meshworld merger (game's native REF: system handles mesh loading)
 *   - Removed d3d8.dll proxy (EntityTransform writes work with position matching)
 *   - Deletes original REF: entity mesh at level load, keeps only CErotator mesh
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
 * Behavior DLL Interface
 * ═══════════════════════════════════════════════════════════════════════════ */

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
    float  extra[12];  /* +0x24 - +0x4F */
} EntityTransform;

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
    /* Position from S1 ref point */
    float              posX, posY, posZ;
    float              rotX, rotY, rotZ;
} CustomEntity;

static CustomEntity g_entities[MAX_CUSTOM_ENTITIES];
static int g_entity_count = 0;
static CRITICAL_SECTION g_cs;
static int g_cs_initialized = 0;

static HANDLE g_thread = NULL;
static volatile int g_running = 1;
static char g_game_dir[MAX_PATH] = {0};
static char g_entities_dir[MAX_PATH] = {0};   /* Levels/CustomEntities/ */
static char g_levels_dir[MAX_PATH] = {0};       /* Levels/ */

static DWORD g_last_board = 0;

/* ═══════════════════════════════════════════════════════════════════════════
 * Structure offsets (from Ghidra decompilation)
 * ═══════════════════════════════════════════════════════════════════════════ */

#define MESHWORLD_OFFSET            0x08
#define MESHWORLD_RENDERCTX_PTR     0x28
#define MESHWORLD_MESHBUFFER_LIST   0x2C

#define ATHENALIST_COUNT            0x04
#define ATHENALIST_DATA             0x40C

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

/* get_board() is provided by bass_proxy.h */

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
 * Check if an entity name contains "CE" (uppercase) — marks it as a Custom Entity
 * ═══════════════════════════════════════════════════════════════════════════ */

static int is_custom_entity(const char* name) {
    return (strstr(name, "CE") != NULL);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Load a behavior DLL from Levels/CustomEntities/
 * ═══════════════════════════════════════════════════════════════════════════ */

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

/* ═══════════════════════════════════════════════════════════════════════════
 * Shutdown all active entities
 * ═══════════════════════════════════════════════════════════════════════════ */

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
 * Scan S1 ref points for custom entity names (containing "CE")
 *
 * The game's entity spawning system iterates S1 ref points via:
 *   board+0x8AC → Level+0x480 → SceneObject+0x894 (AthenaList of S1 entries)
 *
 * Each S1 entry: char* name@+0x00, float pos[3]@+0x04, float rot[3]@+0x10
 * ═══════════════════════════════════════════════════════════════════════════ */

static void scan_s1_ref_points(DWORD board, FILE* logf) {
    if (!board) return;

    if (IsBadReadPtr((void*)(board + BOARD_LEVEL), 4)) return;
    DWORD level = *(DWORD*)(board + BOARD_LEVEL);
    if (!level || level < 0x10000) return;

    if (IsBadReadPtr((void*)(level + LEVEL_SCENEOBJECT), 4)) return;
    DWORD sceneobj = *(DWORD*)(level + LEVEL_SCENEOBJECT);
    if (!sceneobj || sceneobj < 0x10000) return;

    DWORD s1_list = sceneobj + SCENEOBJ_S1_LIST;
    if (s1_list < 0x10000) return;

    if (IsBadReadPtr((void*)(s1_list + ATHENALIST_COUNT), 4)) return;
    int s1_count = *(int*)(s1_list + ATHENALIST_COUNT);
    if (s1_count < 1 || s1_count > 10000) return;

    if (IsBadReadPtr((void*)(s1_list + ATHENALIST_DATA), 4)) return;
    DWORD* s1_array = *(DWORD**)(s1_list + ATHENALIST_DATA);
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

        /* Check if this is a custom entity (name contains "CE") */
        if (!is_custom_entity(name)) continue;

        /* Skip if already tracked */
        int k;
        for (k = 0; k < g_entity_count; k++) {
            if (_stricmp(g_entities[k].entity_name, name) == 0)
                break;
        }
        if (k < g_entity_count) continue;

        /* Read S1 position and rotation */
        float posX = *(float*)(entry + S1ENTRY_POS_X);
        float posY = *(float*)(entry + S1ENTRY_POS_Y);
        float posZ = *(float*)(entry + S1ENTRY_POS_Z);
        float rotX = *(float*)(entry + S1ENTRY_ROT_X);
        float rotY = *(float*)(entry + S1ENTRY_ROT_Y);
        float rotZ = *(float*)(entry + S1ENTRY_ROT_Z);

        if (logf) fprintf(logf, "  S1[%d]: name='%s' pos=(%.1f,%.1f,%.1f) rot=(%.3f,%.3f,%.3f)\n",
                i, name, posX, posY, posZ, rotX, rotY, rotZ);

        /* Find the game's EntityTransform by matching position.
         * The game's native REF: system loads the CErotator.MESHWORLD
         * and creates MeshBuffers with EntityTransforms. We need to
         * find the one at the S1 ref point's position. */
        EntityTransform* game_transform = NULL;

        DWORD scene = get_scene(board);
        if (scene) {
            DWORD mw = get_meshworld(scene);
            if (mw) {
                if (!IsBadReadPtr((void*)(mw + MESHWORLD_MESHBUFFER_LIST), 4)) {
                    DWORD mb_list = *(DWORD*)(mw + MESHWORLD_MESHBUFFER_LIST);
                    if (mb_list && mb_list >= 0x10000) {
                        if (!IsBadReadPtr((void*)(mb_list + ATHENALIST_COUNT), 4)) {
                            int mb_count = *(int*)(mb_list + ATHENALIST_COUNT);
                            if (mb_count >= 1 && mb_count <= 10000) {
                                if (!IsBadReadPtr((void*)(mb_list + ATHENALIST_DATA), 4)) {
                                    DWORD* mb_array = *(DWORD**)(mb_list + ATHENALIST_DATA);
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
                                                        if (logf) fprintf(logf, "    -> Found EntityTransform by pos match: mb[%d] ctx=%d pos=(%.1f,%.1f,%.1f)\n",
                                                                mb_i, ctx_idx, t->posX, t->posY, t->posZ);
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
            }
        }

        /* Load behavior DLL */
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
            /* Fallback: local transform (won't affect rendering) */
            ent->transform = (EntityTransform*)LocalAlloc(LPTR, sizeof(EntityTransform));
            if (!ent->transform) continue;
            if (logf) fprintf(logf, "    -> WARNING: No matching EntityTransform found (disconnected)\n");
        }

        /* Initialize transform with S1 position/rotation */
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
 * Delete the original REF: entity mesh (hide it by setting alpha to 0)
 *
 * The game's native REF: system loads the original CErotator mesh from
 * the level's octree. This mesh is static (doesn't rotate). We need to
 * hide it so only the CErotator.MESHWORLD mesh (loaded by the game's
 * REF: system as a separate sub-mesh) is visible and rotating.
 *
 * We do this by scanning MeshBuffers for ones with names containing
 * "CErotator" and zeroing their vertex counts or setting them invisible.
 * Actually, the safest approach: set the EntityTransform's posScale to 0
 * for the ORIGINAL mesh (the one at the S1 ref point position that
 * belongs to the level's own geometry, not the REF-loaded mesh).
 * ═══════════════════════════════════════════════════════════════════════════ */

static void hide_original_entity_mesh(DWORD board, FILE* logf) {
    DWORD scene = get_scene(board);
    if (!scene) return;
    DWORD mw = get_meshworld(scene);
    if (!mw) return;

    /* Scan MeshBuffers for ones whose name contains the entity name */
    if (IsBadReadPtr((void*)(mw + MESHWORLD_MESHBUFFER_LIST), 4)) return;
    DWORD mb_list = *(DWORD*)(mw + MESHWORLD_MESHBUFFER_LIST);
    if (!mb_list || mb_list < 0x10000) return;

    if (IsBadReadPtr((void*)(mb_list + ATHENALIST_COUNT), 4)) return;
    int mb_count = *(int*)(mb_list + ATHENALIST_COUNT);
    if (mb_count < 1 || mb_count > 10000) return;

    if (IsBadReadPtr((void*)(mb_list + ATHENALIST_DATA), 4)) return;
    DWORD* mb_array = *(DWORD**)(mb_list + ATHENALIST_DATA);
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

        /* Check if this MeshBuffer's name matches our entity */
        int j;
        for (j = 0; j < g_entity_count; j++) {
            if (!g_entities[j].initialized) continue;
            const char* ent_name = g_entities[j].entity_name;

            /* If the MeshBuffer name contains the entity name AND it's
             * the level's original mesh (not the REF-loaded one), hide it.
             * The level's original mesh has the entity name without "N:" prefix.
             * The REF-loaded mesh has "N:" prefix (from CErotator.MESHWORLD octree). */
            if (_stricmp(name, ent_name) == 0) {
                /* This is the level's original placement mesh — hide it */
                DWORD ctx_idx = *(DWORD*)(mb + MESHBUFFER_CTX_INDEX);
                if (ctx_idx <= 10000) {
                    EntityTransform* t = &transforms[ctx_idx];
                    if (!IsBadReadPtr(t, sizeof(EntityTransform))) {
                        t->posScale = 0.0f;  /* Scale to zero = invisible */
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

    {
        char log_path[MAX_PATH];
        snprintf(log_path, MAX_PATH, "%s\\custom_entities.log", g_game_dir);
        FILE* f = NULL;
        fopen_s(&f, log_path, "a");
        if (f) {
            fprintf(f, "=== Custom Entities Mod v4 Started ===\n");
            fprintf(f, "Game dir: %s\n", g_game_dir);
            fprintf(f, "Entities dir: %s\n", g_entities_dir);
            fprintf(f, "Levels dir: %s\n", g_levels_dir);
            fclose(f);
        }
    }

    while (g_running) {
        DWORD board = get_board();

        if (board != g_last_board) {
            if (g_last_board != 0) {
                shutdown_all_entities();
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

                    scan_s1_ref_points(board, logf);

                    /* Hide the original entity mesh (keep only REF-loaded mesh) */
                    hide_original_entity_mesh(board, logf);

                    if (logf) {
                        fprintf(logf, "Scan complete: %d custom entities loaded\n\n", g_entity_count);
                        fclose(logf);
                    }
                }
            }
        }

        /* Call Behavior_Update for all active entities */
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

        /* Merge CE entity meshes from Levels/CustomEntities/ into Levels/*.MESHWORLD
         * files before the game loads them. Scans S1 for names containing "CE". */
        extern void merge_all_levels(const char* game_dir);
        merge_all_levels(g_game_dir);

        InitializeCriticalSection(&g_cs);
        g_cs_initialized = 1;

        g_thread = CreateThread(NULL, 0, mod_thread, NULL, 0, NULL);
    }
    else if (reason == DLL_PROCESS_DETACH) {
        g_running = 0;
        if (g_thread) {
            WaitForSingleObject(g_thread, 2000);
            CloseHandle(g_thread);
        }
        shutdown_all_entities();
        if (g_cs_initialized) {
            DeleteCriticalSection(&g_cs);
            g_cs_initialized = 0;
        }
    }
    return TRUE;
}
