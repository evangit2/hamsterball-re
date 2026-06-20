/*
 * player_clones_mod.c — BASS.dll proxy that clones players 1-4 with AI.
 *
 * Features:
 *   1. Clones any player (1-4) with configurable count per player
 *   2. Cloned balls use the same COMPUTER AI as Rodent Rumble arenas
 *   3. Clones share their original player's score (same player_index)
 *   4. Works in BOTH races and arenas (patches the AI gate for races)
 *   5. AI targets ALL entities (players + other clones + 8balls)
 *
 * ═══════════════════════════════════════════════════════════════════════════
 * HOW IT WORKS
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * The game's AI chase logic lives in Ball_UpdateAndAI (0x408390, Ball vtable[4]).
 * It has a GATE at 0x4083AE that skips AI unless:
 *   - ball+0xC74 (collision_contacts) != 0, OR
 *   - App+0x237 (arena_mode) != 0
 *
 * PATCH 1: NOP the JZ at 0x4083AE so AI runs in races too.
 *
 * The AI targeting loop iterates Scene+0x29D4 (bad_balls_list) and skips any
 * ball with player_index == -1 (the "CMP [edi+0x18], -1; JE skip" at 0x4085CD).
 * This means 8-balls can't target other 8-balls. But cloned player balls have
 * player_index 0-3, so they PASS this filter naturally.
 *
 * PATCH 2: Change the targeting filter to self-exclusion (cmp esi, edi) so
 * ALL balls can target ALL other balls (including other clones).
 *
 * CLONE CREATION:
 *   A background thread waits for the game to spawn balls, then:
 *   1. Reads the first ball's Scene pointer (ball+0x14)
 *   2. For each player to clone, finds that player's ball in the all_balls_list
 *   3. Creates N clones via operator_new(0xC98) + Ball_ctor(mem, scene)
 *   4. Copies position, sets AI params (CHASE_DISTANCE, HOME_RADIUS, SPIN_DISTANCE)
 *   5. Sets player_index to match original (for score sharing)
 *   6. Adds clones to bad_balls_list and all_balls_list
 *
 * SCORE SHARING:
 *   Score is at App+0x5E4 + player_index * 0xA0 (float).
 *   Arena knockoff score at RumbleBoard+0x47B4 + player_index * 4 (int).
 *   Since clones share the same player_index as the original, any score
 *   they earn automatically goes to the original player's total.
 *
 * ═══════════════════════════════════════════════════════════════════════════
 * BUILD
 * ═══════════════════════════════════════════════════════════════════════════
 *
 *   i686-w64-mingw32-gcc -shared -o bass.dll player_clones_mod.c \
 *       bass_exports.def -lwinmm -Wl,--enable-stdcall-fixup \
 *       -O2 -static -static-libgcc -Wl,--add-stdcall-alias
 *
 * Installation (Windows):
 *   1. In your Hamsterball game folder, rename bass.dll -> bass_real.dll
 *   2. Copy this proxy bass.dll into the game folder
 *   3. Copy player_clones.ini into the game folder
 *   4. Edit player_clones.ini to set clone counts per player
 *
 * If you already use the FPS mod's bass.dll, you can merge this into that
 * proxy — both use the same pattern. Just combine the patch_thread functions.
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winuser.h>
#include <mmsystem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * BASS Proxy Exports (stubs — Hamsterball only needs import resolution)
 * ═══════════════════════════════════════════════════════════════════════════ */

__declspec(dllexport) void BASS_Init(void) {}
__declspec(dllexport) void BASS_Free(void) {}
__declspec(dllexport) void BASS_Start(void) {}
__declspec(dllexport) void BASS_Stop(void) {}
__declspec(dllexport) void BASS_Pause(void) {}
__declspec(dllexport) void BASS_SetVolume(void) {}
__declspec(dllexport) void BASS_GetVolume(void) {}
__declspec(dllexport) void BASS_GetDevice(void) {}
__declspec(dllexport) void BASS_SetDevice(void) {}
__declspec(dllexport) void BASS_GetInfo(void) {}
__declspec(dllexport) void BASS_Update(void) {}
__declspec(dllexport) void BASS_ErrorGetCode(void) {}
__declspec(dllexport) void BASS_StreamCreateFile(void) {}
__declspec(dllexport) void BASS_MusicLoad(void) {}
__declspec(dllexport) void BASS_SampleLoad(void) {}
__declspec(dllexport) void BASS_ChannelPlay(void) {}
__declspec(dllexport) void BASS_ChannelStop(void) {}
__declspec(dllexport) void BASS_ChannelSetAttribute(void) {}
__declspec(dllexport) void BASS_ChannelGetAttribute(void) {}
__declspec(dllexport) void BASS_ChannelGetData(void) {}
__declspec(dllexport) void BASS_ChannelGetLevel(void) {}
__declspec(dllexport) void BASS_ChannelSetPosition(void) {}
__declspec(dllexport) void BASS_ChannelGetPosition(void) {}
__declspec(dllexport) void BASS_ChannelIsActive(void) {}
__declspec(dllexport) void BASS_ChannelRemoveSync(void) {}
__declspec(dllexport) void BASS_ChannelSetSync(void) {}
__declspec(dllexport) void BASS_SampleCreate(void) {}
__declspec(dllexport) void BASS_SampleGetChannel(void) {}

/* If we loaded the real bass_real.dll, forward to it */
static HMODULE g_hRealBass = NULL;

typedef void (__stdcall *BASS_Stop_t)(void);
typedef int  (__stdcall *BASS_ChannelSetAttributes_t)(DWORD, float, int, int);
typedef int  (__stdcall *BASS_Free_t)(void);
typedef int  (__stdcall *BASS_Init_t)(int, DWORD, DWORD, HWND, void*);
typedef int  (__stdcall *BASS_Start_t)(void);
typedef int  (__stdcall *BASS_SetConfig_t)(DWORD, DWORD);
typedef int  (__stdcall *BASS_ChannelStop_t)(DWORD);
typedef int  (__stdcall *BASS_MusicPlayEx_t)(DWORD, DWORD, BOOL);
typedef int  (__stdcall *BASS_ErrorGetCode_t)(void);
typedef DWORD (__stdcall *BASS_MusicLoad_t)(int, void*, DWORD, DWORD, DWORD, DWORD);

static BASS_Stop_t              real_BASS_Stop = NULL;
static BASS_ChannelSetAttributes_t real_BASS_ChannelSetAttributes = NULL;
static BASS_Free_t              real_BASS_Free = NULL;
static BASS_Init_t              real_BASS_Init = NULL;
static BASS_Start_t             real_BASS_Start = NULL;
static BASS_SetConfig_t         real_BASS_SetConfig = NULL;
static BASS_ChannelStop_t       real_BASS_ChannelStop = NULL;
static BASS_MusicPlayEx_t       real_BASS_MusicPlayEx = NULL;
static BASS_ErrorGetCode_t      real_BASS_ErrorGetCode = NULL;
static BASS_MusicLoad_t         real_BASS_MusicLoad = NULL;

/* ═══════════════════════════════════════════════════════════════════════════
 * Game Constants & Struct Offsets
 * ═══════════════════════════════════════════════════════════════════════════ */

#define IMAGE_BASE        0x400000

/* Global App pointer (BSS, filled at runtime) */
#define APP_PTR_ADDR      0x005341E0

/* Function addresses (VA) */
#define OPERATOR_NEW      0x004BA57B
#define BALL_CTOR         0x0040AFE0   /* __thiscall: ECX=mem, stack=scene */
#define ATHENA_LIST_APPEND 0x00453780  /* __thiscall: ECX=list, stack=item */

/* Ball struct offsets */
#define BALL_VTABLE       0x000
#define BALL_APP          0x010   /* App* */
#define BALL_SCENE        0x014   /* Scene/Board* */
#define BALL_PLAYER_IDX   0x018   /* int: -1=NPC, 0-3=Player 1-4 */
#define BALL_DISPLAY_X    0x164   /* float */
#define BALL_DISPLAY_Y    0x168   /* float */
#define BALL_DISPLAY_Z    0x16C   /* float */
#define BALL_RADIUS_VAL   0x284   /* float: ball size value */
#define BALL_DEAD          0x768   /* byte: 0=alive, nonzero=dead/fallen */
#define BALL_HOME_X       0xC60   /* float: home/spawn position */
#define BALL_HOME_Y       0xC64
#define BALL_HOME_Z       0xC68
#define BALL_CHASE_DIST   0xC6C   /* float: AI chase activation range */
#define BALL_HOME_RADIUS  0xC70   /* float: AI patrol radius */
#define BALL_SPIN_DIST    0xC7C   /* float: AI orbit distance */
#define BALL_COLLISIONS   0xC74   /* int: collision contact counter */
#define BALL_SIZE_FLAG    0xC4C   /* byte: 1 if SIZE tag set */
#define BALL_ALLOC_SIZE   0xC98   /* allocation size for Ball */

/* Scene/Board struct offsets */
#define SCENE_BAD_BALLS   0x29D4   /* AthenaList (bad_balls_list) */
#define SCENE_ALL_BALLS   0x2DEC   /* AthenaList (all_balls_list) */

/* AthenaList struct offsets */
#define ATHENA_LIST_COUNT  0x004   /* int: element count */
#define ATHENA_LIST_ARRAY  0x40C   /* int*: heap-allocated element array */

/* App struct offsets */
#define APP_ARENA_MODE    0x237   /* byte: 1=in arena/game-in-progress */

/* Patch sites (RVA = VA - IMAGE_BASE) */
#define GATE0_RVA         0x83AE   /* JZ → NOP (enable AI in races) */
#define FILTER_RVA        0x85CD   /* targeting filter → self-exclusion */

/* AI default params for cloned balls */
#define DEFAULT_CHASE_DIST  1500.0f   /* large range = always chase */
#define DEFAULT_HOME_RADIUS 500.0f    /* moderate patrol radius */
#define DEFAULT_SPIN_DIST   100.0f    /* tight orbit when no target */
#define DEFAULT_BALL_RADIUS  3.0f     /* standard ball radius (3.0=normal, not giant) */

/* ═══════════════════════════════════════════════════════════════════════════
 * Config
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef struct {
    int clones_per_player[4];  /* how many clones for player 1-4 */
    int enable_race_ai;         /* 1=patch gate for races, 0=arenas only */
    int target_all;             /* 1=patch targeting filter, 0=players only */
} clone_config_t;

static clone_config_t g_cfg = {
    { 0, 0, 0, 0 },  /* default: no clones */
    1,                 /* enable AI in races */
    1,                 /* target all entities */
};

static int g_clones_spawned = 0;  /* flag: have we spawned clones for this level? */

/* ═══════════════════════════════════════════════════════════════════════════
 * Helpers
 * ═══════════════════════════════════════════════════════════════════════════ */

static int file_exists(const char *path)
{
    DWORD attr = GetFileAttributesA(path);
    return attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY);
}

static int read_ini_int(const char *path, const char *section, const char *key, int default_val)
{
    char buf[32];
    DWORD n = GetPrivateProfileStringA(section, key, "", buf, sizeof(buf), path);
    if (n == 0) return default_val;
    return atoi(buf);
}

static void load_config(const char *ini_path)
{
    if (!file_exists(ini_path)) {
        /* Write a default config so users know what to edit */
        FILE *f = fopen(ini_path, "w");
        if (f) {
            fprintf(f, "[Clones]\n");
            fprintf(f, "; Number of AI clones per player (0=none)\n");
            fprintf(f, "; Clones share their original player's score\n");
            fprintf(f, "; Clones use COMPUTER AI (chase all entities)\n");
            fprintf(f, "Player1=0\n");
            fprintf(f, "Player2=2\n");
            fprintf(f, "Player3=1\n");
            fprintf(f, "Player4=0\n\n");
            fprintf(f, "[Options]\n");
            fprintf(f, "; 1=enable AI in races (not just arenas)\n");
            fprintf(f, "EnableRaceAI=1\n");
            fprintf(f, "; 1=clones target ALL balls (players+clones+8balls)\n");
            fprintf(f, "; 0=clones only target players 1-4\n");
            fprintf(f, "TargetAll=1\n");
            fclose(f);
        }
    }
    g_cfg.clones_per_player[0] = read_ini_int(ini_path, "Clones", "Player1", 0);
    g_cfg.clones_per_player[1] = read_ini_int(ini_path, "Clones", "Player2", 0);
    g_cfg.clones_per_player[2] = read_ini_int(ini_path, "Clones", "Player3", 0);
    g_cfg.clones_per_player[3] = read_ini_int(ini_path, "Clones", "Player4", 0);
    g_cfg.enable_race_ai = read_ini_int(ini_path, "Options", "EnableRaceAI", 1);
    g_cfg.target_all = read_ini_int(ini_path, "Options", "TargetAll", 1);

    /* Clamp */
    for (int i = 0; i < 4; i++) {
        if (g_cfg.clones_per_player[i] < 0) g_cfg.clones_per_player[i] = 0;
        if (g_cfg.clones_per_player[i] > 8) g_cfg.clones_per_player[i] = 8;
    }
}

/* Memory patching: verify-then-patch */
static int patch_bytes(BYTE *addr, const BYTE *expected, const BYTE *replacement, SIZE_T len)
{
    DWORD oldProtect;
    if (memcmp(addr, expected, len) != 0)
        return 0;  /* wrong version or already patched */
    if (!VirtualProtect(addr, len, PAGE_EXECUTE_READWRITE, &oldProtect))
        return 0;
    memcpy(addr, replacement, len);
    VirtualProtect(addr, len, oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), addr, len);
    return 1;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Game Function Wrappers
 * ═══════════════════════════════════════════════════════════════════════════ */

/* operator_new(size_t) — __cdecl, returns void* in EAX */
typedef void* (__cdecl *operator_new_t)(size_t);
static operator_new_t pfn_operator_new = NULL;

/* Ball_ctor(void* mem, void* scene) — __thiscall: ECX=mem, stack=scene
 * Returns Ball* (in EAX) */
typedef void* (__thiscall *ball_ctor_t)(void* mem, void* scene);
static ball_ctor_t pfn_ball_ctor = NULL;

/* AthenaList_Append(void* list, int item) — __thiscall: ECX=list, stack=item */
typedef void (__thiscall *athena_list_append_t)(void* list, int item);
static athena_list_append_t pfn_athena_list_append = NULL;

static void resolve_functions(void)
{
    HMODULE hExe = GetModuleHandleA(NULL);
    if (!hExe) return;
    BYTE *base = (BYTE*)hExe;
    pfn_operator_new = (operator_new_t)(base + (OPERATOR_NEW - IMAGE_BASE));
    pfn_ball_ctor = (ball_ctor_t)(base + (BALL_CTOR - IMAGE_BASE));
    pfn_athena_list_append = (athena_list_append_t)(base + (ATHENA_LIST_APPEND - IMAGE_BASE));
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Clone Creation
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Find a player's ball in the all_balls_list by player_index.
 * Returns the Ball* or NULL if not found. */
static void* find_player_ball(void* scene, int player_index)
{
    BYTE *s = (BYTE*)scene;
    int count = *(int*)(s + SCENE_ALL_BALLS + ATHENA_LIST_COUNT);
    int *array = *(int**)(s + SCENE_ALL_BALLS + ATHENA_LIST_ARRAY);
    if (!array) return NULL;
    for (int i = 0; i < count; i++) {
        void *ball = (void*)array[i];
        if (!ball) continue;
        if (IsBadReadPtr(ball, 0x20)) continue;
        int idx = *(int*)((BYTE*)ball + BALL_PLAYER_IDX);
        if (idx == player_index)
            return ball;
    }
    return NULL;
}

/* Create a single clone of a player's ball.
 * Returns the new Ball* or NULL on failure. */
static void* create_clone(void* scene, void* source_ball, int player_index)
{
    BYTE *src = (BYTE*)source_ball;
    BYTE *s = (BYTE*)scene;

    /* 1. Allocate memory for the new ball */
    void *mem = pfn_operator_new(BALL_ALLOC_SIZE);
    if (!mem) return NULL;

    /* 2. Construct the ball: Ball_ctor(mem, scene)
     *    __thiscall: ECX=mem, push scene */
    void *clone = pfn_ball_ctor(mem, scene);
    if (!clone) return NULL;

    BYTE *c = (BYTE*)clone;

    /* 3. Call vtable[1] = Ball_InitPhysicsDefaults */
    void **vtable = *(void***)c;
    void (__thiscall *init_fn)(void*) = (void (__thiscall *)(void*))vtable[1];
    init_fn(clone);

    /* 4. Copy position from source ball (offset slightly to avoid overlap) */
    float src_x = *(float*)(src + BALL_DISPLAY_X);
    float src_y = *(float*)(src + BALL_DISPLAY_Y);
    float src_z = *(float*)(src + BALL_DISPLAY_Z);
    /* Small random offset so clones don't stack perfectly */
    src_x += 10.0f;

    *(float*)(c + BALL_DISPLAY_X) = src_x;
    *(float*)(c + BALL_DISPLAY_Y) = src_y;
    *(float*)(c + BALL_DISPLAY_Z) = src_z;

    /* 5. Set home position (where AI returns to when no target) */
    *(float*)(c + BALL_HOME_X) = src_x;
    *(float*)(c + BALL_HOME_Y) = src_y;
    *(float*)(c + BALL_HOME_Z) = src_z;

    /* 6. Set AI parameters — these control the COMPUTER behavior */
    *(float*)(c + BALL_CHASE_DIST)  = DEFAULT_CHASE_DIST;
    *(float*)(c + BALL_HOME_RADIUS) = DEFAULT_HOME_RADIUS;
    *(float*)(c + BALL_SPIN_DIST)   = DEFAULT_SPIN_DIST;

    /* 7. Copy ball radius/size from source */
    *(float*)(c + BALL_RADIUS_VAL) = *(float*)(src + BALL_RADIUS_VAL);

    /* 8. Set player_index to match original → score is shared! */
    *(int*)(c + BALL_PLAYER_IDX) = player_index;

    /* 9. Ensure the ball is alive and active */
    *(BYTE*)(c + BALL_DEAD) = 0;

    /* 10. Add to scene ball lists */
    pfn_athena_list_append(s + SCENE_BAD_BALLS, (int)clone);
    pfn_athena_list_append(s + SCENE_ALL_BALLS, (int)clone);

    return clone;
}

/* Find Scene pointer by reading ball+0x14 from the first ball in the all_balls_list.
 * Returns Scene* or NULL. */
static void* find_scene_from_balls(void)
{
    /* Read App pointer from global */
    DWORD *pAppPtr = (DWORD*)APP_PTR_ADDR;
    if (IsBadReadPtr(pAppPtr, 4)) return NULL;
    DWORD app = *pAppPtr;
    if (!app || IsBadReadPtr((void*)app, 0x300)) return NULL;

    /* We need the Scene pointer. Each ball stores it at ball+0x14.
     * But we need to find a ball first. The ball lists are IN the Scene...
     *
     * Strategy: Search memory for the Scene/Board vtable pattern.
     * The Board/Scene vtable is at 0x4D0260. When a Board is constructed,
     * its first 4 bytes are set to this vtable pointer. We can scan the
     * heap for this pattern.
     *
     * Alternative: The App struct stores the Scene somewhere.
     * From the App struct definition, there's a large gap from +0x560 to +0x914.
     * Let's try reading App as a pointer table and look for Scene-like pointers.
     *
     * Actually, the simplest approach: poll for any ball in the all_balls_list
     * by checking if Scene+0x2DEC has a count > 0. But we need Scene first...
     *
     * BRUTE FORCE: Scan App struct for pointers that point to objects whose
     * +0x00 matches the Scene/Board vtable (0x4D0260). The Board_ctor sets
     * this vtable. */
    DWORD board_vtable = 0x4D0260;

    /* Scan App struct for a pointer to the current Board/Scene */
    DWORD *app_ptr = (DWORD*)app;
    for (int offset = 0x100; offset < 0xA00; offset += 4) {
        DWORD candidate = *(DWORD*)((BYTE*)app + offset);
        if (candidate == 0 || candidate < 0x10000) continue;
        if (IsBadReadPtr((void*)candidate, 4)) continue;
        DWORD vt = *(DWORD*)candidate;
        if (vt == board_vtable) {
            return (void*)candidate;
        }
    }

    /* Also try the RumbleBoard vtable (0x4D0260 is the base Scene vtable,
     * RumbleBoard might use a different one) */
    /* From memory: Ball vtable=0x4CF3A0, Scene vtable=0x4D0260 */
    /* Let's also scan for the bad_balls_list pattern: a valid AthenaList
     * at candidate+0x29D4 with count > 0 */
    for (int offset = 0x100; offset < 0xA00; offset += 4) {
        DWORD candidate = *(DWORD*)((BYTE*)app + offset);
        if (candidate == 0 || candidate < 0x10000) continue;
        if (IsBadReadPtr((void*)candidate, 0x3000)) continue;
        /* Check if this looks like a Scene: +0x29D4 has a valid AthenaList */
        DWORD list_count = *(DWORD*)((BYTE*)candidate + 0x29D8);  /* count at list+0x04 */
        DWORD list_array = *(DWORD*)((BYTE*)candidate + 0x2DE0); /* array at list+0x40C */
        if (list_count > 0 && list_count < 100 && list_array != 0) {
            /* Looks like a Scene with balls in the bad_balls_list */
            if (!IsBadReadPtr((void*)list_array, 4)) {
                DWORD first_ball = *(DWORD*)list_array;
                if (first_ball != 0 && !IsBadReadPtr((void*)first_ball, 0x20)) {
                    return (void*)candidate;
                }
            }
        }
    }

    return NULL;
}

/* Main clone spawning function */
static void spawn_clones(const char *log_path)
{
    FILE *f = fopen(log_path, "w");
    if (!f) f = stdout;

    fprintf(f, "Hamsterball Player Clones Mod\n");
    fprintf(f, "==============================\n");
    fprintf(f, "Config: P1=%d P2=%d P3=%d P4=%d  RaceAI=%d TargetAll=%d\n",
            g_cfg.clones_per_player[0], g_cfg.clones_per_player[1],
            g_cfg.clones_per_player[2], g_cfg.clones_per_player[3],
            g_cfg.enable_race_ai, g_cfg.target_all);
    fprintf(f, "\n");

    /* Find the Scene/Board */
    void *scene = find_scene_from_balls();
    if (!scene) {
        fprintf(f, "ERROR: Could not find Scene/Board pointer.\n");
        fprintf(f, "       Make sure a level or arena is loaded.\n");
        if (f != stdout) fclose(f);
        return;
    }
    fprintf(f, "Scene found at 0x%08X\n", (unsigned)scene);

    /* Get ball lists */
    int bad_count = *(int*)((BYTE*)scene + SCENE_BAD_BALLS + ATHENA_LIST_COUNT);
    int all_count = *(int*)((BYTE*)scene + SCENE_ALL_BALLS + ATHENA_LIST_COUNT);
    fprintf(f, "Balls: bad_balls=%d, all_balls=%d\n", bad_count, all_count);

    int total_clones = 0;
    for (int p = 0; p < 4; p++) {
        int n = g_cfg.clones_per_player[p];
        if (n <= 0) continue;

        /* Find this player's ball */
        void *src_ball = find_player_ball(scene, p);
        if (!src_ball) {
            fprintf(f, "Player %d: no ball found (skipping)\n", p + 1);
            continue;
        }

        float sx = *(float*)((BYTE*)src_ball + BALL_DISPLAY_X);
        float sy = *(float*)((BYTE*)src_ball + BALL_DISPLAY_Y);
        float sz = *(float*)((BYTE*)src_ball + BALL_DISPLAY_Z);
        fprintf(f, "Player %d: source ball at (%.1f, %.1f, %.1f) — creating %d clones\n",
                p + 1, sx, sy, sz, n);

        for (int i = 0; i < n; i++) {
            void *clone = create_clone(scene, src_ball, p);
            if (clone) {
                float cx = *(float*)((BYTE*)clone + BALL_DISPLAY_X);
                float cy = *(float*)((BYTE*)clone + BALL_DISPLAY_Y);
                float cz = *(float*)((BYTE*)clone + BALL_DISPLAY_Z);
                fprintf(f, "  Clone %d: 0x%08X at (%.1f, %.1f, %.1f) [player_idx=%d]\n",
                        i + 1, (unsigned)clone, cx, cy, cz, p);
                total_clones++;
            } else {
                fprintf(f, "  Clone %d: FAILED\n", i + 1);
            }
        }
    }

    fprintf(f, "\nTotal clones created: %d\n", total_clones);
    if (total_clones > 0) {
        g_clones_spawned = 1;
    }

    fprintf(f, "\nDone.\n");
    if (f != stdout) fclose(f);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Binary Patches
 * ═══════════════════════════════════════════════════════════════════════════ */

static void apply_patches(const char *log_path)
{
    FILE *f = fopen(log_path, "a");
    if (!f) f = stdout;

    HMODULE hExe = GetModuleHandleA(NULL);
    if (!hExe) {
        fprintf(f, "ERROR: GetModuleHandle failed\n");
        if (f != stdout) fclose(f);
        return;
    }

    /* PATCH 1: Gate 0 @ RVA 0x83AE — enable AI in races
     *
     * Original: 0F 84 76 04 00 00   (JZ +0x476 → skip AI)
     * Patched:  90 90 90 90 90 90   (NOP → AI always runs)
     *
     * This makes ALL balls run the AI chase logic regardless of arena mode.
     * Without this, only balls in arenas (App+0x237=1) get AI. */
    if (g_cfg.enable_race_ai) {
        BYTE *addr = (BYTE*)hExe + GATE0_RVA;
        static const BYTE expected[6]  = { 0x0F, 0x84, 0x76, 0x04, 0x00, 0x00 };
        static const BYTE patched[6]  = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
        int ok = patch_bytes(addr, expected, patched, 6);
        fprintf(f, "Patch 1 (Gate0 @0x4083AE): %s\n", ok ? "OK" : "SKIPPED (already patched or wrong bytes)");
    }

    /* PATCH 2: Targeting filter @ RVA 0x85CD — target ALL entities
     *
     * Original: 83 7F 18 FF 74 61   (CMP [edi+0x18],-1; JE skip)
     *          → skips balls with player_index == -1 (8-balls)
     * Patched:  39 FE 90 90 74 61   (CMP ESI,EDI; NOP; NOP; JE skip)
     *          → skips only self (ESI=this ball, EDI=candidate ball)
     *
     * This makes clones target ALL other balls: players, clones, and 8-balls.
     * Without this, clones would only target players 1-4 (not other AI balls). */
    if (g_cfg.target_all) {
        BYTE *addr = (BYTE*)hExe + FILTER_RVA;
        static const BYTE expected[6]  = { 0x83, 0x7F, 0x18, 0xFF, 0x74, 0x61 };
        static const BYTE patched[6]  = { 0x39, 0xFE, 0x90, 0x90, 0x74, 0x61 };
        int ok = patch_bytes(addr, expected, patched, 6);
        fprintf(f, "Patch 2 (Filter @0x4085CD): %s\n", ok ? "OK" : "SKIPPED (already patched or wrong bytes)");
    }

    if (f != stdout) fclose(f);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Background Thread
 * ═══════════════════════════════════════════════════════════════════════════ */

static DWORD WINAPI clone_thread(LPVOID lpParam)
{
    (void)lpParam;

    char ini_path[MAX_PATH];
    char log_path[MAX_PATH];

    GetModuleFileNameA(NULL, ini_path, MAX_PATH);
    char *p = strrchr(ini_path, '\\');
    if (p) strcpy(p + 1, "player_clones.ini");
    else strcat(ini_path, "player_clones.ini");

    GetModuleFileNameA(NULL, log_path, MAX_PATH);
    p = strrchr(log_path, '.');
    if (p) strcpy(p, "_clones.log");
    else strcat(log_path, "_clones.log");

    load_config(ini_path);

    /* Check if any clones are requested */
    int total_requested = 0;
    for (int i = 0; i < 4; i++)
        total_requested += g_cfg.clones_per_player[i];
    if (total_requested == 0 && !g_cfg.enable_race_ai && !g_cfg.target_all) {
        return 0;  /* nothing to do */
    }

    resolve_functions();

    /* Apply binary patches immediately (they affect all balls) */
    apply_patches(log_path);

    /* If no clones requested, we're done (patches only) */
    if (total_requested == 0) {
        return 0;
    }

    /* Wait for the game to load a level and spawn balls.
     * Poll every 500ms for up to 60 seconds. */
    for (int attempt = 0; attempt < 120; attempt++) {
        Sleep(500);

        void *scene = find_scene_from_balls();
        if (!scene) continue;

        /* Verify balls exist in the all_balls_list */
        int all_count = *(int*)((BYTE*)scene + SCENE_ALL_BALLS + ATHENA_LIST_COUNT);
        if (all_count < 1) continue;

        /* Found balls! Spawn clones now. */
        spawn_clones(log_path);
        break;
    }

    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DllMain
 * ═══════════════════════════════════════════════════════════════════════════ */

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hinstDLL);

        /* Load real bass_real.dll for audio forwarding */
        g_hRealBass = LoadLibraryA("bass_real.dll");
        if (g_hRealBass == NULL) {
            char path[MAX_PATH];
            if (GetModuleFileNameA(hinstDLL, path, MAX_PATH)) {
                char *p = strrchr(path, '\\');
                if (p) { strcpy(p + 1, "bass_real.dll"); g_hRealBass = LoadLibraryA(path); }
            }
        }

        if (g_hRealBass != NULL) {
            real_BASS_Stop              = (BASS_Stop_t)GetProcAddress(g_hRealBass, "BASS_Stop");
            real_BASS_ChannelSetAttributes = (BASS_ChannelSetAttributes_t)GetProcAddress(g_hRealBass, "BASS_ChannelSetAttributes");
            real_BASS_Free              = (BASS_Free_t)GetProcAddress(g_hRealBass, "BASS_Free");
            real_BASS_Init              = (BASS_Init_t)GetProcAddress(g_hRealBass, "BASS_Init");
            real_BASS_Start             = (BASS_Start_t)GetProcAddress(g_hRealBass, "BASS_Start");
            real_BASS_SetConfig         = (BASS_SetConfig_t)GetProcAddress(g_hRealBass, "BASS_SetConfig");
            real_BASS_ChannelStop       = (BASS_ChannelStop_t)GetProcAddress(g_hRealBass, "BASS_ChannelStop");
            real_BASS_MusicPlayEx       = (BASS_MusicPlayEx_t)GetProcAddress(g_hRealBass, "BASS_MusicPlayEx");
            real_BASS_ErrorGetCode      = (BASS_ErrorGetCode_t)GetProcAddress(g_hRealBass, "BASS_ErrorGetCode");
            real_BASS_MusicLoad         = (BASS_MusicLoad_t)GetProcAddress(g_hRealBass, "BASS_MusicLoad");
        }

        CreateThread(NULL, 0, clone_thread, NULL, 0, NULL);
        break;

    case DLL_PROCESS_DETACH:
        if (g_hRealBass) { FreeLibrary(g_hRealBass); g_hRealBass = NULL; }
        break;
    }
    return TRUE;
}
