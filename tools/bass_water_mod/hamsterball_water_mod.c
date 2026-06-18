/*
 * hamsterball_water_mod.c — BASS.dll proxy that adds realistic E:WATER physics.
 *
 * Installation (Windows):
 *   1. In your Hamsterball game folder, rename bass.dll -> bass_real.dll
 *   2. Copy this proxy bass.dll into the game folder
 *   3. Copy hamsterball_water.ini into the game folder
 *   4. Place invisible collision planes named "E:WATER" in your level
 *      (the E: prefix makes them invisible event triggers)
 *
 * How it works:
 *   - The game loads this proxy instead of the real bass.dll.
 *   - We forward all BASS calls to bass_real.dll.
 *   - A background thread patches:
 *       * Ball vtable[4] (Ball_Update) -> our wrapper
 *       * CALL CreateNoDizzy in Level_HandleCollision / Arena_HandleCollision
 *   - The CreateNoDizzy hook detects "E:WATER" collisions and records
 *     per-ball water state (surface height, entry-damped flag).
 *   - The Ball_Update wrapper applies buoyancy/drag while the ball is
 *     marked as touching water.
 *
 * Build: i686-w64-mingw32-gcc -shared -o bass.dll hamsterball_water_mod.c \
 *          bass_exports.def -lwinmm -Wl,--enable-stdcall-fixup -O2 \
 *          -static -static-libgcc -Wl,--add-stdcall-alias
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winuser.h>
#include <mmsystem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ---- BASS Function Types (__stdcall) ---- */
typedef void (__stdcall *BASS_Stop_t)(void);
typedef int  (__stdcall *BASS_ChannelSetAttributes_t)(DWORD, float, int, int);
typedef int  (__stdcall *BASS_Free_t)(void);
typedef int  (__stdcall *BASS_Init_t)(int, DWORD, DWORD, HWND, void*);
typedef int  (__stdcall *BASS_Start_t)(void);
typedef int  (__stdcall *BASS_SetConfig_t)(DWORD, DWORD);
typedef int  (__stdcall *BASS_ChannelStop_t)(DWORD);
typedef int  (__stdcall *BASS_MusicPlayEx_t)(DWORD, DWORD, BOOL);
typedef int  (__stdcall *BASS_ErrorGetCode_t)(void);
typedef DWORD (__stdcall *BASS_MusicLoad_t)(int, void*, DWORD, DWORD, DWORD, DWORD);

static HMODULE g_hRealBass = NULL;
static BASS_Stop_t              real_BASS_Stop = NULL;
static BASS_ChannelSetAttributes_t real_BASS_ChannelSetAttributes = NULL;
static BASS_Free_t              real_BASS_Free = NULL;
static BASS_Init_t              real_BASS_Init = NULL;
static BASS_Start_t             real_BASS_Start = NULL;
static BASS_SetConfig_t         real_BASS_SetConfig = NULL;
static BASS_ChannelStop_t       real_BASS_ChannelStop = NULL;
static BASS_MusicPlayEx_t       real_BASS_MusicPlayEx = NULL;
static BASS_ErrorGetCode_t      real_BASS_ErrorGetCode = NULL;
static BASS_MusicLoad_t         real_BASS_MusicLoad = NULL;

/* ---- Config ---- */
typedef struct {
    float entry_damping;       /* vertical speed multiplier on entry (0.7 = lose 30%) */
    float drag;                /* velocity drag per frame when fully submerged */
    float horizontal_drag;     /* extra horizontal velocity drag per frame */
    float gravity_equivalent;  /* downward acceleration per frame used for buoyancy balance */
    float accel_damp;          /* horizontal acceleration dampening (max speed scale) */
    int   timer_frames;        /* frames before water state expires without refresh */
    int   debug;               /* write detailed log */
} water_config_t;

static water_config_t g_cfg = {
    0.70f,   /* entry_damping */
    0.03f,   /* drag */
    0.04f,   /* horizontal_drag */
    0.45f,   /* gravity_equivalent */
    0.92f,   /* accel_damp */
    3,       /* timer_frames */
    0        /* debug */
};

/* ---- Game addresses (RVA from image base 0x400000) ---- */
#define GAME_BASE_VA          0x00400000
#define ADDR_BALL_VTABLE      0x004CF3A0   /* global Ball vtable */
#define ADDR_BALL_UPDATE_SLOT 0x004CF3B0   /* vtable[4] -> Ball_Update thunk */
#define ADDR_CREATENODIZZY    0x0040C5D0
#define ADDR_LEVEL_HANDLER    0x0040DCD0
#define ADDR_ARENA_HANDLER    0x0040E6A0

/* ---- Ball struct offsets ---- */
#define OFF_BALL_VEL_X        0x170
#define OFF_BALL_VEL_Y        0x174
#define OFF_BALL_VEL_Z        0x178
#define OFF_BALL_POS_X        0x164
#define OFF_BALL_POS_Y        0x168
#define OFF_BALL_POS_Z        0x16C
#define OFF_BALL_RADIUS       0x284
#define OFF_BALL_MAX_SPEED    0x188
#define OFF_BALL_SPEED_SCALE  0x18C
#define OFF_BALL_ACCEL_X      0x2BC
#define OFF_BALL_ACCEL_Y      0x2C0
#define OFF_BALL_ACCEL_Z      0x2C4
#define OFF_BALL_IS_FALLING   0x281
#define OFF_BALL_WATER_FLAG   0x2D5   /* original N:WATER flag */

/* ---- Collider offset ---- */
#define OFF_COLLIDER_NAME     0x864

static DWORD g_game_base = 0;
static FILE *g_log = NULL;

static inline DWORD game_rva(DWORD va) {
    return g_game_base + (va - GAME_BASE_VA);
}

static inline float read_f(void *base, DWORD off) {
    return *(float *)((BYTE *)base + off);
}
static inline void write_f(void *base, DWORD off, float v) {
    *(float *)((BYTE *)base + off) = v;
}
static inline BYTE read_b(void *base, DWORD off) {
    return *(BYTE *)((BYTE *)base + off);
}
static inline void write_b(void *base, DWORD off, BYTE v) {
    *(BYTE *)((BYTE *)base + off) = v;
}

/* ---- Water state ---- */
#define MAX_BALLS 64
#define MAX_ZONES 16

typedef struct {
    void *ball;
    BOOL  in_water;
    BOOL  entry_damped;
    int   timer;
    float surface_y;
} ball_water_state_t;

typedef struct {
    BOOL  active;
    float min_x, min_y, min_z;
    float max_x, max_y, max_z;
    float surface_y;
} water_zone_t;

static ball_water_state_t g_balls[MAX_BALLS];
static water_zone_t g_zones[MAX_ZONES];
static int g_zone_count = 0;

static CRITICAL_SECTION g_water_cs;

/* ---- Helpers ---- */
static int file_exists(const char *path)
{
    DWORD attr = GetFileAttributesA(path);
    return attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY);
}

static int read_ini_int(const char *path, const char *section, const char *key, int default_val)
{
    char buf[32];
    DWORD n = GetPrivateProfileStringA(section, key, "", buf, sizeof(buf), path);
    if (n == 0) return default_val;
    return atoi(buf);
}

static float read_ini_float(const char *path, const char *section, const char *key, float default_val)
{
    char buf[32];
    DWORD n = GetPrivateProfileStringA(section, key, "", buf, sizeof(buf), path);
    if (n == 0) return default_val;
    return (float)atof(buf);
}

static void load_config(const char *ini_path)
{
    if (!file_exists(ini_path)) {
        FILE *f = fopen(ini_path, "w");
        if (f) {
            fprintf(f, "[WaterPhysics]\n");
            fprintf(f, "; Vertical velocity multiplier on first entry while falling.\n");
            fprintf(f, "; 0.70 means the ball keeps 70%% of its downward speed (loses 30%%).\n");
            fprintf(f, "EntryDamping=0.70\n\n");
            fprintf(f, "; Overall velocity drag per frame while fully submerged (0.03 = 3%%).\n");
            fprintf(f, "Drag=0.03\n\n");
            fprintf(f, "; Extra horizontal-only drag per frame while in water.\n");
            fprintf(f, "HorizontalDrag=0.04\n\n");
            fprintf(f, "; Effective gravity per frame. Buoyancy is tuned so a ball floats\n");
            fprintf(f, "; half-submerged when vertical velocity reaches zero.\n");
            fprintf(f, "GravityEquivalent=0.45\n\n");
            fprintf(f, "; Horizontal acceleration dampening. 0.92 lowers the max horizontal\n");
            fprintf(f, "; speed in water to about 92%% of normal.\n");
            fprintf(f, "AccelerationDamp=0.92\n\n");
            fprintf(f, "; Frames a water effect persists without the ball touching the plane again.\n");
            fprintf(f, "TimerFrames=3\n\n");
            fprintf(f, "[Debug]\n");
            fprintf(f, "; Set to 1 to write a water_mod.log in the game folder.\n");
            fprintf(f, "Debug=0\n");
            fclose(f);
        }
    }
    g_cfg.entry_damping   = read_ini_float(ini_path, "WaterPhysics", "EntryDamping", 0.70f);
    g_cfg.drag            = read_ini_float(ini_path, "WaterPhysics", "Drag", 0.03f);
    g_cfg.horizontal_drag = read_ini_float(ini_path, "WaterPhysics", "HorizontalDrag", 0.04f);
    g_cfg.gravity_equivalent = read_ini_float(ini_path, "WaterPhysics", "GravityEquivalent", 0.45f);
    g_cfg.accel_damp      = read_ini_float(ini_path, "WaterPhysics", "AccelerationDamp", 0.92f);
    g_cfg.timer_frames    = read_ini_int(ini_path, "WaterPhysics", "TimerFrames", 3);
    g_cfg.debug           = read_ini_int(ini_path, "Debug", "Debug", 0);

    if (g_cfg.entry_damping < 0.0f) g_cfg.entry_damping = 0.0f;
    if (g_cfg.entry_damping > 1.0f) g_cfg.entry_damping = 1.0f;
    if (g_cfg.drag < 0.0f) g_cfg.drag = 0.0f;
    if (g_cfg.drag > 1.0f) g_cfg.drag = 1.0f;
    if (g_cfg.horizontal_drag < 0.0f) g_cfg.horizontal_drag = 0.0f;
    if (g_cfg.horizontal_drag > 1.0f) g_cfg.horizontal_drag = 1.0f;
    if (g_cfg.timer_frames < 1) g_cfg.timer_frames = 1;
    if (g_cfg.timer_frames > 60) g_cfg.timer_frames = 60;
}

/* ---- Hook state ---- */
typedef void (__thiscall *Ball_Update_t)(void *ball);
typedef void (__thiscall *CreateNoDizzy_t)(void *this_, void *ball, void *collObj);

static Ball_Update_t orig_Ball_Update = NULL;
static CreateNoDizzy_t orig_CreateNoDizzy = NULL;

/* ---- Memory patching ---- */
static int patch_call_targets(BYTE *start, SIZE_T size, DWORD target, void *new_target, int max_hits)
{
    int hits = 0;
    for (SIZE_T i = 0; i + 5 <= size && hits < max_hits; i++) {
        if (start[i] == 0xE8) {
            DWORD rel;
            memcpy(&rel, start + i + 1, 4);
            DWORD dst = (DWORD)(start + i + 5) + rel;
            if (dst == target) {
                DWORD new_rel = (DWORD)new_target - (DWORD)(start + i + 5);
                DWORD oldProt;
                if (VirtualProtect(start + i + 1, 4, PAGE_EXECUTE_READWRITE, &oldProt)) {
                    memcpy(start + i + 1, &new_rel, 4);
                    VirtualProtect(start + i + 1, 4, oldProt, &oldProt);
                    FlushInstructionCache(GetCurrentProcess(), start + i + 1, 4);
                    hits++;
                }
            }
        }
    }
    return hits;
}

/* ---- Water physics ---- */
static ball_water_state_t *get_ball_state(void *ball, int create)
{
    for (int i = 0; i < MAX_BALLS; i++) {
        if (g_balls[i].ball == ball)
            return &g_balls[i];
    }
    if (!create) return NULL;
    for (int i = 0; i < MAX_BALLS; i++) {
        if (g_balls[i].ball == NULL) {
            g_balls[i].ball = ball;
            return &g_balls[i];
        }
    }
    return NULL;
}

static water_zone_t *find_or_create_zone(float x, float y, float pos_z, float surface_y)
{
    /* Reuse an existing zone if the surface is close and X/Z is within a
     * generous horizontal box. Water planes are usually horizontal, so the
     * surface Y is the main discriminant. */
    for (int i = 0; i < g_zone_count; i++) {
        if (!g_zones[i].active) continue;
        if (fabsf(g_zones[i].surface_y - surface_y) < 5.0f &&
            x >= g_zones[i].min_x && x <= g_zones[i].max_x &&
            pos_z >= g_zones[i].min_z && pos_z <= g_zones[i].max_z) {
            return &g_zones[i];
        }
    }
    if (g_zone_count >= MAX_ZONES) return NULL;
    water_zone_t *zone = &g_zones[g_zone_count++];
    zone->active = TRUE;
    zone->surface_y = surface_y;
    zone->min_x = x - 150.0f;
    zone->max_x = x + 150.0f;
    zone->min_y = surface_y - 80.0f;
    zone->max_y = surface_y + 80.0f;
    zone->min_z = pos_z - 150.0f;
    zone->max_z = pos_z + 150.0f;
    return zone;
}

static BOOL point_in_zone(water_zone_t *zone, float x, float y, float pos_z)
{
    return (x >= zone->min_x && x <= zone->max_x &&
            y >= zone->min_y && y <= zone->max_y &&
            pos_z >= zone->min_z && pos_z <= zone->max_z);
}

static water_zone_t *ball_get_zone(void *ball)
{
    float x = read_f(ball, OFF_BALL_POS_X);
    float y = read_f(ball, OFF_BALL_POS_Y);
    float z = read_f(ball, OFF_BALL_POS_Z);
    for (int i = 0; i < g_zone_count; i++) {
        if (g_zones[i].active && point_in_zone(&g_zones[i], x, y, z))
            return &g_zones[i];
    }
    return NULL;
}

/* Compute submersion ratio s in [0,1].
 * The game's gravity vector is (0,1,0), so +Y is DOWN. */
static float submersion_ratio(void *ball, float surface_y)
{
    float pos_y = read_f(ball, OFF_BALL_POS_Y);
    float radius = read_f(ball, OFF_BALL_RADIUS);
    if (radius <= 0.0f) radius = 27.0f;

    float bottom = pos_y + radius;  /* lowest point in +Y-down space */
    float top    = pos_y - radius;  /* highest point */

    if (top >= surface_y) return 1.0f;          /* fully below surface */
    if (bottom <= surface_y) return 0.0f;       /* fully above surface */

    float submerged = bottom - surface_y;
    float height = 2.0f * radius;
    return submerged / height;
}

static void apply_water_physics(void *ball)
{
    ball_water_state_t *st = get_ball_state(ball, 0);
    if (!st || !st->in_water) return;

    water_zone_t *zone = ball_get_zone(ball);
    if (zone) st->surface_y = zone->surface_y;

    float s = submersion_ratio(ball, st->surface_y);
    if (s <= 0.0f) {
        /* Still touching by timer but physically above water; only expire. */
        return;
    }

    float vx = read_f(ball, OFF_BALL_VEL_X);
    float vy = read_f(ball, OFF_BALL_VEL_Y);
    float vz = read_f(ball, OFF_BALL_VEL_Z);

    /* 1) Entry damping: first time entering while falling (+Y is down). */
    if (!st->entry_damped && vy > 0.0f) {
        vy *= g_cfg.entry_damping;
        st->entry_damped = TRUE;
        if (g_log) fprintf(g_log, "[Water] entry damp: vy -> %.3f (ball %p)\n", vy, ball);
    }

    /* 2) Buoyancy: scales with depth, equilibrium at half-submerged (s=0.5).
     *    net_delta_y = gravity_equivalent * (1 - 2*s)
     *    s < 0.5 -> net downward, s > 0.5 -> net upward. */
    float buoyancy = g_cfg.gravity_equivalent * (1.0f - 2.0f * s);
    vy -= buoyancy;  /* subtract because +Y is down */

    /* 3) Drag: overall velocity damping, stronger horizontal component. */
    float drag_total = g_cfg.drag * s;
    float drag_horiz = g_cfg.horizontal_drag * s;
    float keep_total = 1.0f - drag_total;
    float keep_horiz = 1.0f - drag_horiz;

    vx *= keep_horiz;
    vz *= keep_horiz;
    vy *= keep_total;

    /* 4) Horizontal acceleration dampening: lower the effective max speed
     *    while in water, making the cap on horizontal velocity slightly lower. */
    float max_speed = read_f(ball, OFF_BALL_MAX_SPEED);
    if (max_speed > 0.0f && s > 0.1f) {
        float damped_max = max_speed * g_cfg.accel_damp;
        write_f(ball, OFF_BALL_MAX_SPEED, damped_max);
    }

    write_f(ball, OFF_BALL_VEL_X, vx);
    write_f(ball, OFF_BALL_VEL_Y, vy);
    write_f(ball, OFF_BALL_VEL_Z, vz);

    /* Consider the ball "grounded" while in water so it can still be controlled. */
    write_b(ball, OFF_BALL_IS_FALLING, 0);
    write_b(ball, OFF_BALL_WATER_FLAG, 1);

    if (g_log && g_cfg.debug) {
        fprintf(g_log, "[Water] s=%.2f vy=%.3f vx=%.3f vz=%.3f (ball %p)\n",
                s, vy, vx, vz, ball);
    }
}

static void update_water_state(void *ball)
{
    ball_water_state_t *st = get_ball_state(ball, 0);
    if (!st) return;

    if (st->in_water) {
        st->timer--;
        if (st->timer <= 0) {
            st->in_water = FALSE;
            st->entry_damped = FALSE;
            write_b(ball, OFF_BALL_WATER_FLAG, 0);
            if (g_log) fprintf(g_log, "[Water] exit (ball %p)\n", ball);
        }
    }
}

/* ---- Hooks ---- */
static void __thiscall Hook_Ball_Update(void *ball)
{
    EnterCriticalSection(&g_water_cs);
    apply_water_physics(ball);
    LeaveCriticalSection(&g_water_cs);

    /* Call original physics tick. */
    if (orig_Ball_Update) orig_Ball_Update(ball);

    EnterCriticalSection(&g_water_cs);
    update_water_state(ball);
    LeaveCriticalSection(&g_water_cs);
}

static void __thiscall Hook_CreateNoDizzy(void *this_, void *ball, void *collObj)
{
    /* collObj is an array of pointers; collObj[1] is the MeshBuffer-like object
     * and the event name string lives at +0x864 inside it. */
    void **coll_array = (void **)collObj;
    if (!coll_array) goto forward;
    void *obj1 = coll_array[1];
    if (!obj1) goto forward;
    char *eventName = *(char **)((BYTE *)obj1 + OFF_COLLIDER_NAME);

    if (eventName) {
        if (_stricmp(eventName, "E:WATER") == 0 || _strnicmp(eventName, "E:WATER<", 8) == 0) {
            EnterCriticalSection(&g_water_cs);
            ball_water_state_t *st = get_ball_state(ball, 1);
            if (st) {
                float pos_y = read_f(ball, OFF_BALL_POS_Y);
                float radius = read_f(ball, OFF_BALL_RADIUS);
                if (radius <= 0.0f) radius = 27.0f;
                float surface_y = pos_y - radius;  /* bottom of ball touching plane */

                /* Refresh/initialize zone. */
                float x = read_f(ball, OFF_BALL_POS_X);
                float z = read_f(ball, OFF_BALL_POS_Z);
                water_zone_t *zone = find_or_create_zone(x, pos_y, z, surface_y);
                if (zone) st->surface_y = zone->surface_y;
                else st->surface_y = surface_y;

                /* If this is a fresh entry (was not in water), reset damped flag. */
                if (!st->in_water) st->entry_damped = FALSE;
                st->in_water = TRUE;
                st->timer = g_cfg.timer_frames;

                if (g_log) fprintf(g_log, "[Water] collision E:WATER ball=%p surface_y=%.2f\n",
                                   ball, st->surface_y);
            }
            LeaveCriticalSection(&g_water_cs);
        }
    }

forward:
    /* Forward to original handler. */
    if (orig_CreateNoDizzy) orig_CreateNoDizzy(this_, ball, collObj);
}

/* ---- Apply hooks ---- */
static int apply_hooks(void)
{
    int ok = 1;

    g_game_base = (DWORD)GetModuleHandle(NULL);
    if (!g_game_base) return 0;

    if (g_log) fprintf(g_log, "[Water] game base = 0x%08lX\n", g_game_base);

    /* Hook Ball_Update via vtable[4]. */
    DWORD *pBallUpdateSlot = (DWORD *)game_rva(ADDR_BALL_UPDATE_SLOT);
    if (!IsBadReadPtr(pBallUpdateSlot, 4) && !IsBadWritePtr(pBallUpdateSlot, 4)) {
        DWORD oldProt;
        if (VirtualProtect(pBallUpdateSlot, 4, PAGE_READWRITE, &oldProt)) {
            orig_Ball_Update = (Ball_Update_t)(*pBallUpdateSlot);
            *pBallUpdateSlot = (DWORD)Hook_Ball_Update;
            VirtualProtect(pBallUpdateSlot, 4, oldProt, &oldProt);
            FlushInstructionCache(GetCurrentProcess(), pBallUpdateSlot, 4);
            if (g_log) fprintf(g_log, "[Water] Ball_Update hooked: orig=0x%08lX new=0x%08lX\n",
                               (DWORD)orig_Ball_Update, (DWORD)Hook_Ball_Update);
        } else {
            ok = 0;
            if (g_log) fprintf(g_log, "[Water] FAILED to patch Ball_Update vtable\n");
        }
    } else {
        ok = 0;
        if (g_log) fprintf(g_log, "[Water] Ball_Update slot not accessible\n");
    }

    /* Hook CreateNoDizzy by patching the two CALL sites in the level/arena handlers. */
    if (ok) {
        BYTE *level_start = (BYTE *)game_rva(ADDR_LEVEL_HANDLER);
        BYTE *arena_start = (BYTE *)game_rva(ADDR_ARENA_HANDLER);
        int level_hits = patch_call_targets(level_start, 0x200, game_rva(ADDR_CREATENODIZZY), Hook_CreateNoDizzy, 1);
        int arena_hits = patch_call_targets(arena_start, 0x200, game_rva(ADDR_CREATENODIZZY), Hook_CreateNoDizzy, 1);
        if (g_log) fprintf(g_log, "[Water] CreateNoDizzy call patches: level=%d arena=%d\n",
                           level_hits, arena_hits);
        if (level_hits == 0 && arena_hits == 0) {
            ok = 0;
            if (g_log) fprintf(g_log, "[Water] FAILED to patch CreateNoDizzy calls\n");
        } else {
            orig_CreateNoDizzy = (CreateNoDizzy_t)game_rva(ADDR_CREATENODIZZY);
        }
    }

    return ok;
}

/* ---- Background Thread ---- */
static DWORD WINAPI patch_thread(LPVOID lpParam)
{
    (void)lpParam;

    char ini_path[MAX_PATH];
    char log_path[MAX_PATH];

    GetModuleFileNameA(NULL, ini_path, MAX_PATH);
    char *p = strrchr(ini_path, '\\');
    if (p) strcpy(p + 1, "hamsterball_water.ini");
    else strcat(ini_path, "hamsterball_water.ini");

    GetModuleFileNameA(NULL, log_path, MAX_PATH);
    p = strrchr(log_path, '.');
    if (p) strcpy(p, "_water_mod.log");
    else strcat(log_path, "_water_mod.log");

    load_config(ini_path);

    if (g_cfg.debug) {
        g_log = fopen(log_path, "w");
    }

    InitializeCriticalSection(&g_water_cs);
    memset(g_balls, 0, sizeof(g_balls));
    memset(g_zones, 0, sizeof(g_zones));
    g_zone_count = 0;

    /* Wait for the game to finish initializing. */
    Sleep(2000);

    int hooked = apply_hooks();
    if (g_log) {
        fprintf(g_log, "[Water] Hook result: %s\n", hooked ? "OK" : "FAILED");
        fprintf(g_log, "[Water] Config: damping=%.2f drag=%.3f hdrag=%.3f g=%.3f accel=%.3f timer=%d\n",
                g_cfg.entry_damping, g_cfg.drag, g_cfg.horizontal_drag,
                g_cfg.gravity_equivalent, g_cfg.accel_damp, g_cfg.timer_frames);
        fflush(g_log);
    }

    return 0;
}

/* ---- BASS Proxy Exports (__stdcall) ---- */
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
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hinstDLL);

        g_hRealBass = LoadLibraryA("bass_real.dll");
        if (g_hRealBass == NULL) {
            char path[MAX_PATH];
            if (GetModuleFileNameA(hinstDLL, path, MAX_PATH)) {
                char *p = strrchr(path, '\\');
                if (p) { strcpy(p + 1, "bass_real.dll"); g_hRealBass = LoadLibraryA(path); }
            }
        }

        if (g_hRealBass != NULL) {
            real_BASS_Stop              = (BASS_Stop_t)GetProcAddress(g_hRealBass, "BASS_Stop");
            real_BASS_ChannelSetAttributes = (BASS_ChannelSetAttributes_t)GetProcAddress(g_hRealBass, "BASS_ChannelSetAttributes");
            real_BASS_Free              = (BASS_Free_t)GetProcAddress(g_hRealBass, "BASS_Free");
            real_BASS_Init              = (BASS_Init_t)GetProcAddress(g_hRealBass, "BASS_Init");
            real_BASS_Start             = (BASS_Start_t)GetProcAddress(g_hRealBass, "BASS_Start");
            real_BASS_SetConfig         = (BASS_SetConfig_t)GetProcAddress(g_hRealBass, "BASS_SetConfig");
            real_BASS_ChannelStop       = (BASS_ChannelStop_t)GetProcAddress(g_hRealBass, "BASS_ChannelStop");
            real_BASS_MusicPlayEx       = (BASS_MusicPlayEx_t)GetProcAddress(g_hRealBass, "BASS_MusicPlayEx");
            real_BASS_ErrorGetCode      = (BASS_ErrorGetCode_t)GetProcAddress(g_hRealBass, "BASS_ErrorGetCode");
            real_BASS_MusicLoad         = (BASS_MusicLoad_t)GetProcAddress(g_hRealBass, "BASS_MusicLoad");
        }

        CreateThread(NULL, 0, patch_thread, NULL, 0, NULL);
        break;

    case DLL_PROCESS_DETACH:
        if (g_log) { fclose(g_log); g_log = NULL; }
        DeleteCriticalSection(&g_water_cs);
        if (g_hRealBass) { FreeLibrary(g_hRealBass); g_hRealBass = NULL; }
        break;
    }
    return TRUE;
}
