/*
 * lua_support — Lua Scripting for Hamsterball Scene Objects
 *
 * Embeds Lua 5.1.5 into bass.dll proxy.  On game launch:
 *   1. Creates a "Lua" folder in the game directory if it doesn't exist.
 *   2. Hooks Scene_Update (0x419C00) to run a per-frame Lua tick.
 *   3. Scans the SpatialTree (scene+0x480) for objects whose name
 *      starts with "L:" prefix.
 *   4. For each "L:xxx" object, loads Lua/xxx.lua (once) and calls
 *      the update(dt) function every frame.
 *   5. Exposes a Hamsterball API to Lua: get_position, set_position,
 *      get_velocity, set_velocity, get_ball, etc.
 *
 * If Lua/xxx.lua doesn't exist, the object stays static (no behaviour).
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll lua_support.c \
 *     lua/src/*.c -I lua/src -lwinmm -Wl,--enable-stdcall-fixup \
 *     -O2 -static -static-libgcc -Wl,--add-stdcall-alias \
 *     -lm -DNO_LUA_STANDALONE
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

/* ── Lua includes ───────────────────────────────────────────────────── */
#include "lua/src/lua.h"
#include "lua/src/lauxlib.h"
#include "lua/src/lualib.h"

/* ── Game constants (from Ghidra RE) ────────────────────────────────── */
#define GAME_BASE          0x00400000
#define APP_ADDR           0x005341E0   /* g_App global */
#define SCENE_UPDATE_ADDR  0x00419C00   /* Scene_Update */
#define SCENE_OFFSET       0x480        /* scene mgr at board+0x480? No... */
#define SPATIALTREE_OFFSET 0x480        /* scene->spatialtree at scene+0x480 */

/* Board struct offsets */
#define BOARD_APP_PTR      0x878        /* board+0x878 = App* */

/* SceneObject offsets (verified from SceneObject_BaseInit + Level_FindObjectByName decomp)
 * SceneObject has Vec3 structs at +0x450 (position) and +0x464 (rotation):
 *   Vec3 layout: +0x00 vtable, +0x04 x, +0x08 y, +0x0C z, +0x10 w(scale, default 1.0)
 * So position floats are at base+0x454/0x458/0x45C, scale at +0x460
 * Rotation floats at base+0x468/0x46C/0x470, scale2 at +0x474
 */
#define OBJ_NAME           0x50         /* char* name at obj+0x50 */
#define OBJ_POS_X          0x454        /* float pos.x (Vec3+0x04) */
#define OBJ_POS_Y          0x458        /* float pos.y (Vec3+0x08) */
#define OBJ_POS_Z          0x45C        /* float pos.z (Vec3+0x0C) */
#define OBJ_SCALE          0x460        /* float scale (Vec3+0x10, default 1.0) */
#define OBJ_ROT_X          0x468        /* float rot.x (Vec3+0x04) */
#define OBJ_ROT_Y          0x46C        /* float rot.y (Vec3+0x08) */
#define OBJ_ROT_Z          0x470        /* float rot.z (Vec3+0x0C) */
#define OBJ_SCALE2         0x474        /* float scale2 (Vec3+0x10, default 1.0) */

/* SpatialTree iteration offsets (from Scene_UpdateChildren decomp) */
/* param_1+0x08 = count, param_1+0x04 = thread index fn, param_1+0x0C+idx*4 = iter index */
/* param_1+0x410 = data array ptr, children at child+0x414 */
#define ST_CHILD_COUNT     0x08
#define ST_CHILD_DATA      0x410
#define ST_CHILD_ITER      0x0C         /* iter index base */
#define ST_GRANDCHILD_COUNT 0x0C
#define ST_GRANDCHILD_DATA  0x414
#define ST_GRANDCHILD_ITER  0x10

/* Ball struct offsets */
#define BALL_POS_X         0x164
#define BALL_POS_Y         0x168
#define BALL_POS_Z         0x16C
#define BALL_VEL_X         0x1A4        /* via phys ptr at ball+0x1A4? Actually... */
#define BALL_VEL_X_DIRECT  0x170
#define BALL_VEL_Y_DIRECT  0x174
#define BALL_VEL_Z_DIRECT  0x178
#define BALL_RADIUS        0x284
#define BALL_PLAYER_IDX    0x18         /* int, 0=P1, -1=AI */
#define BALL_VTABLE_ADDR   0x4CF3A0
#define BALL_IS_FALLING    0x2E8        /* byte: set to 1 to kill/respawn the ball */

/* App struct offsets */
#define APP_SCENE_MGR      0x184        /* App+0x184 = scene manager (MeshWorld) */
#define APP_PROFILE        0x220        /* App+0x220 = PlayerProfile* */
#define APP_BALL_LIST      0x29D4       /* scene+0x29D4 = AthenaList of balls */
#define APP_SCORE_BASE     0x5E4        /* App+0x5E4 + player_idx*0xA0 = score (float) */

/* AthenaList offsets */
#define AL_COUNT           0x08         /* count at list+0x08 (via GetSize) */
#define AL_DATA            0x04         /* internal data — actually complex */

/* Scene struct offsets */
#define SCENE_APP          0x21E        /* scene+0x21E*4 = scene[0x21E] = App* (int* indexing) */
#define SCENE_SPATIALTREE  0x480        /* scene+0x480 = SpatialTree* (from Level_FindObjectByName) */
/* Actually: this->+0x8ac = level/mesh object, this->+0x480 = spatial root */
/* Level_FindObjectByName: *(int*)(this + 0x480) = spatial tree root */
/* Objects list at root+0xcb0 count, root+0x10b8 = data ptr */
#define ST_OBJ_COUNT       0xCB0
#define ST_OBJ_DATA        0x10B8
#define ST_OBJ_ITER        0xCB4

/* How many Lua entities we can track simultaneously */
#define MAX_LUA_ENTITIES  64

/* ── BASS proxy exports ─────────────────────────────────────────────── */
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

typedef unsigned int __int64 QWORD;
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

/* Extra stubs */
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

static void lazy_load_real_bass(void);
static void load_real_bass(void)
{
    if (g_hRealBass) return;
    char path[MAX_PATH];
    /* Try game directory first (bass_real.dll — user renames original) */
    GetModuleFileNameA(NULL, path, MAX_PATH);
    char *slash = strrchr(path, '\\');
    if (slash) {
        strcpy(slash + 1, "bass_real.dll");
        g_hRealBass = LoadLibraryA(path);
    }
    /* Fallback: system directory */
    if (!g_hRealBass) {
        GetSystemDirectoryA(path, MAX_PATH);
        lstrcatA(path, "\\bass.dll");
        g_hRealBass = LoadLibraryA(path);
    }
    if (!g_hRealBass) return;
    real_BASS_Init = (BASS_Init_t)GetProcAddress(g_hRealBass, "BASS_Init");
    real_BASS_Free = (BASS_Free_t)GetProcAddress(g_hRealBass, "BASS_Free");
    real_BASS_Start = (BASS_Start_t)GetProcAddress(g_hRealBass, "BASS_Start");
    real_BASS_Stop = (BASS_Stop_t)GetProcAddress(g_hRealBass, "BASS_Stop");
    real_BASS_ErrorGetCode = (BASS_ErrorGetCode_t)GetProcAddress(g_hRealBass, "BASS_ErrorGetCode");
    real_BASS_MusicLoad = (BASS_MusicLoad_t)GetProcAddress(g_hRealBass, "BASS_MusicLoad");
    real_BASS_ChannelStop = (BASS_ChannelStop_t)GetProcAddress(g_hRealBass, "BASS_ChannelStop");
    real_BASS_MusicPlayEx = (BASS_MusicPlayEx_t)GetProcAddress(g_hRealBass, "BASS_MusicPlayEx");
    real_BASS_SetConfig = (BASS_SetConfig_t)GetProcAddress(g_hRealBass, "BASS_SetConfig");
    real_BASS_ChannelSetAttributes = (BASS_ChannelSetAttributes_t)GetProcAddress(g_hRealBass, "BASS_ChannelSetAttributes");
}

static void lazy_load_real_bass(void) { load_real_bass(); }

/* ── Utility: get game directory ────────────────────────────────────── */
static void get_game_dir(char *out, DWORD len)
{
    GetModuleFileNameA(NULL, out, len);
    char *slash = strrchr(out, '\\');
    if (slash) slash[1] = '\0';
    else out[0] = '\0';
}

/* ── Utility: safe memory read ──────────────────────────────────────── */
static int safe_read_ptr(DWORD addr)
{
    if (addr < 0x10000) return 0;
    if (IsBadReadPtr((void*)addr, 4)) return 0;
    return *(int*)addr;
}

static float safe_read_float(DWORD addr)
{
    if (addr < 0x10000) return 0.0f;
    if (IsBadReadPtr((void*)addr, 4)) return 0.0f;
    return *(float*)addr;
}

static void safe_write_float(DWORD addr, float val)
{
    if (addr < 0x10000) return;
    if (IsBadWritePtr((void*)addr, 4)) return;
    *(float*)addr = val;
}

static void safe_write_dword(DWORD addr, DWORD val)
{
    if (addr < 0x10000) return;
    if (IsBadWritePtr((void*)addr, 4)) return;
    *(DWORD*)addr = val;
}

/* ── Lua entity tracking ─────────────────────────────────────────────── */
typedef struct {
    DWORD obj_addr;       /* address of the SceneObject in game memory */
    char  name[64];       /* entity name without "L:" prefix */
    lua_State *L;         /* Lua state for this entity (NULL if no script) */
    int   loaded;         /* 0=not tried, 1=loaded OK, 2=no script */
    float last_pos[3];    /* last known position (for velocity calc) */
    float prev_time;      /* for dt calculation */
} LuaEntity;

static LuaEntity g_entities[MAX_LUA_ENTITIES];
static int g_entity_count = 0;
static lua_State *g_shared_L = NULL;  /* shared Lua state */
static char g_lua_dir[MAX_PATH] = {0};
static DWORD g_last_tick = 0;
static float g_dt = 0.016f;  /* default 60fps */

/* ── Lua API: hamsterball.get_position(entity_id) -> x,y,z ──────────── */
static int lua_get_position(lua_State *L)
{
    int idx = luaL_checkint(L, 1) - 1;  /* 1-indexed */
    if (idx < 0 || idx >= g_entity_count || !g_entities[idx].obj_addr) {
        lua_pushnumber(L, 0);
        lua_pushnumber(L, 0);
        lua_pushnumber(L, 0);
        return 3;
    }
    DWORD obj = g_entities[idx].obj_addr;
    float x = safe_read_float(obj + OBJ_POS_X);
    float y = safe_read_float(obj + OBJ_POS_Y);
    float z = safe_read_float(obj + OBJ_POS_Z);
    lua_pushnumber(L, x);
    lua_pushnumber(L, y);
    lua_pushnumber(L, z);
    return 3;
}

/* ── Lua API: hamsterball.set_position(entity_id, x, y, z) ─────────── */
static int lua_set_position(lua_State *L)
{
    int idx = luaL_checkint(L, 1) - 1;
    if (idx < 0 || idx >= g_entity_count || !g_entities[idx].obj_addr)
        return 0;
    float x = (float)luaL_checknumber(L, 2);
    float y = (float)luaL_checknumber(L, 3);
    float z = (float)luaL_checknumber(L, 4);
    DWORD obj = g_entities[idx].obj_addr;
    safe_write_float(obj + OBJ_POS_X, x);
    safe_write_float(obj + OBJ_POS_Y, y);
    safe_write_float(obj + OBJ_POS_Z, z);
    return 0;
}

/* ── Lua API: hamsterball.get_rotation(entity_id) -> x,y,z ──────────── */
static int lua_get_rotation(lua_State *L)
{
    int idx = luaL_checkint(L, 1) - 1;
    if (idx < 0 || idx >= g_entity_count || !g_entities[idx].obj_addr) {
        lua_pushnumber(L, 0);
        lua_pushnumber(L, 0);
        lua_pushnumber(L, 0);
        return 3;
    }
    DWORD obj = g_entities[idx].obj_addr;
    lua_pushnumber(L, safe_read_float(obj + OBJ_ROT_X));
    lua_pushnumber(L, safe_read_float(obj + OBJ_ROT_Y));
    lua_pushnumber(L, safe_read_float(obj + OBJ_ROT_Z));
    return 3;
}

/* ── Lua API: hamsterball.set_rotation(entity_id, x, y, z) ──────────── */
static int lua_set_rotation(lua_State *L)
{
    int idx = luaL_checkint(L, 1) - 1;
    if (idx < 0 || idx >= g_entity_count || !g_entities[idx].obj_addr)
        return 0;
    float x = (float)luaL_checknumber(L, 2);
    float y = (float)luaL_checknumber(L, 3);
    float z = (float)luaL_checknumber(L, 4);
    DWORD obj = g_entities[idx].obj_addr;
    safe_write_float(obj + OBJ_ROT_X, x);
    safe_write_float(obj + OBJ_ROT_Y, y);
    safe_write_float(obj + OBJ_ROT_Z, z);
    return 0;
}

/* ── Lua API: hamsterball.get_ball_pos(ball_index) -> x,y,z ─────────── */
static int lua_get_ball_pos(lua_State *L)
{
    int ball_idx = luaL_optint(L, 1, 0);  /* default player 1 */
    
    /* Get App, find ball list */
    int app = safe_read_ptr(APP_ADDR);
    if (!app) { lua_pushnumber(L, 0); lua_pushnumber(L, 0); lua_pushnumber(L, 0); return 3; }
    
    /* App+0x220 = profile, profile+0xC = board */
    int profile = safe_read_ptr(app + 0x220);
    if (!profile) { lua_pushnumber(L, 0); lua_pushnumber(L, 0); lua_pushnumber(L, 0); return 3; }
    
    int board = safe_read_ptr(profile + 0x0C);
    if (!board) { lua_pushnumber(L, 0); lua_pushnumber(L, 0); lua_pushnumber(L, 0); return 3; }
    
    /* board+0x878 = App (scene's app ptr), board+0x29D4 = ball list */
    /* Actually: scene is at board+0x878? No — board IS the scene. */
    /* Scene_Update receives board as param_1. board+0x21E*4 = App* */
    /* Wait — param_1 is int*, so param_1[0x21E] = *(int*)(board + 0x21E*4) */
    /* 0x21E * 4 = 0x878 — so board+0x878 = App*. That matches! */
    /* Ball list: scene+0x29D4*4? No, scene+0x29D4 is at byte offset 0x29D4 */
    /* Actually AthenaList is at board+0x29D4 (byte offset). */
    /* AthenaList_GetSize takes list ptr, reads list+0x08 as count. */
    /* But the list struct is complex. Let's use the simpler approach: */
    /* Get the ball directly from App+0x5DC (physics_ball) — but that's often NULL. */
    /* Better: App+0x220 -> profile -> board, then board+0x29D4 is AthenaList. */
    /* AthenaList_GetSize at 0x4536A0, AthenaList_GetAt at 0x40A020. */
    
    /* Use function pointers to call game's own AthenaList functions */
    typedef int (__fastcall *AthenaList_GetSize_t)(int);
    typedef int (__fastcall *AthenaList_GetAt_t)(int, int);
    static AthenaList_GetSize_t pGetSize = NULL;
    static AthenaList_GetAt_t pGetAt = NULL;
    if (!pGetSize) pGetSize = (AthenaList_GetSize_t)0x004536A0;
    if (!pGetAt) pGetAt = (AthenaList_GetAt_t)0x0040A020;
    
    int list_addr = board + 0x29D4;  /* AthenaList for balls */
    int count = pGetSize(list_addr);
    if (ball_idx < 0 || ball_idx >= count) {
        lua_pushnumber(L, 0); lua_pushnumber(L, 0); lua_pushnumber(L, 0);
        return 3;
    }
    int ball = pGetAt(list_addr, ball_idx);
    if (!ball) { lua_pushnumber(L, 0); lua_pushnumber(L, 0); lua_pushnumber(L, 0); return 3; }
    
    lua_pushnumber(L, safe_read_float(ball + BALL_POS_X));
    lua_pushnumber(L, safe_read_float(ball + BALL_POS_Y));
    lua_pushnumber(L, safe_read_float(ball + BALL_POS_Z));
    return 3;
}

/* ── Lua API: hamsterball.get_delta_time() -> seconds ───────────────── */
static int lua_get_delta_time(lua_State *L)
{
    lua_pushnumber(L, g_dt);
    return 1;
}

/* ── Lua API: hamsterball.get_entity_count() -> int ──────────────────── */
static int lua_get_entity_count(lua_State *L)
{
    lua_pushinteger(L, g_entity_count);
    return 1;
}

/* ── Lua API: hamsterball.get_entity_name(id) -> string ─────────────── */
static int lua_get_entity_name(lua_State *L)
{
    int idx = luaL_checkint(L, 1) - 1;
    if (idx < 0 || idx >= g_entity_count) {
        lua_pushstring(L, "");
        return 1;
    }
    lua_pushstring(L, g_entities[idx].name);
    return 1;
}

/* ── Lua API: hamsterball.find_entity(name) -> id or 0 ──────────────── */
static int lua_find_entity(lua_State *L)
{
    const char *name = luaL_checkstring(L, 1);
    for (int i = 0; i < g_entity_count; i++) {
        if (_stricmp(g_entities[i].name, name) == 0) {
            lua_pushinteger(L, i + 1);
            return 1;
        }
    }
    lua_pushinteger(L, 0);
    return 1;
}

/* ── Lua API: hamsterball.log(msg) — write to Lua/log.txt ───────────── */
static int lua_log(lua_State *L)
{
    const char *msg = luaL_optstring(L, 1, "");
    char path[MAX_PATH];
    _snprintf(path, MAX_PATH, "%sLua\\log.txt", g_lua_dir);
    FILE *f = NULL;
    if (fopen_s(&f, path, "a") != 0 || !f) return 0;
    fprintf(f, "%s\n", msg);
    fclose(f);
    return 0;
}

/* ── Lua API: hamsterball.get_scale(entity_id) -> sx, sy, sz ────────── */
static int lua_get_scale(lua_State *L)
{
    int idx = luaL_checkint(L, 1) - 1;
    if (idx < 0 || idx >= g_entity_count || !g_entities[idx].obj_addr) {
        lua_pushnumber(L, 1.0);
        lua_pushnumber(L, 1.0);
        lua_pushnumber(L, 1.0);
        return 3;
    }
    DWORD obj = g_entities[idx].obj_addr;
    /* Scale is stored as w-component in position Vec3 (+0x460) and rotation Vec3 (+0x474) */
    /* We return both as x and y, with z=1 for uniformity */
    float sx = safe_read_float(obj + OBJ_SCALE);
    float sy = safe_read_float(obj + OBJ_SCALE2);
    lua_pushnumber(L, sx);
    lua_pushnumber(L, sy);
    lua_pushnumber(L, 1.0);
    return 3;
}

/* ── Lua API: hamsterball.set_scale(entity_id, sx, sy, sz) ─────────── */
static int lua_set_scale(lua_State *L)
{
    int idx = luaL_checkint(L, 1) - 1;
    if (idx < 0 || idx >= g_entity_count || !g_entities[idx].obj_addr)
        return 0;
    float sx = (float)luaL_optnumber(L, 2, 1.0);
    float sy = (float)luaL_optnumber(L, 3, 1.0);
    /* sz ignored — only 2 scale fields available */
    DWORD obj = g_entities[idx].obj_addr;
    safe_write_float(obj + OBJ_SCALE, sx);
    safe_write_float(obj + OBJ_SCALE2, sy);
    return 0;
}

/* ── Lua API: hamsterball.kill_ball(ball_index) — kill/respawn ball ──── */
static int lua_kill_ball(lua_State *L)
{
    int ball_idx = luaL_optint(L, 1, 0);  /* default player 1 */

    int app = safe_read_ptr(APP_ADDR);
    if (!app) return 0;
    int profile = safe_read_ptr(app + 0x220);
    if (!profile) return 0;
    int board = safe_read_ptr(profile + 0x0C);
    if (!board) return 0;

    /* Use game's AthenaList functions to find the ball */
    typedef int (__fastcall *AthenaList_GetSize_t)(int);
    typedef int (__fastcall *AthenaList_GetAt_t)(int, int);
    static AthenaList_GetSize_t pGetSize = NULL;
    static AthenaList_GetAt_t pGetAt = NULL;
    if (!pGetSize) pGetSize = (AthenaList_GetSize_t)0x004536A0;
    if (!pGetAt) pGetAt = (AthenaList_GetAt_t)0x0040A020;

    int list_addr = board + 0x29D4;
    int count = pGetSize(list_addr);
    if (ball_idx < 0 || ball_idx >= count) return 0;

    int ball = pGetAt(list_addr, ball_idx);
    if (!ball) return 0;

    /* Set ball+0x2E8 = 1 (is_falling flag → triggers kill/respawn) */
    if (!IsBadWritePtr((void*)(ball + BALL_IS_FALLING), 1)) {
        *(BYTE*)(ball + BALL_IS_FALLING) = 1;
    }
    return 0;
}

/* ── Lua API: hamsterball.distance_to_ball(entity_id, ball_index) ──── */
static int lua_distance_to_ball(lua_State *L)
{
    int idx = luaL_checkint(L, 1) - 1;
    int ball_idx = luaL_optint(L, 2, 0);

    if (idx < 0 || idx >= g_entity_count || !g_entities[idx].obj_addr) {
        lua_pushnumber(L, -1.0);
        return 1;
    }

    /* Get entity position */
    DWORD obj = g_entities[idx].obj_addr;
    float ex = safe_read_float(obj + OBJ_POS_X);
    float ey = safe_read_float(obj + OBJ_POS_Y);
    float ez = safe_read_float(obj + OBJ_POS_Z);

    /* Get ball position */
    int app = safe_read_ptr(APP_ADDR);
    if (!app) { lua_pushnumber(L, -1.0); return 1; }
    int profile = safe_read_ptr(app + 0x220);
    if (!profile) { lua_pushnumber(L, -1.0); return 1; }
    int board = safe_read_ptr(profile + 0x0C);
    if (!board) { lua_pushnumber(L, -1.0); return 1; }

    typedef int (__fastcall *AthenaList_GetSize_t)(int);
    typedef int (__fastcall *AthenaList_GetAt_t)(int, int);
    static AthenaList_GetSize_t pGetSize = NULL;
    static AthenaList_GetAt_t pGetAt = NULL;
    if (!pGetSize) pGetSize = (AthenaList_GetSize_t)0x004536A0;
    if (!pGetAt) pGetAt = (AthenaList_GetAt_t)0x0040A020;

    int list_addr = board + 0x29D4;
    int count = pGetSize(list_addr);
    if (ball_idx < 0 || ball_idx >= count) { lua_pushnumber(L, -1.0); return 1; }

    int ball = pGetAt(list_addr, ball_idx);
    if (!ball) { lua_pushnumber(L, -1.0); return 1; }

    float bx = safe_read_float(ball + BALL_POS_X);
    float by = safe_read_float(ball + BALL_POS_Y);
    float bz = safe_read_float(ball + BALL_POS_Z);

    float dx = bx - ex, dy = by - ey, dz = bz - ez;
    float dist = (float)sqrt(dx*dx + dy*dy + dz*dz);
    lua_pushnumber(L, dist);
    return 1;
}

/* ── Lua API: hamsterball.add_score(points, ball_index) — add to score ─ */
static int lua_add_score(lua_State *L)
{
    float points = (float)luaL_checknumber(L, 1);
    int ball_idx = luaL_optint(L, 2, 0);

    int app = safe_read_ptr(APP_ADDR);
    if (!app) return 0;

    /* Get player index from ball */
    typedef int (__fastcall *AthenaList_GetSize_t)(int);
    typedef int (__fastcall *AthenaList_GetAt_t)(int, int);
    static AthenaList_GetSize_t pGetSize = NULL;
    static AthenaList_GetAt_t pGetAt = NULL;
    if (!pGetSize) pGetSize = (AthenaList_GetSize_t)0x004536A0;
    if (!pGetAt) pGetAt = (AthenaList_GetAt_t)0x0040A020;

    int profile = safe_read_ptr(app + APP_PROFILE);
    if (!profile) return 0;
    int board = safe_read_ptr(profile + 0x0C);
    if (!board) return 0;

    int list_addr = board + APP_BALL_LIST;
    int count = pGetSize(list_addr);
    if (ball_idx < 0 || ball_idx >= count) return 0;

    int ball = pGetAt(list_addr, ball_idx);
    if (!ball) return 0;

    /* ball+0x18 = player index (0=P1, 1=P2, -1=AI) */
    int player_idx = safe_read_ptr(ball + BALL_PLAYER_IDX);
    if (player_idx < 0 || player_idx > 3) return 0;

    /* App+0x5E4 + player_idx*0xA0 = score (float) */
    DWORD score_addr = app + APP_SCORE_BASE + player_idx * 0xA0;
    float current = safe_read_float(score_addr);
    safe_write_float(score_addr, current + points);

    return 0;
}

/* ── Lua API: hamsterball.get_frame_count() -> int ───────────────────── */
static int lua_get_frame_count(lua_State *L)
{
    int app = safe_read_ptr(APP_ADDR);
    if (!app) { lua_pushinteger(L, 0); return 1; }
    /* Scene_Update increments param_1[0xD88] = scene+0xD88*4 = scene+0x3620 */
    int profile = safe_read_ptr(app + 0x220);
    if (!profile) { lua_pushinteger(L, 0); return 1; }
    int board = safe_read_ptr(profile + 0x0C);
    if (!board) { lua_pushinteger(L, 0); return 1; }
    /* param_1[0xD88] = *(int*)(board + 0xD88 * 4) = *(int*)(board + 0x3620) */
    lua_pushinteger(L, safe_read_ptr(board + 0x3620));
    return 1;
}

/* ════════════════════════════════════════════════════════════════════ */
/*  SOUND EFFECTS API                                                   */
/* ════════════════════════════════════════════════════════════════════ */

/* Sound name → Board offset lookup table (from TimerDisplay 0x4298C0) */
typedef struct {
    const char *name;
    int offset;
} SoundEntry;

static const SoundEntry sound_table[] = {
    {"collide",        0x43C},
    {"roll",           0x440},
    {"whistle",        0x444},
    {"bumper",         0x448},
    {"ballbreak",      0x44C},
    {"ballbreaksmall", 0x450},
    {"thwomp",         0x454},
    {"snap",           0x458},
    {"popup",          0x45C},
    {"dropin",         0x460},
    {"dropinshort",    0x464},
    {"popout",         0x468},
    {"pipebump1",      0x46C},
    {"pipebump2",      0x470},
    {"pipebump3",      0x474},
    {"gearclank",      0x478},
    {"bridgeslam",     0x47C},
    {"platformtick",   0x480},
    {"gluestuck",      0x484},
    {"bubble1",        0x488},
    {"bubble2",        0x48C},
    {"wheelcreak",     0x490},
    {"catapult",       0x494},
    {"trapdoor",       0x498},
    {"fwing",          0x49C},
    {"clink",          0x4A0},
    {"whoosh",         0x4A4},
    {"chomp",          0x4A8},
    {"fan-start",      0x4AC},
    {"fan-blow",       0x4B0},
    {"crack",          0x4B4},
    {"crumble",        0x4B8},
    {"sawstartup",     0x4BC},
    {"sawcut",         0x4C0},
    {"minipop",        0x4C4},
    {"bell",           0x4C8},
    {"zip",            0x4CC},
    {"ting",           0x4D0},
    {"shrink",         0x4D4},
    {"grow",           0x4D8},
    {"tweet",          0x4DC},
    {"creakyplatform", 0x4E0},
    {"wubba",          0x4E4},
    {"saw",            0x4E8},
    {"sawspeedy",      0x4EC},
    {"dawgstep1",      0x4F0},
    {"dawgstep2",      0x4F4},
    {"dawgsmash",      0x4F8},
    {"sizzle",         0x4FC},
    {"explode",        0x500},
    {"vac-o-sux",      0x504},
    {"speedcylinder",  0x508},
    {"bonuspop",       0x50C},
    {"buzzbonus",      0x510},
    {"breakbridge",    0x514},
    {"unlock",         0x518},
    {"NeonRide",       0x51C},
    {"NeonFlicker",    0x520},
    {"ZoopDown",       0x524},
    {"LightsOff",      0x528},
    {"GlassBonus",     0x52C},
    {NULL, 0}
};

/* DSBPLAY_LOOPING flag */
#define DSBPLAY_LOOPING 0x00000001

/* Game function pointers */
typedef void (__fastcall *Sound_PlayChannel_t)(int channel);
typedef void (__fastcall *Sound_Play3D_t)(int channel, int dummy, float x, float y, float z);
static Sound_PlayChannel_t pPlayChannel = (Sound_PlayChannel_t)0x004597B0;
static Sound_Play3D_t    pPlay3D      = (Sound_Play3D_t)0x00459860;

/* DSound COM vtable method types (__fastcall with dummy EDX = __thiscall) */
typedef int  (__fastcall *dsb_getlong_t)(int dsb, int dummy, int *out);
typedef int  (__fastcall *dsb_setlong_t)(int dsb, int dummy, int val);
typedef int  (__fastcall *dsb_play_t)(int dsb, int dummy, int pri, int flags, int reserved);
typedef int  (__fastcall *dsb_stop_t)(int dsb);

/* Helper: find sound offset by name (case-insensitive) */
static int find_sound_offset(const char *name)
{
    for (int i = 0; sound_table[i].name; i++) {
        if (_stricmp(sound_table[i].name, name) == 0)
            return sound_table[i].offset;
    }
    return -1;
}

/* Helper: get the current Board pointer */
static int get_board_ptr(void)
{
    int app = safe_read_ptr(APP_ADDR);
    if (!app) return 0;
    int profile = safe_read_ptr(app + 0x220);
    if (!profile) return 0;
    return safe_read_ptr(profile + 0x0C);
}

/* Helper: get SoundChannel* from board + sound name.
   Returns 0 on failure. */
static int get_sound_channel(const char *name)
{
    int offset = find_sound_offset(name);
    if (offset < 0) return 0;
    int board = get_board_ptr();
    if (!board) return 0;
    return safe_read_ptr(board + offset);
}

/* Helper: get IDirectSoundBuffer8* from a SoundChannel (first buffer).
   Access chain: channel+0x414 = data ptr → [0] = SoundBuffer* → +0x04 = DSB */
static int get_dsound_buffer(int channel)
{
    if (!channel || channel < 0x10000) return 0;
    int count = safe_read_ptr(channel + 0x0C);
    if (count < 1) return 0;
    int data_ptr = safe_read_ptr(channel + 0x414);
    if (!data_ptr) return 0;
    int buffer = safe_read_ptr(data_ptr);
    if (!buffer || buffer < 0x10000) return 0;
    int dsb = safe_read_ptr(buffer + 0x04);
    if (!dsb || dsb < 0x10000) return 0;
    return dsb;
}

/* Helper: call a DSound vtable method on a buffer.
   vtable_offset is the byte offset into the COM vtable (e.g. 0x3C for SetVolume). */
static int get_dsb_method(int dsb, int vtable_offset)
{
    if (!dsb || dsb < 0x10000) return 0;
    int vtable = safe_read_ptr(dsb);
    if (!vtable || vtable < 0x10000) return 0;
    return safe_read_ptr(vtable + vtable_offset);
}

/* ── Lua API: hamsterball.play_sound(name [, opts]) ─────────────────── */
/* opts is a table with optional keys:
 *   volume  (float)   0.0=silence, 1.0=full (default: 1.0)
 *   pitch   (float)    frequency multiplier, 1.0=normal, 0.5=half, 2.0=double
 *   pan     (float)    -1.0=left, 0.0=center, 1.0=right
 *   loop    (bool)     true = loop continuously, false = play once
 *   x, y, z (float)    3D position — if provided, uses Sound_Play3D
 * Returns: true on success, false on failure
 */
static int lua_play_sound(lua_State *L)
{
    const char *name = luaL_checkstring(L, 1);

    int channel = get_sound_channel(name);
    if (!channel) {
        lua_pushboolean(L, 0);
        return 1;
    }

    /* Check for options table */
    int has_opts = lua_istable(L, 2);
    float volume = 1.0f;
    float pitch  = 1.0f;
    float pan    = 0.0f;
    int   loop   = 0;
    int   has_3d = 0;
    float x = 0, y = 0, z = 0;

    if (has_opts) {
        lua_getfield(L, 2, "volume");
        if (!lua_isnil(L, -1)) volume = (float)luaL_checknumber(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, 2, "pitch");
        if (!lua_isnil(L, -1)) pitch = (float)luaL_checknumber(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, 2, "pan");
        if (!lua_isnil(L, -1)) pan = (float)luaL_checknumber(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, 2, "loop");
        if (!lua_isnil(L, -1)) loop = lua_toboolean(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, 2, "x");
        if (!lua_isnil(L, -1)) {
            x = (float)luaL_checknumber(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "y");
            y = (float)luaL_checknumber(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "z");
            z = (float)luaL_checknumber(L, -1);
            lua_pop(L, 1);
            has_3d = 1;
        } else {
            lua_pop(L, 1);
        }
    }

    /* Apply volume/pitch/pan directly to the DSound buffer before playing */
    int dsb = get_dsound_buffer(channel);
    if (dsb) {
        /* Set volume: Lua 0.0-1.0 → DSound -10000 to 0 (hundredths of dB) */
        if (volume < 0.0f) volume = 0.0f;
        if (volume > 1.0f) volume = 1.0f;
        int ds_vol = (int)((volume - 1.0f) * 10000.0f);
        int fn = get_dsb_method(dsb, 0x3C); /* SetVolume */
        if (fn) ((dsb_setlong_t)fn)(dsb, 0, ds_vol);

        /* Set pan: Lua -1.0 to 1.0 → DSound -10000 to 10000 */
        if (pan < -1.0f) pan = -1.0f;
        if (pan > 1.0f) pan = 1.0f;
        int ds_pan = (int)(pan * 10000.0f);
        fn = get_dsb_method(dsb, 0x40); /* SetPan */
        if (fn) ((dsb_setlong_t)fn)(dsb, 0, ds_pan);

        /* Set frequency (pitch): multiply original freq by pitch factor */
        if (pitch != 1.0f) {
            if (pitch < 0.0f) pitch = 0.0f;
            /* Get original frequency */
            int orig_freq = 0;
            fn = get_dsb_method(dsb, 0x20); /* GetFrequency */
            if (fn) ((dsb_getlong_t)fn)(dsb, 0, &orig_freq);
            if (orig_freq > 0) {
                int new_freq = (int)(orig_freq * pitch);
                if (new_freq < 100) new_freq = 100;     /* DSBFREQUENCY_MIN */
                if (new_freq > 200000) new_freq = 200000; /* DSBFREQUENCY_MAX */
                fn = get_dsb_method(dsb, 0x44); /* SetFrequency */
                if (fn) ((dsb_setlong_t)fn)(dsb, 0, new_freq);
            }
        }

        /* If looping, play directly via DSound Play with DSBPLAY_LOOPING */
        if (loop) {
            /* Stop first to restart from beginning */
            fn = get_dsb_method(dsb, 0x48); /* Stop */
            if (fn) ((dsb_stop_t)fn)(dsb);
            /* Set position to 0 */
            fn = get_dsb_method(dsb, 0x34); /* SetCurrentPosition */
            if (fn) ((dsb_setlong_t)fn)(dsb, 0, 0);
            /* Play with looping flag */
            fn = get_dsb_method(dsb, 0x30); /* Play */
            if (fn) ((dsb_play_t)fn)(dsb, 0, 0, DSBPLAY_LOOPING, 0);
        }
    }

    /* If not looping, use the game's own play function (handles round-robin) */
    if (!loop) {
        if (has_3d) {
            /* Sound_Play3D is __thiscall(channel, x, y, z) */
            pPlay3D(channel, 0, x, y, z);
        } else {
            /* Sound_PlayChannel is __fastcall(channel) */
            pPlayChannel(channel);
        }
    }

    lua_pushboolean(L, 1);
    return 1;
}

/* ── Lua API: hamsterball.stop_sound(name) ──────────────────────────── */
/* Stops all buffer clones of the named sound. */
static int lua_stop_sound(lua_State *L)
{
    const char *name = luaL_checkstring(L, 1);
    int channel = get_sound_channel(name);
    if (!channel) { lua_pushboolean(L, 0); return 1; }

    /* Stop all buffer clones */
    int count = safe_read_ptr(channel + 0x0C);
    int data_ptr = safe_read_ptr(channel + 0x414);
    if (!data_ptr || count < 1) { lua_pushboolean(L, 0); return 1; }

    for (int i = 0; i < count && i < 64; i++) {
        int buffer = safe_read_ptr(data_ptr + i * 4);
        if (!buffer || buffer < 0x10000) continue;
        int dsb = safe_read_ptr(buffer + 0x04);
        if (!dsb || dsb < 0x10000) continue;
        int fn = get_dsb_method(dsb, 0x48); /* Stop */
        if (fn) ((dsb_stop_t)fn)(dsb);
    }

    lua_pushboolean(L, 1);
    return 1;
}

/* ── Lua API: hamsterball.set_sound_volume(name, volume) ─────────────── */
/* volume: 0.0 = silence, 1.0 = full volume */
static int lua_set_sound_volume(lua_State *L)
{
    const char *name = luaL_checkstring(L, 1);
    float volume = (float)luaL_checknumber(L, 2);

    int channel = get_sound_channel(name);
    if (!channel) { lua_pushboolean(L, 0); return 1; }

    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;
    int ds_vol = (int)((volume - 1.0f) * 10000.0f);

    int count = safe_read_ptr(channel + 0x0C);
    int data_ptr = safe_read_ptr(channel + 0x414);
    if (!data_ptr || count < 1) { lua_pushboolean(L, 0); return 1; }

    for (int i = 0; i < count && i < 64; i++) {
        int buffer = safe_read_ptr(data_ptr + i * 4);
        if (!buffer || buffer < 0x10000) continue;
        int dsb = safe_read_ptr(buffer + 0x04);
        if (!dsb || dsb < 0x10000) continue;
        int fn = get_dsb_method(dsb, 0x3C); /* SetVolume */
        if (fn) ((dsb_setlong_t)fn)(dsb, 0, ds_vol);
    }

    lua_pushboolean(L, 1);
    return 1;
}

/* ── Lua API: hamsterball.set_sound_pitch(name, pitch) ───────────────── */
/* pitch: 1.0 = normal, 0.5 = half speed, 2.0 = double speed */
static int lua_set_sound_pitch(lua_State *L)
{
    const char *name = luaL_checkstring(L, 1);
    float pitch = (float)luaL_checknumber(L, 2);

    int channel = get_sound_channel(name);
    if (!channel) { lua_pushboolean(L, 0); return 1; }

    if (pitch < 0.0f) pitch = 0.0f;

    int count = safe_read_ptr(channel + 0x0C);
    int data_ptr = safe_read_ptr(channel + 0x414);
    if (!data_ptr || count < 1) { lua_pushboolean(L, 0); return 1; }

    for (int i = 0; i < count && i < 64; i++) {
        int buffer = safe_read_ptr(data_ptr + i * 4);
        if (!buffer || buffer < 0x10000) continue;
        int dsb = safe_read_ptr(buffer + 0x04);
        if (!dsb || dsb < 0x10000) continue;

        /* Get original frequency */
        int orig_freq = 0;
        int fn = get_dsb_method(dsb, 0x20); /* GetFrequency */
        if (fn) ((dsb_getlong_t)fn)(dsb, 0, &orig_freq);
        if (orig_freq > 0) {
            int new_freq = (int)(orig_freq * pitch);
            if (new_freq < 100) new_freq = 100;
            if (new_freq > 200000) new_freq = 200000;
            fn = get_dsb_method(dsb, 0x44); /* SetFrequency */
            if (fn) ((dsb_setlong_t)fn)(dsb, 0, new_freq);
        }
    }

    lua_pushboolean(L, 1);
    return 1;
}

/* ── Lua API: hamsterball.set_sound_pan(name, pan) ───────────────────── */
/* pan: -1.0 = full left, 0.0 = center, 1.0 = full right */
static int lua_set_sound_pan(lua_State *L)
{
    const char *name = luaL_checkstring(L, 1);
    float pan = (float)luaL_checknumber(L, 2);

    int channel = get_sound_channel(name);
    if (!channel) { lua_pushboolean(L, 0); return 1; }

    if (pan < -1.0f) pan = -1.0f;
    if (pan > 1.0f) pan = 1.0f;
    int ds_pan = (int)(pan * 10000.0f);

    int count = safe_read_ptr(channel + 0x0C);
    int data_ptr = safe_read_ptr(channel + 0x414);
    if (!data_ptr || count < 1) { lua_pushboolean(L, 0); return 1; }

    for (int i = 0; i < count && i < 64; i++) {
        int buffer = safe_read_ptr(data_ptr + i * 4);
        if (!buffer || buffer < 0x10000) continue;
        int dsb = safe_read_ptr(buffer + 0x04);
        if (!dsb || dsb < 0x10000) continue;
        int fn = get_dsb_method(dsb, 0x40); /* SetPan */
        if (fn) ((dsb_setlong_t)fn)(dsb, 0, ds_pan);
    }

    lua_pushboolean(L, 1);
    return 1;
}

/* ── Lua API: hamsterball.list_sounds() -> table ─────────────────────── */
/* Returns a table of all available sound names. */
static int lua_list_sounds(lua_State *L)
{
    int count = 0;
    for (int i = 0; sound_table[i].name; i++) count++;

    lua_createtable(L, count, 0);
    for (int i = 0; sound_table[i].name; i++) {
        lua_pushstring(L, sound_table[i].name);
        lua_rawseti(L, -2, i + 1);
    }
    return 1;
}

/* ── Register Lua API ────────────────────────────────────────────────── */
static const luaL_Reg hamsterball_funcs[] = {
    {"get_position",     lua_get_position},
    {"set_position",     lua_set_position},
    {"get_rotation",     lua_get_rotation},
    {"set_rotation",     lua_set_rotation},
    {"get_scale",        lua_get_scale},
    {"set_scale",        lua_set_scale},
    {"kill_ball",        lua_kill_ball},
    {"distance_to_ball", lua_distance_to_ball},
    {"add_score",        lua_add_score},
    {"get_ball_pos",     lua_get_ball_pos},
    {"get_delta_time",   lua_get_delta_time},
    {"get_entity_count", lua_get_entity_count},
    {"get_entity_name",  lua_get_entity_name},
    {"find_entity",      lua_find_entity},
    {"log",              lua_log},
    {"get_frame_count",  lua_get_frame_count},
    {"play_sound",       lua_play_sound},
    {"stop_sound",       lua_stop_sound},
    {"set_sound_volume", lua_set_sound_volume},
    {"set_sound_pitch",  lua_set_sound_pitch},
    {"set_sound_pan",    lua_set_sound_pan},
    {"list_sounds",      lua_list_sounds},
    {NULL, NULL}
};

static void register_api(lua_State *L)
{
    luaL_register(L, "hamsterball", hamsterball_funcs);
}

/* ── Load a Lua script for an entity ────────────────────────────────── */
static int load_entity_script(LuaEntity *ent)
{
    char path[MAX_PATH];
    _snprintf(path, MAX_PATH, "%sLua\\%s.lua", g_lua_dir, ent->name);
    
    if (GetFileAttributesA(path) == INVALID_FILE_ATTRIBUTES) {
        ent->loaded = 2;  /* no script found — static */
        return 0;
    }
    
    /* Use the shared Lua state */
    if (!g_shared_L) return 0;
    
    /* Load the file */
    int status = luaL_loadfile(g_shared_L, path);
    if (status != 0) {
        ent->loaded = 2;
        return 0;
    }
    
    /* Execute the chunk to define functions */
    status = lua_pcall(g_shared_L, 0, 0, 0);
    if (status != 0) {
        /* Script errored — treat as static */
        lua_pop(g_shared_L, lua_gettop(g_shared_L));
        ent->loaded = 2;
        return 0;
    }
    
    ent->loaded = 1;
    ent->L = g_shared_L;  /* all entities share the state */
    return 1;
}

/* ── Call the update function for an entity ──────────────────────────── */
static void call_entity_update(LuaEntity *ent, float dt)
{
    if (ent->loaded != 1 || !ent->L) return;
    
    /* Push the update function from the global table */
    lua_getglobal(ent->L, "update");
    if (!lua_isfunction(ent->L, -1)) {
        lua_pop(ent->L, 1);
        return;
    }
    
    /* Pass entity ID (1-indexed) and dt */
    int ent_id = (int)(ent - g_entities) + 1;
    lua_pushinteger(ent->L, ent_id);
    lua_pushnumber(ent->L, dt);
    
    if (lua_pcall(ent->L, 2, 0, 0) != 0) {
        /* Error calling update — pop error message */
        lua_pop(ent->L, 1);
    }
}

/* ── Scan SpatialTree for "L:" objects ───────────────────────────────── */
static void scan_entities(void)
{
    g_entity_count = 0;
    
    /* Get the current board via App->profile->board */
    int app = safe_read_ptr(APP_ADDR);
    if (!app) return;
    int profile = safe_read_ptr(app + 0x220);
    if (!profile) return;
    int board = safe_read_ptr(profile + 0x0C);
    if (!board) return;
    
    /* Level_FindObjectByName (0x00460530) reads this+0x480 directly.
     * 'this' is the board. So board+0x480 = SpatialTree pointer. */
    int spatial_root = safe_read_ptr(board + SPATIALTREE_OFFSET);
    if (!spatial_root) return;
    
    /* SpatialTree root has named objects in a list: */
    /* root+0xCB0 = count, root+0x10B8 = data ptr (from Level_FindObjectByName) */
    int count = safe_read_ptr(spatial_root + ST_OBJ_COUNT);
    if (count < 1) return;
    
    int data_ptr = safe_read_ptr(spatial_root + ST_OBJ_DATA);
    if (!data_ptr) return;
    
    /* Iterate through the object array */
    for (int i = 0; i < count && g_entity_count < MAX_LUA_ENTITIES; i++) {
        int obj_addr = safe_read_ptr(data_ptr + i * 4);
        if (!obj_addr || obj_addr < 0x10000) continue;
        if (IsBadReadPtr((void*)obj_addr, 0x100)) continue;
        
        /* Read the name pointer at obj+0x50 */
        int name_ptr = safe_read_ptr(obj_addr + OBJ_NAME);
        if (!name_ptr || name_ptr < 0x10000) continue;
        if (IsBadReadPtr((void*)name_ptr, 64)) continue;
        
        /* Check if name starts with "L:" */
        char *name = (char*)name_ptr;
        if (name[0] != 'L' || name[1] != ':') continue;
        
        /* Extract the entity name (after "L:") */
        char ent_name[64];
        const char *src = name + 2;
        int j;
        for (j = 0; j < 63 && src[j]; j++) {
            ent_name[j] = src[j];
        }
        ent_name[j] = '\0';
        
        /* Register the entity */
        LuaEntity *ent = &g_entities[g_entity_count];
        memset(ent, 0, sizeof(LuaEntity));
        ent->obj_addr = obj_addr;
        strncpy(ent->name, ent_name, 63);
        ent->name[63] = '\0';
        ent->loaded = 0;
        
        /* Try to load the Lua script */
        load_entity_script(ent);
        
        /* Store initial position */
        ent->last_pos[0] = safe_read_float(obj_addr + OBJ_POS_X);
        ent->last_pos[1] = safe_read_float(obj_addr + OBJ_POS_Y);
        ent->last_pos[2] = safe_read_float(obj_addr + OBJ_POS_Z);
        
        g_entity_count++;
    }
}

/* ── Scene_Update hook ───────────────────────────────────────────────── */
/* Scene_Update at 0x419C00 is __fastcall: ECX = scene/board ptr */
/* Original prologue (first 7 bytes): */
/*   6A FF              PUSH -1 */
/*   68 5E 9A 4C 00     PUSH 0x4C9A5E */
/* We detour at the entry, call original via trampoline, then run our logic. */

static const unsigned char ORIG_PROLOGUE[7] = {
    0x6A, 0xFF,           /* PUSH -1 (SEH frame) */
    0x68, 0xFB, 0x9F, 0x4C, 0x00  /* PUSH 0x4C9FFB (SEH handler) */
};

static unsigned char *g_tramp = NULL;
static int g_hook_installed = 0;
static int g_scene_frame = 0;

/* Forward declarations */
static void __fastcall scene_update_hook(int *board);

static void install_hook(void)
{
    DWORD target = SCENE_UPDATE_ADDR;
    DWORD old_prot;
    unsigned char jmp_patch[7];
    
    /* Verify original bytes */
    if (memcmp((void*)target, ORIG_PROLOGUE, 7) != 0) {
        /* Prologue mismatch — wrong EXE version? */
        return;
    }
    
    /* Allocate executable trampoline (16 bytes) */
    g_tramp = VirtualAlloc(NULL, 16,
                MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!g_tramp) return;
    
    /* Trampoline: original prologue (7 bytes) + JMP back to target+7 (5 bytes) */
    memcpy(g_tramp, ORIG_PROLOGUE, 7);
    g_tramp[7] = 0xE9;  /* JMP rel32 */
    *(DWORD*)(g_tramp + 8) = (target + 7) - ((DWORD)g_tramp + 12);
    
    /* Patch original: JMP to hook + 2 NOPs (total 7 bytes) */
    VirtualProtect((void*)target, 7, PAGE_EXECUTE_READWRITE, &old_prot);
    jmp_patch[0] = 0xE9;  /* JMP rel32 */
    *(DWORD*)(jmp_patch + 1) = (DWORD)&scene_update_hook - (target + 5);
    jmp_patch[5] = 0x90;  /* NOP */
    jmp_patch[6] = 0x90;  /* NOP */
    memcpy((void*)target, jmp_patch, 7);
    VirtualProtect((void*)target, 7, old_prot, &old_prot);
    FlushInstructionCache(GetCurrentProcess(), (void*)target, 7);
    
    g_hook_installed = 1;
}

static void __fastcall scene_update_hook(int *board)
{
    /* Call original Scene_Update via trampoline */
    typedef void (__fastcall *orig_fn_t)(int*);
    ((orig_fn_t)g_tramp)(board);
    
    /* Calculate delta time */
    DWORD now = GetTickCount();
    if (g_last_tick > 0) {
        g_dt = (float)(now - g_last_tick) / 1000.0f;
        if (g_dt > 0.1f) g_dt = 0.1f;  /* clamp */
        if (g_dt <= 0.0f) g_dt = 0.016f;
    }
    g_last_tick = now;
    
    g_scene_frame++;
    
    /* Re-scan entities every 60 frames (1 second at 60fps) */
    /* This catches level changes / new objects being loaded */
    if (g_scene_frame % 60 == 1) {
        scan_entities();
    }
    
    /* Run Lua update for each entity */
    if (g_shared_L && g_entity_count > 0) {
        for (int i = 0; i < g_entity_count; i++) {
            if (g_entities[i].loaded == 1) {
                call_entity_update(&g_entities[i], g_dt);
            }
        }
    }
}

/* ── Create the Lua directory ───────────────────────────────────────── */
static void create_lua_directory(void)
{
    char lua_path[MAX_PATH];
    get_game_dir(g_lua_dir, MAX_PATH);
    
    _snprintf(lua_path, MAX_PATH, "%sLua", g_lua_dir);
    CreateDirectoryA(lua_path, NULL);
    /* Ignore error if it already exists */
}

/* ── Initialize the Lua state ────────────────────────────────────────── */
static void init_lua(void)
{
    g_shared_L = luaL_newstate();
    if (!g_shared_L) return;
    
    /* Open standard libraries */
    luaL_openlibs(g_shared_L);
    
    /* Register our API */
    register_api(g_shared_L);
    
    /* Set package.path to include our Lua directory */
    lua_getglobal(g_shared_L, "package");
    if (lua_istable(g_shared_L, -1)) {
        char path_str[MAX_PATH * 2];
        _snprintf(path_str, MAX_PATH * 2,
                  "%sLua\\?.lua;%sLua\\?.lua;%sLua\\?\\init.lua",
                  g_lua_dir, g_lua_dir, g_lua_dir);
        lua_pushstring(g_shared_L, path_str);
        lua_setfield(g_shared_L, -2, "path");
    }
    lua_pop(g_shared_L, 1);
}

/* ── Mod thread ──────────────────────────────────────────────────────── */
static DWORD WINAPI mod_thread(LPVOID param)
{
    (void)param;
    
    /* Wait for the game to fully initialize */
    Sleep(3000);
    
    /* Create the Lua directory */
    create_lua_directory();
    
    /* Initialize Lua */
    init_lua();
    
    /* Install the Scene_Update hook */
    install_hook();
    
    return 0;
}

/* ── DllMain ─────────────────────────────────────────────────────────── */
BOOL APIENTRY DllMain(HINSTANCE hInst, DWORD reason, LPVOID lpReserved)
{
    (void)lpReserved;
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hInst);
        load_real_bass();
        CreateThread(NULL, 0, mod_thread, NULL, 0, NULL);
        break;
    case DLL_PROCESS_DETACH:
        if (g_shared_L) {
            lua_close(g_shared_L);
            g_shared_L = NULL;
        }
        break;
    }
    return TRUE;
}
