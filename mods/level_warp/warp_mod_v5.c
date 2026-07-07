/*
 * E:WARP (Level Warp) Mod v5 — Fixed Special Effects Edition
 *
 * v5 fixes:
 *   - CRITICAL: Restores D3D texture stage states after overlay draw
 *     (v4 left COLOROP=SELECTARG2, causing new level to render as vertex points only)
 *   - CRITICAL: Switched to GetTickCount()-based timing (framerate-independent)
 *     (v4 used frame counts which were too fast if game runs >25fps)
 *   - Fixed: vsnprintf replaces wvsprintfA (which doesn't support %f)
 *   - Added PHASE_REVEAL: fades FROM white after level loads
 *   - FADE duration increased to 2 seconds (was 1 second)
 *   - Screen stays white during PHASE_LOAD (was reset to 0 before loading)
 *
 * Phase timeline (real-time, not frame-based):
 *   JIGGLE: 2.0 sec — Ball frozen + jiggling + sound + music fade start
 *   FLASH:  0.5 sec — Ball invisible + quick white flash
 *   FADE:   2.0 sec — Screen fades to solid white
 *   LOAD:   instant — Load target level while screen stays white
 *   REVEAL: 1.0 sec — Fade from white to reveal new level
 *   Total:  ~5.5 sec
 *
 * Music fade: 3.0 sec starting at JIGGLE start
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll warp_mod_v5.c -lwinmm \
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
#define GRAPHICS_PRESENT_OR_END     0x00455A90

/* App offsets */
#define APP_PROFILE_PTR          0x220
#define APP_ARENA_FLAG           0x237
#define APP_GRAPHICS_PTR         0x174
#define APP_MUSIC_DEVICE_PTR     0x17C

/* Graphics offsets */
#define GFX_D3D_DEVICE           0x154

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
#define BALL_IMPACT_FREEZE       0x808
#define BALL_IN_TAR              0x2CC
#define BALL_ALPHA               0x2FC

/* Collision event pair offsets */
#define COLL_OBJ_NAME_OFFSET     0x864

/* Board goal-reached flag */
#define BOARD_GOAL_REACHED       0xCD0

/* Per-player flags */
#define APP_PLAYER_REACHED_GOAL_BASE   0x5D6
#define APP_PLAYER_STRIDE              0xA0
#define APP_PLAYER_SCORED_BASE         0x5FC
#define APP_PLAYER_NEWLY_GOAL_BASE     0x5F0

/* MusicChannel offsets */
#define MUSIC_CHAN_BASS_CHANNEL  0x08
#define MUSIC_CHAN_VOLUME       0x528
#define MUSIC_CHAN_FADE_RATE    0x52C
#define MUSIC_CHAN_FADE_OUT     0x530
#define MUSIC_CHAN_FADE_IN      0x531

/* MusicDevice offsets */
#define MUSIC_DEV_CHANNEL_LIST  0x418

/* D3D8 Render State values */
#define D3DRS_ALPHABLENDENABLE   27
#define D3DRS_SRCBLEND          19
#define D3DRS_DESTBLEND         20
#define D3DRS_FOGENABLE          8

/* D3D8 Blend values */
#define D3DBLEND_SRCALPHA        5
#define D3DBLEND_INVSRCALPHA     6

/* D3D8 Texture Stage values */
#define D3DTSS_COLOROP           0
#define D3DTSS_COLORARG1         1
#define D3DTSS_COLORARG2         2
#define D3DTSS_ALPHAOP           3
#define D3DTSS_ALPHAARG1         4
#define D3DTSS_ALPHAARG2         5

/* D3D8 Texture Operation constants */
#define D3DTOP_DISABLE           1
#define D3DTOP_SELECTARG1        2
#define D3DTOP_SELECTARG2        3
#define D3DTOP_MODULATE          4

/* D3D8 Texture Argument constants */
#define D3DTA_DIFFUSE            0
#define D3DTA_TEXTURE            2

/* FVF for TL vertex (transformed, lit) */
#define D3DFVF_XYZRHW           0x001
#define D3DFVF_DIFFUSE          0x040

typedef struct {
    float x, y, z;
    float rhw;
    DWORD color;
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
 * Warp state machine — time-based (GetTickCount), not frame-based
 * ============================================================ */

typedef enum {
    PHASE_IDLE = 0,
    PHASE_JIGGLE,
    PHASE_FLASH,
    PHASE_FADE,
    PHASE_LOAD,
    PHASE_REVEAL
} WarpPhase;

static volatile WarpPhase g_phase = PHASE_IDLE;
static volatile DWORD g_phaseStartTime = 0;   /* GetTickCount() when phase started */
static volatile DWORD g_warpStartTime = 0;     /* GetTickCount() when warp started */
static volatile int g_warpLevelIndex = -1;
static volatile int g_musicFadeStarted = 0;
static volatile float g_musicOrigVolume = 1.0f;

/* Phase durations in milliseconds (framerate-independent!) */
#define JIGGLE_DURATION_MS    2000   /* 2.0 sec */
#define FLASH_DURATION_MS      500   /* 0.5 sec */
#define FADE_DURATION_MS      2000   /* 2.0 sec */
#define REVEAL_DURATION_MS    1000   /* 1.0 sec */
#define MUSIC_FADE_MS         3000   /* 3.0 sec */

/* Flash sub-timing */
#define FLASH_PEAK_MS         250   /* peak white at 250ms into 500ms flash */

/* White screen alpha (0.0 = transparent, 1.0 = fully white) */
static volatile float g_whiteAlpha = 0.0f;

/* Ball position saved at warp start for jiggling */
static volatile float g_ballOrigY = 0.0f;

/* ============================================================
 * Memory helpers
 * ============================================================ */

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
        for (i = 0; i < chanCount; i++) {
            int chan = *(int *)(chanListData + i * 4);
            if (chan) {
                g_musicOrigVolume = *(float *)(chan + MUSIC_CHAN_VOLUME);
                *(char *)(chan + MUSIC_CHAN_FADE_OUT) = 1;
                *(float *)(chan + MUSIC_CHAN_FADE_RATE) = g_musicOrigVolume / (float)(MUSIC_FADE_MS / 40);  /* per-frame approx */
                diag_logf("[music] Channel %d: origVol=%.3f, fadeRate=%.5f",
                          i, g_musicOrigVolume, g_musicOrigVolume / (float)(MUSIC_FADE_MS / 40));
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
    float t, vol;

    if (!g_musicFadeStarted) return;

    elapsed = GetTickCount() - g_warpStartTime;
    t = (float)elapsed / (float)MUSIC_FADE_MS;
    if (t >= 1.0f) {
        vol = 0.0f;
    } else {
        vol = g_musicOrigVolume * (1.0f - t);
    }

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
                *(float *)(chan + MUSIC_CHAN_VOLUME) = vol;
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

/* ============================================================
 * D3D white screen overlay
 *
 * v5 FIX: Restores ALL modified texture stage states after draw!
 * v4 left COLOROP=SELECTARG2 which caused textures to be ignored
 * on the next frame, making the new level show only vertex points.
 *
 * After drawing, we restore texture stage 0 to the game defaults:
 *   COLOROP  = MODULATE (texture × diffuse)
 *   COLORARG1 = TEXTURE
 *   COLORARG2 = DIFFUSE
 *   ALPHAOP  = MODULATE
 *   ALPHAARG1 = TEXTURE
 *   ALPHAARG2 = DIFFUSE
 * ============================================================ */

typedef long (__stdcall *D3DSetRenderState_t)(void*, DWORD, DWORD);
typedef long (__stdcall *D3DDrawPrimitiveUP_t)(void*, DWORD, UINT, const void*, UINT);
typedef long (__stdcall *D3DSetTextureStageState_t)(void*, DWORD, DWORD, DWORD);

static void drawWhiteOverlay(void) {
    int app;
    int gfx;
    void *device;
    void **vtable;
    D3DSetRenderState_t pSetRenderState;
    D3DDrawPrimitiveUP_t pDrawPrimitiveUP;
    D3DSetTextureStageState_t pSetTextureStageState;

    if (g_whiteAlpha <= 0.001f) return;

    app = GetApp();
    if (!app) return;
    gfx = *(int *)((char *)app + APP_GRAPHICS_PTR);
    if (!gfx) return;
    device = *(void **)((char *)gfx + GFX_D3D_DEVICE);
    if (!device) return;
    vtable = *(void ***)device;
    if (!vtable) return;

    pSetRenderState = (D3DSetRenderState_t)vtable[50];
    pDrawPrimitiveUP = (D3DDrawPrimitiveUP_t)vtable[72];
    pSetTextureStageState = (D3DSetTextureStageState_t)vtable[63];

    if (!pSetRenderState || !pDrawPrimitiveUP || !pSetTextureStageState) return;

    {
        DWORD whiteAlpha = (DWORD)(g_whiteAlpha * 255.0f) << 24;
        DWORD color = 0x00FFFFFF | whiteAlpha;

        TLVertex verts[4] = {
            {    0.0f,    0.0f, 0.0f, 1.0f, color },
            { 1920.0f,    0.0f, 0.0f, 1.0f, color },
            {    0.0f, 1080.0f, 0.0f, 1.0f, color },
            { 1920.0f, 1080.0f, 0.0f, 1.0f, color },
        };

        /* --- Set states for overlay draw --- */
        pSetRenderState(device, D3DRS_ALPHABLENDENABLE, TRUE);
        pSetRenderState(device, D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        pSetRenderState(device, D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        pSetRenderState(device, D3DRS_FOGENABLE, FALSE);

        /* Texture stage: use diffuse color only (ignore textures) */
        pSetTextureStageState(device, 0, D3DTSS_COLOROP, D3DTOP_SELECTARG2);
        pSetTextureStageState(device, 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        pSetTextureStageState(device, 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
        pSetTextureStageState(device, 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

        /* Draw the white quad */
        pDrawPrimitiveUP(device, 4, 2, verts, sizeof(TLVertex));

        /* --- CRITICAL: Restore texture stage states to game defaults! --- */
        /* v4 forgot this, causing new level to render without textures */
        pSetTextureStageState(device, 0, D3DTSS_COLOROP,   D3DTOP_MODULATE);
        pSetTextureStageState(device, 0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        pSetTextureStageState(device, 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        pSetTextureStageState(device, 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE);
        pSetTextureStageState(device, 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        pSetTextureStageState(device, 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

        /* Restore render states */
        pSetRenderState(device, D3DRS_FOGENABLE, TRUE);
    }
}

/* ============================================================
 * Graphics_PresentOrEnd hook
 *
 * Original prologue (7 bytes):
 *   8A 44 24 04    MOV AL, [ESP+4]    (4 bytes)
 *   83 EC 20       SUB ESP, 0x20     (3 bytes)
 *
 * Detour: save ECX (gfx), call drawWhiteOverlay, restore ECX,
 * execute original 7 bytes, JMP back.
 * ============================================================ */

static unsigned char g_presentOrig[8];
static unsigned char *g_presentDetour = NULL;
static volatile void *g_savedGfx = NULL;

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
    int board;
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

    /* Get ball via App→Profile→Board→Scene→ball */
    {
        int profile = *(int *)((char *)app + APP_PROFILE_PTR);
        if (!profile) { return; }
        board = *(int *)((char *)profile + PROFILE_BOARD_PTR);
        if (!board) { return; }
    }
    {
        int scene = *(int *)((char *)board + BOARD_SCENE_PTR_OFFSET);
        if (!scene) { return; }
        ball = *(int *)((char *)scene + 0x29D0);
    }

    switch (g_phase) {
    case PHASE_JIGGLE: {
        elapsed = now - g_phaseStartTime;

        if (g_phaseStartTime == g_warpStartTime) {
            /* First frame of JIGGLE */
            if (ball) {
                g_ballOrigY = *(float *)((char *)ball + BALL_POS_Y);
                *(int *)((char *)ball + BALL_IMPACT_FREEZE) = 1000;
                *(char *)((char *)ball + BALL_IN_TAR) = 1;
            }
            diag_log("[warp] TODO: Play warp_enter sound effect");
            startMusicFade();
            diag_logf("[warp] PHASE_JIGGLE start: ballY=%.2f", g_ballOrigY);
            /* Mark that we've done the init */
            g_phaseStartTime = now;
            g_warpStartTime = now;
        }

        /* Per-frame: jiggle ball upward */
        if (ball) {
            float jiggle = 0.25f;
            *(float *)((char *)ball + BALL_POS_Y) += jiggle;
        }

        updateMusicFade();

        if (elapsed >= JIGGLE_DURATION_MS) {
            g_phase = PHASE_FLASH;
            g_phaseStartTime = now;
            diag_log("[warp] -> PHASE_FLASH");
        }
        break;
    }

    case PHASE_FLASH: {
        elapsed = now - g_phaseStartTime;

        if (elapsed == 0 || (g_phase == PHASE_FLASH && elapsed < 50)) {
            /* First frame of FLASH */
            /* Only do this once — check if we just transitioned */
        }

        /* On first entry: make ball invisible, play sound */
        if (elapsed < 50 && ball) {
            /* This runs on the first frame or two */
            /* Check if ball alpha is still 1.0 (not yet made invisible) */
            float curAlpha = *(float *)((char *)ball + BALL_ALPHA);
            if (curAlpha > 0.5f) {
                diag_log("[warp] TODO: Play warp_exit sound effect");
                *(float *)((char *)ball + BALL_ALPHA) = 0.0f;
                diag_log("[warp] Ball invisible, starting white flash");
            }
        }

        /* Quick flash: ramp up to peak, then back down */
        if (elapsed < FLASH_PEAK_MS) {
            g_whiteAlpha = (float)elapsed / (float)FLASH_PEAK_MS;
        } else {
            DWORD remaining = FLASH_DURATION_MS - elapsed;
            g_whiteAlpha = (float)remaining / (float)(FLASH_DURATION_MS - FLASH_PEAK_MS);
            if (g_whiteAlpha < 0.0f) g_whiteAlpha = 0.0f;
        }

        updateMusicFade();

        if (elapsed >= FLASH_DURATION_MS) {
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
                                g_phaseStartTime = 0;  /* 0 = not yet initialized */
                                g_warpStartTime = 0;
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
static unsigned char g_trampoline[16];
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

    memcpy(g_trampoline, dispatchAddr, 8);
    g_trampoline[8] = 0xE9;
    *(DWORD *)(g_trampoline + 9) = (DWORD)(dispatchAddr + 8) - (DWORD)(g_trampoline + 13);
    VirtualProtect(g_trampoline, sizeof(g_trampoline), PAGE_EXECUTE_READWRITE, &oldProtect);

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

/* Graphics_PresentOrEnd hook */
static void InstallPresentHook(void) {
    unsigned char *presentAddr = (unsigned char *)GRAPHICS_PRESENT_OR_END;
    DWORD oldProtect;

    if (presentAddr[0] != 0x8A || presentAddr[1] != 0x44 ||
        presentAddr[2] != 0x24 || presentAddr[3] != 0x04 ||
        presentAddr[4] != 0x83 || presentAddr[5] != 0xEC || presentAddr[6] != 0x20) {
        diag_log("[FATAL] Graphics_PresentOrEnd signature mismatch!");
        return;
    }
    diag_log("[InstallHooks] Graphics_PresentOrEnd signature OK");

    memcpy(g_presentOrig, presentAddr, 7);

    g_presentDetour = (unsigned char *)VirtualAlloc(NULL, 256, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (!g_presentDetour) return;

    {
        unsigned char *p = g_presentDetour;
        *p++ = 0x51;  /* PUSH ECX */
        *p++ = 0x52;  /* PUSH EDX */
        *p++ = 0xB8; *(DWORD *)p = (DWORD)&drawWhiteOverlay; p += 4;
        *p++ = 0xFF; *p++ = 0xD0;  /* CALL EAX */
        *p++ = 0x5A;  /* POP EDX */
        *p++ = 0x59;  /* POP ECX */
        *p++ = 0x8A; *p++ = 0x44; *p++ = 0x24; *p++ = 0x04;  /* MOV AL, [ESP+4] */
        *p++ = 0x83; *p++ = 0xEC; *p++ = 0x20;  /* SUB ESP, 0x20 */
        *p++ = 0xE9;
        *(DWORD *)p = (DWORD)(presentAddr + 7) - (DWORD)(p + 4);
        p += 4;
    }

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

    diag_log("[warp mod v5] All hooks installed. Ready for E:WARP events.");
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

        diag_log("=== LEVEL WARP MOD v5 LOADED ===");
        diag_log("v5 fixes: texture stage restore, GetTickCount timing, vsnprintf, PHASE_REVEAL");

        load_real_bass();
        diag_logf("bass_real.dll handle: 0x%08X", (unsigned)g_hRealBass);

        CreateThread(NULL, 0, InitThread, NULL, 0, NULL);
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
