/*
 * hamsterball_water_mod.c — BASS.dll proxy that adds custom water physics.
 *
 * BUILD (Linux -> Windows): make
 *
 * INSTALLATION (Windows):
 *   1. In the Hamsterball game folder rename original bass.dll -> bass_real.dll
 *   2. Copy this proxy bass.dll + hamsterball_water.ini into the game folder
 *   3. Place invisible collision planes named "E:WATER" in custom levels
 *   4. Run Hamsterball.exe normally
 *
 * HOW IT WORKS:
 *   - Forwards all BASS audio calls to bass_real.dll.
 *   - Hooks Ball vtable slot 4 (Ball_Update thunk at 0x408390).
 *   - Hook_Ball_Update saves the ball's pre-update position, calls the original
 *     Ball_Update, then modifies the resulting position delta to simulate:
 *       1. Entry damping (30% vertical speed cut on first water contact)
 *       2. Drag (per-frame velocity reduction)
 *       3. Buoyancy (upward force proportional to submersion depth)
 *       4. Equilibrium at half-submerged (net vertical force = 0)
 *       5. Horizontal dampening (slightly lower max horizontal speed)
 *       6. No extra max vertical velocity cap
 *   - Water surface Y is discovered by scanning the collision MeshWorld's
 *     object list for MeshBuffer objects named "E:WATER", then reading the
 *     Y coordinate from their first collision face vertex.
 *   - Fallback: if no E:WATER objects are found, water planes can be
 *     specified in the INI file.
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winuser.h>
#include <mmsystem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ---- BASS forward declarations (__stdcall) ---- */

typedef void  (__stdcall *BASS_Stop_t)(void);
typedef int   (__stdcall *BASS_ChannelSetAttributes_t)(DWORD, float, int, int);
typedef int   (__stdcall *BASS_Free_t)(void);
typedef int   (__stdcall *BASS_Init_t)(int, DWORD, DWORD, HWND, void*);
typedef int   (__stdcall *BASS_Start_t)(void);
typedef int   (__stdcall *BASS_SetConfig_t)(DWORD, DWORD);
typedef int   (__stdcall *BASS_ChannelStop_t)(DWORD);
typedef int   (__stdcall *BASS_MusicPlayEx_t)(DWORD, DWORD, BOOL);
typedef int   (__stdcall *BASS_ErrorGetCode_t)(void);
typedef DWORD (__stdcall *BASS_MusicLoad_t)(int, void*, DWORD, DWORD, DWORD, DWORD);

static HMODULE g_hRealBass = NULL;
static BASS_Stop_t                 real_BASS_Stop = NULL;
static BASS_ChannelSetAttributes_t real_BASS_ChannelSetAttributes = NULL;
static BASS_Free_t                 real_BASS_Free = NULL;
static BASS_Init_t                 real_BASS_Init = NULL;
static BASS_Start_t                real_BASS_Start = NULL;
static BASS_SetConfig_t            real_BASS_SetConfig = NULL;
static BASS_ChannelStop_t          real_BASS_ChannelStop = NULL;
static BASS_MusicPlayEx_t          real_BASS_MusicPlayEx = NULL;
static BASS_ErrorGetCode_t         real_BASS_ErrorGetCode = NULL;
static BASS_MusicLoad_t            real_BASS_MusicLoad = NULL;

/* ---- Memory layout constants (verified against Hamsterball.exe) ---- */

#define IMAGE_BASE              0x00400000

#define ADDR_BALL_VTABLE        0x004CF3A0
#define VTABLE_SLOT_UPDATE      4           /* slot 4 → 0x408390 thunk → Ball_Update @ 0x405E00 */

/* Ball struct offsets */
#define OFF_BALL_SCENE          0x14    /* Scene* back-pointer */
#define OFF_BALL_POS_X          0x164
#define OFF_BALL_POS_Y          0x168
#define OFF_BALL_POS_Z          0x16C
#define OFF_BALL_RADIUS         0x284

/* Scene struct offsets */
#define OFF_SCENE_COLLISION_MW  0x08    /* collision MeshWorld* (set by Level_LoadCollision) */

/* MeshWorld struct offsets */
#define OFF_MW_OBJ_LIST         0x2C    /* AthenaList object_list */
/* AthenaList layout (consistent across all uses in the game):
 *   +0x00: state (used by AthenaList_GetIndex)
 *   +0x04: count
 *   +0x08: iteration indices[256] (0x400 bytes)
 *   +0x40C: data pointer (void**)
 */
#define ATHENA_COUNT_OFFSET     0x04
#define ATHENA_DATA_OFFSET      0x40C

/* MeshBuffer struct offsets (0x874 bytes) */
#define OFF_MB_FACE_LIST         0x0C    /* AthenaList of CollisionFace */
#define OFF_MB_NAME              0x864   /* char* name string */
#define OFF_MB_INTERACTIVE       0x85D
#define OFF_MB_NO_RENDER         0x863

/* CollisionFace struct offsets (0x60 bytes) */
#define OFF_FACE_V0_X            0x00
#define OFF_FACE_V0_Y            0x04
#define OFF_FACE_V0_Z            0x08

/* ---- Config ---- */

#define MAX_WATER_PLANES 16

typedef struct {
    float entry_damping;      /* vertical speed multiplier on first contact while falling (0-1) */
    float drag;               /* per-frame velocity drag applied to all axes (0-1) */
    float horizontal_drag;    /* extra drag on X/Z axes only (0-1) */
    float gravity_equivalent;  /* buoyancy calibration — should match game's per-frame gravity */
    int   debug;              /* write log file */
    /* Fallback water planes (used if no E:WATER objects found in level) */
    int   plane_count;
    float plane_y[MAX_WATER_PLANES];
} water_cfg_t;

static water_cfg_t g_cfg = {
    0.70f,  /* entry_damping: 30% reduction */
    0.03f,  /* drag */
    0.04f,  /* horizontal_drag */
    0.45f,  /* gravity_equivalent */
    1,      /* debug */
    0,      /* plane_count */
    {0}     /* plane_y */
};

/* ---- Water plane cache ---- */

typedef struct {
    float surface_y;       /* world Y of water surface */
    int   found;           /* 1 = valid */
} water_plane_t;

static water_plane_t g_planes[MAX_WATER_PLANES];
static int g_plane_count = 0;
static int g_scan_done = 0;             /* set after first scan attempt */
static void *g_last_scene = NULL;       /* detect scene changes to re-scan */

/* ---- Per-ball water state ---- */

#define MAX_BALLS 32

typedef struct {
    void *ball;              /* ball pointer (key); NULL = unused */
    int   in_water;         /* currently in water? */
    int   entry_damped;     /* already applied entry damping this water session? */
    float last_y;           /* ball Y from previous frame (for surface-cross detection) */
    int   have_last_y;      /* is last_y valid? */
} water_state_t;

static water_state_t g_states[MAX_BALLS];

/* ---- Log ---- */

static FILE *g_log = NULL;

static void log_msg(const char *fmt, ...)
{
    if (!g_log) return;
    va_list ap;
    va_start(ap, fmt);
    vfprintf(g_log, fmt, ap);
    va_end(ap);
    fflush(g_log);
}

static int file_exists(const char *path)
{
    DWORD attr = GetFileAttributesA(path);
    return attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY);
}

static float read_ini_float(const char *path, const char *section, const char *key, float def)
{
    char buf[64];
    DWORD n = GetPrivateProfileStringA(section, key, "", buf, sizeof(buf), path);
    if (n == 0) return def;
    return (float)atof(buf);
}

static int read_ini_int(const char *path, const char *section, const char *key, int def)
{
    char buf[32];
    DWORD n = GetPrivateProfileStringA(section, key, "", buf, sizeof(buf), path);
    if (n == 0) return def;
    return atoi(buf);
}

static void load_config(const char *ini_path)
{
    if (!file_exists(ini_path)) return;

    g_cfg.entry_damping     = read_ini_float(ini_path, "WaterPhysics", "EntryDamping", 0.70f);
    g_cfg.drag              = read_ini_float(ini_path, "WaterPhysics", "Drag", 0.03f);
    g_cfg.horizontal_drag   = read_ini_float(ini_path, "WaterPhysics", "HorizontalDrag", 0.04f);
    g_cfg.gravity_equivalent = read_ini_float(ini_path, "WaterPhysics", "GravityEquivalent", 0.45f);
    g_cfg.debug             = read_ini_int(ini_path, "WaterPhysics", "Debug", 1);

    if (g_cfg.entry_damping < 0.0f) g_cfg.entry_damping = 0.0f;
    if (g_cfg.entry_damping > 1.0f) g_cfg.entry_damping = 1.0f;
    if (g_cfg.drag < 0.0f) g_cfg.drag = 0.0f;
    if (g_cfg.drag > 1.0f) g_cfg.drag = 1.0f;
    if (g_cfg.horizontal_drag < 0.0f) g_cfg.horizontal_drag = 0.0f;
    if (g_cfg.horizontal_drag > 1.0f) g_cfg.horizontal_drag = 1.0f;

    /* Fallback water planes from INI */
    g_cfg.plane_count = read_ini_int(ini_path, "WaterPlanes", "Count", 0);
    if (g_cfg.plane_count < 0) g_cfg.plane_count = 0;
    if (g_cfg.plane_count > MAX_WATER_PLANES) g_cfg.plane_count = MAX_WATER_PLANES;
    for (int i = 0; i < g_cfg.plane_count; i++) {
        char key[16];
        wsprintfA(key, "Y%d", i);
        g_cfg.plane_y[i] = read_ini_float(ini_path, "WaterPlanes", key, 0.0f);
    }
}

static void open_log(const char *path)
{
    if (g_log || !g_cfg.debug) return;
    g_log = fopen(path, "w");
    if (g_log) {
        fprintf(g_log, "Hamsterball water mod log started\n");
        fflush(g_log);
    }
}

static void open_log_fallback(const char *game_path)
{
    if (g_log || !g_cfg.debug) return;

    const char *candidates[3];
    char cwd_path[MAX_PATH];
    char temp_path[MAX_PATH];

    candidates[0] = game_path;

    GetCurrentDirectoryA(sizeof(cwd_path), cwd_path);
    if (strlen(cwd_path) + 1 + 14 < sizeof(cwd_path))
        strcat(cwd_path, "\\water_mod.log");
    else
        cwd_path[0] = '\0';
    candidates[1] = cwd_path;

    GetTempPathA(sizeof(temp_path), temp_path);
    if (strlen(temp_path) + 26 < sizeof(temp_path))
        strcat(temp_path, "hamsterball_water_mod.log");
    else
        temp_path[0] = '\0';
    candidates[2] = temp_path;

    for (int i = 0; i < 3; i++) {
        if (!candidates[i] || !candidates[i][0]) continue;
        FILE *f = fopen(candidates[i], "w");
        if (f) {
            g_log = f;
            fprintf(g_log, "Hamsterball water mod log (path=%s)\n", candidates[i]);
            fflush(g_log);
            return;
        }
    }
}

/* ---- Memory patching primitives ---- */

static int set_memory_writable(void *addr, SIZE_T len, DWORD *old_protect)
{
    return VirtualProtect(addr, len, PAGE_EXECUTE_READWRITE, old_protect) ? 1 : 0;
}

static void restore_memory_protect(void *addr, SIZE_T len, DWORD old_protect)
{
    DWORD tmp;
    VirtualProtect(addr, len, old_protect, &tmp);
}

/* ---- Patching: modify a vtable slot ---- */

static void** slot_ptr(DWORD vtable_addr, int slot_index)
{
    return (void **)(vtable_addr + slot_index * sizeof(void*));
}

static int patch_vtable_slot(int slot_index, void *new_func, void **old_func_out)
{
    void **slot = slot_ptr(ADDR_BALL_VTABLE, slot_index);
    DWORD old_protect;

    if (IsBadReadPtr(slot, sizeof(void*))) return 0;
    if (!set_memory_writable(slot, sizeof(void*), &old_protect)) return 0;

    *old_func_out = *slot;
    *slot = new_func;

    restore_memory_protect(slot, sizeof(void*), old_protect);
    FlushInstructionCache(GetCurrentProcess(), slot, sizeof(void*));
    return 1;
}

/* ---- Ball accessors ---- */

static void *get_scene(void *ball)
{
    if (!ball) return NULL;
    return *(void **)((char *)ball + OFF_BALL_SCENE);
}

static float get_ball_pos_x(void *ball) { return *(float *)((char *)ball + OFF_BALL_POS_X); }
static float get_ball_pos_y(void *ball) { return *(float *)((char *)ball + OFF_BALL_POS_Y); }
static float get_ball_pos_z(void *ball) { return *(float *)((char *)ball + OFF_BALL_POS_Z); }
static float get_ball_radius(void *ball) { return *(float *)((char *)ball + OFF_BALL_RADIUS); }

static void set_ball_pos(void *ball, float x, float y, float z)
{
    *(float *)((char *)ball + OFF_BALL_POS_X) = x;
    *(float *)((char *)ball + OFF_BALL_POS_Y) = y;
    *(float *)((char *)ball + OFF_BALL_POS_Z) = z;
}

/* ---- AthenaList iteration helpers ---- */

static int athena_get_count(void *list_ptr)
{
    if (!list_ptr || IsBadReadPtr(list_ptr, 0x414)) return 0;
    return *(int *)((char *)list_ptr + ATHENA_COUNT_OFFSET);
}

static void **athena_get_data(void *list_ptr)
{
    if (!list_ptr || IsBadReadPtr(list_ptr, 0x414)) return NULL;
    void **data = *(void ***)((char *)list_ptr + ATHENA_DATA_OFFSET);
    if (!data || IsBadReadPtr(data, sizeof(void*))) return NULL;
    return data;
}

/* ---- Water plane discovery ---- */

/* Read the Y coordinate of the first face vertex of a MeshBuffer.
 * MeshBuffer+0x0C = face_list (AthenaList)
 * First face's v0.y is at face_struct + 0x04 */
static float read_meshbuffer_face_y(void *meshbuf)
{
    if (!meshbuf || IsBadReadPtr(meshbuf, 0x874)) return 0.0f;

    void *face_list = (char *)meshbuf + OFF_MB_FACE_LIST;
    int face_count = athena_get_count(face_list);
    if (face_count < 1) return 0.0f;

    void **face_data = athena_get_data(face_list);
    if (!face_data) return 0.0f;

    void *first_face = face_data[0];
    if (!first_face || IsBadReadPtr(first_face, 0x60)) return 0.0f;

    return *(float *)((char *)first_face + OFF_FACE_V0_Y);
}

/* Get the name string from a MeshBuffer */
static const char *get_meshbuffer_name(void *meshbuf)
{
    if (!meshbuf || IsBadReadPtr(meshbuf, 0x874)) return NULL;
    char *name = *(char **)((char *)meshbuf + OFF_MB_NAME);
    if (!name || IsBadReadPtr(name, 1)) return NULL;
    return name;
}

static int is_water_name(const char *name)
{
    if (!name) return 0;
    return (_strnicmp(name, "E:WATER", 7) == 0);
}

/* Scan a MeshWorld's object list for E:WATER objects.
 * Returns the number of water planes found. */
static int scan_meshworld_for_water(void *meshworld, const char *label)
{
    if (!meshworld || IsBadReadPtr(meshworld, 0x488)) {
        log_msg("scan_meshworld: %s meshworld is NULL or unreadable\n", label);
        return 0;
    }

    void *obj_list = (char *)meshworld + OFF_MW_OBJ_LIST;
    int count = athena_get_count(obj_list);
    void **data = athena_get_data(obj_list);

    log_msg("scan_meshworld: %s has %d objects, data=%p\n", label, count, data);

    if (count < 1 || !data) return 0;

    int found = 0;

    for (int i = 0; i < count && found < MAX_WATER_PLANES; i++) {
        if (IsBadReadPtr(&data[i], sizeof(void*))) break;
        void *obj = data[i];
        if (!obj || IsBadReadPtr(obj, 0x874)) continue;

        const char *name = get_meshbuffer_name(obj);
        if (!name) {
            log_msg("  %s[%d]: (null name)\n", label, i);
            continue;
        }

        /* Log ALL object names for debugging */
        log_msg("  %s[%d]: '%s'\n", label, i, name);

        if (is_water_name(name)) {
            float face_y = read_meshbuffer_face_y(obj);
            if (found < MAX_WATER_PLANES) {
                g_planes[found].surface_y = face_y;
                g_planes[found].found = 1;
                found++;
                log_msg("  FOUND water plane #%d: name='%s' surface_y=%.2f (from %s)\n",
                        found - 1, name, face_y, label);
            }
        }
    }

    return found;
}

/* Scan the game's collision MeshWorld for E:WATER planes.
 * The collision MeshWorld is NOT at Scene+0x08 (that's NULL at runtime).
 * It's inside the CollisionLevel at Scene+0x8B0, at offset +0x08.
 * (Confirmed by FUN_004606d0 which reads param_1+8 as collision MW,
 *  where param_1 can be either Scene or CollisionLevel.) */
static void scan_for_water_planes(void *scene)
{
    /* Clear previous results */
    memset(g_planes, 0, sizeof(g_planes));
    g_plane_count = 0;

    if (!scene || IsBadReadPtr(scene, 0x1000)) {
        log_msg("scan_for_water_planes: scene is NULL or unreadable\n");
        return;
    }

    log_msg("scan_for_water_planes: scene=%p\n", scene);

    /* Dump first 32 bytes of scene struct for debugging */
    {
        unsigned int *p = (unsigned int *)scene;
        log_msg("  scene raw: +0x00=%08X +0x04=%08X +0x08=%08X +0x0C=%08X\n",
                p[0], p[1], p[2], p[3]);
        log_msg("  scene raw: +0x10=%08X +0x14=%08X +0x18=%08X +0x1C=%08X\n",
                p[4], p[5], p[6], p[7]);
    }

    /* Try Scene+0x08 first (should be NULL but check anyway) */
    void *collision_mw_sc = *(void **)((char *)scene + OFF_SCENE_COLLISION_MW);
    int n1 = 0;
    if (collision_mw_sc && !IsBadReadPtr(collision_mw_sc, 0x488)) {
        n1 = scan_meshworld_for_water(collision_mw_sc, "scene+0x08");
    } else {
        log_msg("scan_for_water_planes: Scene+0x08 collision MW is NULL or unreadable\n");
    }

    /* Try CollisionLevel at Scene+0x8B0.
     * CollisionLevel+0x08 = collision MeshWorld (from FUN_004606d0 raw decomp).
     * CollisionLevel+0x18 = runtime collision object list (AthenaList). */
    int n2 = 0;
    void *collision_level = *(void **)((char *)scene + 0x8B0);
    if (collision_level && !IsBadReadPtr(collision_level, 0x10D0)) {
        log_msg("scan_for_water_planes: CollisionLevel (Scene+0x8B0)=%p\n", collision_level);

        /* Dump CollisionLevel first 32 bytes */
        {
            unsigned int *cl = (unsigned int *)collision_level;
            log_msg("  collvl raw: +0x00=%08X +0x04=%08X +0x08=%08X +0x0C=%08X\n",
                    cl[0], cl[1], cl[2], cl[3]);
            log_msg("  collvl raw: +0x10=%08X +0x14=%08X +0x18=%08X +0x1C=%08X\n",
                    cl[4], cl[5], cl[6], cl[7]);
        }

        /* Try CollisionLevel+0x08 as collision MeshWorld */
        void *cl_mw = *(void **)((char *)collision_level + 0x08);
        if (cl_mw && !IsBadReadPtr(cl_mw, 0x488)) {
            n2 = scan_meshworld_for_water(cl_mw, "collvl+0x08");
        } else {
            log_msg("scan_for_water_planes: CollisionLevel+0x08 MW is NULL or unreadable\n");
        }

        /* Also dump CollisionLevel+0x18 list (runtime collision objects) */
        {
            void *cl_list = (char *)collision_level + 0x18;
            int cl_count = athena_get_count(cl_list);
            void **cl_data = athena_get_data(cl_list);
            log_msg("scan_for_water_planes: CollisionLevel+0x18 list has %d items, data=%p\n",
                    cl_count, cl_data);

            /* Dump names of items in this list for debugging */
            if (cl_count > 0 && cl_data) {
                for (int i = 0; i < cl_count && i < 20; i++) {
                    if (IsBadReadPtr(&cl_data[i], sizeof(void*))) break;
                    void *item = cl_data[i];
                    if (!item || IsBadReadPtr(item, 0x20)) {
                        log_msg("  cl_list[%d]: (null or unreadable)\n", i);
                        continue;
                    }
                    /* Try reading name at +0x864 (MeshBuffer name offset) */
                    char *name = *(char **)((char *)item + 0x864);
                    if (name && !IsBadReadPtr(name, 1)) {
                        log_msg("  cl_list[%d]: %p name@0x864='%s'\n", i, item, name);
                    } else {
                        /* Try reading as a raw object with name at offset 0 */
                        char *name0 = *(char **)item;
                        if (name0 && !IsBadReadPtr(name0, 1)) {
                            log_msg("  cl_list[%d]: %p name@0x00='%s'\n", i, item, name0);
                        } else {
                            log_msg("  cl_list[%d]: %p (no readable name)\n", i, item);
                        }
                    }
                }
            }
        }
    } else {
        log_msg("scan_for_water_planes: CollisionLevel (Scene+0x8B0) is NULL or unreadable\n");
    }

    g_plane_count = n1 + n2;

    /* Fallback to INI-configured planes */
    if (g_plane_count == 0 && g_cfg.plane_count > 0) {
        for (int i = 0; i < g_cfg.plane_count && g_plane_count < MAX_WATER_PLANES; i++) {
            g_planes[g_plane_count].surface_y = g_cfg.plane_y[i];
            g_planes[g_plane_count].found = 1;
            g_plane_count++;
            log_msg("  FOUND water plane #%d from INI: surface_y=%.2f\n",
                    g_plane_count - 1, g_cfg.plane_y[i]);
        }
    }

    log_msg("scan_for_water_planes: total %d water planes found (scene+0x08=%d, collvl+0x08=%d)\n",
            g_plane_count, n1, n2);
    g_scan_done = 1;
}

/* ---- Water state tracking ---- */

static water_state_t *get_ball_state(void *ball)
{
    int free_idx = -1;
    for (int i = 0; i < MAX_BALLS; i++) {
        if (g_states[i].ball == ball) return &g_states[i];
        if (free_idx == -1 && g_states[i].ball == NULL) free_idx = i;
    }
    if (free_idx >= 0) {
        memset(&g_states[free_idx], 0, sizeof(water_state_t));
        g_states[free_idx].ball = ball;
        return &g_states[free_idx];
    }
    return NULL;
}

/* Check if the ball is currently touching/inside any water plane.
 * Returns the surface Y, or -99999.0f if not in water.
 * Engine uses Y-up: positive Y is up, gravity pulls toward -Y.
 * "Bottom of ball" (lowest point) = ball_y - radius. */
static float check_in_water(float ball_y, float radius)
{
    float bottom_y = ball_y - radius;  /* lowest point of ball (Y-up) */

    for (int i = 0; i < g_plane_count; i++) {
        if (!g_planes[i].found) continue;
        float surface_y = g_planes[i].surface_y;
        /* Ball is in water if its bottom is below the surface */
        if (bottom_y < surface_y) {
            return surface_y;
        }
    }
    return -99999.0f;
}

/* Compute submersion fraction: 0 = just touching, 0.5 = half submerged, 1 = fully under.
 * Y-up: ball falls by decreasing Y. Surface is at surface_y.
 * Bottom (deepest) = ball_y - radius. Top (shallowest) = ball_y + radius. */
static float compute_submersion(float ball_y, float radius, float surface_y)
{
    float bottom_y = ball_y - radius;  /* lowest point (deepest in water) */
    float top_y = ball_y + radius;    /* highest point (shallowest) */

    if (bottom_y >= surface_y) return 0.0f;  /* ball completely above water */
    if (top_y <= surface_y) return 1.0f;     /* ball completely below water */

    float submerged = (surface_y - bottom_y) / (2.0f * radius);
    if (submerged < 0.0f) submerged = 0.0f;
    if (submerged > 1.0f) submerged = 1.0f;
    return submerged;
}

/* ---- Water physics ---- */

/* Apply water physics to the position delta.
 *
 * position delta (dx, dy, dz) is the change from pre-update to post-update position.
 * We modify it in-place to simulate water behavior.
 *
 * Requirements:
 * 1. Entry damping: 30% vertical speed cut on first contact while falling
 * 2. Drag: small per-frame velocity reduction
 * 3. Buoyancy: upward force proportional to submersion depth
 * 4. Equilibrium at half-submerged (net vertical force = 0)
 * 5. Horizontal dampening: slightly lower max horizontal speed
 * 6. No extra max vertical velocity cap
 */
static void apply_water_physics(void *ball, water_state_t *st,
                                float *dx, float *dy, float *dz,
                                float surface_y)
{
    float radius = get_ball_radius(ball);
    float ball_y = get_ball_pos_y(ball);

    /* Compute submersion depth: how far the ball's bottom is below the surface */
    float submerged = compute_submersion(ball_y, radius, surface_y);

    if (submerged <= 0.0f) {
        /* Ball is no longer touching water */
        if (st->in_water) {
            log_msg("[water] ball %p exited water (was in, now above surface)\n", ball);
            st->in_water = 0;
            st->entry_damped = 0;
        }
        return;
    }

    /* Ball is in water */
    if (!st->in_water) {
        /* Just entered water this frame */
        st->in_water = 1;
        st->entry_damped = 0;
        log_msg("[water] ball %p entered water at surface_y=%.2f ball_y=%.2f\n",
                ball, surface_y, ball_y);
    }

    /* 1. Entry damping: on first contact while falling (dy < 0 = downward in Y-up),
     *    reduce vertical speed by entry_damping factor (default 30% reduction). */
    if (!st->entry_damped && *dy < 0.0f) {
        float original_dy = *dy;
        *dy *= g_cfg.entry_damping;
        st->entry_damped = 1;
        log_msg("[water] entry damping: dy %.3f -> %.3f (factor %.2f)\n",
                original_dy, *dy, g_cfg.entry_damping);
    }

    /* 2. Drag: reduce all velocity components by a small factor each frame. */
    float vscale = 1.0f - g_cfg.drag;
    float hscale = 1.0f - (g_cfg.drag + g_cfg.horizontal_drag);
    if (vscale < 0.0f) vscale = 0.0f;
    if (hscale < 0.0f) hscale = 0.0f;

    /* 5. Horizontal dampening: extra drag on X and Z axes.
     *    This slightly lowers the max horizontal velocity in water. */
    *dx *= hscale;
    *dz *= hscale;
    *dy *= vscale;

    /* 3. Buoyancy: upward force proportional to submersion depth.
     *    At submerged=0 (just touching): no buoyancy, full gravity.
     *    At submerged=0.5 (half submerged): buoyancy = gravity_equivalent, net force = 0.
     *    At submerged=1.0 (fully submerged): buoyancy = 2*gravity_equivalent, net upward.
     *
     * 4. Equilibrium: at half-submerged, the buoyancy exactly cancels gravity,
     *    so the ball floats with zero vertical acceleration.
     *
     * In Y-up: upward = positive dy. We ADD buoyancy to dy (push up).
     * Gravity already pulled dy negative (downward); buoyancy counteracts it. */
    float buoyancy = g_cfg.gravity_equivalent * submerged * 2.0f;
    *dy += buoyancy;

    /* 6. No extra maximum vertical velocity cap is applied.
     *    The engine's own CollisionMesh speed limit remains unchanged. */

    log_msg("[water] ball %p: subm=%.2f dy=%.3f->%.3f buoy=%.3f dx=%.3f dz=%.3f\n",
            ball, submerged, *dy - buoyancy, *dy, buoyancy, *dx, *dz);
}

/* ---- Hooks ---- */

typedef void (__thiscall *ball_update_t)(void *ball);

static ball_update_t orig_Ball_Update = NULL;
static volatile int g_hook_calls = 0;

static void __thiscall Hook_Ball_Update(void *ball)
{
    g_hook_calls++;
    if (g_hook_calls <= 3 || (g_hook_calls % 500) == 0) {
        log_msg("Hook_Ball_Update called #%d, ball=%p\n", g_hook_calls, ball);
    }

    if (!ball) {
        if (orig_Ball_Update) orig_Ball_Update(ball);
        return;
    }

    /* Get scene pointer for water plane scanning */
    void *scene = get_scene(ball);

    if (g_hook_calls <= 3) {
        log_msg("  ball=%p scene=%p scan_done=%d\n", ball, scene, g_scan_done);
    }

    /* Scan for water planes if not done yet or scene changed */
    if (!g_scan_done || scene != g_last_scene) {
        g_last_scene = scene;
        scan_for_water_planes(scene);
    }

    /* If no water planes exist, just call original and return */
    if (g_plane_count == 0) {
        if (orig_Ball_Update) orig_Ball_Update(ball);
        return;
    }

    /* Save pre-update position */
    float old_x = get_ball_pos_x(ball);
    float old_y = get_ball_pos_y(ball);
    float old_z = get_ball_pos_z(ball);

    /* Call original Ball_Update */
    if (orig_Ball_Update) orig_Ball_Update(ball);

    /* Read post-update position */
    float new_x = get_ball_pos_x(ball);
    float new_y = get_ball_pos_y(ball);
    float new_z = get_ball_pos_z(ball);

    /* Compute position delta (what Ball_Update did this frame) */
    float dx = new_x - old_x;
    float dy = new_y - old_y;
    float dz = new_z - old_z;

    /* Get per-ball water state */
    water_state_t *st = get_ball_state(ball);
    if (!st) return;

    /* Check if ball is in water using post-update position */
    float surface_y = check_in_water(new_y, get_ball_radius(ball));

    if (surface_y > -99998.0f) {
        /* Ball is in water — apply physics to the delta */
        apply_water_physics(ball, st, &dx, &dy, &dz, surface_y);
    } else {
        /* Ball is not in water — clear state if was previously in water */
        if (st->in_water) {
            st->in_water = 0;
            st->entry_damped = 0;
        }
    }

    /* Apply modified position: old_position + modified_delta */
    set_ball_pos(ball, old_x + dx, old_y + dy, old_z + dz);
}

/* ---- Apply patches (run once from a background thread) ---- */

/* Heartbeat thread: logs every 5 seconds to prove the mod is alive */
static DWORD WINAPI heartbeat_thread(LPVOID lpParam)
{
    (void)lpParam;
    int ticks = 0;
    while (1) {
        Sleep(5000);
        ticks++;
        log_msg("heartbeat #%d (mod is running, hook_calls=%d)\n", ticks, g_hook_calls);
    }
    return 0;
}

static void apply_patches(const char *log_path)
{
    open_log_fallback(log_path);

    char exe_path[MAX_PATH];
    GetModuleFileNameA(NULL, exe_path, sizeof(exe_path));
    log_msg("Water mod initializing. exe=%s\n", exe_path);
    log_msg("bass_real.dll handle=0x%p\n", (void *)g_hRealBass);
    log_msg("Config: debug=%d entry_damp=%.2f drag=%.3f hdrag=%.3f gravity_eq=%.3f ini_planes=%d\n",
            g_cfg.debug, g_cfg.entry_damping, g_cfg.drag, g_cfg.horizontal_drag,
            g_cfg.gravity_equivalent, g_cfg.plane_count);

    /* Start heartbeat thread */
    CreateThread(NULL, 0, heartbeat_thread, NULL, 0, NULL);

    /* Dump first 16 bytes of Ball_Update thunk at 0x408390 */
    {
        unsigned char *fn = (unsigned char *)0x00408390;
        if (!IsBadReadPtr(fn, 16)) {
            log_msg("Ball_Update thunk at 0x408390 bytes: %02X %02X %02X %02X %02X %02X %02X %02X\n",
                    fn[0], fn[1], fn[2], fn[3], fn[4], fn[5], fn[6], fn[7]);
        }
    }

    /* Dump vtable slot BEFORE patching */
    {
        void **slot = (void **)(ADDR_BALL_VTABLE + VTABLE_SLOT_UPDATE * sizeof(void*));
        if (!IsBadReadPtr(slot, sizeof(void*))) {
            log_msg("vtable[4] BEFORE patch: slot=%p value=0x%p\n", slot, *slot);
        }
    }

    log_msg("Applying vtable hook...\n");

    int slot_ok = patch_vtable_slot(VTABLE_SLOT_UPDATE, Hook_Ball_Update, (void **)&orig_Ball_Update);

    log_msg("Ball_Update vtable[%d] hook (orig=0x%p): %s\n",
            VTABLE_SLOT_UPDATE, orig_Ball_Update, slot_ok ? "OK" : "FAILED");

    /* Verify the patch actually stuck by re-reading the slot */
    {
        void **slot = (void **)(ADDR_BALL_VTABLE + VTABLE_SLOT_UPDATE * sizeof(void*));
        if (!IsBadReadPtr(slot, sizeof(void*))) {
            log_msg("vtable[4] AFTER patch: slot=%p value=0x%p (hook=%p, match=%d)\n",
                    slot, *slot, (void*)Hook_Ball_Update, *slot == (void*)Hook_Ball_Update);
        }
    }

    if (slot_ok) {
        log_msg("Water mod active. Will scan for E:WATER objects on first Ball_Update.\n");
    }
}

/* ---- Background loader thread ---- */

static DWORD WINAPI mod_thread(LPVOID lpParam)
{
    (void)lpParam;

    char ini_path[MAX_PATH];
    char log_path[MAX_PATH];

    GetModuleFileNameA(NULL, ini_path, MAX_PATH);
    char *p = strrchr(ini_path, '\\');
    if (p) strcpy(p + 1, "hamsterball_water.ini");
    else    strcat(ini_path, "hamsterball_water.ini");

    GetModuleFileNameA(NULL, log_path, MAX_PATH);
    p = strrchr(log_path, '\\');
    char *dot = p ? strrchr(p, '.') : NULL;
    if (dot) strcpy(dot, "_water_mod.log");
    else     strcat(log_path, "_water_mod.log");

    load_config(ini_path);

    /* Wait for the game to finish loading/initializing the Ball vtable. */
    Sleep(1500);
    apply_patches(log_path);
    return 0;
}

/* ---- BASS proxy exports (__stdcall) ---- */

__declspec(dllexport) void __stdcall BASS_Stop(void)
{ if (real_BASS_Stop) real_BASS_Stop(); }

__declspec(dllexport) int __stdcall BASS_ChannelSetAttributes(DWORD handle, float freq, int vol, int pan)
{ if (real_BASS_ChannelSetAttributes) return real_BASS_ChannelSetAttributes(handle, freq, vol, pan); return 0; }

__declspec(dllexport) int __stdcall BASS_Free(void)
{ if (real_BASS_Free) return real_BASS_Free(); return 0; }

__declspec(dllexport) int __stdcall BASS_Init(int device, DWORD freq, DWORD flags, HWND win, void *clsid)
{ if (real_BASS_Init) return real_BASS_Init(device, freq, flags, win, clsid); return 0; }

__declspec(dllexport) int __stdcall BASS_Start(void)
{ if (real_BASS_Start) return real_BASS_Start(); return 0; }

__declspec(dllexport) int __stdcall BASS_SetConfig(DWORD option, DWORD value)
{ if (real_BASS_SetConfig) return real_BASS_SetConfig(option, value); return 0; }

__declspec(dllexport) int __stdcall BASS_ChannelStop(DWORD handle)
{ if (real_BASS_ChannelStop) return real_BASS_ChannelStop(handle); return 0; }

__declspec(dllexport) int __stdcall BASS_MusicPlayEx(DWORD handle, DWORD pos, BOOL seek)
{ if (real_BASS_MusicPlayEx) return real_BASS_MusicPlayEx(handle, pos, seek); return 0; }

__declspec(dllexport) int __stdcall BASS_ErrorGetCode(void)
{ if (real_BASS_ErrorGetCode) return real_BASS_ErrorGetCode(); return 0; }

__declspec(dllexport) DWORD __stdcall BASS_MusicLoad(int mem, void *file, DWORD offset, DWORD length, DWORD flags, DWORD freq)
{ if (real_BASS_MusicLoad) return real_BASS_MusicLoad(mem, file, offset, length, flags, freq); return 0; }

/* ---- DllMain ---- */

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    (void)hinstDLL;
    (void)lpvReserved;

    switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hinstDLL);

        g_hRealBass = LoadLibraryA("bass_real.dll");
        if (!g_hRealBass) {
            char path[MAX_PATH];
            if (GetModuleFileNameA(hinstDLL, path, MAX_PATH)) {
                char *p = strrchr(path, '\\');
                if (p) {
                    strcpy(p + 1, "bass_real.dll");
                    g_hRealBass = LoadLibraryA(path);
                }
            }
        }

        if (g_hRealBass) {
            real_BASS_Stop                 = (BASS_Stop_t)GetProcAddress(g_hRealBass, "BASS_Stop");
            real_BASS_ChannelSetAttributes = (BASS_ChannelSetAttributes_t)GetProcAddress(g_hRealBass, "BASS_ChannelSetAttributes");
            real_BASS_Free                 = (BASS_Free_t)GetProcAddress(g_hRealBass, "BASS_Free");
            real_BASS_Init                 = (BASS_Init_t)GetProcAddress(g_hRealBass, "BASS_Init");
            real_BASS_Start                = (BASS_Start_t)GetProcAddress(g_hRealBass, "BASS_Start");
            real_BASS_SetConfig            = (BASS_SetConfig_t)GetProcAddress(g_hRealBass, "BASS_SetConfig");
            real_BASS_ChannelStop          = (BASS_ChannelStop_t)GetProcAddress(g_hRealBass, "BASS_ChannelStop");
            real_BASS_MusicPlayEx          = (BASS_MusicPlayEx_t)GetProcAddress(g_hRealBass, "BASS_MusicPlayEx");
            real_BASS_ErrorGetCode         = (BASS_ErrorGetCode_t)GetProcAddress(g_hRealBass, "BASS_ErrorGetCode");
            real_BASS_MusicLoad            = (BASS_MusicLoad_t)GetProcAddress(g_hRealBass, "BASS_MusicLoad");
        }

        CreateThread(NULL, 0, mod_thread, NULL, 0, NULL);
        break;

    case DLL_PROCESS_DETACH:
        if (g_log) { fclose(g_log); g_log = NULL; }
        if (g_hRealBass) { FreeLibrary(g_hRealBass); g_hRealBass = NULL; }
        break;
    }
    return TRUE;
}
