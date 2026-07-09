/*
 * WARP (Level Warp) Mod v7
 *
 * CHANGES FROM v6f:
 *   - REMOVED DispatchCollisionEvents hook entirely. Warp is no longer
 *     triggered by E:WARP() collision events on floor planes.
 *   - ADDED MeshWorld Section 1 node-point proximity scanner. Level designers
 *     place a node named WARP(levelname) in the MESHWORLD (e.g. WARP(3) or
 *     WARP(neon)). When the player ball gets within 30 units (XZ) and 50
 *     units (Y) of the node position, the warp sequence triggers.
 *     This mirrors how the Up Race vacuum system works: Scene_SetupLevelUp
 *     reads VAC-IN/VAC-OUT/VAC-VEC nodes from the MeshWorld, then
 *     CollisionFace_Update checks ball proximity per-frame.
 *   - ADDED 2-second cooldown after warp completes to prevent immediate
 *     re-triggering.
 *   - Removed all collision-hook related code: g_detourBuf, g_savedBoard,
 *     g_savedBall, g_savedCollObj, WarpCollisionHandler, InstallCollisionHook,
 *     DISPATCH_COLLISION_EVENTS constant, COLL_OBJ_NAME_OFFSET constant.
 *   - Removed isWarpEvent() — replaced by isWarpNode() checking "WARP(" prefix.
 *
 * Node access pattern (from decompiled Scene_SetupLevelUp @ 0x411540):
 *   board+0x8AC  = MeshWorld*
 *   MW+0x480     = section3 data*
 *   section3+0x898 = node count (AthenaList+0x04 from section3+0x894)
 *   section3+0xCA0 = node data array ptr (AthenaList+0x40C)
 *   Each node entry: [0]=char* name, [4]=float X, [8]=float Y, [12]=float Z
 *
 * Ball list access pattern (from decompiled CollisionFace_Update @ 0x43D160):
 *   board+0x29D4 = AthenaList (ball list)
 *   board+0x2DE0 = ball data array ptr (AthenaList+0x40C)
 *   First entry = player ball pointer
 *
 * Proximity thresholds (matching vacuum system):
 *   XZ distance < 30.0 (vacuum uses Math_FastDistance2D < 0x1E)
 *   |Y difference| < 50.0 (vacuum uses _DAT_004d5d10 = 50.0 double)
 *
 * Phase timeline (real-time, not frame-based):
 *   RUMBLE: 2.0 sec — Ball frozen + CPUID jitter + sound + music fade start
 *   FLASH:  0.25 sec — Ball invisible + quick white flash
 *   HOLD:   1.0 sec — Pause, screen clear, ball stays invisible
 *   FADE:   2.0 sec — Screen fades to solid white
 *   LOAD:   instant — Load target level while screen stays white
 *   REVEAL: 1.0 sec — Fade from white to reveal new level
 *   Total:  ~6.25 sec
 *
 * Music fade: 3.0 sec starting at RUMBLE start
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll warp_mod_v7.c -lwinmm \
 *     -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* ============================================================
 * Diagnostic logging — uses vsnprintf (supports %f, unlike wvsprintfA)
 * ============================================================ */

static char g_logPath[MAX_PATH] = "";

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

/* ============================================================
 * BASS Proxy Exports
 * ============================================================ */

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

typedef int  (__stdcall *BASS_Init_t)(int, DWORD, DWORD, DWORD, void*);
static BASS_Init_t real_BASS_Init = NULL;
__declspec(dllexport) int __stdcall BASS_Init(int a, DWORD b, DWORD c, DWORD d, void* e) {
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

/* Extra BASS stubs */
__declspec(dllexport) void __stdcall BASS_Pause(void) {}
__declspec(dllexport) void __stdcall BASS_SetVolume(DWORD a) { (void)a; }
__declspec(dllexport) DWORD __stdcall BASS_GetVolume(void) { return 0; }
__declspec(dllexport) int __stdcall BASS_GetDevice(void) { return 0; }
__declspec(dllexport) int __stdcall BASS_SetDevice(DWORD a) { (void)a; return 1; }
__declspec(dllexport) void __stdcall BASS_GetInfo(void *a) { (void)a; }
__declspec(dllexport) int __stdcall BASS_Update(DWORD a) { (void)a; return 0; }
__declspec(dllexport) DWORD __stdcall BASS_StreamCreateFile(void *a, void *b, DWORD c, DWORD d, DWORD e) { (void)a;(void)b;(void)c;(void)d;(void)e; return 0; }
__declspec(dllexport) DWORD __stdcall BASS_SampleLoad(int a, void *b, DWORD c, DWORD d, DWORD e) { (void)a;(void)b;(void)c;(void)d;(void)e; return 0; }
__declspec(dllexport) int __stdcall BASS_ChannelPlay(DWORD a, BOOL b) { (void)a;(void)b; return 1; }
__declspec(dllexport) int __stdcall BASS_ChannelSetAttribute(DWORD a, DWORD b, float c) { (void)a;(void)b;(void)c; return 1; }
__declspec(dllexport) int __stdcall BASS_ChannelGetAttribute(DWORD a, DWORD b, float *c) { (void)a;(void)b;(void)c; return 1; }
__declspec(dllexport) DWORD __stdcall BASS_ChannelGetData(DWORD a, void *b, DWORD c) { (void)a;(void)b;(void)c; return 0; }
__declspec(dllexport) DWORD __stdcall BASS_ChannelGetLevel(DWORD a) { (void)a; return 0; }
__declspec(dllexport) int __stdcall BASS_ChannelSetPosition(DWORD a, void *b, DWORD c) { (void)a;(void)b;(void)c; return 1; }
__declspec(dllexport) DWORD __stdcall BASS_ChannelGetPosition(DWORD a, DWORD b) { (void)a;(void)b; return 0; }
__declspec(dllexport) int __stdcall BASS_ChannelIsActive(DWORD a) { (void)a; return 0; }
__declspec(dllexport) int __stdcall BASS_ChannelRemoveSync(DWORD a, DWORD b) { (void)a;(void)b; return 1; }
__declspec(dllexport) DWORD __stdcall BASS_ChannelSetSync(DWORD a, DWORD b, DWORD c, void *d, void *e) { (void)a;(void)b;(void)c;(void)d;(void)e; return 0; }
__declspec(dllexport) DWORD __stdcall BASS_SampleCreate(DWORD a, DWORD b, DWORD c, DWORD d, DWORD e) { (void)a;(void)b;(void)c;(void)d;(void)e; return 0; }
__declspec(dllexport) DWORD __stdcall BASS_SampleGetChannel(DWORD a, BOOL b) { (void)a;(void)b; return 0; }

static void load_real_bass(void)
{
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
        real_BASS_ChannelSetAttributes = (BASS_ChannelSetAttributes_t)GetProcAddress(g_hRealBass, "BASS_ChannelSetAttributes");
        real_BASS_MusicPlayEx          = (BASS_MusicPlayEx_t)GetProcAddress(g_hRealBass, "BASS_MusicPlayEx");
        real_BASS_SetConfig             = (BASS_SetConfig_t)GetProcAddress(g_hRealBass, "BASS_SetConfig");
        real_BASS_Init                  = (BASS_Init_t)GetProcAddress(g_hRealBass, "BASS_Init");
        real_BASS_Free                  = (BASS_Free_t)GetProcAddress(g_hRealBass, "BASS_Free");
        real_BASS_Start                 = (BASS_Start_t)GetProcAddress(g_hRealBass, "BASS_Start");
        real_BASS_Stop                  = (BASS_Stop_t)GetProcAddress(g_hRealBass, "BASS_Stop");
        real_BASS_ErrorGetCode          = (BASS_ErrorGetCode_t)GetProcAddress(g_hRealBass, "BASS_ErrorGetCode");
        real_BASS_MusicLoad             = (BASS_MusicLoad_t)GetProcAddress(g_hRealBass, "BASS_MusicLoad");
        real_BASS_ChannelStop            = (BASS_ChannelStop_t)GetProcAddress(g_hRealBass, "BASS_ChannelStop");
    }
}

/* ============================================================
 * Game constants (verified via Ghidra decompilation)
 * ============================================================ */

#define EXE_BASE        0x00400000
#define APP_PTR         0x005341E0

/* Function addresses */
#define APP_START_PRACTICE_RACE     0x00428C50

/* App offsets */
#define APP_PROFILE_PTR          0x220
#define APP_MUSIC_DEVICE_PTR     0x17C
#define APP_BTT_RECORDING        0x90C
#define APP_BTT_PLAYBACK         0x910

/* Board/Scene offsets */
#define BOARD_SCENE_PTR_OFFSET   0x878

/* MeshWorld access (from decompiled Scene_SetupLevelUp @ 0x411540) */
#define BOARD_MESHWORLD_PTR      0x8AC   /* board+0x8AC = MeshWorld* (same as scene->meshworld) */
#define MW_SECTION3_PTR          0x480   /* MeshWorld+0x480 = section3 data* */
#define MW_NODELIST_OFFSET       0x894   /* section3+0x894 = AthenaList start (node list) */
#define MW_NODELIST_COUNT        0x898   /* section3+0x898 = node count (AthenaList+0x04) */
#define MW_NODELIST_DATA         0xCA0   /* section3+0xCA0 = data array ptr (AthenaList+0x40C) */

/* Node entry layout (16 bytes each) */
#define NODE_NAME_OFFSET         0x00   /* char* name pointer */
#define NODE_X_OFFSET            0x04   /* float X position */
#define NODE_Y_OFFSET            0x08   /* float Y position */
#define NODE_Z_OFFSET            0x0C   /* float Z position */

/* Ball list access (from decompiled CollisionFace_Update @ 0x43D160) */
#define BALL_LIST_OFFSET         0x29D4  /* board+0x29D4 = AthenaList (ball list) */
#define BALL_LIST_DATA_OFFSET    0x2DE0  /* board+0x2DE0 = data array ptr (AthenaList+0x40C from 0x29D4) */

/* PlayerProfile offsets */
#define PROFILE_RACE_INDEX       0x08
#define PROFILE_BOARD_PTR        0x0C
#define PROFILE_IS_PRACTICE      0x11  /* byte: 1=practice/TT, 0=tournament */
#define PROFILE_SCORE_ARRAY      0x14  /* float[16] */
#define PROFILE_TIME_ARRAY       0x50  /* int[16] */

/* Ball offsets */
#define BALL_DEATH_PENDING       0x2E9
#define BALL_POS_X               0x164
#define BALL_POS_Y               0x168
#define BALL_POS_Z               0x16C
#define BALL_IMPACT_FREEZE       0x808
#define BALL_IN_TAR              0x2CC
#define BALL_ALPHA               0x2FC
#define BALL_RENDER_JITTER       0x2D4

/* Board player ball color table (set by Board_ctor Vec3_Init calls) */
#define BOARD_COLOR_BASE         0x3AB0   /* P1 RGBA (4 floats) */
#define BOARD_COLOR_STRIDE      0x14     /* 20 bytes per player */

/* Board goal-reached flag */
#define BOARD_GOAL_REACHED       0xCD0

/* MusicChannel offsets */
#define MUSIC_CHAN_BASS_CHANNEL  0x08
#define MUSIC_CHAN_VOLUME       0x528
#define MUSIC_CHAN_FADE_RATE    0x52C
#define MUSIC_CHAN_FADE_OUT     0x530
#define MUSIC_CHAN_FADE_IN      0x531

/* MusicDevice offsets */
#define MUSIC_DEV_CHANNEL_LIST  0x418

/* Proximity thresholds (matching vacuum system) */
#define WARP_XZ_DIST_SQ         900.0f  /* 30.0 squared */
#define WARP_Y_DIST             50.0f

/* Cooldown after warp completes */
#define WARP_COOLDOWN_MS        2000

/* ============================================================
 * Level name -> race index mapping
 * ============================================================ */

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

/* ============================================================
 * Warp state machine — time-based (GetTickCount), not frame-based
 * ============================================================ */

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

#define MAX_MUSIC_CHANNELS 8
static float g_musicOrigVolumes[MAX_MUSIC_CHANNELS];
static int g_musicChannelCount = 0;

#define RUMBLE_DURATION_MS    2000
#define FLASH_DURATION_MS      250
#define HOLD_DURATION_MS      1000
#define FADE_DURATION_MS      2000
#define REVEAL_DURATION_MS    1000
#define MUSIC_FADE_MS         3000

#define FLASH_PEAK_MS         125

static volatile float g_whiteAlpha = 0.0f;

static volatile int g_warpBall = 0;
static volatile int g_rumbleInit = 0;

/* Saved original ball color (P1) for restore after warp */
static float g_origBallR = 1.0f, g_origBallG = 1.0f, g_origBallB = 1.0f;
static int g_colorSaved = 0;

/* ============================================================
 * Memory helpers
 * ============================================================ */

static int GetApp(void) {
    return *(int *)APP_PTR;
}

static int is_valid_ball(int ball) {
    if (!ball) return 0;
    if (ball < 0x10000) return 0;
    return 1;
}

static int is_valid_ptr(int ptr) {
    if (!ptr) return 0;
    if (ptr < 0x10000) return 0;
    return 1;
}

/* ============================================================
 * Parse level name from WARP(LevelName)
 * ============================================================ */

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

/* ============================================================
 * Music fade — fade out music over MUSIC_FADE_MS (3 seconds)
 * ============================================================ */

static void startMusicFade(void) {
    int app = GetApp();
    int musicDev;
    int chanListData;
    int chanCount;
    int i;

    if (!app) return;
    musicDev = *(int *)((char *)app + APP_MUSIC_DEVICE_PTR);
    if (!musicDev) return;

    chanListData = *(int *)(musicDev + MUSIC_DEV_CHANNEL_LIST);
    chanCount = *(int *)(musicDev + 0x10);

    if (chanCount > 0 && chanListData) {
        int count = (chanCount < MAX_MUSIC_CHANNELS) ? chanCount : MAX_MUSIC_CHANNELS;
        g_musicChannelCount = count;
        for (i = 0; i < count; i++) {
            int chan = *(int *)(chanListData + i * 4);
            if (chan) {
                float vol = *(float *)(chan + MUSIC_CHAN_VOLUME);
                g_musicOrigVolumes[i] = vol;
            } else {
                g_musicOrigVolumes[i] = 0.0f;
            }
        }
    }

    g_musicFadeStarted = 1;
}

static void updateMusicFade(void) {
    int app, musicDev;
    int chanListData, chanCount, i;
    DWORD elapsed;
    float t;

    if (!g_musicFadeStarted) return;

    elapsed = GetTickCount() - g_warpStartTime;
    t = (float)elapsed / (float)MUSIC_FADE_MS;
    if (t > 1.0f) t = 1.0f;

    app = GetApp();
    if (!app) return;
    musicDev = *(int *)((char *)app + APP_MUSIC_DEVICE_PTR);
    if (!musicDev) return;

    chanListData = *(int *)(musicDev + MUSIC_DEV_CHANNEL_LIST);
    chanCount = *(int *)(musicDev + 0x10);

    if (chanCount > 0 && chanListData) {
        int count = (chanCount < MAX_MUSIC_CHANNELS) ? chanCount : MAX_MUSIC_CHANNELS;
        for (i = 0; i < count; i++) {
            int chan = *(int *)(chanListData + i * 4);
            if (chan) {
                float origVol = g_musicOrigVolumes[i];
                float chanVol = origVol * (1.0f - t);
                *(float *)(chan + MUSIC_CHAN_VOLUME) = chanVol;
                if (real_BASS_ChannelSetAttributes) {
                    int bassChan = *(int *)(chan + MUSIC_CHAN_BASS_CHANNEL);
                    if (bassChan) {
                        real_BASS_ChannelSetAttributes(bassChan, -1.0f, (int)(chanVol * 100.0f), -1);
                    }
                }
            }
        }
    }
}

/* Scene offset for the game's native fade alpha. */
#define SCENE_FADE_ALPHA        0x3624

/* ============================================================
 * Node proximity scanner — checks MeshWorld Section 1 nodes
 * for WARP(Name) entries and triggers warp when ball is close.
 *
 * Access pattern (from decompiled Scene_SetupLevelUp @ 0x411540
 * and CollisionFace_Update @ 0x43D160):
 *   board+0x8AC  → MeshWorld*
 *   MW+0x480     → section3 data*
 *   section3+0x898 = node count
 *   section3+0xCA0 = data array (array of node entry pointers)
 *   Each node: [char* name, float x, float y, float z]
 *
 * Ball list:
 *   board+0x2DE0 = ball data array ptr
 *   First entry = player ball pointer
 * ============================================================ */

static void scanWarpNodes(void) {
    int app, profile, board, meshWorld, section3;
    int nodeCount, nodeDataArray;
    int ballDataArray, ball;
    float ballX, ballY, ballZ;
    int i;

    if (g_phase != PHASE_IDLE) return;

    /* Cooldown after warp completes */
    if (GetTickCount() < g_cooldownUntil) return;

    app = GetApp();
    if (!app) return;

    profile = *(int *)((char *)app + APP_PROFILE_PTR);
    if (!profile) return;

    board = *(int *)((char *)profile + PROFILE_BOARD_PTR);
    if (!board) return;

    /* Don't trigger if goal already reached */
    if (*((char *)board + BOARD_GOAL_REACHED) != 0) return;

    /* Get MeshWorld pointer */
    meshWorld = *(int *)((char *)board + BOARD_MESHWORLD_PTR);
    if (!is_valid_ptr(meshWorld)) return;

    /* Get section3 data pointer */
    section3 = *(int *)((char *)meshWorld + MW_SECTION3_PTR);
    if (!is_valid_ptr(section3)) return;

    /* Get node list */
    nodeCount = *(int *)((char *)section3 + MW_NODELIST_COUNT);
    nodeDataArray = *(int *)((char *)section3 + MW_NODELIST_DATA);
    if (nodeCount <= 0 || !is_valid_ptr(nodeDataArray)) return;
    if (nodeCount > 256) nodeCount = 256;  /* Safety cap */

    /* Get player ball from ball list data array */
    ballDataArray = *(int *)((char *)board + BALL_LIST_DATA_OFFSET);
    if (!is_valid_ptr(ballDataArray)) return;
    ball = *(int *)((char *)ballDataArray);  /* First ball = player */
    if (!is_valid_ball(ball)) return;

    /* Check death_pending */
    if (*((char *)ball + BALL_DEATH_PENDING) != 0) return;

    /* Get ball position */
    ballX = *(float *)((char *)ball + BALL_POS_X);
    ballY = *(float *)((char *)ball + BALL_POS_Y);
    ballZ = *(float *)((char *)ball + BALL_POS_Z);

    /* Iterate MeshWorld Section 1 nodes */
    for (i = 0; i < nodeCount; i++) {
        int entry = *(int *)((char *)nodeDataArray + i * 4);
        const char *name;

        if (!is_valid_ptr(entry)) continue;

        name = *(const char **)((char *)entry);
        if (!name) continue;

        if (isWarpNode(name)) {
            float nodeX = *(float *)((char *)entry + NODE_X_OFFSET);
            float nodeY = *(float *)((char *)entry + NODE_Y_OFFSET);
            float nodeZ = *(float *)((char *)entry + NODE_Z_OFFSET);

            float dx = ballX - nodeX;
            float dy = ballY - nodeY;
            float dz = ballZ - nodeZ;
            float xzDistSq = dx * dx + dz * dz;
            float absDy = (dy < 0.0f) ? -dy : dy;

            if (xzDistSq < WARP_XZ_DIST_SQ && absDy < WARP_Y_DIST) {
                /* Close enough! Parse target and trigger warp */
                char levelName[128];
                if (parseWarpLevel(name, levelName, sizeof(levelName))) {
                    int raceIndex = findRaceIndex(levelName);
                    diag_logf("[WARP] Node \"%s\" at (%.1f, %.1f, %.1f) — ball at (%.1f, %.1f, %.1f), xzDistSq=%.1f",
                              name, nodeX, nodeY, nodeZ, ballX, ballY, ballZ, xzDistSq);
                    if (raceIndex > 0) {
                        /* App_StartPracticeRace expects 0-based index (0-14) */
                        g_warpLevelIndex = raceIndex - 1;
                        g_phase = PHASE_RUMBLE;
                        g_rumbleInit = 0;
                        g_colorSaved = 0;
                        g_warpBall = ball;
                        {
                            DWORD now = GetTickCount();
                            g_phaseStartTime = now;
                            g_warpStartTime = now;
                        }
                        g_whiteAlpha = 0.0f;
                        g_musicFadeStarted = 0;
                        diag_logf("[WARP] Triggered! Starting effect sequence, level=%d", raceIndex);
                    } else {
                        diag_logf("[WARP] Unknown level name \"%s\" in node \"%s\"", levelName, name);
                    }
                }
                return;  /* Only trigger one warp per frame */
            }
        }
    }
}

/* ============================================================
 * App_FrameUpdate epilogue hook
 * ============================================================ */

#define APP_FRAME_UPDATE_EPILOGUE 0x0046C1F1
static unsigned char *g_frameUpdateDetour = NULL;

/* ============================================================
 * Warp state machine — called every frame from FrameUpdateHandler
 * ============================================================ */

static void updateWarpStateMachine(void) {
    int app;
    int board = 0;
    int ball;
    DWORD now;
    DWORD elapsed;

    if (g_phase == PHASE_IDLE) return;

    now = GetTickCount();

    app = GetApp();
    if (!app) {
        diag_log("[warp] App null during warp, aborting");
        g_phase = PHASE_IDLE;
        return;
    }

    ball = g_warpBall;
    if (!is_valid_ball(ball)) {
        if (ball) {
            diag_logf("[warp] g_warpBall 0x%08X invalid — continuing without ball", ball);
        }
        ball = 0;
    }

    {
        int profile = *(int *)((char *)app + APP_PROFILE_PTR);
        if (profile) {
            board = *(int *)((char *)profile + PROFILE_BOARD_PTR);
        }
    }
    if (!board) board = 0;

    /* Write fade alpha to board's native fade field */
    if (board) {
        *(float *)((char *)board + SCENE_FADE_ALPHA) = g_whiteAlpha;
    }

    switch (g_phase) {
    case PHASE_RUMBLE: {
        elapsed = now - g_phaseStartTime;

        if (!g_rumbleInit && ball) {
            g_rumbleInit = 1;
            /* Only disable player steering — don't set in_tar (ball+0x2CC)
             * which adds drag/stickiness. ball+0x808 alone blocks
             * Ball_ApplyForce so the player can't steer, but physics
             * (gravity, collisions, rolling) continue normally. */
            *(int *)((char *)ball + BALL_IMPACT_FREEZE) = 1000;
            *(char *)((char *)ball + BALL_RENDER_JITTER) = 1;
            startMusicFade();
            /* Save original P1 ball color from board color table */
            if (board && !IsBadReadPtr((void*)(board + BOARD_COLOR_BASE), 16)) {
                g_origBallR = *(float*)(board + BOARD_COLOR_BASE);
                g_origBallG = *(float*)(board + BOARD_COLOR_BASE + 4);
                g_origBallB = *(float*)(board + BOARD_COLOR_BASE + 8);
                g_colorSaved = 1;
            }
            diag_logf("[warp] PHASE_RUMBLE start: steering disabled (ball+0x808=1000), jitter on");
        }

        /* Fade ball color from original to yellow over RUMBLE phase.
         * Yellow = (1.0, 1.0, 0.0). By the time FLASH starts, ball is fully yellow. */
        if (g_colorSaved && board && !IsBadWritePtr((void*)(board + BOARD_COLOR_BASE), 12)) {
            float t = (float)elapsed / (float)RUMBLE_DURATION_MS;
            if (t > 1.0f) t = 1.0f;
            float r = g_origBallR + (1.0f - g_origBallR) * t;
            float g = g_origBallG + (1.0f - g_origBallG) * t;
            float b = g_origBallB + (0.0f - g_origBallB) * t;
            *(float*)(board + BOARD_COLOR_BASE)     = r;
            *(float*)(board + BOARD_COLOR_BASE + 4) = g;
            *(float*)(board + BOARD_COLOR_BASE + 8) = b;
        }

        /* Fade ball alpha from 1.0 (opaque) to 0.5 (half transparent)
         * over RUMBLE phase. The ball becomes progressively more ghostly. */
        if (ball) {
            int respawning = *((unsigned char *)((char *)ball + 0x2F9));
            if (!respawning) {
                float t = (float)elapsed / (float)RUMBLE_DURATION_MS;
                if (t > 1.0f) t = 1.0f;
                *(float *)((char *)ball + BALL_ALPHA) = 1.0f - (0.5f * t);
            }
        }

        updateMusicFade();

        if (elapsed >= RUMBLE_DURATION_MS) {
            g_phase = PHASE_FLASH;
            g_phaseStartTime = now;
            diag_logf("[warp] -> PHASE_FLASH (rumble ran %lums)", (unsigned long)elapsed);
        }
        break;
    }

    case PHASE_FLASH: {
        elapsed = now - g_phaseStartTime;

        if (elapsed < FLASH_PEAK_MS) {
            g_whiteAlpha = (float)elapsed / (float)FLASH_PEAK_MS;
        } else {
            DWORD remaining = FLASH_DURATION_MS - elapsed;
            g_whiteAlpha = (float)remaining / (float)(FLASH_DURATION_MS - FLASH_PEAK_MS);
            if (g_whiteAlpha < 0.0f) g_whiteAlpha = 0.0f;
        }

        /* Turn ball invisible ONLY when the screen is fully white (at peak).
         * Before that, the ball should still be visible — the flash ramps up
         * over the ball, hiding the disappearance behind the white screen. */
        if (g_whiteAlpha >= 0.99f && ball) {
            int respawning = *((unsigned char *)((char *)ball + 0x2F9));
            if (!respawning) {
                *(float *)((char *)ball + BALL_ALPHA) = 0.0f;
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
            int respawning = *((unsigned char *)((char *)ball + 0x2F9));
            if (!respawning) {
                *(float *)((char *)ball + BALL_ALPHA) = 0.0f;
            }
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
            int respawning = *((unsigned char *)((char *)ball + 0x2F9));
            if (!respawning) {
                *(float *)((char *)ball + BALL_ALPHA) = 0.0f;
            }
        }

        updateMusicFade();

        if (elapsed >= FADE_DURATION_MS) {
            g_whiteAlpha = 1.0f;
            g_phase = PHASE_LOAD;
            g_phaseStartTime = now;
            diag_log("[warp] -> PHASE_LOAD: loading level");
        }
        break;
    }

    case PHASE_LOAD: {
        int levelIdx = g_warpLevelIndex;
        g_warpLevelIndex = -1;

        /* Clear ball freeze + jitter flags before loading */
        if (ball) {
            *(int *)((char *)ball + BALL_IMPACT_FREEZE) = 0;
            *(char *)((char *)ball + BALL_RENDER_JITTER) = 0;
            *(float *)((char *)ball + BALL_ALPHA) = 1.0f;
        }

        if (levelIdx >= 0 && levelIdx <= 14) {
            void *func = (void *)APP_START_PRACTICE_RACE;
            int appVal = app;
            int idx = levelIdx;
            char savedDifficulty = *((char *)app + 0x23C);
            int oldProfile = *(int *)((char *)app + APP_PROFILE_PTR);

            char wasInTournament = 0;
            float savedScores[16];
            int savedTimes[16];
            int hasTournamentData = 0;
            int savedTimeRemaining = 0;
            int savedPlayerScore = 0;

            if (oldProfile) {
                char isPractice = *((char *)oldProfile + PROFILE_IS_PRACTICE);
                wasInTournament = (isPractice == 0) ? 1 : 0;
                if (wasInTournament) {
                    int raceIdx = *(int *)((char *)oldProfile + 0x08);
                    int oldBoard = *(int *)((char *)oldProfile + 0x0C);
                    hasTournamentData = 1;
                    memcpy(savedScores, (void *)((char *)oldProfile + PROFILE_SCORE_ARRAY), sizeof(savedScores));
                    memcpy(savedTimes, (void *)((char *)oldProfile + PROFILE_TIME_ARRAY), sizeof(savedTimes));
                    if (raceIdx >= 0 && raceIdx < 16) {
                        savedScores[raceIdx] = *(float *)((char *)app + 0x5E8);
                        diag_logf("[warp] Saved race score %f at index %d", savedScores[raceIdx], raceIdx);
                    }
                    if (oldBoard) {
                        savedTimeRemaining = *(int *)((char *)oldBoard + 0x2990);
                        diag_logf("[warp] Saved time remaining: %d", savedTimeRemaining);
                    }
                    savedPlayerScore = *(int *)((char *)app + 0x5E4);
                    diag_logf("[warp] Saved player score: %d", savedPlayerScore);
                }
            }

            diag_logf("[warp] App_StartPracticeRace(app=0x%08X, level=%d, difficulty=%d, tourney=%d)",
                       appVal, idx, (int)savedDifficulty, (int)wasInTournament);

            __asm__ volatile (
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

            *((char *)app + 0x23C) = savedDifficulty;

            if (wasInTournament) {
                {
                    int bttRec = *(int *)((char *)app + APP_BTT_RECORDING);
                    int bttPlay = *(int *)((char *)app + APP_BTT_PLAYBACK);
                    if (bttRec) {
                        *(int *)((char *)app + APP_BTT_RECORDING) = 0;
                        diag_log("[warp] Cleared BTT recording pointer (tournament mode)");
                    }
                    if (bttPlay) {
                        *(int *)((char *)app + APP_BTT_PLAYBACK) = 0;
                        diag_log("[warp] Cleared BTT playback pointer (tournament mode)");
                    }
                }

                {
                    int newProfile = *(int *)((char *)app + APP_PROFILE_PTR);
                    if (newProfile) {
                        *((char *)newProfile + PROFILE_IS_PRACTICE) = 0;
                        if (hasTournamentData) {
                            memcpy((void *)((char *)newProfile + PROFILE_SCORE_ARRAY), savedScores, sizeof(savedScores));
                            memcpy((void *)((char *)newProfile + PROFILE_TIME_ARRAY), savedTimes, sizeof(savedTimes));
                        }
                        diag_log("[warp] Tournament mode restored: practice=0, scores copied to new profile");

                        if (savedTimeRemaining > 0) {
                            int newBoard = *(int *)((char *)newProfile + 0x0C);
                            if (newBoard) {
                                *(int *)((char *)newBoard + 0x2990) = savedTimeRemaining;
                                *(int *)((char *)newBoard + 0x2994) = savedTimeRemaining;
                                *(int *)((char *)app + 0x5E8) = savedTimeRemaining;
                                *(int *)((char *)app + 0x5EC) = 0;
                                diag_logf("[warp] Timer carried over: %d ticks remaining", savedTimeRemaining);
                            }
                        }
                        *(int *)((char *)app + 0x5E4) = savedPlayerScore;
                        diag_logf("[warp] Player score restored: %d", savedPlayerScore);
                    }
                }
            }

            diag_log("[warp] Level loaded OK");
        } else {
            diag_logf("[warp] Invalid level index %d, aborting", levelIdx);
        }

        g_warpBall = 0;

        g_whiteAlpha = 1.0f;
        g_phase = PHASE_REVEAL;
        g_phaseStartTime = now;
        g_musicFadeStarted = 0;
        diag_log("[warp] -> PHASE_REVEAL: fading from white");
        break;
    }

    case PHASE_REVEAL: {
        elapsed = now - g_phaseStartTime;
        g_whiteAlpha = 1.0f - (float)elapsed / (float)REVEAL_DURATION_MS;
        if (g_whiteAlpha < 0.0f) g_whiteAlpha = 0.0f;

        if (elapsed >= REVEAL_DURATION_MS) {
            g_whiteAlpha = 0.0f;
            g_phase = PHASE_IDLE;
            g_cooldownUntil = GetTickCount() + WARP_COOLDOWN_MS;
            diag_log("[warp] -> PHASE_IDLE: warp complete (2s cooldown)");
        }
        break;
    }

    default:
        g_phase = PHASE_IDLE;
        break;
    }
}

/* ============================================================
 * FrameUpdateHandler — called from App_FrameUpdate epilogue detour
 * ============================================================ */

static void FrameUpdateHandler(void) {
    if (g_phase == PHASE_IDLE) {
        scanWarpNodes();
    }
    updateWarpStateMachine();
}

/* ============================================================
 * Install hooks — only FrameUpdate epilogue (no collision hook)
 * ============================================================ */

static void InstallFrameUpdateHook(void) {
    unsigned char *epiAddr = (unsigned char *)APP_FRAME_UPDATE_EPILOGUE;
    DWORD oldProtect;

    if (epiAddr[0] != 0x5E || epiAddr[1] != 0x83 || epiAddr[2] != 0xC4 ||
        epiAddr[3] != 0x08 || epiAddr[4] != 0xC3) {
        diag_log("[FATAL] FrameUpdate epilogue signature mismatch!");
        return;
    }
    diag_log("[InstallHooks] FrameUpdate epilogue signature OK");

    g_frameUpdateDetour = (unsigned char *)VirtualAlloc(NULL, 256, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (!g_frameUpdateDetour) return;

    {
        unsigned char *p = g_frameUpdateDetour;
        *p++ = 0x56;  /* PUSH ESI */
        *p++ = 0xB8; *(DWORD *)p = (DWORD)&FrameUpdateHandler; p += 4;
        *p++ = 0xFF; *p++ = 0xD0;  /* CALL EAX */
        *p++ = 0x5E;  /* POP ESI */
        *p++ = 0x5E;  /* original: POP ESI */
        *p++ = 0x83; *p++ = 0xC4; *p++ = 0x08;  /* ADD ESP, 0x8 */
        *p++ = 0xC3;  /* RET */
    }

    VirtualProtect(epiAddr, 5, PAGE_EXECUTE_READWRITE, &oldProtect);
    epiAddr[0] = 0xE9;
    *(DWORD *)(epiAddr + 1) = (DWORD)g_frameUpdateDetour - (DWORD)(epiAddr + 5);
    VirtualProtect(epiAddr, 5, oldProtect, &oldProtect);

    diag_log("[InstallHooks] FrameUpdate epilogue hook installed");
}

/* ============================================================
 * Init thread
 * ============================================================ */

static DWORD WINAPI InitThread(LPVOID param) {
    (void)param;
    Sleep(2000);

    InstallFrameUpdateHook();

    diag_log("[warp mod v7] Hooks installed. Scanning MeshWorld nodes for WARP(Name) entries.");
    return 0;
}

/* ============================================================
 * DllMain
 * ============================================================ */

BOOL APIENTRY DllMain(HMODULE hInst, DWORD reason, LPVOID lpReserved) {
    (void)lpReserved;
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hInst);

        {
            char mod_path[MAX_PATH];
            if (GetModuleFileNameA(hInst, mod_path, MAX_PATH)) {
                char *p = strrchr(mod_path, '\\');
                if (p) {
                    strcpy(p + 1, "warp_log.txt");
                    strncpy(g_logPath, mod_path, MAX_PATH - 1);
                }
            }
        }

        diag_log("=== LEVEL WARP MOD v7 LOADED ===");
        diag_log("v7: Node-point proximity trigger (no collision hook). WARP(Name) in MESHWORLD Section 1.");

        load_real_bass();
        diag_logf("bass_real.dll handle: 0x%08X", (unsigned)g_hRealBass);

        CreateThread(NULL, 0, InitThread, NULL, 0, NULL);
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
