#include "HamsterballAPI.h"
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>

/*
 * Level Warp for HB+ v2.1
 *
 * Port of level_warp (bass.dll proxy) to HB+ API.
 *
 * Differences from bass.dll version:
 *  - No BASS proxy shell. Music fade uses the real bass.dll directly.
 *  - Frame epilogue hook replaced by onGameUpdate callback.
 *  - Memory patches applied through IModAPI::PatchMemory.
 *
 * Behavior preserved:
 *  - Scans S1 ref points prefixed "WARP(Name)" for warp zones.
 *  - Multi-phase warp effect (rumble, flash, hold, fade, load, reveal).
 *  - Freeze race timer via code caves during warp.
 *  - Block pause input during warp.
 *  - Fade out / restore music during warp.
 *  - Load target level via App_StartPracticeRace.
 *  - Preserve tournament score/time across warps.
 */

/* ---- Config ---- */
#define ENABLE_LOGGING

/* ---- Function addresses ---- */
#define ADDR_APP_START_PRACTICE_RACE 0x00428C50

/* ---- App offsets ---- */
#define APP_PTR                 0x005341E0
#define APP_PROFILE_PTR         0x220
#define APP_MUSIC_DEVICE_PTR    0x17C
#define APP_BTT_RECORDING       0x90C
#define APP_BTT_PLAYBACK        0x910
#define APP_DIFFICULTY          0x23C
#define APP_SCORE               0x5E4
#define APP_TIME_REMAINING      0x5E8
#define APP_TIMER_FINISHED      0x5D6

/* ---- Profile / Board offsets ---- */
#define PROFILE_RACE_INDEX      0x08
#define PROFILE_BOARD_PTR       0x0C
#define PROFILE_IS_PRACTICE     0x11
#define PROFILE_SCORE_ARRAY     0x50
#define PROFILE_TIME_ARRAY      0x14

#define BOARD_APP_PTR           0x878
#define BOARD_MESHWORLD_PTR     0x8AC
#define BOARD_GOAL_REACHED      0xCD0
#define BOARD_PAUSED_FLAG       0x874
#define BOARD_SCENE_FADE_ALPHA  0x3624

/* ---- MeshWorld / S1 offsets ---- */
#define MW_SECTION3_PTR         0x480
#define MW_NODELIST_COUNT       0x898
#define MW_NODELIST_DATA        0xCA0
#define NODE_NAME_OFFSET        0x00
#define NODE_X_OFFSET           0x04
#define NODE_Y_OFFSET           0x08
#define NODE_Z_OFFSET           0x0C

/* ---- Ball offsets ---- */
#define BALL_POS_X              0x164
#define BALL_POS_Y              0x168
#define BALL_POS_Z              0x16C
#define BALL_COLOR_R            0x2AC
#define BALL_COLOR_G            0x2B0
#define BALL_COLOR_B            0x2B4
#define BALL_ALPHA              0x2FC
#define BALL_IMPACT_FREEZE      0x808
#define BALL_RENDER_JITTER      0x2D4
#define BALL_DEATH_PENDING      0x2E9
#define BALL_IN_TAR             0x2CC

/* ---- Music offsets ---- */
#define MUSIC_DEV_CHANNEL_LIST  0x418
#define MUSIC_DEV_COUNT         0x10
#define MUSIC_CHAN_BASS_CHANNEL 0x08
#define MUSIC_CHAN_VOLUME       0x528
#define MUSIC_CHAN_FADE_RATE    0x52C
#define MUSIC_CHAN_FADE_OUT     0x530
#define MUSIC_CHAN_FADE_IN      0x531

/* ---- Timer patch RVAs ---- */
#define TIMER_DEC_PATCH_RVA     0x1B3E5
#define TIMER_DEC_PATCH_SIZE    9
#define TIMER_DEC_RETURN_RVA    0x1B3EE
#define TIMER_DEC_SKIP_RVA      0x1B49D

#define TIMER_INC_PATCH_RVA     0x1B50C
#define TIMER_INC_PATCH_SIZE    5
#define TIMER_INC_RETURN_RVA    0x1B511

/* ---- Pause-block patch RVAs ---- */
#define PAUSE_PATCH_PATH1       0x19D5B
#define PAUSE_PATCH_PATH2       0x130B5
#define PAUSE_PATCH_PATH3       0x0B405

/* ---- Warp params ---- */
#define WARP_TRIGGER_DIST_SQ    625.0f  /* 25^2: ball radius distance from S1 point */
#define WARP_COOLDOWN_MS        2000

#define RUMBLE_DURATION_MS      2000
#define FLASH_DURATION_MS       150
#define HOLD_DURATION_MS        1000
#define FADE_DURATION_MS        2000
#define REVEAL_DURATION_MS      1000
#define MUSIC_FADE_MS           3000

#define MAX_MUSIC_CHANNELS      8

#pragma pack(push, 1)
typedef struct {
    DWORD magic;
    DWORD version;
    DWORD time;
    DWORD frameCount;
} GhostFileHeader;
#pragma pack(pop)

/* ---- Logging ---- */
#ifdef ENABLE_LOGGING
static char g_logPath[MAX_PATH] = "";
static void diag_log(const char *msg) {
    if (!g_logPath[0]) return;
    HANDLE h = CreateFileA(g_logPath, FILE_APPEND_DATA,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, NULL);
    if (h != INVALID_HANDLE_VALUE) {
        DWORD w; SetFilePointer(h, 0, NULL, FILE_END);
        WriteFile(h, msg, (DWORD)strlen(msg), &w, NULL);
        WriteFile(h, "\r\n", 2, &w, NULL);
        CloseHandle(h);
    }
}
static void diag_logf(const char *fmt, ...) {
    char buf[512]; va_list args;
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

/* ---- BASS function pointer for music fade ---- */
typedef int (__stdcall *BASS_ChannelSetAttributes_t)(DWORD handle, float freq, int volume, int pan);
static BASS_ChannelSetAttributes_t g_realBASS_ChannelSetAttributes = NULL;

static void load_bass_function(void) {
    if (g_realBASS_ChannelSetAttributes) return;
    HMODULE hBass = LoadLibraryA("bass.dll");
    if (hBass) {
        g_realBASS_ChannelSetAttributes = (BASS_ChannelSetAttributes_t)GetProcAddress(hBass, "BASS_ChannelSetAttributes");
    }
}

/* ---- Pause-aware game clock ---- */
static DWORD g_gameClock = 0;
static DWORD g_lastRealTime = 0;

static int is_game_paused(DWORD board) {
    if (!board) return 0;
    if (IsBadReadPtr((void*)(board + BOARD_PAUSED_FLAG), 1)) return 0;
    return *(BYTE*)(board + BOARD_PAUSED_FLAG) != 0;
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

/* ---- Memory patch helpers ---- */

static void read_bytes(DWORD addr, void *out, DWORD size) {
    memcpy(out, (void*)addr, size);
}

static void write_jmp(unsigned char *at, DWORD target) {
    at[0] = 0xE9;
    *(DWORD*)(at + 1) = target - (DWORD)at - 5;
}

/* ---- Timer freeze caves ---- */
static BYTE g_decOrigBytes[TIMER_DEC_PATCH_SIZE];
static BYTE g_incOrigBytes[TIMER_INC_PATCH_SIZE];
static int g_decPatched = 0;
static int g_incPatched = 0;
static unsigned char *g_decCave = NULL;
static unsigned char *g_incCave = NULL;
static volatile BYTE g_freezeTimer = 0;

static void install_timer_caves(DWORD base) {
    DWORD oldProt;

    /* ---- Cave 1: DEC path (tournament countdown) ---- */
    if (!g_decPatched) {
        DWORD patchAddr = base + TIMER_DEC_PATCH_RVA;
        DWORD returnAddr = base + TIMER_DEC_RETURN_RVA;
        DWORD skipAddr = base + TIMER_DEC_SKIP_RVA;
        DWORD freezeAddr = (DWORD)&g_freezeTimer;

        g_decCave = (unsigned char*)VirtualAlloc(NULL, 64, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        if (!g_decCave) { LOG("VirtualAlloc failed for DEC cave"); return; }

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

        /* DEC [EDI+0x1C] (3 bytes) */
        p[0] = 0xFF; p[1] = 0x4F; p[2] = 0x1C;
        p += 3;

        /* JMP return */
        write_jmp(p, returnAddr);
        p += 5;

        /* skip_target: JMP skipAddr */
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

    /* ---- Cave 2: INC path (practice/TT/party) ---- */
    if (!g_incPatched) {
        DWORD patchAddr = base + TIMER_INC_PATCH_RVA;
        DWORD returnAddr = base + TIMER_INC_RETURN_RVA;
        DWORD freezeAddr = (DWORD)&g_freezeTimer;

        g_incCave = (unsigned char*)VirtualAlloc(NULL, 64, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        if (!g_incCave) { LOG("VirtualAlloc failed for INC cave"); return; }

        unsigned char *p = g_incCave;

        /* JNZ skip_inc (short, 2 bytes) */
        p[0] = 0x75;
        p[1] = 0x0C; /* skip 12 bytes to skip_inc JMP */
        p += 2;

        /* CMP byte [g_freezeTimer], 0 (7 bytes) */
        p[0] = 0x80; p[1] = 0x3D;
        *(DWORD*)(p + 2) = freezeAddr;
        p[6] = 0x00;
        p += 7;

        /* JNE skip_inc (short, 2 bytes) */
        p[0] = 0x75;
        p[1] = 0x02;
        p += 2;

        /* INC [EDX+0x1C] (3 bytes) */
        p[0] = 0xFF; p[1] = 0x42; p[2] = 0x1C;
        p += 3;

        /* skip_inc: JMP return */
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

/* ---- Level name mapping ---- */
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
    for (int j = 0; levelMap[j].meshName; j++) {
        if (_stricmp(levelName, levelMap[j].meshName) == 0)
            return levelMap[j].raceIndex;
    }
    return -1;
}

/* ---- Warp state ---- */
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

static float g_musicOrigVolumes[MAX_MUSIC_CHANNELS];
static int g_musicChannelCount = 0;

static volatile float g_whiteAlpha = 0.0f;
static volatile DWORD g_warpBall = 0;
static volatile int g_rumbleInit = 0;
static DWORD g_gameBase = 0x00400000;
static float g_origBallR = 1.0f, g_origBallG = 1.0f, g_origBallB = 1.0f;
static int g_colorSaved = 0;

#define TARGET_PURPLE_R (219.0f / 255.0f)
#define TARGET_PURPLE_G (3.0f / 255.0f)
#define TARGET_PURPLE_B (252.0f / 255.0f)

/* ---- Music fade ---- */

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
            if (chan) {
                g_musicOrigVolumes[i] = *(float*)(chan + MUSIC_CHAN_VOLUME);
            } else {
                g_musicOrigVolumes[i] = 0.0f;
            }
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
                    if (bassChan) {
                        g_realBASS_ChannelSetAttributes(bassChan, -1.0f, (int)(vol * 100.0f), -1);
                    }
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
                    if (bassChan) {
                        g_realBASS_ChannelSetAttributes(bassChan, -1.0f, (int)(g_musicOrigVolumes[i] * 100.0f), -1);
                    }
                }
            }
        }
    }
    g_musicFadeStarted = 0;
    LOGS("[warp] Music channels restored");
}

/* ---- Warp node scan ---- */

static int isWarpNode(const char *nodeName) {
    return nodeName && _strnicmp(nodeName, "WARP(", 5) == 0;
}

static int parseWarpLevel(const char *nodeName, char *out, int outSize) {
    const char *start = strchr(nodeName, '(');
    if (!start) return 0;
    start++;
    const char *end = strchr(start, ')');
    if (!end) return 0;
    int len = (int)(end - start);
    if (len <= 0 || len >= outSize) return 0;
    strncpy(out, start, len);
    out[len] = '\0';
    return 1;
}

static DWORD get_app(void) {
    return *(DWORD*)APP_PTR;
}

static DWORD get_board_from_app(DWORD app) {
    if (!app || IsBadReadPtr((void*)app, 0x1000)) return 0;
    DWORD profile = *(DWORD*)(app + APP_PROFILE_PTR);
    if (!profile || IsBadReadPtr((void*)profile, 0x100)) return 0;
    DWORD board = *(DWORD*)(profile + PROFILE_BOARD_PTR);
    if (!board || IsBadReadPtr((void*)board, 0x4000)) return 0;
    return board;
}

static void scanWarpNodes(DWORD board) {
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

    DWORD app = get_app();
    DWORD profile = app ? *(DWORD*)(app + APP_PROFILE_PTR) : 0;
    DWORD ball = 0;
    if (profile && !IsBadReadPtr((void*)profile, 0x100)) {
        DWORD board2 = *(DWORD*)(profile + PROFILE_BOARD_PTR);
        if (board2 == board) {
            /* Get first ball from ball list */
            DWORD ballListData = *(DWORD*)(board + 0x2DE0);
            if (ballListData && !IsBadReadPtr((void*)ballListData, 4)) {
                ball = *(DWORD*)ballListData;
            }
        }
    }
    if (!ball || IsBadReadPtr((void*)ball, 0xC60)) return;
    if (*(BYTE*)(ball + BALL_DEATH_PENDING) != 0) return;

    float bx = *(float*)(ball + BALL_POS_X);
    float by = *(float*)(ball + BALL_POS_Y);
    float bz = *(float*)(ball + BALL_POS_Z);

    for (int i = 0; i < nodeCount; i++) {
        DWORD entry = *(DWORD*)(nodeData + i * 4);
        if (!entry || IsBadReadPtr((void*)entry, 0x10)) continue;
        char *name = *(char**)entry;
        if (!name || IsBadReadPtr(name, 1)) continue;
        if (!isWarpNode(name)) continue;

        float nx = *(float*)(entry + NODE_X_OFFSET);
        float ny = *(float*)(entry + NODE_Y_OFFSET);
        float nz = *(float*)(entry + NODE_Z_OFFSET);

        float dx = bx - nx;
        float dy = by - ny;
        float dz = bz - nz;
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

/* ---- App_StartPracticeRace wrapper ---- */

static void call_app_start_practice_race(DWORD app, int idx) {
    DWORD addr = ADDR_APP_START_PRACTICE_RACE;
    __asm__ volatile (
        "push %1\n\t"
        "movl %0, %%ecx\n\t"
        "call *%2\n\t"
        : : "r"(app), "r"(idx), "r"(addr)
        : "eax", "ecx", "edx", "memory", "st", "st(1)", "st(2)", "st(3)",
          "st(4)", "st(5)", "st(6)", "st(7)"
    );
}

/* ---- State machine ---- */

static void updateWarpStateMachine(DWORD app, DWORD board) {
    if (g_phase == PHASE_IDLE) return;

    if (!app || IsBadReadPtr((void*)app, 0x1000)) {
        LOGS("[warp] App null during warp, aborting");
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
        if (g_phase == PHASE_FLASH || g_phase == PHASE_FADE || g_phase == PHASE_REVEAL) {
            *(float*)(board + BOARD_SCENE_FADE_ALPHA) = g_whiteAlpha;
        }
    }

    /* Abort if tournament timer expires during RUMBLE or early FLASH.
     * App+0x5D6 (obj+0x0A finished flag) is set on timeout or goal.
     * Let the game's natural timeout sequence play out. */
    if (g_phase == PHASE_RUMBLE || (g_phase == PHASE_FLASH && g_whiteAlpha < 0.99f)) {
        if (!IsBadReadPtr((void*)(app + APP_TIMER_FINISHED), 1)) {
            char finished = *(char*)(app + APP_TIMER_FINISHED);
            if (finished) {
                LOG("[warp] ABORT: tournament timer expired during %s", g_phase == PHASE_RUMBLE ? "RUMBLE" : "FLASH");
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
                LOGS("[warp] Warp aborted — returning to IDLE with game timeout intact");
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
                    LOGS("[warp] Ball vanished — timer frozen + in_tar set");
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
            if (!respawning) {
                *(float*)(ball + BALL_ALPHA) = 0.0f;
            }
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
            if (!respawning) {
                *(float*)(ball + BALL_ALPHA) = 0.0f;
            }
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
            int oldProfile = profile;
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
                if (levelIdx == currentRaceIdx - 1) {
                    isSameLevel = 1;
                }

                if (wasInTournament) {
                    int raceIdx = *(int*)(oldProfile + PROFILE_RACE_INDEX);
                    hasTournamentData = 1;
                    memcpy(savedScores, (void*)(oldProfile + PROFILE_SCORE_ARRAY), sizeof(savedScores));
                    memcpy(savedTimes, (void*)(oldProfile + PROFILE_TIME_ARRAY), sizeof(savedTimes));
                    if (raceIdx >= 0 && raceIdx < 16) {
                        savedScores[raceIdx] = (int)*(float*)(app + APP_SCORE);
                    }
                    if (isSameLevel) {
                        if (raceIdx >= 0 && raceIdx < 16) {
                            savedTimeRemaining = *(int*)(oldProfile + PROFILE_TIME_ARRAY + raceIdx * 4);
                        }
                    } else {
                        savedTimeRemaining = *(int*)(app + APP_TIME_REMAINING);
                    }
                    savedPlayerScore = *(float*)(app + APP_SCORE);
                } else if (isSameLevel) {
                    savedTimeRemaining = *(int*)(app + APP_TIME_REMAINING);
                }
            }

            call_app_start_practice_race(app, levelIdx);

            if (app) *(char*)(app + APP_DIFFICULTY) = savedDifficulty;

            if (wasInTournament && app) {
                int bttRec = *(int*)(app + APP_BTT_RECORDING);
                int bttPlay = *(int*)(app + APP_BTT_PLAYBACK);
                if (bttRec) *(int*)(app + APP_BTT_RECORDING) = 0;
                if (bttPlay) *(int*)(app + APP_BTT_PLAYBACK) = 0;

                DWORD newProfile = *(DWORD*)(app + APP_PROFILE_PTR);
                if (newProfile) {
                    *(char*)(newProfile + PROFILE_IS_PRACTICE) = 0;
                    if (hasTournamentData) {
                        memcpy((void*)(newProfile + PROFILE_SCORE_ARRAY), savedScores, sizeof(savedScores));
                        memcpy((void*)(newProfile + PROFILE_TIME_ARRAY), savedTimes, sizeof(savedTimes));
                    }

                    DWORD newBoard = *(DWORD*)(newProfile + PROFILE_BOARD_PTR);
                    int levelBaseTime = 0;
                    int difficultyBonus = 0;
                    int finalTimer = savedTimeRemaining;

                    if (newBoard && !IsBadReadPtr((void*)(newBoard + 0x2998), 4)) {
                        levelBaseTime = *(int*)(newBoard + 0x2998);
                    }

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

        /* Set white alpha on new board */
        if (app) {
            DWORD newProfile = *(DWORD*)(app + APP_PROFILE_PTR);
            if (newProfile) {
                DWORD newBoard = *(DWORD*)(newProfile + PROFILE_BOARD_PTR);
                if (newBoard) {
                    *(float*)(newBoard + BOARD_SCENE_FADE_ALPHA) = 1.0f;
                }
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

/* ---- Mod class ---- */

class LevelWarpMod : public HamsterballAPI {
private:
    IModAPI* api = nullptr;
    DWORD g_base = 0x00400000;

public:
    const char* GetModName() override      { return "Level Warp"; }
    const char* GetAuthorName() override   { return "RodentRacer"; }
    const char* GetContributors() override { return "Hamsterbot"; }
    int GetApiVersion() override           { return HAMSTERBALL_API_VERSION; }

    void Initialize(IModAPI* modApi) override {
        api = modApi;
        if (api) { g_base = api->GetGameBaseAddress(); g_gameBase = g_base; }
        else { g_base = 0x00400000; g_gameBase = 0x00400000; }

#ifdef ENABLE_LOGGING
        HMODULE hSelf = NULL;
        GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                          | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                          (LPCSTR)&load_bass_function, &hSelf);
        if (hSelf) {
            char path[MAX_PATH];
            if (GetModuleFileNameA(hSelf, path, MAX_PATH)) {
                char *p = strrchr(path, '\\');
                if (p) strcpy(p + 1, "warp_log.txt");
                else strcpy(path, "warp_log.txt");
                strncpy(g_logPath, path, MAX_PATH - 1);
                g_logPath[MAX_PATH - 1] = '\0';
            }
        }
#endif
        load_bass_function();
        install_timer_caves(g_base);

        LOGS("=== Level Warp (HB+) loaded ===");
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

        DWORD board = get_board_from_app(app);
        updateGameClock(board);

        if (g_phase == PHASE_IDLE && board) {
            scanWarpNodes(board);
        }
        updateWarpStateMachine(app, board);
    }

    void onSceneEnd() override {
        g_phase = PHASE_IDLE;
        g_freezeTimer = 0;
        g_warpBall = 0;
        g_musicFadeStarted = 0;
        unblock_pause(g_gameBase);
        if (api) {
            DWORD app = (DWORD)api->GetApp();
            if (app) restoreMusicFade(app);
        }
    }
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    return new LevelWarpMod();
}
