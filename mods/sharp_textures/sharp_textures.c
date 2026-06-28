/*
 * sharp_textures — Force sharp (point/nearest) texture filtering,
 *                  with separate override for Checker/Brick textures.
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
 * Additionally, textures whose filenames contain "checker" or "brick"
 * get a SEPARATE filter setting that overrides the default. This lets
 * you keep checker/brick textures smooth while making everything else
 * sharp, or vice versa.
 *
 * Approach: Hook Graphics_BeginFrame (0x453B50), called every frame.
 * After the original runs, grab the D3D8 device from Graphics+0x154
 * and iterate the texture cache at Graphics+0x2E4 to identify
 * checker/brick textures by filename, then use GetTexture (vtable[61])
 * per stage to check which texture is bound and apply the right filter.
 *
 * Config: sharp_textures.txt (next to bass.dll)
 *   MAGFILTER = 1          (default for all textures)
 *   MINFILTER = 1
 *   MIPFILTER = 1
 *   CHECKER_MAGFILTER = 2  (override for checker/brick textures)
 *   CHECKER_MINFILTER = 2
 *   CHECKER_MIPFILTER = 2
 *
 * Values: 0=none, 1=point(sharp), 2=linear(smooth), 3=anisotropic
 * If config is missing, defaults are generated automatically.
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

/* D3D8 device vtable indices */
#define VTBL_GET_TEXTURE           61   /* offset 0xF4 */
#define VTBL_SET_TEXTURE           62   /* offset 0xF8 */
#define VTBL_SET_TEX_STAGE_STATE   63   /* offset 0xFC */
#define VTBL_RELEASE               2    /* on texture object, not device */

/* ── Config: default filters (all textures) ── */
static DWORD g_mag_filter = D3DTEXF_POINT;
static DWORD g_min_filter = D3DTEXF_POINT;
static DWORD g_mip_filter = D3DTEXF_POINT;

/* ── Config: checker/brick override filters ── */
static DWORD g_checker_mag = D3DTEXF_LINEAR;
static DWORD g_checker_min = D3DTEXF_LINEAR;
static DWORD g_checker_mip = D3DTEXF_LINEAR;

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
    fprintf(f, "# Default filter for ALL textures:\n");
    fprintf(f, "MAGFILTER = 1\n");
    fprintf(f, "MINFILTER = 1\n");
    fprintf(f, "MIPFILTER = 1\n\n");
    fprintf(f, "# Override for textures containing 'checker' or 'brick' in filename:\n");
    fprintf(f, "CHECKER_MAGFILTER = 2\n");
    fprintf(f, "CHECKER_MINFILTER = 2\n");
    fprintf(f, "CHECKER_MIPFILTER = 2\n");
    fclose(f);
}

/* Parse "KEY = value" lines; returns value or -1 if not found */
static long parse_value(const char* line, const char* key) {
    if (_strnicmp(line, key, strlen(key)) != 0) return -1;
    char* eq = strchr(line, '=');
    if (!eq) return -1;
    return strtol(eq + 1, NULL, 10);
}

static void read_config(void) {
    char path[MAX_PATH];
    FILE* f = NULL;
    char line[256];

    /* Defaults */
    g_mag_filter = D3DTEXF_POINT;
    g_min_filter = D3DTEXF_POINT;
    g_mip_filter = D3DTEXF_POINT;
    g_checker_mag = D3DTEXF_LINEAR;
    g_checker_min = D3DTEXF_LINEAR;
    g_checker_mip = D3DTEXF_LINEAR;

    get_config_path(path, MAX_PATH);
    if (fopen_s(&f, path, "r") != 0 || !f) {
        generate_default_config(path);
        return;
    }

    while (fgets(line, sizeof(line), f)) {
        char* p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '#' || *p == '\n' || *p == '\r' || *p == '\0') continue;

        long val;
        if ((val = parse_value(p, "CHECKER_MAGFILTER")) >= 0) g_checker_mag = (DWORD)val;
        else if ((val = parse_value(p, "CHECKER_MINFILTER")) >= 0) g_checker_min = (DWORD)val;
        else if ((val = parse_value(p, "CHECKER_MIPFILTER")) >= 0) g_checker_mip = (DWORD)val;
        else if ((val = parse_value(p, "MAGFILTER")) >= 0) g_mag_filter = (DWORD)val;
        else if ((val = parse_value(p, "MINFILTER")) >= 0) g_min_filter = (DWORD)val;
        else if ((val = parse_value(p, "MIPFILTER")) >= 0) g_mip_filter = (DWORD)val;
    }
    fclose(f);
}

/* ── Game offsets ── */
#define GRAPHICS_BEGIN_FRAME  0x00453B50

/* Graphics struct offsets */
#define OFF_D3D_DEVICE        0x154
#define GFX_TEX_COUNT         0x2E8   /* int: number of cached textures */
#define GFX_TEX_ARRAY         0x6F0   /* int**: array of texture object pointers */

/* Texture object offsets (0x74 bytes each, vtable 0x4DA648) */
#define TEX_OBJ_D3D           0x04   /* IDirect3DTexture8* */
#define TEX_OBJ_NAME           0x08   /* char* filename */

/* ── Tracked checker/brick texture pointers ── */
#define MAX_TRACKED_TEX  64
static void* g_tracked_tex[MAX_TRACKED_TEX];
static int g_tracked_count = 0;
static int g_last_tex_count = -1;  /* rescan when count changes */

/* Case-insensitive check: does filename contain "checker" or "brick"? */
static int is_checker_or_brick(const char* name) {
    char lower[260];
    int i;
    if (!name || IsBadReadPtr(name, 1)) return 0;
    for (i = 0; i < 259 && name[i]; i++)
        lower[i] = (name[i] >= 'A' && name[i] <= 'Z') ? (char)(name[i] + 32) : name[i];
    lower[i] = '\0';
    return (strstr(lower, "checker") != NULL || strstr(lower, "brick") != NULL);
}

static int is_tracked(void* tex) {
    int i;
    for (i = 0; i < g_tracked_count; i++)
        if (g_tracked_tex[i] == tex) return 1;
    return 0;
}

/* Scan Graphics texture cache for checker/brick textures */
static void scan_textures(char* gfx) {
    g_tracked_count = 0;

    if (IsBadReadPtr(gfx + GFX_TEX_COUNT, 4)) return;
    int count = *(int*)(gfx + GFX_TEX_COUNT);
    if (count <= 0 || count > 512) return;

    if (IsBadReadPtr(gfx + GFX_TEX_ARRAY, 4)) return;
    int** tex_array = *(int***)(gfx + GFX_TEX_ARRAY);
    if (!tex_array || IsBadReadPtr(tex_array, count * sizeof(int*))) return;

    int i;
    for (i = 0; i < count && g_tracked_count < MAX_TRACKED_TEX; i++) {
        int* tex_obj = tex_array[i];
        if (!tex_obj || IsBadReadPtr(tex_obj, 0x20)) continue;

        char* name = *(char**)(tex_obj + (TEX_OBJ_NAME / sizeof(int)));
        if (is_checker_or_brick(name)) {
            void* d3d_tex = *(void**)(tex_obj + (TEX_OBJ_D3D / sizeof(int)));
            if (d3d_tex && !IsBadReadPtr(d3d_tex, 4)) {
                g_tracked_tex[g_tracked_count++] = d3d_tex;
            }
        }
    }
}

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

/* D3D8 COM method typedefs — all __stdcall (this on stack) */
typedef int (__stdcall *SetTSS_t)(void*, DWORD, DWORD, DWORD);
typedef int (__stdcall *GetTexture_t)(void*, DWORD, void**);
typedef unsigned long (__stdcall *Release_t)(void*);

/* ── Hook function ── */
/* Graphics_BeginFrame is __thiscall: this=ECX, param_1=[ESP+4] (after return addr) */
/* It has RET 0x4 — one stack parameter (4 bytes) */
static void __fastcall begin_frame_hook(void* this_, void* edx, int param_1) {
    /* Call original via trampoline */
    typedef void (__fastcall *orig_fn_t)(void*, void*, int);
    ((orig_fn_t)g_tramp)(this_, edx, param_1);

    /* Get D3D8 device from Graphics+0x154 */
    char* gfx = (char*)this_;
    if (IsBadReadPtr(gfx, 0x200)) return;

    int* device = *(int**)(gfx + OFF_D3D_DEVICE);
    if (!device || IsBadReadPtr(device, 4)) return;

    int* vtable = *(int**)device;
    if (!vtable || IsBadReadPtr(vtable, (VTBL_SET_TEX_STAGE_STATE + 1) * 4)) return;

    SetTSS_t pSetTSS = (SetTSS_t)vtable[VTBL_SET_TEX_STAGE_STATE];
    GetTexture_t pGetTex = (GetTexture_t)vtable[VTBL_GET_TEXTURE];
    if (!pSetTSS || !pGetTex) return;

    /* Rescan texture cache if count changed (level load, etc.) */
    if (!IsBadReadPtr(gfx + GFX_TEX_COUNT, 4)) {
        int current_count = *(int*)(gfx + GFX_TEX_COUNT);
        if (current_count != g_last_tex_count) {
            scan_textures(gfx);
            g_last_tex_count = current_count;
        }
    }

    /* For each stage: check current texture, apply appropriate filter */
    int stage;
    for (stage = 0; stage < 8; stage++) {
        void* current_tex = NULL;

        /* GetTexture increments ref count — we must Release after */
        pGetTex(device, stage, &current_tex);

        DWORD mag, min, mip;
        if (current_tex && is_tracked(current_tex)) {
            /* Checker/brick texture: use override filter */
            mag = g_checker_mag;
            min = g_checker_min;
            mip = g_checker_mip;
        } else {
            /* Normal texture: use default filter */
            mag = g_mag_filter;
            min = g_min_filter;
            mip = g_mip_filter;
        }

        pSetTSS(device, stage, D3DTSS_MAGFILTER, mag);
        pSetTSS(device, stage, D3DTSS_MINFILTER, min);
        pSetTSS(device, stage, D3DTSS_MIPFILTER, mip);

        /* Release the ref that GetTexture added */
        if (current_tex && !IsBadReadPtr(current_tex, 4)) {
            int* tex_vtable = *(int**)current_tex;
            if (tex_vtable && !IsBadReadPtr(tex_vtable, 12)) {
                Release_t pRelease = (Release_t)tex_vtable[VTBL_RELEASE];
                if (pRelease) pRelease(current_tex);
            }
        }
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
