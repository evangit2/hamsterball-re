/*
 * Time Warp for HB+ v2.1
 *
 * Port of time_warp (bass.dll proxy) to the Hamsterball Plus API.
 *
 * Subsystems combined:
 *   1. Level Warp core     — WARP(Name) node triggers, warp state machine.
 *   2. Ghost Saver         — Time Trial ghost persistence to .ghost files.
 *   3. Ghost Event         — E:GHOST(filename) event plane collisions.
 *   4. Time Warp specific  — same-level segment capture, Ghost 2 spawn,
 *                              multi-segment [N] / (N) ghost file handling.
 */
#define _CRT_SECURE_NO_WARNINGS
#include "HamsterballAPI.h"
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>

/* ---- Config ---- */
#define ENABLE_LOGGING

/* ---- Game function RVAs (relative to base) ---- */
#define RVA_APP_START_PRACTICE_RACE 0x0028C50
#define RVA_APP_START_TOURNAMENT_RACE 0x00288B0
#define RVA_BTT_CTOR                  0x0027660
#define RVA_BTT_DTOR                  0x00278C0
#define RVA_BALL_CTOR                 0x00039E0
#define RVA_BALL_DTOR                 0x0002A50
#define RVA_ALIST_APPEND              0x0053780
#define RVA_ALIST_REMOVE              0x00534D0
#define RVA_OPERATOR_NEW              0x00BA57B
#define RVA_GAME_FREE                 0x00BA74D

/* ---- Absolute / fixed addresses ---- */
#define APP_PTR                       0x005341E0
#define RACE_NAME_TABLE               0x004F7080
#define BTT_VTABLE_ADDR               0x004D262C
#define ATHENALIST_VT                 0x004D875C

/* ---- App offsets ---- */
#define APP_PROFILE_PTR               0x220
#define APP_MUSIC_DEVICE_PTR          0x17C
#define APP_BTT_RECORDING             0x90C
#define APP_BTT_PLAYBACK              0x910
#define APP_DIFFICULTY                0x23C
#define APP_SCORE                     0x5E4
#define APP_TIME_REMAINING            0x5E8
#define APP_TIMER_FINISHED            0x5D6
#define APP_PARTY_FLAG                0x234

/* ---- Profile / Board offsets ---- */
#define PROFILE_RACE_INDEX            0x08
#define PROFILE_BOARD_PTR             0x0C
#define PROFILE_IS_PRACTICE           0x11
#define PROFILE_SCORE_ARRAY           0x50
#define PROFILE_TIME_ARRAY            0x14

#define BOARD_APP_PTR                 0x878
#define BOARD_MESHWORLD_PTR           0x8AC
#define BOARD_GOAL_REACHED            0xCD0
#define BOARD_PAUSED_FLAG             0x874
#define BOARD_SCENE_FADE_ALPHA        0x3624
#define BOARD_BALL_LIST               0x29D4
#define BOARD_BALL_LIST_DATA          0x2DE0
#define BOARD_GHOST_BALL              0x361C

/* ---- MeshWorld / S1 / node offsets ---- */
#define MW_SECTION3_PTR               0x480
#define MW_NODELIST_COUNT             0x898
#define MW_NODELIST_DATA              0xCA0
#define MW_S1_COUNT                   0x898
#define MW_S1_ARRAY                   0xCA0

#define NODE_NAME_OFFSET              0x00
#define NODE_X_OFFSET                 0x04
#define NODE_Y_OFFSET                 0x08
#define NODE_Z_OFFSET                 0x0C

/* ---- Ball offsets ---- */
#define BALL_VTABLE                   0x00
#define BALL_PLAYER_ID                0x018
#define BALL_POS_X                    0x164
#define BALL_POS_Y                    0x168
#define BALL_POS_Z                    0x16C
#define BALL_MAX_SPEED                0x188
#define BALL_MASS                     0x1A0
#define BALL_GRAVITY                  0x278
#define BALL_GRAVITY_SCALE            0x27C
#define BALL_RADIUS                   0x284
#define BALL_RESPAWN_FLAG             0x281
#define BALL_ALPHA                    0x2FC
#define BALL_COLOR_R                  0x2AC
#define BALL_COLOR_G                  0x2B0
#define BALL_COLOR_B                  0x2B4
#define BALL_IN_TAR                   0x2CC
#define BALL_DEATH_PENDING            0x2E9
#define BALL_RENDER_JITTER            0x2D4
#define BALL_IMPACT_FREEZE            0x808

/* ---- BTT layout ---- */
#define BTT_SIZE                      0x528
#define BTT_ALIST_COUNT               0x08
#define BTT_LIST_ARRAY                0x410
#define BTT_PLAYBACK_IDX              0x41C
#define BTT_RACE_TIME                 0x420
#define BTT_RACE_NAME                 0x424
#define BTT_BEST_TIME                 0x524
#define SNAP_SIZE                     0x28
#define SNAP_DWORDS                   10
#define SNAP_BYTES                    40
#define NO_TIME                       9999999

/* ---- Music offsets ---- */
#define MUSIC_DEV_CHANNEL_LIST        0x418
#define MUSIC_DEV_COUNT               0x10
#define MUSIC_CHAN_BASS_CHANNEL       0x08
#define MUSIC_CHAN_VOLUME             0x528
#define MUSIC_CHAN_FADE_RATE          0x52C
#define MUSIC_CHAN_FADE_OUT           0x530
#define MUSIC_CHAN_FADE_IN            0x531
#define MAX_MUSIC_CHANNELS            32

/* ---- Timer patch RVAs ---- */
#define TIMER_DEC_PATCH_RVA           0x1B3E5
#define TIMER_DEC_PATCH_SIZE          9
#define TIMER_DEC_RETURN_RVA          0x1B3EE
#define TIMER_DEC_SKIP_RVA            0x1B49D

#define TIMER_INC_PATCH_RVA           0x1B50C
#define TIMER_INC_PATCH_SIZE          5
#define TIMER_INC_RETURN_RVA          0x1B511

/* ---- Pause-block patch RVAs ---- */
#define PAUSE_PATCH_PATH1             0x19D5B
#define PAUSE_PATCH_PATH2             0x130B5
#define PAUSE_PATCH_PATH3             0x0B405

/* ---- Warp params ---- */
#define WARP_TRIGGER_DIST_SQ          625.0f
#define WARP_COOLDOWN_MS              2000
#define RUMBLE_DURATION_MS            2000
#define FLASH_DURATION_MS             150
#define HOLD_DURATION_MS              1000
#define FADE_DURATION_MS              2000
#define REVEAL_DURATION_MS            1000
#define MUSIC_FADE_MS                 3000

/* ---- Time warp constants ---- */
#define MAX_SEGMENTS                  32
#define DEFAULT_TRIGGER_RADIUS        50.0f
#define TRIGGER_COOLDOWN_FRAMES       60
#define MAX_TRIGGERS                  64
#define TRIGGER_PREFIX                "GT:"

/* ---- Heliotrope purple (#db03fc) ---- */
#define TARGET_PURPLE_R               (219.0f / 255.0f)
#define TARGET_PURPLE_G               (3.0f / 255.0f)
#define TARGET_PURPLE_B               (252.0f / 255.0f)

/* ---- Logging ---- */
#ifdef ENABLE_LOGGING
static char g_logPath[MAX_PATH] = "";
static void diag_log(const char* msg) {
    if (!g_logPath[0]) return;
    HANDLE h = CreateFileA(g_logPath, FILE_APPEND_DATA,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, NULL);
    if (h != INVALID_HANDLE_VALUE) {
        DWORD w;
        SetFilePointer(h, 0, NULL, FILE_END);
        WriteFile(h, msg, (DWORD)strlen(msg), &w, NULL);
        WriteFile(h, "\r\n", 2, &w, NULL);
        CloseHandle(h);
    }
}
static void diag_logf(const char* fmt, ...) {
    char buf[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    diag_log(buf);
}
#define LOG(...)   diag_logf(__VA_ARGS__)
#define LOGS(s)    diag_log(s)
#else
#define LOG(...)   do {} while(0)
#define LOGS(s)    do {} while(0)
#endif

/* ---- Ghost file header ---- */
#pragma pack(push, 1)
typedef struct {
    DWORD magic;
    DWORD version;
    DWORD time;
    DWORD frameCount;
} GhostFileHeader;
#pragma pack(pop)
#define GHOST_MAGIC     0x47485347  /* "GHSG" */
#define GHOST_VERSION   1

/* ---- Globals ---- */
static IModAPI* g_api = nullptr;
static DWORD g_gameBase = 0x00400000;
static char g_ghostDir[MAX_PATH] = "";

static bool g_twEnabled = true;            /* toggle button state */
static bool g_warpEnabled = true;           /* internal TIME_WARP_ENABLED */

/* ---- BASS music fade ---- */
typedef int (__stdcall* BASS_ChannelSetAttributes_t)(DWORD handle, float freq, int volume, int pan);
static BASS_ChannelSetAttributes_t g_realBASS_ChannelSetAttributes = NULL;

static void load_bass_function(void) {
    if (g_realBASS_ChannelSetAttributes) return;
    HMODULE hBass = LoadLibraryA("bass.dll");
    if (hBass) {
        g_realBASS_ChannelSetAttributes = (BASS_ChannelSetAttributes_t)GetProcAddress(hBass, "BASS_ChannelSetAttributes");
    }
}

/* ---- Utility helpers ---- */
static DWORD get_app(void) {
    if (IsBadReadPtr((void*)APP_PTR, 4)) return 0;
    DWORD app = *(DWORD*)APP_PTR;
    if (!app || app < 0x10000) return 0;
    return app;
}

static DWORD get_board(DWORD app) {
    if (!app || IsBadReadPtr((void*)app, 0x1000)) return 0;
    DWORD profile = *(DWORD*)(app + APP_PROFILE_PTR);
    if (!profile || IsBadReadPtr((void*)profile, 0x100)) return 0;
    DWORD board = *(DWORD*)(profile + PROFILE_BOARD_PTR);
    if (!board || IsBadReadPtr((void*)board, 0x4000)) return 0;
    return board;
}

static int is_game_paused(DWORD board) {
    if (!board) return 0;
    if (IsBadReadPtr((void*)(board + BOARD_PAUSED_FLAG), 1)) return 0;
    return *(BYTE*)(board + BOARD_PAUSED_FLAG) != 0;
}

static DWORD g_gameClock = 0;
static DWORD g_lastRealTime = 0;

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

static void write_jmp(unsigned char* at, DWORD target) {
    at[0] = 0xE9;
    *(DWORD*)(at + 1) = target - (DWORD)at - 5;
}

/* ---- Timer freeze code caves ---- */
static BYTE g_decOrigBytes[TIMER_DEC_PATCH_SIZE];
static BYTE g_incOrigBytes[TIMER_INC_PATCH_SIZE];
static int g_decPatched = 0;
static int g_incPatched = 0;
static unsigned char* g_decCave = NULL;
static unsigned char* g_incCave = NULL;
static volatile BYTE g_freezeTimer = 0;

static void install_timer_caves(DWORD base) {
    DWORD oldProt;
    if (!g_decPatched) {
        DWORD patchAddr = base + TIMER_DEC_PATCH_RVA;
        DWORD returnAddr = base + TIMER_DEC_RETURN_RVA;
        DWORD skipAddr = base + TIMER_DEC_SKIP_RVA;
        DWORD freezeAddr = (DWORD)&g_freezeTimer;

        g_decCave = (unsigned char*)VirtualAlloc(NULL, 64, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        if (!g_decCave) { LOG("VirtualAlloc failed for DEC cave"); return; }

        unsigned char* p = g_decCave;
        p[0] = 0x0F; p[1] = 0x85;
        *(DWORD*)(p + 2) = (DWORD)(p + 6 + 7 + 6 + 3 + 5) - (DWORD)(p + 2 + 4);
        p += 6;

        p[0] = 0x80; p[1] = 0x3D;
        *(DWORD*)(p + 2) = freezeAddr;
        p[6] = 0x00;
        p += 7;

        p[0] = 0x0F; p[1] = 0x85;
        *(DWORD*)(p + 2) = (DWORD)(p + 6 + 3 + 5) - (DWORD)(p + 2 + 4);
        p += 6;

        p[0] = 0xFF; p[1] = 0x4F; p[2] = 0x1C;
        p += 3;

        write_jmp(p, returnAddr);
        p += 5;
        write_jmp(p, skipAddr);
        p += 5;

        memcpy(g_decOrigBytes, (void*)patchAddr, TIMER_DEC_PATCH_SIZE);
        VirtualProtect((void*)patchAddr, TIMER_DEC_PATCH_SIZE, PAGE_EXECUTE_READWRITE, &oldProt);
        write_jmp((unsigned char*)patchAddr, (DWORD)g_decCave);
        memset((unsigned char*)patchAddr + 5, 0x90, TIMER_DEC_PATCH_SIZE - 5);
        VirtualProtect((void*)patchAddr, TIMER_DEC_PATCH_SIZE, oldProt, &oldProt);
        g_decPatched = 1;
        LOG("DEC timer cave installed at 0x%08X", patchAddr);
    }

    if (!g_incPatched) {
        DWORD patchAddr = base + TIMER_INC_PATCH_RVA;
        DWORD returnAddr = base + TIMER_INC_RETURN_RVA;
        DWORD freezeAddr = (DWORD)&g_freezeTimer;

        g_incCave = (unsigned char*)VirtualAlloc(NULL, 64, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        if (!g_incCave) { LOG("VirtualAlloc failed for INC cave"); return; }

        unsigned char* p = g_incCave;
        p[0] = 0x75;
        p[1] = 0x0C;
        p += 2;

        p[0] = 0x80; p[1] = 0x3D;
        *(DWORD*)(p + 2) = freezeAddr;
        p[6] = 0x00;
        p += 7;

        p[0] = 0x75;
        p[1] = 0x03;
        p += 2;

        p[0] = 0xFF; p[1] = 0x42; p[2] = 0x1C;
        p += 3;

        write_jmp(p, returnAddr);
        p += 5;

        memcpy(g_incOrigBytes, (void*)patchAddr, TIMER_INC_PATCH_SIZE);
        VirtualProtect((void*)patchAddr, TIMER_INC_PATCH_SIZE, PAGE_EXECUTE_READWRITE, &oldProt);
        write_jmp((unsigned char*)patchAddr, (DWORD)g_incCave);
        VirtualProtect((void*)patchAddr, TIMER_INC_PATCH_SIZE, oldProt, &oldProt);
        g_incPatched = 1;
        LOG("INC timer cave installed at 0x%08X", patchAddr);
    }
}

static void restore_timer_caves(DWORD base) {
    DWORD oldProt;
    if (g_decPatched && g_decCave) {
        DWORD patchAddr = base + TIMER_DEC_PATCH_RVA;
        VirtualProtect((void*)patchAddr, TIMER_DEC_PATCH_SIZE, PAGE_EXECUTE_READWRITE, &oldProt);
        memcpy((void*)patchAddr, g_decOrigBytes, TIMER_DEC_PATCH_SIZE);
        VirtualProtect((void*)patchAddr, TIMER_DEC_PATCH_SIZE, oldProt, &oldProt);
        VirtualFree(g_decCave, 0, MEM_RELEASE);
        g_decCave = NULL;
        g_decPatched = 0;
        LOGS("DEC timer cave restored");
    }
    if (g_incPatched && g_incCave) {
        DWORD patchAddr = base + TIMER_INC_PATCH_RVA;
        VirtualProtect((void*)patchAddr, TIMER_INC_PATCH_SIZE, PAGE_EXECUTE_READWRITE, &oldProt);
        memcpy((void*)patchAddr, g_incOrigBytes, TIMER_INC_PATCH_SIZE);
        VirtualProtect((void*)patchAddr, TIMER_INC_PATCH_SIZE, oldProt, &oldProt);
        VirtualFree(g_incCave, 0, MEM_RELEASE);
        g_incCave = NULL;
        g_incPatched = 0;
        LOGS("INC timer cave restored");
    }
}

/* ---- Pause-block patches ---- */
static BYTE g_pauseOrigBytes[3] = {0, 0, 0};
static int g_pauseBlocked = 0;

static void block_pause(DWORD base) {
    if (g_pauseBlocked) return;
    DWORD addrs[3] = {PAUSE_PATCH_PATH1, PAUSE_PATCH_PATH2, PAUSE_PATCH_PATH3};
    DWORD oldProt;
    for (int i = 0; i < 3; i++) {
        DWORD addr = base + addrs[i];
        VirtualProtect((void*)addr, 1, PAGE_EXECUTE_READWRITE, &oldProt);
        g_pauseOrigBytes[i] = *(BYTE*)addr;
        *(BYTE*)addr = 0xEB;
        VirtualProtect((void*)addr, 1, oldProt, &oldProt);
    }
    g_pauseBlocked = 1;
    LOGS("Pause blocked");
}

static void unblock_pause(DWORD base) {
    if (!g_pauseBlocked) return;
    DWORD addrs[3] = {PAUSE_PATCH_PATH1, PAUSE_PATCH_PATH2, PAUSE_PATCH_PATH3};
    DWORD oldProt;
    for (int i = 0; i < 3; i++) {
        DWORD addr = base + addrs[i];
        VirtualProtect((void*)addr, 1, PAGE_EXECUTE_READWRITE, &oldProt);
        *(BYTE*)addr = g_pauseOrigBytes[i];
        VirtualProtect((void*)addr, 1, oldProt, &oldProt);
    }
    g_pauseBlocked = 0;
    LOGS("Pause unblocked");
}

/* Ghost mode code cave — allows E:GHOST ghosts in all modes,
 * but keeps normal ghosts restricted to Time Trial only.
 *
 * Patches 0x40B7F0 (17 bytes) which normally has:
 *   MOV DL, [ECX+0x11]   ; profile->practice
 *   TEST DL, DL
 *   JZ +0x3D              ; skip (to 0x40B834) if NOT TT
 *   MOV CL, [EAX+0x234]  ; app->party_mode
 *   TEST CL, CL
 *   JNZ +0x33             ; skip (to 0x40B834) if party
 *   ; continue (0x40B801): ghost render code
 *
 * Cave flow:
 *   if (g_ghostFromEvent) -> allow render (JMP 0x40B801)
 *   else run original checks, jumping to skip (0x40B834) or
 *        continuing at 0x40B801 like normal.
 */
extern BOOL g_ghostFromEvent;
#define GHOST_MODE_PATCH_RVA   0x0B7F0
#define GHOST_MODE_PATCH_SIZE  17
#define GHOST_MODE_CONTINUE    0x0B801
#define GHOST_MODE_SKIP        0x0B834

static unsigned char* g_ghostModeCave = NULL;
static unsigned char g_ghostModeOrigBytes[17];

static void install_ghost_mode_cave(DWORD base) {
    DWORD patchAddr = base + GHOST_MODE_PATCH_RVA;
    DWORD continueAddr = base + GHOST_MODE_CONTINUE;
    DWORD skipAddr = base + GHOST_MODE_SKIP;
    DWORD ghostFromEventAddr = (DWORD)&g_ghostFromEvent;

    g_ghostModeCave = (unsigned char*)VirtualAlloc(NULL, 64, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!g_ghostModeCave) { LOGS("GhostMode: VirtualAlloc failed"); return; }

    unsigned char* p = g_ghostModeCave;

    /* CMP byte [g_ghostFromEvent], 0  (7 bytes) */
    p[0] = 0x80; p[1] = 0x3D;
    *(DWORD*)(p + 2) = ghostFromEventAddr;
    p[6] = 0x00;
    p += 7;

    /* JNE rel8 to ALLOW path (skip gate checks if ghostFromEvent is set) */
    /* Target: the ALLOW JMP below */
    p[0] = 0x75;
    p[1] = 0; /* placeholder */
    unsigned char* jnePatcher = p;
    p += 2;

    /* Gate check 1: MOV DL, [ECX+0x11] (3 bytes) */
    p[0] = 0x8A; p[1] = 0x51; p[2] = 0x11;
    p += 3;

    /* Gate check 1: TEST DL, DL (2 bytes) */
    p[0] = 0x84; p[1] = 0xD2;
    p += 2;

    /* JZ rel8 to SKIP JMP (if NOT TT, skip ghost) */
    p[0] = 0x74;
    p[1] = 0; /* placeholder */
    unsigned char* jzPatcher = p;
    p += 2;

    /* Gate check 2: MOV CL, [EAX+0x234] (6 bytes) */
    p[0] = 0x8A; p[1] = 0x88;
    *(DWORD*)(p + 2) = 0x234;
    p += 6;

    /* Gate check 2: TEST CL, CL (2 bytes) */
    p[0] = 0x84; p[1] = 0xC9;
    p += 2;

    /* JNZ rel8 to SKIP JMP (if party, skip ghost) */
    p[0] = 0x75;
    p[1] = 0; /* placeholder */
    unsigned char* jnzPatcher = p;
    p += 2;

    /* ALLOW: JMP to ghost render code (5 bytes) */
    write_jmp(p, continueAddr);
    unsigned char* allowJmp = p;
    p += 5;

    /* SKIP: JMP over ghost render code (5 bytes) */
    write_jmp(p, skipAddr);
    unsigned char* skipJmp = p;

    /* Now patch the forward jump displacements (all targets are ahead, so
     * rel8 = target - (patch_address + 2) where patch is 2 bytes) */
    jnePatcher[1] = (unsigned char)(allowJmp - (jnePatcher + 2)); /* skip to ALLOW */
    jzPatcher[1]  = (unsigned char)(skipJmp  - (jzPatcher  + 2)); /* skip ghost */
    jnzPatcher[1] = (unsigned char)(skipJmp  - (jnzPatcher + 2)); /* skip ghost */

    DWORD oldProt;
    memcpy(g_ghostModeOrigBytes, (void*)patchAddr, GHOST_MODE_PATCH_SIZE);
    VirtualProtect((void*)patchAddr, GHOST_MODE_PATCH_SIZE, PAGE_EXECUTE_READWRITE, &oldProt);
    write_jmp((unsigned char*)patchAddr, (DWORD)g_ghostModeCave);
    memset((unsigned char*)patchAddr + 5, 0x90, GHOST_MODE_PATCH_SIZE - 5);
    VirtualProtect((void*)patchAddr, GHOST_MODE_PATCH_SIZE, oldProt, &oldProt);
    LOGS("Ghost mode code cave installed at 0x40B7F0");
}

static void restore_ghost_mode_cave(DWORD base) {
    (void)base;
    if (!g_ghostModeCave) return;
    DWORD oldProt;
    DWORD patchAddr = base + GHOST_MODE_PATCH_RVA;
    VirtualProtect((void*)patchAddr, GHOST_MODE_PATCH_SIZE, PAGE_EXECUTE_READWRITE, &oldProt);
    memcpy((void*)patchAddr, g_ghostModeOrigBytes, GHOST_MODE_PATCH_SIZE);
    VirtualProtect((void*)patchAddr, GHOST_MODE_PATCH_SIZE, oldProt, &oldProt);
    VirtualFree(g_ghostModeCave, 0, MEM_RELEASE);
    g_ghostModeCave = NULL;
    LOGS("Ghost mode code cave restored");
}

/* ---- Level name mapping ----
 * The mod contract: WARP(...) takes ONLY a race NUMBER or race NAME.
 * Filenames (level1..level10, cascade, up, dark, glass, impossible) are
 * NOT valid inputs — file numbering does not match race numbering
 * (e.g. levels\level4 is Tower = race 5). */
typedef struct {
    const char* meshName;
    int raceIndex;
} LevelMapping;

static const LevelMapping levelMap[] = {
    {"1", 1}, {"warmup", 1}, {"warm-up", 1},
    {"2", 2}, {"beginner", 2}, {"cascade", 2},
    {"3", 3}, {"intermediate", 3},
    {"4", 4}, {"dizzy", 4},
    {"5", 5}, {"tower", 5},
    {"6", 6}, {"up", 6},
    {"7", 7}, {"neon", 7}, {"dark", 7},
    {"8", 8}, {"expert", 8},
    {"9", 9}, {"odd", 9},
    {"10", 10}, {"toob", 10},
    {"11", 11}, {"wobbly", 11},
    {"12", 12}, {"glass", 12},
    {"13", 13}, {"sky", 13},
    {"14", 14}, {"master", 14},
    {"15", 15}, {"impossible", 15},
    {NULL, 0}
};

static int findRaceIndex(const char* levelName) {
    /* Numeric input: race NUMBER (1..15). Handles 1- and 2-digit numbers. */
    if (levelName[0] >= '1' && levelName[0] <= '9') {
        char* end = NULL;
        long num = strtol(levelName, &end, 10);
        if (end && *end == '\0' && num >= 1 && num <= 15) return (int)num;
    }
    for (int j = 0; levelMap[j].meshName; j++) {
        if (_stricmp(levelName, levelMap[j].meshName) == 0)
            return levelMap[j].raceIndex;
    }
    return -1;
}

/* ---- Music fade ---- */
static float g_musicOrigVolumes[MAX_MUSIC_CHANNELS];
static int g_musicChannelCount = 0;
static int g_musicFadeStarted = 0;

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
static volatile DWORD g_cooldownUntil = 0;
static volatile float g_whiteAlpha = 0.0f;
static volatile DWORD g_warpBall = 0;
static volatile int g_rumbleInit = 0;
static float g_origBallR = 1.0f, g_origBallG = 1.0f, g_origBallB = 1.0f;
static int g_colorSaved = 0;

static void startMusicFade(DWORD app) {
    if (!app) return;
    DWORD musicDev = *(DWORD*)(app + APP_MUSIC_DEVICE_PTR);
    if (!musicDev) return;

    DWORD chanListData = *(DWORD*)(musicDev + MUSIC_DEV_CHANNEL_LIST);
    int chanCount = *(int*)(musicDev + MUSIC_DEV_COUNT);

    if (chanCount > 0 && chanListData) {
        int count = (chanCount < MAX_MUSIC_CHANNELS) ? chanCount : MAX_MUSIC_CHANNELS;
        g_musicChannelCount = count;
        for (int i = 0; i < count; i++) {
            DWORD chan = *(DWORD*)(chanListData + i * 4);
            if (chan) g_musicOrigVolumes[i] = *(float*)(chan + MUSIC_CHAN_VOLUME);
            else g_musicOrigVolumes[i] = 0.0f;
        }
        for (int i = 0; i < count; i++) {
            DWORD chan = *(DWORD*)(chanListData + i * 4);
            if (chan) {
                *(BYTE*)(chan + MUSIC_CHAN_FADE_OUT) = 0;
                *(BYTE*)(chan + MUSIC_CHAN_FADE_IN) = 0;
                *(float*)(chan + MUSIC_CHAN_FADE_RATE) = 0.0f;
            }
        }
    }
    g_musicFadeStarted = 1;
}

static void updateMusicFade(DWORD app) {
    if (!g_musicFadeStarted) return;
    DWORD elapsed = getGameTime() - g_warpStartTime;
    float t = (float)elapsed / (float)MUSIC_FADE_MS;
    if (t > 1.0f) t = 1.0f;
    if (!app) return;
    DWORD musicDev = *(DWORD*)(app + APP_MUSIC_DEVICE_PTR);
    if (!musicDev) return;

    DWORD chanListData = *(DWORD*)(musicDev + MUSIC_DEV_CHANNEL_LIST);
    int chanCount = *(int*)(musicDev + MUSIC_DEV_COUNT);
    if (chanCount > 0 && chanListData) {
        int count = (chanCount < MAX_MUSIC_CHANNELS) ? chanCount : MAX_MUSIC_CHANNELS;
        for (int i = 0; i < count; i++) {
            DWORD chan = *(DWORD*)(chanListData + i * 4);
            if (chan) {
                float vol = g_musicOrigVolumes[i] * (1.0f - t);
                *(float*)(chan + MUSIC_CHAN_VOLUME) = vol;
                if (g_realBASS_ChannelSetAttributes) {
                    DWORD bassChan = *(DWORD*)(chan + MUSIC_CHAN_BASS_CHANNEL);
                    if (bassChan) g_realBASS_ChannelSetAttributes(bassChan, -1.0f, (int)(vol * 100.0f), -1);
                }
            }
        }
    }
}

static void restoreMusicFade(DWORD app) {
    if (!g_musicFadeStarted) return;
    if (!app) return;
    DWORD musicDev = *(DWORD*)(app + APP_MUSIC_DEVICE_PTR);
    if (!musicDev) return;

    DWORD chanListData = *(DWORD*)(musicDev + MUSIC_DEV_CHANNEL_LIST);
    int chanCount = *(int*)(musicDev + MUSIC_DEV_COUNT);
    if (chanCount > 0 && chanListData) {
        int count = (chanCount < MAX_MUSIC_CHANNELS) ? chanCount : MAX_MUSIC_CHANNELS;
        for (int i = 0; i < count; i++) {
            DWORD chan = *(DWORD*)(chanListData + i * 4);
            if (chan) {
                *(float*)(chan + MUSIC_CHAN_VOLUME) = g_musicOrigVolumes[i];
                *(BYTE*)(chan + MUSIC_CHAN_FADE_OUT) = 0;
                *(BYTE*)(chan + MUSIC_CHAN_FADE_IN) = 1;
                *(float*)(chan + MUSIC_CHAN_FADE_RATE) = 0.0f;
                if (g_realBASS_ChannelSetAttributes) {
                    DWORD bassChan = *(DWORD*)(chan + MUSIC_CHAN_BASS_CHANNEL);
                    if (bassChan) g_realBASS_ChannelSetAttributes(bassChan, -1.0f, (int)(g_musicOrigVolumes[i] * 100.0f), -1);
                }
            }
        }
    }
    g_musicFadeStarted = 0;
    LOGS("Music channels restored");
}

/* ================================================================
 * Warp state machine
 * ================================================================ */
static int isWarpNode(const char* nodeName) {
    return nodeName && _strnicmp(nodeName, "WARP(", 5) == 0;
}

static int parseWarpLevel(const char* nodeName, char* out, int outSize) {
    const char* start = strchr(nodeName, '(');
    if (!start) return 0;
    start++;
    const char* end = strchr(start, ')');
    if (!end) return 0;
    int len = (int)(end - start);
    if (len <= 0 || len >= outSize) return 0;
    strncpy(out, start, len);
    out[len] = '\0';
    return 1;
}

static void scanWarpNodes(DWORD board) {
    if (!g_warpEnabled) return;
    if (g_phase != PHASE_IDLE) return;
    if (getGameTime() < g_cooldownUntil) return;
    if (*(BYTE*)(board + BOARD_GOAL_REACHED) != 0) return;

    DWORD meshWorld = *(DWORD*)(board + BOARD_MESHWORLD_PTR);
    if (!meshWorld || IsBadReadPtr((void*)meshWorld, 0x1000)) return;

    DWORD section3 = *(DWORD*)(meshWorld + MW_SECTION3_PTR);
    if (!section3 || IsBadReadPtr((void*)section3, 0x1000)) return;

    int nodeCount = *(int*)(section3 + MW_NODELIST_COUNT);
    DWORD nodeData = *(DWORD*)(section3 + MW_NODELIST_DATA);
    if (nodeCount <= 0 || !nodeData || IsBadReadPtr((void*)nodeData, nodeCount * 4)) return;
    if (nodeCount > 256) nodeCount = 256;

    DWORD ballListData = *(DWORD*)(board + BOARD_BALL_LIST_DATA);
    if (!ballListData || IsBadReadPtr((void*)ballListData, 4)) return;
    DWORD ball = *(DWORD*)ballListData;
    if (!ball || IsBadReadPtr((void*)ball, 0xC60)) return;
    if (*(BYTE*)(ball + BALL_DEATH_PENDING) != 0) return;

    float bx = *(float*)(ball + BALL_POS_X);
    float by = *(float*)(ball + BALL_POS_Y);
    float bz = *(float*)(ball + BALL_POS_Z);

    for (int i = 0; i < nodeCount; i++) {
        DWORD entry = *(DWORD*)(nodeData + i * 4);
        if (!entry || IsBadReadPtr((void*)entry, 0x10)) continue;
        char* name = *(char**)entry;
        if (!name || IsBadReadPtr(name, 1)) continue;
        if (!isWarpNode(name)) continue;

        float nx = *(float*)(entry + NODE_X_OFFSET);
        float ny = *(float*)(entry + NODE_Y_OFFSET);
        float nz = *(float*)(entry + NODE_Z_OFFSET);

        float dx = bx - nx, dy = by - ny, dz = bz - nz;
        float distSq = dx * dx + dy * dy + dz * dz;

        if (distSq < WARP_TRIGGER_DIST_SQ) {
            char levelName[128];
            if (parseWarpLevel(name, levelName, sizeof(levelName))) {
                int raceIndex = findRaceIndex(levelName);
                LOG("WARP node '%s' -> raceIndex %d", name, raceIndex);
                if (raceIndex > 0) {
                    g_warpLevelIndex = raceIndex - 1;
                    g_phase = PHASE_RUMBLE;
                    g_rumbleInit = 0;
                    g_colorSaved = 0;
                    g_warpBall = ball;
                    DWORD now = getGameTime();
                    g_phaseStartTime = now;
                    g_warpStartTime = now;
                    g_whiteAlpha = 0.0f;
                    g_musicFadeStarted = 0;
                }
            }
            return;
        }
    }
}

static void call_app_start_practice_race(DWORD app, int idx) {
    DWORD addr = g_gameBase + RVA_APP_START_PRACTICE_RACE;
    __asm__ volatile (
        "push %1\n\t"
        "movl %0, %%ecx\n\t"
        "call *%2\n\t"
        : : "r"(app), "r"(idx), "r"(addr)
        : "eax", "ecx", "edx", "memory", "st", "st(1)", "st(2)", "st(3)",
          "st(4)", "st(5)", "st(6)", "st(7)"
    );
}

static void call_app_start_tournament_race(DWORD app, int idx) {
    DWORD addr = g_gameBase + RVA_APP_START_TOURNAMENT_RACE;
    __asm__ volatile (
        "push %1\n\t"
        "movl %0, %%ecx\n\t"
        "call *%2\n\t"
        : : "r"(app), "r"(idx), "r"(addr)
        : "eax", "ecx", "edx", "memory", "st", "st(1)", "st(2)", "st(3)",
          "st(4)", "st(5)", "st(6)", "st(7)"
    );
}

/* ================================================================
 * Ghost file helpers
 * ================================================================ */
static void init_ghost_dir(void) {
    char dir[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, dir);
    snprintf(g_ghostDir, MAX_PATH, "%s\\Ghosts\\", dir);
    CreateDirectoryA(g_ghostDir, NULL);
}

static void race_name_to_filename(const char* raceName, char* out, int outLen) {
    char base[128];
    strncpy(base, raceName, sizeof(base) - 1);
    base[sizeof(base) - 1] = '\0';

    int len = (int)strlen(base);
    if (len >= 5 && _stricmp(base + len - 5, " RACE") == 0)
        base[len - 5] = '\0';

    int newWord = 1;
    for (int i = 0; base[i]; i++) {
        char c = base[i];
        if (c == ' ' || c == '-' || c == '_') newWord = 1;
        else if (newWord) { if (c >= 'a' && c <= 'z') base[i] = c - 32; newWord = 0; }
        else { if (c >= 'A' && c <= 'Z') base[i] = c + 32; }
    }

    for (int i = 0; base[i]; i++) {
        char c = base[i];
        if (c == '\\' || c == '/' || c == ':' || c == '*' ||
            c == '?' || c == '"' || c == '<' || c == '>' || c == '|')
            base[i] = '_';
    }

    snprintf(out, outLen, "%s%s.ghost", g_ghostDir, base);
}

static void tw_race_name_to_filename(const char* raceName, char* out, int outLen) {
    char base[128];
    strncpy(base, raceName, sizeof(base) - 1);
    base[sizeof(base) - 1] = '\0';

    int len = (int)strlen(base);
    if (len >= 5 && _stricmp(base + len - 5, " RACE") == 0)
        base[len - 5] = '\0';

    int newWord = 1;
    for (int i = 0; base[i]; i++) {
        char c = base[i];
        if (c == ' ' || c == '-' || c == '_') newWord = 1;
        else if (newWord) { if (c >= 'a' && c <= 'z') base[i] = c - 32; newWord = 0; }
        else { if (c >= 'A' && c <= 'Z') base[i] = c + 32; }
    }

    for (int i = 0; base[i]; i++) {
        char c = base[i];
        if (c == '\\' || c == '/' || c == ':' || c == '*' ||
            c == '?' || c == '"' || c == '<' || c == '>' || c == '|')
            base[i] = '_';
    }

    snprintf(out, outLen, "%s", base);
}

static int get_saved_time(const char* raceName) {
    char path[MAX_PATH];
    race_name_to_filename(raceName, path, sizeof(path));
    HANDLE h = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return NO_TIME;
    int result = NO_TIME;
    DWORD magic, version, time, br;
    if (ReadFile(h, &magic, 4, &br, NULL) && magic == GHOST_MAGIC &&
        ReadFile(h, &version, 4, &br, NULL) && version == GHOST_VERSION &&
        ReadFile(h, &time, 4, &br, NULL)) {
        result = (int)time;
    }
    CloseHandle(h);
    return result;
}

static void save_ghost_for_race(const char* raceName, int time,
                              DWORD (*snaps)[SNAP_DWORDS], int count) {
    char path[MAX_PATH];
    race_name_to_filename(raceName, path, sizeof(path));
    char tmpPath[MAX_PATH];
    snprintf(tmpPath, sizeof(tmpPath), "%s.tmp", path);

    HANDLE h = CreateFileA(tmpPath, GENERIC_WRITE, 0, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) { LOG("ERROR: cannot create %s", tmpPath); return; }

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
        if (!WriteFile(h, snaps, totalBytes, &written, NULL) || written != totalBytes) ok = 0;
    }
    if (ok) FlushFileBuffers(h);
    CloseHandle(h);

    if (ok) {
        if (MoveFileExA(tmpPath, path, MOVEFILE_REPLACE_EXISTING)) {
            LOG("Saved %s (%d frames, time=%d)", path, count, time);
        } else {
            LOG("ERROR: MoveFileEx failed (err=%d)", GetLastError());
            DeleteFileA(tmpPath);
        }
    } else {
        DeleteFileA(tmpPath);
    }
}

static void save_segment_ghost(const char* raceName, int segment, int time,
                              DWORD (*snaps)[SNAP_DWORDS], int count, char bracket) {
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
    if (h == INVALID_HANDLE_VALUE) { LOG("ERROR: cannot create %s", tmpPath); return; }

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
        if (!WriteFile(h, snaps, totalBytes, &written, NULL) || written != totalBytes) ok = 0;
    }
    if (ok) FlushFileBuffers(h);
    CloseHandle(h);

    if (ok) {
        if (MoveFileExA(tmpPath, path, MOVEFILE_REPLACE_EXISTING)) {
            LOG("Saved segment %s (%d frames, time=%d)", path, count, time);
        } else {
            LOG("ERROR: MoveFileEx failed (err=%d)", GetLastError());
            DeleteFileA(tmpPath);
        }
    } else {
        DeleteFileA(tmpPath);
    }
}

static DWORD* load_segment_ghost(const char* raceName, int segment, int* outCount, int* outTime, char bracket) {
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
    DWORD* snaps = NULL;
    if (ReadFile(hf, &magic, 4, &br, NULL) && magic == GHOST_MAGIC &&
        ReadFile(hf, &version, 4, &br, NULL) && version == GHOST_VERSION &&
        ReadFile(hf, &time, 4, &br, NULL) &&
        ReadFile(hf, &frameCount, 4, &br, NULL) &&
        frameCount > 0 && frameCount < 200000) {

        DWORD totalBytes = frameCount * SNAP_BYTES;
        snaps = (DWORD*)malloc(totalBytes);
        if (snaps) {
            DWORD bytesRead = 0;
            if (ReadFile(hf, snaps, totalBytes, &bytesRead, NULL) && bytesRead == totalBytes) {
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

static int count_confirmed_segments(const char* raceName) {
    int count = 0;
    char base[128];
    tw_race_name_to_filename(raceName, base, sizeof(base));
    for (int i = 1; i <= MAX_SEGMENTS; i++) {
        char path[MAX_PATH];
        snprintf(path, sizeof(path), "%s%s(%d).ghost", g_ghostDir, base, i);
        DWORD attr = GetFileAttributesA(path);
        if (attr != INVALID_FILE_ATTRIBUTES) count++;
        else break;
    }
    return count;
}

static int get_confirmed_total_time(const char* raceName) {
    int total = 0, found = 0;
    char base[128];
    tw_race_name_to_filename(raceName, base, sizeof(base));
    for (int i = 1; i <= MAX_SEGMENTS; i++) {
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

static void rename_temp_to_confirmed(const char* raceName, int maxSeg) {
    char base[128];
    tw_race_name_to_filename(raceName, base, sizeof(base));
    for (int i = 1; i <= maxSeg; i++) {
        char src[MAX_PATH], dst[MAX_PATH];
        snprintf(src, sizeof(src), "%s%s[%d].ghost", g_ghostDir, base, i);
        snprintf(dst, sizeof(dst), "%s%s(%d).ghost", g_ghostDir, base, i);
        if (MoveFileExA(src, dst, MOVEFILE_REPLACE_EXISTING)) {
            LOG("Renamed %s -> %s", src, dst);
        }
    }
}

static void delete_temp_segments(const char* raceName, int maxSeg) {
    char base[128];
    tw_race_name_to_filename(raceName, base, sizeof(base));
    for (int i = 1; i <= maxSeg; i++) {
        char path[MAX_PATH];
        snprintf(path, sizeof(path), "%s%s[%d].ghost", g_ghostDir, base, i);
        if (DeleteFileA(path)) LOG("Deleted temp segment %s", path);
    }
}

static void delete_confirmed_segments(const char* raceName, int maxSeg) {
    char base[128];
    tw_race_name_to_filename(raceName, base, sizeof(base));
    for (int i = 1; i <= maxSeg; i++) {
        char path[MAX_PATH];
        snprintf(path, sizeof(path), "%s%s(%d).ghost", g_ghostDir, base, i);
        if (DeleteFileA(path)) LOG("Deleted confirmed segment %s", path);
    }
}

/* ================================================================
 * Ghost Saver (Time Trial ghost persistence)
 * ================================================================ */
static DWORD (*g_rawSnaps)[SNAP_DWORDS] = NULL;
static int g_rawCount = 0;
static int g_rawCapacity = 0;
static char g_currentRaceName[128] = "";
static char g_hookRaceName[128] = "";
static int g_recording = 0;
static int g_raceFinished = 0;
static int g_prevGoalFlag = 0;
static DWORD g_prevRecording = 0;
static DWORD g_savedOldPlayback = 0;

/* Time Warp multi-segment state (shared with check_race_state) */
static char g_twRaceName[128] = "";
static int g_segmentCounter = 0;
static int g_segmentCount = 0;
static int g_segmentTimes[MAX_SEGMENTS];
static BOOL g_isTimeWarpLevel = FALSE;

typedef void (__fastcall *AppStartPracticeRace_t)(void* app, void* edx, DWORD race_index);
static AppStartPracticeRace_t orig_AppStartPracticeRace = nullptr;

typedef void (__fastcall *AppStartTournamentRace_t)(void* app, void* edx, DWORD race_index);
static AppStartTournamentRace_t orig_AppStartTournamentRace = nullptr;

/* TT recording gate patch */
#define TT_RECORDING_NOP_ADDR   0x0041B690
#define TT_RECORDING_NOP_SIZE   7
static BYTE g_ttRecOrigBytes[TT_RECORDING_NOP_SIZE];
static int g_ttRecPatched = 0;

static void install_tt_recording_nop(void) {
    if (g_ttRecPatched) return;
    DWORD addr = TT_RECORDING_NOP_ADDR;
    DWORD oldProt;
    if (VirtualProtect((void*)addr, TT_RECORDING_NOP_SIZE, PAGE_EXECUTE_READWRITE, &oldProt)) {
        memcpy(g_ttRecOrigBytes, (void*)addr, TT_RECORDING_NOP_SIZE);
        memset((void*)addr, 0x90, TT_RECORDING_NOP_SIZE);
        VirtualProtect((void*)addr, TT_RECORDING_NOP_SIZE, oldProt, &oldProt);
        g_ttRecPatched = 1;
        LOGS("TT recording NOP patch installed at 0x41B690");
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
    LOGS("TT recording NOP patch restored");
}

// Forward declaration — get_race_name_by_index is defined later in the file.
static int get_race_name_by_index(DWORD race_index, char* out, int outLen);

static void create_tournament_recording_btt(DWORD app) {
    if (!app) return;
    DWORD bttRec = 0;
    if (!IsBadReadPtr((void*)(app + APP_BTT_RECORDING), 4))
        bttRec = *(DWORD*)(app + APP_BTT_RECORDING);
    if (bttRec) return;

    void* newBTT = Call<void*>(RVA_OPERATOR_NEW, (SIZE_T)BTT_SIZE);
    if (!newBTT) return;
    CallMethod<void>(RVA_BTT_CTOR, newBTT);
    DWORD vt = *(DWORD*)newBTT;
    if (vt == BTT_VTABLE_ADDR) {
        *(DWORD*)((char*)newBTT + BTT_BEST_TIME) = NO_TIME;
        // Name the tournament BTT so get_race_name() resolves a valid
        // race name — segment saving depends on g_twRaceName being set.
        DWORD profile = 0;
        if (!IsBadReadPtr((void*)(app + APP_PROFILE_PTR), 4))
            profile = *(DWORD*)(app + APP_PROFILE_PTR);
        DWORD raceIdx = 0;
        if (profile && !IsBadReadPtr((void*)(profile + PROFILE_RACE_INDEX), 4))
            raceIdx = *(DWORD*)(profile + PROFILE_RACE_INDEX);
        char raceName[128] = "";
        if (get_race_name_by_index(raceIdx, raceName, sizeof(raceName)) && raceName[0]) {
            strncpy((char*)newBTT + BTT_RACE_NAME, raceName, 64);
            *((char*)newBTT + BTT_RACE_NAME + 63) = '\0';
            LOG("Tournament recording BTT named '%s'", raceName);
        }
        *(DWORD*)(app + APP_BTT_RECORDING) = (DWORD)newBTT;
        LOG("Created Tournament recording BTT at 0x%X", (DWORD)newBTT);
    } else {
        LOG("Tournament BTT ctor failed vtable=0x%X", vt);
        Call<void>(RVA_GAME_FREE, newBTT);
    }
}

static void __fastcall hook_AppStartTournamentRace(void* app_ptr, void* edx, DWORD race_index) {
    (void)edx;
    DWORD app = (DWORD)app_ptr;
    LOG("HOOK: App_StartTournamentRace(race_index=%d)", race_index);
    if (orig_AppStartTournamentRace)
        orig_AppStartTournamentRace(app_ptr, edx, race_index);
    create_tournament_recording_btt(app);

    // Set the race name for tournament mode so check_race_state can
    // record segments (tournament = temporary [N] ghosts only).
    char raceName[128] = "";
    if (get_race_name_by_index(race_index, raceName, sizeof(raceName)) && raceName[0]) {
        strncpy(g_hookRaceName, raceName, sizeof(g_hookRaceName) - 1);
        g_hookRaceName[sizeof(g_hookRaceName) - 1] = '\0';
        LOG("HOOK: tournament race name '%s'", g_hookRaceName);
    }
}

static int is_time_trial_active(void) {
    DWORD app = get_app();
    if (!app) return 0;
    if (IsBadReadPtr((void*)(app + APP_PROFILE_PTR), 4)) return 0;
    DWORD profile = *(DWORD*)(app + APP_PROFILE_PTR);
    if (!profile || profile < 0x10000) return 0;
    if (IsBadReadPtr((void*)(profile + PROFILE_IS_PRACTICE), 1)) return 0;
    if (*(BYTE*)(profile + PROFILE_IS_PRACTICE) == 0) return 0;
    if (IsBadReadPtr((void*)(app + APP_PARTY_FLAG), 1)) return 0;
    if (*(BYTE*)(app + APP_PARTY_FLAG) != 0) return 0;
    return 1;
}

/* Tournament mode: like time trial, but PROFILE_IS_PRACTICE is 0 and the
 * game creates no recording BTT at App+0x90C. The mod creates and names
 * the BTT via create_tournament_recording_btt, and the TT-recording NOP
 * (0x41B690) lets the game record into it. Used to allow segment saving
 * in tournament (temporary [N] ghosts only). */
static int is_tournament_active(void) {
    DWORD app = get_app();
    if (!app) return 0;
    if (IsBadReadPtr((void*)(app + APP_PARTY_FLAG), 1)) return 0;
    if (*(BYTE*)(app + APP_PARTY_FLAG) != 0) return 0;
    if (IsBadReadPtr((void*)(app + APP_PROFILE_PTR), 4)) return 0;
    DWORD profile = *(DWORD*)(app + APP_PROFILE_PTR);
    if (!profile || profile < 0x10000) return 0;
    if (IsBadReadPtr((void*)(profile + PROFILE_IS_PRACTICE), 1)) return 0;
    /* practice=0 and we created a recording BTT -> tournament */
    if (*(BYTE*)(profile + PROFILE_IS_PRACTICE) != 0) return 0;
    DWORD bttRec = 0;
    if (!IsBadReadPtr((void*)(app + APP_BTT_RECORDING), 4))
        bttRec = *(DWORD*)(app + APP_BTT_RECORDING);
    if (!bttRec || bttRec < 0x10000) return 0;
    return 1;
}

static int is_time_trial_precheck(void) {
    DWORD app = get_app();
    if (!app) return 0;
    if (IsBadReadPtr((void*)(app + APP_PARTY_FLAG), 1)) return 0;
    if (*(BYTE*)(app + APP_PARTY_FLAG) != 0) return 0;
    return 1;
}

static int get_race_name(char* out, int outLen) {
    out[0] = '\0';
    DWORD app = get_app();
    if (!app) return 0;
    if (IsBadReadPtr((void*)(app + APP_BTT_RECORDING), 4)) return 0;
    DWORD btt = *(DWORD*)(app + APP_BTT_RECORDING);
    if (!btt || btt < 0x10000) return 0;
    if (IsBadReadPtr((void*)(btt + BTT_RACE_NAME), 1)) return 0;
    char* name = (char*)(btt + BTT_RACE_NAME);
    if (name[0] < 0x20 || name[0] > 0x7E) return 0;
    for (int i = 0; i < 64 && name[i]; i++) {
        if (name[i] < 0x20 || name[i] > 0x7E) { name[i] = '\0'; break; }
    }
    strncpy(out, name, outLen - 1);
    out[outLen - 1] = '\0';
    return 1;
}

static int get_race_name_table_count(void) {
    DWORD* nameTable = (DWORD*)RACE_NAME_TABLE;
    for (int i = 0; i < 64; i++) {
        if (IsBadReadPtr(nameTable + i, 4)) return i;
        DWORD namePtr = nameTable[i];
        if (!namePtr || namePtr < 0x400000) return i;
        if (IsBadReadPtr((void*)namePtr, 2)) return i;
        char c = *(char*)namePtr;
        if (c < 0x20 || c > 0x7E) return i;
    }
    return 64;
}

static int get_race_name_by_index(DWORD race_index, char* out, int outLen) {
    int tableCount = get_race_name_table_count();
    if ((int)race_index >= tableCount) return 0;
    DWORD* nameTable = (DWORD*)RACE_NAME_TABLE;
    char* name = (char*)nameTable[race_index];
    if (!name || (DWORD)name < 0x400000) return 0;
    if (IsBadReadPtr(name, 2)) return 0;
    if (name[0] < 0x20 || name[0] > 0x7E) return 0;
    strncpy(out, name, outLen - 1);
    out[outLen - 1] = '\0';
    return 1;
}

static void snaps_reserve(int needed) {
    if (needed <= g_rawCapacity) return;
    int newCap = g_rawCapacity ? g_rawCapacity : 5000;
    while (newCap < needed) newCap *= 2;
    DWORD (*newBuf)[SNAP_DWORDS] = (DWORD(*)[SNAP_DWORDS])realloc(g_rawSnaps, newCap * SNAP_BYTES);
    if (!newBuf) { LOG("ERROR: realloc failed for %d snaps", newCap); return; }
    g_rawSnaps = newBuf;
    g_rawCapacity = newCap;
}

static void snaps_reset(void) {
    if (g_rawSnaps) { free(g_rawSnaps); g_rawSnaps = NULL; }
    g_rawCount = 0;
    g_rawCapacity = 0;
}

static void inject_saved_ghost(const char* raceName) {
    DWORD app = get_app();
    if (!app) return;

    char path[MAX_PATH];
    race_name_to_filename(raceName, path, sizeof(path));

    HANDLE hf = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hf == INVALID_HANDLE_VALUE) { LOG("No ghost file for '%s'", raceName); return; }

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
            if (ReadFile(hf, savedSnaps, totalBytes, &br, NULL) && br == totalBytes) {
                savedCount = (int)frameCount;
            } else {
                free(savedSnaps);
                savedSnaps = NULL;
            }
        }
    }
    CloseHandle(hf);
    if (!savedSnaps || savedCount == 0) { if (savedSnaps) free(savedSnaps); return; }

    LOG("Loading ghost: '%s' time=%d frames=%d", raceName, savedTime, savedCount);

    void* btt = Call<void*>(RVA_OPERATOR_NEW, (SIZE_T)BTT_SIZE);
    if (!btt) { free(savedSnaps); return; }
    CallMethod<void>(RVA_BTT_CTOR, btt);
    DWORD vtable = *(DWORD*)btt;
    if (vtable != BTT_VTABLE_ADDR) {
        LOG("ERROR: BTT ctor failed — vtable=0x%X", vtable);
        free(savedSnaps);
        Call<void>(RVA_GAME_FREE, btt);
        return;
    }

    *(DWORD*)((char*)btt + BTT_BEST_TIME) = savedTime;
    char* bttName = (char*)((char*)btt + BTT_RACE_NAME);
    strncpy(bttName, raceName, 127);
    bttName[127] = '\0';

    DWORD* alist = (DWORD*)((char*)btt + 4);
    for (int i = 0; i < savedCount; i++) {
        DWORD* snap = (DWORD*)Call<void*>(RVA_OPERATOR_NEW, (SIZE_T)SNAP_SIZE);
        if (!snap) continue;
        memcpy(snap, savedSnaps[i], SNAP_BYTES);
        CallMethod<void>(RVA_ALIST_APPEND, alist, snap);
    }

    *(int*)((char*)btt + BTT_PLAYBACK_IDX) = 0;
    *(DWORD*)(app + APP_BTT_PLAYBACK) = (DWORD)btt;
    free(savedSnaps);
    LOG("Ghost injected into App+0x910 (btt=%p)", btt);
}

static void __fastcall hook_AppStartPracticeRace(void* app_ptr, void* edx, DWORD race_index) {
    (void)edx;
    DWORD app = (DWORD)app_ptr;
    LOG("HOOK: App_StartPracticeRace(race_index=%d)", race_index);

    // 1. Let the game do its full setup FIRST. App_StartPracticeRace
    //    (0x428C50) destroys any stale playback at App+0x910 on the
    //    fresh-start path (0x428CDB: unconditional vtable[0] call when
    //    App+0x90C was NULL at entry), compares best times, name-checks
    //    the playback against the new race, and creates a fresh recording
    //    BTT at App+0x90C. Injecting BEFORE it runs means the game's own
    //    cleanup destroys our ghost.
    orig_AppStartPracticeRace(app_ptr, edx, race_index);

    g_savedOldPlayback = 0;

    // 2. Inject our saved ghost AFTER the original returns. The game has
    //    finished all of its App+0x910 management, so nothing will destroy
    //    the injected ghost before the race starts.
    if (is_time_trial_precheck()) {
        char raceName[128] = "";
        if (get_race_name_by_index(race_index, raceName, sizeof(raceName)) && raceName[0]) {
            LOG("HOOK: post-inject for race '%s'", raceName);
            strncpy(g_hookRaceName, raceName, sizeof(g_hookRaceName) - 1);
            g_hookRaceName[sizeof(g_hookRaceName) - 1] = '\0';

            int savedTime = get_saved_time(raceName);
            if (savedTime != NO_TIME) {
                // Save whatever the game left at App+0x910 (a kept
                // matching-name ghost from a previous race) so we can free
                // it after overwriting — it becomes orphaned.
                if (!IsBadReadPtr((void*)(app + APP_BTT_PLAYBACK), 4)) {
                    DWORD existing = *(DWORD*)(app + APP_BTT_PLAYBACK);
                    if (existing && existing > 0x10000) g_savedOldPlayback = existing;
                }
                inject_saved_ghost(raceName);
            } else {
                LOG("No saved ghost for '%s', clearing playback", raceName);
                if (!IsBadReadPtr((void*)(app + APP_BTT_PLAYBACK), 4)) {
                    DWORD existing = *(DWORD*)(app + APP_BTT_PLAYBACK);
                    if (existing && existing > 0x10000) g_savedOldPlayback = existing;
                    *(DWORD*)(app + APP_BTT_PLAYBACK) = 0;
                }
            }
        }
    }

    // 3. If the game left a playback BTT behind that we replaced (or
    //    cleared), it is now orphaned — free it. If the game had already
    //    destroyed it (or our injection failed and left it in place), the
    //    pointer comparison keeps it alive.
    if (g_savedOldPlayback && g_savedOldPlayback > 0x10000) {
        DWORD currPlay = 0;
        if (!IsBadReadPtr((void*)(app + APP_BTT_PLAYBACK), 4))
            currPlay = *(DWORD*)(app + APP_BTT_PLAYBACK);
        if (currPlay != g_savedOldPlayback) {
            if (!IsBadReadPtr((void*)g_savedOldPlayback, 4)) {
                DWORD vt = *(DWORD*)g_savedOldPlayback;
                if (vt == BTT_VTABLE_ADDR) {
                    LOG("Destroying orphaned playback BTT at 0x%X", g_savedOldPlayback);
                    CallMethod<void>(RVA_BTT_DTOR, (void*)g_savedOldPlayback, (DWORD)1);
                }
            }
        }
        g_savedOldPlayback = 0;
    }
}

// Forward declaration: handle_tw_goal_touch is defined after
// check_race_state in this file (Time Warp multi-segment section).
static void handle_tw_goal_touch(void);

static void check_race_state(void) {
    DWORD app = get_app();
    if (!app) return;

    int tt = is_time_trial_active();
    int tourney = is_tournament_active();
    int recordingMode = tt || tourney;
    if (!recordingMode) {
        if (g_recording) {
            LOG("Left Time Trial/Tournament mode (was recording %d frames)", g_rawCount);
            g_recording = 0;
            g_raceFinished = 0;
            snaps_reset();
            g_prevGoalFlag = 0;
            g_currentRaceName[0] = '\0';
            g_hookRaceName[0] = '\0';
        }
        if (!IsBadReadPtr((void*)(app + APP_BTT_RECORDING), 4))
            g_prevRecording = *(DWORD*)(app + APP_BTT_RECORDING);
        return;
    }

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
            g_prevGoalFlag = *(BYTE*)(app + APP_TIMER_FINISHED);
            LOG("RACE START: '%s' (BTT=0x%X)%s", raceName, currRecording,
                tourney ? " [TOURNAMENT]" : "");
        } else {
            g_prevRecording = 0;
        }
    }

    if (g_recording && !g_raceFinished) {
        BYTE goalFlag = *(BYTE*)(app + APP_TIMER_FINISHED);
        if (goalFlag && !g_prevGoalFlag) {
            g_raceFinished = 1;
            int finishTime = NO_TIME;
            DWORD btt = 0;
            if (!IsBadReadPtr((void*)(app + APP_BTT_RECORDING), 4)) {
                btt = *(DWORD*)(app + APP_BTT_RECORDING);
                if (btt && btt > 0x10000 && !IsBadReadPtr((void*)(btt + BTT_BEST_TIME), 4))
                    finishTime = *(DWORD*)(btt + BTT_BEST_TIME);
            }
            LOG("GOAL! finishTime=%d", finishTime);

            // Time Warp multi-segment handling: save this segment and compare
            // against the confirmed best. Must run BEFORE the generic
            // save_ghost_for_race below so segment bookkeeping is current.
            if (g_isTimeWarpLevel && g_twRaceName[0])
                handle_tw_goal_touch();

            if (finishTime != NO_TIME && btt && g_currentRaceName[0]) {
                DWORD count = *(DWORD*)(btt + BTT_ALIST_COUNT);
                DWORD* data = *(DWORD**)(btt + BTT_LIST_ARRAY);
                if (count > 0 && count < 200000 && data && (DWORD)data > 0x10000 &&
                    !IsBadReadPtr(data, count * 4)) {
                    LOG("Reading %d frames from game recording", count);
                    snaps_reserve((int)count);
                    if (g_rawSnaps) {
                        g_rawCount = 0;
                        for (int i = 0; i < (int)count; i++) {
                            DWORD* snap = (DWORD*)data[i];
                            if (snap && (DWORD)snap > 0x10000 && !IsBadReadPtr(snap, SNAP_BYTES)) {
                                memcpy(g_rawSnaps[g_rawCount], snap, SNAP_BYTES);
                                g_rawCount++;
                            }
                        }
                        LOG("Read %d snapshots", g_rawCount);
                    }
                }

                if (g_rawCount > 0) {
                    /* Tournament mode: only temporary [N] segment ghosts
                     * are allowed (already saved via handle_tw_goal_touch).
                     * Do NOT write full race ghosts here — that would
                     * clobber the Time Trial confirmed records. */
                    if (!is_tournament_active()) {
                        save_ghost_for_race("Previous_Run", finishTime, g_rawSnaps, g_rawCount);
                        int existingTime = get_saved_time(g_currentRaceName);
                        if (existingTime == NO_TIME) {
                            LOG("No existing ghost — saving");
                            save_ghost_for_race(g_currentRaceName, finishTime, g_rawSnaps, g_rawCount);
                        } else if (finishTime < existingTime) {
                            LOG("New time %d < saved %d — overwriting", finishTime, existingTime);
                            save_ghost_for_race(g_currentRaceName, finishTime, g_rawSnaps, g_rawCount);
                        } else {
                            LOG("New time %d >= saved %d — discarding", finishTime, existingTime);
                        }
                    } else {
                        LOG("[tournament] Skipping full-race ghost save (temp segments only)");
                    }
                } else {
                    LOG("0 snapshots — likely stale goal flag, resetting");
                    g_raceFinished = 0;
                }
            }
        }
        g_prevGoalFlag = goalFlag;
    }
}

/* ================================================================
 * Time Warp multi-segment state
 * (globals g_twRaceName/g_segmentCounter/g_segmentCount/
 *  g_segmentTimes/g_isTimeWarpLevel are declared near the top of
 *  the ghost-saver section, shared with check_race_state)
 * ================================================================ */

static void save_warp_segment(void) {
    DWORD app = get_app();
    if (!app || !g_twRaceName[0]) return;

    DWORD btt = 0;
    if (!IsBadReadPtr((void*)(app + APP_BTT_RECORDING), 4))
        btt = *(DWORD*)(app + APP_BTT_RECORDING);
    if (!btt || btt < 0x10000 || IsBadReadPtr((void*)btt, 0x600)) {
        LOG("No BTT recording to save as segment");
        return;
    }

    int segTime = NO_TIME;
    if (!IsBadReadPtr((void*)(btt + BTT_RACE_TIME), 4))
        segTime = *(int*)((char*)btt + BTT_RACE_TIME);
    if (segTime == NO_TIME || segTime <= 0) {
        if (!IsBadReadPtr((void*)(btt + BTT_BEST_TIME), 4))
            segTime = *(int*)((char*)btt + BTT_BEST_TIME);
    }

    if (IsBadReadPtr((void*)(btt + BTT_ALIST_COUNT), 4)) return;
    DWORD count = *(DWORD*)(btt + BTT_ALIST_COUNT);
    if (count == 0 || count >= 200000) { LOG("Invalid snapshot count %d", count); return; }

    if (IsBadReadPtr((void*)(btt + BTT_LIST_ARRAY), 4)) return;
    DWORD* data = *(DWORD**)(btt + BTT_LIST_ARRAY);
    if (!data || (DWORD)data < 0x10000 || IsBadReadPtr(data, count * 4)) {
        LOG("Invalid BTT data array");
        return;
    }

    DWORD (*snaps)[SNAP_DWORDS] = (DWORD(*)[SNAP_DWORDS])malloc(count * SNAP_BYTES);
    if (!snaps) { LOG("malloc failed for segment save"); return; }

    int valid = 0;
    for (DWORD i = 0; i < count; i++) {
        DWORD* snap = (DWORD*)data[i];
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

static void handle_tw_goal_touch(void) {
    if (!g_isTimeWarpLevel || !g_twRaceName[0]) return;
    DWORD app = get_app();
    if (!app) return;

    save_warp_segment();

    int totalTime = 0;
    for (int i = 0; i < g_segmentCount && i < MAX_SEGMENTS; i++)
        totalTime += g_segmentTimes[i];

    if (is_time_trial_active()) {
        if (!IsBadReadPtr((void*)(app + APP_TIME_REMAINING), 4)) {
            int timerVal = *(int*)(app + APP_TIME_REMAINING);
            if (timerVal > 0 && timerVal < NO_TIME)
                totalTime = timerVal;
        }
    }

    LOG("Goal touch: segments=%d, totalTime=%d", g_segmentCount, totalTime);

    /* Tournament mode: temporary [N] ghosts ONLY. Never promote to
     * confirmed (N), never delete confirmed records — a tournament run
     * must not clobber the Time Trial best. Segments just accumulate
     * for the duration of the tournament session. */
    if (is_tournament_active()) {
        LOG("[tournament] Keeping %d temporary segment(s), no promotion");
        g_segmentCounter = 0;
        g_segmentCount = 0;
        memset(g_segmentTimes, 0, sizeof(g_segmentTimes));
        return;
    }

    int prevBest = get_confirmed_total_time(g_twRaceName);
    int prevSegCount = count_confirmed_segments(g_twRaceName);

    if (prevSegCount == 0) {
        rename_temp_to_confirmed(g_twRaceName, g_segmentCounter);
        LOG("First best: renamed %d segments", g_segmentCounter);
    } else if (totalTime < prevBest) {
        delete_confirmed_segments(g_twRaceName, prevSegCount);
        rename_temp_to_confirmed(g_twRaceName, g_segmentCounter);
        LOG("New best! totalTime=%d < prevBest=%d", totalTime, prevBest);
    } else {
        delete_temp_segments(g_twRaceName, g_segmentCounter);
        LOG("Not best: discarded (totalTime=%d >= prevBest=%d)", totalTime, prevBest);
    }

    g_segmentCounter = 0;
    g_segmentCount = 0;
    memset(g_segmentTimes, 0, sizeof(g_segmentTimes));
}

/* ================================================================
 * Ghost 1 (saved best / multi-segment) state
 * ================================================================ */
typedef struct {
    int currentSegment;
    int playbackIdx;
    int totalSegments;
    DWORD btt;
    char raceName[128];
    BOOL active;
} Ghost1State;
static Ghost1State g_ghost1 = {0, 0, 0, 0, "", FALSE};

static void ghost1_load_segment(const char* raceName, int segment, int playbackIdx, char bracket) {
    DWORD app = get_app();
    if (!app) return;

    int segTime = NO_TIME, segCount = 0;
    DWORD* snaps = load_segment_ghost(raceName, segment, &segCount, &segTime, bracket);
    if (!snaps || segCount == 0) { if (snaps) free(snaps); return; }

    if (g_ghost1.btt && g_ghost1.btt > 0x10000) {
        if (!IsBadReadPtr((void*)g_ghost1.btt, 4)) {
            DWORD vt = *(DWORD*)g_ghost1.btt;
            if (vt == BTT_VTABLE_ADDR)
                CallMethod<void>(RVA_BTT_DTOR, (void*)g_ghost1.btt, (DWORD)1);
            else
                Call<void>(RVA_GAME_FREE, (void*)g_ghost1.btt);
        }
        g_ghost1.btt = 0;
    }

    if (!IsBadReadPtr((void*)(app + APP_BTT_PLAYBACK), 4)) {
        DWORD existing = *(DWORD*)(app + APP_BTT_PLAYBACK);
        if (existing && existing > 0x10000 && existing != g_ghost1.btt) {
            if (!IsBadReadPtr((void*)existing, 4)) {
                DWORD vt = *(DWORD*)existing;
                if (vt == BTT_VTABLE_ADDR)
                    CallMethod<void>(RVA_BTT_DTOR, (void*)existing, (DWORD)1);
                else
                    Call<void>(RVA_GAME_FREE, (void*)existing);
            }
        }
    }

    void* btt = Call<void*>(RVA_OPERATOR_NEW, (SIZE_T)BTT_SIZE);
    if (!btt) { free(snaps); return; }
    CallMethod<void>(RVA_BTT_CTOR, btt);
    DWORD vtable = *(DWORD*)btt;
    if (vtable != BTT_VTABLE_ADDR) {
        LOG("BTT ctor vtable=0x%X (expected 0x%X)", vtable, BTT_VTABLE_ADDR);
        Call<void>(RVA_GAME_FREE, btt);
        free(snaps);
        return;
    }

    *(DWORD*)((char*)btt + BTT_BEST_TIME) = (DWORD)segTime;
    DWORD* alist = (DWORD*)((char*)btt + 0x04);
    for (int i = 0; i < segCount; i++) {
        DWORD* snap = (DWORD*)Call<void*>(RVA_OPERATOR_NEW, (SIZE_T)SNAP_SIZE);
        if (!snap) continue;
        memcpy(snap, snaps + i * SNAP_DWORDS, SNAP_BYTES);
        CallMethod<void>(RVA_ALIST_APPEND, alist, snap);
    }
    free(snaps);

    if (playbackIdx < 0) playbackIdx = 0;
    if (playbackIdx >= segCount) playbackIdx = 0;
    *(DWORD*)((char*)btt + BTT_PLAYBACK_IDX) = (DWORD)playbackIdx;

    *(DWORD*)(app + APP_BTT_PLAYBACK) = (DWORD)btt;
    g_ghost1.btt = (DWORD)btt;
    g_ghost1.currentSegment = segment;
    g_ghost1.playbackIdx = playbackIdx;
    LOG("Ghost1 loaded segment %d (%c): %d frames, time=%d, playIdx=%d",
        segment, bracket, segCount, segTime, playbackIdx);
}

static void ghost1_save_state(void) {
    DWORD app = get_app();
    if (!app) return;
    if (!IsBadReadPtr((void*)(app + APP_BTT_PLAYBACK), 4)) {
        DWORD btt = *(DWORD*)(app + APP_BTT_PLAYBACK);
        if (btt && btt > 0x10000 && !IsBadReadPtr((void*)(btt + BTT_PLAYBACK_IDX), 4)) {
            g_ghost1.playbackIdx = *(DWORD*)((char*)btt + BTT_PLAYBACK_IDX);
            g_ghost1.btt = btt;
            LOG("Ghost1 saved state: segment=%d, playIdx=%d, btt=0x%X",
                g_ghost1.currentSegment, g_ghost1.playbackIdx, btt);
        }
    }
}

static void ghost1_restore_after_warp(void) {
    if (!g_ghost1.active || !g_twRaceName[0]) return;
    int seg = g_ghost1.currentSegment;
    if (seg < 1) seg = 1;

    int segTime = 0, segCount = 0;
    DWORD* snaps = load_segment_ghost(g_twRaceName, seg, &segCount, &segTime, '(');
    char bracket = '(';
    if (!snaps) {
        snaps = load_segment_ghost(g_twRaceName, seg, &segCount, &segTime, '[');
        bracket = '[';
    }
    if (snaps) {
        free(snaps);
        ghost1_load_segment(g_twRaceName, seg, g_ghost1.playbackIdx, bracket);
    } else {
        LOG("No segment %d found to restore", seg);
    }
}

static void ghost1_check_advance(void) {
    if (!g_ghost1.active || !g_ghost1.btt || !g_twRaceName[0]) return;
    DWORD btt = g_ghost1.btt;
    if (IsBadReadPtr((void*)btt, 0x600)) return;

    DWORD playIdx = *(DWORD*)((char*)btt + BTT_PLAYBACK_IDX);
    DWORD count = *(DWORD*)((char*)btt + BTT_ALIST_COUNT);
    if (count == 0 || playIdx < count - 1) return;

    int nextSeg = g_ghost1.currentSegment + 1;
    if (nextSeg > g_ghost1.totalSegments) nextSeg = 1;

    LOG("Segment %d ended, advancing to %d", g_ghost1.currentSegment, nextSeg);

    int segTime = 0, segCount = 0;
    DWORD* snaps = load_segment_ghost(g_twRaceName, nextSeg, &segCount, &segTime, '(');
    if (!snaps) snaps = load_segment_ghost(g_twRaceName, nextSeg, &segCount, &segTime, '[');
    if (snaps) {
        free(snaps);
        char bracket = '(';
        /* Re-probe to determine which bracket actually succeeded. */
        {
            int pt = 0, pc = 0;
            DWORD* p = load_segment_ghost(g_twRaceName, nextSeg, &pc, &pt, '(');
            if (p) { free(p); bracket = '('; }
            else { bracket = '['; }
        }
        ghost1_load_segment(g_twRaceName, nextSeg, 0, bracket);
    } else {
        LOG("No segment %d to advance to", nextSeg);
    }
}

static void ghost1_init_for_tw(const char* raceName) {
    strncpy(g_ghost1.raceName, raceName, sizeof(g_ghost1.raceName) - 1);
    g_ghost1.raceName[sizeof(g_ghost1.raceName) - 1] = '\0';

    g_ghost1.totalSegments = count_confirmed_segments(raceName);
    g_ghost1.currentSegment = 1;
    g_ghost1.playbackIdx = 0;
    g_ghost1.active = TRUE;
    LOG("Ghost1 initialized for TW: %d confirmed segments", g_ghost1.totalSegments);
}

/* ================================================================
 * Ghost 2 (secondary purple replay ball)
 * ================================================================ */
typedef struct {
    DWORD ball;
    DWORD btt;
    DWORD* snapshots;
    int frameCount;
    int playbackIdx;
    BOOL active;
} Ghost2;
static Ghost2 g_ghost2 = {0, 0, NULL, 0, 0, FALSE};

static DWORD (*g_ghost2Capture)[SNAP_DWORDS] = NULL;
static int g_ghost2CaptureCount = 0;
static BOOL g_ghost2Pending = FALSE;

static void ghost2_destroy(void) {
    if (g_ghost2.ball) {
        DWORD ball = g_ghost2.ball;
        if (!IsBadReadPtr((void*)ball, 0x100)) {
            DWORD app = get_app();
            if (app) {
                DWORD profile = *(DWORD*)(app + APP_PROFILE_PTR);
                if (profile && !IsBadReadPtr((void*)profile, 0x100)) {
                    DWORD board = *(DWORD*)(profile + PROFILE_BOARD_PTR);
                    if (board && !IsBadReadPtr((void*)board, 0x4000)) {
                        DWORD* ballList = (DWORD*)((char*)board + BOARD_BALL_LIST);
                        CallMethod<void>(RVA_ALIST_REMOVE, ballList, (void*)ball);
                        LOG("Ghost2 ball removed from AthenaList at 0x%X", ball);
                    }
                }
            }
            DWORD vt = *(DWORD*)ball;
            if (vt && !IsBadReadPtr((void*)vt, 4)) {
                CallMethod<void>(RVA_BALL_DTOR, (void*)ball, (DWORD)1);
                LOG("Ghost2 ball destroyed at 0x%X", ball);
            }
        }
        g_ghost2.ball = 0;
    }

    if (g_ghost2.btt) {
        DWORD btt = g_ghost2.btt;
        if (!IsBadReadPtr((void*)btt, 0x100)) {
            DWORD vt = *(DWORD*)btt;
            if (vt == BTT_VTABLE_ADDR)
                CallMethod<void>(RVA_BTT_DTOR, (void*)btt, (DWORD)1);
            else
                Call<void>(RVA_GAME_FREE, (void*)btt);
        }
        g_ghost2.btt = 0;
    }

    if (g_ghost2.snapshots) { free(g_ghost2.snapshots); g_ghost2.snapshots = NULL; }
    g_ghost2.frameCount = 0;
    g_ghost2.playbackIdx = 0;
    g_ghost2.active = FALSE;
}

static void ghost2_capture(void) {
    DWORD app = get_app();
    if (!app) return;

    DWORD btt = 0;
    if (!IsBadReadPtr((void*)(app + APP_BTT_RECORDING), 4))
        btt = *(DWORD*)(app + APP_BTT_RECORDING);
    if (!btt || btt < 0x10000 || IsBadReadPtr((void*)btt, 0x100)) {
        LOG("No BTT recording at App+0x90C to capture");
        return;
    }

    if (IsBadReadPtr((void*)(btt + BTT_ALIST_COUNT), 4)) return;
    DWORD count = *(DWORD*)(btt + BTT_ALIST_COUNT);
    if (count == 0 || count >= 200000) { LOG("Invalid snapshot count %d", count); return; }

    if (IsBadReadPtr((void*)(btt + BTT_LIST_ARRAY), 4)) return;
    DWORD* data = *(DWORD**)(btt + BTT_LIST_ARRAY);
    if (!data || (DWORD)data < 0x10000 || IsBadReadPtr(data, count * 4)) {
        LOG("Invalid BTT data array");
        return;
    }

    DWORD (*buf)[SNAP_DWORDS] = (DWORD(*)[SNAP_DWORDS])malloc(count * SNAP_BYTES);
    if (!buf) { LOG("malloc failed for capture buffer"); return; }

    int valid = 0;
    for (DWORD i = 0; i < count; i++) {
        DWORD* snap = (DWORD*)data[i];
        if (snap && (DWORD)snap > 0x10000 && !IsBadReadPtr(snap, SNAP_BYTES)) {
            memcpy(buf[valid], snap, SNAP_BYTES);
            valid++;
        }
    }

    if (g_ghost2Capture) free(g_ghost2Capture);
    g_ghost2Capture = buf;
    g_ghost2CaptureCount = valid;
    g_ghost2Pending = TRUE;
    LOG("Ghost2 captured %d snapshots from BTT at 0x%X", valid, btt);
}

static void ghost2_create(DWORD board, DWORD* snaps, int count) {
    if (!board || !snaps || count <= 0) return;

    void* btt = Call<void*>(RVA_OPERATOR_NEW, (SIZE_T)BTT_SIZE);
    if (!btt) { LOG("Ghost2: operator_new failed for BTT"); return; }
    CallMethod<void>(RVA_BTT_CTOR, btt);
    DWORD vtable = *(DWORD*)btt;
    if (vtable != BTT_VTABLE_ADDR) {
        LOG("Ghost2: BTT ctor vtable=0x%X", vtable);
        Call<void>(RVA_GAME_FREE, btt);
        return;
    }

    *(DWORD*)((char*)btt + BTT_BEST_TIME) = NO_TIME;
    *(DWORD*)((char*)btt + BTT_PLAYBACK_IDX) = 0;
    DWORD* alist = (DWORD*)((char*)btt + 0x04);
    for (int i = 0; i < count; i++) {
        DWORD* snap = (DWORD*)Call<void*>(RVA_OPERATOR_NEW, (SIZE_T)SNAP_SIZE);
        if (!snap) continue;
        memcpy(snap, snaps + i * SNAP_DWORDS, SNAP_BYTES);
        CallMethod<void>(RVA_ALIST_APPEND, alist, snap);
    }

    DWORD ballAddr = (DWORD)Call<void*>(RVA_OPERATOR_NEW, (SIZE_T)0xC60);
    if (!ballAddr) {
        LOG("Ghost2: operator_new failed for ball");
        CallMethod<void>(RVA_BTT_DTOR, btt, (DWORD)1);
        return;
    }
    CallMethod<void>(RVA_BALL_CTOR, (void*)ballAddr, board);
    DWORD ballVtable = *(DWORD*)ballAddr;
    if (!ballVtable || IsBadReadPtr((void*)ballVtable, 4)) {
        LOG("Ghost2: Ball_ctor failed vtable=0x%X", ballVtable);
        Call<void>(RVA_GAME_FREE, (void*)ballAddr);
        CallMethod<void>(RVA_BTT_DTOR, btt, (DWORD)1);
        return;
    }

    DWORD func1 = *(DWORD*)(ballVtable + 0x04);
    if (func1 && !IsBadReadPtr((void*)func1, 4)) {
        __asm__ volatile (
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
    *(float*)(ballAddr + BALL_MAX_SPEED) = 1000.0f;
    *(BYTE*)(ballAddr + BALL_RESPAWN_FLAG) = 0;
    *(float*)(ballAddr + BALL_ALPHA) = 0.45f;
    *(float*)(ballAddr + BALL_POS_X) = 0.0f;
    *(float*)(ballAddr + BALL_POS_Y) = 0.0f;
    *(float*)(ballAddr + BALL_POS_Z) = 0.0f;
    *(float*)(ballAddr + BALL_COLOR_R) = TARGET_PURPLE_R;
    *(float*)(ballAddr + BALL_COLOR_G) = TARGET_PURPLE_G;
    *(float*)(ballAddr + BALL_COLOR_B) = TARGET_PURPLE_B;

    DWORD* ballList = (DWORD*)((char*)board + BOARD_BALL_LIST);
    CallMethod<void>(RVA_ALIST_APPEND, ballList, (void*)ballAddr);

    g_ghost2.ball = ballAddr;
    g_ghost2.btt = (DWORD)btt;
    g_ghost2.snapshots = (DWORD*)malloc(count * SNAP_BYTES);
    if (g_ghost2.snapshots) {
        memcpy(g_ghost2.snapshots, snaps, count * SNAP_BYTES);
        g_ghost2.frameCount = count;
        g_ghost2.playbackIdx = 0;
        g_ghost2.active = TRUE;
        LOG("Ghost2 created: ball=0x%X btt=0x%X frames=%d", ballAddr, (DWORD)btt, count);
    } else {
        g_ghost2.frameCount = 0;
        g_ghost2.active = FALSE;
    }

    /* Clear pending flag: creation is fully complete */
    g_ghost2Pending = FALSE;
    if (g_ghost2Capture) { free(g_ghost2Capture); g_ghost2Capture = NULL; }
    g_ghost2CaptureCount = 0;
}

static void ghost2_playback(void) {
    if (!g_ghost2.active || !g_ghost2.ball || !g_ghost2.snapshots) return;
    int idx = g_ghost2.playbackIdx;
    if (idx >= g_ghost2.frameCount) idx = g_ghost2.frameCount - 1;

    DWORD* snap = g_ghost2.snapshots + idx * SNAP_DWORDS;
    DWORD ball = g_ghost2.ball;
    if (IsBadReadPtr((void*)ball, 0xC60)) return;

    *(float*)(ball + BALL_POS_X) = *(float*)(snap + 0);
    *(float*)(ball + BALL_POS_Y) = *(float*)(snap + 1);
    *(float*)(ball + BALL_POS_Z) = *(float*)(snap + 2);
    *(float*)(ball + 0x190) = *(float*)(snap + 3);
    *(float*)(ball + 0x194) = *(float*)(snap + 4);
    *(float*)(ball + 0x150) = *(float*)(snap + 5);
    *(float*)(ball + 0x748) = *(float*)(snap + 6);
    *(float*)(ball + 0x74C) = *(float*)(snap + 7);
    *(float*)(ball + 0x750) = *(float*)(snap + 8);
    *(float*)(ball + BALL_RADIUS) = *(float*)(snap + 9);

    /* Zero velocity/force accumulators so physics doesn't fight position writes */
    *(float*)(ball + 0x170) = 0.0f;
    *(float*)(ball + 0x174) = 0.0f;
    *(float*)(ball + 0x178) = 0.0f;

    *(float*)(ball + BALL_COLOR_R) = TARGET_PURPLE_R;
    *(float*)(ball + BALL_COLOR_G) = TARGET_PURPLE_G;
    *(float*)(ball + BALL_COLOR_B) = TARGET_PURPLE_B;
    *(float*)(ball + BALL_ALPHA) = 0.45f;

    if (g_ghost2.playbackIdx < g_ghost2.frameCount - 1)
        g_ghost2.playbackIdx++;
}

static void ghost2_check_board_change(DWORD board) {
    if (!g_ghost2.active) {
        if (g_ghost2Pending && board && !IsBadReadPtr((void*)board, 0x4000)) {
            DWORD app = get_app();
            if (app) {
                BYTE partyMode = 0;
                if (!IsBadReadPtr((void*)(app + APP_PARTY_FLAG), 1))
                    partyMode = *(BYTE*)(app + APP_PARTY_FLAG);
                if (partyMode == 0) {
                    ghost2_create(board, (DWORD*)g_ghost2Capture, g_ghost2CaptureCount);
                } else {
                    LOG("Party mode — skipping Ghost2 creation");
                    if (g_ghost2Capture) { free(g_ghost2Capture); g_ghost2Capture = NULL; }
                    g_ghost2Pending = FALSE;
                }
            }
        }
        return;
    }
    if (!board) {
        LOG("Ghost2: Board lost — destroying");
        ghost2_destroy();
    }
}

/* ================================================================
 * Ghost Event (E:GHOST filename collision)
 * ================================================================ */
static DWORD g_loadedBTT = 0;
static BOOL g_ghostActive = FALSE;
static BOOL g_ghostBallCreated = FALSE;
static BOOL g_ghostFromEvent = FALSE;
static BOOL g_needManualAdvance = FALSE;
static char g_pendingGhostFile[256] = "";

static int load_ghost_file(const char* filename, DWORD** outSnapshots, DWORD* outCount, DWORD* outFinishTime) {
    char path[MAX_PATH];
    HMODULE hSelf = NULL;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                     | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                     (LPCSTR)&load_ghost_file, &hSelf);
    if (hSelf && GetModuleFileNameA(hSelf, path, MAX_PATH)) {
        char* p = strrchr(path, '\\');
        if (p) {
            strcpy(p + 1, "Ghosts\\");
            strncat(path, filename, MAX_PATH - strlen(path) - 1);
            size_t plen = strlen(path);
            if (plen < MAX_PATH - 7 && (plen < 6 || _stricmp(path + plen - 6, ".ghost") != 0))
                strcat(path, ".ghost");
        } else {
            snprintf(path, MAX_PATH, "Ghosts\\%s.ghost", filename);
        }
    } else {
        snprintf(path, MAX_PATH, "Ghosts\\%s.ghost", filename);
    }

    LOG("Loading ghost file: %s", path);
    HANDLE hFile = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        LOG("Failed to open: %s (err=%d)", path, GetLastError());
        return 0;
    }

    DWORD bytesRead;
    GhostFileHeader header;
    if (!ReadFile(hFile, &header, sizeof(header), &bytesRead, NULL) || bytesRead != sizeof(header)) {
        CloseHandle(hFile);
        return 0;
    }

    if (header.magic != GHOST_MAGIC) {
        LOG("No magic, legacy format (magic=0x%X) — seeking back 8 bytes", header.magic);
        DWORD count = header.magic;
        DWORD time = header.version;
        header.frameCount = count;
        header.time = time;
        SetFilePointer(hFile, 8, NULL, FILE_BEGIN);
    } else {
        LOG("Magic OK: v%d time=%d frames=%d", header.version, header.time, header.frameCount);
    }

    if (header.frameCount == 0 || header.frameCount > 50000) {
        LOG("Invalid frame count: %d", header.frameCount);
        CloseHandle(hFile);
        return 0;
    }

    DWORD dataSize = header.frameCount * SNAP_SIZE;
    DWORD* snapshots = (DWORD*)malloc(dataSize);
    if (!snapshots) { CloseHandle(hFile); return 0; }

    if (!ReadFile(hFile, snapshots, dataSize, &bytesRead, NULL) || bytesRead != dataSize) {
        free(snapshots);
        CloseHandle(hFile);
        return 0;
    }

    CloseHandle(hFile);
    *outSnapshots = snapshots;
    *outCount = header.frameCount;
    *outFinishTime = header.time;
    LOG("Ghost loaded: %d frames, time=%d", header.frameCount, header.time);
    return 1;
}

static DWORD create_btt_from_ghost(DWORD* snapshots, DWORD count, DWORD finishTime, const char* raceName) {
    DWORD bttAddr = (DWORD)Call<void*>(RVA_OPERATOR_NEW, (SIZE_T)BTT_SIZE);
    if (!bttAddr) { LOG("operator_new failed for BTT"); return 0; }
    CallMethod<void>(RVA_BTT_CTOR, (void*)bttAddr);

    DWORD vtable = *(DWORD*)bttAddr;
    if (vtable != BTT_VTABLE_ADDR) {
        LOG("ERROR: BTT ctor failed — vtable=0x%X (expected 0x%X)", vtable, BTT_VTABLE_ADDR);
        Call<void>(RVA_GAME_FREE, (void*)bttAddr);
        return 0;
    }

    *(DWORD*)(bttAddr + BTT_BEST_TIME) = finishTime;
    if (raceName && raceName[0]) {
        char* bttName = (char*)(bttAddr + BTT_RACE_NAME);
        strncpy(bttName, raceName, 127);
        bttName[127] = '\0';
    }

    DWORD alist = bttAddr + 0x04;
    for (DWORD i = 0; i < count; i++) {
        DWORD* snap = (DWORD*)Call<void*>(RVA_OPERATOR_NEW, (SIZE_T)SNAP_SIZE);
        if (!snap) { LOG("ERROR: operator_new failed for snapshot %d", i); continue; }
        memcpy(snap, (BYTE*)snapshots + i * SNAP_SIZE, SNAP_SIZE);
        CallMethod<void>(RVA_ALIST_APPEND, (DWORD*)alist, snap);
    }
    *(DWORD*)(bttAddr + BTT_PLAYBACK_IDX) = 0;
    LOG("BTT created at 0x%08X: count=%d, time=%d", bttAddr, count, finishTime);
    return bttAddr;
}

static DWORD create_ghost_ball(DWORD board) {
    DWORD ballAddr = (DWORD)Call<void*>(RVA_OPERATOR_NEW, (SIZE_T)0xC60);
    if (!ballAddr) { LOG("operator_new failed for ghost ball"); return 0; }
    CallMethod<void>(RVA_BALL_CTOR, (void*)ballAddr, board);

    DWORD ballVtable = *(DWORD*)ballAddr;
    if (!ballVtable || IsBadReadPtr((void*)ballVtable, 4)) {
        LOG("ERROR: Ball_ctor failed — vtable=0x%X", ballVtable);
        return 0;
    }

    DWORD func1 = *(DWORD*)(ballVtable + 0x04);
    if (func1 && !IsBadReadPtr((void*)func1, 4)) {
        __asm__ volatile (
            "movl %0, %%ecx\n\t"
            "call *%1\n\t"
            : : "r"(ballAddr), "r"(func1)
            : "eax", "ecx", "edx", "memory"
        );
    }

    *(float*)(ballAddr + BALL_COLOR_R) = 1.0f;
    *(float*)(ballAddr + BALL_COLOR_G) = 1.0f;
    *(float*)(ballAddr + BALL_COLOR_B) = 0.35f;
    *(DWORD*)(ballAddr + BALL_PLAYER_ID) = 0xFFFFFFFF;
    *(float*)(ballAddr + BALL_GRAVITY) = 0.5f;
    *(float*)(ballAddr + BALL_GRAVITY_SCALE) = 0.1f;
    *(float*)(ballAddr + BALL_RADIUS) = 26.0f;
    *(float*)(ballAddr + BALL_MASS) = 0.8f;
    *(float*)(ballAddr + BALL_MAX_SPEED) = 1000.0f;
    *(BYTE*)(ballAddr + BALL_RESPAWN_FLAG) = 0;
    *(float*)(ballAddr + BALL_ALPHA) = 0.45f;
    *(float*)(ballAddr + BALL_POS_X) = 0.0f;
    *(float*)(ballAddr + BALL_POS_Y) = 0.0f;
    *(float*)(ballAddr + BALL_POS_Z) = 0.0f;

    *(DWORD*)(board + BOARD_GHOST_BALL) = ballAddr;
    LOG("Ghost ball created at 0x%08X in board 0x%08X", ballAddr, board);
    return ballAddr;
}

static void cleanup_previous_ghost(DWORD app) {
    DWORD board = get_board(app);

    if (g_ghostBallCreated && board) {
        DWORD ghostBall = *(DWORD*)(board + BOARD_GHOST_BALL);
        if (ghostBall && !IsBadReadPtr((void*)ghostBall, 0x100)) {
            CallMethod<void>(RVA_BALL_DTOR, (void*)ghostBall, (DWORD)1);
            *(DWORD*)(board + BOARD_GHOST_BALL) = 0;
            LOG("Old ghost ball destroyed at 0x%08X", ghostBall);
        }
        g_ghostBallCreated = FALSE;
    }

    if (g_loadedBTT && app) {
        if (*(DWORD*)(app + APP_BTT_PLAYBACK) == g_loadedBTT) {
            *(DWORD*)(app + APP_BTT_PLAYBACK) = 0;
            LOG("Cleared App+0x910 (playback) — was BTT 0x%08X", g_loadedBTT);
        }
    }

    if (g_loadedBTT) {
        if (!IsBadReadPtr((void*)g_loadedBTT, 0x100)) {
            DWORD vt = *(DWORD*)g_loadedBTT;
            if (vt == BTT_VTABLE_ADDR)
                CallMethod<void>(RVA_BTT_DTOR, (void*)g_loadedBTT, (DWORD)1);
            else
                Call<void>(RVA_GAME_FREE, (void*)g_loadedBTT);
        }
        g_loadedBTT = 0;
    }

    g_ghostActive = FALSE;
    g_ghostFromEvent = FALSE;
    g_needManualAdvance = FALSE;
}

static void ghost_event_frame(DWORD app, DWORD board) {
    if (!app || !board) return;

    if (g_pendingGhostFile[0]) {
        char filename[256];
        strncpy(filename, g_pendingGhostFile, sizeof(filename) - 1);
        filename[sizeof(filename) - 1] = '\0';
        g_pendingGhostFile[0] = '\0';

        if (g_ghostActive || g_loadedBTT) cleanup_previous_ghost(app);

        DWORD* snapshots = NULL;
        DWORD count = 0;
        DWORD finishTime = 0;
        if (!load_ghost_file(filename, &snapshots, &count, &finishTime)) {
            LOG("Failed to load ghost file: %s", filename);
            return;
        }

        DWORD newBTT = create_btt_from_ghost(snapshots, count, finishTime, filename);
        if (!newBTT) {
            free(snapshots);
            LOG("Failed to create BTT");
            return;
        }

        *(DWORD*)(app + APP_BTT_PLAYBACK) = newBTT;
        g_loadedBTT = newBTT;

        DWORD ghostBall = *(DWORD*)(board + BOARD_GHOST_BALL);
        if (!ghostBall || IsBadReadPtr((void*)ghostBall, 0x100)) {
            ghostBall = create_ghost_ball(board);
            if (ghostBall) g_ghostBallCreated = TRUE;
            else LOG("Failed to create ghost ball");
        } else {
            g_ghostBallCreated = FALSE;
            LOG("Ghost ball already exists, reusing: 0x%08X", ghostBall);
        }

        *(DWORD*)(newBTT + BTT_PLAYBACK_IDX) = 0;
        g_ghostActive = TRUE;
        g_ghostFromEvent = TRUE;

        BYTE partyMode = 0;
        if (!IsBadReadPtr((void*)(app + APP_PARTY_FLAG), 1))
            partyMode = *(BYTE*)(app + APP_PARTY_FLAG);
        g_needManualAdvance = (partyMode != 0);
        LOG("Ghost playback started: BTT=0x%08X, ball=0x%08X, manualAdvance=%d",
            newBTT, ghostBall, g_needManualAdvance);
        free(snapshots);
    }

    if (g_ghostActive && g_loadedBTT) {
        DWORD ghostBall = *(DWORD*)(board + BOARD_GHOST_BALL);
        if (!ghostBall || IsBadReadPtr((void*)ghostBall, 0x100)) {
            LOG("Ghost ball lost — cleaning up");
            cleanup_previous_ghost(app);
            return;
        }

        if (g_needManualAdvance) {
            DWORD playIdx = *(DWORD*)(g_loadedBTT + BTT_PLAYBACK_IDX);
            DWORD count = *(DWORD*)(g_loadedBTT + BTT_ALIST_COUNT);
            if (count > 0 && playIdx < count - 1)
                *(DWORD*)(g_loadedBTT + BTT_PLAYBACK_IDX) = playIdx + 1;
        }

        if (g_ghostFromEvent) {
            DWORD playIdx = *(DWORD*)(g_loadedBTT + BTT_PLAYBACK_IDX);
            DWORD count = *(DWORD*)(g_loadedBTT + BTT_ALIST_COUNT);
            if (count > 0 && playIdx >= count - 1) {
                LOG("Ghost playback finished (idx=%d/%d) — cleaning up for re-trigger", playIdx, count);
                cleanup_previous_ghost(app);
            }
        }
    }
}

/* ================================================================
 * Ghost triggers (GT: proximity events)
 * ================================================================ */
typedef struct {
    float pos_x, pos_y, pos_z;
    float radius, radius_sq;
    char name[64];
    int inside_ghost2;
    int cooldown2;
} Trigger;
static Trigger g_triggers[MAX_TRIGGERS];
static int g_triggerCount = 0;
static DWORD g_triggerBoard = 0;

static void scan_s1_triggers(DWORD board) {
    g_triggerCount = 0;
    if (!board || IsBadReadPtr((void*)board, 0x4000)) return;

    DWORD level = *(DWORD*)(board + BOARD_MESHWORLD_PTR);
    if (!level || IsBadReadPtr((void*)level, 0x1000)) return;

    DWORD mwData = *(DWORD*)(level + MW_SECTION3_PTR);
    if (!mwData || IsBadReadPtr((void*)mwData, 0x1000)) return;

    int s1Count = *(int*)(mwData + MW_S1_COUNT);
    DWORD s1Array = *(DWORD*)(mwData + MW_S1_ARRAY);
    if (s1Count <= 0 || s1Count > 1000 || !s1Array || IsBadReadPtr((void*)s1Array, s1Count * 4)) return;

    for (int i = 0; i < s1Count && g_triggerCount < MAX_TRIGGERS; i++) {
        DWORD entry = *(DWORD*)(s1Array + i * 4);
        if (!entry || IsBadReadPtr((void*)entry, 28)) continue;
        char* name = *(char**)entry;
        if (!name || IsBadReadPtr(name, 4)) continue;

        if (_strnicmp(name, TRIGGER_PREFIX, 3) == 0) {
            Trigger* t = &g_triggers[g_triggerCount];
            const char* eventName = name + 3;
            strncpy(t->name, eventName, sizeof(t->name) - 1);
            t->name[sizeof(t->name) - 1] = '\0';
            t->pos_x = *(float*)(entry + 4);
            t->pos_y = *(float*)(entry + 8);
            t->pos_z = *(float*)(entry + 12);
            t->radius = *(float*)(entry + 20);
            if (t->radius <= 0.0f || t->radius > 5000.0f) t->radius = DEFAULT_TRIGGER_RADIUS;
            t->radius_sq = t->radius * t->radius;
            t->inside_ghost2 = 0;
            t->cooldown2 = 0;
            g_triggerCount++;
        }
    }
}

static void fire_trigger_event(Trigger* t) {
    LOG("TRIGGER FIRED: '%s' on ghost2 (purple Time Warp ghost)", t->name);
    if (_stricmp(t->name, "RESET") == 0) {
        if (g_ghost2.active) g_ghost2.playbackIdx = 0;
    } else if (_stricmp(t->name, "STOP") == 0) {
        if (g_ghost2.btt) g_ghost2.active = FALSE;
    } else if (_stricmp(t->name, "START") == 0) {
        if (g_ghost2.btt) g_ghost2.active = TRUE;
    }
}

static void check_ghost_triggers(DWORD board) {
    if (g_triggerCount == 0) return;

    /* Option 1 (purple-only): GT: triggers fire ONLY for ghost2 — the
     * Time Warp purple ghost. The ghost1 (yellow replay ball) proximity
     * loop was intentionally removed so the normal ghost can never
     * activate GT: events. */
    if (g_ghost2.active && g_ghost2.ball && !IsBadReadPtr((void*)g_ghost2.ball, 0x200)) {
        float bx = *(float*)(g_ghost2.ball + BALL_POS_X);
        float by = *(float*)(g_ghost2.ball + BALL_POS_Y);
        float bz = *(float*)(g_ghost2.ball + BALL_POS_Z);
        for (int i = 0; i < g_triggerCount; i++) {
            Trigger* t = &g_triggers[i];
            if (t->cooldown2 > 0) t->cooldown2--;
            float dx = bx - t->pos_x, dy = by - t->pos_y, dz = bz - t->pos_z;
            float dist_sq = dx * dx + dy * dy + dz * dz;
            int wasInside = t->inside_ghost2;
            t->inside_ghost2 = (dist_sq < t->radius_sq) ? 1 : 0;
            if (!wasInside && t->inside_ghost2 && t->cooldown2 == 0) {
                fire_trigger_event(t);
                t->cooldown2 = TRIGGER_COOLDOWN_FRAMES;
            }
        }
    }
}

/* ================================================================
 * Warp state machine (continued)
 * ================================================================ */
static void updateWarpStateMachine(DWORD app, DWORD board) {
    if (g_phase == PHASE_IDLE) return;
    if (!app || IsBadReadPtr((void*)app, 0x1000)) {
        LOGS("App null during warp, aborting");
        g_freezeTimer = 0;
        unblock_pause(g_gameBase);
        g_phase = PHASE_IDLE;
        g_warpBall = 0;
        return;
    }

    DWORD now = getGameTime();
    DWORD ball = g_warpBall;
    DWORD elapsed;

    if (board && g_phase != PHASE_IDLE) {
        if (g_phase == PHASE_FLASH || g_phase == PHASE_FADE || g_phase == PHASE_REVEAL)
            *(float*)(board + BOARD_SCENE_FADE_ALPHA) = g_whiteAlpha;
    }

    if (g_phase == PHASE_RUMBLE || (g_phase == PHASE_FLASH && g_whiteAlpha < 0.99f)) {
        if (!IsBadReadPtr((void*)(app + APP_TIMER_FINISHED), 1)) {
            char finished = *(char*)(app + APP_TIMER_FINISHED);
            if (finished) {
                LOG("ABORT: timer expired during %s", g_phase == PHASE_RUMBLE ? "RUMBLE" : "FLASH");
                if (ball) {
                    *(int*)(ball + BALL_IMPACT_FREEZE) = 0;
                    *(BYTE*)(ball + BALL_RENDER_JITTER) = 0;
                    *(BYTE*)(ball + BALL_IN_TAR) = 0;
                }
                restoreMusicFade(app);
                g_freezeTimer = 0;
                unblock_pause(g_gameBase);
                if (board) *(float*)(board + BOARD_SCENE_FADE_ALPHA) = 0.0f;
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
            block_pause(g_gameBase);
            *(int*)(ball + BALL_IMPACT_FREEZE) = 1000;
            *(BYTE*)(ball + BALL_RENDER_JITTER) = 1;
            startMusicFade(app);
            if (!IsBadReadPtr((void*)(ball + BALL_COLOR_R), 12)) {
                g_origBallR = *(float*)(ball + BALL_COLOR_R);
                g_origBallG = *(float*)(ball + BALL_COLOR_G);
                g_origBallB = *(float*)(ball + BALL_COLOR_B);
                g_colorSaved = 1;
            }
            LOG("RUMBLE start");
        }

        if (g_colorSaved && ball && !IsBadWritePtr((void*)(ball + BALL_COLOR_R), 12)) {
            float t = (float)elapsed / (float)RUMBLE_DURATION_MS;
            if (t > 1.0f) t = 1.0f;
            *(float*)(ball + BALL_COLOR_R) = g_origBallR + (TARGET_PURPLE_R - g_origBallR) * t;
            *(float*)(ball + BALL_COLOR_G) = g_origBallG + (TARGET_PURPLE_G - g_origBallG) * t;
            *(float*)(ball + BALL_COLOR_B) = g_origBallB + (TARGET_PURPLE_B - g_origBallB) * t;
        }

        if (ball) {
            BYTE respawning = *(BYTE*)(ball + 0x2F9);
            if (!respawning) {
                float t = (float)elapsed / (float)RUMBLE_DURATION_MS;
                if (t > 1.0f) t = 1.0f;
                *(float*)(ball + BALL_ALPHA) = 1.0f - (0.5f * t);
            }
        }

        updateMusicFade(app);
        if (elapsed >= RUMBLE_DURATION_MS) {
            g_phase = PHASE_FLASH;
            g_phaseStartTime = now;
        }
        break;
    }

    case PHASE_FLASH: {
        elapsed = now - g_phaseStartTime;
        g_whiteAlpha = 1.0f - (float)elapsed / (float)FLASH_DURATION_MS;
        if (g_whiteAlpha < 0.0f) g_whiteAlpha = 0.0f;

        if (g_whiteAlpha >= 0.99f && ball) {
            BYTE respawning = *(BYTE*)(ball + 0x2F9);
            if (!respawning) {
                *(float*)(ball + BALL_ALPHA) = 0.0f;
                *(BYTE*)(ball + BALL_IN_TAR) = 1;
                if (!g_freezeTimer) {
                    g_freezeTimer = 1;
                    LOGS("Ball vanished — timer frozen + in_tar set");
                }
            }
        }

        updateMusicFade(app);
        if (elapsed >= FLASH_DURATION_MS) {
            g_whiteAlpha = 0.0f;
            g_phase = PHASE_HOLD;
            g_phaseStartTime = now;
        }
        break;
    }

    case PHASE_HOLD: {
        elapsed = now - g_phaseStartTime;
        g_whiteAlpha = 0.0f;
        if (ball) {
            BYTE respawning = *(BYTE*)(ball + 0x2F9);
            if (!respawning) *(float*)(ball + BALL_ALPHA) = 0.0f;
        }
        updateMusicFade(app);
        if (elapsed >= HOLD_DURATION_MS) {
            g_phase = PHASE_FADE;
            g_phaseStartTime = now;
        }
        break;
    }

    case PHASE_FADE: {
        elapsed = now - g_phaseStartTime;
        g_whiteAlpha = (float)elapsed / (float)FADE_DURATION_MS;
        if (g_whiteAlpha > 1.0f) g_whiteAlpha = 1.0f;
        if (ball) {
            BYTE respawning = *(BYTE*)(ball + 0x2F9);
            if (!respawning) *(float*)(ball + BALL_ALPHA) = 0.0f;
        }
        updateMusicFade(app);
        if (elapsed >= FADE_DURATION_MS) {
            g_whiteAlpha = 1.0f;
            g_phase = PHASE_LOAD;
            g_phaseStartTime = now;
        }
        break;
    }

    case PHASE_LOAD: {
        int levelIdx = g_warpLevelIndex;
        g_warpLevelIndex = -1;

        if (ball) {
            *(int*)(ball + BALL_IMPACT_FREEZE) = 0;
            *(BYTE*)(ball + BALL_RENDER_JITTER) = 0;
            *(float*)(ball + BALL_ALPHA) = 1.0f;
            *(BYTE*)(ball + BALL_IN_TAR) = 0;
            if (g_colorSaved && !IsBadWritePtr((void*)(ball + BALL_COLOR_R), 12)) {
                *(float*)(ball + BALL_COLOR_R) = g_origBallR;
                *(float*)(ball + BALL_COLOR_G) = g_origBallG;
                *(float*)(ball + BALL_COLOR_B) = g_origBallB;
                g_colorSaved = 0;
            }
        }
        g_freezeTimer = 0;

        if (levelIdx >= 0 && levelIdx <= 14) {
            DWORD profile = app ? *(DWORD*)(app + APP_PROFILE_PTR) : 0;
            char savedDifficulty = app ? *(char*)(app + APP_DIFFICULTY) : 0;
            DWORD oldProfile = profile;
            char wasInTournament = 0;
            int savedScores[16];
            int savedTimes[16];
            int hasTournamentData = 0;
            int savedTimeRemaining = 0;
            float savedPlayerScore = 0.0f;
            int isSameLevel = 0;

            if (oldProfile) {
                char isPractice = *(char*)(oldProfile + PROFILE_IS_PRACTICE);
                wasInTournament = (isPractice == 0) ? 1 : 0;

                int currentRaceIdx = *(int*)(oldProfile + PROFILE_RACE_INDEX);
                if (levelIdx == currentRaceIdx - 1) isSameLevel = 1;

                if (isSameLevel) {
                    if (!g_twRaceName[0]) {
                        char raceName[128];
                        if (get_race_name(raceName, sizeof(raceName))) {
                            strncpy(g_twRaceName, raceName, sizeof(g_twRaceName) - 1);
                            g_twRaceName[sizeof(g_twRaceName) - 1] = '\0';
                            LOG("TW race name set to '%s'", g_twRaceName);
                        }
                    }

                    ghost1_save_state();
                    if (g_twRaceName[0]) save_warp_segment();

                    BYTE partyMode = 0;
                    if (!IsBadReadPtr((void*)(app + APP_PARTY_FLAG), 1))
                        partyMode = *(BYTE*)(app + APP_PARTY_FLAG);
                    if (partyMode == 0) {
                        if (g_ghost2.active || g_ghost2.ball || g_ghost2.btt)
                            ghost2_destroy();
                        g_ghost2Pending = FALSE;
                        ghost2_capture();
                    }
                }

                if (wasInTournament) {
                    int raceIdx = *(int*)(oldProfile + PROFILE_RACE_INDEX);
                    hasTournamentData = 1;
                    memcpy(savedScores, (void*)(oldProfile + PROFILE_SCORE_ARRAY), sizeof(savedScores));
                    memcpy(savedTimes, (void*)(oldProfile + PROFILE_TIME_ARRAY), sizeof(savedTimes));
                    if (raceIdx >= 0 && raceIdx < 16)
                        savedScores[raceIdx] = (int)*(float*)(app + APP_SCORE);
                    if (isSameLevel) {
                        if (raceIdx >= 0 && raceIdx < 16)
                            savedTimeRemaining = *(int*)(oldProfile + PROFILE_TIME_ARRAY + raceIdx * 4);
                    } else {
                        savedTimeRemaining = *(int*)(app + APP_TIME_REMAINING);
                    }
                    savedPlayerScore = *(float*)(app + APP_SCORE);
                } else if (isSameLevel) {
                    savedTimeRemaining = *(int*)(app + APP_TIME_REMAINING);
                }
            }

            if (wasInTournament && isSameLevel) {
                call_app_start_tournament_race(app, levelIdx);
            } else {
                call_app_start_practice_race(app, levelIdx);
            }

            if (app) *(char*)(app + APP_DIFFICULTY) = savedDifficulty;

            if (isSameLevel && g_twRaceName[0]) {
                if (!g_ghost1.active)
                    ghost1_init_for_tw(g_twRaceName);
                if (g_ghost1.active)
                    ghost1_restore_after_warp();
            } else if (!isSameLevel && g_twRaceName[0]) {
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

            if (wasInTournament && isSameLevel) {
                DWORD newProfile = *(DWORD*)(app + APP_PROFILE_PTR);
                if (newProfile) {
                    *(char*)(newProfile + PROFILE_IS_PRACTICE) = 0;
                    if (hasTournamentData) {
                        memcpy((void*)(newProfile + PROFILE_SCORE_ARRAY), savedScores, sizeof(savedScores));
                        memcpy((void*)(newProfile + PROFILE_TIME_ARRAY), savedTimes, sizeof(savedTimes));
                    }
                    DWORD newProfile2 = *(DWORD*)(app + APP_PROFILE_PTR);
                    DWORD newBoard = 0;
                    int levelBaseTime = 0;
                    int difficultyBonus = 0;
                    int finalTimer = savedTimeRemaining;
                    if (newProfile2)
                        newBoard = *(DWORD*)(newProfile2 + PROFILE_BOARD_PTR);
                    if (newBoard && !IsBadReadPtr((void*)(newBoard + 0x2998), 4))
                        levelBaseTime = *(int*)(newBoard + 0x2998);
                    char diff = app ? *(char*)(app + APP_DIFFICULTY) : 0;
                    if (diff == 0) difficultyBonus = 1000;
                    else if (diff == 1) difficultyBonus = 500;
                    finalTimer = savedTimeRemaining + levelBaseTime + difficultyBonus;
                    *(int*)(app + APP_TIME_REMAINING) = finalTimer;
                    *(float*)(app + APP_SCORE) = savedPlayerScore;
                }
            } else if (wasInTournament) {
                DWORD bttRec = *(DWORD*)(app + APP_BTT_RECORDING);
                DWORD bttPlay = *(DWORD*)(app + APP_BTT_PLAYBACK);
                if (bttRec) {
                    if (!IsBadReadPtr((void*)bttRec, 4)) {
                        DWORD vt = *(DWORD*)bttRec;
                        if (vt == BTT_VTABLE_ADDR)
                            CallMethod<void>(RVA_BTT_DTOR, (void*)bttRec, (DWORD)1);
                        else
                            Call<void>(RVA_GAME_FREE, (void*)bttRec);
                    }
                    *(DWORD*)(app + APP_BTT_RECORDING) = 0;
                }
                if (bttPlay) {
                    if (!IsBadReadPtr((void*)bttPlay, 4)) {
                        DWORD vt = *(DWORD*)bttPlay;
                        if (vt == BTT_VTABLE_ADDR)
                            CallMethod<void>(RVA_BTT_DTOR, (void*)bttPlay, (DWORD)1);
                        else
                            Call<void>(RVA_GAME_FREE, (void*)bttPlay);
                    }
                    *(DWORD*)(app + APP_BTT_PLAYBACK) = 0;
                }

                void* newBTT = Call<void*>(RVA_OPERATOR_NEW, (SIZE_T)BTT_SIZE);
                if (newBTT) {
                    CallMethod<void>(RVA_BTT_CTOR, newBTT);
                    DWORD vt = *(DWORD*)newBTT;
                    if (vt == BTT_VTABLE_ADDR) {
                        *(DWORD*)((char*)newBTT + BTT_BEST_TIME) = NO_TIME;
                        // Name the warp-created tournament BTT so
                        // get_race_name() resolves a valid race name.
                        DWORD prof2 = *(DWORD*)(app + APP_PROFILE_PTR);
                        DWORD ridx = 0;
                        if (prof2 && !IsBadReadPtr((void*)(prof2 + PROFILE_RACE_INDEX), 4))
                            ridx = *(DWORD*)(prof2 + PROFILE_RACE_INDEX);
                        char rn[128] = "";
                        if (get_race_name_by_index(ridx, rn, sizeof(rn)) && rn[0]) {
                            strncpy((char*)newBTT + BTT_RACE_NAME, rn, 64);
                            *((char*)newBTT + BTT_RACE_NAME + 63) = '\0';
                        }
                        *(DWORD*)(app + APP_BTT_RECORDING) = (DWORD)newBTT;
                    } else {
                        Call<void>(RVA_GAME_FREE, newBTT);
                    }
                }

                DWORD newProfile = *(DWORD*)(app + APP_PROFILE_PTR);
                if (newProfile) {
                    *(char*)(newProfile + PROFILE_IS_PRACTICE) = 0;
                    if (hasTournamentData) {
                        memcpy((void*)(newProfile + PROFILE_SCORE_ARRAY), savedScores, sizeof(savedScores));
                        memcpy((void*)(newProfile + PROFILE_TIME_ARRAY), savedTimes, sizeof(savedTimes));
                    }
                    DWORD newProfile2 = *(DWORD*)(app + APP_PROFILE_PTR);
                    DWORD newBoard = 0;
                    int levelBaseTime = 0;
                    int difficultyBonus = 0;
                    int finalTimer = savedTimeRemaining;
                    if (newProfile2)
                        newBoard = *(DWORD*)(newProfile2 + PROFILE_BOARD_PTR);
                    if (newBoard && !IsBadReadPtr((void*)(newBoard + 0x2998), 4))
                        levelBaseTime = *(int*)(newBoard + 0x2998);
                    char diff = app ? *(char*)(app + APP_DIFFICULTY) : 0;
                    if (diff == 0) difficultyBonus = 1000;
                    else if (diff == 1) difficultyBonus = 500;
                    finalTimer = savedTimeRemaining + levelBaseTime + difficultyBonus;
                    *(int*)(app + APP_TIME_REMAINING) = finalTimer;
                    *(float*)(app + APP_SCORE) = savedPlayerScore;
                }
            } else if (isSameLevel && savedTimeRemaining > 0 && app) {
                *(int*)(app + APP_TIME_REMAINING) = savedTimeRemaining;
            }
        }

        if (app) {
            DWORD newProfile = *(DWORD*)(app + APP_PROFILE_PTR);
            if (newProfile) {
                DWORD newBoard = *(DWORD*)(newProfile + PROFILE_BOARD_PTR);
                if (newBoard) *(float*)(newBoard + BOARD_SCENE_FADE_ALPHA) = 1.0f;
            }
        }

        restoreMusicFade(app);
        g_warpBall = 0;
        g_whiteAlpha = 1.0f;
        g_phase = PHASE_REVEAL;
        g_phaseStartTime = now;
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
            unblock_pause(g_gameBase);
        }
        break;
    }

    default:
        g_freezeTimer = 0;
        unblock_pause(g_gameBase);
        g_phase = PHASE_IDLE;
        break;
    }
}

/* ================================================================
 * Mod class
 * ================================================================ */
class TimeWarpMod : public HamsterballAPI {
private:
    IModAPI* api = nullptr;

public:
    const char* GetModName() override      { return "Time Warp"; }
    const char* GetAuthorName() override   { return "RodentRacer"; }
    const char* GetContributors() override { return "Hamsterbot"; }
    int GetApiVersion() override           { return HAMSTERBALL_API_VERSION; }

    void Initialize(IModAPI* modApi) override {
        api = modApi;
        g_api = modApi;
        g_gameBase = modApi ? modApi->GetGameBaseAddress() : 0x00400000;

#ifdef ENABLE_LOGGING
        HMODULE hSelf = NULL;
        GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                          | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                          (LPCSTR)&load_bass_function, &hSelf);
        if (hSelf) {
            char path[MAX_PATH];
            if (GetModuleFileNameA(hSelf, path, MAX_PATH)) {
                char* p = strrchr(path, '\\');
                if (p) strcpy(p + 1, "time_warp_log.txt");
                else strcpy(path, "time_warp_log.txt");
                strncpy(g_logPath, path, MAX_PATH - 1);
                g_logPath[MAX_PATH - 1] = '\0';
            }
        }
#endif

        load_bass_function();
        install_timer_caves(g_gameBase);

        api->CreateToggleButton(CustomButton("TIME_WARP_ENABLED", "Time Warp"), this);

        api->RegisterCustomHook(g_gameBase + RVA_APP_START_PRACTICE_RACE,
            (void*)hook_AppStartPracticeRace,
            (void**)&orig_AppStartPracticeRace);

        api->RegisterCustomHook(g_gameBase + RVA_APP_START_TOURNAMENT_RACE,
            (void*)hook_AppStartTournamentRace,
            (void**)&orig_AppStartTournamentRace);

        install_ghost_mode_cave(g_gameBase);
        install_tt_recording_nop();

        init_ghost_dir();
        LOGS("=== Time Warp (HB+) loaded ===");
    }

    void onButtonToggle(const char* buttonId, bool newState) override {
        if (strcmp(buttonId, "TIME_WARP_ENABLED") == 0) {
            g_warpEnabled = newState;
        }
    }

    void onGameUpdate() override {
        DWORD app = get_app();
        if (!app || IsBadReadPtr((void*)app, 0x1000)) {
            if (g_phase != PHASE_IDLE) {
                g_freezeTimer = 0;
                unblock_pause(g_gameBase);
                g_phase = PHASE_IDLE;
            }
            return;
        }

        DWORD board = get_board(app);
        updateGameClock(board);
        check_race_state();

        ghost2_check_board_change(board);
        if (g_ghost2.active) ghost2_playback();
        ghost1_check_advance();

        if (board && board != g_triggerBoard) {
            g_triggerBoard = board;
            scan_s1_triggers(board);
        }
        if (board && g_triggerCount > 0) check_ghost_triggers(board);

        ghost_event_frame(app, board);

        if (g_phase == PHASE_IDLE && board) {
            scanWarpNodes(board);
        }
        updateWarpStateMachine(app, board);
    }

    void onEventPlaneCollide(Ball* collidingBall, char* eventPlaneID) override {
        (void)collidingBall;
        if (!eventPlaneID || !eventPlaneID[0]) return;

        if (_strnicmp(eventPlaneID, "E:GHOST", 7) == 0) {
            if (g_ghostFromEvent && g_ghostActive && g_loadedBTT) return;
            LOG("onEventPlaneCollide: E:GHOST full='%s'", eventPlaneID);
            const char* p1 = strchr(eventPlaneID, '(');
            if (p1) {
                const char* p2 = strchr(p1, ')');
                if (p2 && (p2 - p1 - 1) > 0) {
                    size_t len = p2 - p1 - 1;
                    if (len > 250) len = 250;
                    memcpy(g_pendingGhostFile, p1 + 1, len);
                    g_pendingGhostFile[len] = '\0';
                    LOG("Parsed filename='%s'", g_pendingGhostFile);
                }
            }
        }
    }

    void onSceneEnd() override {
        // NOTE: Do NOT restore the one-time session caves here. They are
        // installed once in Initialize() and must stay live for the whole
        // session — including across warps (the warp's PHASE_LOAD calls the
        // game's race-start function directly, which tears down the scene;
        // restoring the caves mid-warp would break the timer freeze and the
        // TT-recording NOP for the remainder of the session).
        // The scene deconstructor fires DURING a warp's PHASE_LOAD (the
        // race-start call tears down the old scene). In that case ghost1
        // state was already saved by ghost1_save_state() and must survive
        // until ghost1_restore_after_warp() runs — so do NOT clear it.
        int midWarp = (g_phase == PHASE_LOAD);
        g_phase = PHASE_IDLE;
        g_freezeTimer = 0;
        g_warpBall = 0;
        g_musicFadeStarted = 0;
        unblock_pause(g_gameBase);
        g_triggerBoard = 0;
        g_triggerCount = 0;
        g_recording = 0;
        g_raceFinished = 0;
        g_prevGoalFlag = 0;
        g_currentRaceName[0] = '\0';
        g_hookRaceName[0] = '\0';
        g_prevRecording = 0;
        // Ghost 1 cleanup on REAL scene end (player left the level, not a
        // warp reload): the scene is gone, so any BTT we injected into
        // App+0x910 is gone with it. Zero the whole state so a stale
        // pointer is never dereferenced in a later race. Also reset the
        // Time Warp level flags — on a real scene end (not a warp) the
        // PHASE_LOAD !isSameLevel branch never runs, so without this a
        // later non-warp race would wrongly run handle_tw_goal_touch().
        if (!midWarp) {
            memset(&g_ghost1, 0, sizeof(g_ghost1));
            g_twRaceName[0] = '\0';
            g_segmentCounter = 0;
            g_segmentCount = 0;
            memset(g_segmentTimes, 0, sizeof(g_segmentTimes));
            g_isTimeWarpLevel = FALSE;
        }
        if (api) {
            DWORD app = (DWORD)api->GetApp();
            if (app) {
                restoreMusicFade(app);
                cleanup_previous_ghost(app);
            }
        }
        ghost2_destroy();
        // CRITICAL: Do NOT free g_ghost2Capture / clear g_ghost2Pending here.
        // The warp capture (ghost2_capture) runs BEFORE the level reload and
        // is consumed by ghost2_check_board_change AFTER the new board loads.
        // Clearing them here breaks Ghost 2 creation across warps.
        snaps_reset();
    }
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    return new TimeWarpMod();
}
