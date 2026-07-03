/*
 * animated_textures — Cycle between numbered texture variants at a custom framerate.
 *
 * The user places .txt files in the Textures/ folder. Each .txt filename (without
 * extension) becomes the animation prefix. The mod scans for textures named
 * "prefix_01", "prefix_02", etc., loads any that aren't already cached, and
 * cycles between them when the game binds any frame from the sequence.
 *
 * Config file format (e.g. Textures/exampleanimtex.txt):
 *   framerate = 1     (frames to advance per tick; 0.5 = every 2 ticks)
 *   looptype = 1      (0=play once, 1=loop forever, 3=ping-pong)
 *
 * Texture naming: prefix_NN.ext (e.g. exampleanimtex_01.png, exampleanimtex_02.bmp)
 * The number after the last underscore determines the frame order.
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── BASS proxy exports (forward to bass_real.dll) ─────────────────── */

static HMODULE g_hRealBass = NULL;

typedef int  (__stdcall *BASS_ChannelSetAttributes_t)(DWORD, DWORD, int, int);
static BASS_ChannelSetAttributes_t real_BASS_ChannelSetAttributes = NULL;
__declspec(dllexport) int __stdcall BASS_ChannelSetAttributes(DWORD a, DWORD b, int c, int d) {
    if (real_BASS_ChannelSetAttributes) return real_BASS_ChannelSetAttributes(a, b, c, d);
    return 1;
}
typedef int  (__stdcall *BASS_MusicPlayEx_t)(DWORD, DWORD, int, DWORD);
static BASS_MusicPlayEx_t real_BASS_MusicPlayEx = NULL;
__declspec(dllexport) int __stdcall BASS_MusicPlayEx(DWORD a, DWORD b, int c, DWORD d) {
    if (real_BASS_MusicPlayEx) return real_BASS_MusicPlayEx(a, b, c, d);
    return 1;
}
typedef int  (__stdcall *BASS_SetConfig_t)(DWORD, DWORD);
static BASS_SetConfig_t real_BASS_SetConfig = NULL;
__declspec(dllexport) int __stdcall BASS_SetConfig(DWORD a, DWORD b) {
    if (real_BASS_SetConfig) return real_BASS_SetConfig(a, b);
    return 1;
}
typedef int  (__stdcall *BASS_Init_t)(int, DWORD, DWORD, HWND, void*);
static BASS_Init_t real_BASS_Init = NULL;
__declspec(dllexport) int __stdcall BASS_Init(int a, DWORD b, DWORD c, HWND d, void* e) {
    if (real_BASS_Init) return real_BASS_Init(a, b, c, d, e);
    return 1;
}
typedef int  (__stdcall *BASS_Free_t)(void);
static BASS_Free_t real_BASS_Free = NULL;
__declspec(dllexport) int __stdcall BASS_Free(void) {
    if (real_BASS_Free) return real_BASS_Free();
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
typedef int  (__stdcall *BASS_ErrorGetCode_t)(void);
static BASS_ErrorGetCode_t real_BASS_ErrorGetCode = NULL;
__declspec(dllexport) int __stdcall BASS_ErrorGetCode(void) {
    if (real_BASS_ErrorGetCode) return real_BASS_ErrorGetCode();
    return 0;
}
typedef DWORD (__stdcall *BASS_MusicLoad_t)(int, void*, DWORD, DWORD, DWORD, DWORD);
static BASS_MusicLoad_t real_BASS_MusicLoad = NULL;
__declspec(dllexport) DWORD __stdcall BASS_MusicLoad(int a, void* b, DWORD c, DWORD d, DWORD e, DWORD f) {
    if (real_BASS_MusicLoad) return real_BASS_MusicLoad(a, b, c, d, e, f);
    return 1;
}
typedef int  (__stdcall *BASS_ChannelStop_t)(DWORD);
static BASS_ChannelStop_t real_BASS_ChannelStop = NULL;
__declspec(dllexport) int __stdcall BASS_ChannelStop(DWORD a) {
    if (real_BASS_ChannelStop) return real_BASS_ChannelStop(a);
    return 1;
}

static void init_bass_proxy(void) {
    g_hRealBass = LoadLibraryA("bass_real.dll");
    if (!g_hRealBass) return;
    real_BASS_ChannelSetAttributes = (BASS_ChannelSetAttributes_t)GetProcAddress(g_hRealBass, "BASS_ChannelSetAttributes");
    real_BASS_MusicPlayEx         = (BASS_MusicPlayEx_t)GetProcAddress(g_hRealBass, "BASS_MusicPlayEx");
    real_BASS_SetConfig           = (BASS_SetConfig_t)GetProcAddress(g_hRealBass, "BASS_SetConfig");
    real_BASS_Init                = (BASS_Init_t)GetProcAddress(g_hRealBass, "BASS_Init");
    real_BASS_Free                = (BASS_Free_t)GetProcAddress(g_hRealBass, "BASS_Free");
    real_BASS_Start               = (BASS_Start_t)GetProcAddress(g_hRealBass, "BASS_Start");
    real_BASS_Stop                = (BASS_Stop_t)GetProcAddress(g_hRealBass, "BASS_Stop");
    real_BASS_ErrorGetCode        = (BASS_ErrorGetCode_t)GetProcAddress(g_hRealBass, "BASS_ErrorGetCode");
    real_BASS_MusicLoad           = (BASS_MusicLoad_t)GetProcAddress(g_hRealBass, "BASS_MusicLoad");
    real_BASS_ChannelStop         = (BASS_ChannelStop_t)GetProcAddress(g_hRealBass, "BASS_ChannelStop");
}

/* ── Constants ──────────────────────────────────────────────────────── */

#define GRAPHICS_BEGIN_FRAME  0x00453B50
#define OFF_D3D_DEVICE        0x154
#define GFX_TEX_COUNT         0x2E8
#define GFX_TEX_ARRAY         0x6F0
#define TEX_OBJ_D3D           0x04
#define TEX_OBJ_NAME          0x08

#define VTBL_SET_TEXTURE      61

#define MAX_ANIMATIONS        16
#define MAX_FRAMES            64
#define MAX_TEX_LOOKUP        1024

/* LoadTexture: __thiscall(void* gfx, char* filename, char search_cache)
 * MinGW C lacks __thiscall, so use __fastcall with a dummy EDX param.
 * ECX=gfx, EDX=unused, stack=[filename, search_cache] — identical layout. */
typedef void* (__fastcall *LoadTexture_t)(void* gfx, void* dummy, const char* name, int search_cache);
static LoadTexture_t game_LoadTexture = (LoadTexture_t)0x00455C50;

/* ── Data structures ───────────────────────────────────────────────── */

typedef struct {
    char prefix[64];
    float framerate;
    int looptype;
    void* frames[MAX_FRAMES];
    int frame_count;
    int current_frame;
    float accumulator;
    int direction;
    int built;
} AnimSeq;

typedef struct {
    void* d3d_tex;
    int anim_idx;
} TexLookup;

static AnimSeq g_anims[MAX_ANIMATIONS];
static int g_anim_count = 0;
static TexLookup g_tex_lookup[MAX_TEX_LOOKUP];
static int g_lookup_count = 0;
static int g_last_tex_count = -1;
static char g_game_dir[MAX_PATH] = "";

/* ── D3D8 vtable hook ──────────────────────────────────────────────── */

typedef int (__stdcall *SetTexture_t)(void*, DWORD, void*);
static SetTexture_t g_orig_SetTexture = NULL;
static int g_vtable_hooked = 0;

static int __stdcall hook_SetTexture(void* device, DWORD stage, void* tex) {
    if (tex) {
        int i;
        for (i = 0; i < g_lookup_count; i++) {
            if (g_tex_lookup[i].d3d_tex == tex) {
                AnimSeq* a = &g_anims[g_tex_lookup[i].anim_idx];
                if (a->frame_count > 0 && a->current_frame >= 0 && a->current_frame < a->frame_count)
                    if (a->frames[a->current_frame])
                        tex = a->frames[a->current_frame];
                break;
            }
        }
    }
    return g_orig_SetTexture(device, stage, tex);
}

static void hook_d3d_vtable(int* device) {
    if (!device || IsBadReadPtr(device, 4)) return;
    int* vtable = *(int**)device;
    if (!vtable || IsBadReadPtr(vtable, (VTBL_SET_TEXTURE + 1) * 4)) return;

    g_orig_SetTexture = (SetTexture_t)vtable[VTBL_SET_TEXTURE];
    if (!g_orig_SetTexture) return;

    DWORD old_prot;
    if (VirtualProtect(&vtable[VTBL_SET_TEXTURE], 4, PAGE_READWRITE, &old_prot)) {
        vtable[VTBL_SET_TEXTURE] = (int)&hook_SetTexture;
        VirtualProtect(&vtable[VTBL_SET_TEXTURE], 4, old_prot, &old_prot);
    }
    g_vtable_hooked = 1;
}

/* ── Config loading ────────────────────────────────────────────────── */

static void get_game_dir(void) {
    HMODULE hSelf = NULL;
    if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                          | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                          (LPCSTR)&get_game_dir, &hSelf) && hSelf) {
        GetModuleFileNameA(hSelf, g_game_dir, MAX_PATH);
        char* slash = strrchr(g_game_dir, '\\');
        if (slash) { slash[1] = '\0'; return; }
    }
    GetCurrentDirectoryA(MAX_PATH, g_game_dir);
    size_t len = strlen(g_game_dir);
    if (len > 0 && g_game_dir[len - 1] != '\\') strcat(g_game_dir, "\\");
}

static void parse_anim_config(const char* txt_path, const char* anim_name) {
    if (g_anim_count >= MAX_ANIMATIONS) return;
    AnimSeq* a = &g_anims[g_anim_count];
    memset(a, 0, sizeof(AnimSeq));
    strncpy(a->prefix, anim_name, 63);
    a->prefix[63] = '\0';
    a->framerate = 1.0f;
    a->looptype = 1;
    a->direction = 1;

    FILE* f = NULL;
    if (fopen_s(&f, txt_path, "r") != 0 || !f) {
        g_anim_count++;
        return;
    }
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        char* p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '#' || *p == '\n' || *p == '\r' || *p == '\0') continue;
        char* eq = strchr(p, '=');
        if (!eq) continue;
        *eq = '\0';
        char* key = p;
        char* val = eq + 1;
        while (*val == ' ' || *val == '\t') val++;
        char* k_end = key + strlen(key) - 1;
        while (k_end > key && (*k_end == ' ' || *k_end == '\t' || *k_end == '\n' || *k_end == '\r'))
            *k_end-- = '\0';
        if (_stricmp(key, "framerate") == 0)
            a->framerate = (float)atof(val);
        else if (_stricmp(key, "looptype") == 0)
            a->looptype = atoi(val);
    }
    fclose(f);
    if (a->framerate < 0.0f) a->framerate = 0.0f;
    if (a->looptype != 0 && a->looptype != 1 && a->looptype != 3) a->looptype = 1;
    g_anim_count++;
}

static void load_configs(void) {
    g_anim_count = 0;
    char pattern[MAX_PATH];
    snprintf(pattern, MAX_PATH, "%sTextures\\*.txt", g_game_dir);
    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(pattern, &fd);
    if (hFind == INVALID_HANDLE_VALUE) return;
    do {
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
        char name[64];
        strncpy(name, fd.cFileName, 63);
        name[63] = '\0';
        char* dot = strrchr(name, '.');
        if (dot) *dot = '\0';
        char full_path[MAX_PATH];
        snprintf(full_path, MAX_PATH, "%sTextures\\%s", g_game_dir, fd.cFileName);
        parse_anim_config(full_path, name);
    } while (FindNextFileA(hFind, &fd));
    FindClose(hFind);
}

/* ── Animation rebuilding (find + load frames, build lookup) ────────── */

static void rebuild_animations(char* gfx) {
    int i;
    for (i = 0; i < g_anim_count; i++) {
        g_anims[i].frame_count = 0;
        g_anims[i].current_frame = 0;
        g_anims[i].accumulator = 0.0f;
        g_anims[i].direction = 1;
        g_anims[i].built = 0;
    }
    g_lookup_count = 0;

    for (int ai = 0; ai < g_anim_count; ai++) {
        AnimSeq* a = &g_anims[ai];
        a->built = 1;

        char file_pattern[MAX_PATH];
        snprintf(file_pattern, MAX_PATH, "%sTextures\\%s_*", g_game_dir, a->prefix);

        WIN32_FIND_DATAA fd;
        HANDLE hFind = FindFirstFileA(file_pattern, &fd);
        if (hFind == INVALID_HANDLE_VALUE) continue;

        int frame_nums[MAX_FRAMES];
        char frame_names[MAX_FRAMES][64];
        int found = 0;

        do {
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
            char* dot = strrchr(fd.cFileName, '.');
            if (dot && (_stricmp(dot, ".txt") == 0)) continue;
            char* uscore = strrchr(fd.cFileName, '_');
            if (!uscore) continue;
            int num = atoi(uscore + 1);
            if (num < 1 || num > MAX_FRAMES) continue;
            if (found >= MAX_FRAMES) continue;

            char base[64];
            if (dot) {
                size_t len = (size_t)(dot - fd.cFileName);
                if (len >= 64) len = 63;
                strncpy(base, fd.cFileName, len);
                base[len] = '\0';
            } else {
                strncpy(base, fd.cFileName, 63);
                base[63] = '\0';
            }
            frame_nums[found] = num;
            strcpy(frame_names[found], base);
            found++;
        } while (FindNextFileA(hFind, &fd));
        FindClose(hFind);

        /* Sort by frame number (simple insertion sort) */
        for (i = 1; i < found; i++) {
            int key_num = frame_nums[i];
            char key_name[64];
            strcpy(key_name, frame_names[i]);
            int j = i - 1;
            while (j >= 0 && frame_nums[j] > key_num) {
                frame_nums[j + 1] = frame_nums[j];
                strcpy(frame_names[j + 1], frame_names[j]);
                j--;
            }
            frame_nums[j + 1] = key_num;
            strcpy(frame_names[j + 1], key_name);
        }

        /* Load each frame via the game's LoadTexture */
        for (i = 0; i < found; i++) {
            void* tex_obj = game_LoadTexture(gfx, NULL, frame_names[i], 1);
            if (!tex_obj || IsBadReadPtr(tex_obj, 0x10)) continue;
            void* d3d_tex = *(void**)((char*)tex_obj + TEX_OBJ_D3D);
            if (!d3d_tex || IsBadReadPtr(d3d_tex, 4)) continue;

            int idx = frame_nums[i] - 1;
            if (idx < 0 || idx >= MAX_FRAMES) continue;
            a->frames[idx] = d3d_tex;
            if (idx + 1 > a->frame_count)
                a->frame_count = idx + 1;

            if (g_lookup_count < MAX_TEX_LOOKUP) {
                g_tex_lookup[g_lookup_count].d3d_tex = d3d_tex;
                g_tex_lookup[g_lookup_count].anim_idx = ai;
                g_lookup_count++;
            }
        }

        /* Fill gaps: duplicate previous frame for any missing slots */
        for (i = 0; i < a->frame_count; i++) {
            if (!a->frames[i]) {
                if (i > 0 && a->frames[i - 1])
                    a->frames[i] = a->frames[i - 1];
                else {
                    int j;
                    for (j = i + 1; j < a->frame_count; j++) {
                        if (a->frames[j]) { a->frames[i] = a->frames[j]; break; }
                    }
                }
            }
        }
    }
}

/* ── Frame advancement ────────────────────────────────────────────── */

static void advance_frame(AnimSeq* a) {
    if (a->frame_count <= 1) return;
    switch (a->looptype) {
    case 0:
        if (a->current_frame < a->frame_count - 1)
            a->current_frame++;
        break;
    case 1:
        a->current_frame = (a->current_frame + 1) % a->frame_count;
        break;
    case 3:
        a->current_frame += a->direction;
        if (a->current_frame >= a->frame_count) {
            a->direction = -1;
            a->current_frame = a->frame_count - 2;
        } else if (a->current_frame < 0) {
            a->direction = 1;
            a->current_frame = 1;
        }
        break;
    }
    if (a->current_frame < 0) a->current_frame = 0;
    if (a->current_frame >= a->frame_count) a->current_frame = a->frame_count - 1;
}

static void advance_animations(void) {
    int i;
    for (i = 0; i < g_anim_count; i++) {
        AnimSeq* a = &g_anims[i];
        if (a->frame_count <= 1 || a->framerate <= 0.0f) continue;
        a->accumulator += a->framerate;
        while (a->accumulator >= 1.0f) {
            a->accumulator -= 1.0f;
            advance_frame(a);
        }
    }
}

/* ── BeginFrame detour ─────────────────────────────────────────────── */

static unsigned char* g_tramp = NULL;
static const int TRAMP_SIZE = 16;
static const unsigned char ORIG_PROLOGUE[7] = {
    0x53, 0x8B, 0xD9, 0x8B, 0x4C, 0x24, 0x08
};

static void __fastcall begin_frame_hook(void* this_, void* edx, int param_1) {
    typedef void (__fastcall *orig_fn_t)(void*, void*, int);
    ((orig_fn_t)g_tramp)(this_, edx, param_1);

    char* gfx = (char*)this_;
    if (!gfx || IsBadReadPtr(gfx, 0x200)) return;

    if (!g_vtable_hooked) {
        int* device = *(int**)(gfx + OFF_D3D_DEVICE);
        if (device && !IsBadReadPtr(device, 4))
            hook_d3d_vtable(device);
    }

    if (!IsBadReadPtr(gfx + GFX_TEX_COUNT, 4)) {
        int current_count = *(int*)(gfx + GFX_TEX_COUNT);
        if (current_count != g_last_tex_count) {
            rebuild_animations(gfx);
            g_last_tex_count = *(int*)(gfx + GFX_TEX_COUNT);
        }
    }

    advance_animations();
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

/* ── DllMain ───────────────────────────────────────────────────────── */

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
        get_game_dir();
        load_configs();
        init_bass_proxy();
        install_detour();
        break;
    }
    return TRUE;
}
