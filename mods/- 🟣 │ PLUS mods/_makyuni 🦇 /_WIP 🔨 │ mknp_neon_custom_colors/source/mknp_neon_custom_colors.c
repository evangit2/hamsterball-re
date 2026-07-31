/*
 * neon_custom_colors — Customizable Neon Race glow colors from .txt file.
 *
 * Reads mknp_neon_custom_colors.txt from the game directory and applies custom RGB
 * values to both the player outline (material colors) and emitter glow
 * (light source) for P1 and P2, plus all neon platform/border mesh nodes.
 *
 * Uses a detour hook on Scene_SetupLevelDark (0x416270) to write colors
 * to the phys/emitter structs after the original function sets them up.
 * This bypasses the 2-byte PUSH imm8 limitation for B channels.
 *
 * Config file: mknp_neon_custom_colors.txt (next to bass.dll)
 * Format (float values, NOT hex):
 *   NEON PLAYER1 OUTLINE:
 *   * R = 1.0
 *   * G = 1.0
 *   * B = 0.0
 *
 *   NEON PLAYER1 GLOW:
 *   * R = 10.0
 *   * G = 10.0
 *   * B = 0.0
 *   (same for PLAYER2 OUTLINE and GLOW)
 *
 * If mknp_neon_custom_colors.txt is missing, a default one is generated automatically.
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

/* ── Config ────────────────────────────────────────────────────────── */
typedef struct { DWORD r, g, b; } ColorRGB;

typedef struct {
    ColorRGB outline;  /* material colors (Ambient/Diffuse/Emissive) */
    ColorRGB glow;     /* emitter light Diffuse */
} PlayerNeonConfig;

/* Defaults = original yellow */
static PlayerNeonConfig g_config[2];

/* Helper: float → DWORD bit representation (avoid strict-aliasing UB) */
static DWORD f2d(float f) {
    DWORD d;
    memcpy(&d, &f, 4);
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

/* ── Config file path ──────────────────────────────────────────────── */
/* Forward declare DllMain for GetModuleHandleEx */
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved);

static void get_config_path(char* out, DWORD len) {
    HMODULE hSelf = NULL;
    char dll_path[MAX_PATH];
    if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
                           (LPCSTR)&DllMain, &hSelf) && hSelf) {
        GetModuleFileNameA(hSelf, dll_path, MAX_PATH);
        /* Strip filename, keep directory */
        char* slash = strrchr(dll_path, '\\');
        if (slash) {
            slash[1] = '\0';
            _snprintf(out, len, "%smknp_neon_custom_colors.txt", dll_path);
            return;
        }
    }
    /* Fallback: try current directory */
    _snprintf(out, len, "mknp_neon_custom_colors.txt");
}

/* ── Generate default config file ──────────────────────────────────── */
static void generate_default_config(const char* path) {
    FILE* f = NULL;
    if (fopen_s(&f, path, "w") != 0 || !f) return;

    fprintf(f, "NEON PLAYER1 OUTLINE:\n");
    fprintf(f, "* R = 1.0\n");
    fprintf(f, "* G = 1.0\n");
    fprintf(f, "* B = 0.0\n");
    fprintf(f, "\n");
    fprintf(f, "NEON PLAYER1 GLOW:\n");
    fprintf(f, "* R = 10.0\n");
    fprintf(f, "* G = 10.0\n");
    fprintf(f, "* B = 0.0\n");
    fprintf(f, "\n");
    fprintf(f, "NEON PLAYER2 OUTLINE:\n");
    fprintf(f, "* R = 1.0\n");
    fprintf(f, "* G = 1.0\n");
    fprintf(f, "* B = 0.0\n");
    fprintf(f, "\n");
    fprintf(f, "NEON PLAYER2 GLOW:\n");
    fprintf(f, "* R = 10.0\n");
    fprintf(f, "* G = 10.0\n");
    fprintf(f, "* B = 0.0\n");

    fclose(f);
}

/* ── Config file parser (float values) ─────────────────────────────── */
static void read_config(void) {
    char path[MAX_PATH];
    FILE* f = NULL;
    char line[512];
    int player = 0, is_glow = 0;

    init_defaults();
    get_config_path(path, MAX_PATH);

    /* If config doesn't exist, generate a default one and return */
    if (fopen_s(&f, path, "r") != 0 || !f) {
        generate_default_config(path);
        return;
    }

    while (fgets(line, sizeof(line), f)) {
        char* p = line;

        /* Skip whitespace */
        while (*p == ' ' || *p == '\t') p++;

        /* Skip empty lines and comments */
        if (*p == '\n' || *p == '\r' || *p == '\0' || *p == '#') continue;

        /* Check for section headers */
        if (_strnicmp(p, "NEON", 4) == 0) {
            char* pl = strstr(p, "PLAYER1");
            char* p2 = strstr(p, "PLAYER2");
            char* gl = strstr(p, "GLOW");
            char* ol = strstr(p, "OUTLINE");
            if (pl) player = 0;
            else if (p2) player = 1;
            if (gl) is_glow = 1;
            else if (ol) is_glow = 0;
            continue;
        }

        /* Check for color values: "* R = 1.0" */
        if (*p == '*') {
            char channel = 0;
            float fval = 0.0f;
            char* eq = strchr(p, '=');
            if (!eq) continue;

            /* Find channel letter before = */
            char* cp = eq - 1;
            while (cp > p && (*cp == ' ' || *cp == '\t')) cp--;
            if (cp > p) channel = *cp;

            /* Parse float value after = */
            char* vp = eq + 1;
            while (*vp == ' ' || *vp == '\t') vp++;
            fval = (float)strtod(vp, NULL);

            /* Convert float to DWORD bits */
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
    }
    fclose(f);
}

/* ── Game offsets ── */
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

/* ── Detour infrastructure ── */
static unsigned char* g_tramp = NULL;
static const int TRAMP_SIZE = 16;

/* Original 7-byte prologue: PUSH -1; PUSH 0x4C9BCC */
static const unsigned char ORIG_PROLOGUE[7] = {
    0x6A, 0xFF,
    0x68, 0xCC, 0x9B, 0x4C, 0x00
};

/* ── Write helpers ── */
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

/* ── Hook function ── */
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

/* ── Install detour ── */
static void install_detour(void) {
    DWORD target = SCENE_SETUP_LEVEL_DARK;
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
        DWORD rel = (DWORD)&neon_hook - (target + 5);
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
            init_defaults();
            read_config();
            init_bass_proxy();
            install_detour();
            break;
    }
    return TRUE;
}
