/*
 * CustomEntities_MinGW.cpp — HB+ port of mkn_custom_entities v53f.
 *
 * Spawns custom entities (Rotators, Pendulums, Gears, etc.) from
 * MESHWORLD section-3 C_entity entries. Extensive entity spawning
 * logic, AI constructors, and DAT property parsing — all preserved
 * from the original bass.dll proxy mod.
 *
 * Uses nocrt + manual 17-entry vtable for HB+ v2.1 compatibility.
 *
 * Original: custom_entities.c (2926 lines, v53f)
 * Ported to HB+ by replacing bass.dll proxy boilerplate with
 * HB+ callbacks (onLevelStart, onSceneEnd, onGameUpdate).
 */

#include "nocrt.h"
#include "HamsterballAPI.h"
#include "hbplus_api.h"
#include <shlwapi.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * Game function pointers — all from original mod, unchanged
 * ═══════════════════════════════════════════════════════════════════════════ */

/* operator_new — game's C++ allocator (malloc wrapper) */
typedef void* (__cdecl *operator_new_t)(SIZE_T);
static operator_new_t pfn_operator_new = (operator_new_t)0x004BA57B;

/* MeshWorld_ctor — loads a .MESHWORLD file into a mesh object */
typedef void* (__thiscall *MeshWorld_ctor_t)(void* this_, void* gfx_device, const char* mesh_path);
static MeshWorld_ctor_t pfn_MeshWorld_ctor = (MeshWorld_ctor_t)0x00461510;

/* MeshNode_ctor — loads a .MESH file into a 0x18-byte MeshNode wrapper */
typedef void* (__thiscall *MeshNode_ctor_t)(void* this_, void* gfx_device, const char* mesh_path);
static MeshNode_ctor_t pfn_MeshNode_ctor = (MeshNode_ctor_t)0x00471C20;

/* _check_file_access — checks if file exists */
typedef int (__cdecl *check_file_access_t)(const char* path, unsigned char mode);
static check_file_access_t pfn_check_file_access = (check_file_access_t)0x004C8FF7;

/* MeshWorld_ctor (simple) */
typedef void* (__thiscall *MeshWorld_ctor_simple_t)(void* this_, void* gfx_device);
static MeshWorld_ctor_simple_t pfn_MeshWorld_ctor_simple = (MeshWorld_ctor_simple_t)0x004706E0;

/* MeshNode size constants */
#define MESHNODE_SIZE           0x18
#define MESHWORLD_INNER_SIZE    0x488

/* Level_RenderCtor */
typedef void* (__thiscall *Level_RenderCtor_t)(void* this_, void* parent_level);
static Level_RenderCtor_t pfn_Level_RenderCtor = (Level_RenderCtor_t)0x00465080;

#define LEVEL_SIZE             0x10D0

/* PopCylinder_ctor */
typedef void* (__thiscall *PopCylinder_ctor_t)(void* this_, void* board, float posX, float posY, float posZ, void* mesh);
static PopCylinder_ctor_t pfn_PopCylinder_ctor = (PopCylinder_ctor_t)0x00436EE0;

/* AthenaList_Append */
typedef void (__thiscall *AthenaList_Append_t)(DWORD* list, int item);
static AthenaList_Append_t pfn_AthenaList_Append = (AthenaList_Append_t)0x00453810;

/* AthenaList_RemoveByValue */
typedef void (__thiscall *AthenaList_Remove_t)(DWORD* list, int item);
static AthenaList_Remove_t pfn_AthenaList_Remove = (AthenaList_Remove_t)0x004534D0;

/* Board layout */
#define BOARD_APP               0x878
#define BOARD_LEVEL             0x8AC
#define BOARD_UPDATE_LIST       0x2578
#define BOARD_RENDER_LIST       0xCD4
#define BOARD_COLLISION_LIST    0x10EC
#define BOARD_SCENE_OBJ         0x8B0
#define BOARD_SCENE_UPDATE_LIST 0x8B8

/* App layout */
#define APP_GFX_DEVICE          0x174

/* Level/SceneObject layout */
#define LEVEL_SCENEOBJECT       0x480

/* PopCylinder layout */
#define PC_COLLISION_OBJ        0x10E0

/* S1 entry layout */
#define S1ENTRY_NAME             0x00
#define S1ENTRY_POS_X            0x04
#define S1ENTRY_POS_Y            0x08
#define S1ENTRY_POS_Z            0x0C

/* Object size constants */
#define MESHWORLD_SIZE          0x10D0
#define POPCYLINDER_SIZE        0x10D0
#define ROTATER_SIZE            0x1508
#define PENDULUM_SIZE           0x1504
#define LOOPER_SIZE             0x1500
#define GEAR_SIZE               0x1514
#define SPATIALTREE_SIZE        68

/* ═══════════════════════════════════════════════════════════════════════════
 * Entity constructors — all verified via Ghidra decompilation
 * (ctor_type values documented in original source)
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef void* (__thiscall *Rotator_ctor_t)(void* this_, void* board, float posX, float posY, float posZ, void* mesh);
static Rotator_ctor_t pfn_Rotator_ctor = (Rotator_ctor_t)0x00435940;
static Rotator_ctor_t pfn_Pendulum_ctor = (Rotator_ctor_t)0x437700;
static Rotator_ctor_t pfn_Looper_ctor = (Rotator_ctor_t)0x437460;
static Rotator_ctor_t pfn_Gear_ctor = (Rotator_ctor_t)0x437690;

typedef void* (__thiscall *ArenaStands_ctor_t)(void* this_, void* board, float x, float y, float z, void* mesh);
static ArenaStands_ctor_t pfn_ArenaStands_ctor = (ArenaStands_ctor_t)0x0043E450;

static void* cEnt_DFloor1_ctor(void* obj, void* board, float x, float y, float z, void* mesh) {
    return pfn_ArenaStands_ctor(obj, board, x, y, z, mesh);
}
static void* cEnt_DFloor2_ctor(void* obj, void* board, float x, float y, float z, void* mesh) {
    return pfn_ArenaStands_ctor(obj, board, x, y, z, mesh);
}
static void* cEnt_DFloor3_ctor(void* obj, void* board, float x, float y, float z, void* mesh) {
    return pfn_ArenaStands_ctor(obj, board, x, y, z, mesh);
}
static void* cEnt_DFloor4_ctor(void* obj, void* board, float x, float y, float z, void* mesh) {
    void* result = pfn_ArenaStands_ctor(obj, board, x, y, z, mesh);
    *(DWORD*)((char*)obj + 0x10DC) = 2;
    *(DWORD*)((char*)obj + 0x10E0) = 0;
    return result;
}
static void* cEnt_FlickRing_ctor(void* obj, void* board, float x, float y, float z, void* mesh) {
    return pfn_ArenaStands_ctor(obj, board, x, y, z, mesh);
}
static void* cEnt_Trode_ctor(void* obj, void* board, float x, float y, float z, void* mesh) {
    return pfn_ArenaStands_ctor(obj, board, x, y, z, mesh);
}

typedef void* (__thiscall *Spinner_Level_ctor_t)(void* this_, void* board, float x, float y, float z, void* mesh, float param);
static Spinner_Level_ctor_t pfn_Spinner_Level_ctor = (Spinner_Level_ctor_t)0x004396F0;

typedef void* (__thiscall *Sprite_ctor_t)(void* this_, void* gfx_device, const char* texture_path);
static Sprite_ctor_t pfn_Sprite_ctor = (Sprite_ctor_t)0x0045D0C0;

typedef void* (__thiscall *Gear_ctor_t)(void* this_, void* board, float x1, float y1, float z1, float x2, float y2, float z2, void* mesh);
static Gear_ctor_t pfn_Gear_ctor_real = (Gear_ctor_t)0x00437690;

typedef void* (__thiscall *Bell_ctor_t)(void* this_, void* board, float x, float y, float z);
static Bell_ctor_t pfn_Bell_ctor = (Bell_ctor_t)0x00434D70;

typedef void* (__thiscall *Fan_ctor_t)(void* this_, void* board, float x, float y, float z, float param);
static Fan_ctor_t pfn_Fan_ctor = (Fan_ctor_t)0x00438C20;

typedef void* (__thiscall *SawBlade_ctor_t)(void* this_, void* board, float x, float y, float z);
static SawBlade_ctor_t pfn_SawBlade_ctor = (SawBlade_ctor_t)0x00434660;

typedef void* (__thiscall *Bonk_ctor_t)(void* this_, void* board, float x, float y, float z);
static Bonk_ctor_t pfn_Bonk_ctor = (Bonk_ctor_t)0x00438850;

typedef void* (__thiscall *BreakBridge_ctor_t)(void* this_, void* board, float x, float y, float z, void* mesh);
static BreakBridge_ctor_t pfn_BreakBridge_ctor = (BreakBridge_ctor_t)0x00436D70;

typedef void* (__thiscall *StandsCtor_t)(void* this_, void* board, float x, float y, float z, void* mesh);
static StandsCtor_t pfn_Catapult_ctor      = (StandsCtor_t)0x00437E10;
static StandsCtor_t pfn_Mace_ctor          = (StandsCtor_t)0x00438750;
static StandsCtor_t pfn_Tipper_ctor        = (StandsCtor_t)0x00437960;
static StandsCtor_t pfn_Lifter_ctor        = (StandsCtor_t)0x00436920;
static StandsCtor_t pfn_SpeedCylinder_ctor = (StandsCtor_t)0x00436A20;
static StandsCtor_t pfn_NeonPlatform_ctor   = (StandsCtor_t)0x0043E110;
static StandsCtor_t pfn_Trapdoor_ctor      = (StandsCtor_t)0x00438290;
static StandsCtor_t pfn_Odd_Lifter_ctor    = (StandsCtor_t)0x00434E60;

typedef void* (__thiscall *GameLevel_ctor_t)(void* this_, void* board, float x, float y, float z, void* mesh);
static GameLevel_ctor_t pfn_GameLevel_ctor = (GameLevel_ctor_t)0x004351F0;

typedef void* (__thiscall *Glass_Level_ctor_t)(void* this_, void* board, void* mesh);
static Glass_Level_ctor_t pfn_Glass_Level_ctor = (Glass_Level_ctor_t)0x004384A0;

typedef void* (__thiscall *Gear_Level_ctor_t)(void* this_, void* board, float x, float y, float z);
static Gear_Level_ctor_t pfn_Gear_Level_ctor = (Gear_Level_ctor_t)0x0043A150;

typedef void* (__thiscall *Secret_ctor_t)(void* this_, void* board, float x, float y, float z, void* mesh);
static Secret_ctor_t pfn_Secret_ctor = (Secret_ctor_t)0x0043DFB0;

typedef void* (__thiscall *FlagWaver_Ctor_t)(void* this_, void* gfx_device);
static FlagWaver_Ctor_t pfn_FlagWaver_Ctor = (FlagWaver_Ctor_t)0x0046AF30;

typedef void (__cdecl *TipperVisual_Attach_t)(void* visual, int behavior);
static TipperVisual_Attach_t pfn_TipperVisual_Attach = (TipperVisual_Attach_t)0x00465200;

typedef void* (__thiscall *Sign_ctor_t)(void* this_, void* board, int gfx1, int gfx2,
    int x, int y, int z, int str1, int str2, int str3);
static Sign_ctor_t pfn_Sign_ctor = (Sign_ctor_t)0x00443B90;

typedef void* (__thiscall *Wavy_ctor_t)(void* this_, void* board, float x, float y, float z, const char* mesh_path);
static Wavy_ctor_t pfn_Wavy_ctor = (Wavy_ctor_t)0x0043AD40;

typedef void (__thiscall *Wavy_Configure_t)(void* this_, int a, float b, float c, float d);
static Wavy_Configure_t pfn_Wavy_Configure = (Wavy_Configure_t)0x00435440;

typedef void* (__thiscall *BadBall_ctor_t)(void* this_, void* board);
static BadBall_ctor_t pfn_BadBall_ctor = (BadBall_ctor_t)0x0040AFE0;

typedef void* (__thiscall *TipperVisual_ctor_t)(void* this_, int parent_mesh);
static TipperVisual_ctor_t pfn_TipperVisual_ctor2 = (TipperVisual_ctor_t)0x004661A0;

typedef void (__thiscall *Sound_Play3D_t)(void* soundChannel, float x, float y, float z);
static Sound_Play3D_t pfn_Sound_Play3D = (Sound_Play3D_t)0x00459860;

typedef void (__thiscall *Gfx_ScaleFn_t)(void* gfx, float val);
static Gfx_ScaleFn_t pfn_Gfx_ScaleZ_Bridge = (Gfx_ScaleFn_t)0x00457CC0;

typedef void (__thiscall *Gfx_SetPosition_t)(void* gfx, float x, float y, float z);
static Gfx_SetPosition_t pfn_Gfx_SetPosition_Bridge = (Gfx_SetPosition_t)0x00457B50;

typedef void (__fastcall *Timer_Init_t)(void* out);
static Timer_Init_t pfn_Timer_Init = (Timer_Init_t)0x00457AD0;
typedef void (__fastcall *Timer_Cleanup_t)(void* out);
static Timer_Cleanup_t pfn_Timer_Cleanup = (Timer_Cleanup_t)0x00457A40;

typedef void (__thiscall *Vec3_Copy_t)(float* dst, float* src);
static Vec3_Copy_t pfn_Vec3_Copy_Bridge = (Vec3_Copy_t)0x00402BF0;

typedef void (__thiscall *Scene_ForEachBall_SetVel_t)(void* board, float x, float y, float z);
static Scene_ForEachBall_SetVel_t pfn_Scene_ForEachBall_SetVel = (Scene_ForEachBall_SetVel_t)0x00419B70;

#define APP_SOUNDFX_BRIDGESLAM  0x47C

/* Object sizes for ctor types */
#define ARENASTANDS_SIZE      0x1104
#define GAMELEVEL_SIZE        0x1524
#define GLASS_LEVEL_SIZE      0x113C
#define GEAR_LEVEL_SIZE       0x1100
#define SECRET_SIZE           0x10EC
#define FLAGWAVER_SIZE        0x8C
#define SIGN_SIZE             0x10FC
#define WAVY_SIZE             0x1AE7C
#define BADBALL_SIZE          0xC98
#define BRIDGESLAM_SIZE       0x10D0
#define SPINNER_LEVEL_SIZE    0x10FC
#define SPRITE_SIZE           0xD4
#define GEAR_REAL_SIZE        0x1514
#define BELL_SIZE             0x10E8
#define FAN_SIZE              0x1188
#define SAWBLADE_SIZE         0x111C
#define BONK_SIZE              0x1200
#define CATAPULT_SIZE         0x1108
#define MACE_SIZE             0x110C
#define TIPPER_SIZE           0x1104
#define LIFTER_SIZE           0x10F4
#define SPEEDCYLINDER_SIZE    0x150C
#define NEONPLATFORM_SIZE     0x1104
#define TRAPDOOR_SIZE         0x10F8
#define ODD_LIFTER_SIZE       0x10F4

/* Level3-Swirl mesh path (game .data at 0x004CFFE0) */
static const char* g_swirl_mesh_path = (const char*)0x004CFFE0;

/* Gfx_Scale function pointers */
typedef void (__cdecl *Gfx_Scale_t)(float);
static Gfx_Scale_t pfn_Gfx_ScaleX = (Gfx_Scale_t)0x00457C60;
static Gfx_Scale_t pfn_Gfx_ScaleY = (Gfx_Scale_t)0x00457C90;
static Gfx_Scale_t pfn_Gfx_ScaleZ = (Gfx_Scale_t)0x00457CC0;

/* AI mesh path table — game .data string addresses for AI 1-5 */
static const char* g_ai_mesh_paths[] = {
    NULL,
    (const char*)0x004D20FC,
    (const char*)0x004D20DC,
    (const char*)0x004D2158,
    (const char*)0x004D213C,
    (const char*)0x004D211C,
};

/* ═══════════════════════════════════════════════════════════════════════════
 * Bridgeslam state machine (preserved from original)
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef struct {
    DWORD obj;
    DWORD render_obj;
    DWORD mesh_world;
    DWORD board;
    float pivot_x, pivot_y, pivot_z;
    float angle;
    int   state;
    int   counter;
    int   active;
} BridgeslamState;

#define MAX_BRIDGESLAMS 16
static BridgeslamState g_bridgeslams[MAX_BRIDGESLAMS];
static int g_bridgeslam_count = 0;

static void cEnt_bridgeslam_update(BridgeslamState* bs) {
    if (!bs || !bs->active) return;
    if (!bs->render_obj) return;

    DWORD board = bs->board;
    DWORD app = 0;
    if (board) {
        app = *(DWORD*)(board + BOARD_APP);
        if (!app || IsBadReadPtr((void*)app, 0x800)) app = 0;
    }

    switch (bs->state) {
    case 0:
        bs->counter--;
        if (bs->counter < 1) bs->state = 1;
        break;

    case 1:
        bs->angle -= 3.0f;
        if (bs->angle < 0.0f) {
            bs->angle = 0.0f;
            bs->counter = 125;
            bs->state = 2;
            if (app && pfn_Sound_Play3D) {
                DWORD snd = *(DWORD*)(app + APP_SOUNDFX_BRIDGESLAM);
                if (snd) {
                    pfn_Sound_Play3D((void*)snd, bs->pivot_x, bs->pivot_y, bs->pivot_z);
                }
            }
            if (board && pfn_Scene_ForEachBall_SetVel) {
                pfn_Scene_ForEachBall_SetVel((void*)board,
                    bs->pivot_y, bs->pivot_z, 0.5f);
            }
        }
        break;

    case 2:
        bs->counter--;
        if (bs->counter < 1) bs->state = 3;
        break;

    case 3:
        bs->angle += 0.5f;
        if (bs->angle >= 45.0f) {
            bs->angle = 45.0f;
            bs->counter = 75;
            bs->state = 0;
        }
        if (app) {
            DWORD gfx = *(DWORD*)(app + APP_GFX_DEVICE);
            if (gfx && pfn_Gfx_ScaleZ_Bridge && pfn_Gfx_SetPosition_Bridge &&
                pfn_Timer_Init && pfn_Timer_Cleanup) {
                char timerBuf[68];
                pfn_Timer_Init(timerBuf);
                pfn_Gfx_ScaleZ_Bridge((void*)gfx, -bs->angle);
                pfn_Gfx_SetPosition_Bridge((void*)gfx,
                    bs->pivot_x, bs->pivot_y, bs->pivot_z);
                DWORD* renderVtbl = *(DWORD**)bs->render_obj;
                if (renderVtbl && !IsBadReadPtr(renderVtbl, 0x60)) {
                    void (__fastcall *fn58)(DWORD) = (void (__fastcall *)(DWORD))renderVtbl[0x16];
                    void (__fastcall *fn54)(DWORD, char*) = (void (__fastcall *)(DWORD, char*))renderVtbl[0x15];
                    if (fn58) fn58((DWORD)bs->render_obj);
                    if (fn54) fn54((DWORD)bs->render_obj, timerBuf);
                }
                pfn_Timer_Cleanup(timerBuf);
            }
        }
        break;
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Rotater tracking
 * ═══════════════════════════════════════════════════════════════════════════ */

#define MAX_ROTATERS 999

typedef struct {
    DWORD obj;
    char  mesh_path[128];
    float rot_x;
    float rot_y;
    float rot_z;
    float ros_x;
    float ros_y;
    float ros_z;
    float angle_x;
    float angle_y;
    float angle_z;
    int   rng_seed;
    float rot_a;
    float rot_d;
    float rot_max;
    float rot_min;
    int   rot_m;
    int   has_custom_limits;
} RotaterConfig;

static RotaterConfig g_rotater_cfg[MAX_ROTATERS];
static int   g_rotater_count = 0;

/* ═══════════════════════════════════════════════════════════════════════════
 * Ball struct offsets for tag processing
 * ═══════════════════════════════════════════════════════════════════════════ */

#define BALL_HOME_POS_X       0xC60
#define BALL_HOME_POS_Y       0xC64
#define BALL_HOME_POS_Z       0xC68
#define BALL_MESH_INDEX_FIELD 0x754
#define BALL_MAX_SPEED        0x188

#define APP_MESH_ARRAY         0x244
#define APP_MESH_8BALL         0x268
#define APP_MESH_FUNBALL       0x26C
#define MESH_SLOT_8BALL        1
#define MESH_SLOT_FUNBALL      2

#define SCENEOBJ_OBJ_COUNT    0x898
#define SCENEOBJ_OBJ_ARRAY    0xCA0
#define BOARD_BAD_BALLS_LIST  0x29D4

/* ═══════════════════════════════════════════════════════════════════════════
 * State — HB+ port: no thread, uses callbacks
 * ═══════════════════════════════════════════════════════════════════════════ */

static void* g_storedApi = NULL;
static char g_game_dir[MAX_PATH] = {0};

/* Track spawned testcube objects */
#define MAX_SPAWNED 16
static DWORD g_spawned_objs[MAX_SPAWNED];
static char  g_spawned_names[MAX_SPAWNED][32];
static int   g_spawned_count = 0;
static DWORD g_spawned_board = 0;

static float g_grid_speed = 3.0f;
static char g_mesh_path[] = "levels\\testcube";

/* ═══════════════════════════════════════════════════════════════════════════
 * nc_ replacements for CRT functions used by original mod
 * ═══════════════════════════════════════════════════════════════════════════ */

/* nc_strstr — substring search */
static const char* nc_strstr(const char* haystack, const char* needle) {
    if (!haystack || !needle) return NULL;
    size_t nlen = nc_strlen(needle);
    if (nlen == 0) return haystack;
    for (; *haystack; haystack++) {
        size_t i;
        for (i = 0; i < nlen; i++) {
            if (haystack[i] != needle[i]) break;
        }
        if (i == nlen) return haystack;
    }
    return NULL;
}

/* nc_strchr — find char in string */
static char* nc_strchr(const char* s, int c) {
    while (*s) {
        if (*s == (char)c) return (char*)s;
        s++;
    }
    return (c == 0) ? (char*)s : NULL;
}

/* nc_strrchr — find last occurrence of char in string */
static char* nc_strrchr(const char* s, int c) {
    const char* last = NULL;
    while (*s) {
        if (*s == (char)c) last = s;
        s++;
    }
    return (char*)((c == 0) ? s : last);
}

/* nc_tolower — ASCII tolower */
static int nc_tolower(int c) {
    if (c >= 'A' && c <= 'Z') return c + 32;
    return c;
}

/* nc_atof — parse float from string (replaces atof) */
static float nc_atof(const char* s) {
    if (!s || !*s) return 0.0f;
    while (*s == ' ' || *s == '\t') s++;
    int negative = 0;
    if (*s == '-') { negative = 1; s++; }
    else if (*s == '+') { s++; }
    float result = 0.0f;
    while (*s >= '0' && *s <= '9') {
        result = result * 10.0f + (float)(*s - '0');
        s++;
    }
    if (*s == '.') {
        s++;
        float div = 10.0f;
        while (*s >= '0' && *s <= '9') {
            result += (float)(*s - '0') / div;
            div *= 10.0f;
            s++;
        }
    }
    /* Handle exponent (e/E) */
    if (*s == 'e' || *s == 'E') {
        s++;
        int exp_neg = 0;
        if (*s == '-') { exp_neg = 1; s++; }
        else if (*s == '+') { s++; }
        int exp_val = 0;
        while (*s >= '0' && *s <= '9') {
            exp_val = exp_val * 10 + (*s - '0');
            s++;
        }
        float exp_f = 1.0f;
        int k;
        for (k = 0; k < exp_val; k++) exp_f *= 10.0f;
        if (exp_neg) result /= exp_f;
        else result *= exp_f;
    }
    return negative ? -result : result;
}

/* nc_strtol — parse long from string (replaces strtol) */
static long nc_strtol(const char* s, char** endptr, int base) {
    if (!s) return 0;
    while (*s == ' ' || *s == '\t') s++;
    int negative = 0;
    if (*s == '-') { negative = 1; s++; }
    else if (*s == '+') { s++; }
    /* Handle 0x prefix for base 0 or 16 */
    if ((base == 0 || base == 16) && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
        s += 2;
        base = 16;
    }
    if (base == 0) base = 10;
    long result = 0;
    while (*s) {
        int digit;
        if (*s >= '0' && *s <= '9') digit = *s - '0';
        else if (*s >= 'a' && *s <= 'f') digit = *s - 'a' + 10;
        else if (*s >= 'A' && *s <= 'F') digit = *s - 'A' + 10;
        else break;
        if (digit >= base) break;
        result = result * base + digit;
        s++;
    }
    if (endptr) *endptr = (char*)s;
    return negative ? -result : result;
}

/* nc_stricmp — case-insensitive string compare (replaces _stricmp) */
static int nc_stricmp(const char* a, const char* b) {
    while (*a && *b) {
        int ca = nc_tolower((unsigned char)*a);
        int cb = nc_tolower((unsigned char)*b);
        if (ca != cb) return ca - cb;
        a++;
        b++;
    }
    return nc_tolower((unsigned char)*a) - nc_tolower((unsigned char)*b);
}

/* nc_strnicmp — case-insensitive compare n chars (replaces _strnicmp) */
static int nc_strnicmp(const char* a, const char* b, size_t n) {
    while (n > 0) {
        int ca = nc_tolower((unsigned char)*a);
        int cb = nc_tolower((unsigned char)*b);
        if (ca != cb) return ca - cb;
        if (*a == 0) return 0;
        a++;
        b++;
        n--;
    }
    return 0;
}

/* nc_stristr — case-insensitive substring search */
static const char* nc_stristr(const char* haystack, const char* needle) {
    if (!haystack || !needle) return NULL;
    size_t nlen = nc_strlen(needle);
    if (nlen == 0) return haystack;
    for (; *haystack; haystack++) {
        if (nc_strnicmp(haystack, needle, nlen) == 0)
            return haystack;
    }
    return NULL;
}

/* nc_fabs — absolute value of float */
static float nc_fabs(float v) {
    return v < 0.0f ? -v : v;
}

/* nc_fopen / nc_fclose / nc_fgets / nc_fprintf — file I/O via Win32
 * These are minimal stubs that use CreateFileA/ReadFile/WriteFile.
 * The original mod used FILE* for logging and config reading.
 * In HB+ mode, logging is optional (NULL logf is handled everywhere). */

typedef struct {
    HANDLE h;
    int is_write;
    char* buf;       /* read buffer */
    size_t buf_size;  /* buffer capacity */
    size_t buf_pos;   /* current position in buffer */
    size_t buf_len;   /* total data in buffer */
} nc_FILE;

static nc_FILE* nc_fopen(const char* path, const char* mode) {
    HANDLE h;
    int is_write = 0;
    if (mode[0] == 'w' || mode[0] == 'a') {
        is_write = 1;
        h = CreateFileA(path, GENERIC_WRITE, FILE_SHARE_READ, NULL,
                        mode[0] == 'a' ? OPEN_ALWAYS : CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL, NULL);
        if (mode[0] == 'a' && h != INVALID_HANDLE_VALUE) {
            SetFilePointer(h, 0, NULL, FILE_END);
        }
    } else {
        h = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL,
                        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    }
    if (h == INVALID_HANDLE_VALUE) return NULL;
    nc_FILE* f = (nc_FILE*)nc_malloc(sizeof(nc_FILE));
    if (!f) { CloseHandle(h); return NULL; }
    f->h = h;
    f->is_write = is_write;
    f->buf = NULL;
    f->buf_size = 0;
    f->buf_pos = 0;
    f->buf_len = 0;
    return f;
}

static int nc_fclose(nc_FILE* f) {
    if (!f) return 0;
    if (f->buf) nc_free(f->buf);
    CloseHandle(f->h);
    nc_free(f);
    return 0;
}

/* nc_fflush — flush write buffer */
static int nc_fflush(nc_FILE* f) {
    if (!f) return 0;
    FlushFileBuffers(f->h);
    return 0;
}

/* nc_fprintf — minimal printf for log output.
 * Only supports %s, %d, %.1f, %.4f, %08X, %02X, %02d, %X, %c patterns. */
static void nc_fprintf(nc_FILE* f, const char* fmt, ...) {
    if (!f) return;
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    /* Use wvsprintfA from user32.dll (already loaded by nc_snprintf) */
    typedef int (WINAPI *wvsprintfA_t)(LPSTR, LPCSTR, va_list);
    static wvsprintfA_t pWvsprintfA = NULL;
    if (!pWvsprintfA) {
        HMODULE hUser = GetModuleHandleA("user32.dll");
        if (hUser) pWvsprintfA = (wvsprintfA_t)GetProcAddress(hUser, "wvsprintfA");
    }
    if (!pWvsprintfA) { va_end(args); return; }
    int result = pWvsprintfA(buf, fmt, args);
    va_end(args);
    if (result > 0) {
        DWORD written;
        WriteFile(f->h, buf, result, &written, NULL);
    }
}

/* nc_fgets — read a line from file. Uses buffered read. */
static char* nc_fgets(char* out, int size, nc_FILE* f) {
    if (!f || !out || size <= 0) return NULL;
    int pos = 0;
    while (pos < size - 1) {
        /* Need to refill buffer? */
        if (f->buf_pos >= f->buf_len) {
            if (!f->buf) {
                f->buf_size = 4096;
                f->buf = (char*)nc_malloc(f->buf_size);
                if (!f->buf) return NULL;
            }
            DWORD bytesRead = 0;
            if (!ReadFile(f->h, f->buf, (DWORD)f->buf_size, &bytesRead, NULL) || bytesRead == 0) {
                if (pos == 0) return NULL;
                break;
            }
            f->buf_pos = 0;
            f->buf_len = bytesRead;
        }
        char c = f->buf[f->buf_pos++];
        out[pos++] = c;
        if (c == '\n') break;
    }
    out[pos] = '\0';
    return (pos > 0) ? out : NULL;
}

/* nc_sscanf — minimal sscanf for "KEY VALUE" parsing.
 * Supports only %s and %d patterns used in load_config/load_entity_def. */
static int nc_sscanf_key_val(const char* str, char* key, int key_size, char* val, int val_size) {
    /* Parse: skip whitespace, read key (non-whitespace), skip whitespace, read val (non-whitespace) */
    const char* p = str;
    while (*p == ' ' || *p == '\t') p++;
    int klen = 0;
    while (*p && *p != ' ' && *p != '\t' && *p != '\n' && *p != '\r' && klen < key_size - 1) {
        key[klen++] = *p++;
    }
    key[klen] = '\0';
    while (*p == ' ' || *p == '\t') p++;
    int vlen = 0;
    while (*p && *p != '\n' && *p != '\r' && *p != ' ' && *p != '\t' && vlen < val_size - 1) {
        val[vlen++] = *p++;
    }
    val[vlen] = '\0';
    return (klen > 0 && vlen > 0) ? 2 : (klen > 0 ? 1 : 0);
}

/* nc_strtok — minimal strtok replacement (not thread-safe, but mod is single-threaded) */
static char* nc_strtok_state = NULL;
static char* nc_strtok(char* str, const char* delim) {
    if (str) nc_strtok_state = str;
    if (!nc_strtok_state || !*nc_strtok_state) return NULL;
    /* Skip leading delimiters */
    char* p = nc_strtok_state;
    while (*p && nc_strchr(delim, *p)) p++;
    if (!*p) { nc_strtok_state = p; return NULL; }
    char* token_start = p;
    while (*p && !nc_strchr(delim, *p)) p++;
    if (*p) { *p = '\0'; p++; }
    nc_strtok_state = p;
    return token_start;
}

/* nc_strcpy — strcpy replacement */
static char* nc_strcpy(char* dst, const char* src) {
    char* d = dst;
    while ((*d++ = *src++));
    return dst;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Helpers — get_board via HB+ API (replaces get_board from bass_proxy.h)
 * ═══════════════════════════════════════════════════════════════════════════ */

static DWORD get_board(void) {
    if (!g_storedApi) return 0;
    HBPlusAPI hb = { g_storedApi };
    Scene* scene = hb.GetScene();
    if (!scene) return 0;
    /* Board is at Scene+0x8AC (level_ptr) — the game's ArenaBoard pointer */
    DWORD board = *(DWORD*)((char*)scene + 0x8AC);
    if (!board || board < 0x10000) return 0;
    if (IsBadReadPtr((void*)board, 0x100)) return 0;
    return board;
}

static DWORD cEnt_get_level(DWORD board) {
    if (!board) return 0;
    if (IsBadReadPtr((void*)(board + BOARD_LEVEL), 4)) return 0;
    return *(DWORD*)(board + BOARD_LEVEL);
}

static DWORD cEnt_get_sceneobj(DWORD board) {
    DWORD level = cEnt_get_level(board);
    if (!level) return 0;
    if (IsBadReadPtr((void*)(level + LEVEL_SCENEOBJECT), 4)) return 0;
    return *(DWORD*)(level + LEVEL_SCENEOBJECT);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * S1 GRID point scanning (preserved from original)
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cEnt_find_grid_points(DWORD board, float* out_x, float* out_y, float* out_z, int max_points, nc_FILE* logf) {
    DWORD sceneobj = cEnt_get_sceneobj(board);
    if (!sceneobj) {
        if (logf) nc_fprintf(logf, "  GRID: sceneobj=NULL\n");
        return 0;
    }

    DWORD s1_list = sceneobj + 0x894;
    if (IsBadReadPtr((void*)(s1_list + 0x04), 4)) {
        if (logf) nc_fprintf(logf, "  GRID: can't read S1 list count (sceneobj=0x%08X)\n", sceneobj);
        return 0;
    }
    int s1_count = *(int*)(s1_list + 0x04);
    if (logf) nc_fprintf(logf, "  GRID: sceneobj=0x%08X, S1 count=%d\n", sceneobj, s1_count);
    if (s1_count <= 0 || s1_count > 1000) return 0;

    if (IsBadReadPtr((void*)(s1_list + 0x40C), 4)) {
        if (logf) nc_fprintf(logf, "  GRID: can't read S1 data ptr\n");
        return 0;
    }
    DWORD* s1_data = *(DWORD**)(s1_list + 0x40C);
    if (!s1_data || IsBadReadPtr(s1_data, s1_count * 4)) {
        if (logf) nc_fprintf(logf, "  GRID: S1 data ptr invalid (0x%08X)\n", (DWORD)s1_data);
        return 0;
    }

    int found = 0;
    int i;
    for (i = 0; i < s1_count && found < max_points; i++) {
        DWORD entry = s1_data[i];
        if (!entry || entry < 0x10000) continue;
        if (IsBadReadPtr((void*)entry, 16)) continue;

        char* name = *(char**)(entry + S1ENTRY_NAME);
        if (name && !IsBadReadPtr(name, 5)) {
            if (logf && i < 10) nc_fprintf(logf, "  GRID: S1[%d] name='%s'\n", i, name);
            if (nc_strstr(name, "GRID") != NULL) {
                float x = *(float*)(entry + S1ENTRY_POS_X);
                float y = *(float*)(entry + S1ENTRY_POS_Y);
                float z = *(float*)(entry + S1ENTRY_POS_Z);
                out_x[found] = x;
                out_y[found] = y;
                out_z[found] = z;
                found++;
                if (logf) nc_fprintf(logf, "  GRID: found %s at (%.1f, %.1f, %.1f)\n", name, x, y, z);
            }
        } else {
            if (logf && i < 10) {
                nc_fprintf(logf, "  GRID: S1[%d] entry=0x%08X (bad nameptr=0x%08X)\n", i, entry, (DWORD)name);
            }
            char* inline_name = (char*)entry;
            if (nc_strstr(inline_name, "GRID") != NULL) {
                float x = *(float*)(entry + S1ENTRY_POS_X);
                float y = *(float*)(entry + S1ENTRY_POS_Y);
                float z = *(float*)(entry + S1ENTRY_POS_Z);
                out_x[found] = x;
                out_y[found] = y;
                out_z[found] = z;
                found++;
                if (logf) nc_fprintf(logf, "  GRID: found (inline) %s at (%.1f, %.1f, %.1f)\n", inline_name, x, y, z);
            }
        }
    }
    return found;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Tag processing (preserved from original)
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cEnt_ci_strstr(const char* haystack, const char* needle) {
    while (*haystack) {
        const char* h = haystack;
        const char* n = needle;
        while (*h && *n && nc_tolower((unsigned char)*h) == nc_tolower((unsigned char)*n)) { h++; n++; }
        if (*n == 0) return 1;
        haystack++;
    }
    return 0;
}

static int cEnt_extract_dat_prop(const char* name, const char* key, char* out_buf, int out_size) {
    const char* p = name;
    while (*p) {
        if (p[0] == '<' && nc_strnicmp(p + 1, "DAT", 3) == 0 && p[4] == '>') {
            const char* dat_start = p + 5;
            const char* dat_end = dat_start;
            while (*dat_end) {
                if (dat_end[0] == '<' && dat_end[1] == '/' &&
                    nc_strnicmp(dat_end + 2, "DAT", 3) == 0 && dat_end[5] == '>') {
                    break;
                }
                dat_end++;
            }
            
            int key_len = (int)nc_strlen(key);
            const char* q = dat_start;
            while (q < dat_end) {
                while (q < dat_end && (*q == ' ' || *q == '\t')) q++;
                if (q + key_len >= dat_end) break;
                
                if (nc_strnicmp(q, key, key_len) == 0 && q[key_len] == '=') {
                    const char* val_start = q + key_len + 1;
                    if (*val_start == '"') val_start++;
                    
                    const char* val_end = val_start;
                    while (val_end < dat_end && *val_end != ',' && *val_end != '"') val_end++;
                    
                    int len = (int)(val_end - val_start);
                    if (len > 0 && len < out_size) {
                        nc_memcpy(out_buf, val_start, len);
                        out_buf[len] = '\0';
                        while (len > 0 && (out_buf[len-1] == ' ' || out_buf[len-1] == '\t')) {
                            out_buf[--len] = '\0';
                        }
                        return 1;
                    }
                    return 0;
                }
                while (q < dat_end && *q != ',') q++;
                if (q < dat_end) q++;
            }
            return 0;
        }
        p++;
    }
    return 0;
}

static int cEnt_extract_tag(const char* name, const char* tag_name, char* out_buf, int out_size) {
    int tag_len = (int)nc_strlen(tag_name);
    const char* p = name;
    while (*p) {
        if (p[0] == '<' && nc_strnicmp(p + 1, tag_name, tag_len) == 0 && p[1 + tag_len] == '>') {
            const char* val_start = p + 1 + tag_len + 1;
            const char* q = val_start;
            while (*q) {
                if (q[0] == '<' && q[1] == '/' &&
                    nc_strnicmp(q + 2, tag_name, tag_len) == 0 && q[2 + tag_len] == '>') {
                    int len = (int)(q - val_start);
                    if (len > 0 && len < out_size) {
                        nc_memcpy(out_buf, val_start, len);
                        out_buf[len] = '\0';
                        return 1;
                    }
                    return 0;
                }
                q++;
            }
            {
                const char* end = val_start;
                while (*end && *end != '<') end++;
                int len = (int)(end - val_start);
                if (len > 0 && len < out_size) {
                    nc_memcpy(out_buf, val_start, len);
                    out_buf[len] = '\0';
                    return 1;
                }
            }
            return 0;
        }
        p++;
    }
    return 0;
}

static void cEnt_process_custom_tags(DWORD board, nc_FILE* logf) {
    if (!board) return;

    if (logf) nc_fprintf(logf, "  TAGS: cEnt_process_custom_tags called (board=0x%08X)\n", board);

    DWORD* bad_balls_list = (DWORD*)(board + BOARD_BAD_BALLS_LIST);
    if (IsBadReadPtr(bad_balls_list, 8)) {
        if (logf) nc_fprintf(logf, "  TAGS: bad_balls_list bad read\n");
        return;
    }
    int ball_count = *(int*)(bad_balls_list + 1);
    if (logf) nc_fprintf(logf, "  TAGS: ball_count=%d\n", ball_count);
    if (ball_count <= 0 || ball_count > 100) return;

    DWORD* ball_items = *(DWORD**)((BYTE*)bad_balls_list + 0x40C);
    if (!ball_items || IsBadReadPtr(ball_items, ball_count * 4)) return;

    DWORD sceneobj = cEnt_get_sceneobj(board);
    if (!sceneobj) {
        if (logf) nc_fprintf(logf, "  TAGS: sceneobj=NULL\n");
        return;
    }

    if (IsBadReadPtr((void*)(sceneobj + SCENEOBJ_OBJ_COUNT), 4)) return;
    int obj_count = *(int*)(sceneobj + SCENEOBJ_OBJ_COUNT);
    if (obj_count <= 0 || obj_count > 1000) return;

    if (IsBadReadPtr((void*)(sceneobj + SCENEOBJ_OBJ_ARRAY), 4)) return;
    DWORD* obj_array_ptr = *(DWORD**)(sceneobj + SCENEOBJ_OBJ_ARRAY);
    if (!obj_array_ptr || IsBadReadPtr(obj_array_ptr, obj_count * 4)) return;

    int mesh_changes = 0;
    int speed_changes = 0;
    int i, j;

    for (i = 0; i < obj_count; i++) {
        DWORD obj_ptr = obj_array_ptr[i];
        if (!obj_ptr || obj_ptr < 0x10000) continue;
        if (IsBadReadPtr((void*)obj_ptr, 20)) continue;

        char* name = *(char**)(obj_ptr);
        if (!name || IsBadReadPtr(name, 8)) continue;

        if (nc_strnicmp(name, "BADBALL", 7) != 0) continue;

        char mesh_value[64] = {0};
        int has_mesh = 0;

        char speed_value[64] = {0};
        int has_speed = cEnt_extract_tag(name, "SPEEDMULT", speed_value, sizeof(speed_value));

        if (!has_mesh && !has_speed) continue;

        float obj_x = *(float*)(obj_ptr + 0x04);
        float obj_y = *(float*)(obj_ptr + 0x08);
        float obj_z = *(float*)(obj_ptr + 0x0C);

        if (logf) {
            nc_fprintf(logf, "  TAGS: BADBALL obj[%d] at (%.1f, %.1f, %.1f)", i, obj_x, obj_y, obj_z);
            if (has_speed) nc_fprintf(logf, " <SPEEDMULT>%s</SPEEDMULT>", speed_value);
            nc_fprintf(logf, "\n");
        }

        for (j = 0; j < ball_count; j++) {
            DWORD ball = ball_items[j];
            if (!ball || ball < 0x10000) continue;
            if (IsBadReadPtr((void*)ball, 0xC70)) continue;

            float ball_home_x = *(float*)(ball + BALL_HOME_POS_X);
            float ball_home_y = *(float*)(ball + BALL_HOME_POS_Y);
            float ball_home_z = *(float*)(ball + BALL_HOME_POS_Z);

            if (ball_home_x == obj_x && ball_home_y == obj_y && ball_home_z == obj_z) {
                if (has_speed) {
                    float mult = nc_atof(speed_value);
                    if (mult > 0.0f && mult <= 100.0f) {
                        float cur_speed = *(float*)(ball + BALL_MAX_SPEED);
                        *(float*)(ball + BALL_MAX_SPEED) = cur_speed * mult;
                        if (logf) nc_fprintf(logf, "  TAGS: ball 0x%08X max_speed %.1f x %.1f = %.1f\n",
                                ball, cur_speed, mult, cur_speed * mult);
                        speed_changes++;
                    }
                }
                break;
            }
        }
    }

    if (logf && (mesh_changes > 0 || speed_changes > 0)) {
        nc_fprintf(logf, "  TAGS: applied %d mesh, %d speedmult\n", mesh_changes, speed_changes);
        nc_fflush(logf);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * CEA spawning pattern (preserved from original)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void cEnt_spawn_testcube_at(DWORD board, float px, float py, float pz, int grid_num, nc_FILE* logf) {
    if (!board) return;

    DWORD app = *(DWORD*)(board + BOARD_APP);
    if (!app || IsBadReadPtr((void*)app, 4)) {
        if (logf) nc_fprintf(logf, "  GRID: app=NULL\n");
        return;
    }
    DWORD gfx_device = *(DWORD*)(app + APP_GFX_DEVICE);
    if (!gfx_device || IsBadReadPtr((void*)gfx_device, 4)) {
        if (logf) nc_fprintf(logf, "  GRID: gfx_device=NULL\n");
        return;
    }

    void* mesh = pfn_operator_new(MESHWORLD_SIZE);
    if (!mesh) {
        if (logf) nc_fprintf(logf, "  GRID: failed to allocate mesh\n");
        return;
    }
    nc_memset(mesh, 0, MESHWORLD_SIZE);

    void* loaded_mesh = pfn_MeshWorld_ctor(mesh, (void*)gfx_device, g_mesh_path);
    if (!loaded_mesh) {
        if (logf) nc_fprintf(logf, "  GRID: MeshWorld_ctor failed for '%s'\n", g_mesh_path);
        return;
    }

    void* obj = pfn_operator_new(POPCYLINDER_SIZE);
    if (!obj) {
        if (logf) nc_fprintf(logf, "  GRID: failed to allocate PopCylinder\n");
        return;
    }
    nc_memset(obj, 0, POPCYLINDER_SIZE);

    void* result = pfn_PopCylinder_ctor(obj, (void*)board, px, py, pz, mesh);
    if (!result) {
        if (logf) nc_fprintf(logf, "  GRID: PopCylinder_ctor failed\n");
        return;
    }

    pfn_AthenaList_Append((DWORD*)(board + BOARD_UPDATE_LIST), (int)obj);
    pfn_AthenaList_Append((DWORD*)(board + BOARD_RENDER_LIST), (int)obj);

    DWORD col_obj = *(DWORD*)((char*)obj + PC_COLLISION_OBJ);
    if (col_obj) {
        pfn_AthenaList_Append((DWORD*)(board + BOARD_COLLISION_LIST), (int)col_obj);
        DWORD scene_col = *(DWORD*)(board + BOARD_SCENE_OBJ);
        if (scene_col) {
            pfn_AthenaList_Append((DWORD*)(scene_col + 0x18), (int)col_obj);
        }
    }

    DWORD level = cEnt_get_level(board);
    if (level) {
        DWORD sceneobj = *(DWORD*)(level + LEVEL_SCENEOBJECT);
        if (sceneobj) {
            pfn_AthenaList_Append((DWORD*)(sceneobj + 0x1C), (int)obj);
        }
    }

    if (logf) {
        nc_fprintf(logf, "  GRID: spawned testcube(GRID%02d) at (%.1f,%.1f,%.1f) obj=0x%08X\n",
                grid_num, px, py, pz, (DWORD)obj);
        nc_fflush(logf);
    }

    if (g_spawned_count < MAX_SPAWNED) {
        g_spawned_objs[g_spawned_count] = (DWORD)obj;
        nc_snprintf(g_spawned_names[g_spawned_count], 32, "testcube(GRID%02d)", grid_num);
        g_spawned_count++;
    }
}

static void cEnt_despawn_object(DWORD board, DWORD obj, nc_FILE* logf) {
    if (!board || !obj) return;
    if (IsBadReadPtr((void*)obj, 0x10D0)) return;

    if (logf) nc_fprintf(logf, "  DESPAWN: removing obj=0x%08X\n", obj);

    *(BYTE*)((char*)obj + 0x10E5) = 1;
    *(BYTE*)((char*)obj + 0x10E4) = 1;

    DWORD col_obj = *(DWORD*)((char*)obj + PC_COLLISION_OBJ);
    if (col_obj) {
        pfn_AthenaList_Remove((DWORD*)(board + BOARD_COLLISION_LIST), (int)col_obj);
        DWORD scene_col = *(DWORD*)(board + BOARD_SCENE_OBJ);
        if (scene_col) {
            pfn_AthenaList_Remove((DWORD*)(scene_col + 0x18), (int)col_obj);
        }
        if (!IsBadReadPtr((void*)col_obj, 4)) {
            DWORD col_vtable = *(DWORD*)col_obj;
            if (col_vtable && !IsBadReadPtr((void*)col_vtable, 4)) {
                DWORD dtor = *(DWORD*)col_vtable;
                if (dtor && dtor > 0x400000) {
                    typedef void (__thiscall *dtor_t)(void* this_, int free_mem);
                    ((dtor_t)dtor)((void*)col_obj, 1);
                }
            }
        }
        *(DWORD*)((char*)obj + PC_COLLISION_OBJ) = 0;
    }

    pfn_AthenaList_Remove((DWORD*)(board + BOARD_UPDATE_LIST), (int)obj);
    pfn_AthenaList_Remove((DWORD*)(board + BOARD_SCENE_UPDATE_LIST), (int)obj);
    pfn_AthenaList_Remove((DWORD*)(board + BOARD_RENDER_LIST), (int)obj);

    DWORD level = cEnt_get_level(board);
    if (level) {
        DWORD sceneobj = *(DWORD*)(level + LEVEL_SCENEOBJECT);
        if (sceneobj) {
            pfn_AthenaList_Remove((DWORD*)(sceneobj + 0x1C), (int)obj);
        }
    }

    if (logf) {
        nc_fprintf(logf, "  DESPAWN: obj=0x%08X removed\n", obj);
        nc_fflush(logf);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Mesh file loading (preserved from original)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void* cEnt_load_mesh_file(DWORD gfx_device, const char* path, int* out_is_mesh_node, nc_FILE* logf) {
    if (!path || !path[0] || !gfx_device) return NULL;
    *out_is_mesh_node = 0;

    int is_mesh_file = (nc_strnicmp(path, "meshes\\", 6) == 0);

    if (is_mesh_file) {
        char check_path[256];
        nc_snprintf(check_path, 255, "%s.mesh", path);
        check_path[255] = 0;

        if (pfn_check_file_access(check_path, 0) != 0) {
            nc_snprintf(check_path, 255, "%s.meshworld", path);
            check_path[255] = 0;
            if (pfn_check_file_access(check_path, 0) != 0) {
                if (logf) nc_fprintf(logf, "  LOAD_MESH: neither .mesh nor .meshworld for '%s'\n", path);
                return NULL;
            }
            is_mesh_file = 0;
        }
    } else {
        char check_path[256];
        nc_snprintf(check_path, 255, "%s.meshworld", path);
        check_path[255] = 0;
        if (pfn_check_file_access(check_path, 0) != 0) {
            if (logf) nc_fprintf(logf, "  LOAD_MESH: '%s' not found\n", check_path);
            return NULL;
        }
    }

    if (is_mesh_file) {
        void* mesh_node = pfn_operator_new(MESHNODE_SIZE);
        if (!mesh_node) {
            if (logf) nc_fprintf(logf, "  LOAD_MESH: failed to alloc MeshNode\n");
            return NULL;
        }
        nc_memset(mesh_node, 0, MESHNODE_SIZE);

        void* result = pfn_MeshNode_ctor(mesh_node, (void*)gfx_device, path);
        if (!result) {
            if (logf) nc_fprintf(logf, "  LOAD_MESH: MeshNode_ctor failed for '%s'\n", path);
            return NULL;
        }

        BYTE has_mesh = *(BYTE*)((char*)mesh_node + 0x0D);
        if (!has_mesh) {
            if (logf) nc_fprintf(logf, "  LOAD_MESH: MeshNode has_mesh=0 for '%s'\n", path);
            return NULL;
        }

        void* mesh_world = *(void**)((char*)mesh_node + 0x08);
        if (!mesh_world || IsBadReadPtr(mesh_world, 0x100)) {
            if (logf) nc_fprintf(logf, "  LOAD_MESH: MeshWorld* invalid for '%s'\n", path);
            return NULL;
        }

        *out_is_mesh_node = 1;
        if (logf) nc_fprintf(logf, "  LOAD_MESH: .MESH OK: '%s'\n", path);
        return mesh_node;
    } else {
        void* mesh = pfn_operator_new(MESHWORLD_SIZE);
        if (!mesh) {
            if (logf) nc_fprintf(logf, "  LOAD_MESH: failed to alloc mesh\n");
            return NULL;
        }
        nc_memset(mesh, 0, MESHWORLD_SIZE);

        char path_buf[256];
        nc_strncpy(path_buf, path, 255);
        path_buf[255] = 0;

        void* loaded = pfn_MeshWorld_ctor(mesh, (void*)gfx_device, path_buf);
        if (!loaded) {
            if (logf) nc_fprintf(logf, "  LOAD_MESH: MeshWorld_ctor failed for '%s'\n", path_buf);
            return NULL;
        }

        return mesh;
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Entity spawning (preserved from original — the big spawn function)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void cEnt_spawn_rotater_at(DWORD board, float px, float py, float pz,
                              const char* mesh_path,
                              float rot_x, float rot_y, float rot_z,
                              float ros_x, float ros_y, float ros_z,
                              int ai_type,
                              nc_FILE* logf) {
    if (!board) return;

    DWORD app = *(DWORD*)(board + BOARD_APP);
    if (!app || IsBadReadPtr((void*)app, 4)) return;
    DWORD gfx_device = *(DWORD*)(app + APP_GFX_DEVICE);
    if (!gfx_device || IsBadReadPtr((void*)gfx_device, 4)) return;

    const char* path = NULL;
    if (mesh_path && mesh_path[0]) {
        path = mesh_path;
    } else if (ai_type >= 1 && ai_type <= 5) {
        path = g_ai_mesh_paths[ai_type];
    } else if (ai_type == 12 || ai_type == 13 || ai_type == 14 ||
               ai_type == 15 || ai_type == 16 || ai_type == 22 || ai_type == 28) {
        path = NULL;
    } else if (ai_type >= 30 && ai_type <= 33) {
        path = NULL;
    } else {
        path = g_swirl_mesh_path;
    }

    if (path && nc_stricmp(path, "levels\\_default") == 0) {
        /* Keep as-is — game's MeshWorld_ctor will try to load it */
    }

    int is_mesh_node = 0;
    void* mesh = NULL;

    if (path) {
        mesh = cEnt_load_mesh_file(gfx_device, path, &is_mesh_node, logf);
        if (!mesh) {
            if (logf) nc_fprintf(logf, "  ROTATER: mesh load failed for '%s', trying Swirl\n", path);
            is_mesh_node = 0;
            mesh = cEnt_load_mesh_file(gfx_device, g_swirl_mesh_path, &is_mesh_node, logf);
            if (!mesh) {
                if (logf) nc_fprintf(logf, "  ROTATER: Swirl fallback failed\n");
                return;
            }
        }
    }

    /* .MESH file path — swap MeshWorld after PopCylinder construction */
    if (is_mesh_node) {
        DWORD mesh_world_from_node = *(DWORD*)((char*)mesh + 0x08);
        if (!mesh_world_from_node || IsBadReadPtr((void*)mesh_world_from_node, 0x100)) {
            if (logf) nc_fprintf(logf, "  ROTATER: .MESH MeshWorld* invalid\n");
            return;
        }

        int swirl_is_node = 0;
        void* swirl_mesh = cEnt_load_mesh_file(gfx_device, g_swirl_mesh_path, &swirl_is_node, logf);
        if (!swirl_mesh || swirl_is_node) {
            if (logf) nc_fprintf(logf, "  ROTATER: Swirl placeholder failed\n");
            return;
        }

        void* obj = pfn_operator_new(POPCYLINDER_SIZE);
        if (!obj) return;
        nc_memset(obj, 0, POPCYLINDER_SIZE);
        void* result0 = pfn_PopCylinder_ctor(obj, (void*)board, px, py, pz, swirl_mesh);
        if (!result0) return;

        *(DWORD*)((char*)obj + 0x08) = mesh_world_from_node;

        if (logf) {
            nc_fprintf(logf, "  ROTATER: spawned (.MESH swap) at (%.1f,%.1f,%.1f) obj=0x%08X mesh='%s'\n",
                    px, py, pz, (DWORD)obj, path);
            nc_fflush(logf);
        }

        pfn_AthenaList_Append((DWORD*)(board + BOARD_UPDATE_LIST), (int)obj);
        pfn_AthenaList_Append((DWORD*)(board + BOARD_RENDER_LIST), (int)obj);

        DWORD col_obj = *(DWORD*)((char*)obj + 0x10E0);
        if (col_obj) {
            pfn_AthenaList_Append((DWORD*)(board + BOARD_COLLISION_LIST), (int)col_obj);
            DWORD scene_col = *(DWORD*)(board + BOARD_SCENE_OBJ);
            if (scene_col) {
                pfn_AthenaList_Append((DWORD*)(scene_col + 0x18), (int)col_obj);
            }
        }

        DWORD level = cEnt_get_level(board);
        if (level) {
            DWORD sceneobj = *(DWORD*)(level + LEVEL_SCENEOBJECT);
            if (sceneobj) {
                pfn_AthenaList_Append((DWORD*)(sceneobj + 0x1C), (int)obj);
            }
        }

        if (g_rotater_count < MAX_ROTATERS) {
            g_rotater_cfg[g_rotater_count].obj = (DWORD)obj;
            g_rotater_cfg[g_rotater_count].rot_x = 0.0f;
            g_rotater_cfg[g_rotater_count].rot_y = 0.0f;
            g_rotater_cfg[g_rotater_count].rot_z = 0.0f;
            g_rotater_cfg[g_rotater_count].ros_x = 0.0f;
            g_rotater_cfg[g_rotater_count].ros_y = 0.0f;
            g_rotater_cfg[g_rotater_count].ros_z = 0.0f;
            g_rotater_cfg[g_rotater_count].angle_x = 0.0f;
            g_rotater_cfg[g_rotater_count].angle_y = 0.0f;
            g_rotater_cfg[g_rotater_count].angle_z = 0.0f;
            if (mesh_path && mesh_path[0]) {
                nc_strncpy(g_rotater_cfg[g_rotater_count].mesh_path, mesh_path, 127);
                g_rotater_cfg[g_rotater_count].mesh_path[127] = 0;
            } else {
                g_rotater_cfg[g_rotater_count].mesh_path[0] = 0;
            }
            g_rotater_count++;
        }
        return;
    }

    /* Allocate object and call constructor based on AI type */
    void* obj = NULL;
    if (ai_type == 6) {
        obj = pfn_operator_new(ROTATER_SIZE);
        if (!obj) return;
        nc_memset(obj, 0, ROTATER_SIZE);
        void* result = pfn_Rotator_ctor(obj, (void*)board, px, py, pz, mesh);
        if (!result) return;
    } else if ((ai_type >= 7 && ai_type <= 13) || (ai_type >= 17 && ai_type <= 22) || (ai_type >= 27 && ai_type <= 33)) {
        switch (ai_type) {
            case 7:
                obj = pfn_operator_new(ARENASTANDS_SIZE); if (!obj) return;
                nc_memset(obj, 0, ARENASTANDS_SIZE);
                cEnt_DFloor1_ctor(obj, (void*)board, px, py, pz, mesh);
                break;
            case 17:
                obj = pfn_operator_new(ARENASTANDS_SIZE); if (!obj) return;
                nc_memset(obj, 0, ARENASTANDS_SIZE);
                cEnt_DFloor2_ctor(obj, (void*)board, px, py, pz, mesh);
                break;
            case 18:
                obj = pfn_operator_new(ARENASTANDS_SIZE); if (!obj) return;
                nc_memset(obj, 0, ARENASTANDS_SIZE);
                cEnt_DFloor3_ctor(obj, (void*)board, px, py, pz, mesh);
                break;
            case 19:
                obj = pfn_operator_new(ARENASTANDS_SIZE); if (!obj) return;
                nc_memset(obj, 0, ARENASTANDS_SIZE);
                cEnt_DFloor4_ctor(obj, (void*)board, px, py, pz, mesh);
                break;
            case 20:
                obj = pfn_operator_new(ARENASTANDS_SIZE); if (!obj) return;
                nc_memset(obj, 0, ARENASTANDS_SIZE);
                cEnt_FlickRing_ctor(obj, (void*)board, px, py, pz, mesh);
                break;
            case 21:
                obj = pfn_operator_new(ARENASTANDS_SIZE); if (!obj) return;
                nc_memset(obj, 0, ARENASTANDS_SIZE);
                cEnt_Trode_ctor(obj, (void*)board, px, py, pz, mesh);
                break;
            case 22: {
                DWORD app2 = *(DWORD*)(board + BOARD_APP);
                DWORD gfx2 = app2 ? *(DWORD*)(app2 + APP_GFX_DEVICE) : 0;
                if (!gfx2) return;
                obj = pfn_operator_new(MESHNODE_SIZE); if (!obj) return;
                nc_memset(obj, 0, MESHNODE_SIZE);
                pfn_MeshNode_ctor(obj, (void*)gfx2, "Meshes\\Chomper");
                break;
            }
            case 27:
                obj = pfn_operator_new(SPINNER_LEVEL_SIZE); if (!obj) return;
                nc_memset(obj, 0, SPINNER_LEVEL_SIZE);
                pfn_Spinner_Level_ctor(obj, (void*)board, px, py, pz, mesh, 0.0f);
                break;
            case 28: {
                DWORD app2 = *(DWORD*)(board + BOARD_APP);
                DWORD gfx2 = app2 ? *(DWORD*)(app2 + APP_GFX_DEVICE) : 0;
                if (!gfx2) return;
                obj = pfn_operator_new(SPRITE_SIZE); if (!obj) return;
                nc_memset(obj, 0, SPRITE_SIZE);
                const char* cloud_path = mesh_path && mesh_path[0] ? mesh_path : "levels\\Cloudscape";
                char check_path[256];
                nc_snprintf(check_path, 255, "%s.meshworld", cloud_path);
                if (pfn_check_file_access(check_path, 0) != 0) {
                    cloud_path = "levels\\_default";
                }
                pfn_Sprite_ctor(obj, (void*)gfx2, cloud_path);
                break;
            }
            case 29:
                obj = pfn_operator_new(GEAR_REAL_SIZE); if (!obj) return;
                nc_memset(obj, 0, GEAR_REAL_SIZE);
                pfn_Gear_ctor_real(obj, (void*)board, px, py, pz, px, py, pz, mesh);
                break;
            case 30:
                obj = pfn_operator_new(BELL_SIZE); if (!obj) return;
                nc_memset(obj, 0, BELL_SIZE);
                pfn_Bell_ctor(obj, (void*)board, px, py, pz);
                break;
            case 31:
                obj = pfn_operator_new(FAN_SIZE); if (!obj) return;
                nc_memset(obj, 0, FAN_SIZE);
                pfn_Fan_ctor(obj, (void*)board, px, py, pz, 0.0f);
                break;
            case 32:
                obj = pfn_operator_new(SAWBLADE_SIZE); if (!obj) return;
                nc_memset(obj, 0, SAWBLADE_SIZE);
                pfn_SawBlade_ctor(obj, (void*)board, px, py, pz);
                break;
            case 33:
                obj = pfn_operator_new(BONK_SIZE); if (!obj) return;
                nc_memset(obj, 0, BONK_SIZE);
                pfn_Bonk_ctor(obj, (void*)board, px, py, pz);
                break;
            case 34:
                obj = pfn_operator_new(POPCYLINDER_SIZE); if (!obj) return;
                nc_memset(obj, 0, POPCYLINDER_SIZE);
                pfn_BreakBridge_ctor(obj, (void*)board, px, py, pz, mesh);
                break;
            case 8:
                obj = pfn_operator_new(GAMELEVEL_SIZE); if (!obj) return;
                nc_memset(obj, 0, GAMELEVEL_SIZE);
                pfn_GameLevel_ctor(obj, (void*)board, px, py, pz, mesh);
                break;
            case 9:
                obj = pfn_operator_new(GLASS_LEVEL_SIZE); if (!obj) return;
                nc_memset(obj, 0, GLASS_LEVEL_SIZE);
                pfn_Glass_Level_ctor(obj, (void*)board, mesh);
                *(float*)((char*)obj + 0x10D8) = px;
                *(float*)((char*)obj + 0x10DC) = py;
                *(float*)((char*)obj + 0x10E0) = pz;
                break;
            case 10:
                obj = pfn_operator_new(GEAR_LEVEL_SIZE); if (!obj) return;
                nc_memset(obj, 0, GEAR_LEVEL_SIZE);
                pfn_Gear_Level_ctor(obj, (void*)board, px, py, pz);
                break;
            case 11:
                obj = pfn_operator_new(SECRET_SIZE); if (!obj) return;
                nc_memset(obj, 0, SECRET_SIZE);
                pfn_Secret_ctor(obj, (void*)board, px, py, pz, mesh);
                break;
            case 12:
                obj = pfn_operator_new(FLAGWAVER_SIZE); if (!obj) return;
                nc_memset(obj, 0, FLAGWAVER_SIZE);
                {
                    DWORD app2 = *(DWORD*)(board + BOARD_APP);
                    DWORD gfx2 = app2 ? *(DWORD*)(app2 + APP_GFX_DEVICE) : 0;
                    if (!gfx2) return;
                    pfn_FlagWaver_Ctor(obj, (void*)gfx2);
                }
                break;
            case 13:
                obj = pfn_operator_new(SIGN_SIZE); if (!obj) return;
                nc_memset(obj, 0, SIGN_SIZE);
                {
                    DWORD app2 = *(DWORD*)(board + BOARD_APP);
                    int gfx1 = app2 ? *(int*)(app2 + 0x58C) : 0;
                    int gfx2 = app2 ? *(int*)(app2 + 0x590) : 0;
                    pfn_Sign_ctor(obj, (void*)board, gfx1, gfx2,
                        *(int*)&px, *(int*)&py, *(int*)&pz, 0, 0, 0);
                }
                break;
            case 14:
                obj = pfn_operator_new(WAVY_SIZE); if (!obj) return;
                nc_memset(obj, 0, WAVY_SIZE);
                {
                    const char* wavy_path = "levels\\Flag";
                    char check_path[256];
                    nc_snprintf(check_path, 255, "%s.meshworld", wavy_path);
                    if (pfn_check_file_access(check_path, 0) != 0) {
                        wavy_path = "levels\\_default";
                    }
                    pfn_Wavy_ctor(obj, (void*)board, px, py, pz, wavy_path);
                    pfn_Wavy_Configure(obj, 0x1c, 20.0f, 2.0f, -3.0f);
                }
                break;
            case 15:
                obj = pfn_operator_new(BADBALL_SIZE); if (!obj) return;
                nc_memset(obj, 0, BADBALL_SIZE);
                pfn_BadBall_ctor(obj, (void*)board);
                {
                    DWORD* vtable = *(DWORD**)obj;
                    if (vtable && vtable[1]) {
                        typedef void (__thiscall *vtable1_t)(void* this_);
                        ((vtable1_t)vtable[1])(obj);
                    }
                }
                {
                    DWORD app2 = *(DWORD*)(board + BOARD_APP);
                    DWORD gfx2 = app2 ? *(DWORD*)(app2 + APP_GFX_DEVICE) : 0;
                    if (gfx2) {
                        void* mesh8 = pfn_MeshNode_ctor(pfn_operator_new(MESHNODE_SIZE),
                                                        (void*)gfx2, "meshes\\8ball");
                        if (mesh8) {
                            *(DWORD*)((char*)obj + 0x10) = (DWORD)mesh8;
                        }
                    }
                }
                *(float*)((char*)obj + BALL_HOME_POS_X) = px;
                *(float*)((char*)obj + BALL_HOME_POS_Y) = py;
                *(float*)((char*)obj + BALL_HOME_POS_Z) = pz;
                break;
            case 16: {
                DWORD app2 = *(DWORD*)(board + BOARD_APP);
                if (!app2 || IsBadReadPtr((void*)app2, 4)) return;
                DWORD gfx2 = *(DWORD*)(app2 + APP_GFX_DEVICE);
                if (!gfx2) return;

                const char* bridge_path = mesh_path && mesh_path[0] ? mesh_path : "levels\\Level2-Bridge";
                void* bmesh = cEnt_load_mesh_file(gfx2, bridge_path, &is_mesh_node, logf);
                if (!bmesh) {
                    bmesh = cEnt_load_mesh_file(gfx2, g_swirl_mesh_path, &is_mesh_node, logf);
                    if (!bmesh) return;
                }

                void* render_obj = pfn_operator_new(BRIDGESLAM_SIZE);
                if (render_obj) {
                    nc_memset(render_obj, 0, BRIDGESLAM_SIZE);
                    render_obj = pfn_Level_RenderCtor(render_obj, bmesh);
                }
                if (render_obj) {
                    pfn_TipperVisual_Attach(render_obj, (int)bmesh);
                }

                obj = pfn_operator_new(BRIDGESLAM_SIZE);
                if (!obj) return;
                nc_memset(obj, 0, BRIDGESLAM_SIZE);

                pfn_AthenaList_Append((DWORD*)(board + BOARD_UPDATE_LIST), (int)obj);
                pfn_AthenaList_Append((DWORD*)(board + BOARD_RENDER_LIST), (int)render_obj);

                if (g_bridgeslam_count < MAX_BRIDGESLAMS) {
                    BridgeslamState* bs = &g_bridgeslams[g_bridgeslam_count];
                    bs->obj = (DWORD)obj;
                    bs->render_obj = (DWORD)render_obj;
                    bs->mesh_world = (DWORD)bmesh;
                    bs->board = board;
                    bs->pivot_x = px;
                    bs->pivot_y = py;
                    bs->pivot_z = pz;
                    bs->angle = 45.0f;
                    bs->state = 0;
                    bs->counter = 50;
                    bs->active = 1;
                    g_bridgeslam_count++;
                }
                break;
            }
        }
    } else {
        Rotator_ctor_t ctor_fn = NULL;
        DWORD alloc_sz = 0;
        
        switch (ai_type) {
            case 1:  ctor_fn = pfn_Rotator_ctor;  alloc_sz = ROTATER_SIZE;   break;
            case 2:  ctor_fn = pfn_Pendulum_ctor; alloc_sz = PENDULUM_SIZE;  break;
            case 3:  ctor_fn = pfn_Looper_ctor;   alloc_sz = LOOPER_SIZE;    break;
            case 4:  ctor_fn = pfn_Gear_ctor;      alloc_sz = GEAR_SIZE;      break;
            case 5:  ctor_fn = pfn_Gear_ctor;      alloc_sz = GEAR_SIZE;      break;
            case 35: ctor_fn = (Rotator_ctor_t)pfn_Catapult_ctor;      alloc_sz = CATAPULT_SIZE;      break;
            case 36: ctor_fn = (Rotator_ctor_t)pfn_Mace_ctor;          alloc_sz = MACE_SIZE;          break;
            case 37: ctor_fn = (Rotator_ctor_t)pfn_Tipper_ctor;        alloc_sz = TIPPER_SIZE;        break;
            case 38: ctor_fn = (Rotator_ctor_t)pfn_Lifter_ctor;       alloc_sz = LIFTER_SIZE;        break;
            case 39: ctor_fn = (Rotator_ctor_t)pfn_SpeedCylinder_ctor; alloc_sz = SPEEDCYLINDER_SIZE; break;
            case 40: ctor_fn = (Rotator_ctor_t)pfn_NeonPlatform_ctor;  alloc_sz = NEONPLATFORM_SIZE;  break;
            case 41: ctor_fn = (Rotator_ctor_t)pfn_Trapdoor_ctor;     alloc_sz = TRAPDOOR_SIZE;      break;
            case 42: ctor_fn = (Rotator_ctor_t)pfn_Odd_Lifter_ctor;   alloc_sz = ODD_LIFTER_SIZE;    break;
            default:
                obj = pfn_operator_new(POPCYLINDER_SIZE);
                if (!obj) return;
                nc_memset(obj, 0, POPCYLINDER_SIZE);
                void* result0 = pfn_PopCylinder_ctor(obj, (void*)board, px, py, pz, mesh);
                if (!result0) return;
                goto spawn_done;
        }
        
        obj = pfn_operator_new(alloc_sz);
        if (!obj) return;
        nc_memset(obj, 0, alloc_sz);
        void* result = ctor_fn(obj, (void*)board, px, py, pz, mesh);
        if (!result) return;
    }
    spawn_done:;

    /* Add to board update list */
    pfn_AthenaList_Append((DWORD*)(board + BOARD_UPDATE_LIST), (int)obj);

    /* Type 8 (GameLevel/Wobbly): also add to Scene_Update list */
    if (ai_type == 8 || ai_type == 34) {
        pfn_AthenaList_Append((DWORD*)(board + BOARD_SCENE_UPDATE_LIST), (int)obj);
    }

    /* BadBall goes into the bad balls list */
    if (ai_type == 15) {
        pfn_AthenaList_Append((DWORD*)(board + BOARD_BAD_BALLS_LIST), (int)obj);
    }

    /* Add to render list */
    pfn_AthenaList_Append((DWORD*)(board + BOARD_RENDER_LIST), (int)obj);

    /* Add collision object */
    {
        DWORD col_off = 0x10E0;
        if (ai_type >= 1 && ai_type <= 6) col_off = 0x10D4;
        else if (ai_type == 7 || (ai_type >= 17 && ai_type <= 21)) col_off = 0x10E0;
        else if (ai_type == 8) col_off = 0x10D4;
        else if (ai_type == 9) col_off = 0x10D4;
        else if (ai_type == 10) col_off = 0x10D4;
        else if (ai_type == 11) col_off = 0x10E0;
        else if (ai_type == 12) col_off = 0;
        else if (ai_type == 13) col_off = 0x10EC;
        else if (ai_type == 14) col_off = 0x10D4;
        else if (ai_type == 15) col_off = 0;
        else if (ai_type == 16) col_off = 0;
        else if (ai_type == 22) col_off = 0;
        else if (ai_type == 27) col_off = 0x10D4;
        else if (ai_type == 28) col_off = 0;
        else if (ai_type == 29) col_off = 0x10D4;
        else if (ai_type == 30) col_off = 0x10D4;
        else if (ai_type == 31) col_off = 0x10D4;
        else if (ai_type == 32) col_off = 0x10D4;
        else if (ai_type == 33) col_off = 0x10D4;
        else if (ai_type == 34) col_off = 0x10E0;
        else if (ai_type >= 35 && ai_type <= 42) col_off = 0x10E0;
        DWORD col_obj = *(DWORD*)((char*)obj + col_off);
        if (col_obj && col_off > 0) {
            pfn_AthenaList_Append((DWORD*)(board + BOARD_COLLISION_LIST), (int)col_obj);
            DWORD scene_col = *(DWORD*)(board + BOARD_SCENE_OBJ);
            if (scene_col) {
                pfn_AthenaList_Append((DWORD*)(scene_col + 0x18), (int)col_obj);
            }
        }
    }

    /* Add to scene spatial tree */
    DWORD level = cEnt_get_level(board);
    if (level) {
        DWORD sceneobj = *(DWORD*)(level + LEVEL_SCENEOBJECT);
        if (sceneobj) {
            pfn_AthenaList_Append((DWORD*)(sceneobj + 0x1C), (int)obj);
        }
    }

    if (logf) {
        nc_fprintf(logf, "  ROTATER: spawned at (%.1f,%.1f,%.1f) obj=0x%08X mesh='%s' rot=(%.4f,%.4f,%.4f)\n",
                px, py, pz, (DWORD)obj, path ? path : "(null)", rot_x, rot_y, rot_z);
        nc_fflush(logf);
    }

    if (g_rotater_count < MAX_ROTATERS) {
        g_rotater_cfg[g_rotater_count].obj = (DWORD)obj;
        g_rotater_cfg[g_rotater_count].rot_x = rot_x;
        g_rotater_cfg[g_rotater_count].rot_y = rot_y;
        g_rotater_cfg[g_rotater_count].rot_z = rot_z;
        g_rotater_cfg[g_rotater_count].ros_x = ros_x;
        g_rotater_cfg[g_rotater_count].ros_y = ros_y;
        g_rotater_cfg[g_rotater_count].ros_z = ros_z;
        g_rotater_cfg[g_rotater_count].angle_x = 0.0f;
        g_rotater_cfg[g_rotater_count].angle_y = 0.0f;
        g_rotater_cfg[g_rotater_count].angle_z = 0.0f;
        if (mesh_path && mesh_path[0]) {
            nc_strncpy(g_rotater_cfg[g_rotater_count].mesh_path, mesh_path, 127);
            g_rotater_cfg[g_rotater_count].mesh_path[127] = 0;
        } else {
            g_rotater_cfg[g_rotater_count].mesh_path[0] = 0;
        }
        g_rotater_count++;
    }
}

static void cEnt_despawn_all_rotaters(DWORD board, nc_FILE* logf) {
    int i;
    for (i = 0; i < g_rotater_count; i++) {
        DWORD obj = g_rotater_cfg[i].obj;
        if (!obj || obj < 0x10000) continue;
        if (IsBadReadPtr((void*)obj, 0x10D0)) continue;

        DWORD vtable = *(DWORD*)obj;
        if (vtable && !IsBadReadPtr((void*)vtable, 0x30)) {
            DWORD remove_fn = *(DWORD*)(vtable + 0x2C);
            if (remove_fn && remove_fn > 0x400000) {
                typedef void (__thiscall *remove_t)(void* this_);
                ((remove_t)remove_fn)((void*)obj);
                if (logf) nc_fprintf(logf, "  ROTATER: removed obj=0x%08X\n", obj);
            }
        }
    }
    g_rotater_count = 0;
}

static void cEnt_apply_rotater_directions(void) {
    int i;
    for (i = 0; i < g_rotater_count; i++) {
        DWORD obj = g_rotater_cfg[i].obj;
        if (!obj || obj < 0x10000) continue;
        if (IsBadReadPtr((void*)obj, 0x10F0)) continue;
        *(float*)(obj + 0x10EC) = g_rotater_cfg[i].rot_y;
    }
}

static void cEnt_update_constant_rotations(void) {
    int i;
    for (i = 0; i < g_rotater_count; i++) {
        if (g_rotater_cfg[i].ros_y != 0.0f) continue;
        DWORD obj = g_rotater_cfg[i].obj;
        if (!obj || obj < 0x10000) continue;
        if (IsBadReadPtr((void*)obj, 0x10F0)) continue;
        *(float*)(obj + 0x10EC) = g_rotater_cfg[i].rot_y;
        float angle = *(float*)(obj + 0x10E8);
        if (angle > 1.99f) *(float*)(obj + 0x10E8) = -1.99f;
        else if (angle < -1.99f) *(float*)(obj + 0x10E8) = 1.99f;
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * S1 rotater tag application (preserved from original)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void cEnt_apply_s1_rotater_tags(DWORD board, nc_FILE* logf) {
    if (!board) return;
    DWORD sceneobj = cEnt_get_sceneobj(board);
    if (!sceneobj) return;

    DWORD s1_list = sceneobj + 0x894;
    if (IsBadReadPtr((void*)(s1_list + 0x04), 4)) return;
    int s1_count = *(int*)(s1_list + 0x04);
    if (s1_count <= 0 || s1_count > 1000) return;
    if (IsBadReadPtr((void*)(s1_list + 0x40C), 4)) return;
    DWORD* s1_data = *(DWORD**)(s1_list + 0x40C);
    if (!s1_data || IsBadReadPtr(s1_data, s1_count * 4)) return;

    DWORD update_list = board + 0x2578;
    if (IsBadReadPtr((void*)(update_list + 0x04), 4)) return;
    int update_count = *(int*)(update_list + 0x04);
    if (update_count <= 0 || update_count > 10000) return;
    if (IsBadReadPtr((void*)(update_list + 0x40C), 4)) return;
    DWORD* update_data = *(DWORD**)(update_list + 0x40C);
    if (!update_data || IsBadReadPtr(update_data, update_count * 4)) return;

    const DWORD rotator_vtables[] = { 0x004D5518, 0x004D5708, 0 };

    int i, j;
    for (i = 0; i < s1_count; i++) {
        DWORD entry = s1_data[i];
        if (!entry || entry < 0x10000) continue;
        if (IsBadReadPtr((void*)entry, 16)) continue;

        char* name = *(char**)(entry);
        if (!name || IsBadReadPtr(name, 8)) continue;
        if (nc_strnicmp(name, "cEnt", 4) != 0 &&
            nc_strnicmp(name, "REF:cEnt", 8) != 0 &&
            nc_strnicmp(name, "C_entity", 8) != 0 &&
            nc_strnicmp(name, "REF:C_entity", 13) != 0) continue;

        char rot_y_str[32] = {0};
        char ros_y_str[32] = {0};
        cEnt_extract_dat_prop(name, "ROT_Y", rot_y_str, sizeof(rot_y_str));
        cEnt_extract_dat_prop(name, "ROS_Y", ros_y_str, sizeof(ros_y_str));
        if (!rot_y_str[0]) continue;

        float rot_y = nc_atof(rot_y_str);
        float ros_y = ros_y_str[0] ? nc_fabs(nc_atof(ros_y_str)) : 2.0f;

        float px = *(float*)(entry + 0x04);
        float py = *(float*)(entry + 0x08);
        float pz = *(float*)(entry + 0x0C);

        for (j = 0; j < update_count; j++) {
            DWORD obj = update_data[j];
            if (!obj || obj < 0x10000) continue;
            if (IsBadReadPtr((void*)obj, 0x10F0)) continue;

            DWORD vtable = *(DWORD*)obj;
            int is_rotator = 0;
            int k;
            for (k = 0; rotator_vtables[k]; k++) {
                if (vtable == rotator_vtables[k]) { is_rotator = 1; break; }
            }
            if (!is_rotator) continue;

            float ox = *(float*)(obj + 0x10D4);
            float oy = *(float*)(obj + 0x10D8);
            float oz = *(float*)(obj + 0x10DC);
            float dx = ox - px; if (dx < 0) dx = -dx;
            float dy = oy - py; if (dy < 0) dy = -dy;
            float dz = oz - pz; if (dz < 0) dz = -dz;
            if (dx < 2.0f && dy < 2.0f && dz < 2.0f) {
                *(float*)(obj + 0x10EC) = rot_y;

                if (ros_y == 0.0f && g_rotater_count < MAX_ROTATERS) {
                    g_rotater_cfg[g_rotater_count].obj = obj;
                    g_rotater_cfg[g_rotater_count].rot_y = rot_y;
                    g_rotater_cfg[g_rotater_count].ros_y = 0.0f;
                    g_rotater_cfg[g_rotater_count].angle_y = 0.0f;
                    g_rotater_cfg[g_rotater_count].mesh_path[0] = 0;
                    g_rotater_count++;
                }

                if (logf) {
                    nc_fprintf(logf, "  ROTATER(S1-tag): obj=0x%08X at (%.1f,%.1f,%.1f) ROT_Y=%.4f ROS_Y=%.1f\n",
                            obj, px, py, pz, rot_y, ros_y);
                    nc_fflush(logf);
                }
                break;
            }
        }
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Hide entity meshbuffers (preserved from original)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void hide_entity_meshbuffers(DWORD board, nc_FILE* logf) {
    if (!board) return;
    DWORD level = cEnt_get_level(board);
    if (!level) return;
    if (IsBadReadPtr((void*)(level + 0x08), 4)) return;
    DWORD meshworld = *(DWORD*)(level + 0x08);
    if (!meshworld || IsBadReadPtr((void*)meshworld, 0x30)) return;

    DWORD* mb_list = (DWORD*)(meshworld + 0x2C);
    if (IsBadReadPtr(mb_list, 8)) return;
    int mb_count = *(int*)(mb_list + 1);
    if (mb_count <= 0 || mb_count > 10000) return;
    if (IsBadReadPtr((void*)((BYTE*)mb_list + 0x40C), 4)) return;
    DWORD* mb_data = *(DWORD**)((BYTE*)mb_list + 0x40C);
    if (!mb_data || IsBadReadPtr(mb_data, mb_count * 4)) return;

    DWORD rc_array = *(DWORD*)(meshworld + 0x28);
    if (!rc_array || IsBadReadPtr((void*)rc_array, mb_count * 0x50)) {
        rc_array = meshworld + 0x28;
        if (IsBadReadPtr((void*)rc_array, mb_count * 0x50)) return;
    }

    int hidden = 0;
    int i;
    for (i = 0; i < mb_count; i++) {
        DWORD mb = mb_data[i];
        if (!mb || mb < 0x10000) continue;
        if (IsBadReadPtr((void*)mb, 0x870)) continue;

        char* name = *(char**)(mb + 0x864);
        if (!name || IsBadReadPtr(name, 8)) continue;

        if (nc_strnicmp(name, "cEnt", 4) != 0 &&
            nc_strnicmp(name, "C_entity", 8) != 0) continue;

        DWORD rc_addr = rc_array + i * 0x50;
        if (!IsBadReadPtr((void*)rc_addr, 0x50)) {
            nc_memset((void*)rc_addr, 0, 0x50);
            hidden++;
            if (logf) nc_fprintf(logf, "  HIDE: meshbuffer '%s' (idx=%d) zeroed\n", name, i);
        }
    }

    if (logf && hidden > 0) {
        nc_fprintf(logf, "  HIDE: hidden %d meshbuffer(s)\n", hidden);
        nc_fflush(logf);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Entity Definition System (preserved from original)
 * ═══════════════════════════════════════════════════════════════════════════ */

#define MAX_ENTITY_CMDS 32
#define MAX_CMD_ARGS 8

typedef struct {
    char cmd[32];
    char args[MAX_CMD_ARGS][64];
    int arg_count;
} entity_cmd_t;

typedef struct {
    char mesh_file[128];
    int obj_size;
    int   rng_seed;
    float rot_a;
    float rot_d;
    float rot_max;
    float rot_min;
    int   rot_m;
    entity_cmd_t create_cmds[MAX_ENTITY_CMDS];
    int create_cmd_count;
    entity_cmd_t update_cmds[MAX_ENTITY_CMDS];
    int update_cmd_count;
} entity_def_t;

static int load_entity_def(const char* txt_path, entity_def_t* def, nc_FILE* logf) {
    nc_memset(def, 0, sizeof(entity_def_t));
    def->obj_size = 0x1508;

    nc_FILE* ef = nc_fopen(txt_path, "r");
    if (!ef) {
        if (logf) nc_fprintf(logf, "  ENTITY: %s not found\n", txt_path);
        return 0;
    }

    char line[256];
    int section = 0;

    while (nc_fgets(line, sizeof(line), ef)) {
        char* hash = nc_strchr(line, '#');
        if (hash) *hash = 0;

        char* p = line;
        while (*p == ' ' || *p == '\t') p++;
        size_t len = nc_strlen(p);
        while (len > 0 && (p[len-1] == '\n' || p[len-1] == '\r' || p[len-1] == ' ' || p[len-1] == '\t'))
            p[--len] = 0;
        if (len == 0) continue;

        if (nc_stricmp(p, "[onCreate]") == 0) { section = 1; continue; }
        if (nc_stricmp(p, "[onUpdate]") == 0) { section = 2; continue; }

        entity_cmd_t* cmd = NULL;
        if (section == 1 && def->create_cmd_count < MAX_ENTITY_CMDS) {
            cmd = &def->create_cmds[def->create_cmd_count++];
        } else if (section == 2 && def->update_cmd_count < MAX_ENTITY_CMDS) {
            cmd = &def->update_cmds[def->update_cmd_count++];
        }
        if (!cmd) {
            char key[32] = {0}, val[128] = {0};
            if (nc_sscanf_key_val(p, key, 32, val, 128) >= 2) {
                if (nc_stricmp(key, "MESH") == 0) {
                    nc_strncpy(def->mesh_file, val, 127);
                    def->mesh_file[127] = 0;
                }
                else if (nc_stricmp(key, "SIZE") == 0) {
                    def->obj_size = (int)nc_strtol(val, NULL, 0);
                }
                else if (nc_stricmp(key, "RNG") == 0) {
                    if (nc_stricmp(val, "true") == 0 || nc_stricmp(val, "1") == 0)
                        def->rng_seed = 1;
                    else
                        def->rng_seed = 0;
                }
                else if (nc_stricmp(key, "ROT_A") == 0) {
                    def->rot_a = nc_atof(val);
                }
                else if (nc_stricmp(key, "ROT_D") == 0) {
                    def->rot_d = nc_atof(val);
                }
                else if (nc_stricmp(key, "ROT_MAX") == 0) {
                    def->rot_max = nc_atof(val);
                }
                else if (nc_stricmp(key, "ROT_MIN") == 0) {
                    def->rot_min = nc_atof(val);
                }
                else if (nc_stricmp(key, "ROT_M") == 0) {
                    if (val[0] == 'X' || val[0] == 'x') def->rot_m = 0;
                    else if (val[0] == 'Y' || val[0] == 'y') def->rot_m = 1;
                    else if (val[0] == 'Z' || val[0] == 'z') def->rot_m = 2;
                    else def->rot_m = 0;
                }
            }
            continue;
        }

        char* tok = nc_strtok(p, " \t");
        if (!tok) continue;
        nc_strncpy(cmd->cmd, tok, 31);
        cmd->cmd[31] = 0;
        cmd->arg_count = 0;
        while ((tok = nc_strtok(NULL, " \t")) && cmd->arg_count < MAX_CMD_ARGS) {
            nc_strncpy(cmd->args[cmd->arg_count], tok, 63);
            cmd->args[cmd->arg_count][63] = 0;
            cmd->arg_count++;
        }
    }
    nc_fclose(ef);
    return 1;
}

static void exec_create_cmd(DWORD obj, DWORD board, float px, float py, float pz,
                            void* mesh, entity_cmd_t* cmd, nc_FILE* logf) {
    if (nc_stricmp(cmd->cmd, "SET_VTABLE") == 0 && cmd->arg_count >= 1) {
        DWORD vt = (DWORD)nc_strtol(cmd->args[0], NULL, 0);
        *(DWORD*)obj = vt;
    }
    else if (nc_stricmp(cmd->cmd, "STORE_BOARD") == 0 && cmd->arg_count >= 1) {
        int off = (int)nc_strtol(cmd->args[0], NULL, 0);
        *(DWORD*)(obj + off) = board;
    }
    else if (nc_stricmp(cmd->cmd, "STORE_POS") == 0 && cmd->arg_count >= 3) {
        int ox = (int)nc_strtol(cmd->args[0], NULL, 0);
        int oy = (int)nc_strtol(cmd->args[1], NULL, 0);
        int oz = (int)nc_strtol(cmd->args[2], NULL, 0);
        *(float*)(obj + ox) = px;
        *(float*)(obj + oy) = py;
        *(float*)(obj + oz) = pz;
    }
    else if (nc_stricmp(cmd->cmd, "INIT_LIST") == 0 && cmd->arg_count >= 1) {
        int off = (int)nc_strtol(cmd->args[0], NULL, 0);
        nc_memset((void*)(obj + off), 0, 0x410);
    }
    else if (nc_stricmp(cmd->cmd, "CREATE_RENDER_CONTEXT") == 0 && cmd->arg_count >= 1) {
        int off = (int)nc_strtol(cmd->args[0], NULL, 0);
        void* rc = pfn_operator_new(0x10D0);
        if (rc) {
            typedef void* (__thiscall *Level_RenderCtor_t2)(void*, int);
            Level_RenderCtor_t2 pfn = (Level_RenderCtor_t2)0x00465080;
            rc = pfn(rc, (int)obj);
        }
        *(void**)(obj + off) = rc;
    }
    else if (nc_stricmp(cmd->cmd, "SET_FLOAT") == 0 && cmd->arg_count >= 2) {
        int off = (int)nc_strtol(cmd->args[0], NULL, 0);
        float val = nc_atof(cmd->args[1]);
        *(float*)(obj + off) = val;
    }
    else if (nc_stricmp(cmd->cmd, "IF_NOT_PRACTICE") == 0) {
    }
    else if (nc_stricmp(cmd->cmd, "ENDIF") == 0) {
    }
}

static void exec_create_cmds(DWORD obj, DWORD board, float px, float py, float pz,
                             void* mesh, entity_def_t* def, nc_FILE* logf) {
    int i;
    int in_not_practice = 0;
    int is_practice = 0;

    DWORD app = *(DWORD*)(board + 0x878);
    if (app && !IsBadReadPtr((void*)(app + 0x237), 1)) {
        is_practice = (*(char*)(app + 0x237) != 0);
    }

    for (i = 0; i < def->create_cmd_count; i++) {
        entity_cmd_t* cmd = &def->create_cmds[i];

        if (nc_stricmp(cmd->cmd, "IF_NOT_PRACTICE") == 0) {
            in_not_practice = 1;
            continue;
        }
        if (nc_stricmp(cmd->cmd, "ENDIF") == 0) {
            in_not_practice = 0;
            continue;
        }
        if (in_not_practice && is_practice) continue;

        exec_create_cmd(obj, board, px, py, pz, mesh, cmd, logf);
    }

    if (def->rot_a != 0.0f) {
        *(float*)(obj + 0x10E8) = def->rot_a;
    }
    if (def->rot_d != 0.0f) {
        *(float*)(obj + 0x10EC) = def->rot_d;
    }
}

static void exec_update_cmds(DWORD obj, entity_def_t* def, nc_FILE* logf) {
    int i;
    for (i = 0; i < def->update_cmd_count; i++) {
        entity_cmd_t* cmd = &def->update_cmds[i];

        if (nc_stricmp(cmd->cmd, "ROTATE_Y") == 0 && cmd->arg_count >= 1) {
            float speed = nc_atof(cmd->args[0]);
            float angle = *(float*)(obj + 0x10E8);
            float dir = *(float*)(obj + 0x10EC);
            angle = dir * speed + angle;
            *(float*)(obj + 0x10E8) = angle;
        }
        else if (nc_stricmp(cmd->cmd, "OSCILLATE") == 0 && cmd->arg_count >= 1) {
            float limit = nc_atof(cmd->args[0]);
            float angle = *(float*)(obj + 0x10E8);
            float dir = *(float*)(obj + 0x10EC);
            if (angle > limit)  dir = -1.0f;
            if (angle < -limit) dir = 1.0f;
            *(float*)(obj + 0x10EC) = dir;
        }
    }

    {
        float rot_max = def->rot_max;
        float rot_min = def->rot_min;
        float angle = *(float*)(obj + 0x10E8);
        float dir = *(float*)(obj + 0x10EC);

        if (rot_max != 0.0f && angle > rot_max) {
            *(float*)(obj + 0x10EC) = -1.0f;
        }
        if (rot_min != 0.0f && angle < rot_min) {
            *(float*)(obj + 0x10EC) = 1.0f;
        }

        {
            float render_angle = *(float*)(obj + 0x10E4);
            Gfx_Scale_t scale_fn = pfn_Gfx_ScaleX;
            if (def->rot_m == 1) scale_fn = pfn_Gfx_ScaleY;
            else if (def->rot_m == 2) scale_fn = pfn_Gfx_ScaleZ;
            scale_fn(render_angle);
        }
    }
}

typedef struct {
    DWORD obj;
    entity_def_t def;
    int active;
} tracked_entity_t;

static tracked_entity_t g_tracked[MAX_ROTATERS];
static int g_tracked_count = 0;

/* ═══════════════════════════════════════════════════════════════════════════
 * Process rotaters — scan section 3 for C_entity entries (preserved)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void process_rotaters(DWORD board, nc_FILE* logf) {
    if (!board) return;

    DWORD sceneobj = cEnt_get_sceneobj(board);
    if (!sceneobj) {
        if (logf) nc_fprintf(logf, "  ROTATER: sceneobj=NULL\n");
        return;
    }

    int found = 0;
    int i;

    if (IsBadReadPtr((void*)(sceneobj + SCENEOBJ_OBJ_COUNT), 4)) {
        if (logf) nc_fprintf(logf, "  S3: sceneobj+0x898 bad read\n");
        return;
    }
    int obj_count = *(int*)(sceneobj + SCENEOBJ_OBJ_COUNT);
    if (logf) nc_fprintf(logf, "  S3: obj_count=%d (sceneobj=0x%08X)\n", obj_count, sceneobj);
    if (obj_count <= 0 || obj_count > 1000) return;

    if (IsBadReadPtr((void*)(sceneobj + SCENEOBJ_OBJ_ARRAY), 4)) {
        if (logf) nc_fprintf(logf, "  S3: sceneobj+0xCA0 bad read\n");
        return;
    }
    DWORD* obj_array_ptr = *(DWORD**)(sceneobj + SCENEOBJ_OBJ_ARRAY);
    if (!obj_array_ptr || IsBadReadPtr(obj_array_ptr, obj_count * 4)) {
        if (logf) nc_fprintf(logf, "  S3: obj_array_ptr bad\n");
        return;
    }

    for (i = 0; i < obj_count; i++) {
        DWORD obj_ptr = obj_array_ptr[i];
        if (!obj_ptr || obj_ptr < 0x10000) continue;
        if (IsBadReadPtr((void*)obj_ptr, 20)) continue;

        char* name = *(char**)(obj_ptr);
        if (!name || IsBadReadPtr(name, 4)) continue;

        char debug_name[81] = {0};
        nc_strncpy(debug_name, name, 80);
        debug_name[80] = 0;
        if (logf) nc_fprintf(logf, "    [%d] name='%s' pos=(%.1f, %.1f, %.1f)\n",
                i, debug_name,
                *(float*)(obj_ptr + 0x04), *(float*)(obj_ptr + 0x08), *(float*)(obj_ptr + 0x0C));

        if (nc_strnicmp(name, "cEnt", 4) != 0 &&
            nc_strnicmp(name, "REF:cEnt", 8) != 0) continue;

        char entity_name[64] = {0};
        {
            const char* ent_start = nc_stristr(name, "<ENTITY>");
            if (ent_start) {
                ent_start += 8;
                while (*ent_start == ' ' || *ent_start == '\t') ent_start++;
                const char* ent_end = nc_stristr(ent_start, "</ENTITY>");
                if (!ent_end) ent_end = ent_start + nc_strlen(ent_start);
                size_t ent_len = ent_end - ent_start;
                if (ent_len > 0 && ent_len < 64) {
                    nc_strncpy(entity_name, ent_start, ent_len);
                    entity_name[ent_len] = 0;
                    while (ent_len > 0 && (entity_name[ent_len-1] == ' ' || entity_name[ent_len-1] == '\t'))
                        entity_name[--ent_len] = 0;
                }
            }
        }
        if (!entity_name[0]) continue;

        float px = *(float*)(obj_ptr + 0x04);
        float py = *(float*)(obj_ptr + 0x08);
        float pz = *(float*)(obj_ptr + 0x0C);

        int ai_type = -1;
        const char* ai_mesh = NULL;

        typedef struct { const char* name; int ctor_type; const char* mesh; } ai_entry_t;
        static const ai_entry_t ai_list[] = {
            { "8ball",            15, "meshes\\8ball" },
            { "BBridge",          0, "levels\\Level10-Bridge1" },
            { "Bell",             0, "meshes\\bell" },
            { "Blockdawg",        0, "levels\\Level8-BlockDawg1" },
            { "Bonk",             33, "levels\\Level5-Bonk" },
            { "Bridge",           34, "levels\\Level2-Bridge" },
            { "Bridgeslam",       16, "levels\\Level2-Bridge" },
            { "Bumper",           0, "levels\\_default" },
            { "Catapult",         35, "levels\\Level4-Catapult" },
            { "Chomper",          22, "meshes\\chomper" },
            { "Chrome",           23, "levels\\_default" },
            { "Cloudscape",       28, "levels\\Cloudscape" },
            { "Drawbridge",       9, "levels\\Level4-Drawbridge" },
            { "Droplifter",       42, "levels\\Level6-Lifter" },
            { "Fan",              0, "meshes\\fanbody" },
            { "Flag",             14, "levels\\\\Flag" },
            { "Flag2",            14, "levels\\Flag" },
            { "Flickfloor1",      7,  "levels\\LevelDark-DFloor1" },
            { "Flickfloor2",     19, "levels\\LevelDark-DFloor4" },
            { "Flickring",       20, "levels\\LevelDark-Flickring" },
            { "Funball",          24, "meshes\\funball" },
            { "Gear",             29, "levels\\LevelImpossible-Gear" },
            { "Glassbreaker",     11, "meshes\\GlassBonus" },
            { "Gluebie",           0, "levels\\Level3-Gluebie" },
            { "Judge",            10, "meshes\\hammyjudge" },
            { "Lifter",           38, "levels\\LevelUp-Lifter" },
            { "Looper",           3, "levels\\LevelImpossible-Looper" },
            { "Mace",              36, "levels\\Level4-Mace" },
            { "Mag",              0, "meshes\\magnifyingglass" },
            { "Mousetrap",        0, "levels\\MouseTrap" },
            { "Neonplatform",     40, "levels\\LevelDark-NeonPlatform" },
            { "Pendulum",         2, "levels\\LevelImpossible-Pendulum" },
            { "Popcylinder",      0, "levels\\Level9-PopCylinder1" },
            { "Rotator",          1, "levels\\LevelImpossible-Rotator" },
            { "Saw",              0, "levels\\Level8-Saw" },
            { "Sawblade",         0, "meshes\\sawblade" },
            { "Sign",             13, "levels\\PopupSign" },
            { "Speedcylinder",    39, "levels\\LevelUp-SpeedCylinder" },
            { "Spinner",          27, "levels\\Level8-Spinny" },
            { "Swirl",            6, "levels\\Level3-Swirl" },
            { "Tarbubble",        25, "meshes\\tarbubble" },
            { "Tarpit",           0, "levels\\_default" },
            { "Timebutton",       0, "levels\\LevelUp-Button" },
            { "Tipper",            37, "levels\\Level3-Tipper" },
            { "Trapdoor",         41, "levels\\Level4-Trapdoor1" },
            { "Trode",            21, "levels\\LevelDark-Trode" },
            { "Waterwheel",       26, "levels\\Level3-WaterWheel" },
            { "Wavy",             0, "levels\\Level7-Wavy1" },
            { "Windmill",         0, "levels\\Level4-Windmill" },
            { "Wobbly",           8, "levels\\Level7-Wobbly1" },
        };
        static const int ai_list_count = sizeof(ai_list) / sizeof(ai_list[0]);

        int j;
        for (j = 0; j < ai_list_count; j++) {
            if (nc_stricmp(entity_name, ai_list[j].name) == 0) {
                ai_type = ai_list[j].ctor_type;
                ai_mesh = ai_list[j].mesh;
                break;
            }
        }

        if (ai_type < 0) {
            if (logf) nc_fprintf(logf, "  cEnt(S3): '%s' — no matching AI, skipping\n", entity_name);
            continue;
        }

        /* Skip Swirl on levels that natively have SWIRL */
        if (ai_type == 6) {
            DWORD app = *(DWORD*)(board + BOARD_APP);
            if (app && !IsBadReadPtr((void*)(app + 0x23C), 4)) {
                char* level_name = NULL;
                if (!IsBadReadPtr((void*)(board + 0x10), 4)) {
                    level_name = *(char**)(board + 0x10);
                }
                if (level_name && !IsBadReadPtr(level_name, 5)) {
                    if (nc_strnicmp(level_name, "Dizzy", 5) == 0 ||
                        nc_strnicmp(level_name, "Master", 6) == 0 ||
                        nc_strnicmp(level_name, "Board (Arena", 12) == 0 ||
                        nc_strnicmp(level_name, "Arena", 5) == 0) {
                        if (logf) nc_fprintf(logf, "  cEnt(S3): '%s' — SKIPPED (native SWIRL on %s)\n",
                                entity_name, level_name);
                        continue;
                    }
                }
            }
        }

        float spawn_ros_y = 2.0f;
        if (ai_type == 1) spawn_ros_y = 0.0f;

        cEnt_spawn_rotater_at(board, px, py, pz, ai_mesh,
                         0.0f, 1.0f, 0.0f,
                         2.0f, spawn_ros_y, 2.0f,
                         ai_type,
                         logf);
        found++;
    }

    if (found > 0 && logf) nc_fprintf(logf, "  Processed %d cEnt entries\n", found);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Init helpers (preserved, adapted for HB+)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void init_game_dir(void) {
    char dll_path[MAX_PATH] = {0};
    HMODULE hMod = NULL;

    MEMORY_BASIC_INFORMATION mbi;
    if (VirtualQuery((void*)init_game_dir, &mbi, sizeof(mbi)) > 0) {
        hMod = (HMODULE)mbi.AllocationBase;
    }
    if (hMod && GetModuleFileNameA(hMod, dll_path, MAX_PATH) > 0) {
        char* p = nc_strrchr(dll_path, '\\');
        if (p) *p = 0;
        nc_strncpy(g_game_dir, dll_path, MAX_PATH - 1);
        g_game_dir[MAX_PATH - 1] = 0;
    }
}

static void load_config(void) {
    char config_path[MAX_PATH];
    nc_snprintf(config_path, MAX_PATH, "%s\\custom_entities.txt", g_game_dir);
    nc_FILE* f = nc_fopen(config_path, "r");
    if (!f) return;

    char line[256];
    while (nc_fgets(line, sizeof(line), f)) {
        char* p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '#' || *p == '\n' || *p == '\r' || *p == '\0') continue;

        if (nc_strnicmp(p, "grid_speed", 10) == 0) {
            char* eq = nc_strchr(p, '=');
            if (eq) {
                float val = nc_atof(eq + 1);
                if (val > 0.0f) g_grid_speed = val;
            }
        }
    }
    nc_fclose(f);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Level spawning logic — called from onLevelStart callback
 * Replaces the entity_thread spawning section
 * ═══════════════════════════════════════════════════════════════════════════ */

static int g_level_spawned = 0;

static void do_level_spawn(DWORD board, nc_FILE* logf) {
    if (!board) return;

    /* Process <MESH> and <SPEEDMULT> tags */
    cEnt_process_custom_tags(board, logf);

    /* Process C_entity entries — spawn custom objects */
    process_rotaters(board, logf);

    /* Hide original meshbuffers */
    hide_entity_meshbuffers(board, logf);

    /* Apply custom rotation directions */
    cEnt_apply_rotater_directions();

    /* Apply S1 rot tags */
    cEnt_apply_s1_rotater_tags(board, logf);

    /* Find and spawn GRID points */
    float grid_x[32], grid_y[32], grid_z[32];
    int grid_count = cEnt_find_grid_points(board, grid_x, grid_y, grid_z, 32, logf);

    if (grid_count > 0) {
        g_spawned_board = board;
        if (logf) nc_fprintf(logf, "  Found %d GRID points (speed=%.1fs)\n", grid_count, g_grid_speed);

        /* Spawn GRID01 */
        cEnt_spawn_testcube_at(board, grid_x[0], grid_y[0], grid_z[0], 1, logf);
        if (logf) nc_fprintf(logf, "  Cycle: GRID01 spawned\n");
    } else {
        g_spawned_board = board;
        cEnt_despawn_all_rotaters(board, logf);
        if (logf) nc_fprintf(logf, "  No GRID points found\n");
    }

    g_level_spawned = 1;
}

static void do_level_cleanup(DWORD board) {
    /* Despawn all testcube objects */
    int j;
    while (g_spawned_count > 0) {
        cEnt_despawn_object(board, g_spawned_objs[0], NULL);
        for (j = 0; j < g_spawned_count - 1; j++) {
            g_spawned_objs[j] = g_spawned_objs[j + 1];
            nc_strcpy(g_spawned_names[j], g_spawned_names[j + 1]);
        }
        g_spawned_count--;
    }

    /* Despawn all rotaters */
    cEnt_despawn_all_rotaters(board, NULL);

    /* Reset bridgeslams */
    int i;
    for (i = 0; i < g_bridgeslam_count; i++) {
        g_bridgeslams[i].active = 0;
    }
    g_bridgeslam_count = 0;

    g_spawned_board = 0;
    g_level_spawned = 0;
    g_tracked_count = 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * HB+ vtable — 17 entries, manual dispatch
 * ═══════════════════════════════════════════════════════════════════════════ */

static void* __thiscall sc_dtor(void* thisptr, int flags) {
    if (flags & 1) nc_free(thisptr);
    return thisptr;
}

static const char* __thiscall get_mod_name(void*) { return "Custom Entities"; }
static const char* __thiscall get_author(void*) { return "BookwormKevin"; }
static int __thiscall get_version(void*) { return HAMSTERBALL_API_VERSION; }
static const char* __thiscall get_contributors(void*) { return "Hamsterbot"; }

static void __thiscall init_impl(void* thisptr, void* modApi) {
    *(void**)((char*)thisptr + 4) = modApi;
    g_storedApi = modApi;
    init_game_dir();
    load_config();
}

/* onGameUpdate — per-frame updates: tracked entities, bridgeslams, constant rotations */
static void __thiscall game_update_impl(void* thisptr) {
    if (!g_storedApi) return;

    /* Per-frame: execute onUpdate scripts for tracked entities */
    {
        int i;
        for (i = 0; i < g_tracked_count; i++) {
            if (!g_tracked[i].active) continue;
            DWORD obj = g_tracked[i].obj;
            if (!obj || obj < 0x10000) { g_tracked[i].active = 0; continue; }
            if (IsBadReadPtr((void*)obj, 0x10F0)) { g_tracked[i].active = 0; continue; }
            exec_update_cmds(obj, &g_tracked[i].def, NULL);
        }
    }

    /* Per-frame: update all active bridgeslams */
    {
        int i;
        for (i = 0; i < g_bridgeslam_count; i++) {
            if (g_bridgeslams[i].active) {
                cEnt_bridgeslam_update(&g_bridgeslams[i]);
            }
        }
    }

    /* Per-frame: constant rotation override */
    cEnt_update_constant_rotations();
}

/* onLevelStart — spawn entities when level loads */
static void __thiscall level_start_impl(void* thisptr) {
    if (!g_storedApi) return;

    /* Wait a moment for level to settle, then spawn */
    /* In HB+ mode, onLevelStart is called after the level is loaded */
    DWORD board = get_board();
    if (!board) return;

    DWORD level = cEnt_get_level(board);
    if (!level) return;

    /* Open log file */
    char log_path[MAX_PATH];
    nc_snprintf(log_path, MAX_PATH, "%s\\custom_entities.log", g_game_dir);
    nc_FILE* logf = nc_fopen(log_path, "a");
    if (logf) {
        nc_fprintf(logf, "\n--- Level loaded (board=0x%08X, level=0x%08X) ---\n", board, level);
    }

    do_level_spawn(board, logf);

    if (logf) {
        nc_fflush(logf);
        nc_fclose(logf);
    }
}

/* onSceneEnd — cleanup when level/scene ends */
static void __thiscall scene_end_impl(void* thisptr) {
    DWORD board = get_board();
    if (board) {
        do_level_cleanup(board);
    }
}

/* No-op implementations for unused callbacks */
static void __thiscall ball_update_impl(void*, void*) {}
static void __thiscall render_apply_impl(void*, void*, float*) {}
static void __thiscall button_toggle_impl(void*, const char*, bool) {}
static void __thiscall slider_change_impl(void*, const char*, float) {}
static void __thiscall cycle_change_impl(void*, const char*, const char*) {}
static void __thiscall event_collide_impl(void*, void*, char*) {}
static void __thiscall text_render_impl(void*) {}
static void __thiscall ball_bump_impl(void*, void*, void*) {}

static void* g_vtable[17] = {
    (void*)sc_dtor,              // [0]
    (void*)get_mod_name,         // [1]
    (void*)get_author,           // [2]
    (void*)get_version,          // [3]
    (void*)get_contributors,     // [4]
    (void*)init_impl,            // [5]
    (void*)ball_update_impl,     // [6]
    (void*)render_apply_impl,    // [7]
    (void*)button_toggle_impl,   // [8]
    (void*)slider_change_impl,   // [9]
    (void*)cycle_change_impl,    // [10]
    (void*)game_update_impl,     // [11]
    (void*)event_collide_impl,   // [12]
    (void*)text_render_impl,     // [13]
    (void*)ball_bump_impl,       // [14]
    (void*)scene_end_impl,       // [15]
    (void*)level_start_impl,     // [16]
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    void* obj = nc_malloc(8);
    if (!obj) return NULL;
    *(void**)obj = g_vtable;
    *(void**)((char*)obj + 4) = NULL;
    return (HamsterballAPI*)obj;
}
