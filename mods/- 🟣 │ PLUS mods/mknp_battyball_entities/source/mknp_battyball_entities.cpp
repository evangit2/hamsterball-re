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

/* Native light objects (Ghidra-verified: Scene_SetupLevelDark 0x416270) */
#define SCENEOBJECT_SIZE    0xD4
#define SCENEOBJECT_CTOR    0x0046B4F0   /* __thiscall(this, gfx), RET 4 */
#define SCENE_REGISTEROBJECT 0x00453BD0  /* __thiscall(gfx, slot, obj), RET 8 */
#define SO_POS_X            0x08     /* light position (SetPosition target) */
#define SO_POS_Y            0x0C
#define SO_POS_Z            0x10
/* RefreshLight 0x46B670 builds D3DLIGHT8 in-object at +0x20: true Range
 * field = +0x68 (game NEVER writes it — +0xCC lands on Falloff). */
#define SO_RANGE_REAL       0x68     /* true D3D Range: MUST be set */
#define SO_EMIT_R           0x94     /* emitter color RGBA */
#define SO_EMIT_G           0x98
#define SO_EMIT_B           0x9C
#define SO_EMIT_A           0xA0
#define SO_VISIBLE          0x88
#define SO_RANGE            0xCC
#define SO_TYPE             0xD0     /* 1 = D3DLIGHT_POINT (SDK truth) */
#define GFX_LIGHT_SLOTS     0x710    /* 8 SceneObject* slots */
#define MAX_LIGHTS          4
#define LIGHT_SLOT_BASE     4        /* Neon owns 0-1, S3 re-registers from 0 */
#define LIGHT_TYPE_POINT    1
#define LIGHT_INTENSITY     5.0f     /* fixed material-color multiplier */

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
static int   g_grid_num[MAX_GRID_POINTS];         /* numeric GRID id (sort key) */
static char  g_grid_digits[MAX_GRID_POINTS][8];   /* digit string ("01") */
static int   g_grid_count = 0;
static int   g_scan_logged = 0;   /* 1 after first detailed scan dump */
static char  g_levels_dir[MAX_PATH];  /* game levels\ dir, trailing backslash */

/* Active level / board the current cycle belongs to */
static DWORD g_active_board = 0;
static int   g_board_ready_delay = 0;   /* frames to wait for level build after board change */

/* Native point lights (S3 DISTANTLIGHTs -> SceneObjects in gfx slots) */
static DWORD g_light_objs[MAX_LIGHTS];   /* never freed, reused per level */
static float g_light_pos[MAX_LIGHTS][3]; /* game coords */
static float g_light_col[MAX_LIGHTS][3];
static int   g_light_src[MAX_LIGHTS];    /* 0 = S3 file light, 1 = POINT ref */
static int   g_light_count = 0;          /* parsed (clamped to MAX_LIGHTS) */
static int   g_light_used = 0;           /* slots currently registered */
static int   g_light_vis = 1;            /* 0 while LIGHTSOFF */
static bool  g_lights_on = true;         /* BATTY_LIGHTS toggle */
static float g_light_range = 400.0f;     /* BATTY_LIGHT_RANGE slider */
static int   g_job = 0;                  /* text_render job: 0 none 1 build 2 refresh */
static DWORD g_job_board = 0;

/* POINTLIGHTxx S1 ref positions (case-insensitive) for ref-driven lights */
static float g_ppt_x[MAX_LIGHTS];
static float g_ppt_y[MAX_LIGHTS];
static float g_ppt_z[MAX_LIGHTS];
static char  g_ppt_name[MAX_LIGHTS][32];  /* S1 ref name (mesh lookup key) */
static int   g_ppt_count = 0;

/* Case-insensitive substring search — POINT refs may be "Point01". */
static const char* nc_istrstr(const char* hay, const char* needle) {
    size_t nl;
    const char* h;
    if (!hay || !needle) return NULL;
    nl = strlen(needle);
    if (nl == 0) return hay;
    for (h = hay; *h; h++) {
        size_t k = 0;
        while (k < nl && h[k]) {
            char a = h[k], b = needle[k];
            if (a >= 'a' && a <= 'z') a -= 32;
            if (b >= 'a' && b <= 'z') b -= 32;
            if (a != b) break;
            k++;
        }
        if (k == nl) return h;
    }
    return NULL;
}

/* True if an S1 ref name marks a light (Pointlight01, POINT01...). */
static int is_light_ref(const char* nm) {
    if (nc_istrstr(nm, "POINTLIGHT") != NULL) return 1;
    if (nc_istrstr(nm, "POINT") != NULL) return 1;
    if (nc_istrstr(nm, "LIGHT") != NULL) return 1;
    return 0;
}

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
    g_ppt_count = 0;
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
    for (int i = 0; i < s1_count &&
         (g_grid_count < MAX_GRID_POINTS || g_ppt_count < MAX_LIGHTS); i++) {
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
            if (nc_strstr(name, "GRID") != NULL &&
                g_grid_count < MAX_GRID_POINTS) {
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
            if (is_light_ref(name) && g_ppt_count < MAX_LIGHTS) {
                int pi = g_ppt_count;
                int ni = 0;
                g_ppt_x[pi] = *(float*)(entry + S1ENTRY_POS_X);
                g_ppt_y[pi] = *(float*)(entry + S1ENTRY_POS_Y);
                g_ppt_z[pi] = *(float*)(entry + S1ENTRY_POS_Z);
                g_ppt_name[pi][0] = '\0';
                while (ni < 31 && name[ni]) {
                    g_ppt_name[pi][ni] = name[ni];
                    ni++;
                }
                g_ppt_name[pi][ni] = '\0';
                g_ppt_count++;
            }
        } else {
            /* name might be an inline char array */
            if (nc_strstr((const char*)entry, "GRID") != NULL &&
                g_grid_count < MAX_GRID_POINTS) {
                g_pts_x[g_grid_count] = *(float*)(entry + S1ENTRY_POS_X);
                g_pts_y[g_grid_count] = *(float*)(entry + S1ENTRY_POS_Y);
                g_pts_z[g_grid_count] = *(float*)(entry + S1ENTRY_POS_Z);
                g_grid_count++;
            }
            if (is_light_ref((const char*)entry) &&
                g_ppt_count < MAX_LIGHTS) {
                g_ppt_x[g_ppt_count] = *(float*)(entry + S1ENTRY_POS_X);
                g_ppt_y[g_ppt_count] = *(float*)(entry + S1ENTRY_POS_Y);
                g_ppt_z[g_ppt_count] = *(float*)(entry + S1ENTRY_POS_Z);
                g_ppt_name[g_ppt_count][0] = '\0';   /* no clean name */
                g_ppt_count++;
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
 * Per-GRID meshes from files: S1 ref GRIDxx -> levels\Gridxx.MESHWORLD.
 * Number = digits right after "GRID" in the ref name ("GRID01(NOCOLLIDE)"
 * -> "01" -> levels\Grid01.MESHWORLD). Whatever the file holds (mesh,
 * affixes) applies natively at load. Missing file = testcube fallback.
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Digits after "GRID" (case-insensitive). Returns 1 + fills out (NUL). */
static int grid_number(const char* name, char* out, unsigned cap) {
    unsigned i = 0;
    if (!name || cap < 2) return 0;
    while (name[i]) {
        if ((name[i] == 'G' || name[i] == 'g') &&
            (name[i + 1] == 'R' || name[i + 1] == 'r') &&
            (name[i + 2] == 'I' || name[i + 2] == 'i') &&
            (name[i + 3] == 'D' || name[i + 3] == 'd')) {
            unsigned j = i + 4, k = 0;
            while (name[j] >= '0' && name[j] <= '9' && k + 1 < cap) {
                out[k++] = name[j++];
            }
            out[k] = '\0';
            return k > 0;
        }
        i++;
    }
    return 0;
}

/* Validate a GridNN file before handing it to MeshWorld_ctor: must parse
 * S1..S6 and hold >=1 geom. Returns geom count, -1 when bad. A malformed
 * file would crash the ctor (heap overrun), so bad files fall back. */
static int validate_grid_file(const char* abs) {
    unsigned dlen = 0;
    unsigned char* d = gm_read_file(abs, &dlen);
    if (!d) return -1;
    {
        int total = gm_list_geoms(d, dlen, NULL, NULL);
        free(d);
        if (total < 1) return -1;
        return total;
    }
}

static void resolve_grid_files(void) {
    int i;
    for (i = 0; i < g_grid_count; i++) g_grid_own[i] = 0;
    if (!g_levels_dir[0]) {
        log_mod("  GRID: levels dir unknown, skip");
        return;
    }
    /* one-time cleanup of old extract temps (no longer used) */
    for (i = 0; i < MAX_GRID_POINTS; i++) {
        char old[MAX_PATH];
        snprintf(old, sizeof(old), "%smknp_grid%d.MESHWORLD",
                 g_levels_dir, i + 1);
        DeleteFileA(old);
    }
    for (i = 0; i < g_grid_count; i++) {
        char digits[8];
        char xbuf[128];
        int num = 9999;
        if (!grid_number(g_grid_names[i], digits, sizeof(digits))) {
            digits[0] = '\0';
            g_grid_num[i] = num;
            g_grid_digits[i][0] = '\0';
            snprintf(xbuf, sizeof(xbuf), "  GRID%d %s: no number, skip",
                     i + 1, g_grid_names[i]);
            log_mod(xbuf);
            continue;
        }
        for (int di = 0; digits[di]; di++) {
            if (num == 9999) num = 0;
            num = num * 10 + (digits[di] - '0');
        }
        g_grid_num[i] = num;
        {
            int ki = 0;
            while (ki < 7 && digits[ki]) {
                g_grid_digits[i][ki] = digits[ki];
                ki++;
            }
            g_grid_digits[i][ki] = '\0';
        }
        snprintf(g_grid_mesh[i], sizeof(g_grid_mesh[i]),
                 "levels\\Grid%s", digits);
        {
            char abs[MAX_PATH];
            snprintf(abs, sizeof(abs), "%sGrid%s.MESHWORLD",
                     g_levels_dir, digits);
            if (GetFileAttributesA(abs) == INVALID_FILE_ATTRIBUTES) {
                g_grid_own[i] = 0;
                snprintf(xbuf, sizeof(xbuf), "  GRID%d %s: Grid%s.MESHWORLD MISSING",
                         i + 1, g_grid_names[i], digits);
            } else {
                int geoms = validate_grid_file(abs);
                if (geoms < 0) {
                    g_grid_own[i] = 0;
                    snprintf(xbuf, sizeof(xbuf), "  GRID%d %s: Grid%s.BAD, fallback",
                             i + 1, g_grid_names[i], digits);
                } else {
                    g_grid_own[i] = 1;
                    snprintf(xbuf, sizeof(xbuf), "  GRID%d %s: file Grid%s (%d geoms)",
                             i + 1, g_grid_names[i], digits, geoms);
                }
            }
        }
        log_mod(xbuf);
    }
}

static const char* mesh_for(int idx) {
    if (idx >= 0 && idx < g_grid_count && g_grid_own[idx] &&
        g_grid_mesh[idx][0])
        return g_grid_mesh[idx];
    return mesh_file_present() ? g_mesh_path : NULL;   /* testcube fallback */
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Native point lights: S3 DISTANTLIGHTs -> SceneObjects in gfx slots 4-7.
 * Mirrors Scene_SetupLevelDark exactly (op_new 0xD4, ctor, +0xD0=1 POINT,
 * emitter +0x94, pos +0x08, range +0xCC, RegisterObject). The ONLY native
 * call is RegisterObject (it runs RefreshLight internally); every other
 * field is written directly. Calls run in text_render (render thread);
 * onGameUpdate only writes plain fields and raises jobs.
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Locate the on-disk level file matching the runtime S1 (hash+count). */
static int find_current_level_file(char* out, unsigned cap) {
    static char files[GM_MAX_LIST][MAX_PATH];
    int n, i;
    if (!g_levels_dir[0] || !g_s1_count) return 0;
    n = gm_list_mw(g_levels_dir, files, GM_MAX_LIST);
    for (i = 0; i < n; i++) {
        unsigned len = 0;
        unsigned char* d;
        int fc = 0;
        unsigned fh;
        if (gm_name_is_temp(files[i])) continue;
        d = gm_read_file(files[i], &len);
        if (!d) continue;
        fh = gm_s1_hash(d, len, &fc);
        free(d);
        if (fc == g_s1_count && fh == g_s1_hash) {
            unsigned k = 0;
            while (k + 1 < cap && files[i][k]) {
                out[k] = files[i][k];
                k++;
            }
            out[k] = '\0';
            return 1;
        }
    }
    return 0;
}

/* Parse S3 lights + S4 ambient from a level file. Stores up to MAX_LIGHTS
 * (pos swizzled file x,z,y -> game x,y,z). Returns light count or -1. */
static int read_level_lights(const char* path) {
    unsigned len = 0;
    unsigned char* d = gm_read_file(path, &len);
    GmCur c;
    int i, n, total = 0;
    if (!d) return -1;
    c.p = d;
    c.end = d + len;
    /* S1 skip (mirror gm_s1_hash walk) */
    if (c.end - c.p < 4) { free(d); return -1; }
    n = gm_i32(&c);
    if (n < 0 || n > 100000) { free(d); return -1; }
    for (i = 0; i < n; i++) {
        int ln;
        if (!gm_need(&c, 4)) { free(d); return -1; }
        ln = gm_i32(&c);
        if (ln < 1 || ln > 1024 || !gm_need(&c, (unsigned)ln)) { free(d); return -1; }
        c.p += (unsigned)ln;
        if (!gm_need(&c, 24 + 4)) { free(d); return -1; }
        c.p += 24;
        {
            unsigned char hm = *c.p;
            c.p += 4;
            if (hm) {
                unsigned ht;
                if (!gm_need(&c, 64 + 4 + 4 + 4)) { free(d); return -1; }
                c.p += 64 + 4 + 4;
                ht = gm_u32(&c);
                if (ht == 1) {
                    int tl;
                    if (!gm_need(&c, 4)) { free(d); return -1; }
                    tl = gm_i32(&c);
                    if (tl < 1 || tl > 1024 || !gm_need(&c, (unsigned)tl)) { free(d); return -1; }
                    c.p += (unsigned)tl;
                }
            }
        }
    }
    /* S2 skip */
    if (!gm_need(&c, 4)) { free(d); return -1; }
    n = gm_i32(&c);
    if (n < 0 || n > 100000) { free(d); return -1; }
    for (i = 0; i < n; i++) {
        int dl, pc;
        if (!gm_need(&c, 4)) { free(d); return -1; }
        dl = gm_i32(&c);
        if (dl < 0 || dl > 1000000 || !gm_need(&c, (unsigned)dl + 4)) { free(d); return -1; }
        c.p += (unsigned)dl;
        pc = gm_i32(&c);
        if (pc < 0 || pc > 1000000 || !gm_need(&c, (unsigned)pc * 12u)) { free(d); return -1; }
        c.p += (unsigned)pc * 12u;
    }
    /* S3 lights */
    if (!gm_need(&c, 4)) { free(d); return -1; }
    n = gm_i32(&c);
    if (n < 0 || n > 64) { free(d); return -1; }
    g_light_count = 0;
    for (i = 0; i < n; i++) {
        int t;
        float v[9];
        int k;
        if (!gm_need(&c, 4)) { free(d); return -1; }
        t = gm_i32(&c);
        /* bare type entries carry no payload (mirrors gm_skip_to_s6) */
        if (t != 0) continue;
        if (!gm_need(&c, 36)) { free(d); return -1; }
        for (k = 0; k < 9; k++) v[k] = gm_f32(&c);
        if (total < MAX_LIGHTS) {
            g_light_pos[total][0] = v[0];
            g_light_pos[total][1] = v[2];   /* file x,z,y -> game x,y,z */
            g_light_pos[total][2] = v[1];
            g_light_col[total][0] = v[6];
            g_light_col[total][1] = v[7];
            g_light_col[total][2] = v[8];
            g_light_src[total] = 0;         /* S3 file light */
            total++;
        }
    }
    g_light_count = total;
    /* S4 ambient (log only — never touched) */
    if (gm_need(&c, 24)) {
        float bg0 = 0, bg1 = 0, bg2 = 0, am0 = 0, am1 = 0, am2 = 0;
        bg0 = gm_f32(&c); bg1 = gm_f32(&c); bg2 = gm_f32(&c);
        am0 = gm_f32(&c); am1 = gm_f32(&c); am2 = gm_f32(&c);
        {
            /* nocrt snprintf has no %.2f — log hundredths as ints */
            char abuf[96];
            snprintf(abuf, sizeof(abuf), "  LIGHT ambient=(%d,%d,%d) bg=(%d,%d,%d)",
                     (int)(am0 * 100.0f), (int)(am1 * 100.0f),
                     (int)(am2 * 100.0f), (int)(bg0 * 100.0f),
                     (int)(bg1 * 100.0f), (int)(bg2 * 100.0f));
            log_mod(abuf);
        }
    }
    free(d);
    return total;
}

/* Find first S6 geom whose name contains `needle` (case-insens), copy its
 * material diffuse RGB (file mat+16/20/24). Returns 1 found, 0 not. */
typedef struct {
    const char* needle;
    float rgb[3];
    int hit;
    int nodes;
} LightFind;

static int light_node(GmCur* c, LightFind* f) {
    int sub, mbc, i;
    if (++f->nodes > GM_MAX_NODES) return 0;
    if (!gm_need(c, 28)) return 0;
    c->p += 24;
    sub = gm_i32(c);
    if (sub < 1) {
        if (!gm_need(c, 4)) return 0;
        mbc = gm_i32(c);
        if (mbc < 0 || mbc > 100000) return 0;
        for (i = 0; i < mbc; i++) {
            int ln, sc;
            const unsigned char* nm;
            const unsigned char* matp;
            unsigned ht;
            if (!gm_need(c, 4)) return 0;
            ln = gm_i32(c);
            if (ln < 1 || ln > 1024 || !gm_need(c, (unsigned)ln)) return 0;
            nm = c->p;
            c->p += (unsigned)ln;
            matp = c->p;
            if (!gm_need(c, 72 + 4)) return 0;
            if (!f->hit) {
                char tmp[64];
                int k = 0;
                while (k < ln && k < 63) {
                    tmp[k] = (char)nm[k];
                    k++;
                }
                tmp[k] = '\0';
                if (nc_istrstr(tmp, f->needle) != NULL) {
                    memcpy(&f->rgb[0], matp + 16, 4);
                    memcpy(&f->rgb[1], matp + 20, 4);
                    memcpy(&f->rgb[2], matp + 24, 4);
                    f->hit = 1;
                    return 2;
                }
            }
            c->p += 72;
            ht = gm_u32(c);
            if (ht == 1) {
                int tl;
                if (!gm_need(c, 4)) return 0;
                tl = gm_i32(c);
                if (tl < 1 || tl > 1024 || !gm_need(c, (unsigned)tl))
                    return 0;
                c->p += (unsigned)tl;
            }
            if (!gm_need(c, 4)) return 0;
            sc = gm_i32(c);
            if (sc < 0 || sc > 1000000) return 0;
            if (!gm_need(c, (unsigned)sc * 8u)) return 0;
            c->p += (unsigned)sc * 8u;
        }
        return 1;
    }
    {
        int k;
        if (sub > 100000) return 0;
        for (k = 0; k < sub; k++) {
            int r = light_node(c, f);
            if (r != 1) return r;
        }
    }
    return 1;
}

static int read_geom_diffuse(const char* path, const char* needle,
                             float* rgb) {
    unsigned len = 0;
    unsigned char* data;
    GmCur c;
    const unsigned char* vbuf = 0;
    int nverts = 0;
    LightFind f;
    int r;
    if (!path || !path[0] || !needle || !needle[0]) return 0;
    data = gm_read_file(path, &len);
    if (!data) return 0;
    c.p = data;
    c.end = data + len;
    if (!gm_skip_to_s6(&c, &vbuf, &nverts)) {
        free(data);
        return 0;
    }
    f.needle = needle;
    f.hit = 0;
    f.nodes = 0;
    f.rgb[0] = f.rgb[1] = f.rgb[2] = 1.0f;
    r = light_node(&c, &f);
    if (r != 1 && r != 2) {
        free(data);
        return 0;
    }
    if (!f.hit) {
        free(data);
        return 0;
    }
    rgb[0] = f.rgb[0];
    rgb[1] = f.rgb[1];
    rgb[2] = f.rgb[2];
    free(data);
    return 1;
}

/* __thiscall wrappers (MinGW asm caves). Callee-cleanup (RET 4/8). */
static DWORD native_new_sceneobject(DWORD gfx) {
    DWORD obj = g_op_new(SCENEOBJECT_SIZE);
    DWORD fn = SCENEOBJECT_CTOR;
    DWORD ret = 0;
    if (!obj) return 0;
    memset(obj, 0, SCENEOBJECT_SIZE);
    __asm__ __volatile__(
        "pushl %2\n\t"
        "movl %1, %%ecx\n\t"
        "call *%3\n\t"
        "movl %%eax, %0\n\t"
        : "=r"(ret) : "r"(obj), "r"(gfx), "r"(fn)
        : "eax", "edx", "ecx", "memory");
    return ret;
}

static void native_register(DWORD gfx, int slot, DWORD obj) {
    DWORD fn = SCENE_REGISTEROBJECT;
    __asm__ __volatile__(
        "pushl %2\n\t"
        "pushl %1\n\t"
        "movl %0, %%ecx\n\t"
        "call *%3\n\t"
        : : "r"(gfx), "r"((DWORD)slot), "r"(obj), "r"(fn)
        : "eax", "edx", "ecx", "memory");
}

/* vtable[1] = SetPosition(x,y,z), __thiscall. Scene_SetupLevelDark calls
 * this (never writes +0x08 directly) — mirror it exactly. */
static void native_setpos(DWORD obj, float x, float y, float z) {
    DWORD fn;
    if (!obj || IsBadReadPtr((void*)obj, 8)) return;
    fn = *(DWORD*)(*(DWORD*)obj + 4);
    if (!fn || IsBadReadPtr((void*)fn, 1)) return;
    __asm__ __volatile__(
        "pushl %3\n\t"
        "pushl %2\n\t"
        "pushl %1\n\t"
        "movl %0, %%ecx\n\t"
        "call *%4\n\t"
        : : "r"(obj), "m"(x), "m"(y), "m"(z), "r"(fn)
        : "eax", "edx", "ecx", "memory");
}

/* vtable[1] helper above; gfx accessor below */
static DWORD gfx_device(void) {
    DWORD app;
    DWORD gfx;
    if (!g_api) return 0;
    app = (DWORD)HBAPI(g_api).GetApp();
    if (!app || IsBadReadPtr((void*)app, 0x700)) return 0;
    gfx = *(DWORD*)(app + APP_GFX_DEVICE);
    if (!gfx || IsBadReadPtr((void*)gfx, 0x800)) return 0;
    return gfx;
}

static void write_light_fields(DWORD obj, int li, int vis) {
    *(int*)((char*)obj + SO_TYPE) = LIGHT_TYPE_POINT;
    *(float*)((char*)obj + SO_EMIT_R) = g_light_col[li][0];
    *(float*)((char*)obj + SO_EMIT_G) = g_light_col[li][1];
    *(float*)((char*)obj + SO_EMIT_B) = g_light_col[li][2];
    *(float*)((char*)obj + SO_EMIT_A) = 1.0f;
    *(float*)((char*)obj + SO_POS_X) = g_light_pos[li][0];
    *(float*)((char*)obj + SO_POS_Y) = g_light_pos[li][1];
    *(float*)((char*)obj + SO_POS_Z) = g_light_pos[li][2];
    *(float*)((char*)obj + SO_RANGE_REAL) = g_light_range;
    *(float*)((char*)obj + SO_RANGE) = g_light_range;
    *(BYTE*)((char*)obj + SO_VISIBLE) = (BYTE)(vis ? 1 : 0);
}

/* Runs in text_render (render thread). job 1 = build, 2 = refresh.
 * quiet=1 skips per-light logs (periodic re-assert). */
static void service_light_job(DWORD board, int quiet) {
    DWORD gfx;
    int i, vis, hi;
    if (g_job == 0 || board != g_job_board) return;
    if (!g_light_count && !g_light_used) { g_job = 0; return; }
    gfx = gfx_device();
    if (!gfx) return;   /* retry next frame */
    vis = (g_lights_on && g_light_vis) ? 1 : 0;
    hi = g_light_used;
    if (g_job == 3) {
        /* level end: switch every used slot off, keep objects alive */
        for (i = 0; i <= hi && i < MAX_LIGHTS; i++) {
            if (g_light_objs[i]) {
                *(BYTE*)((char*)g_light_objs[i] + SO_VISIBLE) = 0;
                native_register(gfx, LIGHT_SLOT_BASE + i, g_light_objs[i]);
            }
        }
        log_mod("  LIGHT: level end, slots off");
        g_job = 0;
        return;
    }
    if (g_light_count - 1 > hi) hi = g_light_count - 1;
    for (i = 0; i <= hi && i < MAX_LIGHTS; i++) {
        DWORD obj = g_light_objs[i];
        char lbuf[128];
        if (i < g_light_count) {
            if (!obj) {
                obj = native_new_sceneobject(gfx);
                if (!obj || IsBadReadPtr((void*)obj, SCENEOBJECT_SIZE)) {
                    snprintf(lbuf, sizeof(lbuf), "  LIGHT%d: ctor failed", i);
                    log_mod(lbuf);
                    g_light_objs[i] = 0;
                    continue;
                }
                g_light_objs[i] = obj;
            }
            write_light_fields(obj, i, vis);
            native_setpos(obj, g_light_pos[i][0], g_light_pos[i][1],
                          g_light_pos[i][2]);
            native_register(gfx, LIGHT_SLOT_BASE + i, obj);
            if (!quiet) {
                DWORD slotptr = gfx + GFX_LIGHT_SLOTS +
                                (DWORD)(LIGHT_SLOT_BASE + i) * 4;
                int ok = (!IsBadReadPtr((void*)slotptr, 4) &&
                          *(DWORD*)slotptr == obj) ? 1 : 0;
                snprintf(lbuf, sizeof(lbuf),
                         "  LIGHT%d: slot %d src=%s pos=(%d,%d,%d) col=(%d,%d,%d) vis=%d ok=%d",
                         i, LIGHT_SLOT_BASE + i,
                         g_light_src[i] ? "POINT" : "S3",
                         (int)g_light_pos[i][0], (int)g_light_pos[i][1],
                         (int)g_light_pos[i][2],
                         (int)(g_light_col[i][0] * 100.0f),
                         (int)(g_light_col[i][1] * 100.0f),
                         (int)(g_light_col[i][2] * 100.0f),
                         vis, ok);
                log_mod(lbuf);
            }
        } else if (obj) {
            /* stale slot from a richer level: switch off, keep alive */
            *(BYTE*)((char*)obj + SO_VISIBLE) = 0;
            native_register(gfx, LIGHT_SLOT_BASE + i, obj);
        }
    }
    g_light_used = hi;
    g_job = 0;
}

/* Parse current level lights (S3 file lights, then POINTnn S1 refs as
 * white lights in remaining slots), raise build job. Runs in onGameUpdate,
 * ONCE per board (called from start_grid_cycle). */
static void start_lights(DWORD board) {
    char cur[MAX_PATH];
    int n, i;
    int have_file = 0;
    char nbuf[64];
    g_light_count = 0;
    g_light_vis = 1;
    cur[0] = '\0';
    if (!find_current_level_file(cur, sizeof(cur))) {
        log_mod("  LIGHT: level file unknown, S3 skipped");
        n = -1;
    } else {
        have_file = 1;
        n = read_level_lights(cur);
        if (n < 0) log_mod("  LIGHT: parse failed, S3 skipped");
    }
    /* POINTnn refs (white) fill whatever slots S3 left free */
    {
        char cbuf[64];
        snprintf(cbuf, sizeof(cbuf), "  LIGHT: %d POINT/LIGHT ref(s) in S1",
                 g_ppt_count);
        log_mod(cbuf);
    }
    for (i = 0; i < g_ppt_count && g_light_count < MAX_LIGHTS; i++) {
        int li = g_light_count;
        char pbuf[128];
        float mr = 1.0f, mg = 1.0f, mb = 1.0f;
        float rgb[3];
        int got = 0;
        g_light_pos[li][0] = g_ppt_x[i];
        g_light_pos[li][1] = g_ppt_y[i];
        g_light_pos[li][2] = g_ppt_z[i];
        if (have_file && read_geom_diffuse(cur, g_ppt_name[i], rgb)) {
            if (rgb[0] >= 0.0f && rgb[0] <= 10.0f &&
                rgb[1] >= 0.0f && rgb[1] <= 10.0f &&
                rgb[2] >= 0.0f && rgb[2] <= 10.0f) {
                mr = rgb[0];
                mg = rgb[1];
                mb = rgb[2];
                got = 1;
            }
        }
        g_light_col[li][0] = mr * LIGHT_INTENSITY;
        g_light_col[li][1] = mg * LIGHT_INTENSITY;
        g_light_col[li][2] = mb * LIGHT_INTENSITY;
        g_light_src[li] = 1;
        g_light_count++;
        snprintf(pbuf, sizeof(pbuf),
                 "  LIGHT: POINT ref %d %s at (%d,%d,%d) mat=(%d,%d,%d)%s",
                 i + 1, g_ppt_name[i],
                 (int)g_ppt_x[i], (int)g_ppt_y[i], (int)g_ppt_z[i],
                 (int)(mr * 100.0f), (int)(mg * 100.0f),
                 (int)(mb * 100.0f), got ? "" : " no-mat");
        log_mod(pbuf);
    }
    snprintf(nbuf, sizeof(nbuf), "  LIGHT: %d point light(s)", g_light_count);
    log_mod(nbuf);
    if (!g_light_count) return;
    g_job = 1;
    g_job_board = board;
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
    start_lights(board);
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
        resolve_grid_files();
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
        /* numeric order: GRID01->GRID02->... regardless of S1 list order */
        for (int si = 1; si < g_order_count; si++) {
            int key = g_order[si];
            int sj = si - 1;
            while (sj >= 0 && g_grid_num[g_order[sj]] > g_grid_num[key]) {
                g_order[sj + 1] = g_order[sj];
                sj--;
            }
            g_order[sj + 1] = key;
        }
        {
            char obuf[160];
            int oi = 0;
            oi += snprintf(obuf + oi, sizeof(obuf) - oi, "  GRID order:");
            for (int qi = 0; qi < g_order_count && oi < 140; qi++)
                oi += snprintf(obuf + oi, sizeof(obuf) - oi, " %s",
                               g_grid_digits[g_order[qi]]);
            log_mod(obuf);
        }
        g_current_grid = 0;   /* index into g_order */
        grid_show(board, g_spawned_objs[g_order[0]]);
        g_last_switch_tick = GetTickCount();
        g_prev_tick = g_last_switch_tick;
        g_cycle_started = true;
    } else {
        log_mod("  No GRID points found");
        /* stay dead until board change — retrying every frame would rescan
         * levels\ + reparse files continuously (extreme slowdown, log spam) */
        g_cycle_started = true;
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
        /* Seed testcube.MESHWORLD into levels\ if missing (fallback
         * currently disabled — kept for later). Silent. */
        if (GetFileAttributesA(src) != INVALID_FILE_ATTRIBUTES &&
            GetFileAttributesA(cand[0]) == INVALID_FILE_ATTRIBUTES &&
            GetFileAttributesA(cand[1]) == INVALID_FILE_ATTRIBUTES)
            CopyFileA(src, cand[0], FALSE);

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

    CustomButton btn2("BATTY_LIGHTS", "Batty Point Lights");
    btn2.defaultState = true;
    HBAPI(api).CreateToggleButton(btn2, (HamsterballAPI*)thisptr);

    CustomSlider s2("BATTY_LIGHT_RANGE", "Light Range", 400.0f);
    s2.lowerBound = 50.0f; s2.upperBound = 3000.0f; s2.stepSize = 10.0f; s2.decimalPlaces = 0;
    HBAPI(api).CreateSlider(s2, (HamsterballAPI*)thisptr);

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
    } else if (strcmp(id, "BATTY_LIGHTS") == 0) {
        g_lights_on = state;
        {
            char lbuf[48];
            snprintf(lbuf, sizeof(lbuf), "  LIGHT: toggle %s",
                     state ? "ON" : "OFF");
            log_mod(lbuf);
        }
        if (g_light_used || g_light_count) {
            g_job = 2;
            g_job_board = player_board();
        }
    }
}

static void __thiscall slider_change(void*, const char* id, float value) {
    if (strcmp(id, "BATTY_GRID_SPEED") == 0) g_speed = value;
    else if (strcmp(id, "BATTY_LIGHT_RANGE") == 0) {
        g_light_range = value < 10.0f ? 10.0f : value;
        if (g_light_used || g_light_count) {
            g_job = 2;
            g_job_board = player_board();
        }
    }
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
    if (g_light_used && board) {
        g_job = 3;
        g_job_board = board;
    }
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
            snprintf(sbuf, sizeof(sbuf), "SWITCH GRID%s->GRID%s (pts=%d preloaded=%d upd=%d rnd=%d skip=%d)",
                     g_grid_digits[g_order[old_ord]], g_grid_digits[g_order[new_ord]],
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
static void __thiscall event_collide(void*, void*, char* name) {
    char c0;
    if (!name || IsBadReadPtr(name, 12)) return;
    c0 = name[0];
    if ((c0 != 'E' && c0 != 'e') || name[1] != ':') return;
    /* E:LIGHTSOFF (11) / E:LIGHTSON (10): mirror native behavior */
    if (strncmp(name + 2, "LIGHTSOFF", 9) == 0) {
        if (g_light_vis) {
            g_light_vis = 0;
            log_mod("  LIGHT: E:LIGHTSOFF -> off");
            if (g_light_used || g_light_count) {
                g_job = 2;
                g_job_board = player_board();
            }
        }
    } else if (strncmp(name + 2, "LIGHTSON", 8) == 0) {
        if (!g_light_vis) {
            g_light_vis = 1;
            log_mod("  LIGHT: E:LIGHTSON -> on");
            if (g_light_used || g_light_count) {
                g_job = 2;
                g_job_board = player_board();
            }
        }
    }
}
static int g_reassert = 0;   /* frames since last slot re-assert */
static void __thiscall text_render(void*) {
    DWORD board;
    board = player_board();
    if (!board || IsBadReadPtr((void*)board, 0x4400)) return;
    if (g_job != 0) { service_light_job(board, 0); return; }
    /* retry wipes slots via ResetObjectSlots: re-assert ~every 2s */
    if ((g_light_count || g_light_used) && board == g_job_board) {
        g_reassert++;
        if ((g_reassert & 127) == 0) {
            g_job = 2;
            service_light_job(board, 1);
        }
    }
}
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