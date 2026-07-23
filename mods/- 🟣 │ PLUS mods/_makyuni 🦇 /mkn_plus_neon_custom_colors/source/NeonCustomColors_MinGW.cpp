/*
 * NeonCustomColors_MinGW.cpp — Customizable Neon Race glow colors (HB+ v2.1, MinGW)
 *
 * Ported from neon_custom_colors.c (bass.dll proxy mod).
 *
 * Reads neon_colors.txt from the Mods\ folder (next to this DLL) and applies
 * custom RGB values to both the player outline (material colors) and emitter
 * glow (light source) for P1 and P2, plus all neon platform/border mesh nodes.
 *
 * Uses a detour hook on Scene_SetupLevelDark (0x416270) to write colors
 * to the phys/emitter structs after the original function sets them up.
 * This bypasses the 2-byte PUSH imm8 limitation for B channels.
 *
 * Uses nocrt + manual 17-entry vtable for HB+ v2.1 compatibility.
 */
#include "nocrt.h"
#include "HamsterballAPI.h"
#include "hbplus_api.h"

/* ═══════════════════════════════════════════════════════════════════════════
 * Config structures
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef struct { DWORD r, g, b; } ColorRGB;

typedef struct {
    ColorRGB outline;  /* material colors (Ambient/Diffuse/Emissive) */
    ColorRGB glow;     /* emitter light Diffuse */
} PlayerNeonConfig;

/* Defaults = original yellow */
static PlayerNeonConfig g_config[2];

/* Helper: float -> DWORD bit representation (avoid strict-aliasing UB) */
static DWORD f2d(float f) {
    DWORD d;
    nc_memcpy(&d, &f, 4);
    return d;
}

static void init_defaults(void) {
    /* Outline: R=1.0, G=1.0, B=0.0 = yellow */
    g_config[0].outline.r = f2d(1.0f);
    g_config[0].outline.g = f2d(1.0f);
    g_config[0].outline.b = f2d(0.0f);
    /* Glow: R=10.0, G=10.0, B=0.0 = bright yellow */
    g_config[0].glow.r = f2d(10.0f);
    g_config[0].glow.g = f2d(10.0f);
    g_config[0].glow.b = f2d(0.0f);
    /* P2 = same as P1 */
    g_config[1] = g_config[0];
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Config file path (next to this DLL in Mods\ folder)
 * Uses VirtualQuery on a function pointer (HB+ pattern)
 * ═══════════════════════════════════════════════════════════════════════════ */

static char g_configPath[MAX_PATH] = "";
static bool g_pathReady = false;

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
                           "neon_colors.txt",
                           MAX_PATH - nc_strlen(g_configPath) - 1);
                g_configPath[MAX_PATH - 1] = '\0';
                g_pathReady = true;
                return;
            }
        }
    }
    nc_strncpy(g_configPath, "neon_colors.txt", MAX_PATH - 1);
    g_configPath[MAX_PATH - 1] = '\0';
    g_pathReady = true;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Generate default config file
 * ═══════════════════════════════════════════════════════════════════════════ */

static void generate_default_config(const char* path) {
    HANDLE h = CreateFileA(path, GENERIC_WRITE, 0, NULL,
                           CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return;

    char buf[2048];
    int pos = 0;
    pos += nc_snprintf(buf + pos, sizeof(buf) - pos,
        "NEON PLAYER1 OUTLINE:\r\n"
        "* R = 1.0\r\n"
        "* G = 1.0\r\n"
        "* B = 0.0\r\n"
        "\r\n"
        "NEON PLAYER1 GLOW:\r\n"
        "* R = 10.0\r\n"
        "* G = 10.0\r\n"
        "* B = 0.0\r\n"
        "\r\n"
        "NEON PLAYER2 OUTLINE:\r\n"
        "* R = 1.0\r\n"
        "* G = 1.0\r\n"
        "* B = 0.0\r\n"
        "\r\n"
        "NEON PLAYER2 GLOW:\r\n"
        "* R = 10.0\r\n"
        "* G = 10.0\r\n"
        "* B = 0.0\r\n");

    DWORD written = 0;
    WriteFile(h, buf, pos, &written, NULL);
    CloseHandle(h);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Config file parser (float values)
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Case-insensitive string comparison (simple version) */
static int nc_stricmp(const char* a, const char* b) {
    while (*a && *b) {
        char ca = *a, cb = *b;
        if (ca >= 'a' && ca <= 'z') ca -= 32;
        if (cb >= 'a' && cb <= 'z') cb -= 32;
        if (ca != cb) return ca - cb;
        a++; b++;
    }
    return (unsigned char)*a - (unsigned char)*b;
}

/* Case-insensitive substring search */
static const char* nc_stristr(const char* haystack, const char* needle) {
    if (!haystack || !needle) return NULL;
    int nlen = (int)nc_strlen(needle);
    if (nlen == 0) return haystack;
    for (; *haystack; haystack++) {
        int i;
        for (i = 0; i < nlen; i++) {
            char h = haystack[i], n = needle[i];
            if (h >= 'a' && h <= 'z') h -= 32;
            if (n >= 'a' && n <= 'z') n -= 32;
            if (h != n) break;
        }
        if (i == nlen) return haystack;
    }
    return NULL;
}

static char* nc_strchr(const char* s, int c) {
    while (*s) {
        if (*s == (char)c) return (char*)s;
        s++;
    }
    return (c == 0) ? (char*)s : NULL;
}

/* Simple float parser */
static float parse_float(const char** pp) {
    const char* p = *pp;
    float val = 0.0f;
    int negative = 0;
    int hadDigits = 0;

    if (*p == '-') { negative = 1; p++; }
    else if (*p == '+') { p++; }

    int integerPart = 0;
    while (*p >= '0' && *p <= '9') {
        integerPart = integerPart * 10 + (*p - '0');
        p++;
        hadDigits = 1;
    }
    float frac = 0.0f;
    if (*p == '.') {
        p++;
        float div = 10.0f;
        while (*p >= '0' && *p <= '9') {
            frac += (*p - '0') / div;
            div *= 10.0f;
            p++;
            hadDigits = 1;
        }
    }
    if (hadDigits) {
        val = (float)integerPart + frac;
        if (negative) val = -val;
    }
    *pp = p;
    return val;
}

static void read_config(void) {
    if (!g_pathReady) return;

    HANDLE h = CreateFileA(g_configPath, GENERIC_READ, FILE_SHARE_READ, NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) {
        generate_default_config(g_configPath);
        return;
    }

    char buf[8192];
    DWORD bytesRead = 0;
    ReadFile(h, buf, sizeof(buf) - 1, &bytesRead, NULL);
    CloseHandle(h);
    if (bytesRead == 0) return;
    buf[bytesRead] = '\0';

    init_defaults();

    int player = 0, is_glow = 0;
    char* p = buf;

    /* Skip BOM */
    if ((unsigned char)p[0] == 0xEF && (unsigned char)p[1] == 0xBB && (unsigned char)p[2] == 0xBF) {
        p += 3;
    }

    while (*p) {
        /* Skip whitespace */
        while (*p == ' ' || *p == '\t') p++;
        /* Skip empty lines and comments */
        if (*p == '\n' || *p == '\r' || *p == '\0' || *p == '#') {
            if (*p == '\0') break;
            p++;
            continue;
        }

        /* Check for section headers */
        if (nc_stristr(p, "NEON") == p) {
            if (nc_stristr(p, "PLAYER1")) player = 0;
            else if (nc_stristr(p, "PLAYER2")) player = 1;
            if (nc_stristr(p, "GLOW")) is_glow = 1;
            else if (nc_stristr(p, "OUTLINE")) is_glow = 0;
            /* Skip to end of line */
            while (*p && *p != '\n') p++;
            continue;
        }

        /* Check for color values: "* R = 1.0" */
        if (*p == '*') {
            char channel = 0;
            char* eq = nc_strchr(p, '=');
            if (!eq) {
                while (*p && *p != '\n') p++;
                continue;
            }

            /* Find channel letter before = */
            char* cp = eq - 1;
            while (cp > p && (*cp == ' ' || *cp == '\t')) cp--;
            if (cp > p) channel = *cp;

            /* Parse float value after = */
            const char* vp = eq + 1;
            while (*vp == ' ' || *vp == '\t') vp++;
            float fval = parse_float(&vp);
            DWORD val = f2d(fval);

            /* Store value */
            ColorRGB* target = NULL;
            if (player == 0 && !is_glow) target = &g_config[0].outline;
            else if (player == 0 && is_glow) target = &g_config[0].glow;
            else if (player == 1 && !is_glow) target = &g_config[1].outline;
            else if (player == 1 && is_glow) target = &g_config[1].glow;

            if (target) {
                if (channel == 'R' || channel == 'r') target->r = val;
                else if (channel == 'G' || channel == 'g') target->g = val;
                else if (channel == 'B' || channel == 'b') target->b = val;
            }
        }

        /* Skip to end of line */
        while (*p && *p != '\n') p++;
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Game offsets
 * ═══════════════════════════════════════════════════════════════════════════ */

#define SCENE_SETUP_LEVEL_DARK  0x00416270
#define MODULE_BASE             0x00400000

/* Offsets from scene/board struct */
#define OFF_APP                 0x878
#define OFF_P1_PHYS             0x5DC
#define OFF_P2_PHYS             0x67C
#define OFF_P2_FLAG             0x677
#define OFF_P1_EMITTER          0x436C
#define OFF_P2_EMITTER          0x4370
#define OFF_MESH_LIST           0x2DEC

/* Offsets within phys/SceneObject */
#define OFF_DIFFUSE             0x1BC
#define OFF_AMBIENT             0x1CC
#define OFF_EMISSIVE            0x1EC
#define OFF_EMITTER_COLOR       0x94

/* ═══════════════════════════════════════════════════════════════════════════
 * Detour infrastructure
 * ═══════════════════════════════════════════════════════════════════════════ */

static unsigned char* g_tramp = NULL;
static const int TRAMP_SIZE = 16;

/* Original 7-byte prologue: PUSH -1; PUSH 0x4C9BCC */
static const unsigned char ORIG_PROLOGUE[7] = {
    0x6A, 0xFF,
    0x68, 0xCC, 0x9B, 0x4C, 0x00
};

/* ═══════════════════════════════════════════════════════════════════════════
 * Write helpers
 * ═══════════════════════════════════════════════════════════════════════════ */

static void safe_write_dword(void* addr, DWORD val) {
    if (IsBadReadPtr(addr, 4)) return;
    *(DWORD*)addr = val;
}

static void write_material_rgba(int* phys, int base_off, ColorRGB* c) {
    if (IsBadReadPtr(phys, 0x200)) return;
    safe_write_dword((char*)phys + base_off + 0,  c->r);
    safe_write_dword((char*)phys + base_off + 4,  c->g);
    safe_write_dword((char*)phys + base_off + 8,  c->b);
    safe_write_dword((char*)phys + base_off + 12, 0x3F800000); /* A=1.0 */
}

static void write_emitter_rgb(int* emitter, ColorRGB* c) {
    if (IsBadReadPtr(emitter, 0xA4)) return;
    safe_write_dword((char*)emitter + OFF_EMITTER_COLOR + 0, c->r);
    safe_write_dword((char*)emitter + OFF_EMITTER_COLOR + 4, c->g);
    safe_write_dword((char*)emitter + OFF_EMITTER_COLOR + 8, c->b);
    /* +0xA0 (w) stays at 1.0 from Vec3_Init */
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Hook function — called after Scene_SetupLevelDark runs
 * ═══════════════════════════════════════════════════════════════════════════ */

static void __fastcall neon_hook(void* this_) {
    int* scene = (int*)this_;
    int* app;
    int* p1_phys;
    int* p2_phys;
    int* p1_emit;
    int* p2_emit;
    int* mesh_list;
    int mesh_count;
    int* mesh_array;
    int i;

    /* Re-read config for live updates */
    read_config();

    /* Call original function via trampoline */
    typedef void (__fastcall *orig_fn_t)(void*);
    ((orig_fn_t)g_tramp)(this_);

    /* Validate scene pointer */
    if (IsBadReadPtr(scene, 0x5000)) return;

    /* Get App pointer */
    app = *(int**)((char*)scene + OFF_APP);
    if (IsBadReadPtr(app, 0x700)) return;

    /* P1 phys — write all 3 material types */
    p1_phys = *(int**)((char*)app + OFF_P1_PHYS);
    if (p1_phys && !IsBadReadPtr(p1_phys, 0x200)) {
        write_material_rgba(p1_phys, OFF_DIFFUSE,  &g_config[0].outline);
        write_material_rgba(p1_phys, OFF_AMBIENT, &g_config[0].outline);
        write_material_rgba(p1_phys, OFF_EMISSIVE, &g_config[0].outline);
    }

    /* P2 phys (only if P2 exists: flag at app+0x677 == 0) */
    if (*(unsigned char*)((char*)app + OFF_P2_FLAG) == 0) {
        p2_phys = *(int**)((char*)app + OFF_P2_PHYS);
        if (p2_phys && !IsBadReadPtr(p2_phys, 0x200)) {
            write_material_rgba(p2_phys, OFF_DIFFUSE,  &g_config[1].outline);
            write_material_rgba(p2_phys, OFF_AMBIENT, &g_config[1].outline);
            write_material_rgba(p2_phys, OFF_EMISSIVE, &g_config[1].outline);
        }
    }

    /* P1 emitter SceneObject */
    p1_emit = *(int**)((char*)scene + OFF_P1_EMITTER);
    if (p1_emit && !IsBadReadPtr(p1_emit, 0xA4)) {
        write_emitter_rgb(p1_emit, &g_config[0].glow);
    }

    /* P2 emitter SceneObject */
    if (*(unsigned char*)((char*)app + OFF_P2_FLAG) == 0) {
        p2_emit = *(int**)((char*)scene + OFF_P2_EMITTER);
        if (p2_emit && !IsBadReadPtr(p2_emit, 0xA4)) {
            write_emitter_rgb(p2_emit, &g_config[1].glow);
        }
    }

    /* Mesh nodes (neon platform/border colors) — use P1 outline */
    mesh_list = (int*)((char*)scene + OFF_MESH_LIST);
    if (!IsBadReadPtr(mesh_list, 0x414)) {
        mesh_count = mesh_list[1]; /* +0x04 = count */
        if (mesh_count > 0 && mesh_count < 1000) {
            mesh_array = *(int**)((char*)mesh_list + 0x40C);
            if (mesh_array && !IsBadReadPtr(mesh_array, mesh_count * 4)) {
                for (i = 0; i < mesh_count; i++) {
                    int* node = (int*)mesh_array[i];
                    if (node && !IsBadReadPtr(node, 0x200)) {
                        write_material_rgba(node, OFF_DIFFUSE,  &g_config[0].outline);
                        write_material_rgba(node, OFF_AMBIENT, &g_config[0].outline);
                        write_material_rgba(node, OFF_EMISSIVE, &g_config[0].outline);
                    }
                }
            }
        }
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Install detour
 * ═══════════════════════════════════════════════════════════════════════════ */

static void install_detour(void) {
    DWORD target = SCENE_SETUP_LEVEL_DARK;
    DWORD old_prot;
    unsigned char jmp_patch[7];

    /* Allocate executable trampoline */
    g_tramp = (unsigned char*)VirtualAlloc(NULL, TRAMP_SIZE,
                  MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!g_tramp) return;

    /* Trampoline: original prologue (7 bytes) + JMP back to target+7 (5 bytes) */
    nc_memcpy(g_tramp, ORIG_PROLOGUE, 7);
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
        DWORD rel = (DWORD)&neon_hook - (target + 5);
        *(DWORD*)(jmp_patch + 1) = rel;
    }
    jmp_patch[5] = 0x90; /* NOP */
    jmp_patch[6] = 0x90; /* NOP */

    nc_memcpy((void*)target, jmp_patch, 7);
    VirtualProtect((void*)target, 7, old_prot, &old_prot);
    FlushInstructionCache(GetCurrentProcess(), (void*)target, 7);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * HB+ Vtable Implementation
 * ═══════════════════════════════════════════════════════════════════════════ */

static void* g_storedApi = NULL;

static void* __thiscall sc_dtor(void* thisptr, int flags) {
    if (flags & 1) nc_free(thisptr);
    return thisptr;
}

static const char* __thiscall get_mod_name(void*) { return "Neon Custom Colors"; }
static const char* __thiscall get_author(void*) { return "MAKYUNI"; }
static int __thiscall get_version(void*) { return HAMSTERBALL_API_VERSION; }
static const char* __thiscall get_contributors(void*) { return "Hamsterbot"; }

static void __thiscall init_impl(void* thisptr, void* modApi) {
    *(void**)((char*)thisptr + 4) = modApi;
    g_storedApi = modApi;

    init_defaults();
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

/* ═══════════════════════════════════════════════════════════════════════════
 * 17-Entry Vtable (HB+ v2.0/v2.1)
 * ═══════════════════════════════════════════════════════════════════════════ */

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
