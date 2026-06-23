/*
 * no-pause mod — Prevents the pause menu from appearing when ESC is pressed.
 *
 * Root cause: Scene_Update (0x419c00) checks Input_CheckKeyCombo(app, 2) for ESC.
 * If pressed, calls Scene_CreateGameOverMenu(scene, 1) at 0x419d61, which creates
 * the PauseMenu overlay and sets scene+0x874 = 1 (the pause flag). While 0x874=1,
 * the game skips all physics/scene update vtable calls.
 *
 * Patch: At 0x419d5b, the instruction JZ 0x419d66 (74 09) skips the pause creation
 * when ESC is NOT pressed. We change it to JMP 0x419d66 (EB 09) so it ALWAYS skips —
 * even when ESC IS pressed. Single-byte patch: 0x74 → 0xEB at RVA 0x19d5b.
 *
 * Result: ESC does nothing. The game continues running normally — no pause menu,
 * no physics freeze, no camera stop.
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

typedef int  (__stdcall *BASS_ChannelSetFX_t)(DWORD, int, int);
static BASS_ChannelSetFX_t real_BASS_ChannelSetFX = NULL;
__declspec(dllexport) int __stdcall BASS_ChannelSetFX(DWORD a, int b, int c) {
    if (real_BASS_ChannelSetFX) return real_BASS_ChannelSetFX(a, b, c);
    return 0;
}

typedef int  (__stdcall *BASS_FXSetParameters_t)(DWORD, void*);
static BASS_FXSetParameters_t real_BASS_FXSetParameters = NULL;
__declspec(dllexport) int __stdcall BASS_FXSetParameters(DWORD a, void* b) {
    if (real_BASS_FXSetParameters) return real_BASS_FXSetParameters(a, b);
    return 1;
}

typedef int  (__stdcall *BASS_SampleLoad_t)(const char*, DWORD, DWORD, int, DWORD);
static BASS_SampleLoad_t real_BASS_SampleLoad = NULL;
__declspec(dllexport) int __stdcall BASS_SampleLoad(const char* a, DWORD b, DWORD c, int d, DWORD e) {
    if (real_BASS_SampleLoad) return real_BASS_SampleLoad(a, b, c, d, e);
    return 0;
}

typedef int  (__stdcall *BASS_SampleGetChannel_t)(DWORD, DWORD);
static BASS_SampleGetChannel_t real_BASS_SampleGetChannel = NULL;
__declspec(dllexport) int __stdcall BASS_SampleGetChannel(DWORD a, DWORD b) {
    if (real_BASS_SampleGetChannel) return real_BASS_SampleGetChannel(a, b);
    return 0;
}

/* ── Mod: Disable Pause ─────────────────────────────────────────────── */

static void apply_patches(void) {
    /* Get module base — Hamsterball.exe loads at 0x400000 */
    HMODULE hExe = GetModuleHandleA("Hamsterball.exe");
    if (!hExe) hExe = GetModuleHandleA(NULL);
    DWORD base = (DWORD)hExe;

    /* Patch: JZ → JMP at Scene_Update (0x419d5b)
     * Original: 74 09  (JZ  +0x09 → skip pause creation)
     * Patched:  EB 09  (JMP +0x09 → always skip pause creation)
     *
     * This is the check after Input_CheckKeyCombo(app, 2) returns.
     * If ESC was pressed (AL != 0), the original code falls through to
     * call Scene_CreateGameOverMenu. By changing JZ to JMP, we always
     * skip past the pause creation regardless of ESC state.
     */
    DWORD addr = base + 0x19d5b;  /* 0x419d5b */
    DWORD oldProt;
    if (VirtualProtect((void*)addr, 1, PAGE_READWRITE, &oldProt)) {
        *((BYTE*)addr) = 0xEB;  /* JZ (0x74) → JMP (0xEB) */
        VirtualProtect((void*)addr, 1, oldProt, &oldProt);
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
    real_BASS_ChannelSetFX          = (BASS_ChannelSetFX_t)GetProcAddress(g_hRealBass, "BASS_ChannelSetFX");
    real_BASS_FXSetParameters       = (BASS_FXSetParameters_t)GetProcAddress(g_hRealBass, "BASS_FXSetParameters");
    real_BASS_SampleLoad            = (BASS_SampleLoad_t)GetProcAddress(g_hRealBass, "BASS_SampleLoad");
    real_BASS_SampleGetChannel      = (BASS_SampleGetChannel_t)GetProcAddress(g_hRealBass, "BASS_SampleGetChannel");
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
