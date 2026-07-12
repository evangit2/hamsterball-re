/*
 * neon_arena_lightfix — Fix missing yellow diffuse light on Neon Arena.
 *
 * Root cause: ArenaLevel_Neon_Init (0x416F40) creates a yellow D3D emitter
 * light (Vec3 R=10, G=10, B=0) but places it at (0,0,0) — world origin —
 * and never moves it to follow the ball. In Neon Race, Scene_SetupLevelDark
 * positions the emitter at the ball's location (ball.x, ball.y+30, ball.z).
 * Additionally, loop 1 forgets to set glow flag (+0xC80=1) on P1 balls, and
 * 8-balls spawn dynamically during gameplay so they never get the flag.
 *
 * Fix (two parts):
 * 1. Background thread sets +0xC80=1 on all balls in both AthenaLists.
 * 2. Background thread continuously moves the emitter (board+0x47E4) to
 *    follow the P1 ball: writes ball position to emitter+0x08/+0x0C/+0x10.
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll neon_arena_lightfix.c \
 *     -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

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
typedef int  (__stdcall *BASS_Init_t)(int, DWORD, DWORD, HWND, void*);
static BASS_Init_t real_BASS_Init = NULL;
__declspec(dllexport) int __stdcall BASS_Init(int a, DWORD b, DWORD c, HWND d, void* e) {
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
__declspec(dllexport) DWORD __stdcall BASS_StreamCreateFile(void *a, void *b, DWORD c, DWORD d, DWORD e) { return 0; }
__declspec(dllexport) DWORD __stdcall BASS_SampleLoad(int a, void *b, DWORD c, DWORD d, DWORD e) { return 0; }
__declspec(dllexport) int __stdcall BASS_ChannelPlay(DWORD a, BOOL b) { return 1; }
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
        GetModuleFileNameA(hSelf, path, MAX_PATH);
        char *p = strrchr(path, '\\');
        if (p) {
            strcpy(p + 1, "bass_real.dll");
            g_hRealBass = LoadLibraryA(path);
        }
    }
    if (g_hRealBass) {
        #define LOAD(name) real_##name = (name##_t)GetProcAddress(g_hRealBass, #name)
        LOAD(BASS_ChannelSetAttributes);
        LOAD(BASS_MusicPlayEx);
        LOAD(BASS_SetConfig);
        LOAD(BASS_Init);
        LOAD(BASS_Free);
        LOAD(BASS_Start);
        LOAD(BASS_Stop);
        LOAD(BASS_ErrorGetCode);
        LOAD(BASS_MusicLoad);
        LOAD(BASS_ChannelStop);
        #undef LOAD
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Neon Arena Light Fix
 * ═══════════════════════════════════════════════════════════════════════════ */

#define APP_PTR_ADDR    0x005341E0
#define BALL_VTABLE     0x004CF3A0

/* Ball struct: glow flag at +0xC80 (byte, 1=render glow) */
#define BALL_CHECK_SIZE  0xC90

/* Ball position: +0x164 (X), +0x168 (Y), +0x16C (Z) */
/* Emitter position: +0x08 (X), +0x0C (Y), +0x10 (Z) */
/* Neon Race uses Y+30 offset (from _DAT_004cf528 = 30.0f float) */

/* SceneObject vtable[3] (offset +0x0C in vtable) re-submits the D3D light
 * to the device with current position. Must be called after position changes
 * or the light stays at its initial position forever. */
static void update_emitter_light(void *emitter) {
    if (!emitter || IsBadReadPtr(emitter, 0x100)) return;
    int vtable = *(int*)emitter;
    if (!vtable || IsBadReadPtr((void*)vtable, 0x10)) return;
    int func = *(int*)(vtable + 0x0C);
    if (!func || IsBadReadPtr((void*)func, 1)) return;
    /* Call func with ECX = emitter (__fastcall) */
    __asm__ volatile (
        "call *%0"
        :
        : "r"(func), "c"(emitter)
        : "eax", "edx", "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)", "memory"
    );
}

static volatile int g_running = 1;

/* Set glow flag on a single ball */
static void set_glow_flag(char *ball) {
    if (!ball || IsBadReadPtr(ball, BALL_CHECK_SIZE)) return;
    if (*(int*)ball != BALL_VTABLE) return;
    *(BYTE*)(ball + 0xC80) = 1;
}

/* Iterate an AthenaList and set glow flag on each ball */
static void apply_glow_to_list(char *list) {
    if (!list || IsBadReadPtr(list, 0x410)) return;
    int count = *(int*)(list + 0x04);
    if (count <= 0 || count > 100) return;
    int *array = *(int**)(list + 0x40C);
    if (!array || IsBadReadPtr(array, count * 4)) return;
    for (int i = 0; i < count; i++)
        set_glow_flag((char*)array[i]);
}

/* Get first ball from P1 AthenaList (board+0x29D4) */
static char* get_p1_ball(char *board) {
    char *list = board + 0x29D4;
    if (IsBadReadPtr(list, 0x410)) return NULL;
    int count = *(int*)(list + 0x04);
    if (count <= 0 || count > 100) return NULL;
    int *array = *(int**)(list + 0x40C);
    if (!array || IsBadReadPtr(array, count * 4)) return NULL;
    char *ball = (char*)array[0];
    if (!ball || IsBadReadPtr(ball, 0x200) || *(int*)ball != BALL_VTABLE)
        return NULL;
    return ball;
}

static DWORD WINAPI neon_fix_thread(LPVOID param) {
    (void)param;
    Sleep(3000);

    while (g_running) {
        char *app = *(char**)APP_PTR_ADDR;
        if (!app || IsBadReadPtr(app, 0x900)) { Sleep(1000); continue; }

        int *profile = *(int**)(app + 0x220);
        if (!profile || IsBadReadPtr(profile, 0x10)) { Sleep(1000); continue; }
        char *board = *(char**)((char*)profile + 0x0C);
        if (!board || IsBadReadPtr(board, 0x4800)) { Sleep(1000); continue; }

        /* Neon Arena detection: board+0x47E4 = emitter SceneObject pointer */
        int emitter = *(int*)(board + 0x47E4);
        if (!emitter) { Sleep(1000); continue; }

        /* 1. Set glow flags on all balls */
        apply_glow_to_list(board + 0x29D4);
        apply_glow_to_list(board + 0x2DEC);

        /* 2. Move emitter to follow P1 ball and re-submit D3D light */
        if (!IsBadReadPtr((void*)emitter, 0x100)) {
            char *ball = get_p1_ball(board);
            if (ball) {
                float bx = *(float*)(ball + 0x164);
                float by = *(float*)(ball + 0x168);
                float bz = *(float*)(ball + 0x16C);
                *(float*)(emitter + 0x08) = bx;
                *(float*)(emitter + 0x0C) = by + 30.0f;
                *(float*)(emitter + 0x10) = bz;
                /* Re-call vtable[3] to submit updated light to D3D device */
                update_emitter_light((void*)emitter);
            }
        }

        Sleep(100);
    }
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DLL Entry Point
 * ═══════════════════════════════════════════════════════════════════════════ */

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
    (void)hModule;
    (void)lpReserved;
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        load_real_bass();
        CreateThread(NULL, 0, neon_fix_thread, NULL, 0, NULL);
        break;
    case DLL_PROCESS_DETACH:
        g_running = 0;
        break;
    }
    return TRUE;
}
