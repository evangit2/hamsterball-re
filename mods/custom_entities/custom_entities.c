/*
 * custom_entities.c — Hamsterball Custom Entities Mod
 *
 * Main mod DLL (bass.dll proxy). Scans all loaded MeshBuffers for custom
 * entity names (CE:CustomName, E:CustomName, N:CustomName). When found,
 * loads the matching behavior DLL from the CustomEntities/ folder and
 * calls its update function every frame.
 *
 * CE: prefix — Custom Entity Reference System:
 *   When an object named "CE:Rotator" is found in a level .MESHWORLD file
 *   from the Levels/ folder, it acts as a reference pointer. The object's
 *   position, rotation, and scale properties define the initial transform
 *   for the custom entity. The mesh geometry and behavior DLL are loaded
 *   from CustomEntities/Rotator.MESHWORLD and CustomEntities/Rotator.dll
 *   respectively.
 *
 *   Flow:
 *     1. Level1.MESHWORLD contains "CE:Rotator" with pos/rot/scale
 *     2. Mod detects CE: prefix, extracts entity name "Rotator"
 *     3. Mesh geometry loaded from CustomEntities/Rotator.MESHWORLD
 *     4. Behavior loaded from CustomEntities/Rotator.dll
 *     5. Behavior_Update called every frame to animate the entity
 *
 * E:/N: prefix — Legacy Custom Entity Support:
 *   Entities named "E:Rotator" or "N:Rotator" are handled the same way
 *   as before, loading behavior DLLs from CustomEntities/<name>.dll.
 *
 * The mod is non-invasive — it doesn't patch any game code or hooks.
 * Instead, it uses a background polling thread that:
 *   1. Waits for the game to load a level (board becomes available)
 *   2. Scans all MeshBuffers in the MeshWorld for custom entity names
 *   3. Loads matching behavior DLLs from CustomEntities/<EntityName>.dll
 *   4. Calls Behavior_Init for each matched entity
 *   5. Calls Behavior_Update every frame (~60Hz)
 *   6. Calls Behavior_Shutdown when the level changes/unloads
 *
 * This approach is safe because:
 *   - No code patches → no crash risk from wrong addresses
 *   - No hooks → no stack/ESP corruption
 *   - All writes go through validated EntityTransform pointers
 *   - IsBadReadPtr guards all pointer dereferences
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll custom_entities.c \
 *     -I../shared -lwinmm -Wl,--enable-stdcall-fixup -O2 -static \
 *     -static-libgcc -Wl,--add-stdcall-alias -msse2 -mfpmath=sse
 *
 * Requirements:
 *   - Rename original bass.dll to bass_real.dll
 *   - Create CustomEntities/ folder in game root
 *   - Place behavior DLLs (e.g. Rotator.dll) and .MESHWORLD files in CustomEntities/
 *   - Place level .MESHWORLD files in Levels/ folder
 */

#include "bass_proxy.h"
#include <shlwapi.h>
#include <ctype.h>
#include <stdio.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * Behavior DLL Interface
 * ═══════════════════════════════════════════════════════════════════════════ */

/* EntityTransform — 0x50 bytes at MeshWorld+0x28 + index * 0x50 */
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

/* Maximum number of custom entities we can track simultaneously */
#define MAX_CUSTOM_ENTITIES 64

/* Entity prefix types */
#define PREFIX_CE   1   /* CE: — Custom Entity reference (loads meshworld + behavior) */
#define PREFIX_E    2   /* E:  — Legacy custom entity (behavior only) */
#define PREFIX_N    3   /* N:  — Legacy custom entity (behavior only) */

typedef struct {
    EntityTransform*  transform;     /* Pointer to entity's transform data */
    HMODULE            behavior_dll; /* Loaded behavior DLL handle */
    Behavior_Init_t    init_fn;      /* Behavior_Init function pointer */
    Behavior_Update_t  update_fn;    /* Behavior_Update function pointer */
    Behavior_Shutdown_t shutdown_fn; /* Behavior_Shutdown function pointer */
    char               entity_name[256]; /* Entity name (e.g. "Rotator") */
    int                prefix_type;  /* PREFIX_CE, PREFIX_E, or PREFIX_N */
    int                initialized;  /* 1 = Behavior_Init called */
} CustomEntity;

static CustomEntity g_entities[MAX_CUSTOM_ENTITIES];
static int g_entity_count = 0;
static CRITICAL_SECTION g_cs;
static int g_cs_initialized = 0;

/* Game thread state */
static HANDLE g_thread = NULL;
static volatile int g_running = 1;
static char g_game_dir[MAX_PATH] = {0};
static char g_entities_dir[MAX_PATH] = {0};   /* CustomEntities/ folder */
static char g_levels_dir[MAX_PATH] = {0};       /* Levels/ folder */

/* Track current board to detect level changes */
static DWORD g_last_board = 0;

/* ═══════════════════════════════════════════════════════════════════════════
 * MeshBuffer structure offsets (from Scene_LoadMeshWorld decompilation)
 *
 * MeshBuffer is 0x874 bytes, allocated at 0x4121D0+ in Scene_LoadMeshWorld.
 * The MeshWorld object stores its MeshBuffer list at MeshWorld+0x2C (AthenaList).
 *
 * MeshBuffer layout:
 *   +0x04    DWORD  render_ctx_index  (maps to EntityTransform at MW+0x28 + idx*0x50)
 *   +0x0864  char*  name             (entity name string, e.g. "CE:Rotator")
 *   +0x0863  BYTE   is_event_entity  (1 if name starts with "E:")
 * ═══════════════════════════════════════════════════════════════════════════ */

#define MESHWORLD_OFFSET            0x08      /* Scene+0x08 → MeshWorld */
#define MESHWORLD_RENDERCTX_PTR     0x28      /* MeshWorld+0x28 → EntityTransform array */
#define MESHWORLD_MESHBUFFER_LIST   0x2C      /* MeshWorld+0x2C → AthenaList of MeshBuffers */

/* AthenaList layout (from AthenaList_Append 0x453780 decompilation) */
#define ATHENALIST_COUNT            0x04      /* +0x04 = item count (int) */
#define ATHENALIST_DATA             0x40C     /* +0x40C = data array (DWORD*) */

/* MeshBuffer offsets */
#define MESHBUFFER_NAME             0x864     /* char* name */
#define MESHBUFFER_CTX_INDEX        0x04      /* DWORD render context index */
#define MESHBUFFER_IS_EVENT        0x863     /* BYTE: 1 if name starts with E: */

/* Board offsets */
#define BOARD_SCENE                 0x878     /* Board+0x878 → Scene* */
#define BOARD_LEVEL                 0x8AC     /* Board+0x8AC → Level* (verified from CreateMouseTrap 0x40BF50) */

/* Level offsets */
#define LEVEL_SCENEOBJECT           0x480     /* Level+0x480 → SceneObject* (spatial tree base, 0x10d4 bytes) */

/* SceneObject offsets — S1 ref point AthenaList (verified from CreateMouseTrap decompilation) */
#define SCENEOBJ_S1_LIST            0x894     /* SceneObject+0x894 → AthenaList base for S1 ref points */
/* AthenaList layout: base+0x04=count, base+0x40C=data array (DWORD* of S1 entry pointers) */

/* S1 ref point entry layout in memory (verified from CreateMouseTrap decompilation):
 *   +0x00: char* name      (puVar5[0] — passed to __stricmp)
 *   +0x04: float  posX     (puVar5[1] — copied to obj+0x10DC)
 *   +0x08: float  posY     (puVar5[2] — copied to obj+0x10E0)
 *   +0x0C: float  posZ     (puVar5[3] — copied to obj+0x10E4)
 *   +0x10: float  rotX      (puVar5[4])
 *   +0x14: float  rotY      (puVar5[5] — used for facing direction: _DAT_004cf44c - rotY)
 *   +0x18: float  rotZ      (puVar5[6])
 * Each entry is accessed as undefined4* (DWORD array), so puVar5[N] = entry + N*4
 */
#define S1ENTRY_NAME                0x00      /* char* name */
#define S1ENTRY_POS_X               0x04      /* float posX */
#define S1ENTRY_POS_Y               0x08      /* float posY */
#define S1ENTRY_POS_Z               0x0C      /* float posZ */
#define S1ENTRY_ROT_X               0x10      /* float rotX */
#define S1ENTRY_ROT_Y               0x14      /* float rotY */
#define S1ENTRY_ROT_Z               0x18      /* float rotZ */
#define S1ENTRY_SIZE                0x1C      /* minimum safe read size */

/* Scene offsets */
#define SCENE_BOARD                 0x47C     /* Scene+0x47C → Board* (param_2 in LoadMeshWorld) */

/* App struct */
#define APP_PROFILE                 0x220     /* App+0x220 → PlayerProfile* */
#define PROFILE_BOARD               0x0C      /* Profile+0x0C → Board* */

/* get_board() is provided by bass_proxy.h (shared header) */

/* ═══════════════════════════════════════════════════════════════════════════
 * Utility: Get Scene pointer from board
 * ═══════════════════════════════════════════════════════════════════════════ */

static DWORD get_scene(DWORD board) {
    if (!board) return 0;
    if (IsBadReadPtr((void*)(board + BOARD_SCENE), 4)) return 0;
    DWORD scene = *(DWORD*)(board + BOARD_SCENE);
    if (!scene || scene < 0x10000) return 0;
    return scene;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Utility: Get MeshWorld pointer from scene
 * ═══════════════════════════════════════════════════════════════════════════ */

static DWORD get_meshworld(DWORD scene) {
    if (!scene) return 0;
    if (IsBadReadPtr((void*)(scene + MESHWORLD_OFFSET), 4)) return 0;
    DWORD mw = *(DWORD*)(scene + MESHWORLD_OFFSET);
    if (!mw || mw < 0x10000) return 0;
    return mw;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Utility: Get game directory (where Hamsterball.exe lives)
 * Sets g_game_dir, g_entities_dir (CustomEntities/), g_levels_dir (Levels/)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void init_game_dir(void) {
    /* Try GetModuleHandleEx first (works on real Windows) */
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
            snprintf(g_entities_dir, MAX_PATH, "%s\\CustomEntities", path);
            snprintf(g_levels_dir, MAX_PATH, "%s\\Levels", path);
            return;
        }
    }

    /* Fallback: use current working directory (works on Wine) */
    if (GetCurrentDirectoryA(MAX_PATH, path) > 0) {
        strcpy(g_game_dir, path);
        snprintf(g_entities_dir, MAX_PATH, "%s\\CustomEntities", path);
        snprintf(g_levels_dir, MAX_PATH, "%s\\Levels", path);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Check if a CustomEntities/<name>.MESHWORLD file exists
 *
 * This is used to verify that the CE: reference points to a valid
 * custom entity definition file.
 * ═══════════════════════════════════════════════════════════════════════════ */

static int entity_meshworld_exists(const char* entity_name) {
    char mw_path[MAX_PATH];
    snprintf(mw_path, MAX_PATH, "%s\\%s.MESHWORLD", g_entities_dir, entity_name);
    DWORD attr = GetFileAttributesA(mw_path);
    return (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY));
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Load a behavior DLL from the CustomEntities/ folder
 *
 * For CE: entities: tries CustomEntities/<EntityName>.dll
 * Returns 1 on success, 0 on failure.
 * Sets behavior_dll, init_fn, update_fn, shutdown_fn on success.
 * ═══════════════════════════════════════════════════════════════════════════ */

static int load_behavior_dll(CustomEntity* ent) {
    char dll_path[MAX_PATH];
    snprintf(dll_path, MAX_PATH, "%s\\%s.dll", g_entities_dir, ent->entity_name);

    ent->behavior_dll = LoadLibraryA(dll_path);
    if (!ent->behavior_dll) {
        /* Try lowercase filename */
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

    /* Update is the only required function */
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
        /* Free S1-allocated transforms (game-owned transforms must NOT be freed).
         * Game-owned transforms point into MeshWorld+0x28 (game heap, < 0x10000000).
         * LocalAlloc'd fallback transforms are > 0x10000000 on Windows. */
        if (ent->transform) {
            if ((DWORD)ent->transform > 0x10000000) {
                LocalFree(ent->transform);
            }
            /* Game-owned transforms (<= 0x10000000) are freed by the game itself */
        }
        ent->initialized = 0;
        ent->transform = NULL;
        ent->init_fn = NULL;
        ent->update_fn = NULL;
        ent->shutdown_fn = NULL;
    }
    g_entity_count = 0;
}

/* Forward declarations — defined later in file, needed by scan_s1_ref_points */
static int detect_entity_prefix(const char* mesh_name, char* ent_name, int ent_name_size);
static int is_known_entity(const char* name);

/* ═══════════════════════════════════════════════════════════════════════════
 * Scan S1 ref points for custom entity names
 *
 * The game's entity spawning system (CreateMouseTrap at 0x40BF50, etc.)
 * iterates S1 ref points via the pointer chain:
 *   board+0x8AC → Level+0x480 → SceneObject+0x894 (AthenaList of S1 entries)
 *
 * Each S1 entry has: char* name at +0x00, float pos[3] at +0x04, float rot[3] at +0x10
 *
 * CE: entities in the level MESHWORLD's S1 section provide position/rotation
 * but the merger may blank the MeshBuffer names. So we MUST scan S1 ref points
 * to find CE: entities — the MeshBuffer scan alone is not sufficient.
 *
 * "REF:" prefix support: Some MESHWORLD files use "REF:CE:Rotator" instead
 * of "CE:Rotator". We strip "REF:" before checking for CE:/E:/N:.
 * ═══════════════════════════════════════════════════════════════════════════ */

static void scan_s1_ref_points(DWORD board, FILE* logf) {
    if (!board) return;

    /* board → Level (board+0x8AC) */
    if (IsBadReadPtr((void*)(board + BOARD_LEVEL), 4)) return;
    DWORD level = *(DWORD*)(board + BOARD_LEVEL);
    if (!level || level < 0x10000) return;

    /* Level → SceneObject (Level+0x480) */
    if (IsBadReadPtr((void*)(level + LEVEL_SCENEOBJECT), 4)) return;
    DWORD sceneobj = *(DWORD*)(level + LEVEL_SCENEOBJECT);
    if (!sceneobj || sceneobj < 0x10000) return;

    /* SceneObject → S1 AthenaList (EMBEDDED at SceneObject+0x894, not a pointer)
     * CreateMouseTrap uses: AthenaList_GetIterator(sceneobj + 0x894)
     *   count = *(sceneobj + 0x894 + 0x04) = *(sceneobj + 0x898)
     *   data  = *(sceneobj + 0x894 + 0x40C) = *(sceneobj + 0xCA0)
     * The AthenaList is embedded inline, not a pointer to an external allocation. */
    DWORD s1_list = sceneobj + SCENEOBJ_S1_LIST;  /* EMBEDDED, not dereferenced */
    if (s1_list < 0x10000) return;

    /* AthenaList: count at +0x04, data array pointer at +0x40C */
    if (IsBadReadPtr((void*)(s1_list + ATHENALIST_COUNT), 4)) return;
    int s1_count = *(int*)(s1_list + ATHENALIST_COUNT);
    if (s1_count < 1 || s1_count > 10000) return;

    if (IsBadReadPtr((void*)(s1_list + ATHENALIST_DATA), 4)) return;
    DWORD* s1_array = *(DWORD**)(s1_list + ATHENALIST_DATA);
    if (!s1_array || IsBadReadPtr(s1_array, s1_count * 4)) return;

    if (logf) fprintf(logf, "  S1 ref point scan: %d entries (sceneobj=0x%08X s1_list=0x%08X)\n",
                     s1_count, (unsigned)sceneobj, (unsigned)s1_list);

    int i;
    for (i = 0; i < s1_count && g_entity_count < MAX_CUSTOM_ENTITIES; i++) {
        DWORD entry = s1_array[i];
        if (!entry || entry < 0x10000) continue;
        if (IsBadReadPtr((void*)entry, S1ENTRY_SIZE)) continue;

        /* Read name pointer */
        if (IsBadReadPtr((void*)(entry + S1ENTRY_NAME), 4)) continue;
        char* name = *(char**)(entry + S1ENTRY_NAME);
        if (!name || IsBadReadPtr(name, 3)) continue;

        /* Strip "REF:" prefix if present (some MESHWORLD files use REF:CE:Name) */
        const char* effective_name = name;
        if (_strnicmp(name, "REF:", 4) == 0) {
            effective_name = name + 4;
        }

        /* Detect entity prefix */
        char ent_name[256];
        int prefix_type = detect_entity_prefix(effective_name, ent_name, sizeof(ent_name));
        if (!prefix_type) continue;

        if (logf) {
            const char* prefix_str = (prefix_type == PREFIX_CE) ? "CE" :
                                     (prefix_type == PREFIX_E)  ? "E"  : "N";
            fprintf(logf, "  S1[%d]: name=\"%s%s\" (effective=\"%s%s\")\n",
                    i, (name != effective_name) ? "REF:" : "", name + (name != effective_name ? 4 : 0),
                    prefix_str, ent_name);
        }

        /* Skip known game entities (only for E:/N: prefix, not CE:) */
        if (prefix_type != PREFIX_CE && is_known_entity(ent_name)) {
            if (logf) fprintf(logf, "    -> '%s' is known game entity, skipping\n", ent_name);
            continue;
        }

        /* Skip if already tracked (by entity name) */
        int k;
        for (k = 0; k < g_entity_count; k++) {
            if (_stricmp(g_entities[k].entity_name, ent_name) == 0)
                break;
        }
        if (k < g_entity_count) {
            if (logf) fprintf(logf, "    -> '%s' already tracked, skipping\n", ent_name);
            continue;
        }

        /* For CE: entities, verify CustomEntities/<name>.MESHWORLD exists */
        if (prefix_type == PREFIX_CE) {
            if (!entity_meshworld_exists(ent_name)) {
                if (logf) fprintf(logf, "    -> CustomEntities/%s.MESHWORLD not found, skipping\n", ent_name);
                continue;
            }
        }

        /* Read S1 position and rotation directly */
        float posX = *(float*)(entry + S1ENTRY_POS_X);
        float posY = *(float*)(entry + S1ENTRY_POS_Y);
        float posZ = *(float*)(entry + S1ENTRY_POS_Z);
        float rotX = *(float*)(entry + S1ENTRY_ROT_X);
        float rotY = *(float*)(entry + S1ENTRY_ROT_Y);
        float rotZ = *(float*)(entry + S1ENTRY_ROT_Z);

        if (logf) {
            const char* prefix_str = (prefix_type == PREFIX_CE) ? "CE" :
                                     (prefix_type == PREFIX_E)  ? "E"  : "N";
            fprintf(logf, "    -> CUSTOM ENTITY '%s:%s' from S1! pos=(%.1f,%.1f,%.1f) rot=(%.3f,%.3f,%.3f)\n",
                    prefix_str, ent_name, posX, posY, posZ, rotX, rotY, rotZ);
        }

        /* Find the game's EntityTransform for this entity.
         *
         * The S1 ref point provides position/rotation, but the game's render
         * system uses EntityTransform at MeshWorld+0x28 + ctx_idx * 0x50.
         * We must find the MeshBuffer with matching name, get its ctx_idx,
         * and use the GAME's EntityTransform — NOT a local copy.
         *
         * A local EntityTransform would be disconnected from the render system:
         * the behavior DLL would modify it, but the game would never read it.
         *
         * Strategy: scan MeshBuffers at MeshWorld+0x2C for matching name.
         * If found, use its ctx_idx to resolve the game EntityTransform.
         * Then write the S1 position/rotation INTO the game's EntityTransform
         * (overriding the identity transform that the file parser sets).
         * If not found, fall back to local EntityTransform. */
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
                                                /* Search for MeshBuffer with matching name */
                                                int mb_i;
                                                for (mb_i = 0; mb_i < mb_count; mb_i++) {
                                                    DWORD mb = mb_array[mb_i];
                                                    if (!mb || mb < 0x10000) continue;
                                                    if (IsBadReadPtr((void*)mb, 0x900)) continue;
                                                    if (IsBadReadPtr((void*)(mb + MESHBUFFER_NAME), 4)) continue;
                                                    char* mb_name = *(char**)(mb + MESHBUFFER_NAME);
                                                    if (!mb_name || IsBadReadPtr(mb_name, 3)) continue;
                                                    if (_stricmp(mb_name, effective_name) == 0) {
                                                        DWORD ctx_idx = *(DWORD*)(mb + MESHBUFFER_CTX_INDEX);
                                                        EntityTransform* t = &transforms[ctx_idx];
                                                        if (!IsBadReadPtr(t, sizeof(EntityTransform))) {
                                                            game_transform = t;
                                                            if (logf) fprintf(logf, "    -> Found matching MeshBuffer[%d] ctx_idx=%d transform=0x%08X\n",
                                                                    mb_i, ctx_idx, (unsigned)game_transform);
                                                        }
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
        strncpy(ent->entity_name, ent_name, 255);
        ent->prefix_type = prefix_type;

        if (game_transform) {
            /* Use the GAME's EntityTransform — write S1 position/rotation into it.
             * This connects the behavior DLL to the game's render system.
             * The behavior DLL's Update will modify rotX/rotY/rotZ directly
             * on the transform the game reads every frame for rendering. */
            ent->transform = game_transform;
            ent->transform->posX = posX;
            ent->transform->posY = posY;
            ent->transform->posZ = posZ;
            ent->transform->rotX = rotX;
            ent->transform->rotY = rotY;
            ent->transform->rotZ = rotZ;
            ent->transform->rotScale = 1.0f;
            ent->transform->posScale = 1.0f;
            if (logf) fprintf(logf, "    -> Using GAME EntityTransform (ctx_idx resolved from MeshBuffer)\n");
        } else {
            /* Fallback: no matching MeshBuffer found.
             * Allocate a local EntityTransform. The behavior DLL will run,
             * but the mesh won't be animated by the game (disconnected). */
            ent->transform = (EntityTransform*)LocalAlloc(LPTR, sizeof(EntityTransform));
            if (!ent->transform) continue;
            ent->transform->posX = posX;
            ent->transform->posY = posY;
            ent->transform->posZ = posZ;
            ent->transform->rotX = rotX;
            ent->transform->rotY = rotY;
            ent->transform->rotZ = rotZ;
            ent->transform->rotScale = 1.0f;
            ent->transform->posScale = 1.0f;
            if (logf) fprintf(logf, "    -> WARNING: No matching MeshBuffer found, using local EntityTransform (disconnected)\n");
        }

        if (!load_behavior_dll(ent)) {
            if (logf) fprintf(logf, "    -> Could not load CustomEntities/%s.dll, skipping\n", ent_name);
            LocalFree(ent->transform);
            ent->transform = NULL;
            continue;
        }

        if (logf) fprintf(logf, "    -> Loaded CustomEntities/%s.dll successfully!\n", ent_name);

        /* Call Behavior_Init */
        if (ent->init_fn) {
            ent->init_fn(ent->transform, (void*)board);
        }
        ent->initialized = 1;
        g_entity_count++;
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Scan MeshWorld MeshBuffers for custom entity names
 *
 * The game's MeshWorld has an AthenaList of MeshBuffers at MeshWorld+0x2C.
 * Each MeshBuffer has a name string at +0x864. Names starting with "CE:",
 * "E:", or "N:" that DON'T match any known game entity type are custom entities.
 *
 * CE: prefix (Custom Entity Reference):
 *   The CE: prefix indicates a custom entity reference. The object in the
 *   level .MESHWORLD provides the initial position, rotation, and scale.
 *   The mesh geometry and behavior DLL are loaded from the CustomEntities/
 *   folder (e.g., CustomEntities/Rotator.MESHWORLD + CustomEntities/Rotator.dll).
 *
 * E:/N: prefix (Legacy Custom Entity):
 *   These work the same as before — behavior DLL loaded from CustomEntities/.
 *
 * For each custom entity found, we:
 *   1. Extract the entity name (after the CE:/E:/N: prefix)
 *   2. For CE: entities, verify CustomEntities/<name>.MESHWORLD exists
 *   3. Load CustomEntities/<EntityName>.dll
 *   4. Resolve Behavior_Init, Behavior_Update, Behavior_Shutdown
 *   5. Call Behavior_Init with the entity's EntityTransform pointer
 * ═══════════════════════════════════════════════════════════════════════════ */

/*
 * Known game entity names that use E: or N: prefix in MESHWORLD files.
 * These are handled by the game's event/collision dispatch and should NOT
 * be intercepted by our mod (to avoid conflicts).
 *
 * IMPORTANT: Mechanical objects (ROTATOR, GEAR, LOOPER, PENDULUM, etc.) are
 * ONLY matched by the game when their name has NO E:/N: prefix. A mesh named
 * "E:Rotator" will NOT trigger the game's native Rotator_ctor — so it's safe
 * for our mod to handle it. Only event entities that the game dispatches via
 * E:/N: prefix are listed here.
 *
 * NOTE: CE: prefix entities are NEVER game-native — the game doesn't
 * recognize the CE: prefix at all, so all CE: entities are custom.
 */
static const char* known_entities[] = {
    /* E: prefix event entities (dispatched by collision system) */
    "JUMP", "NODIZZY", "LIMIT", "TELEPORT", "WATER",
    "DROPIN", "PIPEBONK", "POPOUT", "VACPOPOUT", "TARPIT",
    "GOAL", "GOAL1", "GOAL2", "GOAL3", "GOAL4", "GOAL5", "GOAL6",
    "BUMPER", "BUMPER1", "BUMPER2", "BUMPER3", "BUMPER4",
    "BUMPER5", "BUMPER6", "BUMPER7", "BUMPER8",
    "FAN", "SAWBLADE", "JUDGE", "BELL", "LAUNCH",
    "MOUSETRAP", "TARBUBBLE", "FALL", "SWIRL", "SPEEDPAD",
    /* N: prefix entities */
    "GLASS",
    NULL
};

static int is_known_entity(const char* name) {
    int i;
    for (i = 0; known_entities[i]; i++) {
        if (_stricmp(name, known_entities[i]) == 0)
            return 1;
    }
    return 0;
}

/*
 * Detect entity prefix and extract the entity name.
 * Returns the prefix type (PREFIX_CE, PREFIX_E, PREFIX_N) or 0 if no match.
 * Sets ent_name to the entity name without the prefix.
 */
static int detect_entity_prefix(const char* mesh_name, char* ent_name, int ent_name_size) {
    int prefix_type = 0;
    const char* suffix = NULL;

    if (_strnicmp(mesh_name, "CE:", 3) == 0) {
        prefix_type = PREFIX_CE;
        suffix = mesh_name + 3;
    } else if (_strnicmp(mesh_name, "E:", 2) == 0) {
        prefix_type = PREFIX_E;
        suffix = mesh_name + 2;
    } else if (_strnicmp(mesh_name, "N:", 2) == 0) {
        prefix_type = PREFIX_N;
        suffix = mesh_name + 2;
    }

    if (!suffix || !prefix_type)
        return 0;

    /* Extract entity name (stop at '(' modifier or end of string) */
    int j;
    for (j = 0; suffix[j] && suffix[j] != '(' && j < ent_name_size - 1; j++)
        ent_name[j] = suffix[j];
    ent_name[j] = '\0';

    return prefix_type;
}

static void scan_for_custom_entities(DWORD board) {
    DWORD scene = get_scene(board);
    if (!scene) return;

    DWORD mw = get_meshworld(scene);
    if (!mw) return;

    /* Get MeshBuffer AthenaList at MeshWorld+0x2C */
    if (IsBadReadPtr((void*)(mw + MESHWORLD_MESHBUFFER_LIST), 4)) return;
    DWORD mb_list = *(DWORD*)(mw + MESHWORLD_MESHBUFFER_LIST);
    if (!mb_list || mb_list < 0x10000) return;

    /* Read AthenaList count and data pointer */
    if (IsBadReadPtr((void*)(mb_list + ATHENALIST_COUNT), 4)) return;
    int mb_count = *(int*)(mb_list + ATHENALIST_COUNT);
    if (mb_count < 1 || mb_count > 10000) return;

    if (IsBadReadPtr((void*)(mb_list + ATHENALIST_DATA), 4)) return;
    DWORD* mb_array = *(DWORD**)(mb_list + ATHENALIST_DATA);
    if (!mb_array || IsBadReadPtr(mb_array, mb_count * 4)) return;

    /* Get RenderContext array (EntityTransform[]) at MeshWorld+0x28 */
    if (IsBadReadPtr((void*)(mw + MESHWORLD_RENDERCTX_PTR), 4)) return;
    EntityTransform* transforms = *(EntityTransform**)(mw + MESHWORLD_RENDERCTX_PTR);
    if (!transforms) return;

    /* Open a log file for diagnostics */
    char log_path[MAX_PATH];
    snprintf(log_path, MAX_PATH, "%s\\\\custom_entities.log", g_game_dir);
    FILE* logf = NULL; fopen_s(&logf, log_path, "a");

    int i;
    for (i = 0; i < mb_count && g_entity_count < MAX_CUSTOM_ENTITIES; i++) {
        DWORD mb = mb_array[i];
        if (!mb || mb < 0x10000) continue;
        if (IsBadReadPtr((void*)mb, 0x900)) continue;

        /* Read name pointer at MeshBuffer+0x864 */
        if (IsBadReadPtr((void*)(mb + MESHBUFFER_NAME), 4)) continue;
        char* name = *(char**)(mb + MESHBUFFER_NAME);
        if (!name || IsBadReadPtr(name, 3)) continue;

        /* Detect entity prefix (CE:, E:, or N:) */
        char ent_name[256];
        int prefix_type = detect_entity_prefix(name, ent_name, sizeof(ent_name));
        if (!prefix_type) continue;

        /* Log every custom entity mesh name for diagnostics */
        if (logf) {
            const char* prefix_str = (prefix_type == PREFIX_CE) ? "CE" :
                                     (prefix_type == PREFIX_E)  ? "E"  : "N";
            fprintf(logf, "  MeshBuffer[%d]: name=\"%s:%s\" ctx_idx=%d\n",
                    i, prefix_str, ent_name, *(DWORD*)(mb + MESHBUFFER_CTX_INDEX));
        }

        /* Skip known game entities (only for E:/N: prefix, not CE:) */
        if (prefix_type != PREFIX_CE && is_known_entity(ent_name)) {
            if (logf) fprintf(logf, "  -> '%s' is known game entity, skipping\n", ent_name);
            continue;
        }

        /* Skip if already tracked */
        int k;
        int already_tracked = 0;
        for (k = 0; k < g_entity_count; k++) {
            if (_stricmp(g_entities[k].entity_name, ent_name) == 0) {
                already_tracked = 1;
                break;
            }
        }
        if (already_tracked) continue;

        /* Read the render context index from MeshBuffer+0x04 */
        DWORD ctx_idx = *(DWORD*)(mb + MESHBUFFER_CTX_INDEX);

        /* Get the EntityTransform pointer */
        EntityTransform* transform = &transforms[ctx_idx];
        if (IsBadReadPtr(transform, sizeof(EntityTransform))) continue;

        if (logf) {
            const char* prefix_str = (prefix_type == PREFIX_CE) ? "CE" :
                                     (prefix_type == PREFIX_E)  ? "E"  : "N";
            fprintf(logf, "  -> CUSTOM ENTITY '%s:%s' found! ctx_idx=%d transform=0x%08X "
                    "pos=(%.1f,%.1f,%.1f) rot=(%.3f,%.3f,%.3f) scale=(%.3f,%.3f)\n",
                    prefix_str, ent_name, ctx_idx, (unsigned)transform,
                    transform->posX, transform->posY, transform->posZ,
                    transform->rotX, transform->rotY, transform->rotZ,
                    transform->rotScale, transform->posScale);
        }

        /* For CE: entities:
         *   The CE:Rotator S1 ref point provides the position/rotation.
         *   The mesh geometry was merged into the level at DllMain time
         *   (by merge_all_levels), so it's already in the vertex buffer
         *   with empty-named geoms in the octree. The game renders these
         *   as unnamed static geometry.
         *
         *   The EntityTransform at the CE: ref point's context index
         *   controls the position/rotation of the mesh group.
         *
         *   We verify CustomEntities/<name>.MESHWORLD exists, then
         *   load the behavior DLL to animate the transform.
         *
         *   Note: After the file merge, the MeshBuffer scan may find
         *   these geoms with empty names (not CE: prefix). The S1 ref
         *   point still has the CE: name, so we detect it here.
         */
        if (prefix_type == PREFIX_CE) {
            if (!entity_meshworld_exists(ent_name)) {
                if (logf) fprintf(logf, "  -> CustomEntities/%s.MESHWORLD not found, skipping\n", ent_name);
                continue;
            }
            if (logf) fprintf(logf, "  -> CustomEntities/%s.MESHWORLD found\n", ent_name);
        }

        /* Load behavior DLL from CustomEntities/ */
        CustomEntity* ent = &g_entities[g_entity_count];
        memset(ent, 0, sizeof(CustomEntity));
        strncpy(ent->entity_name, ent_name, 255);
        ent->transform = transform;
        ent->prefix_type = prefix_type;

        if (!load_behavior_dll(ent)) {
            if (logf) fprintf(logf, "  -> Could not load CustomEntities/%s.dll, skipping\n", ent_name);
            continue;
        }

        if (logf) fprintf(logf, "  -> Loaded CustomEntities/%s.dll successfully!\n", ent_name);

        /* Call Behavior_Init */
        if (ent->init_fn) {
            ent->init_fn(transform, (void*)board);
        }
        ent->initialized = 1;
        g_entity_count++;
    }

    if (logf) {
        fprintf(logf, "MeshBuffer scan complete: %d custom entities loaded\n", g_entity_count);
        fprintf(logf, "Now scanning S1 ref points...\n");
    }

    /* Also scan S1 ref points (CE: entities may only appear in S1, not MeshBuffers) */
    scan_s1_ref_points(board, logf);

    if (logf) {
        fprintf(logf, "Scan complete: %d total custom entities loaded\n\n", g_entity_count);
        fclose(logf);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Main mod thread — polls for level changes, scans entities, calls updates
 * ═══════════════════════════════════════════════════════════════════════════ */

static DWORD WINAPI mod_thread(LPVOID param) {
    /* Wait for game to finish initializing */
    Sleep(3000);

    /* Write startup log */
    {
        char log_path[MAX_PATH];
        snprintf(log_path, MAX_PATH, "%s\\custom_entities.log", g_game_dir);
        FILE* f = NULL;
        fopen_s(&f, log_path, "a");
        if (f) {
            fprintf(f, "=== Custom Entities Mod Started ===\n");
            fprintf(f, "Game dir: %s\n", g_game_dir);
            fprintf(f, "CustomEntities dir: %s\n", g_entities_dir);
            fprintf(f, "Levels dir: %s\n", g_levels_dir);
            fclose(f);
        }
    }

    while (g_running) {
        DWORD board = get_board();

        /* Check for level change */
        if (board != g_last_board) {
            /* Level changed — shutdown old entities */
            if (g_last_board != 0) {
                shutdown_all_entities();
            }
            g_last_board = board;

            /* If new board is valid, scan for custom entities */
            if (board) {
                Sleep(500); /* Give level loading time to finish */
                board = get_board(); /* Re-read in case it changed */
                if (board) {
                    scan_for_custom_entities(board);
                }
            }
        }

        /* Call Behavior_Update for all active entities */
        if (g_entity_count > 0 && board) {
            int i;
            for (i = 0; i < g_entity_count; i++) {
                CustomEntity* ent = &g_entities[i];
                if (ent->initialized && ent->update_fn && ent->transform) {
                    /* Validate transform pointer is still readable */
                    if (!IsBadReadPtr(ent->transform, sizeof(EntityTransform))) {
                        ent->update_fn(ent->transform, (void*)board);
                    }
                }
            }
        }

        Sleep(16); /* ~60Hz update rate */
    }

    shutdown_all_entities();
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DllMain — entry point
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Merge CE: entity meshes into level files before game loads them */
extern void merge_all_levels(const char* game_dir);

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        load_real_bass();
        init_game_dir();

        /* Merge CustomEntities/*.MESHWORLD geometry into Levels/*.MESHWORLD
         * files before the game loads them. This happens at DLL load time,
         * which is before the game's level loader runs. */
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
