/*
 * power_ball.c — Standalone BASS.dll proxy mod for Hamsterball
 *
 * Power Ball: makes the player ball stronger than 8-Ball and Fun Ball.
 * Increases player radius to 55.0 (vs 8-ball 35.0, fun ball 26.0)
 * and max speed to 10.0 (default 6.0).
 *
 * Knockout formula: if otherRadius < myRadius * 0.7 → knockout
 *   Player 55.0 × 0.7 = 38.5 > 35.0 (8-ball)  → can KO 8-ball
 *   8-ball 35.0 × 0.7 = 24.5 < 55.0 (player) → cannot KO player
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll power_ball.c \
 *     -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias
 *
 * Install:
 *   1. Rename original bass.dll → bass_real.dll
 *   2. Copy this bass.dll into the game folder
 *   3. Launch the game!
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winuser.h>
#include <mmsystem.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * BASS Proxy Exports — forward to bass_real.dll for real audio
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
            if (p) {
                strcpy(p + 1, "bass_real.dll");
                g_hRealBass = LoadLibraryA(path);
            }
        }
    }
}

__declspec(dllexport) int __stdcall BASS_Init(int a, DWORD b, DWORD c, HWND d, void* e) {
    if (g_hRealBass) { typedef int (__stdcall *fn_t)(int,DWORD,DWORD,HWND,void*); fn_t fn=(fn_t)GetProcAddress(g_hRealBass,"BASS_Init"); if(fn) return fn(a,b,c,d,e); }
    return 1;
}
__declspec(dllexport) int __stdcall BASS_Free(void) {
    if (g_hRealBass) { typedef int (__stdcall *fn_t)(void); fn_t fn=(fn_t)GetProcAddress(g_hRealBass,"BASS_Free"); if(fn) return fn(); }
    return 1;
}
__declspec(dllexport) int __stdcall BASS_Start(void) {
    if (g_hRealBass) { typedef int (__stdcall *fn_t)(void); fn_t fn=(fn_t)GetProcAddress(g_hRealBass,"BASS_Start"); if(fn) return fn(); }
    return 1;
}
__declspec(dllexport) int __stdcall BASS_Stop(void) {
    if (g_hRealBass) { typedef int (__stdcall *fn_t)(void); fn_t fn=(fn_t)GetProcAddress(g_hRealBass,"BASS_Stop"); if(fn) return fn(); }
    return 1;
}
__declspec(dllexport) int __stdcall BASS_SetConfig(DWORD a, DWORD b) {
    if (g_hRealBass) { typedef int (__stdcall *fn_t)(DWORD,DWORD); fn_t fn=(fn_t)GetProcAddress(g_hRealBass,"BASS_SetConfig"); if(fn) return fn(a,b); }
    return 1;
}
__declspec(dllexport) int __stdcall BASS_ErrorGetCode(void) {
    if (g_hRealBass) { typedef int (__stdcall *fn_t)(void); fn_t fn=(fn_t)GetProcAddress(g_hRealBass,"BASS_ErrorGetCode"); if(fn) return fn(); }
    return 0;
}
__declspec(dllexport) DWORD __stdcall BASS_MusicLoad(int a, void* b, DWORD c, DWORD d, DWORD e, DWORD f) {
    if (g_hRealBass) { typedef DWORD (__stdcall *fn_t)(int,void*,DWORD,DWORD,DWORD,DWORD); fn_t fn=(fn_t)GetProcAddress(g_hRealBass,"BASS_MusicLoad"); if(fn) return fn(a,b,c,d,e,f); }
    return 0;
}
__declspec(dllexport) int __stdcall BASS_MusicPlayEx(DWORD a, DWORD b, BOOL c) {
    if (g_hRealBass) { typedef int (__stdcall *fn_t)(DWORD,DWORD,BOOL); fn_t fn=(fn_t)GetProcAddress(g_hRealBass,"BASS_MusicPlayEx"); if(fn) return fn(a,b,c); }
    return 1;
}
__declspec(dllexport) int __stdcall BASS_ChannelSetAttributes(DWORD a, float b, int c, int d) {
    if (g_hRealBass) { typedef int (__stdcall *fn_t)(DWORD,float,int,int); fn_t fn=(fn_t)GetProcAddress(g_hRealBass,"BASS_ChannelSetAttributes"); if(fn) return fn(a,b,c,d); }
    return 1;
}
__declspec(dllexport) int __stdcall BASS_ChannelStop(DWORD a) {
    if (g_hRealBass) { typedef int (__stdcall *fn_t)(DWORD); fn_t fn=(fn_t)GetProcAddress(g_hRealBass,"BASS_ChannelStop"); if(fn) return fn(a); }
    return 1;
}
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
 * Power Ball Mod — background thread writes ball physics every 16ms
 *
 * Ball struct offsets (verified from Ghidra decompilation):
 *   +0x018  playerID   (int, -1 = badball, 0-3 = player)
 *   +0x188  max_speed  (float, default 6.0)
 *   +0x284  radius     (float, default 26.0, 8-ball = 35.0)
 *
 * Player ball is found via: App(0x5341E0) → +0x220 (PlayerProfile*) → +0x0C (Board*)
 *   → Scene+0x29D4 (AthenaList/ball_list) → first entry
 * But simplest reliable path on Wine: scan App→profile→board→scene→ball_list
 * ═══════════════════════════════════════════════════════════════════════════ */

#define APP_ADDR     0x5341E0
#define BALL_VTABLE  0x4CF3A0

#define POWER_RADIUS  55.0f
#define POWER_SPEED   10.0f

static int g_initialized = 0;
static DWORD g_last_check = 0;

static void apply_power_ball(void)
{
    DWORD now = GetTickCount();
    if (now - g_last_check < 50) return;  /* check every 50ms */
    g_last_check = now;

    DWORD* pApp = (DWORD*)APP_ADDR;
    if (IsBadReadPtr(pApp, 4)) return;

    DWORD app = *pApp;
    if (!app || app < 0x10000) return;
    if (IsBadReadPtr((void*)app, 0x1000)) return;

    /* App → +0x220 → PlayerProfile* */
    if (IsBadReadPtr((void*)(app + 0x220), 4)) return;
    DWORD profile = *(DWORD*)(app + 0x220);
    if (!profile || profile < 0x10000) return;

    /* PlayerProfile → +0x0C → Board* */
    if (IsBadReadPtr((void*)(profile + 0x0C), 4)) return;
    DWORD board = *(DWORD*)(profile + 0x0C);
    if (!board || board < 0x10000) return;
    if (IsBadReadPtr((void*)board, 4)) return;

    /* Board → +0x878 → Scene* (board+0x878 = scene ptr, verified) */
    if (IsBadReadPtr((void*)(board + 0x878), 4)) return;
    DWORD scene = *(DWORD*)(board + 0x878);
    if (!scene || scene < 0x10000) return;
    if (IsBadReadPtr((void*)scene, 4)) return;

    /* Scene → +0x29D4 → AthenaList (ball list)
     * AthenaList: +0x00 = vtable, +0x04 = count, +0x08 = array ptr
     * Actually: Scene+0x29D4 is the AthenaList itself (not a pointer to it)
     * The list has: [vtable][count][array_ptr] at offsets 0x00, 0x04, 0x08
     * But the game uses Scene+0x29D0 = current_ball_ptr (direct ball pointer)
     * Let's use the simpler direct path: Scene+0x29D0 */
    if (IsBadReadPtr((void*)(scene + 0x29D0), 4)) return;
    DWORD ball = *(DWORD*)(scene + 0x29D0);
    if (!ball || ball < 0x10000) return;
    if (IsBadReadPtr((void*)ball, 0x300)) return;

    /* Verify ball vtable to avoid garbage */
    DWORD vtable = *(DWORD*)ball;
    /* Ball vtable is at 0x4CF3A0 — but game uses different vtables for
     * different ball types. Just check it's in the .rdata range */
    if (vtable < 0x4C0000 || vtable > 0x4E0000) return;

    /* Check playerID: 0-3 = player, -1 = badball */
    int playerID = *(int*)(ball + 0x18);
    if (playerID < 0 || playerID > 3) return;

    /* Write radius and max_speed */
    float* radius = (float*)(ball + 0x284);
    float* max_speed = (float*)(ball + 0x188);

    if (IsBadWritePtr(radius, 4) || IsBadWritePtr(max_speed, 4)) return;

    *radius = POWER_RADIUS;
    *max_speed = POWER_SPEED;
}

static DWORD WINAPI power_ball_thread(LPVOID param)
{
    /* Wait for game to fully load */
    Sleep(3000);

    while (1) {
        apply_power_ball();
        Sleep(16);
    }
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DLL Entry
 * ═══════════════════════════════════════════════════════════════════════════ */

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hinstDLL);
        load_real_bass();
        if (!g_initialized) {
            g_initialized = 1;
            CreateThread(NULL, 0, power_ball_thread, NULL, 0, NULL);
        }
        break;
    }
    return TRUE;
}
