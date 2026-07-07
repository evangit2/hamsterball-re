/*
 * E:WARP (Level Warp) Mod v4 — Special Effects Edition
 *
 * When the ball touches a floor panel tagged E:WARP(LevelName),
 * a multi-phase special effect sequence plays before loading the
 * target level:
 *
 * Phase 1 (JIGGLE, ~2 seconds / 50 frames):
 *   - Ball is frozen (ball+0x808=1000, ball+0x2CC=1)
 *   - Ball jiggles upward slightly each frame (like Up Race vacuum suck)
 *   - Sound effect "warp_enter.wav" plays
 *   - Music fades out over 3 seconds (starting in this phase)
 *
 * Phase 2 (FLASH, ~0.5 seconds / 12 frames):
 *   - Ball jiggling stops
 *   - Second sound effect "warp_exit.wav" plays
 *   - Ball becomes invisible (ball+0x2FC = 0.0)
 *   - Screen flashes white (quick fade in + fade out)
 *
 * Phase 3 (FADE, ~1 second / 25 frames):
 *   - Screen fades to white (faster than end-game fade)
 *   - Music should be fully faded out by now
 *
 * Phase 4 (LOAD):
 *   - Once screen is fully white, load the target level
 *   - Calls App_StartPracticeRace(app, levelIndex)
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll warp_mod_v4.c -lwinmm \
 *     -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* ============================================================
 * Diagnostic logging
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
    wvsprintfA(buf, fmt, args);
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
#define GRAPHICS_PRESENT_OR_END     0x00455A90

/* App offsets */
#define APP_PROFILE_PTR          0x220
#define APP_ARENA_FLAG           0x237
#define APP_GRAPHICS_PTR         0x174   /* Graphics object */
#define APP_MUSIC_DEVICE_PTR     0x17C   /* MusicDevice* */

/* Graphics offsets */
#define GFX_D3D_DEVICE           0x154   /* IDirect3DDevice8* */

/* Board offsets */
#define BOARD_SCENE_PTR_OFFSET   0x878

/* PlayerProfile offsets */
#define PROFILE_RACE_INDEX       0x08
#define PROFILE_BOARD_PTR        0x0C

/* Ball offsets */
#define BALL_PLAYER_INDEX        0x18
#define BALL_DEATH_PENDING       0x2E9
#define BALL_POS_X               0x164
#define BALL_POS_Y               0x168
#define BALL_POS_Z               0x16C
#define BALL_IMPACT_FREEZE       0x808   /* int: blocks Ball_ApplyForce when nonzero */
#define BALL_IN_TAR              0x2CC   /* byte: blocks Ball_ApplyForce when nonzero */
#define BALL_ALPHA               0x2FC   /* float: 0.0=invisible, 1.0=opaque */

/* Collision event pair offsets */
#define COLL_OBJ_NAME_OFFSET     0x864

/* Board goal-reached flag */
#define BOARD_GOAL_REACHED       0xCD0

/* Per-player flags */
#define APP_PLAYER_REACHED_GOAL_BASE   0x5D6
#define APP_PLAYER_STRIDE              0xA0
#define APP_PLAYER_SCORED_BASE         0x5FC
#define APP_PLAYER_NEWLY_GOAL_BASE     0x5F0

/* MusicChannel offsets (from MusicChannel_FadeUpdate decompilation) */
#define MUSIC_CHAN_BASS_CHANNEL  0x08   /* DWORD: BASS channel handle */
#define MUSIC_CHAN_VOLUME       0x528   /* float: current volume (0.0-1.0) */
#define MUSIC_CHAN_FADE_RATE   0x52C   /* float: per-frame volume delta */
#define MUSIC_CHAN_FADE_OUT     0x530   /* byte: fade-out active flag */
#define MUSIC_CHAN_FADE_IN      0x531   /* byte: fade-in active flag */

/* MusicDevice offsets (from MusicDevice_FadeAll decompilation) */
#define MUSIC_DEV_CHANNEL_LIST  0x418   /* AthenaList data pointer */

/* D3D8 vtable indices (0-indexed from IUnknown) */
/* SetRenderState = vtable[50] = offset 0xC8 */
/* DrawPrimitiveUP = vtable[72] = offset 0x120 */
/* BeginScene = vtable[?] */
/* We use BeginScene = vtable[34] = offset 0x88 */
/* EndScene = vtable[35] = offset 0x8C */

/* D3D8 Render State values */
#define D3DRS_ALPHABLENDENABLE   27
#define D3DRS_SRCBLEND          19
#define D3DRS_DESTBLEND         20
#define D3DRS_FOGENABLE         8
#define D3DRS_FOGCOLOR          21
#define D3DRS_FOGTABLEMODE      22

/* D3D8 Blend values */
#define D3DBLEND_SRCALPHA       5
#define D3DBLEND_INVSRCALPHA    6

/* D3D8 Texture Stage values */
#define D3DTSS_COLOROP          0
#define D3DTSS_COLORARG1        1
#define D3DTSS_COLORARG2        2
#define D3DTSS_ALPHAOP          3
#define D3DTSS_ALPHAARG1        4
#define D3DTSS_ALPHAARG2        5

#define D3DTOP_SELECTARG1       2
#define D3DTOP_SELECTARG2       3
#define D3DTOP_DISABLE          1

#define D3DTA_TFACTOR           3

/* FVF for TL vertex (transformed, lit) */
#define D3DFVF_XYZRHW           0x001
#define D3DFVF_DIFFUSE          0x040

typedef struct {
    float x, y, z;   /* screen position (z=0, rhw=1) */
    float rhw;
    DWORD color;     /* ARGB */
} TLVertex;

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
 * Warp state machine
 * ============================================================
 *
 * PHASE_IDLE     (0) - Not warping, normal gameplay
 * PHASE_JIGGLE   (1) - Ball frozen + jiggling + sound + music fade start
 * PHASE_FLASH    (2) - Ball invisible + screen white flash
 * PHASE_FADE     (3) - Screen fades to solid white
 * PHASE_LOAD     (4) - Load target level
 *
 * Frame counts (at 25fps):
 *   JIGGLE: 50 frames (2.0 sec)
 *   FLASH:  12 frames (0.48 sec) — quick flash in+out
 *   FADE:   25 frames (1.0 sec)
 *   Total:  87 frames (3.48 sec)
 *
 * Music fade: starts at JIGGLE frame 0, fades over 75 frames (3.0 sec)
 *             so it reaches 0 at end of FLASH phase.
 */

typedef enum {
    PHASE_IDLE = 0,
    PHASE_JIGGLE,
    PHASE_FLASH,
    PHASE_FADE,
    PHASE_LOAD
} WarpPhase;

static volatile WarpPhase g_phase = PHASE_IDLE;
static volatile int g_phaseFrame = 0;
static volatile int g_warpLevelIndex = -1;
static volatile int g_musicFadeStarted = 0;
static volatile float g_musicOrigVolume = 1.0f;

/* Phase durations in frames (25fps) */
#define JIGGLE_FRAMES   50
#define FLASH_FRAMES    12
#define FADE_FRAMES     25
#define MUSIC_FADE_FRAMES 75

/* Flash sub-timing within FLASH phase */
#define FLASH_PEAK_FRAME 6  /* peak white at frame 6 of 12 */

/* White screen alpha (0.0 = transparent, 1.0 = fully white) */
static volatile float g_whiteAlpha = 0.0f;

/* Ball position saved at warp start for jiggling */
static volatile float g_ballOrigY = 0.0f;

/* ============================================================
 * Memory helpers
 * ============================================================ */

static void WriteMemory(void *addr, const void *data, SIZE_T size) {
    DWORD oldProtect;
    if (VirtualProtect(addr, size, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        memcpy(addr, data, size);
        VirtualProtect(addr, size, oldProtect, &oldProtect);
    }
}

static int GetApp(void) {
    return *(int *)APP_PTR;
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
 * Set win state (replicate what N:GOAL does, minus audio/popups)
 * ============================================================ */

static void setWinState(void *board, int *ball) {
    int app = GetApp();
    int playerIdx;
    if (!app) return;

    playerIdx = ball[BALL_PLAYER_INDEX / 4];

    *((char *)board + BOARD_GOAL_REACHED) = 1;
    *((char *)app + APP_PLAYER_REACHED_GOAL_BASE + playerIdx * APP_PLAYER_STRIDE) = 1;

    if (*((char *)app + APP_PLAYER_SCORED_BASE + playerIdx * APP_PLAYER_STRIDE) == 0) {
        *((int *)((char *)app + APP_PLAYER_SCORED_BASE + playerIdx * APP_PLAYER_STRIDE)) = 1;
    }
    *((char *)app + APP_PLAYER_NEWLY_GOAL_BASE + playerIdx * APP_PLAYER_STRIDE) = 1;
}

/* ============================================================
 * Music fade — fade out music over MUSIC_FADE_FRAMES
 *
 * We directly manipulate the MusicChannel struct's volume field
 * and call BASS_ChannelSetAttributes to apply it. This is the same
 * pattern the game uses in MusicChannel_FadeUpdate (0x46A180).
 *
 * MusicDevice is at App+0x17C.
 * MusicDevice has an AthenaList at +0x418 of MusicChannel objects.
 * Each MusicChannel has:
 *   +0x08 = BASS channel handle (DWORD)
 *   +0x528 = current volume (float)
 *   +0x530 = fade-out flag (byte)
 *
 * We set the fade-out flag and rate so the game's own FadeAll
 * (called every frame in App_FrameUpdate) handles the actual
 * volume stepping. But we also directly set volume as a fallback.
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

    /* MusicDevice struct:
     * +0x08 = BASS channel (for the "current" music)
     * +0x0C = AthenaList (channel list base)
     * +0x10 = count
     * +0x418 = data ptr
     */
    chanListData = *(int *)(musicDev + MUSIC_DEV_CHANNEL_LIST);
    chanCount = *(int *)(musicDev + 0x10);

    if (chanCount > 0 && chanListData) {
        for (i = 0; i < chanCount; i++) {
            int chan = *(int *)(chanListData + i * 4);
            if (chan) {
                /* Save original volume */
                g_musicOrigVolume = *(float *)(chan + MUSIC_CHAN_VOLUME);
                /* Set fade-out flag and rate */
                *(char *)(chan + MUSIC_CHAN_FADE_OUT) = 1;
                /* Rate = origVolume / MUSIC_FADE_FRAMES per frame */
                *(float *)(chan + MUSIC_CHAN_FADE_RATE) = g_musicOrigVolume / (float)MUSIC_FADE_FRAMES;
                diag_logf("[music] Channel %d: origVol=%.3f, fadeRate=%.5f",
                          i, g_musicOrigVolume, g_musicOrigVolume / (float)MUSIC_FADE_FRAMES);
            }
        }
    } else {
        /* Fallback: fade via BASS directly on MusicDevice+0x08 */
        int bassChan = *(int *)(musicDev + 0x08);
        if (bassChan && real_BASS_ChannelSetAttributes) {
            /* Start volume at 100 (BASS uses 0-100 integer) */
            g_musicOrigVolume = 1.0f;
            diag_logf("[music] Fallback BASS direct fade, channel=%d", bassChan);
        }
    }

    g_musicFadeStarted = 1;
}

/* Per-frame music volume update (fallback if game's FadeAll doesn't work) */
static void updateMusicFade(void) {
    int app, musicDev;
    int chanListData, chanCount, i;

    if (!g_musicFadeStarted) return;

    app = GetApp();
    if (!app) return;
    musicDev = *(int *)((char *)app + APP_MUSIC_DEVICE_PTR);
    if (!musicDev) return;

    chanListData = *(int *)(musicDev + MUSIC_DEV_CHANNEL_LIST);
    chanCount = *(int *)(musicDev + 0x10);

    if (chanCount > 0 && chanListData) {
        for (i = 0; i < chanCount; i++) {
            int chan = *(int *)(chanListData + i * 4);
            if (chan) {
                /* Directly set volume each frame as well */
                int frameIntoFade = g_phaseFrame;
                if (g_phase > PHASE_JIGGLE) {
                    frameIntoFade += JIGGLE_FRAMES;
                }
                if (g_phase > PHASE_FLASH) {
                    frameIntoFade += FLASH_FRAMES;
                }
                {
                    float t = (float)frameIntoFade / (float)MUSIC_FADE_FRAMES;
                    float vol;
                    if (t >= 1.0f) {
                        vol = 0.0f;
                    } else {
                        vol = g_musicOrigVolume * (1.0f - t);
                    }
                    *(float *)(chan + MUSIC_CHAN_VOLUME) = vol;
                    /* Also call BASS directly to ensure it applies */
                    if (real_BASS_ChannelSetAttributes) {
                        int bassChan = *(int *)(chan + MUSIC_CHAN_BASS_CHANNEL);
                        if (bassChan) {
                            real_BASS_ChannelSetAttributes(bassChan, -1.0f, (int)(vol * 100.0f), -1);
                        }
                    }
                }
            }
        }
    }
}

/* ============================================================
 * D3D white screen overlay
 *
 * Draws a fullscreen white quad with alpha = g_whiteAlpha.
 * Called from our Graphics_PresentOrEnd hook, AFTER the game
 * has finished its Present/Swap but BEFORE it returns.
 *
 * Actually, we hook BEFORE Present — we draw our overlay as part
 * of the scene (before EndScene/Present). The game calls:
 *   Graphics_BeginFrame → vtable[0x24] → vtable[0x28] → vtable[0x2c] → Graphics_PresentOrEnd
 *
 * vtable[0x2c] = Scene_RenderFrame or similar (renders 3D + UI).
 * We hook Graphics_PresentOrEnd's entry — at that point all
 * rendering is done but Present hasn't been called yet.
 * We draw our white quad, then call the original.
 *
 * D3D8 device is at gfx+0x154. We access it via App+0x174→+0x154.
 *
 * DrawPrimitiveUP vtable[72] (0x120 offset):
 *   __stdcall HRESULT DrawPrimitiveUP(
 *     D3DPRIMITIVETYPE PrimitiveType,  // 4 = TRIANGLESTRIP
 *     UINT PrimitiveCount,             // 2 (for a quad = 2 triangles)
 *     const void* pVertexStreamZeroData,
 *     UINT VertexStreamZeroStride
 *   )
 *
 * SetRenderState vtable[50] (0xC8 offset):
 *   __stdcall HRESULT SetRenderState(D3DRENDERSTATETYPE State, DWORD Value)
 * ============================================================ */

typedef long (__stdcall *D3DSetRenderState_t)(void*, DWORD, DWORD);
typedef long (__stdcall *D3DDrawPrimitiveUP_t)(void*, DWORD, UINT, const void*, UINT);
typedef long (__stdcall *D3DSetTextureStageState_t)(void*, DWORD, DWORD, DWORD);

static void drawWhiteOverlay(void) {
    int app;
    int gfx;
    void *device;
    void **vtable;

    if (g_whiteAlpha <= 0.001f) return;

    app = GetApp();
    if (!app) return;
    gfx = *(int *)((char *)app + APP_GRAPHICS_PTR);
    if (!gfx) return;
    device = *(void **)((char *)gfx + GFX_D3D_DEVICE);
    if (!device) return;
    vtable = *(void ***)device;
    if (!vtable) return;

    {
        /* Build the white quad vertices (screen-space) */
        /* TLVertex: x, y, z, rhw, color */
        DWORD whiteAlpha = (DWORD)(g_whiteAlpha * 255.0f) << 24;  /* ARGB: A=alpha, R=G=B=0xFF */
        DWORD color = 0x00FFFFFF | whiteAlpha;  /* white with alpha */

        /* Use large coordinates to cover any resolution */
        TLVertex verts[4] = {
            {    0.0f,    0.0f, 0.0f, 1.0f, color },
            { 1920.0f,    0.0f, 0.0f, 1.0f, color },
            {    0.0f, 1080.0f, 0.0f, 1.0f, color },
            { 1920.0f, 1080.0f, 0.0f, 1.0f, color },
        };

        D3DSetRenderState_t pSetRenderState = (D3DSetRenderState_t)vtable[50];
        D3DDrawPrimitiveUP_t pDrawPrimitiveUP = (D3DDrawPrimitiveUP_t)vtable[72];
        D3DSetTextureStageState_t pSetTextureStageState = (D3DSetTextureStageState_t)vtable[63];

        if (!pSetRenderState || !pDrawPrimitiveUP || !pSetTextureStageState) return;

        /* Save and set render states for alpha-blended white quad */
        /* Enable alpha blending */
        pSetRenderState(device, D3DRS_ALPHABLENDENABLE, TRUE);
        pSetRenderState(device, D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        pSetRenderState(device, D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

        /* Disable fog so it doesn't tint our overlay */
        pSetRenderState(device, D3DRS_FOGENABLE, FALSE);

        /* Set texture stage to use TFACTOR (our vertex color) */
        pSetTextureStageState(device, 0, D3DTSS_COLOROP, D3DTOP_SELECTARG2);   /* SELECTARG2 = diffuse */
        pSetTextureStageState(device, 0, D3DTSS_COLORARG2, 0);  /* D3DTA_DIFFUSE = 0 */
        pSetTextureStageState(device, 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
        pSetTextureStageState(device, 0, D3DTSS_ALPHAARG2, 0);  /* D3DTA_DIFFUSE */

        /* Draw the quad as a triangle strip (2 triangles, 4 vertices) */
        /* D3DPT_TRIANGLESTRIP = 4 */
        pDrawPrimitiveUP(device, 4, 2, verts, sizeof(TLVertex));

        /* Restore fog */
        pSetRenderState(device, D3DRS_FOGENABLE, TRUE);
    }
}

/* ============================================================
 * Graphics_PresentOrEnd hook
 *
 * We hook the entry of Graphics_PresentOrEnd to draw our white
 * overlay BEFORE the present/swap. This means our quad appears
 * on top of all game rendering.
 *
 * Graphics_PresentOrEnd is __thiscall: ECX=gfx, stack: [param_1] RET 4
 * Original prologue:
 *   8A 44 24 04    MOV AL, [ESP+4]    (4 bytes)
 *   83 EC 20       SUB ESP, 0x20     (3 bytes)
 * Total: 7 bytes — we use a 5-byte JMP + 2 NOPs
 * ============================================================ */

static unsigned char g_presentOrig[8];
static unsigned char *g_presentDetour = NULL;

/* This is called from the detour BEFORE original prologue executes.
 * At this point ECX = gfx (Graphics object), [ESP+4] = param_1.
 * We can safely read the D3D device from gfx+0x154.
 *
 * We must preserve ALL registers since the original function's
 * prologue hasn't run yet. We only touch the D3D device through
 * its vtable, which is a __stdcall convention (doesn't clobber
 * ECX in a way that matters since we restore it).
 *
 * Actually — the D3D calls are __stdcall and will clobber EAX/ECX/EDX.
 * We need to save ECX (gfx ptr) since the original function needs it.
 * Our detour saves ECX, calls drawWhiteOverlay, restores ECX,
 * then executes original 7 bytes + JMP back.
 */

static volatile void *g_savedGfx = NULL;

/* ============================================================
 * App_FrameUpdate epilogue hook
 *
 * Same as v3 — hook the epilogue (POP ESI + ADD ESP,8 + RET)
 * at 0x46C1F1. Our handler runs the warp state machine.
 * ============================================================ */

#define APP_FRAME_UPDATE_EPILOGUE 0x0046C1F1

static unsigned char *g_frameUpdateDetour = NULL;

/* ============================================================
 * Warp state machine — called every frame from FrameUpdateHandler
 * ============================================================ */

static void updateWarpStateMachine(void) {
    int app;
    int board;
    int ball;

    if (g_phase == PHASE_IDLE) return;

    app = GetApp();
    if (!app) {
        diag_log("[warp] App null during warp, aborting");
        g_phase = PHASE_IDLE;
        return;
    }

    /* Get ball via App→Profile→Board→Scene→ball_list */
    {
        int profile = *(int *)((char *)app + APP_PROFILE_PTR);
        if (!profile) { diag_log("[warp] Profile null"); return; }
        board = *(int *)((char *)profile + PROFILE_BOARD_PTR);
        if (!board) { diag_log("[warp] Board null"); return; }
    }

    /* Get player ball from Scene+0x29D0 (direct ball ptr, simpler than AthenaList) */
    {
        int scene = *(int *)((char *)board + BOARD_SCENE_PTR_OFFSET);
        if (!scene) { diag_log("[warp] Scene null"); return; }
        ball = *(int *)((char *)scene + 0x29D0);
    }

    switch (g_phase) {
    case PHASE_JIGGLE: {
        /* Ball is frozen + jiggling upward */
        if (g_phaseFrame == 0) {
            /* Start of jiggle: freeze ball, save Y, play sound, start music fade */
            if (ball) {
                g_ballOrigY = *(float *)((char *)ball + BALL_POS_Y);
                *(int *)((char *)ball + BALL_IMPACT_FREEZE) = 1000;
                *(char *)((char *)ball + BALL_IN_TAR) = 1;
            }
            /* Play enter sound — placeholder for sound effect */
            /* TODO: Load and play "warp_enter.wav" via DirectSound */
            diag_log("[warp] TODO: Play warp_enter sound effect");
            startMusicFade();
            diag_logf("[warp] PHASE_JIGGLE start: ballY=%.2f", g_ballOrigY);
        }

        /* Per-frame: jiggle ball upward */
        if (ball) {
            float jiggle = 0.25f;  /* same as Up Race vacuum suck */
            *(float *)((char *)ball + BALL_POS_Y) += jiggle;
        }

        /* Update music fade */
        updateMusicFade();

        g_phaseFrame++;
        if (g_phaseFrame >= JIGGLE_FRAMES) {
            g_phase = PHASE_FLASH;
            g_phaseFrame = 0;
            diag_log("[warp] → PHASE_FLASH");
        }
        break;
    }

    case PHASE_FLASH: {
        /* Ball becomes invisible, screen flashes white */
        if (g_phaseFrame == 0) {
            /* Play exit sound — placeholder */
            /* TODO: Load and play "warp_exit.wav" via DirectSound */
            diag_log("[warp] TODO: Play warp_exit sound effect");
            if (ball) {
                *(float *)((char *)ball + BALL_ALPHA) = 0.0f;
            }
            diag_log("[warp] Ball invisible, starting white flash");
        }

        /* Quick flash: ramp up to peak, then back down */
        if (g_phaseFrame < FLASH_PEAK_FRAME) {
            /* Fade in: 0 → 1 over FLASH_PEAK_FRAME frames */
            g_whiteAlpha = (float)g_phaseFrame / (float)FLASH_PEAK_FRAME;
        } else {
            /* Fade out: 1 → 0 over remaining frames */
            int remaining = FLASH_FRAMES - g_phaseFrame;
            g_whiteAlpha = (float)remaining / (float)(FLASH_FRAMES - FLASH_PEAK_FRAME);
            if (g_whiteAlpha < 0.0f) g_whiteAlpha = 0.0f;
        }

        /* Continue music fade */
        updateMusicFade();

        g_phaseFrame++;
        if (g_phaseFrame >= FLASH_FRAMES) {
            g_phase = PHASE_FADE;
            g_phaseFrame = 0;
            diag_log("[warp] → PHASE_FADE");
        }
        break;
    }

    case PHASE_FADE: {
        /* Screen fades from current to solid white */
        /* Start from 0 (flash ended at 0) and ramp to 1.0 */
        g_whiteAlpha = (float)g_phaseFrame / (float)FADE_FRAMES;
        if (g_whiteAlpha > 1.0f) g_whiteAlpha = 1.0f;

        /* Continue music fade (should be near 0 by now) */
        updateMusicFade();

        g_phaseFrame++;
        if (g_phaseFrame >= FADE_FRAMES) {
            g_whiteAlpha = 1.0f;  /* Full white */
            g_phase = PHASE_LOAD;
            g_phaseFrame = 0;
            diag_log("[warp] → PHASE_LOAD: loading level");
        }
        break;
    }

    case PHASE_LOAD: {
        /* Load the target level */
        int levelIdx = g_warpLevelIndex;
        g_warpLevelIndex = -1;

        /* Reset state */
        g_phase = PHASE_IDLE;
        g_phaseFrame = 0;
        g_whiteAlpha = 0.0f;
        g_musicFadeStarted = 0;

        /* Clear ball freeze flags before loading */
        if (ball) {
            *(int *)((char *)ball + BALL_IMPACT_FREEZE) = 0;
            *(char *)((char *)ball + BALL_IN_TAR) = 0;
            *(float *)((char *)ball + BALL_ALPHA) = 1.0f;
        }

        /* Call App_StartPracticeRace(app, levelIndex) */
        if (levelIdx >= 1 && levelIdx <= 15) {
            void *func = (void *)APP_START_PRACTICE_RACE;
            int appVal = app;
            int idx = levelIdx;

            diag_logf("[warp] App_StartPracticeRace(app=0x%08X, level=%d)", appVal, idx);

            __asm__ volatile (
                "push %[idx]\n\t"
                "movl %[appVal], %%ecx\n\t"
                "call *%[func]\n\t"
                : /* no outputs */
                : [func] "r" (func),
                  [appVal] "r" (appVal),
                  [idx] "r" (idx)
                : "eax", "edx", "ecx", "esp",
                  "st", "st(1)", "st(2)", "st(3)",
                  "st(4)", "st(5)", "st(6)", "st(7)", "memory"
            );

            diag_log("[warp] Level loaded OK");
        } else {
            diag_logf("[warp] Invalid level index %d, aborting", levelIdx);
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

    if (g_phase != PHASE_IDLE) return;  /* Already warping, ignore */

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
                                setWinState(board, ball);
                                g_warpLevelIndex = raceIndex;
                                g_phase = PHASE_JIGGLE;
                                g_phaseFrame = 0;
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

/* DispatchCollisionEvents detour: same as v3 */
static unsigned char g_trampoline[16];
static unsigned char *g_detourBuf = NULL;

static void InstallCollisionHook(void) {
    unsigned char *dispatchAddr = (unsigned char *)DISPATCH_COLLISION_EVENTS;
    DWORD oldProtect;

    /* Verify signature: 6A FF 64 A1 00 00 00 00 */
    if (dispatchAddr[0] != 0x6A || dispatchAddr[1] != 0xFF ||
        dispatchAddr[2] != 0x64 || dispatchAddr[3] != 0xA1) {
        diag_log("[FATAL] DispatchCollisionEvents signature mismatch!");
        return;
    }
    diag_log("[InstallHooks] DispatchCollisionEvents signature OK");

    /* Save original 8 bytes */
    memcpy(g_trampoline, dispatchAddr, 8);
    g_trampoline[8] = 0xE9;
    *(DWORD *)(g_trampoline + 9) = (DWORD)(dispatchAddr + 8) - (DWORD)(g_trampoline + 13);
    VirtualProtect(g_trampoline, sizeof(g_trampoline), PAGE_EXECUTE_READWRITE, &oldProtect);

    /* Build detour */
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

    /* Patch original */
    VirtualProtect(dispatchAddr, 8, PAGE_EXECUTE_READWRITE, &oldProtect);
    dispatchAddr[0] = 0xE9;
    *(DWORD *)(dispatchAddr + 1) = (DWORD)g_detourBuf - (DWORD)(dispatchAddr + 5);
    dispatchAddr[5] = 0x90;
    dispatchAddr[6] = 0x90;
    dispatchAddr[7] = 0x90;
    VirtualProtect(dispatchAddr, 8, oldProtect, &oldProtect);

    diag_log("[InstallHooks] Collision hook installed");
}

/* FrameUpdate epilogue hook: same as v3 */
static void InstallFrameUpdateHook(void) {
    unsigned char *epiAddr = (unsigned char *)APP_FRAME_UPDATE_EPILOGUE;
    DWORD oldProtect;

    /* Verify: 5E 83 C4 08 C3 */
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

/* Graphics_PresentOrEnd hook
 *
 * Original prologue (7 bytes):
 *   8A 44 24 04    MOV AL, [ESP+4]    (4 bytes)
 *   83 EC 20       SUB ESP, 0x20     (3 bytes)
 *
 * We patch with 5-byte JMP + 2 NOPs.
 * Detour: save ECX (gfx), call drawWhiteOverlay, restore ECX,
 * execute original 7 bytes, JMP back.
 */
static void InstallPresentHook(void) {
    unsigned char *presentAddr = (unsigned char *)GRAPHICS_PRESENT_OR_END;
    DWORD oldProtect;

    /* Verify: 8A 44 24 04 83 EC 20 */
    if (presentAddr[0] != 0x8A || presentAddr[1] != 0x44 ||
        presentAddr[2] != 0x24 || presentAddr[3] != 0x04 ||
        presentAddr[4] != 0x83 || presentAddr[5] != 0xEC || presentAddr[6] != 0x20) {
        diag_log("[FATAL] Graphics_PresentOrEnd signature mismatch!");
        return;
    }
    diag_log("[InstallHooks] Graphics_PresentOrEnd signature OK");

    /* Save original 7 bytes */
    memcpy(g_presentOrig, presentAddr, 7);

    /* Allocate detour buffer */
    g_presentDetour = (unsigned char *)VirtualAlloc(NULL, 256, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (!g_presentDetour) return;

    {
        unsigned char *p = g_presentDetour;

        /* Save ECX (gfx ptr) — D3D calls will clobber it */
        *p++ = 0x51;  /* PUSH ECX */
        *p++ = 0x52;  /* PUSH EDX */

        /* Call drawWhiteOverlay (reads D3D device from App→Gfx→Device) */
        *p++ = 0xB8; *(DWORD *)p = (DWORD)&drawWhiteOverlay; p += 4;
        *p++ = 0xFF; *p++ = 0xD0;  /* CALL EAX */

        /* Restore registers */
        *p++ = 0x5A;  /* POP EDX */
        *p++ = 0x59;  /* POP ECX */

        /* Execute original 7 bytes */
        *p++ = 0x8A; *p++ = 0x44; *p++ = 0x24; *p++ = 0x04;  /* MOV AL, [ESP+4] */
        *p++ = 0x83; *p++ = 0xEC; *p++ = 0x20;  /* SUB ESP, 0x20 */

        /* JMP to presentAddr+7 */
        *p++ = 0xE9;
        *(DWORD *)p = (DWORD)(presentAddr + 7) - (DWORD)(p + 4);
        p += 4;
    }

    /* Patch original: 5-byte JMP + 2 NOPs */
    VirtualProtect(presentAddr, 7, PAGE_EXECUTE_READWRITE, &oldProtect);
    presentAddr[0] = 0xE9;
    *(DWORD *)(presentAddr + 1) = (DWORD)g_presentDetour - (DWORD)(presentAddr + 5);
    presentAddr[5] = 0x90;
    presentAddr[6] = 0x90;
    VirtualProtect(presentAddr, 7, oldProtect, &oldProtect);

    diag_log("[InstallHooks] Graphics_PresentOrEnd hook installed");
}

/* ============================================================
 * Init thread
 * ============================================================ */

static DWORD WINAPI InitThread(LPVOID param) {
    (void)param;
    Sleep(2000);

    InstallCollisionHook();
    InstallFrameUpdateHook();
    InstallPresentHook();

    diag_log("[warp mod v4] All hooks installed. Ready for E:WARP events.");
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

        /* Set up log path next to bass.dll */
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

        diag_log("=== LEVEL WARP MOD v4 LOADED ===");
        diag_log("Features: ball freeze + jiggle, music fade, white flash, screen fade, level load");

        load_real_bass();
        diag_logf("bass_real.dll handle: 0x%08X", (unsigned)g_hRealBass);

        CreateThread(NULL, 0, InitThread, NULL, 0, NULL);
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
