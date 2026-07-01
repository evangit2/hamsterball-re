/*
 * neon_arena_lightfix — Fix missing yellow diffuse light on Neon Arena.
 *
 * Root cause: ArenaLevel_Neon_Init (0x416F40) loop 1 writes yellow material
 * colors (ambient/diffuse/emissive) to P1 balls but FORGETS to set the glow
 * render flag (+0xC80 = 1). Loop 2 (P2/badballs) correctly sets +0xC80 = 1.
 * Without this flag, the emissive material is written but never rendered.
 *
 * Fix: A background thread detects Neon Arena (board+0x47E4 non-zero) and
 * sets +0xC80 = 1 on every ball in the P1 AthenaList (board+0x29D4).
 * No material writes needed — the game already wrote them.
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll neon_arena_lightfix.c \
 *     -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * BASS Proxy Exports (all 10 game imports + extras)
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
 * Neon Arena Light Fix — minimal: only set glow flag
 * ═══════════════════════════════════════════════════════════════════════════ */

#define APP_PTR_ADDR    0x005341E0
#define BALL_VTABLE     0x004CF3A0

static volatile int g_running = 1;

static DWORD WINAPI neon_fix_thread(LPVOID param) {
    (void)param;
    Sleep(3000);

    while (g_running) {
        char *app = *(char**)APP_PTR_ADDR;
        if (!app || IsBadReadPtr(app, 0x900)) { Sleep(1000); continue; }

        /* App → +0x220 (PlayerProfile*) → +0xC (Board*) */
        int *profile = *(int**)(app + 0x220);
        if (!profile || IsBadReadPtr(profile, 0x10)) { Sleep(1000); continue; }
        char *board = *(char**)((char*)profile + 0x0C);
        if (!board || IsBadReadPtr(board, 0x4800)) { Sleep(1000); continue; }

        /* Check board+0x47E4 (Neon Arena emitter SceneObject) */
        int emitter = *(int*)(board + 0x47E4);
        if (!emitter) { Sleep(1000); continue; }

        /* Neon Arena detected — set glow flag on P1 balls */
        char *ball_list = board + 0x29D4;
        if (!IsBadReadPtr(ball_list, 0x410)) {
            int count = *(int*)(ball_list + 0x04);
            if (count > 0 && count < 100) {
                int *array = *(int**)(ball_list + 0x40C);
                if (array && !IsBadReadPtr(array, count * 4)) {
                    for (int i = 0; i < count; i++) {
                        char *ball = (char*)array[i];
                        if (ball && !IsBadReadPtr(ball, 0xC90)) {
                            /* Verify it's a ball by vtable */
                            if (*(int*)ball == BALL_VTABLE) {
                                /* THE fix: set glow flag.
                                 * Game already wrote yellow materials in loop 1,
                                 * but forgot this flag. */
                                *(BYTE*)(ball + 0xC80) = 1;
                            }
                        }
                    }
                }
            }
        }

        /* Also set on P2/badballs list (board+0x2DEC) for completeness */
        char *ball_list2 = board + 0x2DEC;
        if (!IsBadReadPtr(ball_list2, 0x410)) {
            int count2 = *(int*)(ball_list2 + 0x04);
            if (count2 > 0 && count2 < 100) {
                int *array2 = *(int**)(ball_list2 + 0x40C);
                if (array2 && !IsBadReadPtr(array2, count2 * 4)) {
                    for (int i = 0; i < count2; i++) {
                        char *ball = (char*)array2[i];
                        if (ball && !IsBadReadPtr(ball, 0xC90)) {
                            if (*(int*)ball == BALL_VTABLE) {
                                *(BYTE*)(ball + 0xC80) = 1;
                            }
                        }
                    }
                }
            }
        }

        Sleep(2000);
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
