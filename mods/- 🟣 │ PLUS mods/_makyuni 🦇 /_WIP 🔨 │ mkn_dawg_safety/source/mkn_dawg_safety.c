/*
 * mkn_dawg_safety — Force sleeping mode on pathless Blockdawgs.
 *
 * Bass.dll proxy mod. When a Blockdawg entity has no DAWGPATH to follow
 * (path pointer at +0x10F0 is NULL), ArenaObject_Update (0x43C4E0) would
 * crash dereferencing it. This mod detours the update function and sets
 * +0x1150 = 1 (sleeping state) before the crash can happen.
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll mkn_dawg_safety.c \
 *     bass.def -O2 -static -static-libgcc \
 *     -Wl,--enable-stdcall-fixup -Wl,--add-stdcall-alias -msse2 -mfpmath=sse
 *
 * Author: Hamsterbot for MAKYUNI 🦇
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include <stdbool.h>

/* ─── BASS proxy: forward all imports to bass_real.dll ──────────────────── */

static HMODULE g_hRealBass = NULL;

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

static BASS_Stop_t                   real_BASS_Stop = NULL;
static BASS_ChannelSetAttributes_t   real_BASS_ChannelSetAttributes = NULL;
static BASS_Free_t                   real_BASS_Free = NULL;
static BASS_Init_t                   real_BASS_Init = NULL;
static BASS_Start_t                  real_BASS_Start = NULL;
static BASS_SetConfig_t              real_BASS_SetConfig = NULL;
static BASS_ChannelStop_t            real_BASS_ChannelStop = NULL;
static BASS_MusicPlayEx_t            real_BASS_MusicPlayEx = NULL;
static BASS_ErrorGetCode_t           real_BASS_ErrorGetCode = NULL;
static BASS_MusicLoad_t              real_BASS_MusicLoad = NULL;

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

static void load_real_bass(void)
{
    g_hRealBass = LoadLibraryA("bass_real.dll");
    if (!g_hRealBass) {
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
}

/* ─── Blockdawg safety detour ───────────────────────────────────────────── */

/*
 * ArenaObject_Update (0x0043C4E0) is the per-frame update for Blockdawgs.
 * When a Blockdawg has no DAWGPATH (path at +0x10F0 is NULL), the update
 * function crashes trying to call Path_GetPosition(NULL).
 *
 * Fix: detour the update, check if path is NULL, force sleeping state
 * (+0x1150 = 1) before calling the original function.
 *
 * Original prologue at 0x0043C4E0:
 *   6A FF           PUSH -1           (2 bytes)
 *   68 93 BB 4C 00  PUSH 0x004CBB93   (5 bytes)
 *   ...SEH frame setup continues...
 *
 * We copy 7 bytes (two complete instructions) to a trampoline.
 */

#define ADDR_UPDATE        0x0043C4E0
#define BLOCKDAWG_VTABLE   0x004D5638

#define OFF_PATH   0x10F0   /* path data pointer (0 = no path) */
#define OFF_STATE  0x1150   /* 0 = active (follows path), 1 = sleeping (safe) */
#define OBJ_SIZE   0x1154   /* Blockdawg struct size */

typedef void (__fastcall *Update_Fn)(DWORD *thisPtr, void *edx);
static Update_Fn g_orig_Update = NULL;
static unsigned char *g_trampoline = NULL;
static bool g_hookInstalled = false;

/* Expected first 7 bytes at 0x0043C4E0: PUSH -1; PUSH 0x004CBB93 */
static const BYTE EXPECTED_PROLOGUE[7] = {
    0x6A, 0xFF,             /* PUSH -1 */
    0x68, 0x93, 0xBB, 0x4C, 0x00  /* PUSH 0x004CBB93 */
};

static void __fastcall Hooked_Update(DWORD *thisPtr, void *edx)
{
    /*
     * Check: is this a Blockdawg with no path?
     * - vtable must match Blockdawg (0x4D5638)
     * - path at +0x10F0 must be NULL or unreadable
     * - state at +0x1150 must be 0 (active — would crash)
     * If all conditions met, force sleeping (state = 1).
     */
    if (!IsBadReadPtr(thisPtr, OBJ_SIZE)) {
        if (*(DWORD *)thisPtr == BLOCKDAWG_VTABLE) {
            DWORD pathVal = *(DWORD *)((BYTE *)thisPtr + OFF_PATH);
            if (pathVal == 0 || IsBadReadPtr((void *)pathVal, 8)) {
                BYTE state = *((BYTE *)thisPtr + OFF_STATE);
                if (state == 0) {
                    *((BYTE *)thisPtr + OFF_STATE) = 1;
                }
            }
        }
    }

    /* Call original function */
    if (g_orig_Update) g_orig_Update(thisPtr, edx);
}

static bool InstallDetour(void)
{
    if (IsBadReadPtr((void *)ADDR_UPDATE, 7)) return false;
    if (memcmp((void *)ADDR_UPDATE, EXPECTED_PROLOGUE, 7) != 0) return false;

    /* Allocate trampoline: 7 bytes original + 5 bytes JMP = 12 */
    unsigned char *tramp = (unsigned char *)VirtualAlloc(NULL, 16,
        MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!tramp) return false;

    /* Copy original 7 bytes */
    memcpy(tramp, (void *)ADDR_UPDATE, 7);

    /* Append JMP back to ADDR_UPDATE + 7 */
    tramp[7] = 0xE9;
    *(DWORD *)(tramp + 8) = (ADDR_UPDATE + 7) - ((DWORD)tramp + 12);

    g_trampoline = tramp;
    g_orig_Update = (Update_Fn)tramp;

    /* Patch original: JMP rel32 + 2 NOPs (7 bytes total) */
    DWORD oldProt;
    VirtualProtect((void *)ADDR_UPDATE, 7, PAGE_EXECUTE_READWRITE, &oldProt);
    *(BYTE *)ADDR_UPDATE = 0xE9;
    *(DWORD *)(ADDR_UPDATE + 1) = (DWORD)Hooked_Update - (ADDR_UPDATE + 5);
    *(WORD *)(ADDR_UPDATE + 5) = 0x9090; /* 2 NOPs */
    VirtualProtect((void *)ADDR_UPDATE, 7, oldProt, &oldProt);
    FlushInstructionCache(GetCurrentProcess(), (void *)ADDR_UPDATE, 7);

    g_hookInstalled = true;
    return true;
}

static void RemoveDetour(void)
{
    if (!g_hookInstalled) return;

    DWORD oldProt;
    VirtualProtect((void *)ADDR_UPDATE, 7, PAGE_EXECUTE_READWRITE, &oldProt);
    memcpy((void *)ADDR_UPDATE, EXPECTED_PROLOGUE, 7);
    VirtualProtect((void *)ADDR_UPDATE, 7, oldProt, &oldProt);
    FlushInstructionCache(GetCurrentProcess(), (void *)ADDR_UPDATE, 7);

    if (g_trampoline) {
        VirtualFree(g_trampoline, 0, MEM_RELEASE);
        g_trampoline = NULL;
    }
    g_orig_Update = NULL;
    g_hookInstalled = false;
}

/* ─── Background thread ─────────────────────────────────────────────────── */

static DWORD WINAPI patch_thread(LPVOID lpParam)
{
    (void)lpParam;
    Sleep(500);
    InstallDetour();
    return 0;
}

/* ─── DllMain ──────────────────────────────────────────────────────────── */

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hinstDLL);
        load_real_bass();
        CreateThread(NULL, 0, patch_thread, NULL, 0, NULL);
        break;

    case DLL_PROCESS_DETACH:
        RemoveDetour();
        if (g_hRealBass) { FreeLibrary(g_hRealBass); g_hRealBass = NULL; }
        break;
    }
    return TRUE;
}
