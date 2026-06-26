/*
 * 8ball_spawn.c — BASS.dll proxy that spawns 8-balls on key press.
 *
 * Press the B key during gameplay to spawn an 8-ball in front of the
 * player's hamster ball. The 8-ball spawns with physics and falls/collides
 * just like the balls in Rodent Rumble arenas.
 *
 * ═══════════════════════════════════════════════════════════════════════════
 * HOW IT WORKS
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * The 8Ball mesh is preloaded by the game's resource loader (TimerDisplay)
 * into the board mesh array at board+0x268 (index 9 of the array starting at
 * board+0x244, stride 4 bytes per pointer). The ball mesh index field is
 * ball+0x754 — setting it to 9 makes the ball render as the 8-ball.
 *
 * Ball creation follows the same pattern as CreateBadBall (0x40BCA0):
 *   1. operator_new(0xC98) to allocate ball memory
 *   2. Ball_ctor(mem, scene) to construct it (__thiscall: ECX=mem)
 *   3. vtable[1]() to init physics defaults
 *   4. Set position in front of the player's ball
 *   5. Set ball+0x754 = 9 (8Ball mesh index)
 *   6. Set ball+0x18 (player_index) = -1 (no player, AI-less debris ball)
 *   7. Copy player's exact velocity vector (same direction and speed)
 *   8. Radius stays at Ball_InitPhysicsDefaults default (35.0) — NOT player radius
 *   9. AthenaList_Append to scene+0x29D4 (bad_balls_list)
 *   10. AthenaList_Append to scene+0x2DEC (all_balls_list)
 *
 * Key detection: GetAsyncKeyState('B') polled in a background thread at
 * 16ms intervals. Edge-triggered (only spawns on key-down transition).
 *
 * Scene/Board discovery: Scans App struct for a pointer whose vtable matches
 * the Scene/Board vtable (0x4D0260). Same brute-force approach as player_clones.
 * Once the board is found, the player's ball is located by scanning the
 * all_balls_list (board+0x2DEC) for a ball with player_index == 0.
 *
 * ═══════════════════════════════════════════════════════════════════════════
 * BUILD
 * ═══════════════════════════════════════════════════════════════════════════
 *
 *   i686-w64-mingw32-gcc -shared -o bass.dll 8ball_spawn.c \
 *       -lwinmm -Wl,--enable-stdcall-fixup \
 *       -O2 -static -static-libgcc -Wl,--add-stdcall-alias
 *
 * Installation (Windows):
 *   1. In your Hamsterball game folder, rename bass.dll -> bass_real.dll
 *   2. Copy this proxy bass.dll into the game folder
 *   3. Launch the game, enter a level or arena, press B to spawn 8-balls
 *
 * ═══════════════════════════════════════════════════════════════════════════
 * CONFIG
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * Edit 8ball_spawn.ini (optional, placed next to bass.dll):
 *   [8ball]
 *   spawn_key=0x42      ; Virtual key code (default: B = 0x42)
 *   spawn_distance=40   ; Distance in front of player (default: 40)
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winuser.h>
#include <mmsystem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * BASS Proxy Exports (stubs — Hamsterball only needs import resolution)
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Match player_clones pattern: plain void, no __stdcall, no .def file.
 * --enable-stdcall-fixup + --add-stdcall-alias handles name mangling. */
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
/* BASS_SetConfig / BASS_GetConfig intentionally NOT exported.
 * Game calls these via SEH-protected code — Wine's "unimplemented function"
 * exception is caught by the game's KiUserCallbackDispatcher. Exporting them
 * with wrong calling convention crashes the game (stack corruption at 0x424). */
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

/* ═══════════════════════════════════════════════════════════════════════════
 * Game Constants (RVAs, struct offsets)
 * ═══════════════════════════════════════════════════════════════════════════ */

#define IMAGE_BASE          0x00400000
#define OPERATOR_NEW        0x004BA57B
#define BALL_CTOR           0x0040AFE0
#define ATHENA_LIST_APPEND  0x004B9A40
#define APP_PTR_ADDR        0x005341E0

/* Ball struct offsets */
#define BALL_SCENE          0x14    /* void* scene (Board*) */
#define BALL_PLAYER_IDX     0x18    /* int player_index (-1 = none) */
#define BALL_DISPLAY_X      0x164   /* float pos_x (display position) */
#define BALL_DISPLAY_Y      0x168   /* float pos_y */
#define BALL_DISPLAY_Z      0x16C   /* float pos_z */
#define BALL_VEL_X          0x170   /* float velocity_x */
#define BALL_VEL_Y          0x174   /* float velocity_y */
#define BALL_VEL_Z          0x178   /* float velocity_z */
#define BALL_RADIUS         0x284   /* float radius */
#define BALL_MESH_INDEX     0x754   /* int mesh index (0=Sphere, 9=8Ball) */
#define BALL_DEAD           0x2F9   /* byte is_falling/dead */
#define BALL_RENDER_SCALE   0x2FC   /* float render_scale (1.0=visible) */
#define BALL_IS_ACTIVE      0x768   /* byte is_active */
#define BALL_FALL_TIMER     0xC60   /* float fall timer */
#define BALL_HOME           0xC70   /* float HOME (leash distance from spawn) */
#define BALL_CHASE          0xC6C   /* float CHASE (player detection radius) */
#define BALL_SPINDISTANCE   0xC7C   /* float SPINDISTANCE (orbit radius) */
#define BALL_ALLOC_SIZE     0xC98   /* Ball allocation size */

/* Scene/Board struct offsets */
#define SCENE_BAD_BALLS     0x29D4  /* AthenaList (bad_balls_list) */
#define SCENE_ALL_BALLS     0x2DEC  /* AthenaList (all_balls_list) */

/* AthenaList struct offsets */
#define ATHENA_LIST_VTABLE  0x00    /* vtable ptr */
#define ATHENA_LIST_COUNT   0x04    /* int count */
#define ATHENA_LIST_ARRAY    0x08   /* void** items (start of inline array) */
/* Note: AthenaList has items at +0x08 when count <= inline capacity.
 * For larger lists, items array is at a pointer. We check both. */
#define ATHENA_LIST_ITEMS_PTR 0x0C  /* void** items (when count > inline) */
#define ATHENA_LIST_CAP     0x04    /* inline capacity = 1, items start at +0x08 */

/* Vtable addresses */
#define SCENE_VTABLE        0x004D0260
#define BALL_VTABLE_ADDR    0x004CF3A0

/* Mesh indices in board mesh array (board+0x244 + index*4) */
#define MESH_SPHERE         0
#define MESH_SPHERE_BREAK1   1
#define MESH_SPHERE_BREAK2   2
#define MESH_HAMSTER_WAIT    3
#define MESH_HAMSTER_TROT1   4
#define MESH_HAMSTER_TROT2   5
#define MESH_HAMSTER_TROT3   6
#define MESH_RB_GLARE        7
#define MESH_SPHERE_TAR      8
#define MESH_8BALL           9
#define MESH_FUNBALL         10
#define MESH_BELL            11
#define MESH_DIZZY           12

/* Config */
static int g_spawn_key = 0x42;      /* 'B' key */
static float g_spawn_distance = 40.0f;
/* g_spawn_velocity removed — 8-ball now copies player's exact velocity */
static int g_prev_key_down = 0;
static int g_spawned_count = 0;

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
 * Config Loader
 * ═══════════════════════════════════════════════════════════════════════════ */

static void load_config(void)
{
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    char *p = strrchr(path, '\\');
    if (p) *(p + 1) = 0; else path[0] = 0;
    strcat_s(path, MAX_PATH, "8ball_spawn.ini");

    FILE *f = NULL;
    if (fopen_s(&f, path, "r") != 0 || !f) return;

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        if (line[0] == '[' || line[0] == ';' || line[0] == '#') continue;
        char *eq = strchr(line, '=');
        if (!eq) continue;
        *eq = 0;
        char *key = line;
        char *val = eq + 1;
        /* trim */
        while (*key == ' ' || *key == '\t') key++;
        char *ek = key + strlen(key) - 1;
        while (ek > key && (*ek == ' ' || *ek == '\t' || *ek == '\n' || *ek == '\r')) *ek-- = 0;
        while (*val == ' ' || *val == '\t') val++;
        char *ev = val + strlen(val) - 1;
        while (ev > val && (*ev == ' ' || *ev == '\t' || *ev == '\n' || *ev == '\r')) *ev-- = 0;

        if (_stricmp(key, "spawn_key") == 0) {
            g_spawn_key = (int)strtol(val, NULL, 0);
        } else if (_stricmp(key, "spawn_distance") == 0) {
            g_spawn_distance = (float)atof(val);
        }
        /* spawn_velocity removed — 8-ball now copies player's exact velocity */
    }
    fclose(f);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Scene/Board Discovery
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Find the current Scene/Board by scanning the App struct for a pointer
 * whose vtable matches SCENE_VTABLE (0x4D0260).
 * Returns Board* or NULL. */
static void* find_scene(void)
{
    DWORD *pAppPtr = (DWORD*)APP_PTR_ADDR;
    if (IsBadReadPtr(pAppPtr, 4)) return NULL;
    DWORD app = *pAppPtr;
    if (!app || IsBadReadPtr((void*)app, 0x300)) return NULL;

    /* Scan App struct for a pointer to the current Board/Scene */
    for (int offset = 0x100; offset < 0xA00; offset += 4) {
        DWORD candidate = *(DWORD*)((BYTE*)app + offset);
        if (candidate == 0 || candidate < 0x10000) continue;
        if (IsBadReadPtr((void*)candidate, 0x3000)) continue;
        DWORD vt = *(DWORD*)candidate;
        if (vt == SCENE_VTABLE) {
            /* Verify: check that bad_balls_list has a reasonable count */
            DWORD list_count = *(DWORD*)((BYTE*)candidate + SCENE_BAD_BALLS + ATHENA_LIST_COUNT);
            if (list_count < 100) {
                return (void*)candidate;
            }
        }
    }
    return NULL;
}

/* Find the player's ball (player_index == 0) in the all_balls_list.
 * Returns Ball* or NULL. */
static void* find_player_ball(void* scene)
{
    BYTE *s = (BYTE*)scene;

    /* AthenaList at scene+SCENE_ALL_BALLS:
     *   +0x00 = vtable
     *   +0x04 = count
     *   +0x08 = items[0] (inline array, capacity=1)
     *   When count > 1, items are stored at the pointer at +0x08
     *   Actually: AthenaList stores items inline from +0x08 when count==1,
     *   and uses a heap-allocated array at *(+0x08) when count > 1.
     *   Let's handle both cases. */

    int count = *(int*)(s + SCENE_ALL_BALLS + ATHENA_LIST_COUNT);
    if (count <= 0 || count > 100) return NULL;

    /* Case 1: inline array (count == 1, items start at +0x08) */
    if (count == 1) {
        void *ball = *(void**)(s + SCENE_ALL_BALLS + ATHENA_LIST_ARRAY);
        if (ball && !IsBadReadPtr(ball, 0x20)) {
            int idx = *(int*)((BYTE*)ball + BALL_PLAYER_IDX);
            if (idx == 0) return ball;
        }
        return NULL;
    }

    /* Case 2: heap array (count > 1, pointer at +0x08) */
    void **items = *(void**)(s + SCENE_ALL_BALLS + ATHENA_LIST_ARRAY);
    if (!items || IsBadReadPtr(items, count * 4)) return NULL;

    for (int i = 0; i < count; i++) {
        void *ball = items[i];
        if (!ball || IsBadReadPtr(ball, 0x20)) continue;
        int idx = *(int*)((BYTE*)ball + BALL_PLAYER_IDX);
        if (idx == 0) return ball;
    }
    return NULL;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * 8-Ball Spawning
 * ═══════════════════════════════════════════════════════════════════════════ */

static void spawn_8ball(void* scene, void* player_ball)
{
    BYTE *src = (BYTE*)player_ball;
    BYTE *s = (BYTE*)scene;

    /* 1. Allocate memory for the new ball */
    void *mem = pfn_operator_new(BALL_ALLOC_SIZE);
    if (!mem) return;

    /* 2. Construct the ball: Ball_ctor(mem, scene) — __thiscall: ECX=mem */
    void *ball = pfn_ball_ctor(mem, scene);
    if (!ball) return;

    BYTE *b = (BYTE*)ball;

    /* 3. Call vtable[1] = Ball_InitPhysicsDefaults */
    void **vtable = *(void***)b;
    void (__thiscall *init_fn)(void*) = (void (__thiscall *)(void*))vtable[1];
    init_fn(ball);

    /* 4. Read player ball position and velocity */
    float px = *(float*)(src + BALL_DISPLAY_X);
    float py = *(float*)(src + BALL_DISPLAY_Y);
    float pz = *(float*)(src + BALL_DISPLAY_Z);

    float vx = *(float*)(src + BALL_VEL_X);
    float vy = *(float*)(src + BALL_VEL_Y);
    float vz = *(float*)(src + BALL_VEL_Z);

    /* Normalize velocity direction for "forward" spawn position */
    float vlen = vx*vx + vy*vy + vz*vz;
    float dirx = 0.0f, diry = 0.0f, dirz = 1.0f;
    if (vlen > 0.001f) {
        vlen = 1.0f / sqrtf(vlen);
        dirx = vx * vlen;
        diry = vy * vlen;
        dirz = vz * vlen;
    }

    /* 5. Position the 8-ball in front of the player */
    float spawn_x = px + dirx * g_spawn_distance;
    float spawn_y = py + 20.0f;  /* Slightly above to avoid spawning inside ground */
    float spawn_z = pz + dirz * g_spawn_distance;

    *(float*)(b + BALL_DISPLAY_X) = spawn_x;
    *(float*)(b + BALL_DISPLAY_Y) = spawn_y;
    *(float*)(b + BALL_DISPLAY_Z) = spawn_z;

    /* 6. Copy the player's exact velocity (same direction and speed) */
    *(float*)(b + BALL_VEL_X) = vx;
    *(float*)(b + BALL_VEL_Y) = vy;
    *(float*)(b + BALL_VEL_Z) = vz;

    /* 7. Set mesh index to 9 (8Ball) */
    *(int*)(b + BALL_MESH_INDEX) = MESH_8BALL;

    /* 8. Set player_index to -1 (no player controls this ball) */
    *(int*)(b + BALL_PLAYER_IDX) = -1;

    /* 9. Ensure the ball is alive and active */
    *(BYTE*)(b + BALL_DEAD) = 0;
    *(BYTE*)(b + BALL_IS_ACTIVE) = 1;
    *(float*)(b + BALL_RENDER_SCALE) = 1.0f;
    *(float*)(b + BALL_FALL_TIMER) = 1.0f;

    /* 10. Radius: keep Ball_InitPhysicsDefaults default (35.0 = 0x420C0000).
     * Do NOT overwrite with player radius — bad balls are larger than player. */

    /* 11. AI tuning: HOME and CHASE set to 100000 (effectively infinite leash
     * and detection range — always chase, never retreat to home).
     * SPINDISTANCE set to 1 (tight orbit when near home, but chase always
     * wins since HOME is 100000). */
    *(float*)(b + BALL_CHASE) = 100000.0f;
    *(float*)(b + BALL_HOME) = 100000.0f;
    *(float*)(b + BALL_SPINDISTANCE) = 1.0f;

    /* 12. Add to scene ball lists */
    pfn_athena_list_append(s + SCENE_BAD_BALLS, (int)ball);
    pfn_athena_list_append(s + SCENE_ALL_BALLS, (int)ball);

    g_spawned_count++;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Background Thread
 * ═══════════════════════════════════════════════════════════════════════════ */

static DWORD WINAPI spawn_thread(LPVOID param)
{
    (void)param;

    /* Wait for game to load */
    Sleep(3000);

    resolve_functions();
    load_config();

    while (1) {
        Sleep(16);

        /* Check key press (edge-triggered) */
        SHORT key_state = GetAsyncKeyState(g_spawn_key);
        int key_down = (key_state & 0x8000) != 0;

        if (key_down && !g_prev_key_down) {
            /* Key just pressed — spawn an 8-ball */

            /* Find the current scene/board */
            void *scene = find_scene();
            if (!scene) continue;

            /* Find the player's ball */
            void *player_ball = find_player_ball(scene);
            if (!player_ball) continue;

            /* Safety check */
            if (IsBadReadPtr(player_ball, 0x20)) continue;

            spawn_8ball(scene, player_ball);
        }

        g_prev_key_down = key_down;
    }

    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DllMain
 * ═══════════════════════════════════════════════════════════════════════════ */

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
    (void)hModule;
    (void)lpReserved;

    if (dwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CreateThread(NULL, 0, spawn_thread, NULL, 0, NULL);
    }

    return TRUE;
}
