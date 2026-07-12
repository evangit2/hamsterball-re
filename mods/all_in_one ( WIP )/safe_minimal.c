/*
 * safe_minimal.c — Minimal BASS.dll proxy for Hamsterball
 *
 * Only the safest mod: No Pause (3 byte patches, no hooks, no memory allocation)
 * Audio stubs return safe values even if bass_real.dll is missing
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll safe_minimal.c \
 *     -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * BASS Proxy Exports — forward to bass_real.dll if available,
 * otherwise return safe stub values (won't crash the game)
 * ═══════════════════════════════════════════════════════════════════════════ */

static HMODULE g_hRealBass = NULL;

/* Helper: try to load real bass */
static void load_real_bass(void)
{
    /* Try direct load */
    g_hRealBass = LoadLibraryA("bass_real.dll");
    
    /* Try same directory as our DLL */
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

/* Core BASS functions — forward if available, stub if not */
__declspec(dllexport) int __stdcall BASS_Init(int a, DWORD b, DWORD c, HWND d, void* e) {
    if (g_hRealBass) {
        typedef int (__stdcall *fn_t)(int, DWORD, DWORD, HWND, void*);
        fn_t fn = (fn_t)GetProcAddress(g_hRealBass, "BASS_Init");
        if (fn) return fn(a, b, c, d, e);
    }
    return 1;  /* Pretend success */
}

__declspec(dllexport) int __stdcall BASS_Free(void) {
    if (g_hRealBass) {
        typedef int (__stdcall *fn_t)(void);
        fn_t fn = (fn_t)GetProcAddress(g_hRealBass, "BASS_Free");
        if (fn) return fn();
    }
    return 1;
}

__declspec(dllexport) int __stdcall BASS_Start(void) {
    if (g_hRealBass) {
        typedef int (__stdcall *fn_t)(void);
        fn_t fn = (fn_t)GetProcAddress(g_hRealBass, "BASS_Start");
        if (fn) return fn();
    }
    return 1;
}

__declspec(dllexport) int __stdcall BASS_Stop(void) {
    if (g_hRealBass) {
        typedef int (__stdcall *fn_t)(void);
        fn_t fn = (fn_t)GetProcAddress(g_hRealBass, "BASS_Stop");
        if (fn) return fn();
    }
    return 1;
}

__declspec(dllexport) int __stdcall BASS_SetConfig(DWORD a, DWORD b) {
    if (g_hRealBass) {
        typedef int (__stdcall *fn_t)(DWORD, DWORD);
        fn_t fn = (fn_t)GetProcAddress(g_hRealBass, "BASS_SetConfig");
        if (fn) return fn(a, b);
    }
    return 1;
}

__declspec(dllexport) int __stdcall BASS_ErrorGetCode(void) {
    if (g_hRealBass) {
        typedef int (__stdcall *fn_t)(void);
        fn_t fn = (fn_t)GetProcAddress(g_hRealBass, "BASS_ErrorGetCode");
        if (fn) return fn();
    }
    return 0;
}

__declspec(dllexport) DWORD __stdcall BASS_MusicLoad(int a, void* b, DWORD c, DWORD d, DWORD e, DWORD f) {
    if (g_hRealBass) {
        typedef DWORD (__stdcall *fn_t)(int, void*, DWORD, DWORD, DWORD, DWORD);
        fn_t fn = (fn_t)GetProcAddress(g_hRealBass, "BASS_MusicLoad");
        if (fn) return fn(a, b, c, d, e, f);
    }
    return 0;  /* No music handle — game should handle gracefully */
}

__declspec(dllexport) int __stdcall BASS_MusicPlayEx(DWORD a, DWORD b, BOOL c) {
    if (g_hRealBass) {
        typedef int (__stdcall *fn_t)(DWORD, DWORD, BOOL);
        fn_t fn = (fn_t)GetProcAddress(g_hRealBass, "BASS_MusicPlayEx");
        if (fn) return fn(a, b, c);
    }
    return 1;
}

__declspec(dllexport) int __stdcall BASS_ChannelSetAttributes(DWORD a, float b, int c, int d) {
    if (g_hRealBass) {
        typedef int (__stdcall *fn_t)(DWORD, float, int, int);
        fn_t fn = (fn_t)GetProcAddress(g_hRealBass, "BASS_ChannelSetAttributes");
        if (fn) return fn(a, b, c, d);
    }
    return 1;
}

__declspec(dllexport) int __stdcall BASS_ChannelStop(DWORD a) {
    if (g_hRealBass) {
        typedef int (__stdcall *fn_t)(DWORD);
        fn_t fn = (fn_t)GetProcAddress(g_hRealBass, "BASS_ChannelStop");
        if (fn) return fn(a);
    }
    return 1;
}

/* Extra stubs — all safe no-ops */
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
 * No Pause Mod — 3 safe byte patches, no hooks, no memory allocation
 * ═══════════════════════════════════════════════════════════════════════════ */

static void apply_no_pause(void)
{
    HMODULE hExe = GetModuleHandleA(NULL);
    if (!hExe) return;
    BYTE *base = (BYTE*)hExe;
    DWORD oldProt;

    /* Path 1: DirectInput ESC — base+0x19d5b: JZ(74)→JMP(EB) */
    {
        BYTE *addr = base + 0x19d5b;
        if (VirtualProtect(addr, 1, PAGE_READWRITE, &oldProt)) {
            *addr = 0xEB;
            VirtualProtect(addr, 1, oldProt, &oldProt);
        }
    }
    /* Path 2: Right-click — base+0x130b5: JZ(74)→JMP(EB) */
    {
        BYTE *addr = base + 0x130b5;
        if (VirtualProtect(addr, 1, PAGE_READWRITE, &oldProt)) {
            *addr = 0xEB;
            VirtualProtect(addr, 1, oldProt, &oldProt);
        }
    }
    /* Path 3: Win32 ESC — base+0x0b405: JNZ(75)→JMP(EB) */
    {
        BYTE *addr = base + 0x0b405;
        if (VirtualProtect(addr, 1, PAGE_READWRITE, &oldProt)) {
            *addr = 0xEB;
            VirtualProtect(addr, 1, oldProt, &oldProt);
        }
    }
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
        load_real_bass();       /* Try to load real audio (OK if missing) */
        apply_no_pause();       /* 3 byte patches — instant, no threads */
        break;
    }
    return TRUE;
}
