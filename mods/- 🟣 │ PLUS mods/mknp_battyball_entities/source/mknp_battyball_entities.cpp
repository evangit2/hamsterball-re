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
#include "entdefs.h"

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
#define MAX_LIGHTS          7        /* ACTIVE mod slots 1-7 (slot 0 = native
                                      * follower, never touched). D3D8 caps
                                      * at 8 simultaneous lights. */
#define MAX_LIGHT_PARSED    64       /* parsed pool: unlimited-ish, nearest
                                      * MAX_LIGHTS win a slot each frame */
#define MAX_PLAYERLIGHTS    4        /* REF:PlayerlightXX follow the ball */
#define LIGHT_SLOT_BASE     1        /* slots 1-3 are P2-P4 follower slots:
                                      * empty in solo; native-priority yield
                                      * (claim_slot) defers if one fills */
#define LIGHT_TYPE_POINT    1
#define LIGHT_INTENSITY     5.0f     /* default material-color multiplier */
#define LIGHT_OUTPUT_TRIM   10.0f    /* mat 1.0 -> emitter 10 at default
                                      * gain 5 (mat x gain x trim / 5) */
#define LIGHT_RANGE_SCALE   1.0f     /* felt range = shown range (0.5 hid
                                      * the pool: D3D hard-cuts beyond Range
                                      * and the ball runs 270-450 away) */
#define LIGHT_TEST_DX       0.0f     /* offset test done: glow is native */

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

/* Named entities (set jsonc ENTITIES pairs): persistent static spawns */
#define ENT_MAX_INST 64
static DWORD g_ent_objs[ENT_MAX_INST];
static int   g_ent_obj_def[ENT_MAX_INST];  /* def idx per instance */
static int   g_ent_obj_type[ENT_MAX_INST]; /* AI ctor type per instance (v1bg) */
static int   g_ent_obj_size[ENT_MAX_INST]; /* alloc size per instance (v1bg) */
static float g_ent_rx[ENT_MAX_INST];      /* REF rot X radians (v1bm) */
static float g_ent_ry[ENT_MAX_INST];      /* REF rot Y radians (v1bm) */
static float g_ent_rz[ENT_MAX_INST];      /* REF rot Z radians (v1bm) */
static int   g_ent_inst_count = 0;
/* Woodbridge behaviour state (per instance) */
static float g_ent_home_x[ENT_MAX_INST];
static float g_ent_home_y[ENT_MAX_INST];
static float g_ent_home_z[ENT_MAX_INST];
static float g_ent_cur[ENT_MAX_INST];      /* current Y offset (0 .. -50) */
static float g_ent_applied[ENT_MAX_INST];  /* last Y offset written + flagged */
static int   g_ent_near[ENT_MAX_INST];     /* proximity latch (edge logs) */
static DWORD g_ent_last_tick = 0;          /* dt clock for smooth motion */
#define ENT_POS_X 0x10D4   /* PopCylinder_ctor stores x,y,z here (0x436EE0) */
#define ENT_POS_Y 0x10D8
#define ENT_DIRTY 0x10E4   /* BYTE: 1 = reposition (native update consumes) */
#define WB_RADIUS 150.0f   /* trigger distance (3D, ball center to ref point) */
#define WB_DROP   50.0f    /* sink depth when player near */
#define WB_SPEED  100.0f   /* units/sec (0.5s down, 0.5s up) */

/* Race slot 1-15 (MAKYUNI order). Set in find_grid_points from the board
 * vtable. Each LevelBoard_X_ctor writes its vtable to board+0x0
 * (Ghidra-verified per ctor); files/renames never change it. Unknown = 0. */
static int   g_race_slot = 0;

static int race_slot_from_vtable(DWORD vt) {
    switch (vt) {
        case 0x4D04A8: return 1;    /* WarmUp */
        case 0x4D1098: return 2;    /* Beginner */
        case 0x4D05A0: return 3;    /* Intermediate */
        case 0x4D0890: return 4;    /* Dizzy */
        case 0x4D0A08: return 5;    /* Tower */
        case 0x4D11A0: return 6;    /* Up */
        case 0x4D1DF0: return 7;    /* Neon */
        case 0x4D0B00: return 8;    /* Expert */
        case 0x4D0BC0: return 9;    /* Odd */
        case 0x4D0E78: return 10;   /* Toob */
        case 0x4D0D38: return 11;   /* Wobbly */
        case 0x4D1F90: return 12;   /* Glass */
        case 0x4D0FC8: return 13;   /* Sky */
        case 0x4D12B0: return 14;   /* Master */
        case 0x4D21C0: return 15;   /* Impossible */
        default: return 0;
    }
}

/* Active level / board the current cycle belongs to */
static DWORD g_active_board = 0;
static int   g_board_ready_delay = 0;   /* frames to wait for level build after board change */

/* Native point lights (S3 DISTANTLIGHTs -> SceneObjects in gfx slots) */
static DWORD g_light_objs[MAX_LIGHTS];   /* never freed, reused per level */
static float g_light_pos[MAX_LIGHT_PARSED][3]; /* game coords */
static float g_light_col[MAX_LIGHT_PARSED][3];
static float g_light_mat[MAX_LIGHT_PARSED][3]; /* raw material ratio (pre-gain) */
static int   g_light_src[MAX_LIGHT_PARSED];    /* 0 = S3 file light, 1 = POINT ref */
static float g_light_rng[MAX_LIGHT_PARSED];    /* per-light felt range */
static int   g_light_rngfix[MAX_LIGHT_PARSED]; /* 1 = (Rnnn) suffix, slider-proof */
static int   g_light_follow[MAX_LIGHT_PARSED]; /* 1 = Playerlight: snaps to ball */
static int   g_light_yield[MAX_LIGHTS];  /* 1 = slot native-held, deferred */
static int   g_light_count = 0;          /* parsed (clamped to MAX_LIGHT_PARSED) */
static int   g_light_used = 0;           /* slots currently registered */
static int   g_active_map[MAX_LIGHTS];   /* parsed idx per slot, -1 = empty */
static int   g_active_n = 0;             /* slots currently assigned */
static int   g_prev_map[MAX_LIGHTS] = { -1, -1, -1, -1, -1, -1, -1 };
static int   g_light_vis = 1;            /* 0 while LIGHTSOFF */
static bool  g_lights_on = true;         /* BATTY_LIGHTS toggle */
static float g_light_range = 400.0f * LIGHT_RANGE_SCALE; /* felt, slider x0.5 */
static float g_light_intensity = LIGHT_INTENSITY; /* BATTY_LIGHT_INTENSITY */
static int   g_job = 0;                  /* text_render job: 0 none 1 build 2 refresh */
static DWORD g_job_board = 0;

/* POINTLIGHTxx S1 ref positions (case-insensitive) for ref-driven lights */
static float g_ppt_x[MAX_LIGHT_PARSED];
static float g_ppt_y[MAX_LIGHT_PARSED];
static float g_ppt_z[MAX_LIGHT_PARSED];
static char  g_ppt_name[MAX_LIGHT_PARSED][32];  /* S1 ref name (mesh lookup key) */
static float g_ppt_rng[MAX_LIGHT_PARSED];       /* (Rnnn) suffix range, 0 = global */
static int   g_ppt_follow[MAX_LIGHT_PARSED];    /* 1 = REF:PlayerlightXX */
static int   g_pl_found = 0;                    /* playerlights kept (max 4) */
static int   g_ppt_count = 0;
static int   g_ppt_total = 0;             /* refs found (kept capped) */

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
    g_ppt_total = 0;
    g_pl_found = 0;
    g_s1_hash = 2166136261u;
    g_s1_count = 0;
    DWORD sceneobj = get_sceneobj(board);
    if (!sceneobj) { log_mod("  GRID: sceneobj=NULL"); return 0; }

    DWORD level = get_level(board);
    g_race_slot = 0;
    if (!IsBadReadPtr((void*)board, 4)) {
        g_race_slot = race_slot_from_vtable(*(DWORD*)board);
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
        char dbuf[160];
        snprintf(dbuf, sizeof(dbuf),
                 "  S1 board=0x%X lvl=0x%X sc=0x%X cnt=%d data=0x%X slot=%d",
                 board, level, sceneobj, s1_count, (DWORD)s1_data, g_race_slot);
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
         (g_grid_count < MAX_GRID_POINTS || g_ppt_count < MAX_LIGHT_PARSED); i++) {
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
            if (is_light_ref(name)) {
                int is_pl = (nc_istrstr(name, "PLAYERLIGHT") != NULL);
                if (is_pl && g_pl_found >= MAX_PLAYERLIGHTS) {
                    g_ppt_total++;   /* over-budget: counted, not kept */
                    continue;
                }
                g_ppt_total++;
                if (g_ppt_count < MAX_LIGHT_PARSED) {
                int pi = g_ppt_count;
                g_ppt_follow[pi] = is_pl;
                if (is_pl) g_pl_found++;
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
                /* (Rnnn) suffix: per-light range; strip so mesh lookup hits */
                g_ppt_rng[pi] = 0.0f;
                {
                    int qi = 0;
                    while (g_ppt_name[pi][qi]) {
                        if (g_ppt_name[pi][qi] == '(' &&
                            (g_ppt_name[pi][qi + 1] == 'R' ||
                             g_ppt_name[pi][qi + 1] == 'r')) {
                            int qj = qi + 2;
                            int rv = 0;
                            int nd = 0;
                            while (qj < 31 && g_ppt_name[pi][qj] >= '0' &&
                                   g_ppt_name[pi][qj] <= '9') {
                                rv = rv * 10 +
                                     (g_ppt_name[pi][qj] - '0');
                                qj++;
                                nd++;
                            }
                            if (nd > 0 && rv >= 10 && rv <= 10000)
                                g_ppt_rng[pi] = (float)rv;
                            g_ppt_name[pi][qi] = '\0';
                            break;
                        }
                        qi++;
                    }
                }
                g_ppt_count++;
                }
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
            if (is_light_ref((const char*)entry)) {
                int is_pl = (nc_istrstr((const char*)entry, "PLAYERLIGHT") != NULL);
                g_ppt_total++;
                if (is_pl && g_pl_found >= MAX_PLAYERLIGHTS) continue;   /* over-budget */
                if (g_ppt_count < MAX_LIGHT_PARSED) {
                g_ppt_follow[g_ppt_count] = is_pl;
                if (is_pl) g_pl_found++;
                g_ppt_x[g_ppt_count] = *(float*)(entry + S1ENTRY_POS_X);
                g_ppt_y[g_ppt_count] = *(float*)(entry + S1ENTRY_POS_Y);
                g_ppt_z[g_ppt_count] = *(float*)(entry + S1ENTRY_POS_Z);
                g_ppt_name[g_ppt_count][0] = '\0';   /* no clean name */
                g_ppt_rng[g_ppt_count] = 0.0f;
                g_ppt_count++;
                }
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
/* Forward: defined in entyaw.h (included below, needs board defines first). */
static int ent_rotate_mesh3(DWORD mw, float rx, float ry, float rz);
static void* create_grid_cube(DWORD board, float px, float py, float pz,
                              int grid_num, const char* mesh_path,
                              float erx, float ery, float erz) {
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

    /* v1bn: NO mesh-source rotation (v1bm baked verts the draw path never
     * reads: only 1 vert found, zero visual). Rotation now happens per-frame
     * at draw via pop_rot_install below (Mousetrap-native compose). */
    (void)erx; (void)ery; (void)erz;

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

/* v1bp: DIAGNOSTIC (behavior-neutral). v1bo drew via slot-21 CallRender and
 * the 4 rotated bridges went invisible: the per-object matrix the draw
 * uses is written by slots 21/22 through renderLevel (obj+0x434, set by
 * ctor; BeginFrame does NOT reset the device matrix). This build wraps
 * slots 21+22 only (11/18 stock, all 5 bridges draw normally) and logs
 * the renderLevel matrix before/after the original, so v1bq can write
 * rotation into exactly the right place. */
#define POPCYL_SLOT21     0x45DF90   /* SceneObject_CallRender __fastcall */
#define POPCYL_SLOT22     0x45DF80   /* SceneObject_CallUpdate __fastcall */
#define POPCYL_UPDATE     0x43DED0   /* slot 11, motion only, __fastcall */
#define POPCYL_VTABLE     0x4D58F0
#define POPCYL_VT_COPY_SZ 0x400
typedef int (__fastcall *pop_upd_t)(void*);  /* orig slot-11 motion */
/* v1bv: Z->X swap per MAKYUNI (angle now pitches about X, model-space
 * M = Rx*T(Q), slot-11 every frame, absolute, zero device touches).
 * v1bw: mirror (v1bv pitched the wrong way): Rx(-a), rows
 * ([1,0,0],[0,c,-s],[0,s,c]); zone X slot negated to match. */
static float ent_sin_deg(float deg) {
    float x = deg * 0.01745329252f;
    float x2;
    while (x > 3.14159265f) x -= 6.28318531f;
    while (x < -3.14159265f) x += 6.28318531f;
    x2 = x * x;
    return x * (1.0f - x2 / 6.0f + x2 * x2 / 120.0f -
                x2 * x2 * x2 / 5040.0f + x2 * x2 * x2 * x2 / 362880.0f);
}
static float ent_cos_deg(float deg) {
    return ent_sin_deg(deg + 90.0f);
}

static void pop_write_matrix(DWORD obj) {
    int i;
    float hx, hy, hz, angdeg, c, s0, qy;
    DWORD rl;
    float* m;
    hx = hy = hz = 0.0f; angdeg = 0.0f;
    for (i = 0; i < g_ent_inst_count; i++) {
        if (g_ent_objs[i] == obj) {
            hx = g_ent_home_x[i]; hy = g_ent_home_y[i]; hz = g_ent_home_z[i];
            angdeg = g_ent_ry[i] * 57.29578f;  /* v1bv: ref angle -> X */
            break;
        }
    }
    if (!obj || IsBadReadPtr((void*)obj, 0x10E0)) return;
    rl = *(DWORD*)(obj + 0x434);
    if (!rl || IsBadReadPtr((void*)rl, 68)) return;
    c = ent_cos_deg(angdeg);
    s0 = ent_sin_deg(angdeg);
    qy = *(float*)(obj + 0x10D8);  /* live sunk Y */
    (void)hy;
    m = (float*)(rl + 4);
    m[0] = 1.0f; m[1] = 0.0f; m[2] = 0.0f; m[3] = 0.0f;
    m[4] = 0.0f; m[5] = c;    m[6] = -s0;  m[7] = 0.0f;
    m[8] = 0.0f; m[9] = s0;   m[10] = c;   m[11] = 0.0f;
    m[12] = hx;  m[13] = qy;  m[14] = hz;  m[15] = 1.0f;
}

static int __fastcall pop_rot_update(void* obj) {
    int rc = ((pop_upd_t)POPCYL_UPDATE)(obj);  /* native motion first */
    pop_write_matrix((DWORD)obj);              /* actuate every frame */
    return rc;
}

typedef void (__thiscall *slot21_t)(void*, void*);
typedef void (__fastcall *slot22_t)(void*);
/* pop_upd_t declared above (used by pop_rot_update) */

static DWORD g_pop_vt_copy[POPCYL_VT_COPY_SZ / 4];
static int g_pop_vt_ready = 0;
/* last-logged matrix per obj (change-only logging): small ring */
static DWORD g_mlog_obj[32];
static DWORD g_mlog_m0[32][4];
static int g_mlog_n = 0;

static void pop_mlog(DWORD obj, const char* tag) {
    DWORD rl;
    float* m;
    DWORD w0, w1, w2, w3;
    char lb[192];
    int L, i, slot;
    if (!obj || IsBadReadPtr((void*)obj, 0x440)) return;
    rl = *(DWORD*)(obj + 0x434);
    if (!rl || IsBadReadPtr((void*)rl, 68)) {
        snprintf(lb, sizeof(lb), "  ENT mlog %s obj=0x%X rl=0x%X BAD",
                 tag, obj, rl);
        log_mod(lb);
        return;
    }
    m = (float*)(rl + 4);
    w0 = ((DWORD*)m)[0]; w1 = ((DWORD*)m)[1];
    w2 = ((DWORD*)m)[2]; w3 = ((DWORD*)m)[3];
    slot = -1;
    for (i = 0; i < g_mlog_n; i++) {
        if (g_mlog_obj[i] == obj) { slot = i; break; }
    }
    if (slot >= 0 && g_mlog_m0[slot][0] == w0 && g_mlog_m0[slot][1] == w1 &&
        g_mlog_m0[slot][2] == w2 && g_mlog_m0[slot][3] == w3)
        return;  /* unchanged row0 = skip */
    if (slot < 0 && g_mlog_n < 32) { slot = g_mlog_n++; g_mlog_obj[slot] = obj; }
    if (slot >= 0) {
        g_mlog_m0[slot][0] = w0; g_mlog_m0[slot][1] = w1;
        g_mlog_m0[slot][2] = w2; g_mlog_m0[slot][3] = w3;
    }
    L = 0;
    L += snprintf(lb + L, sizeof(lb) - L, "  ENT mlog %s obj=0x%X rl=0x%X m=",
                  tag, obj, rl);
    for (i = 0; i < 16 && L + 9 < (int)sizeof(lb); i++)
        L += snprintf(lb + L, sizeof(lb) - L, "%X%c",
                      ((DWORD*)m)[i], i == 15 ? '\0' : ' ');
    log_mod(lb);
}

static void __thiscall wrap_slot21(void* obj, void* t) {
    pop_mlog((DWORD)obj, "s21-pre");
    ((slot21_t)POPCYL_SLOT21)(obj, t);
    pop_write_matrix((DWORD)obj);  /* v1bs: rotation landing */
    pop_mlog((DWORD)obj, "s21-post");
}

/* v1bq: slot-22 takes NO stack arg natively (game calls (ECX=obj), clean-0;
 * v1bp pushed t + RET 8 = stack imbalance = crash 0001:00078EDD).
 * Wrapper + orig call are both bare fastcall now. Slot-21 keeps (obj,t). */
static void __fastcall wrap_slot22(void* obj) {
    pop_mlog((DWORD)obj, "s22-pre");
    ((slot22_t)POPCYL_SLOT22)(obj);
    pop_mlog((DWORD)obj, "s22-post");
}

/* v1bs: slot-11 compose removed (device-matrix poison, see note above). */

/* v1bt: vtable copy: slot 11 = motion+write (every frame), slots 21+22
 * call orig, log, then overwrite renderLevel+4. Slot 18 STOCK. */
static void pop_rot_install(DWORD obj, float rx, float ry, float rz) {
    char ibuf[128];
    DWORD m438[4];
    if (!obj || IsBadReadPtr((void*)obj, 0x10D0)) return;
    if (!g_pop_vt_ready) {
        memcpy(g_pop_vt_copy, (void*)POPCYL_VTABLE, POPCYL_VT_COPY_SZ);
        g_pop_vt_copy[11] = (DWORD)pop_rot_update;
        g_pop_vt_copy[21] = (DWORD)wrap_slot21;
        g_pop_vt_copy[22] = (DWORD)wrap_slot22;
        g_pop_vt_ready = 1;
    }
    *(DWORD*)obj = (DWORD)g_pop_vt_copy;
    /* v1bs: snapshot obj+0x438 (candidate second matrix?) at install */
    if (!IsBadReadPtr((void*)(obj + 0x438), 16)) {
        int k;
        for (k = 0; k < 4; k++) m438[k] = *(DWORD*)(obj + 0x438 + k * 4);
        snprintf(ibuf, sizeof(ibuf), "  ENT rotdiag: obj=0x%X rl=0x%X m438=%X %X %X %X",
                 obj, *(DWORD*)(obj + 0x434),
                 m438[0], m438[1], m438[2], m438[3]);
    } else {
        snprintf(ibuf, sizeof(ibuf), "  ENT rotdiag: obj=0x%X rl=0x%X m438=BAD",
                 obj, *(DWORD*)(obj + 0x434));
    }
    log_mod(ibuf);
    if (rx > 0.000001f || rx < -0.000001f) {
        snprintf(ibuf, sizeof(ibuf), "  ENT rotdiag: obj=0x%X rx nonzero, yaw-only",
                 obj);
        log_mod(ibuf);
    }
    (void)ry; (void)rz;
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

/* Forward: defined in aibeh.h (included below, needs board defines first). */
static void aibeh_remove(DWORD board, DWORD obj, int type, unsigned size);
/* Forward: defined in entyaw.h (included below). */
static int ent_rotate_mesh3(DWORD mw, float rx, float ry, float rz);
/* Forward: level-file matcher used by the S1-rot fallback. */
static int find_current_level_file(char* out, unsigned cap);

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
    for (i = 0; i < ENT_MAX_INST; i++) {
        if (g_ent_objs[i]) {
            aibeh_remove(board, g_ent_objs[i], g_ent_obj_type[i],
                         (unsigned)g_ent_obj_size[i]);
            g_ent_objs[i] = 0;
            g_ent_obj_type[i] = 0;
            g_ent_obj_size[i] = 0;
            g_ent_rx[i] = 0.0f;
            g_ent_ry[i] = 0.0f;
            g_ent_rz[i] = 0.0f;
        }
    }
    g_ent_inst_count = 0;
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

/* aibeh.h needs board defines + g_op_new/g_append/g_remove + get_level +
 * log_mod, so it is included here (not at the top with the other headers). */
#include "aibeh.h"
/* entyaw.h needs the same (IsBadReadPtr, log_mod, gm_read_file, free). */
#include "entyaw.h"
/* entarea.h needs gridmesh (GmCur), entdefs (def tables), log_mod. */
#include "entarea.h"
#include "entsnd.h"    /* v1cb: creak acquire/frame (needs log_mod) */

/* ═══════════════════════════════════════════════════════════════════════════
 * Named entities: set jsonc pairs -> S1 REF:<Name> -> Levels/<mesh> spawn.
 * v1bg: behaviour selects the native ctor (AI list verbatim); static types
 * use the PopCylinder chain, shown once at level start, never cycled.
 * ═══════════════════════════════════════════════════════════════════════════ */
static int ent_match_def(const char* s1name) {
    int d;
    if (!s1name) return -1;
    for (d = 0; d < g_ent_count; d++) {
        if (g_ent_name[d][0] && nc_istrstr(s1name, g_ent_name[d]) != NULL)
            return d;
    }
    return -1;
}

/* v1bz: area-def match (checked BEFORE ent_match_def so area markers never
 * bind the Woodbridge def even when it is listed first in the set) */
static int ent_match_area(const char* s1name) {
    int d;
    if (!s1name) return -1;
    for (d = 0; d < g_ent_count; d++) {
        if (g_ent_area[d] && g_ent_name[d][0] &&
            nc_istrstr(s1name, g_ent_name[d]) != NULL)
            return d;
    }
    return -1;
}

static void scan_spawn_entities(DWORD board) {
    DWORD sceneobj;
    DWORD level;
    DWORD s1_list;
    int s1_count;
    DWORD* s1_data;
    int i;
    char levelfile[MAX_PATH];   /* v1bk: level file for S1-rot fallback */
    int have_levelfile = 0;
    for (i = 0; i < ENT_MAX_INST; i++) {
        g_ent_objs[i] = 0;
        g_ent_obj_type[i] = 0;
        g_ent_obj_size[i] = 0;
        g_ent_rx[i] = 0.0f;
        g_ent_ry[i] = 0.0f;
        g_ent_rz[i] = 0.0f;
    }
    g_ent_inst_count = 0;
    area_reset();   /* v1bz: clear gate quads every scan */
    load_entities_file();
    if (!g_ent_count) return;
    if (!g_levels_dir[0]) {
        log_mod("  ENT: levels dir unknown, skip");
        return;
    }
    sceneobj = get_sceneobj(board);
    if (!sceneobj) { log_mod("  ENT: sceneobj=NULL"); return; }
    level = get_level(board);
    (void)level;
    s1_list = sceneobj + SCENEOBJ_S1_OFF;
    if (IsBadReadPtr((void*)(s1_list + ALIST_COUNT), 4)) return;
    s1_count = *(int*)(s1_list + ALIST_COUNT);
    s1_data = NULL;
    if (!IsBadReadPtr((void*)(s1_list + ALIST_ITEMS), 4))
        s1_data = *(DWORD**)(s1_list + ALIST_ITEMS);
    if (s1_count <= 0 || s1_count > 1000) return;
    if (!s1_data || IsBadReadPtr(s1_data, s1_count * 4)) return;
    {   /* v1bk: resolve level file once for S1-rot fallback (not per ref) */
        levelfile[0] = '\0';
        have_levelfile = find_current_level_file(levelfile, sizeof(levelfile));
    }
    {   /* v1bl diag: which file the S1-rot fallback reads (once per scan) */
        char ybuf[128];
        const char* src = have_levelfile ? levelfile : "NONE";
        int L = 0, k = 0;
        const char* pre = "  ENT yaw: levelfile=";
        while (pre[L]) { ybuf[L] = pre[L]; L++; }
        while (k < 100 && src[k] && L + 1 < (int)sizeof(ybuf)) {
            ybuf[L++] = src[k++];
        }
        ybuf[L] = '\0';
        log_mod(ybuf);
    }
    for (i = 0; i < s1_count && g_ent_inst_count < ENT_MAX_INST; i++) {
        DWORD entry = s1_data[i];
        const char* nm = NULL;
        char* ptr = NULL;
        int d;
        float px, py, pz;
        float erx = 0.0f, ery = 0.0f, erz = 0.0f; /* v1bm: REF rot triple */
        int yaw_src = 0;        /* 0 none, 1 DAT ROT_Y, 2 S1 file rot */
        char base[ENT_MESH_N];
        char noext[ENT_MESH_N];
        char ctor[MAX_PATH];
        char abs[MAX_PATH];
        char ebuf[160];
        void* obj;
        if (!entry || entry < 0x10000) continue;
        if (IsBadReadPtr((void*)entry, 16)) continue;
        ptr = *(char**)(entry + S1ENTRY_NAME);
        if (ptr && !IsBadReadPtr(ptr, 5)) nm = ptr;
        else nm = (const char*)entry;   /* inline name fallback */
        if (ent_match_area(nm) >= 0) {  /* v1bz: zone marker, no instance */
            g_area_refs_seen++;
            continue;
        }
        d = ent_match_def(nm);
        if (d < 0) continue;
        px = *(float*)(entry + S1ENTRY_POS_X);
        py = *(float*)(entry + S1ENTRY_POS_Y);
        pz = *(float*)(entry + S1ENTRY_POS_Z);
        {   /* v1bm: REF rotation (DAT ROT_Y deg -> Y, else S1 file triple) */
            float yaw_deg = 0.0f;
            if (ent_dat_yaw_deg(nm, &yaw_deg)) {
                ery = yaw_deg * 0.01745329252f;
                yaw_src = 1;
            } else if (have_levelfile &&
                       ent_file_rot(levelfile, nm, px, py, pz,
                                    &erx, &ery, &erz)) {
                yaw_src = 2;
            } else if (have_levelfile) {
                /* v1bl diag: file present but this ref unmatched */
                const char* s1h = nm ? nm : "?";
                int L = 0, k = 0;
                snprintf(ebuf, sizeof(ebuf), "  ENT %s: filerot none s1=",
                         g_ent_name[d]);
                while (ebuf[L]) L++;
                while (k < 24 && s1h[k] && L + 1 < (int)sizeof(ebuf)) {
                    ebuf[L++] = s1h[k++];
                }
                ebuf[L] = '\0';
                log_mod(ebuf);
            }
            if (yaw_src &&
                ((erx > 0.000001f || erx < -0.000001f) ||
                 (ery > 0.000001f || ery < -0.000001f) ||
                 (erz > 0.000001f || erz < -0.000001f))) {
                snprintf(ebuf, sizeof(ebuf), "  ENT %s: rot %d,%d,%ddeg (%s)",
                         g_ent_name[d], (int)(erx * 57.29578f),
                         (int)(ery * 57.29578f), (int)(erz * 57.29578f),
                         yaw_src == 1 ? "DAT" : "file");
                log_mod(ebuf);
            }
        }
        {   /* v1bg: no-mesh native types skip file checks entirely */
            int et0 = aibeh_type(g_ent_beh[d]);
            if (!aibeh_needs_mesh(et0) && !aibeh_is_static(et0)) {
                DWORD app0 = 0;
                unsigned sz0;
                if (g_api) {
                    DWORD apv = (DWORD)HBAPI(g_api).GetApp();
                    if (apv && !IsBadReadPtr((void*)apv, 4)) app0 = apv;
                }
                obj = aibeh_spawn(board, px, py, pz, NULL, et0, app0);
                if (!obj) {
                    snprintf(ebuf, sizeof(ebuf), "  ENT %s: native spawn failed type=%d",
                             g_ent_name[d], et0);
                    log_mod(ebuf);
                    continue;
                }
                sz0 = aibeh_size(et0);
                aibeh_show(board, (DWORD)obj, et0);
                g_ent_objs[g_ent_inst_count] = (DWORD)obj;
                g_ent_obj_def[g_ent_inst_count] = d;
                g_ent_obj_type[g_ent_inst_count] = et0;
                g_ent_obj_size[g_ent_inst_count] = (int)sz0;
                g_ent_rx[g_ent_inst_count] = erx;
                g_ent_ry[g_ent_inst_count] = ery;
                g_ent_rz[g_ent_inst_count] = erz;
                g_ent_home_x[g_ent_inst_count] = px;
                g_ent_home_y[g_ent_inst_count] = py;
                g_ent_home_z[g_ent_inst_count] = pz;
                g_ent_cur[g_ent_inst_count] = 0.0f;
                g_ent_applied[g_ent_inst_count] = 0.0f;
                g_ent_near[g_ent_inst_count] = 0;
                g_ent_inst_count++;
                snprintf(ebuf, sizeof(ebuf),
                         "  ENT: spawned %s behaviour=%s type=%d at (%d,%d,%d) obj=0x%X",
                         g_ent_name[d], g_ent_beh[d], et0,
                         (int)px, (int)py, (int)pz, (DWORD)obj);
                log_mod(ebuf);
                continue;
            }
        }
        ent_basename(g_ent_mesh[d], base, sizeof(base));
        if (!base[0]) {
            snprintf(ebuf, sizeof(ebuf), "  ENT %s: empty mesh, skip",
                     g_ent_name[d]);
            log_mod(ebuf);
            continue;
        }
        {
            int hasdot = 0;
            int k = 0;
            while (base[k]) { if (base[k] == '.') hasdot = 1; k++; }
            if (hasdot) snprintf(abs, sizeof(abs), "%s%s", g_levels_dir, base);
            else snprintf(abs, sizeof(abs), "%s%s.MESHWORLD", g_levels_dir, base);
        }
        if (GetFileAttributesA(abs) == INVALID_FILE_ATTRIBUTES) {
            snprintf(ebuf, sizeof(ebuf), "  ENT %s: MISSING %s",
                     g_ent_name[d], base);
            log_mod(ebuf);
            continue;
        }
        if (validate_grid_file(abs) < 1) {
            snprintf(ebuf, sizeof(ebuf), "  ENT %s: BAD %s",
                     g_ent_name[d], base);
            log_mod(ebuf);
            continue;
        }
        ent_strip_ext(base, noext, sizeof(noext));
        snprintf(ctor, sizeof(ctor), "levels\\%s", noext);
        {   /* v1bg: behaviour selects static fallback or native ctor */
            int etype = aibeh_type(g_ent_beh[d]);
            unsigned esize = 0;
            if (aibeh_is_static(etype)) {
                obj = create_grid_cube(board, px, py, pz, 900 + d, ctor,
                                       erx, ery, erz);
                if (!obj) {
                    snprintf(ebuf, sizeof(ebuf), "  ENT %s: spawn failed",
                             g_ent_name[d]);
                    log_mod(ebuf);
                    continue;
                }
                esize = POPCYLINDER_SIZE;
                grid_show(board, (DWORD)obj);
                /* v1bo: install compose-draw for X/Y only (rz has no
                 * native draw op; rz-only refs log + skip) */
                if ((erx > 0.000001f || erx < -0.000001f) ||
                    (ery > 0.000001f || ery < -0.000001f))
                    pop_rot_install((DWORD)obj, erx, ery, erz);
                else if (erz > 0.000001f || erz < -0.000001f) {
                    snprintf(ebuf, sizeof(ebuf),
                             "  ENT %s: rz-only, no native Z draw path",
                             g_ent_name[d]);
                    log_mod(ebuf);
                }
            } else {
                void* nmesh;
                void* mloaded;
                DWORD app_for_spawn = 0;
                DWORD gfx_for_spawn = 0;
                if (g_api) {
                    DWORD appv = (DWORD)HBAPI(g_api).GetApp();
                    if (appv && !IsBadReadPtr((void*)appv, 4)) {
                        app_for_spawn = appv;
                        gfx_for_spawn = *(DWORD*)(appv + APP_GFX_DEVICE);
                        if (!gfx_for_spawn || IsBadReadPtr((void*)gfx_for_spawn, 4))
                            gfx_for_spawn = 0;
                    }
                }
                if (!gfx_for_spawn) {
                    snprintf(ebuf, sizeof(ebuf), "  ENT %s: no gfx, skip type=%d",
                             g_ent_name[d], etype);
                    log_mod(ebuf);
                    continue;
                }
                nmesh = g_op_new(MESHWORLD_SIZE);
                if (!nmesh) continue;
                memset(nmesh, 0, MESHWORLD_SIZE);
                mloaded = g_mw_ctor(nmesh, (void*)gfx_for_spawn, ctor);
                if (!mloaded) {
                    snprintf(ebuf, sizeof(ebuf), "  ENT %s: mesh load failed",
                             g_ent_name[d]);
                    log_mod(ebuf);
                    continue;
                }
                obj = aibeh_spawn(board, px, py, pz, nmesh, etype, app_for_spawn);
                if (!obj) {
                    snprintf(ebuf, sizeof(ebuf), "  ENT %s: native spawn failed type=%d",
                             g_ent_name[d], etype);
                    log_mod(ebuf);
                    continue;
                }
                esize = aibeh_size(etype);
                aibeh_show(board, (DWORD)obj, etype);
            }
            g_ent_obj_type[g_ent_inst_count] = etype;
            g_ent_obj_size[g_ent_inst_count] = (int)esize;
        }
        g_ent_objs[g_ent_inst_count] = (DWORD)obj;
        g_ent_obj_def[g_ent_inst_count] = d;
        g_ent_rx[g_ent_inst_count] = erx;
        g_ent_ry[g_ent_inst_count] = ery;
        g_ent_rz[g_ent_inst_count] = erz;
        g_ent_home_x[g_ent_inst_count] = px;
        g_ent_home_y[g_ent_inst_count] = py;
        g_ent_home_z[g_ent_inst_count] = pz;
        g_ent_cur[g_ent_inst_count] = 0.0f;
        g_ent_applied[g_ent_inst_count] = 0.0f;
        g_ent_near[g_ent_inst_count] = 0;
        g_ent_inst_count++;
        snprintf(ebuf, sizeof(ebuf),
                 "  ENT: spawned %s behaviour=%s type=%d at (%d,%d,%d) obj=0x%X",
                 g_ent_name[d], g_ent_beh[d], g_ent_obj_type[g_ent_inst_count - 1],
                 (int)px, (int)py, (int)pz, (DWORD)obj);
        log_mod(ebuf);
    }
    {
        char cbuf[64];
        snprintf(cbuf, sizeof(cbuf), "  ENT: %d instance(s) active",
                 g_ent_inst_count);
        log_mod(cbuf);
    }
    {   /* v1bz: area quads from the level file (one S6 walk per area def) */
        int ad;
        for (ad = 0; ad < g_ent_count; ad++) {
            if (!g_ent_area[ad]) continue;
            if (have_levelfile)
                area_scan_file(levelfile, g_ent_name[ad], g_ent_vis[ad]);
            else
                log_mod("  AREA: no level file, gate open");
        }
        {
            char abuf[96];
            snprintf(abuf, sizeof(abuf),
                     "  AREA: %d S1 marker ref(s), %d quad(s)%s",
                     g_area_refs_seen, g_area_count,
                     g_area_count ? ", gate armed" : ", gate open");
            log_mod(abuf);
        }
    }
}

/* Per-frame entity behaviours. Woodbridge: sinks to home-low_Y while the
 * ball is inside the proximity ellipsoid, rises back to home when it
 * leaves. low_Y/speed_Y/proximity/proximity_scaleX/Y/Z come from the set
 * jsonc per def (defaults 50 / 0.5s full travel / 150 / 1/1/1). speed_Y =
 * seconds for full home-low travel. Y is measured from fixed home, so
 * bridge motion can never detune its own latch (v1bh flicker bug).
 * Move = write obj+0x10D8 + BYTE +0x10E4=1; native update (0x43DED0)
 * rebuilds Timer + slots 21/22 reposition render+collision absolutely
 * (0x46FBB0 fstp = set-from-source, repeat-safe). Flag set ONLY on
 * changed frames; pause freezes (no advance, no write). */
static int ent_is_woodbridge(int di) {
    if (di < 0 || di >= g_ent_count) return 0;
    if (g_ent_area[di]) return 0;   /* v1bz: gate defs never instances */
    if (!g_ent_beh[di][0]) return 0;
    return nc_istrstr(g_ent_beh[di], "Woodbridge") != NULL;
}

static void entity_frame(DWORD board) {
    DWORD now;
    float dt;
    void* b;
    float bx, by, bz;
    int has_ball;
    int wb_gate;   /* v1bz: Woodbridge_area gate (1 = proximity runs) */
    float frav[ENT_MAX_DEFS];  /* v1ce: per-def travel (creak volume) */
    int fi;
    int i;
    if (!board || IsBadReadPtr((void*)board, 0x4400)) return;
    if (!g_ent_inst_count) return;
    now = GetTickCount();
    if (!g_ent_last_tick) g_ent_last_tick = now;
    /* pause gate: freeze motion, keep clock (no time-jump on resume) */
    if (!IsBadReadPtr((void*)(board + BOARD_PAUSED), 4) &&
        *(int*)(board + BOARD_PAUSED)) {
        g_ent_last_tick = now;
        return;
    }
    dt = (float)(now - g_ent_last_tick) / 1000.0f;
    g_ent_last_tick = now;
    if (dt < 0.0f) dt = 0.0f;
    if (dt > 0.25f) dt = 0.25f;
    b = g_api ? (void*)HBAPI(g_api).GetPlayer() : NULL;
    has_ball = (b && !IsBadReadPtr(b, 0x300)) ? 1 : 0;
    if (has_ball) {
        bx = *(float*)((char*)b + 0x164);
        by = *(float*)((char*)b + 0x168);
        bz = *(float*)((char*)b + 0x16C);
    } else {
        bx = by = bz = 0.0f;
    }
    wb_gate = area_gate(bx, bz, has_ball);  /* v1bz: closed => hold home */
    for (fi = 0; fi < ENT_MAX_DEFS; fi++) frav[fi] = 0.0f;
    snd_level_acquire(board);   /* v1ce: default ch + one custom/def */
    for (i = 0; i < g_ent_inst_count && i < ENT_MAX_INST; i++) {
        DWORD obj = g_ent_objs[i];
        float dx, dy, dz, dist, target, cur, step, diff;
        float prox, low, spd, rate, sx, sy, sz, rx, ry, rz;
        int di;
        char ebuf[128];
        if (!ent_is_woodbridge(g_ent_obj_def[i])) continue;
        if (!obj || IsBadReadPtr((void*)obj, 0x10E8)) continue;
        di = g_ent_obj_def[i];
        prox = (di >= 0 && di < ENT_MAX_DEFS) ? g_ent_prox[di] : 150.0f;
        low = (di >= 0 && di < ENT_MAX_DEFS) ? g_ent_low[di] : 50.0f;
        spd = (di >= 0 && di < ENT_MAX_DEFS) ? g_ent_spd[di] : 0.5f;
        sx = (di >= 0 && di < ENT_MAX_DEFS) ? g_ent_sx[di] : 1.0f;
        sy = (di >= 0 && di < ENT_MAX_DEFS) ? g_ent_sy[di] : 1.0f;
        sz = (di >= 0 && di < ENT_MAX_DEFS) ? g_ent_sz[di] : 1.0f;
        if (prox <= 0.0f) prox = 150.0f;
        if (low <= 0.0f) low = 50.0f;
        if (sx <= 0.0f) sx = 1.0f;
        if (sy <= 0.0f) sy = 1.0f;
        if (sz <= 0.0f) sz = 1.0f;
        rate = (spd > 0.001f) ? (low / spd) : 1000000000.0f;
        rx = prox * sx;
        ry = prox * sy;
        rz = prox * sz;
        dx = has_ball ? (bx - g_ent_home_x[i]) : 999999.0f;
        /* v1bj: Y anchored to HOME (never home+cur: measuring against the
         * moving bridge was the v1bh flicker bug). Ellipsoid semi-axes
         * prox*sx/sy/sz; all 1 = sphere radius prox around home. */
        dy = has_ball ? (by - g_ent_home_y[i]) : 999999.0f;
        dz = has_ball ? (bz - g_ent_home_z[i]) : 999999.0f;
        {   /* v1bv: un-rotate offset by inverse ref rotation so the
             * zone follows the visual (same R; the stored ref angle
             * now drives the X slot, negated to match the v1bw mirror) */
            float ux = -(g_ent_ry[i]), uy = g_ent_rx[i], uz = g_ent_rz[i];
            if ((ux > 0.000001f || ux < -0.000001f) ||
                (uy > 0.000001f || uy < -0.000001f) ||
                (uz > 0.000001f || uz < -0.000001f)) {
                float ox, oy, oz;
                ent_unrot_pt3(dx, dy, dz, ux, uy, uz, &ox, &oy, &oz);
                dx = ox; dy = oy; dz = oz;
            }
        }
        dist = (dx * dx) / (rx * rx) + (dy * dy) / (ry * ry) +
               (dz * dz) / (rz * rz);
        /* inside the ellipsoid (< 1) the bridge sinks (gate open only) */
        if (has_ball && wb_gate && dist < 1.0f) {
            if (!g_ent_near[i]) {
                g_ent_near[i] = 1;
                snprintf(ebuf, sizeof(ebuf),
                         "  ENT Woodbridge%d: near -> sinking dy=%f.1 ry=%f.1", i, dy, ry);
                log_mod(ebuf);
            }
        } else if (g_ent_near[i]) {
            g_ent_near[i] = 0;
            snprintf(ebuf, sizeof(ebuf),
                     "  ENT Woodbridge%d: far -> rising dy=%f.1 ry=%f.1", i, dy, ry);
            log_mod(ebuf);
        }
        /* v1ca: distance falloff — nearest bridge rides nearest low_Y,
         * fading to 0 at the zone edge (dynamic, never constant) */
        if (g_ent_near[i]) {
            float f = 1.0f - dist;
            if (f < 0.0f) f = 0.0f;
            if (f > 1.0f) f = 1.0f;
            target = -low * f;
        } else {
            target = 0.0f;
        }
        cur = g_ent_cur[i];
        if (cur != target) {
            float oldc = cur;   /* v1cb: creak travel measurement */
            step = rate * dt;
            if (target < cur) {
                cur -= step;
                if (cur < target) cur = target;
            } else {
                cur += step;
                if (cur > target) cur = target;
            }
            g_ent_cur[i] = cur;
            if (low > 0.0f) {
                float dd = cur - oldc;
                int dddef = g_ent_obj_def[i];   /* v1cd: per-def sens */
                float sens = 1.0f;
                if (dd < 0.0f) dd = -dd;
                if (dddef >= 0 && dddef < ENT_MAX_DEFS &&
                    dddef < g_ent_count)
                    sens = g_ent_sndsens[dddef];
                if (dddef >= 0 && dddef < ENT_MAX_DEFS)
                    frav[dddef] += (dd / low) * sens;
            }
            if (cur == target) {
                snprintf(ebuf, sizeof(ebuf),
                         "  ENT Woodbridge%d: reached %d", i, (int)target);
                log_mod(ebuf);
            }
        }
        diff = g_ent_cur[i] - g_ent_applied[i];
        if (diff < 0.0f) diff = -diff;
        if (diff > 0.001f) {
            *(float*)((char*)obj + ENT_POS_Y) =
                g_ent_home_y[i] + g_ent_cur[i];
            *(BYTE*)((char*)obj + ENT_DIRTY) = 1;
            g_ent_applied[i] = g_ent_cur[i];
        }
    }
    for (fi = 0; fi < g_ent_count && fi < ENT_MAX_DEFS; fi++) {
        if (frav[fi] > 0.0f || g_snd_heard[fi])
            snd_creak_frame_d(fi, frav[fi]);   /* v1ce: per-def sound */
    }
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

/* Parse S3 lights + S4 ambient from a level file. Stores up to MAX_LIGHT_PARSED
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
        if (total < MAX_LIGHT_PARSED) {
            g_light_pos[total][0] = v[0];
            g_light_pos[total][1] = v[2];   /* file x,z,y -> game x,y,z */
            g_light_pos[total][2] = v[1];
            g_light_col[total][0] = v[6];
            g_light_col[total][1] = v[7];
            g_light_col[total][2] = v[8];
            g_light_mat[total][0] = v[6] / g_light_intensity;
            g_light_mat[total][1] = v[7] / g_light_intensity;
            g_light_mat[total][2] = v[8] / g_light_intensity;
            g_light_src[total] = 0;         /* S3 file light */
            g_light_rng[total] = g_light_range;
            g_light_rngfix[total] = 0;
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
 * material diffuse RGB (file mat+16/20/24) + emissive RGB (mat+48/52/56).
 * S6 order per MESHWORLD_BINARY_FORMAT_OFFICIAL: Ambient/Diffuse/Specular/
 * Emissive quads (+0/+16/+32/+48) + Power. Returns 1 found, 0 not. */
typedef struct {
    const char* needle;
    float rgb[3];
    float aux[4];   /* emissive rgb + emissive alpha (per-light gain) */
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
                    memcpy(&f->aux[0], matp + 48, 4);
                    memcpy(&f->aux[1], matp + 52, 4);
                    memcpy(&f->aux[2], matp + 56, 4);
                    memcpy(&f->aux[3], matp + 60, 4);
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
                             float* rgb, float* aux) {
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
    f.aux[0] = f.aux[1] = f.aux[2] = 0.0f;
    f.aux[3] = 1.0f;
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
    if (aux) {
        aux[0] = f.aux[0];
        aux[1] = f.aux[1];
        aux[2] = f.aux[2];
        aux[3] = f.aux[3];
    }
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

/* Native-priority slot claim. Registers obj ONLY if the slot is empty or
 * already ours; if a native owns it (party follower joining late, S3
 * re-register), we defer instead of clobbering. Returns 1 claimed. */
static int claim_slot(DWORD gfx, int slot, DWORD obj) {
    DWORD slotptr;
    DWORD cur;
    if (!gfx || IsBadReadPtr((void*)gfx, 0x800)) return 0;
    slotptr = gfx + GFX_LIGHT_SLOTS + (DWORD)slot * 4;
    if (IsBadReadPtr((void*)slotptr, 4)) return 0;
    cur = *(DWORD*)slotptr;
    if (cur != 0 && cur != obj) return 0;
    native_register(gfx, slot, obj);
    return 1;
}

/* vtable[1] of the light vtable (0x4D934C) is 0x46B650, a conditional
 * destroy-helper - NOT SetPosition (real SetPosition is vtable[2] =
 * 0x46B490, but direct +0x08 writes already verify ok=1, so never call
 * any vtable slot for position). Kept for reference; DO NOT USE. */
static void native_setpos(DWORD obj, float x, float y, float z) {
    (void)obj; (void)x; (void)y; (void)z;
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
    *(float*)((char*)obj + SO_RANGE_REAL) = g_light_rng[li];
    *(float*)((char*)obj + SO_RANGE) = g_light_rng[li];
    *(BYTE*)((char*)obj + SO_VISIBLE) = (BYTE)(vis ? 1 : 0);
}

/* Ball pos (ball+0x164/168/16C). 0 when no player (menus). */
#define BALL_POS_X 0x164
#define BALL_POS_Y 0x168
#define BALL_POS_Z 0x16C
static int light_ball_pos(float* ox, float* oy, float* oz) {
    void* b;
    if (!g_api || !ox || !oy || !oz) return 0;
    b = (void*)HBAPI(g_api).GetPlayer();
    if (!b || IsBadReadPtr(b, 0x200)) return 0;
    if (IsBadReadPtr((char*)b + BALL_POS_X, 12)) return 0;
    *ox = *(float*)((char*)b + BALL_POS_X);
    *oy = *(float*)((char*)b + BALL_POS_Y);
    *oz = *(float*)((char*)b + BALL_POS_Z);
    return 1;
}

/* Nearest-MAX_LIGHTS selection. Fills g_active_map (parsed idx per slot,
 * nearest first) + g_active_n. No ball yet -> first-N. Returns active n. */
static int light_select(void) {
    float bx = 0.0f, by = 0.0f, bz = 0.0f;
    int have_ball;
    int n, s, i;
    int best[MAX_LIGHTS];
    float bestd[MAX_LIGHTS];
    if (g_light_count <= 0) {
        for (s = 0; s < MAX_LIGHTS; s++) g_active_map[s] = -1;
        g_active_n = 0;
        return 0;
    }
    n = g_light_count;
    if (n > MAX_LIGHT_PARSED) n = MAX_LIGHT_PARSED;
    have_ball = light_ball_pos(&bx, &by, &bz);
    if (have_ball) {
        /* Playerlights ride the ball: d2=0, always win a slot */
        for (i = 0; i < n; i++) {
            if (g_light_follow[i]) {
                g_light_pos[i][0] = bx;
                g_light_pos[i][1] = by;
                g_light_pos[i][2] = bz;
            }
        }
    }
    for (s = 0; s < MAX_LIGHTS; s++) { best[s] = -1; bestd[s] = 0.0f; }
    if (!have_ball) {
        /* menus / no player: stable first-N window */
        int m = n < MAX_LIGHTS ? n : MAX_LIGHTS;
        for (s = 0; s < m; s++) { best[s] = s; g_active_map[s] = s; }
        for (; s < MAX_LIGHTS; s++) g_active_map[s] = -1;
        g_active_n = m;
        return m;
    }
    for (i = 0; i < n; i++) {
        float dx = g_light_pos[i][0] - bx;
        float dy = g_light_pos[i][1] - by;
        float dz = g_light_pos[i][2] - bz;
        float d2 = dx * dx + dy * dy + dz * dz;
        for (s = 0; s < MAX_LIGHTS; s++) {
            if (best[s] < 0 || d2 < bestd[s]) {
                int t;
                for (t = MAX_LIGHTS - 1; t > s; t--) {
                    best[t] = best[t - 1];
                    bestd[t] = bestd[t - 1];
                }
                best[s] = i;
                bestd[s] = d2;
                break;
            }
        }
    }
    {
        int m = n < MAX_LIGHTS ? n : MAX_LIGHTS;
        for (s = 0; s < m; s++) g_active_map[s] = best[s];
        for (; s < MAX_LIGHTS; s++) g_active_map[s] = -1;
        g_active_n = m;
        return m;
    }
}

/* Ensure slot s shows parsed idx pi (creates obj on demand). */
static void light_apply_slot(DWORD gfx, int s, int pi, int vis, int quiet) {
    DWORD obj;
    char lbuf[128];
    if (s < 0 || s >= MAX_LIGHTS || pi < 0 || pi >= MAX_LIGHT_PARSED) return;
    obj = g_light_objs[s];
    if (!obj || IsBadReadPtr((void*)obj, SCENEOBJECT_SIZE)) {
        obj = native_new_sceneobject(gfx);
        if (!obj || IsBadReadPtr((void*)obj, SCENEOBJECT_SIZE)) {
            if (!quiet) {
                snprintf(lbuf, sizeof(lbuf), "  LIGHT s%d: ctor failed", s);
                log_mod(lbuf);
            }
            g_light_objs[s] = 0;
            return;
        }
        g_light_objs[s] = obj;
    }
    write_light_fields(obj, pi, vis);
    native_setpos(obj, g_light_pos[pi][0], g_light_pos[pi][1],
                  g_light_pos[pi][2]);
    if (claim_slot(gfx, LIGHT_SLOT_BASE + s, obj)) {
        if (g_light_yield[s]) {
            g_light_yield[s] = 0;
            if (!quiet) {
                snprintf(lbuf, sizeof(lbuf), "  LIGHT s%d: slot %d reclaimed (P%d)",
                         s, LIGHT_SLOT_BASE + s, pi);
                log_mod(lbuf);
            }
        }
    } else if (!g_light_yield[s]) {
        g_light_yield[s] = 1;
        if (!quiet) {
            snprintf(lbuf, sizeof(lbuf), "  LIGHT s%d: slot %d native-held, yield (P%d)",
                     s, LIGHT_SLOT_BASE + s, pi);
            log_mod(lbuf);
        }
    }
    if (!quiet) {
        DWORD slotptr = gfx + GFX_LIGHT_SLOTS + (DWORD)(LIGHT_SLOT_BASE + s) * 4;
        int ok = (!IsBadReadPtr((void*)slotptr, 4) &&
                  *(DWORD*)slotptr == obj) ? 1 : 0;
        snprintf(lbuf, sizeof(lbuf),
                 "  LIGHT s%d: slot %d P%d src=%s pos=(%d,%d,%d) col=(%d,%d,%d) vis=%d ok=%d",
                 s, LIGHT_SLOT_BASE + s, pi,
                 (g_light_src[pi] == 2 ? "PLAYER" : (g_light_src[pi] ? "POINT" : "S3")),
                 (int)g_light_pos[pi][0], (int)g_light_pos[pi][1],
                 (int)g_light_pos[pi][2],
                 (int)(g_light_col[pi][0] * 100.0f),
                 (int)(g_light_col[pi][1] * 100.0f),
                 (int)(g_light_col[pi][2] * 100.0f),
                 vis, ok);
        log_mod(lbuf);
    }
}

/* Switch an unused slot off (only when still ours — never native). */
static void light_switch_off_slot(DWORD gfx, int s) {
    DWORD obj;
    DWORD slotptr;
    DWORD cur = 0;
    if (s < 0 || s >= MAX_LIGHTS) return;
    obj = g_light_objs[s];
    if (!obj || IsBadReadPtr((void*)obj, SCENEOBJECT_SIZE)) return;
    slotptr = gfx + GFX_LIGHT_SLOTS + (DWORD)(LIGHT_SLOT_BASE + s) * 4;
    if (!IsBadReadPtr((void*)slotptr, 4)) cur = *(DWORD*)slotptr;
    if (cur == 0 || cur == obj) {
        *(BYTE*)((char*)obj + SO_VISIBLE) = 0;
        native_register(gfx, LIGHT_SLOT_BASE + s, obj);
    }
}

/* 0x46B7EA bakes Attenuation1=0.04 (mov [esi+0x78],0x3D23D70A — LINEAR
 * falloff 1/(0.04d), NOT squared). REVERTED: zeroing it blasted native
 * follower lights to full range AND made our Att0=Att1=Att2=0 (div-by-0).
 * Natural 0.04 falloff is plenty visible with bright colors. This function
 * now RESTORES the byte pattern (repairs a v1u-v1aa-patched exe in case
 * the pattern was already zeroed this session) and is otherwise a no-op. */
static void patch_attenuation(void) {
    /* RETIRED v1ah: fresh exe image always carries stock bytes (in-memory
     * zeroing could never persist), and pattern-matching risks writing a
     * coincidental site on foreign exe versions. No-op by design. */
    log_mod("  LIGHT: Att patch retired (no-op)");
    return;
    static int done = 0;   /* dead, keeps later refs declared */
    HMODULE exe;
    DWORD peoff, sec, s;
    WORD ns;
    int found = 0;
    char lbuf[64];
    if (done) return;
    done = 1;
    exe = GetModuleHandleA(NULL);
    if (!exe) return;
    if (IsBadReadPtr(exe, 64)) return;
    peoff = *(DWORD*)((char*)exe + 0x3C);
    if (peoff > 0x1000) return;
    if (IsBadReadPtr((char*)exe + peoff, 64)) return;
    if (*(DWORD*)((char*)exe + peoff) != 0x4550) return;  /* "PE\0\0" */
    ns = *(WORD*)((char*)exe + peoff + 6);
    sec = peoff + 248;
    for (s = 0; s < ns && s < 16; s++) {
        DWORD vaddr, vsize;
        unsigned char* p;
        unsigned char* end;
        if (IsBadReadPtr((char*)exe + sec, 40)) break;
        vsize = *(DWORD*)((char*)exe + sec + 8);
        vaddr = *(DWORD*)((char*)exe + sec + 12);
        /* RefreshLight RVA 0x6B670 must live in this section */
        if (0x6B670 < vaddr || 0x6B670 >= vaddr + vsize) {
            sec += 40;
            continue;
        }
        if (IsBadReadPtr((char*)exe + vaddr, vsize)) break;
        p = (unsigned char*)exe + vaddr;
        end = p + vsize - 7;
        for (; p < end; p++) {
            if (p[0] == 0xC7 && p[1] == 0x46 && p[2] == 0x78 &&
                ((p[3] == 0x0A && p[4] == 0xD7 &&
                  p[5] == 0x23 && p[6] == 0x3D) ||   /* stock 0.04 */
                 (p[3] == 0 && p[4] == 0 &&
                  p[5] == 0 && p[6] == 0))) {        /* v1u-v1aa zeroed */
                DWORD oldp = 0;
                if (VirtualProtect(p + 3, 4, PAGE_EXECUTE_READWRITE,
                                   &oldp)) {
                    p[3] = 0x0A;
                    p[4] = 0xD7;
                    p[5] = 0x23;
                    p[6] = 0;
                    VirtualProtect(p + 3, 4, oldp, &oldp);
                    FlushInstructionCache(GetCurrentProcess(), p, 7);
                    found++;
                }
            }
        }
        break;
    }
    snprintf(lbuf, sizeof(lbuf), "  LIGHT: Att1 restore x%d", found);
    log_mod(lbuf);
}

/* Runs in text_render (render thread). job 1 = build, 2 = refresh.
 * quiet=1 skips per-light logs (periodic re-assert). */
static void service_light_job(DWORD board, int quiet) {
    DWORD gfx;
    int s, vis, n;
    if (g_job == 0 || board != g_job_board) return;
    if (!g_light_count && !g_light_used) { g_job = 0; return; }
    gfx = gfx_device();
    if (!gfx) return;   /* retry next frame */
    patch_attenuation();   /* one-time: restore Att1 0.04 */
    vis = (g_lights_on && g_light_vis) ? 1 : 0;
    if (g_job == 3) {
        /* level end: switch every used slot off, keep objects alive */
        for (s = 0; s < MAX_LIGHTS; s++) light_switch_off_slot(gfx, s);
        log_mod("  LIGHT: level end, slots off");
        g_job = 0;
        return;
    }
    /* build/refresh: nearest-MAX_LIGHTS win slots (real-time window) */
    n = light_select();
    for (s = 0; s < MAX_LIGHTS; s++) {
        if (s < n && g_active_map[s] >= 0)
            light_apply_slot(gfx, s, g_active_map[s], vis, quiet);
        else
            light_switch_off_slot(gfx, s);
    }
    g_light_used = n;
    g_active_n = n;
    if (!quiet) {
        /* one-time census: who owns gfx slots 0-3 (native?) at build */
        char cbuf[128];
        int k;
        char* cp = cbuf;
        cp += snprintf(cp, sizeof(cbuf) - (cp - cbuf), "  LIGHT: census");
        for (k = 0; k < 4; k++) {
            DWORD so = 0;
            int tp = -9;
            if (!IsBadReadPtr((void*)(gfx + GFX_LIGHT_SLOTS), 16))
                so = *(DWORD*)(gfx + GFX_LIGHT_SLOTS + (DWORD)k * 4);
            if (so && !IsBadReadPtr((void*)so, SCENEOBJECT_SIZE))
                tp = *(int*)((char*)so + SO_TYPE);
            cp += snprintf(cp, sizeof(cbuf) - (cp - cbuf),
                           " %d=0x%X t%d", k, so, tp);
        }
        log_mod(cbuf);
    }
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
    int yi;
    g_light_count = 0;
    g_light_vis = 1;
    /* BallBorder ring color for this race slot (neon_ballring_player1) */
    {
        float rgba[4];
        if (gridset_level_ring_slot(g_race_slot, rgba)) {
            g_ring_active[0] = rgba[0];
            g_ring_active[1] = rgba[1];
            g_ring_active[2] = rgba[2];
            g_ring_active[3] = rgba[3];
            g_ring_on = 1;
        } else {
            g_ring_on = 0;
        }
    }
    /* P1 emitter glow for this race slot (neon_glow_player1) */
    {
        float rgba[4];
        if (gridset_level_glow_slot(g_race_slot, rgba)) {
            g_glow_active[0] = rgba[0];
            g_glow_active[1] = rgba[1];
            g_glow_active[2] = rgba[2];
            g_glow_active[3] = rgba[3];
            g_glow_on = 1;
        } else {
            g_glow_on = 0;
        }
    }
    for (yi = 0; yi < MAX_LIGHTS; yi++) g_light_yield[yi] = 0;
    for (yi = 0; yi < MAX_LIGHTS; yi++) { g_active_map[yi] = -1; g_prev_map[yi] = -2; }
    g_active_n = 0;
    for (yi = 0; yi < MAX_LIGHT_PARSED; yi++) g_light_follow[yi] = 0;
    cur[0] = '\0';
    if (!find_current_level_file(cur, sizeof(cur))) {
        log_mod("  LIGHT: level file unknown, S3 skipped");
        n = -1;
    } else {
        have_file = 1;
        n = read_level_lights(cur);
        if (n < 0) log_mod("  LIGHT: parse failed, S3 skipped");
    }
    /* POINTnn refs fill whatever slots S3 left free (first MAX win) */
    {
        char cbuf[64];
        snprintf(cbuf, sizeof(cbuf), "  LIGHT: %d POINT/LIGHT ref(s) in S1 (%d kept)",
                 g_ppt_total, g_ppt_count);
        log_mod(cbuf);
    }
    for (i = 0; i < g_ppt_count && g_light_count < MAX_LIGHT_PARSED; i++) {
        int li = g_light_count;
        char pbuf[128];
        float mr = 1.0f, mg = 1.0f, mb = 1.0f;   /* missing-mat fallback:
                                                     unity white (emitter 1) */
        float rgb[3];
        float auxt[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
        float mult = 1.0f;   /* emissive-alpha per-light gain */
        int got = 0;
        const char* csrc = "white";
        g_light_pos[li][0] = g_ppt_x[i] + LIGHT_TEST_DX;   /* TEMP +10 X */
        g_light_pos[li][1] = g_ppt_y[i];
        g_light_pos[li][2] = g_ppt_z[i];
        if (have_file && read_geom_diffuse(cur, g_ppt_name[i], rgb, auxt)) {
            if (auxt[3] > 0.0001f && auxt[3] <= 10.0f) mult = auxt[3];
            if (auxt[0] >= 0.0f && auxt[0] <= 10.0f &&
                auxt[1] >= 0.0f && auxt[1] <= 10.0f &&
                auxt[2] >= 0.0f && auxt[2] <= 10.0f &&
                (auxt[0] + auxt[1] + auxt[2]) > 0.0001f) {
                /* primary: emissive (the glow paint). LevelD green lives
                 * here: mat+48/52/56 = (0,1,0.003). */
                mr = auxt[0] * mult;
                mg = auxt[1] * mult;
                mb = auxt[2] * mult;
                got = 1;
                csrc = "emi";
            } else if (rgb[0] >= 0.0f && rgb[0] <= 10.0f &&
                       rgb[1] >= 0.0f && rgb[1] <= 10.0f &&
                       rgb[2] >= 0.0f && rgb[2] <= 10.0f &&
                       (rgb[0] + rgb[1] + rgb[2]) > 0.0001f) {
                /* fallback: diffuse (older files paint this instead) */
                mr = rgb[0] * mult;
                mg = rgb[1] * mult;
                mb = rgb[2] * mult;
                got = 1;
                csrc = "dif";
            }
        }
        if (g_ppt_rng[i] >= 10.0f) {
            g_light_rng[li] = g_ppt_rng[i];
            g_light_rngfix[li] = 1;
        } else {
            g_light_rng[li] = g_light_range;
            g_light_rngfix[li] = 0;
        }
        g_light_col[li][0] = mr * g_light_intensity * LIGHT_OUTPUT_TRIM / LIGHT_INTENSITY;
        g_light_col[li][1] = mg * g_light_intensity * LIGHT_OUTPUT_TRIM / LIGHT_INTENSITY;
        g_light_col[li][2] = mb * g_light_intensity * LIGHT_OUTPUT_TRIM / LIGHT_INTENSITY;
        g_light_mat[li][0] = mr * LIGHT_OUTPUT_TRIM / LIGHT_INTENSITY;
        g_light_mat[li][1] = mg * LIGHT_OUTPUT_TRIM / LIGHT_INTENSITY;
        g_light_mat[li][2] = mb * LIGHT_OUTPUT_TRIM / LIGHT_INTENSITY;
        g_light_src[li] = g_ppt_follow[i] ? 2 : 1;   /* 2 = Playerlight */
        g_light_follow[li] = g_ppt_follow[i];
        g_light_count++;
        snprintf(pbuf, sizeof(pbuf),
                 "  LIGHT: POINT ref %d %s at (%d,%d,%d) mat=(%d,%d,%d) %s%s rng=%d gm=%d",
                 i + 1, g_ppt_name[i],
                 (int)g_ppt_x[i], (int)g_ppt_y[i], (int)g_ppt_z[i],
                 (int)(mr * 100.0f), (int)(mg * 100.0f),
                 (int)(mb * 100.0f), csrc, got ? "" : " no-mat",
                 (int)g_light_rng[li], (int)(mult * 100.0f));
        log_mod(pbuf);
        if (g_ppt_follow[i]) {
            char fbuf[96];
            snprintf(fbuf, sizeof(fbuf), "  LIGHT: Playerlight %d %s follows ball",
                     i + 1, g_ppt_name[i]);
            log_mod(fbuf);
        }
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
    scan_spawn_entities(board);
    if (count > 0) {
        char buf[64];
        g_mult = gridset_level_mult_slot(g_race_slot);
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
                                         g_pts_z[pi], pi + 1, mesh_for(pi),
                                         0.0f, 0.0f, 0.0f);
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

    CustomSlider s3("BATTY_LIGHT_INTENSITY", "Light Intensity", LIGHT_INTENSITY);
    s3.lowerBound = 0.0f; s3.upperBound = 100.0f; s3.stepSize = 0.5f; s3.decimalPlaces = 1;
    HBAPI(api).CreateSlider(s3, (HamsterballAPI*)thisptr);

    {
        char ibuf[512];
        snprintf(ibuf, sizeof(ibuf), "INIT Battyball Entities v1ce log=%s set=%s",
                 g_log_path, g_set_path);
        log_mod(ibuf);
    }
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
        int i;
        g_light_range = value < 10.0f ? 10.0f : value;
        g_light_range *= LIGHT_RANGE_SCALE;   /* felt = shown x scale */
        /* suffixed lights keep their own range */
        for (i = 0; i < g_light_count && i < MAX_LIGHT_PARSED; i++) {
            if (!g_light_rngfix[i]) g_light_rng[i] = g_light_range;
        }
        if (g_light_used || g_light_count) {
            g_job = 2;
            g_job_board = player_board();
        }
    }
    else if (strcmp(id, "BATTY_LIGHT_INTENSITY") == 0) {
        int i;
        char ibuf[48];
        g_light_intensity = value < 0.0f ? 0.0f : value;
        snprintf(ibuf, sizeof(ibuf), "  LIGHT: intensity %d",
                 (int)(g_light_intensity * 10.0f));
        log_mod(ibuf);
        /* rescale parsed colors from stored material ratios (no re-parse) */
        for (i = 0; i < g_light_count && i < MAX_LIGHT_PARSED; i++) {
            g_light_col[i][0] = g_light_mat[i][0] * g_light_intensity;
            g_light_col[i][1] = g_light_mat[i][1] * g_light_intensity;
            g_light_col[i][2] = g_light_mat[i][2] * g_light_intensity;
        }
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
    g_ent_inst_count = 0;
    g_ent_last_tick = 0;
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

/* BallBorder (ballborder.png) material: Ball_Render passes ball+0x1B8 as the
 * sprite material; SetMaterial consumes D3DMATERIAL8 at +4 (ball+0x1BC).
 * Only sprite draws (border/burner) use it — mesh draws use mesh mats. */
#define BALL_BORDER_MAT   0x1BC    /* D3DMATERIAL8: Diffuse+0 Ambient+16 */
#define BALL_MAT_EMISSIVE 0x30     /* Specular+32 Emissive+48 Power+64 */
/* Per-level P1 border color (neon_ballring_player1 in the set jsonc).
 * Level off (commented/absent) = native black (no glow). Master toggle gates.
 * Aligned float writes: render-thread safe. */
static void border_frame(void) {
    void* b;
    char* m;
    float r, g, bl, a;
    if (!g_enabled || !g_api) return;
    if (!g_ring_on && !g_race_slot) return;   /* unknown board: touch nothing */
    b = (void*)HBAPI(g_api).GetPlayer();
    if (!b || IsBadReadPtr(b, 0x300)) return;
    m = (char*)b + BALL_BORDER_MAT;
    if (IsBadReadPtr(m, 0x44)) return;
    if (g_ring_on) {
        r = g_ring_active[0];
        g = g_ring_active[1];
        bl = g_ring_active[2];
        a = g_ring_active[3];
    } else {
        r = g = bl = 0.0f;
        a = *(float*)(m + 0x0C);   /* keep native alpha when off */
    }
    *(float*)(m + 0x00) = r;
    *(float*)(m + 0x04) = g;
    *(float*)(m + 0x08) = bl;
    *(float*)(m + 0x0C) = a;
    *(float*)(m + BALL_MAT_EMISSIVE + 0x00) = r;
    *(float*)(m + BALL_MAT_EMISSIVE + 0x04) = g;
    *(float*)(m + BALL_MAT_EMISSIVE + 0x08) = bl;
}

/* P1 emitter glow (Neon_colors GLOW half): scene+0x436C emitter +0x94 RGB,
 * +0xA0 A. Key absent = untouched (native glow). NULL-safe: emitter exists
 * on Neon; other races skip silently. */
#define BOARD_P1_EMITTER  0x436C   /* P1 emitter SceneObject (cf. 0x416270) */
static void glow_frame(void) {
    DWORD board;
    DWORD emit;
    char* e;
    if (!g_enabled || !g_api) return;
    if (!g_glow_on) return;
    board = player_board();
    if (!board || IsBadReadPtr((void*)board, 0x4400)) return;
    if (IsBadReadPtr((void*)(board + BOARD_P1_EMITTER), 4)) return;
    emit = *(DWORD*)(board + BOARD_P1_EMITTER);
    if (!emit || IsBadReadPtr((void*)emit, 0xA4)) return;
    e = (char*)emit;
    *(float*)(e + SO_EMIT_R) = g_glow_active[0];
    *(float*)(e + SO_EMIT_G) = g_glow_active[1];
    *(float*)(e + SO_EMIT_B) = g_glow_active[2];
    *(float*)(e + SO_EMIT_A) = g_glow_active[3];
}

static void light_frame(void);   /* defined below (used by game_update) */

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
        g_race_slot = 0;   /* unknown until scanned: ring/glow touch nothing */
        g_ring_on = 0;
        g_glow_on = 0;
        {
            int ei;
            for (ei = 0; ei < ENT_MAX_INST; ei++) g_ent_objs[ei] = 0;
            g_ent_inst_count = 0;
            g_ent_last_tick = 0;
        }
        return;
    }

    light_frame();   /* pin/heartbeat/service also from game_update */
    border_frame();    /* slot RGBA -> P1 border (overrides exe, Neon too) */
    glow_frame();      /* slot RGBA -> P1 emitter glow (Neon_colors GLOW) */
    entity_frame(board); /* named entities: Woodbridge motion, rest native-driven */

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
static DWORD g_last_pin_log = 0;   /* throttle for re-pin lines */
static DWORD g_last_beat = 0;      /* last heartbeat tick */
static DWORD g_frames = 0;         /* text_render call counter */
static DWORD g_seen_slot4 = 0;     /* first slot4 ptr seen (swap detector) */
/* Something per-frame drags light objects to the ball. Hold our ground:
 * if a used slot's position differs from its parsed spot, drag it back
 * (log throttled). Runs every frame in text_render (render thread). */
/* Nearest-window pin: every frame the MAX_LIGHTS nearest parsed lights own
 * slots 1-7; mapping changes re-apply, otherwise hold position vs drag.
 * Runs every frame in text_render (render thread). */
static void pin_lights(DWORD board, DWORD gfx) {
    int s, vis, n, changed = 0;
    DWORD now;
    (void)board;
    vis = (g_lights_on && g_light_vis) ? 1 : 0;
    n = light_select();
    g_light_used = n;
    g_active_n = n;
    now = GetTickCount();
    for (s = 0; s < MAX_LIGHTS; s++)
        if (g_active_map[s] != g_prev_map[s]) changed = 1;
    if (changed) {
        if ((int)(now - g_last_pin_log) >= 500) {
            char mbuf[160];
            int o = 0;
            o += snprintf(mbuf + o, sizeof(mbuf) - o, "  LIGHT: window");
            for (s = 0; s < MAX_LIGHTS && o < 130; s++)
                o += snprintf(mbuf + o, sizeof(mbuf) - o, " s%d=P%d",
                              LIGHT_SLOT_BASE + s, g_active_map[s]);
            log_mod(mbuf);
            g_last_pin_log = now;
        }
        for (s = 0; s < MAX_LIGHTS; s++) {
            if (s < n && g_active_map[s] >= 0)
                light_apply_slot(gfx, s, g_active_map[s], vis, 1);
            else
                light_switch_off_slot(gfx, s);
            g_prev_map[s] = g_active_map[s];
        }
        return;
    }
    for (s = 0; s < n; s++) {
        int pi = g_active_map[s];
        DWORD obj;
        DWORD slotptr;
        DWORD cur = 0;
        float ox, oy, oz;
        if (pi < 0) continue;
        obj = g_light_objs[s];
        if (!obj || IsBadReadPtr((void*)obj, SCENEOBJECT_SIZE)) {
            light_apply_slot(gfx, s, pi, vis, 1);
            continue;
        }
        /* native-priority: slot taken by a native since? yield / reclaim */
        slotptr = gfx + GFX_LIGHT_SLOTS + (DWORD)(LIGHT_SLOT_BASE + s) * 4;
        if (!IsBadReadPtr((void*)slotptr, 4)) cur = *(DWORD*)slotptr;
        if (cur != 0 && cur != obj) {
            if (!g_light_yield[s]) {
                g_light_yield[s] = 1;
                if ((int)(now - g_last_pin_log) >= 500) {
                    char ybuf[64];
                    snprintf(ybuf, sizeof(ybuf),
                             "  LIGHT s%d: slot %d native-held, yield", s,
                             LIGHT_SLOT_BASE + s);
                    log_mod(ybuf);
                    g_last_pin_log = now;
                }
            }
            continue;
        }
        if (g_light_yield[s] && cur == 0) {
            g_light_yield[s] = 0;
            write_light_fields(obj, pi, vis);
            claim_slot(gfx, LIGHT_SLOT_BASE + s, obj);
            if ((int)(now - g_last_pin_log) >= 500) {
                char rbuf[64];
                snprintf(rbuf, sizeof(rbuf), "  LIGHT s%d: slot %d reclaimed",
                         s, LIGHT_SLOT_BASE + s);
                log_mod(rbuf);
                g_last_pin_log = now;
            }
            continue;
        }
        if (g_light_yield[s]) g_light_yield[s] = 0;
        ox = *(float*)((char*)obj + SO_POS_X);
        oy = *(float*)((char*)obj + SO_POS_Y);
        oz = *(float*)((char*)obj + SO_POS_Z);
        if (ox != g_light_pos[pi][0] || oy != g_light_pos[pi][1] ||
            oz != g_light_pos[pi][2]) {
            if ((int)(now - g_last_pin_log) >= 500) {
                char pbuf[128];
                snprintf(pbuf, sizeof(pbuf),
                         "  LIGHT s%d: re-pin P%d (was %d,%d,%d)",
                         s, pi, (int)ox, (int)oy, (int)oz);
                log_mod(pbuf);
                g_last_pin_log = now;
            }
            write_light_fields(obj, pi, vis);
            native_setpos(obj, g_light_pos[pi][0], g_light_pos[pi][1],
                          g_light_pos[pi][2]);
            native_register(gfx, LIGHT_SLOT_BASE + s, obj);
        }
    }
    for (s = n; s < MAX_LIGHTS; s++) light_switch_off_slot(gfx, s);
}
/* Per-frame light service. Runs from BOTH text_render and onGameUpdate:
 * text_render barely fires in-race, so game_update carries the pin. */
static void light_frame(void) {
    DWORD board;
    DWORD gfx;
    board = player_board();
    if (!board || IsBadReadPtr((void*)board, 0x4400)) return;
    /* board can be recreated after load (load-board vs race-board): adopt
     * the live one so pin/reassert guards keep running, re-register ours */
    if ((g_light_count || g_light_used) && g_job == 0 &&
        board != g_job_board) {
        char abuf[64];
        DWORD gfxa = gfx_device();
        int i;
        g_job_board = board;
        snprintf(abuf, sizeof(abuf), "  LIGHT: adopt board 0x%X", board);
        log_mod(abuf);
        if (gfxa) {
            int vis = (g_lights_on && g_light_vis) ? 1 : 0;
            int n = light_select();
            int s;
            for (s = 0; s < MAX_LIGHTS; s++) {
                if (s < n && g_active_map[s] >= 0) {
                    DWORD obj = g_light_objs[s];
                    if (!obj ||
                        IsBadReadPtr((void*)obj, SCENEOBJECT_SIZE)) continue;
                    write_light_fields(obj, g_active_map[s], vis);
                    if (!claim_slot(gfxa, LIGHT_SLOT_BASE + s, obj))
                        g_light_yield[s] = 1;
                    else
                        g_light_yield[s] = 0;
                } else {
                    light_switch_off_slot(gfxa, s);
                }
                g_prev_map[s] = g_active_map[s];
            }
            g_light_used = n;
            g_active_n = n;
        }
    }
    /* heartbeat FIRST (before gfx check): silence itself is data */
    {
        DWORD now = GetTickCount();
        if ((int)(now - g_last_beat) >= 1000) {
            char hbuf[96];
            DWORD gfx0 = gfx_device();
            g_last_beat = now;
            snprintf(hbuf, sizeof(hbuf),
                     "  LIGHT: beat f=%u job=%d board=0x%X gfx=0x%X",
                     g_frames, g_job, board, gfx0);
            log_mod(hbuf);
        }
    }
    if (g_job != 0) { service_light_job(board, 0); return; }
    gfx = gfx_device();
    if (!gfx) return;
    g_frames++;
    /* slot detail ~1/sec (frame cadence): swap detector */
    if ((g_frames % 60) == 0) {
        char hbuf[192];
        DWORD s4 = 0, s0 = 0;
        float fx = 0.0f, fy = 0.0f, fz = 0.0f;
        float nx = 0.0f, ny = 0.0f, nz = 0.0f;
        int i, tp = -9, vs = -9;
            if (!IsBadReadPtr((void*)(gfx + GFX_LIGHT_SLOTS), 32)) {
                s4 = *(DWORD*)(gfx + GFX_LIGHT_SLOTS + 4 * 4);
                s0 = *(DWORD*)(gfx + GFX_LIGHT_SLOTS);
            }
            if (!g_seen_slot4) g_seen_slot4 = s4;
            if (g_light_used >= 0 && g_light_objs[0] &&
                !IsBadReadPtr((void*)g_light_objs[0], SCENEOBJECT_SIZE)) {
                fx = *(float*)((char*)g_light_objs[0] + SO_POS_X);
                fy = *(float*)((char*)g_light_objs[0] + SO_POS_Y);
                fz = *(float*)((char*)g_light_objs[0] + SO_POS_Z);
                tp = *(int*)((char*)g_light_objs[0] + SO_TYPE);
                vs = *(BYTE*)((char*)g_light_objs[0] + SO_VISIBLE);
            }
            if (s0 && !IsBadReadPtr((void*)s0, SCENEOBJECT_SIZE)) {
                nx = *(float*)((char*)s0 + SO_POS_X);
                ny = *(float*)((char*)s0 + SO_POS_Y);
                nz = *(float*)((char*)s0 + SO_POS_Z);
            }
            snprintf(hbuf, sizeof(hbuf),
                     "  LIGHT: beat f=%u slot4=0x%X obj0=0x%X pos=(%d,%d,%d) arr=(%d,%d,%d) t=%d v=%d s0=(%d,%d,%d)%s",
                     g_frames, s4, g_light_objs[0],
                     (int)fx, (int)fy, (int)fz,
                     (int)g_light_pos[0][0], (int)g_light_pos[0][1],
                     (int)g_light_pos[0][2], tp, vs,
                     (int)nx, (int)ny, (int)nz,
                     (s4 && s4 != g_seen_slot4) ? " SWAPPED" : "");
            log_mod(hbuf);
            /* BallBorder gate diag (frame-based: no timer dependency) */
            {
                char dbuf[96];
                void* b = (void*)HBAPI(g_api).GetPlayer();
                int b280 = -9, b324 = -9, b754 = -9, g182 = -9;
                if (b && !IsBadReadPtr(b, 0x800)) {
                    b280 = *(BYTE*)((char*)b + 0x280);
                    b324 = *(BYTE*)((char*)b + 0x324);
                    b754 = *(int*)((char*)b + 0x754);
                }
                if (gfx && !IsBadReadPtr((void*)(gfx + 0x182), 1))
                    g182 = *(BYTE*)(gfx + 0x182);
                snprintf(dbuf, sizeof(dbuf),
                         "  BORDER: b280=%d b324=%d b754=%d gfx182=%d",
                         b280, b324, b754, g182);
                log_mod(dbuf);
            }
            /* on swap, fingerprint all 8 slots once */
            if (s4 && s4 != g_seen_slot4) {
                g_seen_slot4 = s4;
                for (i = 0; i < 8; i++) {
                    char sbuf[96];
                    DWORD so = *(DWORD*)(gfx + GFX_LIGHT_SLOTS + (DWORD)i * 4);
                    int tp = -9, vs = -9;
                    if (so && !IsBadReadPtr((void*)so, SCENEOBJECT_SIZE)) {
                        tp = *(int*)((char*)so + SO_TYPE);
                        vs = *(BYTE*)((char*)so + SO_VISIBLE);
                    }
                    snprintf(sbuf, sizeof(sbuf),
                             "  LIGHT: slot %d obj=0x%X type=%d vis=%d",
                             i, so, tp, vs);
                    log_mod(sbuf);
                }
            }
    }
    if ((g_light_count || g_light_used) && board == g_job_board)
        pin_lights(board, gfx);
    /* retry wipes slots via ResetObjectSlots: re-assert ~every 2s */
    if ((g_light_count || g_light_used) && board == g_job_board) {
        g_reassert++;
        if ((g_reassert & 127) == 0) {
            g_job = 2;
            service_light_job(board, 1);
        }
    }
}
static void __thiscall text_render(void*) {
    light_frame();
    border_frame();   /* render-time re-assert: slot RGBA wins over exe, Neon too */
    glow_frame();     /* render-time re-assert: P1 emitter glow */
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