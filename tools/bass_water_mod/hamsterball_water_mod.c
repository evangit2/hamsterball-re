/*
 * hamsterball_water_mod.c — v3: Collision-triggered water physics
 *
 * BUILD (Linux -> Windows):
 *   i686-w64-mingw32-gcc -shared -o bass.dll hamsterball_water_mod.c -lwinmm \
 *     -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias
 *
 * INSTALLATION (Windows):
 *   1. In the Hamsterball game folder rename original bass.dll -> bass_real.dll
 *   2. Copy this proxy bass.dll + hamsterball_water.ini into the game folder
 *   3. Place collision objects named "E:WATER" in custom levels
 *   4. Run Hamsterball.exe normally
 *
 * HOW IT WORKS (v3):
 *   Two hooks:
 *
 *   HOOK 1 — DispatchCollisionEvents (0x40C5D0) trampoline:
 *     Intercepts all collision events. When the collision object's name starts
 *     with "E:WATER", fires the 3-step trigger:
 *       1. Sets in_water flag (gates ongoing water physics)
 *       2. Reduces ALL velocity axes by entry_damping (default 30%)
 *       3. Captures ball's Y as water_surface_y for this session
 *     Then calls the original DispatchCollisionEvents so the game processes
 *     the collision normally.
 *
 *   HOOK 2 — Phase 15 code cave (0x407BB4) in Ball_Update:
 *     Runs every frame for every ball. If in_water is set, applies:
 *       - Drag (all velocity axes scaled per frame)
 *       - Horizontal drag (extra scaling on X/Z)
 *       - Buoyancy (upward acceleration proportional to submersion)
 *     If ball rises above captured surface, clears in_water.
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

/* Ball struct offsets */
#define BALL_POS_X              0x164
#define BALL_POS_Y              0x168
#define BALL_POS_Z              0x16C
#define BALL_PHYS_PTR           0x1A4   /* Physics struct pointer */
#define BALL_RADIUS             0x284

/* Physics struct offsets */
#define PHYS_VEL_X              0xCA4
#define PHYS_VEL_Y              0xCA8
#define PHYS_VEL_Z              0xCAC

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
    int   debug;               /* write log file */
} water_cfg_t;

static water_cfg_t g_cfg = {
    0.70f,   /* entry_damping */
    0.03f,   /* drag */
    0.04f,   /* horizontal_drag */
    0.45f,   /* buoyancy_strength */
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

    g_cfg.entry_damping     = read_ini_float(ini_path, "WaterPhysics", "EntryDamping", 0.70f);
    g_cfg.drag              = read_ini_float(ini_path, "WaterPhysics", "Drag", 0.03f);
    g_cfg.horizontal_drag   = read_ini_float(ini_path, "WaterPhysics", "HorizontalDrag", 0.04f);
    g_cfg.buoyancy_strength = read_ini_float(ini_path, "WaterPhysics", "BuoyancyStrength", 0.45f);
    g_cfg.debug             = read_ini_int(ini_path, "WaterPhysics", "Debug", 1);

    if (g_cfg.entry_damping < 0.0f) g_cfg.entry_damping = 0.0f;
    if (g_cfg.entry_damping > 1.0f) g_cfg.entry_damping = 1.0f;
    if (g_cfg.drag < 0.0f) g_cfg.drag = 0.0f;
    if (g_cfg.drag > 1.0f) g_cfg.drag = 1.0f;
    if (g_cfg.horizontal_drag < 0.0f) g_cfg.horizontal_drag = 0.0f;
    if (g_cfg.horizontal_drag > 1.0f) g_cfg.horizontal_drag = 1.0f;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Per-ball water state
 * ═══════════════════════════════════════════════════════════════════════════ */

#define MAX_BALLS 32

typedef struct {
    DWORD ball;              /* ball pointer (key); 0 = unused */
    int   in_water;          /* currently in water? (gates physics) */
    float water_surface_y;   /* ball's Y at moment of contact */
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

/* Trigger function — called from the hooked DispatchCollisionEvents.
 * Does the 3-step trigger: set flag, damp velocity, capture Y. */
static void trigger_water_contact(void *ball_ptr)
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
    if (st->in_water) return;

    /* Step 1: Set in_water flag */
    st->in_water = 1;

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
        wsprintfA(buf, "WATER TRIGGER #%u: ball=%08X y=%.2f vel=(%.2f,%.2f,%.2f)->(%.2f,%.2f,%.2f)",
                  g_trigger_count, ball, ball_y,
                  *vel_x / damp, *vel_y / damp, *vel_z / damp,
                  *vel_x, *vel_y, *vel_z);
        diag_log(buf);
    }
}

/* The hooked DispatchCollisionEvents */
void __fastcall hook_DispatchCollisionEvents(void *this_, void *edx_dummy,
                                              void *ball, void *collObj)
{
    (void)edx_dummy;

    /* Check if this is an E:WATER collision */
    const char *name = get_collision_name(collObj);
    if (name && _strnicmp(name, "E:WATER", 7) == 0) {
        trigger_water_contact(ball);
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
        wsprintfA(buf, "DispatchCollisionEvents byte mismatch: %02X %02X %02X %02X %02X %02X %02X %02X",
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

/* Compute submersion fraction relative to captured surface Y */
static float compute_submersion(float ball_y, float radius, float surface_y)
{
    float bottom_y = ball_y - radius;
    float top_y = ball_y + radius;

    if (bottom_y >= surface_y) return 0.0f;
    if (top_y <= surface_y) return 1.0f;

    float submerged = (surface_y - bottom_y) / (2.0f * radius);
    if (submerged < 0.0f) submerged = 0.0f;
    if (submerged > 1.0f) submerged = 1.0f;
    return submerged;
}

/* Ongoing water physics — called from Phase 15 code cave every frame */
static void __cdecl apply_water_physics(DWORD ball)
{
    if (!ball || IsBadReadPtr((void*)ball, 0x300)) return;

    water_state_t *st = get_ball_state(ball);
    if (!st || !st->in_water) return;

    float ball_y = *(float*)(ball + BALL_POS_Y);
    float radius = *(float*)(ball + BALL_RADIUS);
    if (radius <= 0.0f || radius > 1000.0f) return;

    /* Exit condition: ball bottom above captured surface */
    if (ball_y - radius > st->water_surface_y) {
        st->in_water = 0;
        st->water_surface_y = 0.0f;
        return;
    }

    DWORD phys = *(DWORD*)(ball + BALL_PHYS_PTR);
    if (!phys || IsBadReadPtr((void*)phys, 0xCB0)) return;

    float *vel_x = (float*)(phys + PHYS_VEL_X);
    float *vel_y = (float*)(phys + PHYS_VEL_Y);
    float *vel_z = (float*)(phys + PHYS_VEL_Z);

    /* Submersion depth relative to captured surface */
    float submersion = compute_submersion(ball_y, radius, st->water_surface_y);
    if (submersion < 0.0f) submersion = 0.0f;

    /* Drag */
    float vscale = 1.0f - g_cfg.drag;
    float hscale = 1.0f - (g_cfg.drag + g_cfg.horizontal_drag);
    if (vscale < 0.0f) vscale = 0.0f;
    if (hscale < 0.0f) hscale = 0.0f;

    *vel_x *= hscale;
    *vel_z *= hscale;
    *vel_y *= vscale;

    /* Buoyancy */
    float buoyancy = g_cfg.buoyancy_strength * submersion * 2.0f;
    *vel_y += buoyancy;
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
    wsprintfA(buf, "Phase15 bytes: %02X %02X %02X %02X %02X %02X",
              hook_addr[0], hook_addr[1], hook_addr[2],
              hook_addr[3], hook_addr[4], hook_addr[5]);
    diag_log(buf);

    if (memcmp(hook_addr, expected, PHASE15_ORIG_BYTES) != 0) {
        diag_log("PHASE15 BYTE MISMATCH!");
        return;
    }

    g_phase15_cave = (BYTE*)VirtualAlloc(NULL, 256, MEM_COMMIT | MEM_RESERVE,
                                           PAGE_EXECUTE_READWRITE);
    if (!g_phase15_cave) { diag_log("phase15: VirtualAlloc FAILED"); return; }

    int p = 0;

    /* PUSHAD */
    g_phase15_cave[p++] = 0x60;
    /* PUSHFD */
    g_phase15_cave[p++] = 0x9C;

    /* PUSH ESI (ball pointer) */
    g_phase15_cave[p++] = 0x56;

    /* CALL [g_water_fn_ptr] */
    g_phase15_cave[p++] = 0xFF; g_phase15_cave[p++] = 0x15;
    *(DWORD*)(g_phase15_cave + p) = (DWORD)&g_water_fn_ptr; p += 4;

    /* ADD ESP, 4 */
    g_phase15_cave[p++] = 0x83; g_phase15_cave[p++] = 0xC4;
    g_phase15_cave[p++] = 0x04;

    /* INC [g_hook_calls] */
    g_phase15_cave[p++] = 0xFF; g_phase15_cave[p++] = 0x05;
    *(DWORD*)(g_phase15_cave + p) = (DWORD)&g_hook_calls; p += 4;

    /* POPFD */
    g_phase15_cave[p++] = 0x9D;
    /* POPAD */
    g_phase15_cave[p++] = 0x61;

    /* Original 6 bytes */
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

    wsprintfA(buf, "PHASE15 HOOK installed: %d bytes, cave=%08X", p, (DWORD)g_phase15_cave);
    diag_log(buf);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Patch Thread
 * ═══════════════════════════════════════════════════════════════════════════ */

static DWORD WINAPI patch_thread(LPVOID param)
{
    (void)param;
    char buf[256];

    diag_log("=== Water mod v3 loaded (collision-triggered) ===");
    Sleep(5000);

    g_water_fn_ptr = apply_water_physics;

    wsprintfA(buf, "Config: entry_damp=%.2f drag=%.3f hdrag=%.3f buoy=%.3f",
              g_cfg.entry_damping, g_cfg.drag, g_cfg.horizontal_drag,
              g_cfg.buoyancy_strength);
    diag_log(buf);

    /* Install Hook 1: DispatchCollisionEvents trampoline */
    install_dispatch_hook();

    /* Install Hook 2: Phase 15 code cave */
    install_phase15_hook();

    diag_log("All hooks installed");

    Sleep(8000);
    wsprintfA(buf, "After 8s: hook_calls=%u triggers=%u",
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

        diag_log("=== Water mod v3 DLL attaching ===");

        load_real_bass();
        {
            char buf[128];
            wsprintfA(buf, "bass_real = %08X", (DWORD)g_hRealBass);
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
