/*
 * always_active_mode — Keeps the game running when you tab out of fullscreen.
 *
 * ROOT CAUSE:
 *   When the game is in exclusive fullscreen D3D8 mode and you Alt-Tab or click
 *   outside, Windows sends WM_ACTIVATEAPP(FALSE). The WndProc at 0x46CBE0
 *   handles this by:
 *     1. Setting App+0x15A (active flag) = 0
 *     2. Skipping Graphics_Defaults (device reset) since active=0
 *     3. Calling App vtable[0x24] with active=0
 *
 *   Then in App_Run (0x46BD80), the render condition is:
 *     if (graphics != NULL && (active_flag != 0 || !fullscreen))
 *   When fullscreen AND active=0, this condition FAILS → game stops rendering.
 *   This causes the black screen.
 *
 *   Additionally, Windows minimizes exclusive fullscreen D3D windows on focus
 *   loss, which is why the game minimizes itself.
 *
 * FIX (two byte patches):
 *   Patch 1: WM_ACTIVATEAPP handler at 0x46CD1A
 *     setnz al (0F 95 C0) → mov al, 1; nop (B0 01 90)
 *     Makes the active flag ALWAYS 1, regardless of wParam.
 *
 *   Patch 2: App_Run render check at 0x46BF6F
 *     jnz +0x39 (75 39) → nop nop (90 90)
 *     Makes the game ALWAYS render frames, bypassing the fullscreen+inactive check.
 *
 *   With both patches, when you tab out:
 *   - The game thinks it's still active (active=1)
 *   - The game continues rendering frames
 *   - Graphics_PresentOrEnd handles D3DERR_DEVICELOST by calling Reset()
 *   - The D3D device recovers automatically when you tab back in
 *
 *   NOTE: On real Windows with exclusive fullscreen, Windows itself may still
 *   minimize the window. The game will continue running in the background and
 *   can be restored by clicking its taskbar button. On Wine/Android, this
 *   typically works without minimize since Wine uses virtual desktop mode.
 *
 *   The "misplaced windows after close" issue is a Windows behavior with
 *   exclusive fullscreen D3D8 — the display mode change affects window
 *   arrangement. Using windowed mode avoids this.
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/* ── BASS proxy exports (REQUIRED — all game imports) ────────────── */
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

/* ── Mod: Always Active Mode ─────────────────────────────────────── */

static void apply_patches(void) {
    HMODULE hExe = GetModuleHandleA("Hamsterball.exe");
    if (!hExe) hExe = GetModuleHandleA(NULL);
    DWORD base = (DWORD)hExe;
    DWORD oldProt;

    /*
     * Patch 1: WM_ACTIVATEAPP handler — always set active=1
     *
     * At 0x46CD1A in the WndProc (0x46CBE0):
     *   Original: 0F 95 C0     setnz al       (al = wParam != 0)
     *   Patched:  B0 01 90     mov al, 1; nop (al = 1 always)
     *
     * The instruction at 0x46CD1F: mov byte [esi+0x15A], al
     * then stores this value as the active flag.
     * With this patch, the active flag is ALWAYS 1.
     */
    {
        DWORD addr = base + 0x2CD1A;  /* RVA = 0x46CD1A - 0x400000 = 0x2CD1A */
        if (VirtualProtect((void*)addr, 3, PAGE_READWRITE, &oldProt)) {
            *((BYTE*)addr)     = 0xB0;  /* mov al, */
            *((BYTE*)(addr+1)) = 0x01;  /* 1       */
            *((BYTE*)(addr+2)) = 0x90;  /* nop     */
            VirtualProtect((void*)addr, 3, oldProt, &oldProt);
        }
    }

    /*
     * Patch 2: App_Run render condition — always render
     *
     * At 0x46BF6F in App_Run (0x46BD80):
     *   The code checks: if (fullscreen && !active) → skip rendering
     *   Original: 75 39   jnz +0x39  (skip render when fullscreen && !active)
     *   Patched:  90 90   nop nop   (always fall through → always render)
     *
     * With Patch 1 making active always 1, this patch is technically redundant
     * for the normal case. But it ensures rendering continues even if some
     * other code path sets active=0.
     */
    {
        DWORD addr = base + 0x2BF6F;  /* RVA = 0x46BF6F - 0x400000 = 0x2BF6F */
        if (VirtualProtect((void*)addr, 2, PAGE_READWRITE, &oldProt)) {
            *((BYTE*)addr)     = 0x90;  /* nop */
            *((BYTE*)(addr+1)) = 0x90;  /* nop */
            VirtualProtect((void*)addr, 2, oldProt, &oldProt);
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
