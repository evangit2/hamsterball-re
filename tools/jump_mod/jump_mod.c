/*
 * jump_mod.c — BASS.dll proxy that lets Player 1 jump with the spacebar.
 *
 * A background thread polls every ~16ms:
 *   1. Finds Player 1's ball via App → Scene → all_balls_list
 *   2. Checks GetAsyncKeyState(VK_SPACE) for a rising edge (press, not hold)
 *   3. Checks ball+0xC4C == 0 (not in fall-off-level mode)
 *   4. Checks ball+0x748 == 1 (gravity plane is Y = on normal floor)
 *   5. Checks ball+0x174 (vel_Y) <= threshold (not already moving upward)
 *   6. If all conditions met, sets ball+0x174 = JUMP_VELOCITY (upward impulse)
 *   7. Cooldown timer prevents rapid re-jumping
 *
 * Installation:
 *   1. Rename bass.dll → bass_real.dll in your Hamsterball game folder
 *   2. Copy this bass.dll into the game folder
 *   3. Launch Hamsterball — press SPACE during gameplay to jump
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll jump_mod.c -lwinmm \
 *     -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc -Wl,--add-stdcall-alias
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * BASS Proxy Exports — forward all 10 game imports to bass_real.dll
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

/* Extra BASS stubs (not imported by Hamsterball but may be needed) */
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
 * Struct Offsets (from Ghidra decompilation)
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Global App pointer — set at runtime by the game */
#define G_APP_ADDR       0x005341E0

/* Ball struct offsets */
#define BALL_SCENE        0x014   /* void* — Scene/Board pointer */
#define BALL_PLAYER_IDX   0x018   /* int: -1=NPC, 0=Player 1 */
#define BALL_VEL_X        0x170   /* float — velocity X */
#define BALL_VEL_Y        0x174   /* float — velocity Y (up = positive) */
#define BALL_VEL_Z        0x178   /* float — velocity Z */
#define BALL_IS_FALLING   0x281   /* byte — 1=ball is falling off level */
#define BALL_FALL_MODE    0xC4C   /* int — 0=normal, 1=fall-off-level mode */
#define BALL_GRAVITY_AXIS 0x748   /* int — gravity plane: 0=X, 1=Y, 2=Z */

/* Scene/Board struct offsets */
#define SCENE_ALL_BALLS   0x2DEC   /* AthenaList (all_balls_list) */

/* AthenaList struct offsets */
#define LIST_COUNT         0x004   /* int — element count */
#define LIST_ARRAY         0x40C   /* int* — heap array of ball pointers */

/* Scene vtable (used for validation) */
#define SCENE_VTABLE       0x4D0260

/* Jump parameters */
#define JUMP_VELOCITY      400.0f   /* upward velocity impulse */
#define VEL_Y_THRESHOLD    50.0f    /* max upward vel_Y to still allow jump */
#define JUMP_COOLDOWN_MS   300      /* min ms between jumps */
#define POLL_INTERVAL_MS   16       /* thread sleep between polls (~60fps) */

/* VK_SPACE = 0x20 (spacebar) */
#define VK_SPACE_KEY       0x20

/* ═══════════════════════════════════════════════════════════════════════════
 * Find Scene pointer by scanning App struct
 * ═══════════════════════════════════════════════════════════════════════════ */

static void* find_scene(void)
{
    DWORD app = *(DWORD*)G_APP_ADDR;
    if (!app || app < 0x10000) return NULL;
    if (IsBadReadPtr((void*)app, 0x300)) return NULL;

    /* Scan App struct for a pointer to the current Board/Scene.
     * The Board vtable is at 0x4D0260 — validate candidates by checking
     * if their first DWORD matches, or if they have a valid all_balls_list. */
    for (int offset = 0x100; offset < 0xA00; offset += 4) {
        DWORD candidate = *(DWORD*)((BYTE*)app + offset);
        if (candidate == 0 || candidate < 0x10000) continue;
        if (IsBadReadPtr((void*)candidate, 0x3000)) continue;

        /* Check if this looks like a Scene: all_balls_list at +0x2DEC */
        DWORD list_count = *(DWORD*)((BYTE*)candidate + SCENE_ALL_BALLS + LIST_COUNT);
        DWORD list_array = *(DWORD*)((BYTE*)candidate + SCENE_ALL_BALLS + LIST_ARRAY);

        if (list_count > 0 && list_count < 100 && list_array != 0) {
            if (!IsBadReadPtr((void*)list_array, 4 * list_count)) {
                DWORD first_ball = *(DWORD*)list_array;
                if (first_ball != 0 && !IsBadReadPtr((void*)first_ball, 0x20)) {
                    /* Validate: ball should have a valid vtable pointer */
                    DWORD ball_vtable = *(DWORD*)first_ball;
                    if (ball_vtable > 0x400000 && ball_vtable < 0x600000) {
                        return (void*)candidate;
                    }
                }
            }
        }
    }
    return NULL;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Find Player 1's ball in the all_balls_list
 * ═══════════════════════════════════════════════════════════════════════════ */

static void* find_player_ball(void *scene)
{
    int count = *(int*)((BYTE*)scene + SCENE_ALL_BALLS + LIST_COUNT);
    int *array = *(int**)((BYTE*)scene + SCENE_ALL_BALLS + LIST_ARRAY);

    if (count <= 0 || count > 100) return NULL;
    if (IsBadReadPtr((void*)array, 4 * count)) return NULL;

    for (int i = 0; i < count; i++) {
        DWORD ball = array[i];
        if (ball == 0 || ball < 0x10000) continue;
        if (IsBadReadPtr((void*)ball, 0xD00)) continue;

        int player_idx = *(int*)((BYTE*)ball + BALL_PLAYER_IDX);
        if (player_idx == 0) {
            return (void*)ball;
        }
    }
    return NULL;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Jump Thread — polls spacebar and applies jump impulse
 * ═══════════════════════════════════════════════════════════════════════════ */

static DWORD WINAPI jump_thread(LPVOID param)
{
    (void)param;

    /* Wait for game to fully load */
    Sleep(3000);

    int space_was_down = 0;
    DWORD last_jump_time = 0;

    for (;;) {
        Sleep(POLL_INTERVAL_MS);

        /* Edge detection: only trigger on key press (rising edge), not hold */
        SHORT key_state = GetAsyncKeyState(VK_SPACE_KEY);
        int space_is_down = (key_state & 0x8000) ? 1 : 0;
        int space_pressed = space_is_down && !space_was_down;
        space_was_down = space_is_down;

        if (!space_pressed) continue;

        /* Find the Scene */
        void *scene = find_scene();
        if (!scene) continue;

        /* Find Player 1's ball */
        void *ball = find_player_ball(scene);
        if (!ball) continue;

        /* Check "on ground" conditions:
         * 1. Not in fall-off-level mode (ball+0xC4C == 0)
         * 2. Not already falling (ball+0x281 == 0)
         * 3. Gravity plane is Y axis (ball+0x748 == 1) = ball is on a floor
         * 4. Vertical velocity is near zero or downward (ball+0x174 <= threshold)
         *    This prevents double-jumping in mid-air */
        int fall_mode = *(int*)((BYTE*)ball + BALL_FALL_MODE);
        if (fall_mode != 0) continue;

        BYTE is_falling = *(BYTE*)((BYTE*)ball + BALL_IS_FALLING);
        if (is_falling != 0) continue;

        int gravity_axis = *(int*)((BYTE*)ball + BALL_GRAVITY_AXIS);
        if (gravity_axis != 1) continue;  /* 1 = Y axis = on floor */

        float vel_y = *(float*)((BYTE*)ball + BALL_VEL_Y);
        if (vel_y > VEL_Y_THRESHOLD) continue;  /* already moving up, don't double-jump */

        /* Cooldown check */
        DWORD now = GetTickCount();
        if (now - last_jump_time < JUMP_COOLDOWN_MS) continue;

        /* Apply jump impulse! */
        *(float*)((BYTE*)ball + BALL_VEL_Y) = JUMP_VELOCITY;
        last_jump_time = now;
    }

    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DllMain — load bass_real.dll and spawn jump thread
 * ═══════════════════════════════════════════════════════════════════════════ */

BOOL APIENTRY DllMain(HMODULE hInst, DWORD reason, LPVOID lpReserved)
{
    (void)lpReserved;
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hInst);
        load_real_bass();
        CreateThread(NULL, 0, jump_thread, NULL, 0, NULL);
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
