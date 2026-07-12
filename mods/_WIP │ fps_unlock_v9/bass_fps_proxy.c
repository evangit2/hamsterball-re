/*
 * bass_fps_proxy.c v7 - BASS.dll proxy that uncaps Hamsterball's FPS.
 *
 * Previous approaches failed because:
 *   - Patching fps_divisor in the constructor: useless, App_Run reads the struct
 *     value via idiv before our 500ms-delayed patch fires.
 *   - Patching the idiv instruction in App_Run: also fires too late — the idiv
 *     runs ONCE at function entry before the main loop. By the time we patch,
 *     render_interval is already computed and on the stack.
 *   - Patching fps_target: causes time drift → white screen.
 *   - Short byte patterns (3 bytes): match dozens of locations → corruption.
 *
 * This version patches the RENDER DECISION itself, which runs EVERY FRAME:
 *   - NOP the `jbe skip_render` at VA 0x46BF55 → game always renders, never skips.
 *   Uses ABSOLUTE ADDRESS patching (no pattern matching) for surgical precision.
 *
 * Patches:
 *   1. IAT hook: GetTickCount → QPC-based high-resolution version
 *   2. NOP render-skip jbe at VA 0x46BF55 (always render)
 *   3. vsync: INTERVAL_ONE → INTERVAL_IMMEDIATE (2 locations)
 *   4. timeBeginPeriod(1)
 *
 * Build: i686-w64-mingw32-gcc -shared -o bass.dll bass_fps_proxy.c \
 *          bass_exports.def -lwinmm -Wl,--enable-stdcall-fixup -O2 \
 *          -static -static-libgcc -Wl,--add-stdcall-alias
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winuser.h>
#include <mmsystem.h>
#include <stdio.h>

/* ---- BASS Function Types (__stdcall) ---- */

typedef void (__stdcall *BASS_Stop_t)(void);
typedef int  (__stdcall *BASS_ChannelSetAttributes_t)(DWORD, float, int, int);
typedef int  (__stdcall *BASS_Free_t)(void);
typedef int  (__stdcall *BASS_Init_t)(int, DWORD, DWORD, HWND, void*);
typedef int  (__stdcall *BASS_Start_t)(void);
typedef int  (__stdcall *BASS_SetConfig_t)(DWORD, DWORD);
typedef int  (__stdcall *BASS_ChannelStop_t)(DWORD);
typedef int  (__stdcall *BASS_MusicPlayEx_t)(DWORD, DWORD, BOOL);
typedef int  (__stdcall *BASS_ErrorGetCode_t)(void);
typedef DWORD (__stdcall *BASS_MusicLoad_t)(int, void*, DWORD, DWORD, DWORD, DWORD);

static HMODULE g_hRealBass = NULL;
static BASS_Stop_t              real_BASS_Stop = NULL;
static BASS_ChannelSetAttributes_t real_BASS_ChannelSetAttributes = NULL;
static BASS_Free_t              real_BASS_Free = NULL;
static BASS_Init_t              real_BASS_Init = NULL;
static BASS_Start_t             real_BASS_Start = NULL;
static BASS_SetConfig_t         real_BASS_SetConfig = NULL;
static BASS_ChannelStop_t       real_BASS_ChannelStop = NULL;
static BASS_MusicPlayEx_t       real_BASS_MusicPlayEx = NULL;
static BASS_ErrorGetCode_t      real_BASS_ErrorGetCode = NULL;
static BASS_MusicLoad_t         real_BASS_MusicLoad = NULL;

/* ---- High-Resolution GetTickCount Hook ---- */

static LARGE_INTEGER g_qpc_freq = {0};
static DWORD (WINAPI *real_GetTickCount)(void) = NULL;

static DWORD WINAPI hooked_GetTickCount(void)
{
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return (DWORD)(unsigned __int64)((now.QuadPart * 1000ULL) / g_qpc_freq.QuadPart);
}

static int hook_gettickcount(void)
{
    HMODULE hExe = GetModuleHandleA(NULL);
    if (!hExe) return 0;

    PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)hExe;
    PIMAGE_NT_HEADERS nt = (PIMAGE_NT_HEADERS)((BYTE*)hExe + dos->e_lfanew);
    PIMAGE_IMPORT_DESCRIPTOR imp = (PIMAGE_IMPORT_DESCRIPTOR)((BYTE*)hExe +
        nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

    while (imp->Name) {
        const char *dll_name = (const char *)((BYTE*)hExe + imp->Name);
        if (_stricmp(dll_name, "kernel32.dll") == 0) {
            PIMAGE_THUNK_DATA thunk = (PIMAGE_THUNK_DATA)((BYTE*)hExe + imp->FirstThunk);
            PIMAGE_THUNK_DATA orig = (PIMAGE_THUNK_DATA)((BYTE*)hExe + imp->OriginalFirstThunk);
            while (orig->u1.AddressOfData) {
                if (!(orig->u1.Ordinal & IMAGE_ORDINAL_FLAG)) {
                    PIMAGE_IMPORT_BY_NAME imp_name = (PIMAGE_IMPORT_BY_NAME)
                        ((BYTE*)hExe + orig->u1.AddressOfData);
                    if (_stricmp((char*)imp_name->Name, "GetTickCount") == 0) {
                        DWORD oldProtect;
                        if (VirtualProtect(&thunk->u1.Function, sizeof(void*),
                                           PAGE_READWRITE, &oldProtect)) {
                            real_GetTickCount = (DWORD (WINAPI*)(void))thunk->u1.Function;
                            thunk->u1.Function = (DWORD_PTR)hooked_GetTickCount;
                            VirtualProtect(&thunk->u1.Function, sizeof(void*),
                                          oldProtect, &oldProtect);
                        }
                        return 1;
                    }
                }
                orig++;
                thunk++;
            }
        }
        imp++;
    }
    return 0;
}

/* ---- Absolute Address Patcher ---- */
/* No pattern matching — patch at exact virtual addresses. Safe and surgical. */

#define IMAGE_BASE 0x400000

/* Patch at a specific VA: verify expected bytes, then overwrite. */
static int patch_at(DWORD va, const BYTE *expected, SIZE_T len, const BYTE *replacement)
{
    BYTE *addr = (BYTE *)va;
    DWORD oldProtect;
    int ok = 1;

    /* Verify current bytes match expected */
    for (SIZE_T i = 0; i < len; i++) {
        if (addr[i] != expected[i]) {
            ok = 0;
            break;
        }
    }
    if (!ok) return 0;

    if (!VirtualProtect(addr, len, PAGE_EXECUTE_READWRITE, &oldProtect))
        return 0;

    memcpy(addr, replacement, len);
    VirtualProtect(addr, len, oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), addr, len);
    return 1;
}

/* Find and patch a pattern in .text — used only for vsync (2 locations, 
 * can't use absolute address since we need to find both) */
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

/* ---- Vsync pattern (2 locations in .text) ---- */
static const BYTE vsync_pattern[] = {
    0xC7, 0x86, 0xF8, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00
};
static const BYTE vsync_replacement[] = {
    0xC7, 0x86, 0xF8, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80
};

static void ApplyPatches(void)
{
    BYTE *text_start = (BYTE *)IMAGE_BASE + 0x1000;
    SIZE_T text_size = 0xCE000;
    int patch_count = 0;
    int hook_ok = 0;
    int idiv_ok = 0, jbe_ok = 0;
    char log_path[MAX_PATH];

    /* 1. Hook GetTickCount for high-resolution timing */
    QueryPerformanceFrequency(&g_qpc_freq);
    hook_ok = hook_gettickcount();
    patch_count += hook_ok;

    /* 2. NOP the render-skip jbe at VA 0x46BF55
     *    Original: 76 5D (jbe skip_render — skips Present() if too soon)
     *    Patched:  90 90 (nop nop — always render, never skip)
     *    This is checked EVERY FRAME, so patch timing doesn't matter. */
    {
        BYTE expected[] = { 0x76, 0x5D };
        BYTE replacement[] = { 0x90, 0x90 };
        jbe_ok = patch_at(0x46BF55, expected, 2, replacement);
        patch_count += jbe_ok;
    }

    /* 3. Patch vsync: INTERVAL_ONE → INTERVAL_IMMEDIATE (2 locations) */
    patch_count += patch_pattern(text_start, text_size,
        vsync_pattern, sizeof(vsync_pattern),
        vsync_replacement);

    /* 4. timeBeginPeriod(1) for high-res timer */
    timeBeginPeriod(1);
    patch_count++;

    /* Write log */
    GetModuleFileNameA(NULL, log_path, MAX_PATH);
    char *p = strrchr(log_path, '.');
    if (p) strcpy(p, "_fps.log");
    else strcat(log_path, "_fps.log");

    FILE *f = fopen(log_path, "w");
    if (f) {
        fprintf(f, "Hamsterball FPS Uncap Proxy v7.0\n");
        fprintf(f, "=================================\n");
        fprintf(f, "Patches applied: %d\n", patch_count);
        fprintf(f, "  GetTickCount IAT hook: %s (QPC-based sub-ms timing)\n", hook_ok ? "OK" : "FAILED");
        fprintf(f, "  render-skip jbe NOP @0x46BF55: %s (always render)\n", jbe_ok ? "OK" : "FAILED (bytes mismatch)");
        fprintf(f, "  vsync: INTERVAL_ONE -> INTERVAL_IMMEDIATE\n");
        fprintf(f, "  timer: timeBeginPeriod(1)\n");
        fprintf(f, "  fps_target: 100 (ORIGINAL - correct physics)\n");
        fprintf(f, "  QPC frequency: %lld Hz (%.3f ns resolution)\n",
            (long long)g_qpc_freq.QuadPart,
            1e9 / (double)g_qpc_freq.QuadPart);
        fclose(f);
    }
}

/* ---- Background Thread ---- */

static DWORD WINAPI patch_thread(LPVOID lpParam)
{
    (void)lpParam;
    Sleep(500);
    ApplyPatches();
    return 0;
}

/* ---- BASS Proxy Exports (__stdcall) ---- */

__declspec(dllexport) void __stdcall BASS_Stop(void)
{ if (real_BASS_Stop) real_BASS_Stop(); }

__declspec(dllexport) int __stdcall BASS_ChannelSetAttributes(DWORD handle, float freq, int vol, int pan)
{ if (real_BASS_ChannelSetAttributes) return real_BASS_ChannelSetAttributes(handle, freq, vol, pan); return 0; }

__declspec(dllexport) int __stdcall BASS_Free(void)
{ if (real_BASS_Free) return real_BASS_Free(); return 0; }

__declspec(dllexport) int __stdcall BASS_Init(int device, DWORD freq, DWORD flags, HWND win, void *clsid)
{ if (real_BASS_Init) return real_BASS_Init(device, freq, flags, win, clsid); return 0; }

__declspec(dllexport) int __stdcall BASS_Start(void)
{ if (real_BASS_Start) return real_BASS_Start(); return 0; }

__declspec(dllexport) int __stdcall BASS_SetConfig(DWORD option, DWORD value)
{ if (real_BASS_SetConfig) return real_BASS_SetConfig(option, value); return 0; }

__declspec(dllexport) int __stdcall BASS_ChannelStop(DWORD handle)
{ if (real_BASS_ChannelStop) return real_BASS_ChannelStop(handle); return 0; }

__declspec(dllexport) int __stdcall BASS_MusicPlayEx(DWORD handle, DWORD pos, BOOL seek)
{ if (real_BASS_MusicPlayEx) return real_BASS_MusicPlayEx(handle, pos, seek); return 0; }

__declspec(dllexport) int __stdcall BASS_ErrorGetCode(void)
{ if (real_BASS_ErrorGetCode) return real_BASS_ErrorGetCode(); return 0; }

__declspec(dllexport) DWORD __stdcall BASS_MusicLoad(int mem, void *file, DWORD offset, DWORD length, DWORD flags, DWORD freq)
{ if (real_BASS_MusicLoad) return real_BASS_MusicLoad(mem, file, offset, length, flags, freq); return 0; }

/* ---- DllMain ---- */

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hinstDLL);

        g_hRealBass = LoadLibraryA("bass_real.dll");
        if (g_hRealBass == NULL) {
            char path[MAX_PATH];
            if (GetModuleFileNameA(hinstDLL, path, MAX_PATH)) {
                char *p = strrchr(path, '\\');
                if (p) { strcpy(p + 1, "bass_real.dll"); g_hRealBass = LoadLibraryA(path); }
            }
        }

        if (g_hRealBass != NULL) {
            real_BASS_Stop              = (BASS_Stop_t)GetProcAddress(g_hRealBass, "BASS_Stop");
            real_BASS_ChannelSetAttributes = (BASS_ChannelSetAttributes_t)GetProcAddress(g_hRealBass, "BASS_ChannelSetAttributes");
            real_BASS_Free              = (BASS_Free_t)GetProcAddress(g_hRealBass, "BASS_Free");
            real_BASS_Init              = (BASS_Init_t)GetProcAddress(g_hRealBass, "BASS_Init");
            real_BASS_Start             = (BASS_Start_t)GetProcAddress(g_hRealBass, "BASS_Start");
            real_BASS_SetConfig         = (BASS_SetConfig_t)GetProcAddress(g_hRealBass, "BASS_SetConfig");
            real_BASS_ChannelStop       = (BASS_ChannelStop_t)GetProcAddress(g_hRealBass, "BASS_ChannelStop");
            real_BASS_MusicPlayEx       = (BASS_MusicPlayEx_t)GetProcAddress(g_hRealBass, "BASS_MusicPlayEx");
            real_BASS_ErrorGetCode      = (BASS_ErrorGetCode_t)GetProcAddress(g_hRealBass, "BASS_ErrorGetCode");
            real_BASS_MusicLoad         = (BASS_MusicLoad_t)GetProcAddress(g_hRealBass, "BASS_MusicLoad");
        }

        CreateThread(NULL, 0, patch_thread, NULL, 0, NULL);
        break;

    case DLL_PROCESS_DETACH:
        timeEndPeriod(1);
        if (g_hRealBass) { FreeLibrary(g_hRealBass); g_hRealBass = NULL; }
        break;
    }
    return TRUE;
}
