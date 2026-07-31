/*
 * time_warp.c — Time Warp Mod
 *
 * A unified bass.dll proxy mod combining 4 systems:
 *   1. Warp system (level transitions, color lerp, flash, timer freeze)
 *   2. Ghost Saver (recording + .ghost file persistence)
 *   3. Ghost Triggers (GT: proximity triggers for ghost balls)
 *   4. Ghost Event (E:GHOST collision events, ghost ball creation, playback)
 *
 * Ghost 2 appears ONLY when warping to the same level, using the player's
 * run recorded in App+0x90C BTT. Ghost 2 is always heliotrope purple.
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll time_warp.c -lwinmm \
 *     -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias -msse2 -mfpmath=sse
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>

/* ================================================================
 * BASS type definitions
 * ================================================================ */
typedef unsigned long long QWORD;
typedef DWORD HSTREAM;
typedef DWORD HMUSIC;
typedef DWORD HFX;

/* ================================================================
 * Game constants and addresses
 * ================================================================ */
#define EXE_BASE                0x00400000
#define APP_PTR                 0x005341E0

/* Function addresses */
#define APP_START_PRACTICE_RACE     0x00428C50
#define APP_START_TOURNAMENT_RACE   0x004288B0
#define DISPATCH_COLLISION_EVENTS  0x0040C5D0
#define APP_FRAME_UPDATE_EPILOGUE  0x0046C1F1
#define BALL_CTOR                  0x004039E0
#define BTT_CTOR_ADDR              0x00427660
#define BTT_DTOR_ADDR              0x004278C0
#define ALIST_APPEND_ADDR          0x00453780
#define ALIST_REMOVE_ADDR          0x004534D0
#define OPERATOR_NEW_ADDR          0x004BA57B
#define OPERATOR_DELETE_ADDR       0x004BA74D
#define BALL_DELETING_DTOR         0x00402A50

/* BTT constants */
#define BTT_SIZE              0x528
#define BTT_VTABLE_ADDR       0x004D262C
#define BTT_AL_COUNT           0x08
#define BTT_LIST_ARRAY         0x410
#define BTT_PLAYBACK_IDX       0x41C
#define BTT_RACE_NAME          0x424
#define BTT_BEST_TIME          0x524
#define SNAP_SIZE              0x28
#define SNAP_DWORDS            10
#define SNAP_BYTES             40
#define NO_TIME                9999999
#define BTT_RACE_TIME          0x420
#define MAX_SEGMENTS           32

/* Ball constants */
#define BALL_SIZE              0xC60
#define BALL_VTABLE            0x00
#define BALL_BOARD             0x14
#define BALL_PLAYER_ID         0x18
#define BALL_POS_X             0x164
#define BALL_POS_Y             0x168
#define BALL_POS_Z             0x16C
#define BALL_VEL_X             0x170
#define BALL_VEL_Y             0x174
#define BALL_VEL_Z             0x178
#define BALL_FACING_X          0x190
#define BALL_FACING_Z          0x194
#define BALL_ROLL_ANGLE        0x150
#define BALL_GRAVITY_PLANE     0x748
#define BALL_SURFACE_A         0x74C
#define BALL_SURFACE_B         0x750
#define BALL_RADIUS            0x284
#define BALL_MAXSPEED          0x188
#define BALL_GRAVITY           0x278
#define BALL_GRAVITY_SCALE     0x27C
#define BALL_MASS              0x1A0
#define BALL_RESPAWN_FLAG      0x281
#define BALL_ALPHA             0x2FC
#define BALL_COLOR_R           0x2AC
#define BALL_COLOR_G           0x2B0
#define BALL_COLOR_B           0x2B4
#define BALL_IN_TAR            0x2CC
#define BALL_DEATH_PENDING     0x2E9
#define BALL_RENDER_JITTER     0x2D4
#define BALL_IMPACT_FREEZE    0x808

/* App offsets */
#define APP_PROFILE_PTR        0x220
#define APP_MUSIC_DEVICE_PTR   0x17C
#define APP_BTT_RECORDING      0x90C
#define APP_BTT_PLAYBACK       0x910
#define APP_5DC_BALL           0x5DC
#define APP_5D6_GOAL_FLAG      0x5D6
#define APP_234_PARTY_MODE     0x234
#define APP_5E4_SCORE          0x5E4
#define APP_5E8_TIMER          0x5E8

/* PlayerProfile offsets */
#define PROFILE_RACE_INDEX     0x08
#define PROFILE_BOARD_PTR      0x0C
#define PROFILE_IS_PRACTICE    0x11
#define PROFILE_SCORE_ARRAY    0x50
#define PROFILE_TIME_ARRAY     0x14

/* Board/Scene offsets */
#define BOARD_APP_PTR          0x878
#define BOARD_GHOST_BALL       0x361C
#define BOARD_MESHWORLD_PTR    0x8AC
#define BOARD_BALL_LIST        0x29D4
#define BOARD_BALL_LIST_DATA   0x2DE0
#define BOARD_GOAL_REACHED     0xCD0
#define BOARD_PAUSED_FLAG      0x874
#define SCENE_FADE_ALPHA       0x3624

/* MeshWorld access */
#define MW_SECTION3_PTR        0x480
#define MW_NODELIST_OFFSET     0x894
#define MW_NODELIST_COUNT      0x898
#define MW_NODELIST_DATA       0xCA0
#define MW_S1_LIST_OFFSET      0x894
#define MW_S1_COUNT_OFFSET     0x898
#define MW_S1_ARRAY_OFFSET     0xCA0

/* Node entry layout (16 bytes each) */
#define NODE_NAME_OFFSET       0x00
#define NODE_X_OFFSET          0x04
#define NODE_Y_OFFSET          0x08
#define NODE_Z_OFFSET           0x0C

/* S1 entry layout (7 DWORDs = 28 bytes) */
#define S1_NAME    0
#define S1_POS_X   1
#define S1_POS_Y   2
#define S1_POS_Z   3
#define S1_ROT_X   4
#define S1_ROT_Y   5
#define S1_ROT_Z   6
#define S1_SIZE    7

/* MusicChannel offsets */
#define MUSIC_CHAN_BASS_CHANNEL  0x08
#define MUSIC_CHAN_VOLUME        0x528
#define MUSIC_CHAN_FADE_RATE     0x52C
#define MUSIC_CHAN_FADE_OUT      0x530
#define MUSIC_CHAN_FADE_IN       0x531
#define MUSIC_DEV_CHANNEL_LIST   0x418

/* Pause-block patch addresses */
#define PAUSE_PATCH_PATH1       0x19D5B
#define PAUSE_PATCH_PATH2       0x130B5
#define PAUSE_PATCH_PATH3       0x0B405

/* Timer freeze patch addresses */
#define TIMER_DEC_PATCH_RVA     0x1B3E5
#define TIMER_DEC_PATCH_SIZE    9
#define TIMER_DEC_RETURN_RVA    0x1B3EE
#define TIMER_DEC_SKIP_RVA      0x1B49D

#define TIMER_INC_PATCH_RVA     0x1B50C
#define TIMER_INC_PATCH_SIZE    5
#define TIMER_INC_RETURN_RVA    0x1B511

/* TT recording NOP patch */
#define TT_RECORDING_NOP_ADDR   0x0041B690
#define TT_RECORDING_NOP_SIZE   7

/* Ghost file format */
#define GHOST_MAGIC             0x47485347  /* "GHSG" */
#define GHOST_VERSION           1

/* Proximity thresholds (matching HB+ plus_level_warp) */
#define WARP_TRIGGER_DIST_SQ    625.0f  /* 25.0 squared */
#define WARP_COOLDOWN_MS        2000
#define DEFAULT_RADIUS          50.0f
#define TRIGGER_COOLDOWN_FRAMES 60
#define MAX_TRIGGERS            64
#define TRIGGER_PREFIX          "GT:"

/* Warp phase durations */
#define RUMBLE_DURATION_MS      2000
#define FLASH_DURATION_MS       150
#define HOLD_DURATION_MS        1000
#define FADE_DURATION_MS        2000
#define REVEAL_DURATION_MS      1000
#define MUSIC_FADE_MS           3000

/* Heliotrope purple (#db03fc) */
#define TARGET_R                (219.0f / 255.0f)
#define TARGET_G                (3.0f / 255.0f)
#define TARGET_B                (252.0f / 255.0f)

#define MAX_MUSIC_CHANNELS      32

/* ================================================================
 * Diagnostic logging — time_warp_log.txt
 * ================================================================ */
static char g_logPath[MAX_PATH] = "";
static char g_ghostDir[MAX_PATH] = "";

static void diag_log(const char *msg) {
    if (g_logPath[0] == '\0') return;
    {
        HANDLE hFile = CreateFileA(g_logPath,
            FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            DWORD written;
            SetFilePointer(hFile, 0, NULL, FILE_END);
            WriteFile(hFile, msg, (DWORD)strlen(msg), &written, NULL);
            WriteFile(hFile, "\r\n", 2, &written, NULL);
            CloseHandle(hFile);
        }
    }
}

static void diag_logf(const char *fmt, ...) {
    char buf[512];
    va_list args;
    if (g_logPath[0] == '\0') return;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    diag_log(buf);
}

/* ================================================================
 * BASS Proxy Layer (merged from all 4 files — most complete set)
 * ================================================================ */
static HMODULE g_hRealBass = NULL;

typedef BOOL (__stdcall *BASS_Init_t)(int, int, DWORD, HWND, const void*);
static BASS_Init_t real_BASS_Init = NULL;
__declspec(dllexport) BOOL __stdcall BASS_Init(int device, int freq, DWORD flags, HWND win, const void *dsguid) {
    if (real_BASS_Init) return real_BASS_Init(device, freq, flags, win, dsguid);
    return TRUE;
}

typedef void (__stdcall *BASS_Free_t)(void);
static BASS_Free_t real_BASS_Free = NULL;
__declspec(dllexport) void __stdcall BASS_Free(void) {
    if (real_BASS_Free) real_BASS_Free();
}

typedef HSTREAM (__stdcall *BASS_StreamCreateFile_t)(BOOL, const void*, QWORD, QWORD, DWORD);
static BASS_StreamCreateFile_t real_BASS_StreamCreateFile = NULL;
__declspec(dllexport) HSTREAM __stdcall BASS_StreamCreateFile(BOOL mem, const void *file, QWORD offset, QWORD length, DWORD flags) {
    if (real_BASS_StreamCreateFile) return real_BASS_StreamCreateFile(mem, file, offset, length, flags);
    return 0;
}

typedef BOOL (__stdcall *BASS_StreamFree_t)(HSTREAM);
static BASS_StreamFree_t real_BASS_StreamFree = NULL;
__declspec(dllexport) BOOL __stdcall BASS_StreamFree(HSTREAM handle) {
    if (real_BASS_StreamFree) return real_BASS_StreamFree(handle);
    return FALSE;
}

typedef HMUSIC (__stdcall *BASS_MusicLoad_t)(BOOL, const void*, DWORD, DWORD, DWORD, DWORD);
static BASS_MusicLoad_t real_BASS_MusicLoad = NULL;
__declspec(dllexport) HMUSIC __stdcall BASS_MusicLoad(BOOL mem, const void *file, DWORD offset, DWORD length, DWORD flags, DWORD freq) {
    if (real_BASS_MusicLoad) return real_BASS_MusicLoad(mem, file, offset, length, flags, freq);
    return (HMUSIC)1;
}

typedef BOOL (__stdcall *BASS_MusicFree_t)(HMUSIC);
static BASS_MusicFree_t real_BASS_MusicFree = NULL;
__declspec(dllexport) BOOL __stdcall BASS_MusicFree(HMUSIC handle) {
    if (real_BASS_MusicFree) return real_BASS_MusicFree(handle);
    return FALSE;
}

typedef int (__stdcall *BASS_ChannelSetAttributes_t)(DWORD, float, int, int);
static BASS_ChannelSetAttributes_t real_BASS_ChannelSetAttributes = NULL;
__declspec(dllexport) int __stdcall BASS_ChannelSetAttributes(DWORD a, float b, int c, int d) {
    if (real_BASS_ChannelSetAttributes) return real_BASS_ChannelSetAttributes(a, b, c, d);
    return 1;
}

typedef int (__stdcall *BASS_MusicPlayEx_t)(DWORD, DWORD, BOOL);
static BASS_MusicPlayEx_t real_BASS_MusicPlayEx = NULL;
__declspec(dllexport) int __stdcall BASS_MusicPlayEx(DWORD a, DWORD b, BOOL c) {
    if (real_BASS_MusicPlayEx) return real_BASS_MusicPlayEx(a, b, c);
    return 1;
}

typedef int (__stdcall *BASS_SetConfig_t)(DWORD, DWORD);
static BASS_SetConfig_t real_BASS_SetConfig = NULL;
__declspec(dllexport) int __stdcall BASS_SetConfig(DWORD a, DWORD b) {
    if (real_BASS_SetConfig) return real_BASS_SetConfig(a, b);
    return 1;
}

typedef int (__stdcall *BASS_Start_t)(void);
static BASS_Start_t real_BASS_Start = NULL;
__declspec(dllexport) void __stdcall BASS_Start(void) {
    if (real_BASS_Start) real_BASS_Start();
}

typedef int (__stdcall *BASS_Stop_t)(void);
static BASS_Stop_t real_BASS_Stop = NULL;
__declspec(dllexport) void __stdcall BASS_Stop(void) {
    if (real_BASS_Stop) real_BASS_Stop();
}

typedef int (__stdcall *BASS_ErrorGetCode_t)(void);
static BASS_ErrorGetCode_t real_BASS_ErrorGetCode = NULL;
__declspec(dllexport) int __stdcall BASS_ErrorGetCode(void) {
    if (real_BASS_ErrorGetCode) return real_BASS_ErrorGetCode();
    return 0;
}

typedef int (__stdcall *BASS_ChannelStop_t)(DWORD);
static BASS_ChannelStop_t real_BASS_ChannelStop = NULL;
__declspec(dllexport) int __stdcall BASS_ChannelStop(DWORD a) {
    if (real_BASS_ChannelStop) return real_BASS_ChannelStop(a);
    return 1;
}

typedef DWORD (__stdcall *BASS_ChannelGetData_t)(DWORD, void*, DWORD);
static BASS_ChannelGetData_t real_BASS_ChannelGetData = NULL;
__declspec(dllexport) DWORD __stdcall BASS_ChannelGetData(DWORD handle, void *buffer, DWORD length) {
    if (real_BASS_ChannelGetData) return real_BASS_ChannelGetData(handle, buffer, length);
    return 0;
}

typedef HFX (__stdcall *BASS_ChannelSetFX_t)(DWORD, DWORD, int);
static BASS_ChannelSetFX_t real_BASS_ChannelSetFX = NULL;
__declspec(dllexport) HFX __stdcall BASS_ChannelSetFX(DWORD handle, DWORD type, int priority) {
    if (real_BASS_ChannelSetFX) return real_BASS_ChannelSetFX(handle, type, priority);
    return 0;
}

/* Extra BASS stubs */
__declspec(dllexport) void __stdcall BASS_Pause(void) {}
__declspec(dllexport) void __stdcall BASS_SetVolume(DWORD a) { (void)a; }
__declspec(dllexport) DWORD __stdcall BASS_GetVolume(void) { return 0; }
__declspec(dllexport) int __stdcall BASS_GetDevice(void) { return 0; }
__declspec(dllexport) int __stdcall BASS_SetDevice(DWORD a) { (void)a; return 1; }
__declspec(dllexport) void __stdcall BASS_GetInfo(void *a) { (void)a; }
__declspec(dllexport) int __stdcall BASS_Update(DWORD a) { (void)a; return 0; }
__declspec(dllexport) DWORD __stdcall BASS_SampleLoad(int a, void *b, DWORD c, DWORD d, DWORD e) { (void)a;(void)b;(void)c;(void)d;(void)e; return 0; }
__declspec(dllexport) int __stdcall BASS_ChannelPlay(DWORD a, BOOL b) { (void)a;(void)b; return 1; }
__declspec(dllexport) int __stdcall BASS_ChannelSetAttribute(DWORD a, DWORD b, float c) { (void)a;(void)b;(void)c; return 1; }
__declspec(dllexport) int __stdcall BASS_ChannelGetAttribute(DWORD a, DWORD b, float *c) { (void)a;(void)b;(void)c; return 1; }
__declspec(dllexport) DWORD __stdcall BASS_ChannelGetLevel(DWORD a) { (void)a; return 0; }
__declspec(dllexport) int __stdcall BASS_ChannelSetPosition(DWORD a, void *b, DWORD c) { (void)a;(void)b;(void)c; return 1; }
__declspec(dllexport) DWORD __stdcall BASS_ChannelGetPosition(DWORD a, DWORD b) { (void)a;(void)b; return 0; }
__declspec(dllexport) int __stdcall BASS_ChannelIsActive(DWORD a) { (void)a; return 0; }
__declspec(dllexport) int __stdcall BASS_ChannelRemoveSync(DWORD a, DWORD b) { (void)a;(void)b; return 1; }
__declspec(dllexport) DWORD __stdcall BASS_ChannelSetSync(DWORD a, DWORD b, DWORD c, void *d, void *e) { (void)a;(void)b;(void)c;(void)d;(void)e; return 0; }
__declspec(dllexport) DWORD __stdcall BASS_SampleCreate(DWORD a, DWORD b, DWORD c, DWORD d, DWORD e) { (void)a;(void)b;(void)c;(void)d;(void)e; return 0; }
__declspec(dllexport) DWORD __stdcall BASS_SampleGetChannel(DWORD a, BOOL b) { (void)a;(void)b; return 0; }

static void load_real_bass(void) {
    g_hRealBass = LoadLibraryA("bass_real.dll");
    if (g_hRealBass == NULL) {
        char path[MAX_PATH];
        HMODULE hSelf = NULL;
        GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                          | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                          (LPCSTR)&load_real_bass, &hSelf);
        if (hSelf && GetModuleFileNameA(hSelf, path, MAX_PATH)) {
            char *p = strrchr(path, '\\');
            if (p) { strcpy(p + 1, "bass_real.dll"); g_hRealBass = LoadLibraryA(path); }
        }
    }
    if (g_hRealBass) {
        real_BASS_Init            = (BASS_Init_t)GetProcAddress(g_hRealBass, "BASS_Init");
        real_BASS_Free            = (BASS_Free_t)GetProcAddress(g_hRealBass, "BASS_Free");
        real_BASS_StreamCreateFile = (BASS_StreamCreateFile_t)GetProcAddress(g_hRealBass, "BASS_StreamCreateFile");
        real_BASS_StreamFree      = (BASS_StreamFree_t)GetProcAddress(g_hRealBass, "BASS_StreamFree");
        real_BASS_MusicLoad       = (BASS_MusicLoad_t)GetProcAddress(g_hRealBass, "BASS_MusicLoad");
        real_BASS_MusicFree       = (BASS_MusicFree_t)GetProcAddress(g_hRealBass, "BASS_MusicFree");
        real_BASS_MusicPlayEx     = (BASS_MusicPlayEx_t)GetProcAddress(g_hRealBass, "BASS_MusicPlayEx");
        real_BASS_SetConfig       = (BASS_SetConfig_t)GetProcAddress(g_hRealBass, "BASS_SetConfig");
        real_BASS_Start           = (BASS_Start_t)GetProcAddress(g_hRealBass, "BASS_Start");
        real_BASS_Stop            = (BASS_Stop_t)GetProcAddress(g_hRealBass, "BASS_Stop");
        real_BASS_ErrorGetCode    = (BASS_ErrorGetCode_t)GetProcAddress(g_hRealBass, "BASS_ErrorGetCode");
        real_BASS_ChannelStop     = (BASS_ChannelStop_t)GetProcAddress(g_hRealBass, "BASS_ChannelStop");
        real_BASS_ChannelSetAttributes = (BASS_ChannelSetAttributes_t)GetProcAddress(g_hRealBass, "BASS_ChannelSetAttributes");
        real_BASS_ChannelGetData  = (BASS_ChannelGetData_t)GetProcAddress(g_hRealBass, "BASS_ChannelGetData");
        real_BASS_ChannelSetFX    = (BASS_ChannelSetFX_t)GetProcAddress(g_hRealBass, "BASS_ChannelSetFX");
    }
}

/* ================================================================
 * Utility functions
 * ================================================================ */

static void patch_bytes(void *addr, const void *data, DWORD size) {
    DWORD old;
    VirtualProtect(addr, size, PAGE_EXECUTE_READWRITE, &old);
    memcpy(addr, data, size);
    VirtualProtect(addr, size, old, &old);
}

static void patch_byte(void *addr, BYTE val) {
    patch_bytes(addr, &val, 1);
}

static void *alloc_executable(DWORD size) {
    return VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
}

static void write_jmp(unsigned char *at, DWORD target) {
    at[0] = 0xE9;
    *(DWORD*)(at + 1) = target - (DWORD)at - 5;
}

static DWORD get_app(void) {
    if (IsBadReadPtr((void*)APP_PTR, 4)) return 0;
    DWORD app = *(DWORD*)APP_PTR;
    if (!app || app < 0x10000) return 0;
    return app;
}

static DWORD get_board(DWORD app) {
    if (!app || IsBadReadPtr((void*)app, 0x1000)) return 0;
    DWORD profile = *(DWORD*)((char*)app + APP_PROFILE_PTR);
    if (!profile || profile < 0x10000) return 0;
    if (IsBadReadPtr((void*)profile, 0x100)) return 0;
    DWORD board = *(DWORD*)((char*)profile + PROFILE_BOARD_PTR);
    if (!board || board < 0x10000) return 0;
    if (IsBadReadPtr((void*)board, 0x4000)) return 0;
    /* Sanity: Board+0x878 should point back to App */
    if (*(DWORD*)((char*)board + BOARD_APP_PTR) != app) return 0;
    return board;
}

static int is_valid_ptr(DWORD ptr) {
    if (!ptr || ptr < 0x10000) return 0;
    return 1;
}

static int is_valid_ball(DWORD ball) {
    if (!ball || ball < 0x10000) return 0;
    return 1;
}

/* ================================================================
 * Inline asm wrappers for __thiscall game functions
 * MinGW __thiscall function pointers silently fail — must use asm.
 * ================================================================ */

static void call_btt_ctor(void *btt) {
    DWORD ctorAddr = BTT_CTOR_ADDR;
    __asm__ volatile(
        "movl %0, %%ecx\n\t"
        "call *%1\n\t"
        : : "r"(btt), "r"(ctorAddr)
        : "eax", "ecx", "edx", "memory"
    );
}

static void call_btt_dtor(void *btt) {
    DWORD dtorAddr = BTT_DTOR_ADDR;
    __asm__ volatile(
        "push $1\n\t"
        "movl %0, %%ecx\n\t"
        "call *%1\n\t"
        : : "r"(btt), "r"(dtorAddr)
        : "eax", "ecx", "edx", "memory"
    );
}

static void call_alist_append(DWORD *list, void *item) {
    DWORD appendAddr = ALIST_APPEND_ADDR;
    __asm__ volatile(
        "push %1\n\t"
        "movl %0, %%ecx\n\t"
        "call *%2\n\t"
        : : "r"(list), "r"(item), "r"(appendAddr)
        : "eax", "ecx", "edx", "memory"
    );
}

static void call_alist_remove(DWORD *list, void *item) {
    DWORD removeAddr = ALIST_REMOVE_ADDR;
    __asm__ volatile(
        "push %1\n\t"
        "movl %0, %%ecx\n\t"
        "call *%2\n\t"
        : : "r"(list), "r"(item), "r"(removeAddr)
        : "eax", "ecx", "edx", "memory"
    );
}

static void *game_operator_new(size_t size) {
    typedef void* (__cdecl *operator_new_t)(size_t);
    operator_new_t op_new = (operator_new_t)OPERATOR_NEW_ADDR;
    return op_new(size);
}

static void game_free(void *ptr) {
    typedef void (__cdecl *game_free_t)(void*);
    game_free_t gf = (game_free_t)OPERATOR_DELETE_ADDR;
    gf(ptr);
}

static void call_ball_ctor(DWORD ball, DWORD board) {
    DWORD ctorAddr = BALL_CTOR;
    __asm__ volatile(
        "push %1\n\t"
        "movl %0, %%ecx\n\t"
        "call *%2\n\t"
        : : "r"(ball), "r"(board), "r"(ctorAddr)
        : "eax", "ecx", "edx", "memory"
    );
}

static void call_ball_dtor(DWORD ball) {
    DWORD dtorAddr = BALL_DELETING_DTOR;
    DWORD flags = 1;
    __asm__ volatile(
        "push %1\n\t"
        "movl %0, %%ecx\n\t"
        "call *%2\n\t"
        : : "r"(ball), "r"(flags), "r"(dtorAddr)
        : "eax", "ecx", "edx", "memory"
    );
}

/* ================================================================
 * Thread synchronization
 * ================================================================ */
static CRITICAL_SECTION g_cs;

/* ================================================================
 * Ghost 2 subsystem — secondary ghost ball from same-level warp
 * ================================================================ */

typedef struct {
    DWORD    ball;           /* Ball* — secondary ghost ball struct */
    DWORD    btt;            /* BTT* — standalone BestTimeTracker */
    DWORD   *snapshots;      /* malloc'd snapshot array (frameCount * SNAP_DWORDS DWORDs) */
    int      frameCount;     /* total frames in recording */
    int      playbackIdx;    /* current playback frame */
    BOOL     active;         /* is this ghost currently playing? */
} Ghost2;

static Ghost2 g_ghost2 = {0, 0, NULL, 0, 0, FALSE};

/* In-memory capture buffer for same-level warp */
static DWORD (*g_ghost2Capture)[SNAP_DWORDS] = NULL;
static int   g_ghost2CaptureCount = 0;
static BOOL  g_ghost2Pending = FALSE;

/* ================================================================
 * Multi-Segment Ghost System — Time Warp levels
 * ================================================================ */

typedef struct {
    int  currentSegment;   /* which (N) file is playing (1-based) */
    int  playbackIdx;       /* saved playback index for current segment */
    int  totalSegments;     /* how many (N) files exist on disk */
    DWORD btt;              /* current BTT at App+0x910 (0 if none) */
    char raceName[128];     /* base race name for file lookup */
    BOOL active;            /* is Ghost 1 chaining active? */
} Ghost1State;

static Ghost1State g_ghost1 = {0, 0, 0, 0, "", FALSE};

/* Segment tracking */
static int  g_segmentCounter = 0;     /* current attempt segment count */
static int  g_segmentTimes[MAX_SEGMENTS]; /* times for each segment in current attempt */
static int  g_segmentCount = 0;       /* number of entries used in g_segmentTimes */

/* Forward declaration — defined in ghost saver section */
static int is_time_trial_active(void);
static char g_twRaceName[128] = "";   /* race name for Time Warp level files */
static BOOL g_isTimeWarpLevel = FALSE; /* set true when first same-level warp fires */

/* Create Ghost 2 from captured snapshot data.
 * Allocates a Ball (0xC60) + BTT (0x528), adds ball to AthenaList. */
static void ghost2_create(DWORD board, DWORD *snaps, int count) {
    if (!board || !snaps || count <= 0) return;

    /* Allocate BTT */
    void *btt = game_operator_new(BTT_SIZE);
    if (!btt) {
        diag_log("[ghost2] ERROR: operator_new failed for BTT");
        return;
    }
    call_btt_ctor(btt);
    DWORD vtable = *(DWORD*)btt;
    if (vtable != BTT_VTABLE_ADDR) {
        diag_logf("[ghost2] BTT ctor vtable=0x%X (expected 0x%X)", vtable, BTT_VTABLE_ADDR);
        game_free(btt);
        return;
    }

    /* Set BTT fields */
    *(DWORD*)((char*)btt + BTT_BEST_TIME) = NO_TIME;
    *(DWORD*)((char*)btt + BTT_PLAYBACK_IDX) = 0;

    /* Append snapshots to BTT's AthenaList */
    DWORD *alist = (DWORD*)((char*)btt + 0x04);
    for (int i = 0; i < count; i++) {
        DWORD *snap = (DWORD*)game_operator_new(SNAP_SIZE);
        if (!snap) continue;
        memcpy(snap, snaps + i * SNAP_DWORDS, SNAP_BYTES);
        call_alist_append(alist, snap);
    }

    /* Allocate Ball */
    DWORD ballAddr = (DWORD)game_operator_new(BALL_SIZE);
    if (!ballAddr) {
        diag_log("[ghost2] ERROR: operator_new failed for ball");
        call_btt_dtor(btt);
        return;
    }

    /* Ball_ctor(ball, board) */
    call_ball_ctor(ballAddr, board);
    DWORD ballVtable = *(DWORD*)ballAddr;
    if (!ballVtable || IsBadReadPtr((void*)ballVtable, 4)) {
        diag_logf("[ghost2] Ball_ctor failed vtable=0x%X", ballVtable);
        game_free((void*)ballAddr);
        call_btt_dtor(btt);
        return;
    }

    /* Call vtable[1] — Ball_SetupCollisionRender */
    DWORD func1 = *(DWORD*)(ballVtable + 0x04);
    if (func1 && !IsBadReadPtr((void*)func1, 4)) {
        __asm__ volatile(
            "movl %0, %%ecx\n\t"
            "call *%1\n\t"
            : : "r"(ballAddr), "r"(func1)
            : "eax", "ecx", "edx", "memory"
        );
    }

    /* Set ghost-specific fields */
    *(DWORD*)(ballAddr + BALL_PLAYER_ID) = 0xFFFFFFFF;  /* -1 = ghost */
    *(float*)(ballAddr + BALL_GRAVITY) = 0.5f;
    *(float*)(ballAddr + BALL_GRAVITY_SCALE) = 0.1f;
    *(float*)(ballAddr + BALL_RADIUS) = 26.0f;
    *(float*)(ballAddr + BALL_MASS) = 0.8f;
    *(float*)(ballAddr + BALL_MAXSPEED) = 1000.0f;
    *(BYTE*)(ballAddr + BALL_RESPAWN_FLAG) = 0;
    *(float*)(ballAddr + BALL_ALPHA) = 0.45f;
    *(float*)(ballAddr + BALL_POS_X) = 0.0f;
    *(float*)(ballAddr + BALL_POS_Y) = 0.0f;
    *(float*)(ballAddr + BALL_POS_Z) = 0.0f;

    /* Set heliotrope purple color */
    *(float*)(ballAddr + BALL_COLOR_R) = TARGET_R;
    *(float*)(ballAddr + BALL_COLOR_G) = TARGET_G;
    *(float*)(ballAddr + BALL_COLOR_B) = TARGET_B;

    /* Add to ball AthenaList at board+0x29D4 */
    DWORD *ballList = (DWORD*)((char*)board + BOARD_BALL_LIST);
    call_alist_append(ballList, (void*)ballAddr);

    /* Store in Ghost2 struct */
    g_ghost2.ball = ballAddr;
    g_ghost2.btt = (DWORD)btt;
    g_ghost2.snapshots = (DWORD*)snaps;
    g_ghost2.frameCount = count;
    g_ghost2.playbackIdx = 0;
    g_ghost2.active = TRUE;

    diag_logf("[ghost2] Created: ball=0x%X btt=0x%X frames=%d", ballAddr, (DWORD)btt, count);
}

/* Destroy Ghost 2: remove from AthenaList, destroy ball + BTT */
static void ghost2_destroy(void) {
    if (g_ghost2.ball) {
        DWORD ball = g_ghost2.ball;
        if (!IsBadReadPtr((void*)ball, 0x100)) {
            /* Remove from ball AthenaList first to prevent use-after-free */
            DWORD app = get_app();
            if (app) {
                DWORD profile = *(DWORD*)((char*)app + APP_PROFILE_PTR);
                if (profile && !IsBadReadPtr((void*)profile, 0x100)) {
                    DWORD board = *(DWORD*)((char*)profile + PROFILE_BOARD_PTR);
                    if (board && !IsBadReadPtr((void*)board, 0x4000)) {
                        DWORD *ballList = (DWORD*)((char*)board + BOARD_BALL_LIST);
                        call_alist_remove(ballList, (void*)ball);
                        diag_logf("[ghost2] Ball removed from AthenaList at 0x%X", ball);
                    }
                }
            }
            DWORD vt = *(DWORD*)ball;
            if (vt && !IsBadReadPtr((void*)vt, 4)) {
                call_ball_dtor(ball);
                diag_logf("[ghost2] Ball destroyed at 0x%X", ball);
            }
        }
        g_ghost2.ball = 0;
    }

    if (g_ghost2.btt) {
        DWORD btt = g_ghost2.btt;
        if (!IsBadReadPtr((void*)btt, 0x100)) {
            DWORD vt = *(DWORD*)btt;
            if (vt == BTT_VTABLE_ADDR) {
                call_btt_dtor((void*)btt);
                diag_logf("[ghost2] BTT destroyed at 0x%X", btt);
            } else {
                game_free((void*)btt);
                diag_logf("[ghost2] BTT freed via game_free (bad vtable) at 0x%X", btt);
            }
        }
        g_ghost2.btt = 0;
    }

    if (g_ghost2.snapshots) {
        free(g_ghost2.snapshots);
        g_ghost2.snapshots = NULL;
    }

    g_ghost2.frameCount = 0;
    g_ghost2.playbackIdx = 0;
    g_ghost2.active = FALSE;
}

/* Capture BTT recording from App+0x90C into in-memory buffer */
static void ghost2_capture(void) {
    DWORD app = get_app();
    if (!app) return;

    DWORD btt = 0;
    if (!IsBadReadPtr((void*)(app + APP_BTT_RECORDING), 4))
        btt = *(DWORD*)(app + APP_BTT_RECORDING);

    if (!btt || btt < 0x10000 || IsBadReadPtr((void*)btt, 0x100)) {
        diag_log("[ghost2] No BTT recording at App+0x90C to capture");
        return;
    }

    /* Read count from BTT AthenaList */
    if (IsBadReadPtr((void*)(btt + BTT_AL_COUNT), 4)) return;
    DWORD count = *(DWORD*)(btt + BTT_AL_COUNT);
    if (count == 0 || count >= 200000) {
        diag_logf("[ghost2] Invalid snapshot count %d", count);
        return;
    }

    /* Read data array pointer */
    if (IsBadReadPtr((void*)(btt + BTT_LIST_ARRAY), 4)) return;
    DWORD *data = *(DWORD**)(btt + BTT_LIST_ARRAY);
    if (!data || (DWORD)data < 0x10000 || IsBadReadPtr(data, count * 4)) {
        diag_log("[ghost2] Invalid BTT data array");
        return;
    }

    /* Allocate capture buffer */
    DWORD (*buf)[SNAP_DWORDS] = (DWORD(*)[SNAP_DWORDS])malloc(count * SNAP_BYTES);
    if (!buf) {
        diag_log("[ghost2] malloc failed for capture buffer");
        return;
    }

    int valid = 0;
    for (DWORD i = 0; i < count; i++) {
        DWORD *snap = (DWORD*)data[i];
        if (snap && (DWORD)snap > 0x10000 && !IsBadReadPtr(snap, SNAP_BYTES)) {
            memcpy(buf[valid], snap, SNAP_BYTES);
            valid++;
        }
    }

    /* Free previous capture if any */
    if (g_ghost2Capture) {
        free(g_ghost2Capture);
    }
    g_ghost2Capture = buf;
    g_ghost2CaptureCount = valid;
    g_ghost2Pending = TRUE;

    diag_logf("[ghost2] Captured %d snapshots from BTT at 0x%X", valid, btt);
}

/* ================================================================
 * Multi-Segment Ghost File Operations
 * ================================================================ */

/* Build base filename from race name for Time Warp segment files.
 * Strips " RACE" suffix, TitleCases the rest, sanitizes. */
static void tw_race_name_to_filename(const char *raceName, char *out, int outLen) {
    char base[128];
    strncpy(base, raceName, sizeof(base) - 1);
    base[sizeof(base) - 1] = '\0';

    int len = strlen(base);
    if (len >= 5 && _stricmp(base + len - 5, " RACE") == 0) {
        base[len - 5] = '\0';
    }

    int newWord = 1;
    for (int i = 0; base[i]; i++) {
        char c = base[i];
        if (c == ' ' || c == '-' || c == '_')
            newWord = 1;
        else if (newWord) {
            if (c >= 'a' && c <= 'z') base[i] = c - 32;
            newWord = 0;
        } else {
            if (c >= 'A' && c <= 'Z') base[i] = c + 32;
        }
    }

    for (int i = 0; base[i]; i++) {
        char c = base[i];
        if (c == '\\' || c == '/' || c == ':' || c == '*' ||
            c == '?' || c == '"' || c == '<' || c == '>' || c == '|')
            base[i] = '_';
    }

    snprintf(out, outLen, "%s", base);
}

/* Save ghost segment to LevelName[N].ghost or LevelName(N).ghost */
static void save_segment_ghost(const char *raceName, int segment, int time,
                              DWORD (*snaps)[SNAP_DWORDS], int count,
                              char bracket) {
    if (segment < 1 || segment > MAX_SEGMENTS) return;
    char base[128];
    tw_race_name_to_filename(raceName, base, sizeof(base));

    char path[MAX_PATH];
    snprintf(path, sizeof(path), "%s%s%c%d%c.ghost", g_ghostDir, base,
             bracket, segment, (bracket == '[') ? ']' : ')');

    char tmpPath[MAX_PATH];
    snprintf(tmpPath, sizeof(tmpPath), "%s.tmp", path);

    HANDLE h = CreateFileA(tmpPath, GENERIC_WRITE, 0, NULL,
                          CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) {
        diag_logf("[seg] ERROR: cannot create %s", tmpPath);
        return;
    }
    DWORD written;
    int ok = 1;
    DWORD magic = GHOST_MAGIC;
    DWORD version = GHOST_VERSION;
    DWORD frameCount = (DWORD)count;

    if (!WriteFile(h, &magic, 4, &written, NULL) || written != 4) ok = 0;
    if (ok && (!WriteFile(h, &version, 4, &written, NULL) || written != 4)) ok = 0;
    if (ok && (!WriteFile(h, (DWORD*)&time, 4, &written, NULL) || written != 4)) ok = 0;
    if (ok && (!WriteFile(h, &frameCount, 4, &written, NULL) || written != 4)) ok = 0;

    if (ok && count > 0) {
        DWORD totalBytes = (DWORD)count * SNAP_BYTES;
        if (!WriteFile(h, snaps, totalBytes, &written, NULL) || written != totalBytes)
            ok = 0;
    }

    if (ok) FlushFileBuffers(h);
    CloseHandle(h);

    if (ok) {
        if (MoveFileExA(tmpPath, path, MOVEFILE_REPLACE_EXISTING)) {
            diag_logf("[seg] Saved %s (%d frames, time=%d)", path, count, time);
        } else {
            diag_logf("[seg] ERROR: MoveFileEx failed (err=%d)", GetLastError());
            DeleteFileA(tmpPath);
        }
    } else {
        diag_logf("[seg] ERROR: write failed for %s", tmpPath);
        DeleteFileA(tmpPath);
    }
}

/* Load a segment ghost file. Returns malloc'd snapshots or NULL. */
static DWORD* load_segment_ghost(const char *raceName, int segment,
                                 int *outCount, int *outTime,
                                 char bracket) {
    if (segment < 1 || segment > MAX_SEGMENTS) return NULL;
    char base[128];
    tw_race_name_to_filename(raceName, base, sizeof(base));

    char path[MAX_PATH];
    snprintf(path, sizeof(path), "%s%s%c%d%c.ghost", g_ghostDir, base,
             bracket, segment, (bracket == '[') ? ']' : ')');

    HANDLE hf = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hf == INVALID_HANDLE_VALUE) return NULL;

    DWORD magic, version, time, frameCount, br;
    DWORD *snaps = NULL;

    if (ReadFile(hf, &magic, 4, &br, NULL) && magic == GHOST_MAGIC &&
        ReadFile(hf, &version, 4, &br, NULL) && version == GHOST_VERSION &&
        ReadFile(hf, &time, 4, &br, NULL) &&
        ReadFile(hf, &frameCount, 4, &br, NULL) &&
        frameCount > 0 && frameCount < 200000) {

        DWORD totalBytes = frameCount * SNAP_BYTES;
        snaps = (DWORD*)malloc(totalBytes);
        if (snaps) {
            DWORD bytesRead = 0;
            if (ReadFile(hf, snaps, totalBytes, &bytesRead, NULL) &&
                bytesRead == totalBytes) {
                *outCount = (int)frameCount;
                *outTime = (int)time;
            } else {
                free(snaps);
                snaps = NULL;
            }
        }
    }
    CloseHandle(hf);
    return snaps;
}

/* Count how many (N).ghost files exist for a race name */
static int count_confirmed_segments(const char *raceName) {
    int count = 0;
    for (int i = 1; i <= MAX_SEGMENTS; i++) {
        char base[128];
        tw_race_name_to_filename(raceName, base, sizeof(base));
        char path[MAX_PATH];
        snprintf(path, sizeof(path), "%s%s(%d).ghost", g_ghostDir, base, i);
        DWORD attr = GetFileAttributesA(path);
        if (attr != INVALID_FILE_ATTRIBUTES) count++;
        else break;
    }
    return count;
}

/* Get total time from confirmed (N) ghost files */
static int get_confirmed_total_time(const char *raceName) {
    int total = 0;
    int found = 0;
    for (int i = 1; i <= MAX_SEGMENTS; i++) {
        char base[128];
        tw_race_name_to_filename(raceName, base, sizeof(base));
        char path[MAX_PATH];
        snprintf(path, sizeof(path), "%s%s(%d).ghost", g_ghostDir, base, i);
        HANDLE hf = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL,
                               OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hf == INVALID_HANDLE_VALUE) break;
        DWORD magic, version, time, br;
        if (ReadFile(hf, &magic, 4, &br, NULL) && magic == GHOST_MAGIC &&
            ReadFile(hf, &version, 4, &br, NULL) && version == GHOST_VERSION &&
            ReadFile(hf, &time, 4, &br, NULL)) {
            total += (int)time;
            found++;
        }
        CloseHandle(hf);
    }
    return found > 0 ? total : NO_TIME;
}

/* Rename all [N].ghost to (N).ghost */
static void rename_temp_to_confirmed(const char *raceName, int maxSeg) {
    char base[128];
    tw_race_name_to_filename(raceName, base, sizeof(base));
    for (int i = 1; i <= maxSeg; i++) {
        char src[MAX_PATH], dst[MAX_PATH];
        snprintf(src, sizeof(src), "%s%s[%d].ghost", g_ghostDir, base, i);
        snprintf(dst, sizeof(dst), "%s%s(%d).ghost", g_ghostDir, base, i);
        if (MoveFileExA(src, dst, MOVEFILE_REPLACE_EXISTING)) {
            diag_logf("[seg] Renamed %s -> %s", src, dst);
        }
    }
}

/* Delete all [N].ghost files */
static void delete_temp_segments(const char *raceName, int maxSeg) {
    char base[128];
    tw_race_name_to_filename(raceName, base, sizeof(base));
    for (int i = 1; i <= maxSeg; i++) {
        char path[MAX_PATH];
        snprintf(path, sizeof(path), "%s%s[%d].ghost", g_ghostDir, base, i);
        if (DeleteFileA(path)) {
            diag_logf("[seg] Deleted temp segment %s", path);
        }
    }
}

/* Delete all (N).ghost files */
static void delete_confirmed_segments(const char *raceName, int maxSeg) {
    char base[128];
    tw_race_name_to_filename(raceName, base, sizeof(base));
    for (int i = 1; i <= maxSeg; i++) {
        char path[MAX_PATH];
        snprintf(path, sizeof(path), "%s%s(%d).ghost", g_ghostDir, base, i);
        if (DeleteFileA(path)) {
            diag_logf("[seg] Deleted confirmed segment %s", path);
        }
    }
}

/* Inject a ghost segment into App+0x910 (Ghost 1 playback slot).
 * Creates a new BTT and writes it to the slot. */
static void ghost1_load_segment(const char *raceName, int segment,
                                int playbackIdx, char bracket) {
    DWORD app = get_app();
    if (!app) return;

    int segTime = NO_TIME;
    int segCount = 0;
    DWORD *snaps = load_segment_ghost(raceName, segment, &segCount, &segTime, bracket);
    if (!snaps || segCount == 0) {
        if (snaps) free(snaps);
        return;
    }

    /* Destroy old playback BTT if we own it */
    if (g_ghost1.btt && g_ghost1.btt > 0x10000) {
        if (!IsBadReadPtr((void*)g_ghost1.btt, 4)) {
            DWORD vt = *(DWORD*)g_ghost1.btt;
            if (vt == BTT_VTABLE_ADDR)
                call_btt_dtor((void*)g_ghost1.btt);
            else
                game_free((void*)g_ghost1.btt);
        }
        g_ghost1.btt = 0;
    }

    /* Also check and clean up any existing playback BTT in App+0x910 */
    if (!IsBadReadPtr((void*)(app + APP_BTT_PLAYBACK), 4)) {
        DWORD existing = *(DWORD*)(app + APP_BTT_PLAYBACK);
        if (existing && existing > 0x10000 && existing != g_ghost1.btt) {
            if (!IsBadReadPtr((void*)existing, 4)) {
                DWORD vt = *(DWORD*)existing;
                if (vt == BTT_VTABLE_ADDR)
                    call_btt_dtor((void*)existing);
                else
                    game_free((void*)existing);
            }
        }
    }

    /* Create new BTT */
    void *btt = game_operator_new(BTT_SIZE);
    if (!btt) { free(snaps); return; }
    call_btt_ctor(btt);
    DWORD vtable = *(DWORD*)btt;
    if (vtable != BTT_VTABLE_ADDR) {
        diag_logf("[ghost1] BTT ctor vtable=0x%X (expected 0x%X)", vtable, BTT_VTABLE_ADDR);
        game_free(btt);
        free(snaps);
        return;
    }

    *(DWORD*)((char*)btt + BTT_BEST_TIME) = (DWORD)segTime;

    /* Append snapshots */
    DWORD *alist = (DWORD*)((char*)btt + 0x04);
    for (int i = 0; i < segCount; i++) {
        DWORD *snap = (DWORD*)game_operator_new(SNAP_SIZE);
        if (!snap) continue;
        memcpy(snap, snaps + i * SNAP_DWORDS, SNAP_BYTES);
        call_alist_append(alist, snap);
    }

    free(snaps);

    /* Set playback index */
    if (playbackIdx < 0) playbackIdx = 0;
    if (playbackIdx >= segCount) playbackIdx = 0;
    *(DWORD*)((char*)btt + BTT_PLAYBACK_IDX) = (DWORD)playbackIdx;

    /* Write to App+0x910 */
    *(DWORD*)(app + APP_BTT_PLAYBACK) = (DWORD)btt;
    g_ghost1.btt = (DWORD)btt;
    g_ghost1.currentSegment = segment;
    g_ghost1.playbackIdx = playbackIdx;

    diag_logf("[ghost1] Loaded segment %d (%c): %d frames, time=%d, playIdx=%d",
              segment, bracket, segCount, segTime, playbackIdx);
}

/* Save Ghost 1's playback index before warp destroys App+0x910 */
static void ghost1_save_state(void) {
    DWORD app = get_app();
    if (!app) return;

    if (!IsBadReadPtr((void*)(app + APP_BTT_PLAYBACK), 4)) {
        DWORD btt = *(DWORD*)(app + APP_BTT_PLAYBACK);
        if (btt && btt > 0x10000 && !IsBadReadPtr((void*)(btt + BTT_PLAYBACK_IDX), 4)) {
            g_ghost1.playbackIdx = *(DWORD*)((char*)btt + BTT_PLAYBACK_IDX);
            g_ghost1.btt = btt;
            diag_logf("[ghost1] Saved state: segment=%d, playIdx=%d, btt=0x%X",
                      g_ghost1.currentSegment, g_ghost1.playbackIdx, btt);
        }
    }
}

/* After level reloads, load the appropriate segment into Ghost 1 */
static void ghost1_restore_after_warp(void) {
    if (!g_ghost1.active || !g_twRaceName[0]) return;
    int seg = g_ghost1.currentSegment;
    if (seg < 1) seg = 1;

    /* Try confirmed (N) first, then temp [N] */
    int segTime = 0, segCount = 0;
    DWORD *snaps = load_segment_ghost(g_twRaceName, seg, &segCount, &segTime, '(');
    char bracket = '(';
    if (!snaps) {
        snaps = load_segment_ghost(g_twRaceName, seg, &segCount, &segTime, '[');
        bracket = '[';
    }
    if (snaps) {
        free(snaps);
        ghost1_load_segment(g_twRaceName, seg, g_ghost1.playbackIdx, bracket);
    } else {
        diag_logf("[ghost1] No segment %d found to restore", seg);
    }
}

/* Check if Ghost 1 reached end of current segment and advance */
static void ghost1_check_advance(void) {
    if (!g_ghost1.active || !g_ghost1.btt || !g_twRaceName[0]) return;

    DWORD btt = g_ghost1.btt;
    if (IsBadReadPtr((void*)btt, 0x600)) return;

    DWORD playIdx = *(DWORD*)((char*)btt + BTT_PLAYBACK_IDX);
    DWORD count = *(DWORD*)((char*)btt + BTT_AL_COUNT);

    if (count == 0 || playIdx < count - 1) return;

    /* Reached end of segment — advance to next */
    int nextSeg = g_ghost1.currentSegment + 1;
    if (nextSeg > g_ghost1.totalSegments) {
        /* Loop back to segment 1 */
        nextSeg = 1;
    }

    diag_logf("[ghost1] Segment %d ended, advancing to %d",
              g_ghost1.currentSegment, nextSeg);

    /* Check if next confirmed (N) segment exists — fallback to temp [N] */
    int segTime = 0, segCount = 0;
    DWORD *snaps = load_segment_ghost(g_twRaceName, nextSeg, &segCount, &segTime, '(');
    if (!snaps) {
        snaps = load_segment_ghost(g_twRaceName, nextSeg, &segCount, &segTime, '[');
        if (snaps) {
            diag_logf("[ghost1] Advancing to temp segment %d", nextSeg);
        }
    }
    if (snaps) {
        free(snaps);
        char bracket = (g_ghost1.currentSegment > 0 && g_ghost1.currentSegment <= g_ghost1.totalSegments) ? '(' : '[';
        ghost1_load_segment(g_twRaceName, nextSeg, 0, bracket);
    } else {
        diag_logf("[ghost1] No segment %d to advance to", nextSeg);
    }
}

/* Initialize Ghost 1 state for a Time Warp level */
static void ghost1_init_for_tw(const char *raceName) {
    strncpy(g_ghost1.raceName, raceName, sizeof(g_ghost1.raceName) - 1);
    g_ghost1.raceName[sizeof(g_ghost1.raceName) - 1] = '\0';

    g_ghost1.totalSegments = count_confirmed_segments(raceName);
    if (g_ghost1.totalSegments > 0) {
        g_ghost1.currentSegment = 1;
        g_ghost1.playbackIdx = 0;
        g_ghost1.active = TRUE;
        diag_logf("[ghost1] Initialized for TW: %d confirmed segments", g_ghost1.totalSegments);
    } else {
        /* No confirmed segments yet — will chain from temp files as they're created */
        g_ghost1.currentSegment = 1;
        g_ghost1.playbackIdx = 0;
        g_ghost1.active = TRUE;
        diag_log("[ghost1] No confirmed segments for TW level yet; chaining from temp files");
    }
}

/* Capture segment from BTT recording and save as temp segment file.
 * Also stores the segment time. */
static void save_warp_segment(void) {
    DWORD app = get_app();
    if (!app || !g_twRaceName[0]) return;

    DWORD btt = 0;
    if (!IsBadReadPtr((void*)(app + APP_BTT_RECORDING), 4))
        btt = *(DWORD*)(app + APP_BTT_RECORDING);

    if (!btt || btt < 0x10000 || IsBadReadPtr((void*)btt, 0x600)) {
        diag_log("[seg] No BTT recording to save as segment");
        return;
    }

    /* Read segment time */
    int segTime = NO_TIME;
    if (!IsBadReadPtr((void*)(btt + BTT_RACE_TIME), 4))
        segTime = *(int*)((char*)btt + BTT_RACE_TIME);
    if (segTime == NO_TIME || segTime <= 0) {
        /* Fall back to best_time field */
        if (!IsBadReadPtr((void*)(btt + BTT_BEST_TIME), 4))
            segTime = *(int*)((char*)btt + BTT_BEST_TIME);
    }

    /* Read snapshot count and data */
    if (IsBadReadPtr((void*)(btt + BTT_AL_COUNT), 4)) return;
    DWORD count = *(DWORD*)(btt + BTT_AL_COUNT);
    if (count == 0 || count >= 200000) {
        diag_logf("[seg] Invalid snapshot count %d", count);
        return;
    }

    if (IsBadReadPtr((void*)(btt + BTT_LIST_ARRAY), 4)) return;
    DWORD *data = *(DWORD**)(btt + BTT_LIST_ARRAY);
    if (!data || (DWORD)data < 0x10000 || IsBadReadPtr(data, count * 4)) {
        diag_log("[seg] Invalid BTT data array");
        return;
    }

    /* Allocate snapshot buffer */
    DWORD (*snaps)[SNAP_DWORDS] = (DWORD(*)[SNAP_DWORDS])malloc(count * SNAP_BYTES);
    if (!snaps) {
        diag_log("[seg] malloc failed for segment save");
        return;
    }

    int valid = 0;
    for (DWORD i = 0; i < count; i++) {
        DWORD *snap = (DWORD*)data[i];
        if (snap && (DWORD)snap > 0x10000 && !IsBadReadPtr(snap, SNAP_BYTES)) {
            memcpy(snaps[valid], snap, SNAP_BYTES);
            valid++;
        }
    }

    if (valid > 0) {
        g_segmentCounter++;
        if (g_segmentCounter <= MAX_SEGMENTS) {
            save_segment_ghost(g_twRaceName, g_segmentCounter, segTime, snaps, valid, '[');
            if (g_segmentCount < MAX_SEGMENTS) {
                g_segmentTimes[g_segmentCount] = segTime;
                g_segmentCount++;
            }
            g_isTimeWarpLevel = TRUE;
        }
    }

    free(snaps);
}

/* On goal touch: save final segment, compare total, rename/discard */
static void handle_tw_goal_touch(void) {
    if (!g_isTimeWarpLevel || !g_twRaceName[0]) return;

    DWORD app = get_app();
    if (!app) return;

    /* Save the final segment */
    save_warp_segment();

    /* Compute total time */
    int totalTime = 0;
    for (int i = 0; i < g_segmentCount && i < MAX_SEGMENTS; i++) {
        totalTime += g_segmentTimes[i];
    }

    /* In TT mode, App+0x5E8 counts up — use that as the authoritative total */
    int tt = is_time_trial_active();
    if (tt) {
        if (!IsBadReadPtr((void*)(app + APP_5E8_TIMER), 4)) {
            int timerVal = *(int*)((char*)app + APP_5E8_TIMER);
            if (timerVal > 0 && timerVal < NO_TIME)
                totalTime = timerVal;
        }
    }

    diag_logf("[seg] Goal touch: segments=%d, totalTime=%d", g_segmentCount, totalTime);

    /* Compare with previous best */
    int prevBest = get_confirmed_total_time(g_twRaceName);
    int prevSegCount = count_confirmed_segments(g_twRaceName);

    if (prevSegCount == 0) {
        /* No previous best — rename all [N] to (N) */
        rename_temp_to_confirmed(g_twRaceName, g_segmentCounter);
        diag_logf("[seg] First best: renamed %d segments", g_segmentCounter);
    } else if (totalTime < prevBest) {
        /* New best — delete old (N), rename [N] to (N) */
        delete_confirmed_segments(g_twRaceName, prevSegCount);
        rename_temp_to_confirmed(g_twRaceName, g_segmentCounter);
        diag_logf("[seg] New best! totalTime=%d < prevBest=%d", totalTime, prevBest);
    } else {
        /* Not better — discard */
        delete_temp_segments(g_twRaceName, g_segmentCounter);
        diag_logf("[seg] Not best: discarded (totalTime=%d >= prevBest=%d)", totalTime, prevBest);
    }

    /* Reset segment tracking */
    g_segmentCounter = 0;
    g_segmentCount = 0;
    memset(g_segmentTimes, 0, sizeof(g_segmentTimes));
}

/* Ghost 2 per-frame playback: read next snapshot, write to ball */
static void ghost2_playback(void) {
    if (!g_ghost2.active || !g_ghost2.ball || !g_ghost2.snapshots) return;

    int idx = g_ghost2.playbackIdx;
    if (idx >= g_ghost2.frameCount) {
        idx = g_ghost2.frameCount - 1;  /* clamp to last frame */
    }

    DWORD *snap = g_ghost2.snapshots + idx * SNAP_DWORDS;
    DWORD ball = g_ghost2.ball;

    if (IsBadReadPtr((void*)ball, 0xC60)) return;

    /* Write position, facing, roll, radius, gravity, surface */
    *(float*)(ball + BALL_POS_X) = *(float*)(snap + 0);
    *(float*)(ball + BALL_POS_Y) = *(float*)(snap + 1);
    *(float*)(ball + BALL_POS_Z) = *(float*)(snap + 2);
    *(float*)(ball + BALL_FACING_X) = *(float*)(snap + 3);
    *(float*)(ball + BALL_FACING_Z) = *(float*)(snap + 4);
    *(float*)(ball + BALL_ROLL_ANGLE) = *(float*)(snap + 5);
    *(float*)(ball + BALL_GRAVITY_PLANE) = *(float*)(snap + 6);
    *(float*)(ball + BALL_SURFACE_A) = *(float*)(snap + 7);
    *(float*)(ball + BALL_SURFACE_B) = *(float*)(snap + 8);
    *(float*)(ball + BALL_RADIUS) = *(float*)(snap + 9);

    /* Zero velocity/force accumulators so physics doesn't fight position writes.
     * The ghost ball is purely kinematic — the game's physics engine will
     * push it away if we leave non-zero velocities. */
    *(float*)(ball + BALL_VEL_X) = 0.0f;
    *(float*)(ball + BALL_VEL_Y) = 0.0f;
    *(float*)(ball + BALL_VEL_Z) = 0.0f;

    /* Maintain heliotrope purple color every frame */
    *(float*)(ball + BALL_COLOR_R) = TARGET_R;
    *(float*)(ball + BALL_COLOR_G) = TARGET_G;
    *(float*)(ball + BALL_COLOR_B) = TARGET_B;
    *(float*)(ball + BALL_ALPHA) = 0.45f;

    /* Advance playback */
    if (g_ghost2.playbackIdx < g_ghost2.frameCount - 1) {
        g_ghost2.playbackIdx++;
    }
}

/* Check for board pointer change and cleanup Ghost 2 */
static void ghost2_check_board_change(DWORD board) {
    if (!g_ghost2.active) {
        /* If we have a pending capture and a new board, create Ghost 2 */
        EnterCriticalSection(&g_cs);
        int pending = g_ghost2Pending;
        if (pending && board && !IsBadReadPtr((void*)board, 0x4000)) {
            DWORD *capture = (DWORD*)g_ghost2Capture;
            int captureCount = g_ghost2CaptureCount;
            DWORD app = get_app();
            if (app) {
                /* Check party mode — Ghost 2 doesn't work in Party */
                BYTE partyMode = 0;
                if (!IsBadReadPtr((void*)(app + APP_234_PARTY_MODE), 1))
                    partyMode = *(BYTE*)(app + APP_234_PARTY_MODE);
                if (partyMode == 0) {
                    g_ghost2Pending = FALSE;
                    LeaveCriticalSection(&g_cs);
                    ghost2_create(board, capture, captureCount);
                    return;
                } else {
                    diag_log("[ghost2] Party mode — skipping Ghost 2 creation");
                    if (g_ghost2Capture) {
                        free(g_ghost2Capture);
                        g_ghost2Capture = NULL;
                    }
                }
                g_ghost2Pending = FALSE;
            }
        }
        LeaveCriticalSection(&g_cs);
        return;
    }

    /* If board changed or became invalid, destroy Ghost 2 */
    if (!board) {
        diag_log("[ghost2] Board lost — destroying Ghost 2");
        ghost2_destroy();
    }
}

/* ================================================================
 * Ghost Saver subsystem — recording + .ghost file persistence
 * ================================================================ */

static DWORD (*g_rawSnaps)[SNAP_DWORDS] = NULL;
static int   g_rawCount = 0;
static int   g_rawCapacity = 0;

static char g_currentRaceName[128] = "";
static char g_hookRaceName[128] = "";
static int  g_recording = 0;
static int  g_raceFinished = 0;
static int  g_prevGoalFlag = 0;
static DWORD g_prevRecording = 0;
static DWORD g_savedOldPlayback = 0;
static DWORD g_dummyRecording = 0;

#define MAX_SNAPSHOTS 5000

static int is_time_trial_active(void) {
    DWORD app = get_app();
    if (!app) return 0;
    if (IsBadReadPtr((void*)(app + APP_PROFILE_PTR), 4)) return 0;
    DWORD profile = *(DWORD*)(app + APP_PROFILE_PTR);
    if (!profile || profile < 0x10000) return 0;
    if (IsBadReadPtr((void*)(profile + PROFILE_IS_PRACTICE), 1)) return 0;
    if (*(BYTE*)(profile + PROFILE_IS_PRACTICE) == 0) return 0;
    if (IsBadReadPtr((void*)(app + APP_234_PARTY_MODE), 1)) return 0;
    if (*(BYTE*)(app + APP_234_PARTY_MODE) != 0) return 0;
    return 1;
}

/* Returns 1 if currently in a Tournament race (not practice, not party, race active). */
static int is_tournament_active(void) {
    DWORD app = get_app();
    if (!app) return 0;
    if (IsBadReadPtr((void*)(app + APP_PROFILE_PTR), 4)) return 0;
    DWORD profile = *(DWORD*)(app + APP_PROFILE_PTR);
    if (!profile || profile < 0x10000) return 0;
    if (IsBadReadPtr((void*)(profile + PROFILE_IS_PRACTICE), 1)) return 0;
    /* Tournament = NOT practice (profile+0x11 == 0) */
    if (*(BYTE*)(profile + PROFILE_IS_PRACTICE) != 0) return 0;
    /* NOT party mode */
    if (IsBadReadPtr((void*)(app + APP_234_PARTY_MODE), 1)) return 0;
    if (*(BYTE*)(app + APP_234_PARTY_MODE) != 0) return 0;
    /* Must have a board (race is running) */
    if (IsBadReadPtr((void*)(profile + PROFILE_BOARD_PTR), 4)) return 0;
    DWORD board = *(DWORD*)((char*)profile + PROFILE_BOARD_PTR);
    if (!board || board < 0x10000) return 0;
    return 1;
}

static int is_time_trial_precheck(void) {
    DWORD app = get_app();
    if (!app) return 0;
    if (IsBadReadPtr((void*)(app + APP_234_PARTY_MODE), 1)) return 0;
    if (*(BYTE*)(app + APP_234_PARTY_MODE) != 0) return 0;
    return 1;
}

static int get_race_name(char *out, int outLen) {
    out[0] = '\0';
    DWORD app = get_app();
    if (!app) return 0;
    if (IsBadReadPtr((void*)(app + APP_BTT_RECORDING), 4)) return 0;
    DWORD btt = *(DWORD*)(app + APP_BTT_RECORDING);
    if (!btt || btt < 0x10000) return 0;
    if (IsBadReadPtr((void*)(btt + BTT_RACE_NAME), 1)) return 0;
    char *name = (char*)((char*)btt + BTT_RACE_NAME);
    if (name[0] < 0x20 || name[0] > 0x7E) return 0;
    for (int i = 0; i < 64 && name[i]; i++) {
        if (name[i] < 0x20 || name[i] > 0x7E) { name[i] = '\0'; break; }
    }
    strncpy(out, name, outLen - 1);
    out[outLen - 1] = '\0';
    return 1;
}

static void race_name_to_filename(const char *raceName, char *out, int outLen) {
    char base[128];
    strncpy(base, raceName, sizeof(base) - 1);
    base[sizeof(base) - 1] = '\0';

    int len = strlen(base);
    if (len >= 5 && _stricmp(base + len - 5, " RACE") == 0) {
        base[len - 5] = '\0';
    }

    int newWord = 1;
    for (int i = 0; base[i]; i++) {
        char c = base[i];
        if (c == ' ' || c == '-' || c == '_')
            newWord = 1;
        else if (newWord) {
            if (c >= 'a' && c <= 'z') base[i] = c - 32;
            newWord = 0;
        } else {
            if (c >= 'A' && c <= 'Z') base[i] = c + 32;
        }
    }

    for (int i = 0; base[i]; i++) {
        char c = base[i];
        if (c == '\\' || c == '/' || c == ':' || c == '*' ||
            c == '?' || c == '"' || c == '<' || c == '>' || c == '|')
            base[i] = '_';
    }

    snprintf(out, outLen, "%s%s.ghost", g_ghostDir, base);
}

static int get_saved_time(const char *raceName) {
    char path[MAX_PATH];
    race_name_to_filename(raceName, path, sizeof(path));

    HANDLE h = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return NO_TIME;

    int result = NO_TIME;
    DWORD magic, version, time;
    DWORD br;

    if (ReadFile(h, &magic, 4, &br, NULL) && magic == GHOST_MAGIC &&
        ReadFile(h, &version, 4, &br, NULL) && version == GHOST_VERSION &&
        ReadFile(h, &time, 4, &br, NULL)) {
        result = (int)time;
    }

    CloseHandle(h);
    return result;
}

static void save_ghost_for_race(const char *raceName, int time,
                                DWORD (*snaps)[SNAP_DWORDS], int count) {
    char path[MAX_PATH];
    race_name_to_filename(raceName, path, sizeof(path));

    char tmpPath[MAX_PATH];
    snprintf(tmpPath, sizeof(tmpPath), "%s.tmp", path);

    HANDLE h = CreateFileA(tmpPath, GENERIC_WRITE, 0, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) {
        diag_logf("[ghost_saver] ERROR: cannot create %s", tmpPath);
        return;
    }
    DWORD written;
    int ok = 1;

    DWORD magic = GHOST_MAGIC;
    DWORD version = GHOST_VERSION;
    DWORD frameCount = (DWORD)count;

    if (!WriteFile(h, &magic, 4, &written, NULL) || written != 4) ok = 0;
    if (ok && (!WriteFile(h, &version, 4, &written, NULL) || written != 4)) ok = 0;
    if (ok && (!WriteFile(h, (DWORD*)&time, 4, &written, NULL) || written != 4)) ok = 0;
    if (ok && (!WriteFile(h, &frameCount, 4, &written, NULL) || written != 4)) ok = 0;

    if (ok && count > 0) {
        DWORD totalBytes = (DWORD)count * SNAP_BYTES;
        if (!WriteFile(h, snaps, totalBytes, &written, NULL) || written != totalBytes) {
            ok = 0;
        }
    }

    if (ok) FlushFileBuffers(h);
    CloseHandle(h);

    if (ok) {
        if (MoveFileExA(tmpPath, path, MOVEFILE_REPLACE_EXISTING)) {
            diag_logf("[ghost_saver] Saved %s (%d frames, time=%d)", path, count, time);
        } else {
            diag_logf("[ghost_saver] ERROR: MoveFileEx failed (err=%d)", GetLastError());
            DeleteFileA(tmpPath);
        }
    } else {
        diag_logf("[ghost_saver] ERROR: write failed for %s", path);
        DeleteFileA(tmpPath);
    }
}

static int snaps_reserve(int needed) {
    if (needed <= g_rawCapacity) return 1;
    int newCap = g_rawCapacity ? g_rawCapacity : MAX_SNAPSHOTS;
    while (newCap < needed)
        newCap *= 2;
    DWORD (*newBuf)[SNAP_DWORDS] = (DWORD(*)[SNAP_DWORDS])
        realloc(g_rawSnaps, newCap * SNAP_BYTES);
    if (!newBuf) return 0;
    g_rawSnaps = newBuf;
    g_rawCapacity = newCap;
    return 1;
}

static void snaps_reset(void) {
    if (g_rawSnaps) {
        free(g_rawSnaps);
        g_rawSnaps = NULL;
    }
    g_rawCount = 0;
    g_rawCapacity = 0;
}

static void inject_saved_ghost(const char *raceName) {
    DWORD app = get_app();
    if (!app) return;

    char path[MAX_PATH];
    race_name_to_filename(raceName, path, sizeof(path));

    HANDLE hf = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hf == INVALID_HANDLE_VALUE) {
        diag_logf("[ghost_saver] No ghost file for '%s' (%s)", raceName, path);
        return;
    }

    int savedTime = NO_TIME;
    DWORD (*savedSnaps)[SNAP_DWORDS] = NULL;
    int savedCount = 0;

    DWORD magic, version, time, frameCount, br;

    if (ReadFile(hf, &magic, 4, &br, NULL) && magic == GHOST_MAGIC &&
        ReadFile(hf, &version, 4, &br, NULL) && version == GHOST_VERSION &&
        ReadFile(hf, &time, 4, &br, NULL) &&
        ReadFile(hf, &frameCount, 4, &br, NULL) &&
        frameCount > 0 && frameCount < 200000) {

        savedTime = (int)time;
        savedSnaps = (DWORD(*)[SNAP_DWORDS])malloc(frameCount * SNAP_DWORDS * sizeof(DWORD));
        if (savedSnaps) {
            DWORD totalBytes = frameCount * SNAP_BYTES;
            DWORD bytesRead = 0;
            if (ReadFile(hf, savedSnaps, totalBytes, &bytesRead, NULL) &&
                bytesRead == totalBytes) {
                savedCount = (int)frameCount;
            } else {
                free(savedSnaps);
                savedSnaps = NULL;
            }
        }
    }
    CloseHandle(hf);

    if (!savedSnaps || savedCount == 0) {
        if (savedSnaps) free(savedSnaps);
        return;
    }

    diag_logf("[ghost_saver] Loading ghost: '%s' time=%d frames=%d", raceName, savedTime, savedCount);

    void *btt = game_operator_new(BTT_SIZE);
    if (!btt) { free(savedSnaps); return; }

    call_btt_ctor(btt);

    DWORD vtable = *(DWORD*)btt;
    if (vtable != BTT_VTABLE_ADDR) {
        diag_logf("[ghost_saver] BTT ctor vtable=0x%X", vtable);
        free(savedSnaps);
        game_free(btt);
        return;
    }

    *(DWORD*)((char*)btt + BTT_BEST_TIME) = savedTime;
    char *bttName = (char*)((char*)btt + BTT_RACE_NAME);
    strncpy(bttName, raceName, 127);
    bttName[127] = '\0';

    DWORD *alist = (DWORD*)((char*)btt + 0x04);
    for (int i = 0; i < savedCount; i++) {
        DWORD *snap = (DWORD*)game_operator_new(SNAP_SIZE);
        if (!snap) continue;
        memcpy(snap, savedSnaps[i], SNAP_BYTES);
        call_alist_append(alist, snap);
    }

    *(DWORD*)((char*)btt + BTT_PLAYBACK_IDX) = 0;
    *(DWORD*)(app + APP_BTT_PLAYBACK) = (DWORD)btt;
    free(savedSnaps);

    diag_logf("[ghost_saver] Ghost injected: %d snapshots into App+0x910 (btt=0x%X)", savedCount, (DWORD)btt);
}

static int get_race_name_table_count(void) {
    DWORD *nameTable = (DWORD*)0x004F7080;
    int i;
    for (i = 0; i < 64; i++) {
        if (IsBadReadPtr(nameTable + i, 4)) return i;
        DWORD namePtr = nameTable[i];
        if (!namePtr || namePtr < 0x400000) return i;
        if (IsBadReadPtr((void*)namePtr, 2)) return i;
        char c = *(char*)namePtr;
        if (c < 0x20 || c > 0x7E) return i;
    }
    return i;
}

static int get_race_name_by_index(DWORD race_index, char *out, int outLen) {
    int tableCount = get_race_name_table_count();
    if ((int)race_index >= tableCount) return 0;
    DWORD *nameTable = (DWORD*)0x004F7080;
    char *name = (char*)nameTable[race_index];
    if (!name || (DWORD)name < 0x400000) return 0;
    if (IsBadReadPtr(name, 2)) return 0;
    if (name[0] < 0x20 || name[0] > 0x7E) return 0;
    strncpy(out, name, outLen - 1);
    out[outLen - 1] = '\0';
    return 1;
}

static void check_race_state(void) {
    DWORD app = get_app();
    if (!app) return;

    EnterCriticalSection(&g_cs);

    int tt = is_time_trial_active();
    int tourney = is_tournament_active();
    int inRace = tt || tourney;

    if (!inRace) {
        if (g_recording) {
            diag_logf("[ghost_saver] Left TT mode (was recording %d frames)", g_rawCount);
            g_recording = 0;
            g_raceFinished = 0;
            snaps_reset();
            g_prevGoalFlag = 0;
            g_currentRaceName[0] = '\0';
            g_hookRaceName[0] = '\0';
        }
        /* Clean up dummy recording BTT */
        if (g_dummyRecording && g_dummyRecording > 0x10000) {
            DWORD curr90C = 0;
            if (!IsBadReadPtr((void*)(app + APP_BTT_RECORDING), 4))
                curr90C = *(DWORD*)(app + APP_BTT_RECORDING);
            if (curr90C == g_dummyRecording) {
                if (!IsBadReadPtr((void*)g_dummyRecording, 4)) {
                    DWORD vt = *(DWORD*)g_dummyRecording;
                    if (vt == BTT_VTABLE_ADDR) {
                        call_btt_dtor((void*)g_dummyRecording);
                        diag_logf("[ghost_saver] Cleaned up dummy BTT at 0x%X", g_dummyRecording);
                    } else {
                        game_free((void*)g_dummyRecording);
                    }
                }
                *(DWORD*)(app + APP_BTT_RECORDING) = 0;
            }
            g_dummyRecording = 0;
        }
        if (!IsBadReadPtr((void*)(app + APP_BTT_RECORDING), 4))
            g_prevRecording = *(DWORD*)(app + APP_BTT_RECORDING);
        LeaveCriticalSection(&g_cs);
        return;
    }

    /* Goal flag monitoring for Time Warp — works in both TT and Tournament */
    if (!IsBadReadPtr((void*)(app + APP_5D6_GOAL_FLAG), 1)) {
        BYTE goalFlag = *(BYTE*)(app + APP_5D6_GOAL_FLAG);
        if (goalFlag && !g_prevGoalFlag) {
            g_raceFinished = 1;
            /* Time Warp level goal touch handling */
            if (g_isTimeWarpLevel && g_segmentCounter > 0) {
                handle_tw_goal_touch();
            }

            /* Only TT mode saves normal ghost files */
            if (tt) {
                int finishTime = NO_TIME;
                DWORD btt = 0;
                if (!IsBadReadPtr((void*)(app + APP_BTT_RECORDING), 4)) {
                    btt = *(DWORD*)(app + APP_BTT_RECORDING);
                    if (btt && btt > 0x10000 && !IsBadReadPtr((void*)(btt + BTT_BEST_TIME), 4))
                        finishTime = *(DWORD*)(btt + BTT_BEST_TIME);
                }
                diag_logf("[ghost_saver] GOAL! finishTime=%d", finishTime);

                if (finishTime != NO_TIME && btt && g_currentRaceName[0]) {
                    /* Skip normal ghost save for Time Warp levels (handled by TW system) */
                    if (!g_isTimeWarpLevel) {
                        if (!IsBadReadPtr((void*)(btt + BTT_AL_COUNT), 4)) {
                            DWORD count = *(DWORD*)(btt + BTT_AL_COUNT);
                            if (!IsBadReadPtr((void*)(btt + BTT_LIST_ARRAY), 4)) {
                                DWORD *data = *(DWORD**)(btt + BTT_LIST_ARRAY);
                                if (count > 0 && count < 200000 && data &&
                                    (DWORD)data > 0x10000 && !IsBadReadPtr(data, count * 4)) {
                                    if (snaps_reserve((int)count)) {
                                        g_rawCount = 0;
                                        for (DWORD i = 0; i < count; i++) {
                                            DWORD *snap = (DWORD*)data[i];
                                            if (snap && (DWORD)snap > 0x10000 &&
                                                !IsBadReadPtr(snap, SNAP_BYTES)) {
                                                memcpy(g_rawSnaps[g_rawCount], snap, SNAP_BYTES);
                                                g_rawCount++;
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        if (g_rawCount > 0) {
                            save_ghost_for_race("Previous_Run", finishTime, g_rawSnaps, g_rawCount);
                            int existingTime = get_saved_time(g_currentRaceName);
                            if (existingTime == NO_TIME || finishTime < existingTime) {
                                save_ghost_for_race(g_currentRaceName, finishTime, g_rawSnaps, g_rawCount);
                            }
                        } else {
                            g_raceFinished = 0;
                        }
                    }
                }
            }
        }
        g_prevGoalFlag = goalFlag;
    }

    /* Only TT mode monitors BTT recording changes */
    if (tt) {
        DWORD currRecording = 0;
        if (!IsBadReadPtr((void*)(app + APP_BTT_RECORDING), 4))
            currRecording = *(DWORD*)(app + APP_BTT_RECORDING);

        if (currRecording != g_prevRecording && currRecording && currRecording > 0x10000) {
            g_prevRecording = currRecording;

            char raceName[128];
            if (g_hookRaceName[0]) {
                strncpy(raceName, g_hookRaceName, sizeof(raceName) - 1);
                raceName[sizeof(raceName) - 1] = '\0';
            } else {
                get_race_name(raceName, sizeof(raceName));
            }
            if (raceName[0]) {
                strncpy(g_currentRaceName, raceName, sizeof(g_currentRaceName) - 1);
                g_currentRaceName[sizeof(g_currentRaceName) - 1] = '\0';
                snaps_reset();
                g_recording = 1;
                g_raceFinished = 0;
                g_prevGoalFlag = 0;
                diag_logf("[ghost_saver] RACE START: '%s' (BTT=0x%X)", raceName, currRecording);
            } else {
                g_prevRecording = 0;
            }
        }
    }

    LeaveCriticalSection(&g_cs);
}

static volatile BOOL g_shuttingDown = FALSE;

static DWORD WINAPI ghost_saver_thread(LPVOID param) {
    (void)param;
    Sleep(3000);
    diag_log("[ghost_saver] Thread started");
    while (!g_shuttingDown) {
        Sleep(16);
        check_race_state();
    }
    diag_log("[ghost_saver] Thread exiting");
    return 0;
}

/* App_StartPracticeRace detour hook */
#define ADDR_APP_START_PRACTICE  0x00428C50
#define HOOK_BYTES               7
#define TRAMPOLINE_SIZE          16

static unsigned char *g_trampoline = NULL;
static unsigned char g_origBytes[HOOK_BYTES];
static int g_hookInstalled = 0;

void hook_impl(DWORD app, DWORD race_index) {
    /* Clean up stale dummy */
    if (g_dummyRecording && g_dummyRecording > 0x10000) {
        DWORD curr90C = 0;
        if (!IsBadReadPtr((void*)(app + APP_BTT_RECORDING), 4))
            curr90C = *(DWORD*)(app + APP_BTT_RECORDING);
        if (curr90C == g_dummyRecording) {
            if (!IsBadReadPtr((void*)g_dummyRecording, 4)) {
                DWORD vt = *(DWORD*)g_dummyRecording;
                if (vt == BTT_VTABLE_ADDR)
                    call_btt_dtor((void*)g_dummyRecording);
                else
                    game_free((void*)g_dummyRecording);
            }
            *(DWORD*)(app + APP_BTT_RECORDING) = 0;
        }
        g_dummyRecording = 0;
    }

    g_savedOldPlayback = 0;
    EnterCriticalSection(&g_cs);

    if (is_time_trial_precheck()) {
        char raceName[128] = "";
        if (get_race_name_by_index(race_index, raceName, sizeof(raceName)) && raceName[0]) {
            diag_logf("[ghost_saver] HOOK: pre-inject for race '%s' (index=%d)", raceName, race_index);
            strncpy(g_hookRaceName, raceName, sizeof(g_hookRaceName) - 1);
            g_hookRaceName[sizeof(g_hookRaceName) - 1] = '\0';

            int savedTime = get_saved_time(raceName);
            if (savedTime != NO_TIME) {
                int injectFailed = 0;
                if (!IsBadReadPtr((void*)(app + APP_BTT_PLAYBACK), 4)) {
                    DWORD existing = *(DWORD*)(app + APP_BTT_PLAYBACK);
                    if (existing && existing > 0x10000) {
                        g_savedOldPlayback = existing;
                    }
                    inject_saved_ghost(raceName);

                    DWORD newPlayback = 0;
                    if (!IsBadReadPtr((void*)(app + APP_BTT_PLAYBACK), 4))
                        newPlayback = *(DWORD*)(app + APP_BTT_PLAYBACK);

                    if (g_savedOldPlayback && newPlayback == g_savedOldPlayback) {
                        g_savedOldPlayback = 0;
                        injectFailed = 1;
                    }

                    if (!injectFailed && newPlayback && newPlayback > 0x10000 &&
                        !IsBadReadPtr((void*)(app + APP_BTT_RECORDING), 4)) {
                        DWORD recording = *(DWORD*)(app + APP_BTT_RECORDING);
                        if (recording && recording > 0x10000 &&
                            !IsBadReadPtr((void*)(recording + BTT_BEST_TIME), 4)) {
                            int oldTime = *(int*)((char*)recording + BTT_BEST_TIME);
                            if (oldTime != NO_TIME)
                                *(int*)((char*)recording + BTT_BEST_TIME) = NO_TIME;
                        }
                        if (!recording || recording < 0x10000) {
                            void *dummyRec = game_operator_new(BTT_SIZE);
                            if (dummyRec) {
                                call_btt_ctor(dummyRec);
                                DWORD vt = *(DWORD*)dummyRec;
                                if (vt == BTT_VTABLE_ADDR) {
                                    *(DWORD*)((char*)dummyRec + BTT_BEST_TIME) = NO_TIME;
                                    *(DWORD*)(app + APP_BTT_RECORDING) = (DWORD)dummyRec;
                                    g_dummyRecording = (DWORD)dummyRec;
                                } else {
                                    game_free(dummyRec);
                                }
                            }
                        }
                    }
                }
            } else {
                if (!IsBadReadPtr((void*)(app + APP_BTT_PLAYBACK), 4)) {
                    DWORD existing = *(DWORD*)(app + APP_BTT_PLAYBACK);
                    if (existing && existing > 0x10000) {
                        g_savedOldPlayback = existing;
                    }
                    *(DWORD*)(app + APP_BTT_PLAYBACK) = 0;
                }
            }
        }
    }

    /* Call original App_StartPracticeRace via trampoline */
    __asm__ volatile(
        "mov %0, %%ecx\n"
        "push %2\n"
        "call *%1\n"
        : : "r"(app), "r"((void*)g_trampoline), "r"(race_index)
        : "eax", "ecx", "edx", "memory"
    );

    /* Destroy old playback BTT */
    if (g_savedOldPlayback && g_savedOldPlayback > 0x10000) {
        if (!IsBadReadPtr((void*)g_savedOldPlayback, 4)) {
            DWORD vt = *(DWORD*)g_savedOldPlayback;
            if (vt == BTT_VTABLE_ADDR) {
                call_btt_dtor((void*)g_savedOldPlayback);
            }
        }
        g_savedOldPlayback = 0;
    }

    LeaveCriticalSection(&g_cs);
}

__attribute__((naked, used)) static void hook_App_StartPracticeRace(void) {
    __asm__ volatile(
        "pushl %%eax\n"
        "pushl %%ecx\n"
        "pushl %%edx\n"
        "movl 16(%%esp), %%eax\n"
        "pushl %%eax\n"
        "pushl %%ecx\n"
        "call _hook_impl\n"
        "addl $8, %%esp\n"
        "popl %%edx\n"
        "popl %%ecx\n"
        "popl %%eax\n"
        "ret $4\n"
        : : : "memory"
    );
}

static void install_practice_hook(void) {
    unsigned char *target = (unsigned char*)ADDR_APP_START_PRACTICE;
    memcpy(g_origBytes, target, HOOK_BYTES);

    g_trampoline = (unsigned char*)VirtualAlloc(NULL, TRAMPOLINE_SIZE,
        MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!g_trampoline) return;

    memcpy(g_trampoline, g_origBytes, HOOK_BYTES);
    DWORD jmp_src = (DWORD)(g_trampoline + HOOK_BYTES + 5);
    DWORD jmp_dst = ADDR_APP_START_PRACTICE + HOOK_BYTES;
    g_trampoline[HOOK_BYTES + 0] = 0xE9;
    *(DWORD*)(g_trampoline + HOOK_BYTES + 1) = jmp_dst - jmp_src;

    DWORD oldProtect;
    if (!VirtualProtect(target, HOOK_BYTES, PAGE_EXECUTE_READWRITE, &oldProtect)) return;

    DWORD hookAddr = (DWORD)&hook_App_StartPracticeRace;
    target[0] = 0xE9;
    *(DWORD*)(target + 1) = hookAddr - (DWORD)target - 5;
    target[5] = 0x90;
    target[6] = 0x90;
    VirtualProtect(target, HOOK_BYTES, oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), target, HOOK_BYTES);
    g_hookInstalled = 1;
    diag_log("[ghost_saver] App_StartPracticeRace hook installed");
}

/* ================================================================
 * App_StartTournamentRace hook — creates BTT for recording
 * The game doesn't create a BTT in Tournament mode. We hook the
 * function and create one at App+0x90C after it returns, so
 * BestTimeTracker_RecordSnapshot has a valid target.
 * ================================================================ */

#define TOURNAMENT_HOOK_BYTES    7
#define TOURNAMENT_TRAMPOLINE_SIZE 16

static unsigned char *g_tournamentTrampoline = NULL;
static unsigned char g_tournamentOrigBytes[TOURNAMENT_HOOK_BYTES];
static int g_tournamentHookInstalled = 0;

void tournament_hook_impl(DWORD app) {
    /* Call original App_StartTournamentRace via trampoline */
    __asm__ volatile(
        "movl %0, %%ecx\n"
        "call *%1\n"
        : : "r"(app), "r"((void*)g_tournamentTrampoline)
        : "eax", "ecx", "edx", "memory"
    );

    /* After App_StartTournamentRace: App+0x90C is NULL (game destroyed it).
     * Create a fresh BTT for recording. */
    DWORD bttRec = 0;
    if (!IsBadReadPtr((void*)(app + APP_BTT_RECORDING), 4))
        bttRec = *(DWORD*)(app + APP_BTT_RECORDING);

    if (!bttRec) {
        void *newBTT = game_operator_new(BTT_SIZE);
        if (newBTT) {
            call_btt_ctor(newBTT);
            DWORD vt = *(DWORD*)newBTT;
            if (vt == BTT_VTABLE_ADDR) {
                *(DWORD*)((char*)newBTT + BTT_BEST_TIME) = NO_TIME;
                *(DWORD*)(app + APP_BTT_RECORDING) = (DWORD)newBTT;
                diag_logf("[tournament_hook] Created recording BTT at 0x%X", (DWORD)newBTT);
            } else {
                game_free(newBTT);
                diag_logf("[tournament_hook] BTT ctor failed vtable=0x%X", vt);
            }
        }
    }
}

__attribute__((naked, used)) static void hook_App_StartTournamentRace(void) {
    __asm__ volatile(
        "pushl %%eax\n"
        "pushl %%ecx\n"
        "pushl %%edx\n"
        "pushl %%ecx\n"         /* pass original ECX (app) as argument */
        "call _tournament_hook_impl\n"
        "addl $4, %%esp\n"
        "popl %%edx\n"
        "popl %%ecx\n"
        "popl %%eax\n"
        "ret\n"                /* __thiscall: no stack args */
        : : : "memory"
    );
}

static void install_tournament_hook(void) {
    unsigned char *target = (unsigned char*)APP_START_TOURNAMENT_RACE;

    /* Verify function signature: 6A FF 68 (PUSH -1; PUSH imm32) */
    if (target[0] != 0x6A || target[1] != 0xFF || target[2] != 0x68) {
        diag_logf("[FATAL] App_StartTournamentRace signature mismatch at 0x%X!", APP_START_TOURNAMENT_RACE);
        return;
    }

    memcpy(g_tournamentOrigBytes, target, TOURNAMENT_HOOK_BYTES);

    g_tournamentTrampoline = (unsigned char*)VirtualAlloc(NULL, TOURNAMENT_TRAMPOLINE_SIZE,
        MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!g_tournamentTrampoline) return;

    memcpy(g_tournamentTrampoline, g_tournamentOrigBytes, TOURNAMENT_HOOK_BYTES);
    DWORD jmp_src = (DWORD)(g_tournamentTrampoline + TOURNAMENT_HOOK_BYTES + 5);
    DWORD jmp_dst = APP_START_TOURNAMENT_RACE + TOURNAMENT_HOOK_BYTES;
    g_tournamentTrampoline[TOURNAMENT_HOOK_BYTES + 0] = 0xE9;
    *(DWORD*)(g_tournamentTrampoline + TOURNAMENT_HOOK_BYTES + 1) = jmp_dst - jmp_src;

    DWORD oldProtect;
    if (!VirtualProtect(target, TOURNAMENT_HOOK_BYTES, PAGE_EXECUTE_READWRITE, &oldProtect)) return;

    DWORD hookAddr = (DWORD)&hook_App_StartTournamentRace;
    target[0] = 0xE9;
    *(DWORD*)(target + 1) = hookAddr - (DWORD)target - 5;
    target[5] = 0x90;
    target[6] = 0x90;
    VirtualProtect(target, TOURNAMENT_HOOK_BYTES, oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), target, TOURNAMENT_HOOK_BYTES);
    g_tournamentHookInstalled = 1;
    diag_log("[tournament_hook] App_StartTournamentRace hook installed");
}

/* ================================================================
 * Ghost Triggers subsystem — GT: proximity triggers
 * ================================================================ */

typedef struct {
    float pos_x, pos_y, pos_z;
    float radius;
    float radius_sq;
    char  name[64];
    int   inside_ghost1;
    int   inside_ghost2;
    int   cooldown1;
    int   cooldown2;
} Trigger;

static Trigger g_triggers[MAX_TRIGGERS];
static int g_triggerCount = 0;
static DWORD g_triggerBoard = 0;

static void scan_s1_triggers(DWORD board) {
    g_triggerCount = 0;
    if (!board || IsBadReadPtr((void*)board, 0x4000)) return;

    DWORD level = *(DWORD*)((char*)board + BOARD_MESHWORLD_PTR);
    if (!level || IsBadReadPtr((void*)level, 0x1000)) return;

    DWORD mwData = *(DWORD*)((char*)level + MW_SECTION3_PTR);
    if (!mwData || IsBadReadPtr((void*)mwData, 0x1000)) return;

    int s1Count = *(int*)((char*)mwData + MW_S1_COUNT_OFFSET);
    DWORD s1Array = *(DWORD*)((char*)mwData + MW_S1_ARRAY_OFFSET);

    if (s1Count <= 0 || s1Count > 1000) return;
    if (!s1Array || IsBadReadPtr((void*)s1Array, s1Count * 4)) return;

    for (int i = 0; i < s1Count && g_triggerCount < MAX_TRIGGERS; i++) {
        DWORD entry = *(DWORD*)(s1Array + i * 4);
        if (!entry || IsBadReadPtr((void*)entry, S1_SIZE * 4)) continue;

        char *name = *(char**)(entry + S1_NAME * 4);
        if (!name || IsBadReadPtr(name, 4)) continue;

        if (_strnicmp(name, TRIGGER_PREFIX, 3) == 0) {
            Trigger *t = &g_triggers[g_triggerCount];
            const char *eventName = name + 3;
            strncpy(t->name, eventName, sizeof(t->name) - 1);
            t->name[sizeof(t->name) - 1] = '\0';

            t->pos_x = *(float*)(entry + S1_POS_X * 4);
            t->pos_y = *(float*)(entry + S1_POS_Y * 4);
            t->pos_z = *(float*)(entry + S1_POS_Z * 4);

            t->radius = *(float*)(entry + S1_ROT_Y * 4);
            if (t->radius <= 0.0f || t->radius > 5000.0f) t->radius = DEFAULT_RADIUS;
            t->radius_sq = t->radius * t->radius;

            t->inside_ghost1 = 0;
            t->inside_ghost2 = 0;
            t->cooldown1 = 0;
            t->cooldown2 = 0;

            diag_logf("[triggers] Found trigger #%d: '%s' at (%.1f,%.1f,%.1f) r=%.1f",
                g_triggerCount, t->name, t->pos_x, t->pos_y, t->pos_z, t->radius);
            g_triggerCount++;
        }
    }
    diag_logf("[triggers] Scan complete: %d GT: triggers found", g_triggerCount);
}

static void fire_trigger_event(Trigger *t, int ghostNum) {
    diag_logf("[triggers] TRIGGER FIRED: '%s' on ghost%d", t->name, ghostNum);

    /* Handle built-in trigger events */
    if (_strnicmp(t->name, "COLOR(", 6) == 0) {
        /* GT:COLOR(#hexcolor) — change ghost ball color */
        /* Not fully implemented for Ghost 1 (game-managed) — would need to write
         * to ball+0x2AC/0x2B0/0x2B4. For Ghost 2 we'd write to its color fields. */
        diag_logf("[triggers] COLOR trigger (not yet applied to ball fields)");
    } else if (_stricmp(t->name, "RESET") == 0) {
        if (ghostNum == 2 && g_ghost2.active) {
            g_ghost2.playbackIdx = 0;
            diag_log("[triggers] Ghost 2 playback reset to frame 0");
        }
    } else if (_strnicmp(t->name, "SPEED(", 6) == 0) {
        /* GT:SPEED(float) — adjust playback speed */
        /* Would need speed multiplier tracking — skipped for simplicity */
        diag_logf("[triggers] SPEED trigger (stub)");
    } else if (_stricmp(t->name, "STOP") == 0) {
        if (ghostNum == 2 && g_ghost2.active) {
            g_ghost2.active = FALSE;  /* pause */
            diag_log("[triggers] Ghost 2 playback stopped");
        }
    } else if (_stricmp(t->name, "START") == 0) {
        if (ghostNum == 2 && g_ghost2.btt) {
            g_ghost2.active = TRUE;  /* resume */
            diag_log("[triggers] Ghost 2 playback resumed");
        }
    }
}

static void check_ghost_triggers(DWORD board) {
    if (g_triggerCount == 0) return;

    /* Check Ghost 1 (game's native ghost ball at board+0x361C) */
    DWORD ghost1 = 0;
    if (!IsBadReadPtr((void*)(board + BOARD_GHOST_BALL), 4))
        ghost1 = *(DWORD*)((char*)board + BOARD_GHOST_BALL);

    if (ghost1 && !IsBadReadPtr((void*)ghost1, 0x200)) {
        float bx = *(float*)((char*)ghost1 + BALL_POS_X);
        float by = *(float*)((char*)ghost1 + BALL_POS_Y);
        float bz = *(float*)((char*)ghost1 + BALL_POS_Z);

        for (int i = 0; i < g_triggerCount; i++) {
            Trigger *t = &g_triggers[i];
            if (t->cooldown1 > 0) t->cooldown1--;

            float dx = bx - t->pos_x;
            float dy = by - t->pos_y;
            float dz = bz - t->pos_z;
            float dist_sq = dx*dx + dy*dy + dz*dz;

            int wasInside = t->inside_ghost1;
            t->inside_ghost1 = (dist_sq < t->radius_sq) ? 1 : 0;

            if (!wasInside && t->inside_ghost1 && t->cooldown1 == 0) {
                fire_trigger_event(t, 1);
                t->cooldown1 = TRIGGER_COOLDOWN_FRAMES;
            }
        }
    }

    /* Check Ghost 2 (our secondary ball) */
    if (g_ghost2.active && g_ghost2.ball && !IsBadReadPtr((void*)g_ghost2.ball, 0x200)) {
        float bx = *(float*)(g_ghost2.ball + BALL_POS_X);
        float by = *(float*)(g_ghost2.ball + BALL_POS_Y);
        float bz = *(float*)(g_ghost2.ball + BALL_POS_Z);

        for (int i = 0; i < g_triggerCount; i++) {
            Trigger *t = &g_triggers[i];
            if (t->cooldown2 > 0) t->cooldown2--;

            float dx = bx - t->pos_x;
            float dy = by - t->pos_y;
            float dz = bz - t->pos_z;
            float dist_sq = dx*dx + dy*dy + dz*dz;

            int wasInside = t->inside_ghost2;
            t->inside_ghost2 = (dist_sq < t->radius_sq) ? 1 : 0;

            if (!wasInside && t->inside_ghost2 && t->cooldown2 == 0) {
                fire_trigger_event(t, 2);
                t->cooldown2 = TRIGGER_COOLDOWN_FRAMES;
            }
        }
    }
}

/* ================================================================
 * Ghost Event subsystem — E:GHOST collision events
 * ================================================================ */

static DWORD g_loadedBTT = 0;
static BOOL  g_ghostActive = FALSE;
static char  g_pendingGhostFile[256] = "";
static BOOL  g_ghostBallCreated = FALSE;
static BOOL  g_ghostFromEvent = FALSE;
static BOOL  g_needManualAdvance = FALSE;

/* DCE handler (called from raw byte stub) */
static int g_dceCallCount = 0;

void __cdecl dce_handler(DWORD board, DWORD ball, DWORD collEntry) {
    g_dceCallCount++;
    if (!collEntry) return;

    DWORD *pair = (DWORD*)collEntry;
    if (IsBadReadPtr(pair, 8)) return;
    if (!pair[1]) return;
    if (IsBadReadPtr((void*)pair[1], 0x868)) return;

    const char *eventName = NULL;
    {
        DWORD namePtr = *(DWORD*)((BYTE*)pair[1] + 0x864);
        if (!namePtr || IsBadReadPtr((void*)namePtr, 1)) return;
        eventName = (const char*)namePtr;
    }
    if (!eventName[0]) return;

    if (g_dceCallCount <= 20 || (g_dceCallCount % 100) == 0) {
        diag_logf("[dce] event='%s' (board=0x%X ball=0x%X)", g_dceCallCount, eventName, board, ball);
    }

    if (_strnicmp(eventName, "E:GHOST", 7) == 0) {
        if (g_ghostFromEvent && g_ghostActive && g_loadedBTT) return;

        diag_logf("[dce] E:GHOST match: '%s'", eventName);
        const char *p1 = strchr(eventName, '(');
        if (p1) {
            const char *p2 = strchr(p1, ')');
            if (p2 && (p2 - p1 - 1) > 0) {
                size_t len = p2 - p1 - 1;
                if (len > 250) len = 250;
                memcpy(g_pendingGhostFile, p1 + 1, len);
                g_pendingGhostFile[len] = '\0';
                diag_logf("[dce] Parsed ghost filename='%s'", g_pendingGhostFile);
            }
        }
    }
}

static int load_ghost_file(const char *filename, DWORD **outSnapshots, DWORD *outCount, DWORD *outFinishTime) {
    char path[MAX_PATH];
    DWORD bytesRead;

    HMODULE hSelf = NULL;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                      | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                      (LPCSTR)&load_ghost_file, &hSelf);
    GetModuleFileNameA(hSelf, path, MAX_PATH);
    char *p = strrchr(path, '\\');
    if (p) {
        strcpy(p + 1, "Ghosts\\");
        strncat(path, filename, MAX_PATH - strlen(path) - 1);
        size_t plen = strlen(path);
        if (plen < MAX_PATH - 7 && (plen < 6 || _stricmp(path + plen - 6, ".ghost") != 0)) {
            strcat(path, ".ghost");
        }
    } else {
        snprintf(path, MAX_PATH, "Ghosts\\%s.ghost", filename);
    }

    diag_logf("[ghost_event] Loading: %s", path);

    HANDLE hFile = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL,
                               OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return 0;

    DWORD magic, version, time, frameCount;
    if (!ReadFile(hFile, &magic, 4, &bytesRead, NULL) || bytesRead != 4) {
        CloseHandle(hFile);
        return 0;
    }

    if (magic != GHOST_MAGIC) {
        /* Legacy format */
        DWORD count = magic;
        DWORD t = 0;
        if (!ReadFile(hFile, &t, 4, &bytesRead, NULL)) t = 0;
        frameCount = count;
        time = t;
    } else {
        if (!ReadFile(hFile, &version, 4, &bytesRead, NULL) || bytesRead != 4 ||
            !ReadFile(hFile, &time, 4, &bytesRead, NULL) || bytesRead != 4 ||
            !ReadFile(hFile, &frameCount, 4, &bytesRead, NULL) || bytesRead != 4) {
            CloseHandle(hFile);
            return 0;
        }
    }

    if (frameCount == 0 || frameCount > 50000) {
        CloseHandle(hFile);
        return 0;
    }

    DWORD dataSize = frameCount * SNAP_SIZE;
    DWORD *snapshots = (DWORD*)malloc(dataSize);
    if (!snapshots) { CloseHandle(hFile); return 0; }

    if (!ReadFile(hFile, snapshots, dataSize, &bytesRead, NULL) || bytesRead != dataSize) {
        free(snapshots);
        CloseHandle(hFile);
        return 0;
    }

    CloseHandle(hFile);
    *outSnapshots = snapshots;
    *outCount = frameCount;
    *outFinishTime = time;
    diag_logf("[ghost_event] Loaded: %d frames, time=%d", frameCount, time);
    return 1;
}

static DWORD create_btt_from_ghost(DWORD *snapshots, DWORD count, DWORD finishTime, const char *raceName) {
    DWORD bttAddr = (DWORD)game_operator_new(BTT_SIZE);
    if (!bttAddr) return 0;

    call_btt_ctor((void*)bttAddr);

    DWORD vtable = *(DWORD*)bttAddr;
    if (vtable != BTT_VTABLE_ADDR) {
        game_free((void*)bttAddr);
        return 0;
    }

    *(DWORD*)(bttAddr + BTT_BEST_TIME) = finishTime;
    if (raceName && raceName[0]) {
        char *bttName = (char*)(bttAddr + BTT_RACE_NAME);
        strncpy(bttName, raceName, 127);
        bttName[127] = '\0';
    }

    DWORD *alist = (DWORD*)(bttAddr + 0x04);
    for (DWORD i = 0; i < count; i++) {
        DWORD *snap = (DWORD*)game_operator_new(SNAP_SIZE);
        if (!snap) continue;
        memcpy(snap, (BYTE*)snapshots + i * SNAP_SIZE, SNAP_SIZE);
        call_alist_append(alist, snap);
    }

    *(DWORD*)(bttAddr + BTT_PLAYBACK_IDX) = 0;
    return bttAddr;
}

static DWORD create_ghost_ball(DWORD board) {
    DWORD ballAddr = (DWORD)game_operator_new(BALL_SIZE);
    if (!ballAddr) return 0;

    call_ball_ctor(ballAddr, board);
    DWORD ballVtable = *(DWORD*)ballAddr;
    if (!ballVtable || IsBadReadPtr((void*)ballVtable, 4)) return 0;

    DWORD func1 = *(DWORD*)(ballVtable + 0x04);
    if (func1 && !IsBadReadPtr((void*)func1, 4)) {
        __asm__ volatile(
            "movl %0, %%ecx\n\t"
            "call *%1\n\t"
            : : "r"(ballAddr), "r"(func1)
            : "eax", "ecx", "edx", "memory"
        );
    }

    *(DWORD*)(ballAddr + BALL_PLAYER_ID) = 0xFFFFFFFF;
    *(float*)(ballAddr + BALL_GRAVITY) = 0.5f;
    *(float*)(ballAddr + BALL_GRAVITY_SCALE) = 0.1f;
    *(float*)(ballAddr + BALL_RADIUS) = 26.0f;
    *(float*)(ballAddr + BALL_MASS) = 0.8f;
    *(float*)(ballAddr + BALL_MAXSPEED) = 1000.0f;
    *(BYTE*)(ballAddr + BALL_RESPAWN_FLAG) = 0;
    *(float*)(ballAddr + BALL_ALPHA) = 0.45f;
    *(float*)(ballAddr + BALL_POS_X) = 0.0f;
    *(float*)(ballAddr + BALL_POS_Y) = 0.0f;
    *(float*)(ballAddr + BALL_POS_Z) = 0.0f;

    *(DWORD*)(board + BOARD_GHOST_BALL) = ballAddr;
    diag_logf("[ghost_event] Ghost ball created at 0x%X", ballAddr);
    return ballAddr;
}

static void cleanup_previous_ghost(DWORD app) {
    DWORD board = get_board(app);

    if (g_ghostBallCreated) {
        if (board) {
            DWORD ghostBall = *(DWORD*)(board + BOARD_GHOST_BALL);
            if (ghostBall && !IsBadReadPtr((void*)ghostBall, 0x100)) {
                call_ball_dtor(ghostBall);
                *(DWORD*)(board + BOARD_GHOST_BALL) = 0;
            }
        }
        g_ghostBallCreated = FALSE;
    }

    if (g_loadedBTT && app) {
        if (*(DWORD*)(app + APP_BTT_PLAYBACK) == g_loadedBTT) {
            *(DWORD*)(app + APP_BTT_PLAYBACK) = 0;
        }
    }

    if (g_loadedBTT) {
        if (!IsBadReadPtr((void*)g_loadedBTT, 0x100)) {
            DWORD vt = *(DWORD*)g_loadedBTT;
            if (vt == BTT_VTABLE_ADDR)
                call_btt_dtor((void*)g_loadedBTT);
            else
                game_free((void*)g_loadedBTT);
        }
        g_loadedBTT = 0;
    }

    g_ghostActive = FALSE;
    g_ghostFromEvent = FALSE;
    g_needManualAdvance = FALSE;
}

static void ghost_event_frame(void) {
    DWORD app = get_app();
    if (!app) return;

    /* Check for pending ghost file load */
    if (g_pendingGhostFile[0]) {
        char filename[256];
        strncpy(filename, g_pendingGhostFile, sizeof(filename) - 1);
        filename[sizeof(filename) - 1] = '\0';
        g_pendingGhostFile[0] = '\0';

        DWORD board = get_board(app);
        if (!board) return;

        if (g_ghostActive || g_loadedBTT) {
            cleanup_previous_ghost(app);
        }

        DWORD *snapshots = NULL;
        DWORD count = 0;
        DWORD finishTime = 0;
        if (!load_ghost_file(filename, &snapshots, &count, &finishTime)) return;

        DWORD newBTT = create_btt_from_ghost(snapshots, count, finishTime, filename);
        if (!newBTT) { free(snapshots); return; }

        *(DWORD*)(app + APP_BTT_PLAYBACK) = newBTT;
        g_loadedBTT = newBTT;

        DWORD ghostBall = *(DWORD*)(board + BOARD_GHOST_BALL);
        if (!ghostBall || IsBadReadPtr((void*)ghostBall, 0x100)) {
            ghostBall = create_ghost_ball(board);
            if (ghostBall) g_ghostBallCreated = TRUE;
        } else {
            g_ghostBallCreated = FALSE;
        }

        *(DWORD*)(newBTT + BTT_PLAYBACK_IDX) = 0;
        g_ghostActive = TRUE;
        g_ghostFromEvent = TRUE;

        BYTE partyMode = 0;
        if (!IsBadReadPtr((void*)(app + APP_234_PARTY_MODE), 1))
            partyMode = *(BYTE*)(app + APP_234_PARTY_MODE);
        g_needManualAdvance = (partyMode != 0);

        free(snapshots);
    }

    /* Cleanup if board/ball lost, advance playback if needed */
    if (g_ghostActive && g_loadedBTT) {
        DWORD board = get_board(app);
        if (!board) {
            cleanup_previous_ghost(app);
        } else {
            DWORD ghostBall = *(DWORD*)(board + BOARD_GHOST_BALL);
            if (!ghostBall || IsBadReadPtr((void*)ghostBall, 0x100)) {
                cleanup_previous_ghost(app);
            } else {
                if (g_needManualAdvance) {
                    DWORD playIdx = *(DWORD*)(g_loadedBTT + BTT_PLAYBACK_IDX);
                    DWORD count = *(DWORD*)(g_loadedBTT + BTT_AL_COUNT);
                    if (count > 0 && playIdx < count - 1) {
                        *(DWORD*)(g_loadedBTT + BTT_PLAYBACK_IDX) = playIdx + 1;
                    }
                }
                if (g_ghostFromEvent) {
                    DWORD playIdx = *(DWORD*)(g_loadedBTT + BTT_PLAYBACK_IDX);
                    DWORD count = *(DWORD*)(g_loadedBTT + BTT_AL_COUNT);
                    if (count > 0 && playIdx >= count - 1) {
                        cleanup_previous_ghost(app);
                    }
                }
            }
        }
    }
}

/* DCE hook stub */
/* DCE hook — 0x40C5D0 has SEH prologue (6A FF 64 A1 00... 21 bytes).
 * Hook at 0x40C5E5 where the function body starts: 83 EC 30 53 55 (5 bytes).
 * Trampoline replicates those 5 bytes and jumps to 0x40C5EA. */
#define DCE_HOOK_ADDR             0x0040C5E5
#define DCE_HOOK_BYTES            5
#define DCE_HOOK_RETURN           0x0040C5EA

static BYTE *g_dce_stub = NULL;
static BYTE g_dce_original[DCE_HOOK_BYTES];
static void *g_dce_trampoline = NULL;

static void build_dce_trampoline(void) {
    BYTE *code = (BYTE*)alloc_executable(16);
    /* Replicate 5 trampoline bytes: sub esp,0x30; push ebx; push ebp */
    code[0] = 0x83; code[1] = 0xEC; code[2] = 0x30;  /* sub esp, 0x30 */
    code[3] = 0x53;                                     /* push ebx */
    code[4] = 0x55;                                     /* push ebp */
    code[5] = 0xE9;
    *(DWORD*)(code + 6) = DCE_HOOK_RETURN - (DWORD)(code + 10);
    g_dce_trampoline = code;
}

static void build_dce_stub(void) {
    BYTE *code = (BYTE*)alloc_executable(96);
    int i = 0;
    /* Grab params from stack BEFORE pushad: [esp+8]=ball, [esp+12]=collEntry.
     * ECX is board (set before hook point by the caller). */
    /* At 0x40C5E5, after SEH prologue (PUSH -1 + MOV EAX,FS:[0] + PUSH EAX + MOV FS:[0],ESP = 4 pushes),
     * stack has: [ESP]=oldSEH, [ESP+4]=handler, [ESP+8]=-1, [ESP+12]=retAddr, [ESP+16]=ball, [ESP+20]=collEntry */
    code[i++] = 0x8B; code[i++] = 0x5C; code[i++] = 0x24; code[i++] = 0x10; /* mov ebx, [esp+16] (ball) */
    code[i++] = 0x8B; code[i++] = 0x74; code[i++] = 0x24; code[i++] = 0x14; /* mov esi, [esp+20] (collEntry) */
    code[i++] = 0x60;                 /* pushad */
    code[i++] = 0x9C;                 /* pushfd */
    code[i++] = 0x51;                 /* push ecx (board) */
    code[i++] = 0x53;                 /* push ebx (ball) */
    code[i++] = 0x56;                 /* push esi (collEntry) */
    code[i++] = 0xE8;
    *(DWORD*)(code + i) = (DWORD)&dce_handler - (DWORD)(code + i + 4);
    i += 4;
    code[i++] = 0x83; code[i++] = 0xC4; code[i++] = 0x0C; /* add esp, 12 */
    code[i++] = 0x9D;               /* popfd */
    code[i++] = 0x61;               /* popad */
    code[i++] = 0xB8;
    *(DWORD*)(code + i) = (DWORD)g_dce_trampoline;
    i += 4;
    code[i++] = 0xFF; code[i++] = 0xE0; /* jmp eax */
    g_dce_stub = code;
}

static void install_dce_hook(void) {
    memcpy(g_dce_original, (void*)DCE_HOOK_ADDR, DCE_HOOK_BYTES);
    build_dce_trampoline();
    build_dce_stub();

    BYTE patch[DCE_HOOK_BYTES];
    patch[0] = 0xE9;
    *(DWORD*)(patch + 1) = (DWORD)g_dce_stub - DCE_HOOK_ADDR - 5;
    patch_bytes((void*)DCE_HOOK_ADDR, patch, DCE_HOOK_BYTES);
    diag_log("[ghost_event] DCE hook installed at 0x40C5E5 (post-SEH prologue)");
}

static void restore_dce_hook(void) {
    if (!g_dce_stub) return;
    patch_bytes((void*)DCE_HOOK_ADDR, g_dce_original, DCE_HOOK_BYTES);
    if (g_dce_trampoline) {
        VirtualFree(g_dce_trampoline, 0, MEM_RELEASE);
        g_dce_trampoline = NULL;
    }
    VirtualFree(g_dce_stub, 0, MEM_RELEASE);
    g_dce_stub = NULL;
    diag_log("[ghost_event] DCE hook restored");
}


/* ================================================================
 * Warp subsystem — state machine, timer caves, color lerp, flash
 * ================================================================ */

typedef enum {
    PHASE_IDLE = 0,
    PHASE_RUMBLE,
    PHASE_FLASH,
    PHASE_HOLD,
    PHASE_FADE,
    PHASE_LOAD,
    PHASE_REVEAL
} WarpPhase;

static volatile WarpPhase g_phase = PHASE_IDLE;
static volatile DWORD g_phaseStartTime = 0;
static volatile DWORD g_warpStartTime = 0;
static volatile int g_warpLevelIndex = -1;
static volatile int g_musicFadeStarted = 0;
static volatile DWORD g_cooldownUntil = 0;

static volatile DWORD g_gameClock = 0;
static volatile DWORD g_lastRealTime = 0;

static float g_musicOrigVolumes[MAX_MUSIC_CHANNELS];
static int g_musicChannelCount = 0;

static volatile float g_whiteAlpha = 0.0f;
static volatile int g_warpBall = 0;
static volatile int g_rumbleInit = 0;

static float g_origBallR = 1.0f, g_origBallG = 1.0f, g_origBallB = 1.0f;
static int g_colorSaved = 0;

static volatile BYTE g_freezeTimer = 0;

/* Ghost mode code cave — allows E:GHOST ghosts in all modes */
/* but restricts normal ghosts to Time Trial only */
static volatile BOOL g_ghostModePatched = 0;
static unsigned char *g_ghostModeCave = NULL;
static unsigned char g_ghostModeOrigBytes[17];

static unsigned char g_decOrigBytes[TIMER_DEC_PATCH_SIZE];
static unsigned char g_incOrigBytes[TIMER_INC_PATCH_SIZE];
static int g_decPatched = 0;
static int g_incPatched = 0;
static unsigned char *g_decCave = NULL;
static unsigned char *g_incCave = NULL;

static BYTE g_pauseOrigBytes[3] = {0, 0, 0};
static int g_pauseBlocked = 0;

/* TT recording NOP saved bytes */
static unsigned char g_ttRecOrigBytes[TT_RECORDING_NOP_SIZE];
static int g_ttRecPatched = 0;

/* Level name -> race index mapping */
typedef struct {
    const char* meshName;
    int raceIndex;
} LevelMapping;

static const LevelMapping levelMap[] = {
    {"level1", 1}, {"warmup", 1}, {"warm-up", 1},
    {"level2", 2}, {"beginner", 2}, {"cascade", 2},
    {"level3", 3}, {"intermediate", 3},
    {"level4", 4}, {"dizzy", 4},
    {"level5", 5}, {"tower", 5},
    {"level6", 6}, {"up", 6},
    {"level7", 7}, {"neon", 7},
    {"level8", 8}, {"expert", 8},
    {"level9", 9}, {"odd", 9},
    {"level10", 10}, {"toob", 10},
    {"level11", 11}, {"wobbly", 11},
    {"level12", 12}, {"glass", 12},
    {"level13", 13}, {"sky", 13},
    {"level14", 14}, {"master", 14},
    {"level15", 15}, {"impossible", 15},
    {NULL, 0}
};

static int findRaceIndex(const char *levelName) {
    if (levelName[0] >= '1' && levelName[0] <= '9') {
        int num = atoi(levelName);
        if (num >= 1 && num <= 15) return num;
    }
    {
        int j;
        for (j = 0; levelMap[j].meshName; j++) {
            if (_stricmp(levelName, levelMap[j].meshName) == 0)
                return levelMap[j].raceIndex;
        }
    }
    return -1;
}

static int is_game_paused(DWORD board) {
    if (!board) return 0;
    if (IsBadReadPtr((void*)(board + BOARD_PAUSED_FLAG), 1)) return 0;
    return *(BYTE*)((char*)board + BOARD_PAUSED_FLAG) != 0;
}

static DWORD getGameTime(void) {
    return g_gameClock;
}

static void updateGameClock(DWORD board) {
    DWORD now = GetTickCount();
    if (g_lastRealTime == 0) {
        g_lastRealTime = now;
        return;
    }
    if (!is_game_paused(board)) {
        g_gameClock += (now - g_lastRealTime);
    }
    g_lastRealTime = now;
}

static void block_pause(void) {
    if (g_pauseBlocked) return;
    DWORD oldProt;
    DWORD addrs[3] = {PAUSE_PATCH_PATH1, PAUSE_PATCH_PATH2, PAUSE_PATCH_PATH3};
    int i;
    for (i = 0; i < 3; i++) {
        DWORD addr = EXE_BASE + addrs[i];
        if (VirtualProtect((void*)addr, 1, PAGE_EXECUTE_READWRITE, &oldProt)) {
            g_pauseOrigBytes[i] = *((BYTE*)addr);
            *((BYTE*)addr) = 0xEB;
            VirtualProtect((void*)addr, 1, oldProt, &oldProt);
        }
    }
    g_pauseBlocked = 1;
    diag_log("[warp] Pause blocked");
}

static void unblock_pause(void) {
    if (!g_pauseBlocked) return;
    DWORD oldProt;
    DWORD addrs[3] = {PAUSE_PATCH_PATH1, PAUSE_PATCH_PATH2, PAUSE_PATCH_PATH3};
    int i;
    for (i = 0; i < 3; i++) {
        DWORD addr = EXE_BASE + addrs[i];
        if (VirtualProtect((void*)addr, 1, PAGE_EXECUTE_READWRITE, &oldProt)) {
            *((BYTE*)addr) = g_pauseOrigBytes[i];
            VirtualProtect((void*)addr, 1, oldProt, &oldProt);
        }
    }
    g_pauseBlocked = 0;
    diag_log("[warp] Pause unblocked");
}

/* Music fade functions */
static void startMusicFade(void) {
    DWORD app = get_app();
    if (!app) return;
    DWORD musicDev = *(DWORD*)((char*)app + APP_MUSIC_DEVICE_PTR);
    if (!musicDev) return;

    DWORD chanListData = *(DWORD*)(musicDev + MUSIC_DEV_CHANNEL_LIST);
    int chanCount = *(int*)(musicDev + 0x10);

    if (chanCount > 0 && chanListData) {
        int count = (chanCount < MAX_MUSIC_CHANNELS) ? chanCount : MAX_MUSIC_CHANNELS;
        g_musicChannelCount = count;
        for (int i = 0; i < count; i++) {
            DWORD chan = *(DWORD*)(chanListData + i * 4);
            if (chan) {
                g_musicOrigVolumes[i] = *(float*)(chan + MUSIC_CHAN_VOLUME);
            } else {
                g_musicOrigVolumes[i] = 0.0f;
            }
        }
        for (int i = 0; i < count; i++) {
            DWORD chan = *(DWORD*)(chanListData + i * 4);
            if (chan) {
                *(BYTE*)((char*)chan + MUSIC_CHAN_FADE_OUT) = 0;
                *(BYTE*)((char*)chan + MUSIC_CHAN_FADE_IN) = 0;
                *(float*)((char*)chan + MUSIC_CHAN_FADE_RATE) = 0.0f;
            }
        }
    }
    g_musicFadeStarted = 1;
}

static void updateMusicFade(void) {
    if (!g_musicFadeStarted) return;

    DWORD elapsed = getGameTime() - g_warpStartTime;
    float t = (float)elapsed / (float)MUSIC_FADE_MS;
    if (t > 1.0f) t = 1.0f;

    DWORD app = get_app();
    if (!app) return;
    DWORD musicDev = *(DWORD*)((char*)app + APP_MUSIC_DEVICE_PTR);
    if (!musicDev) return;

    DWORD chanListData = *(DWORD*)(musicDev + MUSIC_DEV_CHANNEL_LIST);
    int chanCount = *(int*)(musicDev + 0x10);

    if (chanCount > 0 && chanListData) {
        int count = (chanCount < MAX_MUSIC_CHANNELS) ? chanCount : MAX_MUSIC_CHANNELS;
        for (int i = 0; i < count; i++) {
            DWORD chan = *(DWORD*)(chanListData + i * 4);
            if (chan) {
                float origVol = g_musicOrigVolumes[i];
                float chanVol = origVol * (1.0f - t);
                *(float*)(chan + MUSIC_CHAN_VOLUME) = chanVol;
                if (real_BASS_ChannelSetAttributes) {
                    DWORD bassChan = *(DWORD*)(chan + MUSIC_CHAN_BASS_CHANNEL);
                    if (bassChan) {
                        real_BASS_ChannelSetAttributes(bassChan, -1.0f, (int)(chanVol * 100.0f), -1);
                    }
                }
            }
        }
    }
}

static void restoreMusicFade(void) {
    if (!g_musicFadeStarted) return;

    DWORD app = get_app();
    if (!app) return;
    DWORD musicDev = *(DWORD*)((char*)app + APP_MUSIC_DEVICE_PTR);
    if (!musicDev) return;

    DWORD chanListData = *(DWORD*)(musicDev + MUSIC_DEV_CHANNEL_LIST);
    int chanCount = *(int*)(musicDev + 0x10);

    if (chanCount > 0 && chanListData) {
        int count = (chanCount < MAX_MUSIC_CHANNELS) ? chanCount : MAX_MUSIC_CHANNELS;
        for (int i = 0; i < count; i++) {
            DWORD chan = *(DWORD*)(chanListData + i * 4);
            if (chan) {
                *(float*)(chan + MUSIC_CHAN_VOLUME) = g_musicOrigVolumes[i];
                *(BYTE*)((char*)chan + MUSIC_CHAN_FADE_OUT) = 0;
                *(BYTE*)((char*)chan + MUSIC_CHAN_FADE_IN) = 1;
                if (real_BASS_ChannelSetAttributes) {
                    DWORD bassChan = *(DWORD*)(chan + MUSIC_CHAN_BASS_CHANNEL);
                    if (bassChan) {
                        real_BASS_ChannelSetAttributes(bassChan, -1.0f,
                            (int)(g_musicOrigVolumes[i] * 100.0f), -1);
                    }
                }
            }
        }
    }
    g_musicFadeStarted = 0;
    diag_log("[warp] Music channels restored");
}

/* Warp node scanning */
static int isWarpNode(const char *nodeName) {
    if (!nodeName) return 0;
    return _strnicmp(nodeName, "WARP(", 5) == 0;
}

static int parseWarpLevel(const char *nodeName, char *outLevelName, int outSize) {
    const char *start = strchr(nodeName, '(');
    const char *end;
    int len;
    if (!start) return 0;
    start++;
    end = strchr(start, ')');
    if (!end) return 0;
    len = (int)(end - start);
    if (len <= 0 || len >= outSize) return 0;
    strncpy(outLevelName, start, len);
    outLevelName[len] = 0;
    return 1;
}

static void scanWarpNodes(void) {
    DWORD app, board, meshWorld, section3;
    DWORD nodeCount, nodeDataArray;
    DWORD ballDataArray, ball;
    float ballX, ballY, ballZ;
    int i;

    if (g_phase != PHASE_IDLE) return;
    if (getGameTime() < g_cooldownUntil) return;

    app = get_app();
    if (!app) return;

    if (IsBadReadPtr((void*)(app + APP_PROFILE_PTR), 4)) return;
    DWORD profile = *(DWORD*)((char*)app + APP_PROFILE_PTR);
    if (!profile) return;

    if (IsBadReadPtr((void*)(profile + PROFILE_BOARD_PTR), 4)) return;
    board = *(DWORD*)((char*)profile + PROFILE_BOARD_PTR);
    if (!board) return;

    if (IsBadReadPtr((void*)(board + BOARD_GOAL_REACHED), 1)) return;
    if (*(BYTE*)((char*)board + BOARD_GOAL_REACHED) != 0) return;

    meshWorld = *(DWORD*)((char*)board + BOARD_MESHWORLD_PTR);
    if (!is_valid_ptr(meshWorld)) return;

    section3 = *(DWORD*)((char*)meshWorld + MW_SECTION3_PTR);
    if (!is_valid_ptr(section3)) return;

    nodeCount = *(DWORD*)((char*)section3 + MW_NODELIST_COUNT);
    nodeDataArray = *(DWORD*)((char*)section3 + MW_NODELIST_DATA);
    if (nodeCount <= 0 || !is_valid_ptr(nodeDataArray)) return;
    if (nodeCount > 256) nodeCount = 256;

    ballDataArray = *(DWORD*)((char*)board + BOARD_BALL_LIST_DATA);
    if (!is_valid_ptr(ballDataArray)) return;

    /* Check ball count — if more than 1 ball (multiplayer), skip warp proximity to avoid
     * triggering for the wrong player. */
    DWORD ballCount = 0;
    if (!IsBadReadPtr((void*)((char*)board + BOARD_BALL_LIST + 4), 4))
        ballCount = *(DWORD*)((char*)board + BOARD_BALL_LIST + 4);
    if (ballCount > 1) return;

    ball = *(DWORD*)((char*)ballDataArray);
    if (!is_valid_ball(ball)) return;

    if (IsBadReadPtr((void*)(ball + BALL_DEATH_PENDING), 1)) return;
    if (*(BYTE*)((char*)ball + BALL_DEATH_PENDING) != 0) return;

    ballX = *(float*)(ball + BALL_POS_X);
    ballY = *(float*)(ball + BALL_POS_Y);
    ballZ = *(float*)(ball + BALL_POS_Z);

    for (i = 0; i < (int)nodeCount; i++) {
        DWORD entry = *(DWORD*)((char*)nodeDataArray + i * 4);
        const char *name;

        if (!is_valid_ptr(entry)) continue;
        if (IsBadReadPtr((void*)entry, 16)) continue;

        name = *(const char**)((char*)entry);
        if (!name) continue;
        if (IsBadReadPtr(name, 1)) continue;

        if (isWarpNode(name)) {
            float nodeX = *(float*)((char*)entry + NODE_X_OFFSET);
            float nodeY = *(float*)((char*)entry + NODE_Y_OFFSET);
            float nodeZ = *(float*)((char*)entry + NODE_Z_OFFSET);

            float dx = ballX - nodeX;
            float dy = ballY - nodeY;
            float dz = ballZ - nodeZ;
            float distSq = dx * dx + dy * dy + dz * dz;

            if (distSq < WARP_TRIGGER_DIST_SQ) {
                char levelName[128];
                if (parseWarpLevel(name, levelName, sizeof(levelName))) {
                    int raceIndex = findRaceIndex(levelName);
                    if (raceIndex > 0) {
                        g_warpLevelIndex = raceIndex - 1;
                        g_phase = PHASE_RUMBLE;
                        g_rumbleInit = 0;
                        g_colorSaved = 0;
                        g_warpBall = ball;
                        g_phaseStartTime = getGameTime();
                        g_warpStartTime = getGameTime();
                        g_whiteAlpha = 0.0f;
                        g_musicFadeStarted = 0;
                        diag_logf("[warp] Triggered! level=%d (0-based=%d)", raceIndex, g_warpLevelIndex);
                    }
                }
                return;
            }
        }
    }
}

/* Timer freeze code caves */
static void install_timer_caves(void) {
    DWORD base = EXE_BASE;
    DWORD oldProt;

    /* Cave 1: DEC path (tournament) */
    if (!g_decPatched) {
        DWORD patchAddr = base + TIMER_DEC_PATCH_RVA;
        DWORD returnAddr = base + TIMER_DEC_RETURN_RVA;
        DWORD skipAddr = base + TIMER_DEC_SKIP_RVA;
        DWORD freezeAddr = (DWORD)&g_freezeTimer;

        g_decCave = (unsigned char *)VirtualAlloc(NULL, 64, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
        if (!g_decCave) return;

        unsigned char *p = g_decCave;

        /* JNZ skip_target (near, 6 bytes) */
        p[0] = 0x0F; p[1] = 0x85;
        *(DWORD*)(p + 2) = (DWORD)(p + 6 + 7 + 6 + 3 + 5) - (DWORD)(p + 2 + 4);
        p += 6;

        /* CMP byte [g_freezeTimer], 0 (7 bytes) */
        p[0] = 0x80; p[1] = 0x3D;
        *(DWORD*)(p + 2) = freezeAddr;
        p[6] = 0x00;
        p += 7;

        /* JNE skip_target (near, 6 bytes) */
        p[0] = 0x0F; p[1] = 0x85;
        *(DWORD*)(p + 2) = (DWORD)(p + 6 + 3 + 5) - (DWORD)(p + 2 + 4);
        p += 6;

        /* DEC [EDI+0x1C] (3 bytes, original) */
        p[0] = 0xFF; p[1] = 0x4F; p[2] = 0x1C;
        p += 3;

        /* JMP return */
        write_jmp(p, returnAddr);
        p += 5;

        /* skip_target: JMP skip */
        write_jmp(p, skipAddr);

        if (VirtualProtect((void*)patchAddr, TIMER_DEC_PATCH_SIZE, PAGE_EXECUTE_READWRITE, &oldProt)) {
            memcpy(g_decOrigBytes, (void*)patchAddr, TIMER_DEC_PATCH_SIZE);
            write_jmp((unsigned char*)patchAddr, (DWORD)g_decCave);
            memset((unsigned char*)patchAddr + 5, 0x90, 4);
            VirtualProtect((void*)patchAddr, TIMER_DEC_PATCH_SIZE, oldProt, &oldProt);
            g_decPatched = 1;
            diag_log("[warp] DEC timer cave installed at 0x41B3E5");
        }
    }

    /* Cave 2: INC path (practice/TT/party) */
    if (!g_incPatched) {
        DWORD patchAddr = base + TIMER_INC_PATCH_RVA;
        DWORD returnAddr = base + TIMER_INC_RETURN_RVA;
        DWORD freezeAddr = (DWORD)&g_freezeTimer;

        g_incCave = (unsigned char *)VirtualAlloc(NULL, 64, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
        if (!g_incCave) return;

        unsigned char *p = g_incCave;

        /* JNZ skip_inc (short, 2 bytes) */
        p[0] = 0x75;
        p[1] = 0x0C;
        p += 2;

        /* CMP byte [g_freezeTimer], 0 (7 bytes) */
        p[0] = 0x80; p[1] = 0x3D;
        *(DWORD*)(p + 2) = freezeAddr;
        p[6] = 0x00;
        p += 7;

        /* JNE skip_inc (short, 2 bytes) */
        p[0] = 0x75;
        p[1] = 0x03;
        p += 2;

        /* INC [EDX+0x1C] (3 bytes, original) */
        p[0] = 0xFF; p[1] = 0x42; p[2] = 0x1C;
        p += 3;

        /* skip_inc: JMP return */
        write_jmp(p, returnAddr);

        if (VirtualProtect((void*)patchAddr, TIMER_INC_PATCH_SIZE, PAGE_EXECUTE_READWRITE, &oldProt)) {
            memcpy(g_incOrigBytes, (void*)patchAddr, TIMER_INC_PATCH_SIZE);
            write_jmp((unsigned char*)patchAddr, (DWORD)g_incCave);
            VirtualProtect((void*)patchAddr, TIMER_INC_PATCH_SIZE, oldProt, &oldProt);
            g_incPatched = 1;
            diag_log("[warp] INC timer cave installed at 0x41B50C");
        }
    }
}

static void restore_timer_caves(void) {
    DWORD base = EXE_BASE;
    DWORD oldProt;

    if (g_decPatched) {
        DWORD addr = base + TIMER_DEC_PATCH_RVA;
        if (VirtualProtect((void*)addr, TIMER_DEC_PATCH_SIZE, PAGE_EXECUTE_READWRITE, &oldProt)) {
            memcpy((void*)addr, g_decOrigBytes, TIMER_DEC_PATCH_SIZE);
            VirtualProtect((void*)addr, TIMER_DEC_PATCH_SIZE, oldProt, &oldProt);
        }
        g_decPatched = 0;
    }

    if (g_incPatched) {
        DWORD addr = base + TIMER_INC_PATCH_RVA;
        if (VirtualProtect((void*)addr, TIMER_INC_PATCH_SIZE, PAGE_EXECUTE_READWRITE, &oldProt)) {
            memcpy((void*)addr, g_incOrigBytes, TIMER_INC_PATCH_SIZE);
            VirtualProtect((void*)addr, TIMER_INC_PATCH_SIZE, oldProt, &oldProt);
        }
        g_incPatched = 0;
    }
}

/* Ghost mode code cave — allows E:GHOST ghosts in all modes,
 * but keeps normal ghosts restricted to Time Trial only.
 *
 * Patches 0x0040B7F0 (17 bytes) which normally has:
 *   MOV DL, [ECX+0x11]   ; profile->practice
 *   TEST DL, DL
 *   JZ skip               ; skip if NOT TT
 *   MOV CL, [EAX+0x234]  ; app->party_mode
 *   TEST CL, CL
 *   JNZ skip              ; skip if party
 *
 * Cave replaces this with g_ghostFromEvent-aware logic:
 *   if (g_ghostFromEvent) goto allow;
 *   else { original checks; }
 *   allow: -> continue at 0x40B802 (ghost update)
 *   skip:  -> jump to 0x40B834 (skip ghost)
 */
#define GHOST_MODE_PATCH_ADDR  0x0040B7F0
#define GHOST_MODE_PATCH_SIZE  17
#define GHOST_MODE_CONTINUE    0x0040B802
#define GHOST_MODE_SKIP        0x0040B834

static void install_ghost_mode_cave(void) {
    if (g_ghostModePatched) return;
    DWORD patchAddr = GHOST_MODE_PATCH_ADDR;
    DWORD continueAddr = GHOST_MODE_CONTINUE;
    DWORD skipAddr = GHOST_MODE_SKIP;
    DWORD ghostFromEventAddr = (DWORD)&g_ghostFromEvent;

    g_ghostModeCave = (unsigned char *)VirtualAlloc(NULL, 64, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (!g_ghostModeCave) return;

    unsigned char *p = g_ghostModeCave;

    /* CMP byte [g_ghostFromEvent], 0  (7 bytes) */
    p[0] = 0x80; p[1] = 0x3D;
    *(DWORD*)(p + 2) = ghostFromEventAddr;
    p[6] = 0x00;
    p += 7;

    /* JNE allow (JNZ rel8, 2 bytes) */
    /* Calculate offset: (allow is after the 5-byte JMP) */
    /* allow label is at: p + 2 + 3 + 2 + 2 + 6 + 2 + 2 + 5 (end of JMP allow) */
    /* But easier: we'll write this as a placeholder and fix it up */
    /* Actually, let me just compute: after the JNE, we have 3+2+2+6+2+2+5+5 = 27 bytes of code */
    /* JNE offset = 27 (skip past original checks + both JMPs) */
    /* Wait, let me recalculate: */
    /* JNE is at bytes 7-8 of cave. The 'allow' JMP is 9 bytes after the original checks. */
    /* Original checks: 3+2+2+6+2+2 = 17 bytes */
    /* Then JMP allow (5 bytes) = 22 bytes from JNE end */
    /* Then JMP skip (5 bytes) = 27 bytes from JNE end */
    /* So JNE rel8 = 27 (0x1B) */
    p[0] = 0x75; p[1] = 0x1B;  /* JNE +27 (skip to 'allow' JMP) */
    p += 2;

    /* Original: MOV DL, [ECX+0x11] (3 bytes) */
    p[0] = 0x8A; p[1] = 0x51; p[2] = 0x11;
    p += 3;

    /* Original: TEST DL, DL (2 bytes) */
    p[0] = 0x84; p[1] = 0xD2;
    p += 2;

    /* Original: JZ skip (2 bytes) */
    /* skip is at: p + 2 + 6 + 2 + 2 + 5 + 5 */
    /* = 22 bytes from here */
    p[0] = 0x74; p[1] = 22;  /* JZ +22 */
    p += 2;

    /* Original: MOV CL, [EAX+0x234] (6 bytes) */
    p[0] = 0x8A; p[1] = 0x88;
    p[2] = 0x34; p[3] = 0x02; p[4] = 0x00; p[5] = 0x00;
    p += 6;

    /* Original: TEST CL, CL (2 bytes) */
    p[0] = 0x84; p[1] = 0xC9;
    p += 2;

    /* Original: JNZ skip (2 bytes) */
    /* skip is at: p + 2 + 5 + 5 = 12 bytes from here */
    p[0] = 0x75; p[1] = 12;  /* JNZ +12 */
    p += 2;

    /* allow: JMP continueAddr (5 bytes) */
    write_jmp(p, continueAddr);
    p += 5;

    /* skip: JMP skipAddr (5 bytes) */
    write_jmp(p, skipAddr);

    /* Save original bytes and patch */
    DWORD oldProt;
    if (VirtualProtect((void*)patchAddr, GHOST_MODE_PATCH_SIZE, PAGE_EXECUTE_READWRITE, &oldProt)) {
        memcpy(g_ghostModeOrigBytes, (void*)patchAddr, GHOST_MODE_PATCH_SIZE);
        write_jmp((unsigned char*)patchAddr, (DWORD)g_ghostModeCave);
        memset((unsigned char*)patchAddr + 5, 0x90, GHOST_MODE_PATCH_SIZE - 5);
        VirtualProtect((void*)patchAddr, GHOST_MODE_PATCH_SIZE, oldProt, &oldProt);
        g_ghostModePatched = 1;
        diag_log("[ghost] Ghost mode code cave installed at 0x40B7F0");
    }
}

static void restore_ghost_mode_cave(void) {
    if (!g_ghostModePatched) return;
    DWORD patchAddr = GHOST_MODE_PATCH_ADDR;
    DWORD oldProt;
    if (VirtualProtect((void*)patchAddr, GHOST_MODE_PATCH_SIZE, PAGE_EXECUTE_READWRITE, &oldProt)) {
        memcpy((void*)patchAddr, g_ghostModeOrigBytes, GHOST_MODE_PATCH_SIZE);
        VirtualProtect((void*)patchAddr, GHOST_MODE_PATCH_SIZE, oldProt, &oldProt);
    }
    if (g_ghostModeCave) {
        VirtualFree(g_ghostModeCave, 0, MEM_RELEASE);
        g_ghostModeCave = NULL;
    }
    g_ghostModePatched = 0;
    diag_log("[ghost] Ghost mode code cave restored");
}

/* TT recording NOP patch — enables recording in Tournament mode */
static void install_tt_recording_nop(void) {
    if (g_ttRecPatched) return;

    DWORD addr = TT_RECORDING_NOP_ADDR;
    DWORD oldProt;
    if (VirtualProtect((void*)addr, TT_RECORDING_NOP_SIZE, PAGE_EXECUTE_READWRITE, &oldProt)) {
        memcpy(g_ttRecOrigBytes, (void*)addr, TT_RECORDING_NOP_SIZE);
        /* NOP 7 bytes: 8A 4A 11 84 C9 74 71 → 90 90 90 90 90 90 90 */
        memset((void*)addr, 0x90, TT_RECORDING_NOP_SIZE);
        VirtualProtect((void*)addr, TT_RECORDING_NOP_SIZE, oldProt, &oldProt);
        g_ttRecPatched = 1;
        diag_log("[warp] TT recording NOP patch installed at 0x41B690 (7 bytes)");
    }
}

static void restore_tt_recording_nop(void) {
    if (!g_ttRecPatched) return;
    DWORD addr = TT_RECORDING_NOP_ADDR;
    DWORD oldProt;
    if (VirtualProtect((void*)addr, TT_RECORDING_NOP_SIZE, PAGE_EXECUTE_READWRITE, &oldProt)) {
        memcpy((void*)addr, g_ttRecOrigBytes, TT_RECORDING_NOP_SIZE);
        VirtualProtect((void*)addr, TT_RECORDING_NOP_SIZE, oldProt, &oldProt);
    }
    g_ttRecPatched = 0;
}

/* Warp state machine */
static void updateWarpStateMachine(void) {
    DWORD app;
    DWORD board = 0;
    DWORD ball;
    DWORD now;
    DWORD elapsed;

    if (g_phase == PHASE_IDLE) return;

    now = getGameTime();
    app = get_app();
    if (!app) {
        g_freezeTimer = 0;
        unblock_pause();
        g_phase = PHASE_IDLE;
        return;
    }

    ball = g_warpBall;
    if (!is_valid_ball(ball)) {
        ball = 0;
    }

    {
        DWORD profile = *(DWORD*)((char*)app + APP_PROFILE_PTR);
        if (profile && !IsBadReadPtr((void*)profile, 0x100)) {
            board = *(DWORD*)((char*)profile + PROFILE_BOARD_PTR);
        }
    }

    /* Write white flash alpha during fade phases */
    if (board) {
        if (g_phase == PHASE_FLASH || g_phase == PHASE_FADE || g_phase == PHASE_REVEAL) {
            if (!IsBadWritePtr((void*)(board + SCENE_FADE_ALPHA), 4))
                *(float*)(board + SCENE_FADE_ALPHA) = g_whiteAlpha;
        }
    }

    /* Abort warp if timer expired during RUMBLE or FLASH */
    if ((g_phase == PHASE_RUMBLE || g_phase == PHASE_FLASH) && ball) {
        if (!IsBadReadPtr((void*)(app + APP_5D6_GOAL_FLAG), 1)) {
            BYTE finished = *(BYTE*)((char*)app + APP_5D6_GOAL_FLAG);
            if (finished) {
                diag_log("[warp] ABORT: timer expired during warp");
                *(DWORD*)(ball + BALL_IMPACT_FREEZE) = 0;
                *(BYTE*)(ball + BALL_RENDER_JITTER) = 0;
                *(BYTE*)(ball + BALL_IN_TAR) = 0;
                restoreMusicFade();
                g_freezeTimer = 0;
                unblock_pause();
                if (board) *(float*)(board + SCENE_FADE_ALPHA) = 0.0f;
                g_whiteAlpha = 0.0f;
                g_phase = PHASE_IDLE;
                g_cooldownUntil = getGameTime() + WARP_COOLDOWN_MS;
                g_warpBall = 0;
                return;
            }
        }
    }

    switch (g_phase) {
    case PHASE_RUMBLE: {
        elapsed = now - g_phaseStartTime;

        if (!g_rumbleInit && ball) {
            g_rumbleInit = 1;
            block_pause();
            *(DWORD*)(ball + BALL_IMPACT_FREEZE) = 1000;
            *(BYTE*)(ball + BALL_RENDER_JITTER) = 1;
            startMusicFade();

            if (!IsBadReadPtr((void*)(ball + BALL_COLOR_R), 12)) {
                g_origBallR = *(float*)(ball + BALL_COLOR_R);
                g_origBallG = *(float*)(ball + BALL_COLOR_G);
                g_origBallB = *(float*)(ball + BALL_COLOR_B);
                g_colorSaved = 1;
            }
        }

        if (g_colorSaved && ball && !IsBadWritePtr((void*)(ball + BALL_COLOR_R), 12)) {
            float t = (float)elapsed / (float)RUMBLE_DURATION_MS;
            if (t > 1.0f) t = 1.0f;
            *(float*)(ball + BALL_COLOR_R) = g_origBallR + (TARGET_R - g_origBallR) * t;
            *(float*)(ball + BALL_COLOR_G) = g_origBallG + (TARGET_G - g_origBallG) * t;
            *(float*)(ball + BALL_COLOR_B) = g_origBallB + (TARGET_B - g_origBallB) * t;
        }

        if (ball) {
            BYTE respawning = *(BYTE*)(ball + 0x2F9);
            if (!respawning && !IsBadWritePtr((void*)(ball + BALL_ALPHA), 4)) {
                float t = (float)elapsed / (float)RUMBLE_DURATION_MS;
                if (t > 1.0f) t = 1.0f;
                *(float*)(ball + BALL_ALPHA) = 1.0f - (0.5f * t);
            }
        }

        updateMusicFade();

        if (elapsed >= RUMBLE_DURATION_MS) {
            g_phase = PHASE_FLASH;
            g_phaseStartTime = now;
            diag_log("[warp] -> PHASE_FLASH");
        }
        break;
    }

    case PHASE_FLASH: {
        elapsed = now - g_phaseStartTime;

        g_whiteAlpha = 1.0f - (float)elapsed / (float)FLASH_DURATION_MS;
        if (g_whiteAlpha < 0.0f) g_whiteAlpha = 0.0f;

        if (g_whiteAlpha >= 0.99f && ball) {
            BYTE respawning = *(BYTE*)(ball + 0x2F9);
            if (!respawning && !IsBadWritePtr((void*)(ball + BALL_ALPHA), 4)) {
                *(float*)(ball + BALL_ALPHA) = 0.0f;
                *(BYTE*)(ball + BALL_IN_TAR) = 1;  /* prevent ball death during warp */
                if (!g_freezeTimer) {
                    g_freezeTimer = 1;
                    diag_log("[warp] Timer frozen + in_tar set (g_freezeTimer=1)");
                }
            }
        }

        updateMusicFade();

        if (elapsed >= FLASH_DURATION_MS) {
            g_whiteAlpha = 0.0f;
            g_phase = PHASE_HOLD;
            g_phaseStartTime = now;
            diag_log("[warp] -> PHASE_HOLD");
        }
        break;
    }

    case PHASE_HOLD: {
        elapsed = now - g_phaseStartTime;
        g_whiteAlpha = 0.0f;

        if (ball) {
            BYTE respawning = *(BYTE*)(ball + 0x2F9);
            if (!respawning && !IsBadWritePtr((void*)(ball + BALL_ALPHA), 4))
                *(float*)(ball + BALL_ALPHA) = 0.0f;
        }

        updateMusicFade();

        if (elapsed >= HOLD_DURATION_MS) {
            g_phase = PHASE_FADE;
            g_phaseStartTime = now;
            diag_log("[warp] -> PHASE_FADE");
        }
        break;
    }

    case PHASE_FADE: {
        elapsed = now - g_phaseStartTime;
        g_whiteAlpha = (float)elapsed / (float)FADE_DURATION_MS;
        if (g_whiteAlpha > 1.0f) g_whiteAlpha = 1.0f;

        if (ball) {
            BYTE respawning = *(BYTE*)(ball + 0x2F9);
            if (!respawning && !IsBadWritePtr((void*)(ball + BALL_ALPHA), 4))
                *(float*)(ball + BALL_ALPHA) = 0.0f;
        }

        updateMusicFade();

        if (elapsed >= FADE_DURATION_MS) {
            g_whiteAlpha = 1.0f;
            g_phase = PHASE_LOAD;
            g_phaseStartTime = now;
            diag_log("[warp] -> PHASE_LOAD");
        }
        break;
    }

    case PHASE_LOAD: {
        int levelIdx = g_warpLevelIndex;
        g_warpLevelIndex = -1;

        if (ball) {
            *(DWORD*)(ball + BALL_IMPACT_FREEZE) = 0;
            *(BYTE*)(ball + BALL_RENDER_JITTER) = 0;
            *(BYTE*)(ball + BALL_IN_TAR) = 0;
            if (!IsBadWritePtr((void*)(ball + BALL_ALPHA), 4))
                *(float*)(ball + BALL_ALPHA) = 1.0f;

            if (g_colorSaved && !IsBadWritePtr((void*)(ball + BALL_COLOR_R), 12)) {
                *(float*)(ball + BALL_COLOR_R) = g_origBallR;
                *(float*)(ball + BALL_COLOR_G) = g_origBallG;
                *(float*)(ball + BALL_COLOR_B) = g_origBallB;
                g_colorSaved = 0;
            }
        }

        g_freezeTimer = 0;

        if (levelIdx >= 0 && levelIdx <= 14) {
            void *func;
            DWORD appVal = app;
            int idx = levelIdx;
            char savedDifficulty = *(BYTE*)((char*)app + 0x23C);
            DWORD oldProfile = *(DWORD*)((char*)app + APP_PROFILE_PTR);

            char wasInTournament = 0;
            int savedScores[16];
            int savedTimes[16];
            int hasTournamentData = 0;
            int savedTimeRemaining = 0;
            float savedPlayerScore = 0.0f;
            int isSameLevel = 0;

            if (oldProfile) {
                char isPractice = *(BYTE*)((char*)oldProfile + PROFILE_IS_PRACTICE);
                wasInTournament = (isPractice == 0) ? 1 : 0;

                {
                    int currentRaceIdx = *(int*)((char*)oldProfile + PROFILE_RACE_INDEX);
                    if (levelIdx == currentRaceIdx - 1) {
                        isSameLevel = 1;
                        diag_logf("[warp] Same-level warp detected (levelIdx=%d, profile raceIdx=%d)",
                                  levelIdx, currentRaceIdx);
                    }
                }

                /* Ghost 2 capture + segment save: before loading */
                if (isSameLevel) {
                    /* Set up TW race name if not already set */
                    if (!g_twRaceName[0]) {
                        char raceName[128];
                        if (get_race_name(raceName, sizeof(raceName))) {
                            strncpy(g_twRaceName, raceName, sizeof(g_twRaceName) - 1);
                            g_twRaceName[sizeof(g_twRaceName) - 1] = '\0';
                            diag_logf("[seg] TW race name set to '%s'", g_twRaceName);
                        }
                    }

                    /* Save Ghost 1 playback state before App_StartPracticeRace destroys it */
                    ghost1_save_state();

                    /* Save the current run segment to [N].ghost */
                    if (g_twRaceName[0]) {
                        save_warp_segment();
                    }

                    /* Check party mode — Ghost 2 only works in TT and Tournament */
                    BYTE partyMode = 0;
                    if (!IsBadReadPtr((void*)(app + APP_234_PARTY_MODE), 1))
                        partyMode = *(BYTE*)(app + APP_234_PARTY_MODE);
                    if (partyMode == 0) {
                        EnterCriticalSection(&g_cs);
                        /* Destroy old Ghost 2 before capturing so the capture buffer is the only copy */
                        if (g_ghost2.active || g_ghost2.ball || g_ghost2.btt) {
                            ghost2_destroy();
                        }
                        g_ghost2Pending = FALSE;
                        ghost2_capture();
                        LeaveCriticalSection(&g_cs);
                    }
                }

                if (wasInTournament) {
                    int raceIdx = *(int*)((char*)oldProfile + PROFILE_RACE_INDEX);
                    hasTournamentData = 1;
                    memcpy(savedScores, (void*)((char*)oldProfile + PROFILE_SCORE_ARRAY), sizeof(savedScores));
                    memcpy(savedTimes, (void*)((char*)oldProfile + PROFILE_TIME_ARRAY), sizeof(savedTimes));
                    if (raceIdx >= 0 && raceIdx < 16) {
                        savedScores[raceIdx] = (int)*(float*)((char*)app + APP_5E4_SCORE);
                    }
                    if (isSameLevel) {
                        if (raceIdx >= 0 && raceIdx < 16) {
                            savedTimeRemaining = *(int*)((char*)oldProfile + PROFILE_TIME_ARRAY + raceIdx * 4);
                        }
                    } else {
                        savedTimeRemaining = *(int*)((char*)app + APP_5E8_TIMER);
                    }
                    savedPlayerScore = *(float*)((char*)app + APP_5E4_SCORE);
                } else if (isSameLevel) {
                    savedTimeRemaining = *(int*)((char*)app + APP_5E8_TIMER);
                }
            }

            /* Use Tournament start for tournament-to-same-level warps, Practice otherwise */
            if (wasInTournament && isSameLevel) {
                func = (void *)APP_START_TOURNAMENT_RACE;
            } else {
                func = (void *)APP_START_PRACTICE_RACE;
            }

            __asm__ volatile(
                "push %[idx]\n\t"
                "movl %[appVal], %%ecx\n\t"
                "call *%[func]\n\t"
                : /* no outputs */
                : [func] "r" (func),
                  [appVal] "r" (appVal),
                  [idx] "r" (idx)
                : "eax", "edx", "ecx",
                  "st", "st(1)", "st(2)", "st(3)",
                  "st(4)", "st(5)", "st(6)", "st(7)", "memory"
            );

            *(BYTE*)((char*)app + 0x23C) = savedDifficulty;

            /* Ghost 1: Initialize for TW level or restore after same-level warp */
            if (isSameLevel && g_twRaceName[0]) {
                /* First time entering this TW level: init Ghost 1 if not active */
                if (!g_ghost1.active) {
                    ghost1_init_for_tw(g_twRaceName);
                }
                /* Restore Ghost 1 playback into the new App+0x910 slot */
                if (g_ghost1.active) {
                    ghost1_restore_after_warp();
                }
            } else if (!isSameLevel && g_twRaceName[0]) {
                /* Different level warp: reset TW state */
                g_ghost1.active = FALSE;
                g_ghost1.btt = 0;
                g_ghost1.currentSegment = 0;
                g_ghost1.playbackIdx = 0;
                g_ghost1.totalSegments = 0;
                g_twRaceName[0] = '\0';
                g_segmentCounter = 0;
                g_segmentCount = 0;
                memset(g_segmentTimes, 0, sizeof(g_segmentTimes));
                g_isTimeWarpLevel = FALSE;
            }

            if (wasInTournament && func == (void*)APP_START_TOURNAMENT_RACE) {
                /* Same-level tournament warp: App_StartTournamentRace already created BTT via hook */
                DWORD newProfile = *(DWORD*)((char*)app + APP_PROFILE_PTR);
                if (newProfile) {
                    *(BYTE*)((char*)newProfile + PROFILE_IS_PRACTICE) = 0;
                    if (hasTournamentData) {
                        memcpy((void*)((char*)newProfile + PROFILE_SCORE_ARRAY), savedScores, sizeof(savedScores));
                        memcpy((void*)((char*)newProfile + PROFILE_TIME_ARRAY), savedTimes, sizeof(savedTimes));
                    }

                    {
                        DWORD newProfile2 = *(DWORD*)((char*)app + APP_PROFILE_PTR);
                        DWORD newBoard2 = 0;
                        int levelBaseTime = 0;
                        int difficultyBonus = 0;
                        int finalTimer = savedTimeRemaining;

                        if (newProfile2)
                            newBoard2 = *(DWORD*)((char*)newProfile2 + PROFILE_BOARD_PTR);
                        if (newBoard2 && !IsBadReadPtr((void*)(newBoard2 + 0x2998), 4))
                            levelBaseTime = *(int*)((char*)newBoard2 + 0x2998);

                        {
                            int diff = (int)*(BYTE*)((char*)app + 0x23C);
                            if (diff == 0) difficultyBonus = 1000;
                            else if (diff == 1) difficultyBonus = 500;
                        }

                        finalTimer = savedTimeRemaining + levelBaseTime + difficultyBonus;
                        *(int*)((char*)app + APP_5E8_TIMER) = finalTimer;
                    }
                    *(float*)((char*)app + APP_5E4_SCORE) = savedPlayerScore;
                }
            } else if (wasInTournament) {
                /* Different-level tournament warp: game created BTTs via App_StartPracticeRace */
                {
                    DWORD bttRec = *(DWORD*)((char*)app + APP_BTT_RECORDING);
                    DWORD bttPlay = *(DWORD*)((char*)app + APP_BTT_PLAYBACK);
                    if (bttRec) {
                        if (!IsBadReadPtr((void*)bttRec, 4)) {
                            DWORD vt = *(DWORD*)bttRec;
                            if (vt == BTT_VTABLE_ADDR)
                                call_btt_dtor((void*)bttRec);
                            else
                                game_free((void*)bttRec);
                        }
                        *(DWORD*)((char*)app + APP_BTT_RECORDING) = 0;
                        diag_log("[warp] Destroyed BTT recording (tournament)");
                    }
                    if (bttPlay) {
                        if (!IsBadReadPtr((void*)bttPlay, 4)) {
                            DWORD vt = *(DWORD*)bttPlay;
                            if (vt == BTT_VTABLE_ADDR)
                                call_btt_dtor((void*)bttPlay);
                            else
                                game_free((void*)bttPlay);
                        }
                        *(DWORD*)((char*)app + APP_BTT_PLAYBACK) = 0;
                        diag_log("[warp] Destroyed BTT playback (tournament)");
                    }
                }

                /* Create new BTT at App+0x90C for Tournament recording */
                {
                    void *newBTT = game_operator_new(BTT_SIZE);
                    if (newBTT) {
                        call_btt_ctor(newBTT);
                        DWORD vt = *(DWORD*)newBTT;
                        if (vt == BTT_VTABLE_ADDR) {
                            *(DWORD*)((char*)newBTT + BTT_BEST_TIME) = NO_TIME;
                            *(DWORD*)(app + APP_BTT_RECORDING) = (DWORD)newBTT;
                            diag_logf("[warp] Created new recording BTT at 0x%X for Tournament", (DWORD)newBTT);
                        } else {
                            game_free(newBTT);
                            diag_logf("[warp] BTT ctor failed vtable=0x%X", vt);
                        }
                    }
                }

                {
                    DWORD newProfile = *(DWORD*)((char*)app + APP_PROFILE_PTR);
                    if (newProfile) {
                        *(BYTE*)((char*)newProfile + PROFILE_IS_PRACTICE) = 0;
                        if (hasTournamentData) {
                            memcpy((void*)((char*)newProfile + PROFILE_SCORE_ARRAY), savedScores, sizeof(savedScores));
                            memcpy((void*)((char*)newProfile + PROFILE_TIME_ARRAY), savedTimes, sizeof(savedTimes));
                        }

                        {
                            DWORD newProfile2 = *(DWORD*)((char*)app + APP_PROFILE_PTR);
                            DWORD newBoard2 = 0;
                            int levelBaseTime = 0;
                            int difficultyBonus = 0;
                            int finalTimer = savedTimeRemaining;

                            if (newProfile2)
                                newBoard2 = *(DWORD*)((char*)newProfile2 + PROFILE_BOARD_PTR);
                            if (newBoard2 && !IsBadReadPtr((void*)(newBoard2 + 0x2998), 4))
                                levelBaseTime = *(int*)((char*)newBoard2 + 0x2998);

                            {
                                int diff = (int)*(BYTE*)((char*)app + 0x23C);
                                if (diff == 0) difficultyBonus = 1000;
                                else if (diff == 1) difficultyBonus = 500;
                            }

                            finalTimer = savedTimeRemaining + levelBaseTime + difficultyBonus;
                            *(int*)((char*)app + APP_5E8_TIMER) = finalTimer;
                        }
                        *(float*)((char*)app + APP_5E4_SCORE) = savedPlayerScore;
                    }
                }
            } else if (isSameLevel && savedTimeRemaining > 0) {
                *(int*)((char*)app + APP_5E8_TIMER) = savedTimeRemaining;
                diag_logf("[warp] Same-level TT: timer resumed at %d", savedTimeRemaining);
            }
        }

        /* Write white alpha to new board */
        {
            DWORD newProfile = *(DWORD*)((char*)app + APP_PROFILE_PTR);
            if (newProfile) {
                DWORD newBoard = *(DWORD*)((char*)newProfile + PROFILE_BOARD_PTR);
                if (newBoard && !IsBadWritePtr((void*)(newBoard + SCENE_FADE_ALPHA), 4))
                    *(float*)((char*)newBoard + SCENE_FADE_ALPHA) = 1.0f;
            }
        }

        restoreMusicFade();

        g_warpBall = 0;
        g_whiteAlpha = 1.0f;
        g_phase = PHASE_REVEAL;
        g_phaseStartTime = now;
        diag_log("[warp] -> PHASE_REVEAL");
        break;
    }

    case PHASE_REVEAL: {
        elapsed = now - g_phaseStartTime;
        g_whiteAlpha = 1.0f - (float)elapsed / (float)REVEAL_DURATION_MS;
        if (g_whiteAlpha < 0.0f) g_whiteAlpha = 0.0f;

        if (elapsed >= REVEAL_DURATION_MS) {
            g_whiteAlpha = 0.0f;
            g_phase = PHASE_IDLE;
            g_cooldownUntil = getGameTime() + WARP_COOLDOWN_MS;
            unblock_pause();
            diag_log("[warp] -> PHASE_IDLE: warp complete");
        }
        break;
    }

    default:
        g_freezeTimer = 0;
        unblock_pause();
        g_phase = PHASE_IDLE;
        break;
    }
}

/* ================================================================
 * Shared frame epilogue hook — dispatches to all subsystems
 * ================================================================ */

void __cdecl frame_epilogue_handler(void) {
    DWORD app = get_app();
    DWORD board = 0;

    /* Update game clock */
    if (app) {
        DWORD profile = *(DWORD*)((char*)app + APP_PROFILE_PTR);
        if (profile && !IsBadReadPtr((void*)profile, 0x100)) {
            board = *(DWORD*)((char*)profile + PROFILE_BOARD_PTR);
            if (board && !IsBadReadPtr((void*)board, 0x4000)) {
                if (*(DWORD*)((char*)board + BOARD_APP_PTR) != app)
                    board = 0;
            } else {
                board = 0;
            }
        }
        updateGameClock(board);
    }

    /* 1. Ghost saver: check goal flag, save ghost on finish (handled by background thread) */

    /* 2. Ghost event: check pending ghost loads, advance playback, cleanup */
    ghost_event_frame();

    /* 3. Ghost 2: board change detection + playback */
    ghost2_check_board_change(board);
    ghost2_playback();

    /* 3b. Ghost 1: multi-segment chaining — advance to next segment if current ended */
    ghost1_check_advance();

    /* 4. Ghost triggers: check both ghosts against triggers */
    if (board && board != g_triggerBoard) {
        g_triggerBoard = board;
        scan_s1_triggers(board);
    }
    if (board && g_triggerCount > 0) {
        check_ghost_triggers(board);
    }

    /* 5. Warp: scan for warp nodes (if idle) + advance state machine */
    if (g_phase == PHASE_IDLE) {
        scanWarpNodes();
    }
    updateWarpStateMachine();
}

/* ================================================================
 * Frame epilogue hook installation
 * ================================================================ */
static BYTE *g_frame_stub = NULL;
static BYTE g_frame_original[5];

static void build_frame_stub(void) {
    BYTE *code = (BYTE*)alloc_executable(64);
    int i = 0;
    code[i++] = 0x60;  /* pushad */
    code[i++] = 0x9C;  /* pushfd */
    code[i++] = 0xE8;   /* call frame_epilogue_handler */
    *(DWORD*)(code + i) = (DWORD)&frame_epilogue_handler - (DWORD)(code + i + 4);
    i += 4;
    code[i++] = 0x9D;  /* popfd */
    code[i++] = 0x61;  /* popad */
    /* original 5 bytes: POP ESI / ADD ESP,8 / RET */
    code[i++] = 0x5E;  /* pop esi */
    code[i++] = 0x83; code[i++] = 0xC4; code[i++] = 0x08;  /* add esp, 8 */
    code[i++] = 0xC3;  /* ret */
    g_frame_stub = code;
}

static void install_frame_hook(void) {
    unsigned char *epiAddr = (unsigned char *)APP_FRAME_UPDATE_EPILOGUE;

    if (epiAddr[0] != 0x5E || epiAddr[1] != 0x83 || epiAddr[2] != 0xC4 ||
        epiAddr[3] != 0x08 || epiAddr[4] != 0xC3) {
        diag_log("[FATAL] FrameUpdate epilogue signature mismatch!");
        return;
    }

    memcpy(g_frame_original, epiAddr, 5);
    build_frame_stub();

    BYTE patch[5];
    patch[0] = 0xE9;
    *(DWORD*)(patch + 1) = (DWORD)g_frame_stub - (DWORD)epiAddr - 5;
    patch_bytes((void*)epiAddr, patch, 5);

    diag_log("[time_warp] Frame epilogue hook installed");
}

/* ================================================================
 * Init thread — install all hooks
 * ================================================================ */
static DWORD WINAPI init_thread(LPVOID param) {
    (void)param;
    Sleep(2000);

    diag_log("=== TIME WARP MOD INIT ===");

    /* Install App_StartTournamentRace hook (creates BTT for Tournament recording) */
    install_tournament_hook();

    /* Install DCE hook (E:GHOST collision events) */
    install_dce_hook();

    /* Install App_StartPracticeRace hook (ghost saver) */
    install_practice_hook();

    /* Install timer freeze code caves */
    install_timer_caves();

    /* Install TT recording NOP patch (enables recording in Tournament) */
    install_tt_recording_nop();

    /* Install shared frame epilogue hook */
    install_frame_hook();

    /* Install ghost mode code cave (E:GHOST works in all modes, normal ghosts restricted to TT) */
    install_ghost_mode_cave();

    /* Start ghost saver background thread */
    CreateThread(NULL, 0, ghost_saver_thread, NULL, 0, NULL);

    diag_log("=== TIME WARP MOD INITIALIZED ===");
    diag_log("  - Frame epilogue hook at 0x46C1F1");
    diag_log("  - App_StartPracticeRace hook at 0x428C50");
    diag_log("  - App_StartTournamentRace hook at 0x4288B0");
    diag_log("  - DCE hook at 0x40C5D0");
    diag_log("  - Timer caves at 0x41B3E5 + 0x41B50C");
    diag_log("  - TT recording NOP at 0x41B690");
    diag_log("  - Ghost saver thread started");
    diag_log("  - Ghost 2 subsystem ready (same-level warp, TT+Tournament only)");
    diag_log("  - Ghost triggers scanning S1 for GT: entries");

    return 0;
}


/* ================================================================
 * Hook cleanup functions
 * ================================================================ */

static void restore_practice_hook(void) {
    if (g_hookInstalled && g_trampoline) {
        unsigned char *target = (unsigned char*)ADDR_APP_START_PRACTICE;
        DWORD oldProtect;
        if (VirtualProtect(target, HOOK_BYTES, PAGE_EXECUTE_READWRITE, &oldProtect)) {
            memcpy(target, g_origBytes, HOOK_BYTES);
            VirtualProtect(target, HOOK_BYTES, oldProtect, &oldProtect);
            FlushInstructionCache(GetCurrentProcess(), target, HOOK_BYTES);
        }
        g_hookInstalled = 0;
        diag_log("[ghost_saver] App_StartPracticeRace hook restored");
    }
}

static void restore_tournament_hook(void) {
    if (g_tournamentHookInstalled) {
        DWORD oldProt;
        if (VirtualProtect((void*)APP_START_TOURNAMENT_RACE, TOURNAMENT_HOOK_BYTES,
            PAGE_EXECUTE_READWRITE, &oldProt)) {
            memcpy((void*)APP_START_TOURNAMENT_RACE, g_tournamentOrigBytes,
                   TOURNAMENT_HOOK_BYTES);
            VirtualProtect((void*)APP_START_TOURNAMENT_RACE, TOURNAMENT_HOOK_BYTES,
                oldProt, &oldProt);
            FlushInstructionCache(GetCurrentProcess(), (void*)APP_START_TOURNAMENT_RACE, TOURNAMENT_HOOK_BYTES);
        }
        g_tournamentHookInstalled = 0;
        diag_log("[tournament_hook] App_StartTournamentRace hook restored");
    }
}

static void restore_frame_hook(void) {
    if (g_frame_stub) {
        patch_bytes((void*)APP_FRAME_UPDATE_EPILOGUE, g_frame_original, 5);
        g_frame_stub = NULL;
        diag_log("[time_warp] Frame epilogue hook restored");
    }
}

/* ================================================================
 * DllMain
 * ================================================================ */
BOOL WINAPI DllMain(HINSTANCE hInst, DWORD reason, LPVOID reserved) {
    (void)reserved;
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hInst);

        /* Set up log path and ghost directory */
        {
            char modPath[MAX_PATH];
            if (GetModuleFileNameA(hInst, modPath, MAX_PATH)) {
                char *p = strrchr(modPath, '\\');
                if (p) {
                    *p = '\0';
                    snprintf(g_logPath, MAX_PATH, "%s\\time_warp_log.txt", modPath);
                    snprintf(g_ghostDir, MAX_PATH, "%s\\Ghosts\\", modPath);
                    CreateDirectoryA(g_ghostDir, NULL);
                }
            }
        }

        diag_log("=== TIME WARP MOD LOADED ===");

        /* CRITICAL: load_real_bass MUST be called before CreateThread */
        load_real_bass();
        diag_logf("bass_real.dll handle: 0x%08X", (unsigned)g_hRealBass);

        InitializeCriticalSection(&g_cs);

        CreateThread(NULL, 0, init_thread, NULL, 0, NULL);
        break;

    case DLL_PROCESS_DETACH:
        /* Signal background thread to exit */
        g_shuttingDown = TRUE;
        Sleep(50);  /* Give thread time to notice */
        g_freezeTimer = 0;
        restore_timer_caves();
        restore_tt_recording_nop();
        unblock_pause();
        restore_practice_hook();
        restore_tournament_hook();
        restore_dce_hook();
        restore_frame_hook();
        restore_ghost_mode_cave();

        /* Clean up Ghost 2 */
        ghost2_destroy();

        /* Clean up Ghost 2 capture buffer */
        if (g_ghost2Capture) {
            free(g_ghost2Capture);
            g_ghost2Capture = NULL;
            g_ghost2CaptureCount = 0;
        }

        /* Clean up ghost event resources */
        if (g_loadedBTT) {
            DWORD app = get_app();
            if (app) {
                if (*(DWORD*)(app + APP_BTT_PLAYBACK) == g_loadedBTT)
                    *(DWORD*)(app + APP_BTT_PLAYBACK) = 0;
            }
            if (!IsBadReadPtr((void*)g_loadedBTT, 0x100)) {
                DWORD vt = *(DWORD*)g_loadedBTT;
                if (vt == BTT_VTABLE_ADDR)
                    call_btt_dtor((void*)g_loadedBTT);
                else
                    game_free((void*)g_loadedBTT);
            }
            g_loadedBTT = 0;
        }

        /* Clean up ghost saver */
        if (g_rawSnaps) {
            free(g_rawSnaps);
            g_rawSnaps = NULL;
        }

        DeleteCriticalSection(&g_cs);
        break;
    }
    return TRUE;
}
