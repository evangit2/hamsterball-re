/*
 * ball_break.c — BASS.dll proxy mod
 *
 * Press X to break your ball and respawn at the nearest checkpoint.
 *
 * How it works:
 *   1. Background thread polls DIK_X (0x2D) every 16ms
 *   2. On rising-edge keypress, checks gates (countdown done, race active)
 *   3. Calls Ball_SplitIntoThree(ball, &dummy_vec3list) — the game's own
 *      "ball breaks into 3 pieces" function
 *   4. Ball_SplitIntoThree sets ball+0x2E8=1 (shattered flag)
 *   5. Next frame, Scene_UpdateBallsAndState sees the shattered flag and
 *      calls Ball_FindClosestRespawnPoint → teleports ball to nearest
 *      respawn point, clears velocity, starts fall animation
 *
 * Game functions called:
 *   Ball_SplitIntoThree (0x408D70) — __thiscall: ECX=ball, [ESP+4]=Vec3List*
 *     - If Vec3List count=0, uses ball position as fallback for split directions
 *     - Sets ball+0x2E8=1 (shattered), creates 3 debris balls, plays sound
 *
 * Gates (same as jump mod):
 *   - Countdown: scene+0x3A4C must be 1 (countdown finished)
 *   - Race end: App+0x5D6 (player finished flag) must be 0
 *   - App+0x5D5 (another player flag) must be 0
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll ball_break.c -lwinmm \
 *     -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
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
/* Extra exports for safety */
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
__declspec(dllexport) int __stdcall BASS_ChannelSetAttribute(DWORD a, DWORD b, float c) { return 1; }
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

/* ═══════════════════════════════════════════════════════════════════════════
 * Ball Break Mod
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Game addresses */
#define ADDR_App                    0x005341E0
#define ADDR_Ball_SplitIntoThree    0x00408D70

/* Ball struct offsets */
#define BALL_BOARD          0x014   /* board pointer */
#define BALL_PLAYER_IDX     0x018   /* player index (0=P1, -1=AI/none) */
#define BALL_POS_X          0x164
#define BALL_POS_Y          0x168
#define BALL_POS_Z          0x16C
#define BALL_IS_SHATTERED   0x2E8   /* set to 1 by Ball_SplitIntoThree */
#define BALL_IS_INVINCIBLE  0x324   /* if 1, ball is destroyed instead of respawned */

/* Scene offsets (via ball+0x14 → board, board+0x878 → scene/app) */
#define SCENE_COUNTDOWN_DONE  0x3A4C

/* App offsets */
#define APP_PLAYER_FINISHED   0x5D6  /* player_data[0]+0x0A */
#define APP_PLAYER_FLAG2      0x5D5  /* player_data[0]+0x09 */

/* DIK_X = 0x2D in DirectInput key enum */
#define DIK_X  0x2D

/* Ball_SplitIntoThree is __thiscall: ECX=ball (this), [ESP+4]=Vec3List*
 * RET 4 (callee cleans 4 bytes = 1 stack param)
 *
 * The Vec3List param provides directional vectors for the 3 split pieces.
 * If the list count (at +4) is 0, the function falls back to using the
 * ball's own position for all 3 directions — which is fine for our purpose.
 *
 * We pass a zeroed 0x410-byte buffer (AthenaList is 0x410 bytes min).
 * With count=0, no data array is accessed, so NULL data ptr is safe.
 */
typedef void (__attribute__((thiscall)) *SplitIntoThree_t)(
    void *ball,      /* this → ECX */
    void *vec3list   /* [ESP+4], RET 4 */
);
static SplitIntoThree_t Ball_SplitIntoThree = (SplitIntoThree_t)ADDR_Ball_SplitIntoThree;

/* Shared state */
static volatile DWORD g_break_count = 0;

/* Dummy Vec3List — zeroed = count 0, data NULL.
 * Ball_SplitIntoThree checks *(int*)(param_1+4) for count.
 * With count=0, it uses ball position as fallback for all 3 splits.
 * Size 0x410 to match AthenaList layout (count at +4, data at +0x40C). */
static char g_dummy_vec3list[0x410];

/* ─── Get player ball pointer ─────────────────────────────────────────────── */
static DWORD get_player_ball(void)
{
    DWORD app = *(DWORD*)ADDR_App;
    if (!app || app < 0x10000) return 0;
    if (IsBadReadPtr((void*)app, 0x300)) return 0;

    /* Scene pointer: ball+0x14 = board, board+0x878 = scene
     * But we need to find the ball first. Use App→+0x220→profile→+0xC→board
     * then board→scene+0x29D4 ball list.
     * Simpler: scan App for scene pointer (like direction_detect does). */
    for (int off = 0x100; off < 0xA00; off += 4) {
        DWORD candidate = *(DWORD*)((BYTE*)app + off);
        if (candidate == 0 || candidate < 0x10000) continue;
        if (IsBadReadPtr((void*)candidate, 0x3000)) continue;

        /* Check for ball_list at Scene+0x29D4 (AthenaList: count at +4) */
        DWORD count = *(DWORD*)((BYTE*)candidate + 0x29D4 + 0x004);
        DWORD array = *(DWORD*)((BYTE*)candidate + 0x29D4 + 0x40C);
        if (count > 0 && count < 100 && array != 0 && !IsBadReadPtr((void*)array, count * 4)) {
            /* Found scene — find player ball (player_idx == 0) */
            for (int i = 0; i < (int)count; i++) {
                DWORD ball = *(DWORD*)(array + i * 4);
                if (!ball || ball < 0x10000) continue;
                if (IsBadReadPtr((void*)ball, 0xD00)) continue;
                if (*(int*)((BYTE*)ball + BALL_PLAYER_IDX) == 0) {
                    return ball;  /* Player 1 */
                }
            }
        }
    }
    return 0;
}

/* ─── Input polling thread ─────────────────────────────────────────────────── */
static volatile int g_prev_x = 0;

static DWORD WINAPI input_thread(LPVOID param)
{
    (void)param;
    Sleep(3000);  /* Wait for game to load */

    while (1) {
        Sleep(16);  /* ~60fps poll */

        DWORD app = *(DWORD*)ADDR_App;
        if (!app || app < 0x10000) continue;
        if (IsBadReadPtr((void*)app, 0x200)) continue;

        /* Get keyboard device: App+0x180 = InputHandler, +0x434 = KeyboardDevice */
        DWORD ih = *(DWORD*)((BYTE*)app + 0x180);
        if (!ih || IsBadReadPtr((void*)ih, 0x500)) continue;

        DWORD kb = *(DWORD*)((BYTE*)ih + 0x434);
        if (!kb || IsBadReadPtr((void*)kb, 0x600)) continue;

        /* Key state: KeyboardDevice + 0xC + DIK_X = offset 0xC + 0x2D = 0x39 */
        BYTE key_state = *((BYTE*)kb + 0xC + DIK_X);
        int x_down = (key_state & 0x80) != 0;

        if (x_down && !g_prev_x) {
            /* X just pressed — check gates */

            /* Gate 1: Countdown must be done (scene+0x3A4C == 1) */
            DWORD ball = get_player_ball();
            if (!ball) goto next_key;

            DWORD board = *(DWORD*)((BYTE*)ball + BALL_BOARD);
            if (!board || IsBadReadPtr((void*)board, 0x900)) goto next_key;

            DWORD scene = *(DWORD*)((BYTE*)board + 0x878);
            if (!scene || IsBadReadPtr((void*)scene, 0x4000)) goto next_key;

            BYTE countdown_done = *(BYTE*)((BYTE*)scene + SCENE_COUNTDOWN_DONE);
            if (!countdown_done) goto next_key;

            /* Gate 2: Race not ended (App+0x5D6 == 0) */
            BYTE finished = *(BYTE*)((BYTE*)app + APP_PLAYER_FINISHED);
            if (finished) goto next_key;

            /* Gate 3: Other player flag (App+0x5D5 == 0) */
            BYTE flag2 = *(BYTE*)((BYTE*)app + APP_PLAYER_FLAG2);
            if (flag2) goto next_key;

            /* Gate 4: Don't break if already shattered */
            BYTE is_shattered = *(BYTE*)((BYTE*)ball + BALL_IS_SHATTERED);
            if (is_shattered) goto next_key;

            /* All gates passed — break the ball!
             * Ball_SplitIntoThree sets ball+0x2E8=1.
             * Next frame, Scene_UpdateBallsAndState detects it and calls
             * Ball_FindClosestRespawnPoint → respawn at nearest checkpoint. */
            Ball_SplitIntoThree((void*)ball, (void*)g_dummy_vec3list);
            g_break_count++;
        }

    next_key:
        g_prev_x = x_down;
    }
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DllMain
 * ═══════════════════════════════════════════════════════════════════════════ */

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
    (void)hModule;
    (void)lpReserved;

    if (reason == DLL_PROCESS_ATTACH) {
        /* Zero out the dummy Vec3List */
        ZeroMemory(g_dummy_vec3list, sizeof(g_dummy_vec3list));

        load_real_bass();
        CreateThread(NULL, 0, input_thread, NULL, 0, NULL);
    }
    return TRUE;
}
