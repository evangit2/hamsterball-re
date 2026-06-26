/*
 * unbreakable_ball.c — BASS.dll proxy mod
 *
 * Translated from XRow's CEA script: no fall damage / unbreakable ball.
 *
 * Patches 8 points in Hamsterball.exe:
 *   5x NOP (prevent shatter flag writes + active flag writes)
 *   3x RET (early-return from Ball_Shatter, Ball_Shatter_OnRamp, variant)
 *
 * Winlator/Android-safe:
 *   - No IAT hooks (GetTickCount hook crashes Android)
 *   - No code caves, no mid-function CALL C functions
 *   - Uses VirtualProtect + memcpy for byte patches (same as no_pause etc.)
 *   - Patches applied once after 3s delay, restored on DLL detach
 *   - All IsBadReadPtr guarded
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll unbreakable_ball.c -lwinmm \
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
 * Unbreakable Ball Mod — Byte Patches
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Hamsterball.exe base address (static at 0x400000) */
#define EXE_BASE 0x400000

/* Patch table: address (RVA), original bytes, patched bytes, length */
typedef struct {
    DWORD rva;          /* offset from module base */
    BYTE  orig[8];      /* original bytes (for restore + verify) */
    BYTE  patch[8];     /* replacement bytes */
    int   len;           /* byte count */
    BOOL  applied;       /* track state */
} patch_t;

static patch_t g_patches[] = {
    /* 1. Ball_Shatter — early RET (was: push -1; mov eax,fs:[0]) */
    { 0x8D70,
      { 0x6A, 0xFF, 0x64, 0xA1, 0x00, 0x00, 0x00, 0x00 },
      { 0xC3, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 },
      3, FALSE },

    /* 2. Ball_Shatter variant — early RET */
    { 0x9050,
      { 0x6A, 0xFF, 0x64, 0xA1, 0x00, 0x00, 0x00, 0x00 },
      { 0xC3, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 },
      3, FALSE },

    /* 3. Ball_Shatter_OnRamp — early RET */
    { 0x9480,
      { 0x6A, 0xFF, 0x64, 0xA1, 0x00, 0x00, 0x00, 0x00 },
      { 0xC3, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 },
      3, FALSE },

    /* 4. NOP: MOV [ebp+0x768], al (sets is_active=0) */
    { 0xC761,
      { 0x88, 0x85, 0x68, 0x07, 0x00, 0x00, 0x00, 0x00 },
      { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 },
      6, FALSE },

    /* 5. NOP: MOV byte [ebp+0x2E9], 1 (sets shatter flag) */
    { 0xC767,
      { 0xC6, 0x85, 0xE9, 0x02, 0x00, 0x00, 0x01, 0x00 },
      { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 },
      7, FALSE },

    /* 6. NOP: MOV byte [esi+0x768], 0 (sets is_active=0) */
    { 0xF226,
      { 0xC6, 0x86, 0x68, 0x07, 0x00, 0x00, 0x00, 0x00 },
      { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 },
      7, FALSE },

    /* 7. NOP: MOV byte [esi+0x2E9], 1 (sets shatter flag) */
    { 0xF22D,
      { 0xC6, 0x86, 0xE9, 0x02, 0x00, 0x00, 0x01, 0x00 },
      { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 },
      7, FALSE },

    /* 8. NOP: INC dword [esi+0x2EC] (fall timer increment) */
    { 0x75C9,
      { 0xFF, 0x86, 0xEC, 0x02, 0x00, 0x00, 0x00, 0x00 },
      { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 },
      6, FALSE },
};

#define NUM_PATCHES (sizeof(g_patches) / sizeof(g_patches[0]))

/* Apply a single byte patch using VirtualProtect */
static BOOL apply_patch(patch_t *p)
{
    DWORD addr = EXE_BASE + p->rva;
    DWORD oldProtect = 0;

    if (IsBadReadPtr((void*)addr, p->len)) return FALSE;

    /* Verify original bytes match (don't patch if already patched or wrong) */
    for (int i = 0; i < p->len; i++) {
        if (((BYTE*)addr)[i] != p->orig[i]) {
            /* Already patched or different — skip to be safe */
            return FALSE;
        }
    }

    if (!VirtualProtect((void*)addr, p->len, PAGE_EXECUTE_READWRITE, &oldProtect))
        return FALSE;

    memcpy((void*)addr, p->patch, p->len);

    VirtualProtect((void*)addr, p->len, oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), (void*)addr, p->len);

    p->applied = TRUE;
    return TRUE;
}

/* Restore original bytes */
static void restore_patch(patch_t *p)
{
    DWORD addr = EXE_BASE + p->rva;
    DWORD oldProtect = 0;

    if (!p->applied) return;
    if (IsBadReadPtr((void*)addr, p->len)) return;

    if (!VirtualProtect((void*)addr, p->len, PAGE_EXECUTE_READWRITE, &oldProtect))
        return;

    memcpy((void*)addr, p->orig, p->len);

    VirtualProtect((void*)addr, p->len, oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), (void*)addr, p->len);

    p->applied = FALSE;
}

/* Background thread: wait for game to load, then apply patches */
static DWORD WINAPI patch_thread(LPVOID param)
{
    Sleep(3000);

    for (int i = 0; i < (int)NUM_PATCHES; i++) {
        apply_patch(&g_patches[i]);
    }

    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DLL Entry
 * ═══════════════════════════════════════════ table═══════════════════════════ */

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        load_real_bass();
        {
            HANDLE hThread = CreateThread(NULL, 0, patch_thread, NULL, 0, NULL);
            if (hThread) CloseHandle(hThread);
        }
        break;

    case DLL_PROCESS_DETACH:
        /* Restore original bytes on unload */
        for (int i = 0; i < (int)NUM_PATCHES; i++) {
            restore_patch(&g_patches[i]);
        }
        break;
    }
    return TRUE;
}
