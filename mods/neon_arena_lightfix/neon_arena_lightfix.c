/*
 * neon_arena_lightfix — Fix missing yellow diffuse light on Neon Arena.
 *
 * Root cause: ArenaLevel_Neon_Init (0x416F40) has two bugs vs Scene_SetupLevelDark
 * (0x416270, Neon Race):
 *
 *   1. First loop (P1 balls at board+0x29D4): writes 3 material blocks
 *      (ambient/diffuse/emissive) but FORGETS to set the glow flag (+0xC80 = 1).
 *      Without this flag, the emissive material is written but never rendered.
 *      The second loop (P2/badballs) correctly sets +0xC80 = 1.
 *
 *   2. After the loops, Scene_SetupLevelDark writes 3 material blocks directly
 *      to the global phys struct (App+0x5DC for P1, App+0x67C for P2).
 *      ArenaLevel_Neon_Init omits these writes entirely.
 *
 * Fix: A background thread polls the App struct. When Neon Arena is detected
 * (board+0x47E4 non-zero), it iterates the ball list (board+0x29D4) and:
 *   - Sets +0xC80 = 1 (glow flag) on each ball
 *   - Writes yellow material colors (R=1, G=1, B=0, A=1) to ambient/diffuse/emissive
 *   - Also writes to App+0x5DC and App+0x67C phys structs if valid
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll neon_arena_lightfix.c \
 *     -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * BASS Proxy Exports (all 10 game imports + extras)
 * ═══════════════════════════════════════════════════════════════════════════ */

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

/* Extra stubs */
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
 * Neon Arena Light Fix
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Game addresses */
#define APP_PTR_ADDR    0x005341E0
#define BALL_VTABLE     0x004CF3A0

/* Float constants */
#define FLOAT_1_0   0x3F800000  /* 1.0f */
#define FLOAT_0_0   0x00000000  /* 0.0f */

/* Material color offsets on phys/ball struct (verified from Scene_SetupLevelDark) */
/* Ambient:  +0x1CC (R), +0x1D0 (G), +0x1D4 (B), +0x1D8 (A) */
/* Diffuse:  +0x1BC (R), +0x1C0 (G), +0x1C4 (B), +0x1C8 (A) */
/* Emissive: +0x1EC (R), +0x1F0 (G), +0x1F4 (B), +0x1F8 (A) */
/* Glow flag: +0xC80 (byte, set to 1 to enable glow rendering) */
/* Has-material flag: +0x204 (byte) */

/* AthenaList structure (at board+0x29D4 for P1 balls) */
/* +0x00: vtable ptr, +0x04: count, +0x08: capacity, +0x0C: ... +0x40C: array ptr */

static volatile int g_running = 1;
static int g_applied = 0;  /* 0 = not yet applied, 1 = applied, reset on level change */

/* Write yellow material colors + glow flag to a ball/phys struct */
static void apply_yellow_glow(char *phys) {
    if (!phys || IsBadReadPtr(phys, 0x210)) return;

    /* Set glow flag (the missing instruction from ArenaLevel_Neon_Init loop 1) */
    *(BYTE*)(phys + 0xC80) = 1;

    /* Ambient = yellow (R=1, G=1, B=0, A=1) */
    *(DWORD*)(phys + 0x1CC) = FLOAT_1_0;
    *(DWORD*)(phys + 0x1D0) = FLOAT_1_0;
    *(DWORD*)(phys + 0x1D4) = FLOAT_0_0;
    *(DWORD*)(phys + 0x1D8) = FLOAT_1_0;

    /* Diffuse = yellow */
    *(DWORD*)(phys + 0x1BC) = FLOAT_1_0;
    *(DWORD*)(phys + 0x1C0) = FLOAT_1_0;
    *(DWORD*)(phys + 0x1C4) = FLOAT_0_0;
    *(DWORD*)(phys + 0x1C8) = FLOAT_1_0;

    /* Emissive = yellow (this is what makes the ball GLOW) */
    *(DWORD*)(phys + 0x1EC) = FLOAT_1_0;
    *(DWORD*)(phys + 0x1F0) = FLOAT_1_0;
    *(DWORD*)(phys + 0x1F4) = FLOAT_0_0;
    *(DWORD*)(phys + 0x1F8) = FLOAT_1_0;

    /* Mark as having material */
    *(BYTE*)(phys + 0x204) = 1;
}

/* Iterate the AthenaList at board+0x29D4 and apply glow to each ball */
static void apply_glow_to_ball_list(char *board) {
    char *ball_list = board + 0x29D4;
    if (IsBadReadPtr(ball_list, 0x410)) return;

    int count = *(int*)(ball_list + 0x04);
    if (count <= 0 || count > 100) return;  /* sanity check */

    int *array = *(int**)(ball_list + 0x40C);
    if (!array || IsBadReadPtr(array, count * 4)) return;

    for (int i = 0; i < count; i++) {
        char *ball = (char*)array[i];
        if (ball && !IsBadReadPtr(ball, 0x210)) {
            /* Verify it's actually a ball by checking vtable */
            if (*(int*)ball == BALL_VTABLE) {
                apply_yellow_glow(ball);
            }
        }
    }
}

/* Check if current level is Neon Arena */
static int is_neon_arena(char *app) {
    if (!app || IsBadReadPtr(app, 0x900)) return 0;

    /* Follow: App → +0x220 (PlayerProfile*) → +0xC (Board*) */
    int *profile = *(int**)(app + 0x220);
    if (!profile || IsBadReadPtr(profile, 0x10)) return 0;
    char *board = *(char**)((char*)profile + 0x0C);
    if (!board || IsBadReadPtr(board, 0x4800)) return 0;

    /* Check board+0x47E4 — Neon Arena stores its emitter SceneObject here.
     * This field is set by ArenaLevel_Neon_Init and is non-zero only in Neon Arena. */
    int emitter_obj = *(int*)(board + 0x47E4);
    return (emitter_obj != 0);
}

/* Get the board pointer from App */
static char* get_board(char *app) {
    if (!app || IsBadReadPtr(app, 0x900)) return NULL;
    int *profile = *(int**)(app + 0x220);
    if (!profile || IsBadReadPtr(profile, 0x10)) return NULL;
    char *board = *(char**)((char*)profile + 0x0C);
    if (!board || IsBadReadPtr(board, 0x4800)) return NULL;
    return board;
}

static DWORD WINAPI neon_fix_thread(LPVOID param) {
    (void)param;
    /* Wait for game to fully load */
    Sleep(3000);

    while (g_running) {
        char *app = *(char**)APP_PTR_ADDR;
        if (app && !IsBadReadPtr(app, 0x900)) {
            if (is_neon_arena(app)) {
                char *board = get_board(app);
                if (board) {
                    /* Apply glow to all balls in the P1 ball list */
                    apply_glow_to_ball_list(board);

                    /* Also write to global phys structs (belt-and-suspenders) */
                    /* App+0x5DC = P1 phys, App+0x67C = P2 phys */
                    char *scene_mgr = *(char**)(board + 0x878);
                    if (scene_mgr && !IsBadReadPtr(scene_mgr, 0x700)) {
                        char *p1_phys = *(char**)(scene_mgr + 0x5DC);
                        if (p1_phys && !IsBadReadPtr(p1_phys, 0x210))
                            apply_yellow_glow(p1_phys);

                        /* P2 only if not human (app+0x677 == 0, same as Neon Race) */
                        BYTE p2_flag = *(BYTE*)(scene_mgr + 0x677);
                        if (p2_flag == 0) {
                            char *p2_phys = *(char**)(scene_mgr + 0x67C);
                            if (p2_phys && !IsBadReadPtr(p2_phys, 0x210))
                                apply_yellow_glow(p2_phys);
                        }
                    }
                }
                /* Re-apply every 2 seconds while in Neon Arena */
                Sleep(2000);
            } else {
                /* Not in Neon Arena — check every 1 second */
                Sleep(1000);
            }
        } else {
            Sleep(1000);
        }
    }
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DLL Entry Point
 * ═══════════════════════════════════════════════════════════════════════════ */

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
    (void)hModule;
    (void)lpReserved;
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        load_real_bass();
        CreateThread(NULL, 0, neon_fix_thread, NULL, 0, NULL);
        break;

    case DLL_PROCESS_DETACH:
        g_running = 0;
        break;
    }
    return TRUE;
}
