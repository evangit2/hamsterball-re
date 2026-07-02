/*
 * global_bg_texture.c — Clouds Background Texture Mod
 *
 * Adds a clouds.png background sprite overlay to any level via a
 * Sprite_RenderQuad hook inside Graphics_RenderScene.
 *
 * RE analysis findings:
 * - Sky Race ctor (0x41F930) creates a clouds.png Sprite at board+0x4374,
 *   but this sprite is NEVER rendered by any function (dead code).
 * - The actual clouds visual on Sky Race comes from:
 *   (1) CLOUDSCAPE mesh geometry in Level9.MESHWORLD (3D textured mesh)
 *   (2) Fog/projection setup in Scene_SetupLevel9 (0x410830)
 * - This mod provides a 2D clouds overlay for non-Sky levels and
 *   optionally hides the Sky Race sprite by zeroing board+0x4374
 *   (cleanup-safe: dtor checks for NULL before freeing).
 *
 * Config: clouds_bg.txt (auto-generated with defaults on first run)
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll global_bg_texture.c -lwinmm \
 *     -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias -msse2 -mfpmath=sse
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * BASS Proxy Exports
 * ═══════════════════════════════════════════════════════════════════════════ */

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
    return 0;
}
typedef int  (__stdcall *BASS_ChannelStop_t)(DWORD);
static BASS_ChannelStop_t real_BASS_ChannelStop = NULL;
__declspec(dllexport) int __stdcall BASS_ChannelStop(DWORD a) {
    if (real_BASS_ChannelStop) return real_BASS_ChannelStop(a);
    return 1;
}

__declspec(dllexport) void __stdcall BASS_Pause(void) {}
__declspec(dllexport) void __stdcall BASS_SetVolume(DWORD a) {}
__declspec(dllexport) DWORD __stdcall BASS_GetVolume(void) { return 0; }
__declspec(dllexport) int __stdcall BASS_GetDevice(void) { return 0; }
__declspec(dllexport) int __stdcall BASS_SetDevice(DWORD a) { return 1; }
__declspec(dllexport) void __stdcall BASS_GetInfo(void *a) {}
__declspec(dllexport) int __stdcall BASS_Update(DWORD a) { return 0; }
__declspec(dllexport) DWORD __stdcall BASS_StreamCreateFile(void *a, void *b, DWORD c, DWORD d, DWORD e) { return 0; }
__declspec(dllexport) DWORD __stdcall BASS_SampleLoad(int a, void *b, DWORD c, DWORD d, DWORD e) { return 0; }
__declspec(dllexport) int __stdcall BASS_ChannelPlay(DWORD a, BOOL b) { return 1; }
__declspec(dllexport) int __stdcall BASS_ChannelGetAttribute(DWORD a, DWORD b, float *c) { return 1; }
__declspec(dllexport) DWORD __stdcall BASS_ChannelGetData(DWORD a, void *b, DWORD c) { return 0; }
__declspec(dllexport) DWORD __stdcall BASS_ChannelGetLevel(DWORD a) { return 0; }
__declspec(dllexport) int __stdcall BASS_ChannelSetPosition(DWORD a, void *b, DWORD c) { return 1; }
__declspec(dllexport) DWORD __stdcall BASS_ChannelGetPosition(DWORD a, DWORD b) { return 0; }
__declspec(dllexport) int __stdcall BASS_ChannelIsActive(DWORD a) { return 0; }
__declspec(dllexport) int __stdcall BASS_ChannelRemoveSync(DWORD a, DWORD b) { return 1; }
__declspec(dllexport) DWORD __stdcall BASS_ChannelSetSync(DWORD a, DWORD b, DWORD c, void *d, void *e) { return 0; }
__declspec(dllexport) DWORD __stdcall BASS_SampleCreate(DWORD a, DWORD b, DWORD c, DWORD d, DWORD e) { return 0; }
__declspec(dllexport) DWORD __stdcall BASS_SampleGetChannel(DWORD a, BOOL b) { return 0; }

static void load_real_bass(void)
{
    g_hRealBass = LoadLibraryA("bass_real.dll");
    if (g_hRealBass == NULL) {
        char path[MAX_PATH];
        HMODULE hSelf = NULL;
        GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                          | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                          (LPCSTR)&load_real_bass, &hSelf);
        GetModuleFileNameA(hSelf, path, MAX_PATH);
        char *p = strrchr(path, '\\');
        if (p) {
            strcpy(p + 1, "bass_real.dll");
            g_hRealBass = LoadLibraryA(path);
        }
    }
    if (g_hRealBass) {
        #define LOAD(name) real_##name = (name##_t)GetProcAddress(g_hRealBass, #name)
        LOAD(BASS_ChannelSetAttributes);
        LOAD(BASS_MusicPlayEx);
        LOAD(BASS_SetConfig);
        LOAD(BASS_Init);
        LOAD(BASS_Free);
        LOAD(BASS_Start);
        LOAD(BASS_Stop);
        LOAD(BASS_ErrorGetCode);
        LOAD(BASS_MusicLoad);
        LOAD(BASS_ChannelStop);
        #undef LOAD
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Clouds Background Texture Mod
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Game addresses */
#define APP_PTR_ADDR     0x005341E0   /* Global pointer to App struct */
#define GFX_FOG_COLOR_OFF 0x738       /* gfx+0x738 = cached fog color */
#define GFX_DEVICE_OFF    0x154        /* gfx+0x154 = D3D device */

/* Function addresses */
#define SPRITE_CTOR_ADDR     0x0045D0C0
#define SPRITE_RENDERQUAD_ADDR 0x0045D660
#define GRAPHICS_RENDERSCENE  0x00454BC0
#define HOOK_ADDR             0x00454CEE  /* MOV ESI, [EBP+0x738] — 6 bytes */

/* Sprite struct size */
#define SPRITE_SIZE  0xD4

/* __thiscall function pointer typedefs */
typedef void* (__attribute__((thiscall)) *Sprite_ctor_fn_t)(
    void *this_ptr,    /* ECX */
    void *gfx,         /* [ESP+0] */
    const char *path   /* [ESP+4] */
);
typedef void (__attribute__((thiscall)) *Sprite_RenderQuad_fn_t)(
    void *this_ptr,    /* ECX */
    float x,           /* [ESP+0] */
    float y,           /* [ESP+4] */
    float z,           /* [ESP+8] */
    float scale,       /* [ESP+12] */
    void *material     /* [ESP+16] */
);

static Sprite_ctor_fn_t p_Sprite_ctor = (Sprite_ctor_fn_t)SPRITE_CTOR_ADDR;
static Sprite_RenderQuad_fn_t p_Sprite_RenderQuad = (Sprite_RenderQuad_fn_t)SPRITE_RENDERQUAD_ADDR;

/* Level names for config file */
static const char *LEVEL_NAMES[] = {
    "Warm-Up",     /* 0 */
    "Beginner",    /* 1 */
    "Intermediate",/* 2 */
    "Dizzy",       /* 3 */
    "Tower",       /* 4 */
    "Up",          /* 5 */
    "Neon",        /* 6 */
    "Expert",      /* 7 */
    "Odd",         /* 8 */
    "Toob",        /* 9 */
    "Wobbly",      /* 10 */
    "Glass",       /* 11 */
    "Sky",         /* 12 */
    "Master",      /* 13 */
    "Impossible"   /* 14 */
};
#define NUM_LEVELS 15

/* Mod state */
static int g_config[NUM_LEVELS];  /* 1=clouds on, 0=off */
static int g_config_loaded = 0;
static int g_should_render = 0;     /* Set when non-Sky level wants clouds */
static void *g_clouds_sprite = NULL;  /* Lazily-created sprite for non-Sky levels */
static int g_prev_level = -1;       /* Previous detected level */
static int g_sky_zeroed = 0;       /* Whether we zeroed Sky's sprite ptr */
static int g_sky_sprite_saved = 0;  /* Saved original Sky sprite ptr */
static DWORD g_saved_sky_sprite = 0;/* The saved pointer value */
static char g_config_path[MAX_PATH] = {0};
static int g_initialized = 0;

/* Code cave */
static unsigned char *g_cave = NULL;
static unsigned char g_orig_bytes[6];

/* ═══════════════════════════════════════════════════════════════════════════
 * Config File
 * ═══════════════════════════════════════════════════════════════════════════ */

static void init_config_path(void)
{
    HMODULE hSelf = NULL;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                      | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                      (LPCSTR)&init_config_path, &hSelf);
    GetModuleFileNameA(hSelf, g_config_path, MAX_PATH);
    char *p = strrchr(g_config_path, '\\');
    if (p) strcpy(p + 1, "clouds_bg.txt");
    else snprintf(g_config_path, MAX_PATH, "clouds_bg.txt");
}

static void write_default_config(void)
{
    /* Only create if file doesn't exist — don't overwrite user edits */
    DWORD attr = GetFileAttributesA(g_config_path);
    if (attr != INVALID_FILE_ATTRIBUTES) return;  /* File exists, don't overwrite */

    HANDLE h = CreateFileA(g_config_path, GENERIC_WRITE, 0, NULL,
                           CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return;

    char buf[2048];
    int pos = 0;
    pos += snprintf(buf + pos, sizeof(buf) - pos,
        "# Clouds Background Texture Mod\r\n"
        "# Set to 1 to enable clouds, 0 to disable\r\n"
        "# Default: clouds only on Sky Race\r\n"
        "\r\n");

    for (int i = 0; i < NUM_LEVELS; i++) {
        int val = (i == 12) ? 1 : 0;  /* Only Sky Race by default */
        pos += snprintf(buf + pos, sizeof(buf) - pos, "%s = %d\r\n",
                        LEVEL_NAMES[i], val);
    }

    DWORD written;
    WriteFile(h, buf, pos, &written, NULL);
    CloseHandle(h);
}

static void read_config(void)
{
    /* Set defaults */
    for (int i = 0; i < NUM_LEVELS; i++)
        g_config[i] = (i == 12) ? 1 : 0;

    HANDLE h = CreateFileA(g_config_path, GENERIC_READ, FILE_SHARE_READ, NULL,
                           OPEN_EXISTING, 0, NULL);
    if (h == INVALID_HANDLE_VALUE) {
        write_default_config();
        return;
    }

    char buf[2048] = {0};
    DWORD read_bytes = 0;
    ReadFile(h, buf, sizeof(buf) - 1, &read_bytes, NULL);
    CloseHandle(h);
    buf[read_bytes] = 0;

    /* Parse lines: "LevelName = 0/1" */
    char *line = buf;
    while (*line) {
        /* Skip whitespace and comments */
        while (*line == ' ' || *line == '\t' || *line == '\r' || *line == '\n')
            line++;
        if (*line == '#' || *line == 0)
            goto next_line;

        /* Find '=' */
        char *eq = strchr(line, '=');
        if (!eq) goto next_line;

        /* Extract level name */
        char name[64] = {0};
        int name_len = eq - line;
        while (name_len > 0 && (line[name_len-1] == ' ' || line[name_len-1] == '\t'))
            name_len--;
        if (name_len >= (int)sizeof(name)) name_len = sizeof(name) - 1;
        memcpy(name, line, name_len);
        name[name_len] = 0;

        /* Parse value */
        char *val_str = eq + 1;
        while (*val_str == ' ' || *val_str == '\t') val_str++;
        int val = (*val_str == '1') ? 1 : 0;

        /* Match level name */
        for (int i = 0; i < NUM_LEVELS; i++) {
            if (_stricmp(name, LEVEL_NAMES[i]) == 0) {
                g_config[i] = val;
                break;
            }
        }

    next_line:
        while (*line && *line != '\n') line++;
        if (*line == '\n') line++;
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Level Detection
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Returns race index 0-14, or -1 if not in a race */
static int get_race_index(void)
{
    /* App → +0x220 = PlayerProfile → +0x08 = race_idx */
    DWORD app = *(DWORD*)APP_PTR_ADDR;
    if (app == 0) return -1;

    DWORD profile = *(DWORD*)(app + 0x220);
    if (profile == 0) return -1;

    int race_idx = *(int*)(profile + 0x08);
    if (race_idx < 0 || race_idx >= NUM_LEVELS) return -1;

    return race_idx;
}

/* Get board pointer (board IS the scene in Hamsterball) */
static DWORD get_board(void)
{
    DWORD app = *(DWORD*)APP_PTR_ADDR;
    if (app == 0) return 0;

    DWORD profile = *(DWORD*)(app + 0x220);
    if (profile == 0) return 0;

    return *(DWORD*)(profile + 0x0C);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Clouds Sprite Management
 * ═══════════════════════════════════════════════════════════════════════════ */

static void create_clouds_sprite(void *gfx)
{
    if (g_clouds_sprite != NULL) return;

    g_clouds_sprite = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, SPRITE_SIZE);
    if (g_clouds_sprite == NULL) return;

    /* Sprite_ctor(sprite, gfx, "textures\clouds.png") */
    p_Sprite_ctor(g_clouds_sprite, gfx, "textures\\clouds.png");
}

static void destroy_clouds_sprite(void)
{
    if (g_clouds_sprite != NULL) {
        HeapFree(GetProcessHeap(), 0, g_clouds_sprite);
        g_clouds_sprite = NULL;
    }
}

/* Render clouds — called from render hook */
static void __cdecl render_clouds_c(void *gfx)
{
    if (!g_should_render || gfx == NULL) return;

    /* Create sprite lazily */
    if (g_clouds_sprite == NULL) {
        create_clouds_sprite(gfx);
        if (g_clouds_sprite == NULL) return;
    }

    /* Get ball position from camera target:
     * board+0x29D0 = camera target (ball pointer, set by Scene_Render)
     * ball+0x164 = x, +0x168 = y, +0x16C = z */
    DWORD board = get_board();
    if (board == 0) return;

    DWORD ball = *(DWORD*)(board + 0x29D0);
    if (ball == 0 || IsBadReadPtr((void*)ball, 0x180)) return;

    float x = *(float*)(ball + 0x164);
    float y = *(float*)(ball + 0x168);
    float z = *(float*)(ball + 0x16C);

    /* Sprite_RenderQuad(sprite, x, y, z, scale=1.33, material=NULL) */
    p_Sprite_RenderQuad(g_clouds_sprite, x, y, z, 1.33f, NULL);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Render Hook (Graphics_RenderScene at 0x454CEE)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void install_render_hook(void)
{
    /* Hook point: 0x00454CEE
     * Original: 8B B5 38 07 00 00   MOV ESI, [EBP+0x738]  (6 bytes)
     * Patch:    E9 XX XX XX XX 90   JMP cave + NOP          (6 bytes)
     * Return:   0x00454CF4
     *
     * At hook point:
     *   EBP = gfx struct (callee-preserved in __cdecl)
     *   ESI = destination (being written — safe)
     *   EAX, ECX, EDX = dead
     *
     * Cave:
     *   MOV ESI, [EBP+0x738]    ; execute original
     *   PUSH EBP                 ; pass gfx to C function
     *   CALL render_clouds_c     ; __cdecl (preserves ESI, EBP, EBX, EDI)
     *   ADD ESP, 4
     *   JMP 0x454CF4
     */

    /* Save original bytes */
    memcpy(g_orig_bytes, (void*)HOOK_ADDR, 6);

    /* Allocate executable memory for code cave */
    g_cave = VirtualAlloc(NULL, 256, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!g_cave) return;

    unsigned char *p = g_cave;

    /* MOV ESI, [EBP+0x738] — original instruction (6 bytes) */
    *p++ = 0x8B; *p++ = 0xB5; *p++ = 0x38; *p++ = 0x07; *p++ = 0x00; *p++ = 0x00;

    /* PUSH EBP (gfx pointer) */
    *p++ = 0x55;

    /* CALL render_clouds_c (relative call) */
    DWORD call_offset = (DWORD)render_clouds_c - (DWORD)(p + 5);
    *p++ = 0xE8;
    *(DWORD*)p = call_offset;
    p += 4;

    /* ADD ESP, 4 (clean up PUSH EBP) */
    *p++ = 0x83; *p++ = 0xC4; *p++ = 0x04;

    /* JMP 0x454CF4 (return after original 6 bytes) */
    DWORD jmp_offset = (DWORD)0x454CF4 - (DWORD)(p + 5);
    *p++ = 0xE9;
    *(DWORD*)p = jmp_offset;
    p += 4;

    /* Patch the hook point */
    DWORD old_protect;
    VirtualProtect((void*)HOOK_ADDR, 6, PAGE_EXECUTE_READWRITE, &old_protect);

    /* JMP cave (5 bytes) + NOP (1 byte) */
    DWORD cave_offset = (DWORD)g_cave - (DWORD)(HOOK_ADDR + 5);
    *(unsigned char*)HOOK_ADDR = 0xE9;
    *(DWORD*)(HOOK_ADDR + 1) = cave_offset;
    *(unsigned char*)(HOOK_ADDR + 5) = 0x90;

    VirtualProtect((void*)HOOK_ADDR, 6, old_protect, &old_protect);
}

static void uninstall_render_hook(void)
{
    if (g_orig_bytes[0] == 0 && g_orig_bytes[1] == 0) return;

    DWORD old_protect;
    VirtualProtect((void*)HOOK_ADDR, 6, PAGE_EXECUTE_READWRITE, &old_protect);
    memcpy((void*)HOOK_ADDR, g_orig_bytes, 6);
    VirtualProtect((void*)HOOK_ADDR, 6, old_protect, &old_protect);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Polling Thread — level detection + config
 * ═══════════════════════════════════════════════════════════════════════════ */

static DWORD WINAPI polling_thread(LPVOID param)
{
    /* Wait for game to initialize */
    Sleep(3000);

    while (1) {
        /* Read config */
        read_config();
        g_config_loaded = 1;

        /* Get current level */
        int level = get_race_index();

        /* Detect level change */
        if (level != g_prev_level) {
            /* Level changed — reset state */
            g_prev_level = level;
            g_should_render = 0;
            g_sky_zeroed = 0;
            destroy_clouds_sprite();
        }

        if (level >= 0 && level < NUM_LEVELS) {
            DWORD board = get_board();

            if (level == 12) {
                /* Sky Race — clouds are native (board+0x4374) */
                if (g_config[12] == 0) {
                    /* Disable clouds: zero the sprite pointer */
                    if (board && !g_sky_zeroed) {
                        g_saved_sky_sprite = *(DWORD*)(board + 0x4374);
                        g_sky_sprite_saved = 1;
                        *(DWORD*)(board + 0x4374) = 0;
                        g_sky_zeroed = 1;
                    }
                } else {
                    /* Enable: restore original sprite if we zeroed it */
                    if (g_sky_zeroed && board && g_sky_sprite_saved) {
                        *(DWORD*)(board + 0x4374) = g_saved_sky_sprite;
                        g_sky_zeroed = 0;
                    }
                }
            } else {
                /* Non-Sky level */
                if (g_config[level] == 1) {
                    g_should_render = 1;
                } else {
                    g_should_render = 0;
                }
            }
        }

        Sleep(500);
    }
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DLL Entry Point
 * ═══════════════════════════════════════════════════════════════════════════ */

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        load_real_bass();
        init_config_path();
        read_config();
        /* Only create default config if it doesn't exist yet */
        write_default_config();
        read_config();           /* Re-read after writing defaults */

        /* Install render hook */
        install_render_hook();

        /* Start polling thread */
        CreateThread(NULL, 0, polling_thread, NULL, 0, NULL);

        g_initialized = 1;
        break;

    case DLL_PROCESS_DETACH:
        uninstall_render_hook();
        destroy_clouds_sprite();
        break;
    }
    return TRUE;
}
