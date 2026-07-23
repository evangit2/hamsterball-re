/*
 * SharpTextures_MinGW.cpp — Force custom texture filtering by hooking
 *                           SetTextureStageState on the D3D8 device vtable.
 *
 * HB+ port of sharp_textures bass.dll proxy mod.
 *
 * Patches the D3D8 device's vtable to intercept EVERY SetTextureStageState
 * and SetTexture call. When the game sets MAGFILTER/MINFILTER/MIPFILTER,
 * we override the value in real-time.
 *
 * For checker/brick textures, a SEPARATE filter profile is applied.
 *
 * Config: sharp_textures.txt (next to the DLL in Mods\)
 *   MAGFILTER = 2          (default for all textures)
 *   MINFILTER = 2
 *   MIPFILTER = 2
 *   CHECKER_MAGFILTER = 1  (override for checker/brick textures)
 *   CHECKER_MINFILTER = 1
 *   CHECKER_MIPFILTER = 1
 *
 * Values: 0=none, 1=point(sharp), 2=linear(smooth), 3=anisotropic
 *
 * Uses nocrt + manual 17-entry vtable for HB+ v2.1 compatibility.
 */
#include "nocrt.h"
#include "HamsterballAPI.h"
#include "hbplus_api.h"

/* ── D3D8 constants ─────────────────────────────────────────────────── */
#define D3DTSS_MAGFILTER   16
#define D3DTSS_MINFILTER   17
#define D3DTSS_MIPFILTER   18
#define D3DTEXF_POINT      1
#define D3DTEXF_LINEAR     2

/* D3D8 device vtable indices */
#define VTBL_GET_TEXTURE    60
#define VTBL_SET_TEXTURE    61
#define VTBL_SET_TSS        63

/* ── Game offsets ── */
#define GRAPHICS_BEGIN_FRAME  0x00453B50
#define OFF_D3D_DEVICE        0x154
#define GFX_TEX_COUNT         0x2E8
#define GFX_TEX_ARRAY         0x6F0
#define TEX_OBJ_D3D           0x04
#define TEX_OBJ_NAME          0x08

/* ── Config ─────────────────────────────────────────────────────────── */
static DWORD g_mag_filter = D3DTEXF_LINEAR;
static DWORD g_min_filter = D3DTEXF_LINEAR;
static DWORD g_mip_filter = D3DTEXF_LINEAR;
static DWORD g_checker_mag = D3DTEXF_POINT;
static DWORD g_checker_min = D3DTEXF_POINT;
static DWORD g_checker_mip = D3DTEXF_POINT;

static char g_configPath[MAX_PATH] = "";
static bool g_pathReady = false;

static void* g_storedApi = NULL;

/* ── Config path: relative to THIS DLL ──────────────────────────────── */

static void buildConfigPath(void) {
    MEMORY_BASIC_INFORMATION mbi;
    char dllPath[MAX_PATH];
    if (VirtualQuery((void*)buildConfigPath, &mbi, sizeof(mbi)) > 0) {
        HMODULE hMod = (HMODULE)mbi.AllocationBase;
        if (hMod && GetModuleFileNameA(hMod, dllPath, MAX_PATH) > 0) {
            char* last = NULL;
            char* p = dllPath;
            while (*p) {
                if (*p == '\\' || *p == '/') last = p;
                p++;
            }
            if (last) {
                *(last + 1) = '\0';
                nc_strncpy(g_configPath, dllPath, MAX_PATH - 1);
                nc_strncpy(g_configPath + nc_strlen(g_configPath),
                           "sharp_textures.txt",
                           MAX_PATH - nc_strlen(g_configPath) - 1);
                g_configPath[MAX_PATH - 1] = '\0';
                g_pathReady = true;
                return;
            }
        }
    }
    nc_strncpy(g_configPath, "sharp_textures.txt", MAX_PATH - 1);
    g_configPath[MAX_PATH - 1] = '\0';
    g_pathReady = true;
}

static char to_lower_c(char c) { return (c >= 'A' && c <= 'Z') ? (c + 32) : c; }

static int nc_strnicmp3(const char *a, const char *b, int n) {
    for (int i = 0; i < n; i++) {
        char ca = to_lower_c(a[i]);
        char cb = to_lower_c(b[i]);
        if (ca != cb) return (unsigned char)ca - (unsigned char)cb;
        if (!a[i]) return 0;
    }
    return 0;
}

static char* nc_strstr(const char *haystack, const char *needle) {
    int nl = (int)nc_strlen(needle);
    int hl = (int)nc_strlen(haystack);
    for (int i = 0; i <= hl - nl; i++) {
        int j;
        for (j = 0; j < nl; j++) {
            if (haystack[i + j] != needle[j]) break;
        }
        if (j == nl) return (char*)(haystack + i);
    }
    return NULL;
}

static int nc_stristr_local(const char *haystack, const char *needle) {
    char lower[260];
    int i;
    if (!haystack || IsBadReadPtr((void*)haystack, 1)) return 0;
    for (i = 0; i < 259 && haystack[i]; i++)
        lower[i] = to_lower_c(haystack[i]);
    lower[i] = '\0';
    /* Lowercase the needle manually */
    char lneedle[64];
    for (i = 0; i < 63 && needle[i]; i++)
        lneedle[i] = to_lower_c(needle[i]);
    lneedle[i] = '\0';
    return nc_strstr(lower, lneedle) != NULL;
}

static int nc_atoi_str(const char *s) {
    while (*s == ' ' || *s == '\t') s++;
    int val = 0;
    while (*s >= '0' && *s <= '9') { val = val * 10 + (*s - '0'); s++; }
    return val;
}

static char* nc_strchr(const char *s, char c) {
    while (*s) {
        if (*s == c) return (char*)s;
        s++;
    }
    return NULL;
}

/* ── Config reading ─────────────────────────────────────────────────── */

static void generate_default_config(const char* path) {
    HANDLE h = CreateFileA(path, GENERIC_WRITE, 0, NULL,
                           CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return;
    const char* hdr1 = "# Sharp Textures config\r\n"
                        "# Values: 0=none, 1=point(sharp), 2=linear(smooth), 3=anisotropic\r\n\r\n"
                        "# Default filter for ALL textures:\r\nMAGFILTER = 2\r\nMINFILTER = 2\r\nMIPFILTER = 2\r\n\r\n"
                        "# Override for textures containing 'checker' or 'brick' in filename:\r\n";
    const char* hdr2 = "CHECKER_MAGFILTER = 1\r\nCHECKER_MINFILTER = 1\r\nCHECKER_MIPFILTER = 1\r\n";
    DWORD written;
    WriteFile(h, hdr1, (DWORD)nc_strlen(hdr1), &written, NULL);
    WriteFile(h, hdr2, (DWORD)nc_strlen(hdr2), &written, NULL);
    CloseHandle(h);
}

static void read_config(void) {
    if (!g_pathReady) buildConfigPath();

    g_mag_filter = D3DTEXF_LINEAR; g_min_filter = D3DTEXF_LINEAR; g_mip_filter = D3DTEXF_LINEAR;
    g_checker_mag = D3DTEXF_POINT; g_checker_min = D3DTEXF_POINT; g_checker_mip = D3DTEXF_POINT;

    HANDLE h = CreateFileA(g_configPath, GENERIC_READ, FILE_SHARE_READ, NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) {
        generate_default_config(g_configPath);
        return;
    }

    char buf[4096];
    DWORD bytesRead = 0;
    ReadFile(h, buf, sizeof(buf) - 1, &bytesRead, NULL);
    CloseHandle(h);
    if (bytesRead == 0) return;
    buf[bytesRead] = '\0';

    char *p = buf;
    while (*p) {
        char *nl = p;
        while (*nl && *nl != '\n') nl++;
        char saved = *nl;
        *nl = '\0';

        char *line = p;
        while (*line == ' ' || *line == '\t') line++;
        if (*line == '#' || *line == '\n' || *line == '\r' || *line == '\0') {
            *nl = saved;
            if (*nl == '\n') nl++;
            p = nl;
            continue;
        }

        char *eq = nc_strchr(line, '=');
        if (!eq) { *nl = saved; if (*nl == '\n') nl++; p = nl; continue; }
        *eq = '\0';
        char *key = line;
        char *val = eq + 1;
        while (*val == ' ' || *val == '\t') val++;
        long val_i = nc_atoi_str(val);

        if (nc_strnicmp3(key, "CHECKER_MAGFILTER", 17) == 0) g_checker_mag = (DWORD)val_i;
        else if (nc_strnicmp3(key, "CHECKER_MINFILTER", 17) == 0) g_checker_min = (DWORD)val_i;
        else if (nc_strnicmp3(key, "CHECKER_MIPFILTER", 17) == 0) g_checker_mip = (DWORD)val_i;
        else if (nc_strnicmp3(key, "MAGFILTER", 9) == 0) g_mag_filter = (DWORD)val_i;
        else if (nc_strnicmp3(key, "MINFILTER", 9) == 0) g_min_filter = (DWORD)val_i;
        else if (nc_strnicmp3(key, "MIPFILTER", 9) == 0) g_mip_filter = (DWORD)val_i;

        *nl = saved;
        if (*nl == '\n') nl++;
        p = nl;
    }
}

/* ── Tracked checker/brick texture pointers ── */
#define MAX_TRACKED_TEX  64
static void* g_tracked_tex[MAX_TRACKED_TEX];
static int g_tracked_count = 0;
static int g_last_tex_count = -1;

/* Currently bound texture per stage (updated by SetTexture hook) */
static void* g_current_tex[8] = {NULL};

static int is_checker_or_brick(const char* name) {
    return nc_stristr_local(name, "checker") || nc_stristr_local(name, "brick");
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
    DWORD* tex_array = *(DWORD**)(gfx + GFX_TEX_ARRAY);
    if (!tex_array || IsBadReadPtr(tex_array, count * sizeof(DWORD))) return;
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
typedef int (__stdcall *SetTSS_t)(void*, DWORD, DWORD, DWORD);
typedef int (__stdcall *SetTexture_t)(void*, DWORD, void*);
static SetTSS_t   g_orig_SetTSS = NULL;
static SetTexture_t g_orig_SetTexture = NULL;
static int g_vtable_hooked = 0;

static int __stdcall hook_SetTexture(void* device, DWORD stage, void* tex) {
    int result = g_orig_SetTexture(device, stage, tex);

    if (stage < 8) {
        g_current_tex[stage] = tex;

        DWORD mag, minf, mip;
        if (tex && is_tracked(tex)) {
            mag = g_checker_mag;
            minf = g_checker_min;
            mip = g_checker_mip;
        } else {
            mag = g_mag_filter;
            minf = g_min_filter;
            mip = g_mip_filter;
        }
        g_orig_SetTSS(device, stage, D3DTSS_MAGFILTER, mag);
        g_orig_SetTSS(device, stage, D3DTSS_MINFILTER, minf);
        g_orig_SetTSS(device, stage, D3DTSS_MIPFILTER, mip);
    }
    return result;
}

static int __stdcall hook_SetTSS(void* device, DWORD stage, DWORD type, DWORD value) {
    if (stage < 8 && (type == D3DTSS_MAGFILTER || type == D3DTSS_MINFILTER || type == D3DTSS_MIPFILTER)) {
        void* tex = g_current_tex[stage];
        if (tex && is_tracked(tex)) {
            if (type == D3DTSS_MAGFILTER)
                value = g_checker_mag;
            else if (type == D3DTSS_MINFILTER)
                value = g_checker_min;
            else
                value = g_checker_mip;
        }
    }
    return g_orig_SetTSS(device, stage, type, value);
}

static void hook_d3d_vtable(void* device) {
    if (!device || IsBadReadPtr(device, 4)) return;
    DWORD* vtable = *(DWORD**)device;
    if (!vtable || IsBadReadPtr(vtable, (VTBL_SET_TSS + 1) * 4)) return;

    DWORD old_prot;

    g_orig_SetTSS = (SetTSS_t)vtable[VTBL_SET_TSS];
    g_orig_SetTexture = (SetTexture_t)vtable[VTBL_SET_TEXTURE];
    if (!g_orig_SetTSS || !g_orig_SetTexture) return;

    if (VirtualProtect(&vtable[VTBL_SET_TSS], 4, PAGE_READWRITE, &old_prot)) {
        vtable[VTBL_SET_TSS] = (DWORD)&hook_SetTSS;
        VirtualProtect(&vtable[VTBL_SET_TSS], 4, old_prot, &old_prot);
    }

    if (VirtualProtect(&vtable[VTBL_SET_TEXTURE], 4, PAGE_READWRITE, &old_prot)) {
        vtable[VTBL_SET_TEXTURE] = (DWORD)&hook_SetTexture;
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

    if (!g_vtable_hooked) {
        char* gfx = (char*)this_;
        if (!IsBadReadPtr(gfx, 0x200)) {
            void* device = *(void**)(gfx + OFF_D3D_DEVICE);
            if (device && !IsBadReadPtr(device, 4))
                hook_d3d_vtable(device);
        }
    }

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

    nc_memcpy(g_tramp, ORIG_PROLOGUE, 7);
    g_tramp[7] = 0xE9;
    *(DWORD*)(g_tramp + 8) = (target + 7) - ((DWORD)g_tramp + 12);

    if (!VirtualProtect((void*)target, 7, PAGE_EXECUTE_READWRITE, &old_prot))
        return;

    jmp_patch[0] = 0xE9;
    *(DWORD*)(jmp_patch + 1) = (DWORD)&begin_frame_hook - (target + 5);
    jmp_patch[5] = 0x90;
    jmp_patch[6] = 0x90;

    nc_memcpy((void*)target, jmp_patch, 7);
    VirtualProtect((void*)target, 7, old_prot, &old_prot);
    FlushInstructionCache(GetCurrentProcess(), (void*)target, 7);
}

/* ── Vtable implementations ─────────────────────────────────────────── */

static void* __thiscall sc_dtor(void* thisptr, int flags) {
    if (flags & 1) nc_free(thisptr);
    return thisptr;
}

static const char* __thiscall get_mod_name(void*) { return "Sharp Textures"; }
static const char* __thiscall get_author(void*) { return "MAKYUNI"; }
static int __thiscall get_version(void*) { return HAMSTERBALL_API_VERSION; }
static const char* __thiscall get_contributors(void*) { return "Ported to HB+ by Hamsterbot"; }

static void __thiscall init_impl(void* thisptr, void* modApi) {
    *(void**)((char*)thisptr + 4) = modApi;
    g_storedApi = modApi;
    buildConfigPath();
    read_config();
    install_detour();
}

/* No-op implementations for unused callbacks */
static void __thiscall ball_update_impl(void*, void*) {}
static void __thiscall render_apply_impl(void*, void*, float*) {}
static void __thiscall button_toggle_impl(void*, const char*, bool) {}
static void __thiscall slider_change_impl(void*, const char*, float) {}
static void __thiscall cycle_change_impl(void*, const char*, const char*) {}
static void __thiscall game_update_impl(void*) {}
static void __thiscall event_collide_impl(void*, void*, char*) {}
static void __thiscall text_render_impl(void*) {}
static void __thiscall ball_bump_impl(void*, void*, void*) {}
static void __thiscall level_start_impl(void*) {}
static void __thiscall scene_end_impl(void*) {}

static void* g_vtable[17] = {
    (void*)sc_dtor,              // [0]
    (void*)get_mod_name,         // [1]
    (void*)get_author,           // [2]
    (void*)get_version,          // [3]
    (void*)get_contributors,     // [4]
    (void*)init_impl,            // [5]
    (void*)ball_update_impl,     // [6]
    (void*)render_apply_impl,    // [7]
    (void*)button_toggle_impl,   // [8]
    (void*)slider_change_impl,   // [9]
    (void*)cycle_change_impl,    // [10]
    (void*)game_update_impl,     // [11]
    (void*)event_collide_impl,   // [12]
    (void*)text_render_impl,     // [13]
    (void*)ball_bump_impl,       // [14]
    (void*)scene_end_impl,       // [15]
    (void*)level_start_impl,     // [16]
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    void* obj = nc_malloc(8);
    if (!obj) return NULL;
    *(void**)obj = g_vtable;
    *(void**)((char*)obj + 4) = NULL;
    return (HamsterballAPI*)obj;
}
