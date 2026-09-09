/* Chapter 0 - core: header, RVAs, offsets, ext heap, globals, string/level/racefile helpers (LevelFeatures.c lines 1-1425) */
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
 * Dedicated Render Data Section (0xA880–0xA8D0, 80 bytes)
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
    {"Tower",0x004D0A08,"Board (Tower)","TOWER RACE","TOWERRACE","Happy Rush",{1.0f,0.75f,0.0f},"levels\\level4",{"0x8620:Levels\\Level4-Catapult","0x8628:Levels\\Level4-Drawbridge","0x85F4:MESH:Meshes\\YellowLink","0x85EC:Levels\\Level4-Mace","0x85F0:Levels\\Level4-Windmill","0x8600:MESH:Meshes\\Chomper","0x861C:Levels\\Level4-Turret"},7,0x852,
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
    {"Toob",0x004D0E78,"Board (Toob)","TOOB RACE","TOOBRACE","Rodenthood",{0.5f,0.5f,1.0f},"levels\\level8",{"0x86C8:Levels\\Level8-Spinny","0x86CC:Levels\\Level8-Saw","0x86D0:Levels\\Level8-Fallout","0x85EC:Levels\\Level8-BlockDawg1","0x85F0:Levels\\Level8-BlockDawg2"},5,0x856,
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
