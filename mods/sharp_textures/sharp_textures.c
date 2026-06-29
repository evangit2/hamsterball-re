/*
 * sharp_textures — Force custom texture filtering by hooking
 *                  SetTextureStageState on the D3D8 device vtable.
 *
 * v5: Instead of setting filters once per frame at BeginFrame (which the game
 * overrides during rendering), this version patches the D3D8 device's vtable
 * to intercept EVERY SetTextureStageState call. When the game sets
 * MAGFILTER/MINFILTER/MIPFILTER, we override the value in real-time.
 *
 * For checker/brick textures, a SEPARATE filter profile is applied.
 * Texture identification: when the game calls SetTexture (vtable[61]),
 * we check if the texture pointer is in our tracked set (built by scanning
 * the texture cache for filenames containing "checker" or "brick").
 *
 * Config: sharp_textures.txt (next to bass.dll)
 *   MAGFILTER = 2          (default for all textures)
 *   MINFILTER = 2
 *   MIPFILTER = 2
 *   CHECKER_MAGFILTER = 1  (override for checker/brick textures)
 *   CHECKER_MINFILTER = 1
 *   CHECKER_MIPFILTER = 1
 *
 * Values: 0=none, 1=point(sharp), 2=linear(smooth), 3=anisotropic
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── BASS proxy exports ─────────────────────────────────────────────── */
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
#define D3DTEXF_POINT      1
#define D3DTEXF_LINEAR     2

/* D3D8 device vtable indices (verified from d3d8.h):
 *   60=GetTexture (0xF0), 61=SetTexture (0xF4)
 *   63=SetTextureStageState (0xFC) */
#define VTBL_GET_TEXTURE    60
#define VTBL_SET_TEXTURE    61
#define VTBL_SET_TSS        63

/* ── Config ─────────────────────────────────────────────────────────── */
static DWORD g_mag_filter = D3DTEXF_LINEAR;
static DWORD g_min_filter = D3DTEXF_LINEAR;
static DWORD g_mip_filter = D3DTEXF_LINEAR;
static DWORD g_checker_mag = D3DTEXF_POINT;
static DWORD g_checker_min = D3DTEXF_POINT;
static DWORD g_checker_mip = D3DTEXF_POINT;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved);

static void get_config_path(char* out, DWORD len) {
    HMODULE hSelf = NULL; char dll_path[MAX_PATH];
    if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCSTR)&DllMain, &hSelf) && hSelf) {
        GetModuleFileNameA(hSelf, dll_path, MAX_PATH);
        char* slash = strrchr(dll_path, '\\');
        if (slash) { slash[1] = '\0'; _snprintf(out, len, "%ssharp_textures.txt", dll_path); return; }
    }
    _snprintf(out, len, "sharp_textures.txt");
}

static void generate_default_config(const char* path) {
    FILE* f = NULL; if (fopen_s(&f, path, "w") != 0 || !f) return;
    fprintf(f, "# Sharp Textures config\n");
    fprintf(f, "# Values: 0=none, 1=point(sharp), 2=linear(smooth), 3=anisotropic\n\n");
    fprintf(f, "# Default filter for ALL textures:\nMAGFILTER = 2\nMINFILTER = 2\nMIPFILTER = 2\n\n");
    fprintf(f, "# Override for textures containing 'checker' or 'brick' in filename:\n");
    fprintf(f, "CHECKER_MAGFILTER = 1\nCHECKER_MINFILTER = 1\nCHECKER_MIPFILTER = 1\n");
    fclose(f);
}

static void read_config(void) {
    char path[MAX_PATH]; FILE* f = NULL; char line[256];
    g_mag_filter = D3DTEXF_LINEAR; g_min_filter = D3DTEXF_LINEAR; g_mip_filter = D3DTEXF_LINEAR;
    g_checker_mag = D3DTEXF_POINT; g_checker_min = D3DTEXF_POINT; g_checker_mip = D3DTEXF_POINT;
    get_config_path(path, MAX_PATH);
    if (fopen_s(&f, path, "r") != 0 || !f) { generate_default_config(path); return; }
    while (fgets(line, sizeof(line), f)) {
        char* p = line; while (*p == ' ' || *p == '\t') p++;
        if (*p == '#' || *p == '\n' || *p == '\r' || *p == '\0') continue;
        char* eq = strchr(p, '='); if (!eq) continue;
        long val = strtol(eq + 1, NULL, 10);
        if (_strnicmp(p, "CHECKER_MAGFILTER", 17) == 0) g_checker_mag = (DWORD)val;
        else if (_strnicmp(p, "CHECKER_MINFILTER", 17) == 0) g_checker_min = (DWORD)val;
        else if (_strnicmp(p, "CHECKER_MIPFILTER", 17) == 0) g_checker_mip = (DWORD)val;
        else if (_strnicmp(p, "MAGFILTER", 9) == 0) g_mag_filter = (DWORD)val;
        else if (_strnicmp(p, "MINFILTER", 9) == 0) g_min_filter = (DWORD)val;
        else if (_strnicmp(p, "MIPFILTER", 9) == 0) g_mip_filter = (DWORD)val;
    }
    fclose(f);
}

/* ── Game offsets ── */
#define GRAPHICS_BEGIN_FRAME  0x00453B50
#define OFF_D3D_DEVICE        0x154
#define GFX_TEX_COUNT         0x2E8
#define GFX_TEX_ARRAY          0x6F0
#define TEX_OBJ_D3D            0x04
#define TEX_OBJ_NAME           0x08

/* ── Tracked checker/brick texture pointers ── */
#define MAX_TRACKED_TEX  64
static void* g_tracked_tex[MAX_TRACKED_TEX];
static int g_tracked_count = 0;
static int g_last_tex_count = -1;

/* Currently bound texture per stage (updated by SetTexture hook) */
static void* g_current_tex[8] = {0};

static int is_checker_or_brick(const char* name) {
    char lower[260]; int i;
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
        char* tex_obj = (char*)tex_array[i];
        if (!tex_obj || IsBadReadPtr(tex_obj, 0x20)) continue;
        char* name = *(char**)(tex_obj + TEX_OBJ_NAME);
        if (is_checker_or_brick(name)) {
            void* d3d_tex = *(void**)(tex_obj + TEX_OBJ_D3D);
            if (d3d_tex && !IsBadReadPtr(d3d_tex, 4))
                g_tracked_tex[g_tracked_count++] = d3d_tex;
        }
    }
}

/* ── D3D8 vtable hook ── */
/* Original function pointers (saved before patching vtable) */
typedef int (__stdcall *SetTSS_t)(void*, DWORD, DWORD, DWORD);
typedef int (__stdcall *SetTexture_t)(void*, DWORD, void*);
static SetTSS_t   g_orig_SetTSS = NULL;
static SetTexture_t g_orig_SetTexture = NULL;
static int g_vtable_hooked = 0;

/* Hook for SetTexture: track which texture is bound to which stage */
static int __stdcall hook_SetTexture(void* device, DWORD stage, void* tex) {
    if (stage < 8) g_current_tex[stage] = tex;
    return g_orig_SetTexture(device, stage, tex);
}

/* Hook for SetTextureStageState: override filter values */
static int __stdcall hook_SetTSS(void* device, DWORD stage, DWORD type, DWORD value) {
    if (stage < 8 && (type == D3DTSS_MAGFILTER || type == D3DTSS_MINFILTER || type == D3DTSS_MIPFILTER)) {
        /* Only override for tracked checker/brick textures.
         * For everything else, pass through the game's original value. */
        int tracked = g_current_tex[stage] && is_tracked(g_current_tex[stage]);

        if (tracked) {
            if (type == D3DTSS_MAGFILTER)
                value = g_checker_mag;
            else if (type == D3DTSS_MINFILTER)
                value = g_checker_min;
            else /* MIPFILTER */
                value = g_checker_mip;
        }
    }
    return g_orig_SetTSS(device, stage, type, value);
}

/* Patch the D3D8 device vtable to intercept SetTexture and SetTextureStageState */
static void hook_d3d_vtable(int* device) {
    if (!device || IsBadReadPtr(device, 4)) return;
    int* vtable = *(int**)device;
    if (!vtable || IsBadReadPtr(vtable, (VTBL_SET_TSS + 1) * 4)) return;

    DWORD old_prot;

    /* Save originals */
    g_orig_SetTSS = (SetTSS_t)vtable[VTBL_SET_TSS];
    g_orig_SetTexture = (SetTexture_t)vtable[VTBL_SET_TEXTURE];
    if (!g_orig_SetTSS || !g_orig_SetTexture) return;

    /* Patch SetTextureStageState (vtable[63]) */
    if (VirtualProtect(&vtable[VTBL_SET_TSS], 4, PAGE_READWRITE, &old_prot)) {
        vtable[VTBL_SET_TSS] = (int)&hook_SetTSS;
        VirtualProtect(&vtable[VTBL_SET_TSS], 4, old_prot, &old_prot);
    }

    /* Patch SetTexture (vtable[61]) */
    if (VirtualProtect(&vtable[VTBL_SET_TEXTURE], 4, PAGE_READWRITE, &old_prot)) {
        vtable[VTBL_SET_TEXTURE] = (int)&hook_SetTexture;
        VirtualProtect(&vtable[VTBL_SET_TEXTURE], 4, old_prot, &old_prot);
    }

    g_vtable_hooked = 1;
}

/* ── BeginFrame detour (installs vtable hook on first call) ── */
static unsigned char* g_tramp = NULL;
static const int TRAMP_SIZE = 16;
static const unsigned char ORIG_PROLOGUE[7] = {
    0x53, 0x8B, 0xD9, 0x8B, 0x4C, 0x24, 0x08
};

static void __fastcall begin_frame_hook(void* this_, void* edx, int param_1) {
    typedef void (__fastcall *orig_fn_t)(void*, void*, int);
    ((orig_fn_t)g_tramp)(this_, edx, param_1);

    /* On first frame, hook the D3D8 vtable */
    if (!g_vtable_hooked) {
        char* gfx = (char*)this_;
        if (!IsBadReadPtr(gfx, 0x200)) {
            int* device = *(int**)(gfx + OFF_D3D_DEVICE);
            if (device && !IsBadReadPtr(device, 4))
                hook_d3d_vtable(device);
        }
    }

    /* Rescan texture cache when count changes */
    char* gfx = (char*)this_;
    if (!IsBadReadPtr(gfx + GFX_TEX_COUNT, 4)) {
        int current_count = *(int*)(gfx + GFX_TEX_COUNT);
        if (current_count != g_last_tex_count) {
            scan_textures(gfx);
            g_last_tex_count = current_count;
        }
    }
}

static void install_detour(void) {
    DWORD target = GRAPHICS_BEGIN_FRAME;
    DWORD old_prot;
    unsigned char jmp_patch[7];

    g_tramp = (unsigned char*)VirtualAlloc(NULL, TRAMP_SIZE,
                  MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!g_tramp) return;

    memcpy(g_tramp, ORIG_PROLOGUE, 7);
    g_tramp[7] = 0xE9;
    *(DWORD*)(g_tramp + 8) = (target + 7) - ((DWORD)g_tramp + 12);

    if (!VirtualProtect((void*)target, 7, PAGE_EXECUTE_READWRITE, &old_prot))
        return;

    jmp_patch[0] = 0xE9;
    *(DWORD*)(jmp_patch + 1) = (DWORD)&begin_frame_hook - (target + 5);
    jmp_patch[5] = 0x90;
    jmp_patch[6] = 0x90;

    memcpy((void*)target, jmp_patch, 7);
    VirtualProtect((void*)target, 7, old_prot, &old_prot);
    FlushInstructionCache(GetCurrentProcess(), (void*)target, 7);
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
            read_config();
            init_bass_proxy();
            install_detour();
            break;
    }
    return TRUE;
}
