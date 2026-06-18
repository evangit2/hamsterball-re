/*
 * hamsterball_water_mod.c — BASS.dll proxy that adds E:WATER physics.
 *
 * Build (Linux -> Windows): make
 *
 * Installation (Windows):
 *   1. In the Hamsterball game folder rename original bass.dll -> bass_real.dll
 *   2. Copy this proxy bass.dll + hamsterball_water.ini into the game folder
 *   3. Place invisible collision planes named "E:WATER" in custom levels
 *   4. Run Hamsterball.exe normally
 *
 * How it works:
 *   - Forwards all BASS audio calls to bass_real.dll.
 *   - Patches Ball vtable slot 4 (0x408390 wrapper) so Hook_Ball_Update runs
 *     before the real Ball_Update (0x405E00).
 *   - Patches CALL sites to CreateNoDizzy (0x40C5D0) so Hook_CreateNoDizzy can
 *     detect collisions with objects named "E:WATER" and record the surface Y.
 *   - Hook_Ball_Update modifies the CollisionMesh persistent velocities
 *     (Ball + 0x1A4 -> +0xCA4/CA8/CAC) and zeroes the engine gravity multiplier
 *     (Ball + 0x1A4 -> +0xC7C) while the ball is in the water, giving the
 *     requested buoyancy/drag behavior.
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
static BASS_ChannelSetAttributes_t   real_BASS_ChannelSetAttributes = NULL;
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

#define ADDR_BALL_VTABLE        0x004CF3A0      /* Ball type vtable */
#define VTABLE_SLOT_UPDATE      4               /* slot 4 == 0x408390 wrapper around Ball_Update @ 0x405E00 */
#define ADDR_CREATENODIZZY      0x0040C5D0
#define ADDR_CREATENODIZZY_END  0x0040C720

/* Offsets inside the Ball struct */
#define OFF_BALL_POS_X          0x164
#define OFF_BALL_POS_Y          0x168
#define OFF_BALL_POS_Z          0x16C
#define OFF_BALL_RADIUS         0x284
#define OFF_BALL_CMESH          0x1A4   /* CollisionMesh* (persistent physics object) */

/* Offsets inside the CollisionMesh struct */
#define OFF_CMESH_VEL_X         0xCA4
#define OFF_CMESH_VEL_Y         0xCA8
#define OFF_CMESH_VEL_Z         0xCAC
#define OFF_CMESH_GRAVITY_MULT  0xC7C   /* float gravity multiplier; 0.0 disables engine gravity */

/* Colliding object name inside CreateNoDizzy parameter */
#define OFF_COLLOBJ_NAME      0x864

/* ---- Config ---- */

typedef struct {
    float entry_damping;     /* vertical speed multiplier when first entering while falling */
    float drag;              /* overall water drag per frame */
    float horizontal_drag;   /* extra horizontal-only drag per frame */
    float gravity_equivalent;/* effective gravity per frame; buoyancy balances at half-submerged */
    int   timer_frames;      /* how many frames physics persists after leaving the plane */
    int   debug;             /* write water_mod.log */
} water_cfg_t;

static water_cfg_t g_cfg = { 0.70f, 0.03f, 0.04f, 0.45f, 10, 0 };

/* ---- Water state per ball ---- */

#define MAX_BALLS 32

typedef struct {
    void  *ball;              /* ball pointer used as key; NULL == unused */
    int    timer;             /* frames remaining; 0 = not in water */
    int    entry_damped;      /* already applied fall slowdown this entry? */
    float  surface_y;         /* world Y of the water surface (+Y is down) */
    float  orig_gravity_mult; /* engine gravity multiplier before water entry */
} water_state_t;

static water_state_t g_states[MAX_BALLS];
static FILE *g_log = NULL;

/* ---- Function prototypes ---- */

static void load_config(const char *ini_path);
static void apply_patches(const char *log_path);

/* ---- Helpers ---- */

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
    if (file_exists(ini_path)) {
        g_cfg.entry_damping    = read_ini_float(ini_path, "WaterPhysics", "EntryDamping", 0.70f);
        g_cfg.drag             = read_ini_float(ini_path, "WaterPhysics", "Drag", 0.03f);
        g_cfg.horizontal_drag  = read_ini_float(ini_path, "WaterPhysics", "HorizontalDrag", 0.04f);
        g_cfg.gravity_equivalent = read_ini_float(ini_path, "WaterPhysics", "GravityEquivalent", 0.45f);
        g_cfg.timer_frames     = read_ini_int(ini_path, "WaterPhysics", "TimerFrames", 10);
        g_cfg.debug            = read_ini_int(ini_path, "Debug", "Debug", 0);
    }

    if (g_cfg.entry_damping < 0.0f) g_cfg.entry_damping = 0.0f;
    if (g_cfg.entry_damping > 1.0f) g_cfg.entry_damping = 1.0f;
    if (g_cfg.drag < 0.0f) g_cfg.drag = 0.0f;
    if (g_cfg.drag > 1.0f) g_cfg.drag = 1.0f;
    if (g_cfg.horizontal_drag < 0.0f) g_cfg.horizontal_drag = 0.0f;
    if (g_cfg.horizontal_drag > 1.0f) g_cfg.horizontal_drag = 1.0f;
    if (g_cfg.timer_frames < 1) g_cfg.timer_frames = 1;
}

static void open_log(const char *path)
{
    if (g_cfg.debug && !g_log) {
        g_log = fopen(path, "w");
        if (g_log) {
            fprintf(g_log, "Hamsterball E:WATER mod log started\n");
            fflush(g_log);
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

static unsigned char *game_rva_to_ptr(DWORD rva)
{
    return (unsigned char *)rva;
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

/* ---- Patching: redirect all CALL sites to a function ---- */

static int patch_call_sites(DWORD target_func, DWORD target_func_end, void *hook_func, int *count_out)
{
    unsigned char *base = game_rva_to_ptr(IMAGE_BASE);
    SIZE_T scan_size = 0xCD000;           /* main .text size */
    int count = 0;

    for (SIZE_T i = 0; i + 5 <= scan_size; i++) {
        if (base[i] != 0xE8) continue;

        DWORD rel = *(DWORD *)(base + i + 1);
        DWORD dest = (DWORD)(base + i + 5) + rel;

        if (dest >= target_func && dest < target_func_end) {
            DWORD new_rel = (DWORD)hook_func - (DWORD)(base + i + 5);
            DWORD old_protect;

            if (set_memory_writable(base + i + 1, sizeof(DWORD), &old_protect)) {
                *(DWORD *)(base + i + 1) = new_rel;
                restore_memory_protect(base + i + 1, sizeof(DWORD), old_protect);
                FlushInstructionCache(GetCurrentProcess(), base + i + 1, sizeof(DWORD));
                count++;
            }
        }
    }

    if (count_out) *count_out = count;
    return count;
}

/* ---- Ball / CollisionMesh accessors ---- */

static void* get_collision_mesh(void *ball)
{
    if (!ball) return NULL;
    return *(void **)((char *)ball + OFF_BALL_CMESH);
}

static float get_ball_pos_y(void *ball)
{
    return *(float *)((char *)ball + OFF_BALL_POS_Y);
}

static float get_ball_radius(void *ball)
{
    return *(float *)((char *)ball + OFF_BALL_RADIUS);
}

static float get_cm_float(void *cmesh, DWORD offset)
{
    return *(float *)((char *)cmesh + offset);
}

static void set_cm_float(void *cmesh, DWORD offset, float val)
{
    *(float *)((char *)cmesh + offset) = val;
}

/* ---- Water state tracking ---- */

static water_state_t* get_ball_state(void *ball)
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

static void clear_ball_state(water_state_t *st)
{
    if (!st) return;
    void *ball = st->ball;
    void *cmesh = ball ? get_collision_mesh(ball) : NULL;
    if (cmesh) {
        /* Restore the engine gravity multiplier we saved on entry. */
        set_cm_float(cmesh, OFF_CMESH_GRAVITY_MULT, st->orig_gravity_mult);
    }
    if (g_log && ball) {
        log_msg("[%.3f] ball %p exited water\n", (float)GetTickCount() / 1000.0f, ball);
    }
    st->ball = NULL;
    st->timer = 0;
    st->entry_damped = 0;
    st->surface_y = 0.0f;
    st->orig_gravity_mult = 0.0f;
}

/* ---- Water physics ---- */

static void apply_water_physics(void *ball, water_state_t *st)
{
    void *cmesh = get_collision_mesh(ball);
    if (!cmesh) return;

    float pos_y = get_ball_pos_y(ball);
    float radius = get_ball_radius(ball);
    float bottom_y = pos_y + radius;        /* lowest point of ball (Y increases downward) */
    float top_y    = pos_y - radius;        /* highest point of ball */

    float submerged = 0.0f;
    if (bottom_y <= st->surface_y) {
        submerged = 0.0f;                   /* ball completely above water */
    } else if (top_y >= st->surface_y) {
        submerged = 1.0f;                   /* ball completely below water */
    } else {
        submerged = (bottom_y - st->surface_y) / (2.0f * radius);
        if (submerged < 0.0f) submerged = 0.0f;
        if (submerged > 1.0f) submerged = 1.0f;
    }

    /* If the ball is no longer overlapping the water volume, end the effect now. */
    if (submerged <= 0.0f) {
        clear_ball_state(st);
        return;
    }

    float vx = get_cm_float(cmesh, OFF_CMESH_VEL_X);
    float vy = get_cm_float(cmesh, OFF_CMESH_VEL_Y);
    float vz = get_cm_float(cmesh, OFF_CMESH_VEL_Z);

    log_msg("[%.3f] ball %p  vy=%.3f  subm=%.3f  surface_y=%.1f  pos_y=%.1f\n",
            (float)GetTickCount() / 1000.0f, ball, vy, submerged, st->surface_y, pos_y);

    /* 1. Entry damping: when first touching the water while falling, reduce downward speed. */
    if (!st->entry_damped && vy > 0.0f) {
        vy *= g_cfg.entry_damping;
        st->entry_damped = 1;
    }

    /* 2. Drag. Vertical drag is 'Drag'; horizontal axes get extra 'HorizontalDrag'. */
    float vscale = 1.0f - g_cfg.drag;
    float hscale = 1.0f - (g_cfg.drag + g_cfg.horizontal_drag);
    if (vscale < 0.0f) vscale = 0.0f;
    if (hscale < 0.0f) hscale = 0.0f;

    vx *= hscale;
    vz *= hscale;
    vy *= vscale;

    /* 3. Buoyancy. We disable engine gravity while in water and apply our own net
     * acceleration: gravity * (1 - 2*submerged).
     * At half-submerged the net vertical acceleration is zero, so a ball with zero
     * vertical velocity floats at the surface. Above half-submerged gravity dominates;
     * below half-submerged buoyancy pushes the ball back up. */
    set_cm_float(cmesh, OFF_CMESH_GRAVITY_MULT, 0.0f);
    vy += g_cfg.gravity_equivalent * (1.0f - 2.0f * submerged);

    /* 4. No artificial vertical speed cap is applied. The engine's own CollisionMesh
     *    speed limit remains unchanged. */

    set_cm_float(cmesh, OFF_CMESH_VEL_X, vx);
    set_cm_float(cmesh, OFF_CMESH_VEL_Y, vy);
    set_cm_float(cmesh, OFF_CMESH_VEL_Z, vz);
}

/* ---- Hooks ---- */

typedef void (__thiscall *ball_update_t)(void *ball);
typedef void (__thiscall *createnodizzy_t)(void *this_, void *ball, void *collObj);

static ball_update_t      orig_Ball_Update = NULL;
static createnodizzy_t    orig_CreateNoDizzy = NULL;

static void __thiscall Hook_Ball_Update(void *ball)
{
    if (ball) {
        water_state_t *st = get_ball_state(ball);
        if (st && st->timer > 0) {
            apply_water_physics(ball, st);
            st->timer--;
            if (st->timer <= 0) {
                clear_ball_state(st);
            }
        }
    }

    if (orig_Ball_Update) {
        orig_Ball_Update(ball);
    }
}

static const char* get_event_name(void *collObj)
{
    if (!collObj) return NULL;
    /* collObj layout: [0] == first collided object, [1] == second collided object.
     * The event-plane object lives at [1] and its descriptive name is at +0x864. */
    void **arr = (void **)collObj;
    void *obj = arr[1];
    if (!obj) return NULL;
    return *(const char **)((char *)obj + OFF_COLLOBJ_NAME);
}

static void __thiscall Hook_CreateNoDizzy(void *this_, void *ball, void *collObj)
{
    (void)this_;

    if (ball && collObj) {
        const char *name = get_event_name(collObj);
        if (name && (_stricmp(name, "E:WATER") == 0 || _strnicmp(name, "E:WATER", 7) == 0)) {
            water_state_t *st = get_ball_state(ball);
            if (st) {
                float pos_y = get_ball_pos_y(ball);
                float radius = get_ball_radius(ball);

                if (st->timer == 0) {
                    /* First contact this session: the bottom of the ball is at the surface. */
                    st->surface_y = pos_y + radius;
                    st->entry_damped = 0;
                    /* Save the engine's current gravity multiplier so we can restore it on exit. */
                    void *cmesh = get_collision_mesh(ball);
                    if (cmesh) st->orig_gravity_mult = get_cm_float(cmesh, OFF_CMESH_GRAVITY_MULT);
                    log_msg("[%.3f] ball %p entered E:WATER at surface_y=%.1f\n",
                            (float)GetTickCount() / 1000.0f, ball, st->surface_y);
                }
                st->timer = g_cfg.timer_frames;
            }
        }
    }

    if (orig_CreateNoDizzy) {
        orig_CreateNoDizzy(this_, ball, collObj);
    }
}

/* ---- Apply patches (run once from a background thread) ---- */

static void apply_patches(const char *log_path)
{
    open_log(log_path);
    log_msg("Applying E:WATER hook patches...\n");

    int n_call = 0;
    int slot_ok = patch_vtable_slot(VTABLE_SLOT_UPDATE, Hook_Ball_Update, (void **)&orig_Ball_Update);
    patch_call_sites(ADDR_CREATENODIZZY, ADDR_CREATENODIZZY_END, Hook_CreateNoDizzy, &n_call);

    log_msg("Ball_Update vtable[%d] hook (orig=0x%p): %s\n",
            VTABLE_SLOT_UPDATE, orig_Ball_Update, slot_ok ? "OK" : "FAILED");
    log_msg("CreateNoDizzy CALL-site redirects: %d patched\n", n_call);

    log_msg("Config: entry_damping=%.2f drag=%.3f hdrag=%.3f gravity_eq=%.3f timer=%d\n",
            g_cfg.entry_damping, g_cfg.drag, g_cfg.horizontal_drag,
            g_cfg.gravity_equivalent, g_cfg.timer_frames);
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
    p = strrchr(log_path, '.');
    if (p) strcpy(p, "_water_mod.log");
    else   strcat(log_path, "_water_mod.log");

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
