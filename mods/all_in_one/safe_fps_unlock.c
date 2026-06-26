/*
 * safe_fps_unlock.c — Safe BASS.dll proxy: No Pause + FPS Unlock (NO IAT hook)
 *
 * The original fps_unlock_v9 used a GetTickCount IAT hook for high-resolution
 * timing. That hook patches the Import Address Table, which can crash on
 * Wine/Android emulators. This version SKIPS the IAT hook entirely and only
 * uses safe byte patches:
 *   - NOP the render-skip jbe (always render every frame)
 *   - Vsync: INTERVAL_ONE → INTERVAL_IMMEDIATE (2 locations)
 *   - timeBeginPeriod(1) for high-res timer
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll safe_fps_unlock.c \
 *     -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winuser.h>
#include <mmsystem.h>
#include <string.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * BASS Proxy Exports — forward to bass_real.dll if available, stub otherwise
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
 * MOD 1: No Pause (3 single-byte patches — proven safe on Android)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void apply_no_pause(BYTE *base)
{
    DWORD oldProt;
    /* Path 1: DirectInput ESC */
    {
        BYTE *addr = base + 0x19d5b;
        if (VirtualProtect(addr, 1, PAGE_READWRITE, &oldProt)) {
            *addr = 0xEB;  /* JZ → JMP */
            VirtualProtect(addr, 1, oldProt, &oldProt);
        }
    }
    /* Path 2: Right-click */
    {
        BYTE *addr = base + 0x130b5;
        if (VirtualProtect(addr, 1, PAGE_READWRITE, &oldProt)) {
            *addr = 0xEB;
            VirtualProtect(addr, 1, oldProt, &oldProt);
        }
    }
    /* Path 3: Win32 message pump ESC */
    {
        BYTE *addr = base + 0x0b405;
        if (VirtualProtect(addr, 1, PAGE_READWRITE, &oldProt)) {
            *addr = 0xEB;
            VirtualProtect(addr, 1, oldProt, &oldProt);
        }
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * MOD 2: FPS Unlock (NO GetTickCount IAT hook — safe for Wine/Android)
 *
 * Only uses safe byte patches:
 *   1. NOP render-skip jbe at 0x46BF55 (always render every frame)
 *   2. Vsync INTERVAL_ONE → INTERVAL_IMMEDIATE (2 pattern matches)
 *   3. timeBeginPeriod(1) for high-res timer
 *
 * The GetTickCount IAT hook from v9 is REMOVED — it was likely causing
 * crashes on Android emulators by patching the import table.
 * ═══════════════════════════════════════════════════════════════════════════ */

#define IMAGE_BASE 0x00400000

/* Patch at exact VA: verify expected bytes, then overwrite */
static int patch_at(DWORD va, const BYTE *expected, SIZE_T len, const BYTE *replacement)
{
    BYTE *addr = (BYTE *)va;
    DWORD oldProtect;
    SIZE_T i;
    for (i = 0; i < len; i++) {
        if (addr[i] != expected[i]) return 0;
    }
    if (!VirtualProtect(addr, len, PAGE_EXECUTE_READWRITE, &oldProtect)) return 0;
    memcpy(addr, replacement, len);
    VirtualProtect(addr, len, oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), addr, len);
    return 1;
}

/* Pattern match + replace in .text section */
static int patch_pattern(BYTE *base, SIZE_T size, const BYTE *pattern, SIZE_T pat_len, const BYTE *replacement)
{
    int count = 0;
    SIZE_T i, j;
    for (i = 0; i + pat_len <= size; i++) {
        for (j = 0; j < pat_len; j++) {
            if (base[i + j] != pattern[j]) break;
        }
        if (j == pat_len) {
            DWORD oldProtect;
            if (VirtualProtect(base + i, pat_len, PAGE_EXECUTE_READWRITE, &oldProtect)) {
                memcpy(base + i, replacement, pat_len);
                VirtualProtect(base + i, pat_len, oldProtect, &oldProtect);
                FlushInstructionCache(GetCurrentProcess(), base + i, pat_len);
                count++;
            }
        }
    }
    return count;
}

/* Vsync pattern: INTERVAL_ONE → INTERVAL_IMMEDIATE (found at 2 locations) */
static const BYTE vsync_pattern[] = {
    0xC7, 0x86, 0xF8, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00
};
static const BYTE vsync_replacement[] = {
    0xC7, 0x86, 0xF8, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80
};

static int apply_fps_unlock(void)
{
    int results = 0;

    /* 1. NOP render-skip jbe at VA 0x46BF55
     *    Original: 76 5D (jbe skip_render — skips Present() if too soon)
     *    Patched:  90 90 (nop nop — always render, never skip) */
    {
        BYTE expected[] = { 0x76, 0x5D };
        BYTE replacement[] = { 0x90, 0x90 };
        results += patch_at(0x46BF55, expected, 2, replacement);
    }

    /* 2. Vsync: INTERVAL_ONE → INTERVAL_IMMEDIATE (2 locations) */
    {
        BYTE *text_start = (BYTE *)IMAGE_BASE + 0x1000;
        SIZE_T text_size = 0xCE000;
        results += patch_pattern(text_start, text_size,
            vsync_pattern, sizeof(vsync_pattern), vsync_replacement);
    }

    /* 3. High-res timer (safe Win32 API call, no patching) */
    timeBeginPeriod(1);
    results++;

    return results;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Patch Thread — waits for game init, then applies FPS patches
 * (No Pause applied directly in DllMain — proven safe)
 * ═══════════════════════════════════════════════════════════════════════════ */

static DWORD WINAPI fps_patch_thread(LPVOID param)
{
    (void)param;
    /* Wait for game to finish loading (it patches its own code during init) */
    Sleep(500);
    apply_fps_unlock();
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
        load_real_bass();       /* Try to load real audio (OK if missing) */

        /* No Pause — apply directly (proven safe, no timing needed) */
        {
            HMODULE hExe = GetModuleHandleA(NULL);
            if (hExe) {
                apply_no_pause((BYTE*)hExe);
            }
        }

        /* FPS Unlock — needs 500ms delay for game init */
        CreateThread(NULL, 0, fps_patch_thread, NULL, 0, NULL);
        break;

    case DLL_PROCESS_DETACH:
        timeEndPeriod(1);
        break;
    }
    return TRUE;
}
