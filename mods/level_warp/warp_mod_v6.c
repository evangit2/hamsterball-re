/*
 * E:WARP (Level Warp) Mod v6f
 *
 * v6d fixes (found in code review):
 *   - Removed dead Graphics_PresentOrEnd hook — was patching 7 bytes of game
 *     code + allocating detour buffer + executing per-frame detour for a no-op
 *     function. Overlay is handled via scene+0x3624 native fade.
 *   - Removed dead setWinState() function (never called)
 *   - Removed dead g_trampoline buffer (filled but never referenced)
 *   - Removed dead g_diagCounter variable
 *   - Removed all dead D3D8 constants, TLVertex struct, player-state defines
 *     (only used by deleted functions)
 *   - Fixed uninitialized 'board' variable — was UB if profile was null
 *   - Fixed misleading log messages referencing v6c features that don't exist
 *
 * v6e fixes:
 *   - Replaced sin/cos position oscillation + game pause with the game's
 *     native render jitter system (ball+0x2D4=1). This is the EXACT same
 *     mechanism the Up Race vacuum (CollisionFace_Update, 0x43D160) uses
 *     to make the ball vibrate — CPUID-based random jitter applied in the
 *     render function (FUN_00403DB8). No pausing, no manual position writes.
 *   - Removed dead g_ballOrigX/Y/Z globals and #include <math.h>.
 *   - Removed board+0x874 pause/unpause — no longer needed since we don't
 *     need to freeze Ball_Update to write positions.
 *   - Renamed ball+0x2D4 to BALL_RENDER_JITTER across all code/docs.
 *
 * v6 fixes (found in prior code review):
 *   - CRITICAL: Race index off-by-one — findRaceIndex returns 1-based but
 *     App_StartPracticeRace expects 0-based (0-14). Game subtracts 1 before
 *     calling. Now converts to 0-based. v5 loaded wrong level every time
 *     and crashed on level 15.
 *   - CRITICAL: DrawPrimitiveUP used TRIANGLELIST(4) with 4 vertices for 2
 *     primitives — needs 6 verts, only provided 4. Switched to TRIANGLESTRIP(5)
 *     which correctly needs 4 verts for 2 triangles.
 *   - CRITICAL: FVF never set before DrawPrimitiveUP — D3D used whatever FVF
 *     the game had (likely includes texture coords not in TLVertex). Now saves
 *     old FVF via GetVertexShader, sets D3DFVF_TLVERTEX, restores after.
 *   - CRITICAL: RUMBLE phase skipped — g_phaseStartTime was 0 on first frame,
 *     so elapsed=GetTickCount()-0=system uptime >> 2000ms. Phase ended in 1
 *     frame. Now initializes timestamps in WarpCollisionHandler before setting
 *     phase.
 *   - Fixed: inline asm clobber list included "esp" — tells GCC stack pointer
 *     is destroyed, can break prologue/epilogue. Removed.
 *   - Cleaned: Removed dead first-frame block in PHASE_FLASH and TODO comments.
 *
 * v5 fixes:
 *   - Restores D3D texture stage states after overlay draw
 *   - GetTickCount()-based timing (framerate-independent)
 *   - vsnprintf replaces wvsprintfA (supports %f)
 *   - Added PHASE_REVEAL: fades FROM white after level loads
 *   - FADE duration increased to 2 seconds
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
 *   i686-w64-mingw32-gcc -shared -o bass.dll warp_mod_v6.c -lwinmm \
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
    vsnprintf(buf, sizeof(buf), fmt, args);  /* vsnprintf supports %f! */
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
#define DISPATCH_COLLISION_EVENTS   0x0040C5D0
#define APP_START_PRACTICE_RACE     0x00428C50
#define BTT_DTOR_VTABLE_OFFSET      0x00   /* vtable[0] = dtor (verified from Ghidra) */

/* App offsets */
#define APP_PROFILE_PTR          0x220
#define APP_MUSIC_DEVICE_PTR     0x17C
#define APP_BTT_RECORDING        0x90C  /* BestTimeTracker* — ghost recording (TT only) */
#define APP_BTT_PLAYBACK         0x910  /* BestTimeTracker* — ghost playback (TT only) */

/* Board offsets */
#define BOARD_SCENE_PTR_OFFSET   0x878

/* PlayerProfile offsets */
#define PROFILE_RACE_INDEX       0x08
#define PROFILE_BOARD_PTR        0x0C
#define PROFILE_IS_TOURNAMENT    0x10  /* byte: 1=tournament, 0=practice */
#define PROFILE_IS_PRACTICE      0x11  /* byte: 1=practice/TT, 0=tournament */
#define PROFILE_SCORE_ARRAY      0x14  /* float[16] — per-race scores */
#define PROFILE_TIME_ARRAY       0x50  /* int[16] — per-race times (ticks) */

/* Ball offsets */
#define BALL_DEATH_PENDING       0x2E9
#define BALL_POS_X               0x164
#define BALL_POS_Y               0x168
#define BALL_POS_Z               0x16C
#define BALL_IMPACT_FREEZE       0x808
#define BALL_IN_TAR              0x2CC
#define BALL_ALPHA               0x2FC
#define BALL_VTABLE              0x4CF3A0
#define BALL_RENDER_JITTER       0x2D4   /* byte: set by vacuum (CollisionFace_Update
                                          * 0x43D160) to enable CPUID-based random
                                          * jitter in the ball render function
                                          * (FUN_00403DB8). When =1, the render
                                          * function adds random offsets to both
                                          * the animation pose and the render
                                          * position, making the ball and hamster
                                          * model visibly shake. Cleared on
                                          * vacuum release. */

/* Collision event pair offsets */
#define COLL_OBJ_NAME_OFFSET     0x864

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
static volatile DWORD g_phaseStartTime = 0;   /* GetTickCount() when phase started */
static volatile DWORD g_warpStartTime = 0;     /* GetTickCount() when warp started */
static volatile int g_warpLevelIndex = -1;
static volatile int g_musicFadeStarted = 0;

/* Per-channel original volumes — each channel fades from its own
 * starting volume rather than all using a single value. */
#define MAX_MUSIC_CHANNELS 8
static float g_musicOrigVolumes[MAX_MUSIC_CHANNELS];
static int g_musicChannelCount = 0;

/* Phase durations in milliseconds (framerate-independent!) */
#define RUMBLE_DURATION_MS    2000   /* 2.0 sec */
#define FLASH_DURATION_MS      250   /* 0.25 sec */
#define HOLD_DURATION_MS      1000   /* 1.0 sec — pause between flash and fade */
#define FADE_DURATION_MS      2000   /* 2.0 sec */
#define REVEAL_DURATION_MS    1000   /* 1.0 sec */
#define MUSIC_FADE_MS         3000   /* 3.0 sec */

/* Flash sub-timing */
#define FLASH_PEAK_MS         125   /* peak white at 125ms into 250ms flash */

/* White screen alpha (0.0 = transparent, 1.0 = fully white) */
static volatile float g_whiteAlpha = 0.0f;

/* Ball pointer saved at warp trigger */
static volatile int g_warpBall = 0;
static volatile int g_rumbleInit = 0;

/* ============================================================
 * Memory helpers
 * ============================================================ */

static int GetApp(void) {
    return *(int *)APP_PTR;
}

/* Validate ball pointer — simple null + sanity check.
 * The vtable check was rejecting valid balls on real Windows
 * (cause unknown — possibly different vtable for player ball vs
 * entity ball, or vtable gets swapped during collision processing).
 * Removed vtable check to restore working behavior from v6e. */
static int is_valid_ball(int ball) {
    if (!ball) return 0;
    if (ball < 0x10000) return 0;  /* Reject obviously bogus pointers */
    return 1;
}

/* ============================================================
 * Parse level name from E:WARP(LevelName)
 * ============================================================ */

static int isWarpEvent(const char *eventName) {
    if (!eventName) return 0;
    return _strnicmp(eventName, "E:WARP", 6) == 0;
}

static int parseWarpLevel(const char *eventName, char *outLevelName, int outSize) {
    const char *start = strchr(eventName, '(');
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
                /* Do NOT set fade_out flag — game's fade system cuts volume too fast.
                 * We manually control volume in updateMusicFade(). */
                diag_logf("[music] Channel %d: origVol=%.3f (manual per-channel fade)",
                          i, vol);
            } else {
                g_musicOrigVolumes[i] = 0.0f;
            }
        }
    }

    g_musicFadeStarted = 1;
}

/* Per-frame music volume update */
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

/* Scene offset for the game's native fade alpha.
 * The board render function (FUN_0041b710) reads scene+0x3624 and draws
 * a fullscreen rect with that alpha via Graphics_DrawScreenRect.
 * By writing our fade alpha here, the game draws the overlay itself —
 * no D3D state corruption, no BeginScene/EndScene issues. */
#define SCENE_FADE_ALPHA        0x3624

/* ============================================================
 * App_FrameUpdate epilogue hook
 * ============================================================ */

#define APP_FRAME_UPDATE_EPILOGUE 0x0046C1F1
static unsigned char *g_frameUpdateDetour = NULL;

/* ============================================================
 * Warp state machine — called every frame from FrameUpdateHandler
 * Uses GetTickCount() for framerate-independent timing.
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

    /* Use ball pointer saved from collision event — scene+0x29D0 is not always set.
     * Validate via vtable check: if the ball was destroyed mid-warp (e.g. fell
     * off the level despite freeze), the pointer is stale. Setting ball=0 lets
     * the state machine continue advancing through phases without ball writes,
     * so the level still loads. */
    ball = g_warpBall;
    if (!is_valid_ball(ball)) {
        if (ball) {
            diag_logf("[warp] g_warpBall 0x%08X invalid (freed or vtable mismatch) — continuing without ball", ball);
        }
        ball = 0;
    }

    /* Still need board/scene for BOARD_GOAL_REACHED etc, but don't block on ball */
    {
        int profile = *(int *)((char *)app + APP_PROFILE_PTR);
        if (profile) {
            board = *(int *)((char *)profile + PROFILE_BOARD_PTR);
        }
    }
    if (!board) board = 0;  /* May be null — phases that need it will skip */

    /* Write fade alpha to board's native fade field (board+0x3624).
     * The game's own board render function (FUN_0041b710) reads this every
     * frame and draws a fullscreen rect through its own material pipeline.
     * NOTE: board IS the scene — they are the same object. board+0x878 is the
     * App back-pointer, NOT a scene pointer. Writing to board+0x878+0x3624
     * would corrupt App memory. Write directly to board+0x3624. */
    if (board) {
        *(float *)((char *)board + SCENE_FADE_ALPHA) = g_whiteAlpha;
    }

    switch (g_phase) {
    case PHASE_RUMBLE: {
        elapsed = now - g_phaseStartTime;

        /* On first frame: set ball+0x2D4=1 (render jitter flag), freeze ball,
         * and start music fade.
         *
         * ball+0x2D4 is the vacuum's render jitter flag. When set to 1, the
         * ball's render function (FUN_00403DB8 at vtable[1]) adds CPUID-based
         * random offsets to both the animation pose and the render position
         * every frame. This is the EXACT same mechanism the Up Race vacuum
         * (CollisionFace_Update, 0x43D160) uses to make the ball shake.
         *
         * We also set ball+0x808=1000 (impact_freeze) and ball+0x2CC=1
         * (in_tar) to freeze the ball's physics, matching the vacuum's
         * capture behavior. This prevents the ball from moving while it
         * vibrates.
         *
         * Unlike the old approach (sin/cos position oscillation + game pause),
         * this uses the game's OWN render jitter system — no pausing needed,
         * no manual position writes, and the vibration is identical to what
         * the vacuum produces. */
        if (!g_rumbleInit && ball) {
            g_rumbleInit = 1;
            *(int *)((char *)ball + BALL_IMPACT_FREEZE) = 1000;
            *(char *)((char *)ball + BALL_IN_TAR) = 1;
            *(char *)((char *)ball + BALL_RENDER_JITTER) = 1;
            startMusicFade();
            diag_logf("[warp] PHASE_RUMBLE start: render jitter enabled (ball+0x2D4=1)");
        }

        updateMusicFade();

        if (elapsed >= RUMBLE_DURATION_MS) {
            g_phase = PHASE_FLASH;
            g_phaseStartTime = now;
            diag_logf("[warp] -> PHASE_FLASH (rumble ran %lums, init=%d)", (unsigned long)elapsed, g_rumbleInit);
        }
        break;
    }

    case PHASE_FLASH: {
        elapsed = now - g_phaseStartTime;

 /* Quick flash: ramp up to peak, then back down */
        if (elapsed < FLASH_PEAK_MS) {
            g_whiteAlpha = (float)elapsed / (float)FLASH_PEAK_MS;
        } else {
            DWORD remaining = FLASH_DURATION_MS - elapsed;
            g_whiteAlpha = (float)remaining / (float)(FLASH_DURATION_MS - FLASH_PEAK_MS);
            if (g_whiteAlpha < 0.0f) g_whiteAlpha = 0.0f;
        }

        /* Force ball invisible every frame — game overwrites alpha to 1.0
         * each frame in its render pass, so a single write is immediately
         * undone. Must continuously force 0.0 while flash is active.
         * Skip during respawn fade-in (ball+0x2F9) like freecam does. */
        if (ball) {
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

        /* Do nothing — just wait. Screen is clear, ball stays invisible. */
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

        /* Screen fades from 0 to solid white over 2 seconds */
        g_whiteAlpha = (float)elapsed / (float)FADE_DURATION_MS;
        if (g_whiteAlpha > 1.0f) g_whiteAlpha = 1.0f;

        /* Keep ball invisible during fade too */
        if (ball) {
            int respawning = *((unsigned char *)((char *)ball + 0x2F9));
            if (!respawning) {
                *(float *)((char *)ball + BALL_ALPHA) = 0.0f;
            }
        }

        updateMusicFade();

        if (elapsed >= FADE_DURATION_MS) {
            g_whiteAlpha = 1.0f;  /* Full white */
            g_phase = PHASE_LOAD;
            g_phaseStartTime = now;
            diag_log("[warp] -> PHASE_LOAD: loading level");
        }
        break;
    }

    case PHASE_LOAD: {
        /* Load the target level while screen stays white */
        int levelIdx = g_warpLevelIndex;
        g_warpLevelIndex = -1;

        /* Clear ball freeze + jitter flags before loading */
        if (ball) {
            *(int *)((char *)ball + BALL_IMPACT_FREEZE) = 0;
            *(char *)((char *)ball + BALL_IN_TAR) = 0;
            *(char *)((char *)ball + BALL_RENDER_JITTER) = 0;
            *(float *)((char *)ball + BALL_ALPHA) = 1.0f;
        }

        /* Load the target level while screen stays white.
         *
         * TOURNAMENT MODE FIX (v6f):
         * App_StartPracticeRace creates a new PlayerProfile with practice=1
         * and a fresh BestTimeTracker for ghost recording. This breaks
         * tournament mode in three ways:
         *   1. profile+0x11=1 causes Tournament_AdvanceRace to skip scoring
         *   2. A BTT at App+0x90C makes the game think it's in TT mode
         *   3. The old profile's score/time arrays are lost
         *
         * Fix: Save the old profile's tournament flags + score/time arrays
         * before the call. After the call, free the BTTs (matching
         * App_StartTournamentRace behavior), copy scores to the new profile,
         * and set the correct mode flags.
         */
        if (levelIdx >= 0 && levelIdx <= 14) {
            void *func = (void *)APP_START_PRACTICE_RACE;
            int appVal = app;
            int idx = levelIdx;
            char savedDifficulty = *((char *)app + 0x23C);
            int oldProfile = *(int *)((char *)app + APP_PROFILE_PTR);

            /* Detect tournament mode: profile+0x11=0 means tournament,
             * profile+0x11=1 means practice/TT. App_StartPracticeRace
             * forces +0x11=1, which is what kicks us out of tournament.
             * profile+0x10 is always 0 (unused flag). */
            char wasInTournament = 0;
            float savedScores[16];
            int savedTimes[16];
            int hasTournamentData = 0;

            if (oldProfile) {
                char isPractice = *((char *)oldProfile + PROFILE_IS_PRACTICE);
                wasInTournament = (isPractice == 0) ? 1 : 0;
                if (wasInTournament) {
                    hasTournamentData = 1;
                    memcpy(savedScores, (void *)((char *)oldProfile + PROFILE_SCORE_ARRAY), sizeof(savedScores));
                    memcpy(savedTimes, (void *)((char *)oldProfile + PROFILE_TIME_ARRAY), sizeof(savedTimes));
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

            /* Restore difficulty so entity factories spawn correctly */
            *((char *)app + 0x23C) = savedDifficulty;

            if (wasInTournament) {
                /* Free the BestTimeTrackers that App_StartPracticeRace created.
                 * App_StartTournamentRace does this:
                 *   if (App+0x90C) { (*(vtable[0])(App+0x90C))(1); App+0x90C=0; }
                 *   if (App+0x910) { (*(vtable[0])(App+0x910))(1); App+0x910=0; }
                 * The dtor is __thiscall: ECX=this, param on stack, RET 0x4.
                 * Using inline asm to ensure correct calling convention. */
                {
                    int bttRec = *(int *)((char *)app + APP_BTT_RECORDING);
                    int bttPlay = *(int *)((char *)app + APP_BTT_PLAYBACK);
                    if (bttRec) {
                        int vtable = *(int *)bttRec;
                        int dtor = *(int *)vtable;
                        __asm__ volatile (
                            "push 1\n\t"
                            "movl %[btt], %%ecx\n\t"
                            "call *%[dtor]\n\t"
                            :
                            : [dtor] "r" (dtor),
                              [btt] "r" (bttRec)
                            : "eax", "edx", "ecx", "memory"
                        );
                        *(int *)((char *)app + APP_BTT_RECORDING) = 0;
                        diag_log("[warp] Freed BTT recording (tournament mode)");
                    }
                    if (bttPlay) {
                        int vtable = *(int *)bttPlay;
                        int dtor = *(int *)vtable;
                        __asm__ volatile (
                            "push 1\n\t"
                            "movl %[btt], %%ecx\n\t"
                            "call *%[dtor]\n\t"
                            :
                            : [dtor] "r" (dtor),
                              [btt] "r" (bttPlay)
                            : "eax", "edx", "ecx", "memory"
                        );
                        *(int *)((char *)app + APP_BTT_PLAYBACK) = 0;
                        diag_log("[warp] Freed BTT playback (tournament mode)");
                    }
                }

                /* Restore tournament flags + score/time arrays on new profile.
                 * Tournament mode = profile+0x11=0 (not practice).
                 * profile+0x10 is always 0 — leave it alone. */
                {
                    int newProfile = *(int *)((char *)app + APP_PROFILE_PTR);
                    if (newProfile) {
                        *((char *)newProfile + PROFILE_IS_PRACTICE) = 0;
                        if (hasTournamentData) {
                            memcpy((void *)((char *)newProfile + PROFILE_SCORE_ARRAY), savedScores, sizeof(savedScores));
                            memcpy((void *)((char *)newProfile + PROFILE_TIME_ARRAY), savedTimes, sizeof(savedTimes));
                        }
                        diag_log("[warp] Tournament mode restored: practice=0, scores copied to new profile");
                    }
                }
            }

            diag_log("[warp] Level loaded OK");
        } else {
            diag_logf("[warp] Invalid level index %d, aborting", levelIdx);
        }

        /* Clear saved ball pointer — old ball was freed by App_StartPracticeRace */
        g_warpBall = 0;

        /* Screen stays white, transition to REVEAL */
        g_whiteAlpha = 1.0f;
        g_phase = PHASE_REVEAL;
        g_phaseStartTime = now;
        g_musicFadeStarted = 0;
        diag_log("[warp] -> PHASE_REVEAL: fading from white");
        break;
    }

    case PHASE_REVEAL: {
        /* Fade from white to transparent, revealing new level */
        elapsed = now - g_phaseStartTime;
        g_whiteAlpha = 1.0f - (float)elapsed / (float)REVEAL_DURATION_MS;
        if (g_whiteAlpha < 0.0f) g_whiteAlpha = 0.0f;

        if (elapsed >= REVEAL_DURATION_MS) {
            g_whiteAlpha = 0.0f;
            g_phase = PHASE_IDLE;
            diag_log("[warp] -> PHASE_IDLE: warp complete");
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
    updateWarpStateMachine();
}

/* ============================================================
 * Collision handler — called from DispatchCollisionEvents detour
 * ============================================================ */

static volatile void *g_savedBoard = NULL;
static volatile int *g_savedBall = NULL;
static volatile int *g_savedCollObj = NULL;
static volatile int g_hookFireCount = 0;

static void WarpCollisionHandler(void) {
    void *board = (void *)g_savedBoard;
    int *ball = (int *)g_savedBall;
    int *collObj = (int *)g_savedCollObj;

    g_hookFireCount++;

    if (g_phase != PHASE_IDLE) return;  /* Already warping */

    if (!board || !ball) return;  /* Entity-entity collision, no ball — skip */

    if (collObj && collObj[1]) {
        const char *eventName = *(const char **)((char *)collObj[1] + COLL_OBJ_NAME_OFFSET);
        if (eventName) {
            if (isWarpEvent(eventName)) {
                {
                    char levelName[128];
                    if (parseWarpLevel(eventName, levelName, sizeof(levelName))) {
                        int raceIndex = findRaceIndex(levelName);
                        diag_logf("[WARP] event=\"%s\" level=\"%s\" raceIndex=%d",
                           eventName, levelName, raceIndex);
                        if (raceIndex > 0) {
                            if (*((char *)board + BOARD_GOAL_REACHED) == 0 &&
                                *((char *)ball + BALL_DEATH_PENDING) == 0) {
                                /* App_StartPracticeRace expects 0-based index (0-14),
                                 * but findRaceIndex returns 1-based (1-15) */
                                g_warpLevelIndex = raceIndex - 1;
                                g_phase = PHASE_RUMBLE;
                                g_rumbleInit = 0;
                                g_warpBall = (int)ball;  /* Save ball pointer from collision */
                                /* Initialize timestamps NOW so elapsed=0 on first frame */
                                {
                                    DWORD now = GetTickCount();
                                    g_phaseStartTime = now;
                                    g_warpStartTime = now;
                                }
                                g_whiteAlpha = 0.0f;
                                g_musicFadeStarted = 0;
                                diag_logf("[WARP] Triggered! Starting effect sequence, level=%d", raceIndex);
                            } else {
                                diag_logf("[WARP] SKIPPED: goal_reached=%d death_pending=%d",
                                   *((char *)board + BOARD_GOAL_REACHED),
                                   *((char *)ball + BALL_DEATH_PENDING));
                            }
                        }
                    }
                }
            }
        }
    }
}

/* ============================================================
 * Install hooks
 * ============================================================ */

/* DispatchCollisionEvents detour */
static unsigned char *g_detourBuf = NULL;

static void InstallCollisionHook(void) {
    unsigned char *dispatchAddr = (unsigned char *)DISPATCH_COLLISION_EVENTS;
    DWORD oldProtect;

    if (dispatchAddr[0] != 0x6A || dispatchAddr[1] != 0xFF ||
        dispatchAddr[2] != 0x64 || dispatchAddr[3] != 0xA1) {
        diag_log("[FATAL] DispatchCollisionEvents signature mismatch!");
        return;
    }
    diag_log("[InstallHooks] DispatchCollisionEvents signature OK");

    g_detourBuf = (unsigned char *)VirtualAlloc(NULL, 256, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (!g_detourBuf) return;

    {
        unsigned char *p = g_detourBuf;
        *p++ = 0x51;  /* PUSH ECX */
        *p++ = 0x52;  /* PUSH EDX */
        *p++ = 0x89; *p++ = 0x0D;  /* MOV [g_savedBoard], ECX */
        *(DWORD *)p = (DWORD)&g_savedBoard; p += 4;
        *p++ = 0x8B; *p++ = 0x44; *p++ = 0x24; *p++ = 0x0C;  /* MOV EAX, [ESP+12] */
        *p++ = 0xA3; *(DWORD *)p = (DWORD)&g_savedBall; p += 4;
        *p++ = 0x8B; *p++ = 0x44; *p++ = 0x24; *p++ = 0x10;  /* MOV EAX, [ESP+16] */
        *p++ = 0xA3; *(DWORD *)p = (DWORD)&g_savedCollObj; p += 4;
        *p++ = 0xB8; *(DWORD *)p = (DWORD)&WarpCollisionHandler; p += 4;
        *p++ = 0xFF; *p++ = 0xD0;  /* CALL EAX */
        *p++ = 0x5A;  /* POP EDX */
        *p++ = 0x59;  /* POP ECX */
        *p++ = 0x6A; *p++ = 0xFF;  /* original: PUSH -1 */
        *p++ = 0x64; *p++ = 0xA1; *p++ = 0x00; *p++ = 0x00; *p++ = 0x00; *p++ = 0x00;
        *p++ = 0xE9;
        *(DWORD *)p = (DWORD)(dispatchAddr + 8) - (DWORD)(p + 4);
        p += 4;
    }

    VirtualProtect(dispatchAddr, 8, PAGE_EXECUTE_READWRITE, &oldProtect);
    dispatchAddr[0] = 0xE9;
    *(DWORD *)(dispatchAddr + 1) = (DWORD)g_detourBuf - (DWORD)(dispatchAddr + 5);
    dispatchAddr[5] = 0x90;
    dispatchAddr[6] = 0x90;
    dispatchAddr[7] = 0x90;
    VirtualProtect(dispatchAddr, 8, oldProtect, &oldProtect);

    diag_log("[InstallHooks] Collision hook installed");
}

/* FrameUpdate epilogue hook */
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

    InstallCollisionHook();
    InstallFrameUpdateHook();

    diag_log("[warp mod v6f] All hooks installed. Ready for E:WARP events.");
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

        diag_log("=== LEVEL WARP MOD v6f LOADED ===");
        diag_log("v6f: native render jitter (ball+0x2D4), no pause, no sin/cos oscillation");

        load_real_bass();
        diag_logf("bass_real.dll handle: 0x%08X", (unsigned)g_hRealBass);

        CreateThread(NULL, 0, InitThread, NULL, 0, NULL);
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
