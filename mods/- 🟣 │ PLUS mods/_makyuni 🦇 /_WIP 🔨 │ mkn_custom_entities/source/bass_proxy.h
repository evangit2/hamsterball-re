/*
 * bass_proxy.h — Shared BASS.dll proxy boilerplate for Hamsterball mods.
 *
 * Include this in every bass.dll proxy mod to avoid 120 lines of repetition.
 * The game imports exactly 10 BASS functions; this header provides stubs
 * that forward to bass_real.dll (renamed original) if available.
 *
 * Usage:
 *   #include "bass_proxy.h"
 *   // ... your mod code ...
 *   BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
 *       if (reason == DLL_PROCESS_ATTACH) { load_real_bass(); your_init(); }
 *       return TRUE;
 *   }
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll yourmod.c -lwinmm \
 *     -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias -msse2 -mfpmath=sse
 */

#ifndef BASS_PROXY_H
#define BASS_PROXY_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winuser.h>
#include <mmsystem.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * BASS Proxy Exports — forward all game imports to bass_real.dll
 * ═══════════════════════════════════════════════════════════════════════════ */

static HMODULE g_hRealBass = NULL;

typedef int  (__stdcall *BASS_ChannelSetAttributes_t)(DWORD, DWORD, int, int);
static BASS_ChannelSetAttributes_t real_BASS_ChannelSetAttributes = NULL;
__declspec(dllexport) int __stdcall BASS_ChannelSetAttributes(DWORD a, DWORD b, int c, int d) {
    if (real_BASS_ChannelSetAttributes) return real_BASS_ChannelSetAttributes(a, b, c, d);
    return 1;
}

typedef int  (__stdcall *BASS_MusicPlayEx_t)(DWORD, DWORD, int, DWORD);
static BASS_MusicPlayEx_t real_BASS_MusicPlayEx = NULL;
__declspec(dllexport) int __stdcall BASS_MusicPlayEx(DWORD a, DWORD b, int c, DWORD d) {
    if (real_BASS_MusicPlayEx) return real_BASS_MusicPlayEx(a, b, c, d);
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

/* Extra stubs (not imported by game, but safe to have) */
__declspec(dllexport) void __stdcall BASS_Pause(void) {}
__declspec(dllexport) void __stdcall BASS_SetVolume(DWORD a) {}
__declspec(dllexport) DWORD __stdcall BASS_GetVolume(void) { return 0; }
__declspec(dllexport) int __stdcall BASS_GetDevice(void) { return 0; }
__declspec(dllexport) int __stdcall BASS_SetDevice(DWORD a) { return 1; }
__declspec(dllexport) void __stdcall BASS_GetInfo(void *a) {}
__declspec(dllexport) int __stdcall BASS_Update(DWORD a) { return 0; }
typedef DWORD (__stdcall *BASS_SampleLoad_t)(BOOL mem, const void *file, DWORD offset, DWORD length, DWORD max, DWORD flags);
static BASS_SampleLoad_t real_BASS_SampleLoad = NULL;
__declspec(dllexport) DWORD __stdcall BASS_SampleLoad(BOOL a, const void *b, DWORD c, DWORD d, DWORD e, DWORD f) {
    if (real_BASS_SampleLoad) return real_BASS_SampleLoad(a, b, c, d, e, f);
    return 0;
}

typedef DWORD (__stdcall *BASS_SampleGetChannel_t)(DWORD sample, BOOL onlynew);
static BASS_SampleGetChannel_t real_BASS_SampleGetChannel = NULL;
__declspec(dllexport) DWORD __stdcall BASS_SampleGetChannel(DWORD a, BOOL b) {
    if (real_BASS_SampleGetChannel) return real_BASS_SampleGetChannel(a, b);
    return 0;
}

typedef int (__stdcall *BASS_ChannelPlay_t)(DWORD handle, BOOL restart);
static BASS_ChannelPlay_t real_BASS_ChannelPlay = NULL;
__declspec(dllexport) int __stdcall BASS_ChannelPlay(DWORD a, BOOL b) {
    if (real_BASS_ChannelPlay) return real_BASS_ChannelPlay(a, b);
    return 1;
}

typedef BOOL (__stdcall *BASS_SampleFree_t)(DWORD sample);
static BASS_SampleFree_t real_BASS_SampleFree = NULL;
__declspec(dllexport) BOOL __stdcall BASS_SampleFree(DWORD a) {
    if (real_BASS_SampleFree) return real_BASS_SampleFree(a);
    return TRUE;
}

typedef DWORD (__stdcall *BASS_StreamCreateFile_t)(BOOL mem, const void *file, DWORD offset, DWORD length, DWORD flags);
static BASS_StreamCreateFile_t real_BASS_StreamCreateFile = NULL;
__declspec(dllexport) DWORD __stdcall BASS_StreamCreateFile(BOOL a, const void *b, DWORD c, DWORD d, DWORD e) {
    if (real_BASS_StreamCreateFile) return real_BASS_StreamCreateFile(a, b, c, d, e);
    return 0;
}

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
        LOAD(BASS_SampleLoad);
        LOAD(BASS_SampleGetChannel);
        LOAD(BASS_ChannelPlay);
        LOAD(BASS_SampleFree);
        LOAD(BASS_StreamCreateFile);
        /* Fallback: some BASS dlls export stdcall functions with decorated names
         * (_Name@N) instead of plain names. Try decorated if plain load failed. */
        if (!real_BASS_SampleLoad)
            real_BASS_SampleLoad = (BASS_SampleLoad_t)GetProcAddress(g_hRealBass, "_BASS_SampleLoad@24");
        if (!real_BASS_SampleGetChannel)
            real_BASS_SampleGetChannel = (BASS_SampleGetChannel_t)GetProcAddress(g_hRealBass, "_BASS_SampleGetChannel@8");
        if (!real_BASS_ChannelPlay)
            real_BASS_ChannelPlay = (BASS_ChannelPlay_t)GetProcAddress(g_hRealBass, "_BASS_ChannelPlay@8");
        if (!real_BASS_SampleFree)
            real_BASS_SampleFree = (BASS_SampleFree_t)GetProcAddress(g_hRealBass, "_BASS_SampleFree@4");
        if (!real_BASS_StreamCreateFile)
            real_BASS_StreamCreateFile = (BASS_StreamCreateFile_t)GetProcAddress(g_hRealBass, "_BASS_StreamCreateFile@20");
        #undef LOAD
    }
}

/* Patch a byte at an absolute address */
static void patch_byte(DWORD addr, BYTE val) {
    DWORD old;
    VirtualProtect((void*)addr, 1, PAGE_EXECUTE_READWRITE, &old);
    *(BYTE*)addr = val;
    VirtualProtect((void*)addr, 1, old, &old);
    FlushInstructionCache(GetCurrentProcess(), (void*)addr, 1);
}

/* Patch N bytes at an absolute address */
static void patch_bytes(DWORD addr, const void* data, SIZE_T len) {
    DWORD old;
    VirtualProtect((void*)addr, len, PAGE_EXECUTE_READWRITE, &old);
    memcpy((void*)addr, data, len);
    VirtualProtect((void*)addr, len, old, &old);
    FlushInstructionCache(GetCurrentProcess(), (void*)addr, len);
}

/* Write a DWORD (4 bytes) at an absolute address */
static void patch_dword(DWORD addr, DWORD val) {
    patch_bytes(addr, &val, 4);
}

/* Write a float at an absolute address */
static void patch_float(DWORD addr, float val) {
    patch_bytes(addr, &val, 4);
}

/* Allocate executable memory and return pointer */
static void* alloc_executable(SIZE_T size) {
    return VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
}

/* Install a JMP hook: writes E9 xx xx xx xx at patchAddr to jump to caveAddr */
static void install_jmp_hook(DWORD patchAddr, DWORD caveAddr) {
    DWORD offset = caveAddr - (patchAddr + 5);
    patch_byte(patchAddr, 0xE9);
    patch_dword(patchAddr + 1, offset);
}

/* Install a JMP hook + NOP pad (for 6-byte instructions → 5-byte JMP + 1 NOP) */
static void install_jmp_hook_nop(DWORD patchAddr, DWORD caveAddr, int totalPatchLen) {
    install_jmp_hook(patchAddr, caveAddr);
    for (int i = 5; i < totalPatchLen; i++)
        patch_byte(patchAddr + i, 0x90);
}

/* Game addresses */
#define GAME_BASE 0x00400000
#define GLOBAL_APP_PTR 0x005341E0
#define GLOBAL_SCENE_PTR 0x005341E4
#define BALL_UPDATE_HOOK 0x00405E22  /* mov eax, [esi+0x0C5C] — ESI = ball */
#define GRAPHICS_PRESENT_HOOK 0x00453330  /* Graphics_PresentOrEnd */

/* Common game function typedefs */
typedef void* (__cdecl *operator_new_t)(SIZE_T);
static operator_new_t game_operator_new = (operator_new_t)0x004BA57B;

typedef void (__thiscall *AthenaList_Append_t)(DWORD* list, void* item);
static AthenaList_Append_t AthenaList_Append = (AthenaList_Append_t)0x00453780;

typedef void (__thiscall *AthenaList_Init_t)(DWORD* list);
static AthenaList_Init_t pfn_AthenaList_Init = (AthenaList_Init_t)0x00453210;

/* Safe pointer chain: Try g_Scene first, fall back to App → PlayerProfile → Board */
static DWORD get_board(void) {
    /* Method 1: g_Scene (0x005341E4) holds the board/scene pointer directly.
     * This works in all modes (Time Trial, Tournament, Practice). */
    if (!IsBadReadPtr((void*)GLOBAL_SCENE_PTR, 4)) {
        DWORD board = *(DWORD*)GLOBAL_SCENE_PTR;
        if (board && board > 0x10000 && !IsBadReadPtr((void*)board, 4)) {
            DWORD vtable = *(DWORD*)board;
            if (vtable >= 0x4D0000 && vtable <= 0x4D2200)
                return board;
        }
    }
    /* Method 2: App → profile → board (fallback for older paths) */
    if (IsBadReadPtr((void*)GLOBAL_APP_PTR, 4)) return 0;
    DWORD app = *(DWORD*)GLOBAL_APP_PTR;
    if (!app || app < 0x10000) return 0;
    if (IsBadReadPtr((void*)(app + 0x220), 4)) return 0;
    DWORD profile = *(DWORD*)(app + 0x220);
    if (!profile || profile < 0x10000) return 0;
    if (IsBadReadPtr((void*)(profile + 0x0C), 4)) return 0;
    DWORD board = *(DWORD*)(profile + 0x0C);
    if (!board || board < 0x10000) return 0;
    if (IsBadReadPtr((void*)board, 4)) return 0;
    DWORD vtable = *(DWORD*)board;
    if (vtable < 0x4D0000 || vtable > 0x4D2200) return 0;
    return board;
}

/* Get player 1's ball from the ball list */
static DWORD get_player_ball(void) {
    DWORD board = get_board();
    if (!board) return 0;
    if (IsBadReadPtr((void*)(board + 0x29D4), 4)) return 0;
    DWORD ball_list = *(DWORD*)(board + 0x29D4);
    if (!ball_list || ball_list < 0x10000) return 0;
    if (IsBadReadPtr((void*)(ball_list + 4), 4)) return 0;
    DWORD count = *(DWORD*)(ball_list + 4);
    if (count == 0) return 0;
    if (IsBadReadPtr((void*)(ball_list + 8), 4)) return 0;
    DWORD ball = *(DWORD*)(ball_list + 8);
    if (!ball || ball < 0x10000) return 0;
    if (IsBadReadPtr((void*)ball, 4)) return 0;
    return ball;
}

/* Read ball position (3 floats: X, Y, Z at ball+0x164/0x168/0x16C) */
static void get_ball_pos(DWORD ball, float* x, float* y, float* z) {
    if (!ball || IsBadReadPtr((void*)(ball + 0x164), 12)) {
        *x = *y = *z = 0.0f;
        return;
    }
    *x = *(float*)(ball + 0x164);
    *y = *(float*)(ball + 0x168);
    *z = *(float*)(ball + 0x16C);
}

#endif /* BASS_PROXY_H */
