/*
 * no-pause mod v2 — Prevents the pause menu from appearing via ANY input method.
 *
 * ROOT CAUSE (why v1 didn't work):
 *   There are THREE separate code paths that call Scene_CreateGameOverMenu,
 *   which creates the pause overlay and sets scene+0x874 = 1 (pause flag).
 *   v1 only patched ONE path. The other two were completely unpatched.
 *
 * Path 1: Scene_Update (0x419c00) — DirectInput ESC poll
 *   Inside Scene_Update, Input_CheckKeyCombo(app, 2) checks for ESC via DirectInput.
 *   If pressed, calls Scene_CreateGameOverMenu(scene, 1).
 *   Patch: 0x419d5b: JZ(74)→JMP(EB) — always skip pause creation.  [v1 had this ✓]
 *
 * Path 2: vtable[5] thunk (0x4130A0) — Right-click on game viewport
 *   App_OnMouseDown dispatches mouse events to the hit-tested scene object via vtable[5].
 *   The thunk checks if param_3 == 1 (right mouse button), then checks App+0x238
 *   and PlayerProfile+0x95 flags. If conditions met, calls Scene_CreateGameOverMenu.
 *   Patch: 0x4130B5: JZ(74)→JMP(EB) — always skip the pause call.  [v1 MISSING ✗]
 *
 * Path 3: vtable[8] thunk (0x40B400) — Win32 message pump ESC (VK_ESCAPE=0x1B)
 *   The Win32 message pump dispatches key events to scene objects via vtable[8].
 *   The thunk checks if param_1 == 0x1B (27 = VK_ESCAPE). If match, tail-calls
 *   Scene_CreateGameOverMenu.
 *   Patch: 0x40B405: JNZ(75)→JMP(EB) — always skip the pause call.  [v1 MISSING ✗]
 *
 * Both Path 2 and Path 3 are shared thunks appearing in 32 scene-object vtables.
 * They are completely independent of the Scene_Update ESC check in Path 1.
 *
 * Result: ESC and right-click do nothing. The game continues running normally.
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/* ── BASS proxy exports (REQUIRED — all 10 game imports) ────────────── */
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

typedef int  (__stdcall *BASS_MusicLoad_t)(const char*, DWORD, DWORD, DWORD, DWORD);
static BASS_MusicLoad_t real_BASS_MusicLoad = NULL;
__declspec(dllexport) int __stdcall BASS_MusicLoad(const char* a, DWORD b, DWORD c, DWORD d, DWORD e) {
    if (real_BASS_MusicLoad) return real_BASS_MusicLoad(a, b, c, d, e);
    return 0;
}

typedef int  (__stdcall *BASS_Free_t)(void);
static BASS_Free_t real_BASS_Free = NULL;
__declspec(dllexport) int __stdcall BASS_Free(void) {
    if (real_BASS_Free) return real_BASS_Free();
    return 1;
}

typedef int  (__stdcall *BASS_Init_t)(int, int, int, HWND, void*);
static BASS_Init_t real_BASS_Init = NULL;
__declspec(dllexport) int __stdcall BASS_Init(int a, int b, int c, HWND d, void* e) {
    if (real_BASS_Init) return real_BASS_Init(a, b, c, d, e);
    return 1;
}

typedef void (__stdcall *BASS_ChannelStop_t)(DWORD);
static BASS_ChannelStop_t real_BASS_ChannelStop = NULL;
__declspec(dllexport) void __stdcall BASS_ChannelStop(DWORD a) {
    if (real_BASS_ChannelStop) real_BASS_ChannelStop(a);
}

typedef int  (__stdcall *BASS_ErrorGetCode_t)(void);
static BASS_ErrorGetCode_t real_BASS_ErrorGetCode = NULL;
__declspec(dllexport) int __stdcall BASS_ErrorGetCode(void) {
    if (real_BASS_ErrorGetCode) return real_BASS_ErrorGetCode();
    return 0;
}

typedef int  (__stdcall *BASS_SetConfig_t)(DWORD, DWORD);
static BASS_SetConfig_t real_BASS_SetConfig = NULL;
__declspec(dllexport) int __stdcall BASS_SetConfig(DWORD a, DWORD b) {
    if (real_BASS_SetConfig) return real_BASS_SetConfig(a, b);
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

/* ── Mod: Disable Pause (all 3 paths) ─────────────────────────────── */

static void apply_patches(void) {
    HMODULE hExe = GetModuleHandleA("Hamsterball.exe");
    if (!hExe) hExe = GetModuleHandleA(NULL);
    DWORD base = (DWORD)hExe;
    DWORD oldProt;

    /* Path 1: Scene_Update DirectInput ESC check (0x419d5b)
     * Original: 74 09  (JZ  +0x09 → skip pause creation when ESC not pressed)
     * Patched:  EB 09  (JMP  +0x09 → always skip pause creation)
     */
    {
        DWORD addr = base + 0x19d5b;
        if (VirtualProtect((void*)addr, 1, PAGE_READWRITE, &oldProt)) {
            *((BYTE*)addr) = 0xEB;  /* JZ → JMP */
            VirtualProtect((void*)addr, 1, oldProt, &oldProt);
        }
    }

    /* Path 2: vtable[5] right-click pause thunk (0x4130b5)
     * This thunk is at the JZ that skips Scene_CreateGameOverMenu when
     * App+0x238 == 0. We change it to always skip.
     * Original: 74 17  (JZ  +0x17 → skip when App flag is zero)
     * Patched:  EB 17  (JMP  +0x17 → always skip)
     */
    {
        DWORD addr = base + 0x130b5;
        if (VirtualProtect((void*)addr, 1, PAGE_READWRITE, &oldProt)) {
            *((BYTE*)addr) = 0xEB;  /* JZ → JMP */
            VirtualProtect((void*)addr, 1, oldProt, &oldProt);
        }
    }

    /* Path 3: vtable[8] ESC message handler thunk (0x40b405)
     * This thunk checks if param == 0x1B (VK_ESCAPE). The JNZ at 0x40b405
     * skips the pause call when param != 0x1B. We change it to always skip.
     * Original: 75 0D  (JNZ +0x0D → skip when key != ESC)
     * Patched:  EB 0D  (JMP  +0x0D → always skip)
     */
    {
        DWORD addr = base + 0x0b405;
        if (VirtualProtect((void*)addr, 1, PAGE_READWRITE, &oldProt)) {
            *((BYTE*)addr) = 0xEB;  /* JNZ → JMP */
            VirtualProtect((void*)addr, 1, oldProt, &oldProt);
        }
    }
}

static void init_bass_proxy(void) {
    g_hRealBass = LoadLibraryA("bass_real.dll");
    if (!g_hRealBass) return;

    real_BASS_ChannelSetAttributes = (BASS_ChannelSetAttributes_t)GetProcAddress(g_hRealBass, "BASS_ChannelSetAttributes");
    real_BASS_MusicPlayEx          = (BASS_MusicPlayEx_t)GetProcAddress(g_hRealBass, "BASS_MusicPlayEx");
    real_BASS_MusicLoad             = (BASS_MusicLoad_t)GetProcAddress(g_hRealBass, "BASS_MusicLoad");
    real_BASS_Free                  = (BASS_Free_t)GetProcAddress(g_hRealBass, "BASS_Free");
    real_BASS_Init                  = (BASS_Init_t)GetProcAddress(g_hRealBass, "BASS_Init");
    real_BASS_ChannelStop           = (BASS_ChannelStop_t)GetProcAddress(g_hRealBass, "BASS_ChannelStop");
    real_BASS_ErrorGetCode          = (BASS_ErrorGetCode_t)GetProcAddress(g_hRealBass, "BASS_ErrorGetCode");
    real_BASS_SetConfig             = (BASS_SetConfig_t)GetProcAddress(g_hRealBass, "BASS_SetConfig");
    real_BASS_Start                 = (BASS_Start_t)GetProcAddress(g_hRealBass, "BASS_Start");
    real_BASS_Stop                  = (BASS_Stop_t)GetProcAddress(g_hRealBass, "BASS_Stop");
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    switch (fdwReason) {
        case DLL_PROCESS_ATTACH:
            init_bass_proxy();
            apply_patches();
            break;
    }
    return TRUE;
}
