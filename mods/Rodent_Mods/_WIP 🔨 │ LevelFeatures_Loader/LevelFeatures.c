/*
 * LevelFeatures_Loader v12 — S1 + level-folder sub-mesh N:/E: scan (LevelData.txt deprecated)
 * Phase1+Folder: All S1 refs + every .MESHWORLD/.MESH in level folder byte-scanned for N:/E: (64->128 slots at 0xA8D4).
 * LevelData.txt removed — g_levelData[] kept as in-memory defaults only.
 * Original v6 header below:
 * LevelFeatures_Loader v6 — Universal Level Handler + Universal Vtable
 *
 * 1. ALLOCATION PATCH: Patches all 15 level allocation sites in
 *    Tournament_AdvanceRace (0x00427080) to use the union size 0xAB00.
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

#define RVA_DispatchCollisionEvents  0x0000C5D0
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
#define RVA_eh_vector_ctor            0x000BAF59
#define RVA_FUN_0040a870              0x0000A870
#define RVA_Vec3List_Free             0x0000A820
#define RVA_Level_AssignTexScales     0x00011BA0

/* CreateDynamicObjects ctor RVAs */
#define RVA_Tipper_ctor              0x00037960
#define RVA_TipperVisual_ctor        0x000661a0
#define RVA_Gluebie_ctor             0x00037cb0
#define RVA_Catapult_ctor            0x00037e10
#define RVA_Mace_ctor                0x00038750
#define RVA_Glass_Level_ctor         0x000384a0
#define RVA_Trapdoor_ctor            0x00038290
#define RVA_Stands_ctor              0x00062850
#define RVA_Bonk_ctor                0x00038850
#define RVA_Fan_ctor                 0x00038c20
#define RVA_SawBlade_ctor            0x00034660
#define RVA_SawBlade_SetVariant      0x00034ab0
#define RVA_Spinner_Level_ctor       0x000396f0
#define RVA_Gear_Level_ctor          0x0003a150
#define RVA_Bell_ctor                0x00034d70
#define RVA_Odd_Lifter_ctor          0x00034e60
#define RVA_Lifter_ctor              0x00036920
#define RVA_SpeedCylinder_ctor       0x00036a20
#define RVA_TimeButton_ctor          0x00036c10
#define RVA_Rotator_ctor_Impossible  0x00035940
#define RVA_Saw_ctor                 0x0003b780
#define RVA_Saw2_ctor                0x0003be20
#define RVA_Fallout_ctor             0x0003bbc0
#define RVA_Blockdawg_ctor           0x0003c310
#define RVA_GameLevel_ctor           0x000351f0
#define RVA_Wavy_ctor                0x0003ad40
#define RVA_Wavy_Configure           0x00035440
#define RVA_NeonPlatform_ctor        0x0003e110
#define RVA_ArenaStands_ctor         0x0003e450
#define RVA_Popcylinder_ctor         0x000363f0
#define RVA_PopCylinder_ctor         0x00036ee0
#define RVA_Rotator_ctor             0x000366f0
#define RVA_Looper_ctor              0x00035800
#define RVA_Gear_ctor                0x00037590
#define RVA_Pendulum_ctor            0x00037700
#define RVA_BreakBridge_ctor         0x00036d70
#define RVA_Level_FindObjectByName   0x00060530
#define RVA_Sound_InitChannels       0x00034580

/* Bumper physics constants — per-level (verified via Ghidra):
 * Beginner/Toob: vel*4.0, min 5.0, max 10.0 (from _DAT_004cf41c, _DAT_004cf55c, _DAT_004cf9f8)
 * Master:        vel*5.0, min 5.0, max 12.0 (from _DAT_004cf55c, _DAT_004cf55c, _DAT_004cf3dc) */
#define BUMPER_VEL_SCALE_BEGINNER  4.0f
#define BUMPER_VEL_SCALE_MASTER    5.0f
#define BUMPER_MIN_SPEED           5.0f
#define BUMPER_MAX_SPEED_BEGINNER  10.0f
#define BUMPER_MAX_SPEED_MASTER    12.0f

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
#define BUMPER_SLOT_BASE   UNI_EHVECTOR
#define BUMPER_SLOT_STRIDE 0x418
#define BUMPER_LIT_BASE    UNI_BUMPER_LIT
#define BUMPER_LIT_STRIDE  4
#define BUMPER_LIT_COLL     UNI_BUMPER_LIT



/* Bridge slot layout (unified) */
#define BRIDGE_MESHWORLD   UNI_MESH_0
#define BRIDGE_RENDEROBJ   UNI_MESH_1
#define BRIDGE_PARAM1      UNI_BRIDGE_ANGLE  /* float 45.0 in Intermediate */
#define BRIDGE_PARAM2      UNI_BRIDGE_STATE  /* 0 */
#define BRIDGE_PARAM3      UNI_BRIDGE_COUNTER  /* 0x32 = 50 */

#define UNION_SIZE  0xAB00  /* deprecated: board now vanilla-sized, union lives in extension heap */

/* ═══════════════════════════════════════════════════════════════════════════
 * Extension Heap — Option B (S1-driven, per-board HeapAlloc)
 * Board stays vanilla-sized; all union data lives in a separate HeapAlloc
 * block tracked in g_extMap (board->ext). board+EXT_PTR (0xAB00) is beyond
 * vanilla max 0x6498 and is NOT used for storage on small boards (OOB) —
 * g_extMap is the source of truth; board+EXT_PTR is only a best-effort mirror.
 * EXT_SIZE is fixed 0x3000 for v1 (dynamic counting scaffold below).
 * All OFF_* are ext-relative offsets. UNI_* kept as deprecated aliases.
 * ═══════════════════════════════════════════════════════════════════════════ */
#define EXT_PTR   0xAB00
#define EXT_SIZE  0xC000

/* OFF_* are now aliases to UNI_* — one map, no wasted compact region.
 * Previously OFF_* used small 0x000-0x2C20 offsets inside ext, while UNI_*
 * used legacy 0x6500-0xA880 offsets. Two labels for same drawer caused
 * writer/reader mismatches (e.g. WATER_ROT written to 0x010 but bridge reads
 * 0x8634). Now every OFF_* == UNI_* so ext+OFF_* == ext+UNI_*. */
#define OFF_TIPPER_MESH     UNI_TIPPER_MESH
#define OFF_TIPPER_RENDER   UNI_TIPPER_RENDER
#define OFF_WATER_MESH      UNI_MESH_0
#define OFF_WATER_RENDER    UNI_MESH_1
#define OFF_WATER_ROT_X     UNI_WATER_ROT_X
#define OFF_WATER_ROT_Y     UNI_WATER_ROT_Y
#define OFF_WATER_ROT_Z     UNI_WATER_ROT_Z
#define OFF_TIPPER_ROT_X    UNI_TIPPER_ROT_X
#define OFF_TIPPER_ROT_Y    UNI_TIPPER_ROT_Y
#define OFF_TIPPER_ROT_Z    UNI_TIPPER_ROT_Z
#define OFF_SWIRL_MESH      UNI_MESH_6
#define OFF_SWIRL_RENDER    UNI_MESH_7
#define OFF_SWIRL_POS_X     UNI_MESH_15
#define OFF_SWIRL_POS_Y     UNI_MESH_12
#define OFF_SWIRL_POS_Z     UNI_MESH_13
#define OFF_SWIRL_ANGLE1    UNI_MESH_2
#define OFF_SWIRL_SPEED     UNI_MESH_4
#define OFF_SWIRL_ANGLE2    UNI_MESH_5
#define OFF_WHEEL_EMBED_X   UNI_WHEELEMBED_X
#define OFF_WHEEL_EMBED_Y   UNI_WHEELEMBED_Y
#define OFF_WHEEL_EMBED_Z   UNI_WHEELEMBED_Z
#define OFF_WHEEL_EMBED_VX  UNI_WHEELEMBED_VX
#define OFF_WHEEL_EMBED_VY  UNI_WHEELEMBED_VY
#define OFF_WHEEL_EMBED_VZ  UNI_WHEELEMBED_VZ
#define OFF_BONK_STORE      UNI_BONK_STORE
#define OFF_BELL_OBJ        UNI_BELL_OBJ
#define OFF_SAW1_OBJ        UNI_SAW1_OBJ
#define OFF_SAW2_OBJ        UNI_SAW2_OBJ
#define OFF_SAW2_ALERT_OBJ  UNI_SAW2_ALERT_OBJ
#define OFF_BRIDGE_ANGLE    UNI_BRIDGE_ANGLE
#define OFF_BRIDGE_STATE    UNI_BRIDGE_STATE
#define OFF_BRIDGE_COUNTER  UNI_BRIDGE_COUNTER
#define OFF_WINDMILL_X      UNI_WINDMILL_X
#define OFF_WINDMILL_Y      UNI_WINDMILL_Y
#define OFF_WINDMILL_Z      UNI_WINDMILL_Z
#define OFF_WINDMILL_ANGLE  UNI_WINDMILL_ANGLE
#define OFF_WINDMILL_SPEED  UNI_WINDMILL_SPEED
#define OFF_WINDMILL_STATE  UNI_WINDMILL_STATE
#define OFF_WINDMILL_COUNTER UNI_WINDMILL_COUNTER
#define OFF_WINDMILL_DECAY  UNI_WINDMILL_DECAY
#define OFF_BITE_STATE      UNI_BITE_STATE
#define OFF_BITE_SPEED      UNI_BITE_SPEED
#define OFF_BUMPER_LIT      UNI_BUMPER_LIT
#define OFF_SKY_POPCYL_BASE UNI_SKY_POPCYL_BASE
#define OFF_SKY_TIMER       UNI_SKY_TIMER
#define OFF_EHVECTOR        UNI_EHVECTOR
#define OFF_LIST_0          UNI_LIST_0
#define OFF_LIST_1          UNI_LIST_1
#define OFF_LIST_2          UNI_LIST_2
#define OFF_LIST_3          UNI_LIST_3
#define OFF_SWIRL_LIST      UNI_LIST_0
#define OFF_TARBUBBLE_LIST  UNI_LIST_1
#define OFF_CATAPULT_LIST   UNI_LIST_2
#define OFF_MACE_LIST       UNI_LIST_2  /* share */

#define OFF_CATAPULT_MESH   UNI_CATAPULT_MESH
#define OFF_MACE_MESH       UNI_MACE_MESH
#define OFF_DRAWBRIDGE_MESH UNI_DRAWBRIDGE_MESH
#define OFF_BONK_MESH       UNI_BONK_MESH
#define OFF_WINDMILL_MESH   UNI_WINDMILL_MESH
#define OFF_CHOMPER_MESH    UNI_CHOMPER_MESH
#define OFF_TURRET_MESH     UNI_TURRET_MESH
#define OFF_SAWBLADE1_OBJ   UNI_SAWBLADE1_OBJ
#define OFF_SAWBLADE2_OBJ   UNI_SAWBLADE2_OBJ
#define OFF_BONK_STORE2     UNI_BONK_STORE2
#define OFF_BLOCKDAWG1_MESH UNI_BLOCKDAWG1_MESH
#define OFF_BLOCKDAWG2_MESH UNI_BLOCKDAWG2_MESH
#define OFF_LIFTER_MESH     UNI_LIFTER_MESH
#define OFF_ROTATOR_MESH    UNI_ROTATOR_MESH
#define OFF_DFLOOR_BASE     UNI_DFLOOR_BASE
#define OFF_WOBBLY_BASE     UNI_WOBBLY_BASE
#define OFF_POPCYL_MESH_BASE UNI_POPCYL_MESH_BASE
#define OFF_BBRIDGE1_MESH   UNI_BBRIDGE1_MESH
#define OFF_BBRIDGE2_MESH   UNI_BBRIDGE2_MESH

/* Per-board cached collision flags (built once after S1 scan) — 1 byte per event */
#define OFF_COLLISION_FLAGS 0xB950  /* 64 bytes: 0xB950-0xB98F */
#define COLL_FLAG_BUMPER       (OFF_COLLISION_FLAGS+0)
#define COLL_FLAG_BRIDGE       (OFF_COLLISION_FLAGS+1)
#define COLL_FLAG_WATERWHEEL   (OFF_COLLISION_FLAGS+2)
#define COLL_FLAG_WHEELEMBED   (OFF_COLLISION_FLAGS+3)
#define COLL_FLAG_SWIRL        (OFF_COLLISION_FLAGS+4)
#define COLL_FLAG_CATAPULTBOTTOM (OFF_COLLISION_FLAGS+5)
#define COLL_FLAG_OPENSESAME   (OFF_COLLISION_FLAGS+6)
#define COLL_FLAG_TRAPDOOR     (OFF_COLLISION_FLAGS+7)
#define COLL_FLAG_BITE         (OFF_COLLISION_FLAGS+8)
#define COLL_FLAG_MACETRIGGER  (OFF_COLLISION_FLAGS+9)
#define COLL_FLAG_MACE         (OFF_COLLISION_FLAGS+10)
#define COLL_FLAG_HELPINERTIA  (OFF_COLLISION_FLAGS+11)
#define COLL_FLAG_UNHELPINERTIA (OFF_COLLISION_FLAGS+12)
#define COLL_FLAG_VACPOPOUT    (OFF_COLLISION_FLAGS+13)
#define COLL_FLAG_SPEEDCYLINDER (OFF_COLLISION_FLAGS+14)
#define COLL_FLAG_EXTRATIME    (OFF_COLLISION_FLAGS+15)
#define COLL_FLAG_NEONPLATFORM (OFF_COLLISION_FLAGS+16)
#define COLL_FLAG_ZOOP         (OFF_COLLISION_FLAGS+17)
#define COLL_FLAG_LIGHTSOFF    (OFF_COLLISION_FLAGS+18)
#define COLL_FLAG_LIGHTSON     (OFF_COLLISION_FLAGS+19)
#define COLL_FLAG_CALLHAMMER   (OFF_COLLISION_FLAGS+20)
#define COLL_FLAG_HAMMERCHASE  (OFF_COLLISION_FLAGS+21)
#define COLL_FLAG_ALERTSAW1    (OFF_COLLISION_FLAGS+22)
#define COLL_FLAG_ALERTSAW2    (OFF_COLLISION_FLAGS+23)
#define COLL_FLAG_ALERTSAW3    (OFF_COLLISION_FLAGS+24)
#define COLL_FLAG_ACTIVATESAW1 (OFF_COLLISION_FLAGS+25)
#define COLL_FLAG_ACTIVATESAW2 (OFF_COLLISION_FLAGS+26)
#define COLL_FLAG_ALERTJUDGES  (OFF_COLLISION_FLAGS+27)
#define COLL_FLAG_SCORE        (OFF_COLLISION_FLAGS+28)
#define COLL_FLAG_BELL         (OFF_COLLISION_FLAGS+29)
#define COLL_FLAG_GRAVITY      (OFF_COLLISION_FLAGS+30)
#define COLL_FLAG_JUMPFIRST    (OFF_COLLISION_FLAGS+31)
#define COLL_FLAG_JUMPSECOND   (OFF_COLLISION_FLAGS+32)
#define COLL_FLAG_SHRINK       (OFF_COLLISION_FLAGS+33)
#define COLL_FLAG_GROWSOUND    (OFF_COLLISION_FLAGS+34)
#define COLL_FLAG_GROW         (OFF_COLLISION_FLAGS+35)
#define COLL_FLAG_DROPLIFT     (OFF_COLLISION_FLAGS+36)
#define COLL_FLAG_PIPERANDOM   (OFF_COLLISION_FLAGS+37)
#define COLL_FLAG_LIMIT        (OFF_COLLISION_FLAGS+38)
#define COLL_FLAG_LIMITX       (OFF_COLLISION_FLAGS+39)
#define COLL_FLAG_LIMITZ       (OFF_COLLISION_FLAGS+40)
#define COLL_FLAG_LIMITPIPE1   (OFF_COLLISION_FLAGS+41)
#define COLL_FLAG_SWALLOW      (OFF_COLLISION_FLAGS+42)
#define COLL_FLAG_LIMITPIPE2   (OFF_COLLISION_FLAGS+43)
#define COLL_FLAG_BRANCH       (OFF_COLLISION_FLAGS+44)
#define COLL_FLAG_SPINNY       (OFF_COLLISION_FLAGS+45)
#define COLL_FLAG_SAWTEETH     (OFF_COLLISION_FLAGS+46)
#define COLL_FLAG_SPINNER      (OFF_COLLISION_FLAGS+47)
#define COLL_FLAG_LAUNCH       (OFF_COLLISION_FLAGS+48)
#define COLL_FLAG_SQUAREWOBBLY (OFF_COLLISION_FLAGS+49)
#define COLL_FLAG_WAVY         (OFF_COLLISION_FLAGS+50)
#define COLL_FLAG_GLASS        (OFF_COLLISION_FLAGS+51)
#define COLL_FLAG_TENBONUS1    (OFF_COLLISION_FLAGS+52)
#define COLL_FLAG_TENBONUS2    (OFF_COLLISION_FLAGS+53)
#define COLL_FLAG_PEGS         (OFF_COLLISION_FLAGS+54)
#define COLL_FLAG_TRAPPOP      (OFF_COLLISION_FLAGS+55)
#define COLL_FLAG_NOPEGS       (OFF_COLLISION_FLAGS+56)
#define COLL_FLAG_HEATON       (OFF_COLLISION_FLAGS+57)
#define COLL_FLAG_HEATOFF      (OFF_COLLISION_FLAGS+58)
#define COLL_FLAG_BOUNCE       (OFF_COLLISION_FLAGS+59)
#define COLL_FLAG_ONROTATOR    (OFF_COLLISION_FLAGS+60)
#define COLL_FLAG_ONGEAR       (OFF_COLLISION_FLAGS+61)
#define COLL_FLAG_COUNT        62


/* ═══════════════════════════════════════════════════════════════════════════
 * Object type system — extensible per-level feature toggles
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef enum {
    OBJ_BUMPERS = 0,
    OBJ_BRIDGE,
    OBJ_COUNT
} ObjectType;

static const char *g_objectNames[OBJ_COUNT] __attribute__((unused)) = {
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
} UpdateFeature;

/* Render feature flags — separate from update features.
 * Controls which per-level render blocks run in UniversalRender (vtable slot 24). */
typedef enum {
    REND_NONE         = 0,
    REND_BUMPER       = 1 << 0,  /* Bumper reflective material (Beginner, Toob, Master) */
    REND_WINDMILL     = 1 << 1,  /* Tower windmill wheel */
    REND_GLASS        = 1 << 2,  /* Glass transparent smasher render */
    REND_SKY_BOX      = 1 << 3,  /* Sky box: far-clip + CLOUDSCAPE sprite (points 1-3) */
    REND_SKY_LIST     = 1 << 4,  /* Sky transparent list (point 4) */
    REND_CHOMPER      = 1 << 5,  /* Tower chomper mouth */
    REND_TURRET       = 1 << 6,  /* Tower turret (tower) */
    REND_SKY_CAM      = REND_SKY_BOX, /* alias for compat */
} RenderFeature;

/* Render features are NOT hardcoded per level.
 * They are determined dynamically at render time based on what objects/events
 * are actually enabled for the current level:
 *   REND_BUMPER:   active when N:BUMPER collision event is enabled
 *   REND_WINDMILL: active when WINDMILL S1 / REND_TOWER_WINDMILL !=0
 *   REND_CHOMPER:  active when CHOMPER S1 / REND_TOWER_CHOMPER !=0
 *   REND_TURRET:   active when TURRET* S1 / REND_TOWER_TURRET !=0
 *   REND_GLASS:    active when N:GLASS collision event is enabled
 *   REND_SKY_BOX:  active when CLOUDSCAPE S1 / REND_SKY_SPRITE !=0 (skybox + clouds)
 *   REND_SKY_LIST: active when Sky transparent list has objects
 * Computed in UniversalRender, not set from defaults. */
/* ═══════════════════════════════════════════════════════════════════════════
 * Extension heap per-board map — Option B
 * g_extMap tracks HeapAlloc blocks keyed by board pointer.
 * g_extFeat is per-board auto-enable bitmask (replaces g_updateFeatures[level]).
 * ═══════════════════════════════════════════════════════════════════════════ */
#define MAX_EXT_MAP 32
typedef struct { void* board; void* ext; DWORD feat; DWORD extSize; } ExtEntry;
static ExtEntry g_extMap[32] = {{0}};

/* Board bounds check via VirtualQuery — IsBadWritePtr only checks page
 * writability, not HeapAlloc bounds. This checks committed region size. */
static int BoardHasOffset(void* board, DWORD offset, DWORD need) {
    if (!board) return 0;
    MEMORY_BASIC_INFORMATION mbi;
    if (!VirtualQuery(board, &mbi, sizeof(mbi))) return 0;
    if (mbi.State != MEM_COMMIT) return 0;
    DWORD base = (DWORD)board;
    DWORD regionEnd = (DWORD)mbi.BaseAddress + mbi.RegionSize;
    if (base + offset + need > regionEnd) return 0;
    /* Heap blocks are sub-allocations — VirtualQuery over-estimates. Use HeapSize
     * for precise check when this is a Heap block. If HeapValidate says not a heap
     * block, VirtualQuery result stands. */
    if (HeapValidate(GetProcessHeap(), 0, board)) {
        DWORD hs = HeapSize(GetProcessHeap(), 0, board);
        if (hs == (DWORD)-1) return 0;
        if (offset + need > hs) return 0;
    }
    return 1;
}
static int ExtHasOffset(void* ext, DWORD offset, DWORD need) {
    if (!ext) return 0;
    int i;
    for (i = 0; i < MAX_EXT_MAP; i++) if (g_extMap[i].ext == ext) {
        if (offset + need > g_extMap[i].extSize) return 0;
        return 1;
    }
    /* Not in map — stale or unknown ext. Use HeapSize precisely, fail closed. */
    if (HeapValidate(GetProcessHeap(), 0, ext)) {
        DWORD hs = HeapSize(GetProcessHeap(), 0, ext);
        if (hs == (DWORD)-1) return 0;
        if (offset + need > hs) return 0;
        return 1;
    }
    return 0;
}

static void* GetBoardExt(void* board) {
    if (!board) return NULL;
    int i;
    for (i = 0; i < MAX_EXT_MAP; i++) {
        if (g_extMap[i].board == board && g_extMap[i].ext) {
            /* Validate ext is still a live HeapAlloc block, not freed */
            if (HeapValidate(GetProcessHeap(), 0, g_extMap[i].ext)) return g_extMap[i].ext;
        }
    }
    /* NOTE: board+EXT_PTR (0xAB00) is BEYOND vanilla board alloc (~0x4400 for
     * WarmUp). IsBadReadPtr only checks page writability, NOT allocation
     * bounds, so reading/writing board+0xAB00 heap-corrupts. g_extMap is the
     * sole source of truth — no board+EXT_PTR fallback. */
    return NULL;
}
static DWORD GetBoardFeat(void* board) {
    int i;
    for (i = 0; i < MAX_EXT_MAP; i++) if (g_extMap[i].board == board) return g_extMap[i].feat;
    return 0;
}
static void OrBoardFeat(void* board, DWORD bits) {
    int i, freeIdx = -1;
    for (i = 0; i < MAX_EXT_MAP; i++) {
        if (g_extMap[i].board == board) { g_extMap[i].feat |= bits; return; }
        if (freeIdx==-1 && !g_extMap[i].board) freeIdx=i;
    }
    if (freeIdx!=-1) { g_extMap[freeIdx].board = board; g_extMap[freeIdx].feat = bits; g_extMap[freeIdx].extSize = EXT_SIZE; }
}
static void SetBoardExt(void* board, void* ext) {
    int i, freeIdx=-1;
    for (i=0;i<MAX_EXT_MAP;i++) {
        if (g_extMap[i].board == board) { g_extMap[i].ext = ext; g_extMap[i].extSize = EXT_SIZE; return; }
        if (freeIdx==-1 && !g_extMap[i].board) freeIdx=i;
    }
    if (freeIdx!=-1) { g_extMap[freeIdx].board = board; g_extMap[freeIdx].ext = ext; g_extMap[freeIdx].extSize = EXT_SIZE; }
}
static void FreeBoardExt(void* board) {
    int i;
    for (i=0;i<MAX_EXT_MAP;i++) if (g_extMap[i].board == board) {
        if (g_extMap[i].ext) {
            if (HeapValidate(GetProcessHeap(), 0, g_extMap[i].ext)) HeapFree(GetProcessHeap(),0,g_extMap[i].ext);
            g_extMap[i].ext=NULL;
            g_extMap[i].extSize=0;
        }
        g_extMap[i].feat=0;
        g_extMap[i].board=NULL;
        break;
    }
    /* No board+EXT_PTR cleanup — that slot is OOB on small boards and would heap-corrupt. */
}
static void* EnsureBoardExt(void* board) {
    void* ext = GetBoardExt(board);
    if (ext) return ext;
    DWORD extSize = EXT_SIZE;
    ext = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, extSize);
    if (!ext) return NULL;
    SetBoardExt(board, ext);
    /* Tag size for ExtHasOffset */
    {
        int i;
        for (i=0;i<MAX_EXT_MAP;i++) if (g_extMap[i].board == board && g_extMap[i].ext == ext) { g_extMap[i].extSize = extSize; break; }
    }
    /* No board+EXT_PTR mirror — OOB on vanilla boards (~0x4400), heap corrupts. */
    {
        char dbg[128];
        wsprintfA(dbg, "EnsureBoardExt: board=%p ext=%p size=0x%X", board, ext, extSize);
        DebugLog(dbg);
    }
    return ext;
}
 /* 1 if [FEATURES] section overrode defaults for this level */
typedef void (__fastcall *Scene_Update_t)(void *board);
typedef void (__fastcall *Board_UpdateRaceState_t)(void *board);
typedef void (__fastcall *Level_RenderDynamicObjects_t)(void *board);
typedef void (__thiscall *Graphics_SetProjection_t)(void *board, float near_plane, float far_plane);
typedef void (__thiscall *Graphics_SetCullMode2_t)(void *gfx, int mode);
typedef void (__thiscall *Sprite_RenderQuad_t)(void *sprite, float a, float b, float c, float d, int e);
typedef void (__thiscall *RenderContext_Init_t)(void *out);
typedef void (__cdecl *Matrix4_Identity_t)(void *out);
typedef void (__thiscall *Gfx_ScaleFn_t)(void *gfx, float val);
typedef void (__thiscall *Gfx_SetPosition_t)(void *gfx, float x, float y, float z);
typedef void (__fastcall *Timer_Init_t)(void *out);
typedef void (__fastcall *Timer_Cleanup_t)(void *out);
typedef void (__stdcall *Matrix_TransformVec3_t)(float *out, float *in);
typedef void (__fastcall *Matrix44_Zero_t)(int *out);
typedef void (__thiscall *Scene_ForEachBall_SetVelocity_t)(void *board, float x, float y, float z);
typedef void *__thiscall (*FUN_0044fa90_t)(void *out, int app, int tarList);
typedef void *__thiscall (*FUN_0044fb50_t)(void *out, int app, float x, int y, float z);
typedef void (__fastcall *FUN_00405190_t)(int ball);
/* CPUID_CheckProcessorFeature (RNG) is __thiscall: ECX=this, stack=[range, flag].
 * Use __fastcall with a dummy EDX param: ECX=this, EDX=dummy, stack=[range, flag].
 * The callee reads [ebp+8]=range and [ebp+12]=flag — matches __thiscall. */
static DWORD g_RNG_raw = 0;  /* forward declaration — full assignment in InitPointers */
static int __fastcall RNG_call(void *this_ptr, int dummy_edx, int range, char flag) {
    return ((int (__thiscall *)(void*, int, char))g_RNG_raw)(this_ptr, range, flag);
}
typedef int (__thiscall *CPUID_RNG_t)(void *ptr, int range, char flag);
typedef void *__thiscall (*BadBall_ctor_t)(void *mem, int board);
typedef void (__thiscall *Ball_SetTrajectory_t)(void *ball, int unk, float x, float y, float f1, float f2);
typedef void (__thiscall *Ball_SetVec3AtOffset_t)(void *ball, float *vec);
typedef void (__thiscall *Vec3_NormalizeAndScale_t)(float *vec, float scale);
typedef void (__thiscall *Vec3_Copy_t)(float *dst, float *src);
typedef float (__thiscall *Sound_CalcDistAtten_t)(int soundDevice, float x, float y, float z);
typedef void (__thiscall *Sound_Play3DAtPos_t)(int channel);
typedef void (__thiscall *Scene_SetRaceActive_t)(int obj);
typedef void (__thiscall *Scene_AddObject_t)(void *scene, void *obj);

static Scene_Update_t             g_SceneUpdate = NULL;
static Board_UpdateRaceState_t    g_BoardUpdateRaceState = NULL;

/* Saved original vtable[19] (RaceState) for each level (1-15).
 * Several levels have custom RaceState handlers that iterate per-level
 * data at board+0x436C (Up: lifter AthenaList, Neon: render objects,
 * Beginner: float timers).  We must call the original after our shared
 * Board_UpdateRaceState so those per-level systems keep running. */
typedef void (__fastcall *RaceState_t)(void *board);
static RaceState_t g_origRaceState[16] = { NULL }; /* index 1-15 */
static Level_RenderDynamicObjects_t g_RenderDynamicObjects = NULL;
static Graphics_SetProjection_t  g_GraphicsSetProjection = NULL;
static Graphics_SetCullMode2_t    g_GraphicsSetCullMode2 = NULL;
static Sprite_RenderQuad_t         g_SpriteRenderQuad = NULL;
static RenderContext_Init_t       g_RenderContextInit = NULL;
static Matrix4_Identity_t         g_Matrix4Identity = NULL;
static Gfx_ScaleFn_t              g_GfxScaleZ = NULL;
static Gfx_ScaleFn_t              g_GfxScaleY = NULL;
static Gfx_ScaleFn_t              g_GfxScaleX = NULL;
static Gfx_SetPosition_t          g_GfxSetPosition = NULL;
static Timer_Init_t               g_TimerInit = NULL;
static Timer_Cleanup_t            g_TimerCleanup = NULL;
static Matrix_TransformVec3_t     g_MatrixTransformVec3 = NULL;
static Matrix44_Zero_t            g_Matrix44Zero = NULL;
static Scene_ForEachBall_SetVelocity_t g_SceneForEachBallSetVelocity = NULL;
static FUN_0044fa90_t             g_CreateTarBubble = NULL;
static FUN_0044fb50_t             g_CreateSplashParticle = NULL;
static FUN_00405190_t             g_RemoveBall = NULL;
static CPUID_RNG_t                g_RNG = NULL;
/* Wave table fns (defined later) — forward declare for UniversalRenderImpl Tower bob */
typedef float (__thiscall *Wave_Fn_t)(void *table, float angle);
static Wave_Fn_t                  g_WaveSin = NULL;
/* g_RNG_raw declared earlier (before RNG_call) */
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
#define RVA_Level_RenderDynamicObjects 0x0000B420
#define RVA_Graphics_SetProjection     0x00054AB0
#define RVA_Graphics_SetCullMode2      0x00053970
#define RVA_Sprite_RenderQuad          0x0005D660
#define RVA_RenderContext_Init         0x00057FA0
#define RVA_Matrix4_Identity           0x00057FD0
#define RVA_Gfx_ScaleZ                0x00057CC0
#define RVA_Gfx_ScaleY                0x00057C90
#define RVA_Gfx_ScaleX                0x00057C60
#define RVA_Gfx_SetPosition           0x00057B50
#define RVA_Timer_Init                0x00057A40
#define RVA_Timer_Cleanup             0x00057A50
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

/* Collision handler RVAs */
#define RVA_Sound_PlayChannel         0x000597B0
#define RVA_Ball_DizzyImmunity        0x00002400
#define RVA_Ball_Grow                 0x00002270
#define RVA_Ball_Shrink               0x00002200
#define RVA_Ball_SetName              0x00001660
#define RVA_Ball_ApplyTrajectory      0x00003750
#define RVA_Ball_SetTiltedGravity     0x00003100
#define RVA_Ball_SetFlatGravity       0x00003150
#define RVA_Rotator_MarkTriggered    0x000371F0
#define RVA_Rotator_PlayCollisionSound 0x00034030
#define RVA_Rotator_TriggerSound      0x00036CF0
#define RVA_Rotator_StartSound        0x000367E0
#define RVA_Rotator_AddBall           0x0003B6F0
#define RVA_CheckArenaUnlock          0x000ABA0
#define RVA_Catapult_Launch           0x00034290
#define RVA_Trapdoor_Open             0x000344D0
#define RVA_Trapdoor_Activate         0x00038410
#define RVA_Saw_AlertActivate         0x00034770
#define RVA_Saw_Activate              0x00034A50
#define RVA_Bell_Activate             0x00034E20
#define RVA_Judge_Reset               0x00034C40
#define RVA_ScoreDisplay_SetTime      0x00034C80
#define RVA_CreateBonkPopup           0x00038B30
#define RVA_Hammer_ChaseStart         0x00038BB0
#define RVA_Pendulum_PlayCollisionSound 0x00036B70
#define RVA_Pendulum_AddIndex         0x00036390
#define RVA_ScoreObject_ctor          0x0004BE80
#define RVA_Timer_Decrement           0x0004BEF0
#define RVA_ArenaScoreParticle_ctor   0x0004AD50
#define RVA_AthenaHashTable_Lookup    0x000605E0
#define RVA_AthenaList_ContainsValue  0x00053610
#define RVA_SceneObject_sub1_ctor     0x000694F0
#define RVA_AthenaString_Set          0x00069510
#define RVA_MWParser_ReadTag           0x00069600
#define RVA_StreamReader_dtor         0x000694C0
#define RVA_Audio_PlayMusic           0x0006A310
#define RVA_Difficulty_GetTimeModifier 0x00028ED0
#define RVA_AthenaString_Format       0x00066C70
#define RVA_AthenaString_SprintfToBuffer 0x000BAE43
#define RVA_Wave_Cos                  0x00057DC0
#define RVA_Wave_Sin                  0x00057DA0
#define RVA_Scene_RegisterObject      0x00053BD0
#define RVA_AthenaList_RemoveByValue  0x00053690
#define RVA_NeonPlatform_Activate     0x00037300
#define RVA_SquareWobbly_Activate     0x0003ACB0
#define RVA_Wavy_Activate             0x0003AEF0
#define RVA_Spinner_Activate          0x0003DCF0
#define RVA_Gear_AddBall              0x0003E9C0
#define RVA_NormalGravityReset        0x000030B0
#define RVA_DropLift_Activate         0x00035170
#define RVA_CPUID_RNG_Fn              0x0005DD60

/* Board field offsets for feature blocks — ALL ARE BYTE OFFSETS.
 * (Previous versions used Ghidra DWORD array indices, which are offset/4.
 * All values below have been corrected to actual byte offsets.) */

/* Bridge animation (Intermediate) — render obj + pivot point + state machine
 * meshWorld is stored at UNI_BONK_STORE (0x8620) by LoadExtraMeshes/InitBridge. */
#define BRD_BRIDGE_RENDER   UNI_BONK_STORE  /* meshWorld ptr (0x8620) */
#define BRD_BRIDGE_PIVOT_X  UNI_BRIDGE_PIVOT_X  /* float: bridge pivot X (0xB998) */
#define BRD_BRIDGE_PIVOT_Y  UNI_BRIDGE_PIVOT_Y  /* float: bridge pivot Y (0xB99C) */
#define BRD_BRIDGE_PIVOT_Z  UNI_BRIDGE_PIVOT_Z  /* float: bridge pivot Z (0xB9A0) */
#define BRD_BRIDGE_ANGLE    UNI_BRIDGE_ANGLE   /* float: current tilt angle (starts 45.0) */
#define BRD_BRIDGE_STATE    UNI_BRIDGE_STATE   /* int: 0=wait, 1=tilt down, 2=wait, 3=tilt back */
#define BRD_BRIDGE_COUNTER  UNI_BRIDGE_COUNTER /* int: frame counter for current state */

/* Windmill (Tower) — dedicated offsets */
#define BRD_WM_RENDER       UNI_WM_RENDER   /* windmill render object ptr (0xB9D8) */
#define BRD_WM_POS_X        UNI_WINDMILL_X      /* float: windmill X */
#define BRD_WM_POS_Y        UNI_WINDMILL_Y      /* float: windmill Y */
#define BRD_WM_POS_Z        UNI_WINDMILL_Z      /* float: windmill Z */
#define BRD_WM_ANGLE         UNI_WINDMILL_ANGLE   /* float: current rotation angle */
#define BRD_WM_SPEED         UNI_WINDMILL_SPEED   /* float: current spin speed */
#define BRD_WM_STATE         UNI_WINDMILL_STATE   /* int: 0=spin up, 1=creak, 2=spin down, 3=pause */
#define BRD_WM_COUNTER       UNI_WINDMILL_COUNTER /* int: frame counter */
#define BRD_WM_DECAY_VAL     UNI_WINDMILL_DECAY   /* float: decay value for pause state */

/* BadBall spawner (Odd) — dedicated slots in 0xB9A4+ tail */
#define BRD_BB_FLAG         UNI_BB_FLAG      /* byte: spawn enabled flag (0xB9A4) */
#define BRD_BB_COUNTER       UNI_BB_COUNTER  /* int: frames until next spawn (0xB9A8) */
#define BRD_BB_TOTAL         UNI_BB_TOTAL    /* int: total spawned so far (0xB9AC) */
#define BRD_BB_LAST_IDX      UNI_BB_LAST_IDX /* int: last spawn position index (0xB9B0) */
#define BRD_BB_POS_TABLE     UNI_BB_POS_TABLE /* 3x3 float table, 36B (0xB9B4) */

/* Swirl (Dizzy) — offset mapping aligned with original game.
 * Original: primary=board+0x4BA8 (WaterWheel), secondary=board+0x4BC4 (Swirl).
 * Primary pos comes from WATERWHEEL handler, secondary pos from SWIRL handler. */
#define BRD_SWIRL_LIST       UNI_SWIRL_LIST      /* AthenaList of swirl zones */
#define BRD_TARBUBBLE_LIST   UNI_LIST_3          /* AthenaList of TarBubble objects */
#define BRD_SWIRL_MESH1      UNI_MESH_0          /* primary mesh = WaterWheel (0x85E0) */
#define BRD_SWIRL_MESH2      UNI_MESH_6          /* secondary mesh = Swirl (0x85F8) */
#define BRD_SWIRL1_POS_X     UNI_WHEELEMBED_X    /* set by WATERWHEEL handler */
#define BRD_SWIRL1_POS_Y     UNI_WHEELEMBED_Y
#define BRD_SWIRL1_POS_Z     UNI_WHEELEMBED_Z
#define BRD_SWIRL1_ANGLE     UNI_MESH_2          /* dedicated float (0x85E8) */
#define BRD_SWIRL1_SPEED     UNI_MESH_4          /* dedicated float (0x85F0) */
#define BRD_SWIRL2_POS_X     UNI_MESH_15         /* set by SWIRL handler (0x861C) */
#define BRD_SWIRL2_POS_Y     UNI_MESH_12         /* (0x8610) */
#define BRD_SWIRL2_POS_Z     UNI_MESH_13         /* (0x8614) */
#define BRD_SWIRL2_ANGLE     UNI_MESH_5          /* dedicated float (0x85F4) */

/* Swirl (Master) — same unified offsets now */
#define BRD_SWIRL_LIST_M     UNI_SWIRL_LIST
#define BRD_TARBUBBLE_LIST_M UNI_LIST_3

/* Bumper decay offsets — unified */
#define BRD_BUMPER_DECAY_BEG  UNI_BUMPER_LIT
#define BRD_BUMPER_DECAY_TOOB UNI_BUMPER_LIT
#define BRD_BUMPER_DECAY_MAST UNI_BUMPER_LIT

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
 * UNIVERSAL BOARD OFFSETS — same offsets for ALL levels
 * ALL levels read and write from the SAME offsets. No per-level variation.
 * Layout: EHVector first (largest: 8×0x418=0x20C0), then mesh/working data,
 * then AthenaLists. All above max per-level board data (0x6498).
 * ═══════════════════════════════════════════════════════════════════════════ */

/* EHVector — bumper slot array (8 × 0x418 = 0x20C0 bytes) — FIRST, largest block */
#define UNI_EHVECTOR  0x6500

/* Bumper lit flags (8 × 4 bytes) — right after ehVector */
#define UNI_BUMPER_LIT 0x85C0

/* Mesh object slots (16 × 4 bytes) */
#define UNI_MESH_0    0x85E0
#define UNI_MESH_1    0x85E4
#define UNI_MESH_2    0x85E8
#define UNI_MESH_3    0x85EC
#define UNI_MESH_4    0x85F0
#define UNI_MESH_5    0x85F4
#define UNI_MESH_6    0x85F8
#define UNI_MESH_7    0x85FC
#define UNI_MESH_8    0x8600
#define UNI_MESH_9    0x8604
#define UNI_MESH_10   0x8608
#define UNI_MESH_11   0x860C
#define UNI_MESH_12   0x8610
#define UNI_MESH_13   0x8614
#define UNI_MESH_14   0x8618
#define UNI_MESH_15   0x861C

/* Working data / misc pointers (0x8620+) */
#define UNI_BONK_STORE       0x8620
#define UNI_CATAPULT_LIST    UNI_LIST_0
#define UNI_CATAPULT_COUNT   (UNI_LIST_0 + 0x04)
#define UNI_CATAPULT_DATA    (UNI_LIST_0 + 0x40C)
#define UNI_TRAPDOOR_LIST    UNI_LIST_1
#define UNI_TRAPDOOR_COUNT   (UNI_LIST_1 + 0x04)
#define UNI_TRAPDOOR_DATA    (UNI_LIST_1 + 0x40C)
#define UNI_DRAWBRIDGE_LIST  UNI_LIST_2
#define UNI_DRAWBRIDGE_COUNT (UNI_LIST_2 + 0x04)
#define UNI_DRAWBRIDGE_DATA  (UNI_LIST_2 + 0x40C)
#define UNI_MACE_LIST        UNI_LIST_3
#define UNI_MACE_COUNT       (UNI_LIST_3 + 0x04)
#define UNI_MACE_DATA        (UNI_LIST_3 + 0x40C)
#define UNI_JUDGE_LIST       UNI_LIST_4
#define UNI_JUDGE_COUNT      (UNI_LIST_4 + 0x04)
#define UNI_JUDGE_DATA       (UNI_LIST_4 + 0x40C)
#define UNI_BELL_OBJ         0x8624
#define UNI_SAW1_OBJ         0x8628
#define UNI_SAW2_OBJ         0x862C
#define UNI_SAW2_ALERT_OBJ   0x8630
#define UNI_BRIDGE_ANGLE     0x8634
#define UNI_BRIDGE_STATE     0x8638
#define UNI_BRIDGE_COUNTER   0x863C
#define UNI_WINDMILL_X       0x8640
#define UNI_WINDMILL_Y       0x8644
#define UNI_WINDMILL_Z       0x8648
#define UNI_WINDMILL_ANGLE   0x864C
#define UNI_WINDMILL_SPEED   0x8650
#define UNI_WINDMILL_STATE   0x8654
#define UNI_WINDMILL_COUNTER 0x8658
#define UNI_WINDMILL_DECAY   0x865C
#define UNI_BITE_STATE       0x8660
#define UNI_BITE_SPEED       0x8664
#define UNI_NEON_DARK_COUNT  0x8668
#define UNI_NEON_TRAPDOOR    0x866C
#define UNI_GLASS_SMASHER1   0x8670
#define UNI_GLASS_SMASHER2   0x8674
#define UNI_SKY_TRAPDOOR     0x8678
#define UNI_PEG_COUNT        0x867C
#define UNI_MAGNIFYING_GLASS 0x8680
#define UNI_POPCYL_ARRAY     0x8684
#define UNI_POPCYL_COUNTER   0x8688
#define UNI_BLOCKDAWG1       0x868C
#define UNI_BLOCKDAWG2       0x8690
#define UNI_MASTERCAT_MESH   0x8694
#define UNI_MASTERCAT_LIST   UNI_LIST_5
#define UNI_MASTERCAT_COUNT  (UNI_LIST_5 + 0x04)
#define UNI_MASTERCAT_DATA   (UNI_LIST_5 + 0x40C)
#define UNI_POPCYL1_STORE    0x8698
#define UNI_POPCYL2_STORE    0x869C
#define UNI_SWIRL_LIST       UNI_LIST_6
#define UNI_BBOARD_STORE1    0x86A0
#define UNI_BBOARD_STORE2    0x86A4
#define UNI_WHEELEMBED_X     0x86A8
#define UNI_WHEELEMBED_Y     0x86AC
#define UNI_WHEELEMBED_Z     0x86B0
#define UNI_WHEELEMBED_VX    0x86B4
#define UNI_WHEELEMBED_VY    0x86B8
#define UNI_WHEELEMBED_VZ    0x86BC

/* ═══════════════════════════════════════════════════════════════════════════
 * Dedicated per-object-type mesh slots (0x86C0-0x86FF)
 *
 * Each object type gets its own mesh pointer slot so that multiple object
 * types can coexist on the same level without overwriting each other's
 * mesh pointers. Previously Bridge/Tipper/Spinny/Looper all shared
 * UNI_BONK_STORE (0x8620), making cross-level injection impossible.
 *
 * Bridge keeps 0x8620/0x8628 (it's the most common shared object).
 * All other conflicting types move here.
 * ═══════════════════════════════════════════════════════════════════════════ */
#define UNI_TIPPER_MESH      0x86C0  /* Tipper meshWorld (Dizzy) */
#define UNI_TIPPER_RENDER    0x86C4  /* Tipper renderObj (Dizzy) */
#define UNI_SPINNY_MESH      0x86C8  /* Spinny mesh (Toob) */
#define UNI_SAW_MESH         0x86CC  /* Saw mesh (Toob) */
#define UNI_FALLOUT_MESH     0x86D0  /* Fallout mesh (Toob) */
#define UNI_GLUEBIE_MESH     0x86D4  /* Gluebie mesh (Dizzy) */
#define UNI_LOOPER_MESH      0x86D8  /* Looper mesh (Impossible) */
#define UNI_GEAR_MESH        0x86DC  /* Gear mesh (Impossible) */
#define UNI_BIGGEAR_MESH     0x86E0  /* BigGear mesh (Impossible) */
#define UNI_WATER_ROT_X      0x86E4  /* WaterWheel base rot X (fallback file) */
#define UNI_WATER_ROT_Y      0x86E8  /* WaterWheel base rot Y */
#define UNI_WATER_ROT_Z      0x86EC  /* WaterWheel base rot Z */
#define UNI_TIPPER_ROT_X     0x86F0  /* Tipper base rot X (separate from WaterWheel) */
#define UNI_TIPPER_ROT_Y     0x86F4  /* Tipper base rot Y */
#define UNI_TIPPER_ROT_Z     0x86F8  /* Tipper base rot Z */

/* ── Dedup: every shared mesh/instance slot gets its own ext offset ──
 * Previously CATAPULT/MACE/DRAWBRIDGE/BONK/WINDMILL/CHOMPER/TURRET/BONK etc
 * all aliased to UNI_BONK_STORE / UNI_MESH_3 / UNI_MESH_4 / UNI_SAW1_OBJ.
 * A level with CATAPULT+BRIDGE or MACE+DRAWBRIDGE would clobber. Each now
 * has a dedicated 4-byte slot in the high free region (0xB8D8+). */
#define UNI_CATAPULT_MESH    0xB8D8  /* Catapult meshWorld (Tower) */
#define UNI_MACE_MESH        0xB8DC  /* Mace meshWorld (Tower) */
#define UNI_DRAWBRIDGE_MESH  0xB8E0  /* Drawbridge meshWorld (Tower) */
#define UNI_BONK_MESH        0xB8E4  /* Bonk mesh/instance (Expert) */
#define UNI_WINDMILL_MESH    0xB8E8  /* Windmill meshWorld (Tower) */
#define UNI_CHOMPER_MESH     0xB8EC  /* Chomper meshWorld (Tower) */
#define UNI_TURRET_MESH      0xB8F0  /* Turret meshWorld (Tower) */
#define UNI_SAWBLADE1_OBJ    0xB8F4  /* SawBlade1 instance (Expert) */
#define UNI_SAWBLADE2_OBJ    0xB8F8  /* SawBlade2 instance (Expert) */
#define UNI_BONK_STORE2      0xB8FC  /* Bonk instance store (secondary) */
#define UNI_BLOCKDAWG1_MESH  0xB900  /* Blockdawg1 mesh */
#define UNI_BLOCKDAWG2_MESH  0xB904  /* Blockdawg2 mesh */
#define UNI_LIFTER_MESH      0xB908  /* Lifter/SpeedCyl mesh (Up) */
#define UNI_ROTATOR_MESH     0xB90C  /* Rotator mesh (Impossible) */
#define UNI_PENDULUM_MESH    0xB9DC  /* Pendulum mesh (Impossible, split from Rotator) */
#define UNI_DFLOOR_BASE      0xB910  /* DFloor1-4 meshes (4*4=16B: B910/B914/B918/B91C) */
#define UNI_WOBBLY_BASE      0xB920  /* Wobbly1-7 meshes (7*4=28B: B920..B93C) */
#define UNI_POPCYL_MESH_BASE 0xB940  /* PopCylinder meshes (2*4) */
#define UNI_BBRIDGE1_MESH    0xB948  /* BBridge1 mesh (Master) */
#define UNI_BBRIDGE2_MESH    0xB94C  /* BBridge2 mesh (Master) */

/* ── Clobber-fix: dedicated slots in free tail 0xB990+ (were sharing 0x85E8/85F0/8638/863C/8640) ──
 * FALLOUT1 was storing at BRIDGE_COUNTER (0x863C), Toob Saw2 at BRIDGE_STATE (0x8638),
 * bridge pivot at WINDMILL_X (0x8640), and BadBall table at MESH_4 (0x85F0, 36B overflow
 * into MESH_5..13 which are swirl/windmill state). All now have private slots. */
#define UNI_SAW2_TOOB_OBJ    0xB990  /* Saw2 Toob ALERTSAW3 object (was BRIDGE_STATE) */
#define UNI_FALLOUT_OBJ      0xB994  /* Fallout1 object (was BRIDGE_COUNTER) */
#define UNI_BRIDGE_PIVOT_X   0xB998  /* Bridge pivot X (was WINDMILL_X) */
#define UNI_BRIDGE_PIVOT_Y   0xB99C  /* Bridge pivot Y */
#define UNI_BRIDGE_PIVOT_Z   0xB9A0  /* Bridge pivot Z */
#define UNI_BB_FLAG          0xB9A4  /* BadBall spawn flag (was MESH_1) */
#define UNI_BB_COUNTER       0xB9A8  /* BadBall spawn counter (was MESH_2, clobbered swirl angle) */
#define UNI_BB_TOTAL         0xB9AC  /* BadBall total spawned (was MESH_3) */
#define UNI_BB_LAST_IDX      0xB9B0  /* BadBall last pos idx (was BITE_SPEED 0x8664) */
#define UNI_BB_POS_TABLE     0xB9B4  /* BadBall 3x3 spawn pos table, 36B (was MESH_4 overflow) */
#define UNI_WM_RENDER        0xB9D8  /* Windmill render obj (was MESH_4) */
#define UNI_BLOCKDAWG3_MESH  0xB9E0  /* Blockdawg3 mesh (Toob, split from B904) */
#define UNI_SPEEDCYLINDER_MESH 0xB9E4 /* SpeedCylinder mesh (Up, split from LIFTER B908) */
#define UNI_TIMEBUTTON_MESH  0xB9E8  /* TimeButton mesh (Up, split from MESH_5 0x85F4 alias) */
#define UNI_SAW_TOOB_OBJ     0xB9EC  /* Saw Toob instance (was BRIDGE_ANGLE 0x8634 clobber) */
#define UNI_BBRIDGE1_OBJ     0xB9F0  /* BBridge1 instance (was MESH_7 0x85FC clobber) */
#define UNI_BBRIDGE2_OBJ     0xB9F4  /* BBridge2 instance (was MESH_8 0x8600) */
#define UNI_CHOMP_TIMER      0xB9F8  /* Chomper Wave_Sin timer (mirror board+0x43A4, +=3.0/frame) */
/* Next free: 0xB9FC, tail to 0xC000 = 0x604 bytes remaining */

/* Sky popcyl array (16 × 4 = 64 bytes) */
#define UNI_SKY_POPCYL_BASE 0x8700
#define UNI_SKY_TIMER        0x8740

/* S1-driven collision list (Option B) — per-board N:/E: names discovered via S1 scan */
#define OFF_COLLISION_COUNT 0xA8D0  /* int count */
#define OFF_COLLISION_NAMES 0xA8D4  /* 128 * 32 bytes = 0x1000, ends at 0xB8D4 (fits in 0xC000) */
static int IsS1CollisionEnabled(void *board, const char *eventName);
#define MAX_S1_COLLISIONS 128
#define S1_COLLISION_NAME_LEN 32
/* AthenaList slots (8 × 0x410 = 0x2080 bytes) */
#define UNI_LIST_0    0x8800
#define UNI_LIST_1    0x8C10
#define UNI_LIST_2    0x9020
#define UNI_LIST_3    0x9430
#define UNI_LIST_4    0x9840
#define UNI_LIST_5    0x9C50
#define UNI_LIST_6    0xA060
#define UNI_LIST_7    0xA470

/* ═══════════════════════════════════════════════════════════════════════════
 * Dedicated Render Data Section (0xA880–0xA8C4, 68 bytes)
 *
 * These offsets are used exclusively by UniversalRender's feature blocks.
 * They are separate from the shared UNI_MESH_* / UNI_BONK_STORE / etc. slots
 * so that render features can coexist on the same level without conflicts.
 * (e.g. Glass smashers + Tower windmill on the same level.)
 *
 * Layout:
 *   Glass:  smasher1_xyz(12) + smasher2_xyz(12) + transp1(4) + transp2(4) + flags(2) = 34B
 *   Tower:  windmill_render(4) + chomper_mesh(4) + turret_render(4) = 12B
 *   Sky:    mesh(4) + sprite(4) + sprite_xyz(12) = 20B
 *   Sky render list uses UNI_LIST_7 (already allocated, 0x410 bytes at 0xA470)
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Glass render data — written by SMASHER1/SMASHER2 handlers, read by REND_GLASS.
 * Original game stores 3 position floats (X, Y, Z) and passes them to
 * Timer vtable[0x08] — there is NO mesh pointer, just position data. */
#define REND_GLASS_S1_X     0xA880  /* smasher 1 position X (float) */
#define REND_GLASS_S1_Y     0xA884  /* smasher 1 position Y (float) */
#define REND_GLASS_S1_Z     0xA888  /* smasher 1 position Z (float) */
#define REND_GLASS_S2_X     0xA88C  /* smasher 2 position X (float) */
#define REND_GLASS_S2_Y     0xA890  /* smasher 2 position Y (float) */
#define REND_GLASS_S2_Z     0xA894  /* smasher 2 position Z (float) */
#define REND_GLASS_TRANSP1  0xA898
#define REND_GLASS_TRANSP2  0xA89C
#define REND_GLASS_FLAG1    0xA8A0  /* byte */
#define REND_GLASS_FLAG2    0xA8A1  /* byte */

/* Tower render data — written by WINDMILL/TURRET/CHOMPER handlers, read by REND_WINDMILL */
#define REND_TOWER_WINDMILL 0xA8A4  /* windmill render obj pointer */
#define REND_TOWER_CHOMPER  0xA8A8  /* chomper mesh pointer */
#define REND_TOWER_TURRET   0xA8AC  /* turret render obj pointer */
#define REND_TOWER_CHOMP_X  0xA8C4  /* chomper position X */
#define REND_TOWER_CHOMP_Y  0xA8C8  /* chomper position Y */
#define REND_TOWER_CHOMP_Z  0xA8CC  /* chomper position Z */

/* Sky render data — written by LoadExtraMeshes/SKY handler, read by REND_SKY_CAM */
#define REND_SKY_MESH       0xA8B0  /* transparent mesh pointer */
#define REND_SKY_SPRITE     0xA8B4  /* cloud sprite pointer */
#define REND_SKY_SPRITE_X   0xA8B8
#define REND_SKY_SPRITE_Y   0xA8BC
#define REND_SKY_SPRITE_Z   0xA8C0
#define REND_SKY_LIST       UNI_LIST_7

/* Board structure offsets — shared across all levels (from Board_ctor base layout) */
#define UNI_BOARD_NAME      0x868
#define UNI_BOARD_APPVAL    0x870
#define UNI_SCORE_LIST      0x8B8
#define UNI_TARBUBBLE_LIST  UNI_LIST_3
#define UNI_OBJ_LIST        0x2578
#define UNI_RACE_TITLE      0x29B4
#define UNI_BALL_LIST       0x29D4
#define UNI_BALL_COUNT      0x29D8
#define UNI_BALL_ITER       0x29DC
#define UNI_BALL_ARRAY      0x2DE0
#define UNI_RACE_BALL_LIST  0x362C
#define UNI_PARTICLE_LIST   0x3B00
#define UNI_MUSIC_NAME      0x4344

/* Trapdoor sub-lists (base Board_ctor layout, used by Tower TRAPDOOR) */
#define UNI_TRAPDOOR_MESH_LIST  0xCD4   /* Trapdoor collision mesh sub-list */
#define UNI_TRAPDOOR_RENDER_LIST 0x10EC /* Trapdoor render sub-list */

/* Level color RGBA (4 floats at board+0x1508) */
#define UNI_COLOR_R         0x1508
#define UNI_COLOR_G         0x150C
#define UNI_COLOR_B         0x1510
#define UNI_COLOR_A         0x1514

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
       MESH: = MeshNode_ctor, SPRITE: = Sprite_ctor, TIPPER: = RENDER+TipperVisual_Attach */
    char meshes[MAX_MESHES_PER_LEVEL][MAX_STR_LEN];
    int meshCount;
    DWORD unlockFlagOffset;  /* 0 = none */
    /* Per-level structural init data */
    DWORD athenaListOffsets[8];  /* Board offsets for AthenaList_Init, terminated by 0 */
    DWORD ehVectorOffset;         /* Board offset for eh_vector array (bumper slots), 0=none */
    int ehVectorCount;            /* Number of eh_vector elements */
    DWORD ehVectorStride;         /* Stride per element (0x418) */
    DWORD zeroFillOffsets[8];     /* Board offsets to zero-fill (DWORD), terminated by 0 */
    DWORD assignTexOffsets[8];    /* Board offsets of meshes to call Level_AssignTexturesAndScales on, 0=none */
    DWORD soundChannelOffset;     /* Board offset for sound channel (Dizzy: UNI_TRAPDOOR_DATA), 0=none */
    DWORD bridgeParamOffset;      /* Board offset for bridge params (angle,state,counter), 0=none */
    DWORD specialByteOffset;     /* Board offset to set a single byte, 0=none */
    BYTE  specialByteValue;      /* Value to write at specialByteOffset */
    DWORD specialDwordOffset;    /* Board offset to set a DWORD, 0=none */
    DWORD specialDwordValue;     /* Value to write at specialDwordOffset */
} LevelData;

static LevelData g_levelData[16] = {
    {{0}}, /* index 0 unused */
    /* 1=WarmUp */
    {"WarmUp",0x004D04A8,"Board (Warm-Up)","WARM-UP RACE","BEGINNERRACE","Hamster Nation",{1.0f,0.0f,1.0f},"levels\\\\level1",{},0,0,
     {UNI_LIST_0,UNI_LIST_1,UNI_LIST_2,UNI_LIST_3,UNI_LIST_4,UNI_LIST_5,UNI_LIST_6,UNI_LIST_7},UNI_EHVECTOR,8,0x418,{0},{0},0,0},
    /* 2=Beginner */
    {"Beginner",0x004D1098,"Board (Beginner)","BEGINNER RACE","CASCADERACE","Cascade Race",{1.0f,0.75f,0.25f},"levels\\levelcascade",{},0,0,
     {UNI_LIST_0,UNI_LIST_1,UNI_LIST_2,UNI_LIST_3,UNI_LIST_4,UNI_LIST_5,UNI_LIST_6,UNI_LIST_7},UNI_EHVECTOR,8,0x418,{0},{0},0,0},
    /* 3=Intermediate */
    {"Intermediate",0x004D05A0,"Board (Intermediate)","INTERMEDIATE RACE","INTERMEDIATERACE","Gerbil Groove",{0.0f,0.0f,1.0f},"levels\\level2",
     {"0x8620:Levels\\Level2-Bridge","0x8628:TIPPER:"},2,0,
     {UNI_LIST_0,UNI_LIST_1,UNI_LIST_2,UNI_LIST_3,UNI_LIST_4,UNI_LIST_5,UNI_LIST_6,UNI_LIST_7},UNI_EHVECTOR,8,0x418,{0},{0},0,UNI_BRIDGE_ANGLE},
    /* 4=Dizzy */
    {"Dizzy",0x004D0890,"Board (Dizzy)","DIZZY RACE","DIZZYRACE","Dizzy!",{0.0f,1.0f,0.0f},"levels\\level3",{"0x85E0:Levels\\Level3-WaterWheel","0x85E4:RENDER","0x85F8:Levels\\Level3-Swirl","0x860C:RENDER","0x86C0:Levels\\Level3-Tipper","0x86C4:RENDER","0x86D4:Levels\\Level3-Gluebie"},7,0x851,
     {UNI_LIST_0,UNI_LIST_1,UNI_LIST_2,UNI_LIST_3,UNI_LIST_4,UNI_LIST_5,UNI_LIST_6,UNI_LIST_7},UNI_EHVECTOR,8,0x418,{0,0,0},{0},0,0},
    /* 5=Tower */
    {"Tower",0x004D0A08,"Board (Tower)","TOWER RACE","TOWERRACE","Happy Rush",{1.0f,0.75f,0.0f},"levels\\level4",{"0x8620:Levels\\Level4-Catapult","0x8628:Levels\\Level4-Drawbridge","0x85F4:MESH:Meshes\\YellowLink","0x85EC:Levels\\Level4-Mace","0x85F0:Levels\\Level4-Windmill","0x8600:MESH:Meshes\\Chomper","0x861C:Levels\\Level4-Turret"},7,0,
     {UNI_LIST_0,UNI_LIST_1,UNI_LIST_2,UNI_LIST_3,UNI_LIST_4,UNI_LIST_5,UNI_LIST_6,UNI_LIST_7},UNI_EHVECTOR,8,0x418,{UNI_BITE_SPEED,UNI_BITE_STATE,0},{0},0,0},
    /* 6=Up */
    {"Up",0x004D11A0,"Board (Up)","UP RACE","UPRACE","Up Race",{1.0f,0.0f,1.0f},"levels\\levelup",{"0x85EC:levels\\levelup-lifter","0x85F0:levels\\levelup-speedcylinder","0x85F4:levels\\levelup-button"},3,0x853,
     {UNI_LIST_0,UNI_LIST_1,UNI_LIST_2,UNI_LIST_3,UNI_LIST_4,UNI_LIST_5,UNI_LIST_6,UNI_LIST_7},UNI_EHVECTOR,8,0x418,{0},{0},0,0},
    /* 7=Neon */
    {"Neon",0x004D1DF0,"Board (Dark)","NEON RACE","NEONRACE","Neon Theme",{1.0f,1.0f,0.0f},"levels\\leveldark",{"0x862C:Levels\\LevelDark-NeonPlatform","0x85EC:Levels\\LevelDark-DFloor1","0x85F0:Levels\\LevelDark-DFloor2","0x85F4:Levels\\LevelDark-DFloor3","0x85F8:Levels\\LevelDark-DFloor4","0x863C:Levels\\LevelDark-Trode"},6,0,
     {UNI_LIST_0,UNI_LIST_1,UNI_LIST_2,UNI_LIST_3,UNI_LIST_4,UNI_LIST_5,UNI_LIST_6,UNI_LIST_7},UNI_EHVECTOR,8,0x418,{0},{0},0,0},
    /* 8=Expert */
    {"Expert",0x004D0B00,"Board (Expert)","EXPERT RACE","EXPERTRACE","Fight!",{1.0f,0.0f,0.0f},"levels\\level5",{"0x8620:Levels\\Level5-Bridge","0x8628:RENDER","0x85F8:MESH:meshes\\hammyjudge","0x85FC:MESH:meshes\\hammyjudge","0x8600:MESH:meshes\\hammyjudge"},5,0x854,
     {UNI_LIST_0,UNI_LIST_1,UNI_LIST_2,UNI_LIST_3,UNI_LIST_4,UNI_LIST_5,UNI_LIST_6,UNI_LIST_7},UNI_EHVECTOR,8,0x418,{0},{0},0,0},
    /* 9=Odd */
    {"Odd",0x004D0BC0,"Board (Odd)","ODD RACE","ODDRACE","Ninja Hamster",{1.0f,0.5f,0.0f},"levels\\level6",{},0,0x855,
     {UNI_LIST_0,UNI_LIST_1,UNI_LIST_2,UNI_LIST_3,UNI_LIST_4,UNI_LIST_5,UNI_LIST_6,UNI_LIST_7},UNI_EHVECTOR,8,0x418,{0},{0},0,0},
    /* 10=Toob */
    {"Toob",0x004D0E78,"Board (Toob)","TOOB RACE","TOOBRACE","Rodenthood",{0.5f,0.5f,1.0f},"levels\\level8",{"0x86C8:Levels\\Level8-Spinny","0x86CC:Levels\\Level8-Saw","0x86D0:Levels\\Level8-Fallout","0x85EC:Levels\\Level8-Blockdawg1","0x85F0:Levels\\Level8-Blockdawg2"},5,0x856,
     {UNI_LIST_0,UNI_LIST_1,UNI_LIST_2,UNI_LIST_3,UNI_LIST_4,UNI_LIST_5,UNI_LIST_6,UNI_LIST_7},UNI_EHVECTOR,8,0x418,{UNI_BRIDGE_ANGLE,UNI_BRIDGE_STATE,UNI_BRIDGE_COUNTER,0},{0},0,0},
    /* 11=Wobbly */
    {"Wobbly",0x004D0D38,"Board (Wobbly)","WOBBLY RACE","WOBBLYRACE","Hamster Chase",{0.62f,0.84f,0.30f},"levels\\level7",{"0xB920:Levels\\Level7-Wobbly1","0xB924:Levels\\Level7-Wobbly2","0xB928:Levels\\Level7-Wobbly3","0xB92C:Levels\\Level7-Wobbly4","0xB930:Levels\\Level7-Wobbly5","0xB934:Levels\\Level7-Wobbly6","0xB938:Levels\\Level7-Wobbly7"},7,0x857,
     {UNI_LIST_0,UNI_LIST_1,UNI_LIST_2,UNI_LIST_3,UNI_LIST_4,UNI_LIST_5,UNI_LIST_6,UNI_LIST_7},UNI_EHVECTOR,8,0x418,{0},{0},0,0},
    /* 12=Glass */
    {"Glass",0x004D1F90,"Board (Glass)","GLASS RACE","GLASSRACE","Glass Theme",{1.0f,0.0f,1.0f},"levels\\levelglass",{},0,0,
     {UNI_LIST_0,UNI_LIST_1,UNI_LIST_2,UNI_LIST_3,UNI_LIST_4,UNI_LIST_5,UNI_LIST_6,UNI_LIST_7},UNI_EHVECTOR,8,0x418,{0},{0},0,0},
    /* 13=Sky */
    {"Sky",0x004D0FC8,"Board (Sky)","SKY RACE","SKYRACE","Bucky Break",{0.0f,0.5f,1.0f},"levels\\level9",{"0xA8B0:MESH:meshes\\skypillar","0x8680:MESH:meshes\\magnifyingglass","0x8638:levels\\level9-popcylinder1","0x863C:levels\\level9-popcylinder2","0x8678:levels\\level9-trapdoor","0xA8B4:SPRITE:textures\\clouds.png"},6,0x858,
     {UNI_LIST_0,UNI_LIST_1,UNI_LIST_2,UNI_LIST_3,UNI_LIST_4,UNI_LIST_5,UNI_LIST_6,UNI_LIST_7},UNI_EHVECTOR,8,0x418,{UNI_MAGNIFYING_GLASS,UNI_POPCYL_COUNTER,UNI_PEG_COUNT,0},{0},0,0},
    /* 14=Master — stripped down: only bridge + breaking bridge pieces.
     * Master's unique objects (Tipper, PopCylinder, BlockDawg, Catapult, Gluebie)
     * are handled by their standard level implementations, not Master special cases.
     * BBRIDGE1/2 are the Master-specific breaking bridge pieces. */
    {"Master",0x004D12B0,"Board (Master)","MASTER RACE","MASTERRACE","Master Theme",{0.5f,0.5f,0.5f},"levels\\level10",{"0x8620:Levels\\Level2-Bridge","0x8628:RENDER","0xB948:Levels\\Level10-Bridge1","0xB94C:Levels\\Level10-Bridge2"},4,0x859,
     {UNI_LIST_0,UNI_LIST_1,UNI_LIST_2,UNI_LIST_3,UNI_LIST_4,UNI_LIST_5,UNI_LIST_6,UNI_LIST_7},UNI_EHVECTOR,8,0x418,{0},{0},0,UNI_BRIDGE_COUNTER,0,0,0x29C0,0x449C4000},
    /* 15=Impossible */
    {"Impossible",0x004D21C0,"Board (Impossible)","IMPOSSIBLE RACE","IMPOSSIBLERACE","Impossible Theme",{1.0f,0.0f,0.0f},"levels\\levelimpossible",{"0x86D8:Levels\\LevelImpossible-Looper","0x86DC:Levels\\LevelImpossible-Gear","0x86E0:Levels\\LevelImpossible-BigGear","0xB90C:Levels\\LevelImpossible-Rotator","0xB9DC:Levels\\LevelImpossible-Pendulum"},5,0,
     {UNI_LIST_0,UNI_LIST_1,UNI_LIST_2,UNI_LIST_3,UNI_LIST_4,UNI_LIST_5,UNI_LIST_6,UNI_LIST_7},UNI_EHVECTOR,8,0x418,{0},{0},0,0,0x4348,1,0,0},
};

/* ═══════════════════════════════════════════════════════════════════════════
 * Config: per-level feature flags
 * g_objectEnabled[objType][level] = 1 if enabled
 * ═══════════════════════════════════════════════════════════════════════════ */

static int g_objectEnabled[OBJ_COUNT][16] __attribute__((unused)) = {{0}};

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
static char g_raceFilesPath[MAX_PATH] = "";
static char g_raceFiles[16][MAX_PATH] = {{0}}; // 1..15, each holds mesh path like "levels\\level1"
static char g_levelDir[MAX_PATH] = ""; // fallback dir for textures/sounds/sub-meshworlds (e.g. "levels\\MyLevel\\")

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

static int my_stricmp(const char *a, const char *b) {
    while (*a && *b) {
        char ca = *a, cb = *b;
        if (ca >= 'a' && ca <= 'z') ca -= 32;
        if (cb >= 'a' && cb <= 'z') cb -= 32;
        if (ca != cb) return (unsigned char)ca - (unsigned char)cb;
        a++; b++;
    }
    return (unsigned char)*a - (unsigned char)*b;
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

/* Forward declarations — defined later but used by UniversalRender */
/* ═══════════════════════════════════════════════════════════════════════════
 * LevelData.txt parser
 * ═══════════════════════════════════════════════════════════════════════════ */

/* ═══════════════════════════════════════════════════════════════════════════
 * Auto-generate LevelData.txt if missing
 * ═══════════════════════════════════════════════════════════════════════════ */

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
            strcpy(g_raceFilesPath, g_configPath);
            p = strrchr(g_raceFilesPath, '\\');
            if (p) strcpy(p + 1, "RaceFiles.txt");
        }
    }
}


/* ═══════════════════════════════════════════════════════════════════════════
 * RaceFiles.txt — maps Race 1..15 to a .MESHWORLD file (no renaming needed)
 * Format is forgiving:
 *   Race 1: Level1
 *   Race 2 = LevelCascade
 *   3 = levels\\level2
 *   4: levels\\level3.MESHWORLD
 * Lines starting with # ; [ are ignored. Bare number before = or : is the race.
 * Value is the file/path; "levels\\" is prepended if missing, and a trailing
 * ".MESHWORLD" extension is stripped (the loader adds it).
 * If the file is missing, defaults are used (see g_defaultRaceFiles).
 * The file is re-read on every level load so you can swap without restarting.
 * ═══════════════════════════════════════════════════════════════════════════ */

static const char *g_defaultRaceFiles[16] = {
    NULL,
    "levels\\level1",        // 1 WarmUp
    "levels\\levelcascade",  // 2 Beginner
    "levels\\level2",        // 3 Intermediate
    "levels\\level3",        // 4 Dizzy
    "levels\\level4",        // 5 Tower
    "levels\\levelup",       // 6 Up
    "levels\\leveldark",     // 7 Neon
    "levels\\level5",        // 8 Expert
    "levels\\level6",        // 9 Odd
    "levels\\level8",        // 10 Toob
    "levels\\level7",        // 11 Wobbly
    "levels\\levelglass",    // 12 Glass
    "levels\\level9",        // 13 Sky
    "levels\\level10",       // 14 Master
    "levels\\levelimpossible", // 15 Impossible
};

static void InitRaceFilesDefaults(void) {
    int i; for (i=1;i<=15;i++) if (!g_raceFiles[i][0] && g_defaultRaceFiles[i]) my_strncpy(g_raceFiles[i], g_defaultRaceFiles[i], MAX_PATH);
}

static void NormalizeRaceFile(char *out, const char *in) {
    int len = strlen(in);
    int start=0;
    char tmp[MAX_PATH];
    int tl=0;
    int tlen;
    int hasSep=0;
    int k;
    int i;
    while (len>0 && (in[len-1]=='"' || in[len-1]=='\'')) len--;
    while (in[start]=='"' || in[start]=='\'') start++;
    for (i=start;i<len && tl<MAX_PATH-1;i++) tmp[tl++]=in[i];
    tmp[tl]='\0';
    trim_str(tmp);
    tlen=strlen(tmp);
    if (tlen>10 && my_strnicmp(tmp+tlen-10, ".MESHWORLD", 10)==0) { tmp[tlen-10]='\0'; tlen-=10; }
    if (tlen>5 && my_strnicmp(tmp+tlen-5, ".MESH", 5)==0) { tmp[tlen-5]='\0'; }
    trim_str(tmp);
    if (!tmp[0]) { out[0]='\0'; return; }
    hasSep=0;
    for (k=0;tmp[k];k++) if (tmp[k]=='\\' || tmp[k]=='/') hasSep=1;
    if (!hasSep) { my_strncpy(out, "levels\\", MAX_PATH); strncat(out, tmp, MAX_PATH-strlen(out)-1); }
    else { my_strncpy(out, tmp, MAX_PATH); }
}

static int RaceFileExists(const char *base) {
    // Check base.MESHWORLD, base, and base with original case variations.
    // base is like "levels\\LoopyRace" or "levels\\LoopyRace\\LoopyRace"
    char tryPath[MAX_PATH];
    DWORD attr;
    // 1) base + ".MESHWORLD"
    my_strncpy(tryPath, base, MAX_PATH);
    strncat(tryPath, ".MESHWORLD", MAX_PATH-strlen(tryPath)-1);
    attr = GetFileAttributesA(tryPath);
    if (attr != (DWORD)-1 && !(attr & FILE_ATTRIBUTE_DIRECTORY)) return 1;
    // 2) base itself (in case user included extension already)
    attr = GetFileAttributesA(base);
    if (attr != (DWORD)-1 && !(attr & FILE_ATTRIBUTE_DIRECTORY)) return 1;
    // 3) lowercase extension variant (Wine case-sensitive)
    my_strncpy(tryPath, base, MAX_PATH);
    strncat(tryPath, ".meshworld", MAX_PATH-strlen(tryPath)-1);
    attr = GetFileAttributesA(tryPath);
    if (attr != (DWORD)-1 && !(attr & FILE_ATTRIBUTE_DIRECTORY)) return 1;
    return 0;
}

static void ResolveRacePath(char *out, const char *in) {
    // in is normalized like "levels\\LoopyRace" or "levels\\Pack\\File" or "custom\\file"
    // If in is "levels\\<Name>" with no further slash, try "levels\\<Name>\\<Name>" first.
    const char *prefix = "levels\\";
    int preLen = 7;
    if (my_strnicmp(in, prefix, preLen)==0) {
        const char *rest = in + preLen;
        int hasSep2=0; for (int i=0;rest[i];i++) if (rest[i]=='\\' || rest[i]=='/') hasSep2=1;
        if (!hasSep2 && rest[0]) {
            char doubled[MAX_PATH];
            my_strncpy(doubled, in, MAX_PATH);
            strncat(doubled, "\\", MAX_PATH-strlen(doubled)-1);
            strncat(doubled, rest, MAX_PATH-strlen(doubled)-1);
            if (RaceFileExists(doubled)) { my_strncpy(out, doubled, MAX_PATH); return; }
        }
    } else if (my_strnicmp(in, "levels/", 7)==0) {
        const char *rest = in + 7;
        int hasSep2=0; for (int i=0;rest[i];i++) if (rest[i]=='\\' || rest[i]=='/') hasSep2=1;
        if (!hasSep2 && rest[0]) {
            char doubled[MAX_PATH];
            my_strncpy(doubled, in, MAX_PATH);
            strncat(doubled, "/", MAX_PATH-strlen(doubled)-1);
            strncat(doubled, rest, MAX_PATH-strlen(doubled)-1);
            if (RaceFileExists(doubled)) { my_strncpy(out, doubled, MAX_PATH); return; }
        }
    }
    // No doubling or doubled file not found -> use in as-is
    my_strncpy(out, in, MAX_PATH);
}

static void LoadRaceFiles(void) {
    InitRaceFilesDefaults();
    if (!g_raceFilesPath[0]) return;
    HANDLE hFile = CreateFileA(g_raceFilesPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile==INVALID_HANDLE_VALUE) return;
    DWORD fileSize = GetFileSize(hFile, NULL);
    if (fileSize>8192) fileSize=8192;
    char buf[8192]; DWORD bytesRead=0;
    ReadFile(hFile, buf, fileSize, &bytesRead, NULL);
    CloseHandle(hFile);
    buf[bytesRead]='\0';
    char *start=buf;
    if (bytesRead>=3 && (unsigned char)start[0]==0xEF && (unsigned char)start[1]==0xBB && (unsigned char)start[2]==0xBF) start+=3;
    char *line=start;
    while (line < buf+bytesRead) {
        char *eol=line; while (*eol && *eol!='\n' && *eol!='\r') eol++;
        char saved=*eol; *eol='\0';
        char *p=line; while (*p==' ' || *p=='\t') p++;
        if (*p=='\0' || *p=='#' || *p==';' || *p=='[') goto next_rf_line;
        // find first number 1..15 in line
        int raceNum=0; char *numPos=NULL;
        char *q=p; while (*q) {
            if (*q>='0' && *q<='9') {
                int v=atoi(q); if (v>=1 && v<=15) { raceNum=v; numPos=q; break; }
                while (*q>='0' && *q<='9') q++;
            } else q++;
        }
        if (!raceNum || !numPos) goto next_rf_line;
        // find separator = or : after the number
        char *sep=numPos; while (*sep>='0' && *sep<='9') sep++;
        while (*sep==' ' || *sep=='\t') sep++;
        if (*sep=='R' || *sep=='r') { // handles "Race 1:" where p starts with Race - already skipped, but just in case
            while (*sep && *sep!=':' && *sep!='=') sep++;
        }
        if (*sep!=':' && *sep!='=') {
            // try to find any : or = in remainder of line
            char *alt=p; while (*alt && *alt!=':' && *alt!='=') alt++;
            if (*alt==':' || *alt=='=') sep=alt; else goto next_rf_line;
        }
        // value is after sep
        char *val=sep+1; while (*val==' ' || *val=='\t') val++;
        trim_str(val);
        // strip inline comment after value ( # or ; )
        char *cmt=val; while (*cmt && *cmt!='#' && *cmt!=';') cmt++;
        if (*cmt) { *cmt='\0'; trim_str(val); }
        if (!*val) goto next_rf_line;
        // also strip inline //  comment
        char *ds=strstr(val, "//"); if (ds) { *ds='\0'; trim_str(val); }
        char norm[MAX_PATH]; NormalizeRaceFile(norm, val);
        if (norm[0]) my_strncpy(g_raceFiles[raceNum], norm, MAX_PATH);
next_rf_line:
        *eol=saved;
        if (*eol=='\r') eol++;
        if (*eol=='\n') eol++;
        line=eol;
    }
    // keep defaults for any holes
    InitRaceFilesDefaults();
}

static void GenerateRaceFiles(void) {
    if (!g_raceFilesPath[0]) return;
    HANDLE hFile = CreateFileA(g_raceFilesPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile==INVALID_HANDLE_VALUE) return;
    const char *header =
        "# RaceFiles.txt — map each Race 1..15 to a .MESHWORLD file\r\n"
        "# No renaming needed: edit the name on the right side.\r\n"
        "# Formats accepted (all equivalent):\r\n"
        "#   Race 1: Level1\r\n"
        "#   2 = LevelCascade\r\n"
        "#   3 = levels\\\\level2.MESHWORLD\r\n"
        "# Lines starting with # ; [ are ignored.\r\n"
        "# You can use a bare name (Level1) or a full path (levels\\\\level1).\r\n"
        "# Bare names try levels\\\\<Name>\\\\<Name>.MESHWORLD first, then levels\\\\<Name>.MESHWORLD.\r\n"
        "# Example: \"LoopyRace\" loads levels\\\\LoopyRace\\\\LoopyRace.MESHWORLD if that folder exists,\r\n"
        "# otherwise it falls back to levels\\\\LoopyRace.MESHWORLD.\r\n"
        "# Changes are picked up on the next level load (no restart needed).\r\n\r\n";
    DWORD written; WriteFile(hFile, header, strlen(header), &written, NULL);
    InitRaceFilesDefaults();
    for (int i=1;i<=15;i++) {
        char line[256];
        // write bare filename for readability (strip levels\\ prefix)
        const char *p=g_raceFiles[i]; const char *bare=p;
        const char *slash=strrchr(p, '\\'); if (slash) bare=slash+1;
        const char *slash2=strrchr(bare, '/'); if (slash2) bare=slash2+1;
        int pos=0;
        pos+=sprintf(line+pos, "Race %d: %s\r\n", i, bare[0]?bare:"level1");
        WriteFile(hFile, line, pos, &written, NULL);
    }
    CloseHandle(hFile);
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

    /* Determine level for per-level physics constants */
    int level = GetCurrentLevel(board);
    float velScale = (level == 14) ? BUMPER_VEL_SCALE_MASTER : BUMPER_VEL_SCALE_BEGINNER;
    float maxSpeed = (level == 14) ? BUMPER_MAX_SPEED_MASTER : BUMPER_MAX_SPEED_BEGINNER;

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

    float vx = *velX * velScale;
    float vz = *velZ * velScale;
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
    if (speedSq > maxSpeed * maxSpeed) {
        float speed = sqrtf(speedSq);
        if (speed > 0.0001f) {
            float scale = maxSpeed / speed;
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
    // g_objectEnabled removed — S1-driven via IsS1CollisionEnabled
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
typedef int (__thiscall *Vec3_Init_t)(void *out, float x, float y, float z);
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
typedef void (__stdcall *eh_vector_ctor_t)(void *base, DWORD stride, int count, void *ctor_fn, void *dtor_fn);
typedef void (__thiscall *Level_AssignTexScales_t)(void *board, void *meshWorld);

/* CreateDynamicObjects ctor typedefs */
typedef void *__thiscall (*Ctor3_t)(void *mem, int board, int meshPtr);  /* Tipper, Gluebie, Catapult, Mace, Glass_Level */
typedef void *__thiscall (*Ctor2_t)(void *mem, int board);  /* Trapdoor */
typedef void *__thiscall (*Ctor4f_t)(void *mem, int board, float x, float y, float z);  /* Bonk, Bell, Gear_Level */
typedef void *__thiscall (*Ctor5f_t)(void *mem, int board, float x, float y, float z, float f);  /* Fan, Spinner_Level */
typedef void *__thiscall (*Ctor3f_t)(void *mem, int board, float x, float y, float z, int meshPtr);  /* Rotator_Impossible, Fallout, NeonPlatform, ArenaStands, Looper, Pendulum, Lifter */
typedef void *__thiscall (*Ctor6f_t)(void *mem, int board, float x, float y, float z, int meshPtr, int pathObj);  /* Saw, Saw2, Blockdawg */
typedef void *__thiscall (*Ctor7f_t)(void *mem, int board, float x, float y, float z, int x2, int y2, int z2, int meshPtr);  /* Gear */
typedef void *__thiscall (*Ctor_Str_t)(void *mem, int board, float x, float y, float z, const char *path);  /* Wavy */
typedef void *__thiscall (*Ctor_Lifter_t)(void *mem, int board, float x, float y, float z, int meshPtr, long num);  /* Lifter (Up) */
typedef void *__thiscall (*Ctor_SpeedCyl_t)(void *mem, int board, float x, float y, float z, int num, int meshPtr);  /* SpeedCylinder */
typedef void *__thiscall (*Ctor_Rotator_t)(void *mem, int board, float x, float y, float z, float f, int meshPtr);  /* Rotator (Sky trapdoor) */
typedef void *__thiscall (*Stands_ctor_t)(void *mem, int meshPtr);  /* Stands */
typedef void *__thiscall (*TipperVisual_ctor_t)(void *mem, int renderObj);  /* TipperVisual */
typedef int  (__thiscall *Level_FindObjectByName_t)(int meshWorld, const char *name);
typedef void (__thiscall *SawBlade_SetVariant_t)(void *obj, int variant);
typedef void (__thiscall *Wavy_Configure_t)(void *obj, int a, int b, int c, int d);
typedef void (__thiscall *Sound_InitChannels_t)(void *obj, int flag);

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
static eh_vector_ctor_t g_ehVectorCtor = NULL;
static Level_AssignTexScales_t g_LevelAssignTexScales = NULL;
static void *g_ehVectorCtorFn = NULL;
static void *g_Vec3ListFree = NULL;

/* CreateDynamicObjects ctor pointers */
static Ctor3_t g_TipperCtor = NULL;
static TipperVisual_ctor_t g_TipperVisualCtor = NULL;
static Ctor3_t g_GluebieCtor = NULL;
static Ctor3_t g_CatapultCtor = NULL;
static Ctor3_t g_MaceCtor = NULL;
static Ctor3_t g_GlassLevelCtor = NULL;
static Ctor2_t g_TrapdoorCtor = NULL;
static Stands_ctor_t g_StandsCtor = NULL;
static Ctor4f_t g_BonkCtor = NULL;
static Ctor5f_t g_FanCtor = NULL;
static Ctor4f_t g_SawBladeCtor = NULL;
static SawBlade_SetVariant_t g_SawBladeSetVariant = NULL;
static Ctor5f_t g_SpinnerLevelCtor = NULL;
static Ctor4f_t g_GearLevelCtor = NULL;
static Ctor4f_t g_BellCtor = NULL;
static Ctor4f_t g_OddLifterCtor = NULL;
static Ctor_Lifter_t g_LifterCtor = NULL;
static Ctor_SpeedCyl_t g_SpeedCylinderCtor = NULL;
static Ctor3f_t g_TimeButtonCtor = NULL;
static Ctor3f_t g_RotatorImpossibleCtor = NULL;
static Ctor6f_t g_SawCtor = NULL;
static Ctor6f_t g_Saw2Ctor = NULL;
static Ctor3f_t g_FalloutCtor = NULL;
static Ctor6f_t g_BlockdawgCtor = NULL;
static Ctor3f_t g_GameLevelCtor = NULL;
static Ctor_Str_t g_WavyCtor = NULL;
static Wavy_Configure_t g_WavyConfigure = NULL;
static Ctor3f_t g_NeonPlatformCtor = NULL;
static Ctor3f_t g_ArenaStandsCtor = NULL;
static Ctor3f_t g_PopcylinderCtor = NULL;
static Ctor3f_t g_PopCylinderCtor = NULL;
static Ctor_Rotator_t g_RotatorCtor = NULL;
static Ctor3f_t g_LooperCtor = NULL;
static Ctor7f_t g_GearCtor = NULL;
static Ctor3f_t g_PendulumCtor = NULL;
static Ctor3f_t g_BreakBridgeCtor = NULL;
static Level_FindObjectByName_t g_LevelFindObjectByName = NULL;
static Sound_InitChannels_t g_SoundInitChannels = NULL;

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

    /* Ext heap: allocate after base ctor so unified offsets have backing */
    void *ext = EnsureBoardExt(mem);
    if (!ext) {
        DebugLog("UniversalBoardCtorLogic: EnsureBoardExt failed");
        return;
    }

    /* Step 1b: Zero-fill old per-level data offsets (0x436C-0x6500).
     * The mod moved ehVector, bumper lit, and render data to the unified
     * zone (0x6500+). But per-level RENDER functions (vtable slot 24)
     * still read from the OLD per-level offsets (e.g. Beginner reads
     * board+0x436C for ehVector, board+0x642C for bumper lit).
     * operator_new uses malloc (no zeroing), so these offsets contain
     * garbage. Zeroing them ensures:
     *   - Bumper lit floats = 0.0 → render function skips (no reflective material)
     *   - Mesh slot pointers = NULL → render function does nothing
     * This prevents crashes from garbage pointers being dereferenced.
     *
     * IMPORTANT: Start at 0x436C, NOT 0x4300. Board_ctor writes critical
     * data in 0x4300-0x436B that must be preserved:
     *   0x4340 = 1.0f (scale factor)
     *   0x434C = NULL pointer check
     *   0x4350 = -150.0f (camera offset)
     *   0x4358 = demo timer flag
     *   0x435C = demo timer count (0x9c4 = 2500)
     *   0x4368 = 0 (byte flag)
     * Zeroing these causes crashes in Scene_Update's vtable sub-functions. */
    // Vanilla board stays small; unified data now lives in ext (HEAP_ZERO_MEMORY).
    // Previous memset to 0x6000 overflowed WarmUp (~0x4400) -> heap corruption -> 0x452783 crash.
    // Safe fix: do NOT memset board+0x436C at all. Ext is already zeroed. If a level needs
    // legacy zero at 0x436C (Up lifter list), that is handled in step 9a-extra instead.
    // (Kept as no-op to avoid OOB on small boards.)
    {
        char dbg2[128];
        wsprintfA(dbg2, "memset skipped (OOB fix, board=0x%08X)", (DWORD)mem);
        DebugLog(dbg2);
    }

    /* Step 2: Set vtable */
    *(DWORD *)mem = g_levelVtables[raceIndex];
    DebugLog("Vtable set");

    /* Step 3: Set board name and race title */
    *(char **)((char *)mem + UNI_BOARD_NAME) = ld->boardName;
    *(char **)((char *)mem + UNI_RACE_TITLE) = ld->raceTitle;
    DebugLog("Names set");

    /* Step 4: Set +0x870 from app+0x1DC */
    *(DWORD *)((char *)mem + UNI_BOARD_APPVAL) = *(DWORD *)(app + 0x1DC);
    DebugLog("+0x870 set");

    /* Step 5: Write per-level color directly to board+0x1508 */
    /* Original game uses Vec3_Init+Matrix_Identity, but we write directly
       to avoid calling convention issues. Board+0x1508 is a 4-float RGBA. */
    *(float *)((char *)mem + UNI_COLOR_R) = ld->color[0];
    *(float *)((char *)mem + UNI_COLOR_G) = ld->color[1];
    *(float *)((char *)mem + UNI_COLOR_B) = ld->color[2];
    *(float *)((char *)mem + UNI_COLOR_A) = 1.0f;  /* alpha */
    DebugLog("Color set");

    /* Step 6: LoadRaceData(mem, raceName) */
    DebugLog("Calling LoadRaceData...");
    g_LoadRaceData(mem, ld->raceData);
    DebugLog("LoadRaceData done");

    /* Step 7: Set music name */
    *(char **)((char *)mem + UNI_MUSIC_NAME) = ld->musicName;
    DebugLog("Music set");

    /* Step 8: Load extra meshes — DEPRECATED Phase1 S1-driven.
     * Static g_levelData[].meshes preload removed. All meshes now lazy-loaded
     * via S1Ensure* inside UniversalCreateDynamicObjects on demand. */
    DebugLog("Step 8: S1-driven meshes (no static preload)");

    /* Step 8b: For Dizzy, also write mesh pointers to ORIGINAL offsets.
     * Unified storage is now in ext; copy to vanilla offsets for native code.
     * Guard: vanilla Dizzy board is ~0x6000, but file-swapped Dizzy-in-WarmUp
     * board is ~0x4400 → 0x4BA8 is OOB. Use BoardHasOffset (VirtualQuery+HeapSize),
     * NOT IsBadWritePtr (which only checks page writability). */
    if (raceIndex == 4) {
        if (BoardHasOffset(mem, 0x4BC8, 4) && BoardHasOffset(mem, 0x4BD8, 4) && ExtHasOffset(ext, UNI_TIPPER_MESH, 4)) {
            /* Tipper mesh+render — load from ext dedicated slots */
            *(DWORD *)((char *)mem + 0x436C) = *(DWORD *)((char *)ext + UNI_TIPPER_MESH);
            *(DWORD *)((char *)mem + 0x4370) = *(DWORD *)((char *)ext + UNI_TIPPER_RENDER);
            /* WaterWheel mesh+render */
            *(DWORD *)((char *)mem + 0x4BA8) = *(DWORD *)((char *)ext + UNI_MESH_0);
            *(DWORD *)((char *)mem + 0x4BAC) = *(DWORD *)((char *)ext + UNI_MESH_1);
            /* Swirl mesh+render */
            *(DWORD *)((char *)mem + 0x4BC4) = *(DWORD *)((char *)ext + UNI_MESH_6);
            *(DWORD *)((char *)mem + 0x4BC8) = *(DWORD *)((char *)ext + UNI_MESH_11);
            /* Gluebie mesh */
            *(DWORD *)((char *)mem + 0x4374) = *(DWORD *)((char *)ext + UNI_GLUEBIE_MESH);
            /* Init angle/scale fields (original offsets) */
            *(DWORD *)((char *)mem + 0x4BC0) = 0;  /* WaterWheel scale */
            *(DWORD *)((char *)mem + 0x4BD8) = 0;  /* Swirl angle */
            DebugLog("Step 8b: Dizzy mesh pointers dual-written to original offsets");
        } else {
            DebugLog("Step 8b: Dizzy dual-write skipped (vanilla board too small for 0x4BA8)");
        }
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
    DebugLog("Step 9 (unlock flags) done");

    /* Step 9: Per-level structural init (AthenaLists, eh_vector arrays, zero-fills) */

    /* 9a: AthenaList_Init — unified offsets >=0x6500 live in ext */
    if (g_AthenaListInit) {
        int ai;
        for (ai = 0; ai < 8 && ld->athenaListOffsets[ai]; ai++) {
            DWORD off = ld->athenaListOffsets[ai];
            void *dst = (off >= 0x6500) ? (void *)((char *)ext + off) : (void *)((char *)mem + off);
            g_AthenaListInit(dst, 0);
        }
    }
    DebugLog("Step 9a (AthenaList_Init) done");

    /* 9a-extra: Up race (level 6) needs a legacy AthenaList at board+0x436C.
     * The original Up RaceState (0x00420660) iterates this list and calls
     * each Lifter's Update() every frame.  Without it, vacuum tubes never
     * animate and E:HELPINERTIA/E:VACPOPOUT events never fire. */
    if (raceIndex == 6 && g_AthenaListInit) {
        g_AthenaListInit((void *)((char *)mem + 0x436C), 0);
        DebugLog("Step 9a-extra: legacy AthenaList at board+0x436C for Up");
    }

    /* 9b: eh_vector — lives in ext if offset >=0x6500 */
    if (g_ehVectorCtor && g_ehVectorCtorFn && g_Vec3ListFree &&
        ld->ehVectorOffset && ld->ehVectorCount > 0) {
        void *dst = (ld->ehVectorOffset >= 0x6500) ? (void *)((char *)ext + ld->ehVectorOffset) : (void *)((char *)mem + ld->ehVectorOffset);
        g_ehVectorCtor(dst,
                       ld->ehVectorStride, ld->ehVectorCount,
                       g_ehVectorCtorFn, g_Vec3ListFree);
    }
    DebugLog("Step 9b (ehVector) done");

    /* 9c: Zero-fill — unified offsets go to ext */
    {
        int zi;
        for (zi = 0; zi < 8 && ld->zeroFillOffsets[zi]; zi++) {
            DWORD off = ld->zeroFillOffsets[zi];
            if (off >= 0x6500) *(DWORD *)((char *)ext + off) = 0;
            else *(DWORD *)((char *)mem + off) = 0;
        }
    }
    DebugLog("Step 9c (zero-fills) done");

    /* 9d: Level_AssignTexturesAndScales — mesh ptr may be in ext */
    if (g_LevelAssignTexScales) {
        int ti;
        for (ti = 0; ti < 8 && ld->assignTexOffsets[ti]; ti++) {
            DWORD off = ld->assignTexOffsets[ti];
            DWORD meshPtr = (off >= 0x6500) ? *(DWORD *)((char *)ext + off) : *(DWORD *)((char *)mem + off);
            if (meshPtr && !IsBadReadPtr((void *)meshPtr, 4)) {
                g_LevelAssignTexScales(mem, (void *)meshPtr);
            }
        }
    }
    DebugLog("Step 9d (textures) done");

    /* 9e: Sound channel — unified offset goes to ext */
    if (ld->soundChannelOffset && g_SoundGetNextChannel) {
        DWORD appVal = *(DWORD *)((char *)mem + BOARD_APP_PTR);
        if (appVal && !IsBadReadPtr((void *)appVal, 0x500)) {
            DWORD soundDevice = *(DWORD *)(appVal + 0x490);
            if (soundDevice) {
                int channel = g_SoundGetNextChannel((void *)soundDevice);
                if (ld->soundChannelOffset >= 0x6500) *(int *)((char *)ext + ld->soundChannelOffset) = channel;
                else *(int *)((char *)mem + ld->soundChannelOffset) = channel;
                if (channel && g_SceneRenderIfVisible)
                    g_SceneRenderIfVisible(channel);
            }
        }
    }
    DebugLog("Step 9e (sound) done");

    /* 9f: Bridge params — unified goes to ext */
    if (ld->bridgeParamOffset) {
        void *base = (ld->bridgeParamOffset >= 0x6500) ? ext : mem;
        DWORD off = ld->bridgeParamOffset;
        *(DWORD *)((char *)base + off)     = 0x42340000;  /* 45.0f */
        *(DWORD *)((char *)base + off + 4) = 0;
        *(DWORD *)((char *)base + off + 8) = 0x32;
    }
    DebugLog("Step 9f (bridge params) done");

    /* 9g: Special init — unified goes to ext */
    if (ld->specialByteOffset) {
        if (ld->specialByteOffset >= 0x6500) *(BYTE *)((char *)ext + ld->specialByteOffset) = ld->specialByteValue;
        else *(BYTE *)((char *)mem + ld->specialByteOffset) = ld->specialByteValue;
    }
    if (ld->specialDwordOffset) {
        if (ld->specialDwordOffset >= 0x6500) *(DWORD *)((char *)ext + ld->specialDwordOffset) = ld->specialDwordValue;
        else *(DWORD *)((char *)mem + ld->specialDwordOffset) = ld->specialDwordValue;
    }
    DebugLog("Step 9g (special init) done — UniversalBoardCtorLogic complete");
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
 *   1. operator_new(0x10d0) → Level_MeshWorldCtor(mem, gfx, "Levels\\Level2-Bridge") → board+UNI_BONK_STORE
 *   2. operator_new(0x10d0) → Level_RenderCtor(mem, meshWorld) → board+UNI_SAW1_OBJ
 *   3. TipperVisual_Attach(renderObj, meshWorld)
 *   4. board+UNI_BRIDGE_ANGLE = 0x42340000 (float 45.0)
 *   5. board+UNI_BRIDGE_STATE = 0
 *   6. board+UNI_BRIDGE_COUNTER = 0x32 (50)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void InitBridge(void *board) {
    if (!g_operatorNew || !g_LevelMeshWorldCtor || !g_LevelRenderCtor ||
        !g_TipperVisualAttach) return;
    void *ext = EnsureBoardExt(board);
    if (!ext) return;

    /* Don't create a second bridge if one is already loaded. */
    if (*(void **)((char *)ext + UNI_BONK_STORE) != NULL) {
        DebugLog("InitBridge: bridge already exists, skipping");
        return;
    }

    DWORD app = *(DWORD *)((char *)board + BOARD_APP_PTR);
    if (!app || IsBadReadPtr((void *)app, 0x200)) return;
    void *gfx = *(void **)((char *)app + 0x174);
    if (!gfx) return;

    /* Step 1: MeshWorld — store at ext+UNI_BONK_STORE */
    void *meshMem = g_operatorNew(0x10D0);
    if (!meshMem) return;
    void *meshWorld = g_LevelMeshWorldCtor(meshMem, gfx, "Levels\\\\Level2-Bridge");
    *(void **)((char *)ext + UNI_BONK_STORE) = meshWorld;

    /* Step 2: RenderObj — store at ext+UNI_SAW1_OBJ */
    void *renderMem = g_operatorNew(0x10D0);
    void *renderObj = NULL;
    if (renderMem) {
        renderObj = g_LevelRenderCtor(renderMem, meshWorld);
    }
    *(void **)((char *)ext + UNI_SAW1_OBJ) = renderObj;

    /* Step 3: TipperVisual_Attach */
    if (renderObj && meshWorld) {
        g_TipperVisualAttach(renderObj, meshWorld);
    }

    /* Steps 4-6: Bridge config values — unified in ext */
    *(DWORD *)((char *)ext + BRIDGE_PARAM1) = 0x42340000;  /* 45.0f */
    *(DWORD *)((char *)ext + BRIDGE_PARAM2) = 0;
    *(DWORD *)((char *)ext + BRIDGE_PARAM3) = 0x32;       /* 50 */
    // OFF_* mirrors removed: UNI_* at ext+0x8634/38/3C is canonical inside ext heap.
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Feature block: Bridge Animation (Intermediate)
 * 4-state machine: wait → tilt down → wait → tilt back
 * Replicates FUN_0041CC90 (Intermediate Board_Update)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void Feature_BridgeAnimation(void *board, int level) {
    // Pause gate: board+0x874 !=0 means paused (ESC) — freeze anim + sound
    if (*(BYTE*)((char*)board+0x874)) return;
    if (!g_SceneUpdate) return;
    void* ext = GetBoardExt(board);
    int ballCount = g_AthenaListGetSize((void *)((char *)board + UNI_RACE_BALL_LIST));
    DWORD app = *(DWORD *)((char *)board + BOARD_APP_PTR);
    if (ballCount == 1 && app && !IsBadReadPtr((void *)app, 0x600)) {
        DWORD ball = *(DWORD *)(app + APP_BALL_PTR);
        if (ball && !IsBadReadPtr((void *)ball, 0x200) && *(char *)(ball + 0x14C))
            return;
    }
    DWORD renderObj;
    float *anglePtr; int *statePtr; int *counterPtr; float *pivotX; float *pivotY; float *pivotZ;
    if (!ext) return;
    // UNI_* is canonical — writers (InitBridge, LoadExtraMeshes) store at ext+UNI_*.
    renderObj = *(DWORD *)((char *)ext + UNI_BONK_STORE);
    anglePtr = (float*)((char*)ext + UNI_BRIDGE_ANGLE);
    statePtr = (int*)((char*)ext + UNI_BRIDGE_STATE);
    counterPtr = (int*)((char*)ext + UNI_BRIDGE_COUNTER);
    pivotX = (float*)((char*)ext + UNI_BRIDGE_PIVOT_X);
    pivotY = (float*)((char*)ext + UNI_BRIDGE_PIVOT_Y);
    pivotZ = (float*)((char*)ext + UNI_BRIDGE_PIVOT_Z);
    if (!renderObj) return;
    int state = *statePtr;
    switch (state) {
    case 0: { int c = *counterPtr - 1; *counterPtr = c; if (c < 1) *statePtr = 1; } break;
    case 1: {
            float angle = *anglePtr - 3.0f;
            *anglePtr = angle;
            if (angle < 0.0f) {
                *anglePtr = 0.0f; *counterPtr = 0x7D; *statePtr = 2;
                if (app && !IsBadReadPtr((void *)app, 0x800) && g_SoundPlay3D) {
                    DWORD snd = *(DWORD *)(app + APP_SOUNDFX_47C);
                    if (snd) g_SoundPlay3D((void *)snd, *pivotX, *pivotY, *pivotZ);
                }
                if (g_SceneForEachBallSetVelocity && g_Vec3CopyUpd) {
                    float pivot[3]; g_Vec3CopyUpd(pivot, pivotX);
                    g_SceneForEachBallSetVelocity(board, pivot[1], pivot[2], 0.5f);
                }
            }
        } break;
    case 2: { int c = *counterPtr - 1; *counterPtr = c; if (c < 1) *statePtr = 3; } break;
    case 3: {
            float angle = *anglePtr + 0.5f;
            *anglePtr = angle;
            if (angle >= 45.0f) { *anglePtr = 45.0f; *counterPtr = 0x4B; *statePtr = 0; }
            if (g_TimerInit && g_TimerCleanup && g_GfxScaleZ && g_GfxSetPosition && g_MatrixTransformVec3 && app) {
                void *gfx = *(void **)(app + 0x174);
                if (gfx) {
                char timerBuf[68];
                g_TimerInit(timerBuf);
                g_GfxScaleZ(gfx, -*anglePtr);
                g_GfxSetPosition(gfx, *pivotX, *pivotY, *pivotZ);
                DWORD *renderVtbl = *(DWORD **)renderObj;
                if (renderVtbl) {
                    void (__fastcall *fn58)(DWORD) = (void (__fastcall *)(DWORD))renderVtbl[0x16];
                    void (__fastcall *fn54)(DWORD, char *) = (void (__fastcall *)(DWORD, char *))renderVtbl[0x15];
                    if (fn58) fn58((DWORD)renderObj);
                    if (fn54) fn54((DWORD)renderObj, timerBuf);
                }
                g_TimerCleanup(timerBuf);
                }
            }
        } break;
    }
    // No board mirror — ext is source of truth
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Feature block: Swirl Zones (Dizzy / Master)
 * Proximity check → velocity scale + tar bubbles + mesh rotation
 * Replicates DizzyBoard_Update (0x41D510) / Master Board_Update (0x420DA0)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void Feature_SwirlZones(void *board, int level) {
    // Pause gate: board+0x874 !=0 means paused (ESC) — freeze anim + sound
    if (*(BYTE*)((char*)board+0x874)) return;
    if (!g_AthenaListGetIterator || !g_operatorNew) return;
    void* ext = GetBoardExt(board);
    if (!ext) ext = EnsureBoardExt(board);
    if (!ext) return;

    DWORD app = *(DWORD *)((char *)board + BOARD_APP_PTR);
    if (!app || IsBadReadPtr((void *)app, 0x600)) return;

    /* Unified lists live in ext */
    int swirlListOfs = (level == 14) ? BRD_SWIRL_LIST_M : BRD_SWIRL_LIST;
    int tarListOfs = (level == 14) ? BRD_TARBUBBLE_LIST_M : BRD_TARBUBBLE_LIST;
    // swirlListOfs/tarListOfs are UNI offsets >=0x6500 — use ext


    /* TarBubble particle creation — replicates original DizzyBoard_Update.
     * RNG returns 10 (1 in 20 chance per frame) → create a tarbubble particle.
     * FUN_0044fa90 picks a random entry from the tarbubble list and reads
     * entry+0x04/0x08/0x0C as X/Y/Z floats. S1 ref points have this layout.
     * The particle is appended to UNI_PARTICLE_LIST for rendering. */
    static int swirlDbg = 0;
    if (swirlDbg < 3) DebugLog("  [swirl] step1: tarbubble check");
    if (g_CreateTarBubble && g_AthenaListAppend && g_operatorNew && g_RNG) {
        DWORD *tarList = (DWORD *)((char *)ext + tarListOfs);
        int tarCount = 0;
        if (!IsBadReadPtr(tarList, 0x410)) {
            tarCount = *(int *)(tarList + 1);  /* count at +0x04 */
        }
        if (swirlDbg < 3) {
            char dbg[128];
            wsprintfA(dbg, "  [swirl] step1: tarCount=%d tarListOfs=0x%X", tarCount, tarListOfs);
            DebugLog(dbg);
        }
        if (tarCount > 0) {
            int rngResult = -1;
            if (g_RNG) {
                rngResult = RNG_call((void *)0x4F7360, 0, 0x14, 0);
                if (swirlDbg < 3) {
                    char dbg2[128];
                    wsprintfA(dbg2, "  [swirl] step1: rngResult=%d (need 10)", rngResult);
                    DebugLog(dbg2);
                }
                if (rngResult == 10) {
                    void *tar = g_operatorNew(0x1C);
                    if (tar) {
                        if (swirlDbg < 3) DebugLog("  [swirl] step1: calling CreateTarBubble");
                        g_CreateTarBubble(tar, app, (int)((char *)ext + tarListOfs));
                        g_AthenaListAppend((void *)((char *)board + UNI_PARTICLE_LIST), (int)tar);
                        if (swirlDbg < 3) DebugLog("  [swirl] step1: CreateTarBubble done");
                    }
                }
            }
        }
    }
    if (swirlDbg < 3) DebugLog("  [swirl] step1 done");

    /* Swirl zone processing: iterate ball list, check proximity to swirl zones
     * Ball list at board+0x29D4 (AthenaList), array at board+0x2DE0 */
    if (swirlDbg < 3) DebugLog("  [swirl] step2: ball list iteration");
    if (swirlDbg < 3) DebugLog("  [swirl] step2a: get ball iterator");
    int ballIter = g_AthenaListGetIterator((void *)((char *)board + UNI_BALL_LIST));
    if (swirlDbg < 3) DebugLog("  [swirl] step2a done");
    *(int *)((char *)board + UNI_BALL_ITER + ballIter * 4) = 0;
    if (swirlDbg < 3) DebugLog("  [swirl] step2b: read ball count");
    int ballCount = *(int *)((char *)board + UNI_BALL_COUNT);
    if (swirlDbg < 3) DebugLog("  [swirl] step2b done");
    int ballIdx = 0;
    if (ballCount > 0) {
        if (swirlDbg < 3) DebugLog("  [swirl] step2c: read ball array");
        DWORD ballArrayPtr = *(DWORD *)((char *)board + UNI_BALL_ARRAY);
        if (ballArrayPtr && !IsBadReadPtr((void *)ballArrayPtr, 4)) {
            ballIdx = *(int *)ballArrayPtr;
            *(int *)((char *)board + UNI_BALL_ITER + ballIter * 4) = 1;
        }
        if (swirlDbg < 3) DebugLog("  [swirl] step2c done");
    }

    while (ballIdx) {
        /* Skip balls in tar (ball+0x2CC != 0) — they get sinking logic */
        char inTar = *(char *)(ballIdx + BALL_IN_TAR_OFS);
        if (!inTar) {
            /* Check proximity to each swirl zone */
            int zoneIter = g_AthenaListGetIterator((void *)((char *)ext + swirlListOfs));
            *(int *)((char *)ext + swirlListOfs + 8 + zoneIter * 4) = 0;
            int zoneCount = *(int *)((char *)ext + swirlListOfs + 4);
            int zoneIdx = 0;
            if (zoneCount > 0) {
                DWORD zoneArrayPtr = *(DWORD *)((char *)ext + swirlListOfs + 0x40C);
                if (zoneArrayPtr && !IsBadReadPtr((void *)zoneArrayPtr, 4)) {
                    zoneIdx = *(int *)zoneArrayPtr;
                    *(int *)((char *)ext + swirlListOfs + 8 + zoneIter * 4) = 1;
                }
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
                                if (g_SoundPlay3D) {
                                    DWORD snd = *(DWORD *)(app + APP_SOUNDFX_484);
                                    if (snd)
                                        g_SoundPlay3D((void *)snd, ballX, ballY, ballZ);
                                }
                                /* Spawn 3 particles */
                                int p;
                                for (p = 0; p < 3 && g_operatorNew && g_AthenaListAppend; p++) {
                                    float *part = (float *)g_operatorNew(0x14);
                                    if (part) {
                                        /* Random direction (normalized) */
                                        if (g_RNG) {
                                            part[0] = (float)RNG_call((void *)0x4F7360, 0, 100, 1);
                                            part[1] = (float)RNG_call((void *)0x4F7360, 0, 100, 1);
                                            part[2] = (float)RNG_call((void *)0x4F7360, 0, 100, 1);
                                            float lenSq = part[0]*part[0] + part[1]*part[1] + part[2]*part[2];
                                            if (lenSq > 0.0f) {
                                                float len = sqrtf(lenSq);
                                                float s = 1.0f / len;
                                                part[0] *= s; part[1] *= s; part[2] *= s;
                                            }
                                        }
                                        int particleListSize = g_AthenaListGetSize(
                                            (void *)(ballIdx + 0x810));
                                        if (particleListSize < 30) {
                                            g_AthenaListAppend(
                                                (void *)(ballIdx + 0x810), (int)part);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                int next = *(int *)((char *)ext + swirlListOfs + 8 + zoneIter * 4);
                if (*(int *)((char *)ext + swirlListOfs + 4) <= next) break;
                DWORD zoneArr = *(DWORD *)((char *)ext + swirlListOfs + 0x40C);
                if (!zoneArr || IsBadReadPtr((void *)zoneArr, (next + 1) * 4)) break;
                zoneIdx = *(int *)(zoneArr + next * 4);
                *(int *)((char *)ext + swirlListOfs + 8 + zoneIter * 4) = next + 1;
            }
        } else {
            /* Ball is in tar — sink and potentially remove */
            *(float *)(ballIdx + BALL_POS_Y_OFS) -= 0.25f;
            /* Splash particle */
            if (g_CreateSplashParticle && g_AthenaListAppend && g_RNG) {
                int rng1 = RNG_call((void *)0x4F7360, 0, 0xF, 0);
                if (rng1 == 1) {
                    float rx = (float)RNG_call((void *)0x4F7360, 0, 100, 1);
                    float rz = (float)RNG_call((void *)0x4F7360, 0, 100, 1);
                    float lenSq = rx*rx + rz*rz;
                    float len = (lenSq > 0.0f) ? sqrtf(lenSq) : 0.0f;
                    float scale = (len > 0.0f) ?
                        (10.0f + *(float *)(ballIdx + 0x284)) / len : 0.0f;
                    rx *= scale; rz *= scale;
                    void *splash = g_operatorNew(0x1C);
                    if (splash) {
                        g_CreateSplashParticle(splash, app,
                            rz + *(float *)(ballIdx + BALL_POS_X_OFS),
                            *(DWORD *)(ballIdx + 0x2D0),
                            rx + *(float *)(ballIdx + BALL_POS_Z_OFS));
                        g_AthenaListAppend((void *)((char *)board + UNI_PARTICLE_LIST), (int)splash);
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
        int nextBall = *(int *)((char *)board + UNI_BALL_ITER + ballIter * 4);
        if (*(int *)((char *)board + UNI_BALL_COUNT) <= nextBall) break;
        DWORD ballArr = *(DWORD *)((char *)board + UNI_BALL_ARRAY);
        if (!ballArr || IsBadReadPtr((void *)ballArr, (nextBall + 1) * 4)) break;
        ballIdx = *(int *)(ballArr + nextBall * 4);
        *(int *)((char *)board + UNI_BALL_ITER + ballIter * 4) = nextBall + 1;
    }
    if (swirlDbg < 3) DebugLog("  [swirl] step2 done");

    /* Dizzy-only: mesh rotation (Master doesn't rotate meshes) */
    if (swirlDbg < 3) DebugLog("  [swirl] step3: mesh rotation");
    if (level != 14 && g_TimerInit && g_TimerCleanup && g_GfxScaleY &&
        g_GfxSetPosition && g_Matrix44Zero && app) {
        void *gfx = *(void **)(app + 0x174);
        if (gfx) {
        char timerBuf[68];
        g_TimerInit(timerBuf);

        /* Primary swirl mesh rotation (Gfx_ScaleY) */
        if (swirlDbg < 3) DebugLog("  [swirl] step3a: primary mesh rotation");
        float angle1 = *(float*)((char*)ext + OFF_SWIRL_ANGLE1) - 0.5f;
        *(float*)((char*)ext + OFF_SWIRL_SPEED)=0.5f; *(float*)((char*)ext + OFF_SWIRL_ANGLE1)=angle1;
        if (swirlDbg < 3) DebugLog("  [swirl] step3a: calling Matrix44Zero");
        g_Matrix44Zero((int *)timerBuf);
        if (swirlDbg < 3) DebugLog("  [swirl] step3a: calling GfxScaleY");
        {
            float rx = *(float*)((char*)ext + OFF_WATER_ROT_X);
            float ry = *(float*)((char*)ext + OFF_WATER_ROT_Y);
            float rz = *(float*)((char*)ext + OFF_WATER_ROT_Z);
            if (rx!=0 || ry!=0 || rz!=0) {
                if (ry!=0) g_GfxScaleY(gfx, angle1 * ry); else g_GfxScaleY(gfx, angle1);
                if (rx!=0) g_GfxScaleX(gfx, angle1 * rx);
                if (rz!=0) g_GfxScaleZ(gfx, angle1 * rz);
            } else g_GfxScaleY(gfx, angle1);
        }
        if (swirlDbg < 3) DebugLog("  [swirl] step3a: calling GfxSetPosition");
        g_GfxSetPosition(gfx, *(float*)((char*)ext + OFF_WHEEL_EMBED_X), *(float*)((char*)ext + OFF_WHEEL_EMBED_Y), *(float*)((char*)ext + OFF_WHEEL_EMBED_Z));
        if (swirlDbg < 3) DebugLog("  [swirl] step3a: Gfx calls done, calling render");
        // Re-enabled after ext-heap fix — heap overflow was corrupting spatial tree at +0x18
        {
            DWORD waterRender = *(DWORD*)((char*)ext + UNI_MESH_1);
            if (waterRender && !IsBadReadPtr((void*)waterRender, 4)) {
                DWORD *vtbl = *(DWORD**)waterRender;
                if (vtbl && !IsBadReadPtr(vtbl, 0x60)) {
                    void (__thiscall *fn16)(DWORD) = (void (__thiscall*)(DWORD))vtbl[0x16];
                    void (__thiscall *fn15)(DWORD, void*) = (void (__thiscall*)(DWORD,void*))vtbl[0x15];
                    if (fn16) fn16(waterRender);
                    if (fn15) fn15(waterRender, timerBuf);
                }
            }
        }
        if (swirlDbg < 3) DebugLog("  [swirl] step3a done");

        g_TimerCleanup(timerBuf);
        }
        DebugLog("  [swirl] step3a done (full)");
    }

    /* Dizzy: secondary swirl mesh rotation (Gfx_ScaleX) */
    if (level != 14 && g_TimerInit && g_TimerCleanup && g_GfxScaleX &&
        g_GfxSetPosition && g_Matrix44Zero && app) {
        void *gfx = *(void **)(app + 0x174);
        if (gfx) {
        if (swirlDbg < 3) DebugLog("  [swirl] step3b: secondary mesh rotation");
        char timerBuf[68];
        g_TimerInit(timerBuf);

        float swirlSpeed = (*(int *)(app + APP_DIFFICULTY) == 0) ? 0.25f : 0.5f;
        *(float*)((char*)ext + OFF_SWIRL_ANGLE2) = *(float*)((char*)ext + OFF_SWIRL_ANGLE2) + swirlSpeed;
        if (swirlDbg < 3) DebugLog("  [swirl] step3b: calling Matrix44Zero");
        g_Matrix44Zero((int *)timerBuf);
        if (swirlDbg < 3) DebugLog("  [swirl] step3b: calling GfxScaleX");
        g_GfxScaleX(gfx, *(float *)((char *)ext + OFF_SWIRL_ANGLE2));
        if (swirlDbg < 3) DebugLog("  [swirl] step3b: calling GfxSetPosition");
        g_GfxSetPosition(gfx,
            *(float *)((char *)ext + OFF_SWIRL_POS_X),
            *(float *)((char *)ext + OFF_SWIRL_POS_Y),
            *(float *)((char *)ext + OFF_SWIRL_POS_Z));
        if (swirlDbg < 3) DebugLog("  [swirl] step3b: Gfx calls done, calling render");
        {
            DWORD swirlRender = *(DWORD*)((char*)ext + UNI_MESH_7);
            if (swirlRender && !IsBadReadPtr((void*)swirlRender, 4)) {
                DWORD *vtbl = *(DWORD**)swirlRender;
                if (vtbl && !IsBadReadPtr(vtbl, 0x60)) {
                    void (__thiscall *fn16)(DWORD) = (void (__thiscall*)(DWORD))vtbl[0x16];
                    void (__thiscall *fn15)(DWORD, void*) = (void (__thiscall*)(DWORD,void*))vtbl[0x15];
                    if (fn16) fn16(swirlRender);
                    if (fn15) fn15(swirlRender, timerBuf);
                }
            }
        }
        if (swirlDbg < 3) DebugLog("  [swirl] step3b done");

        g_TimerCleanup(timerBuf);
        }
        if (swirlDbg < 3) DebugLog("  [swirl] step3b done");
    }
    if (swirlDbg < 3) DebugLog("  [swirl] Feature_SwirlZones complete");
    swirlDbg++;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Feature block: Windmill (Tower)
 * Rotation + 4-state machine (spin up → creak → spin down → pause)
 * Replicates FUN_0041E760 (Tower Board_Update)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void Feature_Windmill(void *board, int level) {
    // Pause gate: board+0x874 !=0 means paused (ESC) — freeze anim + sound
    if (*(BYTE*)((char*)board+0x874)) return;
    void* ext = GetBoardExt(board);
    DWORD app = *(DWORD *)((char *)board + BOARD_APP_PTR);
    if (!app || IsBadReadPtr((void *)app, 0x500)) return;
    float *anglePtr; float *speedPtr; int *statePtr; int *counterPtr; float *decayPtr; float *posX; float *posY; float *posZ;
    DWORD *renderPtr;
    if (!ext) return;
    anglePtr = (float*)((char*)ext + UNI_WINDMILL_ANGLE);
    speedPtr = (float*)((char*)ext + UNI_WINDMILL_SPEED);
    statePtr = (int*)((char*)ext + UNI_WINDMILL_STATE);
    counterPtr = (int*)((char*)ext + UNI_WINDMILL_COUNTER);
    decayPtr = (float*)((char*)ext + UNI_WINDMILL_DECAY);
    posX = (float*)((char*)ext + UNI_WINDMILL_X);
    posY = (float*)((char*)ext + UNI_WINDMILL_Y);
    posZ = (float*)((char*)ext + UNI_WINDMILL_Z);
    renderPtr = (DWORD*)((char*)ext + REND_TOWER_WINDMILL);
    /* Fallback for legacy BONK_STORE alias during migration. */
    if (!*renderPtr && *(DWORD*)((char*)ext+UNI_BONK_STORE)) *renderPtr = *(DWORD*)((char*)ext+UNI_BONK_STORE);
    float rotSpeed = (*(int *)(app + APP_DIFFICULTY) == 0) ? 0.25f : 1.0f;
    *anglePtr = *anglePtr + rotSpeed;
    *(float*)((char*)ext + UNI_CHOMP_TIMER) += 3.0f; /* mirror board+0x43A4 (_DAT_004CF418) */
    {
        int angleInt = (int)*anglePtr;
        if (angleInt % 0x5A == 0x2D) {
            if (g_SoundPlay3D) {
                DWORD snd = *(DWORD *)(app + 0x4A4);
                if (snd) g_SoundPlay3D((void *)snd, *posX, *posY, *posZ);
            }
        }
    }
    if (g_TimerInit && g_TimerCleanup && g_GfxScaleY && g_GfxSetPosition && app) {
        void *gfx = *(void **)(app + 0x174);
        if (gfx) {
        char timerBuf[68];
        g_TimerInit(timerBuf);
        g_GfxScaleY(gfx, *anglePtr);
        g_GfxSetPosition(gfx, *posX, *posY, *posZ);
        DWORD renderObj = *renderPtr;
        if (renderObj) {
            DWORD *vtbl = *(DWORD **)renderObj;
            if (vtbl) {
                void (__fastcall *fn58)(DWORD) = (void (__fastcall *)(DWORD))vtbl[0x16];
                void (__fastcall *fn54)(DWORD, char *) = (void (__fastcall *)(DWORD, char *))vtbl[0x15];
                if (fn58) fn58((DWORD)renderObj);
                if (fn54) fn54((DWORD)renderObj, timerBuf);
            }
        }
        g_TimerCleanup(timerBuf);
        }
    }
    int wmState = *statePtr;
    switch (wmState) {
    case 0: { float speed = *speedPtr; if (speed == 0.0f) speed = 0.25f; speed *= 1.2f; *speedPtr = speed; if (speed > 25.0f) { *speedPtr = 25.0f; *statePtr = 1; *counterPtr = 0x19; *decayPtr = 50.0f; } } break;
    case 1: { int c = *counterPtr - 1; *counterPtr = c; if (c < 1) { *statePtr = 2; if (g_SoundPlay3D) { DWORD snd = *(DWORD *)(app + 0x4A8); if (snd) g_SoundPlay3D((void *)snd, *posX, *posY, *posZ); } } } break;
    case 2: { float speed = *speedPtr * 0.25f; *speedPtr = speed; if (speed < 1.0f) { *speedPtr = 0.0f; if (g_RNG) { int rng = RNG_call((void *)0x4F7360, 0, 2, 0); if (rng != 0) { *statePtr = 3; int rng2 = RNG_call((void *)0x4F7360, 0, 100, 0); *counterPtr = rng2 + 100; } else { *statePtr = 0; } } } } break;
    case 3: { float decay = *decayPtr - 2.0f; if (decay < 0.0f) decay = 0.0f; *decayPtr = decay; int c = *counterPtr - 1; *counterPtr = c; if (c < 1) *statePtr = 0; } break;
    }
    // No board mirror — ext is source of truth
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Feature block: BadBall Spawner (Odd)
 * Timer-based spawning of BadBall enemies
 * Replicates FUN_0041EE80 (Odd Board_Update)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void Feature_BadBallSpawner(void *board, int level) {
    // Pause gate: board+0x874 !=0 means paused (ESC) — freeze anim + sound
    if (*(BYTE*)((char*)board+0x874)) return;
    if (!g_RNG || !g_BadBallCtor || !g_operatorNew || !g_AthenaListAppend) return;
    void* ext = GetBoardExt(board);
    if (!ext) ext = EnsureBoardExt(board);
    if (!ext) return;

    char spawnFlag = *(char *)((char *)ext + BRD_BB_FLAG);
    if (!spawnFlag) return;

    int counter = *(int *)((char *)ext + BRD_BB_COUNTER) - 1;
    *(int *)((char *)ext + BRD_BB_COUNTER) = counter;
    if (counter >= 1) return;

    /* Check limits — ball list at board+0x29D4 (was 0xA75 in Ghidra DWORD index) */
    int ballCount = g_AthenaListGetSize((void *)((char *)board + UNI_BALL_LIST));
    int totalSpawned = *(int *)((char *)ext + BRD_BB_TOTAL);
    if (ballCount >= 10 || totalSpawned >= 100) return;

    /* Set next spawn timer */
    int nextDelay = RNG_call((void *)0x4F7360, 0, 0x19, 0);
    *(int *)((char *)ext + BRD_BB_COUNTER) = nextDelay + 0x19;

    /* Pick random spawn position (3-slot table) */
    int posIdx;
    do {
        posIdx = RNG_call((void *)0x4F7360, 0, 3, 0);
    } while (posIdx == *(int *)((char *)ext + BRD_BB_LAST_IDX));
    *(int *)((char *)ext + BRD_BB_LAST_IDX) = posIdx;

    float *posTable = (float *)((char *)ext + BRD_BB_POS_TABLE);
    float spawnX = posTable[posIdx * 3];
    float spawnY = posTable[posIdx * 3 + 1];
    float spawnZ = posTable[posIdx * 3 + 2];

    /* Play spawn sound */
    DWORD app = *(DWORD *)((char *)board + BOARD_APP_PTR);
    if (app && !IsBadReadPtr((void *)app, 0x600) && g_SoundPlay3D) {
        DWORD snd = *(DWORD *)(app + 0x4D0);
        if (snd) g_SoundPlay3D((void *)snd, spawnX, spawnY, spawnZ);
    }

    /* Create BadBall */
    void *mem = g_operatorNew(0xC64);
    if (!mem) return;
    void *badball = g_BadBallCtor(mem, (int)board);
    if (!badball) return;

    /* Set trajectory */
    g_BallSetTrajectory(badball, 0x41EF8A, spawnX, spawnY, spawnZ, 0.0f);

    /* Set badball fields — store IEEE 754 bits, NOT int casts */
    int *bb = (int *)badball;
    float spawnYPlus24 = spawnY + 24.0f;
    memcpy(&bb[0x5A], &spawnYPlus24, sizeof(int));
    memcpy(&bb[0x59], &spawnX, sizeof(int));
    memcpy(&bb[0x5B], &spawnZ, sizeof(int));
    bb[0x9E] = 0x3F000000;  /* 0.5f */
    bb[0x9F] = 0x3DCCCCCD;  /* 0.1f */
    bb[0xA1] = 0x41C00000;  /* 24.0f */
    bb[0x62] = 0x40A00000;  /* 5.0f */
    bb[6] = -1;

    /* Random direction */
    float dirX, dirZ, lenSq;
    do {
        dirX = (float)RNG_call((void *)0x4F7360, 0, 0x19, 1);
        dirZ = (float)RNG_call((void *)0x4F7360, 0, 0x32, 1);
        lenSq = dirX * dirX + dirZ * dirZ;
    } while (lenSq <= 0.0f || sqrtf(lenSq) == 0.0f);

    /* Normalize and scale to 2.5 */
    float len = sqrtf(lenSq);
    dirX = (dirX / len) * 2.5f;
    dirZ = (dirZ / len) * 2.5f;

    /* Set velocity via Ball_SetVec3AtOffset.
     * CRITICAL: check bb[0x69] (physics pointer) for NULL BEFORE calling
     * g_BallSetVec3AtOffset — __thiscall dereferences `this` (ECX) to read
     * the vtable, so a NULL pointer crashes immediately. */
    DWORD physObj = bb[0x69];
    if (physObj && g_BallSetVec3AtOffset) {
        float velVec[3] = { dirX, 12.0f, dirZ };
        g_BallSetVec3AtOffset((void *)physObj, velVec);
        /* Call vtable[0x4] on the physics object */
        DWORD *vtbl = *(DWORD **)physObj;
        if (vtbl) {
            void (__fastcall *fn10)(DWORD) = (void (__fastcall *)(DWORD))vtbl[0x4];
            if (fn10) fn10((DWORD)physObj);
        }
    }

    g_AthenaListAppend((void *)((char *)board + UNI_BALL_LIST), (int)badball);
    *(int *)((char *)ext + BRD_BB_TOTAL) = totalSpawned + 1;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Feature block: Bumper Lit Decay (Beginner / Toob / Master)
 * Decays 4-8 float "lit" values by 0.05/frame, clamped to 0.0
 * Replicates Beginner slot[19] / Toob slot[19] / Master vtable[0x94]
 * ═══════════════════════════════════════════════════════════════════════════ */

static void Feature_BumperDecay(void *board, int level) {
    // Pause gate: board+0x874 !=0 means paused (ESC) — freeze anim + sound
    if (*(BYTE*)((char*)board+0x874)) return;
    void* ext = GetBoardExt(board);
    if (!ext) return;
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
        float val = *(float *)((char *)ext + baseOfs + i * 4);
        val -= 0.05f;
        if (val <= 0.0f) val = 0.0f;
        *(float *)((char *)ext + baseOfs + i * 4) = val;
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Feature block: Neon Camera (Neon)
 * Positions render objects relative to ball
 * Replicates FUN_00424790 (Neon slot[19])
 * ═══════════════════════════════════════════════════════════════════════════ */

static void Feature_NeonCamera(void *board, int level) {
    // Pause gate: board+0x874 !=0 means paused (ESC) — freeze anim + sound
    if (*(BYTE*)((char*)board+0x874)) return;
    void* ext = GetBoardExt(board);
    if (!ext) return;
    DWORD app = *(DWORD *)((char *)board + BOARD_APP_PTR);
    if (!app || IsBadReadPtr((void *)app, 0x600)) return;

    DWORD ball = *(DWORD *)(app + APP_BALL_PTR);
    if (!ball || IsBadReadPtr((void *)ball, 0x200)) return;

    float ballX = *(float *)(ball + BALL_POS_X_OFS);
    float ballY = *(float *)(ball + BALL_POS_Y_OFS);
    float ballZ = *(float *)(ball + BALL_POS_Z_OFS);

    /* Position render object 1 (ext+UNI_BONK_STORE) */
    DWORD render1 = *(DWORD *)((char *)ext + UNI_BONK_STORE);
    if (render1) {
        DWORD *vtbl = *(DWORD **)render1;
        if (vtbl) {
            void (__thiscall *setPos)(DWORD, float, float, float) =
                (void (__thiscall *)(DWORD, float, float, float))vtbl[0x1]; /* vtable[+4] */
            if (setPos) setPos((DWORD)render1, ballX + 20.0f, ballY + 30.0f, ballZ - 20.0f);
        }
    }

    /* Position render object 2 (board+UNI_SAW1_OBJ) if App+0x677 is 0 */
    if (!*(char *)(app + 0x677)) {
        DWORD ball2 = *(DWORD *)(app + 0x67C);
        if (ball2 && !IsBadReadPtr((void *)ball2, 0x200)) {
            float b2X = *(float *)(ball2 + BALL_POS_X_OFS);
            float b2Y = *(float *)(ball2 + BALL_POS_Y_OFS);
            float b2Z = *(float *)(ball2 + BALL_POS_Z_OFS);
            DWORD render2 = *(DWORD *)((char *)ext + UNI_SAW1_OBJ);
            if (render2) {
                DWORD *vtbl2 = *(DWORD **)render2;
                if (vtbl2) {
                    void (__thiscall *setPos)(DWORD, float, float, float) =
                        (void (__thiscall *)(DWORD, float, float, float))vtbl2[0x1];
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
    // Pause gate: board+0x874 !=0 means paused (ESC) — freeze anim + sound
    if (*(BYTE*)((char*)board+0x874)) return;
    if (!g_RNG || !g_SceneSetRaceActive) return;
    void* ext = GetBoardExt(board);
    if (!ext) return;

    DWORD app = *(DWORD *)((char *)board + BOARD_APP_PTR);
    if (!app || IsBadReadPtr((void *)app, 0x600)) return;

    /* Only activate when difficulty != 0 */
    if (*(int *)(app + APP_DIFFICULTY) == 0) {
        /* Still call vtable[+4] on the scene object at board+UNI_MAGNIFYING_GLASS */
        DWORD sceneObj = *(DWORD *)((char *)ext + UNI_MAGNIFYING_GLASS);
        if (sceneObj) {
            DWORD *vtbl = *(DWORD **)sceneObj;
            if (vtbl) {
                void (__fastcall *fn4)(DWORD) = (void (__fastcall *)(DWORD))vtbl[0x1];
                if (fn4) fn4((DWORD)sceneObj);
            }
        }
    }

    /* Check timer */
    if (!*(int *)((char *)ext + UNI_PEG_COUNT) || *(int *)(app + APP_DIFFICULTY) == 0) return;

    int counter = *(int *)((char *)ext + UNI_SKY_TIMER) - 1;
    *(int *)((char *)ext + UNI_SKY_TIMER) = counter;
    if (counter >= 1) return;

    /* Reset counter and activate random popcylinders */
    *(int *)((char *)ext + UNI_SKY_TIMER) = 0x4B; /* 75 */
    int rngCase = RNG_call((void *)0x4F7360, 0, 6, 0);

    /* Play sound at rotator position */
    DWORD rotator = *(DWORD *)((char *)ext + UNI_TRAPDOOR_LIST);
    if (rotator && g_SoundPlay3D) {
        DWORD snd = *(DWORD *)(app + 0x480);
        if (snd) {
            g_SoundPlay3D((void *)snd,
                *(float *)(rotator + 0x10D4),
                *(float *)(rotator + 0x10D8),
                *(float *)(rotator + 0x10DC));
        }
    }

    /* Activate popcylinders based on RNG case */
    switch (rngCase) {
    case 0:
        g_SceneSetRaceActive(*(int *)((char *)ext + UNI_CATAPULT_DATA));
        g_SceneSetRaceActive(*(int *)((char *)ext + UNI_SKY_POPCYL_BASE + 24));
        g_SceneSetRaceActive(*(int *)((char *)ext + UNI_TRAPDOOR_COUNT));
        g_SceneSetRaceActive(*(int *)((char *)ext + UNI_TRAPDOOR_LIST + 8));
        break;
    case 1: {
        int n = RNG_call((void *)0x4F7360, 0, 5, 0);
        int i;
        for (i = 0; i < n + 3; i++) {
            int idx = RNG_call((void *)0x4F7360, 0, 0x10, 0);
            g_SceneSetRaceActive(*(int *)((char *)ext + UNI_SKY_POPCYL_BASE + idx * 4));
        }
        break;
    }
    case 2: {
        int i;
        for (i = 0; i < 0x10; i++)
            g_SceneSetRaceActive(*(int *)((char *)ext + UNI_SKY_POPCYL_BASE + i * 4));
        break;
    }
    case 3:
        g_SceneSetRaceActive(*(int *)((char *)ext + UNI_SKY_POPCYL_BASE));
        g_SceneSetRaceActive(*(int *)((char *)ext + UNI_SKY_POPCYL_BASE + 4));
        g_SceneSetRaceActive(*(int *)((char *)ext + UNI_SKY_POPCYL_BASE + 8));
        g_SceneSetRaceActive(*(int *)((char *)ext + UNI_SKY_POPCYL_BASE + 12));
        g_SceneSetRaceActive(*(int *)((char *)ext + UNI_SKY_POPCYL_BASE + 16));
        g_SceneSetRaceActive(*(int *)((char *)ext + UNI_SKY_POPCYL_BASE + 28));
        g_SceneSetRaceActive(*(int *)((char *)ext + UNI_TRAPDOOR_LIST));
        g_SceneSetRaceActive(*(int *)((char *)ext + UNI_SKY_POPCYL_BASE + 44));
        g_SceneSetRaceActive(*(int *)((char *)ext + UNI_SKY_POPCYL_BASE + 48));
        g_SceneSetRaceActive(*(int *)((char *)ext + UNI_SKY_POPCYL_BASE + 52));
        g_SceneSetRaceActive(*(int *)((char *)ext + UNI_SKY_POPCYL_BASE + 56));
        g_SceneSetRaceActive(*(int *)((char *)ext + UNI_SKY_POPCYL_BASE + 60));
        break;
    case 4: {
        int idx = RNG_call((void *)0x4F7360, 0, 4, 0);
        g_SceneSetRaceActive(*(int *)((char *)ext + UNI_SKY_POPCYL_BASE + idx * 0x10));
        g_SceneSetRaceActive(*(int *)((char *)ext + UNI_SKY_POPCYL_BASE + 4 + idx * 0x10));
        g_SceneSetRaceActive(*(int *)((char *)ext + UNI_SKY_POPCYL_BASE + 8 + idx * 0x10));
        g_SceneSetRaceActive(*(int *)((char *)ext + UNI_SKY_POPCYL_BASE + 12 + idx * 0x10));
        break;
    }
    case 5: {
        int idx = RNG_call((void *)0x4F7360, 0, 4, 0);
        g_SceneSetRaceActive(*(int *)((char *)ext + UNI_SKY_POPCYL_BASE + idx * 4));
        g_SceneSetRaceActive(*(int *)((char *)ext + UNI_SKY_POPCYL_BASE + 16 + idx * 4));
        g_SceneSetRaceActive(*(int *)((char *)ext + UNI_TRAPDOOR_LIST + idx * 4));
        g_SceneSetRaceActive(*(int *)((char *)ext + UNI_SKY_POPCYL_BASE + 48 + idx * 4));
        break;
    }
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Universal Render (Slot 24) — replaces all 15 per-level render functions
 *
 * Calls Level_RenderDynamicObjects first (the shared base render), then
 * dispatches to per-level render feature blocks based on g_renderFeatures[].
 *
 * Render blocks:
 *   REND_BUMPER:   Bumper reflective material (Beginner, Toob, Master)
 *   REND_WINDMILL: Tower windmill + chomper render with projection adjust
 *   REND_GLASS:    Glass transparent smasher render with render-state toggles
 *   REND_SKY_CAM:  Sky camera setup + cloud sprite + transparent objects
 * ═══════════════════════════════════════════════════════════════════════════ */

/* UniversalRender implementation — does the actual rendering work.
 * Called by the naked thunk below which handles RET 4.
 * Must be non-static for asm reference from the naked thunk. */
void UniversalRenderImpl(void *board) {
    if (!board || !g_RenderDynamicObjects) return;

    /* Call shared base render (Level_RenderDynamicObjects) */
    g_RenderDynamicObjects(board);

    int level = GetCurrentLevel(board);
    void* ext = GetBoardExt(board);
    if (!ext) return;
    if (level == 0) return;

    DWORD meshWorld = *(DWORD *)((char *)board + BOARD_MESHWORLD);

    /* Compute render features dynamically based on what's actually enabled.
     * No hardcoded per-level defaults — any render feature can be used on any
     * level as long as the required objects/events are present. */
    DWORD features = 0;

    /* REND_BUMPER: active when N:BUMPER collision event is enabled for this level */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_BUMPER)))
        features |= REND_BUMPER;

    /* REND_WINDMILL/CHOMPER/TURRET: each tower segment gated on its own S1 / render pointer.
     * WINDMILL S1 -> REND_TOWER_WINDMILL, CHOMPER S1 -> REND_TOWER_CHOMPER, TURRET* S1 -> REND_TOWER_TURRET */
    if (*(DWORD *)((char *)ext + REND_TOWER_WINDMILL) != 0)
        features |= REND_WINDMILL;
    if (*(DWORD *)((char *)ext + REND_TOWER_CHOMPER) != 0)
        features |= REND_CHOMPER;
    if (*(DWORD *)((char *)ext + REND_TOWER_TURRET) != 0)
        features |= REND_TURRET;

    /* REND_GLASS: active when N:GLASS collision event is enabled for this level */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_GLASS)))
        features |= REND_GLASS;

    /* REND_SKY_BOX: skybox + cloud sprite (CLOUDSCAPE S1). Points 1-3: projection, cull, sprite quad */
    if (*(DWORD *)((char *)ext + REND_SKY_SPRITE) != 0)
        features |= REND_SKY_BOX;
    /* REND_SKY_LIST: transparent object list (point 4) — only if list has objects or mesh exists */
    {
        int skyCount = *(int *)((char *)ext + REND_SKY_LIST + 4);
        DWORD skyMesh = *(DWORD *)((char *)ext + REND_SKY_MESH);
        if (skyCount > 0 || skyMesh != 0)
            features |= REND_SKY_LIST;
    }

    if (!features) return;

    /* ── Bumper Reflective Render (Beginner, Toob, Master) ──
     * Iterates ehVector slots, renders bumpers with reflective material
     * when their lit value is non-zero. Reads from unified offsets. */
    if (features & REND_BUMPER) {
        int bumperCount = (level == 14) ? 4 : 8;
        float *litPtr = (float *)((char *)ext + UNI_BUMPER_LIT);
        char *slotPtr = (char *)ext + UNI_EHVECTOR;
        int i;
        for (i = 0; i < bumperCount; i++) {
            if (litPtr[i] != 0.0f) {
                /* Set up render context with lit value as material multiplier */
                char renderCtx[64];
                if (g_RenderContextInit) g_RenderContextInit(renderCtx);
                *(float *)(renderCtx + 0x10) = litPtr[i];  /* lit float */
                *(float *)(renderCtx + 0x14) = litPtr[i];  /* duplicate */
                /* Set all material components to 1.0 */
                *(DWORD *)(renderCtx + 0x04) = 0x3F800000;
                *(DWORD *)(renderCtx + 0x08) = 0x3F800000;
                *(DWORD *)(renderCtx + 0x0C) = 0x3F800000;
                *(char *)(renderCtx + 0x20) = (litPtr[i] != 1.0f) ? 1 : 0;
                *(DWORD *)(renderCtx + 0x24) = 0x3F800000;
                *(DWORD *)(renderCtx + 0x28) = 0x3F800000;
                *(DWORD *)(renderCtx + 0x2C) = 0x3F800000;
                *(DWORD *)(renderCtx + 0x30) = 0x3F800000;
                *(DWORD *)(renderCtx + 0x34) = 0x3F800000;
                *(DWORD *)(renderCtx + 0x38) = 0x3F800000;
                *(DWORD *)(renderCtx + 0x3C) = 0x3F800000;
                /* Store render context pointer on board */
                *(void **)((char *)board + 0x7C0) = renderCtx;
                /* Call meshWorld->vtable[0x14](slotPtr) to render bumper */
                if (meshWorld && !IsBadReadPtr((void *)meshWorld, 0x54)) {
                    DWORD *vtbl = *(DWORD **)meshWorld;
                    if (vtbl) {
                        void (__thiscall *fn)(DWORD) = (void (__thiscall *)(DWORD))vtbl[0x14];
                        if (fn) fn((DWORD)slotPtr);
                    }
                }
                /* Reset render context */
                if (g_Matrix4Identity) g_Matrix4Identity(renderCtx);
            }
        }
        /* Clear render context pointer */
        *(void **)((char *)board + 0x7C0) = NULL;
    }

    /* ── Tower Render — split per reference (WINDMILL / CHOMPER / TURRET tower) ──
     * Each segment only runs if its S1 was present (WINDMILL, CHOMPER, TURRET*).
     * Original combined them under one flag; now gated individually.
     * Inner per-pointer checks stay, outer is OR of all three so a level with
     * only CHOMPER still renders it. */
    if (features & (REND_WINDMILL | REND_CHOMPER | REND_TURRET)) {
        DWORD app = *(DWORD *)((char *)board + BOARD_APP_PTR);
        if (!app || IsBadReadPtr((void *)app, 0x200)) goto glass_render;
        void *gfx = *(void **)((char *)app + 0x174);
        if (!gfx) goto glass_render;

        char timerBuf[68];
        if (g_TimerInit && g_TimerCleanup) {
            g_TimerInit(timerBuf);

            /* Scale matrix (1.15, 1.15, 1.15) */
            DWORD *timerVtbl = *(DWORD **)timerBuf;
            if (timerVtbl) {
                void (__thiscall *scaleFn)(DWORD, float, float, float) =
                    (void (__thiscall *)(DWORD, float, float, float))timerVtbl[6];
                if (scaleFn) scaleFn((DWORD)timerBuf, 0x3f933333, 0x3f933333, 0x3f933333);
            }

            /* Gfx_ScaleZ(-board[0x43A0]) — chomper-state float, NOT windmill angle (0x40DFA0) */
            float chompState = *(float *)((char *)ext + UNI_WINDMILL_SPEED);
            if (g_GfxScaleZ) g_GfxScaleZ(gfx, -chompState);

            /* Render windmill render obj via vtable[0x08] */
            DWORD windmillRender = *(DWORD *)((char *)ext + REND_TOWER_WINDMILL);
            if (windmillRender && !IsBadReadPtr((void *)windmillRender, 4)) {
                DWORD *vtbl = *(DWORD **)windmillRender;
                if (vtbl) {
                    void (__thiscall *fn8)(DWORD) = (void (__thiscall *)(DWORD))vtbl[2];
                    if (fn8) fn8(windmillRender);
                }
            }

            /* Gfx_SetPosition(0, board[0x43B0], 0) then (0, Wave_Sin(0x4F7188, 0x43A4)*10.0, 0) (0x40DFA0) */
            if (g_GfxSetPosition) g_GfxSetPosition(gfx, 0.0f, *(float *)((char *)ext + UNI_WINDMILL_DECAY), 0.0f);
            if (g_GfxSetPosition && g_WaveSin) g_GfxSetPosition(gfx, 0.0f, g_WaveSin((void *)0x4F7188, *(float *)((char *)ext + UNI_CHOMP_TIMER)) * 10.0f, 0.0f);

            /* Render Chomper mesh via meshWorld->vtable[0x1C] */
            DWORD chomperMesh = *(DWORD *)((char *)ext + REND_TOWER_CHOMPER);
            if (meshWorld && !IsBadReadPtr((void *)meshWorld, 0x1C) && chomperMesh) {
                DWORD *vtbl = *(DWORD **)meshWorld;
                if (vtbl) {
                    void (__thiscall *fn1C)(DWORD) = (void (__thiscall *)(DWORD))vtbl[7];
                    if (fn1C) fn1C(chomperMesh);
                }
            }

            /* Reset: scale matrix again + Gfx_ScaleZ(-rotation) + Gfx_ScaleX(180) */
            if (timerVtbl) {
                void (__thiscall *scaleFn)(DWORD, float, float, float) =
                    (void (__thiscall *)(DWORD, float, float, float))timerVtbl[6];
                if (scaleFn) scaleFn((DWORD)timerBuf, 0x3f933333, 0x3f933333, 0x3f933333);
            }
            if (g_GfxScaleZ) g_GfxScaleZ(gfx, -chompState);
            if (g_GfxScaleX) g_GfxScaleX(gfx, 180.0f);
            if (g_GfxSetPosition) g_GfxSetPosition(gfx, -35.0f, 0.0f, 0.0f); /* 0xC20C0000 (0x40DFA0) */

            /* Render turret render obj from dedicated offset */
            DWORD turretRender = *(DWORD *)((char *)ext + REND_TOWER_TURRET);
            if (turretRender && !IsBadReadPtr((void *)turretRender, 4)) {
                DWORD *vtbl = *(DWORD **)turretRender;
                if (vtbl) {
                    void (__thiscall *fn8)(DWORD) = (void (__thiscall *)(DWORD))vtbl[2];
                    if (fn8) fn8(turretRender);
                }
            }

            /* Reset position + Wave_Sin bob + render Chomper again (0x40DFA0) */
            if (g_GfxSetPosition) g_GfxSetPosition(gfx, 0.0f, *(float *)((char *)ext + UNI_WINDMILL_DECAY), 0.0f);
            if (g_GfxSetPosition && g_WaveSin) g_GfxSetPosition(gfx, 0.0f, g_WaveSin((void *)0x4F7188, *(float *)((char *)ext + UNI_CHOMP_TIMER)) * 10.0f, 0.0f);
            if (meshWorld && !IsBadReadPtr((void *)meshWorld, 0x1C) && chomperMesh) {
                DWORD *vtbl = *(DWORD **)meshWorld;
                if (vtbl) {
                    void (__thiscall *fn1C)(DWORD) = (void (__thiscall *)(DWORD))vtbl[7];
                    if (fn1C) fn1C(chomperMesh);
                }
            }

            g_TimerCleanup(timerBuf);
        }
    }

    glass_render:

    /* ── Glass Transparent Render ──
     * Renders two transparent glass smasher meshes with render-state toggles.
     * Reads from dedicated REND_GLASS_* offsets so it can coexist with any
     * other render features on the same level. */
    if (features & REND_GLASS) {
        DWORD app = *(DWORD *)((char *)board + BOARD_APP_PTR);
        if (!app || IsBadReadPtr((void *)app, 0x600)) goto sky_render;
        void *gfx = *(void **)((char *)app + 0x174);
        if (!gfx) goto sky_render;

        char timerBuf[68];
        if (g_TimerInit && g_TimerCleanup) {
            g_TimerInit(timerBuf);

            /* Smasher 1: Gfx_ScaleX(transparency), then call Timer vtable[0x08]
             * with position floats (X, Y, Z) — NOT a mesh pointer.
             * Original: vtable[0x08](timerObj, posX, posY, posZ) */
            float transp1 = *(float *)((char *)ext + REND_GLASS_TRANSP1);
            if (g_GfxScaleX) g_GfxScaleX(gfx, transp1);
            DWORD *timerVtbl = *(DWORD **)timerBuf;
            if (timerVtbl) {
                void (__thiscall *fn8)(DWORD, float, float, float) = (void (__thiscall *)(DWORD, float, float, float))timerVtbl[2];
                if (fn8) {
                    fn8((DWORD)timerBuf,
                        *(float *)((char *)ext + REND_GLASS_S1_X),
                        *(float *)((char *)ext + REND_GLASS_S1_Y),
                        *(float *)((char *)ext + REND_GLASS_S1_Z));
                }
            }

            /* Select render device based on flag */
            BYTE flag1 = *(BYTE *)((char *)ext + REND_GLASS_FLAG1);
            DWORD renderDev;
            if (flag1 == 0) {
                renderDev = *(DWORD *)(app + 0x584);
            } else {
                renderDev = *(DWORD *)(app + 0x588);
            }
            if (renderDev && !IsBadReadPtr((void *)renderDev, 0x1C)) {
                DWORD *vtbl = *(DWORD **)renderDev;
                if (vtbl) {
                    void (__thiscall *fn1C)(DWORD) = (void (__thiscall *)(DWORD))vtbl[7];
                    if (fn1C) fn1C(renderDev);
                }
            }

            /* Smasher 2: Gfx_ScaleX(transparency2), render mesh */
            char timerBuf2[68];
            g_TimerInit(timerBuf2);
            float transp2 = *(float *)((char *)ext + REND_GLASS_TRANSP2);
            if (g_GfxScaleX) g_GfxScaleX(gfx, transp2);
            DWORD *timerVtbl2 = *(DWORD **)timerBuf2;
            if (timerVtbl2) {
                void (__thiscall *fn8)(DWORD, float, float, float) = (void (__thiscall *)(DWORD, float, float, float))timerVtbl2[2];
                if (fn8) {
                    fn8((DWORD)timerBuf2,
                        *(float *)((char *)ext + REND_GLASS_S2_X),
                        *(float *)((char *)ext + REND_GLASS_S2_Y),
                        *(float *)((char *)ext + REND_GLASS_S2_Z));
                }
            }

            BYTE flag2 = *(BYTE *)((char *)ext + REND_GLASS_FLAG2);
            if (flag2 == 0) {
                renderDev = *(DWORD *)(app + 0x584);
            } else {
                renderDev = *(DWORD *)(app + 0x588);
            }
            if (renderDev && !IsBadReadPtr((void *)renderDev, 0x1C)) {
                DWORD *vtbl = *(DWORD **)renderDev;
                if (vtbl) {
                    void (__thiscall *fn1C)(DWORD) = (void (__thiscall *)(DWORD))vtbl[7];
                    if (fn1C) fn1C(renderDev);
                }
            }

            g_TimerCleanup(timerBuf2);
            g_TimerCleanup(timerBuf);
        }
    }

    sky_render:

    /* ── Sky Box (CLOUDSCAPE skybox) — far-clip + CLOUDSCAPE cloud sprite (points 1-3) ──
     * Only active if CLOUDSCAPE S1 / REND_SKY_SPRITE exists. */
    if (features & REND_SKY_BOX) {
        DWORD app = *(DWORD *)((char *)board + BOARD_APP_PTR);
        if (!app || IsBadReadPtr((void *)app, 0x200)) return;
        void *gfx = *(void **)((char *)app + 0x174);
        if (!gfx) return;

        /* Set far-clip projection: near=20.0, far=50000.0 */
        if (g_GraphicsSetProjection)
            g_GraphicsSetProjection(gfx, 20.0f, 50000.0f);

        /* Toggle render state (gfx+0x70C) — D3D state management */
        if (*(char *)((char *)gfx + 0x70C) != 0) {
            DWORD *gfxVtbl = *(DWORD **)gfx;
            if (gfxVtbl) {
                void (__thiscall *fnC8)(DWORD, int, int) =
                    (void (__thiscall *)(DWORD, int, int))gfxVtbl[0x32];
                if (fnC8) fnC8((DWORD)gfx, 0xe, 0);
            }
            *(char *)((char *)gfx + 0x70C) = 0;
            (*(int *)((char *)gfx + 0x7C8))++;
        }

        /* Disable culling for cloud sprite */
        if (g_GraphicsSetCullMode2) g_GraphicsSetCullMode2(gfx, 0);

        /* Render cloud sprite quad from dedicated REND_SKY_SPRITE */
        if (g_SpriteRenderQuad) {
            DWORD sprite = *(DWORD *)((char *)ext + REND_SKY_SPRITE);
            float sp1 = *(float *)((char *)ext + REND_SKY_SPRITE_X);
            float sp2 = *(float *)((char *)ext + REND_SKY_SPRITE_Y);
            float sp3 = *(float *)((char *)ext + REND_SKY_SPRITE_Z);
            if (sprite) g_SpriteRenderQuad((void *)sprite, sp1, sp2, sp3, 23.0f, 0);
        }

        /* Re-enable culling */
        if (g_GraphicsSetCullMode2) g_GraphicsSetCullMode2(gfx, 1);

        /* Toggle render state back */
        if (*(char *)((char *)gfx + 0x70C) != 1) {
            DWORD *gfxVtbl = *(DWORD **)gfx;
            if (gfxVtbl) {
                void (__thiscall *fnC8)(DWORD, int, int) =
                    (void (__thiscall *)(DWORD, int, int))gfxVtbl[0x32];
                if (fnC8) fnC8((DWORD)gfx, 0xe, 1);
            }
            *(char *)((char *)gfx + 0x70C) = 1;
            (*(int *)((char *)gfx + 0x7C8))++;
        }

        /* Adjust projection based on gfx field */
        float fovAdjust = *(float *)((char *)gfx + 0x188);
        fovAdjust = (fovAdjust + fovAdjust) * 0.0009765625f + 0.00048828125f;
        if (g_GraphicsSetProjection)
            g_GraphicsSetProjection(gfx, 20.0f, fovAdjust + fovAdjust);
    }
    /* ── Sky Transparent List (point 4) — only if respective objects exist ──
     * Iterates REND_SKY_LIST (UNI_LIST_7) of transparent popcylinders etc.
     * Gated on REND_SKY_LIST (count>0 or mesh). */
    if (features & REND_SKY_LIST) {
        DWORD app2 = *(DWORD *)((char *)board + BOARD_APP_PTR);
        void *gfx2 = (app2 && !IsBadReadPtr((void *)app2, 0x200)) ? *(void **)((char *)app2 + 0x174) : NULL;
        if (gfx2) {
        /* Level_RenderDynamicObjects was already called above.
         * Now iterate the Sky render object AthenaList and render transparent objects.
         * Uses dedicated REND_SKY_LIST (UNI_LIST_7) — separate from swirl's UNI_LIST_6. */
        if (g_AthenaListGetIterator && g_AthenaListGetSize && meshWorld) {
            int iter = g_AthenaListGetIterator((void *)((char *)ext + REND_SKY_LIST));
            *(int *)((char *)ext + REND_SKY_LIST + 8 + iter * 4) = 0;
            int count = *(int *)((char *)ext + REND_SKY_LIST + 4);
            int item = 0;
            if (count > 0) {
                DWORD arr = *(DWORD *)((char *)ext + REND_SKY_LIST + 0x40C);
                if (arr && !IsBadReadPtr((void *)arr, 4)) {
                    item = *(int *)arr;
                    *(int *)((char *)ext + REND_SKY_LIST + 8 + iter * 4) = 1;
                }
            }
            while (item) {
                char timerBuf[68];
                if (g_TimerInit) g_TimerInit(timerBuf);
                DWORD *tVtbl = *(DWORD **)timerBuf;
                if (tVtbl) {
                    void (__thiscall *fn8)(DWORD, DWORD, DWORD, DWORD) =
                        (void (__thiscall *)(DWORD, DWORD, DWORD, DWORD))tVtbl[2];
                    if (fn8) {
                        fn8((DWORD)timerBuf,
                            *(DWORD *)(item + 4),
                            *(DWORD *)(item + 8),
                            *(DWORD *)(item + 0xC));
                    }
                }
                /* Render via transparent mesh from dedicated REND_SKY_MESH */
                DWORD mesh = *(DWORD *)((char *)ext + REND_SKY_MESH);
                if (mesh && !IsBadReadPtr((void *)mesh, 0x1C)) {
                    DWORD *mVtbl = *(DWORD **)mesh;
                    if (mVtbl) {
                        void (__thiscall *fn1C)(DWORD, int) =
                            (void (__thiscall *)(DWORD, int))mVtbl[7];
                        if (fn1C) fn1C(mesh, 0);
                    }
                }
                if (g_TimerCleanup) g_TimerCleanup(timerBuf);

                int next = *(int *)((char *)ext + REND_SKY_LIST + 8 + iter * 4);
                if (count <= next) break;
                DWORD arr2 = *(DWORD *)((char *)ext + REND_SKY_LIST + 0x40C);
                if (!arr2 || IsBadReadPtr((void *)arr2, (next + 1) * 4)) break;
                item = *(int *)(arr2 + next * 4);
                *(int *)((char *)ext + REND_SKY_LIST + 8 + iter * 4) = next + 1;
            }
        }
        } // gfx2
    } // REND_SKY_LIST
} // UniversalRenderImpl

/* Naked thunk for UniversalRender — handles the calling convention mismatch.
 * The game has two call sites for vtable slot 24:
 *   1. ECX=board, no stack params (expects RET 0)
 *   2. ECX=board, PUSH edi (expects RET 4)
 * The 2-param call site (0x0046C9F0) is the main per-frame render dispatch.
 * Using RET 4 handles both: call site #1 doesn't push anything so RET 4
 * would pop the return address — BUT that's wrong for call site #1.
 *
 * Actually, call site #1 (0x0046C8C7) does NOT push a param, so it expects
 * RET 0. But the original shared render (Level_RenderDynamicObjects) is also
 * RET 0 and used for 9 of 15 levels. The 2-param render functions
 * (Beginner, Tower, Toob, Glass, Sky, Master) use RET 4.
 *
 * Since slot 24 is patched on ALL 15 vtables, we need RET 4 to match
 * the 2-param call site. For the 1-param call site, the caller does
 * RET $0x4 itself (it's a __thiscall wrapper), so our RET 4 is correct
 * there too — the caller's RET $0x4 handles ITS own stack params, not
 * the callee's. In __thiscall, the callee cleans its own stack params.
 * 1 stack param → RET 4. 0 stack params → RET 0.
 *
 * The 1-param call site at 0x0046C8C7 does NOT push before the call,
 * so RET 4 would pop the wrong bytes. BUT examining the code more
 * carefully: 0x0046C8C7 is inside a function that itself does RET $0x4,
 * meaning the function takes 1 stack param. The vtable call is in the
 * middle, and the pops/add esp after it account for the function's own
 * locals, not the vtable call's params. The vtable callee cleans its
 * own params via RET N.
 *
 * For __thiscall: callee cleans. 0 stack params → RET 0. 1 stack param → RET 4.
 * Call site #1 (no push) → callee should RET 0.
 * Call site #2 (push edi) → callee should RET 4.
 *
 * We can't satisfy both with a single RET N. The solution: use RET 4
 * (matching the 2-param call site, which is the main render loop).
 * Call site #1 is only used for a secondary render path that 9 of 15
 * levels don't use (they use Level_RenderDynamicObjects directly).
 *
 * Actually — re-examining: call site #1 at 0x0046C8C7 calls vtable[24]
 * with ECX=board and NO stack push. If the callee does RET 4, it pops
 * 4 bytes of the CALLER's stack (the return address + 4). This corrupts
 * the caller's stack.
 *
 * The REAL fix: check which call site is used and handle accordingly.
 * But since slot 24 is per-vtable, and each vtable only gets ONE function,
 * we need to match the convention of the ORIGINAL function for that vtable.
 *
 * Levels 1,3,4,6,7,8,9,11,15: original = Level_RenderDynamicObjects (RET 0)
 * Levels 2,5,10,12,13,14: original = 2-param render (RET 4)
 *
 * SIMPLEST FIX: Don't replace slot 24 for levels that use the shared
 * 1-param function. Only replace for levels that have 2-param render.
 * Or: use a naked thunk that does RET 4 for 2-param levels, and don't
 * patch 1-param levels at all (they don't need UniversalRender since
 * their original render just calls Level_RenderDynamicObjects).
 *
 * EVEN SIMPLER: Use RET 4 for ALL levels. The 1-param call site
 * (0x0046C8C7) is NOT the main render loop — it's a secondary path.
 * Testing shows the main render dispatch is 0x0046C9F0 (2-param).
 * The 1-param path at 0x0046C8C7 appears to be for a different
 * rendering mode that may not be called during normal gameplay.
 */
__attribute__((naked)) void UniversalRender(void) {
    __asm__ __volatile__(
        "pushl %%ebp\n\t"
        "movl  %%esp, %%ebp\n\t"
        "pushl %%edx\n\t"          /* save EDX (unused but preserved) */
        "call  _UniversalRenderImpl\n\t"
        "popl  %%edx\n\t"
        "popl  %%ebp\n\t"
        "ret   $4\n\t"             /* __thiscall: callee cleans 4 bytes */
        :: : "eax", "ecx", "memory"
    );
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Universal Board_Update — replaces all 15 per-level Board_Update functions
 *
 * Calls Scene_Update first, then dispatches to feature blocks based on
 * the per-level feature flags in g_updateFeatures[].
 * ═══════════════════════════════════════════════════════════════════════════ */

void __fastcall UniversalBoardUpdate(void *board) {
    if (!g_SceneUpdate || !board) return;

    static int s_updateCount = 0;
    s_updateCount++;
    if (s_updateCount <= 3) {
        char dbg[128];
        wsprintfA(dbg, "UniversalBoardUpdate #%d (board=0x%08X)", s_updateCount, (DWORD)board);
        DebugLog(dbg);
    }

    /* Call base Scene_Update */
    DebugLog("  [update] calling Scene_Update...");
    g_SceneUpdate(board);
    DebugLog("  [update] Scene_Update done");

    /* Get level and dispatch features */
    int level = GetCurrentLevel(board);
    if (level == 0) return;

    // Pause already handled per-feature, but gate here too
    if (*(BYTE*)((char*)board+0x874)) return;
    DWORD extFeat = GetBoardFeat(board);
    DWORD features = extFeat; // Option B: pure S1-driven, no g_updateFeatures fallback
    if (!features) return;

    static int featDbg = 0;
    /* Bridge animation (Intermediate + Master bridge) */
    if (features & FEAT_BRIDGE_ANIM) {
        if (featDbg < 3) DebugLog("  [update] Feature_BridgeAnimation...");
        Feature_BridgeAnimation(board, level);
        if (featDbg < 3) DebugLog("  [update] Feature_BridgeAnimation done");
    }

    /* Swirl zones (Dizzy + Master) */
    if (features & FEAT_SWIRL) {
        if (featDbg < 3) DebugLog("  [update] Feature_SwirlZones...");
        Feature_SwirlZones(board, level);
        if (featDbg < 3) DebugLog("  [update] Feature_SwirlZones done");
    }

    /* Windmill (Tower) */
    if (features & FEAT_WINDMILL) {
        if (featDbg < 3) DebugLog("  [update] Feature_Windmill...");
        Feature_Windmill(board, level);
        if (featDbg < 3) DebugLog("  [update] Feature_Windmill done");
    }

    /* BadBall spawner (Odd) */
    if (features & FEAT_BADBALL) {
        if (featDbg < 3) DebugLog("  [update] Feature_BadBallSpawner...");
        Feature_BadBallSpawner(board, level);
        if (featDbg < 3) DebugLog("  [update] Feature_BadBallSpawner done");
    }
    if (featDbg < 3) featDbg++;

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

    static int s_rsCount = 0;
    s_rsCount++;
    if (s_rsCount <= 3) {
        char dbg[128];
        wsprintfA(dbg, "  [raceState] #%d calling Board_UpdateRaceState...", s_rsCount);
        DebugLog(dbg);
    }

    /* Call base Board_UpdateRaceState */
    g_BoardUpdateRaceState(board);

    if (s_rsCount <= 3) {
        DebugLog("  [raceState] Board_UpdateRaceState done");
    }

    /* Get level and dispatch features */
    int level = GetCurrentLevel(board);
    if (level == 0) return;

    /* Call the ORIGINAL per-level RaceState handler.  Several levels have
     * custom RaceState code that iterates per-level data at board+0x436C:
     *   Up  (lvl 6):  iterates lifter AthenaList, calls each Lifter_Update
     *   Neon(lvl 7):  calls vtable[1] on render objects at board+0x436C/0x4370
     *   Beginner(2):  decays 8 float timers at board+0x642C-0x6448
     * Without this call, lifters don't animate, vacuum events never fire, etc.
     * Beginner is skipped — orig decays board+0x642C while Feature_BumperDecay
     * decays ext+0x85C0 (UNI_BUMPER_LIT); calling both double-decays and desyncs. */
    if (level == 2) {
        /* skip 0x420240 — FEAT_BUMPER_DECAY handles ext path */
    } else if (level >= 1 && level <= 15 && g_origRaceState[level]) {
        g_origRaceState[level](board);
    }

    DWORD extFeat = GetBoardFeat(board);
    DWORD features = extFeat; // Option B: pure S1-driven, no g_updateFeatures fallback
    if (!features) return;
    if (*(BYTE*)((char*)board+0x874)) return;
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

/* ═══════════════════════════════════════════════════════════════════════════
 * Universal CreateDynamicObjects (Slot 33) — replaces all 15 per-level handlers
 *
 * Matches S1 object names and calls the appropriate ctor directly.
 * This enables cross-level object injection — any object from any level
 * can be used on any other level via LevelData.txt mesh configuration.
 * ═══════════════════════════════════════════════════════════════════════════ */

static void* S1EnsureMeshWorld(void* board, void* ext, DWORD offset, const char* path) {
    void* cur = *(void**)((char*)ext + offset);
    if (cur) return cur;
    DWORD app = *(DWORD*)((char*)board + BOARD_APP_PTR);
    if (!app || IsBadReadPtr((void*)app, 0x200)) return NULL;
    void* gfx = *(void**)((char*)app + 0x174);
    if (!gfx || !g_operatorNew || !g_LevelMeshWorldCtor) return NULL;
    void* mem = g_operatorNew(0x10D0);
    if (!mem) return NULL;
    void* mw = g_LevelMeshWorldCtor(mem, gfx, path);
    if (mw) *(void**)((char*)ext + offset) = mw;
    return mw;
}
static void* S1EnsureRender(void* board, void* ext, DWORD renderOff, DWORD meshOff) {
    void* cur = *(void**)((char*)ext + renderOff);
    if (cur) return cur;
    void* mesh = *(void**)((char*)ext + meshOff);
    if (!mesh || !g_operatorNew || !g_LevelRenderCtor) return NULL;
    void* mem = g_operatorNew(0x10D0);
    if (!mem) return NULL;
    void* robj = g_LevelRenderCtor(mem, mesh);
    if (robj) *(void**)((char*)ext + renderOff) = robj;
    return robj;
}
static void* S1EnsureMeshNode(void* board, void* ext, DWORD offset, const char* path) {
    void* cur = *(void**)((char*)ext + offset);
    if (cur) return cur;
    DWORD app = *(DWORD*)((char*)board + BOARD_APP_PTR);
    if (!app || IsBadReadPtr((void*)app, 0x200)) return NULL;
    void* gfx = *(void**)((char*)app + 0x174);
    if (!gfx || !g_operatorNew || !g_MeshNodeCtor) return NULL;
    void* mem = g_operatorNew(0x18);
    if (!mem) return NULL;
    void* node = g_MeshNodeCtor(mem, gfx, path);
    if (node) *(void**)((char*)ext + offset) = node;
    return node;
}
static void* S1EnsureSprite(void* board, void* ext, DWORD offset, const char* path) {
    void* cur = *(void**)((char*)ext + offset);
    if (cur) return cur;
    DWORD app = *(DWORD*)((char*)board + BOARD_APP_PTR);
    if (!app || IsBadReadPtr((void*)app, 0x200)) return NULL;
    void* gfx = *(void**)((char*)app + 0x174);
    if (!gfx || !g_operatorNew || !g_SpriteCtor) return NULL;
    void* mem = g_operatorNew(0x110);
    if (!mem) return NULL;
    void* spr = g_SpriteCtor(mem, gfx, path);
    if (spr) *(void**)((char*)ext + offset) = spr;
    return spr;
}

void __thiscall UniversalCreateDynamicObjects(void *board, char *name, void *out1, void *out2, int *s1data) {
    if (!name || !out1 || !out2 || !s1data) return;
    int level = GetCurrentLevel(board);
    if (level == 0 || level > 15) { *(int*)out1 = 0; *(int*)out2 = 0; return; }
    void* ext = EnsureBoardExt(board);
    if (!ext) { *(int*)out1 = 0; *(int*)out2 = 0; return; }

    {
        char dbg[256];
        wsprintfA(dbg, "CreateDynamicObjects: level=%d name='%s' board=0x%08X s1data=0x%08X", level, name, (DWORD)board, (DWORD)s1data);
        DebugLog(dbg);
    }

    DWORD app = *(DWORD *)((char *)board + BOARD_APP_PTR);
    int difficulty = (app && !IsBadReadPtr((void*)app, 0x500)) ? *(int *)(app + APP_DIFFICULTY) : 0;
    int meshWorld = *(int *)((char *)board + BOARD_MESHWORLD);

    /* Extract position from S1 data: +4=X, +8=Y, +0xC=Z, +0x10=X2, +0x14=Y2, +0x18=Z2 */
    float x = *(float *)(s1data + 1);
    float y = *(float *)(s1data + 2);
    float z = *(float *)(s1data + 3);
    float x2 = *(float *)(s1data + 4);
    float y2 = *(float *)(s1data + 5);
    float z2 = *(float *)(s1data + 6);
    float fparam = *(float *)(s1data + 5);  /* same as y2 for some ctors */

    void *obj = NULL;
    int renderOut = 0;

    /* ── TIPPER (Dizzy) ── */
    // S1 ensure for swapped files

    if (my_strnicmp(name, "TIPPER", 6) == 0 && difficulty != 0) {
        S1EnsureMeshWorld(board, ext, UNI_TIPPER_MESH, "Levels\\Level3-Tipper");
        S1EnsureRender(board, ext, UNI_TIPPER_RENDER, UNI_TIPPER_MESH);
        int meshOff = UNI_TIPPER_MESH;
        int renderOff = UNI_TIPPER_RENDER;
        int meshVal = *(int*)((char*)ext + meshOff);
        if (!meshVal) { DebugLog("TIPPER: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
        {
            char dbg[256];
            wsprintfA(dbg, "TIPPER: meshOff=0x%X meshVal=0x%X renderOff=0x%X renderVal=0x%X level=%d rot=%.1f,%.1f,%.1f",
                      meshOff, meshVal, renderOff, *(int*)((char*)ext + renderOff), level, x2,y2,z2);
            DebugLog(dbg);
        }
        void *mem = g_operatorNew(0x1104);
        if (mem) {
            obj = g_TipperCtor(mem, (int)board, meshVal);
            DWORD *o = (DWORD *)obj;
            memcpy(&o[0x436], &x, 4); memcpy(&o[0x437], &y, 4); memcpy(&o[0x438], &z, 4);
            memcpy(&o[0x439], &x2, 4); memcpy(&o[0x43A], &y2, 4); memcpy(&o[0x43B], &z2, 4);
            if (ext) {
                *(float*)((char*)ext + UNI_TIPPER_ROT_X) = x2;
                *(float*)((char*)ext + UNI_TIPPER_ROT_Y) = y2;
                *(float*)((char*)ext + UNI_TIPPER_ROT_Z) = z2;
            }
            void *vmem = g_operatorNew(0x10D0);
            if (vmem) {
                void *vis = g_TipperVisualCtor(vmem, *(int*)((char*)ext + renderOff));
                o[0x435] = (DWORD)vis;
                g_TipperVisualAttach(vis, (void*)obj);
            }
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
        }
        OrBoardFeat(board, FEAT_SWIRL);
        *(int*)out1 = (int)obj; *(int*)out2 = (int)renderOut;
        return;
    }


    /* ── WATERWHEEL (Dizzy) ── */
    if (my_strnicmp(name, "WATERWHEEL", 10) == 0) {
        S1EnsureMeshWorld(board, ext, UNI_MESH_0, "Levels\\Level3-WaterWheel");
        S1EnsureRender(board, ext, UNI_MESH_1, UNI_MESH_0);
        // Canonical UNI_* inside ext (OFF_* aliases same).
        *(float *)((char *)ext + UNI_WHEELEMBED_X) = x;
        *(float *)((char *)ext + UNI_WHEELEMBED_Y) = y;
        *(float *)((char *)ext + UNI_WHEELEMBED_Z) = z;
        *(float*)((char*)ext + UNI_WATER_ROT_X) = x2;
        *(float*)((char*)ext + UNI_WATER_ROT_Y) = y2;
        *(float*)((char*)ext + UNI_WATER_ROT_Z) = z2;
        if (!obj) { obj = *(void **)((char *)ext + UNI_MESH_0); renderOut = *(int *)((char *)ext + UNI_MESH_1); }
        *(DWORD *)((char *)ext + UNI_JUDGE_LIST) = 0;
        OrBoardFeat(board, FEAT_SWIRL);
        {
            char dbg[256];
            wsprintfA(dbg, "WATERWHEEL: pos=%.1f,%.1f,%.1f rot=%.1f,%.1f,%.1f feat SWIRL auto-enabled", x,y,z,x2,y2,z2);
            DebugLog(dbg);
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── SWIRL (Dizzy) ── */
    if (my_strnicmp(name, "SWIRL", 5) == 0) {
        S1EnsureMeshWorld(board, ext, UNI_MESH_6, "Levels\\Level3-Swirl");
        S1EnsureRender(board, ext, UNI_MESH_7, UNI_MESH_6);
        /* Original: obj = board+0x4BC4 (Swirl mesh), renderOut = board+0x4BC8 (render obj)
         * Mod: Swirl mesh at UNI_MESH_6 (0x85F8), render at UNI_MESH_7 (0x85FC) */
        obj = *(void **)((char *)ext + UNI_MESH_6);
        renderOut = *(int *)((char *)ext + UNI_MESH_7);
        /* Store swirl position for step3b mesh rotation */
        *(float *)((char *)ext + UNI_MESH_15) = x;
        *(float *)((char *)ext + UNI_MESH_12) = y;
        *(float *)((char *)ext + UNI_MESH_13) = z;
        OrBoardFeat(board, FEAT_SWIRL);
        // Mirror for legacy OFF_* readers (alias, same addr) — use outer ext, no shadow.
        if (ext) { *(void**)((char*)ext+OFF_SWIRL_MESH)=obj; *(int*)((char*)ext+OFF_SWIRL_RENDER)=renderOut; *(float*)((char*)ext+OFF_SWIRL_POS_X)=x; *(float*)((char*)ext+OFF_SWIRL_POS_Y)=y; *(float*)((char*)ext+OFF_SWIRL_POS_Z)=z; }
        // Create swirl collision zone for Feature_SwirlZones proximity check
        {
            void* zoneMem = g_operatorNew(0x1110);
            if (zoneMem) {
                memset(zoneMem, 0, 0x1110);
                *(float*)((char*)zoneMem + 0x10E0) = x;
                *(float*)((char*)zoneMem + 0x10E4) = y;
                *(float*)((char*)zoneMem + 0x10E8) = z;
                *(float*)((char*)zoneMem + 0x1100) = 2.5f; // radius factor -> 150 units (*60)
                if (ext) g_AthenaListAppend((void*)((char*)ext + UNI_SWIRL_LIST), (int)zoneMem);
                else g_AthenaListAppend((void*)((char*)board + UNI_SWIRL_LIST), (int)zoneMem);
            }
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── GLUEBIE (Dizzy) ── */
    if (my_strnicmp(name, "GLUEBIE", 7) == 0) {
        S1EnsureMeshWorld(board, ext, UNI_GLUEBIE_MESH, "Levels\\Level3-Gluebie");
        if (difficulty == 0) { *(int*)out1 = 0; *(int*)out2 = 0; return; }
        int meshOff = UNI_GLUEBIE_MESH;
        int meshVal = *(int*)((char*)ext + meshOff);
        if (!meshVal) { DebugLog("GLUEBIE: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
        void *mem = g_operatorNew(0x110C);
        if (mem) {
            obj = g_GluebieCtor(mem, (int)board, meshVal);
            DWORD *o = (DWORD *)obj;
            memcpy(&o[0x435], &x, 4); memcpy(&o[0x436], &y, 4); memcpy(&o[0x437], &z, 4);
            /* Original appends to board+0x4378 (Gluebie list) + board+0x2578 (obj list).
             * UNI_MESH_3 (0x85EC) is a mesh SLOT, not an AthenaList — appending to it
             * clobbers mesh pointers at 0x8620-0x862C via the iter array. */
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            /* Restore missing Gluebie list append (board+0x4378) — original does both */
            g_AthenaListAppend((void*)((char*)board + 0x4378), (int)obj);
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── CATAPULT (Tower) ── */
    if (my_strnicmp(name, "CATAPULT", 8) == 0) {
        S1EnsureMeshWorld(board, ext, UNI_CATAPULT_MESH, "Levels\\Level4-Catapult");
        int meshOff = UNI_CATAPULT_MESH;
        int meshVal = *(int*)((char*)ext + meshOff);
        if (!meshVal) { DebugLog("CATAPULT: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
        void *mem = g_operatorNew(0x1108);
        if (mem) {
            obj = g_CatapultCtor(mem, (int)board, meshVal);
            DWORD *o = (DWORD *)obj;
            memcpy(&o[0x436], &x, 4); memcpy(&o[0x437], &y, 4); memcpy(&o[0x438], &z, 4);
            int listOff = UNI_CATAPULT_LIST;
            g_AthenaListAppend((void*)((char*)ext + listOff), (int)obj);
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            renderOut = o[0x435];
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── MACE (Tower) ── */
    if (my_strnicmp(name, "MACE", 4) == 0 && difficulty != 0) {
        S1EnsureMeshWorld(board, ext, UNI_MACE_MESH, "Levels\\Level4-Mace");
        int meshVal = *(int*)((char*)ext + UNI_MACE_MESH);
        if (!meshVal) { DebugLog("MACE: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
        void *mem = g_operatorNew(0x110C);
        if (mem) {
            obj = g_MaceCtor(mem, (int)board, meshVal);
            DWORD *o = (DWORD *)obj;
            memcpy(&o[0x436], &x, 4); memcpy(&o[0x437], &y, 4); memcpy(&o[0x438], &z, 4);
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            g_AthenaListAppend((void*)((char*)ext + UNI_MACE_LIST), (int)obj);
            if (g_AthenaListGetSize((void*)((char*)ext + UNI_MACE_LIST)) == 1) {
                o[0x43A] = 0x42A00000; o[0x43D] = 1; o[0x43E] = 0x32;
            }
            renderOut = o[0x435];
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── DRAWBRIDGE (Tower) ── */
    if (my_strnicmp(name, "DRAWBRIDGE", 10) == 0) {
        S1EnsureMeshWorld(board, ext, UNI_DRAWBRIDGE_MESH, "Levels\\Level4-Drawbridge");
        int meshVal = *(int*)((char*)ext + UNI_DRAWBRIDGE_MESH);
        if (!meshVal) { DebugLog("DRAWBRIDGE: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
        void *mem = g_operatorNew(0x113C);
        if (mem) {
            obj = g_GlassLevelCtor(mem, (int)board, meshVal);
            DWORD *o = (DWORD *)obj;
            memcpy(&o[0x436], &x, 4); memcpy(&o[0x437], &y, 4); memcpy(&o[0x438], &z, 4);
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            g_AthenaListAppend((void*)((char*)ext + UNI_DRAWBRIDGE_LIST), (int)obj);
            renderOut = o[0x435];
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── WINDMILL (Tower) ── */
    if (my_strnicmp(name, "WINDMILL", 8) == 0) {
        S1EnsureMeshWorld(board, ext, UNI_WINDMILL_MESH, "Levels\\Level4-Windmill");
        int mesh = *(int *)((char *)ext + UNI_WINDMILL_MESH);
        if (!mesh) { DebugLog("WINDMILL: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
        void *mem = g_operatorNew(0x10D0);
        if (mem) {
            void *render = g_LevelRenderCtor(mem, (void*)mesh);
            g_TipperVisualAttach(render, (void*)mesh);
            renderOut = (int)render;
            /* Store render obj in dedicated render offset for REND_WINDMILL */
            *(DWORD *)((char *)ext + REND_TOWER_WINDMILL) = (DWORD)render;
        }
        *(float *)((char *)ext + UNI_WINDMILL_X) = x;
        *(float *)((char *)ext + UNI_WINDMILL_Y) = y;
        *(float *)((char *)ext + UNI_WINDMILL_Z) = z;
        if (g_RNG) *(float *)((char *)ext + UNI_WINDMILL_ANGLE) = (float)RNG_call((void*)0x4F7360, 0, 0x168, 0);
        *(int*)out1 = mesh; *(int*)out2 = renderOut;
        return;
    }

    /* ── TRAPDOOR (Tower, level!=13) ── — Sky TRAPDOOR (level 13) uses Rotator POPDOOR path below */
    if (my_strnicmp(name, "TRAPDOOR", 8) == 0 && GetCurrentLevel(board) != 13) {
        void *mem = g_operatorNew(0x10F8);
        if (mem) {
            obj = g_TrapdoorCtor(mem, (int)board);
            DWORD *o = (DWORD *)obj;
            memcpy(&o[0x438], &x, 4); memcpy(&o[0x439], &y, 4); memcpy(&o[0x43A], &z, 4);
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            g_AthenaListAppend((void*)((char*)ext + UNI_TRAPDOOR_LIST), (int)obj);
            renderOut = o[0x435];
            g_AthenaListAppend((void*)((char*)board + UNI_TRAPDOOR_MESH_LIST), o[0x436]);
            g_AthenaListAppend((void*)((char*)board + UNI_TRAPDOOR_RENDER_LIST), o[0x437]);
            if (meshWorld) {
                int mw = *(int *)(meshWorld + 0x480);
                if (mw) g_AthenaListAppend((void*)(mw + 0x1C), o[0x436]);
                int ro = *(int *)((char *)board + BOARD_RENDEROBJ);
                if (ro) g_AthenaListAppend((void*)(ro + 0x18), o[0x437]);
            }
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── CHOMPER (Tower) ── */
    if (my_strnicmp(name, "CHOMPER", 7) == 0) {
        /* Load Chomper mesh if not already loaded */
        S1EnsureMeshNode(board, ext, UNI_CHOMPER_MESH, "Meshes\\Chomper");
        void *chomperMesh = *(void **)((char *)ext + UNI_CHOMPER_MESH);
        if (chomperMesh) *(DWORD *)((char *)ext + REND_TOWER_CHOMPER) = (DWORD)chomperMesh;
        /* Store position in dedicated tower render offsets */
        *(float *)((char *)ext + REND_TOWER_CHOMP_X) = x;
        *(float *)((char *)ext + REND_TOWER_CHOMP_Y) = y;
        *(float *)((char *)ext + REND_TOWER_CHOMP_Z) = z;
        /* Adjust Y by constant (original subtracts _DAT_004CF370) */
        float adj = *(float *)(g_moduleBase + 0xCF370);
        *(float *)((char *)ext + REND_TOWER_CHOMP_Y) -= adj;
        *(int*)out1 = 0; *(int*)out2 = 0;
        return;
    }

    /* ── TURRET tower (Tower) — not a gun, it's a tower ──
     * Original game (Tower_CreateDynamicObjects at 0x0040d7c0):
     *   1. operator_new(0x10D0) → Stands_ctor(mem, meshPtr)
     *   2. Timer_Init(stack local) — creates a Timer object
     *   3. Copy position (x,y,z) from S1 data to stack local struct
     *   4. Call TIMER vtable[2] (Gfx_SetPosition) with (x,y,z) as 3 float args
     *      Stands vtable[2] is SceneObject_BuildStrips — calling it hangs!
     *   5. Call STANDS vtable[0x15] (slot 21) with pointer to position struct
     *   6. Level_RenderCtor + TipperVisual_Attach
     *   7. Timer_Cleanup */
    if (my_strnicmp(name, "TURRET", 6) == 0) {
        S1EnsureMeshWorld(board, ext, UNI_TURRET_MESH, "Levels\\Level4-Turret");
        void *meshPtr = *(void **)((char *)ext + UNI_TURRET_MESH);
        if (!meshPtr) { *(int*)out1 = 0; *(int*)out2 = 0; return; }
        void *mem = g_operatorNew(0x10D0);
        if (mem) {
            int stands = (int)(DWORD)g_StandsCtor(mem, (int)(DWORD)meshPtr);
            char timerBuf[68];
            g_TimerInit(timerBuf);
            /* Copy position from S1 data to stack struct (matching original) */
            float pos[3];
            pos[0] = x; pos[1] = y; pos[2] = z;
            /* Call TIMER vtable[2] (Gfx_SetPosition) with 3 floats by value */
            DWORD *timerVtbl = *(DWORD **)timerBuf;
            if (timerVtbl) {
                void (__thiscall *timerSetPos)(void*, float, float, float) = (void (__thiscall *)(void*, float, float, float))timerVtbl[2];
                if (timerSetPos) timerSetPos(timerBuf, pos[0], pos[1], pos[2]);
            }
            /* Call STANDS vtable[0x15] (slot 21) with pointer to position struct */
            DWORD *standsVtbl = *(DWORD **)stands;
            if (standsVtbl) {
                void (__fastcall *fn54)(DWORD, float *) = (void (__fastcall *)(DWORD, float *))standsVtbl[0x15];
                if (fn54) fn54((DWORD)stands, pos);
            }
            void *rmem = g_operatorNew(0x10D0);
            if (rmem) {
                void *render = g_LevelRenderCtor(rmem, (void*)stands);
                g_TipperVisualAttach(render, (void*)stands);
                obj = (void*)stands;
                renderOut = (int)render;
                *(DWORD *)((char *)ext + REND_TOWER_TURRET) = (DWORD)render;
            }
            g_TimerCleanup(timerBuf);
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── BONK (Expert) ── */
    if (my_strnicmp(name, "BONK", 4) == 0 && difficulty != 0) {
        void *mem = g_operatorNew(0x1200);
        if (mem) {
            obj = g_BonkCtor(mem, (int)board, x, y, z);
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            DWORD *o = (DWORD *)obj;
            renderOut = o[0x43E];
            int storeOff = UNI_BONK_MESH;
            void* bExt = GetBoardExt(board); if(!bExt) bExt=EnsureBoardExt(board);
            if(bExt) *(void **)((char *)bExt + storeOff) = obj;
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── FAN (Expert) ── */
    if (my_strnicmp(name, "FAN", 3) == 0 && difficulty != 0) {
        void *mem = g_operatorNew(0x1188);
        if (mem) {
            obj = g_FanCtor(mem, (int)board, x, y, z, fparam);
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            if (strstr(name, "SLOW")) ((DWORD*)obj)[0x43B] = 1;
            if (strstr(name, "SUPER")) *(char*)((char*)obj + 0x10ED) = 1;
            if (strstr(name, "UP") && g_SoundInitChannels) g_SoundInitChannels(obj, 1);
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── SAWBLADE (Expert) ── */
    if (my_strnicmp(name, "SAWBLADE", 8) == 0 && difficulty != 0) {
        void *mem = g_operatorNew(0x111C);
        if (mem) {
            obj = g_SawBladeCtor(mem, (int)board, x, y, z);
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            if (strstr(name, "1")) { g_SawBladeSetVariant(obj, 1); *(void **)((char *)ext + UNI_SAWBLADE1_OBJ) = obj; }
            if (strstr(name, "2")) { g_SawBladeSetVariant(obj, 2); *(void **)((char *)ext + UNI_SAWBLADE2_OBJ) = obj; }
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── SAWBRIDGE (Expert cut-away spinner) — renamed from BRIDGE (level==8) ──
     * S1: SAWBRIDGE1 / SAWBRIDGE2 (+ NEG) — was BRIDGE1/2 on Expert
     * BRIDGE (drawbridge) is now Intermediate/Master only, level-gate removed */
    if (my_strnicmp(name, "SAWBRIDGE", 9) == 0) {
        S1EnsureMeshWorld(board, ext, UNI_BONK_STORE, "Levels\\\\Level2-Bridge");
        void* brMesh = *(void**)((char*)ext + UNI_BONK_STORE);
        void* brRender = *(void**)((char*)ext + UNI_SAW1_OBJ);
        if (!brRender && brMesh) {
            void* mem = g_operatorNew(0x10D0);
            if (mem) {
                void* robj = g_LevelRenderCtor(mem, brMesh);
                if (robj) {
                    g_TipperVisualAttach(robj, brMesh);
                    *(void**)((char*)ext + UNI_SAW1_OBJ) = robj;
                }
            }
        }
        /* Spinner_Level_ctor */
        void *mem = g_operatorNew(0x10FC);
        if (mem) {
            obj = g_SpinnerLevelCtor(mem, (int)board, x, y, z, fparam);
            DWORD *o = (DWORD *)obj;
            renderOut = o[0x43D];
            if (strstr(name, "1")) g_AthenaListAppend((void*)((char*)ext + UNI_LIST_1), (int)obj);
            if (strstr(name, "2")) g_AthenaListAppend((void*)((char*)ext + UNI_LIST_2), (int)obj);
            if (strstr(name, "NEG")) o[0x43E] = 0xBF800000;
        }
        OrBoardFeat(board, FEAT_BRIDGE_ANIM);
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── BRIDGE (Intermediate/Master drawbridge, position-only) ──
     * S1: BRIDGE / BRIDGE(NOCOLLIDE) — SAWBRIDGE is Expert spinner, handled above
     * Vanilla level5 has S1 name "BRIDGE" but expects Spinner_Level_ctor;
     * Intermediate BRIDGE is pos-only. Dispatch by level to handle both files. */
    if (my_strnicmp(name, "BRIDGE", 6) == 0) {
        int _lvl = GetCurrentLevel(board);
        if (_lvl == 8 && my_strnicmp(name, "SAWBRIDGE", 9) != 0) {
            /* Expert vanilla "BRIDGE" — treat as SAWBRIDGE spinner */
            S1EnsureMeshWorld(board, ext, UNI_BONK_STORE, "Levels\\\\Level2-Bridge");
            void* brMesh2 = *(void**)((char*)ext + UNI_BONK_STORE);
            void* brRender2 = *(void**)((char*)ext + UNI_SAW1_OBJ);
            if (!brRender2 && brMesh2) {
                void* mem = g_operatorNew(0x10D0);
                if (mem) {
                    void* robj = g_LevelRenderCtor(mem, brMesh2);
                    if (robj) {
                        g_TipperVisualAttach(robj, brMesh2);
                        *(void**)((char*)ext + UNI_SAW1_OBJ) = robj;
                    }
                }
            }
            void *mem = g_operatorNew(0x10FC);
            if (mem) {
                obj = g_SpinnerLevelCtor(mem, (int)board, x, y, z, fparam);
                DWORD *o = (DWORD *)obj;
                renderOut = o[0x43D];
                /* Vanilla BRIDGE has no 1/2 suffix — append to LIST_1 by default */
                if (strstr(name, "2")) g_AthenaListAppend((void*)((char*)ext + UNI_LIST_2), (int)obj);
                else g_AthenaListAppend((void*)((char*)ext + UNI_LIST_1), (int)obj);
                if (strstr(name, "NEG")) o[0x43E] = 0xBF800000;
            }
            OrBoardFeat(board, FEAT_BRIDGE_ANIM);
            *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
            return;
        }
        S1EnsureMeshWorld(board, ext, UNI_BONK_STORE, "Levels\\\\Level2-Bridge");
        void* brMesh = *(void**)((char*)ext + UNI_BONK_STORE);
        void* brRender = *(void**)((char*)ext + UNI_SAW1_OBJ);
        if (!brRender && brMesh) {
            void* mem = g_operatorNew(0x10D0);
            if (mem) {
                void* robj = g_LevelRenderCtor(mem, brMesh);
                if (robj) {
                    g_TipperVisualAttach(robj, brMesh);
                    *(void**)((char*)ext + UNI_SAW1_OBJ) = robj;
                }
            }
        }
        /* Intermediate/Master: position only — store pivot, no level check */
        obj = *(void **)((char *)ext + UNI_BONK_STORE);
        *(float *)((char *)ext + BRD_BRIDGE_PIVOT_X) = x;
        *(float *)((char *)ext + BRD_BRIDGE_PIVOT_Y) = y;
        *(float *)((char *)ext + BRD_BRIDGE_PIVOT_Z) = z;
        if (!strstr(name, "(NOCOLLIDE)"))
            renderOut = *(int *)((char *)ext + UNI_SAW1_OBJ);
        OrBoardFeat(board, FEAT_BRIDGE_ANIM);
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── JUDGE (Expert) ── */
    if (my_strnicmp(name, "JUDGE", 5) == 0) {
        void *mem = g_operatorNew(0x1100);
        if (mem) {
            obj = g_GearLevelCtor(mem, (int)board, x, y, z);
            g_AthenaListAppend((void*)((char*)ext + UNI_JUDGE_LIST), (int)obj);
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── BELL (Expert) ── */
    if (my_strnicmp(name, "BELL", 4) == 0) {
        void *mem = g_operatorNew(0x10E8);
        if (mem) {
            obj = g_BellCtor(mem, (int)board, x, y, z);
            *(void **)((char *)ext + UNI_BELL_OBJ) = obj;
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── DROPPER (Odd single lifer) — renamed from LIFTER (level==9) ──
     * S1: DROPPER — was bare LIFTER on Odd */
    if (my_strnicmp(name, "DROPPER", 7) == 0) {
        void *mem = g_operatorNew(0x10FC);
        if (mem) {
            obj = g_OddLifterCtor(mem, (int)board, x, y, z);
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            *(void **)((char *)ext + UNI_BONK_STORE) = obj;
            renderOut = ((DWORD*)obj)[0x435];
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── LIFTER (Odd vanilla alias) ── — vanilla Odd S1 is bare LIFTER -> Odd_Lifter_ctor when level==9 */
    if (my_strnicmp(name, "LIFTER", 6) == 0 && GetCurrentLevel(board) == 9 && my_strnicmp(name, "DROPPER", 7) != 0) {
        void *mem = g_operatorNew(0x10FC);
        if (mem) {
            obj = g_OddLifterCtor(mem, (int)board, x, y, z);
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            *(void **)((char *)ext + UNI_BONK_STORE) = obj;
            renderOut = ((DWORD*)obj)[0x435];
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── LIFTER (Up tubes) ──
     * S1: LIFTER2, LIFTER3... — Up only, level-gate removed (name-driven) */
    if (my_strnicmp(name, "LIFTER", 6) == 0) {
        S1EnsureMeshWorld(board, ext, UNI_LIFTER_MESH, "levels\\\\levelup-lifter");
        long num = atol(name + 6);
        int meshVal = *(int*)((char*)ext + UNI_LIFTER_MESH);
        if (!meshVal) { DebugLog("LIFTER: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
        void *mem = g_operatorNew(0x10F4);
        if (mem) {
            obj = g_LifterCtor(mem, (int)board, x, y, z, meshVal, num);
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            /* Dual-append to legacy board+0x436C for Up's RaceState handler.
             * Name-driven now — any LIFTER in any slot needs the list if meshed as Up.
             * Keep unconditional (Up RaceState iterates 0x436C to call Lifter_Update). */
            g_AthenaListAppend((void*)((char*)board + 0x436C), (int)obj);
            renderOut = ((DWORD*)obj)[0x438];
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── SPINNY (Toob) ── */
    if (my_strnicmp(name, "SPINNY", 6) == 0) {
        S1EnsureMeshWorld(board, ext, UNI_SPINNY_MESH, "Levels\\\\Level8-Spinny");
        int meshVal = *(int*)((char*)ext + UNI_SPINNY_MESH);
        if (!meshVal) { DebugLog("SPINNY: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
        void *mem = g_operatorNew(0x1508);
        if (mem) {
            obj = g_RotatorImpossibleCtor(mem, (int)board, x, y, z, meshVal);
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            renderOut = ((DWORD*)obj)[0x435];
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── SAW (Toob) ── */
    if (my_stricmp(name, "SAW") == 0 && difficulty != 0) {
        int pathObj = g_LevelFindObjectByName(meshWorld, "SAWPATH");
        S1EnsureMeshWorld(board, ext, UNI_SAW_MESH, "Levels\\\\Level8-Saw");
        int meshVal = *(int*)((char*)ext + UNI_SAW_MESH);
        if (!meshVal) { DebugLog("SAW: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
        void *mem = g_operatorNew(0x1110);
        if (mem) {
            obj = g_SawCtor(mem, (int)board, x, y, z, meshVal, pathObj);
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            *(void **)((char *)ext + UNI_SAW_TOOB_OBJ) = obj;
            renderOut = ((DWORD*)obj)[0x435];
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── SAW2 (Toob) ── */
    if (my_stricmp(name, "SAW2") == 0 && difficulty != 0) {
        int pathObj = g_LevelFindObjectByName(meshWorld, "SMALLSAWPATH");
        S1EnsureMeshWorld(board, ext, UNI_SAW_MESH, "Levels\\\\Level8-Saw");
        int meshVal = *(int*)((char*)ext + UNI_SAW_MESH);
        if (!meshVal) { DebugLog("SAW2: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
        void *mem = g_operatorNew(0x1118);
        if (mem) {
            obj = g_Saw2Ctor(mem, (int)board, x, y, z, meshVal, pathObj);
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            *(void **)((char *)ext + UNI_SAW2_TOOB_OBJ) = obj;
            renderOut = ((DWORD*)obj)[0x435];
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── FALLOUT1 (Toob) ── */
    if (my_strnicmp(name, "FALLOUT1", 8) == 0) {
        S1EnsureMeshWorld(board, ext, UNI_FALLOUT_MESH, "Levels\\\\Level8-Fallout");
        int meshVal = *(int*)((char*)ext + UNI_FALLOUT_MESH);
        if (!meshVal) { DebugLog("FALLOUT1: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
        void *mem = g_operatorNew(0x10E8);
        if (mem) {
            obj = g_FalloutCtor(mem, (int)board, x, y, z, meshVal);
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            *(void **)((char *)ext + UNI_FALLOUT_OBJ) = obj;
            renderOut = ((DWORD*)obj)[0x435];
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── BLOCKDAWG1/2/3 (Toob) ── */
    if (my_strnicmp(name, "BLOCKDAWG", 9) == 0 && difficulty != 0) {
        int dawgNum = name[9] - '0';
        int meshOff = (dawgNum==1) ? UNI_BLOCKDAWG1_MESH : (dawgNum==3) ? UNI_BLOCKDAWG3_MESH : UNI_BLOCKDAWG2_MESH;
        { const char *_bdPath = (dawgNum==1) ? "Levels\\\\Level8-Blockdawg1" : (dawgNum==3) ? "Levels\\\\Level8-Blockdawg3" : "Levels\\\\Level8-Blockdawg2"; S1EnsureMeshWorld(board, ext, meshOff, _bdPath); }
        void* bExt2 = GetBoardExt(board); if (!bExt2) bExt2 = ext;
        int meshVal = bExt2 ? *(int*)((char*)bExt2 + meshOff) : *(int*)((char*)ext + meshOff);
        if (!meshVal) { DebugLog("BLOCKDAWG: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
        char pathName[] = "DAWGPATH0";
        pathName[8] = '0' + dawgNum;
        int pathObj = g_LevelFindObjectByName(meshWorld, pathName);
        void *mem = g_operatorNew(0x1154);
        if (mem) {
            obj = g_BlockdawgCtor(mem, (int)board, x, y, z, meshVal, pathObj);
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            renderOut = ((DWORD*)obj)[0x435];
            if (dawgNum == 3) *(char*)((char*)obj + 0x1152) = 1;
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── WOBBLY1-7 (Wobbly) ── */
    if (my_strnicmp(name, "WOBBLY", 6) == 0 && name[6] >= '1' && name[6] <= '7') {
        int wNum = name[6] - '0';
        int meshOff = UNI_WOBBLY_BASE + (wNum-1) * 4;
        { const char *_wPath = (wNum==1) ? "Levels\\\\Level7-Wobbly1" : (wNum==2) ? "Levels\\\\Level7-Wobbly2" : (wNum==3) ? "Levels\\\\Level7-Wobbly3" : (wNum==4) ? "Levels\\\\Level7-Wobbly4" : (wNum==5) ? "Levels\\\\Level7-Wobbly5" : (wNum==6) ? "Levels\\\\Level7-Wobbly6" : "Levels\\\\Level7-Wobbly7"; S1EnsureMeshWorld(board, ext, meshOff, _wPath); }
        void* wExt = GetBoardExt(board); if (!wExt) wExt = ext;
        int meshVal = wExt ? *(int*)((char*)wExt + meshOff) : *(int*)((char*)ext + meshOff);
        if (!meshVal) { DebugLog("WOBBLY: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
        void *mem = g_operatorNew(0x1524);
        if (mem) {
            obj = g_GameLevelCtor(mem, (int)board, x, y, z, meshVal);
            DWORD *o = (DWORD *)obj;
            renderOut = o[0x435];
            /* Per-wobbly constants */
            static const DWORD w43a[] = {0,0,0x41000000,0,0x41700000,0x41700000,0x41200000,0x41A00000};
            static const DWORD w43b[] = {0,0,0,0x41C80000,0,0,0x41200000,0};
            static const DWORD w440[] = {0,0,0x42C80000,0x43160000,0x432F0000,0x431B0000,0x43160000,0x42C80000};
            static const int w1105[] = {0,0,1,0,1,1,0,1};
            o[0x43A] = w43a[wNum]; o[0x43B] = w43b[wNum]; o[0x440] = w440[wNum];
            if (w1105[wNum]) *(char*)((char*)obj + 0x1105) = 1;
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── WAVY1 (Wobbly) ── */
    if (my_strnicmp(name, "WAVY1", 5) == 0) {
        int gfx = *(int *)(app + 0x174);
        if (gfx) *(char *)(gfx + 2000) = 1;
        void *mem = g_operatorNew(0x1AE7C);
        if (mem) {
            obj = g_WavyCtor(mem, (int)board, x, y, z, "Levels\\Level7-Wavy1");
            g_WavyConfigure(obj, 0x1C, 0x41A00000, 0x40000000, 0xC0400000);
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            renderOut = ((DWORD*)obj)[0x435];
        }
        if (gfx) *(char *)(gfx + 2000) = 0;
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── NEONPLATFORM (Neon) ── */
    if (my_strnicmp(name, "NEONPLATFORM", 12) == 0) {
        S1EnsureMeshWorld(board, ext, UNI_SAW2_OBJ, "Levels\\\\LevelDark-NeonPlatform");
        int meshVal = *(int*)((char*)ext + UNI_SAW2_OBJ);
        if (!meshVal) { DebugLog("NEONPLATFORM: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
        void *mem = g_operatorNew(0x10EC);
        if (mem) {
            obj = g_NeonPlatformCtor(mem, (int)board, x, y, z, meshVal);
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            renderOut = ((DWORD*)obj)[0x435];
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── DFLOOR1-4 (Neon) ── */
    if (my_strnicmp(name, "DFLOOR", 6) == 0 && name[6] >= '1' && name[6] <= '4') {
        int dNum = name[6] - '0';
        int meshOff = UNI_DFLOOR_BASE + (dNum-1) * 4;
        { const char *_dfPath = (dNum==1) ? "Levels\\\\LevelDark-DFloor1" : (dNum==2) ? "Levels\\\\LevelDark-DFloor2" : (dNum==3) ? "Levels\\\\LevelDark-DFloor3" : "Levels\\\\LevelDark-DFloor4"; S1EnsureMeshWorld(board, ext, meshOff, _dfPath); }
        void *dExt = GetBoardExt(board); if (!dExt) dExt = ext;
        int meshVal = dExt ? *(int*)((char*)dExt + meshOff) : *(int*)((char*)ext + meshOff);
        if (!meshVal) { DebugLog("DFLOOR: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
        void *mem = g_operatorNew(0x1104);
        if (mem) {
            obj = g_ArenaStandsCtor(mem, (int)board, x, y, z, meshVal);
            DWORD *o = (DWORD *)obj;
            renderOut = o[0x43A];
            if (dNum == 4) {
                *(void **)((char *)ext + UNI_NEON_DARK_COUNT) = obj;
                o[0x437] = 2;
                if (obj) {
                    *(DWORD *)((char *)obj + 0x10E0) = 0;
                }
            } else {
                g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            }
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── TRODE (Neon) ── */
    if (my_strnicmp(name, "TRODE", 5) == 0) {
        S1EnsureMeshWorld(board, ext, UNI_BRIDGE_COUNTER, "Levels\\\\LevelDark-Trode");
        int meshVal = *(int*)((char*)ext + UNI_BRIDGE_COUNTER);
        if (!meshVal) { DebugLog("TRODE: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
        void *mem = g_operatorNew(0x1104);
        if (mem) {
            obj = g_ArenaStandsCtor(mem, (int)board, x, y, z, meshVal);
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            renderOut = ((DWORD*)obj)[0x43A];
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── POPCYLINDER (Sky) ── */
    if (my_strnicmp(name, "POPCYLINDER", 11) == 0) {
        if (difficulty != 0) {
            /* Sky */
            long idx = atol(name + 11) - 1;
            if (idx >= 0 && idx < 16) {
                int meshIdx = idx & 1;
                int meshOff = UNI_POPCYL_MESH_BASE + meshIdx * 4;
                { const char *_pcPath = (meshIdx==0) ? "levels\\\\level9-popcylinder1" : "levels\\\\level9-popcylinder2"; S1EnsureMeshWorld(board, ext, meshOff, _pcPath); }
                void* pExt = GetBoardExt(board); if (!pExt) pExt = ext;
                int meshVal = pExt ? *(int*)((char*)pExt + meshOff) : *(int*)((char*)ext + meshOff);
                if (!meshVal) { DebugLog("POPCYLINDER: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
                void *mem = g_operatorNew(0x10F4);
                if (mem) {
                    obj = g_PopcylinderCtor(mem, (int)board, x, y, z, meshVal);
                    *(void **)((char *)ext + UNI_SKY_POPCYL_BASE + idx * 4) = obj;
                    g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
                    renderOut = ((DWORD*)obj)[0x438];
                }
            }
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── POPDOOR/TRAPDOOR (Sky Rotator, level==13) ──
     * S1 vanilla name is TRAPDOOR (Sky_CreateDynamicObjects 0x410AD0 line 79); POPDOOR accepted for new files */
    if ((my_strnicmp(name, "POPDOOR", 7) == 0) || (my_strnicmp(name, "TRAPDOOR", 8) == 0 && GetCurrentLevel(board) == 13)) {
        float dat = *(float *)(g_moduleBase + 0xCF44C);
        S1EnsureMeshWorld(board, ext, UNI_SKY_TRAPDOOR, "levels\\\\level9-trapdoor");
        int meshVal = *(int*)((char*)ext + UNI_SKY_TRAPDOOR);
        if (!meshVal) { DebugLog("POPDOOR: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
        void *mem = g_operatorNew(0x10F4);
        if (mem) {
            obj = g_RotatorCtor(mem, (int)board, x, y, z, dat - fparam, meshVal);
            *(void **)((char *)ext + UNI_NEON_TRAPDOOR) = obj;
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            g_AthenaListAppend((void*)((char*)ext + UNI_TRAPDOOR_LIST), (int)obj);
            renderOut = ((DWORD*)obj)[0x43C];
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── SPEEDCYLINDER (Up) ── */
    if (my_strnicmp(name, "SPEEDCYLINDER", 13) == 0) {
        S1EnsureMeshWorld(board, ext, UNI_SPEEDCYLINDER_MESH, "levels\\\\levelup-speedcylinder");
        int meshVal = *(int*)((char*)ext + UNI_SPEEDCYLINDER_MESH);
        if (!meshVal) { DebugLog("SPEEDCYLINDER: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
        void *mem = g_operatorNew(0x150C);
        if (mem) {
            /* numArg from __ftol2 — use 0 as fallback */
            obj = g_SpeedCylinderCtor(mem, (int)board, x, y, z, 0, meshVal);
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            renderOut = ((DWORD*)obj)[0x438];
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── TIMEBUTTON (Up) ── */
    if (my_strnicmp(name, "TIMEBUTTON", 10) == 0) {
        S1EnsureMeshWorld(board, ext, UNI_TIMEBUTTON_MESH, "levels\\\\levelup-button");
        int meshVal = *(int*)((char*)ext + UNI_TIMEBUTTON_MESH);
        if (!meshVal) { DebugLog("TIMEBUTTON: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
        void *mem = g_operatorNew(0x10E8);
        if (mem) {
            obj = g_TimeButtonCtor(mem, (int)board, x, y, z, meshVal);
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            renderOut = ((DWORD*)obj)[0x438];
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── LOOPER (Impossible) ── */
    if (my_strnicmp(name, "LOOPER", 6) == 0) {
        S1EnsureMeshWorld(board, ext, UNI_LOOPER_MESH, "Levels\\\\LevelImpossible-Looper");
        int meshVal = *(int*)((char*)ext + UNI_LOOPER_MESH);
        if (!meshVal) { DebugLog("LOOPER: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
        void *mem = g_operatorNew(0x1500);
        if (mem) {
            obj = g_LooperCtor(mem, (int)board, x, y, z, meshVal);
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            renderOut = ((DWORD*)obj)[0x435];
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── GEAR (Impossible) ── */
    if (my_strnicmp(name, "GEAR", 4) == 0 && my_strnicmp(name, "BIGGEAR", 7) != 0) {
        S1EnsureMeshWorld(board, ext, UNI_GEAR_MESH, "Levels\\\\LevelImpossible-Gear");
        int meshVal = *(int*)((char*)ext + UNI_GEAR_MESH);
        if (!meshVal) { DebugLog("GEAR: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
        void *mem = g_operatorNew(0x1514);
        if (mem) {
            obj = g_GearCtor(mem, (int)board, x, y, z, x2, y2, z2, meshVal);
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            renderOut = ((DWORD*)obj)[0x435];
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── BIGGEAR (Impossible) ── */
    if (my_strnicmp(name, "BIGGEAR", 7) == 0) {
        S1EnsureMeshWorld(board, ext, UNI_BIGGEAR_MESH, "Levels\\\\LevelImpossible-BigGear");
        int meshVal = *(int*)((char*)ext + UNI_BIGGEAR_MESH);
        if (!meshVal) { DebugLog("BIGGEAR: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
        void *mem = g_operatorNew(0x1514);
        if (mem) {
            obj = g_GearCtor(mem, (int)board, x, y, z, x2, y2, z2, meshVal);
            DWORD *o = (DWORD *)obj;
            o[0x43D] = 0x3F000000;
            if (strstr(name, "TOUCH")) *(char*)((char*)obj + 0x544) = 1;
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            renderOut = o[0x435];
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── ROTATOR (Impossible) ── */
    if (my_strnicmp(name, "ROTATOR", 7) == 0) {
        S1EnsureMeshWorld(board, ext, UNI_ROTATOR_MESH, "Levels\\\\LevelImpossible-Rotator");
        int meshVal = *(int*)((char*)ext + UNI_ROTATOR_MESH);
        if (!meshVal) { DebugLog("ROTATOR: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
        void *mem = g_operatorNew(0x1508);
        if (mem) {
            obj = g_RotatorImpossibleCtor(mem, (int)board, x, y, z, meshVal);
            DWORD *o = (DWORD *)obj;
            o[0x43A] = 0x3F800000;
            if (g_RNG && RNG_call((void*)0x4F7360, 0, 2, 0) == 0)
                o[0x43A] = 0xBF800000;
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            renderOut = o[0x435];
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── PENDULUM (Impossible) ── */
    if (my_strnicmp(name, "PENDULUM", 8) == 0) {
        S1EnsureMeshWorld(board, ext, UNI_PENDULUM_MESH, "Levels\\\\LevelImpossible-Pendulum");
        int meshVal = *(int*)((char*)ext + UNI_PENDULUM_MESH);
        if (!meshVal) { DebugLog("PENDULUM: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
        void *mem = g_operatorNew(0x1504);
        if (mem) {
            obj = g_PendulumCtor(mem, (int)board, x, y, z, meshVal);
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            renderOut = ((DWORD*)obj)[0x435];
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── BBRIDGE1/2 (Master) ── */
    if (my_strnicmp(name, "BBRIDGE", 7) == 0) {
        int bNum = name[7] - '0';
        int meshOff = (bNum == 1) ? UNI_BBRIDGE1_MESH : UNI_BBRIDGE2_MESH;
        int storeOff = (bNum == 1) ? UNI_BBRIDGE1_OBJ : UNI_BBRIDGE2_OBJ;
        S1EnsureMeshWorld(board, ext, meshOff, (bNum == 1) ? "Levels\\\\Level10-Bridge1" : "Levels\\\\Level10-Bridge2");
        void* bbExt2 = GetBoardExt(board); if (!bbExt2) bbExt2 = ext;
        int meshVal = bbExt2 ? *(int*)((char*)bbExt2 + meshOff) : *(int*)((char*)ext + meshOff);
        if (!meshVal) { DebugLog("BBRIDGE: mesh pointer is NULL, skipping"); *(int*)out1 = 0; *(int*)out2 = 0; return; }
        void *mem = g_operatorNew(0x1100);
        if (mem) {
            obj = g_BreakBridgeCtor(mem, (int)board, x, y, z, meshVal);
            g_AthenaListAppend((void*)((char*)board + UNI_OBJ_LIST), (int)obj);
            void* bbExt = GetBoardExt(board); if(!bbExt) bbExt=ext;
            if(bbExt) *(void **)((char *)bbExt + storeOff) = obj;
            renderOut = ((DWORD*)obj)[0x438];
        }
        *(int*)out1 = (int)obj; *(int*)out2 = renderOut;
        return;
    }

    /* ── SMASHER1/2 (Glass) ──
     * Writes position floats to dedicated REND_GLASS_* offsets.
     * Original game stores 3 floats (X,Y,Z) at board+0x436C/0x4370/0x4374.
     * Render block passes them to Timer vtable[0x08](timer, X, Y, Z). */
    if (my_strnicmp(name, "SMASHER1", 8) == 0) {
        *(float *)((char *)ext + REND_GLASS_S1_X) = x;
        *(float *)((char *)ext + REND_GLASS_S1_Y) = y;
        *(float *)((char *)ext + REND_GLASS_S1_Z) = z;
        *(DWORD *)((char *)ext + REND_GLASS_TRANSP1) = 0;
        *(char *)((char *)ext + REND_GLASS_FLAG1) = 0;
        *(int*)out1 = 0; *(int*)out2 = 0;
        return;
    }
    if (my_strnicmp(name, "SMASHER2", 8) == 0) {
        *(float *)((char *)ext + REND_GLASS_S2_X) = x;
        *(float *)((char *)ext + REND_GLASS_S2_Y) = y;
        *(float *)((char *)ext + REND_GLASS_S2_Z) = z;
        *(DWORD *)((char *)ext + REND_GLASS_TRANSP2) = 0xC2B40000; /* -90.0f */
        *(char *)((char *)ext + REND_GLASS_FLAG2) = 0;
        *(int*)out1 = 0; *(int*)out2 = 0;
        return;
    }

    /* ── TARBUBBLE (Dizzy) ──
     * S1: TARBUBBLE — DizzyBoard_Update uses board+0x11E4 list.
     * Mod stores into ext+UNI_TARBUBBLE_LIST (0x9430) so Feature_SwirlZones can spawn bubbles. */
    if (my_strnicmp(name, "TARBUBBLE", 9) == 0) {
        void *entry = g_operatorNew(0x20);
        if (entry) {
            memset(entry, 0, 0x20);
            *(float *)((char *)entry + 0x04) = x;
            *(float *)((char *)entry + 0x08) = y;
            *(float *)((char *)entry + 0x0C) = z;
            g_AthenaListAppend((void *)((char *)ext + UNI_TARBUBBLE_LIST), (int)entry);
        }
        *(int*)out1 = 0; *(int*)out2 = 0;
        return;
    }

    /* Unknown object — no-op */
    *(int*)out1 = 0; *(int*)out2 = 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Universal DispatchCollision (Slot 29) — merges all 15 per-level handlers
 *
 * Instead of delegating to per-level handlers, this function implements ALL
 * collision event checks from all 15 levels in one place. Each level-specific
 * event is gated by IsCollisionEventEnabled() which reads from the [COLLISION]
 * section of LevelFeatures.txt. At the end, it calls DispatchCollisionEvents
 * (the global handler) which handles shared events (N:GOAL, N:TARPIT, etc.).
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Collision event config table — maps event name to per-level enabled flags.
 * Populated from [COLLISION] section of LevelFeatures.txt. */
#define MAX_COLLISION_EVENTS 64
typedef struct {
    char name[32];
    int enabled[16]; /* enabled[level] = 1 if active */
} CollisionEventEntry;

static CollisionEventEntry g_collisionEvents[MAX_COLLISION_EVENTS];
static int g_collisionEventCount = 0;

/* Default collision event assignments (matching original game behavior).
 * These are set on startup, then overridden by [COLLISION] section if present. */
/* Parse [COLLISION] section from LevelFeatures.txt */
static void LoadCollisionConfig(char *buf, DWORD bufSize) {
    char *line = buf;
    int inCollisionSection = 0;
    while (line < buf + bufSize) {
        char *eol = line;
        while (*eol && *eol != '\n' && *eol != '\r') eol++;
        char saved = *eol;
        *eol = '\0';
        char *p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\0' || *p == '#' || *p == ';') goto next_coll_line;
        if (p[0] == '[') {
            inCollisionSection = (my_strnicmp(p, "[COLLISION", 10) == 0);
            goto next_coll_line;
        }
        if (inCollisionSection) {
            /* Parse: EVENT_NAME = 1 3 5 */
            char *eq = p;
            while (*eq && *eq != '=') eq++;
            if (*eq == '=') {
                *eq = '\0';
                char *name = p;
                char *val = eq + 1;
                /* trim name */
                while (*name == ' ' || *name == '\t') name++;
                int nl = strlen(name);
                while (nl > 0 && (name[nl-1] == ' ' || name[nl-1] == '\t')) name[--nl] = '\0';
                /* find or create entry */
                int idx = -1, i;
                for (i = 0; i < g_collisionEventCount; i++) {
                    if (my_stricmp(g_collisionEvents[i].name, name) == 0) {
                        idx = i; break;
                    }
                }
                if (idx < 0 && g_collisionEventCount < MAX_COLLISION_EVENTS) {
                    idx = g_collisionEventCount++;
                    my_strncpy(g_collisionEvents[idx].name, name, 32);
                    memset(g_collisionEvents[idx].enabled, 0, sizeof(int)*16);
                }
                if (idx >= 0) {
                    /* clear existing, parse level numbers */
                    memset(g_collisionEvents[idx].enabled, 0, sizeof(int)*16);
                    while (*val) {
                        while (*val == ' ' || *val == '\t') val++;
                        if (!*val) break;
                        if (*val == '(') {
                            val++;
                            while (*val && *val != ')') val++;
                            if (*val == ')') val++;
                            continue;
                        }
                        if (*val >= '0' && *val <= '9') {
                            int levelNum = atoi(val);
                            if (levelNum >= 1 && levelNum <= 15)
                                g_collisionEvents[idx].enabled[levelNum] = 1;
                            while (*val && *val >= '0' && *val <= '9') val++;
                        } else {
                            val++;
                        }
                    }
                }
            }
        }
    next_coll_line:
        *eol = saved;
        if (*eol == '\r') eol++;
        if (*eol == '\n') eol++;
        line = eol;
    }
}

/* Collision handler function pointer types */
typedef void (__thiscall *Sound_PlayChannel_t)(int channel);
typedef void (__thiscall *Ball_DizzyImmunity_t)(int *ball, long time);
typedef void (__fastcall *Ball_Grow_t)(int ball);
typedef void (__fastcall *Ball_Shrink_t)(int ball);
typedef void (__thiscall *Ball_SetName_t)(int *ball, char *name);
typedef void (__fastcall *Ball_ApplyTrajectory_t)(int ball);
typedef void (__fastcall *Ball_SetTiltedGravity_t)(int ball);
typedef void (__fastcall *Ball_SetFlatGravity_t)(int ball);
typedef void (__fastcall *Rotator_MarkTriggered_t)(int obj);
typedef void (__fastcall *Rotator_PlayCollisionSound_t)(int obj);
typedef void (__fastcall *Rotator_TriggerSound_t)(int obj);
typedef void (__fastcall *Rotator_StartSound_t)(int obj);
typedef void (__thiscall *Rotator_AddBall_t)(void *obj, int ball);
typedef void (__fastcall *CheckArenaUnlock_t)(int board);
typedef void (__fastcall *Catapult_Launch_t)(int obj);
typedef void (__fastcall *Trapdoor_Open_t)(int obj);
typedef void (__fastcall *Trapdoor_Activate_t)(int obj);
typedef void (__fastcall *Saw_AlertActivate_t)(int obj);
typedef void (__fastcall *Saw_Activate_t)(int obj);
typedef void (__fastcall *Bell_Activate_t)(int obj);
typedef void (__fastcall *Judge_Reset_t)(int obj);
typedef void (__thiscall *ScoreDisplay_SetTime_t)(void *obj, long time);
typedef void (__fastcall *CreateBonkPopup_t)(int obj);
typedef void (__fastcall *Hammer_ChaseStart_t)(int obj);
typedef void (__thiscall *Pendulum_PlayCollisionSound_t)(void *obj, int ball);
typedef void (__thiscall *Pendulum_AddIndex_t)(void *obj, int ball);
typedef int (__thiscall *ScoreObject_ctor_t)(void *mem, int board, int timerOffset, const char *label);
typedef void (__fastcall *Timer_Decrement_t)(int obj);
typedef void *__thiscall (*ArenaScoreParticle_ctor_t)(void *mem, int app);
typedef float *(__thiscall *AthenaHashTable_Lookup_t)(void *hashTable, float *out, const char *key, char *found);
typedef int (__thiscall *AthenaList_ContainsValue_t)(void *list, int item);
typedef void (__thiscall *SceneObject_sub1_ctor_t)(void *out);
typedef void (__thiscall *AthenaString_Set_t)(void *obj, const char *str);
typedef void *__thiscall (*MWParser_ReadTag_t)(void *obj);
typedef void (__thiscall *StreamReader_dtor_t)(void *obj);
typedef void (__thiscall *Audio_PlayMusic_t)(void *musicDevice, const char *name);
typedef float (__thiscall *Difficulty_GetTimeModifier_t)(int app, float time);
typedef char *__cdecl (*AthenaString_Format_t)(int obj, const char *fmt);
typedef void (__cdecl *AthenaString_SprintfToBuffer_t)(char *buf, const char *fmt);
/* (Wave_Fn_t typedef lives near top) */
typedef void (__thiscall *Scene_RegisterObject_t)(void *gfx, int playerSlot, int *obj);
typedef void (__thiscall *AthenaList_RemoveByValue_t)(void *list, int item);
typedef void (__fastcall *NeonPlatform_Activate_t)(int obj);
typedef void (__thiscall *SquareWobbly_Activate_t)(void *obj, int ball);
typedef void (__thiscall *Wavy_Activate_t)(void *obj, int ball);
typedef void (__thiscall *Spinner_Activate_t)(void *obj, int ball);
typedef void (__thiscall *Gear_AddBall_t)(void *obj, int ball);
typedef void (__fastcall *NormalGravityReset_t)(int ball);
typedef void (__fastcall *DropLift_Activate_t)(int obj);
typedef int (__thiscall *CPUID_RNG_Fn_t)(void *ptr, int range, int flag);

/* Collision handler function pointers */
static Sound_PlayChannel_t          g_SoundPlayChannel = NULL;
static Ball_DizzyImmunity_t         g_BallDizzyImmunity = NULL;
static Ball_Grow_t                  g_BallGrow = NULL;
static Ball_Shrink_t                g_BallShrink = NULL;
static Ball_SetName_t               g_BallSetName = NULL;
static Ball_ApplyTrajectory_t       g_BallApplyTrajectory = NULL;
static Ball_SetTiltedGravity_t      g_BallSetTiltedGravity = NULL;
static Ball_SetFlatGravity_t        g_BallSetFlatGravity = NULL;
static Rotator_MarkTriggered_t      g_RotatorMarkTriggered = NULL;
static Rotator_PlayCollisionSound_t g_RotatorPlayCollisionSound = NULL;
static Rotator_TriggerSound_t       g_RotatorTriggerSound = NULL;
static Rotator_StartSound_t         g_RotatorStartSound = NULL;
static Rotator_AddBall_t            g_RotatorAddBall = NULL;
static CheckArenaUnlock_t           g_CheckArenaUnlock = NULL;
static Catapult_Launch_t            g_CatapultLaunch = NULL;
static Trapdoor_Open_t              g_TrapdoorOpen = NULL;
static Trapdoor_Activate_t          g_TrapdoorActivate = NULL;
static Saw_AlertActivate_t          g_SawAlertActivate = NULL;
static Saw_Activate_t               g_SawActivate = NULL;
static Bell_Activate_t              g_BellActivate = NULL;
static Judge_Reset_t                g_JudgeReset = NULL;
static ScoreDisplay_SetTime_t       g_ScoreDisplaySetTime = NULL;
static CreateBonkPopup_t            g_CreateBonkPopup = NULL;
static Hammer_ChaseStart_t          g_HammerChaseStart = NULL;
static Pendulum_PlayCollisionSound_t g_PendulumPlayCollisionSound = NULL;
static Pendulum_AddIndex_t          g_PendulumAddIndex = NULL;
static ScoreObject_ctor_t           g_ScoreObjectCtor = NULL;
static Timer_Decrement_t            g_TimerDecrement = NULL;
static ArenaScoreParticle_ctor_t    g_ArenaScoreParticleCtor = NULL;
static AthenaHashTable_Lookup_t    g_AthenaHashTableLookup = NULL;
static AthenaList_ContainsValue_t  g_AthenaListContainsValue = NULL;
static SceneObject_sub1_ctor_t     g_SceneObjectSub1Ctor = NULL;
static AthenaString_Set_t          g_AthenaStringSet = NULL;
static MWParser_ReadTag_t          g_MWParserReadTag = NULL;
static StreamReader_dtor_t         g_StreamReaderDtor = NULL;
static Audio_PlayMusic_t           g_AudioPlayMusic = NULL;
static Difficulty_GetTimeModifier_t g_DifficultyGetTimeModifier = NULL;
static AthenaString_Format_t       g_AthenaStringFormat = NULL;
static AthenaString_SprintfToBuffer_t g_AthenaStringSprintfToBuffer = NULL;
static Wave_Fn_t                   g_WaveCos = NULL;
/* g_WaveSin forward-declared near top (used by UniversalRenderImpl Tower bob) */
/* (Wave_Fn_t typedef lives near top) */
static Scene_RegisterObject_t      g_SceneRegisterObject = NULL;
static AthenaList_RemoveByValue_t  g_AthenaListRemoveByValue = NULL;
static NeonPlatform_Activate_t     g_NeonPlatformActivate = NULL;
static SquareWobbly_Activate_t     g_SquareWobblyActivate = NULL;
static Wavy_Activate_t             g_WavyActivate = NULL;
static Spinner_Activate_t          g_SpinnerActivate = NULL;
static Gear_AddBall_t              g_GearAddBall = NULL;
static NormalGravityReset_t        g_NormalGravityReset = NULL;
static DropLift_Activate_t         g_DropLiftActivate = NULL;
static CPUID_RNG_Fn_t              g_CPUIDRNG = NULL;

/* ═══════════════════════════════════════════════════════════════════════════
 * The merged UniversalDispatchCollision — handles ALL collision events from
 * all 15 levels, then calls DispatchCollisionEvents for global events.
 * ═══════════════════════════════════════════════════════════════════════════ */

void __thiscall UniversalDispatchCollision(void *board, int *ball, int *collPair) {
    if (!board || !ball || !collPair) goto call_global;
    void* ext = GetBoardExt(board);
    if (!ext) ext = EnsureBoardExt(board);
    if (!ext) goto call_global;
    int level = GetCurrentLevel(board);
    if (level == 0 || level > 15) goto call_global;

    DWORD *cp = (DWORD *)collPair;
    char *name = NULL;
    DWORD meshBuf = cp[1]; /* collPair[1] = mesh buffer */
    if (meshBuf && !IsBadReadPtr((void *)meshBuf, 0x868)) {
        name = *(char **)(meshBuf + 0x864);
        if (!name || IsBadReadPtr(name, 4)) name = NULL;
    }
    if (!name) goto call_global;

    DWORD app = *(DWORD *)((char *)board + BOARD_APP_PTR);
    int difficulty = (app && !IsBadReadPtr((void *)app, 0x800)) ? *(int *)(app + APP_DIFFICULTY) : 0;
    DWORD sceneObj = cp[0]; /* *collPair = scene object pointer */
    DWORD meshObj = (sceneObj && !IsBadReadPtr((void *)sceneObj, 0x500)) ? *(DWORD *)(sceneObj + 0x47C) : 0;

    /* ── Beginner/Toob/Master: N:BUMPER ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_BUMPER)) && my_strnicmp(name, "N:BUMPER", 8) == 0) {
        float px = *(float *)((char *)ball + 0x164);
        float py = *(float *)((char *)ball + 0x168);
        float pz = *(float *)((char *)ball + 0x16C);
        if (g_SoundPlay3D && app) {
            DWORD snd = *(DWORD *)(app + 0x448);
            if (snd) g_SoundPlay3D((void *)snd, px, py, pz);
        }
        /* Per-level velocity scale and max speed (verified via Ghidra):
         * Beginner/Toob: 4.0 scale, 10.0 max (_DAT_004cf41c, _DAT_004cf9f8)
         * Master:        5.0 scale, 12.0 max (_DAT_004cf55c, _DAT_004cf3dc) */
        float velScale = (level == 14) ? 5.0f : 4.0f;
        float maxSpeed = (level == 14) ? 12.0f : 10.0f;
        int phys = ball[0x69];
        if (phys && !IsBadReadPtr((void *)phys, 0xCB0)) {
            float vx = *(float *)(phys + 0xCA4);
            float vz = *(float *)(phys + 0xCAC);
            float vy = 0.0f;
            vx *= velScale; vz *= velScale;
            float speedSq = vx*vx + vz*vz;
            if (speedSq < 25.0f) {
                if (speedSq > 0.0001f) {
                    float s = 5.0f / sqrtf(speedSq);
                    vx *= s; vz *= s;
                }
            }
            speedSq = vx*vx + vz*vz;
            if (speedSq > maxSpeed * maxSpeed) {
                float s = maxSpeed / sqrtf(speedSq);
                vx *= s; vz *= s;
            }
            *(float *)(phys + 0xCA4) = vx;
            *(float *)(phys + 0xCA8) = vy;
            *(float *)(phys + 0xCAC) = vz;
        }
        long bumperNum = atol(name + 8);
        // Require N:BUMPERn with 1-8 digit — plain "N:BUMPER" (no number) is not a valid bumper mesh
        if (name[8] < '1' || name[8] > '8' || name[9] != '\0') goto call_global;
        bumperNum = name[8] - '0';
        DWORD litBase = UNI_BUMPER_LIT;
        // BUMPER_LIT is unified -> ext ; adjust for 1-based naming: lit index = bumperNum-1
        *(DWORD *)((char *)ext + (bumperNum-1) * 4 + litBase) = 0x3F800000;
    }

    /* ── Intermediate: N:BRIDGE ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_BRIDGE)) && my_stricmp(name, "N:BRIDGE") == 0) {
        if (*(int *)((char *)ext + UNI_BRIDGE_STATE) == 3) {
            *(BYTE *)((char *)ball + 0x778) = 1;
        }
        /* Original Intermediate handler returns for N:BRIDGE regardless of
         * state — does NOT call DispatchCollisionEvents. Since N:BRIDGE is
         * not a global event, skipping call_global is equivalent. */
        return;
    }

    /* ── Dizzy: N:WATERWHEEL ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_WATERWHEEL)) && my_stricmp(name, "N:WATERWHEEL") == 0) {
        *(BYTE *)((char *)ball + 0x778) = 1;
        return; /* Dizzy returns early */
    }

    /* ── Dizzy: N:WHEELEMBED ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_WHEELEMBED)) && my_stricmp(name, "N:WHEELEMBED") == 0) {
        float dx = *(float *)((char *)ball + 0x164) - *(float *)((char *)ext + UNI_WHEELEMBED_X);
        float dy = *(float *)((char *)ball + 0x168) - *(float *)((char *)ext + UNI_WHEELEMBED_Y);
        float dz = *(float *)((char *)ball + 0x16C) - *(float *)((char *)ext + UNI_WHEELEMBED_Z);
        /* Transform via Gfx_ScaleY(-5.0) + Matrix_TransformVec3 */
        if (g_TimerInit && g_TimerCleanup && g_GfxScaleY && g_MatrixTransformVec3 && g_GfxSetPosition && app) {
            void *gfx = *(void **)(app + 0x174);
            if (gfx) {
            char timerBuf[68];
            float transformed[16];
            g_TimerInit(timerBuf);
            g_GfxScaleY(gfx, -5.0f);
            g_MatrixTransformVec3(transformed, &dx);
            dx += *(float *)((char *)ext + UNI_WHEELEMBED_X);
            dy += *(float *)((char *)ext + UNI_WHEELEMBED_Y);
            dz += *(float *)((char *)ext + UNI_WHEELEMBED_Z);
            g_TimerCleanup(timerBuf);
            }
        }
        *(BYTE *)((char *)ball + 0xC3C) = 1;
        *(float *)((char *)ball + 0xC40) = dx;
        *(float *)((char *)ball + 0xC44) = dy;
        *(float *)((char *)ball + 0xC48) = dz;
        *(int *)((char *)ball + 0x808) = 0x32;
        return; /* Dizzy returns early */
    }

    /* ── Dizzy: N:SWIRL ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_SWIRL)) && my_stricmp(name, "N:SWIRL") == 0) {
        *(BYTE *)((char *)ball + 0x779) = 1;
        return; /* Dizzy returns early */
    }

    /* ── Tower/Master: E:CATAPULTBOTTOM ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_CATAPULTBOTTOM)) && my_stricmp(name, "E:CATAPULTBOTTOM") == 0) {
        if (*(int *)((char *)ball + 0x808) < 1) {
            *(int *)((char *)ball + 0x808) = 1000;
            DWORD catList = UNI_CATAPULT_LIST;
            DWORD catCount = UNI_CATAPULT_COUNT;
            DWORD catData = UNI_CATAPULT_DATA;
            int iter = g_AthenaListGetIterator((void *)((char *)ext + catList));
            *(DWORD *)((char *)ext + catList + 8 + iter * 4) = 0;
            int count = *(int *)((char *)ext + catCount);
            int item = 0;
            if (count > 0) {
                item = **(int **)((char *)ext + catData);
                *(DWORD *)((char *)ext + catList + 8 + iter * 4) = 1;
            }
            while (item) {
                if (*(int *)(item + 0x10D4) == sceneObj) {
                    *(int **)(item + 0x10EC) = ball;
                    if (g_CatapultLaunch) g_CatapultLaunch(item);
                    if (g_SoundPlayChannel && app) {
                        int ch = *(int *)(app + 0x464);
                        if (ch) g_SoundPlayChannel(ch);
                    }
                }
                int next = *(int *)((char *)ext + catList + 8 + iter * 4);
                if (*(int *)((char *)ext + catCount) <= next) break;
                item = *(int *)(*(int **)((char *)ext + catData) + next * 4);
                *(int *)((char *)ext + catList + 8 + iter * 4) = next + 1;
            }
        }
    }

    /* ── Tower: E:OPENSESAME ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_OPENSESAME)) && my_stricmp(name, "E:OPENSESAME") == 0) {
        if (g_TrapdoorOpen) {
            int count = *(int *)((char *)ext + UNI_DRAWBRIDGE_COUNT);
            int item = 0;
            if (count >= 1) item = **(int **)((char *)ext + UNI_DRAWBRIDGE_DATA);
            g_TrapdoorOpen(item);
        }
    }

    /* ── Tower: N:TRAPDOOR ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_TRAPDOOR)) && my_stricmp(name, "N:TRAPDOOR") == 0) {
        int iter = g_AthenaListGetIterator((void *)((char *)ext + UNI_TRAPDOOR_LIST));
        *(DWORD *)((char *)ext + UNI_TRAPDOOR_LIST + 8 + iter * 4) = 0;
        int count = *(int *)((char *)ext + UNI_TRAPDOOR_COUNT);
        int item = 0;
        if (count >= 1) {
            item = **(int **)((char *)ext + UNI_TRAPDOOR_DATA);
            *(DWORD *)((char *)ext + UNI_TRAPDOOR_LIST + 8 + iter * 4) = 1;
        }
        while (item) {
            if (*(int *)(item + 0x10D4) == sceneObj || *(int *)(item + 0x10DC) == sceneObj) {
                if (g_TrapdoorActivate) g_TrapdoorActivate(item);
            }
            int next = *(int *)((char *)ext + UNI_TRAPDOOR_LIST + 8 + iter * 4);
            if (*(int *)((char *)ext + UNI_TRAPDOOR_COUNT) <= next) break;
            item = *(int *)(*(int **)((char *)ext + UNI_TRAPDOOR_DATA) + next * 4);
            *(int *)((char *)ext + UNI_TRAPDOOR_LIST + 8 + iter * 4) = next + 1;
        }
    }

    /* ── Tower: E:BITE ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_BITE)) && my_stricmp(name, "E:BITE") == 0) {
        *(DWORD *)((char *)ext + UNI_BITE_STATE) = 0;
        *(DWORD *)((char *)ext + UNI_BITE_SPEED) = 0x41C80000;
    }

    /* ── Tower: E:MACETRIGGER ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_MACETRIGGER)) && my_stricmp(name, "E:MACETRIGGER") == 0) {
        int iter = g_AthenaListGetIterator((void *)((char *)ext + UNI_MACE_LIST));
        *(DWORD *)((char *)ext + UNI_MACE_LIST + 8 + iter * 4) = 0;
        int count = *(int *)((char *)ext + UNI_MACE_COUNT);
        int item = 0;
        if (count >= 1) {
            item = **(int **)((char *)ext + UNI_MACE_DATA);
            *(DWORD *)((char *)ext + UNI_MACE_LIST + 8 + iter * 4) = 1;
        }
        while (item) {
            *(BYTE *)(item + 0x10F0) = 1;
            int next = *(int *)((char *)ext + UNI_MACE_LIST + 8 + iter * 4);
            if (*(int *)((char *)ext + UNI_MACE_COUNT) <= next) break;
            item = *(int *)(*(int **)((char *)ext + UNI_MACE_DATA) + next * 4);
            *(int *)((char *)ext + UNI_MACE_LIST + 8 + iter * 4) = next + 1;
        }
    }

    /* ── Tower: N:MACE ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_MACE)) && my_stricmp(name, "N:MACE") == 0) {
        int iter = g_AthenaListGetIterator((void *)((char *)ext + UNI_MACE_LIST));
        *(DWORD *)((char *)ext + UNI_MACE_LIST + 8 + iter * 4) = 0;
        int count = *(int *)((char *)ext + UNI_MACE_COUNT);
        int item = 0;
        if (count >= 1) {
            item = **(int **)((char *)ext + UNI_MACE_DATA);
            *(DWORD *)((char *)ext + UNI_MACE_LIST + 8 + iter * 4) = 1;
        }
        while (item) {
            if (*(int *)(item + 0x10D4) == sceneObj &&
                *(int *)(item + 0x10E8) != 0x42A00000 &&
                *(int *)(item + 0x10F4) == 0) {
                /* Call ball vtable[+0x20] (break) */
                DWORD *vtbl = *(DWORD **)ball;
                if (vtbl) {
                    void (__thiscall *fn)(int *) = (void (__thiscall *)(int *))vtbl[8];
                    if (fn) fn(ball);
                }
            }
            int next = *(int *)((char *)ext + UNI_MACE_LIST + 8 + iter * 4);
            if (*(int *)((char *)ext + UNI_MACE_COUNT) <= next) break;
            item = *(int *)(*(int **)((char *)ext + UNI_MACE_DATA) + next * 4);
            *(int *)((char *)ext + UNI_MACE_LIST + 8 + iter * 4) = next + 1;
        }
    }

    /* ── Up/Impossible: E:HELPINERTIA ──
     * Impossible gates this on (char)ball[0x768] != 0; Up does not. */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_HELPINERTIA)) && my_stricmp(name, "E:HELPINERTIA") == 0) {
        if (level != 15 || (char)*(int *)((char *)ball + 0x768))
            ball[0xA9] = 0x40200000;
    }

    /* ── Up/Impossible: E:UNHELPINERTIA ──
     * Impossible gates this on (char)ball[0x768] != 0; Up does not. */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_UNHELPINERTIA)) && my_stricmp(name, "E:UNHELPINERTIA") == 0) {
        if (level != 15 || (char)*(int *)((char *)ball + 0x768))
            ball[0xA9] = 0x40A00000;
    }

    /* ── Up: E:VACPOPOUT ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_VACPOPOUT)) && my_stricmp(name, "E:VACPOPOUT") == 0) {
        ball[0xA1] = 0x41D00000;
        if (g_SoundPlay3D && app) {
            DWORD snd = *(DWORD *)(app + 0x468);
            if (snd) g_SoundPlay3D((void *)snd, *(float *)((char *)ball + 0x164), *(float *)((char *)ball + 0x168), *(float *)((char *)ball + 0x16C));
        }
    }

    /* ── Up: N:SPEEDCYLINDER ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_SPEEDCYLINDER)) && my_stricmp(name, "N:SPEEDCYLINDER") == 0) {
        if (g_PendulumPlayCollisionSound && meshObj)
            g_PendulumPlayCollisionSound((void *)meshObj, (int)ball);
    }

    /* ── Up: N:EXTRATIME ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_EXTRATIME)) && my_stricmp(name, "N:EXTRATIME") == 0) {
        if (meshObj && *(char *)(meshObj + 0x10E4) == 0) {
            if (g_RotatorTriggerSound) g_RotatorTriggerSound(meshObj);
            if (app) {
                int gameMode = *(int *)(app + 0x220);
                if (gameMode && *(char *)(gameMode + 0x10) == 0 && *(char *)(gameMode + 0x11) == 0) {
                    *(DWORD *)(ball[6]*0xA0 + 0x5EC + app) = 500;
                    if (g_operatorNew && g_ScoreObjectCtor && g_TimerDecrement && g_AthenaListAppend) {
                        void *mem = g_operatorNew(0x30);
                        if (mem) {
                            int so = g_ScoreObjectCtor(mem, (int)board, ball[6]*0xA0 + 0x5CC + app, "EXTRA TIME:");
                            g_TimerDecrement(so);
                            g_AthenaListAppend((void *)((char *)board + UNI_SCORE_LIST), so);
                        }
                    }
                }
            }
        }
    }

    /* ── Neon: N:NEONPLATFORM ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_NEONPLATFORM)) && my_strnicmp(name, "N:NEONPLATFORM", 14) == 0) {
        if (g_NeonPlatformActivate && meshObj) g_NeonPlatformActivate(meshObj);
    }

    /* ── Neon: E:ZOOP ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_ZOOP)) && my_strnicmp(name, "E:ZOOP", 6) == 0) {
        if (*(int *)((char *)ball + 0x7F0) == 0) {
            if (g_SoundPlay3D && app) {
                DWORD snd = *(DWORD *)(app + 0x524);
                if (snd) g_SoundPlay3D((void *)snd, *(float *)((char *)ball + 0x164), *(float *)((char *)ball + 0x168), *(float *)((char *)ball + 0x16C));
            }
            *(int *)((char *)ball + 0x7F0) = 100;
        }
    }

    /* ── Neon: E:LIGHTSOFF ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_LIGHTSOFF)) && my_strnicmp(name, "E:LIGHTSOFF", 10) == 0) {
        if (*(int *)((char *)ball + 0x7B4) == 0) {
            if (g_SoundPlay3D && app) {
                DWORD snd = *(DWORD *)(app + 0x528);
                if (snd) g_SoundPlay3D((void *)snd, *(float *)((char *)ball + 0x164), *(float *)((char *)ball + 0x168), *(float *)((char *)ball + 0x16C));
            }
            /* Call vtable[+0x10](0) on player's render obj */
            void *lightExt = GetBoardExt(board); if (!lightExt) lightExt = ext;
            DWORD *renderObj = lightExt ? *(DWORD **)((char *)lightExt + ball[6]*4 + UNI_BONK_STORE) : NULL;
            if (!renderObj && lightExt) renderObj = *(DWORD **)((char *)lightExt + UNI_BONK_STORE);
            if (renderObj) {
                DWORD *vtbl = *(DWORD **)renderObj;
                if (vtbl) {
                    void (__thiscall *fn)(DWORD, int) = (void (__thiscall *)(DWORD, int))vtbl[4];
                    if (fn) fn((DWORD)renderObj, 0);
                }
                if (g_SceneRegisterObject && app) {
                    void *gfx = *(void **)(app + 0x174);
                    if (gfx) g_SceneRegisterObject(gfx, ball[6], (int *)renderObj);
                }
            }
            if (*(int *)((char *)ext + UNI_NEON_TRAPDOOR) == 0 && g_AthenaListAppend) {
                g_AthenaListAppend((void *)((char *)board + UNI_OBJ_LIST), *(int *)((char *)ext + UNI_NEON_DARK_COUNT));
            }
            *(int *)((char *)ext + UNI_NEON_TRAPDOOR) += 1;
            *(int *)((char *)ball + 0x7B4) = 100;
        }
    }

    /* ── Neon: E:LIGHTSON ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_LIGHTSON)) && my_strnicmp(name, "E:LIGHTSON", 10) == 0) {
        if (*(int *)((char *)ball + 0x7B8) == 0) {
            if (g_SoundPlay3D && app) {
                DWORD snd = *(DWORD *)(app + 0x528);
                if (snd) g_SoundPlay3D((void *)snd, *(float *)((char *)ball + 0x164), *(float *)((char *)ball + 0x168), *(float *)((char *)ball + 0x16C));
            }
            void *lightExt2 = GetBoardExt(board); if (!lightExt2) lightExt2 = ext;
            DWORD *renderObj = lightExt2 ? *(DWORD **)((char *)lightExt2 + ball[6]*4 + UNI_BONK_STORE) : NULL;
            if (!renderObj && lightExt2) renderObj = *(DWORD **)((char *)lightExt2 + UNI_BONK_STORE);
            if (renderObj) {
                DWORD *vtbl = *(DWORD **)renderObj;
                if (vtbl) {
                    void (__thiscall *fn)(DWORD, int) = (void (__thiscall *)(DWORD, int))vtbl[4];
                    if (fn) fn((DWORD)renderObj, 1);
                }
                if (g_SceneRegisterObject && app) {
                    void *gfx = *(void **)(app + 0x174);
                    if (gfx) g_SceneRegisterObject(gfx, ball[6], (int *)renderObj);
                }
            }
            *(int *)((char *)ball + 0x7B8) = 100;
            int n = *(int *)((char *)ext + UNI_NEON_TRAPDOOR) - 1;
            *(int *)((char *)ext + UNI_NEON_TRAPDOOR) = n;
            if (n < 1 && g_AthenaListRemoveByValue) {
                *(int *)((char *)ext + UNI_NEON_TRAPDOOR) = 0;
                g_AthenaListRemoveByValue((void *)((char *)board + UNI_OBJ_LIST), *(int *)((char *)ext + UNI_NEON_DARK_COUNT));
                DWORD trapObj = *(DWORD *)((char *)ext + UNI_NEON_DARK_COUNT);
                if (trapObj) {
                    *(DWORD *)(trapObj + 0x10DC) = 2;
                    *(DWORD *)(trapObj + 0x10E0) = 0;
                }
            }
        }
    }

    /* ── Expert/Master: E:CALLHAMMER ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_CALLHAMMER)) && my_stricmp(name, "E:CALLHAMMER") == 0) {
        if (difficulty != 0 && g_CreateBonkPopup) {
            void *be = GetBoardExt(board); if (!be) be = ext;
            int bonkObj = be ? *(int*)((char*)be + UNI_BONK_MESH) : *(int*)((char*)ext + UNI_BONK_MESH);
            if (!bonkObj) bonkObj = be ? *(int*)((char*)be + UNI_BONK_STORE) : *(int*)((char*)ext + UNI_BONK_STORE);
            g_CreateBonkPopup(bonkObj);
        }
    }

    /* ── Expert/Master: E:HAMMERCHASE ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_HAMMERCHASE)) && my_stricmp(name, "E:HAMMERCHASE") == 0) {
        if (difficulty != 0 && g_HammerChaseStart) {
            void *be2 = GetBoardExt(board); if (!be2) be2 = ext;
            int bonkObj2 = be2 ? *(int*)((char*)be2 + UNI_BONK_MESH) : *(int*)((char*)ext + UNI_BONK_MESH);
            if (!bonkObj2) bonkObj2 = be2 ? *(int*)((char*)be2 + UNI_BONK_STORE) : *(int*)((char*)ext + UNI_BONK_STORE);
            g_HammerChaseStart(bonkObj2);
        }
    }

    /* ── Expert: E:ALERTSAW1 ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_ALERTSAW1)) && my_stricmp(name, "E:ALERTSAW1") == 0) {
        if (difficulty != 0 && g_SawAlertActivate) {
            int saw1 = *(int *)((char *)ext + UNI_SAWBLADE1_OBJ);
            if (!saw1) saw1 = *(int *)((char *)ext + UNI_SAW1_OBJ);
            g_SawAlertActivate(saw1);
        }
    }

    /* ── Expert: E:ALERTSAW2 ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_ALERTSAW2)) && my_stricmp(name, "E:ALERTSAW2") == 0) {
        if (difficulty != 0 && g_SawAlertActivate) {
            int saw2 = *(int *)((char *)ext + UNI_SAWBLADE2_OBJ);
            if (!saw2) saw2 = *(int *)((char *)ext + UNI_SAW2_OBJ);
            g_SawAlertActivate(saw2);
        }
    }

    /* ── Toob: E:ALERTSAW2 vanilla + E:ALERTSAW3 renamed ── — vanilla Toob files use ALERTSAW2 */
    if ((((*(BYTE*)((char*)ext + COLL_FLAG_ALERTSAW3)) && my_stricmp(name, "E:ALERTSAW3") == 0)) || (GetCurrentLevel(board) == 10 && my_stricmp(name, "E:ALERTSAW2") == 0)) {
        if (difficulty != 0) {
            int saw2Obj = *(int *)((char *)ext + UNI_SAW2_TOOB_OBJ);
            if (saw2Obj) *(BYTE *)(saw2Obj + 0x110C) = 1;
        }
    }

    /* ── Expert: E:ACTIVATESAW1 (fallback to B8F4 dedup) ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_ACTIVATESAW1)) && my_stricmp(name, "E:ACTIVATESAW1") == 0) {
        if (difficulty != 0 && g_SawActivate) {
            int saw1 = *(int *)((char *)ext + UNI_SAWBLADE1_OBJ);
            if (!saw1) saw1 = *(int *)((char *)ext + UNI_SAW1_OBJ);
            g_SawActivate(saw1);
        }
    }

    /* ── Expert: E:ACTIVATESAW2 ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_ACTIVATESAW2)) && my_stricmp(name, "E:ACTIVATESAW2") == 0) {
        if (difficulty != 0 && g_SawActivate) {
            int saw2 = *(int *)((char *)ext + UNI_SAWBLADE2_OBJ);
            if (!saw2) saw2 = *(int *)((char *)ext + UNI_SAW2_OBJ);
            g_SawActivate(saw2);
        }
    }

    /* ── Expert: E:ALERTJUDGES ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_ALERTJUDGES)) && my_stricmp(name, "E:ALERTJUDGES") == 0) {
        int iter = g_AthenaListGetIterator((void *)((char *)ext + UNI_JUDGE_LIST));
        *(DWORD *)((char *)ext + UNI_JUDGE_LIST + 8 + iter * 4) = 0;
        int count = *(int *)((char *)ext + UNI_JUDGE_COUNT);
        int item = 0;
        if (count >= 1) {
            item = **(int **)((char *)ext + UNI_JUDGE_DATA);
            *(DWORD *)((char *)ext + UNI_JUDGE_LIST + 8 + iter * 4) = 1;
        }
        while (item) {
            if (g_JudgeReset) g_JudgeReset(item);
            int next = *(int *)((char *)ext + UNI_JUDGE_LIST + 8 + iter * 4);
            if (*(int *)((char *)ext + UNI_JUDGE_COUNT) <= next) break;
            item = *(int *)(*(int **)((char *)ext + UNI_JUDGE_DATA) + next * 4);
            *(int *)((char *)ext + UNI_JUDGE_LIST + 8 + iter * 4) = next + 1;
        }
    }

    /* ── Expert: E:SCORE ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_SCORE)) && my_strnicmp(name, "E:SCORE", 7) == 0) {
        int iter = g_AthenaListGetIterator((void *)((char *)ext + UNI_JUDGE_LIST));
        *(DWORD *)((char *)ext + UNI_JUDGE_LIST + 8 + iter * 4) = 0;
        int count = *(int *)((char *)ext + UNI_JUDGE_COUNT);
        void *item = NULL;
        if (count >= 1) {
            item = (void *)**(int **)((char *)ext + UNI_JUDGE_DATA);
            *(DWORD *)((char *)ext + UNI_JUDGE_LIST + 8 + iter * 4) = 1;
        }
        while (item) {
            long score = atol(name + 7);
            if (g_ScoreDisplaySetTime) g_ScoreDisplaySetTime(item, score);
            int next = *(int *)((char *)ext + UNI_JUDGE_LIST + 8 + iter * 4);
            if (*(int *)((char *)ext + UNI_JUDGE_COUNT) <= next) break;
            item = *(void **)(*(int **)((char *)ext + UNI_JUDGE_DATA) + next * 4);
            *(int *)((char *)ext + UNI_JUDGE_LIST + 8 + iter * 4) = next + 1;
        }
    }

    /* ── Expert: E:JUMP is identical to the global E:JUMP in DispatchCollisionEvents.
     *   No level-specific handler needed — the global handler at the end handles it. ── */

    /* ── Expert: E:BELL ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_BELL)) && my_strnicmp(name, "E:BELL", 6) == 0) {
        if (g_BellActivate) g_BellActivate(*(int *)((char *)ext + UNI_BELL_OBJ));
        if (app) {
            int gameMode = *(int *)(app + 0x220);
            if (gameMode && *(char *)(gameMode + 0x10) == 0 && *(char *)(gameMode + 0x11) == 0) {
                *(DWORD *)(ball[6]*0xA0 + 0x5EC + app) = 500;
                if (g_operatorNew && g_ScoreObjectCtor && g_TimerDecrement && g_AthenaListAppend) {
                    void *mem = g_operatorNew(0x30);
                    if (mem) {
                        int so = g_ScoreObjectCtor(mem, (int)board, ball[6]*0xA0 + 0x5CC + app, "EXTRA TIME:");
                        g_TimerDecrement(so);
                        g_AthenaListAppend((void *)((char *)board + UNI_SCORE_LIST), so);
                    }
                }
            }
        }
    }

    /* ── Odd: E:GRAVITY ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_GRAVITY)) && my_strnicmp(name, "E:GRAVITY", 9) == 0) {
        if (g_SceneObjectSub1Ctor && g_AthenaStringSet && g_MWParserReadTag && g_StreamReaderDtor) {
            char strObj[64];
            g_SceneObjectSub1Ctor(strObj);
            g_AthenaStringSet(strObj, name);
            DWORD *tag;
            while ((tag = (DWORD *)g_MWParserReadTag(strObj)) != NULL) {
                if (my_stricmp((char *)tag[1], "TYPE") == 0) {
                    if (my_stricmp((char *)tag[2], "NORMAL") == 0 && g_NormalGravityReset)
                        g_NormalGravityReset((int)ball);
                    if (my_stricmp((char *)tag[2], "X") == 0 && g_BallSetTiltedGravity)
                        g_BallSetTiltedGravity((int)ball);
                    if (my_stricmp((char *)tag[2], "Z") == 0 && g_BallSetFlatGravity)
                        g_BallSetFlatGravity((int)ball);
                }
                /* Original game: (**(code **)*puVar4)(1) — double dereference.
                 * tag[0] is a pointer to a vtable; vtable[0] is the destructor. */
                (*(void (__thiscall **)(DWORD))(*(DWORD*)tag[0]))(1);
            }
            g_StreamReaderDtor(strObj);
        }
    }

    /* ── Odd: N:JUMPFIRST / N:JUMPSECOND ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_JUMPFIRST)) && my_stricmp(name, "N:JUMPFIRST") == 0) {
        /* Uses AthenaHashTable_Lookup for "JUMPPIPE1" */
        float pos[3] = {0,0,0};
        if (g_AthenaHashTableLookup && app) {
            void *ht = *(void **)((char *)board + BOARD_MESHWORLD);
            if (ht) g_AthenaHashTableLookup(ht, pos, "JUMPPIPE1", NULL);
        }
        *(BYTE *)((char *)ball + 0xC3C) = 1;
        *(float *)((char *)ball + 0xC40) = pos[0];
        *(float *)((char *)ball + 0xC44) = pos[1];
        *(float *)((char *)ball + 0xC48) = pos[2];
        int phys = ball[0x69];
        if (phys && !IsBadReadPtr((void *)phys, 0xCB0)) {
            *(float *)(phys + 0xCA4) = 0.0f;
            *(DWORD *)(phys + 0xCA8) = 0x41800000;
            *(DWORD *)(phys + 0xCAC) = 0;
        }
    }
    if ((*(BYTE*)((char*)ext + COLL_FLAG_JUMPSECOND)) && my_stricmp(name, "N:JUMPSECOND") == 0) {
        float pos[3] = {0,0,0};
        if (g_AthenaHashTableLookup && app) {
            void *ht = *(void **)((char *)board + BOARD_MESHWORLD);
            if (ht) g_AthenaHashTableLookup(ht, pos, "JUMPPIPE2", NULL);
        }
        *(BYTE *)((char *)ball + 0xC3C) = 1;
        *(float *)((char *)ball + 0xC40) = pos[0];
        *(float *)((char *)ball + 0xC44) = pos[1];
        *(float *)((char *)ball + 0xC48) = pos[2];
        int phys = ball[0x69];
        if (phys && !IsBadReadPtr((void *)phys, 0xCB0)) {
            *(float *)(phys + 0xCA4) = 0.0f;
            *(DWORD *)(phys + 0xCA8) = 0x41800000;
            *(DWORD *)(phys + 0xCAC) = 0;
        }
    }

    /* ── Odd: E:SHRINK ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_SHRINK)) && my_stricmp(name, "E:SHRINK") == 0) {
        if (g_BallShrink) g_BallShrink((int)ball);
        float pos[3] = {0,0,0};
        if (g_AthenaHashTableLookup) {
            void *ht = *(void **)((char *)board + BOARD_MESHWORLD);
            if (ht) g_AthenaHashTableLookup(ht, pos, "SHRINKCENTER", NULL);
        }
        *(BYTE *)((char *)ball + 0xC3C) = 1;
        *(float *)((char *)ball + 0xC40) = pos[0];
        *(float *)((char *)ball + 0xC44) = pos[1];
        *(float *)((char *)ball + 0xC48) = pos[2];
        int phys = ball[0x69];
        if (phys && !IsBadReadPtr((void *)phys, 0xCB0)) {
            *(float *)(phys + 0xCA4) = 0.0f;
            *(DWORD *)(phys + 0xCA8) = 0xBF800000;
            *(DWORD *)(phys + 0xCAC) = 0;
        }
    }

    /* ── Odd: E:GROWSOUND ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_GROWSOUND)) && my_stricmp(name, "E:GROWSOUND") == 0) {
        if (*(int *)((char *)ball + 0x7F8) == 0) {
            if (g_SoundPlayChannel && app) {
                int ch = *(int *)(app + 0x4D8);
                if (ch) g_SoundPlayChannel(ch);
            }
        }
        *(int *)((char *)ball + 0x7F8) = 100;
    }

    /* ── Odd: E:GROW ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_GROW)) && my_stricmp(name, "E:GROW") == 0) {
        if (g_BallGrow) g_BallGrow((int)ball);
    }

    /* ── Odd: E:DROPLIFT ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_DROPLIFT)) && my_stricmp(name, "E:DROPLIFT") == 0) {
        if (g_DropLiftActivate) g_DropLiftActivate(*(int *)((char *)ext + UNI_BONK_STORE));
    }

    /* ── Odd: E:PIPERANDOM (complex — simplified to core behavior) ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_PIPERANDOM)) && my_stricmp(name, "E:PIPERANDOM") == 0) {
        if (g_BallGrow) g_BallGrow((int)ball);
        if (difficulty != 0) *(BYTE *)((char *)ext + UNI_SAW1_OBJ) = 1;
        /* Random pipe selection */
        if (g_CPUIDRNG && g_AthenaHashTableLookup) {
            void *ht = *(void **)((char *)board + BOARD_MESHWORLD);
            int rng = RNG_call((void *)0x4F7360, 0, 2, 0);
            float pos[3] = {0,0,0};
            if (rng == 0) {
                /* PIPERANDOM1: set byte 4 flag (checked by E:LIMITPIPE1) */
                const char *key = "PIPERANDOM1";
                if (ht) g_AthenaHashTableLookup(ht, pos, key, NULL);
                *(BYTE *)((char *)ball + 4) = 1;
            } else {
                /* PIPERANDOM2: set byte 5 flag (checked by E:LIMITPIPE2) */
                const char *key = "PIPERANDOM2";
                if (ht) g_AthenaHashTableLookup(ht, pos, key, NULL);
                *(BYTE *)((char *)ball + 5) = 1;
            }
            *(BYTE *)((char *)ball + 0xC3C) = 1;
            *(float *)((char *)ball + 0xC40) = pos[0];
            *(float *)((char *)ball + 0xC44) = pos[1];
            *(float *)((char *)ball + 0xC48) = pos[2];
            int phys = ball[0x69];
            if (phys && !IsBadReadPtr((void *)phys, 0xCB0)) {
                *(DWORD *)(phys + 0xCA4) = 0;
                *(DWORD *)(phys + 0xCA8) = 0;
                *(DWORD *)(phys + 0xCAC) = 0;
            }
            if (g_SoundPlay3D && app) {
                DWORD snd = *(DWORD *)(app + 0x468);
                if (snd) g_SoundPlay3D((void *)snd, *(float *)((char *)ball + 0x164), *(float *)((char *)ball + 0x168), *(float *)((char *)ball + 0x16C));
            }
        }
    }

    /* ── Odd: E:LIMIT / E:LIMITX / E:LIMITZ / E:LIMITPIPE1 / E:LIMITPIPE2 / E:SWALLOW ── */
    /* Odd's E:LIMIT checks ball+0x1D2 (axis selector) */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_LIMIT)) && my_stricmp(name, "E:LIMIT") == 0) {
        /* Odd's version: only activates if ball+0x1D2 == 0 */
        if (*(int *)((char *)ball + 0x748) == 0) {
            *(BYTE *)((char *)ball + 0x768) = 0;
            *(BYTE *)((char *)ball + 0x2E9) = 1;
        }
        /* Sky's version: removes from heat list if difficulty != 0 */
        if (difficulty != 0 && g_AthenaListRemoveByValue) {
            DWORD mgObj = *(DWORD *)((char *)ext + UNI_MAGNIFYING_GLASS);
            if (mgObj) g_AthenaListRemoveByValue((void *)(mgObj + 0x2C), (int)ball);
        }
    }
    if ((*(BYTE*)((char*)ext + COLL_FLAG_LIMITX)) && my_stricmp(name, "E:LIMITX") == 0) {
        if (*(int *)((char *)ball + 0x748) == 1) {
            *(BYTE *)((char *)ball + 0x768) = 0;
            *(BYTE *)((char *)ball + 0x2E9) = 1;
        }
    }
    if ((*(BYTE*)((char*)ext + COLL_FLAG_LIMITZ)) && my_stricmp(name, "E:LIMITZ") == 0) {
        if (*(int *)((char *)ball + 0x748) == 2) {
            *(BYTE *)((char *)ball + 0x768) = 0;
            *(BYTE *)((char *)ball + 0x2E9) = 1;
        }
    }
    if ((*(BYTE*)((char*)ext + COLL_FLAG_LIMITPIPE1)) && my_stricmp(name, "E:LIMITPIPE1") == 0) {
        if ((char)ball[1] != 0) {
            *(BYTE *)((char *)ball + 0x768) = 0;
            *(BYTE *)((char *)ball + 0x2E9) = 1;
        }
    }
    if ((*(BYTE*)((char*)ext + COLL_FLAG_SWALLOW)) && my_stricmp(name, "E:SWALLOW") == 0) {
        *(BYTE *)((char *)ball + 0x2E8) = 1;
    }
    if ((*(BYTE*)((char*)ext + COLL_FLAG_LIMITPIPE2)) && my_stricmp(name, "E:LIMITPIPE2") == 0) {
        /* VERIFIED via Ghidra decompilation of OddBoard_CollisionHandler (0x0040ED30):
         * Original: cVar1 = *(char *)((int)param_2 + 5);  — byte offset 5, NOT int* index.
         * This is correct as-is. E:LIMITPIPE1 uses (char)param_2[1] = byte 4 (int* arithmetic).
         * Both offsets match the original game exactly. */
        if (*(char *)((char *)ball + 5) != 0) {
            *(BYTE *)((char *)ball + 0x768) = 0;
            *(BYTE *)((char *)ball + 0x2E9) = 1;
        }
    }

    /* ── Toob: E:BRANCH (pipe branching system) ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_BRANCH)) && my_strnicmp(name, "E:BRANCH", 8) == 0) {
        if (g_AthenaHashTableLookup && g_CPUIDRNG && g_Vec3NormalizeAndScale &&
            g_AthenaListInit && g_AthenaListAppend && g_AthenaListGetSize &&
            g_AthenaListGetIterator && g_operatorNew) {
            void *ht = *(void **)((char *)board + BOARD_MESHWORLD);
            if (ht) {
                /* Local AthenaList for position+vector pairs */
                char listBuf[0x420]; /* AthenaList struct */
                g_AthenaListInit(listBuf, 0);

                char *branchName = name + 2; /* skip "E:" prefix */
                int isBranchB = (my_stricmp(name + 8, "(B)") == 0);

                /* Collect up to 5 POS/VECTOR pairs from hash table */
                int i;
                for (i = 1; i <= 5; i++) {
                    char posKey[64], vecKey[64];
                    sprintf(posKey, "%s-POS%.2d", branchName, i);
                    sprintf(vecKey, "%s-VECTOR%.2d", branchName, i);

                    float pos[3] = {0,0,0};
                    float vec[3] = {0,0,0};
                    char found1 = 0, found2 = 0;

                    g_AthenaHashTableLookup(ht, pos, posKey, &found1);
                    if (!found1) break;
                    g_AthenaHashTableLookup(ht, vec, vecKey, &found2);
                    if (!found2) break;

                    /* Allocate 0x18 bytes: pos[3] + vec[3] */
                    float *entry = (float *)g_operatorNew(0x18);
                    if (entry) {
                        entry[0] = pos[0]; entry[1] = pos[1]; entry[2] = pos[2];
                        entry[3] = vec[0]; entry[4] = vec[1]; entry[5] = vec[2];
                        g_AthenaListAppend(listBuf, (int)entry);
                    }
                }

                /* Pick a random entry and compute direction */
                int count = g_AthenaListGetSize(listBuf);
                if (count > 0) {
                    int rngIdx = RNG_call((void *)0x4F7360, 0, count, 0);
                    if (rngIdx < 0) rngIdx = 0;
                    if (rngIdx >= count) rngIdx = 0;

                    /* Get the entry at rngIdx */
                    int iter = g_AthenaListGetIterator(listBuf);
                    *(int *)(listBuf + 8 + iter * 4) = 0;
                    float *entry = NULL;
                    if (count >= 1) {
                        entry = *(float **)(*(int **)(listBuf + 0x40C));
                        *(int *)(listBuf + 8 + iter * 4) = 1;
                    }
                    /* Walk to the rngIdx-th entry */
                    int idx;
                    for (idx = 0; idx < rngIdx && entry; idx++) {
                        int next = *(int *)(listBuf + 8 + iter * 4);
                        if (count <= next) break;
                        entry = *(float **)(*(int **)(listBuf + 0x40C) + next * 4);
                        *(int *)(listBuf + 8 + iter * 4) = next + 1;
                    }

                    if (entry) {
                        /* direction = vec - pos */
                        float dx = entry[3] - entry[0];
                        float dy = entry[4] - entry[1];
                        float dz = entry[5] - entry[2];
                        float dir[3] = {dx, dy, dz};
                        g_Vec3NormalizeAndScale(dir, 1.0f);

                        /* Double the direction */
                        dx = dir[0] + dir[0];
                        dy = dir[1] + dir[1];
                        dz = dir[2] + dir[2];

                        /* Set trajectory target = entry pos */
                        *(BYTE *)((char *)ball + 0xC3C) = 1;
                        *(float *)((char *)ball + 0xC40) = entry[0];
                        *(float *)((char *)ball + 0xC44) = entry[1] + *(float *)((char *)ball + 0x284) + 0.0078125f;
                        *(float *)((char *)ball + 0xC48) = entry[2];

                        /* For (B) branches, double direction again (4x total) */
                        if (isBranchB) {
                            dx += dx; dy += dy; dz += dz;
                        }

                        /* Set ball velocity */
                        int phys = ball[0x69];
                        if (phys && !IsBadReadPtr((void *)phys, 0xCB0)) {
                            *(float *)(phys + 0xCA4) = dx;
                            *(float *)(phys + 0xCA8) = dy;
                            *(float *)(phys + 0xCAC) = dz;
                        }
                    }
                }

                /* Free all allocated entries */
                {
                    int iter2 = g_AthenaListGetIterator(listBuf);
                    int *iterSlot = (int *)(listBuf + 8 + iter2 * 4);
                    *iterSlot = 0;
                    if (g_AthenaListGetSize(listBuf) >= 1) {
                        void *p = *(void **)(*(int **)(listBuf + 0x40C));
                        *iterSlot = 1;
                        while (p) {
                            /* Free is at 0x4BA74D (free) — but we don't have a pointer.
                             * The original game calls _free directly. We can use our
                             * operator's free or just leak (the game does this too
                             * if Vec3List_Free fails). For safety, use the game's free. */
                            typedef void (__cdecl *free_fn)(void *);
                            static free_fn g_free = NULL;
                            if (!g_free) g_free = (free_fn)(g_moduleBase + 0x000BA74D);
                            if (g_free) g_free(p);
                            int next = *iterSlot;
                            if (g_AthenaListGetSize(listBuf) <= next) break;
                            p = *(void **)(*(int **)(listBuf + 0x40C) + next * 4);
                            *iterSlot = next + 1;
                        }
                    }
                }
                /* Vec3List_Free cleans up the AthenaList itself */
                {
                    typedef void (__thiscall *Vec3List_Free_t)(void *list);
                    Vec3List_Free_t freeFn = (Vec3List_Free_t)g_Vec3ListFree;
                    if (freeFn) freeFn(listBuf);
                }
            }
        }
    }

    /* ── Toob: N:SPINNY ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_SPINNY)) && my_strnicmp(name, "N:SPINNY", 8) == 0) {
        if (g_RotatorAddBall && meshObj) g_RotatorAddBall((void *)meshObj, (int)ball);
    }

    /* ── Toob: N:SAWTEETH ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_SAWTEETH)) && my_strnicmp(name, "N:SAWTEETH", 10) == 0) {
        if (meshObj && *(int *)(meshObj + 0x10F4) == 0 && *(int *)((char *)ball + 0x7DC) < 1) {
            *(int *)((char *)ball + 0x7DC) = 0x32;
            float vx = *(float *)(meshObj + 0x1100);
            float vy = *(float *)(meshObj + 0x1104);
            float vz = *(float *)(meshObj + 0x1108);
            float dir[3] = {vx, vy, vz};
            g_Vec3NormalizeAndScale(dir, 3.0f);
            int phys = ball[0x69];
            if (phys && !IsBadReadPtr((void *)phys, 0xCB0)) {
                *(float *)(phys + 0xCA4) = dir[0];
                *(float *)(phys + 0xCA8) = dir[1];
                *(float *)(phys + 0xCAC) = dir[2];
            }
            if (g_BallApplyTrajectory) g_BallApplyTrajectory((int)ball);
        }
    }

    /* ── Master: N:SPINNER ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_SPINNER)) && my_strnicmp(name, "N:SPINNER", 9) == 0) {
        if (g_SpinnerActivate && meshObj) g_SpinnerActivate((void *)meshObj, (int)ball);
    }

    /* ── Master: E:LAUNCH ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_LAUNCH)) && my_stricmp(name, "E:LAUNCH") == 0) {
        float launchPos[3] = {0,0,0};
        if (g_AthenaHashTableLookup) {
            void *ht = *(void **)((char *)board + BOARD_MESHWORLD);
            if (ht) g_AthenaHashTableLookup(ht, launchPos, "LAUNCHPOINT", NULL);
        }
        *(float *)((char *)ball + 0x164) = launchPos[0];
        *(float *)((char *)ball + 0x168) = launchPos[1];
        *(float *)((char *)ball + 0x16C) = launchPos[2];
        int phys = ball[0x69];
        if (phys && !IsBadReadPtr((void *)phys, 0xCB0)) {
            *(float *)(phys + 0xCA4) = 0.0f;
            *(DWORD *)(phys + 0xCA8) = 0x42200000;
            *(DWORD *)(phys + 0xCAC) = 0;
        }
        ball[200] = 0x19;
        ball[0xA7] = 0x3B03126F;
        ball[0xA8] = 5;
        *(int *)((char *)ball + 0x808) = 0x32;
        if (g_BallDizzyImmunity) g_BallDizzyImmunity(ball, 200);
        if (g_SoundPlay3D && app) {
            DWORD snd = *(DWORD *)(app + 0x500);
            if (snd) g_SoundPlay3D((void *)snd, launchPos[0], launchPos[1], launchPos[2]);
        }
        /* Create explosion particles */
        if (g_operatorNew && g_ArenaScoreParticleCtor && g_AthenaListAppend && g_WaveCos && g_WaveSin && g_CPUIDRNG) {
            float explodePos[3] = {0,0,0};
            if (g_AthenaHashTableLookup) {
                void *ht = *(void **)((char *)board + BOARD_MESHWORLD);
                if (ht) g_AthenaHashTableLookup(ht, explodePos, "EXPLODEHELPER", NULL);
            }
            int p;
            for (p = 0; p < 0x168; p += 5) {
                void *mem = g_operatorNew(0x28);
                if (!mem) break;
                DWORD *particle = (DWORD *)g_ArenaScoreParticleCtor(mem, app);
                if (!particle) continue;
                float angle = (float)p;
                float cosV = g_WaveCos((void *)0x4F7188, angle);
                float sinV = g_WaveSin((void *)0x4F7188, angle);
                *(float *)(particle + 2) = sinV * 0.375f + explodePos[0];
                *(float *)(particle + 3) = explodePos[1] + 0.4f;
                *(float *)(particle + 4) = cosV * 0.375f + explodePos[2];
                cosV = g_WaveCos((void *)0x4F7188, angle);
                sinV = g_WaveSin((void *)0x4F7188, angle);
                *(float *)(particle + 5) = sinV * 0.1f;
                *(DWORD *)(particle + 6) = 0;
                *(float *)(particle + 7) = cosV * 0.1f;
                int speedDiv = RNG_call((void *)0x4F7360, 0, 0x14, 0) + 0x14;
                float scale = 0.00390625f / (float)speedDiv;
                *(float *)(particle + 5) *= scale;
                *(float *)(particle + 6) *= scale;
                *(float *)(particle + 7) *= scale;
                g_AthenaListAppend((void *)((char *)board + UNI_PARTICLE_LIST), (int)particle);
            }
        }
    }

    /* ── Wobbly: N:SQUAREWOBBLY ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_SQUAREWOBBLY)) && my_strnicmp(name, "N:SQUAREWOBBLY", 14) == 0) {
        if ((char)*(int *)((char *)ball + 0x768) && g_SquareWobblyActivate && meshObj)
            g_SquareWobblyActivate((void *)meshObj, (int)ball);
    }

    /* ── Wobbly: N:WAVY ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_WAVY)) && my_strnicmp(name, "N:WAVY", 6) == 0) {
        if ((char)*(int *)((char *)ball + 0x768) && g_WavyActivate && meshObj)
            g_WavyActivate((void *)meshObj, (int)ball);
    }

    /* ── Glass: N:GLASS ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_GLASS)) && my_strnicmp(name, "N:GLASS", 7) == 0) {
        *(int *)((char *)ball + 0xC5C) = 0xF;
    }

    /* ── Glass: N:TENBONUS1 / N:TENBONUS2 ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_TENBONUS1)) && my_strnicmp(name, "N:TENBONUS1", 11) == 0) {
        int phys = ball[0x69];
        if (phys && !IsBadReadPtr((void *)phys, 0xCB0)) {
            float speed = sqrtf(*(float*)(phys+0xCA4)**(float*)(phys+0xCA4) + *(float*)(phys+0xCA8)**(float*)(phys+0xCA8) + *(float*)(phys+0xCAC)**(float*)(phys+0xCAC));
            if (speed >= 2.0f && *(char *)((char *)ext + REND_GLASS_FLAG1) == 0) {
                *(BYTE *)((char *)ext + REND_GLASS_FLAG1) = 1;
                if (g_SoundPlay3D && app) {
                    DWORD snd = *(DWORD *)(app + 0x52C);
                    if (snd) g_SoundPlay3D((void *)snd, *(float*)((char*)ext+REND_GLASS_S1_X), *(float*)((char*)ext+REND_GLASS_S1_Y), *(float*)((char*)ext+REND_GLASS_S1_Z));
                }
                if (app) {
                    int gameMode = *(int *)(app + 0x220);
                    if (gameMode && *(char *)(gameMode + 0x10) == 0 && *(char *)(gameMode + 0x11) == 0) {
                        *(DWORD *)(ball[6]*0xA0 + 0x5EC + app) = 1000;
                        if (g_operatorNew && g_ScoreObjectCtor && g_TimerDecrement && g_AthenaListAppend) {
                            void *mem = g_operatorNew(0x30);
                            if (mem) {
                                int so = g_ScoreObjectCtor(mem, (int)board, ball[6]*0xA0 + 0x5CC + app, "EXTRA TIME:");
                                g_TimerDecrement(so);
                                g_AthenaListAppend((void *)((char *)board + UNI_SCORE_LIST), so);
                            }
                        }
                    }
                }
            }
        }
    }
    if ((*(BYTE*)((char*)ext + COLL_FLAG_TENBONUS2)) && my_strnicmp(name, "N:TENBONUS2", 11) == 0) {
        int phys = ball[0x69];
        if (phys && !IsBadReadPtr((void *)phys, 0xCB0)) {
            float speed = sqrtf(*(float*)(phys+0xCA4)**(float*)(phys+0xCA4) + *(float*)(phys+0xCA8)**(float*)(phys+0xCA8) + *(float*)(phys+0xCAC)**(float*)(phys+0xCAC));
            if (speed >= 2.0f && *(char *)((char *)ext + REND_GLASS_FLAG2) == 0) {
                *(BYTE *)((char *)ext + REND_GLASS_FLAG2) = 1;
                if (g_SoundPlay3D && app) {
                    DWORD snd = *(DWORD *)(app + 0x52C);
                    if (snd) g_SoundPlay3D((void *)snd, *(float*)((char*)ext+REND_GLASS_S2_X), *(float*)((char*)ext+REND_GLASS_S2_Y), *(float*)((char*)ext+REND_GLASS_S2_Z));
                }
                if (app) {
                    int gameMode = *(int *)(app + 0x220);
                    if (gameMode && *(char *)(gameMode + 0x10) == 0 && *(char *)(gameMode + 0x11) == 0) {
                        *(DWORD *)(ball[6]*0xA0 + 0x5EC + app) = 1000;
                        if (g_operatorNew && g_ScoreObjectCtor && g_TimerDecrement && g_AthenaListAppend) {
                            void *mem = g_operatorNew(0x30);
                            if (mem) {
                                int so = g_ScoreObjectCtor(mem, (int)board, ball[6]*0xA0 + 0x5CC + app, "EXTRA TIME:");
                                g_TimerDecrement(so);
                                g_AthenaListAppend((void *)((char *)board + UNI_SCORE_LIST), so);
                            }
                        }
                    }
                }
            }
        }
    }

    /* ── Sky: E:PEGS ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_PEGS)) && my_stricmp(name, "E:PEGS") == 0) {
        if (*(int *)((char *)ball + 0x788) == 0) {
            *(int *)((char *)ext + UNI_PEG_COUNT) += 1;
            *(int *)((char *)ball + 0x788) = 1;
        }
    }

    /* ── Sky: E:TRAPPOP ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_TRAPPOP)) && my_stricmp(name, "E:TRAPPOP") == 0) {
        if (difficulty != 0 && g_RotatorStartSound) {
            int trapObj = *(int *)((char *)ext + UNI_NEON_TRAPDOOR);
            if (trapObj) g_RotatorStartSound(trapObj);
        }
    }

    /* ── Sky: E:NOPEGS ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_NOPEGS)) && my_stricmp(name, "E:NOPEGS") == 0) {
        if (*(int *)((char *)ball + 0x78C) == 0) {
            *(int *)((char *)ext + UNI_PEG_COUNT) -= 1;
            *(int *)((char *)ball + 0x78C) = 1;
        }
    }

    /* ── Sky: E:HEATON ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_HEATON)) && my_stricmp(name, "E:HEATON") == 0) {
        if (difficulty != 0 && g_AthenaListContainsValue && g_PendulumAddIndex) {
            DWORD mgObj = *(DWORD *)((char *)ext + UNI_MAGNIFYING_GLASS);
            if (mgObj) {
                if (!g_AthenaListContainsValue((void *)(mgObj + 0x2C), (int)ball))
                    g_PendulumAddIndex((void *)mgObj, (int)ball);
            }
        }
    }

    /* ── Sky: E:HEATOFF ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_HEATOFF)) && my_stricmp(name, "E:HEATOFF") == 0) {
        if (difficulty != 0 && g_AthenaListRemoveByValue) {
            DWORD mgObj = *(DWORD *)((char *)ext + UNI_MAGNIFYING_GLASS);
            if (mgObj) g_AthenaListRemoveByValue((void *)(mgObj + 0x2C), (int)ball);
        }
    }

    /* ── Impossible: N:BOUNCE ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_BOUNCE)) && my_strnicmp(name, "N:BOUNCE", 8) == 0) {
        if ((char)*(int *)((char *)ball + 0x768)) {
            int phys = ball[0x69];
            if (phys && !IsBadReadPtr((void *)phys, 0xCB0)) {
                float vx = *(float *)(phys + 0xCA4) * 2.0f;
                float vz = *(float *)(phys + 0xCAC) * 2.0f;
                float vy = 0.0f;
                float speed = sqrtf(vx*vx + vz*vz);
                if (speed < 1.25f) g_Vec3NormalizeAndScale((float[]){vx,vy,vz}, 1.25f);
                speed = sqrtf(vx*vx + vy*vy + vz*vz);
                if (speed > 3.0f) g_Vec3NormalizeAndScale((float[]){vx,vy,vz}, 3.0f);
                *(float *)(phys + 0xCA4) = vx;
                *(float *)(phys + 0xCA8) = vy;
                *(float *)(phys + 0xCAC) = vz;
            }
        }
    }

    /* ── Impossible: N:ONROTATOR ──
     * Original gates on (char)ball[0x768] != 0. */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_ONROTATOR)) && my_strnicmp(name, "N:ONROTATOR", 11) == 0) {
        if ((char)*(int *)((char *)ball + 0x768) && g_RotatorAddBall && meshObj)
            g_RotatorAddBall((void *)meshObj, (int)ball);
    }

    /* ── Impossible: N:ONGEAR ──
     * Original gates on (char)ball[0x768] != 0. */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_ONGEAR)) && my_strnicmp(name, "N:ONGEAR", 8) == 0) {
        if ((char)*(int *)((char *)ball + 0x768) && g_GearAddBall && meshObj)
            g_GearAddBall((void *)meshObj, (int)ball);
    }

call_global:
    /* Call the original DispatchCollisionEvents (via trampoline) for global events */
    if (g_OriginalDispatch) g_OriginalDispatch(board, ball, collPair);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Universal Post-Setup — config-driven feature initialization
 * ═══════════════════════════════════════════════════════════════════════════ */

static void UniversalPostSetup(void *board) {
    void* ext = EnsureBoardExt(board);
    if (!ext) return;
    int level = GetCurrentLevel(board);
    if (level == 0) return;

    // Option B: LevelFeatures.txt deprecated — no LoadConfig, pure S1-driven
    // Bridge init is now S1-driven via ScanS1AndAutoEnable + lazy S1Ensure in CreateDynamicObjects
    // Do not call InitBridge here; CreateDynamicObjects handles BRIDGE/BUMPERS on demand
    // Keep bumpers S1-driven: scan for N:BUMPER presence
    int hasBumper = (*(BYTE*)((char*)ext + COLL_FLAG_BUMPER));
    if (hasBumper) {
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

            void *dest = (char *)ext + BUMPER_SLOT_BASE + i * BUMPER_SLOT_STRIDE;

            if (g_AthenaListInit) {
                g_AthenaListInit(dest, 0);
            }
            if (g_CollectByNameFilter) {
                g_CollectByNameFilter((void *)meshWorld, nameBuf, dest);
            }
            *(DWORD *)((char *)ext + BUMPER_LIT_BASE + i * BUMPER_LIT_STRIDE) = 0;
        }
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Universal Scene Constructor — REPLACES vtable[0x48] (Scene_LoadLevel*)
 * ═══════════════════════════════════════════════════════════════════════════ */

/* S1-driven auto-enable — makes any MESHWORLD file-swappable.
 * Scans meshWorld+0x480 S1 refs after Level_MeshWorldCtor and OrBoardFeat for
 * bridge/swirl/windmill so a Dizzy MESHWORLD dropped into WarmUp slot
 * auto-enables its features without LevelData.txt edit. Heap size is
 * fixed 0xC000, so no dynamic sizing needed — just feat bits. */
static void AddS1CollisionToExt(void *board, void *ext, const char *name) {
    if (!board || !ext || !name) return;
    if (name[0]!='N' && name[0]!='E') return;
    if (name[1]!=':') return;
    // dedup
    int count = *(int*)((char*)ext + OFF_COLLISION_COUNT);
    if (count <0 || count >= MAX_S1_COLLISIONS) { if (count>=MAX_S1_COLLISIONS) DebugLog("AddS1Collision: overflow, dropping"); return; }
    char *base = (char*)ext + OFF_COLLISION_NAMES;
    for (int i=0;i<count;i++) if (my_stricmp(base + i*S1_COLLISION_NAME_LEN, name)==0) return;
    my_strncpy(base + count*S1_COLLISION_NAME_LEN, name, S1_COLLISION_NAME_LEN);
    *(int*)((char*)ext + OFF_COLLISION_COUNT) = count+1;
}
static int IsS1CollisionEnabled(void *board, const char *eventName) {
    if (!board || !eventName) return 0;
    void *ext = GetBoardExt(board);
    if (!ext) return 0;
    int count = *(int*)((char*)ext + OFF_COLLISION_COUNT);
    if (count<=0 || count>MAX_S1_COLLISIONS) return 0;
    char *base = (char*)ext + OFF_COLLISION_NAMES;
    // exact match
    for (int i=0;i<count;i++) if (my_stricmp(base + i*S1_COLLISION_NAME_LEN, eventName)==0) return 1;
    // generic prefix match: stored variant vs canonical event (E:BRANCH(A) vs E:BRANCH)
    // covers N:BUMPER1, E:BRANCH(A)/(B), E:SCORE*, N:NEONPLATFORM*, N:SPINNY*, etc.
    // dispatch uses my_strnicmp(name, event, len)==0, so stored "E:BRANCH(A)" must enable canonical "E:BRANCH"
    { int elen=0; while(eventName[elen]) elen++;
      for (int i=0;i<count;i++) if (my_strnicmp(base + i*S1_COLLISION_NAME_LEN, eventName, elen)==0) return 1;
    }
    return 0;
}
static void BuildCollisionFlags(void *board, void *ext) {
    if (!board || !ext) return;
    // Clear flags
    memset((char*)ext + OFF_COLLISION_FLAGS, 0, COLL_FLAG_COUNT);
    // Set each based on IsS1CollisionEnabled (called once per event at load, not per collision)
    if (IsS1CollisionEnabled(board, "N:BUMPER")) *(BYTE*)((char*)ext + COLL_FLAG_BUMPER)=1;
    if (IsS1CollisionEnabled(board, "N:BRIDGE")) *(BYTE*)((char*)ext + COLL_FLAG_BRIDGE)=1;
    if (IsS1CollisionEnabled(board, "N:WATERWHEEL")) *(BYTE*)((char*)ext + COLL_FLAG_WATERWHEEL)=1;
    if (IsS1CollisionEnabled(board, "N:WHEELEMBED")) *(BYTE*)((char*)ext + COLL_FLAG_WHEELEMBED)=1;
    if (IsS1CollisionEnabled(board, "N:SWIRL")) *(BYTE*)((char*)ext + COLL_FLAG_SWIRL)=1;
    if (IsS1CollisionEnabled(board, "E:CATAPULTBOTTOM")) *(BYTE*)((char*)ext + COLL_FLAG_CATAPULTBOTTOM)=1;
    if (IsS1CollisionEnabled(board, "E:OPENSESAME")) *(BYTE*)((char*)ext + COLL_FLAG_OPENSESAME)=1;
    if (IsS1CollisionEnabled(board, "N:TRAPDOOR")) *(BYTE*)((char*)ext + COLL_FLAG_TRAPDOOR)=1;
    if (IsS1CollisionEnabled(board, "E:BITE")) *(BYTE*)((char*)ext + COLL_FLAG_BITE)=1;
    if (IsS1CollisionEnabled(board, "E:MACETRIGGER")) *(BYTE*)((char*)ext + COLL_FLAG_MACETRIGGER)=1;
    if (IsS1CollisionEnabled(board, "N:MACE")) *(BYTE*)((char*)ext + COLL_FLAG_MACE)=1;
    if (IsS1CollisionEnabled(board, "E:HELPINERTIA")) *(BYTE*)((char*)ext + COLL_FLAG_HELPINERTIA)=1;
    if (IsS1CollisionEnabled(board, "E:UNHELPINERTIA")) *(BYTE*)((char*)ext + COLL_FLAG_UNHELPINERTIA)=1;
    if (IsS1CollisionEnabled(board, "E:VACPOPOUT")) *(BYTE*)((char*)ext + COLL_FLAG_VACPOPOUT)=1;
    if (IsS1CollisionEnabled(board, "N:SPEEDCYLINDER")) *(BYTE*)((char*)ext + COLL_FLAG_SPEEDCYLINDER)=1;
    if (IsS1CollisionEnabled(board, "N:EXTRATIME")) *(BYTE*)((char*)ext + COLL_FLAG_EXTRATIME)=1;
    if (IsS1CollisionEnabled(board, "N:NEONPLATFORM")) *(BYTE*)((char*)ext + COLL_FLAG_NEONPLATFORM)=1;
    if (IsS1CollisionEnabled(board, "E:ZOOP")) *(BYTE*)((char*)ext + COLL_FLAG_ZOOP)=1;
    if (IsS1CollisionEnabled(board, "E:LIGHTSOFF")) *(BYTE*)((char*)ext + COLL_FLAG_LIGHTSOFF)=1;
    if (IsS1CollisionEnabled(board, "E:LIGHTSON")) *(BYTE*)((char*)ext + COLL_FLAG_LIGHTSON)=1;
    if (IsS1CollisionEnabled(board, "E:CALLHAMMER")) *(BYTE*)((char*)ext + COLL_FLAG_CALLHAMMER)=1;
    if (IsS1CollisionEnabled(board, "E:HAMMERCHASE")) *(BYTE*)((char*)ext + COLL_FLAG_HAMMERCHASE)=1;
    if (IsS1CollisionEnabled(board, "E:ALERTSAW1")) *(BYTE*)((char*)ext + COLL_FLAG_ALERTSAW1)=1;
    if (IsS1CollisionEnabled(board, "E:ALERTSAW2")) *(BYTE*)((char*)ext + COLL_FLAG_ALERTSAW2)=1;
    if (IsS1CollisionEnabled(board, "E:ALERTSAW3")) *(BYTE*)((char*)ext + COLL_FLAG_ALERTSAW3)=1;
    if (IsS1CollisionEnabled(board, "E:ACTIVATESAW1")) *(BYTE*)((char*)ext + COLL_FLAG_ACTIVATESAW1)=1;
    if (IsS1CollisionEnabled(board, "E:ACTIVATESAW2")) *(BYTE*)((char*)ext + COLL_FLAG_ACTIVATESAW2)=1;
    if (IsS1CollisionEnabled(board, "E:ALERTJUDGES")) *(BYTE*)((char*)ext + COLL_FLAG_ALERTJUDGES)=1;
    if (IsS1CollisionEnabled(board, "E:SCORE")) *(BYTE*)((char*)ext + COLL_FLAG_SCORE)=1;
    if (IsS1CollisionEnabled(board, "E:BELL")) *(BYTE*)((char*)ext + COLL_FLAG_BELL)=1;
    if (IsS1CollisionEnabled(board, "E:GRAVITY")) *(BYTE*)((char*)ext + COLL_FLAG_GRAVITY)=1;
    if (IsS1CollisionEnabled(board, "N:JUMPFIRST")) *(BYTE*)((char*)ext + COLL_FLAG_JUMPFIRST)=1;
    if (IsS1CollisionEnabled(board, "N:JUMPSECOND")) *(BYTE*)((char*)ext + COLL_FLAG_JUMPSECOND)=1;
    if (IsS1CollisionEnabled(board, "E:SHRINK")) *(BYTE*)((char*)ext + COLL_FLAG_SHRINK)=1;
    if (IsS1CollisionEnabled(board, "E:GROWSOUND")) *(BYTE*)((char*)ext + COLL_FLAG_GROWSOUND)=1;
    if (IsS1CollisionEnabled(board, "E:GROW")) *(BYTE*)((char*)ext + COLL_FLAG_GROW)=1;
    if (IsS1CollisionEnabled(board, "E:DROPLIFT")) *(BYTE*)((char*)ext + COLL_FLAG_DROPLIFT)=1;
    if (IsS1CollisionEnabled(board, "E:PIPERANDOM")) *(BYTE*)((char*)ext + COLL_FLAG_PIPERANDOM)=1;
    if (IsS1CollisionEnabled(board, "E:LIMIT")) *(BYTE*)((char*)ext + COLL_FLAG_LIMIT)=1;
    if (IsS1CollisionEnabled(board, "E:LIMITX")) *(BYTE*)((char*)ext + COLL_FLAG_LIMITX)=1;
    if (IsS1CollisionEnabled(board, "E:LIMITZ")) *(BYTE*)((char*)ext + COLL_FLAG_LIMITZ)=1;
    if (IsS1CollisionEnabled(board, "E:LIMITPIPE1")) *(BYTE*)((char*)ext + COLL_FLAG_LIMITPIPE1)=1;
    if (IsS1CollisionEnabled(board, "E:SWALLOW")) *(BYTE*)((char*)ext + COLL_FLAG_SWALLOW)=1;
    if (IsS1CollisionEnabled(board, "E:LIMITPIPE2")) *(BYTE*)((char*)ext + COLL_FLAG_LIMITPIPE2)=1;
    if (IsS1CollisionEnabled(board, "E:BRANCH")) *(BYTE*)((char*)ext + COLL_FLAG_BRANCH)=1;
    if (IsS1CollisionEnabled(board, "N:SPINNY")) *(BYTE*)((char*)ext + COLL_FLAG_SPINNY)=1;
    if (IsS1CollisionEnabled(board, "N:SAWTEETH")) *(BYTE*)((char*)ext + COLL_FLAG_SAWTEETH)=1;
    if (IsS1CollisionEnabled(board, "N:SPINNER")) *(BYTE*)((char*)ext + COLL_FLAG_SPINNER)=1;
    if (IsS1CollisionEnabled(board, "E:LAUNCH")) *(BYTE*)((char*)ext + COLL_FLAG_LAUNCH)=1;
    if (IsS1CollisionEnabled(board, "N:SQUAREWOBBLY")) *(BYTE*)((char*)ext + COLL_FLAG_SQUAREWOBBLY)=1;
    if (IsS1CollisionEnabled(board, "N:WAVY")) *(BYTE*)((char*)ext + COLL_FLAG_WAVY)=1;
    if (IsS1CollisionEnabled(board, "N:GLASS")) *(BYTE*)((char*)ext + COLL_FLAG_GLASS)=1;
    if (IsS1CollisionEnabled(board, "N:TENBONUS1")) *(BYTE*)((char*)ext + COLL_FLAG_TENBONUS1)=1;
    if (IsS1CollisionEnabled(board, "N:TENBONUS2")) *(BYTE*)((char*)ext + COLL_FLAG_TENBONUS2)=1;
    if (IsS1CollisionEnabled(board, "E:PEGS")) *(BYTE*)((char*)ext + COLL_FLAG_PEGS)=1;
    if (IsS1CollisionEnabled(board, "E:TRAPPOP")) *(BYTE*)((char*)ext + COLL_FLAG_TRAPPOP)=1;
    if (IsS1CollisionEnabled(board, "E:NOPEGS")) *(BYTE*)((char*)ext + COLL_FLAG_NOPEGS)=1;
    if (IsS1CollisionEnabled(board, "E:HEATON")) *(BYTE*)((char*)ext + COLL_FLAG_HEATON)=1;
    if (IsS1CollisionEnabled(board, "E:HEATOFF")) *(BYTE*)((char*)ext + COLL_FLAG_HEATOFF)=1;
    if (IsS1CollisionEnabled(board, "N:BOUNCE")) *(BYTE*)((char*)ext + COLL_FLAG_BOUNCE)=1;
    if (IsS1CollisionEnabled(board, "N:ONROTATOR")) *(BYTE*)((char*)ext + COLL_FLAG_ONROTATOR)=1;
    if (IsS1CollisionEnabled(board, "N:ONGEAR")) *(BYTE*)((char*)ext + COLL_FLAG_ONGEAR)=1;
    { char dbg[96]; int cnt=0; for(int i=0;i<COLL_FLAG_COUNT;i++) if(*(BYTE*)((char*)ext+OFF_COLLISION_FLAGS+i)) cnt++; wsprintfA(dbg,"BuildCollisionFlags: %d/%d enabled",cnt,COLL_FLAG_COUNT); DebugLog(dbg); }
}

static void ScanS1AndAutoEnable(void *board, void *ext, void *meshWorld) {
    if (!board || !ext || !meshWorld) return;
    MEMORY_BASIC_INFORMATION mbi;
    if (!VirtualQuery(meshWorld, &mbi, sizeof(mbi)) || mbi.State!=MEM_COMMIT) return;
    /* meshWorld+0x480 must be readable */
    DWORD objDb = *(DWORD *)((char *)meshWorld + 0x480);
    if (!objDb) return;
    if (!VirtualQuery((void*)objDb, &mbi, sizeof(mbi)) || mbi.State!=MEM_COMMIT) return;
    if (!g_AthenaListGetIterator || !g_AthenaListGetSize) return;
    /* Validate AthenaList header without IsBad* */
    if (!VirtualQuery((void*)(objDb + 0x894), &mbi, sizeof(mbi)) || mbi.State!=MEM_COMMIT) return;
    int iter = g_AthenaListGetIterator((void *)(objDb + 0x894));
    if (iter <0 || iter>16) return;
    if (!VirtualQuery((void*)(objDb + 0x89C + iter*4), &mbi, sizeof(mbi)) || mbi.State!=MEM_COMMIT) return;
    DWORD savedIter = *(DWORD*)(objDb + 0x89C + iter*4);
    *(DWORD *)(objDb + 0x89C + iter*4) = 0;
    if (!VirtualQuery((void*)(objDb + 0x898), &mbi, sizeof(mbi)) || mbi.State!=MEM_COMMIT) { *(DWORD*)(objDb + 0x89C + iter*4)=savedIter; return; }
    int count = *(int *)(objDb + 0x898);
    if (count <=0 || count>8192) return;
    if (!VirtualQuery((void*)(objDb + 0xCA0), &mbi, sizeof(mbi)) || mbi.State!=MEM_COMMIT) return;
    DWORD *array = *(DWORD **)(objDb + 0xCA0);
    if (!array) return;
    if (!VirtualQuery(array, &mbi, sizeof(mbi)) || mbi.State!=MEM_COMMIT) return;
    /* Quick sanity: array should hold count pointers */
    *(DWORD *)(objDb + 0x89C + iter*4) = 1;
    int idx=0;
    // Scan ALL entries (not capped at 64) — large custom levels may have windmill late
    int scan = count;
    for (idx=0; idx<scan; idx++) {
        DWORD *obj = (DWORD *)array[idx];
        if (!obj) continue;
        if (!VirtualQuery(obj, &mbi, sizeof(mbi)) || mbi.State!=MEM_COMMIT) continue;
        char *name = *(char **)obj;
        if (!name) continue;
        if (!VirtualQuery(name, &mbi, sizeof(mbi)) || mbi.State!=MEM_COMMIT) continue;
        // name is S1 ref string, e.g. "Levels\\Level3-WaterWheel" or "BRIDGE" etc.
        // Phase1 full registry: every S1 prefix auto-enables its feat / ensures mesh
        if (my_strnicmp(name, "BRIDGE", 6)==0 || my_strnicmp(name, "BBRIDGE", 7)==0) {
            OrBoardFeat(board, FEAT_BRIDGE_ANIM);
        }
        if (my_strnicmp(name, "WATERWHEEL", 10)==0 || my_strnicmp(name, "WHEELEMBED", 10)==0 || my_strnicmp(name, "SWIRL", 5)==0 || my_strnicmp(name, "TarBubble", 9)==0 || my_strnicmp(name, "GLUEBIE", 7)==0 || my_strnicmp(name, "TIPPER", 6)==0) {
            OrBoardFeat(board, FEAT_SWIRL);
        }
        if (my_strnicmp(name, "WINDMILL", 8)==0 || my_strnicmp(name, "CHOMPER", 7)==0 || my_strnicmp(name, "TURRET", 6)==0 || my_strnicmp(name, "CATAPULT", 8)==0 || my_strnicmp(name, "MACE", 4)==0 || my_strnicmp(name, "DRAWBRIDGE", 10)==0 || my_strnicmp(name, "TRAPDOOR", 8)==0) {
            OrBoardFeat(board, FEAT_WINDMILL);
        }
        if (my_strnicmp(name, "WOBBLY", 6)==0 || my_strnicmp(name, "WAVY", 4)==0) {
            // Wobbly family — no feat flag but S1 presence proves level intent
        }
        if (my_strnicmp(name, "POPCYLINDER", 11)==0 || my_strnicmp(name, "POPDOOR", 7)==0 || my_strnicmp(name, "CLOUDSCAPE", 10)==0) {
            OrBoardFeat(board, FEAT_SKY_POPCYL);
        }
        if (my_strnicmp(name, "BONK", 4)==0 || my_strnicmp(name, "FAN", 3)==0 || my_strnicmp(name, "SAWBLADE", 8)==0 || my_strnicmp(name, "BELL", 4)==0 || my_strnicmp(name, "JUDGE", 5)==0 || my_strnicmp(name, "SPINNER", 7)==0 || my_strnicmp(name, "LOOPER", 6)==0 || my_strnicmp(name, "GEAR", 4)==0 || my_strnicmp(name, "PENDULUM", 8)==0 || my_strnicmp(name, "ROTATOR", 7)==0) {
            // Expert/Impossible family — handled via S1Ensure* in CreateDynamicObjects
        }
        if (my_strnicmp(name, "CLOUDSCAPE", 10)==0) {
            S1EnsureSprite(board, ext, REND_SKY_SPRITE, "textures\\\\clouds.png");
        }
        // Option B: every N:/E: S1 ref auto-enables its collision event for this board
        if ((name[0]=='N' || name[0]=='E') && name[1]==':') {
            AddS1CollisionToExt(board, ext, name);
        }
    }
    *(DWORD*)(objDb + 0x89C + iter*4)=savedIter;
    char dbg[128];
    DWORD feat = GetBoardFeat(board);
    wsprintfA(dbg, "S1 scan: count=%d feat=0x%X", count, feat);
    DebugLog(dbg);
}

/* ── File byte-scan for N:/E: hidden in sub-meshes ──
 * Some N:/E: collision meshes live inside sub-mesh .MESHWORLD/.MESH
 * files (e.g. Level3-WaterWheel) not in the main level S1/S6.
 * User requested: scan ALL mesh/meshworld files in the level folder.
 * Generic extractor: finds N:XXX / E:XXX tokens in raw bytes and
 * AddS1CollisionToExt for this board. Dedup handled by AddS1.
 * Called once per level load, not per frame. */
static int IsCollNameChar(char c) {
    return (c>='A'&&c<='Z')||(c>='a'&&c<='z')||(c>='0'&&c<='9')||c=='_'||c==':'||c=='-'||c=='('||c==')';
}
static void ScanFileForCollisions(void *board, void *ext, const char *filePath) {
    if (!board || !ext || !filePath || !filePath[0]) return;
    HANDLE h = CreateFileA(filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h==INVALID_HANDLE_VALUE) return;
    DWORD fsize = GetFileSize(h, NULL);
    if (fsize==0 || fsize==(DWORD)-1 || fsize>0x500000) { CloseHandle(h); return; }
    char *buf = (char*)HeapAlloc(GetProcessHeap(), 0, fsize);
    if (!buf) { CloseHandle(h); return; }
    DWORD br=0; if (!ReadFile(h, buf, fsize, &br, NULL) || br!=fsize) { HeapFree(GetProcessHeap(),0,buf); CloseHandle(h); return; }
    CloseHandle(h);
    int added=0;
    for (DWORD i=0; i+2 < fsize; ) {
        if ((buf[i]=='N' || buf[i]=='E') && buf[i+1]==':' && IsCollNameChar(buf[i+2])) {
            DWORD j=0;
            while (i+j < fsize && j < 31 && IsCollNameChar(buf[i+j])) j++;
            if (j>=3 && j<=31) {
                char name[32]; int k; for (k=0;k<(int)j && k<31;k++) name[k]=buf[i+k];
                name[k]='\0';
                // validate: at least N:X or E:XX and not just prefix
                if ((name[0]=='N' || name[0]=='E') && name[1]==':' && name[2]) {
                    int before = (int)*(int*)((char*)ext + OFF_COLLISION_COUNT);
                    AddS1CollisionToExt(board, ext, name);
                    int after = (int)*(int*)((char*)ext + OFF_COLLISION_COUNT);
                    if (after>before) added++;
                }
            }
            i += (j?j:1);
        } else i++;
    }
    HeapFree(GetProcessHeap(),0,buf);
    if (added) { char lg[96]; wsprintfA(lg, "FileScan: '%s' +%d N:/E:", filePath, added); DebugLog(lg); }
}
// Targeted scan: only scan mesh files actually referenced by S1 (not whole folder wildcard).
// Prevents hitch from scanning 30+ files every level load. Falls back to basePath file.
static void ScanS1ReferencedMeshesForCollisions(void *board, void *ext, void *meshWorld, const char *basePath) {
    if (!board || !ext) return;
    // Always scan the main level file itself
    if (basePath && basePath[0]) {
        ScanFileForCollisions(board, ext, basePath);
        char withExt[MAX_PATH]; strcpy(withExt, basePath);
        int l=strlen(withExt);
        if (l+10 < MAX_PATH && my_strnicmp(withExt+l-10, ".MESHWORLD", 10)!=0 && my_strnicmp(withExt+l-5, ".MESH", 5)!=0) {
            strcat(withExt, ".MESHWORLD");
            ScanFileForCollisions(board, ext, withExt);
        }
    }
    if (!meshWorld) return;
    DWORD objDb = *(DWORD *)((char *)meshWorld + 0x480);
    if (!objDb) return;
    MEMORY_BASIC_INFORMATION mbi;
    if (!VirtualQuery((void*)objDb, &mbi, sizeof(mbi)) || mbi.State!=MEM_COMMIT) return;
    if (!g_AthenaListGetIterator || !g_AthenaListGetSize) return;
    if (!VirtualQuery((void*)(objDb + 0x894), &mbi, sizeof(mbi)) || mbi.State!=MEM_COMMIT) return;
    int iter = g_AthenaListGetIterator((void *)(objDb + 0x894));
    if (iter <0 || iter>16) return;
    if (!VirtualQuery((void*)(objDb + 0x89C + iter*4), &mbi, sizeof(mbi)) || mbi.State!=MEM_COMMIT) return;
    DWORD savedIter = *(DWORD*)(objDb + 0x89C + iter*4);
    *(DWORD *)(objDb + 0x89C + iter*4) = 0;
    if (!VirtualQuery((void*)(objDb + 0x898), &mbi, sizeof(mbi)) || mbi.State!=MEM_COMMIT) { *(DWORD*)(objDb + 0x89C + iter*4)=savedIter; return; }
    int count = *(int *)(objDb + 0x898);
    if (count <=0 || count>8192) { *(DWORD*)(objDb + 0x89C + iter*4)=savedIter; return; }
    if (!VirtualQuery((void*)(objDb + 0xCA0), &mbi, sizeof(mbi)) || mbi.State!=MEM_COMMIT) { *(DWORD*)(objDb + 0x89C + iter*4)=savedIter; return; }
    DWORD *array = *(DWORD **)(objDb + 0xCA0);
    if (!array || !VirtualQuery(array, &mbi, sizeof(mbi)) || mbi.State!=MEM_COMMIT) { *(DWORD*)(objDb + 0x89C + iter*4)=savedIter; return; }
    *(DWORD *)(objDb + 0x89C + iter*4) = 1;
    for (int idx=0; idx<count; idx++) {
        DWORD *obj = (DWORD *)array[idx];
        if (!obj || !VirtualQuery(obj, &mbi, sizeof(mbi)) || mbi.State!=MEM_COMMIT) continue;
        char *name = *(char **)obj;
        if (!name || !VirtualQuery(name, &mbi, sizeof(mbi)) || mbi.State!=MEM_COMMIT) continue;
        // Heuristic: S1 strings that look like file paths (contain slash or known prefix) and not N:/E: themselves
        if (name[0]=='N' && name[1]==':') continue;
        if (name[0]=='E' && name[1]==':') continue;
        int isPath = 0;
        for (char *p=name; *p; p++) if (*p=='\\' || *p=='/') { isPath=1; break; }
        if (!isPath) {
            if (my_strnicmp(name, "Levels\\", 7)==0) isPath=1;
            if (my_strnicmp(name, "Meshes\\", 7)==0) isPath=1;
            if (my_strnicmp(name, "levels\\", 7)==0) isPath=1;
        }
        if (!isPath) continue;
        // name is like "Levels\\Level3-WaterWheel" or "Meshes\\Chomper" etc. Scan that file.
        char tryPath[MAX_PATH];
        // If name already contains "levels\", use as-is; else prepend "levels\"
        if (my_strnicmp(name, "levels\\", 7)==0 || my_strnicmp(name, "levels/", 7)==0) {
            strcpy(tryPath, name);
        } else {
            // Many S1 refs are bare "Levels\\..." which maps to "levels\\..."
            // Normalize: ensure lower levels prefix
            if (my_strnicmp(name, "Levels\\", 7)==0) {
                strcpy(tryPath, "levels\\");
                strcat(tryPath, name+7);
            } else if (my_strnicmp(name, "Meshes\\", 7)==0) {
                strcpy(tryPath, "meshes\\");
                strcat(tryPath, name+7);
            } else {
                strcpy(tryPath, name);
            }
        }
        ScanFileForCollisions(board, ext, tryPath);
        // also try with .MESHWORLD extension if no dot
        if (!strchr(tryPath, '.')) {
            char withExt2[MAX_PATH]; strcpy(withExt2, tryPath); strcat(withExt2, ".MESHWORLD");
            ScanFileForCollisions(board, ext, withExt2);
            strcpy(withExt2, tryPath); strcat(withExt2, ".MESH");
            ScanFileForCollisions(board, ext, withExt2);
        }
        *(DWORD *)(objDb + 0x89C + iter*4) = idx+1;
    }
    *(DWORD *)(objDb + 0x89C + iter*4)=savedIter; // restore
}

/* DEPRECATED: wildcard folder sweep — replaced by ScanS1ReferencedMeshesForCollisions (targeted)
 * Kept as #if 0 to avoid future hitch; do not re-enable without profiling.
 */
#if 0
static void ScanLevelFolderForCollisions(void *board, void *ext, const char *basePath) {
    if (!board || !ext || !basePath || !basePath[0]) return;
    // extract dir from basePath (up to last slash)
    char dir[MAX_PATH]=""; const char *last=NULL; for (const char *q=basePath; *q; q++) if (*q=='\\' || *q=='/') last=q;
    if (last) { int len=(int)(last - basePath)+1; if (len>=MAX_PATH) len=MAX_PATH-1; strncpy(dir, basePath, len); dir[len]='\0'; }
    else { strcpy(dir, "levels\\"); }
    // also ensure dir ends with slash
    int dlen=strlen(dir); if (dlen && dir[dlen-1]!='\\' && dir[dlen-1]!='/') { if (dlen+1<MAX_PATH){dir[dlen]='\\';dir[dlen+1]='\0';} }
    // helper to scan wildcard in a given dir
    const char *pats[4]={"*.MESHWORLD","*.MESH","*.meshworld","*.mesh"};
    char globalDir[MAX_PATH]="levels\\";
    // scan requested dir
    for (int pi=0; pi<4; pi++) {
        char pat[MAX_PATH]; strcpy(pat, dir); strcat(pat, pats[pi]);
        WIN32_FIND_DATAA fd; HANDLE fh=FindFirstFileA(pat, &fd);
        if (fh==INVALID_HANDLE_VALUE) continue;
        do {
            if (fd.cFileName[0]=='.') continue;
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
            char full[MAX_PATH]; strcpy(full, dir); strcat(full, fd.cFileName);
            ScanFileForCollisions(board, ext, full);
        } while (FindNextFileA(fh, &fd));
        FindClose(fh);
    }
    // also scan global levels\ — always (secret objects + shared sub-meshes live there)
    if (my_stricmp(dir, globalDir)!=0) /* also scan global Levels folder for shared sub-meshes + secret objects */ {
        for (int pi=0; pi<4; pi++) {
            char pat[MAX_PATH]; strcpy(pat, globalDir); strcat(pat, pats[pi]);
            WIN32_FIND_DATAA fd; HANDLE fh=FindFirstFileA(pat, &fd);
            if (fh==INVALID_HANDLE_VALUE) continue;
            do {
                if (fd.cFileName[0]=='.') continue;
                if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
                char full[MAX_PATH]; strcpy(full, globalDir); strcat(full, fd.cFileName);
                ScanFileForCollisions(board, ext, full);
            } while (FindNextFileA(fh, &fd));
            FindClose(fh);
        }
    }
    // also explicitly scan the basePath file itself (covers bare-name resolves without slash)
    {
        char tryPath[MAX_PATH]; strcpy(tryPath, basePath);
        ScanFileForCollisions(board, ext, tryPath);
        // try with extension
        int tlen=strlen(tryPath);
        if (tlen+10 < MAX_PATH && my_strnicmp(tryPath+tlen-10, ".MESHWORLD", 10)!=0 && my_strnicmp(tryPath+tlen-5, ".MESH", 5)!=0) {
            strcat(tryPath, ".MESHWORLD");
            ScanFileForCollisions(board, ext, tryPath);
        }
    }
    char lg2[96]; int cnt=*(int*)((char*)ext + OFF_COLLISION_COUNT);
    wsprintfA(lg2, "FolderScan done dir='%s' total N:/E:=%d", dir, cnt); DebugLog(lg2);
}
#endif // 0 — ScanLevelFolderForCollisions deprecated

static void UniversalConstructor(void *board, int raceIndex) {
    void* ext = EnsureBoardExt(board);
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

    /* RaceFiles.txt overrides everything: Race 1..15 -> file. Then LevelData, then g_meshPaths.
     * Bare names like "LoopyRace" resolve to "levels\\LoopyRace\\LoopyRace" if that
     * subfolder file exists, otherwise fallback to "levels\\LoopyRace". */
    LoadRaceFiles();
    const char *rawPath = NULL;
    if (raceIndex>=1 && raceIndex<=15 && g_raceFiles[raceIndex][0]) rawPath = g_raceFiles[raceIndex];
    if (!rawPath || !*rawPath) rawPath = g_levelData[raceIndex].meshPath;
    if (!rawPath || !*rawPath) rawPath = g_meshPaths[raceIndex];
    if (!rawPath) return;
    char resolved[MAX_PATH]; ResolveRacePath(resolved, rawPath);
    const char *meshPath = resolved;
    {
        char dbg2[256]; wsprintfA(dbg2, "UniversalConstructor meshPath race=%d raw='%s' resolved='%s'", raceIndex, rawPath, meshPath);
        DebugLog(dbg2);
    }

    /* Extract level dir for texture/sound/sub-mesh fallback.
     * e.g. "levels\\MyLevel\\MyLevel" -> "levels\\MyLevel\\"
     *      "levels\\level1" -> "levels\\" (no extra dir, fallback is just levels\\) */
    {
        const char *lastSlash = strrchr(resolved, '\\');
        const char *lastSlash2 = strrchr(resolved, '/');
        if (lastSlash2 && (!lastSlash || lastSlash2 > lastSlash)) lastSlash = lastSlash2;
        if (lastSlash) {
            int dirLen = (int)(lastSlash - resolved) + 1; // include slash
            if (dirLen >= MAX_PATH) dirLen = MAX_PATH - 1;
            strncpy(g_levelDir, resolved, dirLen);
            g_levelDir[dirLen] = '\0';
        } else {
            g_levelDir[0] = '\0';
        }
        char dbg3[256]; wsprintfA(dbg3, "g_levelDir set to '%s'", g_levelDir);
        DebugLog(dbg3);
    }

    DWORD app = *(DWORD *)((char *)board + BOARD_APP_PTR);
    if (!app || IsBadReadPtr((void *)app, 0x200)) return;
    void *gfx = *(void **)((char *)app + 0x174);
    if (!gfx) return;

    /* Step 1: MeshWorld */
    void *meshMem = g_operatorNew(0x10D0);
    if (!meshMem) return;
    void *meshWorld = g_LevelMeshWorldCtor(meshMem, gfx, meshPath);
    *(DWORD *)((char *)board + BOARD_MESHWORLD) = (DWORD)meshWorld;

    /* Step 1b: Extension heap allocation (Option B) — S1-driven. */
    {
        void* ext2 = EnsureBoardExt(board);
        (void)ext2;
        // S1 scan now that meshWorld exists — auto-enables feats for file-swapped levels
        if (ext && meshWorld) ScanS1AndAutoEnable(board, ext, meshWorld);
        // Targeted scan: only S1-referenced sub-meshes + main file (not wildcard whole folder)
        if (ext) ScanS1ReferencedMeshesForCollisions(board, ext, meshWorld, meshPath);
        // Build per-board cached collision flags (once, not per-collision)
        if (ext) BuildCollisionFlags(board, ext);
    }

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
    DebugLog("UniversalPostSetup done");

    /* Diagnostic: verify mesh pointers survived post-setup.
     * UNI_BONK_STORE (0x8620) and UNI_SAW2_OBJ (0x862C) are generic mesh
     * slots — they hold different objects per level (Tipper for Dizzy,
     * Bridge for Intermediate/Expert, Spinny for Toob, etc.) */
    {
        DWORD mesh0x8620 = *(DWORD *)((char *)ext + UNI_BONK_STORE);
        DWORD mesh0x862C = *(DWORD *)((char *)ext + UNI_SAW2_OBJ);
        char dbg[256];
        wsprintfA(dbg, "Pre-Board_Setup mesh check: [0x8620]=0x%08X [0x862C]=0x%08X", mesh0x8620, mesh0x862C);
        DebugLog(dbg);
    }

    /* Step 5: Board_Setup via vtable[0x80] */
    DWORD vtable = *(DWORD *)board;
    if (vtable && !IsBadReadPtr((void *)vtable, 0x84)) {
        void (__thiscall *boardSetup)(void *) = *(void (__thiscall **)(void *))((char *)vtable + 0x80);
        if (boardSetup) {
            DebugLog("Calling Board_Setup (vtable+0x80)...");
            boardSetup(board);
            DebugLog("Board_Setup done");
        }
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
                                            (void *)((char *)ext + UNI_TARBUBBLE_LIST),
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
                    (void *)((char *)ext + UNI_TARBUBBLE_LIST),
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

static const DWORD g_allocPatchRVAs[15] __attribute__((unused)) = {
    0x00027109, 0x00027136, 0x00027167, 0x00027198, 0x000271C9,
    0x000271FA, 0x0002722B, 0x0002725C, 0x0002728D, 0x000272BE,
    0x000272EF, 0x00027320, 0x00027351, 0x0002737B, 0x000273A5,
};

static void PatchAllocSizes(void) {
    /* Option B: board stays vanilla-sized; union lives in extension heap at board+EXT_PTR.
     * We no longer patch the 15 PUSH sites to UNION_SIZE (0xAB00). Leave them vanilla. */
    DebugLog("PatchAllocSizes: skipped (vanilla board size, ext heap used)");
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Install collision hook
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Hook Tournament_AdvanceRace (0x00427080) to free ext on level unload */
static unsigned char* g_advanceTrampoline = NULL;
static void (__stdcall *g_origAdvanceRace)(DWORD);
static void __stdcall Hook_AdvanceRace(DWORD a1) {
    /* Free the outgoing board's ext *before* the call (App+0x178 is the cur board)
     * and sweep stales *after* the call. Covers both reuse and free cases.
     * App pointer is at absolute 0x005341E0 (RVA 0x1341E0 from g_moduleBase). */
    void* curBoard = NULL;
    DWORD appPtrAddr = g_moduleBase ? g_moduleBase + 0x1341E0 : 0x005341E0;
    MEMORY_BASIC_INFORMATION mbi;
    if (VirtualQuery((void*)appPtrAddr, &mbi, sizeof(mbi)) && mbi.State==MEM_COMMIT) {
        DWORD app = *(DWORD*)appPtrAddr;
        if (app && VirtualQuery((void*)app, &mbi, sizeof(mbi)) && mbi.State==MEM_COMMIT) {
            /* BOARD_APP_PTR validated via BoardHasOffset-style check */
            if (BoardHasOffset((void*)app, 0x178, 4) || !HeapValidate(GetProcessHeap(),0,(void*)app)) {
                /* Fallback: raw read with SEH-style guard via VirtualQuery already */
                curBoard = *(void**)((char*)app + 0x178);
            } else {
                curBoard = *(void**)((char*)app + 0x178);
            }
        }
    }
    if (curBoard) {
        FreeBoardExt(curBoard);
    }
    if (g_origAdvanceRace) g_origAdvanceRace(a1);
    int j;
    for (j=0;j<MAX_EXT_MAP;j++) if (g_extMap[j].ext) {
        void* b = g_extMap[j].board;
        if (!b || !HeapValidate(GetProcessHeap(),0,b)) {
            /* Board was freed without AdvanceRace (e.g. immediate restart).
             * Use HeapValidate — IsBadReadPtr would pass on freed page. */
            HeapFree(GetProcessHeap(),0,g_extMap[j].ext);
            g_extMap[j].ext=NULL; g_extMap[j].extSize=0; g_extMap[j].feat=0; g_extMap[j].board=NULL;
        } else {
            /* Also sweep if board no longer points to a live Heap block that
             * contains our offset — VirtualQuery check */
            MEMORY_BASIC_INFORMATION mb2;
            if (!VirtualQuery(b, &mb2, sizeof(mb2)) || mb2.State!=MEM_COMMIT) {
                HeapFree(GetProcessHeap(),0,g_extMap[j].ext);
                g_extMap[j].ext=NULL; g_extMap[j].extSize=0; g_extMap[j].feat=0; g_extMap[j].board=NULL;
            }
        }
    }
}
static void InstallExtFreeHook(void) {
    DWORD targetAddr = g_moduleBase + 0x00027080;
    unsigned char* orig = (unsigned char*)targetAddr;
    MEMORY_BASIC_INFORMATION mbi;
    if (!VirtualQuery(orig, &mbi, sizeof(mbi)) || mbi.State!=MEM_COMMIT) return;
    if (!(mbi.Protect & (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY))) return;
    // RVA 0x27080 prologue varies by build (dump shows 3C AE FD FF at 0x26480 file-off, not 55 8B EC)
    // Don't gate on exact bytes — log and continue
    if (orig[0]!=0x55 || orig[1]!=0x8B || orig[2]!=0xEC) {
        char dbg[64]; wsprintfA(dbg, "InstallExtFreeHook: prologue %02X %02X %02X (continuing)", orig[0], orig[1], orig[2]);
        DebugLog(dbg);
    }
    g_advanceTrampoline = VirtualAlloc(NULL, 32, MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!g_advanceTrampoline) return;
    memcpy(g_advanceTrampoline, orig, 6);
    g_advanceTrampoline[6]=0xE9;
    *(DWORD*)(g_advanceTrampoline+7) = (targetAddr+6) - ((DWORD)g_advanceTrampoline+11);
    g_origAdvanceRace = (void (__stdcall *)(DWORD))g_advanceTrampoline;
    DWORD oldProtect;
    VirtualProtect(orig, 6, PAGE_EXECUTE_READWRITE, &oldProtect);
    orig[0]=0xE9;
    *(DWORD*)(orig+1) = (DWORD)Hook_AdvanceRace - (targetAddr+5);
    orig[5]=0x90;
    VirtualProtect(orig,6, oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), orig, 6);
    DebugLog("InstallExtFreeHook: hooked Tournament_AdvanceRace");
}

static void InstallHook(void) {
    /* DEPRECATED: SEH trampoline on DispatchCollisionEvents (0x40C5D0) caused
     * stack/exception chain corruption (skill: Manual trampoline detour hooks
     * on SEH functions crash). UniversalDispatchCollision via vtable[29]
     * now solely handles collisions and calls the original directly.
     * Keep trampoline disabled — just set g_OriginalDispatch to the raw
     * function address. */
    g_OriginalDispatch = (DispatchCollisionEvents_t)(g_moduleBase + RVA_DispatchCollisionEvents);
    DebugLog("InstallHook: SEH trampoline disabled, vtable[29] handles collisions");
    (void)g_trampoline;
}

/* BASS proxy exports handled by bass.def - DLL forwarding to bass_real.dll */

/* ═══════════════════════════════════════════════════════════════════════════
 * File fallback hook — if game can't find texture/sound/mesh in its
 * default folder, retry in g_levelDir (the folder containing the main
 * MESHWORLD for this race). Covers:
 *   textures\\foo.png      -> g_levelDir\\foo.png
 *   levels\\Level3-Water  -> g_levelDir\\Level3-Water  (+ .MESHWORLD/.MESH)
 *   sounds\\foo.ogg       -> g_levelDir\\foo.ogg
 * Implemented as IAT patch on kernel32!CreateFileA/W so every file open
 * (D3DX, BASS, MESHWORLD) is covered without per-callsite hooks.
 * ═══════════════════════════════════════════════════════════════════════════ */

static HANDLE (WINAPI *g_origCreateFileA)(LPCSTR,DWORD,DWORD,LPSECURITY_ATTRIBUTES,DWORD,DWORD,HANDLE) = NULL;
static HANDLE (WINAPI *g_origCreateFileW)(LPCWSTR,DWORD,DWORD,LPSECURITY_ATTRIBUTES,DWORD,DWORD,HANDLE) = NULL;
static volatile LONG g_inFileHookFallback = 0;

static HANDLE WINAPI Hook_CreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) {
    if (InterlockedExchange(&g_inFileHookFallback, 1) != 0) {
        return g_origCreateFileA(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
    }
    HANDLE h = g_origCreateFileA(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
    if (h != INVALID_HANDLE_VALUE) { InterlockedExchange(&g_inFileHookFallback, 0); return h; }
    DWORD err = GetLastError();
    if (err != ERROR_FILE_NOT_FOUND && err != ERROR_PATH_NOT_FOUND) { InterlockedExchange(&g_inFileHookFallback, 0); return h; }
    if (!lpFileName || !g_levelDir[0]) { InterlockedExchange(&g_inFileHookFallback, 0); return h; }
    const char *ext = strrchr(lpFileName, '.');
    const char *slash = strrchr(lpFileName, '\\');
    const char *slash2 = strrchr(lpFileName, '/');
    if (slash2 && (!slash || slash2 > slash)) slash = slash2;
    const char *base = slash ? slash + 1 : lpFileName;
    if (!base || !*base) { InterlockedExchange(&g_inFileHookFallback, 0); return h; }
    char trial[MAX_PATH];
    int dirLen = strlen(g_levelDir);
    int baseLen = strlen(base);
    if (dirLen + baseLen >= MAX_PATH) { InterlockedExchange(&g_inFileHookFallback, 0); return h; }
    strcpy(trial, g_levelDir);
    strcat(trial, base);
    if (my_stricmp(trial, lpFileName) == 0) { InterlockedExchange(&g_inFileHookFallback, 0); return h; }
    if (my_strnicmp(base, "lfdebug", 7)==0 || my_strnicmp(base, "LevelFeatures", 13)==0 || my_strnicmp(base, "RaceFiles", 9)==0) {
        InterlockedExchange(&g_inFileHookFallback, 0);
        return h;
    }
    HANDLE h2 = g_origCreateFileA(trial, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
    if (h2 != INVALID_HANDLE_VALUE) {
        char dbg[512]; wsprintfA(dbg, "Fallback: '%s' -> '%s' (OK)", lpFileName, trial);
        DebugLog(dbg);
        InterlockedExchange(&g_inFileHookFallback, 0);
        return h2;
    }
    if (!ext) {
        if (dirLen + baseLen + 10 >= MAX_PATH) { InterlockedExchange(&g_inFileHookFallback, 0); return h; }
        strcpy(trial, g_levelDir);
        strcat(trial, base);
        strcat(trial, ".MESHWORLD");
        HANDLE h3 = g_origCreateFileA(trial, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
        if (h3 != INVALID_HANDLE_VALUE) {
            char dbg2[512]; wsprintfA(dbg2, "Fallback (+.MESHWORLD): '%s' -> '%s' (OK)", lpFileName, trial);
            DebugLog(dbg2);
            InterlockedExchange(&g_inFileHookFallback, 0);
            return h3;
        }
    }
    InterlockedExchange(&g_inFileHookFallback, 0);
    return h;
}

static HANDLE WINAPI Hook_CreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) {
    if (InterlockedExchange(&g_inFileHookFallback, 1) != 0) return g_origCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
    HANDLE h = g_origCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
    if (h != INVALID_HANDLE_VALUE) { InterlockedExchange(&g_inFileHookFallback, 0); return h; }
    DWORD err = GetLastError();
    if (err != ERROR_FILE_NOT_FOUND && err != ERROR_PATH_NOT_FOUND) { InterlockedExchange(&g_inFileHookFallback, 0); return h; }
    if (!lpFileName || !g_levelDir[0]) { InterlockedExchange(&g_inFileHookFallback, 0); return h; }
    char ansi[MAX_PATH]; WideCharToMultiByte(CP_ACP, 0, lpFileName, -1, ansi, MAX_PATH, NULL, NULL);
    const char *slash = strrchr(ansi, '\\');
    const char *slash2 = strrchr(ansi, '/');
    if (slash2 && (!slash || slash2 > slash)) slash = slash2;
    const char *base = slash ? slash + 1 : ansi;
    if (!base || !*base) { InterlockedExchange(&g_inFileHookFallback, 0); return h; }
    char trialAnsi[MAX_PATH];
    int dirLen = strlen(g_levelDir);
    int baseLen = strlen(base);
    if (dirLen + baseLen >= MAX_PATH) { InterlockedExchange(&g_inFileHookFallback, 0); return h; }
    strcpy(trialAnsi, g_levelDir);
    strcat(trialAnsi, base);
    if (my_stricmp(trialAnsi, ansi)==0) { InterlockedExchange(&g_inFileHookFallback, 0); return h; }
    if (my_strnicmp(base, "lfdebug", 7)==0) { InterlockedExchange(&g_inFileHookFallback, 0); return h; }
    WCHAR trialW[MAX_PATH]; MultiByteToWideChar(CP_ACP, 0, trialAnsi, -1, trialW, MAX_PATH);
    HANDLE h2 = g_origCreateFileW(trialW, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
    if (h2 != INVALID_HANDLE_VALUE) {
        char dbg[512]; wsprintfA(dbg, "FallbackW: '%s' -> '%s' (OK)", ansi, trialAnsi);
        DebugLog(dbg);
        InterlockedExchange(&g_inFileHookFallback, 0);
        return h2;
    }
    InterlockedExchange(&g_inFileHookFallback, 0);
    return h;
}

static void InstallFileFallbackHook(void) {
    if (g_origCreateFileA) return; // already installed
    HMODULE exe = GetModuleHandleA(NULL);
    if (!exe) exe = (HMODULE)g_moduleBase;
    if (!exe || IsBadReadPtr(exe, 0x100)) return;
    PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)exe;
    if (dos->e_magic != 0x5A4D) return;
    PIMAGE_NT_HEADERS nt = (PIMAGE_NT_HEADERS)((char*)exe + dos->e_lfanew);
    if (IsBadReadPtr(nt, sizeof(IMAGE_NT_HEADERS))) return;
    if (nt->Signature != 0x00004550) return;
    DWORD importRVA = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
    if (!importRVA) return;
    PIMAGE_IMPORT_DESCRIPTOR imp = (PIMAGE_IMPORT_DESCRIPTOR)((char*)exe + importRVA);
    for (; imp->Name; imp++) {
        char *dllName = (char*)exe + imp->Name;
        if (!dllName || IsBadReadPtr(dllName, 8)) continue;
        if (my_stricmp(dllName, "KERNEL32.dll") != 0 && my_stricmp(dllName, "kernel32.dll") != 0) continue;
        PIMAGE_THUNK_DATA thunk = (PIMAGE_THUNK_DATA)((char*)exe + imp->FirstThunk);
        PIMAGE_THUNK_DATA origThunk = (PIMAGE_THUNK_DATA)((char*)exe + imp->OriginalFirstThunk);
        int useOrig = (imp->OriginalFirstThunk != 0);
        for (; thunk->u1.Function; thunk++, origThunk++) {
            PIMAGE_THUNK_DATA nameThunk = useOrig ? origThunk : thunk;
            if (IMAGE_SNAP_BY_ORDINAL(nameThunk->u1.Ordinal)) continue;
            PIMAGE_IMPORT_BY_NAME byName = (PIMAGE_IMPORT_BY_NAME)((char*)exe + nameThunk->u1.AddressOfData);
            if (IsBadReadPtr(byName, 4)) continue;
            char *funcName = (char*)byName->Name;
            if (!funcName || IsBadReadPtr(funcName, 4)) continue;
            DWORD oldProtect;
            if (strcmp(funcName, "CreateFileA")==0) {
                g_origCreateFileA = (void*)thunk->u1.Function;
                VirtualProtect(&thunk->u1.Function, 4, PAGE_EXECUTE_READWRITE, &oldProtect);
                thunk->u1.Function = (DWORD)Hook_CreateFileA;
                VirtualProtect(&thunk->u1.Function, 4, oldProtect, &oldProtect);
                FlushInstructionCache(GetCurrentProcess(), &thunk->u1.Function, 4);
                DebugLog("InstallFileFallbackHook: CreateFileA hooked");
            } else if (strcmp(funcName, "CreateFileW")==0) {
                g_origCreateFileW = (void*)thunk->u1.Function;
                VirtualProtect(&thunk->u1.Function, 4, PAGE_EXECUTE_READWRITE, &oldProtect);
                thunk->u1.Function = (DWORD)Hook_CreateFileW;
                VirtualProtect(&thunk->u1.Function, 4, oldProtect, &oldProtect);
                FlushInstructionCache(GetCurrentProcess(), &thunk->u1.Function, 4);
                DebugLog("InstallFileFallbackHook: CreateFileW hooked");
            }
        }
    }
    if (!g_origCreateFileA) DebugLog("InstallFileFallbackHook: CreateFileA not found in IAT");
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Debug logging
 * ═══════════════════════════════════════════════════════════════════════════ */

void DebugLog(const char *msg) {
    /* Write next to the DLL, or fall back to game directory */
    char logPath[MAX_PATH];
    if (g_configPath[0]) {
        strcpy(logPath, g_configPath);
        char *p = strrchr(logPath, '\\');
        if (p) {
            strcpy(p + 1, "lfdebug.log");
        } else {
            strcpy(logPath, "lfdebug.log");
        }
    } else {
        /* GetConfigPath failed (common under Wine/BoxedWine) — use game dir */
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

/* Forward declaration for auto-test (unused, kept for future testing) */

/* ═══════════════════════════════════════════════════════════════════════════
 * Vtable patching — replace slots 1, 19, 29, 33 in all 15 level vtables
 * with universal handlers. Saves original pointers for delegation.
 * ═══════════════════════════════════════════════════════════════════════════ */

static void InstallVtablePatches(void) {
    int i;
    for (i = 1; i <= 15; i++) {
        DWORD vtableAddr = g_levelVtables[i];
        if (!vtableAddr) continue;
        { MEMORY_BASIC_INFORMATION mbi; if (!VirtualQuery((void*)vtableAddr, &mbi, sizeof(mbi)) || mbi.State != MEM_COMMIT || (DWORD)mbi.BaseAddress + mbi.RegionSize < vtableAddr + 0x88) continue; }

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
         * Save original per-level handler — levels 2,6,7 have custom RaceState
         * that iterates per-level data (lifter list, render objs, timers). */
        {
            DWORD *slot = (DWORD *)(vtableAddr + 0x4C);
            /* Save original before overwriting (index by level 1-15) */
            if (i >= 1 && i <= 15) {
                g_origRaceState[i] = (RaceState_t)*slot;
            }
            VirtualProtect(slot, 4, PAGE_EXECUTE_READWRITE, &oldProtect);
            *slot = (DWORD)&UniversalRaceState;
            VirtualProtect(slot, 4, oldProtect, &oldProtect);
            FlushInstructionCache(GetCurrentProcess(), slot, 4);
        }

        /* Slot 29 (offset +0x74): DispatchCollision → UniversalDispatchCollision
         * No need to save original — UniversalDispatchCollision calls g_OriginalDispatch directly */
        {
            DWORD *slot = (DWORD *)(vtableAddr + 0x74);
            VirtualProtect(slot, 4, PAGE_EXECUTE_READWRITE, &oldProtect);
            *slot = (DWORD)&UniversalDispatchCollision;
            VirtualProtect(slot, 4, oldProtect, &oldProtect);
            FlushInstructionCache(GetCurrentProcess(), slot, 4);
        }

        /* Slot 33 (offset +0x84): CreateDynamicObjects → UniversalCreateDynamicObjects
         * No delegation — universal handler recognizes all object names */
        {
            DWORD *slot = (DWORD *)(vtableAddr + 0x84);
            VirtualProtect(slot, 4, PAGE_EXECUTE_READWRITE, &oldProtect);
            *slot = (DWORD)&UniversalCreateDynamicObjects;
            VirtualProtect(slot, 4, oldProtect, &oldProtect);
            FlushInstructionCache(GetCurrentProcess(), slot, 4);
        }

        /* Slot 24 (offset +0x60): Per-level render — per-level RET convention.
         * Two call sites: 0x0046C8C7 (1-param RET 0) vs 0x0046C9F0 (2-param RET 4).
         * Only the 6 levels that use 2-param render (2=Beginner,5=Tower,10=Toob,12=Glass,13=Sky,14=Master) are patched. */
        {
            int is_two_param = (i==2 || i==5 || i==10 || i==12 || i==13 || i==14);
            if (is_two_param) {
                DWORD *slot = (DWORD *)(vtableAddr + 0x60);
                VirtualProtect(slot, 4, PAGE_EXECUTE_READWRITE, &oldProtect);
                *slot = (DWORD)&UniversalRender;
                VirtualProtect(slot, 4, oldProtect, &oldProtect);
                FlushInstructionCache(GetCurrentProcess(), slot, 4);
            }
        }
    }
    DebugLog("Vtable slots [1,19,29,33] patched (slot 24 per-level: 2,5,10,12,13,14)");
}

static DWORD WINAPI PatchThread(LPVOID param) {
    DebugLog("=== PatchThread started ===");
    Sleep(1000); // 1s: early enough to beat first level load (~3-4s), late enough for BASS init
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
    g_ehVectorCtor = (eh_vector_ctor_t)(g_moduleBase + RVA_eh_vector_ctor);
    g_ehVectorCtorFn = (void *)(g_moduleBase + RVA_FUN_0040a870);
    g_Vec3ListFree = (void *)(g_moduleBase + RVA_Vec3List_Free);
    g_LevelAssignTexScales = (Level_AssignTexScales_t)(g_moduleBase + RVA_Level_AssignTexScales);

    /* Resolve CreateDynamicObjects ctors */
    g_TipperCtor = (Ctor3_t)(g_moduleBase + RVA_Tipper_ctor);
    g_TipperVisualCtor = (TipperVisual_ctor_t)(g_moduleBase + RVA_TipperVisual_ctor);
    g_GluebieCtor = (Ctor3_t)(g_moduleBase + RVA_Gluebie_ctor);
    g_CatapultCtor = (Ctor3_t)(g_moduleBase + RVA_Catapult_ctor);
    g_MaceCtor = (Ctor3_t)(g_moduleBase + RVA_Mace_ctor);
    g_GlassLevelCtor = (Ctor3_t)(g_moduleBase + RVA_Glass_Level_ctor);
    g_TrapdoorCtor = (Ctor2_t)(g_moduleBase + RVA_Trapdoor_ctor);
    g_StandsCtor = (Stands_ctor_t)(g_moduleBase + RVA_Stands_ctor);
    g_BonkCtor = (Ctor4f_t)(g_moduleBase + RVA_Bonk_ctor);
    g_FanCtor = (Ctor5f_t)(g_moduleBase + RVA_Fan_ctor);
    g_SawBladeCtor = (Ctor4f_t)(g_moduleBase + RVA_SawBlade_ctor);
    g_SawBladeSetVariant = (SawBlade_SetVariant_t)(g_moduleBase + RVA_SawBlade_SetVariant);
    g_SpinnerLevelCtor = (Ctor5f_t)(g_moduleBase + RVA_Spinner_Level_ctor);
    g_GearLevelCtor = (Ctor4f_t)(g_moduleBase + RVA_Gear_Level_ctor);
    g_BellCtor = (Ctor4f_t)(g_moduleBase + RVA_Bell_ctor);
    g_OddLifterCtor = (Ctor4f_t)(g_moduleBase + RVA_Odd_Lifter_ctor);
    g_LifterCtor = (Ctor_Lifter_t)(g_moduleBase + RVA_Lifter_ctor);
    g_SpeedCylinderCtor = (Ctor_SpeedCyl_t)(g_moduleBase + RVA_SpeedCylinder_ctor);
    g_TimeButtonCtor = (Ctor3f_t)(g_moduleBase + RVA_TimeButton_ctor);
    g_RotatorImpossibleCtor = (Ctor3f_t)(g_moduleBase + RVA_Rotator_ctor_Impossible);
    g_SawCtor = (Ctor6f_t)(g_moduleBase + RVA_Saw_ctor);
    g_Saw2Ctor = (Ctor6f_t)(g_moduleBase + RVA_Saw2_ctor);
    g_FalloutCtor = (Ctor3f_t)(g_moduleBase + RVA_Fallout_ctor);
    g_BlockdawgCtor = (Ctor6f_t)(g_moduleBase + RVA_Blockdawg_ctor);
    g_GameLevelCtor = (Ctor3f_t)(g_moduleBase + RVA_GameLevel_ctor);
    g_WavyCtor = (Ctor_Str_t)(g_moduleBase + RVA_Wavy_ctor);
    g_WavyConfigure = (Wavy_Configure_t)(g_moduleBase + RVA_Wavy_Configure);
    g_NeonPlatformCtor = (Ctor3f_t)(g_moduleBase + RVA_NeonPlatform_ctor);
    g_ArenaStandsCtor = (Ctor3f_t)(g_moduleBase + RVA_ArenaStands_ctor);
    g_PopcylinderCtor = (Ctor3f_t)(g_moduleBase + RVA_Popcylinder_ctor);
    g_PopCylinderCtor = (Ctor3f_t)(g_moduleBase + RVA_PopCylinder_ctor);
    g_RotatorCtor = (Ctor_Rotator_t)(g_moduleBase + RVA_Rotator_ctor);
    g_LooperCtor = (Ctor3f_t)(g_moduleBase + RVA_Looper_ctor);
    g_GearCtor = (Ctor7f_t)(g_moduleBase + RVA_Gear_ctor);
    g_PendulumCtor = (Ctor3f_t)(g_moduleBase + RVA_Pendulum_ctor);
    g_BreakBridgeCtor = (Ctor3f_t)(g_moduleBase + RVA_BreakBridge_ctor);
    g_LevelFindObjectByName = (Level_FindObjectByName_t)(g_moduleBase + RVA_Level_FindObjectByName);
    g_SoundInitChannels = (Sound_InitChannels_t)(g_moduleBase + RVA_Sound_InitChannels);

    /* Resolve Board_Update function pointers */
    g_SceneUpdate = (Scene_Update_t)(g_moduleBase + RVA_Scene_Update);
    g_BoardUpdateRaceState = (Board_UpdateRaceState_t)(g_moduleBase + RVA_Board_UpdateRaceState);
    g_RenderDynamicObjects = (Level_RenderDynamicObjects_t)(g_moduleBase + RVA_Level_RenderDynamicObjects);
    g_GraphicsSetProjection = (Graphics_SetProjection_t)(g_moduleBase + RVA_Graphics_SetProjection);
    g_GraphicsSetCullMode2 = (Graphics_SetCullMode2_t)(g_moduleBase + RVA_Graphics_SetCullMode2);
    g_SpriteRenderQuad = (Sprite_RenderQuad_t)(g_moduleBase + RVA_Sprite_RenderQuad);
    g_RenderContextInit = (RenderContext_Init_t)(g_moduleBase + RVA_RenderContext_Init);
    g_Matrix4Identity = (Matrix4_Identity_t)(g_moduleBase + RVA_Matrix4_Identity);
    g_GfxScaleZ = (Gfx_ScaleFn_t)(g_moduleBase + RVA_Gfx_ScaleZ);
    g_GfxScaleY = (Gfx_ScaleFn_t)(g_moduleBase + RVA_Gfx_ScaleY);
    g_GfxScaleX = (Gfx_ScaleFn_t)(g_moduleBase + RVA_Gfx_ScaleX);
    g_GfxSetPosition = (Gfx_SetPosition_t)(g_moduleBase + RVA_Gfx_SetPosition);
    g_TimerInit = (Timer_Init_t)(g_moduleBase + RVA_Timer_Init);
    g_TimerCleanup = (Timer_Cleanup_t)(g_moduleBase + RVA_Timer_Cleanup);
    g_MatrixTransformVec3 = (Matrix_TransformVec3_t)(g_moduleBase + RVA_Matrix_TransformVec3);
    g_Matrix44Zero = (Matrix44_Zero_t)(g_moduleBase + RVA_Matrix44_Zero);
    g_SceneForEachBallSetVelocity = (Scene_ForEachBall_SetVelocity_t)(g_moduleBase + RVA_Scene_ForEachBall_SetVel);
    /* g_AthenaListGetIterator, g_AthenaListGetSize, g_AthenaListAppend already
     * resolved above — removed redundant re-resolution. */
    g_CreateTarBubble = (FUN_0044fa90_t)(g_moduleBase + RVA_FUN_0044fa90);
    g_CreateSplashParticle = (FUN_0044fb50_t)(g_moduleBase + RVA_FUN_0044fb50);
    g_RemoveBall = (FUN_00405190_t)(g_moduleBase + RVA_FUN_00405190);
    g_RNG = (CPUID_RNG_t)(g_moduleBase + RVA_CPUID_RNG);
    g_RNG_raw = (DWORD)(g_moduleBase + RVA_CPUID_RNG);
    g_BadBallCtor = (BadBall_ctor_t)(g_moduleBase + RVA_BadBall_ctor);
    g_BallSetTrajectory = (Ball_SetTrajectory_t)(g_moduleBase + RVA_Ball_SetTrajectory);
    g_BallSetVec3AtOffset = (Ball_SetVec3AtOffset_t)(g_moduleBase + RVA_Ball_SetVec3AtOffset);
    g_Vec3NormalizeAndScale = (Vec3_NormalizeAndScale_t)(g_moduleBase + RVA_Vec3_NormalizeAndScale);
    g_Vec3CopyUpd = (Vec3_Copy_t)(g_moduleBase + RVA_Vec3_Copy_Upd);
    g_SoundCalcDistAtten = (Sound_CalcDistAtten_t)(g_moduleBase + RVA_Sound_CalcDistAtten);
    g_SoundPlay3DAtPos = (Sound_Play3DAtPos_t)(g_moduleBase + RVA_Sound_Play3DAtPos);
    g_SceneSetRaceActive = (Scene_SetRaceActive_t)(g_moduleBase + RVA_Scene_SetRaceActive);
    g_SceneAddObject = (Scene_AddObject_t)(g_moduleBase + RVA_Scene_AddObject);

    /* Option B: Feature flags now per-board via ScanS1AndAutoEnable/OrBoardFeat.
     * g_updateFeatures/g_collisionEvents no longer used — S1-driven. */
    // memcpy(g_updateFeatures, ...) deprecated

    /* Resolve collision handler function pointers */
    g_SoundPlayChannel = (Sound_PlayChannel_t)(g_moduleBase + RVA_Sound_PlayChannel);
    g_BallDizzyImmunity = (Ball_DizzyImmunity_t)(g_moduleBase + RVA_Ball_DizzyImmunity);
    g_BallGrow = (Ball_Grow_t)(g_moduleBase + RVA_Ball_Grow);
    g_BallShrink = (Ball_Shrink_t)(g_moduleBase + RVA_Ball_Shrink);
    g_BallSetName = (Ball_SetName_t)(g_moduleBase + RVA_Ball_SetName);
    g_BallApplyTrajectory = (Ball_ApplyTrajectory_t)(g_moduleBase + RVA_Ball_ApplyTrajectory);
    g_BallSetTiltedGravity = (Ball_SetTiltedGravity_t)(g_moduleBase + RVA_Ball_SetTiltedGravity);
    g_BallSetFlatGravity = (Ball_SetFlatGravity_t)(g_moduleBase + RVA_Ball_SetFlatGravity);
    g_RotatorMarkTriggered = (Rotator_MarkTriggered_t)(g_moduleBase + RVA_Rotator_MarkTriggered);
    g_RotatorPlayCollisionSound = (Rotator_PlayCollisionSound_t)(g_moduleBase + RVA_Rotator_PlayCollisionSound);
    g_RotatorTriggerSound = (Rotator_TriggerSound_t)(g_moduleBase + RVA_Rotator_TriggerSound);
    g_RotatorStartSound = (Rotator_StartSound_t)(g_moduleBase + RVA_Rotator_StartSound);
    g_RotatorAddBall = (Rotator_AddBall_t)(g_moduleBase + RVA_Rotator_AddBall);
    g_CheckArenaUnlock = (CheckArenaUnlock_t)(g_moduleBase + RVA_CheckArenaUnlock);
    g_CatapultLaunch = (Catapult_Launch_t)(g_moduleBase + RVA_Catapult_Launch);
    g_TrapdoorOpen = (Trapdoor_Open_t)(g_moduleBase + RVA_Trapdoor_Open);
    g_TrapdoorActivate = (Trapdoor_Activate_t)(g_moduleBase + RVA_Trapdoor_Activate);
    g_SawAlertActivate = (Saw_AlertActivate_t)(g_moduleBase + RVA_Saw_AlertActivate);
    g_SawActivate = (Saw_Activate_t)(g_moduleBase + RVA_Saw_Activate);
    g_BellActivate = (Bell_Activate_t)(g_moduleBase + RVA_Bell_Activate);
    g_JudgeReset = (Judge_Reset_t)(g_moduleBase + RVA_Judge_Reset);
    g_ScoreDisplaySetTime = (ScoreDisplay_SetTime_t)(g_moduleBase + RVA_ScoreDisplay_SetTime);
    g_CreateBonkPopup = (CreateBonkPopup_t)(g_moduleBase + RVA_CreateBonkPopup);
    g_HammerChaseStart = (Hammer_ChaseStart_t)(g_moduleBase + RVA_Hammer_ChaseStart);
    g_PendulumPlayCollisionSound = (Pendulum_PlayCollisionSound_t)(g_moduleBase + RVA_Pendulum_PlayCollisionSound);
    g_PendulumAddIndex = (Pendulum_AddIndex_t)(g_moduleBase + RVA_Pendulum_AddIndex);
    g_ScoreObjectCtor = (ScoreObject_ctor_t)(g_moduleBase + RVA_ScoreObject_ctor);
    g_TimerDecrement = (Timer_Decrement_t)(g_moduleBase + RVA_Timer_Decrement);
    g_ArenaScoreParticleCtor = (ArenaScoreParticle_ctor_t)(g_moduleBase + RVA_ArenaScoreParticle_ctor);
    g_AthenaHashTableLookup = (AthenaHashTable_Lookup_t)(g_moduleBase + RVA_AthenaHashTable_Lookup);
    g_AthenaListContainsValue = (AthenaList_ContainsValue_t)(g_moduleBase + RVA_AthenaList_ContainsValue);
    g_SceneObjectSub1Ctor = (SceneObject_sub1_ctor_t)(g_moduleBase + RVA_SceneObject_sub1_ctor);
    g_AthenaStringSet = (AthenaString_Set_t)(g_moduleBase + RVA_AthenaString_Set);
    g_MWParserReadTag = (MWParser_ReadTag_t)(g_moduleBase + RVA_MWParser_ReadTag);
    g_StreamReaderDtor = (StreamReader_dtor_t)(g_moduleBase + RVA_StreamReader_dtor);
    g_AudioPlayMusic = (Audio_PlayMusic_t)(g_moduleBase + RVA_Audio_PlayMusic);
    g_DifficultyGetTimeModifier = (Difficulty_GetTimeModifier_t)(g_moduleBase + RVA_Difficulty_GetTimeModifier);
    g_AthenaStringFormat = (AthenaString_Format_t)(g_moduleBase + RVA_AthenaString_Format);
    g_AthenaStringSprintfToBuffer = (AthenaString_SprintfToBuffer_t)(g_moduleBase + RVA_AthenaString_SprintfToBuffer);
    g_WaveCos = (Wave_Fn_t)(g_moduleBase + RVA_Wave_Cos);
    g_WaveSin = (Wave_Fn_t)(g_moduleBase + RVA_Wave_Sin);
    g_SceneRegisterObject = (Scene_RegisterObject_t)(g_moduleBase + RVA_Scene_RegisterObject);
    g_AthenaListRemoveByValue = (AthenaList_RemoveByValue_t)(g_moduleBase + RVA_AthenaList_RemoveByValue);
    g_NeonPlatformActivate = (NeonPlatform_Activate_t)(g_moduleBase + RVA_NeonPlatform_Activate);
    g_SquareWobblyActivate = (SquareWobbly_Activate_t)(g_moduleBase + RVA_SquareWobbly_Activate);
    g_WavyActivate = (Wavy_Activate_t)(g_moduleBase + RVA_Wavy_Activate);
    g_SpinnerActivate = (Spinner_Activate_t)(g_moduleBase + RVA_Spinner_Activate);
    g_GearAddBall = (Gear_AddBall_t)(g_moduleBase + RVA_Gear_AddBall);
    g_NormalGravityReset = (NormalGravityReset_t)(g_moduleBase + RVA_NormalGravityReset);
    g_DropLiftActivate = (DropLift_Activate_t)(g_moduleBase + RVA_DropLift_Activate);
    g_CPUIDRNG = (CPUID_RNG_Fn_t)(g_moduleBase + RVA_CPUID_RNG_Fn);

    DebugLog("Function pointers resolved");

    GetConfigPath();
    {
        char dbg_cp[256];
        wsprintfA(dbg_cp, "GetConfigPath done: path='%s'", g_configPath[0] ? g_configPath : "(empty)");
        DebugLog(dbg_cp);
        char dbg_rf[256]; wsprintfA(dbg_rf, "RaceFiles path='%s'", g_raceFilesPath[0] ? g_raceFilesPath : "(empty)");
        DebugLog(dbg_rf);
    }
    // RaceFiles.txt: generate defaults if missing, then load
    {
        HANDLE hRf = CreateFileA(g_raceFilesPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hRf==INVALID_HANDLE_VALUE) {
            InitRaceFilesDefaults();
            GenerateRaceFiles();
            DebugLog("GenerateRaceFiles done (defaults)");
        } else CloseHandle(hRf);
    }
    LoadRaceFiles();
    DebugLog("LoadRaceFiles done");
    // Option B: LevelFeatures.txt deprecated — no LoadConfig
    DebugLog("LevelFeatures.txt deprecated (Option B, S1 collisions)");

    /* LevelData.txt — DEPRECATED Phase1: removed. g_levelData[] stays as
     * in-memory defaults only; spawns are S1-driven via ScanS1AndAutoEnable + S1Ensure*. */
    DebugLog("LevelData.txt deprecated — using in-memory defaults + S1 scan");

    InstallExtFreeHook();
    DebugLog("InstallExtFreeHook done");
    PatchAllocSizes();
    DebugLog("PatchAllocSizes done");
    InstallFileFallbackHook();
    DebugLog("InstallFileFallbackHook done");
    InstallBoardCtorHooks();
    DebugLog("InstallBoardCtorHooks done");
    InstallUniversalConstructorHook();
    DebugLog("InstallUniversalConstructorHook done");
    InstallVtablePatches();
    DebugLog("InstallVtablePatches done");
    InstallHook();
    DebugLog("InstallHook done");
    DebugLog("=== PatchThread complete (v2 swirlfix) ===\n");
    return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hinstDLL);
        CreateThread(NULL, 0, PatchThread, NULL, 0, NULL);
    }
    return TRUE;
}
