/*
 * sharp_textures — Force custom texture filtering in Hamsterball.
 *
 * In D3D8, texture filtering is controlled via SetTextureStageState:
 *   D3DTSS_MAGFILTER = 16  (magnification filter)
 *   D3DTSS_MINFILTER = 17  (minification filter)
 *   D3DTSS_MIPFILTER = 18  (mipmap filter)
 * Values: 0=NONE, 1=POINT (sharp), 2=LINEAR (smooth/blurry), 3=ANISOTROPIC
 *
 * The game uses LINEAR (2) by default, producing blurry textures.
 * This mod forces custom filtering on all 8 stages each frame.
 *
 * Approach: Hook Graphics_BeginFrame (0x453B50), called every frame.
 * After the original runs, grab the D3D8 device from Graphics+0x154
 * and call SetTextureStageState (vtable[63], offset 0xFC) on all 8 stages.
 *
 * Config: sharp_textures.txt (next to bass.dll)
 *   MAGFILTER = 1   (0=none, 1=point/sharp, 2=linear/smooth, 3=anisotropic)
 *   MINFILTER = 1
 *   MIPFILTER = 1
 *
 * If config is missing, defaults to all POINT (1) and auto-generates the file.
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

/* ── D3D8 constants ─────────────────────────────────────────────────── */
#define D3DTSS_MAGFILTER   16
#define D3DTSS_MINFILTER   17
#define D3DTSS_MIPFILTER   18

#define D3DTEXF_NONE       0
#define D3DTEXF_POINT      1   /* sharp / nearest */
#define D3DTEXF_LINEAR     2   /* smooth / blurry */
#define D3DTEXF_ANISOTROPIC 3

/* D3D8 device vtable: SetTextureStageState is at index 63 (offset 0xFC) */
#define VTBL_SET_TEX_STAGE_STATE  63

/* ── Config ─────────────────────────────────────────────────────────── */
static DWORD g_mag_filter = D3DTEXF_POINT;
static DWORD g_min_filter = D3DTEXF_POINT;
static DWORD g_mip_filter = D3DTEXF_POINT;

/* Forward declare DllMain for GetModuleHandleEx path detection */
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved);

static void get_config_path(char* out, DWORD len) {
    HMODULE hSelf = NULL;
    char dll_path[MAX_PATH];
    if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
                           (LPCSTR)&DllMain, &hSelf) && hSelf) {
        GetModuleFileNameA(hSelf, dll_path, MAX_PATH);
        char* slash = strrchr(dll_path, '\\');
        if (slash) {
            slash[1] = '\0';
            _snprintf(out, len, "%ssharp_textures.txt", dll_path);
            return;
        }
    }
    _snprintf(out, len, "sharp_textures.txt");
}

static void generate_default_config(const char* path) {
    FILE* f = NULL;
    if (fopen_s(&f, path, "w") != 0 || !f) return;
    fprintf(f, "# Sharp Textures config\n");
    fprintf(f, "# Values: 0=none, 1=point(sharp), 2=linear(smooth), 3=anisotropic\n\n");
    fprintf(f, "MAGFILTER = 1\n");
    fprintf(f, "MINFILTER = 1\n");
    fprintf(f, "MIPFILTER = 1\n");
    fclose(f);
}

static void read_config(void) {
    char path[MAX_PATH];
    FILE* f = NULL;
    char line[256];

    g_mag_filter = D3DTEXF_POINT;
    g_min_filter = D3DTEXF_POINT;
    g_mip_filter = D3DTEXF_POINT;

    get_config_path(path, MAX_PATH);
    if (fopen_s(&f, path, "r") != 0 || !f) {
        generate_default_config(path);
        return;
    }

    while (fgets(line, sizeof(line), f)) {
        char* p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '#' || *p == '\n' || *p == '\r' || *p == '\0') continue;

        if (_strnicmp(p, "MAGFILTER", 9) == 0) {
            char* eq = strchr(p, '=');
            if (eq) g_mag_filter = (DWORD)strtoul(eq + 1, NULL, 10);
        } else if (_strnicmp(p, "MINFILTER", 9) == 0) {
            char* eq = strchr(p, '=');
            if (eq) g_min_filter = (DWORD)strtoul(eq + 1, NULL, 10);
        } else if (_strnicmp(p, "MIPFILTER", 9) == 0) {
            char* eq = strchr(p, '=');
            if (eq) g_mip_filter = (DWORD)strtoul(eq + 1, NULL, 10);
        }
    }
    fclose(f);
}

/* ── Game offsets ── */
#define GRAPHICS_BEGIN_FRAME  0x00453B50

/* Graphics struct: D3D8 device pointer at +0x154 */
#define OFF_D3D_DEVICE        0x154

/* ── Detour infrastructure ── */
static unsigned char* g_tramp = NULL;
static const int TRAMP_SIZE = 16;

/*
 * Graphics_BeginFrame prologue (7 bytes):
 *   00453b50: 53              PUSH EBX
 *   00453b51: 8B D9           MOV EBX,ECX
 *   00453b53: 8B 4C 24 08     MOV ECX,[ESP+8]
 */
static const unsigned char ORIG_PROLOGUE[7] = {
    0x53, 0x8B, 0xD9, 0x8B, 0x4C, 0x24, 0x08
};

/* ── Hook function ── */
/* Graphics_BeginFrame is __thiscall: this=ECX, param_1=[ESP+4] (after return addr) */
/* It has RET 0x4 — one stack parameter (4 bytes) */
static void __fastcall begin_frame_hook(void* this_, void* edx, int param_1) {
    /* Call original via trampoline */
    typedef void (__fastcall *orig_fn_t)(void*, void*, int);
    ((orig_fn_t)g_tramp)(this_, edx, param_1);

    /* Get D3D8 device from Graphics+0x154 */
    int* graphics = (int*)this_;
    if (IsBadReadPtr(graphics, 0x200)) return;

    int* device = *(int**)((char*)graphics + OFF_D3D_DEVICE);
    if (!device || IsBadReadPtr(device, 4)) return;

    /* device points to the D3D8 device object. Its first DWORD is the vtable pointer. */
    int* vtable = *(int**)device;
    if (!vtable || IsBadReadPtr(vtable, (VTBL_SET_TEX_STAGE_STATE + 1) * 4)) return;

    /* SetTextureStageState is vtable[63] (offset 0xFC) */
    /* D3D8 COM methods use __stdcall: this pointer pushed on stack as first param */
    typedef int (__stdcall *SetTSS_t)(void*, DWORD, DWORD, DWORD);
    SetTSS_t pSetTSS = (SetTSS_t)vtable[VTBL_SET_TEX_STAGE_STATE];
    if (!pSetTSS) return;

    /* Apply to all 8 texture stages (D3D8 supports up to 8) */
    int stage;
    for (stage = 0; stage < 8; stage++) {
        pSetTSS(device, stage, D3DTSS_MAGFILTER, g_mag_filter);
        pSetTSS(device, stage, D3DTSS_MINFILTER, g_min_filter);
        pSetTSS(device, stage, D3DTSS_MIPFILTER, g_mip_filter);
    }
}

/* ── Install detour ── */
static void install_detour(void) {
    DWORD target = GRAPHICS_BEGIN_FRAME;
    DWORD old_prot;
    unsigned char jmp_patch[7];

    /* Allocate executable trampoline */
    g_tramp = (unsigned char*)VirtualAlloc(NULL, TRAMP_SIZE,
                  MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!g_tramp) return;

    /* Trampoline: original prologue (7 bytes) + JMP back to target+7 (5 bytes) */
    memcpy(g_tramp, ORIG_PROLOGUE, 7);
    g_tramp[7] = 0xE9; /* JMP rel32 */
    {
        DWORD rel = (target + 7) - ((DWORD)g_tramp + 7 + 5);
        *(DWORD*)(g_tramp + 8) = rel;
    }

    /* Patch original: JMP to hook + 2 NOPs (total 7 bytes) */
    if (!VirtualProtect((void*)target, 7, PAGE_EXECUTE_READWRITE, &old_prot))
        return;

    jmp_patch[0] = 0xE9; /* JMP rel32 */
    {
        DWORD rel = (DWORD)&begin_frame_hook - (target + 5);
        *(DWORD*)(jmp_patch + 1) = rel;
    }
    jmp_patch[5] = 0x90; /* NOP */
    jmp_patch[6] = 0x90; /* NOP */

    memcpy((void*)target, jmp_patch, 7);
    VirtualProtect((void*)target, 7, old_prot, &old_prot);
    FlushInstructionCache(GetCurrentProcess(), (void*)target, 7);
}

/* ── BASS proxy init ── */
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

/* ── DLL entry ── */
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    switch (fdwReason) {
        case DLL_PROCESS_ATTACH:
            read_config();
            init_bass_proxy();
            install_detour();
            break;
    }
    return TRUE;
}
