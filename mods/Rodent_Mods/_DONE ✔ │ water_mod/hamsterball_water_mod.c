/*
 * hamsterball_water_mod.c — v6: Clear 0x2E9 on water entry
 *
 * BUILD (Linux -> Windows):
 *   i686-w64-mingw32-gcc -shared -o bass.dll hamsterball_water_mod.c -lwinmm \
 *     -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias -msse2 -mfpmath=sse
 *
 * v7.6: E:WATERFLOW now uses numbers 1-8 instead of N/S/E/W, enabling
 *     diagonals. Format: E:WATERFLOW(N). 1-8 = N,NE,E,SE,S,SW,W,NW
 *     clockwise from North. Diagonal directions split the force across
 *     both axes by 1/sqrt(2) so push magnitude is equal in all 8.
 *
 * v7.5: Reworked running water. E:WATERFLOW-<dir> is now handled as a
 *     subset of E:WATER: it does everything plain E:WATER does (flag,
 *     damp, capture Y), then sets a per-ball flow direction flag
 *     (0=none,1=N,2=S,3=E,4=W). Each frame while in water it adds a
 *     constant force (flow_force, default 0.18) to the ball's force
 *     accumulators (ball+0x170/174/178) in that direction — the same
 *     accumulator mechanism as jump/magnet, giving smooth, drag-capped
 *     flow. Plain E:WATER keeps flow_dir=0 (calm, no force).
 *
 * v7.4: Added E:WATERFLOW-<dir> running-water current. Touching a plane
 *     named E:WATERFLOW-N/S/E/W carries the ball in that compass direction
 *     (N=-Z, S=+Z, E=+X, W=-X) at CurrentStrength speed. Capped-river
 *     model: horizontal velocity eases toward the flow each frame, so the
 *     ball is carried along and can be fought by inputting against it.
 *     Touching plain E:WATER (calm) clears the current; E:WATEREXIT turns
 *     water off entirely. Checked before the E:WATER prefix match.
 *
 * v7.3: Added E:WATEREXIT event plane. Touching an object named
 *     "E:WATEREXIT" turns OFF the water flag entirely and immediately
 *     (in_water = 0, surface/prev_submersion reset, NO grace period —
 *     the exit is final). Repeated triggers are idempotent; re-entering
 *     water via E:WATER right after re-activates the flag normally.
 *     Checked BEFORE the E:WATER prefix match so it isn't swallowed by it.
 *
 * v7.2: Surgical type-5 block — block ONLY the 0x2E9 death-flag write
 *     while submerged / in the grace period, instead of skipping the
 *     whole type-5 death block. Hook 3's in-water path now JMPs to
 *     0x407398 (right after the "MOV byte [ESI+0x2E9],1" at 0x407391)
 *     instead of 0x40743D, so the camera snap (Scene_SetCamera),
 *     viewport, and 0x2E8 split-flag logic still run normally. The
 *     death flag is the ONLY thing suppressed — the ball can't be
 *     killed by death checks #1/#2 while in water / grace, but falls
 *     still behave naturally (camera follows, split logic intact).
 *     E:LIMIT deaths still work (separate writer via
 *     DispatchCollisionEvents).
 *
 * v7.1: Clear 0x2E9 on water entry + dizzy immunity while submerged.
 *     On water entry: clear bounce counter (ball+0x2EC) — same as E:NODIZZY.
 *     Every frame in water: clear bounce counter + set dizzy_immunity_timer
 *     (ball+0x2F4) to GRACE_PERIOD_FRAMES. Mirrors Ball_DizzyImmunity(0x402400):
 *     only increases the timer, never shortens existing immunity.
 *     v7.1 normalizes submersion to 0-1 (was 0-2).
 *
 * v6: Clear ball+0x2E9 (falling flag) when ball enters water AND every frame
 *     while in water or during grace period. Extend Hook 3 to suppress type 5
 *     during grace period too (not just while submerged).
 *     Root cause: type 5 collision re-sets 0x2E9 after ball exits water
 *     (ball clips through mesh on the way up). Hook 3 only checked in_water,
 *     so after exit it fell through and 0x2E9 got set. Death check #2 then
 *     fired at the apex (0x2E9==1 + position delta < 2.0 → vtable[8]).
 *
 * v5: Fix FPU state corruption crash at 0x407BC6.
 *     Phase 15 code cave now saves/restores full x87 FPU state via
 *     FNSAVE/FRSTOR (108 bytes). Compiled with -mssse2 -mfpmath=sse
 *     so the C function uses SSE instead of x87, avoiding stack overflow.
 *     Without this, apply_water_physics() corrupts the FPU register stack
 *     that Ball_Update depends on for collision/render math.
 *
 * INSTALLATION (Windows):
 *   1. In the Hamsterball game folder rename original bass.dll -> bass_real.dll
 *   2. Copy this proxy bass.dll + hamsterball_water.ini into the game folder
 *   3. Place collision objects named "E:WATER" in custom levels
 *      (optional: "E:WATEREXIT" planes turn the water flag OFF when touched)
 *   4. Run Hamsterball.exe normally
 *
 * HOW IT WORKS (v7.3):
 *   Four hooks:
 *
 *   HOOK 1 — DispatchCollisionEvents (0x40C5D0) trampoline:
 *     Intercepts all collision events. When the collision object's name starts
 *     with "E:WATER", fires the 3-step trigger:
 *       1. Sets in_water flag (gates ongoing water physics)
 *       2. Reduces ALL velocity axes by entry_damping (default 10% cut)
 *       3. Captures ball's Y as water_surface_y for this session
 *     When the name is "E:WATEREXIT", instead turns OFF the water flag
 *     entirely (in_water = 0, surface/prev_submersion reset, NO grace
 *     period — the exit is final). Repeated triggers are idempotent;
 *     re-entering water via E:WATER re-activates the flag normally.
 *     E:WATERFLOW(N) is handled as a SUBSET of E:WATER — it does all
 *     the normal E:WATER entry, then sets a flow direction flag parsed
 *     from the name (1=N,2=NE,3=E,4=SE,5=S,6=SW,7=W,8=NW). E:WATEREXIT is
 *     checked first so it isn't swallowed by the E:WATER prefix match.
 *     Then calls the original DispatchCollisionEvents so the game processes
 *     the collision normally. E:LIMIT events still set 0x2E9 normally.
 *
 *   HOOK 2 — Phase 15 code cave (0x407BB4) in Ball_Update:
 *     Runs every frame for every ball. If in_water is set, applies:
 *       - Drag (all velocity axes scaled per frame)
 *       - Horizontal drag (extra scaling on X/Z)
 *       - Buoyancy (upward acceleration proportional to submersion)
 *     If ball rises above captured surface, clears in_water and starts
 *     a grace period (GRACE_PERIOD_FRAMES frames of death suppression).
 *
 *   HOOK 3 — Type 5 collision suppressor (0x407377) in Ball_Update:
 *     Code cave at the JNZ that gates the type 5 death block. When the
 *     ball is in water, skips the entire block (0x2E9 set + camera switch).
 *     This prevents geometric mesh-penetration from setting the falling
 *     flag while submerged. E:LIMIT events still set 0x2E9 through
 *     DispatchCollisionEvents, so the ball can still die from level
 *     boundaries while in water.
 *
 *   HOOK 4 — Ball vtable[8] (0x4CF3C0) — Ball_FallDeath suppression:
 *     When Ball_FallDeath is called, checks if ball is in water OR within
 *     the grace period after leaving water. If so, skips death entirely.
 *     This covers the bounce-out scenario: ball exits water, flies through
 *     the air, and the velocity-stop death check fires at the apex — the
 *     grace period keeps the suppression active until the ball lands.
 *
 *   No background scan thread. No MeshWorld parsing. No AthenaList iteration.
 *   The game's own collision system tells us when the ball touches water.
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll hamsterball_water_mod.c -lwinmm \
 *     -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winuser.h>
#include <mmsystem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * BASS Proxy Exports
 * ═══════════════════════════════════════════════════════════════════════════ */

static HMODULE g_hRealBass = NULL;

typedef int  (__stdcall *BASS_ChannelSetAttributes_t)(DWORD, float, int, int);
static BASS_ChannelSetAttributes_t real_BASS_ChannelSetAttributes = NULL;
__declspec(dllexport) int __stdcall BASS_ChannelSetAttributes(DWORD a, float b, int c, int d) {
    if (real_BASS_ChannelSetAttributes) return real_BASS_ChannelSetAttributes(a, b, c, d);
    return 1;
}
typedef int  (__stdcall *BASS_MusicPlayEx_t)(DWORD, DWORD, BOOL);
static BASS_MusicPlayEx_t real_BASS_MusicPlayEx = NULL;
__declspec(dllexport) int __stdcall BASS_MusicPlayEx(DWORD a, DWORD b, BOOL c) {
    if (real_BASS_MusicPlayEx) return real_BASS_MusicPlayEx(a, b, c);
    return 1;
}
typedef int  (__stdcall *BASS_SetConfig_t)(DWORD, DWORD);
static BASS_SetConfig_t real_BASS_SetConfig = NULL;
__declspec(dllexport) int __stdcall BASS_SetConfig(DWORD a, DWORD b) {
    if (real_BASS_SetConfig) return real_BASS_SetConfig(a, b);
    return 1;
}
typedef int  (__stdcall *BASS_Init_t)(int, DWORD, DWORD, DWORD, void*);
static BASS_Init_t real_BASS_Init = NULL;
__declspec(dllexport) int __stdcall BASS_Init(int a, DWORD b, DWORD c, DWORD d, void* e) {
    if (real_BASS_Init) return real_BASS_Init(a, b, c, d, e);
    return 1;
}
typedef int  (__stdcall *BASS_Free_t)(void);
static BASS_Free_t real_BASS_Free = NULL;
__declspec(dllexport) int __stdcall BASS_Free(void) {
    if (real_BASS_Free) return real_BASS_Free();
    return 1;
}
typedef int  (__stdcall *BASS_Start_t)(void);
static BASS_Start_t real_BASS_Start = NULL;
__declspec(dllexport) int __stdcall BASS_Start(void) {
    if (real_BASS_Start) return real_BASS_Start();
    return 1;
}
typedef int  (__stdcall *BASS_Stop_t)(void);
static BASS_Stop_t real_BASS_Stop = NULL;
__declspec(dllexport) int __stdcall BASS_Stop(void) {
    if (real_BASS_Stop) return real_BASS_Stop();
    return 1;
}
typedef int  (__stdcall *BASS_ErrorGetCode_t)(void);
static BASS_ErrorGetCode_t real_BASS_ErrorGetCode = NULL;
__declspec(dllexport) int __stdcall BASS_ErrorGetCode(void) {
    if (real_BASS_ErrorGetCode) return real_BASS_ErrorGetCode();
    return 0;
}
typedef DWORD (__stdcall *BASS_MusicLoad_t)(int, void*, DWORD, DWORD, DWORD, DWORD);
static BASS_MusicLoad_t real_BASS_MusicLoad = NULL;
__declspec(dllexport) DWORD __stdcall BASS_MusicLoad(int a, void* b, DWORD c, DWORD d, DWORD e, DWORD f) {
    if (real_BASS_MusicLoad) return real_BASS_MusicLoad(a, b, c, d, e, f);
    return 0;
}
typedef int  (__stdcall *BASS_ChannelStop_t)(DWORD);
static BASS_ChannelStop_t real_BASS_ChannelStop = NULL;
__declspec(dllexport) int __stdcall BASS_ChannelStop(DWORD a) {
    if (real_BASS_ChannelStop) return real_BASS_ChannelStop(a);
    return 1;
}

/* Extra stubs */
__declspec(dllexport) void __stdcall BASS_Pause(void) {}
__declspec(dllexport) void __stdcall BASS_SetVolume(DWORD a) {}
__declspec(dllexport) DWORD __stdcall BASS_GetVolume(void) { return 0; }
__declspec(dllexport) int __stdcall BASS_GetDevice(void) { return 0; }
__declspec(dllexport) int __stdcall BASS_SetDevice(DWORD a) { return 1; }
__declspec(dllexport) void __stdcall BASS_GetInfo(void *a) {}
__declspec(dllexport) int __stdcall BASS_Update(DWORD a) { return 0; }
__declspec(dllexport) DWORD __stdcall BASS_StreamCreateFile(void *a, void* b, DWORD c, DWORD d, DWORD e) { return 0; }
__declspec(dllexport) DWORD __stdcall BASS_SampleLoad(int a, void* b, DWORD c, DWORD d, DWORD e) { return 0; }
__declspec(dllexport) int __stdcall BASS_ChannelPlay(DWORD a, BOOL b) { return 1; }
__declspec(dllexport) int __stdcall BASS_ChannelSetAttribute(DWORD a, DWORD b, float c) { return 1; }
__declspec(dllexport) int __stdcall BASS_ChannelGetAttribute(DWORD a, DWORD b, float *c) { return 1; }
__declspec(dllexport) DWORD __stdcall BASS_ChannelGetData(DWORD a, void *b, DWORD c) { return 0; }
__declspec(dllexport) DWORD __stdcall BASS_ChannelGetLevel(DWORD a) { return 0; }
__declspec(dllexport) int __stdcall BASS_ChannelSetPosition(DWORD a, void *b, DWORD c) { return 1; }
__declspec(dllexport) DWORD __stdcall BASS_ChannelGetPosition(DWORD a, DWORD b) { return 0; }
__declspec(dllexport) int __stdcall BASS_ChannelIsActive(DWORD a) { return 0; }
__declspec(dllexport) int __stdcall BASS_ChannelRemoveSync(DWORD a, DWORD b) { return 1; }
__declspec(dllexport) DWORD __stdcall BASS_ChannelSetSync(DWORD a, DWORD b, DWORD c, void *d, void *e) { return 0; }
__declspec(dllexport) DWORD __stdcall BASS_SampleCreate(DWORD a, DWORD b, DWORD c, DWORD d, DWORD e) { return 0; }
__declspec(dllexport) DWORD __stdcall BASS_SampleGetChannel(DWORD a, BOOL b) { return 0; }

static void load_real_bass(void)
{
    g_hRealBass = LoadLibraryA("bass_real.dll");
    if (g_hRealBass == NULL) {
        char path[MAX_PATH];
        HMODULE hSelf = NULL;
        GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                          | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                          (LPCSTR)&load_real_bass, &hSelf);
        if (hSelf && GetModuleFileNameA(hSelf, path, MAX_PATH)) {
            char *p = strrchr(path, '\\');
            if (p) { strcpy(p + 1, "bass_real.dll"); g_hRealBass = LoadLibraryA(path); }
        }
    }
    if (g_hRealBass) {
        real_BASS_ChannelSetAttributes = (BASS_ChannelSetAttributes_t)GetProcAddress(g_hRealBass, "BASS_ChannelSetAttributes");
        real_BASS_MusicPlayEx          = (BASS_MusicPlayEx_t)GetProcAddress(g_hRealBass, "BASS_MusicPlayEx");
        real_BASS_SetConfig             = (BASS_SetConfig_t)GetProcAddress(g_hRealBass, "BASS_SetConfig");
        real_BASS_Init                  = (BASS_Init_t)GetProcAddress(g_hRealBass, "BASS_Init");
        real_BASS_Free                  = (BASS_Free_t)GetProcAddress(g_hRealBass, "BASS_Free");
        real_BASS_Start                 = (BASS_Start_t)GetProcAddress(g_hRealBass, "BASS_Start");
        real_BASS_Stop                  = (BASS_Stop_t)GetProcAddress(g_hRealBass, "BASS_Stop");
        real_BASS_ErrorGetCode          = (BASS_ErrorGetCode_t)GetProcAddress(g_hRealBass, "BASS_ErrorGetCode");
        real_BASS_MusicLoad             = (BASS_MusicLoad_t)GetProcAddress(g_hRealBass, "BASS_MusicLoad");
        real_BASS_ChannelStop            = (BASS_ChannelStop_t)GetProcAddress(g_hRealBass, "BASS_ChannelStop");
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Diagnostic logging
 * ═══════════════════════════════════════════════════════════════════════════ */

static char g_logpath[MAX_PATH] = "";

static void diag_log(const char *msg)
{
    if (g_logpath[0] == '\0') return;
    HANDLE hFile = CreateFileA(g_logpath, FILE_APPEND_DATA,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD written;
        SetFilePointer(hFile, 0, NULL, FILE_END);
        WriteFile(hFile, msg, (DWORD)strlen(msg), &written, NULL);
        WriteFile(hFile, "\r\n", 2, &written, NULL);
        CloseHandle(hFile);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Memory layout constants
 * ═══════════════════════════════════════════════════════════════════════════ */

#define IMAGE_BASE              0x00400000

/* Hook 1: DispatchCollisionEvents — trampoline hook
 * First 8 bytes: PUSH -1 (2) + MOV EAX,FS:[0] (6) = two complete instructions.
 * Must copy 8 bytes to trampoline (not 5) to avoid splitting the FS:[0] read. */
#define ADDR_DISPATCH_COLLISIONS  0x0040C5D0
#define TRAMP_SIZE               16
#define DISPATCH_PATCH_SIZE      8       /* 5-byte JMP + 3 NOPs */

/* Hook 2: Phase 15 in Ball_Update — code cave */
#define PHASE15_HOOK            0x00407BB4
#define PHASE15_ORIG_BYTES      6

/* Hook 3: Type 5 collision suppressor — code cave at 0x407377 in Ball_Update
 * When ball is in water, skip the entire type 5 death block (sets 0x2E9 +
 * camera switch). This prevents geometric mesh-penetration detection from
 * setting the falling flag while the ball is submerged, while still allowing
 * E:LIMIT events to set it through DispatchCollisionEvents. */
#define ADDR_TYPE5_JNZ          0x00407377  /* 6-byte JNZ at penetration check */
#define TYPE5_SKIP_TARGET       0x0040743D  /* jump here to skip type 5 death block */
#define TYPE5_AFTER_FLAG        0x00407398  /* jump here to skip ONLY the 0x2E9 flag write
                                             * (MOV byte [ESI+0x2E9],1 = 7 bytes at 0x407391),
                                             * keeping the camera snap + viewport + split flag.
                                             * Next instr = CALL Scene_SetCamera at 0x407398. */
#define TYPE5_NEXT_INSTR        0x0040737D  /* instruction after the JNZ */
#define TYPE5_PATCH_SIZE        6           /* 5-byte JMP + 1 NOP */

/* Hook 4: Ball vtable[8] — Ball_FallDeath (fall death) — grace period */
#define ADDR_BALL_VTABLE        0x004CF3A0
#define VTABLE_SLOT_ONRAMP      8           /* slot 8 → 0x409480 = fall death/shatter */
#define GRACE_PERIOD_FRAMES     120         /* ~5 seconds at 25fps to suppress death after leaving water */

/* Ball struct offsets */
#define BALL_POS_X              0x164
#define BALL_POS_Y              0x168
#define BALL_POS_Z              0x16C
#define BALL_PHYS_PTR           0x1A4   /* Physics struct pointer */
#define BALL_RADIUS             0x284
#define BALL_FALLING_FLAG        0x2E9   /* BYTE: set by type 5 collision, E:LIMIT, etc.
                                          * Cleared only by Ball ctor + Ball_Respawn.
                                          * Must clear here too, or death check #2
                                          * at 0x40721F fires at apex of bounce-out. */
#define BALL_BOUNCE_COUNT       0x2EC   /* INT: dizzy bounce counter (0→1→2, >1 && !falling → dizzy) */
#define BALL_DIZZY_IMMUNITY     0x2F4   /* INT: dizzy immunity timer in frames (while > 0, blocks 0→1 bounce) */

/* Physics struct offsets */
#define PHYS_VEL_X              0xCA4
#define PHYS_VEL_Y              0xCA8
#define PHYS_VEL_Z              0xCAC

/* Ball force accumulators (physics inputs, consumed by the physics engine
 * with proper collision response — do NOT write position directly). */
#define BALL_FORCE_X            0x170
#define BALL_FORCE_Y            0x174
#define BALL_FORCE_Z            0x178

/* E:WATERFLOW direction suffix (e.g. "E:WATERFLOW(3)") */
#define WATERFLOW_PREFIX        "E:WATERFLOW"
#define WATERFLOW_PREFIX_LEN    13

/* Flow direction (stored per-ball in water_state_t.flow_dir). 8-way
 * compass, clockwise from North; 0 = no current (calm water).
 *   1=N(-Z) 2=NE(+X,-Z) 3=E(+X) 4=SE(+X,+Z)
 *   5=S(+Z) 6=SW(-X,+Z) 7=W(-X) 8=NW(-X,-Z)
 * Per-frame current force is applied to the ball force accumulators;
 * diagonals (2,4,6,8) split the force across both axes by 1/sqrt(2)
 * so the push magnitude is identical in every direction. */
#define FLOW_NONE   0
#define FLOW_FORCE_NORMALIZE 0.7071067811865476f /* 1/sqrt(2), for diagonals */
#define DEFAULT_FLOW_FORCE 0.18f

/* Collision entry struct: pair of pointers
 *   [0] = type/board ptr
 *   [1] = MeshBuffer pointer (name at +0x864) */
#define COLLOBJ_MESHBUFFER      1       /* index into the pair */
#define MESHBUFFER_NAME_OFFSET  0x864

/* ═══════════════════════════════════════════════════════════════════════════
 * Config
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef struct {
    float entry_damping;       /* velocity multiplier on first contact (0-1) */
    float drag;                /* per-frame velocity drag on all axes (0-1) */
    float horizontal_drag;     /* extra drag on X/Z axes (0-1) */
    float buoyancy_strength;   /* upward acceleration at full submersion */
    float flow_force;          /* per-frame current force applied to ball while in running water (flow_dir != 0) */
    int   debug;               /* write log file */
} water_cfg_t;

static water_cfg_t g_cfg = {
    0.90f,   /* entry_damping (10% velocity reduction on contact) */
    0.02f,   /* drag (2% per frame) */
    0.04f,   /* horizontal_drag */
    1.0f,    /* buoyancy_strength (max upward accel at full submersion, 0-1 normalized) */
    DEFAULT_FLOW_FORCE, /* flow_force (per-frame current force in running water) */
    1        /* debug */
};

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
    DWORD attr = GetFileAttributesA(ini_path);
    if (attr == INVALID_FILE_ATTRIBUTES || (attr & FILE_ATTRIBUTE_DIRECTORY)) return;

    g_cfg.entry_damping     = read_ini_float(ini_path, "WaterPhysics", "EntryDamping", 0.90f);
    g_cfg.drag              = read_ini_float(ini_path, "WaterPhysics", "Drag", 0.02f);
    g_cfg.horizontal_drag   = read_ini_float(ini_path, "WaterPhysics", "HorizontalDrag", 0.04f);
    g_cfg.buoyancy_strength = read_ini_float(ini_path, "WaterPhysics", "BuoyancyStrength", 1.0f);
    g_cfg.flow_force = read_ini_float(ini_path, "WaterPhysics", "CurrentStrength", DEFAULT_FLOW_FORCE);
    g_cfg.debug             = read_ini_int(ini_path, "WaterPhysics", "Debug", 1);

    if (g_cfg.entry_damping < 0.0f) g_cfg.entry_damping = 0.0f;
    if (g_cfg.entry_damping > 1.0f) g_cfg.entry_damping = 1.0f;
    if (g_cfg.drag < 0.0f) g_cfg.drag = 0.0f;
    if (g_cfg.drag > 1.0f) g_cfg.drag = 1.0f;
    if (g_cfg.horizontal_drag < 0.0f) g_cfg.horizontal_drag = 0.0f;
    if (g_cfg.horizontal_drag > 1.0f) g_cfg.horizontal_drag = 1.0f;
    if (g_cfg.flow_force < 0.0f) g_cfg.flow_force = 0.0f;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Per-ball water state
 * ═══════════════════════════════════════════════════════════════════════════ */

#define MAX_BALLS 32

typedef struct {
    DWORD ball;              /* ball pointer (key); 0 = unused */
    int   in_water;          /* currently in water? (gates physics) */
    float water_surface_y;   /* ball's Y at moment of contact */
    int   grace_frames;      /* frames remaining of death suppression after leaving water */
    float prev_submersion;   /* last frame's submersion (for surface crossing detection) */
    int   flow_dir;          /* running-water direction: 0=none, 1-8 = N,NE,E,SE,S,SW,W,NW */
} water_state_t;

static water_state_t g_states[MAX_BALLS];

static water_state_t *get_ball_state(DWORD ball)
{
    int free_idx = -1;
    for (int i = 0; i < MAX_BALLS; i++) {
        if (g_states[i].ball == ball) return &g_states[i];
        if (free_idx == -1 && g_states[i].ball == 0) free_idx = i;
    }
    if (free_idx >= 0) {
        memset(&g_states[free_idx], 0, sizeof(water_state_t));
        g_states[free_idx].ball = ball;
        return &g_states[free_idx];
    }
    return NULL;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * HOOK 1: DispatchCollisionEvents trampoline
 *
 * __thiscall: ECX = this (board), stack params: ball, collObj, RET 0x8
 * MinGW workaround: __fastcall with dummy EDX.
 *   __fastcall(this, dummy_edx, ball, collObj) == __thiscall(this, ball, collObj)
 *
 * collObj is a pair of pointers: collObj[0]=type, collObj[1]=MeshBuffer
 * MeshBuffer+0x864 = name string (e.g. "E:WATER")
 *
 * When name starts with "E:WATER", fire the 3-step trigger:
 *   1. Set in_water = 1
 *   2. Reduce all velocity by entry_damping
 *   3. Capture ball Y as water_surface_y
 * ═══════════════════════════════════════════════════════════════════════════ */

/* __thiscall workaround: __fastcall with dummy EDX arg */
typedef void (__fastcall *dispatch_t)(void *this_, void *edx_dummy,
                                        void *ball, void *collObj);

static dispatch_t g_orig_Dispatch = NULL;
static unsigned char g_tramp[TRAMP_SIZE];
static volatile DWORD g_trigger_count = 0;

/* Get the event name from a collision object.
 * collObj is int** — collObj[1] points to MeshBuffer, name at +0x864 */
static const char *get_collision_name(void *collObj)
{
    if (!collObj || IsBadReadPtr(collObj, 8)) return NULL;
    int *pair = (int *)collObj;
    int meshbuf = pair[COLLOBJ_MESHBUFFER];
    if (!meshbuf || IsBadReadPtr((void*)meshbuf, 0x868)) return NULL;
    int nameptr = *(int*)(meshbuf + MESHBUFFER_NAME_OFFSET);
    if (!nameptr || IsBadReadPtr((void*)nameptr, 1)) return NULL;
    return (const char*)nameptr;
}

/* Forward declaration (definition follows after trigger_water_exit). */
static int parse_flow_direction(const char *name);

/* Trigger function — called from the hooked DispatchCollisionEvents.
 * Does the E:WATER entry trigger: set flag, damp velocity, capture Y.
 * If the plane is E:WATERFLOW(N), additionally sets the running-water
 * direction flag (flow_dir 1-8). Plain E:WATER has flow_dir = 0 (calm). */
static void trigger_water_contact(void *ball_ptr, const char *name)
{
    DWORD ball = (DWORD)ball_ptr;
    if (!ball || IsBadReadPtr((void*)ball, 0x300)) return;

    float ball_y = *(float*)(ball + BALL_POS_Y);
    float radius = *(float*)(ball + BALL_RADIUS);
    if (radius <= 0.0f || radius > 1000.0f) return;

    DWORD phys = *(DWORD*)(ball + BALL_PHYS_PTR);
    if (!phys || IsBadReadPtr((void*)phys, 0xCB0)) return;

    float *vel_x = (float*)(phys + PHYS_VEL_X);
    float *vel_y = (float*)(phys + PHYS_VEL_Y);
    float *vel_z = (float*)(phys + PHYS_VEL_Z);

    water_state_t *st = get_ball_state(ball);
    if (!st) return;

    /* Only trigger if not already in water */
    if (st->in_water) {
        if (g_cfg.debug) {
            char buf[256];
            snprintf(buf, sizeof(buf), "WATER SKIP: ball=%08X already in_water=1 (vel=%.2f,%.2f,%.2f)",
                     ball, *vel_x, *vel_y, *vel_z);
            diag_log(buf);
        }
        return;
    }

    /* Step 1: Set in_water flag */
    st->in_water = 1;
    /* Determine flow direction from the plane name. E:WATERFLOW-<dir>
     * sets flow_dir 1-4 (running water); plain E:WATER is calm (0). */
    st->flow_dir = parse_flow_direction(name);
    if (st->flow_dir != FLOW_NONE && g_cfg.debug) {
        char buf[128];
        snprintf(buf, sizeof(buf), "WATERFLOW: ball=%08X dir=%d (running water)",
                 ball, st->flow_dir);
        diag_log(buf);
    }

    /* Step 1.5: Clear the falling flag (ball+0x2E9).
     * During a long fall, type 5 mesh-penetration sets 0x2E9=1 before
     * the ball reaches water. Hook 3 prevents NEW type 5 sets while
     * submerged, but the flag set during the fall is never cleared.
     * Death check #2 at 0x40721F fires when 0x2E9 is still set AND
     * the ball's position delta drops below 2.0 — exactly what happens
     * at the apex of the bounce-out. The grace period only delayed
     * this; clearing the flag eliminates the root cause. */
    *(BYTE*)((DWORD)ball_ptr + BALL_FALLING_FLAG) = 0;

    /* Step 1.6: Reset bounce counter (ball+0x2EC) on water entry.
     * Same as E:NODIZZY calling Ball_DizzyImmunity — clears the 2-strike
     * dizzy counter so the ball starts fresh when it hits water. */
    *(int*)((DWORD)ball_ptr + BALL_BOUNCE_COUNT) = 0;

    /* Step 2: Reduce ALL velocity by entry_damping */
    float damp = g_cfg.entry_damping;
    *vel_x *= damp;
    *vel_y *= damp;
    *vel_z *= damp;

    /* Step 3: Capture ball Y as water surface height */
    st->water_surface_y = ball_y;

    g_trigger_count++;

    if (g_cfg.debug) {
        char buf[256];
        snprintf(buf, sizeof(buf), "WATER TRIGGER #%u: ball=%08X y=%.2f 0x2E9 cleared vel=(%.2f,%.2f,%.2f)->(%.2f,%.2f,%.2f)",
                 g_trigger_count, ball, ball_y,
                 *vel_x / damp, *vel_y / damp, *vel_z / damp,
                 *vel_x, *vel_y, *vel_z);
        diag_log(buf);
    }
}

/* Trigger function — called when the ball touches an E:WATEREXIT plane.
 * Turns OFF the water flag entirely and immediately:
 *   - in_water = 0 (stops all water physics instantly)
 *   - water_surface_y / prev_submersion reset
 *   - grace_frames = 0 (NO grace period — the exit is final; the user
 *     explicitly wants it to just switch off and be done. Repeated
 *     triggers are harmless (idempotent), and re-entering water via
 *     E:WATER right after re-activates the flag normally.) */
static void trigger_water_exit(void *ball_ptr)
{
    DWORD ball = (DWORD)ball_ptr;
    if (!ball || IsBadReadPtr((void*)ball, 0x300)) return;

    water_state_t *st = get_ball_state(ball);
    if (!st) return;

    /* Idempotent: if already out of water, nothing to do.
     * Repeated E:WATEREXIT triggers are fine — they no-op here. */
    if (!st->in_water) return;

    st->in_water = 0;
    st->water_surface_y = 0.0f;
    st->prev_submersion = 0.0f;
    st->grace_frames = 0;

    if (g_cfg.debug) {
        char buf[128];
        snprintf(buf, sizeof(buf), "WATEREXIT TRIGGER: ball=%08X in_water cleared (no grace)",
                 ball);
        diag_log(buf);
    }
}

/* Parse the direction number from an E:WATERFLOW(N) name.
 * Returns the flow direction flag (1-8), or FLOW_NONE (0)
 * if there is no flow / the direction is unknown. A plain E:WATER name
 * (no "FLOW" suffix) naturally returns FLOW_NONE = calm water.
 *   format: E:WATERFLOW(N)  where N is 1-8
 *   1=N(-Z) 2=NE(+X,-Z) 3=E(+X) 4=SE(+X,+Z)
 *   5=S(+Z) 6=SW(-X,+Z) 7=W(-X) 8=NW(-X,-Z) */
static int parse_flow_direction(const char *name)
{
    /* Plain E:WATER has no "FLOW" suffix -> calm. */
    if (!name || !strstr(name, "FLOW")) return FLOW_NONE;

    /* The direction digit is right after the "E:WATERFLOW" prefix, in the
     * form E:WATERFLOW(N) or E:WATERFLOWN. Start at the known offset. */
    const char *d = name + WATERFLOW_PREFIX_LEN;
    if (*d == '(' || *d == '-' || *d == ':') d++;

    if (*d >= '1' && *d <= '8') {
        int n = *d - '0';
        /* validate: must be a 1-digit token ("E:WATERFLOW(3)" or
         * "E:WATERFLOW3"), so multi-digit garbage doesn't read as 1 */
        char after = d[1];
        if (after == ')' || after == '\0')
            return n;
    }
    return FLOW_NONE;
}

/* Trigger function — called when the ball touches an E:WATEREXIT plane.
 * Turns OFF the water flag entirely and immediately.
 *   - in_water = 0 (stops all water physics instantly)
 *   - water_surface_y / prev_submersion reset
 *   - grace_frames = 0 (NO grace period — the exit is final) */
void __fastcall hook_DispatchCollisionEvents(void *this_, void *edx_dummy,
                                              void *ball, void *collObj)
{
    (void)edx_dummy;

    /* Check E:WATER / E:WATEREXIT collisions.
     * E:WATEREXIT is checked first (it starts with "E:WATER"). E:WATERFLOW
     * is deliberately handled as plain E:WATER — it should do all the normal
     * E:WATER logic, and the flow direction is parsed from the name inside
     * trigger_water_contact. So only E:WATEREXIT needs its own branch. */
    const char *name = get_collision_name(collObj);
    if (name && _strnicmp(name, "E:WATEREXIT", 11) == 0) {
        trigger_water_exit(ball);
    } else if (name && _strnicmp(name, "E:WATER", 7) == 0) {
        trigger_water_contact(ball, name);
    }

    /* Call original */
    if (g_orig_Dispatch)
        g_orig_Dispatch(this_, NULL, ball, collObj);
}

/* Install trampoline hook on DispatchCollisionEvents.
 * First 8 bytes are: PUSH -1 (2B) + MOV EAX,FS:[0] (6B) — two complete instructions.
 * We copy all 8 to the trampoline, then overwrite the target with 5-byte JMP + 3 NOPs. */
static int install_dispatch_hook(void)
{
    unsigned char *target = (unsigned char *)ADDR_DISPATCH_COLLISIONS;
    DWORD oldProtect;

    /* Verify expected bytes: 6A FF 64 A1 00 00 00 00 */
    unsigned char expected[] = { 0x6A, 0xFF, 0x64, 0xA1, 0x00, 0x00, 0x00, 0x00 };
    if (memcmp(target, expected, DISPATCH_PATCH_SIZE) != 0) {
        char buf[256];
        snprintf(buf, sizeof(buf), "DispatchCollisionEvents byte mismatch: %02X %02X %02X %02X %02X %02X %02X %02X",
                 target[0], target[1], target[2], target[3], target[4], target[5], target[6], target[7]);
        diag_log(buf);
        return 0;
    }

    if (!VirtualProtect(target, DISPATCH_PATCH_SIZE, PAGE_EXECUTE_READWRITE, &oldProtect))
        return 0;

    /* Copy 8 original bytes to trampoline */
    memcpy(g_tramp, target, DISPATCH_PATCH_SIZE);

    /* Trampoline: original 8 bytes + JMP back to target+8 */
    g_tramp[DISPATCH_PATCH_SIZE] = 0xE9;
    *(unsigned long *)(g_tramp + DISPATCH_PATCH_SIZE + 1) =
        (unsigned long)((char *)target + DISPATCH_PATCH_SIZE - (char *)(g_tramp + DISPATCH_PATCH_SIZE) - 5);

    /* Make trampoline executable */
    DWORD tp;
    VirtualProtect(g_tramp, TRAMP_SIZE, PAGE_EXECUTE_READWRITE, &tp);

    /* Overwrite target: JMP to our hook (5 bytes) + 3 NOPs */
    unsigned long rel = (unsigned long)((char *)hook_DispatchCollisionEvents - (char *)target - 5);
    target[0] = 0xE9;
    *(unsigned long *)(target + 1) = rel;
    target[5] = 0x90;  /* NOP */
    target[6] = 0x90;  /* NOP */
    target[7] = 0x90;  /* NOP */

    FlushInstructionCache(GetCurrentProcess(), target, DISPATCH_PATCH_SIZE);

    g_orig_Dispatch = (dispatch_t)g_tramp;

    diag_log("DispatchCollisionEvents hook installed (8-byte trampoline)");
    return 1;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * HOOK 2: Phase 15 Code Cave (0x407BB4)
 *
 * Same as before — applies ongoing drag/buoyancy while in_water is set.
 * Called every frame for every ball via PUSHAD/PUSHFD + C function call.
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Compute normalized submersion (0.0 to 1.0) relative to captured surface Y.
 * Uses ball_y, radius, and surface_y:
 *   - 0.0 when ball is completely out of water (bottom at surface)
 *   - 0.5 when ball is half submerged (center at surface)
 *   - 1.0 when ball is fully submerged (top at surface)
 * Clamped to [0, 1] for above-surface and deep-underwater cases. */
static float compute_submersion(float ball_y, float radius, float surface_y)
{
    if (radius <= 0.0f) return 0.0f;
    float sub = (surface_y - ball_y + radius) / (2.0f * radius);
    if (sub < 0.0f) sub = 0.0f;
    if (sub > 1.0f) sub = 1.0f;
    return sub;
}

/* Ongoing water physics — called from Phase 15 code cave every frame */
static void __cdecl apply_water_physics(DWORD ball)
{
    if (!ball || IsBadReadPtr((void*)ball, 0x300)) return;

    water_state_t *st = get_ball_state(ball);
    if (!st) return;

    /* Decrement grace timer every frame, even if not in water */
    if (st->grace_frames > 0) st->grace_frames--;

    /* Clear 0x2E9 every frame while in water.
     * Hook 3 prevents type 5 from setting it while submerged, but as
     * belt-and-suspenders for edge cases (E:LIMIT via DispatchCollisionEvents,
     * or a frame timing gap), clear it here too.
     * During grace period, Hook 3 suppresses type 5 entirely — no need
     * to clear 0x2E9 there since it was never re-set. */
    if (st->in_water) {
        *(BYTE*)((DWORD)ball + BALL_FALLING_FLAG) = 0;

        /* Grant dizzy immunity every frame while submerged.
         * Mirrors Ball_DizzyImmunity(0x402400): clears bounce counter
         * and sets dizzy_immunity_timer to GRACE_PERIOD_FRAMES (only
         * increases, never shortens existing immunity). This means
         * the ball can't go dizzy while in water, and when it exits,
         * the remaining grace period also covers the bounce-out arc. */
        *(int*)((DWORD)ball + BALL_BOUNCE_COUNT) = 0;
        int cur = *(int*)((DWORD)ball + BALL_DIZZY_IMMUNITY);
        if (cur < GRACE_PERIOD_FRAMES)
            *(int*)((DWORD)ball + BALL_DIZZY_IMMUNITY) = GRACE_PERIOD_FRAMES;
    }

    if (!st->in_water) return;

    float ball_y = *(float*)(ball + BALL_POS_Y);
    float radius = *(float*)(ball + BALL_RADIUS);
    if (radius <= 0.0f || radius > 1000.0f) return;

    /* Exit condition: ball must be CLEARLY above the surface */
    if (ball_y - radius > st->water_surface_y + radius * 0.5f) {
        st->in_water = 0;
        st->water_surface_y = 0.0f;
        st->prev_submersion = 0.0f;
        st->grace_frames = GRACE_PERIOD_FRAMES;
        if (g_cfg.debug) {
            char buf[128];
            snprintf(buf, sizeof(buf), "WATER EXIT: ball=%08X y=%.2f grace=%d",
                     ball, ball_y, st->grace_frames);
            diag_log(buf);
        }
        return;
    }

    DWORD phys = *(DWORD*)(ball + BALL_PHYS_PTR);
    if (!phys || IsBadReadPtr((void*)phys, 0xCB0)) return;

    float *vel_x = (float*)(phys + PHYS_VEL_X);
    float *vel_y = (float*)(phys + PHYS_VEL_Y);
    float *vel_z = (float*)(phys + PHYS_VEL_Z);

    float submersion = compute_submersion(ball_y, radius, st->water_surface_y);

    /* Surface crossing detection: when the ball goes from above the 50%
     * submersion mark to below it (i.e. crosses the waterline downward),
     * apply entry_damping to Y velocity only. This catches every re-entry
     * into the water — bobbing, splashing, falling back in — without
     * relying on collision events or the in_water flag. */
    if (st->prev_submersion < 0.5f && submersion >= 0.5f) {
        float old_vy = *vel_y;
        *vel_y *= g_cfg.entry_damping;
        if (g_cfg.debug) {
            char buf[256];
            snprintf(buf, sizeof(buf), "SURFACE CROSS: ball=%08X sub %.2f->%.2f vy=%.2f->%.2f",
                     ball, st->prev_submersion, submersion, old_vy, *vel_y);
            diag_log(buf);
        }
    }
    st->prev_submersion = submersion;

    float vscale = 1.0f - g_cfg.drag;
    float hscale = 1.0f - (g_cfg.drag + g_cfg.horizontal_drag);
    if (vscale < 0.0f) vscale = 0.0f;
    if (hscale < 0.0f) hscale = 0.0f;

    *vel_x *= hscale;
    *vel_z *= hscale;

    /* Apply Y drag in BOTH directions (sinking and rising). */
    *vel_y *= vscale;

    /* Buoyancy = strength * submersion (0-1 normalized).
     * With strength=1.0 and gravity_scale=0.5, equilibrium is at
     * submersion=0.5 (ball floats half-submerged at the surface). */
    float buoyancy = g_cfg.buoyancy_strength * submersion;
    *vel_y += buoyancy;

    /* Running-water current (E:WATERFLOW).
     * Adds a constant per-frame force (flow_force) into the ball's force
     * accumulators (ball+0x170/174/178) in the flow direction. The physics
     * engine consumes these accumulators with proper collision response,
     * so the current pushes/carries the ball without teleporting it, and
     * is naturally capped by drag/friction exactly like other forces.
     * flow_dir: 1=N 2=NE 3=E 4=SE 5=S 6=SW 7=W 8=NW; 0 = calm, no force. */
    if (st->flow_dir != FLOW_NONE) {
        float *fx = (float*)((DWORD)ball + BALL_FORCE_X);
        float *fz = (float*)((DWORD)ball + BALL_FORCE_Z);
        float f = g_cfg.flow_force;
        /* Diagonal components (2,4,6,8) scaled by 1/sqrt(2) so the
         * resulting push magnitude equals f in every direction. */
        switch (st->flow_dir) {
            case 1:  *fz -= f;                                          break; /* N  */
            case 2:  *fx += f * FLOW_FORCE_NORMALIZE; *fz -= f * FLOW_FORCE_NORMALIZE; break; /* NE */
            case 3:  *fx += f;                                          break; /* E  */
            case 4:  *fx += f * FLOW_FORCE_NORMALIZE; *fz += f * FLOW_FORCE_NORMALIZE; break; /* SE */
            case 5:  *fz += f;                                          break; /* S  */
            case 6:  *fx -= f * FLOW_FORCE_NORMALIZE; *fz += f * FLOW_FORCE_NORMALIZE; break; /* SW */
            case 7:  *fx -= f;                                          break; /* W  */
            case 8:  *fx -= f * FLOW_FORCE_NORMALIZE; *fz -= f * FLOW_FORCE_NORMALIZE; break; /* NW */
            default: break; /* shouldn't happen */
        }
    }
}

/* Function pointer for code cave to call */
static void (__cdecl *g_water_fn_ptr)(DWORD) = NULL;

static BYTE *g_phase15_cave = NULL;
static volatile DWORD g_hook_calls = 0;

static void install_phase15_hook(void)
{
    BYTE *hook_addr = (BYTE*)PHASE15_HOOK;
    char buf[256];

    BYTE expected[] = { 0x8B, 0x4C, 0x24, 0x1C, 0x8B, 0x11 };
    snprintf(buf, sizeof(buf), "Phase15 bytes: %02X %02X %02X %02X %02X %02X",
             hook_addr[0], hook_addr[1], hook_addr[2],
             hook_addr[3], hook_addr[4], hook_addr[5]);
    diag_log(buf);

    if (memcmp(hook_addr, expected, PHASE15_ORIG_BYTES) != 0) {
        diag_log("PHASE15 BYTE MISMATCH!");
        return;
    }

    g_phase15_cave = (BYTE*)VirtualAlloc(NULL, 512, MEM_COMMIT | MEM_RESERVE,
                                           PAGE_EXECUTE_READWRITE);
    if (!g_phase15_cave) { diag_log("phase15: VirtualAlloc FAILED"); return; }

    int p = 0;

    /* PUSHAD — save all general-purpose registers */
    g_phase15_cave[p++] = 0x60;

    /* SUB ESP, 108 — make room for FNSAVE buffer (108 bytes = full x87 state) */
    g_phase15_cave[p++] = 0x83; g_phase15_cave[p++] = 0xEC;
    g_phase15_cave[p++] = 108;

    /* FNSAVE [ESP] — save complete x87 FPU state (stack + control/status tags)
     * This is the critical fix: PUSHFD does NOT save the FPU register stack.
     * Without this, float operations in apply_water_physics() corrupt the FPU
     * state that Ball_Update expects, causing crash at 0x407BC6.
     * DD /6 = FNSAVE m94byte. mod=00, rm=100 (SIB), base=ESP → DD 34 24 */
    g_phase15_cave[p++] = 0xDD; g_phase15_cave[p++] = 0x34; g_phase15_cave[p++] = 0x24;

    /* PUSHFD — save EFLAGS */
    g_phase15_cave[p++] = 0x9C;

    /* PUSH ESI (ball pointer) */
    g_phase15_cave[p++] = 0x56;

    /* CALL [g_water_fn_ptr] */
    g_phase15_cave[p++] = 0xFF; g_phase15_cave[p++] = 0x15;
    *(DWORD*)(g_phase15_cave + p) = (DWORD)&g_water_fn_ptr; p += 4;

    /* ADD ESP, 4 — clean up __cdecl arg */
    g_phase15_cave[p++] = 0x83; g_phase15_cave[p++] = 0xC4;
    g_phase15_cave[p++] = 0x04;

    /* INC [g_hook_calls] */
    g_phase15_cave[p++] = 0xFF; g_phase15_cave[p++] = 0x05;
    *(DWORD*)(g_phase15_cave + p) = (DWORD)&g_hook_calls; p += 4;

    /* POPFD — restore EFLAGS */
    g_phase15_cave[p++] = 0x9D;

    /* FRSTOR [ESP] — restore complete x87 FPU state
     * Must be AFTER POPFD so interrupts are in correct state.
     * FRSTOR restores: control word, status word, tag word, FPU registers
     * DD /4 = FRSTOR m94byte. mod=00, reg=100, rm=100 (SIB, base=ESP)
     * modrm = 0b00_100_100 = 0x24, SIB = 0x24 → DD 24 24
     * BUG FIX: v5 had DD 2C 24 = DD /5 = FLD m80t (loaded one garbage
     * float instead of restoring state → crash at race restart) */
    g_phase15_cave[p++] = 0xDD; g_phase15_cave[p++] = 0x24; g_phase15_cave[p++] = 0x24;

    /* ADD ESP, 108 — free FNSAVE buffer */
    g_phase15_cave[p++] = 0x83; g_phase15_cave[p++] = 0xC4;
    g_phase15_cave[p++] = 108;

    /* POPAD — restore all general-purpose registers */
    g_phase15_cave[p++] = 0x61;

    /* Original 6 bytes: MOV ECX,[ESP+0x1C] ; MOV EDX,[ECX] */
    g_phase15_cave[p++] = 0x8B; g_phase15_cave[p++] = 0x4C;
    g_phase15_cave[p++] = 0x24; g_phase15_cave[p++] = 0x1C;
    g_phase15_cave[p++] = 0x8B; g_phase15_cave[p++] = 0x11;

    /* JMP back */
    g_phase15_cave[p++] = 0xE9;
    *(DWORD*)(g_phase15_cave + p) =
        (DWORD)(hook_addr + PHASE15_ORIG_BYTES) - (DWORD)(g_phase15_cave + p + 4);
    p += 4;

    /* Patch hook site */
    DWORD old_protect;
    VirtualProtect(hook_addr, PHASE15_ORIG_BYTES, PAGE_EXECUTE_READWRITE, &old_protect);
    DWORD jmp_offset = (DWORD)(g_phase15_cave - hook_addr - 5);
    hook_addr[0] = 0xE9;
    *(DWORD*)(hook_addr + 1) = jmp_offset;
    hook_addr[5] = 0x90;
    VirtualProtect(hook_addr, PHASE15_ORIG_BYTES, old_protect, &old_protect);
    FlushInstructionCache(GetCurrentProcess(), hook_addr, PHASE15_ORIG_BYTES);

    snprintf(buf, sizeof(buf), "PHASE15 HOOK installed: %d bytes, cave=%08X (with FNSAVE/FRSTOR)", p, (DWORD)g_phase15_cave);
    diag_log(buf);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * HOOK 3: Type 5 collision suppressor — code cave at 0x407377 in Ball_Update
 *
 * The type 5 collision handler at 0x407377 checks if the ball has penetrated
 * past the mesh edge > 1.0 units. If so, it sets ball+0x2E9 (falling flag)
 * and switches the camera to falling mode.
 *
 * While the ball is in water (or within the grace period after leaving),
 * we suppress the death-flag WRITE (the MOV byte [ESI+0x2E9],1 at
 * 0x407391) by jumping to 0x407398 — the instruction right after it.
 * This is SURGICAL: the camera snap (CALL Scene_SetCamera), viewport
 * setup, and 0x2E8 split-flag logic all still run. Only the "falling/
 * dead" marker is never set, so death checks #1/#2 (0x406C0A, 0x40721F)
 * can't fire while in water / grace.
 *
 * E:LIMIT events still set 0x2E9 through DispatchCollisionEvents, so the
 * ball can still die from level boundaries while in water.
 *
 * Context at hook site:
 *   ESI = ball pointer
 *   FPU state has FCOMIP result from penetration comparison
 *   Original instruction: JNZ 0x40743D (6 bytes: 0F 85 C0 00 00 00)
 *
 * Cave logic:
 *   1. Check if ball (ESI) has in_water flag set
 *   2. If in_water → JMP 0x40743D (skip type 5 death block entirely)
 *   3. If not in_water → reproduce original JNZ 0x40743D, then fall
 *      through to 0x40737D (continue normal type 5 processing)
 * ═══════════════════════════════════════════════════════════════════════════ */

static BYTE *g_type5_cave = NULL;

/* Check if a ball pointer should be protected from type 5 death.
 * Returns 1 if ball is in water OR within the grace period after leaving water.
 * This prevents the type 5 death-flag WRITE (0x2E9) both while submerged
 * AND during the bounce-out arc where the ball may clip through mesh.
 * Camera snap / viewport / 0x2E8 split logic still run (surgical block). */
static int __cdecl is_ball_in_water(DWORD ball)
{
    if (!ball || IsBadReadPtr((void*)ball, 0x300)) return 0;
    water_state_t *st = get_ball_state(ball);
    return (st && (st->in_water || st->grace_frames > 0)) ? 1 : 0;
}

static int (__cdecl *g_is_in_water_ptr)(DWORD) = NULL;

static void install_type5_hook(void)
{
    BYTE *hook_addr = (BYTE*)ADDR_TYPE5_JNZ;
    char buf[256];

    /* Verify expected bytes: 0F 85 C0 00 00 00 (JNZ rel32) */
    BYTE expected[] = { 0x0F, 0x85, 0xC0, 0x00, 0x00, 0x00 };
    snprintf(buf, sizeof(buf), "Type5 bytes: %02X %02X %02X %02X %02X %02X",
             hook_addr[0], hook_addr[1], hook_addr[2],
             hook_addr[3], hook_addr[4], hook_addr[5]);
    diag_log(buf);

    if (memcmp(hook_addr, expected, TYPE5_PATCH_SIZE) != 0) {
        diag_log("TYPE5 BYTE MISMATCH!");
        return;
    }

    g_type5_cave = (BYTE*)VirtualAlloc(NULL, 256, MEM_COMMIT | MEM_RESERVE,
                                         PAGE_EXECUTE_READWRITE);
    if (!g_type5_cave) { diag_log("type5: VirtualAlloc FAILED"); return; }

    g_is_in_water_ptr = is_ball_in_water;

    int p = 0;

    /* --- Cave entry ---
     * ESI = ball pointer (preserved by __cdecl callee)
     * Flags = result from TEST AH,0x41 (penetration comparison)
     *
     * Stack order MUST be: PUSHFD first, then PUSH ESI (argument).
     * If reversed, the C function receives flags as its argument instead
     * of the ball pointer, and POPFD loads garbage → crash. */

    /* PUSHFD — save original flags (from TEST AH, 0x41 / penetration check) */
    g_type5_cave[p++] = 0x9C;

    /* PUSH ESI — ball pointer, becomes __cdecl argument at [ESP] */
    g_type5_cave[p++] = 0x56;

    /* CALL [g_is_in_water_ptr] — __cdecl, reads arg from [ESP+4] after retaddr */
    g_type5_cave[p++] = 0xFF; g_type5_cave[p++] = 0x15;
    *(DWORD*)(g_type5_cave + p) = (DWORD)&g_is_in_water_ptr; p += 4;

    /* ADD ESP, 4 — clean up __cdecl arg (ESI still preserved by callee) */
    g_type5_cave[p++] = 0x83; g_type5_cave[p++] = 0xC4;
    g_type5_cave[p++] = 0x04;

    /* TEST EAX, EAX — check is_in_water return (clobbers flags, but
     * original flags are still safe on the stack from PUSHFD) */
    g_type5_cave[p++] = 0x85; g_type5_cave[p++] = 0xC0;

    /* JNZ to cleanup — if in water, we need to clean up PUSHFD before
     * jumping to skip target. Jump to the ADD ESP,4 below, then
     * skip POPFD (flags don't matter — we're skipping the type 5 block)
     * and JMP to TYPE5_SKIP_TARGET. */
    g_type5_cave[p++] = 0x0F; g_type5_cave[p++] = 0x85;
    /* Target: the "in water cleanup" label below (we'll patch this) */
    int jnz_inwater_offset = p;
    p += 4;  /* placeholder for rel32 */

    /* --- Not in water path: --- */
    /* POPFD — restore original flags from TEST AH,0x41 penetration check */
    g_type5_cave[p++] = 0x9D;

    /* JNZ 0x40743D — reproduce original conditional jump
     * (penetration ≤ 1.0 → skip type 5 block) */
    g_type5_cave[p++] = 0x0F; g_type5_cave[p++] = 0x85;
    {
        DWORD src = (DWORD)(g_type5_cave + p + 4);
        DWORD dst = (DWORD)TYPE5_SKIP_TARGET;
        *(DWORD*)(g_type5_cave + p) = dst - src;
    }
    p += 4;

    /* Fall through: penetration > 1.0 and not in water → continue to 0x40737D */
    g_type5_cave[p++] = 0xE9;
    *(DWORD*)(g_type5_cave + p) =
        (DWORD)TYPE5_NEXT_INSTR - (DWORD)(g_type5_cave + p + 4);
    p += 4;

    /* --- In water cleanup path: --- */
    /* We reach here from the JNZ above. PUSHFD is still on stack.
     * Clean it up, then jump to TYPE5_AFTER_FLAG (0x407398).
     *
     * SURGICAL FIX (v7.2): instead of JMPing to TYPE5_SKIP_TARGET
     * (0x40743D, past the ENTIRE block), we JMP to 0x407398 — the
     * instruction right AFTER the "MOV byte [ESI+0x2E9],1" flag write.
     * This blocks ONLY the death-flag set (the thing that makes death
     * check #2 fire at the apex) while STILL letting the camera snap
     * (CALL Scene_SetCamera), viewport setup, and the split-flag
     * (0x2E8) run normally. So in water / within the grace period,
     * the ball survives the fall but the camera still follows it. */
    int inwater_cleanup_addr = p;
    /* ADD ESP, 4 — remove the saved EFLAGS from stack */
    g_type5_cave[p++] = 0x83; g_type5_cave[p++] = 0xC4;
    g_type5_cave[p++] = 0x04;
    /* JMP to TYPE5_AFTER_FLAG (0x407398) — skip only the 0x2E9 flag write */
    g_type5_cave[p++] = 0xE9;
    {
        DWORD src = (DWORD)(g_type5_cave + p + 4);
        DWORD dst = (DWORD)TYPE5_AFTER_FLAG;
        *(DWORD*)(g_type5_cave + p) = dst - src;
    }
    p += 4;

    /* Now patch the in-water JNZ to target the cleanup path */
    {
        DWORD src = (DWORD)(g_type5_cave + jnz_inwater_offset + 4);
        DWORD dst = (DWORD)(g_type5_cave + inwater_cleanup_addr);
        *(DWORD*)(g_type5_cave + jnz_inwater_offset) = dst - src;
    }

    /* --- Patch hook site: replace 6-byte JNZ with 5-byte JMP + 1 NOP --- */
    DWORD old_protect;
    VirtualProtect(hook_addr, TYPE5_PATCH_SIZE, PAGE_EXECUTE_READWRITE, &old_protect);
    DWORD jmp_offset = (DWORD)g_type5_cave - (DWORD)hook_addr - 5;
    hook_addr[0] = 0xE9;
    *(DWORD*)(hook_addr + 1) = jmp_offset;
    hook_addr[5] = 0x90;  /* NOP */
    VirtualProtect(hook_addr, TYPE5_PATCH_SIZE, old_protect, &old_protect);
    FlushInstructionCache(GetCurrentProcess(), hook_addr, TYPE5_PATCH_SIZE);

    snprintf(buf, sizeof(buf), "TYPE5 HOOK installed: %d bytes, cave=%08X", p, (DWORD)g_type5_cave);
    diag_log(buf);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * HOOK 4: Ball vtable[8] — suppress fall death while in water or grace period
 *
 * vtable[8] at 0x4CF3C0 → Ball_FallDeath (0x409480) is called when
 * the player ball should shatter from falling off an edge. If the
 * ball is currently in water OR within the grace period after leaving
 * water, skip the death entirely. E:LIMIT deaths are NOT suppressed
 * here — those go through DispatchCollisionEvents which sets 0x2E9,
 * and the death check fires here, but we skip it during grace period
 * too. That's acceptable because the grace period is short (5 seconds)
 * and the ball should be landing on solid ground by then.
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef void (__thiscall *ball_falldeath_t)(void *ball);
static ball_falldeath_t orig_Ball_FallDeath = NULL;

static void __thiscall Hook_Ball_FallDeath(void *ball)
{
    if (ball) {
        water_state_t *st = get_ball_state((DWORD)ball);
        if (st && (st->in_water || st->grace_frames > 0)) {
            if (g_cfg.debug) {
                char buf[128];
                snprintf(buf, sizeof(buf), "SUPPRESSED fall death: ball=%08X (in_water=%d grace=%d)",
                         (DWORD)ball, st->in_water, st->grace_frames);
                diag_log(buf);
            }
            return;  /* skip death — ball is in water or within grace period */
        }
    }
    if (orig_Ball_FallDeath) orig_Ball_FallDeath(ball);
}

static int install_falldeath_hook(void)
{
    void **slot = (void **)(ADDR_BALL_VTABLE + VTABLE_SLOT_ONRAMP * sizeof(void*));
    DWORD old_protect;

    if (IsBadReadPtr(slot, sizeof(void*))) {
        diag_log("FallDeath: vtable slot unreadable");
        return 0;
    }

    if (!VirtualProtect(slot, sizeof(void*), PAGE_EXECUTE_READWRITE, &old_protect))
        return 0;

    orig_Ball_FallDeath = (ball_falldeath_t)*slot;
    *slot = (void*)Hook_Ball_FallDeath;

    VirtualProtect(slot, sizeof(void*), old_protect, &old_protect);
    FlushInstructionCache(GetCurrentProcess(), slot, sizeof(void*));

    {
        char buf[128];
        snprintf(buf, sizeof(buf), "FallDeath hook installed: orig=%08X new=%08X",
                 (DWORD)orig_Ball_FallDeath, (DWORD)Hook_Ball_FallDeath);
        diag_log(buf);
    }
    return 1;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Patch Thread
 * ═══════════════════════════════════════════════════════════════════════════ */

static DWORD WINAPI patch_thread(LPVOID param)
{
    (void)param;
    char buf[256];

    diag_log("=== Water mod v7.6 loaded (E:WATERFLOW 1-8 directions) ===");
    Sleep(5000);

    g_water_fn_ptr = apply_water_physics;

    snprintf(buf, sizeof(buf), "Config: entry_damp=%.2f drag=%.3f hdrag=%.3f buoy=%.3f",
             g_cfg.entry_damping, g_cfg.drag, g_cfg.horizontal_drag,
             g_cfg.buoyancy_strength);
    diag_log(buf);

    /* Install Hook 1: DispatchCollisionEvents trampoline */
    install_dispatch_hook();

    /* Install Hook 2: Phase 15 code cave (water physics) */
    install_phase15_hook();

    /* Install Hook 3: Type 5 collision suppressor (prevent 0x2E9 while in water) */
    install_type5_hook();

    /* Install Hook 4: vtable[8] — suppress fall death during grace period */
    install_falldeath_hook();

    diag_log("All hooks installed");

    Sleep(8000);
    snprintf(buf, sizeof(buf), "After 8s: hook_calls=%u triggers=%u",
             g_hook_calls, g_trigger_count);
    diag_log(buf);

    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DllMain
 * ═══════════════════════════════════════════════════════════════════════════ */

BOOL APIENTRY DllMain(HMODULE hInst, DWORD reason, LPVOID lpReserved)
{
    (void)lpReserved;
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hInst);

        GetModuleFileNameA(hInst, g_logpath, MAX_PATH);
        {
            char *p = strrchr(g_logpath, '\\');
            if (p) strcpy(p + 1, "water_mod_log.txt");
        }

        diag_log("=== Water mod v7.6 DLL attaching ===");

        load_real_bass();
        {
            char buf[128];
            snprintf(buf, sizeof(buf), "bass_real = %08X", (DWORD)g_hRealBass);
            diag_log(buf);
        }

        {
            char ini_path[MAX_PATH];
            GetModuleFileNameA(hInst, ini_path, MAX_PATH);
            char *p = strrchr(ini_path, '\\');
            if (p) strcpy(p + 1, "hamsterball_water.ini");
            load_config(ini_path);
        }

        CreateThread(NULL, 0, patch_thread, NULL, 0, NULL);
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
