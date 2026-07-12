/*
 * hamsterball_fps_mod.c — BASS.dll proxy that lets users set a custom FPS cap.
 *
 * Installation (Windows):
 *   1. In your Hamsterball game folder, rename bass.dll -> bass_real.dll
 *   2. Copy this proxy bass.dll into the game folder
 *   3. Copy hamsterball_fps.ini into the game folder
 *   4. Edit hamsterball_fps.ini to set TargetFPS / RenderFPS
 *
 * How it works:
 *   - The game loads this proxy instead of the real bass.dll.
 *   - We forward all BASS calls to bass_real.dll.
 *   - On startup we read hamsterball_fps.ini and write App+0x16C / App+0x170
 *     via the App pointer at DAT_005341E0.
 *   - Optional Uncap=1 also NOPs the render-skip jbe and disables vsync.
 *
 * Build: i686-w64-mingw32-gcc -shared -o bass.dll hamsterball_fps_mod.c \
 *          bass_exports.def -lwinmm -Wl,--enable-stdcall-fixup -O2 \
 *          -static -static-libgcc -Wl,--add-stdcall-alias
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winuser.h>
#include <mmsystem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

/* ---- Config ---- */

typedef struct {
    int target_fps;
    int render_fps;
    int uncap;
} fps_config_t;

static fps_config_t g_cfg = { 144, 144, 0 };

/* ---- Helpers ---- */

static int file_exists(const char *path)
{
    DWORD attr = GetFileAttributesA(path);
    return attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY);
}

static int read_ini_int(const char *path, const char *section, const char *key, int default_val)
{
    char buf[32];
    DWORD n = GetPrivateProfileStringA(section, key, "", buf, sizeof(buf), path);
    if (n == 0) return default_val;
    return atoi(buf);
}

static void load_config(const char *ini_path)
{
    if (!file_exists(ini_path)) {
        /* Write a default config so users know what to edit */
        FILE *f = fopen(ini_path, "w");
        if (f) {
            fprintf(f, "[FPS]\n");
            fprintf(f, "TargetFPS=144\n");
            fprintf(f, "RenderFPS=144\n\n");
            fprintf(f, "[Uncap]\n");
            fprintf(f, "; Set to 1 to remove render throttle and vsync entirely.\n");
            fprintf(f, "Uncap=0\n");
            fclose(f);
        }
    }
    g_cfg.target_fps = read_ini_int(ini_path, "FPS", "TargetFPS", 144);
    g_cfg.render_fps = read_ini_int(ini_path, "FPS", "RenderFPS", 144);
    g_cfg.uncap      = read_ini_int(ini_path, "Uncap", "Uncap", 0);

    if (g_cfg.target_fps < 1) g_cfg.target_fps = 1;
    if (g_cfg.target_fps > 1000) g_cfg.target_fps = 1000;
    if (g_cfg.render_fps < 1) g_cfg.render_fps = 1;
    if (g_cfg.render_fps > 1000) g_cfg.render_fps = 1000;
}

/* ---- Memory patching ---- */

#define IMAGE_BASE 0x400000

static int patch_at(DWORD va, const BYTE *expected, SIZE_T len, const BYTE *replacement)
{
    BYTE *addr = (BYTE *)va;
    DWORD oldProtect;
    int ok = 1;

    for (SIZE_T i = 0; i < len; i++) {
        if (addr[i] != expected[i]) { ok = 0; break; }
    }
    if (!ok) return 0;

    if (!VirtualProtect(addr, len, PAGE_EXECUTE_READWRITE, &oldProtect))
        return 0;

    memcpy(addr, replacement, len);
    VirtualProtect(addr, len, oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), addr, len);
    return 1;
}

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

static const BYTE vsync_pattern[] = {
    0xC7, 0x86, 0xF8, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00
};
static const BYTE vsync_replacement[] = {
    0xC7, 0x86, 0xF8, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80
};

static void apply_fps_mod(const char *log_path)
{
    FILE *f = fopen(log_path, "w");
    if (!f) f = stdout;

    fprintf(f, "Hamsterball FPS Mod (bass.dll proxy)\n");
    fprintf(f, "=====================================\n");

    /* Resolve App pointer */
    DWORD *pAppPtr = (DWORD *)0x005341E0;
    DWORD app_addr = 0;
    if (!IsBadReadPtr(pAppPtr, 4)) {
        app_addr = *pAppPtr;
    }

    if (app_addr && !IsBadWritePtr((void *)app_addr, 0x200)) {
        DWORD *pTargetFPS = (DWORD *)(app_addr + 0x16C);
        DWORD *pRenderFPS = (DWORD *)(app_addr + 0x170);

        DWORD oldProtect;
        if (VirtualProtect(pTargetFPS, 4, PAGE_READWRITE, &oldProtect)) {
            *pTargetFPS = g_cfg.target_fps;
            VirtualProtect(pTargetFPS, 4, oldProtect, &oldProtect);
            fprintf(f, "Set App+0x16C target FPS: %d\n", g_cfg.target_fps);
        } else {
            fprintf(f, "FAILED to write App+0x16C\n");
        }

        if (VirtualProtect(pRenderFPS, 4, PAGE_READWRITE, &oldProtect)) {
            *pRenderFPS = g_cfg.render_fps;
            VirtualProtect(pRenderFPS, 4, oldProtect, &oldProtect);
            fprintf(f, "Set App+0x170 render FPS: %d\n", g_cfg.render_fps);
        } else {
            fprintf(f, "FAILED to write App+0x170\n");
        }
    } else {
        fprintf(f, "Could not resolve App pointer (0x%08lX)\n", app_addr);
    }

    if (g_cfg.uncap) {
        int jbe_ok = 0;
        BYTE expected[] = { 0x76, 0x5D };
        BYTE replacement[] = { 0x90, 0x90 };
        jbe_ok = patch_at(0x46BF55, expected, 2, replacement);
        fprintf(f, "Render-skip jbe NOP @0x46BF55: %s\n", jbe_ok ? "OK" : "FAILED (bytes mismatch)");

        BYTE *text_start = (BYTE *)IMAGE_BASE + 0x1000;
        SIZE_T text_size = 0xCE000;
        int n = patch_pattern(text_start, text_size, vsync_pattern, sizeof(vsync_pattern), vsync_replacement);
        fprintf(f, "Vsync INTERVAL_ONE -> INTERVAL_IMMEDIATE patches: %d\n", n);
        timeBeginPeriod(1);
        fprintf(f, "timeBeginPeriod(1) called\n");
    } else {
        fprintf(f, "Uncap mode disabled (only FPS cap changed)\n");
    }

    fprintf(f, "\nConfig: TargetFPS=%d RenderFPS=%d Uncap=%d\n",
            g_cfg.target_fps, g_cfg.render_fps, g_cfg.uncap);

    if (f != stdout) fclose(f);
}

/* ---- Background Thread ---- */

static DWORD WINAPI patch_thread(LPVOID lpParam)
{
    (void)lpParam;

    char ini_path[MAX_PATH];
    char log_path[MAX_PATH];

    GetModuleFileNameA(NULL, ini_path, MAX_PATH);
    char *p = strrchr(ini_path, '\\');
    if (p) strcpy(p + 1, "hamsterball_fps.ini");
    else strcat(ini_path, "hamsterball_fps.ini");

    GetModuleFileNameA(NULL, log_path, MAX_PATH);
    p = strrchr(log_path, '.');
    if (p) strcpy(p, "_fps_mod.log");
    else strcat(log_path, "_fps_mod.log");

    load_config(ini_path);

    /* Wait for App to be constructed. App_Ctor runs early, App_Initialize overwrites
     * the value shortly after, so we patch after initialization settles. */
    Sleep(2000);
    apply_fps_mod(log_path);
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
        if (g_cfg.uncap) timeEndPeriod(1);
        if (g_hRealBass) { FreeLibrary(g_hRealBass); g_hRealBass = NULL; }
        break;
    }
    return TRUE;
}
