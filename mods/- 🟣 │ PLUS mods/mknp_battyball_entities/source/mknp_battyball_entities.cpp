/*
 * mknp_battyball_entities.cpp — Battyball Entities (HB+ v2.1, MinGW build)
 *
 * First of the "battyball" series of HB+ mods. This mod recreates the GRID
 * object system from mknp_custom_entities (bass.dll proxy) from scratch, as
 * a standalone HB+ mod.
 *
 * WHAT THIS MOD DOES
 * ------------------
 * Scans the active level's S1 reference points for names containing "GRID".
 * Each GRID ref point is a position where a "testcube" mesh is spawned.
 * Only ONE GRID cube is visible at a time; the visible one cycles from
 * GRID01 -> GRID02 -> ... -> GRIDnn -> GRID01 -> ... every `grid_speed`
 * seconds. When the cycle advances, the previous cube is despawned (removed
 * from update/render/collision lists, collision object destroyed) and the
 * next is spawned.
 *
 * Uses the SAME native spawning mechanics as mknp_custom_entities:
 *   - operator_new (0x4BA57B) to allocate
 *   - MeshWorld_ctor (0x461510) to load levels\testcube.MESHWORLD
 *   - PopCylinder_ctor (0x436EE0) to create the object
 *   - AthenaList_Append/Remove (0x453810 / 0x4534D0) to register/unregister
 *
 * Differences from the bass mod:
 *   - Runs on the HB+ callback loop (onGameUpdate) instead of a background
 *     thread, using GetTickCount() timing instead of Sleep().
 *   - Uses HB+ GetScene()/GetApp() instead of the g_Scene global.
 *   - Config via HB+ slider (GRID speed in seconds) instead of a .txt file.
 *
 * Author: MAKYUNI / Hamsterbot
 */

#include "nocrt.h"
#include "HamsterballAPI.h"
#include "hbplus_api.h"

#include <windows.h>

#define malloc nc_malloc
#define free nc_free
#define memcpy nc_memcpy
#define memset nc_memset
#define strlen nc_strlen
#define strcmp nc_strcmp
#define strncmp nc_strncmp
#define memcmp nc_memcmp
#define strncpy nc_strncpy
#define snprintf nc_snprintf

#include "gridmesh.h"
#include "gridset.h"

/* ═══════════════════════════════════════════════════════════════════════════
 * Game addresses + offsets (verified against mknp_custom_entities / Hamsterball.exe)
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Game functions (addresses are absolute entry points in Hamsterball.exe) */
#define OPERATOR_NEW        0x004BA57B   /* __cdecl(size) */
#define MESHWORLD_CTOR      0x00461510   /* __thiscall(this, gfx_device, path) loads .MESHWORLD */
#define POPCYLINDER_CTOR    0x00436EE0   /* __thiscall(this, board, x, y, z, mesh) */
#define ATHENALIST_APPEND   0x00453810   /* __thiscall(list, item) */
#define ATHENALIST_REMOVE   0x004534D0   /* __thiscall(list, item_value) */

/* Object sizes */
#define MESHWORLD_SIZE      0x10D0
#define POPCYLINDER_SIZE    0x10D0

/* Board (Scene) offsets — HB+ GetScene() returns the 0x4400-byte Board/Scene */
#define BOARD_LEVEL         0x8AC    /* level ptr */
#define BOARD_UPDATE_LIST   0x2578   /* update list (Scene iterates this) */
#define BOARD_RENDER_LIST   0xCD4    /* render list */
#define BOARD_COLLISION_LIST 0x10EC  /* collision objects list */
#define BOARD_SCENE_OBJ     0x8B0    /* SceneObject chain */
#define BOARD_SCENE_UPDATE_LIST 0x8B8
#define BOARD_PAUSED        0x874    /* nonzero while game paused */

/* Level offsets */
#define LEVEL_SCENEOBJECT   0x480    /* SceneObject ptr */

/* SceneObject S1 ref-point list */
#define SCENEOBJ_S1_OFF     0x894    /* embedded AthenaList: count at +0x04, items at +0x40C */
#define SCENEOBJ_OBJ_COUNT  0x898
#define SCENEOBJ_OBJ_ARRAY  0xCA0

/* App offsets */
#define APP_GFX_DEVICE      0x174    /* D3D device (needed by MeshWorld_ctor) */

/* PopCylinder collision object pointer (set by PopCylinder_ctor) */
#define PC_COLLISION_OBJ    0x10E0

/* S1 entry layout */
#define S1ENTRY_NAME        0x00
#define S1ENTRY_POS_X       0x04
#define S1ENTRY_POS_Y       0x08
#define S1ENTRY_POS_Z       0x0C

/* AthenaList embedded layout */
#define ALIST_COUNT         0x04
#define ALIST_ITEMS         0x40C

/* Config defaults */
#define GRID_SPEED_DEFAULT  3.0f
#define MAX_GRID_POINTS     32
#define MAX_SPAWNED         32   /* one preloaded slot per GRID point */

/* ═══════════════════════════════════════════════════════════════════════════
 * Native function typedefs
 * ═══════════════════════════════════════════════════════════════════════════ */
typedef void* (__cdecl *operator_new_t)(size_t size);
typedef void* (__thiscall *meshworld_ctor_t)(void* self, void* gfx_device, const char* path);
typedef void* (__thiscall *popcylinder_ctor_t)(void* self, void* board, float x, float y, float z, void* mesh);
typedef void  (__thiscall *athena_append_t)(void* list, void* item);
typedef void  (__thiscall *athena_remove_t)(void* list, int item_value);

static operator_new_t    g_op_new     = (operator_new_t)OPERATOR_NEW;
static meshworld_ctor_t  g_mw_ctor    = (meshworld_ctor_t)MESHWORLD_CTOR;
static popcylinder_ctor_t g_pc_ctor   = (popcylinder_ctor_t)POPCYLINDER_CTOR;
static athena_append_t   g_append     = (athena_append_t)ATHENALIST_APPEND;
static athena_remove_t   g_remove     = (athena_remove_t)ATHENALIST_REMOVE;

/* ═══════════════════════════════════════════════════════════════════════════
 * Mod state
 * ═══════════════════════════════════════════════════════════════════════════ */
static IModAPI* g_api     = NULL;
static bool     g_enabled = true;
static float    g_speed   = GRID_SPEED_DEFAULT;

/* Mesh path — testcube.MESHWORLD is copied to levels\ at init */
static char g_mesh_path[] = "levels\\testcube";

static HMODULE mod_self(void);

static int mesh_file_present(void) {
char full[MAX_PATH];
full[0] = '\0';
HMODULE self = mod_self();
if (self) GetModuleFileNameA(self, full, MAX_PATH);
if (!full[0]) GetModuleFileNameA(NULL, full, MAX_PATH);
/* game root = exe dir; levels\ under it */
char* slash = NULL;
for (char* p = full; *p; p++) if (*p == '\\') slash = p;
if (slash) {
    /* DLL may sit in Mods\: exe dir is one up from Mods, else same dir */
    *(slash + 1) = '\0';
    char cand[MAX_PATH];
    snprintf(cand, sizeof(cand), "%slevels\\testcube.MESHWORLD", full);
    if (GetFileAttributesA(cand) != INVALID_FILE_ATTRIBUTES) return 1;
    snprintf(cand, sizeof(cand), "%s..\\levels\\testcube.MESHWORLD", full);
    if (GetFileAttributesA(cand) != INVALID_FILE_ATTRIBUTES) return 1;
    return 0;
}
return GetFileAttributesA("levels\\testcube.MESHWORLD") !=
       INVALID_FILE_ATTRIBUTES;
}

/* Spawned object registry (for despawn) */
static DWORD g_spawned_objs[MAX_SPAWNED];
static char  g_spawned_names[MAX_SPAWNED][32];
static int   g_spawned_count = 0;
/* Cycle order: point indices that preloaded OK (skips mesh-less points) */
static int   g_order[MAX_GRID_POINTS];
static int   g_order_count = 0;

/* GRID points found in the current level */
static float g_pts_x[MAX_GRID_POINTS];
static float g_pts_y[MAX_GRID_POINTS];
static float g_pts_z[MAX_GRID_POINTS];
static char  g_grid_names[MAX_GRID_POINTS][32];   /* S1 ref name per point */
static char  g_grid_mesh[MAX_GRID_POINTS][MAX_PATH]; /* ctor path of own mesh */
static int   g_grid_own[MAX_GRID_POINTS];         /* 1 = own mesh extracted */
static int   g_grid_count = 0;
static int   g_scan_logged = 0;   /* 1 after first detailed scan dump */
static char  g_levels_dir[MAX_PATH];  /* game levels\ dir, trailing backslash */

/* Active level / board the current cycle belongs to */
static DWORD g_active_board = 0;
static int   g_board_ready_delay = 0;   /* frames to wait for level build after board change */

/* Time-based cycle state */
static DWORD g_last_switch_tick = 0;    /* GetTickCount() when current cube spawned */
static DWORD g_prev_tick = 0;           /* last frame tick (pause freeze math) */
static DWORD g_last_switch_log = 0;     /* last SWITCH line (log throttle) */
static int   g_current_grid = 1;        /* 1-based current GRID (1 = GRID01) */
static bool  g_cycle_started = false;

/* Own-module handle (for locating testcube.MESHWORLD next to the DLL) */
static HMODULE g_module = NULL;

/* ═══════════════════════════════════════════════════════════════════════════
 * Small helpers
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Case-sensitive substring search (like strstr) — GRID match uses this. */
static const char* nc_strstr(const char* hay, const char* needle) {
    if (!hay || !needle) return NULL;
    size_t nl = strlen(needle);
    if (nl == 0) return hay;
    const char* h = hay;
    while (*h) {
        if (*h == *needle && strncmp(h, needle, nl) == 0) return h;
        h++;
    }
    return NULL;
}

static DWORD player_board(void) {
    if (!g_api) return 0;
    Scene* scene = HBAPI(g_api).GetScene();
    if (!scene) return 0;
    return (DWORD)scene;
}

static HMODULE mod_self(void) {
    if (g_module) return g_module;
    MEMORY_BASIC_INFORMATION mbi;
    if (VirtualQuery((void*)mod_self, &mbi, sizeof(mbi)))
        g_module = (HMODULE)mbi.AllocationBase;
    return g_module;
}

static char g_log_path[MAX_PATH];   /* resolved once at init */

static void resolve_log_path(char* out) {
    out[0] = '\0';
    HMODULE self = mod_self();
    if (self) GetModuleFileNameA(self, out, MAX_PATH);
    if (!out[0]) GetModuleFileNameA(NULL, out, MAX_PATH);
    char* slash = NULL;
    for (char* p = out; *p; p++) if (*p == '\\') slash = p;
    if (slash) {
        strncpy(slash + 1, "mknp_battyball_entities.log",
                MAX_PATH - (slash + 1 - out) - 1);
    } else {
        strncpy(out, "mknp_battyball_entities.log", MAX_PATH - 1);
        out[MAX_PATH - 1] = '\0';
    }
}

static void log_mod(const char* msg) {
    char path[MAX_PATH];
    if (g_log_path[0]) {
        strncpy(path, g_log_path, MAX_PATH);
        path[MAX_PATH - 1] = '\0';
    } else {
        resolve_log_path(path);
    }
    HANDLE f = CreateFileA(path, GENERIC_WRITE, FILE_SHARE_READ, NULL,
                           OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (f == INVALID_HANDLE_VALUE) {
        /* Fallback: game dir via exe path */
        char exep[MAX_PATH];
        if (GetModuleFileNameA(NULL, exep, MAX_PATH) > 0) {
            char* s2 = NULL;
            for (char* p = exep; *p; p++) if (*p == '\\') s2 = p;
            if (s2) {
                strncpy(s2 + 1, "mknp_battyball_entities.log",
                        MAX_PATH - (s2 + 1 - exep) - 1);
                f = CreateFileA(exep, GENERIC_WRITE, FILE_SHARE_READ, NULL,
                                OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            }
        }
        if (f == INVALID_HANDLE_VALUE) return;
    }
    SetFilePointer(f, 0, NULL, FILE_END);
    DWORD wrote = 0;
    WriteFile(f, msg, (DWORD)strlen(msg), &wrote, NULL);
    WriteFile(f, "\r\n", 2, &wrote, NULL);
    CloseHandle(f);
}

static int list_count(DWORD list) {
    if (!list || IsBadReadPtr((void*)(list + ALIST_COUNT), 4)) return -1;
    return *(int*)(list + ALIST_COUNT);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * GRID ref-point scanning (mirrors cEnt_find_grid_points from mknp_custom_entities)
 * ═══════════════════════════════════════════════════════════════════════════ */
static DWORD get_level(DWORD board) {
    if (!board) return 0;
    if (IsBadReadPtr((void*)(board + BOARD_LEVEL), 4)) return 0;
    return *(DWORD*)(board + BOARD_LEVEL);
}
static DWORD get_sceneobj(DWORD board) {
    DWORD level = get_level(board);
    if (!level) return 0;
    if (IsBadReadPtr((void*)(level + LEVEL_SCENEOBJECT), 4)) return 0;
    return *(DWORD*)(level + LEVEL_SCENEOBJECT);
}

static int find_grid_points(DWORD board) {
    g_grid_count = 0;
    g_s1_hash = 2166136261u;
    g_s1_count = 0;
    DWORD sceneobj = get_sceneobj(board);
    if (!sceneobj) { log_mod("  GRID: sceneobj=NULL"); return 0; }

    DWORD level = get_level(board);
    char bname[32];
    bname[0] = '\0';
    if (!IsBadReadPtr((void*)(board + 0x29B4), 4)) {
        char* bn = *(char**)(board + 0x29B4);
        if (bn && !IsBadReadPtr(bn, 24)) {
            int bi = 0;
            while (bi < 31 && bn[bi]) { bname[bi] = bn[bi]; bi++; }
            bname[bi] = '\0';
        }
    }

    DWORD s1_list = sceneobj + SCENEOBJ_S1_OFF;
    if (IsBadReadPtr((void*)(s1_list + ALIST_COUNT), 4)) {
        log_mod("  GRID: s1_list unreadable");
        return 0;
    }
    int s1_count = *(int*)(s1_list + ALIST_COUNT);
    DWORD* s1_data = NULL;
    if (!IsBadReadPtr((void*)(s1_list + ALIST_ITEMS), 4))
        s1_data = *(DWORD**)(s1_list + ALIST_ITEMS);
    {
        char dbuf[128];
        snprintf(dbuf, sizeof(dbuf),
                 "  S1 board=0x%X lvl=0x%X sc=0x%X cnt=%d data=0x%X nm=%s",
                 board, level, sceneobj, s1_count, (DWORD)s1_data, bname);
        if (!g_scan_logged) log_mod(dbuf);
        g_scan_logged = 1;
    }
    if (s1_count <= 0 || s1_count > 1000) return 0;

    if (!s1_data || IsBadReadPtr(s1_data, s1_count * 4)) return 0;

    g_s1_count = s1_count;
    s1_feed_byte((unsigned char)(s1_count & 0xFF));
    s1_feed_byte((unsigned char)((s1_count >> 8) & 0xFF));
    s1_feed_byte((unsigned char)((s1_count >> 16) & 0xFF));
    s1_feed_byte((unsigned char)((s1_count >> 24) & 0xFF));

    int dumped = 0;
    for (int i = 0; i < s1_count && g_grid_count < MAX_GRID_POINTS; i++) {
        DWORD entry = s1_data[i];
        if (!entry || entry < 0x10000) continue;
        if (IsBadReadPtr((void*)entry, 16)) continue;

        /* fingerprint: every readable ref name feeds the level hash */
        {
            char* hn = *(char**)(entry + S1ENTRY_NAME);
            if (hn && !IsBadReadPtr(hn, 64)) {
                int hi = 0;
                while (hi < 63 && hn[hi]) {
                    s1_feed_byte((unsigned char)hn[hi]);
                    hi++;
                }
            }
        }

        if (!g_scan_logged && dumped < 3) {
            char nbuf[28];
            nbuf[0] = '\0';
            char* nm = *(char**)(entry + S1ENTRY_NAME);
            if (nm && !IsBadReadPtr(nm, 20)) {
                int ni = 0;
                while (ni < 27 && nm[ni]) { nbuf[ni] = nm[ni]; ni++; }
                nbuf[ni] = '\0';
            }
            char ebuf[96];
            snprintf(ebuf, sizeof(ebuf), "  E%d=0x%X nm=%s",
                     i, entry, nbuf);
            log_mod(ebuf);
            dumped++;
        }

        char* name = *(char**)(entry + S1ENTRY_NAME);
        if (name && !IsBadReadPtr(name, 5)) {
            if (nc_strstr(name, "GRID") != NULL) {
                g_pts_x[g_grid_count] = *(float*)(entry + S1ENTRY_POS_X);
                g_pts_y[g_grid_count] = *(float*)(entry + S1ENTRY_POS_Y);
                g_pts_z[g_grid_count] = *(float*)(entry + S1ENTRY_POS_Z);
                {
                    int ni = 0;
                    g_grid_names[g_grid_count][0] = '\0';
                    while (ni < 31 && name[ni]) {
                        g_grid_names[g_grid_count][ni] = name[ni];
                        ni++;
                    }
                    g_grid_names[g_grid_count][ni] = '\0';
                }
                g_grid_count++;
            }
        } else {
            /* name might be an inline char array */
            if (nc_strstr((const char*)entry, "GRID") != NULL) {
                g_pts_x[g_grid_count] = *(float*)(entry + S1ENTRY_POS_X);
                g_pts_y[g_grid_count] = *(float*)(entry + S1ENTRY_POS_Y);
                g_pts_z[g_grid_count] = *(float*)(entry + S1ENTRY_POS_Z);
                g_grid_count++;
            }
        }
    }
    return g_grid_count;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Preload model: all GRID objects are created once at level start (mesh
 * load + ctor). Switches only move list membership (show/hide) — no disk,
 * no ctor/dtor. Full destroy happens at level quit (despawn_all).
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Create the object for a GRID point (NOT list-registered). NULL on failure. */
static void* create_grid_cube(DWORD board, float px, float py, float pz,
                              int grid_num, const char* mesh_path) {
    if (!board) return NULL;

    const char* path = mesh_path;
    if (!path || !path[0]) {
        /* fallback: shared testcube mesh */
        path = mesh_file_present() ? g_mesh_path : NULL;
    }
    if (!path) {
        char mbuf[64];
        snprintf(mbuf, sizeof(mbuf), "  GRID: no mesh for %d, skip",
                 grid_num);
        log_mod(mbuf);
        return NULL;
    }

    DWORD app = g_api ? (DWORD)HBAPI(g_api).GetApp() : 0;
    if (!app || IsBadReadPtr((void*)app, 4)) { log_mod("  GRID: app=NULL"); return NULL; }
    DWORD gfx_device = *(DWORD*)(app + APP_GFX_DEVICE);
    if (!gfx_device || IsBadReadPtr((void*)gfx_device, 4)) { log_mod("  GRID: gfx_device=NULL"); return NULL; }

    /* Load mesh via MeshWorld_ctor */
    void* mesh = g_op_new(MESHWORLD_SIZE);
    if (!mesh) { log_mod("  GRID: failed to alloc mesh"); return NULL; }
    memset(mesh, 0, MESHWORLD_SIZE);
    void* loaded = g_mw_ctor(mesh, (void*)gfx_device, path);
    if (!loaded) { log_mod("  GRID: MeshWorld_ctor failed"); return NULL; }

    /* Allocate + construct PopCylinder object */
    void* obj = g_op_new(POPCYLINDER_SIZE);
    if (!obj) { log_mod("  GRID: failed to alloc PopCylinder"); return NULL; }
    memset(obj, 0, POPCYLINDER_SIZE);
    void* result = g_pc_ctor(obj, (void*)board, px, py, pz, mesh);
    if (!result) { log_mod("  GRID: PopCylinder_ctor failed"); return NULL; }

    char buf[96];
    snprintf(buf, sizeof(buf), "  GRID: preloaded %s at (%d,%d,%d) obj=0x%X",
             path, (int)px, (int)py, (int)pz, (DWORD)obj);
    log_mod(buf);
    return obj;
}

/* Show: register a preloaded object into update/render/collision lists */
static void grid_show(DWORD board, DWORD obj) {
    DWORD col_obj;
    DWORD scene_col;
    DWORD level;
    DWORD sceneobj;
    if (!board || !obj) return;
    if (IsBadReadPtr((void*)obj, 0x10D0)) return;
    g_append((void*)(board + BOARD_UPDATE_LIST), (void*)obj);
    g_append((void*)(board + BOARD_RENDER_LIST), (void*)obj);
    col_obj = *(DWORD*)((char*)obj + PC_COLLISION_OBJ);
    if (col_obj) {
        g_append((void*)(board + BOARD_COLLISION_LIST), (void*)col_obj);
        scene_col = *(DWORD*)(board + BOARD_SCENE_OBJ);
        if (scene_col) g_append((void*)(scene_col + 0x18), (void*)col_obj);
    }
    level = get_level(board);
    if (level) {
        sceneobj = *(DWORD*)(level + LEVEL_SCENEOBJECT);
        if (sceneobj) g_append((void*)(sceneobj + 0x1C), (void*)obj);
    }
}

/* Hide: unregister from all lists, object stays alive for later show */
static void grid_hide(DWORD board, DWORD obj) {
    DWORD col_obj;
    DWORD scene_col;
    DWORD level;
    DWORD sceneobj;
    if (!board || !obj) return;
    if (IsBadReadPtr((void*)obj, 0x10D0)) return;
    col_obj = *(DWORD*)((char*)obj + PC_COLLISION_OBJ);
    if (col_obj) {
        g_remove((void*)(board + BOARD_COLLISION_LIST), (int)col_obj);
        scene_col = *(DWORD*)(board + BOARD_SCENE_OBJ);
        if (scene_col) g_remove((void*)(scene_col + 0x18), (int)col_obj);
    }
    g_remove((void*)(board + BOARD_UPDATE_LIST), (int)obj);
    g_remove((void*)(board + BOARD_SCENE_UPDATE_LIST), (int)obj);
    g_remove((void*)(board + BOARD_RENDER_LIST), (int)obj);
    level = get_level(board);
    if (level) {
        sceneobj = *(DWORD*)(level + LEVEL_SCENEOBJECT);
        if (sceneobj) g_remove((void*)(sceneobj + 0x1C), (int)obj);
    }
}

static void despawn_object(DWORD board, DWORD obj) {
    if (!board || !obj) return;
    if (IsBadReadPtr((void*)obj, 0x10D0)) return;

    /* mark removed/inactive */
    *(BYTE*)((char*)obj + 0x10E5) = 1;
    *(BYTE*)((char*)obj + 0x10E4) = 1;

    DWORD col_obj = *(DWORD*)((char*)obj + PC_COLLISION_OBJ);
    if (col_obj) {
        g_remove((void*)(board + BOARD_COLLISION_LIST), (int)col_obj);
        DWORD scene_col = *(DWORD*)(board + BOARD_SCENE_OBJ);
        if (scene_col) g_remove((void*)(scene_col + 0x18), (int)col_obj);
        /* call collision object destructor (vtable[0] with flag 1 = free) */
        if (!IsBadReadPtr((void*)col_obj, 4)) {
            DWORD vtable = *(DWORD*)col_obj;
            if (vtable && !IsBadReadPtr((void*)vtable, 4)) {
                DWORD dtor = *(DWORD*)vtable;
                if (dtor && dtor > 0x400000) {
                    typedef void (__thiscall *dtor_t)(void*, int);
                    ((dtor_t)dtor)((void*)col_obj, 1);
                }
            }
        }
        *(DWORD*)((char*)obj + PC_COLLISION_OBJ) = 0;
    }

    g_remove((void*)(board + BOARD_UPDATE_LIST), (int)obj);
    g_remove((void*)(board + BOARD_SCENE_UPDATE_LIST), (int)obj);
    g_remove((void*)(board + BOARD_RENDER_LIST), (int)obj);

    DWORD level = get_level(board);
    if (level) {
        DWORD sceneobj = *(DWORD*)(level + LEVEL_SCENEOBJECT);
        if (sceneobj) g_remove((void*)(sceneobj + 0x1C), (int)obj);
    }
}

static void despawn_all(DWORD board) {
    int i;
    for (i = 0; i < MAX_SPAWNED; i++) {
        if (g_spawned_objs[i]) {
            despawn_object(board, g_spawned_objs[i]);
            g_spawned_objs[i] = 0;
        }
    }
    g_spawned_count = 0;
    g_order_count = 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Per-GRID own meshes: extract each ref's geom from a levels\ file into
 * levels\mknp_grid<N>.MESHWORLD, rebased to origin. Affix rules (NOCOLLIDE
 * => non-solid) apply natively when the game loads the temp file.
 * ═══════════════════════════════════════════════════════════════════════════ */
static void extract_grid_meshes(void) {
    int i;
    for (i = 0; i < g_grid_count; i++) g_grid_own[i] = 0;
    if (!g_levels_dir[0]) {
        log_mod("  GRID: levels dir unknown, testcube fallback");
        return;
    }
    for (i = 0; i < g_grid_count; i++) {
        const char* full = g_grid_names[i];
        const char* stripped = full;
        char base[32];
        char base_stripped[32];
        char abs[MAX_PATH];
        char xbuf[96];
        int bi = 0;
        if (stripped[0] == 'R' && stripped[1] == 'E' &&
            stripped[2] == 'F' && stripped[3] == ':')
            stripped += 4;
        /* affix-cut copies ("GRID01(NOCOLLIDE)" -> "GRID01") for matching;
         * the extracted file keeps the LIBRARY geom's own name/affix */
        while (bi < 31 && full[bi] && full[bi] != '(') {
            base[bi] = full[bi];
            bi++;
        }
        base[bi] = '\0';
        {
            const char* bs = base;
            int bj = 0;
            if (bs[0] == 'R' && bs[1] == 'E' && bs[2] == 'F' && bs[3] == ':')
                bs += 4;
            while (bj < 31 && bs[bj]) {
                base_stripped[bj] = bs[bj];
                bj++;
            }
            base_stripped[bj] = '\0';
        }
        snprintf(g_grid_mesh[i], sizeof(g_grid_mesh[i]),
                 "levels\\mknp_grid%d", i + 1);
        snprintf(abs, sizeof(abs), "%smknp_grid%d.MESHWORLD",
                 g_levels_dir, i + 1);
        if (gm_find_and_extract(g_levels_dir, full, stripped, abs) ||
            (base[0] && base_stripped[0] &&
             (strcmp(base, full) != 0 ||
              strcmp(base_stripped, stripped) != 0) &&
             gm_find_and_extract(g_levels_dir, base, base_stripped, abs))) {
            g_grid_own[i] = 1;
            snprintf(xbuf, sizeof(xbuf), "  GRID%d %s: own mesh OK",
                     i + 1, full);
        } else {
            snprintf(xbuf, sizeof(xbuf), "  GRID%d %s: no lib mesh",
                     i + 1, full);
        }
        log_mod(xbuf);
    }
}

static const char* mesh_for(int idx) {
    if (idx >= 0 && idx < g_grid_count && g_grid_own[idx] &&
        g_grid_mesh[idx][0])
        return g_grid_mesh[idx];
    return mesh_file_present() ? g_mesh_path : NULL;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Level-start: scan GRID points and begin the cycle with GRID01
 * ═══════════════════════════════════════════════════════════════════════════ */
static void start_grid_cycle(DWORD board) {
    g_active_board = board;
    g_cycle_started = false;
    g_board_ready_delay = 0;
    g_grid_count = 0;
    g_spawned_count = 0;

    int count = find_grid_points(board);
    if (count > 0) {
        char buf[64];
        g_mult = gridset_level_mult(g_levels_dir, g_s1_hash, g_s1_count);
        snprintf(buf, sizeof(buf), "  Found %d GRID points, starting cycle (speed=%dms)", count, (int)(g_speed * g_mult * 1000.0f));
        log_mod(buf);
        for (int pi = 0; pi < count; pi++) {
            char pbuf[96];
            snprintf(pbuf, sizeof(pbuf), "  PT %d (%d,%d,%d)",
                     pi + 1, (int)g_pts_x[pi], (int)g_pts_y[pi], (int)g_pts_z[pi]);
            log_mod(pbuf);
        }
        g_current_grid = 0;   /* index into g_order (set below) */
        extract_grid_meshes();
        /* preload every point once; cycle only moves list membership */
        for (int si = 0; si < MAX_SPAWNED; si++) g_spawned_objs[si] = 0;
        g_spawned_count = 0;
        g_order_count = 0;
        for (int pi = 0; pi < count; pi++) {
            void* obj = create_grid_cube(board, g_pts_x[pi], g_pts_y[pi],
                                         g_pts_z[pi], pi + 1, mesh_for(pi));
            if (obj) {
                g_spawned_objs[pi] = (DWORD)obj;
                g_order[g_order_count++] = pi;
                g_spawned_count++;
            } else {
                char fbuf[64];
                snprintf(fbuf, sizeof(fbuf), "  GRID%d: preload failed",
                         pi + 1);
                log_mod(fbuf);
            }
        }
        if (!g_order_count) {
            log_mod("  GRID: nothing preloaded, no cycle");
            /* stay dead until board change — retrying every frame would
             * rescan levels\ + reload files continuously (extreme slowdown) */
            g_cycle_started = true;
            return;
        }
        g_current_grid = 0;   /* index into g_order */
        grid_show(board, g_spawned_objs[g_order[0]]);
        g_last_switch_tick = GetTickCount();
        g_prev_tick = g_last_switch_tick;
        g_cycle_started = true;
    } else {
        log_mod("  No GRID points found");
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * HB+ callback wiring
 * ═══════════════════════════════════════════════════════════════════════════ */

static void* __thiscall sc_dtor(void* thisptr, int flags) {
    if (flags & 1) operator delete(thisptr);
    return thisptr;
}
static const char* __thiscall get_mod_name(void*) { return "Battyball Entities"; }
static const char* __thiscall get_author(void*) { return "MAKYUNI"; }
static int __thiscall get_version(void*) { return HAMSTERBALL_API_VERSION; }
static const char* __thiscall get_contributors(void*) { return "Hamsterbot"; }

static void __thiscall init_impl(void* thisptr, IModAPI* api) {
    g_api = api;
    *(void**)((char*)thisptr + 4) = api;

    /* locate own module via VirtualQuery for init game dir */
    MEMORY_BASIC_INFORMATION mbi;
    VirtualQuery((void*)&init_impl, &mbi, sizeof(mbi));
    g_module = (HMODULE)mbi.AllocationBase;
    resolve_log_path(g_log_path);

    /* Copy testcube.MESHWORLD (next to DLL) into the game's levels\ folder.
     * The DLL sits in Mods\; levels\ is one level up in the game root. */
    if (g_module) {
        char mod_path[MAX_PATH];
        GetModuleFileNameA(g_module, mod_path, MAX_PATH);
        char* slash = mod_path;
        for (char* p = mod_path; *p; p++) if (*p == '\\') slash = p;
        *slash = '\0';   /* folder containing the DLL */
        gridset_init(mod_path);
        char src[MAX_PATH];
        snprintf(src, sizeof(src), "%s\\testcube.MESHWORLD", mod_path);

        /* Candidate levels\ paths: same dir, then one level up (game root). */
        char cand[2][MAX_PATH];
        snprintf(cand[0], sizeof(cand[0]), "%s\\levels\\testcube.MESHWORLD", mod_path);
        snprintf(cand[1], sizeof(cand[1]), "%s\\..\\levels\\testcube.MESHWORLD", mod_path);
        for (int i = 0; i < 2; i++) {
            /* only copy if the source exists */
            if (GetFileAttributesA(src) != INVALID_FILE_ATTRIBUTES) {
                CopyFileA(src, cand[i], FALSE);
                break;
            }
            /* if source is gone but dest already exists, nothing to do */
        }

        /* Resolve the game levels\ dir (trailing backslash) for mesh extract */
        g_levels_dir[0] = '\0';
        {
            char d0[MAX_PATH], d1[MAX_PATH];
            snprintf(d0, sizeof(d0), "%s\\levels\\", mod_path);
            snprintf(d1, sizeof(d1), "%s\\..\\levels\\", mod_path);
            char probe[MAX_PATH];
            DWORD fa;
            snprintf(probe, sizeof(probe), "%s\\levels", mod_path);
            fa = GetFileAttributesA(probe);
            if (fa != INVALID_FILE_ATTRIBUTES &&
                (fa & FILE_ATTRIBUTE_DIRECTORY)) {
                strncpy(g_levels_dir, d0, MAX_PATH);
            } else {
                snprintf(probe, sizeof(probe), "%s\\..\\levels", mod_path);
                fa = GetFileAttributesA(probe);
                if (fa != INVALID_FILE_ATTRIBUTES &&
                    (fa & FILE_ATTRIBUTE_DIRECTORY))
                    strncpy(g_levels_dir, d1, MAX_PATH);
            }
        }
    }

    CustomButton btn("BATTY_ENTITIES", "Battyball Entities");
    btn.defaultState = true;
    HBAPI(api).CreateToggleButton(btn, (HamsterballAPI*)thisptr);

    CustomSlider s1("BATTY_GRID_SPEED", "Grid Speed (s)", GRID_SPEED_DEFAULT);
    s1.lowerBound = 0.5f; s1.upperBound = 30.0f; s1.stepSize = 0.5f; s1.decimalPlaces = 1;
    HBAPI(api).CreateSlider(s1, (HamsterballAPI*)thisptr);

    log_mod("INIT Battyball Entities v1 (mod loaded)");
}

static void __thiscall button_toggle(void*, const char* id, bool state) {
    if (strcmp(id, "BATTY_ENTITIES") == 0) {
        g_enabled = state;
        if (!state) {
            DWORD board = player_board();
            despawn_all(board);
            g_cycle_started = false;
        }
    }
}

static void __thiscall slider_change(void*, const char* id, float value) {
    if (strcmp(id, "BATTY_GRID_SPEED") == 0) g_speed = value;
}

static void __thiscall level_start(void*) {
    /* A new level is loading — clear state; cycle restarts when board is ready */
    g_active_board = 0;
    g_cycle_started = false;
    g_grid_count = 0;
    g_spawned_count = 0;
    g_order_count = 0;
}

static void __thiscall scene_end(void*) {
    DWORD board = player_board();
    despawn_all(board);
    g_cycle_started = false;
    g_active_board = 0;
}

static void __thiscall game_update(void*) {
    if (!g_enabled) return;
    if (!g_api) return;

    DWORD board = player_board();
    if (!board || IsBadReadPtr((void*)board, 0x4400)) return;

    /* Detect board change (new level loaded, or tournament board reuse) */
    if (board != g_active_board) {
        char bbuf[64];
        snprintf(bbuf, sizeof(bbuf), "NEWBOARD 0x%08X (was 0x%08X)",
                 board, g_active_board);
        log_mod(bbuf);
        g_active_board = board;
        g_scan_logged = 0;
        g_board_ready_delay = 40;   /* wait ~40 frames for the level to finish building */
        g_cycle_started = false;
        g_spawned_count = 0;
        g_grid_count = 0;
        g_order_count = 0;
        return;
    }

    if (!g_cycle_started) {
        if (g_board_ready_delay > 0) { g_board_ready_delay--; return; }
        start_grid_cycle(board);
        return;
    }

    /* Pause gate: freeze cycle progress while paused (resume continues
     * exactly where it left off — paused time never counts, never resets) */
    {
        DWORD now = GetTickCount();
        if (!g_prev_tick) g_prev_tick = now;
        if (!IsBadReadPtr((void*)(board + BOARD_PAUSED), 4) &&
            *(int*)(board + BOARD_PAUSED)) {
            g_last_switch_tick += (now - g_prev_tick);
            g_prev_tick = now;
            return;
        }
        g_prev_tick = now;
    }

    /* Time-based cycling (slider seconds x per-level multiplier) */
    DWORD now = GetTickCount();
    int wait_ms = (int)(g_speed * g_mult * 1000.0f);
    if (wait_ms < 10) wait_ms = 10;
    if ((int)(now - g_last_switch_tick) < wait_ms) return;

    /* Advance to next GRID (show/hide only — everything preloaded).
     * Frame-skipping: if several intervals elapsed, jump them in ONE
     * hide/show pair. Switch logs throttled to ~2/sec at high speed. */
    if (!g_order_count) return;
    {
        int old_ord = g_current_grid;
        int new_ord;
        int steps = 1;
        int elapsed = (int)(now - g_last_switch_tick);
        if (wait_ms > 0) {
            steps = elapsed / wait_ms;
            if (steps < 1) steps = 1;
            if (steps > 64) steps = 64;
        }
        new_ord = (old_ord + steps) % g_order_count;
        if ((int)(now - g_last_switch_log) >= 500 || steps > 1) {
            char sbuf[128];
            snprintf(sbuf, sizeof(sbuf), "SWITCH %d->%d (pts=%d preloaded=%d upd=%d rnd=%d skip=%d)",
                     g_order[old_ord] + 1, g_order[new_ord] + 1,
                     g_grid_count, g_spawned_count,
                     list_count(board + BOARD_UPDATE_LIST),
                     list_count(board + BOARD_RENDER_LIST),
                     steps - 1);
            log_mod(sbuf);
            g_last_switch_log = now;
        }
        g_current_grid = new_ord;
        if (new_ord != old_ord) {
            grid_hide(board, g_spawned_objs[g_order[old_ord]]);
            grid_show(board, g_spawned_objs[g_order[new_ord]]);
        }
        g_last_switch_tick += (DWORD)(steps * wait_ms);
    }
    g_prev_tick = now;
}

static void __thiscall ball_update(void*, void*) {}
static void __thiscall render_apply(void*, void*, float*) {}
static void __thiscall cycle_option_change(void*, const char*, const char*) {}
static void __thiscall event_collide(void*, void*, char*) {}
static void __thiscall text_render(void*) {}
static void __thiscall ball_bump(void*, void*, void*) {}

/* 17-entry vtable (HB+ v2.0/v2.1) */
static void* g_vtable[17] = {
    (void*)sc_dtor,              // [0]  ~HamsterballAPI
    (void*)get_mod_name,         // [1]  GetModName
    (void*)get_author,           // [2]  GetAuthorName
    (void*)get_version,          // [3]  GetApiVersion
    (void*)get_contributors,     // [4]  GetContributors
    (void*)init_impl,            // [5]  Initialize
    (void*)ball_update,          // [6]  onBallUpdate
    (void*)render_apply,         // [7]  onRenderApply
    (void*)button_toggle,        // [8]  onButtonToggle
    (void*)slider_change,        // [9]  onSliderChange
    (void*)cycle_option_change,  // [10] onCycleOptionChange (v2.0+)
    (void*)game_update,          // [11] onGameUpdate
    (void*)event_collide,        // [12] onEventPlaneCollide
    (void*)text_render,          // [13] onTextRenderLoop
    (void*)ball_bump,            // [14] onBallBump
    (void*)scene_end,            // [15] onSceneEnd
    (void*)level_start,          // [16] onLevelStart
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    void* obj = operator new(8);
    if (!obj) return NULL;
    *(void**)obj = g_vtable;
    *(void**)((char*)obj + 4) = NULL;
    return (HamsterballAPI*)obj;
}