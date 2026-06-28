/*
 * custom_lifter — Modified Up Race lifter state machine
 *
 * Replaces the Rotator vtable[0x0B] render function (0x0043D420) with a
 * custom state machine:
 *   State 0 (bottom pause): 300 frames, then → state 1, timer=1500
 *   State 1 (rising):       3 sub-steps/frame × +0.1 Y, no ball carry, no sound
 *                           1500 frames, then → state 2, timer=300
 *   State 2 (top pause):    300 frames, then → state 3, timer=1500
 *   State 3 (falling):      3 sub-steps/frame × -0.2 Y, no ball carry, no sound
 *                           1500 frames, then → state 0, timer=400, NO arrival sound
 *
 * The dirty flag (mesh repositioning) is handled by calling the original
 * function with a temporary no-op state (0, timer=99999), then restoring
 * our real state/timer/pos_y and running the custom state machine.
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

typedef unsigned __int64 QWORD;

/* ═══════════════════════════════════════════════════════════════════
 *  BASS proxy exports (10 required game imports)
 * ═══════════════════════════════════════════════════════════════════ */

static HMODULE g_hRealBass = NULL;
static void lazy_load_real_bass(void);  /* forward decl */

typedef int  (__stdcall *BASS_ChannelSetAttributes_t)(DWORD, float, int, int);
static BASS_ChannelSetAttributes_t real_BASS_ChannelSetAttributes = NULL;
__declspec(dllexport) int __stdcall BASS_ChannelSetAttributes(DWORD a, float b, int c, int d) {
    lazy_load_real_bass();
    if (real_BASS_ChannelSetAttributes) return real_BASS_ChannelSetAttributes(a, b, c, d);
    return 1;
}

typedef int  (__stdcall *BASS_MusicPlayEx_t)(DWORD, DWORD, BOOL);
static BASS_MusicPlayEx_t real_BASS_MusicPlayEx = NULL;
__declspec(dllexport) int __stdcall BASS_MusicPlayEx(DWORD a, DWORD b, BOOL c) {
    lazy_load_real_bass();
    if (real_BASS_MusicPlayEx) return real_BASS_MusicPlayEx(a, b, c);
    return 1;
}

typedef int  (__stdcall *BASS_MusicLoad_t)(int, const char*, QWORD, DWORD, DWORD, DWORD);
static BASS_MusicLoad_t real_BASS_MusicLoad = NULL;
__declspec(dllexport) int __stdcall BASS_MusicLoad(int a, const char* b, QWORD c, DWORD d, DWORD e, DWORD f) {
    lazy_load_real_bass();
    if (real_BASS_MusicLoad) return real_BASS_MusicLoad(a, b, c, d, e, f);
    return 0;
}

typedef int  (__stdcall *BASS_SampleLoad_t)(int, const char*, QWORD, DWORD, DWORD, DWORD);
static BASS_SampleLoad_t real_BASS_SampleLoad = NULL;
__declspec(dllexport) int __stdcall BASS_SampleLoad(int a, const char* b, QWORD c, DWORD d, DWORD e, DWORD f) {
    lazy_load_real_bass();
    if (real_BASS_SampleLoad) return real_BASS_SampleLoad(a, b, c, d, e, f);
    return 0;
}

typedef int  (__stdcall *BASS_StreamCreateFile_t)(int, const char*, QWORD, QWORD, DWORD);
static BASS_StreamCreateFile_t real_BASS_StreamCreateFile = NULL;
__declspec(dllexport) int __stdcall BASS_StreamCreateFile(int a, const char* b, QWORD c, QWORD d, DWORD e) {
    lazy_load_real_bass();
    if (real_BASS_StreamCreateFile) return real_BASS_StreamCreateFile(a, b, c, d, e);
    return 0;
}

typedef int  (__stdcall *BASS_SampleGetChannel_t)(DWORD, BOOL);
static BASS_SampleGetChannel_t real_BASS_SampleGetChannel = NULL;
__declspec(dllexport) int __stdcall BASS_SampleGetChannel(DWORD a, BOOL b) {
    lazy_load_real_bass();
    if (real_BASS_SampleGetChannel) return real_BASS_SampleGetChannel(a, b);
    return 0;
}

typedef int  (__stdcall *BASS_ChannelPlay_t)(DWORD, BOOL);
static BASS_ChannelPlay_t real_BASS_ChannelPlay = NULL;
__declspec(dllexport) int __stdcall BASS_ChannelPlay(DWORD a, BOOL b) {
    lazy_load_real_bass();
    if (real_BASS_ChannelPlay) return real_BASS_ChannelPlay(a, b);
    return 1;
}

typedef int  (__stdcall *BASS_ChannelStop_t)(DWORD);
static BASS_ChannelStop_t real_BASS_ChannelStop = NULL;
__declspec(dllexport) int __stdcall BASS_ChannelStop(DWORD a) {
    lazy_load_real_bass();
    if (real_BASS_ChannelStop) return real_BASS_ChannelStop(a);
    return 1;
}

typedef int  (__stdcall *BASS_Init_t)(int, DWORD, DWORD, DWORD, DWORD);
static BASS_Init_t real_BASS_Init = NULL;
__declspec(dllexport) int __stdcall BASS_Init(int a, DWORD b, DWORD c, DWORD d, DWORD e) {
    lazy_load_real_bass();
    if (real_BASS_Init) return real_BASS_Init(a, b, c, d, e);
    return 1;
}

typedef int  (__stdcall *BASS_Free_t)(void);
static BASS_Free_t real_BASS_Free = NULL;
__declspec(dllexport) int __stdcall BASS_Free(void) {
    lazy_load_real_bass();
    if (real_BASS_Free) return real_BASS_Free();
    return 1;
}

typedef int  (__stdcall *BASS_Stop_t)(void);
static BASS_Stop_t real_BASS_Stop = NULL;
__declspec(dllexport) int __stdcall BASS_Stop(void) {
    lazy_load_real_bass();
    if (real_BASS_Stop) return real_BASS_Stop();
    return 1;
}

typedef int  (__stdcall *BASS_Start_t)(void);
static BASS_Start_t real_BASS_Start = NULL;
__declspec(dllexport) int __stdcall BASS_Start(void) {
    lazy_load_real_bass();
    if (real_BASS_Start) return real_BASS_Start();
    return 1;
}

typedef int  (__stdcall *BASS_SetConfig_t)(DWORD, DWORD);
static BASS_SetConfig_t real_BASS_SetConfig = NULL;
__declspec(dllexport) int __stdcall BASS_SetConfig(DWORD a, DWORD b) {
    lazy_load_real_bass();
    if (real_BASS_SetConfig) return real_BASS_SetConfig(a, b);
    return 1;
}

typedef int  (__stdcall *BASS_ErrorGetCode_t)(void);
static BASS_ErrorGetCode_t real_BASS_ErrorGetCode = NULL;
__declspec(dllexport) int __stdcall BASS_ErrorGetCode(void) {
    lazy_load_real_bass();
    if (real_BASS_ErrorGetCode) return real_BASS_ErrorGetCode();
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════
 *  Lifter mod
 * ═══════════════════════════════════════════════════════════════════ */

/* Rotator struct offsets (byte offsets) */
#define OFF_BOARD      0x10D0
#define OFF_POS_X      0x10D4
#define OFF_POS_Y      0x10D8   /* this is what moves up/down */
#define OFF_POS_Z      0x10DC
#define OFF_CHILD_MESH 0x10E0
#define OFF_STATE      0x10E4
#define OFF_TIMER      0x10E8
#define OFF_PAUSE_DUR  0x10EC
#define OFF_DIRTY      0x10F0

/* Rotator vtable address (in .rdata) */
#define ROTATOR_VTABLE  0x004D5770
/* vtable[0x0B] = index 11 = byte offset 0x2C */
#define VTABLE_0B_OFFSET 0x2C

/* Original function pointer (saved before patching) */
typedef int (__thiscall *RenderFn_t)(void *thisptr);
static RenderFn_t g_OriginalRender = NULL;

/* Timer durations (frames) */
#define T_BOTTOM_PAUSE_1  300
#define T_RISE            1500
#define T_TOP_PAUSE       300
#define T_FALL            1500
#define T_BOTTOM_PAUSE_2  400

/* Movement per sub-step */
#define RISE_STEP  0.1f
#define FALL_STEP  0.2f
#define SUB_STEPS  3

/*
 * Custom lifter render function.
 * Replaces Rotator vtable[0x0B] (original at 0x0043D420).
 *
 * Strategy: Call the original function with a temporary no-op state
 * (state=0, timer=99999) so it handles the dirty-flag mesh repositioning
 * (Timer_Init, Gfx_SetPosition, vtable calls, Timer_Cleanup) without
 * running the original state machine. Then restore our real state/timer/
 * pos_y and run the custom state machine.
 */
__attribute__((used, noinline))
int __thiscall CustomLifter_Render(void *thisptr)
{
    BYTE  *bytes  = (BYTE *)thisptr;

    /* ── Save our real state/timer/pos_y ────────────────────────── */
    int   saved_state  = *(int  *)(bytes + OFF_STATE);
    int   saved_timer  = *(int  *)(bytes + OFF_TIMER);
    float saved_pos_y  = *(float*)(bytes + OFF_POS_Y);

    /* ── Temporarily set no-op state so original does nothing ───── */
    *(int *)(bytes + OFF_STATE) = 0;       /* state 0 = idle, just timer countdown */
    *(int *)(bytes + OFF_TIMER) = 99999;  /* huge timer → no state transition */

    /* ── Call original: handles dirty flag + mesh repositioning ── */
    /* Original in state 0 with timer 99999: decrements timer, no
       movement, no sound, no ball scan. Dirty flag is processed at
       the top before the switch, so mesh gets repositioned if needed. */
    g_OriginalRender(thisptr);

    /* ── Restore our real values ───────────────────────────────── */
    *(int  *)(bytes + OFF_STATE) = saved_state;
    *(int  *)(bytes + OFF_TIMER) = saved_timer;
    *(float*)(bytes + OFF_POS_Y) = saved_pos_y;

    /* ── Run custom state machine ──────────────────────────────── */
    int   *state  = (int  *)(bytes + OFF_STATE);
    int   *timer  = (int  *)(bytes + OFF_TIMER);
    float *pos_y  = (float*)(bytes + OFF_POS_Y);
    BYTE  *dirty  = (BYTE *)(bytes + OFF_DIRTY);

    switch (*state) {

    /* ── State 0: IDLE (bottom pause) ───────────────────────────── */
    case 0:
        (*timer)--;
        if (*timer < 1) {
            *state = 1;          /* → rising */
            *timer = T_RISE;
        }
        break;

    /* ── State 1: RISING ────────────────────────────────────────── */
    case 1:
        for (int i = 0; i < SUB_STEPS; i++) {
            *pos_y += RISE_STEP;
        }
        *dirty = 1;              /* mark mesh for repositioning next frame */
        /* No ball carrying */
        /* No clunk sound */
        (*timer)--;
        if (*timer < 1) {
            *state = 2;          /* → top pause */
            *timer = T_TOP_PAUSE;
        }
        break;

    /* ── State 2: IDLE (top pause) ──────────────────────────────── */
    case 2:
        (*timer)--;
        if (*timer < 1) {
            *state = 3;          /* → falling */
            *timer = T_FALL;
        }
        break;

    /* ── State 3: FALLING ──────────────────────────────────────── */
    case 3:
        for (int i = 0; i < SUB_STEPS; i++) {
            *pos_y -= FALL_STEP;
        }
        *dirty = 1;
        /* No ball carrying */
        (*timer)--;
        if (*timer < 1) {
            *state = 0;          /* → bottom pause */
            *timer = T_BOTTOM_PAUSE_2;
            /* No arrival sound */
        }
        break;
    }

    return 1;
}

/* ═══════════════════════════════════════════════════════════════════
 *  Hook installation
 * ═══════════════════════════════════════════════════════════════════ */

static void install_lifter_hook(void)
{
    /* The Rotator vtable is at 0x004D5770 in .rdata.
       vtable[0x0B] is at offset 0x2C = address 0x004D579C.
       Original function at 0x0043D420. */

    DWORD *vtable_entry = (DWORD *)(ROTATOR_VTABLE + VTABLE_0B_OFFSET);
    DWORD old_protect;

    /* Verify the original function pointer matches expected value */
    DWORD expected = 0x0043D420;
    if (*vtable_entry != expected) {
        /* Vtable entry doesn't match — wrong game version or already patched */
        return;
    }

    /* Save original */
    g_OriginalRender = (RenderFn_t)(*vtable_entry);

    /* Make vtable entry writable */
    VirtualProtect(vtable_entry, 4, PAGE_EXECUTE_READWRITE, &old_protect);

    /* Patch with our function */
    *vtable_entry = (DWORD)&CustomLifter_Render;

    /* Restore protection */
    VirtualProtect(vtable_entry, 4, old_protect, &old_protect);
    FlushInstructionCache(GetCurrentProcess(), vtable_entry, 4);
}

/* ═══════════════════════════════════════════════════════════════════
 *  BASS proxy initialization
 * ═══════════════════════════════════════════════════════════════════ */

/* Lazy initialization: loads real bass.dll on first BASS proxy call.
 * This runs synchronously during the game's BASS_Init (App::Initialize),
 * so the real DLL is available before any audio operations. */
static void lazy_load_real_bass(void)
{
    if (g_hRealBass) return;  /* already loaded */

    char path[MAX_PATH];

    /* Try game directory first (bass_real.dll — user renames original) */
    GetModuleFileNameA(NULL, path, MAX_PATH);
    char *slash = strrchr(path, '\\');
    if (slash) {
        strcpy(slash + 1, "bass_real.dll");
        g_hRealBass = LoadLibraryA(path);
    }

    /* Fallback: system directory */
    if (!g_hRealBass) {
        GetSystemDirectoryA(path, MAX_PATH);
        lstrcatA(path, "\\bass.dll");
        g_hRealBass = LoadLibraryA(path);
    }

    if (!g_hRealBass) return;

    real_BASS_ChannelSetAttributes = (BASS_ChannelSetAttributes_t)
        GetProcAddress(g_hRealBass, "BASS_ChannelSetAttributes");
    real_BASS_MusicPlayEx = (BASS_MusicPlayEx_t)
        GetProcAddress(g_hRealBass, "BASS_MusicPlayEx");
    real_BASS_MusicLoad = (BASS_MusicLoad_t)
        GetProcAddress(g_hRealBass, "BASS_MusicLoad");
    real_BASS_SampleLoad = (BASS_SampleLoad_t)
        GetProcAddress(g_hRealBass, "BASS_SampleLoad");
    real_BASS_StreamCreateFile = (BASS_StreamCreateFile_t)
        GetProcAddress(g_hRealBass, "BASS_StreamCreateFile");
    real_BASS_SampleGetChannel = (BASS_SampleGetChannel_t)
        GetProcAddress(g_hRealBass, "BASS_SampleGetChannel");
    real_BASS_ChannelPlay = (BASS_ChannelPlay_t)
        GetProcAddress(g_hRealBass, "BASS_ChannelPlay");
    real_BASS_ChannelStop = (BASS_ChannelStop_t)
        GetProcAddress(g_hRealBass, "BASS_ChannelStop");
    real_BASS_Init = (BASS_Init_t)
        GetProcAddress(g_hRealBass, "BASS_Init");
    real_BASS_Free = (BASS_Free_t)
        GetProcAddress(g_hRealBass, "BASS_Free");
    real_BASS_Stop = (BASS_Stop_t)
        GetProcAddress(g_hRealBass, "BASS_Stop");
    real_BASS_Start = (BASS_Start_t)
        GetProcAddress(g_hRealBass, "BASS_Start");
    real_BASS_SetConfig = (BASS_SetConfig_t)
        GetProcAddress(g_hRealBass, "BASS_SetConfig");
    real_BASS_ErrorGetCode = (BASS_ErrorGetCode_t)
        GetProcAddress(g_hRealBass, "BASS_ErrorGetCode");
}

static DWORD WINAPI patch_thread(LPVOID param)
{
    (void)param;
    /* Wait for game to finish loading */
    Sleep(2000);

    install_lifter_hook();

    return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    (void)hinstDLL;
    (void)lpvReserved;

    if (fdwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hinstDLL);
        CreateThread(NULL, 0, patch_thread, NULL, 0, NULL);
    }
    return TRUE;
}
