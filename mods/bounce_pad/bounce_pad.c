/*
 * bounce_pad.c — BASS.dll proxy — N:BOUNCE custom collision event
 *
 * Adds a custom "N:BOUNCE" event to Hamsterball's collision dispatch system.
 * When the ball touches an object named "N:BOUNCE" in a MESHWORLD level,
 * the ball's Y velocity is reversed and damped — a proportional bounce.
 *
 * Fall faster = bounce higher. Bounce factor 0.8 = 20% energy loss per bounce.
 * X and Z velocities are never read or modified.
 *
 * How it works:
 *   1. 8-byte inline detour on DispatchCollisionEvents (0x0040C5D0)
 *   2. Hook checks collision entry's event name string for "N:BOUNCE"
 *   3. If match: reads phys+0xCA8 (Y velocity), writes -vy * 0.8
 *   4. Calls original handler for ALL events (N:BOUNCE is a no-op in original)
 *
 * MESHWORLD usage:
 *   Name an object "N:BOUNCE" in a custom level. When the ball lands on it,
 *   it bounces back up proportional to fall speed.
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll bounce_pad.c -lwinmm \
 *     -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * BASS Proxy Exports
 * ═══════════════════════════════════════════════════════════════════════════ */

static HMODULE g_hRealBass = NULL;

static void load_real_bass(void)
{
    g_hRealBass = LoadLibraryA("bass_real.dll");
    if (!g_hRealBass) {
        char path[MAX_PATH];
        HMODULE hSelf = NULL;
        GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                          | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                          (LPCSTR)&load_real_bass, &hSelf);
        if (GetModuleFileNameA(hSelf, path, MAX_PATH)) {
            char *p = strrchr(path, '\\');
            if (p) { strcpy(p + 1, "bass_real.dll"); g_hRealBass = LoadLibraryA(path); }
        }
    }
}

typedef int  (__stdcall *BASS_Init_t)(int, DWORD, DWORD, HWND, void*);
static BASS_Init_t real_BASS_Init = NULL;
__declspec(dllexport) int __stdcall BASS_Init(int a, DWORD b, DWORD c, HWND d, void* e) {
    if (g_hRealBass) { if (!real_BASS_Init) real_BASS_Init = (BASS_Init_t)GetProcAddress(g_hRealBass, "BASS_Init"); if (real_BASS_Init) return real_BASS_Init(a, b, c, d, e); } return 1;
}
typedef int  (__stdcall *BASS_Free_t)(void);
static BASS_Free_t real_BASS_Free = NULL;
__declspec(dllexport) int __stdcall BASS_Free(void) {
    if (g_hRealBass) { if (!real_BASS_Free) real_BASS_Free = (BASS_Free_t)GetProcAddress(g_hRealBass, "BASS_Free"); if (real_BASS_Free) return real_BASS_Free(); } return 1;
}
typedef int  (__stdcall *BASS_Start_t)(void);
static BASS_Start_t real_BASS_Start = NULL;
__declspec(dllexport) int __stdcall BASS_Start(void) {
    if (g_hRealBass) { if (!real_BASS_Start) real_BASS_Start = (BASS_Start_t)GetProcAddress(g_hRealBass, "BASS_Start"); if (real_BASS_Start) return real_BASS_Start(); } return 1;
}
typedef int  (__stdcall *BASS_Stop_t)(void);
static BASS_Stop_t real_BASS_Stop = NULL;
__declspec(dllexport) int __stdcall BASS_Stop(void) {
    if (g_hRealBass) { if (!real_BASS_Stop) real_BASS_Stop = (BASS_Stop_t)GetProcAddress(g_hRealBass, "BASS_Stop"); if (real_BASS_Stop) return real_BASS_Stop(); } return 1;
}
typedef int  (__stdcall *BASS_SetConfig_t)(DWORD, DWORD);
static BASS_SetConfig_t real_BASS_SetConfig = NULL;
__declspec(dllexport) int __stdcall BASS_SetConfig(DWORD a, DWORD b) {
    if (g_hRealBass) { if (!real_BASS_SetConfig) real_BASS_SetConfig = (BASS_SetConfig_t)GetProcAddress(g_hRealBass, "BASS_SetConfig"); if (real_BASS_SetConfig) return real_BASS_SetConfig(a, b); } return 1;
}
typedef int  (__stdcall *BASS_ErrorGetCode_t)(void);
static BASS_ErrorGetCode_t real_BASS_ErrorGetCode = NULL;
__declspec(dllexport) int __stdcall BASS_ErrorGetCode(void) {
    if (g_hRealBass) { if (!real_BASS_ErrorGetCode) real_BASS_ErrorGetCode = (BASS_ErrorGetCode_t)GetProcAddress(g_hRealBass, "BASS_ErrorGetCode"); if (real_BASS_ErrorGetCode) return real_BASS_ErrorGetCode(); } return 0;
}
typedef int  (__stdcall *BASS_MusicPlayEx_t)(DWORD, DWORD, BOOL);
static BASS_MusicPlayEx_t real_BASS_MusicPlayEx = NULL;
__declspec(dllexport) int __stdcall BASS_MusicPlayEx(DWORD a, DWORD b, BOOL c) {
    if (g_hRealBass) { if (!real_BASS_MusicPlayEx) real_BASS_MusicPlayEx = (BASS_MusicPlayEx_t)GetProcAddress(g_hRealBass, "BASS_MusicPlayEx"); if (real_BASS_MusicPlayEx) return real_BASS_MusicPlayEx(a, b, c); } return 1;
}
typedef int  (__stdcall *BASS_ChannelSetAttributes_t)(DWORD, float, int, int);
static BASS_ChannelSetAttributes_t real_BASS_ChannelSetAttributes = NULL;
__declspec(dllexport) int __stdcall BASS_ChannelSetAttributes(DWORD a, float b, int c, int d) {
    if (g_hRealBass) { if (!real_BASS_ChannelSetAttributes) real_BASS_ChannelSetAttributes = (BASS_ChannelSetAttributes_t)GetProcAddress(g_hRealBass, "BASS_ChannelSetAttributes"); if (real_BASS_ChannelSetAttributes) return real_BASS_ChannelSetAttributes(a, b, c, d); } return 1;
}
typedef int  (__stdcall *BASS_ChannelStop_t)(DWORD);
static BASS_ChannelStop_t real_BASS_ChannelStop = NULL;
__declspec(dllexport) int __stdcall BASS_ChannelStop(DWORD a) {
    if (g_hRealBass) { if (!real_BASS_ChannelStop) real_BASS_ChannelStop = (BASS_ChannelStop_t)GetProcAddress(g_hRealBass, "BASS_ChannelStop"); if (real_BASS_ChannelStop) return real_BASS_ChannelStop(a); } return 1;
}
typedef DWORD (__stdcall *BASS_MusicLoad_t)(int, void*, DWORD, DWORD, DWORD, DWORD);
static BASS_MusicLoad_t real_BASS_MusicLoad = NULL;
__declspec(dllexport) DWORD __stdcall BASS_MusicLoad(int a, void* b, DWORD c, DWORD d, DWORD e, DWORD f) {
    if (g_hRealBass) { if (!real_BASS_MusicLoad) real_BASS_MusicLoad = (BASS_MusicLoad_t)GetProcAddress(g_hRealBass, "BASS_MusicLoad"); if (real_BASS_MusicLoad) return real_BASS_MusicLoad(a, b, c, d, e, f); } return 0;
}

/* Extra stubs */
__declspec(dllexport) void __stdcall BASS_Pause(void) {}
__declspec(dllexport) void __stdcall BASS_SetVolume(DWORD a) {}
__declspec(dllexport) DWORD __stdcall BASS_GetVolume(void) { return 0; }
__declspec(dllexport) int __stdcall BASS_GetDevice(void) { return 0; }
__declspec(dllexport) int __stdcall BASS_SetDevice(DWORD a) { return 1; }
__declspec(dllexport) void __stdcall BASS_GetInfo(void *a) {}
__declspec(dllexport) int __stdcall BASS_Update(DWORD a) { return 0; }
__declspec(dllexport) DWORD __stdcall BASS_StreamCreateFile(void *a, void *b, DWORD c, DWORD d, DWORD e) { return 0; }
__declspec(dllexport) DWORD __stdcall BASS_SampleLoad(int a, void *b, DWORD c, DWORD d, DWORD e) { return 0; }
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

/* ═══════════════════════════════════════════════════════════════════════════
 * N:BOUNCE Collision Event Mod
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Game addresses */
#define ADDR_DispatchCollisionEvents  0x0040C5D0

/* Ball struct offsets */
#define BALL_PHYS_OBJ      0x1A4   /* int* — PhysicsObject pointer */
#define BALL_COOLDOWN      0x7DC   /* int — E:JUMP uses this, decremented by game */

/* PhysicsObject offsets */
#define PHYS_VEL_Y         0xCA8   /* float — Y velocity (negative = falling) */

/* Bounce parameters */
#define BOUNCE_FACTOR      0.8f    /* 80% energy retention = 20% loss per bounce */
#define BOUNCE_COOLDOWN   10      /* frames before next bounce can trigger */

/*
 * DispatchCollisionEvents entry bytes (verified via GhidraMCP read_memory):
 *   6A FF              PUSH -1           (2 bytes)
 *   64 A1 00 00 00 00  MOV EAX, FS:[0]   (6 bytes)
 * Total: 8 bytes = 2 complete instructions.
 * A 5-byte detour would split the MOV instruction → crash.
 * Must use 8-byte detour (JMP + 3 NOPs).
 */
#define DETOUR_SIZE        8
static const BYTE EXPECTED_ENTRY[DETOUR_SIZE] = {
    0x6A, 0xFF,                           /* PUSH -1 */
    0x64, 0xA1, 0x00, 0x00, 0x00, 0x00    /* MOV EAX, FS:[0] */
};

/*
 * __thiscall workaround for MinGW:
 * DispatchCollisionEvents is __thiscall(this, param1, param2) with RET 0x8.
 * __fastcall(this, dummy_edx, param1, param2) matches the calling convention:
 *   ECX = this (board), stack params = [ball, entry_pair], callee cleans 8.
 */
typedef void (__fastcall *DispatchFn)(void *board, void *edx_dummy,
                                        void *ball, void *entry_pair);

static DispatchFn g_orig_Dispatch = NULL;
static unsigned char g_trampoline[16];

/* ── Hook function ──────────────────────────────────────────────────────── */

static void __fastcall hook_Dispatch(void *board, void *dummy,
                                      void *ball, void *entry_pair)
{
    (void)dummy;

    /* Check if this is an N:BOUNCE event */
    if (ball && entry_pair && !IsBadReadPtr(entry_pair, 8)) {

        int *pair = (int *)entry_pair;
        int collision_data = pair[1];

        if (collision_data && !IsBadReadPtr((void *)collision_data, 0x868)) {

            char *event_name = *(char **)((char *)collision_data + 0x864);

            if (event_name && !IsBadReadPtr(event_name, 8)) {

                if (_stricmp(event_name, "N:BOUNCE") == 0) {

                    /* Cooldown check (same field E:JUMP uses) */
                    int cooldown = *(int *)((char *)ball + BALL_COOLDOWN);
                    if (cooldown < 1) {

                        /* Get PhysicsObject from ball */
                        int *phys = *(int **)((char *)ball + BALL_PHYS_OBJ);
                        if (phys && !IsBadReadPtr(phys, 0xCB0)) {

                            /* Read Y velocity (negative = falling) */
                            float vy = *(float *)((char *)phys + PHYS_VEL_Y);

                            /* Bounce: reverse Y velocity × factor
                             * Fall at -30 → bounce at +24 (upward, 80%)
                             * Fall at -10 → bounce at +8  (upward, 80%)
                             * X and Z velocities are never touched */
                            *(float *)((char *)phys + PHYS_VEL_Y) = -vy * BOUNCE_FACTOR;

                            /* Set cooldown to prevent retriggering */
                            *(int *)((char *)ball + BALL_COOLDOWN) = BOUNCE_COOLDOWN;
                        }
                    }
                }
            }
        }
    }

    /* Call original handler for ALL events.
     * N:BOUNCE is unrecognized by the original → falls through all
     * __stricmp checks → returns without doing anything. Safe. */
    if (g_orig_Dispatch)
        g_orig_Dispatch(board, NULL, ball, entry_pair);
}

/* ── 8-byte inline detour installation ──────────────────────────────────── */

static int install_detour(void)
{
    unsigned char *target = (unsigned char *)ADDR_DispatchCollisionEvents;
    DWORD old_protect;

    /* Verify entry bytes match expected pattern */
    if (memcmp(target, EXPECTED_ENTRY, DETOUR_SIZE) != 0)
        return 0;  /* Wrong game version or prior patch */

    /* Copy original 8 bytes to trampoline */
    memcpy(g_trampoline, target, DETOUR_SIZE);

    /* Append JMP back to target + DETOUR_SIZE */
    g_trampoline[DETOUR_SIZE] = 0xE9;  /* JMP rel32 */
    *(unsigned long *)(g_trampoline + DETOUR_SIZE + 1) =
        (unsigned long)((target + DETOUR_SIZE) -
                        (g_trampoline + DETOUR_SIZE + 5));

    /* Make trampoline executable */
    DWORD tp;
    VirtualProtect(g_trampoline, 16, PAGE_EXECUTE_READWRITE, &tp);

    /* Set up original function pointer */
    g_orig_Dispatch = (DispatchFn)g_trampoline;

    /* Overwrite target: JMP rel32 + 3 NOPs = 8 bytes */
    if (!VirtualProtect(target, DETOUR_SIZE + 3, PAGE_EXECUTE_READWRITE, &old_protect))
        return 0;

    unsigned long rel = (unsigned long)((unsigned char *)hook_Dispatch - target - 5);
    target[0] = 0xE9;                              /* JMP rel32 */
    *(unsigned long *)(target + 1) = rel;
    target[5] = 0x90;                               /* NOP */
    target[6] = 0x90;                               /* NOP */
    target[7] = 0x90;                               /* NOP */

    FlushInstructionCache(GetCurrentProcess(), target, DETOUR_SIZE);
    VirtualProtect(target, DETOUR_SIZE + 3, old_protect, &old_protect);

    return 1;
}

/* ── Patch thread ───────────────────────────────────────────────────────── */

static DWORD WINAPI patch_thread(LPVOID param)
{
    (void)param;
    Sleep(5000);  /* Wait for game to fully load */

    if (!install_detour())
        return 1;

    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DLL Entry
 * ═══════════════════════════════════════════════════════════════════════════ */

BOOL APIENTRY DllMain(HMODULE hInst, DWORD reason, LPVOID lpReserved)
{
    (void)lpReserved;
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hInst);
        load_real_bass();
        CreateThread(NULL, 0, patch_thread, NULL, 0, NULL);
        break;
    }
    return TRUE;
}
