/*
 * LevelFeatures_Loader v6 — Universal Level Handler + Universal Vtable
 *
 * 1. ALLOCATION PATCH: Patches all 15 level allocation sites in
 *    Tournament_AdvanceRace (0x00427080) to use the union size 0xA2F8.
 *
 * 2. BOARD CONSTRUCTOR HOOK: Patches all 15 CALL LevelBoard_*_ctor
 *    instructions to call a single UniversalBoardCtor driven by
 *    LevelData.txt config file.
 *
 * 3. SCENE LOADING HOOK: Hooks vtable[0x48] (CALL [EDX+0x48]) to
 *    replace all per-level Scene_LoadLevel* functions with a universal
 *    4-step scene loader (MeshWorldCtor, RenderCtor, InitScene, Board_Setup).
 *
 * 4. COLLISION HOOK: Hooks DispatchCollisionEvents (0x0040C5D0) to
 *    handle N:BUMPER collisions on any level, driven by LevelFeatures.txt.
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef unsigned long long QWORD;

/* ═══════════════════════════════════════════════════════════════════════════
 * Debug logging (forward declaration — defined later, used by hook functions)
 * ═══════════════════════════════════════════════════════════════════════════ */
void DebugLog(const char *msg);

/* ═══════════════════════════════════════════════════════════════════════════
 * Game addresses (RVAs)
 * ═══════════════════════════════════════════════════════════════════════════ */

#define RVA_DispatchCollisionEvents  0x000CC5D0
#define RVA_Sound_Play3D             0x00059860
#define RVA_Scene_CollectByNameFilter 0x000602F0
#define RVA_AthenaList_Init          0x00053210
#define RVA_operator_new              0x000BA57B
#define RVA_Level_MeshWorldCtor       0x00061510
#define RVA_Level_RenderCtor          0x00065080
#define RVA_Level_InitScene           0x0000B090
#define RVA_Board_ctor                0x00019030
#define RVA_LoadRaceData              0x0000A120
#define RVA_Vec3_Init                 0x00053180
#define RVA_Matrix_Identity           0x00053200
#define RVA_MeshNode_ctor              0x00071C20
#define RVA_Sprite_ctor                0x0005D0C0
#define RVA_TipperVisual_Attach       0x00065200
#define RVA_Level_AssignTexAndScales  0x00011BA0
#define RVA_Sound_GetNextChannel      0x00059810
#define RVA_Scene_RenderIfVisible     0x00059610
#define RVA_AthenaList_Append         0x00053810
#define RVA_AthenaList_GetSize        0x000536A0
#define RVA_AthenaList_GetIterator    0x000532B0

/* Bumper physics constants */
#define BUMPER_VEL_SCALE  4.0f
#define BUMPER_MIN_SPEED  5.0f
#define BUMPER_MAX_SPEED  10.0f

/* Struct offsets */
#define BALL_POS_X        0x164
#define BALL_POS_Y        0x168
#define BALL_POS_Z        0x16C
#define BALL_PHYS_PTR     0x1A4
#define PHYS_VEL_X        0xCA4
#define PHYS_VEL_Y        0xCA8
#define PHYS_VEL_Z        0xCAC
#define APP_SOUNDFX_LIST  0x448
#define COLL_MESHBUF      0x4
#define MESHBUF_NAME      0x864
#define BOARD_APP_PTR     0x878
#define BOARD_MESHWORLD   0x8AC
#define BOARD_RENDEROBJ   0x8B0

/* Bumper slot layout */
#define BUMPER_SLOT_BASE   0x436C
#define BUMPER_SLOT_STRIDE 0x418
#define BUMPER_LIT_BASE    0x642C
#define BUMPER_LIT_STRIDE  4
#define BUMPER_LIT_COLL     0x6428

/* Bridge slot layout (board+0x436C..0x4388) */
#define BRIDGE_MESHWORLD   0x436C
#define BRIDGE_RENDEROBJ   0x4370
#define BRIDGE_PARAM1      0x4380  /* float 45.0 in Intermediate */
#define BRIDGE_PARAM2      0x4384  /* 0 */
#define BRIDGE_PARAM3      0x4388  /* 0x32 = 50 */

#define UNION_SIZE  0xA2F8

/* ═══════════════════════════════════════════════════════════════════════════
 * Object type system — extensible per-level feature toggles
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef enum {
    OBJ_BUMPERS = 0,
    OBJ_BRIDGE,
    OBJ_COUNT
} ObjectType;

static const char *g_objectNames[OBJ_COUNT] = {
    "BUMPERS",
    "BRIDGE",
};

/* ═══════════════════════════════════════════════════════════════════════════
 * Update feature system — per-level Board_Update feature blocks
 * Driven by LevelData.txt "Features" key (semicolon-separated)
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef enum {
    FEAT_NONE          = 0,
    FEAT_BRIDGE_ANIM   = 1 << 0,  /* Intermediate's 4-state bridge tilt */
    FEAT_SWIRL         = 1 << 1,  /* Dizzy/Master swirl zones + tar bubbles */
    FEAT_WINDMILL      = 1 << 2,  /* Tower windmill rotation */
    FEAT_BADBALL       = 1 << 3,  /* Odd badball spawner */
    FEAT_BUMPER_DECAY  = 1 << 4,  /* Beginner/Toob/Master bumper lit decay */
    FEAT_NEON_CAM      = 1 << 5,  /* Neon ball-following camera */
    FEAT_SKY_POPCYL    = 1 << 6,  /* Sky popcylinder activator */
    FEAT_MASTER_EXTRA  = 1 << 7,  /* Master vtable[0x90]+[0x94] calls */
} UpdateFeature;

static DWORD g_updateFeatures[16] = {0};

/* Default feature assignments per level (matching original game behavior) */
static const DWORD g_defaultFeatures[16] = {
    0,
    /* 1=WarmUp */       0,
    /* 2=Beginner */     FEAT_BUMPER_DECAY,
    /* 3=Intermediate */ FEAT_BRIDGE_ANIM,
    /* 4=Dizzy */        FEAT_SWIRL,
    /* 5=Tower */        FEAT_WINDMILL,
    /* 6=Up */           0,
    /* 7=Neon */         FEAT_NEON_CAM,
    /* 8=Expert */       0,
    /* 9=Odd */          FEAT_BADBALL,
    /* 10=Toob */        FEAT_BUMPER_DECAY,
    /* 11=Wobbly */      0,
    /* 12=Glass */       0,
    /* 13=Sky */         FEAT_SKY_POPCYL,
    /* 14=Master */      FEAT_SWIRL | FEAT_BUMPER_DECAY | FEAT_BRIDGE_ANIM | FEAT_MASTER_EXTRA,
    /* 15=Impossible */  0,
};

/* Game function typedefs for Board_Update feature blocks */
typedef void (__fastcall *Scene_Update_t)(void *board);
typedef void (__fastcall *Board_UpdateRaceState_t)(void *board);
typedef void (__fastcall *Gfx_ScaleFn_t)(float val);
typedef void (__fastcall *Gfx_SetPosition_t)(float x, float y, float z);
typedef void (__fastcall *Timer_Init_t)(void *out);
typedef void (__fastcall *Timer_Cleanup_t)(void *out);
typedef void (__fastcall *Matrix_TransformVec3_t)(float *out, float *in);
typedef void (__fastcall *Matrix44_Zero_t)(int *out);
typedef void (__thiscall *Scene_ForEachBall_SetVelocity_t)(void *board, float x, float y, float z);
typedef int  (__thiscall *AthenaList_GetIterator_t)(void *list);
typedef int  (__thiscall *AthenaList_GetSize_t)(void *list);
typedef void (__thiscall *AthenaList_Append_t)(void *list, int item);
typedef void *__cdecl  (*operator_new_t)(unsigned int size);
typedef void (__fastcall *Sound_Play3D_Fn_t)(void *soundChannel, float x, float y, float z);
typedef void *__fastcall (*FUN_0044fa90_t)(void *out, int app, int tarList);
typedef void *__fastcall (*FUN_0044fb50_t)(void *out, int app, float x, int y, float z);
typedef void (__fastcall *FUN_00405190_t)(int ball);
typedef int  (__cdecl *CPUID_RNG_t)(void *ptr, int range, char flag);
typedef void *__fastcall (*BadBall_ctor_t)(void *mem, int board);
typedef void (__fastcall *Ball_SetTrajectory_t)(void *ball, int unk, float x, float y, float f1, float f2);
typedef void (__fastcall *Ball_SetVec3AtOffset_t)(void *offset, float *vec);
typedef void (__fastcall *Vec3_NormalizeAndScale_t)(float *vec, float scale);
typedef void (__fastcall *Vec3_Copy_t)(float *dst, float *src);
typedef int  (__cdecl *Sound_CalcDistAtten_t)(int soundDevice, float x, float y, float z);
typedef void (__thiscall *Sound_Play3DAtPos_t)(int channel);
typedef void (__thiscall *Scene_SetRaceActive_t)(int obj);
typedef void (__fastcall *Scene_AddObject_t)(void *scene, void *obj);

static Scene_Update_t             g_SceneUpdate = NULL;
static Board_UpdateRaceState_t    g_BoardUpdateRaceState = NULL;
static Gfx_ScaleFn_t              g_GfxScaleZ = NULL;
static Gfx_ScaleFn_t              g_GfxScaleY = NULL;
static Gfx_ScaleFn_t              g_GfxScaleX = NULL;
static Gfx_SetPosition_t          g_GfxSetPosition = NULL;
static Timer_Init_t               g_TimerInit = NULL;
static Timer_Cleanup_t            g_TimerCleanup = NULL;
static Matrix_TransformVec3_t     g_MatrixTransformVec3 = NULL;
static Matrix44_Zero_t            g_Matrix44Zero = NULL;
static Scene_ForEachBall_SetVelocity_t g_SceneForEachBallSetVelocity = NULL;
static AthenaList_GetIterator_t   g_AthenaListGetIteratorUpd = NULL;
static AthenaList_GetSize_t       g_AthenaListGetSizeUpd = NULL;
static AthenaList_Append_t        g_AthenaListAppendUpd = NULL;
static operator_new_t            g_OperatorNewUpd = NULL;
static Sound_Play3D_Fn_t          g_SoundPlay3DUpd = NULL;
static FUN_0044fa90_t             g_CreateTarBubble = NULL;
static FUN_0044fb50_t             g_CreateSplashParticle = NULL;
static FUN_00405190_t             g_RemoveBall = NULL;
static CPUID_RNG_t                g_RNG = NULL;
static BadBall_ctor_t             g_BadBallCtor = NULL;
static Ball_SetTrajectory_t       g_BallSetTrajectory = NULL;
static Ball_SetVec3AtOffset_t     g_BallSetVec3AtOffset = NULL;
static Vec3_NormalizeAndScale_t   g_Vec3NormalizeAndScale = NULL;
static Vec3_Copy_t                g_Vec3CopyUpd = NULL;
static Sound_CalcDistAtten_t      g_SoundCalcDistAtten = NULL;
static Sound_Play3DAtPos_t        g_SoundPlay3DAtPos = NULL;
static Scene_SetRaceActive_t      g_SceneSetRaceActive = NULL;
static Scene_AddObject_t          g_SceneAddObject = NULL;

/* RVA constants for Board_Update functions */
#define RVA_Scene_Update              0x00019C00
#define RVA_Board_UpdateRaceState     0x0001B130
#define RVA_Gfx_ScaleZ                0x00057CC0
#define RVA_Gfx_ScaleY                0x00057C90
#define RVA_Gfx_ScaleX                0x00057C60
#define RVA_Gfx_SetPosition           0x00057B50
#define RVA_Timer_Init                0x00057AD0
#define RVA_Timer_Cleanup             0x00057A40
#define RVA_Matrix_TransformVec3      0x0000A0B0
#define RVA_Matrix44_Zero             0x00057B10
#define RVA_Scene_ForEachBall_SetVel  0x00019B70
#define RVA_FUN_0044fa90              0x0004FA90
#define RVA_FUN_0044fb50              0x0004FB50
#define RVA_FUN_00405190              0x00005190
#define RVA_CPUID_RNG                 0x0005DD60
#define RVA_BadBall_ctor              0x0000AFE0
#define RVA_Ball_SetTrajectory        0x00003850
#define RVA_Ball_SetVec3AtOffset      0x00002A20
#define RVA_Vec3_NormalizeAndScale    0x00001AA0
#define RVA_Vec3_Copy_Upd             0x00002BF0
#define RVA_Sound_CalcDistAtten       0x00066750
#define RVA_Sound_Play3DAtPos         0x00058EE0
#define RVA_Scene_SetRaceActive       0x000366E0
#define RVA_Scene_AddObject           0x00069990

/* Board field offsets for feature blocks */
/* Bridge animation (Intermediate) */
#define BRD_BRIDGE_RENDER   0x10DB   /* render object ptr */
#define BRD_BRIDGE_PIVOT_X  0x10DD
#define BRD_BRIDGE_PIVOT_Y  0x10DE
#define BRD_BRIDGE_PIVOT_Z  0x10DF
#define BRD_BRIDGE_ANGLE    0x10E0
#define BRD_BRIDGE_STATE    0x10E1
#define BRD_BRIDGE_COUNTER  0x10E2

/* Windmill (Tower) */
#define BRD_WM_RENDER       0x10DF   /* reuses bridge pivot_Z slot; different levels use different offsets */
#define BRD_WM_POS_X        0x10E0
#define BRD_WM_POS_Y        0x10E1
#define BRD_WM_POS_Z        0x10E2
#define BRD_WM_ANGLE         0x10E3
#define BRD_WM_SPEED         0x10E8
#define BRD_WM_STATE         0x10EA
#define BRD_WM_COUNTER       0x10EB
#define BRD_WM_DECAY_VAL     0x10EC

/* BadBall spawner (Odd) */
#define BRD_BB_FLAG         0x10DC
#define BRD_BB_COUNTER       0x10DD
#define BRD_BB_TOTAL         0x10DE
#define BRD_BB_LAST_IDX      0x10E8
#define BRD_BB_POS_TABLE     0x10DF   /* 3×3 float table */

/* Swirl (Dizzy) */
#define BRD_SWIRL_LIST       0x10DE
#define BRD_TARBUBBLE_LIST   0x11E4
#define BRD_SWIRL_MESH1      0x12EA
#define BRD_SWIRL_MESH2      0x12F1
#define BRD_SWIRL1_POS_X     0x12EC
#define BRD_SWIRL1_POS_Y     0x12ED
#define BRD_SWIRL1_POS_Z     0x12EE
#define BRD_SWIRL1_ANGLE     0x12EF
#define BRD_SWIRL1_SPEED     0x12F0
#define BRD_SWIRL2_POS_X     0x12F3
#define BRD_SWIRL2_POS_Y     0x12F4
#define BRD_SWIRL2_POS_Z     0x12F5
#define BRD_SWIRL2_ANGLE     0x12F6

/* Swirl (Master) — different offsets */
#define BRD_SWIRL_LIST_M     0x1820
#define BRD_TARBUBBLE_LIST_M 0x1719

/* Bumper decay offsets */
#define BRD_BUMPER_DECAY_BEG  0x642C
#define BRD_BUMPER_DECAY_TOOB 0x644C
#define BRD_BUMPER_DECAY_MAST 0x53FC

/* Ball offsets */
#define BALL_POS_X_OFS      0x164
#define BALL_POS_Y_OFS      0x168
#define BALL_POS_Z_OFS      0x16C
#define BALL_PHYS_PTR_OFS   0x1A4
#define BALL_PHYS_VEL_X     0xCA4
#define BALL_PHYS_VEL_Y     0xCA8
#define BALL_PHYS_VEL_Z     0xCAC
#define BALL_IN_TAR_OFS     0x2CC
#define BALL_TAR_SOUND_FLAG 0x2BC   /* +700 dec = 0x2BC */

/* App offsets */
#define APP_DIFFICULTY      0x23C
#define APP_BALL_PTR         0x5DC
#define APP_SOUNDFX_47C      0x47C
#define APP_SOUNDFX_478      0x478
#define APP_SOUNDFX_484      0x484

/* ═══════════════════════════════════════════════════════════════════════════
 * Level vtable addresses (absolute — module base 0x00400000)
 * ═══════════════════════════════════════════════════════════════════════════ */

static DWORD g_levelVtables[16] = {
    0,              /* index 0 unused */
    0x004D04A8,     /* 1=WarmUp */
    0x004D1098,     /* 2=Beginner */
    0x004D05A0,     /* 3=Intermediate */
    0x004D0890,     /* 4=Dizzy */
    0x004D0A08,     /* 5=Tower */
    0x004D11A0,     /* 6=Up */
    0x004D1DF0,     /* 7=Neon */
    0x004D0B00,     /* 8=Expert */
    0x004D0BC0,     /* 9=Odd */
    0x004D0E78,     /* 10=Toob */
    0x004D0D38,     /* 11=Wobbly */
    0x004D1F90,     /* 12=Glass */
    0x004D0FC8,     /* 13=Sky */
    0x004D12B0,     /* 14=Master */
    0x004D21C0,     /* 15=Impossible */
};

/* ═══════════════════════════════════════════════════════════════════════════
 * Per-level data defaults (extracted from Ghidra decompilation)
 * ═══════════════════════════════════════════════════════════════════════════ */

#define MAX_MESHES_PER_LEVEL 16
#define MAX_STR_LEN 128

typedef struct {
    char name[MAX_STR_LEN];
    DWORD vtable;
    char boardName[MAX_STR_LEN];
    char raceTitle[MAX_STR_LEN];
    char raceData[MAX_STR_LEN];
    char musicName[MAX_STR_LEN];
    float color[3];
    char meshPath[MAX_STR_LEN];
    /* Extra meshes: offset:path pairs. Path prefixes:
       bare = Level_MeshWorldCtor, RENDER = Level_RenderCtor(prev),
       MESH: = MeshNode_ctor, SPRITE: = Sprite_ctor */
    char meshes[MAX_MESHES_PER_LEVEL][MAX_STR_LEN];
    int meshCount;
    DWORD unlockFlagOffset;  /* 0 = none */
} LevelData;

static LevelData g_levelData[16] = {
    {{0}}, /* index 0 unused */
    /* 1=WarmUp */
    {"WarmUp",0x004D04A8,"Board (Warm-Up)","WARM-UP RACE","BEGINNERRACE","Hamster Nation",{1.0f,0.0f,1.0f},"levels\\level1",{},0,0},
    /* 2=Beginner */
    {"Beginner",0x004D1098,"Board (Beginner)","BEGINNER RACE","CASCADERACE","Cascade Race",{1.0f,0.75f,0.25f},"levels\\levelcascade",{},0,0},
    /* 3=Intermediate */
    {"Intermediate",0x004D05A0,"Board (Intermediate)","INTERMEDIATE RACE","INTERMEDIATERACE","Gerbil Groove",{0.0f,0.0f,1.0f},"levels\\level2",{},0,0},
    /* 4=Dizzy */
    {"Dizzy",0x004D0890,"Board (Dizzy)","DIZZY RACE","DIZZYRACE","Dizzy!",{0.0f,1.0f,0.0f},"levels\\level3",{"0x436C:Levels\\Level3-Tipper","0x4370:RENDER","0x4BA8:Levels\\Level3-WaterWheel","0x4BAC:RENDER","0x4BC4:Levels\\Level3-Swirl","0x4BC8:RENDER","0x4374:Levels\\Level3-Gluebie"},7,0x851},
    /* 5=Tower */
    {"Tower",0x004D0A08,"Board (Tower)","TOWER RACE","TOWERRACE","Happy Rush",{1.0f,0.75f,0.0f},"levels\\level4",{"0x436C:Levels\\Level4-Catapult","0x4370:Levels\\Level4-Drawbridge","0x4374:MESH:Meshes\\YellowLink","0x4378:Levels\\Level4-Mace","0x437C:Levels\\Level4-Windmill","0x4390:MESH:Meshes\\Chomper","0x43B4:Levels\\Level4-Turret"},7,0},
    /* 6=Up */
    {"Up",0x004D11A0,"Board (Up)","UP RACE","UPRACE","Up Race",{1.0f,0.0f,1.0f},"levels\\levelup",{"0x4784:levels\\levelup-lifter","0x4788:levels\\levelup-speedcylinder","0x478C:levels\\levelup-button"},3,0x853},
    /* 7=Neon */
    {"Neon",0x004D1DF0,"Board (Dark)","NEON RACE","NEONRACE","Neon Theme",{1.0f,1.0f,0.0f},"levels\\leveldark",{"0x4374:Levels\\LevelDark-NeonPlatform","0x4378:Levels\\LevelDark-DFloor1","0x437C:Levels\\LevelDark-DFloor2","0x4380:Levels\\LevelDark-DFloor3","0x4384:Levels\\LevelDark-DFloor4","0x4388:Levels\\LevelDark-Trode"},6,0},
    /* 8=Expert */
    {"Expert",0x004D0B00,"Board (Expert)","EXPERT RACE","EXPERTRACE","Fight!",{1.0f,0.0f,0.0f},"levels\\level5",{"0x4378:Levels\\Level5-Bridge","0x437C:RENDER","0x4BB0:MESH:meshes\\hammyjudge","0x4BB4:MESH:meshes\\hammyjudge","0x4BB8:MESH:meshes\\hammyjudge"},5,0x854},
    /* 9=Odd */
    {"Odd",0x004D0BC0,"Board (Odd)","ODD RACE","ODDRACE","Ninja Hamster",{1.0f,0.5f,0.0f},"levels\\level6",{},0,0x855},
    /* 10=Toob */
    {"Toob",0x004D0E78,"Board (Toob)","TOOB RACE","TOOBRACE","Rodenthood",{0.5f,0.5f,1.0f},"levels\\level8",{"0x436C:Levels\\Level8-Spinny","0x4370:Levels\\Level8-Saw","0x4374:Levels\\Level8-Fallout","0x4378:Levels\\Level8-Blockdawg1","0x437C:Levels\\Level8-Blockdawg2"},5,0x856},
    /* 11=Wobbly */
    {"Wobbly",0x004D0D38,"Board (Wobbly)","WOBBLY RACE","WOBBLYRACE","Hamster Chase",{0.62f,0.84f,0.30f},"levels\\level7",{"0x436C:Levels\\Level7-Wobbly1","0x4370:Levels\\Level7-Wobbly2","0x4374:Levels\\Level7-Wobbly3","0x4378:Levels\\Level7-Wobbly4","0x437C:Levels\\Level7-Wobbly5","0x4380:Levels\\Level7-Wobbly6","0x4384:Levels\\Level7-Wobbly7"},7,0x857},
    /* 12=Glass */
    {"Glass",0x004D1F90,"Board (Glass)","GLASS RACE","GLASSRACE","Glass Theme",{1.0f,0.0f,1.0f},"levels\\levelglass",{},0,0},
    /* 13=Sky */
    {"Sky",0x004D0FC8,"Board (Sky)","SKY RACE","SKYRACE","Bucky Break",{0.0f,0.5f,1.0f},"levels\\level9",{"0x436C:MESH:meshes\\skypillar","0x4370:MESH:meshes\\magnifyingglass","0x4384:levels\\level9-popcylinder1","0x4388:levels\\level9-popcylinder2","0x438C:levels\\level9-trapdoor","0x4374:SPRITE:textures\\clouds.png"},6,0x858},
    /* 14=Master */
    {"Master",0x004D12B0,"Board (Master)","MASTER RACE","MASTERRACE","Master Theme",{0.5f,0.5f,0.5f},"levels\\level10",{"0x436C:Levels\\Level2-Bridge","0x4370:TIPPER:","0x4374:Levels\\Level10-2PBridge","0x4378:RENDER","0x4394:Levels\\Level3-Tipper","0x4398:RENDER","0x5410:Levels\\Level10-Bridge1","0x5414:Levels\\Level10-Bridge2","0x5420:levels\\level9-popcylinder1","0x5424:levels\\level9-popcylinder2","0x5840:Levels\\Level8-Blockdawg1","0x5844:Levels\\Level8-Blockdawg2","0x5848:Levels\\Level4-Catapult","0x607C:Levels\\Level3-Gluebie"},14,0x859},
    /* 15=Impossible */
    {"Impossible",0x004D21C0,"Board (Impossible)","IMPOSSIBLE RACE","IMPOSSIBLERACE","Impossible Theme",{1.0f,0.0f,0.0f},"levels\\levelimpossible",{"0x436C:Levels\\LevelImpossible-Looper","0x4370:Levels\\LevelImpossible-Gear","0x4374:Levels\\LevelImpossible-BigGear","0x4378:Levels\\LevelImpossible-Rotator","0x437C:Levels\\LevelImpossible-Pendulum"},5,0},
};

/* ═══════════════════════════════════════════════════════════════════════════
 * Config: per-level feature flags
 * g_objectEnabled[objType][level] = 1 if enabled
 * ═══════════════════════════════════════════════════════════════════════════ */

static int g_objectEnabled[OBJ_COUNT][16] = {{0}};

/* ═══════════════════════════════════════════════════════════════════════════
 * Globals
 * ═══════════════════════════════════════════════════════════════════════════ */

static DWORD g_moduleBase = 0;
typedef void (__thiscall *Sound_Play3D_t)(void *soundChannel, float x, float y, float z);
static Sound_Play3D_t g_SoundPlay3D = NULL;

/* Must be non-static for asm reference */
typedef void (__thiscall *DispatchCollisionEvents_t)(void *board, void *ball, void *collPair);
DispatchCollisionEvents_t g_OriginalDispatch = NULL;

static unsigned char *g_trampoline = NULL;
static char g_configPath[MAX_PATH] = "";
static char g_levelDataPath[MAX_PATH] = "";

/* Pending race index for board constructor thunks */
/* Must be non-static for asm reference */
int g_pendingRaceIndex = 0;

/* ═══════════════════════════════════════════════════════════════════════════
 * String helpers
 * ═══════════════════════════════════════════════════════════════════════════ */

static int my_strnicmp(const char *a, const char *b, int n) {
    int i;
    for (i = 0; i < n; i++) {
        char ca = a[i], cb = b[i];
        if (ca >= 'a' && ca <= 'z') ca -= 32;
        if (cb >= 'a' && cb <= 'z') cb -= 32;
        if (ca != cb) return (unsigned char)ca - (unsigned char)cb;
        if (ca == 0) return 0;
    }
    return 0;
}

static void my_strncpy(char *dst, const char *src, int max) {
    int i;
    for (i = 0; i < max - 1 && src[i]; i++) dst[i] = src[i];
    dst[i] = '\0';
}

static void trim_str(char *s) {
    int len;
    while (*s == ' ' || *s == '\t' || *s == '\r') {
        char *p = s;
        while (*p) { *p = p[1]; p++; }
    }
    len = strlen(s);
    while (len > 0 && (s[len-1] == ' ' || s[len-1] == '\t' || s[len-1] == '\r')) {
        s[--len] = '\0';
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Level identification
 * ═══════════════════════════════════════════════════════════════════════════ */

static int GetCurrentLevel(void *board) {
    DWORD vtable = *(DWORD *)board;
    int i;
    for (i = 1; i <= 15; i++) {
        if (vtable == g_levelVtables[i])
            return i;
    }
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Config parser (LevelFeatures.txt)
 * Format:
 *   [OBJECTS]
 *   BUMPERS = 2 5 8
 *   BRIDGE = 3 14
 *
 * Object name = level numbers (1-15). Empty () = disabled.
 * ═══════════════════════════════════════════════════════════════════════════ */

static void LoadConfig(void) {
    memset(g_objectEnabled, 0, sizeof(g_objectEnabled));
    HANDLE hFile = CreateFileA(g_configPath, GENERIC_READ, FILE_SHARE_READ,
                               NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return;
    DWORD fileSize = GetFileSize(hFile, NULL);
    if (fileSize > 8192) fileSize = 8192;
    char buf[8192];
    DWORD bytesRead = 0;
    ReadFile(hFile, buf, fileSize, &bytesRead, NULL);
    CloseHandle(hFile);
    buf[bytesRead] = '\0';

    /* Skip BOM if present */
    char *start = buf;
    if (bytesRead >= 3 && (unsigned char)start[0] == 0xEF &&
        (unsigned char)start[1] == 0xBB && (unsigned char)start[2] == 0xBF)
        start += 3;

    char *line = start;
    int inObjectsSection = 0;
    while (line < buf + bytesRead) {
        char *eol = line;
        while (*eol && *eol != '\n' && *eol != '\r') eol++;
        char saved = *eol;
        *eol = '\0';
        char *p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\0' || *p == '#' || *p == ';') goto next_line;
        if (p[0] == '[') {
            inObjectsSection = (my_strnicmp(p, "[OBJECTS", 8) == 0);
            goto next_line;
        }
        if (inObjectsSection) {
            /* Find object name match */
            int objType = -1;
            int i;
            for (i = 0; i < OBJ_COUNT; i++) {
                int nameLen = strlen(g_objectNames[i]);
                if (my_strnicmp(p, g_objectNames[i], nameLen) == 0) {
                    /* Make sure next char is '=' or whitespace (not a prefix) */
                    char after = p[nameLen];
                    if (after == '=' || after == ' ' || after == '\t') {
                        objType = i;
                        break;
                    }
                }
            }
            if (objType < 0) goto next_line;

            char *eq = p;
            while (*eq && *eq != '=') eq++;
            if (*eq == '=') {
                eq++;
                while (*eq) {
                    while (*eq == ' ' || *eq == '\t') eq++;
                    if (!*eq) break;
                    if (*eq == '(') {
                        eq++;
                        while (*eq && *eq != ')') eq++;
                        if (*eq == ')') eq++;
                        continue;
                    }
                    if (*eq >= '0' && *eq <= '9') {
                        int levelNum = atoi(eq);
                        if (levelNum >= 1 && levelNum <= 15)
                            g_objectEnabled[objType][levelNum] = 1;
                        while (*eq && *eq >= '0' && *eq <= '9') eq++;
                    } else {
                        eq++;
                    }
                }
            }
        }
    next_line:
        *eol = saved;
        if (*eol == '\r') eol++;
        if (*eol == '\n') eol++;
        line = eol;
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * LevelData.txt parser
 * ═══════════════════════════════════════════════════════════════════════════ */

static void LoadLevelData(void) {
    HANDLE hFile = CreateFileA(g_levelDataPath, GENERIC_READ, FILE_SHARE_READ,
                               NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return;
    DWORD fileSize = GetFileSize(hFile, NULL);
    if (fileSize > 32768) fileSize = 32768;
    char buf[32768];
    DWORD bytesRead = 0;
    ReadFile(hFile, buf, fileSize, &bytesRead, NULL);
    CloseHandle(hFile);
    buf[bytesRead] = '\0';

    /* Skip BOM */
    char *start = buf;
    if (bytesRead >= 3 && (unsigned char)start[0] == 0xEF &&
        (unsigned char)start[1] == 0xBB && (unsigned char)start[2] == 0xBF)
        start += 3;

    int currentLevel = 0;
    char *line = start;
    while (line < buf + bytesRead) {
        char *eol = line;
        while (*eol && *eol != '\n' && *eol != '\r') eol++;
        char saved = *eol;
        *eol = '\0';
        trim_str(line);
        char *p = line;

        if (*p == '\0' || *p == '#' || *p == ';') goto next_ld_line;

        if (p[0] == '[') {
            /* Parse [N] section header */
            char *cl = p + 1;
            while (*cl && *cl != ']') cl++;
            if (*cl == ']') {
                *cl = '\0';
                currentLevel = atoi(p + 1);
                if (currentLevel < 1 || currentLevel > 15) currentLevel = 0;
            }
            goto next_ld_line;
        }

        if (currentLevel >= 1 && currentLevel <= 15) {
            LevelData *ld = &g_levelData[currentLevel];
            char *eq = p;
            while (*eq && *eq != '=') eq++;
            if (*eq != '=') goto next_ld_line;
            *eq = '\0';
            char *key = p;
            char *val = eq + 1;
            trim_str(key);
            trim_str(val);

            if (my_strnicmp(key, "Name", 4) == 0) {
                my_strncpy(ld->name, val, MAX_STR_LEN);
            } else if (my_strnicmp(key, "Vtable", 6) == 0) {
                ld->vtable = (DWORD)strtoul(val, NULL, 0);
                g_levelVtables[currentLevel] = ld->vtable;
            } else if (my_strnicmp(key, "BoardName", 9) == 0) {
                my_strncpy(ld->boardName, val, MAX_STR_LEN);
            } else if (my_strnicmp(key, "RaceTitle", 9) == 0) {
                my_strncpy(ld->raceTitle, val, MAX_STR_LEN);
            } else if (my_strnicmp(key, "RaceData", 8) == 0) {
                my_strncpy(ld->raceData, val, MAX_STR_LEN);
            } else if (my_strnicmp(key, "Music", 5) == 0) {
                my_strncpy(ld->musicName, val, MAX_STR_LEN);
            } else if (my_strnicmp(key, "Color", 5) == 0) {
                float r = 0, g = 0, b = 0;
                if (sscanf(val, "%f %f %f", &r, &g, &b) == 3) {
                    ld->color[0] = r; ld->color[1] = g; ld->color[2] = b;
                }
            } else if (my_strnicmp(key, "MeshPath", 8) == 0) {
                my_strncpy(ld->meshPath, val, MAX_STR_LEN);
            } else if (my_strnicmp(key, "Meshes", 6) == 0) {
                ld->meshCount = 0;
                char *tok = val;
                while (*tok && ld->meshCount < MAX_MESHES_PER_LEVEL) {
                    char *semi = tok;
                    while (*semi && *semi != ';') semi++;
                    char sv = *semi;
                    *semi = '\0';
                    trim_str(tok);
                    if (*tok) {
                        my_strncpy(ld->meshes[ld->meshCount], tok, MAX_STR_LEN);
                        ld->meshCount++;
                    }
                    *semi = sv;
                    if (!sv) break;
                    tok = semi + 1;
                }
            }
        }
    next_ld_line:
        *eol = saved;
        if (*eol == '\r') eol++;
        if (*eol == '\n') eol++;
        line = eol;
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Auto-generate LevelData.txt if missing
 * ═══════════════════════════════════════════════════════════════════════════ */

static void GenerateLevelData(void) {
    HANDLE hFile = CreateFileA(g_levelDataPath, GENERIC_WRITE, 0,
                               NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return;

    const char *header =
        "# LevelData.txt — Per-level configuration for LevelFeatures_Loader\n"
        "# Level numbers: 1=WarmUp 2=Beginner 3=Intermediate 4=Dizzy 5=Tower\n"
        "#   6=Up 7=Neon 8=Expert 9=Odd 10=Toob 11=Wobbly 12=Glass 13=Sky\n"
        "#   14=Master 15=Impossible\n"
        "# Meshes format: 0xOFFSET:PATH;0xOFFSET:PATH;...\n"
        "# Path types: bare=Level_MeshWorldCtor, RENDER=Level_RenderCtor(prev),\n"
        "#   TIPPER:=Level_RenderCtor(prev)+TipperVisual_Attach (bridge visual),\n"
        "#   MESH:path=MeshNode_ctor, SPRITE:path=Sprite_ctor\n\n";
    DWORD written;
    WriteFile(hFile, header, strlen(header), &written, NULL);

    int i;
    for (i = 1; i <= 15; i++) {
        LevelData *ld = &g_levelData[i];
        char line[4096];
        int pos = 0;

        pos += sprintf(line + pos, "[%d]\n", i);
        pos += sprintf(line + pos, "Name=%s\n", ld->name);
        pos += sprintf(line + pos, "Vtable=0x%08lX\n", (unsigned long)ld->vtable);
        pos += sprintf(line + pos, "BoardName=%s\n", ld->boardName);
        pos += sprintf(line + pos, "RaceTitle=%s\n", ld->raceTitle);
        pos += sprintf(line + pos, "RaceData=%s\n", ld->raceData);
        pos += sprintf(line + pos, "Music=%s\n", ld->musicName);
        pos += sprintf(line + pos, "Color=%.3f %.3f %.3f\n", ld->color[0], ld->color[1], ld->color[2]);
        pos += sprintf(line + pos, "MeshPath=%s\n", ld->meshPath);

        pos += sprintf(line + pos, "Meshes=");
        int j;
        for (j = 0; j < ld->meshCount; j++) {
            pos += sprintf(line + pos, "%s%s", j > 0 ? ";" : "", ld->meshes[j]);
        }
        pos += sprintf(line + pos, "\n\n");

        WriteFile(hFile, line, pos, &written, NULL);
    }

    CloseHandle(hFile);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Get config paths (next to this DLL)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void GetConfigPath(void) {
    HMODULE hSelf = NULL;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                       | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                       (LPCSTR)&GetConfigPath, &hSelf);
    if (hSelf && GetModuleFileNameA(hSelf, g_configPath, MAX_PATH)) {
        char *p = strrchr(g_configPath, '\\');
        if (p) {
            strcpy(p + 1, "LevelFeatures.txt");
            strcpy(g_levelDataPath, g_configPath);
            p = strrchr(g_levelDataPath, '\\');
            if (p) strcpy(p + 1, "LevelData.txt");
        }
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Apply bumper bounce physics
 * ═══════════════════════════════════════════════════════════════════════════ */

static void ApplyBumperBounce(void *board, void *ball, void *collPair) {
    DWORD *pair = (DWORD *)collPair;
    DWORD meshBuf = pair[COLL_MESHBUF / 4];
    if (!meshBuf || IsBadReadPtr((void *)meshBuf, 0x870)) return;

    char *name = *(char **)(meshBuf + MESHBUF_NAME);
    if (!name || IsBadReadPtr(name, 9)) return;

    if (my_strnicmp(name, "N:BUMPER", 8) != 0)
        return;

    if (IsBadReadPtr(ball, 0x1A8)) return;
    float posX = *(float *)((char *)ball + BALL_POS_X);
    float posY = *(float *)((char *)ball + BALL_POS_Y);
    float posZ = *(float *)((char *)ball + BALL_POS_Z);

    DWORD *app = *(DWORD **)((char *)board + BOARD_APP_PTR);
    if (app && !IsBadReadPtr(app, 0x500)) {
        DWORD soundList = app[APP_SOUNDFX_LIST / 4];
        if (soundList && g_SoundPlay3D) {
            g_SoundPlay3D((void *)soundList, posX, posY, posZ);
        }
    }

    DWORD phys = *(DWORD *)((char *)ball + BALL_PHYS_PTR);
    if (!phys || IsBadReadPtr((void *)phys, 0xCB0)) return;

    float *velX = (float *)(phys + PHYS_VEL_X);
    float *velY = (float *)(phys + PHYS_VEL_Y);
    float *velZ = (float *)(phys + PHYS_VEL_Z);

    float vx = *velX * BUMPER_VEL_SCALE;
    float vz = *velZ * BUMPER_VEL_SCALE;
    float vy = 0.0f;

    float speedSq = vx * vx + vz * vz;
    if (speedSq < BUMPER_MIN_SPEED * BUMPER_MIN_SPEED) {
        if (speedSq > 0.0001f) {
            float speed = sqrtf(speedSq);
            float scale = BUMPER_MIN_SPEED / speed;
            vx *= scale; vz *= scale;
        }
    }
    speedSq = vx * vx + vz * vz;
    if (speedSq > BUMPER_MAX_SPEED * BUMPER_MAX_SPEED) {
        float speed = sqrtf(speedSq);
        if (speed > 0.0001f) {
            float scale = BUMPER_MAX_SPEED / speed;
            vx *= scale; vz *= scale;
        }
    }

    *velX = vx; *velY = vy; *velZ = vz;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Universal collision logic
 * ═══════════════════════════════════════════════════════════════════════════ */

void __cdecl BumperCollisionLogic(void *board, void *ball, void *collPair) {
    int level = GetCurrentLevel(board);
    if (level == 0) return;
    if (!g_objectEnabled[OBJ_BUMPERS][level]) return;
    ApplyBumperBounce(board, ball, collPair);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Naked thunk for DispatchCollisionEvents hook
 * ═══════════════════════════════════════════════════════════════════════════ */

__attribute__((naked)) static void Hook_DispatchCollisionEvents(void) {
    __asm__ __volatile__(
        "pushl %%ebp\n\t"
        "movl  %%esp, %%ebp\n\t"
        "pushl %%ecx\n\t"
        "pushl %%edx\n\t"

        "pushl 12(%%ebp)\n\t"      /* collPair */
        "pushl 8(%%ebp)\n\t"       /* ball */
        "pushl -4(%%ebp)\n\t"      /* board (saved on stack) */
        "call  _BumperCollisionLogic\n\t"
        "addl  $12, %%esp\n\t"

        "popl  %%edx\n\t"
        "popl  %%ecx\n\t"
        "popl  %%ebp\n\t"

        "jmpl  *_g_OriginalDispatch\n\t"
        :: : "eax", "memory"
    );
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Game function typedefs
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef void *(__cdecl *operator_new_t)(unsigned int size);
typedef void *(__thiscall *Level_MeshWorldCtor_t)(void *mem, void *gfx, const char *meshPath);
typedef void *(__thiscall *Level_RenderCtor_t)(void *mem, void *meshWorld);
typedef void (__thiscall *Level_InitScene_t)(void *board);
typedef void (__thiscall *Scene_CollectByNameFilter_t)(void *meshWorld, char *name, void *destList);
typedef void *(__thiscall *AthenaList_Init_t)(void *this, int capacity);
typedef void *(__thiscall *Board_ctor_t)(void *this, int app);
typedef void (__thiscall *LoadRaceData_t)(void *board, const char *raceName);
typedef int (__cdecl *Vec3_Init_t)(void *out, float x, float y, float z);
typedef void (__cdecl *Matrix_Identity_t)(void *out);
typedef void *(__thiscall *MeshNode_ctor_t)(void *mem, void *gfx, const char *path);
typedef void *(__thiscall *Sprite_ctor_t)(void *mem, void *gfx, const char *path);
typedef void (__thiscall *TipperVisual_Attach_t)(void *renderObj, void *meshWorld);
typedef void (__thiscall *Level_AssignTex_t)(void *board, void *meshWorld);
typedef int (__thiscall *Sound_GetNextChannel_t)(void *soundDevice);
typedef void (__thiscall *Scene_RenderIfVisible_t)(int obj);
typedef void (__thiscall *AthenaList_Append_t)(void *list, int item);
typedef int (__thiscall *AthenaList_GetSize_t)(void *list);
typedef int (__thiscall *AthenaList_GetIterator_t)(void *list);

static operator_new_t g_operatorNew = NULL;
static Level_MeshWorldCtor_t g_LevelMeshWorldCtor = NULL;
static Level_RenderCtor_t g_LevelRenderCtor = NULL;
static Level_InitScene_t g_LevelInitScene = NULL;
static Scene_CollectByNameFilter_t g_CollectByNameFilter = NULL;
static AthenaList_Init_t g_AthenaListInit = NULL;
static Board_ctor_t g_BoardCtor = NULL;
static LoadRaceData_t g_LoadRaceData = NULL;
static Vec3_Init_t g_Vec3Init = NULL;
static Matrix_Identity_t g_MatrixIdentity = NULL;
static MeshNode_ctor_t g_MeshNodeCtor = NULL;
static Sprite_ctor_t g_SpriteCtor = NULL;
static TipperVisual_Attach_t g_TipperVisualAttach = NULL;
static Level_AssignTex_t g_LevelAssignTex = NULL;
static Sound_GetNextChannel_t g_SoundGetNextChannel = NULL;
static Scene_RenderIfVisible_t g_SceneRenderIfVisible = NULL;
static AthenaList_Append_t g_AthenaListAppend = NULL;
static AthenaList_GetSize_t g_AthenaListGetSize = NULL;
static AthenaList_GetIterator_t g_AthenaListGetIterator = NULL;

/* Forward declaration */
static void UniversalPostSetup(void *board);

/* ═══════════════════════════════════════════════════════════════════════════
 * Mesh path table — one per level (for scene loading hook)
 * ═══════════════════════════════════════════════════════════════════════════ */

static const char *g_meshPaths[16] = {
    NULL,                          /* 0 unused */
    "levels\\level1",              /* 1=WarmUp */
    "levels\\levelcascade",        /* 2=Beginner */
    "levels\\level2",              /* 3=Intermediate */
    "levels\\level3",              /* 4=Dizzy */
    "levels\\level4",              /* 5=Tower */
    "levels\\levelup",             /* 6=Up */
    "levels\\leveldark",           /* 7=Neon */
    "levels\\level5",              /* 8=Expert */
    "levels\\level6",              /* 9=Odd */
    "levels\\level8",              /* 10=Toob */
    "levels\\level7",              /* 11=Wobbly */
    "levels\\levelglass",          /* 12=Glass */
    "levels\\level9",              /* 13=Sky */
    "levels\\level10",             /* 14=Master */
    "levels\\levelimpossible",     /* 15=Impossible */
};

/* ═══════════════════════════════════════════════════════════════════════════
 * Universal Board Constructor — REPLACES all 15 per-level board constructors
 *
 * Called via 15 naked thunks that set g_pendingRaceIndex.
 * Does the same steps every LevelBoard_*_ctor does:
 *   1. Board_ctor(mem, app)
 *   2. Set vtable
 *   3. Set board name (+0x868), race title (+0x29B4)
 *   4. Set +0x870 from app+0x1DC
 *   5. Vec3_Init with per-level color, store at +0x1508
 *   6. LoadRaceData(mem, raceName)
 *   7. Set music name (+0x4344)
 *   8. Load extra meshes (per-level)
 *   9. Set unlock flags (per-level)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void LoadExtraMeshes(void *board, LevelData *ld) {
    DWORD app = *(DWORD *)((char *)board + BOARD_APP_PTR);
    if (!app || IsBadReadPtr((void *)app, 0x200)) return;
    void *gfx = *(void **)((char *)app + 0x174);
    if (!gfx) return;

    void *prevMesh = NULL;
    int i;
    for (i = 0; i < ld->meshCount; i++) {
        char *entry = ld->meshes[i];
        if (!*entry) continue;

        /* Parse: 0xOFFSET:PATH */
        DWORD offset = (DWORD)strtoul(entry, NULL, 0);
        char *colon = strchr(entry, ':');
        if (!colon) continue;
        char *path = colon + 1;
        if (!*path) continue;

        void *result = NULL;

        if (my_strnicmp(path, "RENDER", 6) == 0) {
            /* Level_RenderCtor (= CollisionLevel_ctorWithLevel) on previous mesh.
             * Guard against NULL prevMesh — RENDER without a preceding
             * MeshWorld entry would pass NULL and likely crash. */
            if (!prevMesh) continue;
            void *mem = g_operatorNew(0x10D0);
            if (mem) result = g_LevelRenderCtor(mem, prevMesh);
        } else if (my_strnicmp(path, "TIPPER:", 7) == 0) {
            /* Level_RenderCtor + TipperVisual_Attach (bridge/tipper visual).
             * Same NULL prevMesh guard as RENDER. */
            if (!prevMesh) continue;
            void *mem = g_operatorNew(0x10D0);
            if (mem) result = g_LevelRenderCtor(mem, prevMesh);
            if (result && prevMesh)
                g_TipperVisualAttach(result, prevMesh);
        } else if (my_strnicmp(path, "MESH:", 5) == 0) {
            /* MeshNode_ctor */
            void *mem = g_operatorNew(0x18);
            if (mem) result = g_MeshNodeCtor(mem, gfx, path + 5);
        } else if (my_strnicmp(path, "SPRITE:", 7) == 0) {
            /* Sprite_ctor */
            void *mem = g_operatorNew(0xD4);
            if (mem) result = g_SpriteCtor(mem, gfx, path + 7);
        } else {
            /* Level_MeshWorldCtor */
            void *mem = g_operatorNew(0x10D0);
            if (mem) result = g_LevelMeshWorldCtor(mem, gfx, path);
            prevMesh = result;
        }

        if (offset > 0 && offset < UNION_SIZE) {
            *(void **)((char *)board + offset) = result;
        }
    }
}

void __cdecl UniversalBoardCtorLogic(void *mem, int app) {
    int raceIndex = g_pendingRaceIndex;
    char buf[256];
    wsprintfA(buf, "UniversalBoardCtorLogic called: mem=%p app=0x%X raceIndex=%d", mem, app, raceIndex);
    DebugLog(buf);
    if (!mem || raceIndex < 1 || raceIndex > 15) {
        DebugLog("UniversalBoardCtorLogic: invalid params, returning");
        return;
    }
    if (!g_BoardCtor || !g_LoadRaceData) {
        DebugLog("UniversalBoardCtorLogic: function pointers not resolved");
        return;
    }

    LevelData *ld = &g_levelData[raceIndex];

    /* Step 1: Board_ctor(mem, app) — base setup */
    DebugLog("Calling Board_ctor...");
    g_BoardCtor(mem, app);
    DebugLog("Board_ctor done");

    /* Step 2: Set vtable */
    *(DWORD *)mem = g_levelVtables[raceIndex];
    DebugLog("Vtable set");

    /* Step 3: Set board name and race title */
    *(char **)((char *)mem + 0x868) = ld->boardName;
    *(char **)((char *)mem + 0x29B4) = ld->raceTitle;
    DebugLog("Names set");

    /* Step 4: Set +0x870 from app+0x1DC */
    *(DWORD *)((char *)mem + 0x870) = *(DWORD *)(app + 0x1DC);
    DebugLog("+0x870 set");

    /* Step 5: Write per-level color directly to board+0x1508 */
    /* Original game uses Vec3_Init+Matrix_Identity, but we write directly
       to avoid calling convention issues. Board+0x1508 is a 4-float RGBA. */
    *(float *)((char *)mem + 0x1508) = ld->color[0];
    *(float *)((char *)mem + 0x150C) = ld->color[1];
    *(float *)((char *)mem + 0x1510) = ld->color[2];
    *(float *)((char *)mem + 0x1514) = 1.0f;  /* alpha */
    DebugLog("Color set");

    /* Step 6: LoadRaceData(mem, raceName) */
    DebugLog("Calling LoadRaceData...");
    g_LoadRaceData(mem, ld->raceData);
    DebugLog("LoadRaceData done");

    /* Step 7: Set music name */
    *(char **)((char *)mem + 0x4344) = ld->musicName;
    DebugLog("Music set");

    /* Step 8: Load extra meshes */
    if (g_operatorNew && g_LevelMeshWorldCtor) {
        DebugLog("Loading extra meshes...");
        LoadExtraMeshes(mem, ld);
        DebugLog("Extra meshes loaded");
    }

    /* Step 9: Set unlock flags */
    if (ld->unlockFlagOffset) {
        DWORD appVal = *(DWORD *)((char *)mem + BOARD_APP_PTR);
        if (appVal && !IsBadReadPtr((void *)appVal, 0x860)) {
            DWORD diff = *(DWORD *)(appVal + 0x23C);
            if (diff != 0) {
                DWORD gameMode = *(DWORD *)(appVal + 0x220);
                if (gameMode && !IsBadReadPtr((void *)gameMode, 0x20)) {
                    if (*(char *)(gameMode + 0x10) == 0) {
                        *(char *)(appVal + ld->unlockFlagOffset) = 1;
                    }
                }
            }
        }
    }

    /* Special: Impossible sets board+0x4348=1 */
    if (raceIndex == 15) {
        *(char *)((char *)mem + 0x4348) = 1;
    }

    /* Special: Master sets board+0x29C0=0x449C4000 */
    if (raceIndex == 14) {
        *(DWORD *)((char *)mem + 0x29C0) = 0x449C4000;
    }

    /* Special: Dizzy (4) — AthenaList_Init, sound channel, swirl state */
    if (raceIndex == 4) {
        /* AthenaList_Init at board+0x4378 and board+0x4790 */
        if (g_AthenaListInit) {
            g_AthenaListInit((void *)((char *)mem + 0x4378), 0);
            g_AthenaListInit((void *)((char *)mem + 0x4790), 0);
        }
        /* Sound channel for waterwheel */
        DWORD appVal = *(DWORD *)((char *)mem + BOARD_APP_PTR);
        if (appVal && !IsBadReadPtr((void *)appVal, 0x500) && g_SoundGetNextChannel) {
            DWORD soundDevice = *(DWORD *)(appVal + 0x490);
            if (soundDevice) {
                int channel = g_SoundGetNextChannel((void *)soundDevice);
                *(int *)((char *)mem + 0x4BDC) = channel;
                if (channel && g_SceneRenderIfVisible)
                    g_SceneRenderIfVisible(channel);
            }
        }
        /* Swirl state init */
        *(DWORD *)((char *)mem + 0x4BC0) = 0;
        *(DWORD *)((char *)mem + 0x4BD8) = 0;
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Universal Board Constructor Entry — naked, handles __thiscall convention
 * ECX=mem, [ESP+4]=app, returns board ptr in EAX
 * ═══════════════════════════════════════════════════════════════════════════ */

__attribute__((naked)) void UniversalBoardCtorEntry(void) {
    __asm__ __volatile__(
        "pushl %%ebp\n\t"
        "movl  %%esp, %%ebp\n\t"
        "pushl %%ecx\n\t"          /* save mem */
        "pushl 8(%%ebp)\n\t"       /* app */
        "pushl %%ecx\n\t"          /* mem (from saved) */
        "call  _UniversalBoardCtorLogic\n\t"
        "addl  $8, %%esp\n\t"
        "popl  %%ecx\n\t"          /* restore mem to ECX */
        "movl  %%ecx, %%eax\n\t"   /* return mem in EAX */
        "popl  %%ebp\n\t"
        "ret   $4\n\t"             /* __thiscall: callee cleans 4 bytes */
        :: : "eax", "ecx", "edx", "memory"
    );
}

/* ═══════════════════════════════════════════════════════════════════════════
 * 15 naked thunks — each sets race index then JMPs to UniversalBoardCtorEntry
 * ═══════════════════════════════════════════════════════════════════════════ */

#define DEFINE_LEVEL_THUNK(NUM) \
__attribute__((naked)) static void Thunk_Level##NUM(void) { \
    __asm__ __volatile__( \
        "movl $" #NUM ", _g_pendingRaceIndex\n\t" \
        "jmp _UniversalBoardCtorEntry\n\t" \
    ); \
}

DEFINE_LEVEL_THUNK(1)
DEFINE_LEVEL_THUNK(2)
DEFINE_LEVEL_THUNK(3)
DEFINE_LEVEL_THUNK(4)
DEFINE_LEVEL_THUNK(5)
DEFINE_LEVEL_THUNK(6)
DEFINE_LEVEL_THUNK(7)
DEFINE_LEVEL_THUNK(8)
DEFINE_LEVEL_THUNK(9)
DEFINE_LEVEL_THUNK(10)
DEFINE_LEVEL_THUNK(11)
DEFINE_LEVEL_THUNK(12)
DEFINE_LEVEL_THUNK(13)
DEFINE_LEVEL_THUNK(14)
DEFINE_LEVEL_THUNK(15)

/* ═══════════════════════════════════════════════════════════════════════════
 * Patch all 15 CALL LevelBoard_*_ctor instructions
 * ═══════════════════════════════════════════════════════════════════════════ */

static const DWORD g_ctorCallRVAs[15] = {
    0x0002712C,  /* 1=WarmUp */
    0x0002715D,  /* 2=Beginner */
    0x0002718E,  /* 3=Intermediate */
    0x000271BF,  /* 4=Dizzy */
    0x000271F0,  /* 5=Tower */
    0x00027221,  /* 6=Up */
    0x00027252,  /* 7=Neon */
    0x00027283,  /* 8=Expert */
    0x000272B4,  /* 9=Odd */
    0x000272E5,  /* 10=Toob */
    0x00027316,  /* 11=Wobbly */
    0x00027347,  /* 12=Glass */
    0x00027374,  /* 13=Sky */
    0x0002739E,  /* 14=Master */
    0x000273C8,  /* 15=Impossible */
};

static void *g_thunkPtrs[16] = {
    NULL,
    Thunk_Level1, Thunk_Level2, Thunk_Level3, Thunk_Level4,
    Thunk_Level5, Thunk_Level6, Thunk_Level7, Thunk_Level8,
    Thunk_Level9, Thunk_Level10, Thunk_Level11, Thunk_Level12,
    Thunk_Level13, Thunk_Level14, Thunk_Level15,
};

static void InstallBoardCtorHooks(void) {
    int i;
    for (i = 0; i < 15; i++) {
        unsigned char *site = (unsigned char *)(g_moduleBase + g_ctorCallRVAs[i]);
        if (IsBadReadPtr(site, 5)) continue;
        if (site[0] != 0xE8) continue;  /* verify CALL opcode */

        DWORD target = (DWORD)g_thunkPtrs[i + 1];
        DWORD rel32 = target - ((DWORD)site + 5);

        DWORD oldProtect;
        VirtualProtect(site, 5, PAGE_EXECUTE_READWRITE, &oldProtect);
        *(DWORD *)(site + 1) = rel32;
        VirtualProtect(site, 5, oldProtect, &oldProtect);
        FlushInstructionCache(GetCurrentProcess(), site, 5);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * InitBridge — replicates LevelBoard_Intermediate_ctor bridge setup
 *
 * Steps (from Ghidra decompilation of 0x0041cb20):
 *   1. operator_new(0x10d0) → Level_MeshWorldCtor(mem, gfx, "Levels\\Level2-Bridge") → board+0x436C
 *   2. operator_new(0x10d0) → Level_RenderCtor(mem, meshWorld) → board+0x4370
 *   3. TipperVisual_Attach(renderObj, meshWorld)
 *   4. board+0x4380 = 0x42340000 (float 45.0)
 *   5. board+0x4384 = 0
 *   6. board+0x4388 = 0x32 (50)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void InitBridge(void *board) {
    if (!g_operatorNew || !g_LevelMeshWorldCtor || !g_LevelRenderCtor ||
        !g_TipperVisualAttach) return;

    /* Don't create a second bridge if one is already loaded at +0x436C.
     * Master (14) has bridge in its mesh list — InitBridge would double-allocate. */
    if (*(void **)((char *)board + BRIDGE_MESHWORLD) != NULL) {
        DebugLog("InitBridge: bridge already exists at +0x436C, skipping");
        return;
    }

    DWORD app = *(DWORD *)((char *)board + BOARD_APP_PTR);
    if (!app || IsBadReadPtr((void *)app, 0x200)) return;
    void *gfx = *(void **)((char *)app + 0x174);
    if (!gfx) return;

    /* Step 1: MeshWorld */
    void *meshMem = g_operatorNew(0x10D0);
    if (!meshMem) return;
    void *meshWorld = g_LevelMeshWorldCtor(meshMem, gfx, "Levels\\Level2-Bridge");
    *(void **)((char *)board + BRIDGE_MESHWORLD) = meshWorld;

    /* Step 2: RenderObj */
    void *renderMem = g_operatorNew(0x10D0);
    void *renderObj = NULL;
    if (renderMem) {
        renderObj = g_LevelRenderCtor(renderMem, meshWorld);
    }
    *(void **)((char *)board + BRIDGE_RENDEROBJ) = renderObj;

    /* Step 3: TipperVisual_Attach */
    if (renderObj && meshWorld) {
        g_TipperVisualAttach(renderObj, meshWorld);
    }

    /* Steps 4-6: Bridge config values */
    *(DWORD *)((char *)board + BRIDGE_PARAM1) = 0x42340000;  /* 45.0f */
    *(DWORD *)((char *)board + BRIDGE_PARAM2) = 0;
    *(DWORD *)((char *)board + BRIDGE_PARAM3) = 0x32;       /* 50 */
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Feature block: Bridge Animation (Intermediate)
 * 4-state machine: wait → tilt down → wait → tilt back
 * Replicates FUN_0041CC90 (Intermediate Board_Update)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void Feature_BridgeAnimation(void *board, int level) {
    if (!g_SceneUpdate) return;

    /* Check ball list size — skip if race ending */
    int ballCount = g_AthenaListGetSizeUpd((void *)((char *)board + 0xD8B));
    DWORD app = *(DWORD *)((char *)board + BOARD_APP_PTR);
    if (ballCount == 1 && app && !IsBadReadPtr((void *)app, 0x600)) {
        DWORD ball = *(DWORD *)(app + APP_BALL_PTR);
        if (ball && !IsBadReadPtr((void *)ball, 0x200) && *(char *)(ball + 0x14C))
            return;
    }

    /* Master uses different offsets for bridge (via vtable[0x90] = 0x421400) */
    int base = (level == 14) ? 0x437C : 0x10DD;  /* pivot X offset */
    int renderOfs = (level == 14) ? 0x10DB : 0x10DB;
    int angleOfs = base + 3;    /* 0x10E0 or 0x4388 */
    int stateOfs = base + 4;    /* 0x10E1 or 0x438C */
    int counterOfs = base + 5; /* 0x10E2 or 0x4390 */

    DWORD renderObj = *(DWORD *)((char *)board + renderOfs);
    if (!renderObj) return;

    int state = *(int *)((char *)board + stateOfs);

    switch (state) {
    case 0: /* Countdown */
        {
            int c = *(int *)((char *)board + counterOfs) - 1;
            *(int *)((char *)board + counterOfs) = c;
            if (c < 1)
                *(int *)((char *)board + stateOfs) = 1;
        }
        break;

    case 1: /* Tilt down */
        {
            float angle = *(float *)((char *)board + angleOfs) - 3.0f;
            *(float *)((char *)board + angleOfs) = angle;
            if (angle < 0.0f) {
                *(float *)((char *)board + angleOfs) = 0.0f;
                *(int *)((char *)board + counterOfs) = 0x7D; /* 125 */
                *(int *)((char *)board + stateOfs) = 2;
                /* Play sound + apply velocity to balls */
                if (app && !IsBadReadPtr((void *)app, 0x500) && g_SoundPlay3DUpd) {
                    DWORD snd = *(DWORD *)(app + APP_SOUNDFX_47C);
                    if (snd) {
                        float px = *(float *)((char *)board + base);
                        float py = *(float *)((char *)board + base + 1);
                        float pz = *(float *)((char *)board + base + 2);
                        g_SoundPlay3DUpd((void *)snd, px, py, pz);
                    }
                }
                if (g_SceneForEachBallSetVelocity && g_Vec3CopyUpd) {
                    float pivot[3];
                    g_Vec3CopyUpd(pivot, (float *)((char *)board + base));
                    g_SceneForEachBallSetVelocity(board,
                        pivot[1], pivot[2], 0.5f);
                }
            }
        }
        break;

    case 2: /* Wait */
        {
            int c = *(int *)((char *)board + counterOfs) - 1;
            *(int *)((char *)board + counterOfs) = c;
            if (c < 1)
                *(int *)((char *)board + stateOfs) = 3;
        }
        break;

    case 3: /* Tilt back up */
        {
            float angle = *(float *)((char *)board + angleOfs) + 0.5f;
            *(float *)((char *)board + angleOfs) = angle;
            if (angle >= 45.0f) {
                *(float *)((char *)board + angleOfs) = 45.0f;
                *(int *)((char *)board + counterOfs) = 0x4B; /* 75 */
                *(int *)((char *)board + stateOfs) = 0;
            }
            /* Play sound at intervals */
            /* Transform ball positions through bridge rotation */
            if (g_TimerInit && g_TimerCleanup && g_GfxScaleZ &&
                g_GfxSetPosition && g_MatrixTransformVec3) {
                char timerBuf[68];
                g_TimerInit(timerBuf);
                g_GfxScaleZ(-*(float *)((char *)board + angleOfs));
                g_GfxSetPosition(
                    *(float *)((char *)board + base),
                    *(float *)((char *)board + base + 1),
                    *(float *)((char *)board + base + 2));
                /* Call render object vtable[0x58] and [0x54] */
                DWORD *renderVtbl = *(DWORD **)renderObj;
                if (renderVtbl) {
                    void (__fastcall *fn58)(DWORD) = (void (__fastcall *)(DWORD))renderVtbl[0x16];
                    void (__fastcall *fn54)(char *) = (void (__fastcall *)(char *))renderVtbl[0x15];
                    if (fn58) fn58((DWORD)renderObj);
                    if (fn54) fn54(timerBuf);
                }
                g_TimerCleanup(timerBuf);
            }
        }
        break;
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Feature block: Swirl Zones (Dizzy / Master)
 * Proximity check → velocity scale + tar bubbles + mesh rotation
 * Replicates DizzyBoard_Update (0x41D510) / Master Board_Update (0x420DA0)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void Feature_SwirlZones(void *board, int level) {
    if (!g_AthenaListGetIteratorUpd || !g_OperatorNewUpd) return;

    DWORD app = *(DWORD *)((char *)board + BOARD_APP_PTR);
    if (!app || IsBadReadPtr((void *)app, 0x600)) return;

    /* Determine offsets based on level */
    int swirlListOfs = (level == 14) ? BRD_SWIRL_LIST_M : BRD_SWIRL_LIST;
    int tarListOfs = (level == 14) ? BRD_TARBUBBLE_LIST_M : BRD_TARBUBBLE_LIST;

    /* TarBubble particle creation (every 10 frames via RNG==10 check) */
    if (g_CreateTarBubble && g_AthenaListAppendUpd) {
        if (g_RNG && g_RNG((void *)0x4F7360, 0x14, 0) == 10) {
            void *tar = g_OperatorNewUpd(0x1C);
            if (tar) {
                g_CreateTarBubble(tar, app, (int)((char *)board + tarListOfs));
                g_AthenaListAppendUpd((void *)((char *)board + 0xEC0), (int)tar);
            }
        }
    }

    /* Swirl zone processing: iterate ball list, check proximity to swirl zones */
    int ballIter = g_AthenaListGetIteratorUpd((void *)((char *)board + 0xA75));
    *(int *)((char *)board + 0xA77 + ballIter * 4) = 0;
    int ballCount = *(int *)((char *)board + 0xA76);
    int ballIdx = 0;
    if (ballCount > 0) {
        ballIdx = *(int *)(*(int *)((char *)board + 0xB78));
        *(int *)((char *)board + 0xA77 + ballIter * 4) = 1;
    }

    while (ballIdx) {
        /* Skip balls in tar (ball+0x2CC != 0) — they get sinking logic */
        char inTar = *(char *)(ballIdx + BALL_IN_TAR_OFS);
        if (!inTar) {
            /* Check proximity to each swirl zone */
            int zoneIter = g_AthenaListGetIteratorUpd((void *)((char *)board + swirlListOfs));
            *(int *)((char *)board + swirlListOfs + 2 + zoneIter * 4) = 0;
            int zoneCount = *(int *)((char *)board + swirlListOfs + 1);
            int zoneIdx = 0;
            if (zoneCount > 0) {
                zoneIdx = *(int *)(*(int *)((char *)board + swirlListOfs + 0x103));
                *(int *)((char *)board + swirlListOfs + 2 + zoneIter * 4) = 1;
            }

            while (zoneIdx) {
                if (*(int *)(ballIdx + 0x18) >= 0) {
                    float ballX = *(float *)(ballIdx + BALL_POS_X_OFS);
                    float ballY = *(float *)(ballIdx + BALL_POS_Y_OFS);
                    float ballZ = *(float *)(ballIdx + BALL_POS_Z_OFS);
                    float zoneX = *(float *)(zoneIdx + 0x10E0);
                    float zoneY = *(float *)(zoneIdx + 0x10E4);
                    float zoneZ = *(float *)(zoneIdx + 0x10E8);
                    float radius = *(float *)(zoneIdx + 0x1100) * 60.0f;

                    float dx = zoneX - ballX;
                    float dy = zoneY - ballY;
                    float dz = zoneZ - ballZ;
                    float distSq = dx*dx + dy*dy + dz*dz;

                    if (distSq < radius * radius) {
                        /* Scale ball velocity */
                        DWORD phys = *(DWORD *)(ballIdx + BALL_PHYS_PTR_OFS);
                        if (phys && !IsBadReadPtr((void *)phys, 0xCB0)) {
                            float vx = *(float *)(phys + BALL_PHYS_VEL_X);
                            float vy = *(float *)(phys + BALL_PHYS_VEL_Y);
                            float vz = *(float *)(phys + BALL_PHYS_VEL_Z);
                            float speedSq = vx*vx + vy*vy + vz*vz;
                            float speed = 0.0f;
                            if (speedSq > 0.0f) speed = sqrtf(speedSq);
                            if (speed > 0.0f) {
                                float scale = (speed * 0.95f) / speed;
                                vx *= scale; vy *= scale; vz *= scale;
                                *(float *)(phys + BALL_PHYS_VEL_X) = vx;
                                *(float *)(phys + BALL_PHYS_VEL_Y) = vy;
                                *(float *)(phys + BALL_PHYS_VEL_Z) = vz;
                            }
                            /* Play sound + spawn particles on first contact */
                            if (!*(char *)(ballIdx + BALL_TAR_SOUND_FLAG)) {
                                if (g_SoundPlay3DUpd) {
                                    DWORD snd = *(DWORD *)(app + APP_SOUNDFX_484);
                                    if (snd)
                                        g_SoundPlay3DUpd((void *)snd, ballX, ballY, ballZ);
                                }
                                /* Spawn 3 particles */
                                int p;
                                for (p = 0; p < 3 && g_OperatorNewUpd && g_AthenaListAppendUpd; p++) {
                                    float *part = (float *)g_OperatorNewUpd(0x14);
                                    if (part) {
                                        /* Random direction (normalized) */
                                        if (g_RNG) {
                                            part[0] = (float)g_RNG((void *)0x4F7360, 100, 1);
                                            part[1] = (float)g_RNG((void *)0x4F7360, 100, 1);
                                            part[2] = (float)g_RNG((void *)0x4F7360, 100, 1);
                                            float lenSq = part[0]*part[0] + part[1]*part[1] + part[2]*part[2];
                                            if (lenSq > 0.0f) {
                                                float len = sqrtf(lenSq);
                                                float s = 1.0f / len;
                                                part[0] *= s; part[1] *= s; part[2] *= s;
                                            }
                                        }
                                        int particleListSize = g_AthenaListGetSizeUpd(
                                            (void *)(ballIdx + 0x810));
                                        if (particleListSize < 30) {
                                            g_AthenaListAppendUpd(
                                                (void *)(ballIdx + 0x810), (int)part);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                int next = *(int *)((char *)board + swirlListOfs + 2 + zoneIter * 4);
                if (*(int *)((char *)board + swirlListOfs + 1) <= next) break;
                zoneIdx = *(int *)(*(int *)((char *)board + swirlListOfs + 0x103) + next * 4);
                *(int *)((char *)board + swirlListOfs + 2 + zoneIter * 4) = next + 1;
            }
        } else {
            /* Ball is in tar — sink and potentially remove */
            *(float *)(ballIdx + BALL_POS_Y_OFS) -= 0.25f;
            /* Splash particle */
            if (g_CreateSplashParticle && g_AthenaListAppendUpd && g_RNG) {
                int rng1 = g_RNG((void *)0x4F7360, 0xF, 0);
                if (rng1 == 1) {
                    float rx = (float)g_RNG((void *)0x4F7360, 100, 1);
                    float rz = (float)g_RNG((void *)0x4F7360, 100, 1);
                    float lenSq = rx*rx + rz*rz;
                    float len = (lenSq > 0.0f) ? sqrtf(lenSq) : 0.0f;
                    float scale = (len > 0.0f) ?
                        (10.0f + *(float *)(ballIdx + 0x284)) / len : 0.0f;
                    rx *= scale; rz *= scale;
                    void *splash = g_OperatorNewUpd(0x1C);
                    if (splash) {
                        g_CreateSplashParticle(splash, app,
                            rz + *(float *)(ballIdx + BALL_POS_X_OFS),
                            *(DWORD *)(ballIdx + 0x2D0),
                            rx + *(float *)(ballIdx + BALL_POS_Z_OFS));
                        g_AthenaListAppendUpd((void *)((char *)board + 0xEC0), (int)splash);
                    }
                }
            }
            /* Remove ball if below threshold */
            float threshold = *(float *)(ballIdx + 0x2D0) -
                *(float *)(ballIdx + 0x284) * 2.5f;
            if (*(float *)(ballIdx + BALL_POS_Y_OFS) < threshold) {
                if (g_RemoveBall) g_RemoveBall(ballIdx);
            }
        }
        int nextBall = *(int *)((char *)board + 0xA77 + ballIter * 4);
        if (*(int *)((char *)board + 0xA76) <= nextBall) break;
        ballIdx = *(int *)(*(int *)((char *)board + 0xB78) + nextBall * 4);
        *(int *)((char *)board + 0xA77 + ballIter * 4) = nextBall + 1;
    }

    /* Dizzy-only: mesh rotation (Master doesn't rotate meshes) */
    if (level != 14 && g_TimerInit && g_TimerCleanup && g_GfxScaleY &&
        g_GfxSetPosition && g_Matrix44Zero) {
        char timerBuf[68];
        g_TimerInit(timerBuf);

        /* Primary swirl mesh rotation (Gfx_ScaleY) */
        float angle1 = *(float *)((char *)board + BRD_SWIRL1_ANGLE) - 0.5f;
        *(float *)((char *)board + BRD_SWIRL1_SPEED) = 0.5f;
        *(float *)((char *)board + BRD_SWIRL1_ANGLE) = angle1;
        g_Matrix44Zero((int *)timerBuf);
        g_GfxScaleY(angle1);
        g_GfxSetPosition(
            *(float *)((char *)board + BRD_SWIRL1_POS_X),
            *(float *)((char *)board + BRD_SWIRL1_POS_Y),
            *(float *)((char *)board + BRD_SWIRL1_POS_Z));
        /* Call mesh vtable[0x58]+[0x54] */
        DWORD mesh1 = *(DWORD *)((char *)board + BRD_SWIRL_MESH1);
        if (mesh1) {
            DWORD *vtbl = *(DWORD **)mesh1;
            if (vtbl) {
                void (__fastcall *fn58)(DWORD) = (void (__fastcall *)(DWORD))vtbl[0x16];
                void (__fastcall *fn54)(char *) = (void (__fastcall *)(char *))vtbl[0x15];
                if (fn58) fn58((DWORD)mesh1);
                if (fn54) fn54(timerBuf);
            }
        }

        g_TimerCleanup(timerBuf);
    }

    /* Dizzy: secondary swirl mesh rotation (Gfx_ScaleX) */
    if (level != 14 && g_TimerInit && g_TimerCleanup && g_GfxScaleX &&
        g_GfxSetPosition && g_Matrix44Zero) {
        char timerBuf[68];
        g_TimerInit(timerBuf);

        float swirlSpeed = (*(int *)(app + APP_DIFFICULTY) == 0) ? 0.25f : 0.5f;
        *(float *)((char *)board + BRD_SWIRL2_ANGLE) =
            *(float *)((char *)board + BRD_SWIRL2_ANGLE) + swirlSpeed;
        g_Matrix44Zero((int *)timerBuf);
        g_GfxScaleX(*(float *)((char *)board + BRD_SWIRL2_ANGLE));
        g_GfxSetPosition(
            *(float *)((char *)board + BRD_SWIRL2_POS_X),
            *(float *)((char *)board + BRD_SWIRL2_POS_Y),
            *(float *)((char *)board + BRD_SWIRL2_POS_Z));
        DWORD mesh2 = *(DWORD *)((char *)board + BRD_SWIRL_MESH2);
        if (mesh2) {
            DWORD *vtbl = *(DWORD **)mesh2;
            if (vtbl) {
                void (__fastcall *fn58)(DWORD) = (void (__fastcall *)(DWORD))vtbl[0x16];
                void (__fastcall *fn54)(char *) = (void (__fastcall *)(char *))vtbl[0x15];
                if (fn58) fn58((DWORD)mesh2);
                if (fn54) fn54(timerBuf);
            }
        }

        g_TimerCleanup(timerBuf);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Feature block: Windmill (Tower)
 * Rotation + 4-state machine (spin up → creak → spin down → pause)
 * Replicates FUN_0041E760 (Tower Board_Update)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void Feature_Windmill(void *board, int level) {
    DWORD app = *(DWORD *)((char *)board + BOARD_APP_PTR);
    if (!app || IsBadReadPtr((void *)app, 0x500)) return;

    /* Accumulate rotation */
    float rotSpeed = (*(int *)(app + APP_DIFFICULTY) == 0) ? 0.25f : 1.0f;
    *(float *)((char *)board + BRD_WM_ANGLE) =
        *(float *)((char *)board + BRD_WM_ANGLE) + rotSpeed;

    /* Play creak sound every 90° (mod 0x5A == 0x2D) */
    {
        int angleInt = *(int *)((char *)board + BRD_WM_ANGLE);
        if (angleInt % 0x5A == 0x2D) {
            if (g_SoundPlay3DUpd) {
                DWORD snd = *(DWORD *)(app + 0x4A4);
                if (snd) {
                    g_SoundPlay3DUpd((void *)snd,
                        *(float *)((char *)board + BRD_WM_POS_X),
                        *(float *)((char *)board + BRD_WM_POS_Y),
                        *(float *)((char *)board + BRD_WM_POS_Z));
                }
            }
        }
    }

    /* Render windmill mesh with rotation */
    if (g_TimerInit && g_TimerCleanup && g_GfxScaleY && g_GfxSetPosition) {
        char timerBuf[68];
        g_TimerInit(timerBuf);
        g_GfxScaleY(*(float *)((char *)board + BRD_WM_ANGLE));
        g_GfxSetPosition(
            *(float *)((char *)board + BRD_WM_POS_X),
            *(float *)((char *)board + BRD_WM_POS_Y),
            *(float *)((char *)board + BRD_WM_POS_Z));
        DWORD renderObj = *(DWORD *)((char *)board + BRD_WM_RENDER);
        if (renderObj) {
            DWORD *vtbl = *(DWORD **)renderObj;
            if (vtbl) {
                void (__fastcall *fn58)(DWORD) = (void (__fastcall *)(DWORD))vtbl[0x16];
                void (__fastcall *fn54)(char *) = (void (__fastcall *)(char *))vtbl[0x15];
                if (fn58) fn58((DWORD)renderObj);
                if (fn54) fn54(timerBuf);
            }
        }
        g_TimerCleanup(timerBuf);
    }

    /* 4-state machine for windmill speed control */
    int wmState = *(int *)((char *)board + BRD_WM_STATE);
    switch (wmState) {
    case 0: /* Spin up (decelerate) */
        {
            float speed = *(float *)((char *)board + BRD_WM_SPEED);
            if (speed == 0.0f) speed = 0.25f;  /* 0x3E800000 */
            speed *= 1.2;  /* double constant */
            *(float *)((char *)board + BRD_WM_SPEED) = speed;
            if (speed > 25.0f) {
                *(float *)((char *)board + BRD_WM_SPEED) = 25.0f;  /* 0x41C80000 */
                *(int *)((char *)board + BRD_WM_STATE) = 1;
                *(int *)((char *)board + BRD_WM_COUNTER) = 0x19;
                *(float *)((char *)board + BRD_WM_DECAY_VAL) = 50.0f;  /* 0x42480000 */
            }
        }
        break;
    case 1: /* Countdown then play sound */
        {
            int c = *(int *)((char *)board + BRD_WM_COUNTER) - 1;
            *(int *)((char *)board + BRD_WM_COUNTER) = c;
            if (c < 1) {
                *(int *)((char *)board + BRD_WM_STATE) = 2;
                if (g_SoundPlay3DUpd) {
                    DWORD snd = *(DWORD *)(app + 0x4A8);
                    if (snd) {
                        g_SoundPlay3DUpd((void *)snd,
                            *(float *)((char *)board + 0x10E5),
                            *(float *)((char *)board + 0x10E6),
                            *(float *)((char *)board + 0x10E7));
                    }
                }
            }
        }
        break;
    case 2: /* Spin down */
        {
            float speed = *(float *)((char *)board + BRD_WM_SPEED) * 0.25f;
            *(float *)((char *)board + BRD_WM_SPEED) = speed;
            if (speed < 1.0f) {
                *(float *)((char *)board + BRD_WM_SPEED) = 0.0f;
                if (g_RNG) {
                    int rng = g_RNG((void *)0x4F7360, 2, 0);
                    if (rng != 0) {
                        *(int *)((char *)board + BRD_WM_STATE) = 3;
                        int rng2 = g_RNG((void *)0x4F7360, 100, 0);
                        *(int *)((char *)board + BRD_WM_COUNTER) = rng2 + 100;
                    } else {
                        *(int *)((char *)board + BRD_WM_STATE) = 0;
                    }
                }
            }
        }
        break;
    case 3: /* Pause + decay */
        {
            float decay = *(float *)((char *)board + BRD_WM_DECAY_VAL) - 2.0f;
            if (decay < 0.0f) decay = 0.0f;
            *(float *)((char *)board + BRD_WM_DECAY_VAL) = decay;
            int c = *(int *)((char *)board + BRD_WM_COUNTER) - 1;
            *(int *)((char *)board + BRD_WM_COUNTER) = c;
            if (c < 1)
                *(int *)((char *)board + BRD_WM_STATE) = 0;
        }
        break;
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Feature block: BadBall Spawner (Odd)
 * Timer-based spawning of BadBall enemies
 * Replicates FUN_0041EE80 (Odd Board_Update)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void Feature_BadBallSpawner(void *board, int level) {
    if (!g_RNG || !g_BadBallCtor || !g_OperatorNewUpd || !g_AthenaListAppendUpd) return;

    char spawnFlag = *(char *)((char *)board + BRD_BB_FLAG);
    if (!spawnFlag) return;

    int counter = *(int *)((char *)board + BRD_BB_COUNTER) - 1;
    *(int *)((char *)board + BRD_BB_COUNTER) = counter;
    if (counter >= 1) return;

    /* Check limits */
    int ballCount = g_AthenaListGetSizeUpd((void *)((char *)board + 0xA75));
    int totalSpawned = *(int *)((char *)board + BRD_BB_TOTAL);
    if (ballCount >= 10 || totalSpawned >= 100) return;

    /* Set next spawn timer */
    int nextDelay = g_RNG((void *)0x4F7360, 0x19, 0);
    *(int *)((char *)board + BRD_BB_COUNTER) = nextDelay + 0x19;

    /* Pick random spawn position (3-slot table) */
    int posIdx;
    do {
        posIdx = g_RNG((void *)0x4F7360, 3, 0);
    } while (posIdx == *(int *)((char *)board + BRD_BB_LAST_IDX));
    *(int *)((char *)board + BRD_BB_LAST_IDX) = posIdx;

    float *posTable = (float *)((char *)board + BRD_BB_POS_TABLE);
    float spawnX = posTable[posIdx * 3];
    float spawnY = posTable[posIdx * 3 + 1];
    float spawnZ = posTable[posIdx * 3 + 2];

    /* Play spawn sound */
    DWORD app = *(DWORD *)((char *)board + BOARD_APP_PTR);
    if (app && !IsBadReadPtr((void *)app, 0x600) && g_SoundPlay3DUpd) {
        DWORD snd = *(DWORD *)(app + 0x4D0);
        if (snd) g_SoundPlay3DUpd((void *)snd, spawnX, spawnY, spawnZ);
    }

    /* Create BadBall */
    void *mem = g_OperatorNewUpd(0xC64);
    if (!mem) return;
    void *badball = g_BadBallCtor(mem, (int)board);
    if (!badball) return;

    /* Set trajectory */
    g_BallSetTrajectory(badball, 0x41EF8A, spawnX, spawnY, spawnZ, 0.0f);

    /* Set badball fields */
    int *bb = (int *)badball;
    bb[0x5A] = (int)(spawnY + 24.0f);
    bb[0x59] = (int)spawnX;
    bb[0x5B] = (int)spawnZ;
    bb[0x9E] = 0x3F000000;  /* 0.5f */
    bb[0x9F] = 0x3DCCCCCD;  /* 0.1f */
    bb[0xA1] = 0x41C00000;  /* 24.0f */
    bb[0x62] = 0x40A00000;  /* 5.0f */
    bb[6] = -1;

    /* Random direction */
    float dirX, dirZ, lenSq;
    do {
        dirX = (float)g_RNG((void *)0x4F7360, 0x19, 1);
        dirZ = (float)g_RNG((void *)0x4F7360, 0x32, 1);
        lenSq = dirX * dirX + dirZ * dirZ;
    } while (lenSq <= 0.0f || sqrtf(lenSq) == 0.0f);

    /* Normalize and scale to 2.5 */
    float len = sqrtf(lenSq);
    dirX = (dirX / len) * 2.5f;
    dirZ = (dirZ / len) * 2.5f;

    /* Set velocity via Ball_SetVec3AtOffset */
    float velVec[3] = { dirX, 12.0f, dirZ };
    if (g_BallSetVec3AtOffset) {
        g_BallSetVec3AtOffset((void *)bb[0x69], velVec);
        /* Call vtable[0x10] on the physics object */
        DWORD physObj = bb[0x69];
        if (physObj) {
            DWORD *vtbl = *(DWORD **)physObj;
            if (vtbl) {
                void (__fastcall *fn10)(DWORD) = (void (__fastcall *)(DWORD))vtbl[0x4];
                if (fn10) fn10((DWORD)physObj);
            }
        }
    }

    g_AthenaListAppendUpd((void *)((char *)board + 0xA75), (int)badball);
    *(int *)((char *)board + BRD_BB_TOTAL) = totalSpawned + 1;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Feature block: Bumper Lit Decay (Beginner / Toob / Master)
 * Decays 4-8 float "lit" values by 0.05/frame, clamped to 0.0
 * Replicates Beginner slot[19] / Toob slot[19] / Master vtable[0x94]
 * ═══════════════════════════════════════════════════════════════════════════ */

static void Feature_BumperDecay(void *board, int level) {
    int baseOfs;
    int count;

    if (level == 2) {        /* Beginner */
        baseOfs = BRD_BUMPER_DECAY_BEG;
        count = 8;
    } else if (level == 10) { /* Toob */
        baseOfs = BRD_BUMPER_DECAY_TOOB;
        count = 8;
    } else if (level == 14) { /* Master */
        baseOfs = BRD_BUMPER_DECAY_MAST;
        count = 4;
    } else {
        return;
    }

    int i;
    for (i = 0; i < count; i++) {
        float val = *(float *)((char *)board + baseOfs + i * 4);
        val -= 0.05;  /* double constant */
        if (val <= 0.0) val = 0.0;  /* double constant */
        *(float *)((char *)board + baseOfs + i * 4) = val;
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Feature block: Neon Camera (Neon)
 * Positions render objects relative to ball
 * Replicates FUN_00424790 (Neon slot[19])
 * ═══════════════════════════════════════════════════════════════════════════ */

static void Feature_NeonCamera(void *board, int level) {
    DWORD app = *(DWORD *)((char *)board + BOARD_APP_PTR);
    if (!app || IsBadReadPtr((void *)app, 0x600)) return;

    DWORD ball = *(DWORD *)(app + APP_BALL_PTR);
    if (!ball || IsBadReadPtr((void *)ball, 0x200)) return;

    float ballX = *(float *)(ball + BALL_POS_X_OFS);
    float ballY = *(float *)(ball + BALL_POS_Y_OFS);
    float ballZ = *(float *)(ball + BALL_POS_Z_OFS);

    /* Position render object 1 (board+0x436C) */
    DWORD render1 = *(DWORD *)((char *)board + 0x436C);
    if (render1) {
        DWORD *vtbl = *(DWORD **)render1;
        if (vtbl) {
            void (__fastcall *setPos)(DWORD, float, float, float) =
                (void (__fastcall *)(DWORD, float, float, float))vtbl[0x1]; /* vtable[+4] */
            if (setPos) setPos((DWORD)render1, ballX + 20.0f, ballY + 30.0f, ballZ - 20.0f);
        }
    }

    /* Position render object 2 (board+0x4370) if App+0x677 is 0 */
    if (!*(char *)(app + 0x677)) {
        DWORD ball2 = *(DWORD *)(app + 0x67C);
        if (ball2 && !IsBadReadPtr((void *)ball2, 0x200)) {
            float b2X = *(float *)(ball2 + BALL_POS_X_OFS);
            float b2Y = *(float *)(ball2 + BALL_POS_Y_OFS);
            float b2Z = *(float *)(ball2 + BALL_POS_Z_OFS);
            DWORD render2 = *(DWORD *)((char *)board + 0x4370);
            if (render2) {
                DWORD *vtbl2 = *(DWORD **)render2;
                if (vtbl2) {
                    void (__fastcall *setPos)(DWORD, float, float, float) =
                        (void (__fastcall *)(DWORD, float, float, float))vtbl2[0x1];
                    if (setPos) setPos((DWORD)render2, b2X + 20.0f, b2Y + 30.0f, b2Z - 20.0f);
                }
            }
        }
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Feature block: Sky Popcylinder Activator (Sky)
 * Randomly activates popcylinders via Scene_SetRaceActive
 * Replicates SkyBoard_Update (0x41FC90)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void Feature_SkyPopcylinder(void *board, int level) {
    if (!g_RNG || !g_SceneSetRaceActive) return;

    DWORD app = *(DWORD *)((char *)board + BOARD_APP_PTR);
    if (!app || IsBadReadPtr((void *)app, 0x600)) return;

    /* Only activate when difficulty != 0 */
    if (*(int *)(app + APP_DIFFICULTY) == 0) {
        /* Still call vtable[+4] on the scene object at board+0x47AC */
        DWORD sceneObj = *(DWORD *)((char *)board + 0x47AC);
        if (sceneObj) {
            DWORD *vtbl = *(DWORD **)sceneObj;
            if (vtbl) {
                void (__fastcall *fn4)(DWORD) = (void (__fastcall *)(DWORD))vtbl[0x1];
                if (fn4) fn4((DWORD)sceneObj);
            }
        }
    }

    /* Check timer */
    if (!*(int *)((char *)board + 0x47F4) || *(int *)(app + APP_DIFFICULTY) == 0) return;

    int counter = *(int *)((char *)board + 0x47F0) - 1;
    *(int *)((char *)board + 0x47F0) = counter;
    if (counter >= 1) return;

    /* Reset counter and activate random popcylinders */
    *(int *)((char *)board + 0x47F0) = 0x4B; /* 75 */
    int rngCase = g_RNG((void *)0x4F7360, 6, 0);

    /* Play sound at rotator position */
    DWORD rotator = *(DWORD *)((char *)board + 0x47D0);
    if (rotator && g_SoundPlay3DUpd) {
        DWORD snd = *(DWORD *)(app + 0x480);
        if (snd) {
            g_SoundPlay3DUpd((void *)snd,
                *(float *)(rotator + 0x10D4),
                *(float *)(rotator + 0x10D8),
                *(float *)(rotator + 0x10DC));
        }
    }

    /* Activate popcylinders based on RNG case */
    switch (rngCase) {
    case 0:
        g_SceneSetRaceActive(*(int *)((char *)board + 0x47C4));
        g_SceneSetRaceActive(*(int *)((char *)board + 0x47C8));
        g_SceneSetRaceActive(*(int *)((char *)board + 0x47D4));
        g_SceneSetRaceActive(*(int *)((char *)board + 0x47D8));
        break;
    case 1: {
        int n = g_RNG((void *)0x4F7360, 5, 0);
        int i;
        for (i = 0; i < n + 3; i++) {
            int idx = g_RNG((void *)0x4F7360, 0x10, 0);
            g_SceneSetRaceActive(*(int *)((char *)board + 0x47B0 + idx * 4));
        }
        break;
    }
    case 2: {
        int i;
        for (i = 0; i < 0x10; i++)
            g_SceneSetRaceActive(*(int *)((char *)board + 0x47B0 + i * 4));
        break;
    }
    case 3:
        g_SceneSetRaceActive(*(int *)((char *)board + 0x47B0));
        g_SceneSetRaceActive(*(int *)((char *)board + 0x47B4));
        g_SceneSetRaceActive(*(int *)((char *)board + 0x47B8));
        g_SceneSetRaceActive(*(int *)((char *)board + 0x47BC));
        g_SceneSetRaceActive(*(int *)((char *)board + 0x47C0));
        g_SceneSetRaceActive(*(int *)((char *)board + 0x47CC));
        g_SceneSetRaceActive(*(int *)((char *)board + 0x47D0));
        g_SceneSetRaceActive(*(int *)((char *)board + 0x47DC));
        g_SceneSetRaceActive(*(int *)((char *)board + 0x47E0));
        g_SceneSetRaceActive(*(int *)((char *)board + 0x47E4));
        g_SceneSetRaceActive(*(int *)((char *)board + 0x47E8));
        g_SceneSetRaceActive(*(int *)((char *)board + 0x47EC));
        break;
    case 4: {
        int idx = g_RNG((void *)0x4F7360, 4, 0);
        g_SceneSetRaceActive(*(int *)((char *)board + 0x47B0 + idx * 0x10));
        g_SceneSetRaceActive(*(int *)((char *)board + 0x47B4 + idx * 0x10));
        g_SceneSetRaceActive(*(int *)((char *)board + 0x47B8 + idx * 0x10));
        g_SceneSetRaceActive(*(int *)((char *)board + 0x47BC + idx * 0x10));
        break;
    }
    case 5: {
        int idx = g_RNG((void *)0x4F7360, 4, 0);
        g_SceneSetRaceActive(*(int *)((char *)board + 0x47B0 + idx * 4));
        g_SceneSetRaceActive(*(int *)((char *)board + 0x47C0 + idx * 4));
        g_SceneSetRaceActive(*(int *)((char *)board + 0x47D0 + idx * 4));
        g_SceneSetRaceActive(*(int *)((char *)board + 0x47E0 + idx * 4));
        break;
    }
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Universal Board_Update — replaces all 15 per-level Board_Update functions
 *
 * Calls Scene_Update first, then dispatches to feature blocks based on
 * the per-level feature flags in g_updateFeatures[].
 * ═══════════════════════════════════════════════════════════════════════════ */

void __fastcall UniversalBoardUpdate(void *board) {
    if (!g_SceneUpdate || !board) return;

    /* Call base Scene_Update */
    g_SceneUpdate(board);

    /* Get level and dispatch features */
    int level = GetCurrentLevel(board);
    if (level == 0) return;

    DWORD features = g_updateFeatures[level];
    if (!features) return;

    /* Bridge animation (Intermediate + Master bridge) */
    if (features & FEAT_BRIDGE_ANIM)
        Feature_BridgeAnimation(board, level);

    /* Swirl zones (Dizzy + Master) */
    if (features & FEAT_SWIRL)
        Feature_SwirlZones(board, level);

    /* Windmill (Tower) */
    if (features & FEAT_WINDMILL)
        Feature_Windmill(board, level);

    /* BadBall spawner (Odd) */
    if (features & FEAT_BADBALL)
        Feature_BadBallSpawner(board, level);

    /* Note: Bumper decay, neon camera, and sky popcylinder are handled in
       UniversalRaceState (slot 19), NOT here. In the original game, these
       are in the RaceState handler, not Board_Update. */
}

/* Naked thunk for vtable slot 1 (Board_Update) */
/* Replaces the original __fastcall Board_Update(board) with UniversalBoardUpdate */
/* Must be callable via vtable indirect call: CALL [vtable+0x4] */
/* We patch each level's vtable[1] to point to UniversalBoardUpdate */

/* ═══════════════════════════════════════════════════════════════════════════
 * Universal Race State (Slot 19) — replaces all 15 per-level RaceState handlers
 *
 * Calls Board_UpdateRaceState first, then dispatches to feature blocks
 * that belong in RaceState (bumper decay, neon camera, sky popcylinders).
 * ═══════════════════════════════════════════════════════════════════════════ */

void __fastcall UniversalRaceState(void *board) {
    if (!g_BoardUpdateRaceState || !board) return;

    /* Call base Board_UpdateRaceState */
    g_BoardUpdateRaceState(board);

    /* Get level and dispatch features */
    int level = GetCurrentLevel(board);
    if (level == 0) return;

    DWORD features = g_updateFeatures[level];
    if (!features) return;

    /* Bumper lit decay (Beginner + Toob + Master) */
    if (features & FEAT_BUMPER_DECAY)
        Feature_BumperDecay(board, level);

    /* Neon camera follow (Neon) */
    if (features & FEAT_NEON_CAM)
        Feature_NeonCamera(board, level);

    /* Sky popcylinder activator (Sky) */
    if (features & FEAT_SKY_POPCYL)
        Feature_SkyPopcylinder(board, level);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Universal CreateDynamicObjects (Slot 33) — replaces all 15 per-level handlers
 *
 * The game calls this for each named mesh object found in the MESHWORLD.
 * Each per-level handler matches its own object names (BRIDGE, TIPPER, MACE, etc.)
 * and creates the appropriate game object via its ctor.
 *
 * Since N:/E: names are unique per level's mesh, there are zero conflicts.
 * We save the original per-level handlers and delegate to them, so each
 * level's objects are created exactly as the original game intended.
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Original per-level CreateDynamicObjects function pointers (saved before patching) */
typedef void (__thiscall *CreateDynamicObjects_t)(void *board, char *name, void *out1, void *out2, int *meshData);
static CreateDynamicObjects_t g_origCreateDynamicObjects[16] = {NULL};

/* The universal handler: look up the level and delegate to original handler */
void __fastcall UniversalCreateDynamicObjects(void *board, char *name, void *out1, void *out2, int *meshData) {
    int level = GetCurrentLevel(board);
    if (level == 0 || level > 15) return;

    /* Delegate to the original per-level handler */
    CreateDynamicObjects_t orig = g_origCreateDynamicObjects[level];
    if (orig) {
        /* Call as __thiscall: ECX=board, stack params */
        orig(board, name, out1, out2, meshData);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Universal DispatchCollision (Slot 29) — replaces all 15 per-level handlers
 *
 * Each per-level handler checks N:/E: collision partner names and does
 * level-specific logic, then calls DispatchCollisionEvents.
 *
 * We save the original per-level handlers and delegate to them.
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Original per-level DispatchCollision function pointers (saved before patching) */
typedef void (__thiscall *DispatchCollision_t)(void *board, int *ball, int *collPair);
static DispatchCollision_t g_origDispatchCollision[16] = {NULL};

/* The universal handler: look up the level and delegate to original handler */
void __fastcall UniversalDispatchCollision(void *board, int *ball, int *collPair) {
    int level = GetCurrentLevel(board);
    if (level == 0 || level > 15) return;

    /* Delegate to the original per-level handler */
    DispatchCollision_t orig = g_origDispatchCollision[level];
    if (orig) {
        orig(board, ball, collPair);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Universal Post-Setup — config-driven feature initialization
 * ═══════════════════════════════════════════════════════════════════════════ */

static void UniversalPostSetup(void *board) {
    int level = GetCurrentLevel(board);
    if (level == 0) return;

    LoadConfig();

    /* Bridge */
    if (g_objectEnabled[OBJ_BRIDGE][level]) {
        InitBridge(board);
    }

    /* Bumpers */
    if (g_objectEnabled[OBJ_BUMPERS][level]) {
        DWORD meshWorld = *(DWORD *)((char *)board + BOARD_MESHWORLD);
        if (!meshWorld || IsBadReadPtr((void *)meshWorld, 0x430)) return;

        int i;
        for (i = 0; i < 8; i++) {
            char nameBuf[16];
            const char *prefix = "N:BUMPER";
            int p = 0, j;
            for (j = 0; prefix[j]; j++) nameBuf[p++] = prefix[j];
            int num = i + 1;
            if (num >= 10) { nameBuf[p++] = '0' + (num / 10); num %= 10; }
            nameBuf[p++] = '0' + num;
            nameBuf[p] = '\0';

            void *dest = (char *)board + BUMPER_SLOT_BASE + i * BUMPER_SLOT_STRIDE;

            if (g_AthenaListInit) {
                g_AthenaListInit(dest, 0);
            }
            if (g_CollectByNameFilter) {
                g_CollectByNameFilter((void *)meshWorld, nameBuf, dest);
            }
            *(DWORD *)((char *)board + BUMPER_LIT_BASE + i * BUMPER_LIT_STRIDE) = 0;
        }
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Universal Scene Constructor — REPLACES vtable[0x48] (Scene_LoadLevel*)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void UniversalConstructor(void *board, int raceIndex) {
    char buf[256];
    wsprintfA(buf, "UniversalConstructor called: board=%p raceIndex=%d", board, raceIndex);
    DebugLog(buf);
    if (!board || raceIndex < 1 || raceIndex > 15) {
        DebugLog("UniversalConstructor: invalid params");
        return;
    }
    if (!g_operatorNew || !g_LevelMeshWorldCtor || !g_LevelRenderCtor ||
        !g_LevelInitScene) {
        DebugLog("UniversalConstructor: function pointers not resolved");
        return;
    }

    /* Use meshPath from LevelData if available, fallback to g_meshPaths */
    const char *meshPath = g_levelData[raceIndex].meshPath;
    if (!meshPath || !*meshPath) meshPath = g_meshPaths[raceIndex];
    if (!meshPath) return;

    DWORD app = *(DWORD *)((char *)board + BOARD_APP_PTR);
    if (!app || IsBadReadPtr((void *)app, 0x200)) return;
    void *gfx = *(void **)((char *)app + 0x174);
    if (!gfx) return;

    /* Step 1: MeshWorld */
    void *meshMem = g_operatorNew(0x10D0);
    if (!meshMem) return;
    void *meshWorld = g_LevelMeshWorldCtor(meshMem, gfx, meshPath);
    *(DWORD *)((char *)board + BOARD_MESHWORLD) = (DWORD)meshWorld;

    /* Step 2: RenderObj */
    void *renderMem = g_operatorNew(0x10D0);
    void *renderObj = NULL;
    if (renderMem) {
        renderObj = g_LevelRenderCtor(renderMem, meshWorld);
    }
    *(DWORD *)((char *)board + BOARD_RENDEROBJ) = (DWORD)renderObj;

    /* Step 3: InitScene */
    g_LevelInitScene(board);

    /* Step 4: Config-driven features (BEFORE Board_Setup)
     * The original game loads sub-meshes (bridge, tipper, etc.) in the
     * board ctor, BEFORE Board_Setup runs. Board_Setup calls
     * Scene_CreateDynamicObjects -> vtable[33] which reads these mesh
     * pointers. If we load them after Board_Setup, vtable[33] sees NULL
     * pointers and the dynamic objects get no mesh. */
    UniversalPostSetup(board);

    /* Step 5: Board_Setup via vtable[0x80] */
    DWORD vtable = *(DWORD *)board;
    if (vtable && !IsBadReadPtr((void *)vtable, 0x84)) {
        void (__thiscall *boardSetup)(void *) = *(void (__thiscall **)(void *))((char *)vtable + 0x80);
        if (boardSetup) boardSetup(board);
    }

    /* Step 6: Per-level post-Board_Setup extras.
     * Dizzy's scene loader (0x40D390) scans MESHWORLD section 3 for
     * "TarBubble" objects and appends them to board+0x11E4. This feeds
     * the TarBubble particle effect system in DizzyBoard_Update.
     *
     * ORIGINAL GAME BUG: FUN_0044fa90 (called from DizzyBoard_Update) picks
     * a random TarBubble from the list. If the list is empty, it falls
     * through to iVar1=0 (NULL), then dereferences *(NULL+4) = crash at
     * address 0x4. This happens when a modded Dizzy level has no TarBubbles.
     *
     * FIX: If no TarBubbles are found, create a dummy object with a valid
     * name pointer and zero position so FUN_0044fa90 reads harmless data
     * instead of crashing. */
    if (raceIndex == 4) {
        int tarBubbleCount = 0;
        DWORD meshWorldPtr = *(DWORD *)((char *)board + BOARD_MESHWORLD);
        if (meshWorldPtr && !IsBadReadPtr((void *)meshWorldPtr, 0x500) &&
            g_AthenaListAppend && g_AthenaListGetIterator && g_AthenaListGetSize) {
            /* Access the MESHWORLD's section 3 object list:
             * meshWorld+0x480 = object database, +0x894 = iterator base,
             * +0x898 = count, +0xCA0 = array pointer */
            DWORD objDb = *(DWORD *)(meshWorldPtr + 0x480);
            if (objDb && !IsBadReadPtr((void *)objDb, 0x20)) {
                int iter = g_AthenaListGetIterator((void *)(objDb + 0x894));
                *(DWORD *)(objDb + 0x89C + iter * 4) = 0;
                int count = *(int *)(objDb + 0x898);
                if (count > 0) {
                    DWORD *array = *(DWORD **)(objDb + 0xCA0);
                    if (array && !IsBadReadPtr((void *)array, count * 4)) {
                        *(DWORD *)(objDb + 0x89C + iter * 4) = 1;
                        int idx = 0;
                        while (idx < count) {
                            DWORD *obj = (DWORD *)array[idx];
                            if (obj && !IsBadReadPtr((void *)obj, 4)) {
                                char *name = *(char **)obj;
                                if (name && !IsBadReadPtr(name, 9)) {
                                    if (my_strnicmp(name, "TarBubble", 9) == 0) {
                                        g_AthenaListAppend(
                                            (void *)((char *)board + 0x11E4),
                                            (int)obj);
                                        tarBubbleCount++;
                                    }
                                }
                            }
                            idx++;
                            *(DWORD *)(objDb + 0x89C + iter * 4) = idx;
                        }
                    }
                }
            }
        }
        /* If no TarBubbles were found, create a dummy entry to prevent
         * FUN_0044fa90 from crashing on an empty list. The dummy needs:
         * +0x00 = char* name (points to a valid string)
         * +0x04 = float X position
         * +0x08 = float Y position
         * +0x0C = float Z position
         * FUN_0044fa90 reads these 4 fields when picking a random entry. */
        if (tarBubbleCount == 0 && g_AthenaListAppend && g_operatorNew) {
            /* Allocate a 0x10-byte dummy object */
            DWORD *dummy = (DWORD *)g_operatorNew(0x10);
            if (dummy) {
                memset(dummy, 0, 0x10);
                /* Set name pointer to a static string */
                static char dummyName[] = "TarBubble";
                dummy[0] = (DWORD)dummyName;
                /* Positions at +4, +8, +0xC are already 0 (memset above) */
                g_AthenaListAppend(
                    (void *)((char *)board + 0x11E4),
                    (int)dummy);
                DebugLog("TarBubble scan: no TarBubbles found, added dummy entry");
            }
        }
    }
}

/* Must be non-static for asm reference */
void __cdecl UniversalConstructorLogic(void *board, int raceIndex) {
    UniversalConstructor(board, raceIndex);
}

__attribute__((naked)) static void Hook_UniversalConstructor(void) {
    __asm__ __volatile__(
        "pushl %%ebp\n\t"
        "movl  %%esp, %%ebp\n\t"
        "pushl %%edx\n\t"
        "pushl %%ecx\n\t"

        "movl  0x08(%%esi), %%eax\n\t"
        "pushl %%eax\n\t"
        "pushl %%ecx\n\t"

        "call  _UniversalConstructorLogic\n\t"
        "addl  $8, %%esp\n\t"

        "popl  %%ecx\n\t"
        "popl  %%edx\n\t"
        "popl  %%ebp\n\t"

        "jmpl  *_g_ctorTrampoline\n\t"
        :: : "eax", "memory"
    );
}

/* Must be non-static for asm reference */
unsigned char *g_ctorTrampoline = NULL;

static void InstallUniversalConstructorHook(void) {
    DWORD targetAddr = g_moduleBase + 0x000273E0;
    unsigned char *orig = (unsigned char *)targetAddr;

    if (orig[0] != 0xFF || orig[1] != 0x52 || orig[2] != 0x48 ||
        orig[3] != 0x8B || orig[4] != 0x4E || orig[5] != 0x04) return;

    g_ctorTrampoline = VirtualAlloc(NULL, 16,
                        MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!g_ctorTrampoline) return;

    g_ctorTrampoline[0] = 0x8B;  /* MOV ECX, [ESI+4] */
    g_ctorTrampoline[1] = 0x4E;
    g_ctorTrampoline[2] = 0x04;
    g_ctorTrampoline[3] = 0xE9;  /* JMP back to original+6 */
    *(DWORD *)(g_ctorTrampoline + 4) = (targetAddr + 6) - ((DWORD)g_ctorTrampoline + 8);

    DWORD oldProtect;
    VirtualProtect(orig, 6, PAGE_EXECUTE_READWRITE, &oldProtect);
    orig[0] = 0xE9;
    *(DWORD *)(orig + 1) = (DWORD)&Hook_UniversalConstructor - (targetAddr + 5);
    orig[5] = 0x90;
    VirtualProtect(orig, 6, oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), orig, 6);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Allocation size patch
 * ═══════════════════════════════════════════════════════════════════════════ */

static const DWORD g_allocPatchRVAs[15] = {
    0x00027109, 0x00027136, 0x00027167, 0x00027198, 0x000271C9,
    0x000271FA, 0x0002722B, 0x0002725C, 0x0002728D, 0x000272BE,
    0x000272EF, 0x00027320, 0x00027351, 0x0002737B, 0x000273A5,
};

static void PatchAllocSizes(void) {
    int i;
    for (i = 0; i < 15; i++) {
        unsigned char *site = (unsigned char *)(g_moduleBase + g_allocPatchRVAs[i]);
        if (IsBadReadPtr(site, 5)) continue;
        if (site[0] != 0x68) continue;

        DWORD oldProtect;
        VirtualProtect(site, 5, PAGE_EXECUTE_READWRITE, &oldProtect);
        *(DWORD *)(site + 1) = UNION_SIZE;
        VirtualProtect(site, 5, oldProtect, &oldProtect);
        FlushInstructionCache(GetCurrentProcess(), site, 5);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Install collision hook
 * ═══════════════════════════════════════════════════════════════════════════ */

static void InstallHook(void) {
    DWORD targetAddr = g_moduleBase + RVA_DispatchCollisionEvents;
    unsigned char *orig = (unsigned char *)targetAddr;

    if (orig[0] != 0x6A || orig[1] != 0xFF ||
        orig[2] != 0x64 || orig[3] != 0xA1) return;

    g_trampoline = VirtualAlloc(NULL, 16,
                    MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!g_trampoline) return;

    memcpy(g_trampoline, orig, 8);
    g_trampoline[8] = 0xE9;
    *(DWORD *)(g_trampoline + 9) = (targetAddr + 8) - ((DWORD)g_trampoline + 13);

    g_OriginalDispatch = (DispatchCollisionEvents_t)g_trampoline;

    DWORD oldProtect;
    VirtualProtect(orig, 16, PAGE_EXECUTE_READWRITE, &oldProtect);
    orig[0] = 0xE9;
    *(DWORD *)(orig + 1) = (DWORD)&Hook_DispatchCollisionEvents - (targetAddr + 5);
    orig[5] = 0x90;
    orig[6] = 0x90;
    orig[7] = 0x90;
    VirtualProtect(orig, 16, oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), orig, 16);
}

/* BASS proxy exports handled by bass.def - DLL forwarding to bass_real.dll */

/* ═══════════════════════════════════════════════════════════════════════════
 * Debug logging
 * ═══════════════════════════════════════════════════════════════════════════ */

void DebugLog(const char *msg) {
    if (!g_configPath[0]) return;
    /* Write next to the DLL — C:\ is not writable under UAC */
    char logPath[MAX_PATH];
    strcpy(logPath, g_configPath);
    char *p = strrchr(logPath, '\\');
    if (p) {
        strcpy(p + 1, "lfdebug.log");
    } else {
        strcpy(logPath, "lfdebug.log");
    }
    HANDLE hFile = CreateFileA(logPath, GENERIC_WRITE,
                               FILE_SHARE_READ, NULL, OPEN_ALWAYS,
                               FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return;
    SetFilePointer(hFile, 0, NULL, FILE_END);
    DWORD written;
    WriteFile(hFile, msg, strlen(msg), &written, NULL);
    WriteFile(hFile, "\r\n", 2, &written, NULL);
    CloseHandle(hFile);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Vtable patching — replace slots 1, 19, 29, 33 in all 15 level vtables
 * with universal handlers. Saves original pointers for delegation.
 * ═══════════════════════════════════════════════════════════════════════════ */

static void InstallVtablePatches(void) {
    int i;
    for (i = 1; i <= 15; i++) {
        DWORD vtableAddr = g_levelVtables[i];
        if (!vtableAddr) continue;
        if (IsBadReadPtr((void *)vtableAddr, 0x88)) continue;

        DWORD oldProtect;

        /* Slot 1 (offset +0x04): Board_Update → UniversalBoardUpdate */
        {
            DWORD *slot = (DWORD *)(vtableAddr + 0x04);
            VirtualProtect(slot, 4, PAGE_EXECUTE_READWRITE, &oldProtect);
            *slot = (DWORD)&UniversalBoardUpdate;
            VirtualProtect(slot, 4, oldProtect, &oldProtect);
            FlushInstructionCache(GetCurrentProcess(), slot, 4);
        }

        /* Slot 19 (offset +0x4C): RaceState → UniversalRaceState
         * Save original pointer for base handler delegation */
        {
            DWORD *slot = (DWORD *)(vtableAddr + 0x4C);
            g_origDispatchCollision[i] = NULL; /* init */
            VirtualProtect(slot, 4, PAGE_EXECUTE_READWRITE, &oldProtect);
            /* We don't save slot 19 — UniversalRaceState calls g_BoardUpdateRaceState directly */
            *slot = (DWORD)&UniversalRaceState;
            VirtualProtect(slot, 4, oldProtect, &oldProtect);
            FlushInstructionCache(GetCurrentProcess(), slot, 4);
        }

        /* Slot 29 (offset +0x74): DispatchCollision → UniversalDispatchCollision
         * Save original pointer for delegation */
        {
            DWORD *slot = (DWORD *)(vtableAddr + 0x74);
            g_origDispatchCollision[i] = (DispatchCollision_t)*slot;
            VirtualProtect(slot, 4, PAGE_EXECUTE_READWRITE, &oldProtect);
            *slot = (DWORD)&UniversalDispatchCollision;
            VirtualProtect(slot, 4, oldProtect, &oldProtect);
            FlushInstructionCache(GetCurrentProcess(), slot, 4);
        }

        /* Slot 33 (offset +0x84): CreateDynamicObjects → UniversalCreateDynamicObjects
         * Save original pointer for delegation */
        {
            DWORD *slot = (DWORD *)(vtableAddr + 0x84);
            g_origCreateDynamicObjects[i] = (CreateDynamicObjects_t)*slot;
            VirtualProtect(slot, 4, PAGE_EXECUTE_READWRITE, &oldProtect);
            *slot = (DWORD)&UniversalCreateDynamicObjects;
            VirtualProtect(slot, 4, oldProtect, &oldProtect);
            FlushInstructionCache(GetCurrentProcess(), slot, 4);
        }
    }
    DebugLog("Vtable slots [1,19,29,33] patched for all 15 levels");
}

static DWORD WINAPI PatchThread(LPVOID param) {
    DebugLog("=== PatchThread started ===");
    Sleep(2000);
    DebugLog("Sleep done, resolving module base");
    g_moduleBase = (DWORD)GetModuleHandleA("Hamsterball.exe");
    if (!g_moduleBase) g_moduleBase = 0x00400000;

    g_operatorNew = (operator_new_t)(g_moduleBase + RVA_operator_new);
    g_LevelMeshWorldCtor = (Level_MeshWorldCtor_t)(g_moduleBase + RVA_Level_MeshWorldCtor);
    g_LevelRenderCtor = (Level_RenderCtor_t)(g_moduleBase + RVA_Level_RenderCtor);
    g_LevelInitScene = (Level_InitScene_t)(g_moduleBase + RVA_Level_InitScene);
    g_SoundPlay3D = (Sound_Play3D_t)(g_moduleBase + RVA_Sound_Play3D);
    g_CollectByNameFilter = (Scene_CollectByNameFilter_t)(g_moduleBase + RVA_Scene_CollectByNameFilter);
    g_AthenaListInit = (AthenaList_Init_t)(g_moduleBase + RVA_AthenaList_Init);
    g_BoardCtor = (Board_ctor_t)(g_moduleBase + RVA_Board_ctor);
    g_LoadRaceData = (LoadRaceData_t)(g_moduleBase + RVA_LoadRaceData);
    g_Vec3Init = (Vec3_Init_t)(g_moduleBase + RVA_Vec3_Init);
    g_MatrixIdentity = (Matrix_Identity_t)(g_moduleBase + RVA_Matrix_Identity);
    g_MeshNodeCtor = (MeshNode_ctor_t)(g_moduleBase + RVA_MeshNode_ctor);
    g_SpriteCtor = (Sprite_ctor_t)(g_moduleBase + RVA_Sprite_ctor);
    g_TipperVisualAttach = (TipperVisual_Attach_t)(g_moduleBase + RVA_TipperVisual_Attach);
    g_LevelAssignTex = (Level_AssignTex_t)(g_moduleBase + RVA_Level_AssignTexAndScales);
    g_SoundGetNextChannel = (Sound_GetNextChannel_t)(g_moduleBase + RVA_Sound_GetNextChannel);
    g_SceneRenderIfVisible = (Scene_RenderIfVisible_t)(g_moduleBase + RVA_Scene_RenderIfVisible);
    g_AthenaListAppend = (AthenaList_Append_t)(g_moduleBase + RVA_AthenaList_Append);
    g_AthenaListGetSize = (AthenaList_GetSize_t)(g_moduleBase + RVA_AthenaList_GetSize);
    g_AthenaListGetIterator = (AthenaList_GetIterator_t)(g_moduleBase + RVA_AthenaList_GetIterator);

    /* Resolve Board_Update function pointers */
    g_SceneUpdate = (Scene_Update_t)(g_moduleBase + RVA_Scene_Update);
    g_BoardUpdateRaceState = (Board_UpdateRaceState_t)(g_moduleBase + RVA_Board_UpdateRaceState);
    g_GfxScaleZ = (Gfx_ScaleFn_t)(g_moduleBase + RVA_Gfx_ScaleZ);
    g_GfxScaleY = (Gfx_ScaleFn_t)(g_moduleBase + RVA_Gfx_ScaleY);
    g_GfxScaleX = (Gfx_ScaleFn_t)(g_moduleBase + RVA_Gfx_ScaleX);
    g_GfxSetPosition = (Gfx_SetPosition_t)(g_moduleBase + RVA_Gfx_SetPosition);
    g_TimerInit = (Timer_Init_t)(g_moduleBase + RVA_Timer_Init);
    g_TimerCleanup = (Timer_Cleanup_t)(g_moduleBase + RVA_Timer_Cleanup);
    g_MatrixTransformVec3 = (Matrix_TransformVec3_t)(g_moduleBase + RVA_Matrix_TransformVec3);
    g_Matrix44Zero = (Matrix44_Zero_t)(g_moduleBase + RVA_Matrix44_Zero);
    g_SceneForEachBallSetVelocity = (Scene_ForEachBall_SetVelocity_t)(g_moduleBase + RVA_Scene_ForEachBall_SetVel);
    g_AthenaListGetIteratorUpd = (AthenaList_GetIterator_t)(g_moduleBase + RVA_AthenaList_GetIterator);
    g_AthenaListGetSizeUpd = (AthenaList_GetSize_t)(g_moduleBase + RVA_AthenaList_GetSize);
    g_AthenaListAppendUpd = (AthenaList_Append_t)(g_moduleBase + RVA_AthenaList_Append);
    g_OperatorNewUpd = (operator_new_t)(g_moduleBase + RVA_operator_new);
    g_SoundPlay3DUpd = (Sound_Play3D_Fn_t)(g_moduleBase + RVA_Sound_Play3D);
    g_CreateTarBubble = (FUN_0044fa90_t)(g_moduleBase + RVA_FUN_0044fa90);
    g_CreateSplashParticle = (FUN_0044fb50_t)(g_moduleBase + RVA_FUN_0044fb50);
    g_RemoveBall = (FUN_00405190_t)(g_moduleBase + RVA_FUN_00405190);
    g_RNG = (CPUID_RNG_t)(g_moduleBase + RVA_CPUID_RNG);
    g_BadBallCtor = (BadBall_ctor_t)(g_moduleBase + RVA_BadBall_ctor);
    g_BallSetTrajectory = (Ball_SetTrajectory_t)(g_moduleBase + RVA_Ball_SetTrajectory);
    g_BallSetVec3AtOffset = (Ball_SetVec3AtOffset_t)(g_moduleBase + RVA_Ball_SetVec3AtOffset);
    g_Vec3NormalizeAndScale = (Vec3_NormalizeAndScale_t)(g_moduleBase + RVA_Vec3_NormalizeAndScale);
    g_Vec3CopyUpd = (Vec3_Copy_t)(g_moduleBase + RVA_Vec3_Copy_Upd);
    g_SoundCalcDistAtten = (Sound_CalcDistAtten_t)(g_moduleBase + RVA_Sound_CalcDistAtten);
    g_SoundPlay3DAtPos = (Sound_Play3DAtPos_t)(g_moduleBase + RVA_Sound_Play3DAtPos);
    g_SceneSetRaceActive = (Scene_SetRaceActive_t)(g_moduleBase + RVA_Scene_SetRaceActive);
    g_SceneAddObject = (Scene_AddObject_t)(g_moduleBase + RVA_Scene_AddObject);

    /* Initialize feature flags from defaults */
    memcpy(g_updateFeatures, g_defaultFeatures, sizeof(g_updateFeatures));

    DebugLog("Function pointers resolved");

    GetConfigPath();
    LoadConfig();

    if (GetFileAttributesA(g_levelDataPath) == INVALID_FILE_ATTRIBUTES) {
        GenerateLevelData();
    }
    LoadLevelData();

    PatchAllocSizes();
    InstallBoardCtorHooks();
    InstallUniversalConstructorHook();
    InstallVtablePatches();
    InstallHook();
    DebugLog("=== PatchThread complete ===");
    return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hinstDLL);
        CreateThread(NULL, 0, PatchThread, NULL, 0, NULL);
    }
    return TRUE;
}
