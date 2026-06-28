/*
 * neon_green_glow — Changes Neon Race glow from yellow to green.
 *
 * Patches 8 R-channel PUSH imm32 immediates in Scene_SetupLevelDark (0x416270).
 * Each PUSH pushes 1.0f (0x3F800000) for the R channel. We zero them to make
 * RGBA = (0, 1, 0, 1) = green instead of (1, 1, 0, 1) = yellow.
 *
 * Patches:
 *   Loop:  platform colors (2 addresses)
 *   P1:    Ambient, Diffuse, Emissive (3 addresses)
 *   P2:    Ambient, Diffuse, Emissive (3 addresses)
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

/* ── Mod: Neon Green Glow ─────────────────────────────────────────── */

/* 8 R-channel push immediates in Scene_SetupLevelDark (0x416270).
 * Each is at file offset (vaddr - 0x400000) in the .text section.
 * We write 0x00000000 (0.0f) over the original 0x3F800000 (1.0f).
 *
 * The PUSH instruction is: 68 XX XX XX XX (5 bytes)
 * The immediate starts at offset+1. We patch the 4-byte immediate only.
 */
static const struct {
    DWORD rva;       /* RVA from module base (vaddr - 0x400000) */
    const char* desc;
} patches[] = {
    { 0x00016360, "Loop block 1 (platforms R)" },
    { 0x000163D4, "Loop block 2 (platforms R)" },
    { 0x00016568, "P1 Ambient R (phys+0x1CC)" },
    { 0x000165D5, "P1 Diffuse R (phys+0x1BC)" },
    { 0x0001663F, "P1 Emissive R (phys+0x1EC)" },
    { 0x000166C0, "P2 Ambient R (phys2+0x1CC)" },
    { 0x0001672D, "P2 Diffuse R (phys2+0x1BC)" },
    { 0x00016797, "P2 Emissive R (phys2+0x1EC)" },
};

static void apply_patches(void) {
    HMODULE hExe = GetModuleHandleA("Hamsterball.exe");
    if (!hExe) hExe = GetModuleHandleA(NULL);
    DWORD base = (DWORD)hExe;
    DWORD oldProt;
    int i;

    for (i = 0; i < 8; i++) {
        /* The immediate is at base + rva (which is already imm offset, not opcode) */
        DWORD addr = base + patches[i].rva;
        if (VirtualProtect((void*)addr, 4, PAGE_READWRITE, &oldProt)) {
            *((DWORD*)addr) = 0x00000000;  /* 0.0f — zero the R channel */
            VirtualProtect((void*)addr, 4, oldProt, &oldProt);
            FlushInstructionCache(GetCurrentProcess(), (void*)addr, 4);
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
