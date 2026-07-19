/*
 * custom_entities.c — Hamsterball Custom Entities Mod v43
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
#include <math.h>
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
#define ROTATER_SIZE            0x1508  /* Rotator_ctor_Impossible alloc size */
#define PENDULUM_SIZE           0x1504
#define LOOPER_SIZE             0x1500
#define GEAR_SIZE               0x1514
#define SPATIALTREE_SIZE        68

/* ═══════════════════════════════════════════════════════════════════════════
 * REF:Rotater — custom spawning system
 *
 * Scans MESHWORLD section 3 for objects named "REF:Rotater" and spawns
 * a Dizzy Race SWIRL (Rotator_ctor_Impossible) at each position.
 *
 * The game's Rotator_ctor_Impossible (0x435940) signature:
 *   __thiscall(this, board, posX, posY, posZ, mesh)
 *
 * Mesh path "levels\Level3-Swirl" is at 0x004CFFE0 (game .data).
 * MeshWorld_ctor (0x461510) loads the SWIRL mesh from disk.
 *
 * Object layout (from Ghidra decompilation):
 *   this+0x10D0 = board ptr
 *   this+0x10D4 = render object (Level_RenderCtor result)
 *   this+0x10D8/DC/E0 = position X/Y/Z
 *   this+0x10E8 = rotation angle (init 0)
 *   this+0x10EC = rotation direction (init 1.0)
 *   this+0x10F0 = AthenaList (initialized)
 *   vtable = 0x4D5518 (Impossible Rotator)
 *   vtable[11] (offset 0x2C) = RemoveAndFree
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Rotator_ctor_Impossible — creates the spinning SWIRL platform */
typedef void* (__thiscall *Rotator_ctor_t)(void* this_, void* board, float posX, float posY, float posZ, void* mesh);
static Rotator_ctor_t pfn_Rotator_ctor = (Rotator_ctor_t)0x00435940;
static Rotator_ctor_t pfn_Pendulum_ctor = (Rotator_ctor_t)0x437700;
static Rotator_ctor_t pfn_Looper_ctor = (Rotator_ctor_t)0x437460;
static Rotator_ctor_t pfn_Gear_ctor = (Rotator_ctor_t)0x437690;

/* Level3-Swirl mesh path (game .data at 0x004CFFE0) */
static const char* g_swirl_mesh_path = (const char*)0x004CFFE0;

/* AI mesh path table — game .data string addresses for AI 1-5 */
static const char* g_ai_mesh_paths[] = {
    NULL,                              /* AI 0: static (use MESH property) */
    (const char*)0x004D20FC,           /* AI 1: Levels\LevelImpossible-Rotator */
    (const char*)0x004D20DC,           /* AI 2: Levels\LevelImpossible-Pendulum */
    (const char*)0x004D2158,           /* AI 3: Levels\LevelImpossible-Looper */
    (const char*)0x004D213C,           /* AI 4: Levels\LevelImpossible-Gear */
    (const char*)0x004D211C,           /* AI 5: Levels\LevelImpossible-BigGear */
};

/* Rotater spawned objects tracking */
#define MAX_ROTATERS 999

/* Per-rotater config: mesh path and rotation speeds */
typedef struct {
    DWORD obj;              /* spawned object pointer */
    char  mesh_path[128];   /* custom mesh path (empty = default Level3-Swirl) */
    float rot_x;            /* X-axis rotation speed (radians/frame) */
    float rot_y;            /* Y-axis rotation speed (radians/frame) */
    float rot_z;            /* Z-axis rotation speed (radians/frame) */
    float ros_x;         /* X-axis oscillation range (radians, default 2.0) */
    float ros_y;         /* Y-axis oscillation range (radians, default 2.0) */
    float ros_z;         /* Z-axis oscillation range (radians, default 2.0) */
    float angle_x;          /* accumulated X angle */
    float angle_y;          /* accumulated Y angle */
    float angle_z;          /* accumulated Z angle */
} RotaterConfig;

static RotaterConfig g_rotater_cfg[MAX_ROTATERS];
static int   g_rotater_count = 0;
static DWORD g_rotater_board = 0;

/* ═══════════════════════════════════════════════════════════════════════════
 * <MESH> and <SPEEDMULT> tag support — custom BADBALL arguments
 *
 * The game's CreateBadBall (0x40BCA0) parses <CHASE>, <HOME>, <SIZE>,
 * <SPINDISTANCE> tags from BADBALL object names in MESHWORLD section 3.
 * We add two new tags:
 *
 *   <MESH>funball</MESH>       → mesh index 10 (FunBall mesh + texture)
 *   no MESH / other value       → mesh index 9  (8Ball: Sphere + 8ball texture)
 *
 *   <SPEEDMULT>2.0</SPEEDMULT> → multiplies ball+0x188 (max_speed) by value
 *   no SPEEDMULT tag            → max_speed stays at default (6.0)
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Ball struct offsets for tag processing */
#define BALL_HOME_POS_X       0xC60   /* float — spawn/return position X (set by CreateBadBall from obj.x) */
#define BALL_HOME_POS_Y       0xC64   /* float — spawn/return position Y (set by CreateBadBall from obj.y) */
#define BALL_HOME_POS_Z       0xC68   /* float — spawn/return position Z (set by CreateBadBall from obj.z) */
#define BALL_MESH_INDEX_FIELD 0x754   /* int — player/slot index into App mesh array at App+0x244 */
                                     /* Ball_Render only renders meshes for indices 0,1,2 (< 3 check) */
                                     /* 0 = Sphere (default player ball) */
                                     /* 1 = SphereBreak1 (we repurpose for 8Ball) */
                                     /* 2 = SphereBreak2 (we repurpose for FunBall) */
#define BALL_MAX_SPEED        0x188   /* float — max_speed (default 6.0, set by Ball_InitPhysicsDefaults) */

/* App mesh array layout (loaded in TimerDisplay at 0x004298c0):
 *   App+0x244 = "Meshes\Sphere"     [index 0] — default player ball
 *   App+0x248 = "Meshes\SphereBreak1" [index 1] — we copy 8Ball mesh ptr here
 *   App+0x24C = "Meshes\SphereBreak2" [index 2] — we copy FunBall mesh ptr here
 *   App+0x268 = "Meshes\8Ball"      [index 9] — source 8Ball mesh
 *   App+0x26C = "Meshes\FunBall"    [index 10] — source FunBall mesh
 *
 * Ball_Render (0x00403db8) renders mesh at App+0x244[ball+0x754*4] only if
 * ball+0x754 < 3. Setting ball+0x754=9 or 10 makes ball invisible (check fails).
 * Fix: copy 8Ball/FunBall mesh ptrs into slots 1/2, use those indices. */
#define APP_MESH_ARRAY         0x244   /* App offset — start of mesh pointer array */
#define APP_MESH_8BALL         0x268   /* App offset — 8Ball mesh pointer (index 9) */
#define APP_MESH_FUNBALL       0x26C   /* App offset — FunBall mesh pointer (index 10) */
#define MESH_SLOT_8BALL        1       /* We use slot 1 (SphereBreak1) for 8Ball */
#define MESH_SLOT_FUNBALL      2       /* We use slot 2 (SphereBreak2) for FunBall */

/* Section 3 object array (accessed via sceneobj) */
#define SCENEOBJ_OBJ_COUNT    0x898   /* int — total section 3 objects */
#define SCENEOBJ_OBJ_ARRAY    0xCA0   /* DWORD* — pointer to object pointer array */

/* Bad balls list (AthenaList at board+0x29D4) */
#define BOARD_BAD_BALLS_LIST  0x29D4

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

/* Config: seconds between GRID switches */
static float g_grid_speed = 3.0f;

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
 * <MESH> tag processing — apply mesh selection to spawned 8-balls
 *
 * After CreateBadBall runs during level load, we scan the MESHWORLD
 * section 3 objects for BADBALL entries with <MESH> tags, then match
 * them to spawned balls by home position (ball+0xC60/0xC64/0xC68 =
 * obj.x/obj.y/obj.z from the MESHWORLD file).
 *
 * If <MESH>funball</MESH> is found, ball+0x754 is set to 10 (FunBall).
 * Otherwise it stays at the default 9 (8Ball: Sphere + 8ball texture).
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Simple case-insensitive substring search */
static int ci_strstr(const char* haystack, const char* needle) {
    while (*haystack) {
        const char* h = haystack;
        const char* n = needle;
        while (*h && *n && tolower(*h) == tolower(*n)) { h++; n++; }
        if (*n == 0) return 1;
        haystack++;
    }
    return 0;
}

/* Extract a property value from a <DAT> block.
 * Format: <DAT> KEY="value", KEY=number, ... </DAT>
 * or: <DAT> KEY="value", KEY=number </DAT> (no closing tag, end of string)
 * 
 * Searches for KEY= in the DAT block and copies the value (quoted or unquoted)
 * until the next comma or end of block.
 * Returns 1 if found, 0 otherwise. */
static int extract_dat_prop(const char* name, const char* key, char* out_buf, int out_size) {
    /* Find <DAT> in the name string (case-insensitive) */
    const char* p = name;
    while (*p) {
        if (p[0] == '<' && _strnicmp(p + 1, "DAT", 3) == 0 && p[4] == '>') {
            const char* dat_start = p + 5;
            /* Find end of DAT block: </DAT> or end of string */
            const char* dat_end = dat_start;
            while (*dat_end) {
                if (dat_end[0] == '<' && dat_end[1] == '/' &&
                    _strnicmp(dat_end + 2, "DAT", 3) == 0 && dat_end[5] == '>') {
                    break;
                }
                dat_end++;
            }
            
            /* Search for KEY= within the DAT block */
            int key_len = (int)strlen(key);
            const char* q = dat_start;
            while (q < dat_end) {
                /* Skip whitespace */
                while (q < dat_end && (*q == ' ' || *q == '\t')) q++;
                if (q + key_len >= dat_end) break;
                
                /* Check for KEY= (case-insensitive) */
                if (_strnicmp(q, key, key_len) == 0 && q[key_len] == '=') {
                    const char* val_start = q + key_len + 1;
                    /* Skip leading quotes */
                    if (*val_start == '"') val_start++;
                    
                    /* Find end of value: comma, closing quote, or end of block */
                    const char* val_end = val_start;
                    while (val_end < dat_end && *val_end != ',' && *val_end != '"') val_end++;
                    
                    int len = (int)(val_end - val_start);
                    if (len > 0 && len < out_size) {
                        memcpy(out_buf, val_start, len);
                        out_buf[len] = '\0';
                        /* Trim trailing whitespace */
                        while (len > 0 && (out_buf[len-1] == ' ' || out_buf[len-1] == '\t')) {
                            out_buf[--len] = '\0';
                        }
                        return 1;
                    }
                    return 0;
                }
                /* Skip to next comma */
                while (q < dat_end && *q != ',') q++;
                if (q < dat_end) q++; /* skip comma */
            }
            return 0;
        }
        p++;
    }
    return 0;
}
static int extract_tag(const char* name, const char* tag_name, char* out_buf, int out_size) {
    int tag_len = (int)strlen(tag_name);
    const char* p = name;
    while (*p) {
        /* Check for <TAGNAME> (case-insensitive) */
        if (p[0] == '<' && _strnicmp(p + 1, tag_name, tag_len) == 0 && p[1 + tag_len] == '>') {
            const char* val_start = p + 1 + tag_len + 1;
            /* Find </TAGNAME> (case-insensitive) */
            const char* q = val_start;
            while (*q) {
                if (q[0] == '<' && q[1] == '/' &&
                    _strnicmp(q + 2, tag_name, tag_len) == 0 && q[2 + tag_len] == '>') {
                    int len = (int)(q - val_start);
                    if (len > 0 && len < out_size) {
                        memcpy(out_buf, val_start, len);
                        out_buf[len] = '\0';
                        return 1;
                    }
                    return 0;
                }
                q++;
            }
            /* No closing tag found — extract up to next '<' or end of string.
             * This handles malformed MESHWORLD entries like <SPEEDMULT>4 (missing </SPEEDMULT>). */
            {
                const char* end = val_start;
                while (*end && *end != '<') end++;
                int len = (int)(end - val_start);
                if (len > 0 && len < out_size) {
                    memcpy(out_buf, val_start, len);
                    out_buf[len] = '\0';
                    return 1;
                }
            }
            return 0; /* opening tag found but no closing tag */
        }
        p++;
    }
    return 0; /* no tag found */
}

/* Process <MESH> and <SPEEDMULT> tags: scan MESHWORLD section 3 for BADBALL
 * objects with custom tags, match to spawned balls by home position, apply. */
static void process_custom_tags(DWORD board, FILE* logf) {
    if (!board) return;

    if (logf) {
        fprintf(logf, "  TAGS: process_custom_tags called (board=0x%08X)\n", board);
        fflush(logf);
    }

    /* Get the bad balls list (AthenaList at board+0x29D4) */
    DWORD* bad_balls_list = (DWORD*)(board + BOARD_BAD_BALLS_LIST);
    if (IsBadReadPtr(bad_balls_list, 8)) {
        if (logf) fprintf(logf, "  TAGS: bad_balls_list at 0x%08X is bad read\n", (DWORD)bad_balls_list);
        return;
    }
    int ball_count = *(int*)(bad_balls_list + 1);  /* count at +0x04 */
    if (logf) fprintf(logf, "  TAGS: ball_count=%d\n", ball_count);
    if (ball_count <= 0 || ball_count > 100) return;

    /* Get ball pointers from the AthenaList items array.
     * AthenaList stores the heap-allocated items pointer at +0x40C
     * (verified via AthenaList_GetAt at 0x0040a020 and AthenaList_Append
     * at 0x00453780 — both use *(this+0x40C) + index*4). */
    DWORD* ball_items = *(DWORD**)((BYTE*)bad_balls_list + 0x40C);
    if (!ball_items || IsBadReadPtr(ball_items, ball_count * 4)) return;

    /* Get the sceneobj to access section 3 objects */
    DWORD sceneobj = get_sceneobj(board);
    if (!sceneobj) {
        if (logf) fprintf(logf, "  TAGS: sceneobj=NULL, skipping tag processing\n");
        return;
    }

    /* Read section 3 object count and array */
    if (IsBadReadPtr((void*)(sceneobj + SCENEOBJ_OBJ_COUNT), 4)) return;
    int obj_count = *(int*)(sceneobj + SCENEOBJ_OBJ_COUNT);
    if (obj_count <= 0 || obj_count > 1000) return;

    if (IsBadReadPtr((void*)(sceneobj + SCENEOBJ_OBJ_ARRAY), 4)) return;
    DWORD* obj_array_ptr = *(DWORD**)(sceneobj + SCENEOBJ_OBJ_ARRAY);
    if (!obj_array_ptr || IsBadReadPtr(obj_array_ptr, obj_count * 4)) return;

    int mesh_changes = 0;
    int speed_changes = 0;
    int i, j;

    /* For each section 3 object */
    for (i = 0; i < obj_count; i++) {
        DWORD obj_ptr = obj_array_ptr[i];
        if (!obj_ptr || obj_ptr < 0x10000) continue;
        if (IsBadReadPtr((void*)obj_ptr, 20)) continue;

        /* Read object name pointer */
        char* name = *(char**)(obj_ptr);
        if (!name || IsBadReadPtr(name, 8)) continue;

        /* Check if this is a BADBALL object */
        if (_strnicmp(name, "BADBALL", 7) != 0) continue;

        /* Check for <MESH> tag — DISABLED v20 (will re-enable later) */
        char mesh_value[64] = {0};
        /* int has_mesh = extract_tag(name, "MESH", mesh_value, sizeof(mesh_value)); */
        int has_mesh = 0;  /* MESH tag inactive */

        /* Check for <SPEEDMULT> tag */
        char speed_value[64] = {0};
        int has_speed = extract_tag(name, "SPEEDMULT", speed_value, sizeof(speed_value));

        if (!has_mesh && !has_speed) continue;

        /* Read object position (x, y, z at obj+0x04, +0x08, +0x0C) */
        float obj_x = *(float*)(obj_ptr + 0x04);
        float obj_y = *(float*)(obj_ptr + 0x08);
        float obj_z = *(float*)(obj_ptr + 0x0C);

        if (logf) {
            fprintf(logf, "  TAGS: BADBALL obj[%d] at (%.1f, %.1f, %.1f)", i, obj_x, obj_y, obj_z);
            if (has_mesh) fprintf(logf, " <MESH>(inactive)");
            if (has_speed) fprintf(logf, " <SPEEDMULT>%s</SPEEDMULT>", speed_value);
            fprintf(logf, "\n");
        }

        /* Match to a spawned ball by home position */
        for (j = 0; j < ball_count; j++) {
            DWORD ball = ball_items[j];
            if (!ball || ball < 0x10000) continue;
            if (IsBadReadPtr((void*)ball, 0xC70)) continue;

            float ball_home_x = *(float*)(ball + BALL_HOME_POS_X);
            float ball_home_y = *(float*)(ball + BALL_HOME_POS_Y);
            float ball_home_z = *(float*)(ball + BALL_HOME_POS_Z);

            /* Match by position (exact float comparison — CreateBadBall copies obj.xyz directly) */
            if (ball_home_x == obj_x && ball_home_y == obj_y && ball_home_z == obj_z) {
                /* Apply <MESH> tag — DISABLED v20 */
                if (0 && has_mesh) {
                    /* Use g_App (0x5341E0) directly — ball+0x10 may not be
                     * initialized yet when process_custom_tags runs.
                     * Ball_Render reads ball+0x10 as App, but it's set by
                     * the game's per-frame loop, not during CreateBadBalls. */
                    DWORD app = *(DWORD*)GLOBAL_APP_PTR;
                    if (app && !IsBadReadPtr((void*)app, 0x280)) {
                        int target_slot = MESH_SLOT_8BALL;  /* default */
                        DWORD src_mesh = 0;

                        if (ci_strstr(mesh_value, "funball")) {
                            target_slot = MESH_SLOT_FUNBALL;
                            src_mesh = *(DWORD*)(app + APP_MESH_FUNBALL);
                        } else {
                            /* Default: 8Ball */
                            src_mesh = *(DWORD*)(app + APP_MESH_8BALL);
                        }

                        if (src_mesh && !IsBadReadPtr((void*)src_mesh, 4)) {
                            /* Copy the 8Ball/FunBall mesh pointer into the target slot
                             * (overwriting SphereBreak1/2, which badballs never use).
                             * Ball_Render checks ball+0x754 < 3, so only slots 0-2 work. */
                            *(DWORD*)(app + APP_MESH_ARRAY + target_slot * 4) = src_mesh;
                            *(int*)(ball + BALL_MESH_INDEX_FIELD) = target_slot;
                            if (logf) fprintf(logf, "  TAGS: ball 0x%08X → mesh slot %d (mesh=0x%08X from App+0x%X)\n",
                                    ball, target_slot, src_mesh,
                                    ci_strstr(mesh_value, "funball") ? APP_MESH_FUNBALL : APP_MESH_8BALL);
                            mesh_changes++;
                        } else if (logf) {
                            fprintf(logf, "  TAGS: ball 0x%08X → mesh src invalid (App+0x%X=0x%08X)\n",
                                    ball, ci_strstr(mesh_value, "funball") ? APP_MESH_FUNBALL : APP_MESH_8BALL, src_mesh);
                        }
                    } else if (logf) {
                        fprintf(logf, "  TAGS: ball 0x%08X → App ptr invalid (0x%08X)\n", ball, app);
                    }
                }

                /* Apply <SPEEDMULT> tag — multiply current max_speed by value */
                if (has_speed) {
                    float mult = (float)atof(speed_value);
                    if (mult > 0.0f && mult <= 100.0f) {
                        float cur_speed = *(float*)(ball + BALL_MAX_SPEED);
                        *(float*)(ball + BALL_MAX_SPEED) = cur_speed * mult;
                        if (logf) fprintf(logf, "  TAGS: ball 0x%08X → max_speed %.1f × %.1f = %.1f\n",
                                ball, cur_speed, mult, cur_speed * mult);
                        speed_changes++;
                    } else if (logf) {
                        fprintf(logf, "  TAGS: ball 0x%08X → SPEEDMULT %.1f ignored (out of range 0.01-100.0)\n",
                                ball, mult);
                    }
                }
                break;
            }
        }
    }

    if (logf && (mesh_changes > 0 || speed_changes > 0)) {
        fprintf(logf, "  TAGS: applied %d mesh tag(s), %d speedmult tag(s)\n", mesh_changes, speed_changes);
        fflush(logf);
    }
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
 * REF:Rotater — Spawn Dizzy SWIRL at REF:Rotater positions in MESHWORLD
 *
 * Based on XRow's SpawnSpinSwirl CEA: loads Level3-Swirl mesh via
 * MeshWorld_ctor, creates Rotator_ctor_Impossible, registers in board lists.
 * ═══════════════════════════════════════════════════════════════════════════ */

static void spawn_rotater_at(DWORD board, float px, float py, float pz,
                              const char* mesh_path,
                              float rot_x, float rot_y, float rot_z,
                              float ros_x, float ros_y, float ros_z,
                              int ai_type,
                              FILE* logf) {
    if (!board) return;

    /* 1. Get gfx_device from App */
    DWORD app = *(DWORD*)(board + BOARD_APP);
    if (!app || IsBadReadPtr((void*)app, 4)) return;
    DWORD gfx_device = *(DWORD*)(app + APP_GFX_DEVICE);
    if (!gfx_device || IsBadReadPtr((void*)gfx_device, 4)) return;

    /* 2. Determine mesh path based on AI type */
    const char* path = NULL;
    if (ai_type >= 1 && ai_type <= 5) {
        /* AI 1-5: use game's built-in mesh path */
        path = g_ai_mesh_paths[ai_type];
    } else if (mesh_path && mesh_path[0]) {
        /* AI 0 or 6: use custom MESH property */
        path = mesh_path;
    } else {
        /* Default: SWIRL mesh */
        path = g_swirl_mesh_path;
    }

    /* Allocate a mutable copy of the path for MeshWorld_ctor */
    char path_buf[256];
    strncpy(path_buf, path, 255);
    path_buf[255] = 0;

    /* 3. Load mesh via MeshWorld_ctor */
    void* mesh = pfn_operator_new(MESHWORLD_SIZE);
    if (!mesh) {
        if (logf) fprintf(logf, "  ROTATER: failed to alloc mesh\n");
        return;
    }
    memset(mesh, 0, MESHWORLD_SIZE);

    void* loaded_mesh = pfn_MeshWorld_ctor(mesh, (void*)gfx_device, path_buf);
    if (!loaded_mesh) {
        if (logf) fprintf(logf, "  ROTATER: MeshWorld_ctor failed for '%s'\n", path_buf);
        return;
    }

    /* 4. Allocate object and call constructor based on AI type */
    void* obj = NULL;
    if (ai_type == 6) {
        /* AI 6: Rotator_ctor_Impossible (SWIRL with rotation) */
        obj = pfn_operator_new(ROTATER_SIZE);
        if (!obj) {
            if (logf) fprintf(logf, "  ROTATER: failed to alloc object\n");
            return;
        }
        memset(obj, 0, ROTATER_SIZE);
        void* result = pfn_Rotator_ctor(obj, (void*)board, px, py, pz, mesh);
        if (!result) {
            if (logf) fprintf(logf, "  ROTATER: Rotator_ctor failed\n");
            return;
        }
    } else {
        /* AI 0-5: Use the correct constructor per AI type.
         * Each constructor initializes the object with the correct vtable
         * and rotation axis (Pendulum=X, Rotator=Y, Looper=Z, etc.). */
        Rotator_ctor_t ctor_fn = NULL;
        DWORD alloc_sz = 0;
        
        switch (ai_type) {
            case 1:  ctor_fn = pfn_Rotator_ctor;  alloc_sz = ROTATER_SIZE;   break; /* Rotator (Y-axis) */
            case 2:  ctor_fn = pfn_Pendulum_ctor; alloc_sz = PENDULUM_SIZE;  break; /* Pendulum (X-axis) */
            case 3:  ctor_fn = pfn_Looper_ctor;   alloc_sz = LOOPER_SIZE;    break; /* Looper (Z-axis) */
            case 4:  ctor_fn = pfn_Gear_ctor;      alloc_sz = GEAR_SIZE;      break; /* Gear Small */
            case 5:  ctor_fn = pfn_Gear_ctor;      alloc_sz = GEAR_SIZE;      break; /* Gear Big */
            default: /* AI 0: static PopCylinder */
                obj = pfn_operator_new(POPCYLINDER_SIZE);
                if (!obj) { if (logf) fprintf(logf, "  ROTATER: failed to alloc\n"); return; }
                memset(obj, 0, POPCYLINDER_SIZE);
                void* result0 = pfn_PopCylinder_ctor(obj, (void*)board, px, py, pz, mesh);
                if (!result0) { if (logf) fprintf(logf, "  ROTATER: PopCylinder_ctor failed\n"); return; }
                goto spawn_done;
        }
        
        obj = pfn_operator_new(alloc_sz);
        if (!obj) { if (logf) fprintf(logf, "  ROTATER: failed to alloc (%d bytes)\n", alloc_sz); return; }
        memset(obj, 0, alloc_sz);
        void* result = ctor_fn(obj, (void*)board, px, py, pz, mesh);
        if (!result) { if (logf) fprintf(logf, "  ROTATER: ctor failed for AI %d\n", ai_type); return; }
    }
    spawn_done:;

    /* 5. Add to board+0x2578 (update list) */
    pfn_AthenaList_Append((DWORD*)(board + BOARD_UPDATE_LIST), obj);

    /* 6. Add to board+0xCD4 (render list) */
    pfn_AthenaList_Append((DWORD*)(board + BOARD_RENDER_LIST), obj);

    /* 7. Add collision object to board+0x10EC */
    DWORD col_obj = *(DWORD*)((char*)obj + (ai_type >= 1 ? 0x10D4 : 0x10E0));
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
        fprintf(logf, "  ROTATER: spawned at (%.1f,%.1f,%.1f) obj=0x%08X mesh='%s' rot=(%.4f,%.4f,%.4f) oc=(%.1f,%.1f,%.1f)\n",
                px, py, pz, (DWORD)obj, path_buf, rot_x, rot_y, rot_z,
                ros_x, ros_y, ros_z);
        fflush(logf);
    }

    /* Track for despawn + per-frame rotation updates */
    if (g_rotater_count < MAX_ROTATERS) {
        g_rotater_cfg[g_rotater_count].obj = (DWORD)obj;
        g_rotater_cfg[g_rotater_count].rot_x = rot_x;
        g_rotater_cfg[g_rotater_count].rot_y = rot_y;
        g_rotater_cfg[g_rotater_count].rot_z = rot_z;
        g_rotater_cfg[g_rotater_count].ros_x = ros_x;
        g_rotater_cfg[g_rotater_count].ros_y = ros_y;
        g_rotater_cfg[g_rotater_count].ros_z = ros_z;
        g_rotater_cfg[g_rotater_count].angle_x = 0.0f;
        g_rotater_cfg[g_rotater_count].angle_y = 0.0f;
        g_rotater_cfg[g_rotater_count].angle_z = 0.0f;
        if (mesh_path && mesh_path[0]) {
            strncpy(g_rotater_cfg[g_rotater_count].mesh_path, mesh_path, 127);
            g_rotater_cfg[g_rotater_count].mesh_path[127] = 0;
        } else {
            g_rotater_cfg[g_rotater_count].mesh_path[0] = 0;
        }
        g_rotater_count++;
    }
}

/* Despawn all rotater objects — calls vtable[11] (RemoveAndFree) on each */
static void despawn_all_rotaters(DWORD board, FILE* logf) {
    int i;
    for (i = 0; i < g_rotater_count; i++) {
        DWORD obj = g_rotater_cfg[i].obj;
        if (!obj || obj < 0x10000) continue;
        if (IsBadReadPtr((void*)obj, 0x10D0)) continue;

        /* Call vtable[11] (offset 0x2C) = RemoveAndFree */
        DWORD vtable = *(DWORD*)obj;
        if (vtable && !IsBadReadPtr((void*)vtable, 0x30)) {
            DWORD remove_fn = *(DWORD*)(vtable + 0x2C);
            if (remove_fn && remove_fn > 0x400000) {
                typedef void (__thiscall *remove_t)(void* this_);
                ((remove_t)remove_fn)((void*)obj);
                if (logf) fprintf(logf, "  ROTATER: removed obj=0x%08X\n", obj);
            }
        }
    }
    g_rotater_count = 0;
}

/* Apply rotation direction and oscillation limits to spawned custom_obj objects.
 * Called once at spawn time. Writes ROT_Y to obj+0x10EC (direction field).
 *
 * The native render function (vtable[11] at 0x0043B330) does:
 *   new_angle = direction * 0.004 + angle
 *   obj[0x10E8] = new_angle   (stores back!)
 *   if new_angle > 2.0:  direction = -1.0  (reverse)
 *   if new_angle < -2.0: direction = +1.0  (reverse)
 *
 * ROT_Y is written to the direction field as a speed multiplier.
 * ROS_Y (oscillation range) is stored in config — the native render
 * uses hardcoded ±2.0, so per-object OC requires a per-frame hook to
 * override the direction flip when the custom OC limit is reached.
 * For now, ROS_Y is stored but the native ±2.0 limit applies. */
static void apply_rotater_directions(void) {
    int i;
    for (i = 0; i < g_rotater_count; i++) {
        DWORD obj = g_rotater_cfg[i].obj;
        if (!obj || obj < 0x10000) continue;
        if (IsBadReadPtr((void*)obj, 0x10F0)) continue;

        /* Write ROT_Y to the direction field — native render uses it as multiplier */
        *(float*)(obj + 0x10EC) = g_rotater_cfg[i].rot_y;
    }
}

/* Per-frame override for objects with ROS_Y=0 (constant rotation).
 * The native render flips direction at ±2.0 radians. For ROS_Y=0,
 * we rewrite ROT_Y to the direction field every frame to prevent
 * the oscillation reversal, keeping rotation constant. */
static void update_constant_rotations(void) {
    int i;
    for (i = 0; i < g_rotater_count; i++) {
        if (g_rotater_cfg[i].ros_y != 0.0f) continue;  /* only for ROS_Y=0 */
        DWORD obj = g_rotater_cfg[i].obj;
        if (!obj || obj < 0x10000) continue;
        if (IsBadReadPtr((void*)obj, 0x10F0)) continue;
        *(float*)(obj + 0x10EC) = g_rotater_cfg[i].rot_y;
    }
}

/* Scan S1 ref points for Rotater entries with custom rot tags.
 * For each found, search the board's update list for the natively-spawned
 * Rotator object at the matching position, and apply ROT_Y to its direction
 * field (+0x10EC). This does NOT spawn — native game already spawned from S1. */
static void apply_s1_rotater_tags(DWORD board, FILE* logf) {
    if (!board) return;
    DWORD sceneobj = get_sceneobj(board);
    if (!sceneobj) return;

    /* Read S1 ref points */
    DWORD s1_list = sceneobj + 0x894;
    if (IsBadReadPtr((void*)(s1_list + 0x04), 4)) return;
    int s1_count = *(int*)(s1_list + 0x04);
    if (s1_count <= 0 || s1_count > 1000) return;
    if (IsBadReadPtr((void*)(s1_list + 0x40C), 4)) return;
    DWORD* s1_data = *(DWORD**)(s1_list + 0x40C);
    if (!s1_data || IsBadReadPtr(s1_data, s1_count * 4)) return;

    /* Read board update list (AthenaList at board+0x2578) */
    DWORD update_list = board + 0x2578;
    if (IsBadReadPtr((void*)(update_list + 0x04), 4)) return;
    int update_count = *(int*)(update_list + 0x04);
    if (update_count <= 0 || update_count > 10000) return;
    if (IsBadReadPtr((void*)(update_list + 0x40C), 4)) return;
    DWORD* update_data = *(DWORD**)(update_list + 0x40C);
    if (!update_data || IsBadReadPtr(update_data, update_count * 4)) return;

    /* Known Rotator vtable addresses */
    const DWORD rotator_vtables[] = { 0x004D5518, 0x004D5708, 0 };

    int i, j;
    for (i = 0; i < s1_count; i++) {
        DWORD entry = s1_data[i];
        if (!entry || entry < 0x10000) continue;
        if (IsBadReadPtr((void*)entry, 16)) continue;

        char* name = *(char**)(entry);
        if (!name || IsBadReadPtr(name, 8)) continue;
        if (_strnicmp(name, "cEnt", 4) != 0 &&
            _strnicmp(name, "REF:cEnt", 8) != 0 &&
            _strnicmp(name, "C_entity", 8) != 0 &&
            _strnicmp(name, "REF:C_entity", 13) != 0) continue;

        /* Parse rotation tags from <DAT> block */
        char rot_y_str[32] = {0};
        char ros_y_str[32] = {0};
        extract_dat_prop(name, "ROT_Y", rot_y_str, sizeof(rot_y_str));
        extract_dat_prop(name, "ROS_Y", ros_y_str, sizeof(ros_y_str));
        if (!rot_y_str[0]) continue;  /* skip if no ROT_Y tag */

        float rot_y = (float)atof(rot_y_str);
        float ros_y = ros_y_str[0] ? (float)fabs(atof(ros_y_str)) : 2.0f;

        /* Get S1 ref point position */
        float px = *(float*)(entry + 0x04);
        float py = *(float*)(entry + 0x08);
        float pz = *(float*)(entry + 0x0C);

        /* Search update list for a Rotator at this position */
        for (j = 0; j < update_count; j++) {
            DWORD obj = update_data[j];
            if (!obj || obj < 0x10000) continue;
            if (IsBadReadPtr((void*)obj, 0x10F0)) continue;

            DWORD vtable = *(DWORD*)obj;
            int is_rotator = 0;
            int k;
            for (k = 0; rotator_vtables[k]; k++) {
                if (vtable == rotator_vtables[k]) { is_rotator = 1; break; }
            }
            if (!is_rotator) continue;

            /* Check position match (within 2.0 units) */
            float ox = *(float*)(obj + 0x10D4);
            float oy = *(float*)(obj + 0x10D8);
            float oz = *(float*)(obj + 0x10DC);
            float dx = ox - px; if (dx < 0) dx = -dx;
            float dy = oy - py; if (dy < 0) dy = -dy;
            float dz = oz - pz; if (dz < 0) dz = -dz;
            if (dx < 2.0f && dy < 2.0f && dz < 2.0f) {
                /* Found it! Write ROT_Y to direction field */
                *(float*)(obj + 0x10EC) = rot_y;

                /* If ROS_Y=0, store for per-frame direction override
                 * (native render flips direction at ±2.0, we need to
                 * continuously rewrite it to prevent oscillation) */
                if (ros_y == 0.0f && g_rotater_count < MAX_ROTATERS) {
                    g_rotater_cfg[g_rotater_count].obj = obj;
                    g_rotater_cfg[g_rotater_count].rot_y = rot_y;
                    g_rotater_cfg[g_rotater_count].ros_y = 0.0f;
                    g_rotater_cfg[g_rotater_count].angle_y = 0.0f;
                    g_rotater_cfg[g_rotater_count].mesh_path[0] = 0;
                    g_rotater_count++;
                }

                if (logf) {
                    fprintf(logf, "  ROTATER(S1-tag): obj=0x%08X at (%.1f,%.1f,%.1f) ROT_Y=%.4f ROS_Y=%.1f applied\n",
                            obj, px, py, pz, rot_y, ros_y);
                    fflush(logf);
                }
                break;
            }
        }
    }
}

/* Hide original meshbuffers that match C_entity names.
 * The level loader creates meshbuffers for ALL section-3 entries, including
 * C_entity_001. These render as part of the level geometry. We need to hide
 * them so only our custom-spawned Rotator (with the MESH property) is visible.
 *
 * Approach: Zero the render context for matching meshbuffers.
 * The render context array is at MeshWorld+0x28, each entry is 0x50 bytes.
 * The meshbuffer list is at MeshWorld+0x2C (AthenaList).
 * We match by name (at MeshBuffer+0x864) and zero the corresponding
 * render context to make it invisible. */
static void hide_entity_meshbuffers(DWORD board, FILE* logf) {
    if (!board) return;
    DWORD level = get_level(board);
    if (!level) return;
    if (IsBadReadPtr((void*)(level + 0x08), 4)) return;
    DWORD meshworld = *(DWORD*)(level + 0x08);
    if (!meshworld || IsBadReadPtr((void*)meshworld, 0x30)) return;

    /* MeshBuffer AthenaList at MeshWorld+0x2C */
    DWORD* mb_list = (DWORD*)(meshworld + 0x2C);
    if (IsBadReadPtr(mb_list, 8)) return;
    int mb_count = *(int*)(mb_list + 1);  /* count at +0x04 */
    if (mb_count <= 0 || mb_count > 10000) return;
    if (IsBadReadPtr((void*)((BYTE*)mb_list + 0x40C), 4)) return;
    DWORD* mb_data = *(DWORD**)((BYTE*)mb_list + 0x40C);
    if (!mb_data || IsBadReadPtr(mb_data, mb_count * 4)) return;

    /* Render context array at MeshWorld+0x28 */
    DWORD rc_array = *(DWORD*)(meshworld + 0x28);
    if (!rc_array || IsBadReadPtr((void*)rc_array, mb_count * 0x50)) {
        /* rc_array might be inline, not a pointer */
        rc_array = meshworld + 0x28;
        if (IsBadReadPtr((void*)rc_array, mb_count * 0x50)) return;
    }

    int hidden = 0;
    int i;
    for (i = 0; i < mb_count; i++) {
        DWORD mb = mb_data[i];
        if (!mb || mb < 0x10000) continue;
        if (IsBadReadPtr((void*)mb, 0x870)) continue;

        /* Read MeshBuffer name at +0x864 */
        char* name = *(char**)(mb + 0x864);
        if (!name || IsBadReadPtr(name, 8)) continue;

        /* Check if name starts with "cEnt" or "REF:cEnt" (case-insensitive) */
        if (_strnicmp(name, "cEnt", 4) != 0 &&
            _strnicmp(name, "C_entity", 8) != 0) continue;

        /* Zero the render context for this meshbuffer (0x50 bytes) */
        DWORD rc_addr = rc_array + i * 0x50;
        if (!IsBadReadPtr((void*)rc_addr, 0x50)) {
            memset((void*)rc_addr, 0, 0x50);
            hidden++;
            if (logf) fprintf(logf, "  HIDE: meshbuffer '%s' (idx=%d, mb=0x%08X) render context zeroed\n", name, i, mb);
        }
    }

    if (logf && hidden > 0) {
        fprintf(logf, "  HIDE: hidden %d original meshbuffer(s)\n", hidden);
        fflush(logf);
    }
}

/* Scan section 3 for REF:Rotater entries and spawn SWIRL at each position.
 * NOTE: Do NOT scan S1 ref points — the game's native vtable[33] handler
 * already spawns Rotators from S1 entries named "Rotater". Scanning S1 would
 * create duplicate objects (double SWIRL). Only section-3 entries are ours. */
static void process_rotaters(DWORD board, FILE* logf) {
    if (!board) return;

    DWORD sceneobj = get_sceneobj(board);
    if (!sceneobj) {
        if (logf) fprintf(logf, "  ROTATER: sceneobj=NULL\n");
        return;
    }

    /* === Scan section 3 objects (sceneobj+0xCA0) for Rotater entries === */
    if (IsBadReadPtr((void*)(sceneobj + SCENEOBJ_OBJ_COUNT), 4)) return;
    int obj_count = *(int*)(sceneobj + SCENEOBJ_OBJ_COUNT);
    if (obj_count <= 0 || obj_count > 1000) return;

    if (IsBadReadPtr((void*)(sceneobj + SCENEOBJ_OBJ_ARRAY), 4)) return;
    DWORD* obj_array_ptr = *(DWORD**)(sceneobj + SCENEOBJ_OBJ_ARRAY);
    if (!obj_array_ptr || IsBadReadPtr(obj_array_ptr, obj_count * 4)) return;

    int found = 0;
    int i;
    for (i = 0; i < obj_count; i++) {
        DWORD obj_ptr = obj_array_ptr[i];
        if (!obj_ptr || obj_ptr < 0x10000) continue;
        if (IsBadReadPtr((void*)obj_ptr, 20)) continue;

        /* Read object name pointer */
        char* name = *(char**)(obj_ptr);
        if (!name || IsBadReadPtr(name, 8)) continue;

        /* Check for Rotater (case-insensitive, prefix match — name may include tags) */
        if (_strnicmp(name, "REF:cEnt", 8) == 0  /* REF:custom_obj or REF:custom_obj_NNN */) {
            /* Full match "REF:Rotater" — tags start at name+11 */
        } else if (_strnicmp(name, "cEnt", 4) == 0  /* custom_obj or custom_obj_NNN */) {
            /* Plain "Rotater" — tags start at name+7 */
        } else {
            continue;
        }

        /* Read position (x, y, z at obj+0x04, +0x08, +0x0C) */
        float px = *(float*)(obj_ptr + 0x04);
        float py = *(float*)(obj_ptr + 0x08);
        float pz = *(float*)(obj_ptr + 0x0C);

        /* Parse tags from the object name */
        char mesh_path[128] = {0};
        char rot_x_str[32] = {0};
        char rot_y_str[32] = {0};
        char rot_z_str[32] = {0};
        char ros_x_str[32] = {0};
        char ros_y_str[32] = {0};
        char ros_z_str[32] = {0};
        char ai_str[32] = {0};

        extract_dat_prop(name, "MESH", mesh_path, sizeof(mesh_path));

        /* Normalize mesh path: strip quotes, replace forward slashes with backslashes */
        if (mesh_path[0]) {
            /* Strip leading/trailing double-quotes */
            char* p = mesh_path;
            while (*p == '"') p++;
            size_t len = strlen(p);
            while (len > 0 && p[len-1] == '"') { p[--len] = 0; }
            /* Move stripped path to front if needed */
            if (p != mesh_path) memmove(mesh_path, p, len + 1);
            /* Replace forward slashes with backslashes */
            for (p = mesh_path; *p; p++) {
                if (*p == '/') *p = '\\';
            }
            /* If path has no directory separator, prepend "levels\" */
            if (!strchr(mesh_path, '\\') && !strchr(mesh_path, ':')) {
                char tmp[128];
                snprintf(tmp, sizeof(tmp), "levels\\%s", mesh_path);
                strncpy(mesh_path, tmp, 127);
                mesh_path[127] = 0;
            }
        }
        extract_dat_prop(name, "ROT_X", rot_x_str, sizeof(rot_x_str));
        extract_dat_prop(name, "ROT_Y", rot_y_str, sizeof(rot_y_str));
        extract_dat_prop(name, "ROT_Z", rot_z_str, sizeof(rot_z_str));
        extract_dat_prop(name, "ROS_X", ros_x_str, sizeof(ros_x_str));
        extract_dat_prop(name, "ROS_Y", ros_y_str, sizeof(ros_y_str));
        extract_dat_prop(name, "ROS_Z", ros_z_str, sizeof(ros_z_str));
        extract_dat_prop(name, "AI", ai_str, sizeof(ai_str));

        /* Parse rotation speeds — default to native SWIRL if not specified.
         * ROT_Y is a SPEED MULTIPLIER: 1.0 = native speed (0.004 rad/frame),
         * 4.0 = 4x speed, 0.5 = half speed, -1.0 = reverse. */
        float rot_x = rot_x_str[0] ? (float)atof(rot_x_str) : 0.0f;
        float rot_y = rot_y_str[0] ? (float)atof(rot_y_str) : 1.0f;  /* native SWIRL default */
        float rot_z = rot_z_str[0] ? (float)atof(rot_z_str) : 0.0f;

        /* Parse oscillation ranges — default 2.0 radians (native SWIRL behavior).
         * Uses absolute value so negative inputs are treated as positive.
         * ROS_Y=0.0 means constant rotation (no oscillation limit). */
        float ros_x = ros_x_str[0] ? (float)fabs(atof(ros_x_str)) : 2.0f;
        float ros_y = ros_y_str[0] ? (float)fabs(atof(ros_y_str)) : 2.0f;
        float ros_z = ros_z_str[0] ? (float)fabs(atof(ros_z_str)) : 2.0f;

        /* Parse AI type — default 6 (SWIRL) if not specified */
        int ai_type = ai_str[0] ? atoi(ai_str) : 6;
        if (ai_type < 0) ai_type = 0;
        if (ai_type > 6) ai_type = 6;

        if (logf) {
            fprintf(logf, "  ROTATER: found '%s' at (%.1f, %.1f, %.1f)\n", name, px, py, pz);
            fprintf(logf, "    MESH='%s' AI=%d ROT_X=%.4f ROT_Y=%.4f ROT_Z=%.4f ROS=(%.1f, %.1f, %.1f)\n",
                    mesh_path[0] ? mesh_path : "(default)", ai_type, rot_x, rot_y, rot_z,
                    ros_x, ros_y, ros_z);
        }

        spawn_rotater_at(board, px, py, pz, mesh_path,
                         rot_x, rot_y, rot_z,
                         ros_x, ros_y, ros_z,
                         ai_type,
                         logf);
        found++;
    }

    if (logf && found > 0) {
        fprintf(logf, "  ROTATER: spawned %d SWIRL object(s)\n", found);
        fflush(logf);
    }
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

/* Load config from custom_entities.txt */
static void load_config(void) {
    char config_path[MAX_PATH];
    snprintf(config_path, MAX_PATH, "%s\\custom_entities.txt", g_game_dir);
    FILE* f = NULL;
    fopen_s(&f, config_path, "r");
    if (!f) return;

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        /* Skip comments and blank lines */
        char* p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '#' || *p == '\n' || *p == '\r' || *p == '\0') continue;

        /* Parse grid_speed */
        if (strncmp(p, "grid_speed", 10) == 0) {
            char* eq = strchr(p, '=');
            if (eq) {
                float val = (float)atof(eq + 1);
                if (val > 0.0f) g_grid_speed = val;
            }
        }
    }
    fclose(f);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Main thread — GRID cycling: spawn current GRID, despawn all others
 * ═══════════════════════════════════════════════════════════════════════════ */

static DWORD WINAPI entity_thread(LPVOID param) {
    init_game_dir();
    load_config();

    /* Open log file */
    char log_path[MAX_PATH];
    snprintf(log_path, MAX_PATH, "%s\\custom_entities.log", g_game_dir);
    FILE* logf = NULL;
    fopen_s(&logf, log_path, "a");
    if (logf) {
        fprintf(logf, "=== Custom Entities Mod v43 Started ===\n");
        fprintf(logf, "Game dir: %s\n", g_game_dir);
        fprintf(logf, "Mesh path: %s\n", g_mesh_path);
        fprintf(logf, "Grid speed: %.1f seconds\n", g_grid_speed);
        fclose(logf);
    }

    /* Wait for game to fully load */
    Sleep(3000);

    while (g_running) {
        /* Per-frame: override direction for ROS_Y=0 objects to prevent oscillation */
        update_constant_rotations();

        /* Small sleep to avoid hogging CPU */
        Sleep(16);  /* ~60fps */

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

        /* Process <MESH> and <SPEEDMULT> tags on spawned 8-balls (after CreateBadBall has run) */
        process_custom_tags(board, logf);

        /* Process C_entity entries — spawn custom objects at each position */
        process_rotaters(board, logf);

        /* Hide original meshbuffers for C_entity entries */
        hide_entity_meshbuffers(board, logf);

        /* Apply custom rotation directions to spawned rotaters */
        apply_rotater_directions();

        /* Apply S1 rot tags to natively-spawned Rotators */
        apply_s1_rotater_tags(board, logf);

        /* Find GRID reference points */
        float grid_x[32], grid_y[32], grid_z[32];
        int grid_count = find_grid_points(board, grid_x, grid_y, grid_z, 32, logf);

        if (grid_count > 0) {
            g_spawned_board = board;
            if (logf) fprintf(logf, "  Found %d GRID points, starting cycle (speed=%.1fs)\n", grid_count, g_grid_speed);

            /* Start with GRID01 visible */
            int current_grid = 1;
            spawn_testcube_at(board, grid_x[0], grid_y[0], grid_z[0], 1, logf);
            if (logf) {
                fprintf(logf, "  Cycle: GRID01 spawned\n");
                fflush(logf);
            }

            /* Cycling loop — keep log file open for entire cycle */
            while (g_running && board == get_board()) {
                /* Wait for grid_speed seconds (check every 100ms for board change) */
                int wait_ms = (int)(g_grid_speed * 1000);
                if (wait_ms < 100) wait_ms = 100;
                int waited = 0;
                while (waited < wait_ms) {
                    Sleep(100);
                    waited += 100;
                    if (!g_running || board != get_board()) break;
                }
                if (!g_running || board != get_board()) break;

                /* Advance to next GRID (1→2→3→4→5→1→...) */
                current_grid++;
                if (current_grid > grid_count) current_grid = 1;

                /* Despawn all spawned objects */
                while (g_spawned_count > 0) {
                    despawn_object(board, g_spawned_objs[0], logf);
                    int j;
                    for (j = 0; j < g_spawned_count - 1; j++) {
                        g_spawned_objs[j] = g_spawned_objs[j + 1];
                        strcpy(g_spawned_names[j], g_spawned_names[j + 1]);
                    }
                    g_spawned_count--;
                }

                /* Spawn the current GRID */
                int idx = current_grid - 1;
                spawn_testcube_at(board, grid_x[idx], grid_y[idx], grid_z[idx], current_grid, logf);
                if (logf) {
                    fprintf(logf, "  Cycle: GRID%02d spawned\n", current_grid);
                    fflush(logf);
                }
            }

            /* Clean up any remaining spawned objects on level exit */
            while (g_spawned_count > 0) {
                despawn_object(board, g_spawned_objs[0], logf);
                int j;
                for (j = 0; j < g_spawned_count - 1; j++) {
                    g_spawned_objs[j] = g_spawned_objs[j + 1];
                    strcpy(g_spawned_names[j], g_spawned_names[j + 1]);
                }
                g_spawned_count--;
            }

            /* Despawn all rotater objects on level exit */
            despawn_all_rotaters(board, logf);
        } else {
            /* No GRID points — still mark board as processed */
            g_spawned_board = board;
            /* Still process rotaters even without GRID points */
            despawn_all_rotaters(board, logf);
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
        load_real_bass();
        g_thread = CreateThread(NULL, 0, entity_thread, NULL, 0, NULL);
    } else if (reason == DLL_PROCESS_DETACH) {
        g_running = 0;
    }
    return TRUE;
}
