/*
/*
 * mknp_custom_entities.c — Hamsterball Custom Entities Mod v55n_22
 *
 * bass.dll proxy mod. Spawns custom entities from MESHWORLD S1 ref points.
 */

#include "bass_proxy.h"
#ifdef NOSTDIO_TEST
#include "nostdio_test.h"   /* TEST-ONLY: no-op file streams for Wine repro */
#endif
#include <shlwapi.h>
#include <math.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

/* Forward declarations for v55m_42f BASS sound helpers */
static void cEnt_load_dropin_sample(FILE* logfile);
static void cEnt_play_dropin_sound(FILE* logfile);
static void cEnt_play_catapult_sound(FILE* logfile);  /* v55m_42i */

/* ═══════════════════════════════════════════════════════════════════════════
 * Game function pointers
 * ═══════════════════════════════════════════════════════════════════════════ */

/* operator_new — game's C++ allocator (malloc wrapper) */
typedef void* (__cdecl *operator_new_t)(SIZE_T);
static operator_new_t pfn_operator_new = (operator_new_t)0x004BA57B;

/* MeshWorld_ctor — loads a .MESHWORLD file into a mesh object
 * __thiscall(this, gfx_device, mesh_path_string)
 * mesh_path_string = e.g. "levels\\testcube" (without .MESHWORLD extension)
 * Returns the mesh object pointer (same as this) */
typedef void* (__thiscall *MeshWorld_ctor_t)(void* this_, void* gfx_device, const char* mesh_path);
static MeshWorld_ctor_t pfn_MeshWorld_ctor = (MeshWorld_ctor_t)0x00461510;

/* MeshNode_ctor — loads a .MESH file into a 0x18-byte MeshNode wrapper
 * __thiscall(this, gfx_device, mesh_path)
 * Internally: alloc 0x488-byte MeshWorld, format "%s.mesh", _check_file_access,
 *   if found: mesh->vtable[1](path) to load binary, vtable[2]() to finalize
 *   else: MeshWorld_Parse(path, 1) for ASE text format
 * MeshNode layout: +0x00 vtable, +0x04 gfx_device, +0x08 MeshWorld*, +0x0C loaded flag, +0x0D has_mesh flag, +0x0E flag, +0x10 unused
 * MeshWorld* is at MeshNode+0x08 */
typedef void* (__thiscall *MeshNode_ctor_t)(void* this_, void* gfx_device, const char* mesh_path);
static MeshNode_ctor_t pfn_MeshNode_ctor = (MeshNode_ctor_t)0x00471C20;

/* _check_file_access — checks if file exists (GetFileAttributesA)
 * __cdecl(path, mode) — returns 0 if exists, 0xFFFFFFFF if not */
typedef int (__cdecl *check_file_access_t)(const char* path, unsigned char mode);
static check_file_access_t pfn_check_file_access = (check_file_access_t)0x004C8FF7;

/* MeshWorld_ctor (simple) — 0x004706E0 — initializes a 0x488-byte MeshWorld
 * __thiscall(this, gfx_device) — no file loading, just struct init */
typedef void* (__thiscall *MeshWorld_ctor_simple_t)(void* this_, void* gfx_device);
static MeshWorld_ctor_simple_t pfn_MeshWorld_ctor_simple = (MeshWorld_ctor_simple_t)0x004706E0;

/* MeshNode size constants */
#define MESHNODE_SIZE           0x18
#define MESHWORLD_INNER_SIZE    0x488

/* Level_RenderCtor — creates a collision Level from a source Level/MeshWorld
 * __thiscall(this, parent_level)
 * Calls Level_ctor (init AthenaLists, Timer, SceneObject) then Level_LoadMeshes
 * (copies MeshBuffers and collision geometry from parent).
 * Parent must have: +0x04 gfx_device, +0x08 MeshWorld*, +0x430 flag,
 * +0x434 Timer*, +0x47C self-ref, +0x480 SceneObject* */
typedef void* (__thiscall *Level_RenderCtor_t)(void* this_, void* parent_level);
static Level_RenderCtor_t pfn_Level_RenderCtor = (Level_RenderCtor_t)0x00465080;

/* Level struct size (same as MESHWORLD_SIZE: 0x10D0) */
#define LEVEL_SIZE             0x10D0

/* PopCylinder_ctor — creates a PopCylinder (bumper) object
 * __thiscall(this, board, posX, posY, posZ, mesh)
 * Stationary object, no path/spline needed */
typedef void* (__thiscall *PopCylinder_ctor_t)(void* this_, void* board, float posX, float posY, float posZ, void* mesh);
static PopCylinder_ctor_t pfn_PopCylinder_ctor = (PopCylinder_ctor_t)0x00436EE0;

/* AthenaList_Append — adds item to an AthenaList
 * __thiscall(list, item) */
/* AthenaList_Append — declared in bass_proxy.h, reusing that typedef */
static AthenaList_Append_t pfn_AthenaList_Append = (AthenaList_Append_t)0x00453810;

/* AthenaList_RemoveByValue — removes item from list by pointer value
 * __thiscall(list, item_value) — at 0x004534D0 */
typedef void (__thiscall *AthenaList_Remove_t)(DWORD* list, int item);
static AthenaList_Remove_t pfn_AthenaList_Remove = (AthenaList_Remove_t)0x004534D0;

/* AthenaList_Clear — clears all entries from an AthenaList (0x453280) */
typedef void (__fastcall *AthenaList_Clear_t)(DWORD* list);
static AthenaList_Clear_t pfn_AthenaList_Clear = (AthenaList_Clear_t)0x00453280;

/* SpatialTree_CloneToLevel / SpatialTree_Cleanup */
typedef void (__thiscall *SpatialTree_CloneToLevel_t)(void* this_);
static SpatialTree_CloneToLevel_t pfn_SpatialTree_CloneToLevel = (SpatialTree_CloneToLevel_t)0x00457AD0;
typedef void (__thiscall *SpatialTree_Cleanup_t)(void* this_);
static SpatialTree_Cleanup_t pfn_SpatialTree_Cleanup = (SpatialTree_Cleanup_t)0x00457A40;

/* Board layout */
#define BOARD_APP               0x878
#define BOARD_LEVEL             0x8AC
#define BOARD_UPDATE_LIST       0x2578
#define BOARD_RENDER_LIST       0xCD4
#define BOARD_COLLISION_LIST   0x10EC
#define BOARD_SCENE_OBJ         0x8B0
#define BOARD_SCENE_UPDATE_LIST 0x8B8  /* Scene_Update iterates this, calls vtable[1] */

/* App layout */
#define APP_GFX_DEVICE          0x174

/* v55j_8: Frame hook for main-thread Gluebie proximity check.
 * App_ResetFrame (0x46C200) runs at START of each frame, BEFORE Ball_Update.
 * This matches native timing — DizzyBoard_Update (board vtable[1]) also runs
 * before Ball_Update. Velocity scaling takes effect on the SAME frame's physics step.
 * Original 9 bytes: 56 8B F1 8B 8E 74 01 00 00
 *   (PUSH ESI; MOV ESI,ECX; MOV ECX,[ESI+0x174])
 * Same PUSHAD/CALL C fn/POPAD + original bytes + JMP back pattern. */
#define PRESENT_HOOK_ADDR       0x0046C200
#define PRESENT_ORIG_BYTES      9

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
#define ROTATER_SIZE            0x1508  /* Rotator_ctor_Impossible alloc size */
/* v55m_48d/v55m_50: cEnt Rotator max rotation speed cap.
 * Native impossible-race Rotator update 0x0043D8C0 clamps the direction
 * field to 20.0 — DAT_004cf370=0x41A00000. The cEnt SWIRL-style rotator's
 * per-frame render angle (obj+0x10E8) is capped at NATIVE_ROTATOR_MAX_SPEED
 * × 0.004 rad/frame to stop the runaway acceleration.
 * v55m_50: raised 20.0 -> 250.0 so 250 × 0.004 = 1.0 rad/frame max
 * (12.5× faster constant spin than the native 20 × 0.004 = 0.08). */
#define NATIVE_ROTATOR_MAX_SPEED 250.0f
#define PENDULUM_SIZE           0x1504
#define LOOPER_SIZE             0x1500
#define GEAR_SIZE               0x1514
#define SPATIALTREE_SIZE        68

/* ═══════════════════════════════════════════════════════════════════════════
 * Entity constructors — all verified via Ghidra decompilation
 *
 * ctor_type values:
 *   0  = PopCylinder_ctor (0x436EE0, size 0x10D0) — fallback for entities with no _ctor
 *   1  = Rotator_ctor (0x435940, size 0x1508) — Y-axis rotation
 *   2  = Pendulum_ctor (0x437700, size 0x1504) — X-axis oscillation
 *   3  = Looper_ctor (0x435800, size 0x1500) — Z-axis rotation
 *   4  = Gear_ctor (0x437690, size 0x1514) — multi-axis
 *   5  = BigGear_ctor (same as 4)
 *   6  = Swirl (Rotator_ctor, constant rotation)
 *   7  = cEnt_DFloor1_ctor (ArenaStands_ctor, 0x43E450, 0x1104) — Neon DFLOOR1
 *   17 = cEnt_DFloor2_ctor (ArenaStands_ctor) — Neon DFLOOR2
 *   18 = cEnt_DFloor3_ctor (ArenaStands_ctor) — Neon DFLOOR3
 *   19 = cEnt_DFloor4_ctor (ArenaStands_ctor + post-config) — Neon DFLOOR4 (obj+0x10DC=2, obj+0x10E0=0)
 *   20 = cEnt_FlickRing_ctor (ArenaStands_ctor) — Neon Arena FLICKRING
 *   21 = cEnt_Trode_ctor (ArenaStands_ctor) — Neon TRODE
 *   8  = GameLevel_ctor (0x4351F0, size 0x1524) — Wobbly platforms
 *   9  = Glass_Level_ctor (0x4384A0, size 0x113C) — Drawbridge (3 params: this,board,mesh)
 *   10 = Gear_Level_ctor (0x43A150, size 0x1100) — Judge (5 params: this,board,x,y,z — no mesh)
 *   11 = Secret_ctor (0x43DFB0, size 0x10EC) — GlassBonus/TENBONUS (6 params: this,board,x,y,z,mesh)
 *   12 = FlagWaver_Ctor (0x46AF30, size 0x8C) — Flag (2 params: this,gfx_device)
 *   13 = Sign_ctor (0x443B90, size 0x10FC) — Popup Sign (complex signature)
 *   14 = WavyFlag2 (Wavy_ctor copy, size 0x1AE7C) — Flag2: uses Flag.MESHWORLD or _default fallback
 *   15 = BadBall_ctor (0x40AFE0, size 0xC70) — 8ball/BadBall (2 params: this, board)
 *   16 = Bridgeslam — isolated Intermediate bridge state machine (no _ctor, custom init)
 *   22 = Chomper_ctor (MeshNode_ctor, 0x471C20, 0x18) — Tower Chomper mesh
 *   23 = Chrome_ctor (no _ctor, board-level behavior, PopCylinder fallback)
 *   24 = Funball_ctor (no _ctor, board-level behavior, PopCylinder fallback)
 *   25 = Tarbubble (no _ctor, no entity spawned — position-only marker, mod handles tar sinking)
 *   26 = Waterwheel (no _ctor, mod loads mesh + rotates per-frame via Gfx_RotateY + mesh vtable)
 *        Default mesh: levels\Waterwheel (user-provided Waterwheel.MESHWORLD),
 *        falls back to levels\_default if missing or invalid (v55m_44h).
 *   27 = Spinner_Level_ctor (0x4396F0, 0x10FC) — Expert Race BRIDGE
 *   28 = Cloudscape (Sprite_ctor, 0x45D0C0, 0xD4) — Sky Race clouds
 *   29 = Gear_ctor (0x437690, 0x1514) — 9 params: (this, board, x, y, z, x2, y2, z2, mesh)
 *   30 = Bell_ctor (0x434D70, 0x10E8) — 5 params: (this, board, x, y, z) — Level_ctor, no mesh
 *   31 = Fan_ctor (0x438C20, 0x1188) — 6 params: (this, board, x, y, z, float) — Level_ctor, no mesh
 *   32 = SawBlade_ctor (0x434660, 0x111C) — 5 params: (this, board, x, y, z) — Level_ctor, no mesh
 *   33 = Bonk_ctor (0x438850, 0x1200) — 5 params: (this, board, x, y, z) — self-loads level5-bonk
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Rotator_ctor_Impossible — creates the spinning SWIRL platform */
typedef void* (__thiscall *Rotator_ctor_t)(void* this_, void* board, float posX, float posY, float posZ, void* mesh);
static Rotator_ctor_t pfn_Rotator_ctor = (Rotator_ctor_t)0x00435940;
static Rotator_ctor_t pfn_Pendulum_ctor = (Rotator_ctor_t)0x437700;
static Rotator_ctor_t pfn_Looper_ctor = (Rotator_ctor_t)0x435800;
static Rotator_ctor_t pfn_Gear_ctor = (Rotator_ctor_t)0x437690;

/* ArenaStands_ctor — Neon Race DFLOOR, FLICKRING, TRODE (all use the same _ctor) */
typedef void* (__thiscall *ArenaStands_ctor_t)(void* this_, void* board, float x, float y, float z, void* mesh);
static ArenaStands_ctor_t pfn_ArenaStands_ctor = (ArenaStands_ctor_t)0x0043E450;

/* Named _ctor wrappers — all call ArenaStands_ctor internally.
 * DFloor4 has extra post-construction config from Neon_CreateDynamicObjects. */
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
    /* Post-construction config from Neon_CreateDynamicObjects (DFLOOR4 case):
     *   obj+0x10DC = 2  (sets collision flag)
     *   obj+0x10E0 = 0  (clears collision object) */
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

/* Spinner_Level_ctor — Expert Race "BRIDGE" (falling bridge piece) */
typedef void* (__thiscall *Spinner_Level_ctor_t)(void* this_, void* board, float x, float y, float z, void* mesh, float param);
static Spinner_Level_ctor_t pfn_Spinner_Level_ctor = (Spinner_Level_ctor_t)0x004396F0;

/* Sprite_ctor — Sky Race cloudscape */
typedef void* (__thiscall *Sprite_ctor_t)(void* this_, void* gfx_device, const char* texture_path);
static Sprite_ctor_t pfn_Sprite_ctor = (Sprite_ctor_t)0x0045D0C0;

/* Gear_ctor — Impossible Race Gear (9 params!) */
typedef void* (__thiscall *Gear_ctor_t)(void* this_, void* board, float x1, float y1, float z1, float x2, float y2, float z2, void* mesh);
static Gear_ctor_t pfn_Gear_ctor_real = (Gear_ctor_t)0x00437690;

/* Bell_ctor — Expert Race Bell (5 params: this, board, x, y, z — no mesh!)
 * Calls Level_ctor internally (no mesh file). Mesh loaded by vtable[1] call. */
typedef void* (__thiscall *Bell_ctor_t)(void* this_, void* board, float x, float y, float z);
static Bell_ctor_t pfn_Bell_ctor = (Bell_ctor_t)0x00434D70;

/* Fan_ctor — Expert Race Fan (6 params: this, board, x, y, z, float — no mesh!)
 * Calls Level_ctor internally. Mesh loaded by vtable[1] call. */
typedef void* (__thiscall *Fan_ctor_t)(void* this_, void* board, float x, float y, float z, float param);
static Fan_ctor_t pfn_Fan_ctor = (Fan_ctor_t)0x00438C20;

/* SawBlade_ctor — Expert Race SawBlade (5 params: this, board, x, y, z — no mesh!)
 * Calls Level_ctor internally. Mesh loaded by vtable[1] call. */
typedef void* (__thiscall *SawBlade_ctor_t)(void* this_, void* board, float x, float y, float z);
static SawBlade_ctor_t pfn_SawBlade_ctor = (SawBlade_ctor_t)0x00434660;

/* Bonk_ctor — Warm-Up Race Bonk (5 params: this, board, x, y, z)
 * Calls Level_MeshWorldCtor("levels\\level5-bonk") — self-loads MESHWORLD! */
typedef void* (__thiscall *Bonk_ctor_t)(void* this_, void* board, float x, float y, float z);
static Bonk_ctor_t pfn_Bonk_ctor = (Bonk_ctor_t)0x00438850;

/* BreakBridge_ctor — Intermediate Race bridge (Pendulum family) */
typedef void* (__thiscall *BreakBridge_ctor_t)(void* this_, void* board, float x, float y, float z, void* mesh);
static BreakBridge_ctor_t pfn_BreakBridge_ctor = (BreakBridge_ctor_t)0x00436D70;

/* Stands_ctor family — entities with their own native _ctor
 * NOTE: These take (this, board, mesh) = 3 params, NOT 6!
 * Do NOT cast to Rotator_ctor_t — that causes stack imbalance. */
typedef void* (__thiscall *StandsCtor_t)(void* this_, void* board, void* mesh);

/* Stands_ctor family variants — different param counts per entity */
/* Lifter_ctor: 7 params (this, board, x, y, z, mesh, lifter_id) */
typedef void* (__thiscall *LifterCtor_t)(void* this_, void* board, float x, float y, float z, void* mesh, int lifter_id);
/* SpeedCylinder_ctor: 7 params (this, board, x, y, z, int_param, mesh) */
typedef void* (__thiscall *SpeedCylCtor_t)(void* this_, void* board, float x, float y, float z, int param, void* mesh);
/* NeonPlatform_ctor: 6 params (this, board, x, y, z, mesh) — same as Rotator_ctor_t */
/* Trapdoor_ctor: 2 params (this, board) */
typedef void* (__thiscall *TrapdoorCtor_t)(void* this_, void* board);
/* Odd_Lifter_ctor: 5 params (this, board, x, y, z) */
typedef void* (__thiscall *OddLifterCtor_t)(void* this_, void* board, float x, float y, float z);
typedef void* (__thiscall *Catapult_ctor_t)(void* this_, void* board, void* mesh);
static Catapult_ctor_t pfn_Catapult_ctor     = (Catapult_ctor_t)0x00437E10;
static StandsCtor_t pfn_Mace_ctor         = (StandsCtor_t)0x00438750;
static StandsCtor_t pfn_Tipper_ctor       = (StandsCtor_t)0x00437960;
static StandsCtor_t pfn_Gluebie_ctor      = (StandsCtor_t)0x00437CB0;
static LifterCtor_t pfn_Lifter_ctor       = (LifterCtor_t)0x00436920;
static SpeedCylCtor_t pfn_SpeedCylinder_ctor = (SpeedCylCtor_t)0x00436A20;
/* TimeButton_ctor: 5 params (this, board, x, y, z, mesh) — same shape as Rotator_ctor_t.
 * Native Up race TimeButton (0x436C10, 0x10E8 bytes, vtable 0x4D5830):
 *   Stands_ctor(this, mesh); then:
 *     +0x10D0 = board, +0x10D4/8/C = pos
 *     +0x10E0 = Level_RenderCtor collision/render Level
 *     puVar1[0x10D] = obj+0x434 mesh ptr  (Level_RenderCtor child mesh ref)
 *     +0x10E4 = 0 (not-yet-pressed latch), +0x10E5 = 1 (render-once pressed pose)
 * Press logic: N:EXTRATIME collision -> Rotator_TriggerSound (0x436CF0):
 *     if (+0x10E4 == 0) { +0x10E4=1; +0x10E5=1; +0x10D8 -= 20.0f (sink); Sound_Play3D(+0x510); }
 * Reward (single-player): timer slot = 500 + "EXTRA TIME:" ScoreObject. */
typedef void* (__thiscall *TimeButtonCtor_t)(void* this_, void* board, float x, float y, float z, void* mesh);
static TimeButtonCtor_t pfn_TimeButton_ctor = (TimeButtonCtor_t)0x00436C10;
static Rotator_ctor_t pfn_NeonPlatform_ctor  = (Rotator_ctor_t)0x0043E110;
static TrapdoorCtor_t pfn_Trapdoor_ctor     = (TrapdoorCtor_t)0x00438290;
static OddLifterCtor_t pfn_Odd_Lifter_ctor   = (OddLifterCtor_t)0x00434E60;

/* Catapult system functions — Ghidra-verified */
/* Catapult_Launch — trigger: sets launching flag + countdown */
typedef void (__fastcall *Catapult_Launch_t)(void* this_);
static Catapult_Launch_t pfn_Catapult_Launch = (Catapult_Launch_t)0x00434290;
/* Catapult_vtable11 — per-frame state machine (wind-up + release) */
typedef void (__fastcall *Catapult_vtable11_t)(void* this_);
static Catapult_vtable11_t pfn_Catapult_vtable11 = (Catapult_vtable11_t)0x00437F10;
/* Catapult_Update — vtable[61]: forward swing animation + velocity */
typedef void (__fastcall *Catapult_Update_t)(void* this_);
static Catapult_Update_t pfn_Catapult_Update = (Catapult_Update_t)0x0043F080;

/* Catapult tracking for per-frame updates */
#define MAX_CATAPULTS 16
typedef struct {
    DWORD obj;
    DWORD board;
    float x, y, z;
    float yaw;       /* v55m_28l: fixed launch yaw from ROT_Y */
    float launch_dx, launch_dy, launch_dz; /* v55m_28i: away direction per trigger */
    int   launching; /* v55m_27e: set to 1 to apply launch force */
    int   cooldown;  /* v55m_27e: frames to wait before re-launching */
    int   was_in_zone; /* v55m_28d: require leaving zone before retrigger */
    DWORD col_obj;   /* v55m_28g: collision Level pointer for E:CATAPULTBOTTOM matching */
    int   collided;  /* v55m_28g: set by DispatchCollisionEvents hook when ball hits bottom mesh */
    int   countdown; /* v55m_42j: 50-frame windup before launch */
    float arm_angle; /* v55m_42p: arm rotation during windup (degrees) */
    DWORD orig_vtable18; /* v55m_42p: saved original vtable[18] for render hook */
    DWORD arm_obj;       /* v55m_42q: separate arm PopCylinder object */
    DWORD arm_orig_vtable18; /* v55m_42q: saved original vtable[18] for arm render hook */
    DWORD arm_mesh;      /* v55m_42q: MeshWorld* for arm (loaded from Level4-Catapult) */
    DWORD arm_active;    /* v55m_42q: arm rotation active */
    int verts_rotating;  /* v55m_43h: 1 = vertex rotation works (matrix hook must be skipped) */
    DWORD orig_verts;    /* v55m_43h: saved copy of original vertex array (MeshWorld+0x448) */
    int orig_vert_count; /* v55m_43h: vertex count */
    DWORD orig_registry[64]; /* v55m_43h rev4: (sub_ptr, orig_copy) pairs, up to 32 sub-meshes */
    DWORD tree_orig;     /* v55m_43h rev5: saved copy of spatial-tree item positions */
    int tree_orig_count; /* v55m_43h rev5: tree item count */
    int tree_rotated;    /* v55m_43m: whether tree-item rotation ran this frame */
    /* v55m_43m: per-list tree state for +0x18 and +0x848 candidates */
    DWORD tree_orig18;   /* saved originals for colLevel+0x18 items */
    int tree_count18;    /* item count at colLevel+0x18 */
    int tree_ok18;       /* rotation succeeded for +0x18 */
    DWORD tree_orig848;  /* saved originals for colLevel+0x848 items */
    int tree_count848;   /* item count at colLevel+0x848 */
    int tree_ok848;      /* rotation succeeded for +0x848 */
    int arm_mb_strips;   /* v55m_43n: strip count of the ARM MeshBuffer (mb[0]) */
    /* v55m_43q: MeshWorld tree list state (mw+0x18) */
    DWORD tree_orig_mw;  /* saved originals for mw+0x18 items */
    int tree_count_mw;   /* item count at mw+0x18 */
    int tree_ok_mw;      /* rotation succeeded for mw+0x18 */
    int rot_dump_ctr;    /* v55m_43s: counter for strip0 position dumps */
    float spin_speed;    /* v55m_43w: rotation speed in rad/frame (Tower-style exponential ramp) */
    int rotating;        /* v55m_43y: rotation active flag — starts at LAUNCH (when sound plays) */
} CatapultState;
static CatapultState g_catapults[MAX_CATAPULTS];
static int g_catapult_count = 0;
static DWORD g_dropin_sample = 0;  /* v55n_2: BASS sample for catapult dropin sound */

/* v55n_2: SpeedCylinder tracking + per-frame slot 11 driver.
 * Native SpeedCylinder (Up race, ctor 0x436A20, vtable 0x4D57D0, 0x150C bytes):
 *   - Stands_ctor(this, mesh) clones spatial trees into obj+0x18 (COLLISION)
 *   - +0x10D0 = board, +0x10D4/8/C = pos, +0x10E0 = Level_RenderCtor (render/collision Level)
 *   - +0x10E4 = speed param (0.0), +0x10E8 = 0, +0x10EC = 0.25 spin, +0x10F0 = AthenaList (tracked balls)
 *   - +0x1508 = launch counter (>175 launches)
 * The collision handler (UpRaceCollisionEvents 0x4119B0) reads [[MeshBuffer]+0x47C] to find
 * the entity, then Pendulum_PlayCollisionSound (0x436B70) tracks the ball in +0x10F0.
 * vtable slot 11 (0x43D8C0) = spin-up 0.25->20.0, 175-frame hold, launch at 65.0 + star trail.
 * The mod's cEnt was missing: (a) MeshBuffer+0x47C->entity link, (b) obj+0x10E0 collision
 * registration, (c) entity+0x47C self-ref, (d) per-frame slot 11 driver. All fixed in v55n_2. */
#define MAX_SPEEDCYLINDERS 16
typedef struct {
    DWORD obj;     /* SpeedCylinder entity (0x150C bytes, vtable 0x4D57D0) */
    DWORD board;   /* owning board */
    float x, y, z; /* spawn position */
    DWORD col_level; /* collision/render Level at obj+0x10E0 */
    DWORD mesh_world; /* the loaded mesh (for MeshBuffer+0x47C fix) */
} SpeedCylState;
static SpeedCylState g_speedcyls[MAX_SPEEDCYLINDERS];
static int g_speedcyl_count = 0;

static int __thiscall cEnt_timebutton_update_noop(void* this_) {
    (void)this_; /* v55n_3: TimeButton vtable[1]+[11] no-op. The board+0x2578
                    update loop (Board_UpdateRaceState 0x41B080) calls
                    vtable[0x2C] (slot 11) and removes objects when the call
                    returns 0 — so the no-op MUST return 1 (keep in list). */
    return 1;
}

/* v55n_3: TimeButton tracking. Native Up race TimeButton (ctor 0x436C10, vtable 0x4D5830).
 * Tracked so the N:EXTRATIME collision handler in our DispatchCollisionEvents hook can
 * find the button entity and replicate Rotator_TriggerSound + timer reward. */
#define MAX_TIMEBUTTONS 16

/* v55n_22: forward decl — translate collision geometry (defined after the render hook). */
static int cEnt_translate_collision_strips(DWORD coll_level, float dx, float dy, float dz, FILE* logf);

/* v55n_22: shared log path (line 1105 in v55n_8). Declared here so the early
 * render hook can log. */
static char g_log_path[MAX_PATH];

typedef struct {
    DWORD obj;        /* TimeButton entity (0x10E8 bytes, vtable 0x4D5830) */
    DWORD board;      /* owning board */
    float x, y, z;    /* spawn position */
    DWORD col_level;  /* collision/render Level at obj+0x10E0 */
    int   pressed;    /* 1 = already pressed (latch mirror) */
    int   geom_translated; /* v55n_22: 1 = collision tree translated to spawn pos */
    DWORD orig_vtable18; /* v55n_5: saved original vtable[18] (0x45E0E0) for render hook */
    /* v55n_22: saved coll-Level mw+0x18 tree originals — the PROVEN catapult
     * pattern. Non-cumulative translate from saved originals every frame. */
    DWORD tree_orig_mw;  /* saved originals for built-coll-Level mw+0x18 items */
    int tree_count_mw;   /* item count at mw+0x18 */
    int tree_ok_mw;      /* translate succeeded for mw+0x18 */
} TimeButtonState;
static TimeButtonState g_timebuttons[MAX_TIMEBUTTONS];
static int g_timebutton_count = 0;
static DWORD g_timebuttons_pos_hook_orig = 0; /* v55n_5: orig vtable[18] of last hooked button (all share 0x45E0E0) */

/* v55n_5: TimeButton render hook (vtable[18] replacement on the private copy).
 * PROBLEM: NOP'ing vtable[11] (0x43DC40) removed the position-set that the
 * native render-once did — Timer_Init + timer vtable[2] (Gfx_SetPosition
 * 0x457B50) + the crashy 0x46DF80/0x46DF90 pair. Result: the button renders
 * at 0,0,0 (its SceneObject world matrix is identity).
 * FIX: Set the world matrix translation to (obj+0x10D4/8/C) BEFORE calling
 * the original render 0x45E0E0 — same proven pattern as cEnt_catapult_render
 * (save matrix -> write translation -> original render -> restore matrix).
 * The world matrix lives at renderLevel+0x4 where renderLevel = obj+0x434
 * (Stands layout: +0x434 = operator_new'd Timer object that acts as the
 * render/view level, ACTUAL matrix at +0x438 = renderLevel+4). */
static void __thiscall cEnt_timebutton_render(DWORD this_, char param_1, int param_2) {
    TimeButtonState* tb = NULL;
    int i;
    for (i = 0; i < g_timebutton_count; i++) {
        if (g_timebuttons[i].obj == this_) { tb = &g_timebuttons[i]; break; }
    }
    if (!tb || !tb->orig_vtable18) {
        typedef void (__thiscall *render_t)(DWORD, char, int);
        ((render_t)0x0045E0E0)(this_, param_1, param_2);
        return;
    }
    /* v55n_22: geometry translation moved OUT of this hook into the Present
     * driver (gluebie_present_helper) — this render hook is NOT guaranteed to
     * fire (the cEnt button is never registered in a render/update list), and
     * a latch here with 0 built verts would pin geom_translated prematurely.
     * driver retries until the buffers are built. Visual world
     * matrix position is still applied below. */
    DWORD renderLevel = 0;
    if (!IsBadReadPtr((void*)(this_ + 0x434), 4)) {
        renderLevel = *(DWORD*)(this_ + 0x434);
    }
    if (renderLevel && !IsBadReadPtr((void*)(renderLevel + 0x4), 64)) {
        float saveMatrix[16];
        memcpy(saveMatrix, (float*)(renderLevel + 0x4), sizeof(saveMatrix));
        /* Set translation to the spawn position. Only modify the 4th row
         * (row-major D3D matrix: [12],[13],[14]) to keep scale/rotation. */
        float* m = (float*)(renderLevel + 0x4);
        m[12] = tb->x;
        m[13] = tb->y;
        m[14] = tb->z;
        typedef void (__thiscall *render_t)(DWORD, char, int);
        ((render_t)tb->orig_vtable18)(this_, param_1, param_2);
        memcpy((float*)(renderLevel + 0x4), saveMatrix, sizeof(saveMatrix));
        return;
    }
    typedef void (__thiscall *render_t)(DWORD, char, int);
    ((render_t)(tb->orig_vtable18 ? tb->orig_vtable18 : 0x0045E0E0))(this_, param_1, param_2);
    return;
}

/* v55n_22+: Translate a collision Level's collision geometry so the ball hits
 * where the render shows it. Confirmed via Ghidra MeshWorld_BuildVertexBuffer
 * (0x46F8D0) + the proven catapult rotation (cEnt_catapult_rotate_collision_verts):
 *   MeshWorld+0x2C = MeshBuffer AthenaList (count +0x30, items +0x438)
 *   MeshBuffer+0x424 = sub-mesh AthenaList (count +0x428, items +0x830)
 *   sub-mesh+0x448 = source vertex array (8 floats/vertex) — THE TREE SOURCE
 *   sub-mesh+0x4  = triangle count (vertex count = tri+2)
 *   MeshBuffer+0x10 = strip count, MeshBuffer+0x418 = strip items (96B each,
 *   3 verts x 32B, X/Y/Z at +0/+4/+8)
 * Both the sub-mesh +0x448 arrays AND the strips feed the collision query. We
 * translate BOTH at ctor time (before the first render builds the packed buffer).
 * v55n_6/v55n_7 read wrong list offsets -> 0 verts; v55n_8 mutated octree node
 * items -> crash. This is safe vertex-data translation. Returns verts translated. */
static int cEnt_translate_collision_strips(DWORD coll_level, float dx, float dy, float dz, FILE* logf) {
    /* v55n_22>: granular diagnostics — log EVERY early-return so a level-start
     * test log reveals exactly which offset/check fails (v55n_22 showed no
     * "geom translated" line at all = silent early return). */
    if (logf) fprintf(logf, "  ROTATER: TBtranslate enter coll=0x%08X d=(%.1f,%.1f,%.1f)\n", coll_level, dx, dy, dz);
    if (!coll_level || coll_level < 0x10000 || IsBadReadPtr((void*)coll_level, 0x100)) { if(logf) fprintf(logf,"  ROTATER:   TBtx fail: bad coll_level\n"); return 0; }
    DWORD mw = *(DWORD*)((char*)coll_level + 0x08);
    if (logf) fprintf(logf, "  ROTATER:   TBtx mw=0x%08X\n", mw);
    if (!mw || mw < 0x10000 || IsBadReadPtr((void*)mw, 0x460)) { if(logf) fprintf(logf,"  ROTATER:   TBtx fail: bad mw\n"); return 0; }
    int total = 0;

    /* v55n_22: REMOVED the collision TREE-ITEM translation here. It crashed
     * every time it ran (v55n_8 at ctor, v55n_22 at Update 0001:0004717E).
     * confirmed: 0x44717E crash EIP is mid-instruction = SEH-resume of heap
     * corruption from writing game-owned collision tree items at
     * coll_level+0x18/0x848/mw+0x18 (the catapult's "tree rotation" operates
     * on a differently-owned/per-frame-rebuilt structure). Translating ONLY
     * the vertex source arrays below (sub-mesh +0x448 + strips) is safe AND
     * sufficient: the balloon's collision tree is rebuilt EVERY frame from
     * these source arrays (SpatialTree_ctor 0x463330 / Ball_Update 0x405E00),
     * so translating the source before the tree builds yields correct positions
     * without ever touching game-owned tree memory. Do NOT add tree writes back. */
    int mb_count = *(int*)((char*)mw + 0x30);
    if (mb_count <= 0 || mb_count > 64) return 0;
    DWORD* mb_items = *(DWORD**)((char*)mw + 0x438);
    if (!mb_items || IsBadReadPtr((void*)mb_items, mb_count * 4)) return 0;
    int bi;
    for (bi = 0; bi < mb_count; bi++) {
        DWORD mb = mb_items[bi];
        if (!mb || mb < 0x10000 || IsBadReadPtr((void*)mb, 0x850)) continue;
        /* 1. Sub-mesh +0x448 source arrays (the tree source). */
        int scnt = *(int*)((char*)mb + 0x428);
        if (scnt > 0 && scnt <= 4096) {
            DWORD* sitems = *(DWORD**)((char*)mb + 0x830);
            if (sitems && !IsBadReadPtr((void*)sitems, scnt * 4)) {
                int si;
                for (si = 0; si < scnt; si++) {
                    DWORD sub = sitems[si];
                    if (!sub || sub < 0x10000 || IsBadReadPtr((void*)sub, 0x20)) continue;
                    DWORD tri = *(DWORD*)((char*)sub + 0x04);
                    int vcnt = (int)tri + 2;
                    if (vcnt <= 0 || vcnt > 65536) continue;
                    float* verts = *(float**)((char*)sub + 0x448);
                    if (verts && !IsBadReadPtr((void*)verts, vcnt * 32)) {
                        int vi;
                        for (vi = 0; vi < vcnt; vi++) {
                            verts[vi * 8 + 0] += dx;
                            verts[vi * 8 + 1] += dy;
                            verts[vi * 8 + 2] += dz;
                        }
                        total += vcnt;
                    }
                    /* Also the transient +0x10 array if still present. */
                    float* verts2 = *(float**)((char*)sub + 0x10);
                    if (verts2 && verts2 != verts && !IsBadReadPtr((void*)verts2, vcnt * 32)) {
                        int vi2;
                        for (vi2 = 0; vi2 < vcnt; vi2++) {
                            verts2[vi2 * 8 + 0] += dx;
                            verts2[vi2 * 8 + 1] += dy;
                            verts2[vi2 * 8 + 2] += dz;
                        }
                    }
                }
            }
        }
        /* 2. Strip vertices (catapult-proven offsets). */
        int strip_count = *(int*)((char*)mb + 0x10);
        if (strip_count > 0 && strip_count <= 4096) {
            DWORD* strip_items = *(DWORD**)((char*)mb + 0x418);
            if (strip_items && !IsBadReadPtr((void*)strip_items, strip_count * 4)) {
                int si2;
                for (si2 = 0; si2 < strip_count; si2++) {
                    DWORD strip = strip_items[si2];
                    if (!strip || strip < 0x10000 || IsBadReadPtr((void*)strip, 0x60)) continue;
                    float* p = (float*)strip;
                    int v;
                    for (v = 0; v < 3; v++) {
                        p[v * 8 + 0] += dx;
                        p[v * 8 + 1] += dy;
                        p[v * 8 + 2] += dz;
                    }
                    total += 3;
                }
            }
        }
    }
    if (logf) fprintf(logf, "  ROTATER: TimeButton coll Level 0x%08X geom translated (%.1f,%.1f,%.1f) %d verts\n",
                      coll_level, dx, dy, dz, total);
    return total;
}

/* v55n_22: TimeButton collision-tree translate — the PROVEN catapult pattern.
 * The catapult translates the BUILT collision Level's mw+0x18 tree items
 * (mw = *(DWORD*)(collLevel+0x08)) NON-CUMULATIVELY from saved originals every
 * frame. This is crash-free because it never mutates the source mesh in place
 * (v55n_22's mistake -> 0001:000570A4) — it writes the built Level's tree from
 * saved copies, idempotently. For TimeButton we TRANSLATE (not rotate) by
 * (x,y,z) = spawn offset. Returns 1 if tree_ok. */
static int cEnt_timebutton_translate_tree(TimeButtonState* tb, FILE* logf) {
    if (!tb || !tb->obj) return 0;
    DWORD col_level = tb->col_level ? tb->col_level : *(DWORD*)((char*)tb->obj + 0x10E0);
    if (!col_level || col_level < 0x10000 || IsBadReadPtr((void*)col_level, 0x100)) return 0;
    DWORD mw = *(DWORD*)((char*)col_level + 0x08);
    if (!mw || mw < 0x10000 || IsBadReadPtr((void*)mw, 0x460)) return 0;

    DWORD mwlist = mw + 0x18;   /* embedded AthenaList: count +0x4, items +0x40C */
    if (IsBadReadPtr((void*)mwlist, 0x20)) return 0;
    int tcount = *(int*)(mwlist + 0x4);
    if (tcount <= 0 || tcount >= 65536) { if(logf)fprintf(logf,"  ROTATER: TBtx tree count=%d (bad)\n", tcount); return 0; }
    DWORD* titems = *(DWORD**)(mwlist + 0x40C);
    if (!titems || IsBadReadPtr((void*)titems, tcount * 4)) { if(logf)fprintf(logf,"  ROTATER: TBtx bad tree items ptr\n"); return 0; }

    /* Lazy-save originals (12 bytes/item). */
    if (!tb->tree_orig_mw) {
        tb->tree_orig_mw = (DWORD)malloc(tcount * 12);
        if (tb->tree_orig_mw) {
            float* dst = (float*)tb->tree_orig_mw;
            int ti;
            for (ti = 0; ti < tcount; ti++) {
                DWORD item = titems[ti];
                if (!item || item < 0x10000 || IsBadReadPtr((void*)item, 0x10)) continue;
                float* src = (float*)item;
                dst[ti*3+0] = src[0]; dst[ti*3+1] = src[1]; dst[ti*3+2] = src[2];
            }
            tb->tree_count_mw = tcount;
        }
    }
    if (!tb->tree_orig_mw || tb->tree_count_mw != tcount) { if(logf)fprintf(logf,"  ROTATER: TBtx save failed\n"); return 0; }

    /* Non-cumulative translate from saved originals. */
    float* tsrc = (float*)tb->tree_orig_mw;
    int ti, done = 0;
    for (ti = 0; ti < tcount; ti++) {
        DWORD item = titems[ti];
        if (!item || item < 0x10000 || IsBadReadPtr((void*)item, 0x10)) continue;
        float* o = tsrc + ti*3;
        float* p = (float*)item;
        p[0] = o[0] + tb->x;
        p[1] = o[1] + tb->y;
        p[2] = o[2] + tb->z;
        done = 1;
    }
    if (done) tb->tree_ok_mw = 1;
    if (logf) fprintf(logf, "  ROTATER: TBtx translated %d tree items to (%.1f,%.1f,%.1f)\n", tcount, tb->x, tb->y, tb->z);
    return tb->tree_ok_mw;
}

/* v55n_22: Translate a loaded MeshWorld's VERTEX SOURCE by (dx,dy,dz) BEFORE
 * any entity ctor clones spatial trees from it. This is THE clean native-matching
 * fix for solidity: TimeButton_ctor -> Stands_ctor builds obj+0x18 collision trees
 * from the mesh at the time of construction. If the mesh is modeled near-origin
 * (cEnt LevelUp-Button) but spawned at the ref point (778.5,-230.4,-522.5), the
 * tree concludes AT ORIGIN -> non-solid. By translating the vertex source BEFORE
 * calling the ctor, Stands_ctor builds the tree AT THE SPAWN POSITION -> solid,
 * zero game-owned tree writes, zero crash (translating already-built tree items
 * at v55n_8/v55n_11 corrupted heap; the catapult "tree rotation" works only on a
 * per-frame-rebuilt structure).
 * Operates directly on a MeshWorld*: mw+0x2C = MeshBuffer list, +0x38 count /
 * +0x438 items (verified), MeshBuffer+0x424 = sub-mesh list (+0x428 count,
 * +0x830 items), sub-mesh+0x448 = source verts (8 floats/vert). Returns verts. */
static int cEnt_translate_meshworld_verts(DWORD mw, float dx, float dy, float dz, FILE* logf) {
    if (logf) fprintf(logf, "  ROTATER: TBtranslate meshworld enter mw=0x%08X d=(%.1f,%.1f,%.1f)\n", mw, dx, dy, dz);
    if (!mw || mw < 0x10000 || IsBadReadPtr((void*)mw, 0x460)) { if(logf) fprintf(logf,"  ROTATER:   TBtx fail: bad mw\n"); return 0; }
    int total = 0;
    int translated_something = 0;  /* v55n_22: any tree OR strip translated */

    /* ═══ v55n_22: +0x18 collision TREE FIRST, UNCONDITIONALLY ═══
     * Stands_ctor (0x462850) at 0x462937 does `add $0x18,%edi` then iterates
     * [edi+0x4]/[edi+0x40C] — i.e. mesh+0x18 is an EMBEDDED AthenaList (count
     * +0x4, items +0x40C), and it CLONES every item into this+0x18 (build-obj
     * tree). Those cloned items ARE the collision (broad-phase + exact test).
     * THIS is what must be translated. It is INDEPENDENT of the +0x2C
     * MeshBuffer/strip list — which at load-time is empty (Level_MeshWorldCtor
     * sets +0x18=15 count but the strips are lazy). v55n_22's bug: it checked
     * +0x2C FIRST, got 0, and return 0ed BEFORE ever translating +0x18 -> button
     * still non-solid. Now +0x18 runs first and is NOT gated by +0x2C. */
    {
        int tcnt = *(int*)((char*)mw + 0x18 + 0x4);
        if (logf) fprintf(logf, "  ROTATER:   TBtx tree_items(at +0x18) count=%d\n", tcnt);
        if (tcnt > 0 && tcnt < 65536) {
            DWORD* titems = *(DWORD**)((char*)mw + 0x18 + 0x40C);
            if (titems && !IsBadReadPtr((void*)titems, tcnt * 4)) {
                int ti;
                int done = 0;
                for (ti = 0; ti < tcnt; ti++) {
                    DWORD item = titems[ti];
                    if (!item || item < 0x10000 || IsBadReadPtr((void*)item, 0x10)) continue;
                    float* p = (float*)item;
                    p[0] += dx;
                    p[1] += dy;
                    p[2] += dz;
                    total += 1;
                    done = 1;
                }
                if (done) translated_something = 1;
            } else {
                if (logf) fprintf(logf, "  ROTATER:   TBtx fail: bad tree_items pointer\n");
            }
        }
    }

    /* ═══ v55n_22/15: +0x2C MeshBuffer/strip list (belt-and-suspenders) ═══
     * The standalone loaded MeshWorld stores its MeshBuffer list as an EMBEDDED
     * AthenaList at mw+0x2C (count +0x4, items +0x40C) — the proven catapult
     * offsets. This is the strip/vertex data (Mesh_FindClosestCollision walks
     * it). It can be EMPTY at load (lazy) — do NOT gate correctness on it. */
    int mb_count = *(int*)((char*)mw + 0x2C + 0x4);
    if (logf) fprintf(logf, "  ROTATER:   TBtx mb_count=%d (AthenaList@+0x2C count+0x4)\n", mb_count);
    if (mb_count > 0 && mb_count <= 64) {
    DWORD* mb_items = *(DWORD**)((char*)mw + 0x2C + 0x40C);
    if (mb_items && !IsBadReadPtr((void*)mb_items, mb_count * 4)) {
    int bi;
    for (bi = 0; bi < mb_count; bi++) {
        DWORD mb = mb_items[bi];
        if (!mb || mb < 0x10000 || IsBadReadPtr((void*)mb, 0x850)) { if(logf) fprintf(logf,"  ROTATER:   TBtx skip mb %d bad\n", bi); continue; }
        /* sub-mesh +0x448 source arrays */
        int scnt = *(int*)((char*)mb + 0x428);
        if (scnt > 0 && scnt <= 4096) {
            DWORD* sitems = *(DWORD**)((char*)mb + 0x830);
            if (sitems && !IsBadReadPtr((void*)sitems, scnt * 4)) {
                int si;
                for (si = 0; si < scnt; si++) {
                    DWORD sub = sitems[si];
                    if (!sub || sub < 0x10000 || IsBadReadPtr((void*)sub, 0x20)) continue;
                    DWORD tri = *(DWORD*)((char*)sub + 0x04);
                    int vcnt = (int)tri + 2;
                    if (vcnt <= 0 || vcnt > 65536) continue;
                    float* verts = *(float**)((char*)sub + 0x448);
                    if (verts && !IsBadReadPtr((void*)verts, vcnt * 32)) {
                        int vi;
                        for (vi = 0; vi < vcnt; vi++) {
                            verts[vi * 8 + 0] += dx;
                            verts[vi * 8 + 1] += dy;
                            verts[vi * 8 + 2] += dz;
                        }
                        total += vcnt;
                    }
                    /* transient +0x10 array if present and different */
                    float* verts2 = *(float**)((char*)sub + 0x10);
                    if (verts2 && verts2 != verts && !IsBadReadPtr((void*)verts2, vcnt * 32)) {
                        int vi2;
                        for (vi2 = 0; vi2 < vcnt; vi2++) {
                            verts2[vi2 * 8 + 0] += dx;
                            verts2[vi2 * 8 + 1] += dy;
                            verts2[vi2 * 8 + 2] += dz;
                        }
                    }
                }
            }
        }
        /* strip vertices (catapult-proven: MeshBuffer+0x10 count, +0x418 items) */
        int strip_count = *(int*)((char*)mb + 0x10);
        if (strip_count > 0 && strip_count <= 4096) {
            DWORD* strip_items = *(DWORD**)((char*)mb + 0x418);
            if (strip_items && !IsBadReadPtr((void*)strip_items, strip_count * 4)) {
                int si2;
                for (si2 = 0; si2 < strip_count; si2++) {
                    DWORD strip = strip_items[si2];
                    if (!strip || strip < 0x10000 || IsBadReadPtr((void*)strip, 0x60)) continue;
                    float* p = (float*)strip;
                    int v;
                    for (v = 0; v < 3; v++) {
                        p[v * 8 + 0] += dx;
                        p[v * 8 + 1] += dy;
                        p[v * 8 + 2] += dz;
                    }
                    total += 3;
                }
            }
        }
    }      /* end for(bi) mb loop */
    }      /* end if (mb_items valid) */
    }      /* end if (mb_count > 0) */
    if (logf) fprintf(logf, "  ROTATER: TB meshworld translated (%.1f,%.1f,%.1f) %d verts\n", dx, dy, dz, total);
    return total;
}


/* v55n_2: Native SpeedCylinder slot 11 = ImpossibleRotator_Update (0x43D8C0).
 * __fastcall(this) — spin-up + 175-frame hold + launch at 65.0 + star trail + render transform. */
typedef int (__fastcall *SpeedCyl_slot11_t)(void* this_);
static SpeedCyl_slot11_t pfn_SpeedCyl_slot11 = (SpeedCyl_slot11_t)0x0043D8C0;
static DWORD g_catapult_sample = 0;  /* v55m_42i: BASS sample for catapult launch sound */
static void* g_dropin_sound = NULL; /* v55m_28l cached sounds\\dropin channel */

/* v55m_2: Chomper tracking — Tower Race chomper entity.
 * v55m_18: Custom vtable[2] for jaw animation (same pattern as native
 * Scene_RenderWithCamera: Timer_Init → Gfx_ScaleZ → Gfx_SetPosition →
 * original BuildStrips → Timer_Cleanup). */
#define MAX_CHOMPERS 16
typedef struct {
    DWORD obj;          /* PopCylinder* (in game render list) */
    DWORD orig_vtable2; /* saved original vtable[2] (BuildStrips) */
    float x, y, z;       /* Chomper position (Y -= 20.0 from S1 ref point) */
    float jaw_angle;     /* board+0x43A0 equivalent (init 0.25) */
    float phase;         /* board+0x43A4 equivalent (+= 3.0/frame for Wave_Sin) */
    int   state;         /* board+0x43A8 equivalent (0=opening, 1=bite, 2=closing, 3=idle) */
    int   countdown;     /* board+0x43AC equivalent */
    float anim_val;      /* board+0x43B0 equivalent */
} ChomperState;
static ChomperState g_chompers[MAX_CHOMPERS];
static int g_chomper_count = 0;

/* Custom vtable[18] for Chomper — forward declaration.
 * Uses __thiscall to match game's vtable calling convention. */
static void __thiscall cEnt_chomper_render(DWORD this_, char param_1, int param_2);

/* v55m_42v: REMOVED — arm render hook was unused. Main catapult vtable[18] is hooked directly.
 * Kept as forward declaration only for compatibility. */

/* v55m_42w: Draw-phase guard flag. Stands vtable[18] is called during Update AND Draw.
 * D3D transforms only work during Draw. Set=1 in Graphics_RenderScene hook, 0 in Present hook. */
static int g_in_draw_phase = 0;

/* v55m_42w: Forward declaration */
static void __cdecl renderscene_helper(void);
static int game_is_quitting(void);  /* v55m_44o: fwd decl (defined at 1587, used at 1128) */

/* v55m_42w: RenderScene hook variables */
static BYTE *g_renderscene_cave = NULL;
static int g_renderscene_hook_installed = 0;

/* v55m_42w: Function pointer for the RenderScene cave */
static void (__cdecl *g_renderscene_fn_ptr)(void) = NULL;

/* v55m_42w: Graphics_RenderScene hook — code cave at 0x454BC0.
 * Prologue: SUB ESP,0xC0 (6 bytes). Cave sets g_in_draw_phase=1, then runs original.
 * This is called DURING Draw phase, after all normal rendering is set up.
 * Stands vtable[18] is called during Update AND Draw — we use this flag to
 * skip D3D transforms during Update. */
static void install_renderscene_hook(void) {
    if (g_renderscene_hook_installed) return;
    BYTE *hook_addr = (BYTE*)0x00454BC0;
    BYTE expected[] = { 0x81, 0xEC, 0xC0, 0x00, 0x00, 0x00 };
    if (memcmp(hook_addr, expected, 6) != 0) return;

    g_renderscene_cave = (BYTE*)VirtualAlloc(NULL, 64, MEM_COMMIT | MEM_RESERVE,
                                              PAGE_EXECUTE_READWRITE);
    if (!g_renderscene_cave) return;

    g_renderscene_fn_ptr = renderscene_helper;

    int p = 0;
    /* PUSHAD + PUSHFD */
    g_renderscene_cave[p++] = 0x60;
    g_renderscene_cave[p++] = 0x9C;
    /* CALL [g_renderscene_fn_ptr] */
    g_renderscene_cave[p++] = 0xFF; g_renderscene_cave[p++] = 0x15;
    *(DWORD*)(g_renderscene_cave + p) = (DWORD)&g_renderscene_fn_ptr; p += 4;
    /* POPFD + POPAD */
    g_renderscene_cave[p++] = 0x9D;
    g_renderscene_cave[p++] = 0x61;
    /* Original first instruction: SUB ESP, 0xC0 */
    g_renderscene_cave[p++] = 0x81; g_renderscene_cave[p++] = 0xEC;
    g_renderscene_cave[p++] = 0xC0; g_renderscene_cave[p++] = 0x00;
    g_renderscene_cave[p++] = 0x00; g_renderscene_cave[p++] = 0x00;
    /* JMP back to hook_addr + 6 */
    g_renderscene_cave[p++] = 0xE9;
    *(DWORD*)(g_renderscene_cave + p) = (DWORD)(hook_addr + 6) - (DWORD)(g_renderscene_cave + p + 4);
    p += 4;

    DWORD old_protect;
    VirtualProtect(hook_addr, 6, PAGE_EXECUTE_READWRITE, &old_protect);
    hook_addr[0] = 0xE9;
    *(DWORD*)(hook_addr + 1) = (DWORD)(g_renderscene_cave - hook_addr - 5);
    hook_addr[5] = 0x90;
    VirtualProtect(hook_addr, 6, old_protect, &old_protect);
    FlushInstructionCache(GetCurrentProcess(), hook_addr, 6);
    g_renderscene_hook_installed = 1;
}

static void uninstall_renderscene_hook(void) {
    if (!g_renderscene_hook_installed) return;
    BYTE *hook_addr = (BYTE*)0x00454BC0;
    DWORD old_protect;
    if (VirtualProtect(hook_addr, 6, PAGE_EXECUTE_READWRITE, &old_protect)) {
        hook_addr[0] = 0x81; hook_addr[1] = 0xEC;
        hook_addr[2] = 0xC0; hook_addr[3] = 0x00;
        hook_addr[4] = 0x00; hook_addr[5] = 0x00;
        VirtualProtect(hook_addr, 6, old_protect, &old_protect);
        FlushInstructionCache(GetCurrentProcess(), hook_addr, 6);
    }
    g_renderscene_hook_installed = 0;
}

/* GameLevel_ctor — Wobbly Race platforms */
typedef void* (__thiscall *GameLevel_ctor_t)(void* this_, void* board, float x, float y, float z, void* mesh);
static GameLevel_ctor_t pfn_GameLevel_ctor = (GameLevel_ctor_t)0x004351F0;

/* Glass_Level_ctor — Tower Race Drawbridge (3 params: this, board, mesh — no position!) */
typedef void* (__thiscall *Glass_Level_ctor_t)(void* this_, void* board, void* mesh);
static Glass_Level_ctor_t pfn_Glass_Level_ctor = (Glass_Level_ctor_t)0x004384A0;

/* Gear_Level_ctor — Expert Race Judge (5 params: this, board, x, y, z — no mesh!) */
typedef void* (__thiscall *Gear_Level_ctor_t)(void* this_, void* board, float x, float y, float z);
static Gear_Level_ctor_t pfn_Gear_Level_ctor = (Gear_Level_ctor_t)0x0043A150;

/* Secret_ctor — Glass Race TENBONUS (6 params: this, board, x, y, z, mesh) */
typedef void* (__thiscall *Secret_ctor_t)(void* this_, void* board, float x, float y, float z, void* mesh);
static Secret_ctor_t pfn_Secret_ctor = (Secret_ctor_t)0x0043DFB0;

/* FlagWaver_Ctor — Flag (2 params: this, gfx_device) — code-generated mesh */
typedef void* (__thiscall *FlagWaver_Ctor_t)(void* this_, void* gfx_device);
static FlagWaver_Ctor_t pfn_FlagWaver_Ctor = (FlagWaver_Ctor_t)0x0046AF30;

/* TipperVisual_Attach — links visual object to behavior object
 * v55b: Changed from __cdecl to __thiscall (ECX=visual/this, stack=behavior).
 * The function starts with MOV ESI,ECX and RET 0x4 — classic __thiscall. */
typedef void (__thiscall *TipperVisual_Attach_t)(void* this_, int behavior);
static TipperVisual_Attach_t pfn_TipperVisual_Attach = (TipperVisual_Attach_t)0x00465200;

/* Sign_ctor — Popup Sign (complex signature, handled specially) */
typedef void* (__thiscall *Sign_ctor_t)(void* this_, void* board, int gfx1, int gfx2,
    int x, int y, int z, int str1, int str2, int str3);
static Sign_ctor_t pfn_Sign_ctor = (Sign_ctor_t)0x00443B90;

/* Wavy_ctor — Wavy platforms (takes string path, not mesh pointer!)
 * Signature: __thiscall(this, board, x, y, z, mesh_path_string)
 * Alloc size: 0x1AE7C (110,204 bytes — huge!)
 * Wavy_Configure is called after construction to set wave parameters */
typedef void* (__thiscall *Wavy_ctor_t)(void* this_, void* board, float x, float y, float z, const char* mesh_path);
static Wavy_ctor_t pfn_Wavy_ctor = (Wavy_ctor_t)0x0043AD40;

typedef void (__thiscall *Wavy_Configure_t)(void* this_, int a, float b, float c, float d);
static Wavy_Configure_t pfn_Wavy_Configure = (Wavy_Configure_t)0x00435440;

/* BadBall_ctor — 8ball/BadBall (2 params: this, board — position set by CreateBadBalls) */
typedef void* (__thiscall *BadBall_ctor_t)(void* this_, void* board);
static BadBall_ctor_t pfn_BadBall_ctor = (BadBall_ctor_t)0x0040AFE0;

/* ── Bridgeslam game function pointers ── */
/* Level_RenderCtor is already declared above — reuse it */

/* TipperVisual_ctor — creates visual object from parent mesh */
typedef void* (__thiscall *TipperVisual_ctor_t)(void* this_, int parent_mesh);
static TipperVisual_ctor_t pfn_TipperVisual_ctor2 = (TipperVisual_ctor_t)0x004661A0;

/* Sound_Play3D — plays a 3D positioned sound
 * __thiscall: ECX = soundChannel (this), 4 stack params (RET 0x10 = 16 bytes)
 * Native call site (0x41D9C2): push 1.0, push z, push y, push x, ECX=[App+0x484]
 * The 4th param (scale) defaults to 1.0 — controls volume/distance scaling. */
typedef void (__thiscall *Sound_Play3D_t)(void* soundChannel, float x, float y, float z, float scale);
static Sound_Play3D_t pfn_Sound_Play3D = (Sound_Play3D_t)0x00459860;
/* Sound_LoadOggOrWav — load a sound channel (ECX=soundList, path no ext) */
typedef void* (__thiscall *Sound_LoadOggOrWav_t)(DWORD soundList, const char* path);
static Sound_LoadOggOrWav_t pfn_Sound_LoadOggOrWav = (Sound_LoadOggOrWav_t)0x00459660;
/* v55m_44c: Sound_GetChannel (0x459810) — round-robin channel allocator.
 * ECX = sound slot (e.g. [soundList]+0x490 = wheelcreak). Returns a playable
 * channel in EAX, or 0 if the slot/count is invalid. Native Dizzy calls it at
 * 0x41D34C: MOV ECX,[board+0x878]+0x490; CALL 0x459810; result stored at
 * board+0x4BDC, played per-frame via Sound_Play3D. */
typedef void* (__thiscall *Sound_GetChannel_t)(DWORD soundSlot);
static Sound_GetChannel_t pfn_Sound_GetChannel = (Sound_GetChannel_t)0x00459810;

/* Sound_PlayChannel — play a non-3D sound channel (App+0x460 = sounds\dropin) */
typedef void (__fastcall *Sound_PlayChannel_t)(void* soundChannel);
static Sound_PlayChannel_t pfn_Sound_PlayChannel = (Sound_PlayChannel_t)0x004597B0;

/* Gfx_ScaleZ — apply Z-axis rotation to gfx device (thiscall with gfx ptr) */
typedef void (__thiscall *Gfx_ScaleFn_t)(void* gfx, float val);
static Gfx_ScaleFn_t pfn_Gfx_ScaleZ_Bridge = (Gfx_ScaleFn_t)0x00457CC0;

/* Gfx_SetPosition — set gfx device position */
typedef void (__thiscall *Gfx_SetPosition_t)(void* gfx, float x, float y, float z);
static Gfx_SetPosition_t pfn_Gfx_SetPosition_Bridge = (Gfx_SetPosition_t)0x00457B50;

/* v55f: Gfx_RotateY — build Y-rotation matrix (deg→rad internally)
 * __thiscall(ECX=matrix_buf, [ESP+4]=angle_degrees)
 * Writes 4x4 matrix to ECX+0x04. RET 4. */
typedef void (__thiscall *Gfx_RotateY_t)(void* matrix_buf, float angle_degrees);
static Gfx_RotateY_t pfn_Gfx_RotateY = (Gfx_RotateY_t)0x00457C90;

/* Timer_Init / Timer_Cleanup — timer context for render transforms.
 * v55m_26: FIXED SWAPPED POINTERS! Was: Init=0x457AD0, Cleanup=0x457A40
 * Timer_Init at 0x457A40: sets vtable ptr, RET 0, __thiscall(ECX=timer_buf)
 * Timer_Cleanup at 0x457A50: restores transform, RET 4, __thiscall(ECX=timer_buf) */
typedef void (__thiscall *Timer_Init_t)(void* timer_buf);
static Timer_Init_t pfn_Timer_Init = (Timer_Init_t)0x00457A40;
typedef void (__thiscall *Timer_Cleanup_t)(void* timer_buf);
static Timer_Cleanup_t pfn_Timer_Cleanup = (Timer_Cleanup_t)0x00457A50;

/* v55n: Native decorative TarBubble object (Ghidra-verified 2026-08-05).
 * 0x1C-byte object, vtable 0x4D6E48 (SimpleList-family bubble).
 *   +0x00 vtable, +0x04 app, +0x08/+0x0C/+0x10 pos x/y/z,
 *   +0x14 scale (start 25.0), +0x18 lifetime (RNG+25, frames),
 *   +0x1C scale2 (used by splotch render variant)
 * ctor FUN_0044fb50: __thiscall(this=ECX, app, x, y, z) RET 0x14 — sets
 *   vtable, app, pos, scale=25.0 (0x41C80000), lifetime=RNG(0x19)+0x19.
 * update FUN_0044fbe0 (vtable[1]): __fastcall(this) — scale *= 0.95,
 *   lifetime -= 1.0, plays bubble1 pop sound (app+0x488) when expired.
 * render FUN_0044f910 (vtable[2]): __fastcall(this) — Timer_Init,
 *   Gfx_SetPosition(x, y - scale*60, z), sprite vtable[0x1C](scale, 0),
 *   Timer_Cleanup. (60.0 = 0x4D039C, scale*60 rises the bubble.) */
typedef void* (__thiscall *TarBubble_ctor_t)(void* this_, DWORD app,
                                              float x, float y, float z);
static TarBubble_ctor_t pfn_TarBubble_ctor = (TarBubble_ctor_t)0x0044FB50;
typedef void (__fastcall *TarBubble_update_t)(DWORD this_);
static TarBubble_update_t pfn_TarBubble_update = (TarBubble_update_t)0x0044FBE0;
typedef void (__fastcall *TarBubble_render_t)(DWORD this_);
static TarBubble_render_t pfn_TarBubble_render = (TarBubble_render_t)0x0044F910;
/* v55n: Bubble dtor (vtable[0] = SimpleList_dtor 0x44FD40): __thiscall
 * (this, flags). flags&1 == free. The native update does NOT free the
 * bubble — the list owner does. We must dtor(1) on expiry (pop), and
 * dtor(1) any live bubbles on level unload, or we leak 0x1C blocks. */
typedef void (__thiscall *TarBubble_dtor_t)(DWORD this_, BYTE flags);
static TarBubble_dtor_t pfn_TarBubble_dtor = (TarBubble_dtor_t)0x0044FD40;

/* Gfx_Scale — set base scale on all 3 axes. __thiscall(ECX=timer_buf, x, y, z) RET 0xC */
typedef void (__thiscall *Gfx_Scale_t)(void* timer_buf, float x, float y, float z);
static Gfx_Scale_t pfn_Gfx_Scale = (Gfx_Scale_t)0x00457B80;

/* v55m_24: Custom vtable[18] for Chomper — applies jaw rotation before rendering.
 * vtable[18] = D3DXSkinMesh_CopyStripData (0x45E0E0) — the ACTUAL render function.
 * Uses __thiscall (NOT __fastcall) to match the game's vtable calling convention.
 /* v55m_27: Custom vtable[18] for Chomper — applies jaw rotation via direct D3D SetTransform.
  * Previous approaches (Gfx_ScaleZ, EntityTransform) all crashed or didn't work.
  * This approach: save world matrix → set Z-rotation → render → restore.
  * D3DTS_WORLD = 256, device vtable[36]=GetTransform(0x90), [37]=SetTransform(0x94). */
 static void __thiscall cEnt_chomper_render(DWORD this_, char param_1, int param_2) {
     ChomperState* cs = NULL;
     int i;
     for (i = 0; i < g_chomper_count; i++) {
         if (g_chompers[i].obj == this_) { cs = &g_chompers[i]; break; }
     }
     if (!cs || !cs->orig_vtable2) {
         /* Not a tracked chomper — call original directly */
         typedef void (__thiscall *render_t)(DWORD, char, int);
         ((render_t)0x0045E0E0)(this_, param_1, param_2);
         return;
     }
     /* v55m_27: Direct D3D SetTransform approach.
      * Build a Z-rotation matrix from jaw_angle and set it as the world matrix.
      * Save/restore the original world matrix around the render call. */
     {
         DWORD app = *(DWORD*)0x005341E0;
         if (app) {
             DWORD gfx = *(DWORD*)((char*)app + 0x174);
             if (gfx) {
                 DWORD device = *(DWORD*)((char*)gfx + 0x154);
                 if (device && !IsBadReadPtr((void*)device, 4)) {
                     DWORD* dev_vtable = *(DWORD**)device;
                     if (dev_vtable && !IsBadReadPtr(dev_vtable, 0x98)) {
                         /* GetTransform = vtable[36] (offset 0x90) */
                         typedef void (__stdcall *GetTransform_t)(DWORD device, DWORD state, void* pMatrix);
                         /* SetTransform = vtable[37] (offset 0x94) */
                         typedef void (__stdcall *SetTransform_t)(DWORD device, DWORD state, void* pMatrix);
                         GetTransform_t pfn_GetTransform = (GetTransform_t)dev_vtable[36];
                         SetTransform_t pfn_SetTransform = (SetTransform_t)dev_vtable[37];

                         /* Save current world matrix */
                         float saveMatrix[16];
                         pfn_GetTransform(device, 256 /* D3DTS_WORLD */, saveMatrix);

                         /* Build Z-rotation matrix:
                          * cos(θ)  sin(θ)  0  0
                          * -sin(θ) cos(θ)  0  0
                          * 0       0       1  0
                          * 0       0       0  1
                          * (D3D uses row-major, transposed from math convention) */
                         float angle = -cs->jaw_angle * 3.14159265f / 180.0f; /* deg→rad */
                         float c = cosf(angle);
                         float s = sinf(angle);
                         float rotMatrix[16] = {
                             c,  s,  0, 0,
                             -s, c,  0, 0,
                             0,  0,  1, 0,
                             0,  0,  0, 1
                         };

                         /* Set rotation as world matrix */
                         pfn_SetTransform(device, 256, rotMatrix);

                         /* Call original render */
                         typedef void (__thiscall *render_t)(DWORD, char, int);
                         ((render_t)cs->orig_vtable2)(this_, param_1, param_2);

                         /* Restore original world matrix */
                         pfn_SetTransform(device, 256, saveMatrix);
                         return;
                     }
                 }
             }
         }
     }
     /* Fallback — no transform, just render */
     {
         typedef void (__thiscall *render_t)(DWORD, char, int);
         ((render_t)cs->orig_vtable2)(this_, param_1, param_2);
     }
     }

/* v55m_27: Waterwheel vtable[18] hook — stub that calls the real impl
 * (defined after WaterWheelState struct). */
static void cEnt_waterwheel_render_impl(DWORD this_, char param_1, int param_2);
static void __thiscall cEnt_waterwheel_render(DWORD this_, char param_1, int param_2) {
    cEnt_waterwheel_render_impl(this_, param_1, param_2);
}

/* v55m_43h: cEnt Catapult render — rotate the catapult object around Y.
 * v55m_42w: Added g_in_draw_phase guard — Stands vtable[18] is called during
 * Update AND Draw. D3D transforms only work during Draw.
 * v55m_43h: Uses the SAME pattern as the working Chomper/Waterwheel hooks:
 * 256-byte vtable copy (64 entries), IsBadReadPtr(orig, 256).
 * The previous 0x50-byte (20-entry) copy was too small — the game read
 * vtable slots beyond entry 20, hitting garbage and corrupting the stack.
 * v55m_43h: FIXED matrix write location. The catapult's world matrix is NOT
 * at gfx+0x74C! Both 0x45E0E0 (catapult vtable[18]) and 0x465650 (collision
 * Level vtable[18]) call Graphics_BeginFrame(0x453B50) with the render Level
 * as the matrix param: mov [this+0x434],eax; push eax; call 0x453B50.
 * BeginFrame does esi = param+4; SetTransform(D3DTS_WORLD, esi) — so the
 * ACTUAL world matrix is at renderLevel+0x4 (obj+0x434+4 = obj+0x438).
 * Stands_ctor (0x462850) initializes it via Timer_Init(0x457AD0) at
 * lea 0x438(%esi),%ecx. We must write the rotated matrix there. */
static void __thiscall cEnt_catapult_render(DWORD this_, char param_1, int param_2) {
    CatapultState* cs = NULL;
    int i;
    for (i = 0; i < g_catapult_count; i++) {
        if (g_catapults[i].obj == this_ || g_catapults[i].arm_obj == this_) {
            cs = &g_catapults[i]; break;
        }
    }
    if (!cs || !cs->orig_vtable18) {
        typedef void (__thiscall *render_t)(DWORD, char, int);
        ((render_t)0x0045E0E0)(this_, param_1, param_2);
        return;
    }

    /* v55m_43h: The render Level is at this_+0x434 (for BOTH the catapult
     * obj and the collision Level — Catapult_ctor copies obj+0x434 into
     * the collision Level's +0x434). The world matrix is at renderLevel+0x4. */
    DWORD renderLevel = 0;
    if (!IsBadReadPtr((void*)(this_ + 0x434), 4)) {
        renderLevel = *(DWORD*)(this_ + 0x434);
    }
    DWORD app = *(DWORD*)0x005341E0;
    if (renderLevel && app &&
        !IsBadReadPtr((void*)(renderLevel + 0x4), 64)) {
        /* v55m_43h: log render hook firing (once per ~60 frames) to prove
         * the hook runs during the catapult's render. */
        {
            static int rlog = 0;
            if ((++rlog & 0x3F) == 1) {
                FILE* df = fopen("mknp_custom_entities_catapult.log", "a");
                if (df) {
                    fprintf(df, "CATAPULT RENDER: this=0x%08X renderLevel=0x%08X arm_angle=%.2f draw=%d\n",
                        this_, renderLevel, cs->arm_angle, g_in_draw_phase);
                    fclose(df);
                }
            }
        }
    }
    /* v55m_43h: The matrix rotation (visual, user-confirmed in v55m_43g)
     * and the vertex-array rotation (collision) are INDEPENDENT:
     * - The catapult's vtable[18] hook applies the matrix → VISIBLE rotation.
     * - cEnt_catapult_rotate_collision_verts rotates the collision Level's
     *   OWN MeshWorld+0x448 vertex array in place → the collision tree
     *   (which references vertex INDICES) follows the rotated verts.
     * No double-rotation: the matrix drives the visual, the array drives
     * the collision. The matrix hook ALWAYS runs. */
    {
        /* Save the current world matrix (renderLevel+0x4). */
        float saveMatrix[16];
        memcpy(saveMatrix, (float*)(renderLevel + 0x4), sizeof(saveMatrix));

        /* Rotate AROUND THE OBJECT'S OWN CENTER (cs->x,y,z). */
        float angle = cs->arm_angle * 3.14159265f / 180.0f;
        float c = cosf(angle);
        float s = sinf(angle);
        float m1[16] = {
            1,0,0,0,  0,1,0,0,  0,0,1,0,
            -cs->x, -cs->y, -cs->z, 1
        };
        float m2[16] = {
            1,0,0,0,  0,c,-s,0,  0,s,c,0,  0,0,0,1
        };
        float m3[16] = {
            1,0,0,0,  0,1,0,0,  0,0,1,0,
            cs->x, cs->y, cs->z, 1
        };
        float finalMatrix[16];
        int row, col, k;
        float tmp1[16], tmp2[16];
        for (row = 0; row < 4; row++) {
            for (col = 0; col < 4; col++) {
                tmp1[row*4+col] = 0;
                for (k = 0; k < 4; k++) {
                    tmp1[row*4+col] += saveMatrix[row*4+k] * m1[k*4+col];
                }
            }
        }
        for (row = 0; row < 4; row++) {
            for (col = 0; col < 4; col++) {
                tmp2[row*4+col] = 0;
                for (k = 0; k < 4; k++) {
                    tmp2[row*4+col] += tmp1[row*4+k] * m2[k*4+col];
                }
            }
        }
        for (row = 0; row < 4; row++) {
            for (col = 0; col < 4; col++) {
                finalMatrix[row*4+col] = 0;
                for (k = 0; k < 4; k++) {
                    finalMatrix[row*4+col] += tmp2[row*4+k] * m3[k*4+col];
                }
            }
        }

        memcpy((float*)(renderLevel + 0x4), finalMatrix, sizeof(finalMatrix));
        typedef void (__thiscall *render_t)(DWORD, char, int);
        DWORD orig_fn = (this_ == cs->arm_obj && cs->arm_orig_vtable18)
                      ? cs->arm_orig_vtable18 : cs->orig_vtable18;
        ((render_t)orig_fn)(this_, param_1, param_2);
        memcpy((float*)(renderLevel + 0x4), saveMatrix, sizeof(saveMatrix));
        return;
    }
    /* Unreachable fallback (kept for structure): call original directly. */
    typedef void (__thiscall *render_t)(DWORD, char, int);
    DWORD orig_fn = (this_ == cs->arm_obj && cs->arm_orig_vtable18)
                  ? cs->arm_orig_vtable18 : cs->orig_vtable18;
    ((render_t)orig_fn)(this_, param_1, param_2);
    return;
}

/* Vec3_Copy — copy 3 floats */
typedef void (__thiscall *Vec3_Copy_t)(float* dst, float* src);
static Vec3_Copy_t pfn_Vec3_Copy_Bridge = (Vec3_Copy_t)0x00402BF0;

/* Scene_ForEachBall_SetVelocity — apply velocity to all balls near a point */
typedef void (__thiscall *Scene_ForEachBall_SetVel_t)(void* board, float x, float y, float z);
static Scene_ForEachBall_SetVel_t pfn_Scene_ForEachBall_SetVel = (Scene_ForEachBall_SetVel_t)0x00419B70;

/* App offsets for sound */
#define APP_SOUNDFX_BRIDGESLAM  0x47C   /* bridgeslam sound channel */

/* ═══════════════════════════════════════════════════════════════════════════
 * Bridgeslam — isolated Intermediate Race bridge behavior
 *
 * Replicates the 4-state machine from Intermediate Board_Update (FUN_0041CC90):
 *   State 0: Countdown (50 frames) → State 1
 *   State 1: Tilt down (45°→0° at 3°/frame) → slam! → State 2
 *   State 2: Wait (125 frames) → State 3
 *   State 3: Tilt back up (0°→45° at 0.5°/frame) → State 0
 *
 * State is stored per-object in a BridgeslamState struct, NOT on the board.
 * The per-frame update runs from the mod's background thread (~60fps).
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef struct {
    DWORD obj;               /* spawned object pointer (for board lists) */
    DWORD render_obj;        /* Level_RenderCtor result (visual) */
    DWORD mesh_world;        /* MeshWorld (Level_MeshWorldCtor result) */
    DWORD board;             /* board pointer (for ball list access) */
    float pivot_x, pivot_y, pivot_z;  /* bridge pivot position */
    float angle;             /* current tilt angle (starts 45.0) */
    int   state;             /* 0=count, 1=tilt down, 2=wait, 3=tilt back */
    int   counter;           /* frame counter for current state */
    int   active;            /* 1 = active (being updated) */
} BridgeslamState;

#define MAX_BRIDGESLAMS 16
static BridgeslamState g_bridgeslams[MAX_BRIDGESLAMS];
static int g_bridgeslam_count = 0;

/* v55c: Gluebie tracking for cross-level proximity behavior */
#define MAX_GLUEBIES 64
static DWORD g_gluebie_objs[MAX_GLUEBIES];
static int   g_gluebie_count = 0;
/* v55j_9: Track which ball is in a Gluebie zone for post-physics visual fix.
 * Set in App_ResetFrame hook, read in Ball_Render hook. */
static DWORD g_gluebie_ball_in_zone = 0;
/* v55j_12: Track whether we've already created tarsplotch particles for this ball.
 * ball+0x2BC is UNINITIALIZED (no memset after operator_new), so we can't rely on it. */
static DWORD g_gluebie_particles_created_ball = 0;  /* ball ptr that already has particles */

/* v55k_1: Tarpit tracking — spawned PopCylinder objects that need tar-sinking behavior.
 * Native N:TARPIT collision event only works on Dizzy/Master levels.
 * We replicate the behavior on ALL levels via proximity check in the Present hook. */
#define MAX_TARPITS 32
static DWORD g_tarpit_objs[MAX_TARPITS];
static int   g_tarpit_count = 0;

/* v55n: TarBubble tracking — DECORATIVE FLOATING BUBBLE (Ghidra-verified 2026-08-05).
 * Native TarBubble = S1 ref point, purely decorative. DizzyBoard_Update/Master
 * occasionally (RNG==10) spawns a 0x1C-byte bubble object (vtable 0x4D6E48):
 *   - ctor FUN_0044fb50 (__thiscall: this, app, x, y, z): scale +0x14=25.0,
 *     lifetime +0x18 = RNG(0x19)+0x19 (25..50 frames), pos at +0x08/+0x0C/+0x10
 *   - update FUN_0044fbe0 (vtable[1]): scale *= 0.95 (0x4D092C), lifetime -=1,
 *     pops (plays bubble1 sound via app+0x488) when lifetime <= 0
 *   - render FUN_0044f910 (vtable[2]): Timer_Init → Gfx_SetPosition(x, y-scale*60, z)
 *     → [app+0x5A4]+vtable[0x1C](scale,0) → Timer_Cleanup
 * The bubble list (board+0x3B00) is driven ONLY by Dizzy/Master board updates,
 * so the mod self-drives these bubbles from the Present hook for ALL boards.
 * Gluebie (43) keeps the slowdown, Tarpit (44) keeps the sinking. */
#define MAX_TARBUBBLES 64
#define TARBUBBLE_SIZE 0x1C
typedef struct {
    float x, y, z;             /* spawn position from S1 ref point */
    DWORD obj;                 /* live 0x1C bubble object (vtable 0x4D6E48) */
    int   countdown;           /* frames until next bubble spawn (decorative) */
} TarBubblePos;
static TarBubblePos g_tarbubble_pos[MAX_TARBUBBLES];
static int g_tarbubble_count = 0;

/* v55f: WaterWheel tracking — mesh object + position + rotation.
 * Native game stores waterwheel mesh at board+0x4BA8, position at board+0x4BB0,
 * angle at board+0x4BBC. DizzyBoard_Update rotates angle -= 0.5/frame and
 * applies transform via mesh vtable[22]+[21]. N:WHEELEMBED collision embeds ball.
 * We replicate by creating the mesh via MeshWorld_ctor and rotating each frame. */
#define MAX_WATERWHEELS 8
struct WaterWheelState {
    DWORD mesh_obj;   /* MeshWorld* (for collision data) */
    DWORD pc_obj;     /* PopCylinder* (the visible object we rotate) */
    float x, y, z;
    float angle;
    int  active;
    DWORD orig_vtable18; /* v55m_27: saved original vtable[18] for render hook */
    DWORD creak_channel; /* v55m_44c: WheelCreak sound channel (loaded at spawn) */
};
static struct WaterWheelState g_waterwheels[MAX_WATERWHEELS];
static int g_waterwheel_count = 0;
static int g_44l_present_logged = 0;  /* one-shot Present-hook proof log */
/* (g_log_path moved to top, v55n_22) */
static DWORD g_wheel_nodes[64];       /* v55m_44m: addresses of waterwheel-tree nodes */
static int g_wheel_node_count = 0;    /*   (recorded during wheel-tree walks) */

/* v55m_44m: probe whether a CollisionLevel node would FAULT in the render
 * walk (0x465650). The render walks [col+0x08]+0x2C (component meshbuffer
 * list): for each meshbuffer with strip count [mb+0x10] > 0 it derefs
 * [mb+0x418] (strip array). The Waterwheel's component meshbuffers are
 * 0x7C bytes with NO strip array → +0x418 is out of bounds → garbage/
 * unmapped → AV. Legit level-geometry meshbuffers have valid +0x418.
 * Returns 1 (broken/needs neutralize) or 0 (safe, leave alone). */
static int cEnt_collision_node_broken(DWORD node) {
    DWORD comp, count, items, i;
    if (!node || IsBadReadPtr((void*)node, 0x480)) return 0;
    if (*(DWORD*)node != 0x4D9068) return 0;
    comp = *(DWORD*)((char*)node + 0x08);
    if (!comp || IsBadReadPtr((void*)comp, 0x440)) return 0;
    count = *(DWORD*)((char*)comp + 0x30);      /* meshbuffer list count */
    if (count <= 0 || count >= 0x1000) return 0; /* no meshbuffers → no crash */
    items = *(DWORD*)((char*)comp + 0x438);     /* list items (+0x40C past +0x2C head) */
    if (!items || IsBadReadPtr((void*)items, count * 4)) return 0;
    for (i = 0; i < count; i++) {
        DWORD mb = *(DWORD*)((char*)items + i * 4);
        if (!mb || IsBadReadPtr((void*)mb, 0x20)) continue;
        if (*(DWORD*)((char*)mb + 0x10) > 0) {  /* strip count nonzero → render derefs */
            DWORD strip = *(DWORD*)((char*)mb + 0x418);
            if (IsBadReadPtr((void*)strip, 4)) return 1;  /* OOB → would fault */
        }
    }
    return 0;
}

/* v55m_44m: is this node one of the waterwheel's own tree nodes (address
 * match)? Used so the board walk still covers the wheel's CollisionLevel
 * even if the probe misses it, without touching unrelated level geometry. */
static int cEnt_is_wheel_node(DWORD node) {
    int i;
    for (i = 0; i < g_wheel_node_count; i++)
        if (g_wheel_nodes[i] == node) return 1;
    return 0;
}

/* v55m_44k: Recursively neutralize a CollisionLevel tree so its render
 * (vtable[18] = 0x465650) skips the broken component-meshbuffer walk.
 *
 * The crash: the Waterwheel mesh loads via the component SceneObject loader
 * (0x4706e0) producing 0x7C-byte meshbuffers with NO strip arrays. The
 * CollisionLevel render walks each meshbuffer:
 *   cmp [mb+0x10],0 ; mov 0x418(%esi),%edx ; mov (%edx),%ecx  (strip array)
 * → +0x418 is OUT OF BOUNDS on a 0x7C meshbuffer → garbage strip pointer
 * → AV at 0x465780/0x465789/0x46578C (SEH-resumed mid-instruction).
 *
 * The render guard at 0x46568f:
 *   cmp [this+0x430],0 ; jne 0x4657fc   (skip meshbuffer walk)
 * Setting +0x430 = 1 skips the walk. But post-init 0x465903 copies +0x430
 * from the PARENT at construction, and the render recurses through the
 * +0x18 sub-list (count +0x1C, items +0x424) calling vtable[18] on every
 * child — so EVERY node in the tree must have +0x430=1, or the walk runs
 * on that node's broken meshbuffers. This walks the whole tree (bounded,
 * cycle-safe) from a given root/list head and neutralizes every
 * CollisionLevel node (vtable 0x4D9068) found.
 *
 * record: if nonzero, store each neutralized node's address in g_wheel_nodes
 * (v55m_44m) so the board walk can recognize the wheel's own nodes even if
 * the meshbuffer probe is inconclusive.
 *
 * Returns number of nodes neutralized. */
static int cEnt_neutralize_collision_tree(DWORD root, FILE* logf, int record) {
    DWORD stack[512];
    DWORD sp = 0;
    DWORD visited[64];
    DWORD vc = 0;
    int count = 0;
    if (!root || IsBadReadPtr((void*)root, 0x480)) return 0;
    stack[sp++] = root;
    while (sp > 0) {
        DWORD node = stack[--sp];
        DWORD vi;
        int seen = 0;
        for (vi = 0; vi < vc; vi++) if (visited[vi] == node) { seen = 1; break; }
        if (seen) continue;
        if (vc < 64) visited[vc++] = node;
        if (!node || IsBadReadPtr((void*)node, 0x480)) continue;
        if (*(DWORD*)node != 0x4D9068) continue;  /* only CollisionLevel nodes */
        *(BYTE*)(node + 0x430) = 1;
        DWORD ncomp = *(DWORD*)((char*)node + 0x08);
        if (ncomp && !IsBadReadPtr((void*)(ncomp + 0x30), 4)) {
            *(DWORD*)((char*)ncomp + 0x30) = 0;  /* zero meshbuffer count */
        }
        if (record && g_wheel_node_count < 64) {
            g_wheel_nodes[g_wheel_node_count++] = node;
        }
        count++;
        if (logf) fprintf(logf, "  WATERWHEEL: collision node 0x%08X render-neutralized (total %d)\n", node, count);
        DWORD ncount = *(DWORD*)((char*)node + 0x1C);
        DWORD nitems = *(DWORD*)((char*)node + 0x424);
        if (ncount > 0 && ncount < 0x1000 && nitems &&
            !IsBadReadPtr((void*)nitems, ncount * 4)) {
            DWORD ci;
            for (ci = ncount; ci > 0; ci--) {
                DWORD gchild = *(DWORD*)((char*)nitems + (ci - 1) * 4);
                if (gchild && sp < 512) stack[sp++] = gchild;
            }
        }
    }
    return count;
}

/* v55m_44k/44m: Neutralize the CollisionLevel tree of a spawned waterwheel
 * (pc_obj). Walks BOTH the pc_obj's +0x18 render sub-list and the
 * CollisionLevel's own sub-list, then the whole tree from each.
 * Wheel-tree node addresses are recorded (v55m_44m) for the board walk. */
static int cEnt_neutralize_waterwheel_collision(DWORD pc_obj, FILE* logf) {
    int total = 0;
    DWORD col = pc_obj ? *(DWORD*)((char*)pc_obj + 0x10E0) : 0;
    if (col && !IsBadReadPtr((void*)col, 0x480) && *(DWORD*)col == 0x4D9068) {
        if (logf) fprintf(logf, "  WATERWHEEL: collision level 0x%08X render-neutralized (root)\n", col);
        total += cEnt_neutralize_collision_tree(col, logf, 1);
    }
    /* Also walk the pc_obj's own +0x18 render sub-list in case the
     * CollisionLevel was cloned there by Stands_ctor 0x462980-0x462989. */
    if (pc_obj && !IsBadReadPtr((void*)(pc_obj + 0x1C), 4)) {
        DWORD rcount = *(DWORD*)((char*)pc_obj + 0x1C);
        DWORD ritems = *(DWORD*)((char*)pc_obj + 0x424);
        if (rcount > 0 && rcount < 0x1000 && ritems &&
            !IsBadReadPtr((void*)ritems, rcount * 4)) {
            DWORD ci;
            for (ci = 0; ci < rcount; ci++) {
                DWORD child = *(DWORD*)((char*)ritems + ci * 4);
                if (child && !IsBadReadPtr((void*)child, 0x480) && *(DWORD*)child == 0x4D9068) {
                    total += cEnt_neutralize_collision_tree(child, logf, 1);
                }
            }
        }
    }
    /* v55m_44m: SELECTIVE board-walk. The game's registration path
     * (0x436FC0) appends spawned CollisionLevels to board+0x8B0+0x18, and
     * during FinishLoad the board's collision render recurses into them.
     * BUT the board's own tree also contains the LEVEL GEOMETRY nodes
     * (ground/walls) — those have VALID meshbuffers and MUST NOT be
     * neutralized (44l neutralized all 11 of them → infinite fall + pale
     * wheel). Only neutralize board nodes that are (a) the wheel's own
     * recorded nodes, or (b) probe-broken (component meshbuffer strip
     * pointer OOB → would fault in the render walk). */
    {
        DWORD board = get_board();
        if (board && !IsBadReadPtr((void*)(board + 0x8B0), 4)) {
            DWORD bcol = *(DWORD*)((char*)board + 0x8B0);
            if (bcol && !IsBadReadPtr((void*)bcol, 0x480) &&
                *(DWORD*)bcol == 0x4D9068) {
                DWORD stack[512], sp = 0, visited[64], vc = 0;
                stack[sp++] = bcol;
                while (sp > 0) {
                    DWORD node = stack[--sp];
                    DWORD vi, ncount, nitems, ci;
                    int seen = 0, broken;
                    for (vi = 0; vi < vc; vi++) if (visited[vi] == node) { seen = 1; break; }
                    if (seen) continue;
                    if (vc < 64) visited[vc++] = node;
                    if (!node || IsBadReadPtr((void*)node, 0x480)) continue;
                    if (*(DWORD*)node != 0x4D9068) continue;
                    broken = cEnt_is_wheel_node(node) || cEnt_collision_node_broken(node);
                    if (broken) {
                        *(BYTE*)(node + 0x430) = 1;
                        DWORD ncomp = *(DWORD*)((char*)node + 0x08);
                        if (ncomp && !IsBadReadPtr((void*)(ncomp + 0x30), 4)) {
                            *(DWORD*)((char*)ncomp + 0x30) = 0;
                        }
                        total++;
                        if (logf) fprintf(logf, "  WATERWHEEL: board collision node 0x%08X render-neutralized (total %d)\n", node, total);
                    }
                    ncount = *(DWORD*)((char*)node + 0x1C);
                    nitems = *(DWORD*)((char*)node + 0x424);
                    if (ncount > 0 && ncount < 0x1000 && nitems &&
                        !IsBadReadPtr((void*)nitems, ncount * 4)) {
                        for (ci = ncount; ci > 0; ci--) {
                            DWORD gchild = *(DWORD*)((char*)nitems + (ci - 1) * 4);
                            if (gchild && sp < 512) stack[sp++] = gchild;
                        }
                    }
                }
            }
        }
    }
    return total;
}

/* v55m_44o: Waterwheel render hook — MESH-based (no PopCylinder).
 * The hook is installed on the MeshWorld's own vtable[18] (0x4D8FB0[18]
 * = 0x470150, SceneObject render). 0x470150 renders the root's own
 * meshbuffer list (+0x2C; empty for a BRANCH octree) AND recurses into
 * child Levels via +0x424 — so the full wheel geometry renders from the
 * mesh's own vertex data, with NO CollisionLevel anywhere (the crash
 * path is structurally gone).
 *
 * Rotation: same X-axis matrix compose as 44 (native Dizzy rotates the
 * wheel about X at 0.5 deg/frame), applied to the MeshWorld's own world
 * matrix at mesh+0x4, translated around the wheel center (x,y,z). */
static void cEnt_waterwheel_render_impl(DWORD this_, char param_1, int param_2) {
    struct WaterWheelState* ww = NULL;
    int i;
    for (i = 0; i < g_waterwheel_count; i++) {
        if (g_waterwheels[i].pc_obj == this_) { ww = &g_waterwheels[i]; break; }
    }
    if (!ww || !ww->orig_vtable18) {
        typedef void (__thiscall *render_t)(DWORD, char, int);
        ((render_t)0x00470150)(this_, param_1, param_2);
        return;
    }
    /* MeshWorld's own world matrix lives at mesh+0x4 (the SceneObject
     * transform block — same offset Graphics_BeginFrame uses for the
     * D3DTS_WORLD transform). */
    if (IsBadReadPtr((void*)(this_ + 0x4), 64)) {
        typedef void (__thiscall *render_t)(DWORD, char, int);
        ((render_t)ww->orig_vtable18)(this_, param_1, param_2);
        return;
    }
    {
        /* Save the current world matrix (mesh+0x4). */
        float saveMatrix[16];
        memcpy(saveMatrix, (float*)(this_ + 0x4), sizeof(saveMatrix));

        /* Native waterwheel: rotate around X by angle (degrees). */
        float angle = ww->angle * 3.14159265f / 180.0f;
        float c = cosf(angle);
        float s = sinf(angle);
        float m1[16] = {
            1,0,0,0,  0,1,0,0,  0,0,1,0,
            -ww->x, -ww->y, -ww->z, 1
        };
        float m2[16] = {
            1,0,0,0,  0,c,-s,0,  0,s,c,0,  0,0,0,1
        };
        float m3[16] = {
            1,0,0,0,  0,1,0,0,  0,0,1,0,
            ww->x, ww->y, ww->z, 1
        };
        float finalMatrix[16];
        int row, col, k;
        float tmp1[16], tmp2[16];
        for (row = 0; row < 4; row++) {
            for (col = 0; col < 4; col++) {
                tmp1[row*4+col] = 0;
                for (k = 0; k < 4; k++) {
                    tmp1[row*4+col] += saveMatrix[row*4+k] * m1[k*4+col];
                }
            }
        }
        for (row = 0; row < 4; row++) {
            for (col = 0; col < 4; col++) {
                tmp2[row*4+col] = 0;
                for (k = 0; k < 4; k++) {
                    tmp2[row*4+col] += tmp1[row*4+k] * m2[k*4+col];
                }
            }
        }
        for (row = 0; row < 4; row++) {
            for (col = 0; col < 4; col++) {
                finalMatrix[row*4+col] = 0;
                for (k = 0; k < 4; k++) {
                    finalMatrix[row*4+col] += tmp2[row*4+k] * m3[k*4+col];
                }
            }
        }

        memcpy((float*)(this_ + 0x4), finalMatrix, sizeof(finalMatrix));
        /* v55m_44o: dead path (mesh not in any list) — but if the game ever
         * calls the hooked vtable[18], forward the ORIGINAL (1,1) args: the
         * game's render-list walk pushes 1,1 before call *0x48. 0x470150
         * reads arg1 as a pointer (mov [ebp+0x41c],esi) — arg1=1 would AV,
         * so never forward 1 here. */
        if (param_1 == 1) param_1 = 0;
        {
            typedef void (__thiscall *render_t)(DWORD, int);
            ((render_t)ww->orig_vtable18)(this_, param_1);
        }
        memcpy((float*)(this_ + 0x4), saveMatrix, sizeof(saveMatrix));
        return;
    }
}

/* v55m_44o: Manual per-frame waterwheel render — called from the Present
 * hook. The mesh is NOT in any render/scene list (registering it made the
 * game create a crashing CollisionLevel), so WE draw it every frame: apply
 * the X-axis rotation to the mesh's own world matrix (mesh+0x4), call the
 * original vtable[18] (0x470150 SceneObject render — draws the octree),
 * restore. Mirrors the native design: the mesh lives in a bare slot and is
 * rendered manually. */
static void cEnt_waterwheel_present_render(struct WaterWheelState* ww) {
    if (!ww || !ww->active || !ww->pc_obj) return;
    DWORD mesh = ww->pc_obj;
    if (!ww->orig_vtable18 || IsBadReadPtr((void*)(mesh + 0x4), 64)) return;
    if (game_is_quitting()) return;

    /* Save the mesh's world matrix, apply rotation around wheel center. */
    float saveMatrix[16];
    memcpy(saveMatrix, (float*)(mesh + 0x4), sizeof(saveMatrix));

    float angle = ww->angle * 3.14159265f / 180.0f;
    float c = cosf(angle);
    float s = sinf(angle);
    float m1[16] = { 1,0,0,0,  0,1,0,0,  0,0,1,0,  -ww->x, -ww->y, -ww->z, 1 };
    float m2[16] = { 1,0,0,0,  0,c,-s,0,  0,s,c,0,  0,0,0,1 };
    float m3[16] = { 1,0,0,0,  0,1,0,0,  0,0,1,0,  ww->x, ww->y, ww->z, 1 };
    float finalMatrix[16], tmp1[16], tmp2[16];
    int row, col, k;
    for (row = 0; row < 4; row++)
        for (col = 0; col < 4; col++) {
            tmp1[row*4+col] = 0;
            for (k = 0; k < 4; k++) tmp1[row*4+col] += saveMatrix[row*4+k] * m1[k*4+col];
        }
    for (row = 0; row < 4; row++)
        for (col = 0; col < 4; col++) {
            tmp2[row*4+col] = 0;
            for (k = 0; k < 4; k++) tmp2[row*4+col] += tmp1[row*4+k] * m2[k*4+col];
        }
    for (row = 0; row < 4; row++)
        for (col = 0; col < 4; col++) {
            finalMatrix[row*4+col] = 0;
            for (k = 0; k < 4; k++) finalMatrix[row*4+col] += tmp2[row*4+k] * m3[k*4+col];
        }

    memcpy((float*)(mesh + 0x4), finalMatrix, sizeof(finalMatrix));
    {
        /* 0x470150 is __thiscall(ECX=this, arg1) — RET $0x4 (cleans 1 arg).
         * arg1 is used as a pointer ONLY when non-zero (cmp esi,ebp; je skips
         * the [ebp+0x41c] write), so passing 0 is safe and standard. */
        typedef void (__thiscall *render_t)(DWORD, int);
        ((render_t)ww->orig_vtable18)(mesh, 0);
    }
    memcpy((float*)(mesh + 0x4), saveMatrix, sizeof(saveMatrix));
}

/* Per-frame update for a single bridgeslam object */
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
    case 0: /* Countdown */
        bs->counter--;
        if (bs->counter < 1) bs->state = 1;
        break;

    case 1: /* Tilt down */
        bs->angle -= 3.0f;
        if (bs->angle < 0.0f) {
            bs->angle = 0.0f;
            bs->counter = 125; /* 0x7D */
            bs->state = 2;
            /* Play bridgeslam sound (4th param = 1.0 scale, required by RET 0x10) */
            if (app && pfn_Sound_Play3D) {
                DWORD snd = *(DWORD*)(app + APP_SOUNDFX_BRIDGESLAM);
                if (snd && snd > 0x10000 && !IsBadReadPtr((void*)snd, 0x20)) {
                    pfn_Sound_Play3D((void*)snd, bs->pivot_x, bs->pivot_y, bs->pivot_z, 1.0f);
                }
            }
            /* Apply velocity to balls near pivot */
            if (board && pfn_Scene_ForEachBall_SetVel) {
                pfn_Scene_ForEachBall_SetVel((void*)board,
                    bs->pivot_y, bs->pivot_z, 0.5f);
            }
        }
        break;

    case 2: /* Wait */
        bs->counter--;
        if (bs->counter < 1) bs->state = 3;
        break;

    case 3: /* Tilt back up */
        bs->angle += 0.5f;
        if (bs->angle >= 45.0f) {
            bs->angle = 45.0f;
            bs->counter = 75; /* 0x4B */
            bs->state = 0;
        }
        /* Apply render transform: Gfx_ScaleZ + Gfx_SetPosition + vtable calls */
        if (app) {
            DWORD gfx = *(DWORD*)(app + APP_GFX_DEVICE);
            if (gfx && pfn_Gfx_ScaleZ_Bridge && pfn_Gfx_SetPosition_Bridge &&
                pfn_Timer_Init && pfn_Timer_Cleanup) {
                char timerBuf[68];
                pfn_Timer_Init(timerBuf);
                pfn_Gfx_ScaleZ_Bridge((void*)gfx, -bs->angle);
                pfn_Gfx_SetPosition_Bridge((void*)gfx,
                    bs->pivot_x, bs->pivot_y, bs->pivot_z);
                /* Call render object vtable[0x16] and [0x15] */
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

/* Object sizes for new ctor types */
#define ARENASTANDS_SIZE      0x1104
#define GAMELEVEL_SIZE        0x1524
#define GLASS_LEVEL_SIZE      0x113C
#define GEAR_LEVEL_SIZE       0x1100
#define SECRET_SIZE           0x10EC
#define FLAGWAVER_SIZE        0x8C
#define SIGN_SIZE             0x10FC
#define WAVY_SIZE             0x1AE7C
#define BADBALL_SIZE          0xC98  /* CreateBadBalls allocates 0xC98, not 0xC70 */
#define BRIDGESLAM_SIZE       0x10D0  /* same as Level/MeshWorld size */
#define SPINNER_LEVEL_SIZE    0x10FC
#define SPRITE_SIZE           0xD4
#define MESHNODE_SIZE         0x18
#define GEAR_REAL_SIZE        0x1514  /* Gear_ctor 9-param variant */
#define BELL_SIZE             0x10E8
#define FAN_SIZE              0x1188
#define SAWBLADE_SIZE         0x111C
#define BONK_SIZE             0x1200
#define CATAPULT_SIZE         0x1108
#define MACE_SIZE             0x110C
#define TIPPER_SIZE           0x1104
#define GLUEBIE_SIZE          0x110C
#define LIFTER_SIZE           0x10F4
#define SPEEDCYLINDER_SIZE    0x150C
#define TIMEBUTTON_SIZE       0x10E8
#define NEONPLATFORM_SIZE     0x1104
#define TRAPDOOR_SIZE         0x10F8
#define ODD_LIFTER_SIZE       0x10F4

/* Level3-Swirl mesh path (game .data at 0x004CFFE0) */
static const char* g_swirl_mesh_path = (const char*)0x004CFFE0;

/* Gfx_Scale function pointers — used by ROT_M axis selection.
 * Native render (0x0043B330) calls Gfx_ScaleX(angle) to build the
 * rotation-to-render matrix. ROT_M lets the .txt config choose which
 * axis function to call instead. */
typedef void (__cdecl *Gfx_ScaleAxis_t)(float);
static Gfx_ScaleAxis_t pfn_Gfx_ScaleX = (Gfx_ScaleAxis_t)0x00457C60;
static Gfx_ScaleAxis_t pfn_Gfx_ScaleY = (Gfx_ScaleAxis_t)0x00457C90;
static Gfx_ScaleAxis_t pfn_Gfx_ScaleZ = (Gfx_ScaleAxis_t)0x00457CC0;

/* AI mesh path table — game .data string addresses for AI 1-5 */
static const char* g_ai_mesh_paths[] = {
    NULL,                              /* AI 0: static (use MESH property) */
    (const char*)0x004D20FC,           /* AI 1: Levels\LevelImpossible-Rotator */
    (const char*)0x004D20DC,           /* AI 2: Levels\LevelImpossible-Pendulum */
    (const char*)0x004D2158,           /* AI 3: Levels\LevelImpossible-Looper */
    (const char*)0x004D213C,           /* AI 4: Levels\LevelImpossible-Gear */
    (const char*)0x004D211C,           /* AI 5: Levels\LevelImpossible-BigGear */
};

/* Rotater spawned objects tracking */
#define MAX_ROTATERS 999

/* Per-rotater config: mesh path and rotation speeds */
typedef struct {
    DWORD obj;              /* spawned object pointer */
    char  mesh_path[128];   /* custom mesh path (empty = default Level3-Swirl) */
    float rot_x;            /* X-axis rotation speed (radians/frame) */
    float rot_y;            /* Y-axis rotation speed (radians/frame) */
    float rot_z;            /* Z-axis rotation speed (radians/frame) */
    float ros_x;         /* X-axis oscillation range (radians, default 2.0) */
    float ros_y;         /* Y-axis oscillation range (radians, default 2.0) */
    float ros_z;         /* Z-axis oscillation range (radians, default 2.0) */
    float angle_x;          /* accumulated X angle */
    float angle_y;          /* accumulated Y angle */
    float angle_z;          /* accumulated Z angle */
    /* v46 config fields — per-entity customization */
    int   rng_seed;         /* RNG seed (unused for now, reserved for future) */
    float rot_a;            /* initial angle (obj+0x10E8, default 0.0) */
    float rot_d;            /* initial direction (obj+0x10EC, default 0.0) */
    float rot_max;          /* max oscillation angle (native 2.0, default 0.0) */
    float rot_min;          /* min oscillation angle (native -2.0, default 0.0) */
    int   rot_m;             /* axis for Gfx_Scale: 0=X, 1=Y, 2=Z (default 0=X) */
    int   has_custom_limits; /* 1 if rot_max/rot_min are non-zero (need per-frame override) */
} RotaterConfig;

static RotaterConfig g_rotater_cfg[MAX_ROTATERS];
static int   g_rotater_count = 0;
/* v55m_48d: CONSTRUCTOR-time snapshot of the tracked entity's initial
 * angle (obj+0x10E8) and direction (obj+0x10EC), captured once at spawn
 * in cEnt_spawn_rotater_at. The per-frame native render mutates these
 * fields, but the "Constructors" debug section must show the static
 * initial values — so we freeze them here. */
static float g_dbg_ctor_angle = 0.0f;
static float g_dbg_ctor_direction = 0.0f;
static DWORD g_rotater_board = 0;

/* ═══════════════════════════════════════════════════════════════════════════
 * Bonk collision event hook
 *
 * E:CALLHAMMER and E:HAMMERCHASE are only natively handled by
 * ExpertCollisionEvents and HandleArenaCollisionEvents (Master).
 * On other levels, these events fall through to DispatchCollisionEvents
 * which doesn't check for them. We detour DispatchCollisionEvents,
 * let the original run, then check for hammer events ourselves.
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Tracked Bonk objects */
#define MAX_BONKS 8
static DWORD g_bonk_objs[MAX_BONKS];
static int g_bonk_count = 0;

/* Game function pointers for Bonk events */
typedef void (__fastcall *CreateBonkPopup_t)(int bonk_obj);
static CreateBonkPopup_t pfn_CreateBonkPopup = (CreateBonkPopup_t)0x00438B30;

typedef void (__fastcall *Hammer_ChaseStart_t)(int bonk_obj);
static Hammer_ChaseStart_t pfn_Hammer_ChaseStart = (Hammer_ChaseStart_t)0x00438BB0;

/* Original DispatchCollisionEvents function — we build a trampoline */
typedef void (__thiscall *DispatchCollisionEvents_t)(void* this_, int* ball, int* collision_data);
static DWORD g_dispatch_collision_addr = 0x0040C5D0;
static unsigned char g_dispatch_orig_bytes[8];  /* first 8 bytes = PUSH 0xFF + MOV EAX,FS:[0] */
static int g_dispatch_hooked = 0;

/* Trampoline buffer: 8 bytes original code + 5 byte JMP back = 13 bytes */
static unsigned char g_trampoline[13];
static DispatchCollisionEvents_t pfn_DispatchCollisionEvents_trampoline = NULL;

/* Check if current level is Expert (7) or Master (13) — they have native handlers */
static int is_expert_or_master_level(DWORD board) {
    if (!board || IsBadReadPtr((void*)board, 0x900)) return 0;
    DWORD app = *(DWORD*)(board + BOARD_APP);
    if (!app || IsBadReadPtr((void*)app, 0x600)) return 0;
    /* App+0x5FC = current level/race index (0-14) */
    int race_idx = *(int*)(app + 0x5FC);
    return (race_idx == 7 || race_idx == 13);
}

/* Our detour: calls original via trampoline, then checks for hammer events */
static void __thiscall hook_DispatchCollisionEvents(void* this_, int* ball, int* collision_data) {
    /* Call original first via trampoline */
    pfn_DispatchCollisionEvents_trampoline(this_, ball, collision_data);

    /* Skip on Expert/Master — they have native handlers */
    if (is_expert_or_master_level((DWORD)this_)) return;

    /* Get event name from collision_data[1]+0x864 */
    if (!collision_data || IsBadReadPtr((void*)collision_data, 8)) return;
    int meshbuf = collision_data[1];
    if (!meshbuf || IsBadReadPtr((void*)meshbuf, 0x868)) return;
    char* event_name = *(char**)(meshbuf + 0x864);
    if (!event_name || IsBadReadPtr(event_name, 1)) return;

    /* Check if we have any tracked Bonk objects */
    if (g_bonk_count > 0) {
        /* Check for E:CALLHAMMER */
    if (_stricmp(event_name, "E:CALLHAMMER") == 0) {
        /* Check difficulty flag: App+0x23C != 0 (same check native code uses) */
        DWORD app = *(DWORD*)((DWORD)this_ + BOARD_APP);
        if (app && !IsBadReadPtr((void*)(app + 0x23C), 4)) {
            if (*(int*)(app + 0x23C) != 0) {
                int i;
                for (i = 0; i < g_bonk_count; i++) {
                    if (g_bonk_objs[i] && !IsBadReadPtr((void*)g_bonk_objs[i], 0x1100)) {
                        pfn_CreateBonkPopup((int)g_bonk_objs[i]);
                    }
                }
            }
        }
    }
    /* Check for E:HAMMERCHASE */
    else if (_stricmp(event_name, "E:HAMMERCHASE") == 0) {
        DWORD app = *(DWORD*)((DWORD)this_ + BOARD_APP);
        if (app && !IsBadReadPtr((void*)(app + 0x23C), 4)) {
            if (*(int*)(app + 0x23C) != 0) {
                int i;
                for (i = 0; i < g_bonk_count; i++) {
                    if (g_bonk_objs[i] && !IsBadReadPtr((void*)g_bonk_objs[i], 0x1100)) {
                        pfn_Hammer_ChaseStart((int)g_bonk_objs[i]);
                    }
                }
            }
        }
    }
    }

    /* Diagnostic: log any collision event name that looks like a catapult event */
    if (event_name && (_strnicmp(event_name, "E:CAT", 5) == 0 || _strnicmp(event_name, "CATAPULT", 8) == 0)) {
        FILE* cdf = fopen("mknp_custom_entities_catapult.log", "a");
        if (cdf) {
            fprintf(cdf, "COLLISION_EVENT: name='%s' data0=0x%08X data1=0x%08X\n",
                event_name, collision_data ? collision_data[0] : 0, collision_data ? collision_data[1] : 0);
            fclose(cdf);
        }
    }

    /* Check for E:CATAPULTBOTTOM — call Catapult_Launch on the matching tracked catapult.
     * v55m_42f: collision_data[0] is the entity/collision object pointer (matches catapult+0x10D4).
     * collision_data[1] is the MeshBuffer (used only for the event name). */
    if (_stricmp(event_name, "E:CATAPULTBOTTOM") == 0) {
        int i;
        DWORD hit_obj = (DWORD)collision_data[0];
        int matched = 0;
        FILE* cdf = fopen("mknp_custom_entities_catapult.log", "a");
        if (cdf) {
            fprintf(cdf, "E:CATAPULT collides with player ball (ball=0x%08X, hit_obj=0x%08X, event='%s')\n",
                (DWORD)ball, hit_obj, event_name);
            fclose(cdf);
        }
        for (i = 0; i < g_catapult_count; i++) {
            if (g_catapults[i].obj && !IsBadReadPtr((void*)g_catapults[i].obj, 4)) {
                /* Native match: catapult+0x10D4 == collision mesh ptr */
                DWORD cat_col_obj = *(DWORD*)((char*)g_catapults[i].obj + 0x10D4);
                if (cat_col_obj && cat_col_obj == hit_obj) {
                    if (pfn_Catapult_Launch) {
                        pfn_Catapult_Launch((void*)g_catapults[i].obj);
                        matched = 1;
                        cdf = fopen("mknp_custom_entities_catapult.log", "a");
                        if (cdf) {
                            fprintf(cdf, "E:CATAPULT is triggering its event (catapult=0x%08X) -> Catapult_Launch\n",
                                g_catapults[i].obj);
                            fclose(cdf);
                        }
                    }
                }
            }
        }
        /* Fallback: if pointer didn't match, launch the closest catapult */
        if (!matched && g_catapult_count > 0 && ball && !IsBadReadPtr((void*)ball, 0x200) && pfn_Catapult_Launch) {
            float bx = *(float*)((DWORD)ball + 0x164);
            float by = *(float*)((DWORD)ball + 0x168);
            float bz = *(float*)((DWORD)ball + 0x16C);
            int best = -1;
            float best_dist = 999999.0f;
            for (i = 0; i < g_catapult_count; i++) {
                float dx = bx - g_catapults[i].x;
                float dy = by - g_catapults[i].y;
                float dz = bz - g_catapults[i].z;
                float d = dx*dx + dy*dy + dz*dz;
                if (d < best_dist) { best_dist = d; best = i; }
            }
            if (best >= 0 && best_dist < 250000.0f) {
                pfn_Catapult_Launch((void*)g_catapults[best].obj);
                FILE* cdf = fopen("mknp_custom_entities_catapult.log", "a");
                if (cdf) {
                    fprintf(cdf, "COLLISION_EVENT: E:CATAPULTBOTTOM fallback closest catapult obj=0x%08X -> Catapult_Launch\n",
                        g_catapults[best].obj);
                    fclose(cdf);
                }
            }
        }
    }

    /* v55n_3: N:EXTRATIME — TimeButton press (replicated from UpRaceCollisionEvents 0x4119B0
     * + Rotator_TriggerSound 0x436CF0). Native: on N:EXTRATIME collision, if button+0x10E4==0:
     *   press latch 0x10E4=1, render flag 0x10E5=1, sink +0x10D8 -= 20.0f, play press sound
     *   (sound list +0x510). Then in single-player (profile+0x10==0 && profile+0x11==0):
     *   set timer slot = 500 + spawn "EXTRA TIME:" ScoreObject.
     * We replicate it for cEnt TimeButtons on ANY level. */
    if (g_timebutton_count > 0 && event_name && _stricmp(event_name, "N:EXTRATIME") == 0) {
        /* Native reads entity via [[MeshBuffer]+0x47C]. Try that first, else nearest tracked. */
        DWORD tb_entity = 0;
        DWORD meshbuf0 = collision_data[0];
        if (meshbuf0 && !IsBadReadPtr((void*)meshbuf0, 0x4C0)) {
            DWORD ent_link = *(DWORD*)((char*)meshbuf0 + 0x47C);
            /* ent_link is the entity IF it looks like our TimeButton:
             * vtable == 0x4D5830 (native) OR the private copy pointing to it
             * (v55n_3+ swaps vtable to a private 0x400B copy). Both have slot
             * 0 == 0x43DC20 (dtor) so check that instead of the vtable base. */
            if (ent_link && !IsBadReadPtr((void*)ent_link, 0x10E8)) {
                DWORD ent_vt = *(DWORD*)ent_link;
                int is_tb = 0;
                if (ent_vt == 0x4D5830) is_tb = 1;
                else if (ent_vt && ent_vt > 0x400000 &&
                         !IsBadReadPtr((void*)ent_vt, 0x100) &&
                         *(DWORD*)ent_vt == 0x43DC20) is_tb = 1; /* private copy of 0x4D5830 */
                if (is_tb) tb_entity = ent_link;
            }
        }
        if (!tb_entity) {
            /* Fallback: nearest tracked TimeButton to ball */
            if (ball && !IsBadReadPtr((void*)ball, 0x200)) {
                float bx = *(float*)((DWORD)ball + 0x164);
                float by = *(float*)((DWORD)ball + 0x168);
                float bz = *(float*)((DWORD)ball + 0x16C);
                int best = -1;
                float best_dist = 999999.0f;
                int i;
                for (i = 0; i < g_timebutton_count; i++) {
                    float dx = bx - g_timebuttons[i].x;
                    float dy = by - g_timebuttons[i].y;
                    float dz = bz - g_timebuttons[i].z;
                    float d = dx*dx + dy*dy + dz*dz;
                    if (d < best_dist) { best_dist = d; best = i; }
                }
                if (best >= 0 && best_dist < 250000.0f) tb_entity = g_timebuttons[best].obj;
            }
        }
        if (tb_entity && !IsBadReadPtr((void*)tb_entity, 0x10E8)) {
            if (*(char*)((char*)tb_entity + 0x10E4) == 0) {   /* not yet pressed */
                /* Replicate Rotator_TriggerSound (0x436CF0) */
                *(char*)((char*)tb_entity + 0x10E4) = 1;      /* latch pressed */
                *(char*)((char*)tb_entity + 0x10E5) = 1;      /* render pressed pose */
                *(float*)((char*)tb_entity + 0x10D8) -= 20.0f;/* sink 20 units */
                /* Press sound: sound list [board+0x878] channel +0x510 */
                DWORD board_tb = *(DWORD*)((char*)tb_entity + 0x10D0);
                if (board_tb && !IsBadReadPtr((void*)(board_tb + 0x880), 0x20)) {
                    DWORD snd_list = *(DWORD*)((char*)board_tb + 0x878);
                    if (snd_list && !IsBadReadPtr((void*)(snd_list + 0x520), 0x10)) {
                        DWORD ch = *(DWORD*)((char*)snd_list + 0x510);
                        if (ch && pfn_Sound_Play3D) {
                            pfn_Sound_Play3D((void*)ch,
                                *(float*)((char*)tb_entity + 0x10D4),
                                *(float*)((char*)tb_entity + 0x10D8),
                                *(float*)((char*)tb_entity + 0x10DC), 1.0f);
                        }
                    }
                }
                /* Mark tracked state pressed */
                {
                    int i;
                    for (i = 0; i < g_timebutton_count; i++) {
                        if (g_timebuttons[i].obj == tb_entity) {
                            g_timebuttons[i].pressed = 1;
                            break;
                        }
                    }
                }
                /* Reward (single-player only): timer slot = 500 + "EXTRA TIME:" popup.
                 * Native: checks profile+0x10==0 && profile+0x11==0, writes 500 to
                 * ball->player_index * 0xA0 + 0x5EC + App offset... In the handler the
                 * App ptr comes from board+0x878. We replicate:
                 *   App = [board+0x878]; profile = [App+0x220];
                 *   if (profile && profile+0x10==0 && profile+0x11==0) {
                 *       DWORD timer_slot = ball+6 (player idx) * 0xA0 + 0x5EC + App;
                 *       *(DWORD*)timer_slot = 500;
                 *   } */
                if (ball && !IsBadReadPtr((void*)ball, 0x200)) {
                    DWORD board_tb2 = *(DWORD*)((char*)tb_entity + 0x10D0);
                    DWORD snd_list2 = board_tb2 ? *(DWORD*)((char*)board_tb2 + 0x878) : 0;
                    if (board_tb2 && snd_list2 && !IsBadReadPtr((void*)snd_list2, 0x240)) {
                        DWORD app_tb = *(DWORD*)((char*)board_tb2 + 0x878);
                        DWORD profile = app_tb ? *(DWORD*)((char*)app_tb + 0x220) : 0;
                        if (app_tb && profile && !IsBadReadPtr((void*)profile, 0x20) &&
                            *(char*)((char*)profile + 0x10) == 0 &&
                            *(char*)((char*)profile + 0x11) == 0) {
                            int player_idx = *(int*)((DWORD)ball + 0x18); /* ball+6 = player index */
                            DWORD timer_slot = player_idx * 0xA0 + 0x5EC + app_tb;
                            if (!IsBadReadPtr((void*)timer_slot, 4)) {
                                *(DWORD*)timer_slot = 500;
                            }
                        }
                    }
                }
                if (logf) fprintf(logf, "  ROTATER: TimeButton pressed (entity=0x%08X)\n", tb_entity);
            }
        }
    }
}

/* Install the DispatchCollisionEvents detour with proper trampoline */
static void install_bonk_collision_hook(void) {
    if (g_dispatch_hooked) return;

    /* Save original 8 bytes (PUSH 0xFF + MOV EAX,FS:[0]) */
    memcpy(g_dispatch_orig_bytes, (void*)g_dispatch_collision_addr, 8);

    /* Build trampoline: 8 bytes original code + JMP back to original+8 */
    memcpy(g_trampoline, g_dispatch_orig_bytes, 8);
    DWORD jmp_back = (g_dispatch_collision_addr + 8) - (DWORD)(g_trampoline + 8 + 5);
    g_trampoline[8] = 0xE9;
    *(DWORD*)(g_trampoline + 9) = jmp_back;

    /* Make trampoline executable */
    DWORD old_protect = 0;
    if (!VirtualProtect(g_trampoline, 13, PAGE_EXECUTE_READWRITE, &old_protect))
        return;
    pfn_DispatchCollisionEvents_trampoline = (DispatchCollisionEvents_t)g_trampoline;

    /* Patch original function: JMP to our hook (5 bytes, overwrites first 8 bytes) */
    if (!VirtualProtect((void*)g_dispatch_collision_addr, 8, PAGE_EXECUTE_READWRITE, &old_protect))
        return;

    DWORD rel_addr = (DWORD)hook_DispatchCollisionEvents - g_dispatch_collision_addr - 5;
    unsigned char jmp_patch[8];
    jmp_patch[0] = 0xE9; /* JMP rel32 */
    *(DWORD*)(jmp_patch + 1) = rel_addr;
    jmp_patch[5] = 0x90; /* NOP — pad to cover 6th byte */
    jmp_patch[6] = 0x90; /* NOP */
    jmp_patch[7] = 0x90; /* NOP */
    memcpy((void*)g_dispatch_collision_addr, jmp_patch, 8);

    VirtualProtect((void*)g_dispatch_collision_addr, 8, old_protect, &old_protect);
    g_dispatch_hooked = 1;
}

/* Uninstall the detour (called on level unload) */
static void uninstall_bonk_collision_hook(void) {
    if (!g_dispatch_hooked) return;
    DWORD old_protect = 0;
    if (VirtualProtect((void*)g_dispatch_collision_addr, 5, PAGE_EXECUTE_READWRITE, &old_protect)) {
        memcpy((void*)g_dispatch_collision_addr, g_dispatch_orig_bytes, 5);
        VirtualProtect((void*)g_dispatch_collision_addr, 5, old_protect, &old_protect);
    }
    g_dispatch_hooked = 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * <MESH> and <SPEEDMULT> tag support — custom BADBALL arguments
 *
 * The game's CreateBadBall (0x40BCA0) parses <CHASE>, <HOME>, <SIZE>,
 * <SPINDISTANCE> tags from BADBALL object names in MESHWORLD section 3.
 * We add two new tags:
 *
 *   <MESH>funball</MESH>       → mesh index 10 (FunBall mesh + texture)
 *   no MESH / other value       → mesh index 9  (8Ball: Sphere + 8ball texture)
 *
 *   <SPEEDMULT>2.0</SPEEDMULT> → multiplies ball+0x188 (max_speed) by value
 *   no SPEEDMULT tag            → max_speed stays at default (6.0)
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Ball struct offsets for tag processing */
#define BALL_HOME_POS_X       0xC60   /* float — spawn/return position X (set by CreateBadBall from obj.x) */
#define BALL_HOME_POS_Y       0xC64   /* float — spawn/return position Y (set by CreateBadBall from obj.y) */
#define BALL_HOME_POS_Z       0xC68   /* float — spawn/return position Z (set by CreateBadBall from obj.z) */
#define BALL_MESH_INDEX_FIELD 0x754   /* int — player/slot index into App mesh array at App+0x244 */
                                     /* Ball_Render only renders meshes for indices 0,1,2 (< 3 check) */
                                     /* 0 = Sphere (default player ball) */
                                     /* 1 = SphereBreak1 (we repurpose for 8Ball) */
                                     /* 2 = SphereBreak2 (we repurpose for FunBall) */
#define BALL_MAX_SPEED        0x188   /* float — max_speed (default 6.0, set by Ball_InitPhysicsDefaults) */

/* App mesh array layout (loaded in TimerDisplay at 0x004298c0):
 *   App+0x244 = "Meshes\Sphere"     [index 0] — default player ball
 *   App+0x248 = "Meshes\SphereBreak1" [index 1] — we copy 8Ball mesh ptr here
 *   App+0x24C = "Meshes\SphereBreak2" [index 2] — we copy FunBall mesh ptr here
 *   App+0x268 = "Meshes\8Ball"      [index 9] — source 8Ball mesh
 *   App+0x26C = "Meshes\FunBall"    [index 10] — source FunBall mesh
 *
 * Ball_Render (0x00403db8) renders mesh at App+0x244[ball+0x754*4] only if
 * ball+0x754 < 3. Setting ball+0x754=9 or 10 makes ball invisible (check fails).
 * Fix: copy 8Ball/FunBall mesh ptrs into slots 1/2, use those indices. */
#define APP_MESH_ARRAY         0x244   /* App offset — start of mesh pointer array */
#define APP_MESH_8BALL         0x268   /* App offset — 8Ball mesh pointer (index 9) */
#define APP_MESH_FUNBALL       0x26C   /* App offset — FunBall mesh pointer (index 10) */
#define MESH_SLOT_8BALL        1       /* We use slot 1 (SphereBreak1) for 8Ball */
#define MESH_SLOT_FUNBALL      2       /* We use slot 2 (SphereBreak2) for FunBall */

/* Section 3 object array (accessed via sceneobj) */
#define SCENEOBJ_OBJ_COUNT    0x898   /* int — total section 3 objects */
#define SCENEOBJ_OBJ_ARRAY    0xCA0   /* DWORD* — pointer to object pointer array */

/* Bad balls list (AthenaList at board+0x29D4) */
#define BOARD_BAD_BALLS_LIST  0x29D4

/* ═══════════════════════════════════════════════════════════════════════════
 * State
 * ═══════════════════════════════════════════════════════════════════════════ */

static HANDLE g_thread = NULL;
static volatile int g_running = 1;
static int g_quit_despawn_done = 0;  /* v55n_7: one-shot quit-time despawn guard */
static volatile int g_shutting_down = 0;  /* v55j_15: prevent hook crash on exit */

/* v55j_16: Check game's quit flag (App+0x159) — set BEFORE any memory is freed.
 * DLL_PROCESS_DETACH fires too late (after game frees board/ball memory).
 * This catches shutdown early enough to prevent use-after-free in hooks. */
static int game_is_quitting(void) {
    if (g_shutting_down) return 1;
    DWORD app = *(DWORD*)GLOBAL_APP_PTR;
    if (app && app > 0x10000 && !IsBadReadPtr((void*)(app + 0x159), 1)) {
        if (*(BYTE*)(app + 0x159) != 0) return 1;
    }
    return 0;
}
static char g_game_dir[MAX_PATH] = {0};

/* Track spawned objects so we can despawn them individually */
#define MAX_SPAWNED 16
static DWORD g_spawned_objs[MAX_SPAWNED];
static char  g_spawned_names[MAX_SPAWNED][32];
static int   g_spawned_count = 0;
static DWORD g_spawned_board = 0;

/* Config: seconds between GRID switches */
static float g_grid_speed = 3.0f;

/* Mesh path string — we copy testcube.MESHWORLD to levels\ at startup */
static char g_mesh_path[] = "levels\\testcube";

/* ═══════════════════════════════════════════════════════════════════════════
 * Helpers
 * ═══════════════════════════════════════════════════════════════════════════ */

/* get_board is declared in bass_proxy.h */

static DWORD cEnt_get_level(DWORD board) {
    if (!board) return 0;
    if (IsBadReadPtr((void*)(board + BOARD_LEVEL), 4)) return 0;
    return *(DWORD*)(board + BOARD_LEVEL);
}

/* Get the SceneObject from the level */
static DWORD cEnt_get_sceneobj(DWORD board) {
    DWORD level = cEnt_get_level(board);
    if (!level) return 0;
    if (IsBadReadPtr((void*)(level + LEVEL_SCENEOBJECT), 4)) return 0;
    return *(DWORD*)(level + LEVEL_SCENEOBJECT);
}

/* Find S1 reference points by scanning the sceneobj's S1 list */
static int cEnt_find_grid_points(DWORD board, float* out_x, float* out_y, float* out_z, int max_points, FILE* logf) {
    DWORD sceneobj = cEnt_get_sceneobj(board);
    if (!sceneobj) {
        if (logf) fprintf(logf, "  GRID: sceneobj=NULL\n");
        return 0;
    }

    /* S1 AthenaList is at sceneobj+0x894 */
    DWORD s1_list = sceneobj + 0x894;
    if (IsBadReadPtr((void*)(s1_list + 0x04), 4)) {
        if (logf) fprintf(logf, "  GRID: can't read S1 list count (sceneobj=0x%08X)\n", sceneobj);
        return 0;
    }
    int s1_count = *(int*)(s1_list + 0x04);
    if (logf) fprintf(logf, "  GRID: sceneobj=0x%08X, S1 count=%d\n", sceneobj, s1_count);
    if (s1_count <= 0 || s1_count > 1000) {
        return 0;
    }

    /* S1 data pointer is at s1_list+0x40C (AthenaList data) */
    if (IsBadReadPtr((void*)(s1_list + 0x40C), 4)) {
        if (logf) fprintf(logf, "  GRID: can't read S1 data ptr\n");
        return 0;
    }
    DWORD* s1_data = *(DWORD**)(s1_list + 0x40C);
    if (!s1_data || IsBadReadPtr(s1_data, s1_count * 4)) {
        if (logf) fprintf(logf, "  GRID: S1 data ptr invalid (0x%08X)\n", (DWORD)s1_data);
        return 0;
    }

    int found = 0;
    int i;
    for (i = 0; i < s1_count && found < max_points; i++) {
        DWORD entry = s1_data[i];
        if (!entry || entry < 0x10000) continue;
        if (IsBadReadPtr((void*)entry, 16)) continue;

        /* Try reading name as char* pointer at offset 0x00 */
        char* name = *(char**)(entry + S1ENTRY_NAME);
        if (name && !IsBadReadPtr(name, 5)) {
            if (logf && i < 10) fprintf(logf, "  GRID: S1[%d] name='%s'\n", i, name);
            /* Use strstr — name may be "testcube(GRID01)" not starting with GRID */
            if (strstr(name, "GRID") != NULL) {
                float x = *(float*)(entry + S1ENTRY_POS_X);
                float y = *(float*)(entry + S1ENTRY_POS_Y);
                float z = *(float*)(entry + S1ENTRY_POS_Z);
                out_x[found] = x;
                out_y[found] = y;
                out_z[found] = z;
                found++;
                if (logf) fprintf(logf, "  GRID: found %s at (%.1f, %.1f, %.1f)\n", name, x, y, z);
            }
        } else {
            /* Name might be inline char array, not a pointer */
            if (logf && i < 10) {
                fprintf(logf, "  GRID: S1[%d] entry=0x%08X (bad nameptr=0x%08X) raw: %02X%02X%02X%02X\n",
                    i, entry, (DWORD)name,
                    *(BYTE*)entry, *((BYTE*)entry+1), *((BYTE*)entry+2), *((BYTE*)entry+3));
            }
            char* inline_name = (char*)entry;
            if (strstr(inline_name, "GRID") != NULL) {
                float x = *(float*)(entry + S1ENTRY_POS_X);
                float y = *(float*)(entry + S1ENTRY_POS_Y);
                float z = *(float*)(entry + S1ENTRY_POS_Z);
                out_x[found] = x;
                out_y[found] = y;
                out_z[found] = z;
                found++;
                if (logf) fprintf(logf, "  GRID: found (inline) %s at (%.1f, %.1f, %.1f)\n", inline_name, x, y, z);
            }
        }
    }

    return found;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * <MESH> tag processing — apply mesh selection to spawned 8-balls
 *
 * After CreateBadBall runs during level load, we scan the MESHWORLD
 * section 3 objects for BADBALL entries with <MESH> tags, then match
 * them to spawned balls by home position (ball+0xC60/0xC64/0xC68 =
 * obj.x/obj.y/obj.z from the MESHWORLD file).
 *
 * If <MESH>funball</MESH> is found, ball+0x754 is set to 10 (FunBall).
 * Otherwise it stays at the default 9 (8Ball: Sphere + 8ball texture).
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Simple case-insensitive substring search */
static int cEnt_ci_strstr(const char* haystack, const char* needle) {
    while (*haystack) {
        const char* h = haystack;
        const char* n = needle;
        while (*h && *n && tolower(*h) == tolower(*n)) { h++; n++; }
        if (*n == 0) return 1;
        haystack++;
    }
    return 0;
}

/* Extract a property value from a <DAT> block.
 * Format: <DAT> KEY="value", KEY=number, ... </DAT>
 * or: <DAT> KEY="value", KEY=number </DAT> (no closing tag, end of string)
 * 
 * Searches for KEY= in the DAT block and copies the value (quoted or unquoted)
 * until the next comma or end of block.
 * Returns 1 if found, 0 otherwise. */
static int cEnt_extract_dat_prop(const char* name, const char* key, char* out_buf, int out_size) {
    /* Find <DAT> in the name string (case-insensitive) */
    const char* p = name;
    while (*p) {
        if (p[0] == '<' && _strnicmp(p + 1, "DAT", 3) == 0 && p[4] == '>') {
            const char* dat_start = p + 5;
            /* Find end of DAT block: </DAT> or end of string */
            const char* dat_end = dat_start;
            while (*dat_end) {
                if (dat_end[0] == '<' && dat_end[1] == '/' &&
                    _strnicmp(dat_end + 2, "DAT", 3) == 0 && dat_end[5] == '>') {
                    break;
                }
                dat_end++;
            }
            
            /* Search for KEY= within the DAT block */
            int key_len = (int)strlen(key);
            const char* q = dat_start;
            while (q < dat_end) {
                /* Skip whitespace */
                while (q < dat_end && (*q == ' ' || *q == '\t')) q++;
                if (q + key_len >= dat_end) break;
                
                /* Check for KEY= (case-insensitive) */
                if (_strnicmp(q, key, key_len) == 0 && q[key_len] == '=') {
                    const char* val_start = q + key_len + 1;
                    /* Skip leading quotes */
                    if (*val_start == '"') val_start++;
                    
                    /* Find end of value: comma, closing quote, or end of block */
                    const char* val_end = val_start;
                    while (val_end < dat_end && *val_end != ',' && *val_end != '"') val_end++;
                    
                    int len = (int)(val_end - val_start);
                    if (len > 0 && len < out_size) {
                        memcpy(out_buf, val_start, len);
                        out_buf[len] = '\0';
                        /* Trim trailing whitespace */
                        while (len > 0 && (out_buf[len-1] == ' ' || out_buf[len-1] == '\t')) {
                            out_buf[--len] = '\0';
                        }
                        return 1;
                    }
                    return 0;
                }
                /* Skip to next comma */
                while (q < dat_end && *q != ',') q++;
                if (q < dat_end) q++; /* skip comma */
            }
            return 0;
        }
        p++;
    }
    return 0;
}
static int cEnt_extract_tag(const char* name, const char* tag_name, char* out_buf, int out_size) {
    int tag_len = (int)strlen(tag_name);
    const char* p = name;
    while (*p) {
        /* Check for <TAGNAME> (case-insensitive) */
        if (p[0] == '<' && _strnicmp(p + 1, tag_name, tag_len) == 0 && p[1 + tag_len] == '>') {
            const char* val_start = p + 1 + tag_len + 1;
            /* Find </TAGNAME> (case-insensitive) */
            const char* q = val_start;
            while (*q) {
                if (q[0] == '<' && q[1] == '/' &&
                    _strnicmp(q + 2, tag_name, tag_len) == 0 && q[2 + tag_len] == '>') {
                    int len = (int)(q - val_start);
                    if (len > 0 && len < out_size) {
                        memcpy(out_buf, val_start, len);
                        out_buf[len] = '\0';
                        return 1;
                    }
                    return 0;
                }
                q++;
            }
            /* No closing tag found — extract up to next '<' or end of string.
             * This handles malformed MESHWORLD entries like <SPEEDMULT>4 (missing </SPEEDMULT>). */
            {
                const char* end = val_start;
                while (*end && *end != '<') end++;
                int len = (int)(end - val_start);
                if (len > 0 && len < out_size) {
                    memcpy(out_buf, val_start, len);
                    out_buf[len] = '\0';
                    return 1;
                }
            }
            return 0; /* opening tag found but no closing tag */
        }
        p++;
    }
    return 0; /* no tag found */
}

/* Process <MESH> and <SPEEDMULT> tags: scan MESHWORLD section 3 for BADBALL
 * objects with custom tags, match to spawned balls by home position, apply. */
static void cEnt_process_custom_tags(DWORD board, FILE* logf) {
    if (!board) return;

    if (logf) {
        fprintf(logf, "  TAGS: cEnt_process_custom_tags called (board=0x%08X)\n", board);
        fflush(logf);
    }

    /* Get the bad balls list (AthenaList at board+0x29D4) */
    DWORD* bad_balls_list = (DWORD*)(board + BOARD_BAD_BALLS_LIST);
    if (IsBadReadPtr(bad_balls_list, 8)) {
        if (logf) fprintf(logf, "  TAGS: bad_balls_list at 0x%08X is bad read\n", (DWORD)bad_balls_list);
        return;
    }
    int ball_count = *(int*)(bad_balls_list + 1);  /* count at +0x04 */
    if (logf) fprintf(logf, "  TAGS: ball_count=%d\n", ball_count);
    if (ball_count <= 0 || ball_count > 100) return;

    /* Get ball pointers from the AthenaList items array.
     * AthenaList stores the heap-allocated items pointer at +0x40C
     * (verified via AthenaList_GetAt at 0x0040a020 and AthenaList_Append
     * at 0x00453780 — both use *(this+0x40C) + index*4). */
    DWORD* ball_items = *(DWORD**)((BYTE*)bad_balls_list + 0x40C);
    if (!ball_items || IsBadReadPtr(ball_items, ball_count * 4)) return;

    /* Get the sceneobj to access section 3 objects */
    DWORD sceneobj = cEnt_get_sceneobj(board);
    if (!sceneobj) {
        if (logf) fprintf(logf, "  TAGS: sceneobj=NULL, skipping tag processing\n");
        return;
    }

    /* Read section 3 object count and array */
    if (IsBadReadPtr((void*)(sceneobj + SCENEOBJ_OBJ_COUNT), 4)) return;
    int obj_count = *(int*)(sceneobj + SCENEOBJ_OBJ_COUNT);
    if (obj_count <= 0 || obj_count > 1000) return;

    if (IsBadReadPtr((void*)(sceneobj + SCENEOBJ_OBJ_ARRAY), 4)) return;
    DWORD* obj_array_ptr = *(DWORD**)(sceneobj + SCENEOBJ_OBJ_ARRAY);
    if (!obj_array_ptr || IsBadReadPtr(obj_array_ptr, obj_count * 4)) return;

    int mesh_changes = 0;
    int speed_changes = 0;
    int i, j;

    /* For each section 3 object */
    for (i = 0; i < obj_count; i++) {
        DWORD obj_ptr = obj_array_ptr[i];
        if (!obj_ptr || obj_ptr < 0x10000) continue;
        if (IsBadReadPtr((void*)obj_ptr, 20)) continue;

        /* Read object name pointer */
        char* name = *(char**)(obj_ptr);
        if (!name || IsBadReadPtr(name, 8)) continue;

        /* Check if this is a BADBALL object */
        if (_strnicmp(name, "BADBALL", 7) != 0) continue;

        /* Check for <MESH> tag — DISABLED v20 (will re-enable later) */
        char mesh_value[64] = {0};
        /* int has_mesh = cEnt_extract_tag(name, "MESH", mesh_value, sizeof(mesh_value)); */
        int has_mesh = 0;  /* MESH tag inactive */

        /* Check for <SPEEDMULT> tag */
        char speed_value[64] = {0};
        int has_speed = cEnt_extract_tag(name, "SPEEDMULT", speed_value, sizeof(speed_value));

        if (!has_mesh && !has_speed) continue;

        /* Read object position (x, y, z at obj+0x04, +0x08, +0x0C) */
        float obj_x = *(float*)(obj_ptr + 0x04);
        float obj_y = *(float*)(obj_ptr + 0x08);
        float obj_z = *(float*)(obj_ptr + 0x0C);

        if (logf) {
            fprintf(logf, "  TAGS: BADBALL obj[%d] at (%.1f, %.1f, %.1f)", i, obj_x, obj_y, obj_z);
            if (has_mesh) fprintf(logf, " <MESH>(inactive)");
            if (has_speed) fprintf(logf, " <SPEEDMULT>%s</SPEEDMULT>", speed_value);
            fprintf(logf, "\n");
        }

        /* Match to a spawned ball by home position */
        for (j = 0; j < ball_count; j++) {
            DWORD ball = ball_items[j];
            if (!ball || ball < 0x10000) continue;
            if (IsBadReadPtr((void*)ball, 0xC70)) continue;

            float ball_home_x = *(float*)(ball + BALL_HOME_POS_X);
            float ball_home_y = *(float*)(ball + BALL_HOME_POS_Y);
            float ball_home_z = *(float*)(ball + BALL_HOME_POS_Z);

            /* Match by position (exact float comparison — CreateBadBall copies obj.xyz directly) */
            if (ball_home_x == obj_x && ball_home_y == obj_y && ball_home_z == obj_z) {
                /* Apply <MESH> tag — DISABLED v20 */
                if (0 && has_mesh) {
                    /* Use g_App (0x5341E0) directly — ball+0x10 may not be
                     * initialized yet when cEnt_process_custom_tags runs.
                     * Ball_Render reads ball+0x10 as App, but it's set by
                     * the game's per-frame loop, not during CreateBadBalls. */
                    DWORD app = *(DWORD*)GLOBAL_APP_PTR;
                    if (app && !IsBadReadPtr((void*)app, 0x280)) {
                        int target_slot = MESH_SLOT_8BALL;  /* default */
                        DWORD src_mesh = 0;

                        if (cEnt_ci_strstr(mesh_value, "funball")) {
                            target_slot = MESH_SLOT_FUNBALL;
                            src_mesh = *(DWORD*)(app + APP_MESH_FUNBALL);
                        } else {
                            /* Default: 8Ball */
                            src_mesh = *(DWORD*)(app + APP_MESH_8BALL);
                        }

                        if (src_mesh && !IsBadReadPtr((void*)src_mesh, 4)) {
                            /* Copy the 8Ball/FunBall mesh pointer into the target slot
                             * (overwriting SphereBreak1/2, which badballs never use).
                             * Ball_Render checks ball+0x754 < 3, so only slots 0-2 work. */
                            *(DWORD*)(app + APP_MESH_ARRAY + target_slot * 4) = src_mesh;
                            *(int*)(ball + BALL_MESH_INDEX_FIELD) = target_slot;
                            if (logf) fprintf(logf, "  TAGS: ball 0x%08X → mesh slot %d (mesh=0x%08X from App+0x%X)\n",
                                    ball, target_slot, src_mesh,
                                    cEnt_ci_strstr(mesh_value, "funball") ? APP_MESH_FUNBALL : APP_MESH_8BALL);
                            mesh_changes++;
                        } else if (logf) {
                            fprintf(logf, "  TAGS: ball 0x%08X → mesh src invalid (App+0x%X=0x%08X)\n",
                                    ball, cEnt_ci_strstr(mesh_value, "funball") ? APP_MESH_FUNBALL : APP_MESH_8BALL, src_mesh);
                        }
                    } else if (logf) {
                        fprintf(logf, "  TAGS: ball 0x%08X → App ptr invalid (0x%08X)\n", ball, app);
                    }
                }

                /* Apply <SPEEDMULT> tag — multiply current max_speed by value */
                if (has_speed) {
                    float mult = (float)atof(speed_value);
                    if (mult > 0.0f && mult <= 100.0f) {
                        float cur_speed = *(float*)(ball + BALL_MAX_SPEED);
                        *(float*)(ball + BALL_MAX_SPEED) = cur_speed * mult;
                        if (logf) fprintf(logf, "  TAGS: ball 0x%08X → max_speed %.1f × %.1f = %.1f\n",
                                ball, cur_speed, mult, cur_speed * mult);
                        speed_changes++;
                    } else if (logf) {
                        fprintf(logf, "  TAGS: ball 0x%08X → SPEEDMULT %.1f ignored (out of range 0.01-100.0)\n",
                                ball, mult);
                    }
                }
                break;
            }
        }
    }

    if (logf && (mesh_changes > 0 || speed_changes > 0)) {
        fprintf(logf, "  TAGS: applied %d mesh tag(s), %d speedmult tag(s)\n", mesh_changes, speed_changes);
        fflush(logf);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * CEA spawning pattern — load mesh + create object + register
 * Based on XRow's "Press S to spawn red bridge GLOBALLY" CEA script
 * ═══════════════════════════════════════════════════════════════════════════ */

static void cEnt_spawn_testcube_at(DWORD board, float px, float py, float pz, int grid_num, FILE* logf) {
    if (!board) return;

    /* 1. Get gfx_device from App */
    DWORD app = *(DWORD*)(board + BOARD_APP);
    if (!app || IsBadReadPtr((void*)app, 4)) {
        if (logf) fprintf(logf, "  GRID: app=NULL\n");
        return;
    }
    DWORD gfx_device = *(DWORD*)(app + APP_GFX_DEVICE);
    if (!gfx_device || IsBadReadPtr((void*)gfx_device, 4)) {
        if (logf) fprintf(logf, "  GRID: gfx_device=NULL\n");
        return;
    }

    /* 2. Load mesh via MeshWorld_ctor */
    void* mesh = pfn_operator_new(MESHWORLD_SIZE);
    if (!mesh) {
        if (logf) fprintf(logf, "  GRID: failed to allocate mesh\n");
        return;
    }
    memset(mesh, 0, MESHWORLD_SIZE);

    void* loaded_mesh = pfn_MeshWorld_ctor(mesh, (void*)gfx_device, g_mesh_path);
    if (!loaded_mesh) {
        if (logf) fprintf(logf, "  GRID: MeshWorld_ctor failed for '%s'\n", g_mesh_path);
        return;
    }

    /* 3. Allocate PopCylinder object */
    void* obj = pfn_operator_new(POPCYLINDER_SIZE);
    if (!obj) {
        if (logf) fprintf(logf, "  GRID: failed to allocate PopCylinder\n");
        return;
    }
    memset(obj, 0, POPCYLINDER_SIZE);

    /* 4. Call PopCylinder_ctor(this, board, X, Y, Z, mesh) */
    void* result = pfn_PopCylinder_ctor(obj, (void*)board, px, py, pz, mesh);
    if (!result) {
        if (logf) fprintf(logf, "  GRID: PopCylinder_ctor failed\n");
        return;
    }

    /* 5. Add to board+0x2578 (update list) */
    pfn_AthenaList_Append((DWORD*)(board + BOARD_UPDATE_LIST), obj);

    /* 6. Add to board+0xCD4 (render list) */
    pfn_AthenaList_Append((DWORD*)(board + BOARD_RENDER_LIST), obj);

    /* 7. Add collision object to board+0x10EC */
    DWORD col_obj = *(DWORD*)((char*)obj + PC_COLLISION_OBJ);
    if (col_obj) {
        pfn_AthenaList_Append((DWORD*)(board + BOARD_COLLISION_LIST), (void*)col_obj);

        /* Also add to board+0x8B0+0x18 (scene collision) */
        DWORD scene_col = *(DWORD*)(board + BOARD_SCENE_OBJ);
        if (scene_col) {
            pfn_AthenaList_Append((DWORD*)(scene_col + 0x18), (void*)col_obj);
        }
    }

    /* 8. Add to scene spatial tree (board+0x8AC+0x480+0x1C) */
    DWORD level = cEnt_get_level(board);
    if (level) {
        DWORD sceneobj = *(DWORD*)(level + LEVEL_SCENEOBJECT);
        if (sceneobj) {
            pfn_AthenaList_Append((DWORD*)(sceneobj + 0x1C), obj);
        }
    }

    if (logf) {
        fprintf(logf, "  GRID: spawned testcube(GRID%02d) at (%.1f,%.1f,%.1f) obj=0x%08X\n",
                grid_num, px, py, pz, (DWORD)obj);
        fflush(logf);
    }

    /* Track spawned object for later despawn */
    if (g_spawned_count < MAX_SPAWNED) {
        g_spawned_objs[g_spawned_count] = (DWORD)obj;
        snprintf(g_spawned_names[g_spawned_count], 32, "testcube(GRID%02d)", grid_num);
        g_spawned_count++;
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Despawn a specific testcube by GRID name
 * Based on Rotator_RemoveAndFree (0x436FC0) pattern:
 *   1. Remove collision obj from board+0x8B0+0x18 and board+0x10EC
 *   2. Remove obj from board+0x2578 (update) and board+0xCD4 (render)
 *   3. Remove obj from sceneobj+0x1C (scene tree)
 *   4. Call collision obj destructor
 *   5. Set flags to prevent double-free
 * ═══════════════════════════════════════════════════════════════════════════ */

static void cEnt_despawn_object(DWORD board, DWORD obj, FILE* logf) {
    if (!board || !obj) return;
    if (IsBadReadPtr((void*)obj, 0x10D0)) return;

    char* name = "(unknown)";
    if (logf) fprintf(logf, "  DESPAWN: removing obj=0x%08X\n", obj);

    /* 1. Mark as removed (prevents update/render from touching it) */
    *(BYTE*)((char*)obj + 0x10E5) = 1;  /* removed flag */
    *(BYTE*)((char*)obj + 0x10E4) = 1;  /* inactive flag */

    /* 2. Remove collision object from board+0x10EC */
    DWORD col_obj = *(DWORD*)((char*)obj + PC_COLLISION_OBJ);
    if (col_obj) {
        /* Remove from board+0x10EC (collision list) */
        pfn_AthenaList_Remove((DWORD*)(board + BOARD_COLLISION_LIST), (int)col_obj);

        /* Remove from board+0x8B0+0x18 (scene collision) */
        DWORD scene_col = *(DWORD*)(board + BOARD_SCENE_OBJ);
        if (scene_col) {
            pfn_AthenaList_Remove((DWORD*)(scene_col + 0x18), (int)col_obj);
        }

        /* Call collision object's destructor (vtable[0] with arg=1 to free) */
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

    /* 3. Remove obj from board+0x2578 (update list) */
    pfn_AthenaList_Remove((DWORD*)(board + BOARD_UPDATE_LIST), (int)obj);
    /* 3a. Also remove from board+0x8B8 (Scene_Update list, type 8/34 only) */
    pfn_AthenaList_Remove((DWORD*)(board + BOARD_SCENE_UPDATE_LIST), (int)obj);

    /* 4. Remove obj from board+0xCD4 (render list) */
    pfn_AthenaList_Remove((DWORD*)(board + BOARD_RENDER_LIST), (int)obj);

    /* 5. Remove obj from sceneobj+0x1C (scene tree) */
    DWORD level = cEnt_get_level(board);
    if (level) {
        DWORD sceneobj = *(DWORD*)(level + LEVEL_SCENEOBJECT);
        if (sceneobj) {
            pfn_AthenaList_Remove((DWORD*)(sceneobj + 0x1C), (int)obj);
        }
    }

    if (logf) {
        fprintf(logf, "  DESPAWN: obj=0x%08X removed from all lists\n", obj);
        fflush(logf);
    }
}

static void cEnt_despawn_by_name(const char* target_name, DWORD board, FILE* logf) {
    int i;
    for (i = 0; i < g_spawned_count; i++) {
        if (strstr(g_spawned_names[i], target_name) != NULL) {
            if (logf) fprintf(logf, "  DESPAWN: found '%s' at index %d, obj=0x%08X\n",
                    g_spawned_names[i], i, g_spawned_objs[i]);
            cEnt_despawn_object(board, g_spawned_objs[i], logf);
            /* Shift array down */
            int j;
            for (j = i; j < g_spawned_count - 1; j++) {
                g_spawned_objs[j] = g_spawned_objs[j + 1];
                strcpy(g_spawned_names[j], g_spawned_names[j + 1]);
            }
            g_spawned_count--;
            return;
        }
    }
    if (logf) fprintf(logf, "  DESPAWN: '%s' not found in spawned objects\n", target_name);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * cEnt_load_mesh_file — loads a .MESH or .MESHWORLD file
 *
 * For .MESHWORLD files: uses Level_MeshWorldCtor (0x461510).
 * For .MESH files: uses MeshNode_ctor (0x471C20) to load the .MESH binary.
 *
 * Returns a Level pointer (0x10D0 bytes, .MESHWORLD) or MeshNode pointer
 * (0x18 bytes, .MESH). Caller must check is_mesh_file to know which.
 * Returns NULL on failure (file not found, parse error, etc).
 * ═══════════════════════════════════════════════════════════════════════════ */
static void* cEnt_load_mesh_file(DWORD gfx_device, const char* path, int* out_is_mesh_node, FILE* logf) {
    if (!path || !path[0] || !gfx_device) return NULL;
    *out_is_mesh_node = 0;

    /* Check if this is a .MESH file (path starts with "meshes\") */
    int is_mesh_file = (_strnicmp(path, "meshes\\", 6) == 0);

    if (is_mesh_file) {
        /* Check if path.mesh exists */
        char check_path[256];
        snprintf(check_path, 255, "%s.mesh", path);
        check_path[255] = 0;

        if (pfn_check_file_access(check_path, 0) != 0) {
            /* .mesh not found — try .meshworld fallback */
            snprintf(check_path, 255, "%s.meshworld", path);
            check_path[255] = 0;
            if (pfn_check_file_access(check_path, 0) != 0) {
                if (logf) fprintf(logf, "  LOAD_MESH: neither .mesh nor .meshworld for '%s'\n", path);
                return NULL;
            }
            is_mesh_file = 0; /* Fall through to .MESHWORLD loading */
        }
    } else {
        /* .MESHWORLD: check if file exists */
        char check_path[256];
        snprintf(check_path, 255, "%s.meshworld", path);
        check_path[255] = 0;
        if (pfn_check_file_access(check_path, 0) != 0) {
            if (logf) fprintf(logf, "  LOAD_MESH: '%s' not found\n", check_path);
            return NULL;
        }
    }

    if (is_mesh_file) {
        /* ═══ .MESH file loading via MeshNode_ctor (0x471C20) ═══ */
        void* mesh_node = pfn_operator_new(MESHNODE_SIZE);
        if (!mesh_node) {
            if (logf) fprintf(logf, "  LOAD_MESH: failed to alloc MeshNode\n");
            return NULL;
        }
        memset(mesh_node, 0, MESHNODE_SIZE);

        void* result = pfn_MeshNode_ctor(mesh_node, (void*)gfx_device, path);
        if (!result) {
            if (logf) fprintf(logf, "  LOAD_MESH: MeshNode_ctor failed for '%s'\n", path);
            return NULL;
        }

        /* Check has_mesh flag at MeshNode+0x0D */
        BYTE has_mesh = *(BYTE*)((char*)mesh_node + 0x0D);
        if (!has_mesh) {
            if (logf) fprintf(logf, "  LOAD_MESH: MeshNode loaded but has_mesh=0 for '%s'\n", path);
            return NULL;
        }

        /* Verify MeshWorld* at MeshNode+0x08 */
        void* mesh_world = *(void**)((char*)mesh_node + 0x08);
        if (!mesh_world || IsBadReadPtr(mesh_world, 0x100)) {
            if (logf) fprintf(logf, "  LOAD_MESH: MeshWorld* invalid for '%s'\n", path);
            return NULL;
        }

        *out_is_mesh_node = 1;
        if (logf) fprintf(logf, "  LOAD_MESH: .MESH OK: MeshNode=0x%08X MeshWorld=0x%08X '%s'\n",
                (DWORD)mesh_node, (DWORD)mesh_world, path);
        return mesh_node;
    } else {
        /* ═══ .MESHWORLD file loading via Level_MeshWorldCtor (0x461510) ═══ */
        void* mesh = pfn_operator_new(MESHWORLD_SIZE);
        if (!mesh) {
            if (logf) fprintf(logf, "  LOAD_MESH: failed to alloc mesh\n");
            return NULL;
        }
        memset(mesh, 0, MESHWORLD_SIZE);

        char path_buf[256];
        strncpy(path_buf, path, 255);
        path_buf[255] = 0;

        void* loaded = pfn_MeshWorld_ctor(mesh, (void*)gfx_device, path_buf);
        if (!loaded) {
            if (logf) fprintf(logf, "  LOAD_MESH: Level_MeshWorldCtor failed for '%s'\n", path_buf);
            return NULL;
        }

        return mesh;
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * REF:Rotater — Spawn Dizzy SWIRL at REF:Rotater positions in MESHWORLD
 *
 * Based on XRow's SpawnSpinSwirl CEA: loads Level3-Swirl mesh via
 * MeshWorld_ctor, creates Rotator_ctor_Impossible, registers in board lists.
 * ═══════════════════════════════════════════════════════════════════════════ */

static void cEnt_spawn_rotater_at(DWORD board, float px, float py, float pz,
                              const char* mesh_path,
                              float rot_x, float rot_y, float rot_z,
                              float ros_x, float ros_y, float ros_z,
                              int ai_type,
                              FILE* logf) {
    if (!board) return;

    /* v55n: TarBubble (ai_type 25) — DECORATIVE FLOATING BUBBLE.
     * Native TarBubble is a purely decorative S1 ref point that occasionally
     * spawns a floating bubble sprite (it does NOT trap/slow the ball — that's
     * Gluebie 43 / Tarpit 44). We store the position and spawn a real native
     * bubble object (0x1C-byte, vtable 0x4D6E48) via ctor FUN_0044fb50.
     * The bubble is driven (update+render) from the Present hook for all boards. */
    if (ai_type == 25) {
        if (g_tarbubble_count < MAX_TARBUBBLES) {
            g_tarbubble_pos[g_tarbubble_count].x = px;
            g_tarbubble_pos[g_tarbubble_count].y = py;
            g_tarbubble_pos[g_tarbubble_count].z = pz;
            g_tarbubble_pos[g_tarbubble_count].obj = 0;
            g_tarbubble_pos[g_tarbubble_count].countdown = 0;
            g_tarbubble_count++;
            if (logf) fprintf(logf, "  TARBUBBLE: decorative bubble ref (%.1f,%.1f,%.1f) [%d]\n",
                    px, py, pz, g_tarbubble_count - 1);
        }
        return;  /* decorative bubble — position stored, bubble spawned per-frame */
    }

    /* v55i: WaterWheel (ai_type 26) — spawn PopCylinder for visibility,
     * AND store mesh for per-frame rotation. The PopCylinder provides the
     * render vtable; we rotate the mesh via vtable[22]+[21] each frame. */
    if (ai_type == 26) {
        DWORD app = *(DWORD*)(board + BOARD_APP);
        if (!app || IsBadReadPtr((void*)app, 4)) return;
        DWORD gfx_device = *(DWORD*)(app + APP_GFX_DEVICE);
        if (!gfx_device || IsBadReadPtr((void*)gfx_device, 4)) return;

        if (g_waterwheel_count < MAX_WATERWHEELS) {
            struct WaterWheelState* ww = &g_waterwheels[g_waterwheel_count];
            /* v55m_44f: Default mesh = "levels\\Waterwheel" (user provides
             * Waterwheel.MESHWORLD). If that file doesn't exist, fall back
             * to "levels\\_default" (user's placeholder, same pattern as
             * Cloudscape case 28). A MESH= tag override still wins. */
            const char* ww_path = mesh_path && mesh_path[0] ? mesh_path : "levels\\\\Waterwheel";
            {
                char ww_check[256];
                snprintf(ww_check, 255, "%s.meshworld", ww_path);
                if (pfn_check_file_access(ww_check, 0) != 0) {
                    if (logf) fprintf(logf, "  WATERWHEEL: '%s' not found, falling back to levels\\_default\n", ww_path);
                    ww_path = "levels\\\\_default";
                }
            }

            /* v55m_44h: Load the mesh; if it produces an invalid meshbuffer
             * list (malformed/corrupt file), retry with levels\_default so a
             * bad custom Waterwheel.MESHWORLD cannot crash the level render
             * (crash 0x465789 = render reads strip array at mb+0x418 of a
             * 0x7C component meshbuffer — out of bounds). */
            DWORD mesh = 0;
            {
                int attempt;
                for (attempt = 0; attempt < 2; attempt++) {
                    /* Create mesh object via MeshWorld_ctor */
                    mesh = (DWORD)pfn_operator_new(MESHWORLD_SIZE);
                    if (!mesh) {
                        if (logf) fprintf(logf, "  WATERWHEEL: failed to alloc mesh\n");
                        return;
                    }
                    memset((void*)mesh, 0, MESHWORLD_SIZE);
                    void* result = pfn_MeshWorld_ctor((void*)mesh, (void*)gfx_device, ww_path);
                    if (!result) {
                        if (logf) fprintf(logf, "  WATERWHEEL: MeshWorld_ctor failed for '%s'\n", ww_path);
                        mesh = 0;
                        if (attempt == 0) { ww_path = "levels\\\\_default"; continue; }
                        return;
                    }
                    /* Validate the meshbuffer list: MeshWorld+0x2C AthenaList,
                     * count at +0x04, items at +0x40C. NOTE (v55m_44h):
                     * count=0 is VALID — the render (0x45E0E0) skips the empty
                     * list (cmp [eax+4],0; jg → falls to SceneObject render
                     * 0x470150) and draws from the mesh's own vertex data.
                     * Rejecting count=0 made the wheel never spawn (44g bug).
                     * Only reject unreadable memory or absurd counts. */
                    if (!IsBadReadPtr((void*)(mesh + 0x2C), 0x10)) {
                        DWORD* mb_list = (DWORD*)(mesh + 0x2C);
                        int mb_count = *(int*)(mb_list + 1);
                        if (mb_count < 0 || mb_count > 10000 ||
                            (mb_count > 0 &&
                             (IsBadReadPtr((void*)((BYTE*)mb_list + 0x40C), 4) ||
                              IsBadReadPtr(*(void**)((BYTE*)mb_list + 0x40C), mb_count * 4)))) {
                            if (logf) fprintf(logf, "  WATERWHEEL: '%s' loaded but meshbuffer list invalid (count=%d), falling back to levels\\_default\n",
                                              ww_path, mb_count);
                            mesh = 0;
                            if (attempt == 0) { ww_path = "levels\\\\_default"; continue; }
                            return;
                        }
                        if (logf) fprintf(logf, "  WATERWHEEL: mesh '%s' meshbuffers=%d\n", ww_path, mb_count);
                    }
                    break;  /* valid mesh loaded */
                }
                if (!mesh) {
                    if (logf) fprintf(logf, "  WATERWHEEL: no usable mesh (both '%s' and levels\\_default failed), skipping\n", ww_path);
                    return;
                }
            }

            ww->mesh_obj = mesh;
            ww->pc_obj = mesh;  /* v55m_44o: pc_obj = the MeshWorld itself.
                                 * NO PopCylinder → NO CollisionLevel → the
                                 * crash (0x465777/0x465789, CollisionLevel
                                 * render walking broken component meshbuffer
                                 * strip arrays) is structurally impossible. */
            ww->x = px;
            ww->y = py;
            ww->z = pz;
            ww->angle = 0.0f;
            ww->active = 1;
            ww->creak_channel = 0;  /* set below */

            /* v55m_44o: The mesh's own vtable is 0x4D8FB0, whose [18] is
             * 0x470150 (SceneObject render). 0x470150 renders the ROOT's
             * meshbuffer list (+0x2C, empty for a BRANCH octree) AND
             * recurses into children via +0x424 (sub-level list) — so it
             * renders the full waterwheel geometry from the mesh's own
             * vertex data. Hook vtable[18] with the rotation wrapper and
             * register the MESH itself in the render/scene lists. No
             * CollisionLevel is created anywhere (PopCylinder_ctor is not
             * called), so the crash path (0x465650 walking component
             * meshbuffers with +0x418 strip-array OOB) never exists. */
            {
                DWORD mesh_vtbl = *(DWORD*)mesh;
                if (mesh_vtbl && !IsBadReadPtr((void*)mesh_vtbl, 0x400)) {
                    DWORD* new_vtbl = (DWORD*)pfn_operator_new(0x400);
                    if (new_vtbl) {
                        memcpy(new_vtbl, (void*)mesh_vtbl, 0x400);
                        ww->orig_vtable18 = new_vtbl[18];
                        new_vtbl[18] = (DWORD)&cEnt_waterwheel_render;
                        *(DWORD*)mesh = (DWORD)new_vtbl;
                        if (logf) fprintf(logf, "  WATERWHEEL: mesh vtable[18] hooked (orig=0x%08X -> 0x%08X)\n",
                                          ww->orig_vtable18, (DWORD)&cEnt_waterwheel_render);
                    }
                }
            }
            /* v55m_44o: The mesh is NOT registered in any render/scene list.
             * Registering a SceneObject (vtable 0x4D8FB0) in board+0xCD4 or
             * sceneobj+0x1C makes the game's FinishLoad process it as level
             * geometry and create a CollisionLevel (0x465080, vtable
             * 0x4D9068) for its component meshbuffers — and those component
             * meshbuffers (0x4D9CDC, 0x7C bytes, no strip arrays at +0x418)
             * are exactly what the CollisionLevel render (vtable[18]=0x465650)
             * faults on (0x465777/0x465789). The native game NEVER registers
             * the waterwheel mesh: it stores the loaded mesh at board+0x4374
             * (a bare slot, verified in the Dizzy setup 0x41D325) and renders
             * it manually. We do the same: the mesh lives in ww->pc_obj and
             * cEnt_waterwheel_present_render draws it every frame via its own
             * vtable[18] (0x470150) with the rotation matrix. NO lists, NO
             * scene tree → the game never sees the mesh → no CollisionLevel
             * → no crash. */
            (void)0;
            /* Load WheelCreak sound channel (native: [board+0x878]+0x490
             * holds the game's pre-loaded wheelcreak slot). Replicate the
             * v55m_44c pattern: grab the slot, get a channel, never crash. */
            ww->creak_channel = 0;
            {
                DWORD app2 = *(DWORD*)(board + BOARD_APP);
                if (app2 && app2 > 0x10000 && !IsBadReadPtr((void*)(app2 + 0x490), 4)) {
                    DWORD slot = *(DWORD*)(app2 + 0x490);
                    if (slot && slot > 0x10000 && !IsBadReadPtr((void*)slot, 0x20)) {
                        if (pfn_Sound_GetChannel) {
                            DWORD ch = (DWORD)pfn_Sound_GetChannel(slot);
                            if (ch && ch > 0x10000 && !IsBadReadPtr((void*)ch, 0x20)) {
                                ww->creak_channel = ch;
                            }
                        }
                    }
                    if (logf) {
                        fprintf(logf, "  WATERWHEEL: creak channel=0x%08X (app=0x%08X)\n",
                                ww->creak_channel, app2);
                        fflush(logf);
                    }
                }
            }
            ww->active = 1;
            g_waterwheel_count++;

            if (logf) fprintf(logf, "  WATERWHEEL: spawned mesh=0x%08X at (%.1f,%.1f,%.1f) path='%s'\n",
                    mesh, px, py, pz, ww_path);
        }
        return;
    }

    /* 1. Get gfx_device from App */
    DWORD app = *(DWORD*)(board + BOARD_APP);
    if (!app || IsBadReadPtr((void*)app, 4)) return;
    DWORD gfx_device = *(DWORD*)(app + APP_GFX_DEVICE);
    if (!gfx_device || IsBadReadPtr((void*)gfx_device, 4)) return;

    /* 2. Determine mesh path — MESH property takes priority over AI default.
     * NULL mesh paths (e.g. FlagWaver with code-generated mesh) are valid. */
    const char* path = NULL;
    if (mesh_path && mesh_path[0]) {
        path = mesh_path;
    } else if (ai_type >= 1 && ai_type <= 5) {
        path = g_ai_mesh_paths[ai_type];
    } else if (ai_type == 12) {
        path = NULL;
    } else if (ai_type == 13) {
        path = NULL;
    } else if (ai_type == 14) {
        path = NULL;
    } else if (ai_type == 15) {
        path = NULL;
    } else if (ai_type == 16) {
        path = NULL;
    } else if (ai_type == 22) {
        path = NULL;  /* Chomper: MeshNode_ctor loads own mesh */
    } else if (ai_type == 28) {
        path = NULL;  /* Cloudscape: Sprite_ctor loads own texture */
    } else if (ai_type >= 30 && ai_type <= 33) {
        path = NULL;  /* Level-family: self-loads MESHWORLD or uses board mesh */
    } else if (ai_type == 41 || ai_type == 42) {
        /* v53g-5: Trapdoor and Odd_Lifter read mesh from board+0x878+0x594/0x5C8
         * (App mesh table). They ignore the mesh parameter. Setting path=NULL
         * avoids loading a mesh file that would be leaked. */
        path = NULL;
    } else {
        path = g_swirl_mesh_path;
    }
    
    /* v54d: Override path=NULL for entity types that handle their own mesh
     * loading internally. The if/else-if chain above sets path=mesh_path
     * when mesh_path is non-NULL, which prevents these overrides from
     * triggering in the else-if branches. This override runs AFTER.
     * 
     * v55: REMOVED ai_types 10, 11, 15, 22 from this list — they have .MESH
     * files and should go through the .MESH swap code (PopCylinder + mesh swap).
     * Keeping them here sent them to native constructors that call Level_ctor → crash. */
    if (ai_type == 12 || ai_type == 13 || ai_type == 14 ||
        ai_type == 16 || ai_type == 22 || ai_type == 28 ||
        (ai_type >= 30 && ai_type <= 33) ||
        ai_type == 41 || ai_type == 42) {
        path = NULL;
    }
    
    /* If mesh path is "levels\\_default", use _default.MESHWORLD as placeholder.
     * This file is provided by the user and serves as a null mesh fallback
     * for entities that don't have a real mesh file (Bumper, Tarpit, Chrome). */
    if (path && _stricmp(path, "levels\\_default") == 0) {
        /* _default.MESHWORLD is the user-provided placeholder */
        /* Keep the path as-is — the game's MeshWorld_ctor will try to load it */
    }

    /* 3. Load mesh file — handles both .MESHWORLD and .MESH formats.
     * NULL path is valid for entities with no mesh file (FlagWaver, Sign).
     *
     * CRITICAL FIX v53f: For constructors that call Stands_ctor/Level_RenderCtor
     * internally (Rotator, ArenaStands, Looper, Gear, Spinner, GameLevel, etc.),
     * the mesh parameter MUST be a fully-initialized Level with vertex data at
     * SceneObject+0x440. Separately-loaded meshes from .MESHWORLD files may have
     * NULL vertex data, causing crashes in Level_LoadMeshes.
     *
     * Solution: Use the board's own Level (board+0x8AC) as the mesh parameter.
     * The board's Level has all vertex data properly loaded. Then after
     * construction, swap obj+0x08 (MeshWorld*) to the desired visual mesh. */
    int is_mesh_node = 0;
    void* mesh = NULL;

    /* Load mesh file — handles both .MESHWORLD and .MESH formats.
     * NULL path is valid for entities with no mesh file (FlagWaver, Sign). */
    if (path) {
        mesh = cEnt_load_mesh_file(gfx_device, path, &is_mesh_node, logf);
        if (!mesh) {
            if (logf) fprintf(logf, "  ROTATER: cEnt_load_mesh_file failed for '%s', trying Swirl fallback\n", path);
            is_mesh_node = 0;
            mesh = cEnt_load_mesh_file(gfx_device, g_swirl_mesh_path, &is_mesh_node, logf);
            if (!mesh) {
                if (logf) fprintf(logf, "  ROTATER: Swirl fallback also failed\n");
                return;
            }
        }
    }

    /* For .MESH files (MeshNode, 0x18 bytes), we can't use PopCylinder_ctor
     * because it calls Stands_ctor which reads Level offsets (+0x18 AthenaList,
     * +0x430 flags, +0x480 SceneObject) that don't exist on a MeshNode.
     *
     * Instead: load a .MESHWORLD placeholder (Swirl) for PopCylinder_ctor,
     * then swap obj+0x08 (MeshWorld*) to the .MESH MeshWorld after construction.
     * This gives the object a proper vtable, position, and collision (from Swirl),
     * but renders the correct .MESH model. */
    if (is_mesh_node) {
        /* Extract MeshWorld* from MeshNode+0x08 */
        DWORD mesh_world_from_node = *(DWORD*)((char*)mesh + 0x08);
        if (!mesh_world_from_node || IsBadReadPtr((void*)mesh_world_from_node, 0x100)) {
            if (logf) fprintf(logf, "  ROTATER: .MESH MeshWorld* invalid, skipping\n");
            return;
        }

        /* Load Swirl as placeholder for PopCylinder_ctor */
        int swirl_is_node = 0;
        void* swirl_mesh = cEnt_load_mesh_file(gfx_device, g_swirl_mesh_path, &swirl_is_node, logf);
        if (!swirl_mesh || swirl_is_node) {
            if (logf) fprintf(logf, "  ROTATER: Swirl placeholder failed, skipping .MESH entity\n");
            return;
        }

        /* Create PopCylinder with Swirl mesh (proper vtable + position + collision) */
        void* obj = pfn_operator_new(POPCYLINDER_SIZE);
        if (!obj) {
            if (logf) fprintf(logf, "  ROTATER: failed to alloc PopCylinder for .MESH\n");
            return;
        }
        memset(obj, 0, POPCYLINDER_SIZE);
        void* result0 = pfn_PopCylinder_ctor(obj, (void*)board, px, py, pz, swirl_mesh);
        if (!result0) {
            if (logf) fprintf(logf, "  ROTATER: PopCylinder_ctor failed for .MESH entity\n");
            return;
        }

        /* SWAP: Replace obj+0x08 (MeshWorld*) with the .MESH MeshWorld*
         * PopCylinder stores the visual MeshWorld at +0x08 (copied from mesh param
         * by Stands_ctor). The render function reads MeshBuffers from this pointer.
         * Collision data is in obj+0x10E0 (Level_RenderCtor copy) — unchanged. */
        *(DWORD*)((char*)obj + 0x08) = mesh_world_from_node;

        if (logf) {
            fprintf(logf, "  ROTATER: spawned (.MESH swap) at (%.1f,%.1f,%.1f) obj=0x%08X mesh='%s' MW=0x%08X\n",
                    px, py, pz, (DWORD)obj, path, mesh_world_from_node);
            fflush(logf);
        }

        /* Register on board lists */
        pfn_AthenaList_Append((DWORD*)(board + BOARD_UPDATE_LIST), obj);
        pfn_AthenaList_Append((DWORD*)(board + BOARD_RENDER_LIST), obj);

        /* Add collision object */
        DWORD col_obj = *(DWORD*)((char*)obj + 0x10E0);
        if (col_obj) {
            pfn_AthenaList_Append((DWORD*)(board + BOARD_COLLISION_LIST), (void*)col_obj);
            DWORD scene_col = *(DWORD*)(board + BOARD_SCENE_OBJ);
            if (scene_col) {
                pfn_AthenaList_Append((DWORD*)(scene_col + 0x18), (void*)col_obj);
            }
        }

        /* Add to scene spatial tree */
        DWORD level = cEnt_get_level(board);
        if (level) {
            DWORD sceneobj = *(DWORD*)(level + LEVEL_SCENEOBJECT);
            if (sceneobj) {
                pfn_AthenaList_Append((DWORD*)(sceneobj + 0x1C), obj);
            }
        }

        /* Track for despawn (no rotation — static .MESH visual) */
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
                strncpy(g_rotater_cfg[g_rotater_count].mesh_path, mesh_path, 127);
                g_rotater_cfg[g_rotater_count].mesh_path[127] = 0;
            } else {
                g_rotater_cfg[g_rotater_count].mesh_path[0] = 0;
            }
            g_rotater_count++;
        }
        return;
    }

    /* 4. Allocate object and call constructor based on AI type */
    void* obj = NULL;
    if (ai_type == 6) {
        /* AI 6: Rotator_ctor_Impossible (SWIRL with rotation) */
        obj = pfn_operator_new(ROTATER_SIZE);
        if (!obj) {
            if (logf) fprintf(logf, "  ROTATER: failed to alloc object\n");
            return;
        }
        memset(obj, 0, ROTATER_SIZE);
        void* result = pfn_Rotator_ctor(obj, (void*)board, px, py, pz, mesh);
        if (!result) {
            if (logf) fprintf(logf, "  ROTATER: Rotator_ctor failed\n");
            return;
        }
        /* v55m_48d: The native SWIRL ctor (0x00435940) sets the initial
         * angle obj+0x10E8 = -0.20f (0xBE4CCCCD) when the board is in race
         * mode (App+0x237 == 0) — which is always the case for cEnt spawns.
         * Reset it to 0.0 by default so the rotater starts unrotated.
         * (The debug table's "Sets Angle" step now yields 0.0.) */
        *(float*)(obj + 0x10E8) = 0.0f;
    } else if ((ai_type >= 7 && ai_type <= 14) || (ai_type >= 17 && ai_type <= 22) || (ai_type >= 27 && ai_type <= 44) || ai_type == 45) {
        /* New constructor types — each with specific signature.
         * Range 7-14: ArenaStands + Wavy family (Flag/Flag2)
         * Range 17-22: ArenaStands variants + Chomper
         * Range 27-42: Level-family + Stands_ctor family */
        switch (ai_type) {
            case 7:  /* cEnt_DFloor1_ctor — Neon DFLOOR1 */
                obj = pfn_operator_new(ARENASTANDS_SIZE);
                if (!obj) { if (logf) fprintf(logf, "  ROTATER: failed to alloc DFloor1\n"); return; }
                memset(obj, 0, ARENASTANDS_SIZE);
                cEnt_DFloor1_ctor(obj, (void*)board, px, py, pz, mesh);
                break;
            case 17: /* cEnt_DFloor2_ctor — Neon DFLOOR2 */
                obj = pfn_operator_new(ARENASTANDS_SIZE);
                if (!obj) { if (logf) fprintf(logf, "  ROTATER: failed to alloc DFloor2\n"); return; }
                memset(obj, 0, ARENASTANDS_SIZE);
                cEnt_DFloor2_ctor(obj, (void*)board, px, py, pz, mesh);
                break;
            case 18: /* cEnt_DFloor3_ctor — Neon DFLOOR3 */
                obj = pfn_operator_new(ARENASTANDS_SIZE);
                if (!obj) { if (logf) fprintf(logf, "  ROTATER: failed to alloc DFloor3\n"); return; }
                memset(obj, 0, ARENASTANDS_SIZE);
                cEnt_DFloor3_ctor(obj, (void*)board, px, py, pz, mesh);
                break;
            case 19: /* cEnt_DFloor4_ctor — Neon DFLOOR4 (with post-construction config) */
                obj = pfn_operator_new(ARENASTANDS_SIZE);
                if (!obj) { if (logf) fprintf(logf, "  ROTATER: failed to alloc DFloor4\n"); return; }
                memset(obj, 0, ARENASTANDS_SIZE);
                cEnt_DFloor4_ctor(obj, (void*)board, px, py, pz, mesh);
                break;
            case 20: /* cEnt_FlickRing_ctor — Neon Arena FLICKRING */
                obj = pfn_operator_new(ARENASTANDS_SIZE);
                if (!obj) { if (logf) fprintf(logf, "  ROTATER: failed to alloc FlickRing\n"); return; }
                memset(obj, 0, ARENASTANDS_SIZE);
                cEnt_FlickRing_ctor(obj, (void*)board, px, py, pz, mesh);
                break;
            case 21: /* cEnt_Trode_ctor — Neon TRODE */
                obj = pfn_operator_new(ARENASTANDS_SIZE);
                if (!obj) { if (logf) fprintf(logf, "  ROTATER: failed to alloc Trode\n"); return; }
                memset(obj, 0, ARENASTANDS_SIZE);
                cEnt_Trode_ctor(obj, (void*)board, px, py, pz, mesh);
                break;
            case 22: /* Chomper — Tower Race chomper.
                     * Uses standard PopCylinder pattern: create PopCylinder with
                     * the Chomper mesh loaded by cEnt_load_mesh_file above.
                     * The game's render pipeline renders via vtable[2] (BuildStrips).
                     * Sound state machine tracked via mesh_path match in spawn_done. */
                obj = pfn_operator_new(POPCYLINDER_SIZE);
                if (!obj) { if (logf) fprintf(logf, "  ROTATER: failed to alloc Chomper PopCylinder\n"); return; }
                memset(obj, 0, POPCYLINDER_SIZE);
                pfn_PopCylinder_ctor(obj, (void*)board, px, py, pz, mesh);
                break;
            case 27: /* Spinner_Level_ctor — Expert Race "BRIDGE" (falling bridge piece)
                      * 6 params: (this, board, x, y, z, mesh, float_param) */
                obj = pfn_operator_new(SPINNER_LEVEL_SIZE);
                if (!obj) { if (logf) fprintf(logf, "  ROTATER: failed to alloc Spinner\n"); return; }
                memset(obj, 0, SPINNER_LEVEL_SIZE);
                pfn_Spinner_Level_ctor(obj, (void*)board, px, py, pz, mesh, 0.0f);
                break;
            case 28: /* Cloudscape — Sky Race clouds (Sprite_ctor, 0xD4 bytes)
                      * Loads "levels\\Cloudscape" texture, falls back to "levels\\_default" */
                {
                    DWORD app = *(DWORD*)(board + BOARD_APP);
                    DWORD gfx_device = app ? *(DWORD*)(app + APP_GFX_DEVICE) : 0;
                    if (!gfx_device) { if (logf) fprintf(logf, "  ROTATER: no gfx_device for Cloudscape\n"); return; }
                    obj = pfn_operator_new(SPRITE_SIZE);
                    if (!obj) { if (logf) fprintf(logf, "  ROTATER: failed to alloc Cloudscape\n"); return; }
                    memset(obj, 0, SPRITE_SIZE);
                    const char* cloud_path = mesh_path && mesh_path[0] ? mesh_path : "levels\\Cloudscape";
                    /* Check if Cloudscape.MESHWORLD exists, else use _default */
                    char check_path[256];
                    snprintf(check_path, 255, "%s.meshworld", cloud_path);
                    if (pfn_check_file_access(check_path, 0) != 0) {
                        cloud_path = "levels\\_default";
                        if (logf) fprintf(logf, "  ROTATER: Cloudscape.MESHWORLD not found, using _default\n");
                    }
                    pfn_Sprite_ctor(obj, (void*)gfx_device, cloud_path);
                }
                break;
            case 29: /* Gear_ctor — Impossible Race Gear (9 params!)
                      * (this, board, x1, y1, z1, x2, y2, z2, mesh)
                      * x2/y2/z2 are a second position point — use same as x1/y1/z1 */
                obj = pfn_operator_new(GEAR_REAL_SIZE);
                if (!obj) { if (logf) fprintf(logf, "  ROTATER: failed to alloc Gear\n"); return; }
                memset(obj, 0, GEAR_REAL_SIZE);
                pfn_Gear_ctor_real(obj, (void*)board, px, py, pz, px, py, pz, mesh);
                break;
            case 30: /* Bell_ctor — Expert Race Bell (5 params: this, board, x, y, z)
                      * v54d: Bell_ctor calls Level_ctor (no mesh). Its vtable[1] is
                      * Rotator_Update which needs vertex data. We use the .MESH swap
                      * pattern: PopCylinder with Swirl mesh, then swap obj+0x08 to
                      * the Bell .MESH MeshWorld.
                      * v54d FIX: Removed vtable override — causes stack corruption
                      * because Bell's vtable functions have different calling
                      * conventions than PopCylinder's. The mesh swap alone gives
                      * the correct visual appearance (static Bell model). */
                {
                    DWORD app2 = *(DWORD*)(board + BOARD_APP);
                    DWORD gfx2 = app2 ? *(DWORD*)(app2 + APP_GFX_DEVICE) : 0;
                    if (!gfx2) { if (logf) fprintf(logf, "  ROTATER: no gfx for Bell\n"); return; }
                    /* Load Bell.MESH via MeshNode_ctor */
                    void* bell_meshnode = cEnt_load_mesh_file(gfx2, "meshes\\Bell", &is_mesh_node, logf);
                    if (!bell_meshnode || !is_mesh_node) {
                        if (logf) fprintf(logf, "  ROTATER: Bell.MESH load failed, trying Swirl\n");
                        bell_meshnode = cEnt_load_mesh_file(gfx2, g_swirl_mesh_path, &is_mesh_node, logf);
                        if (!bell_meshnode) return;
                    }
                    DWORD bell_mw = *(DWORD*)((char*)bell_meshnode + 0x08);
                    /* Load Swirl as PopCylinder base */
                    int bell_isnode = 0;
                    void* bell_swirl = cEnt_load_mesh_file(gfx2, g_swirl_mesh_path, &bell_isnode, logf);
                    if (!bell_swirl || bell_isnode) {
                        if (logf) fprintf(logf, "  ROTATER: Bell Swirl base failed\n");
                        return;
                    }
                    obj = pfn_operator_new(BELL_SIZE);
                    if (!obj) { if (logf) fprintf(logf, "  ROTATER: failed to alloc Bell\n"); return; }
                    memset(obj, 0, BELL_SIZE);
                    pfn_PopCylinder_ctor(obj, (void*)board, px, py, pz, bell_swirl);
                    /* Swap MeshWorld to Bell mesh */
                    if (bell_mw) *(DWORD*)((char*)obj + 0x08) = bell_mw;
                    /* v54d: NO vtable override — PopCylinder vtable is correct */
                }
                break;
            case 31: /* Fan_ctor — Expert Race Fan (6 params: this, board, x, y, z, float)
                      * v54d: Same Level_ctor issue as Bell. Uses .MESH swap pattern.
                      * Fan mesh is "meshes\\fanbody". No vtable override (stack safe). */
                {
                    DWORD app2 = *(DWORD*)(board + BOARD_APP);
                    DWORD gfx2 = app2 ? *(DWORD*)(app2 + APP_GFX_DEVICE) : 0;
                    if (!gfx2) { if (logf) fprintf(logf, "  ROTATER: no gfx for Fan\n"); return; }
                    void* fan_meshnode = cEnt_load_mesh_file(gfx2, "meshes\\fanbody", &is_mesh_node, logf);
                    if (!fan_meshnode || !is_mesh_node) {
                        if (logf) fprintf(logf, "  ROTATER: FanBody.MESH load failed, trying Swirl\n");
                        fan_meshnode = cEnt_load_mesh_file(gfx2, g_swirl_mesh_path, &is_mesh_node, logf);
                        if (!fan_meshnode) return;
                    }
                    DWORD fan_mw = *(DWORD*)((char*)fan_meshnode + 0x08);
                    int fan_isnode = 0;
                    void* fan_swirl = cEnt_load_mesh_file(gfx2, g_swirl_mesh_path, &fan_isnode, logf);
                    if (!fan_swirl || fan_isnode) {
                        if (logf) fprintf(logf, "  ROTATER: Fan Swirl base failed\n");
                        return;
                    }
                    obj = pfn_operator_new(FAN_SIZE);
                    if (!obj) { if (logf) fprintf(logf, "  ROTATER: failed to alloc Fan\n"); return; }
                    memset(obj, 0, FAN_SIZE);
                    pfn_PopCylinder_ctor(obj, (void*)board, px, py, pz, fan_swirl);
                    if (fan_mw) *(DWORD*)((char*)obj + 0x08) = fan_mw;
                    /* v54d: NO vtable override — PopCylinder vtable is correct */
                }
                break;
            case 32: /* SawBlade_ctor — Expert Race SawBlade (5 params: this, board, x, y, z)
                      * v54d: Same Level_ctor issue. Uses Level8-Saw.MESHWORLD.
                      * No vtable override (stack safe). */
                {
                    DWORD app2 = *(DWORD*)(board + BOARD_APP);
                    DWORD gfx2 = app2 ? *(DWORD*)(app2 + APP_GFX_DEVICE) : 0;
                    if (!gfx2) { if (logf) fprintf(logf, "  ROTATER: no gfx for SawBlade\n"); return; }
                    /* SawBlade has a .MESHWORLD file: levels\\Level8-Saw */
                    void* saw_mesh = cEnt_load_mesh_file(gfx2, "levels\\Level8-Saw", &is_mesh_node, logf);
                    if (!saw_mesh) {
                        if (logf) fprintf(logf, "  ROTATER: Level8-Saw.MESHWORLD load failed, trying Swirl\n");
                        saw_mesh = cEnt_load_mesh_file(gfx2, g_swirl_mesh_path, &is_mesh_node, logf);
                        if (!saw_mesh) return;
                    }
                    obj = pfn_operator_new(SAWBLADE_SIZE);
                    if (!obj) { if (logf) fprintf(logf, "  ROTATER: failed to alloc SawBlade\n"); return; }
                    memset(obj, 0, SAWBLADE_SIZE);
                    /* Use PopCylinder_ctor to set up the Level properly */
                    pfn_PopCylinder_ctor(obj, (void*)board, px, py, pz, saw_mesh);
                    /* v54d: NO vtable override — PopCylinder vtable is correct */
                }
                break;
            case 33: /* Bonk_ctor — Warm-Up Race Bonk (5 params: this, board, x, y, z)
                      * Self-loads "levels\\level5-bonk" MESHWORLD via Level_MeshWorldCtor. */
                obj = pfn_operator_new(BONK_SIZE);
                if (!obj) { if (logf) fprintf(logf, "  ROTATER: failed to alloc Bonk\n"); return; }
                memset(obj, 0, BONK_SIZE);
                pfn_Bonk_ctor(obj, (void*)board, px, py, pz);
                /* Track Bonk for collision event hook (E:CALLHAMMER/E:HAMMERCHASE)
                 * v55m_42f: hook disabled — manual SEH trampoline at DispatchCollisionEvents
                 * causes Draw crashes (0x452376) on non-native levels. */
                if (g_bonk_count < MAX_BONKS) {
                    g_bonk_objs[g_bonk_count] = (DWORD)obj;
                    g_bonk_count++;
                }
                break;
            case 34: /* BreakBridge_ctor — Intermediate Race bridge (6 params: this, board, x, y, z, mesh)
                      * Uses Pendulum vtable with Rotator_Update for tilt animation. */
                obj = pfn_operator_new(POPCYLINDER_SIZE);
                if (!obj) { if (logf) fprintf(logf, "  ROTATER: failed to alloc BreakBridge\n"); return; }
                memset(obj, 0, POPCYLINDER_SIZE);
                pfn_BreakBridge_ctor(obj, (void*)board, px, py, pz, mesh);
                break;
            /* Stands_ctor family (35-42) — all take (this, board, mesh) = 3 params.
             * Position is NOT passed to constructor — set after construction.
             * These are NOT Rotator_ctor_t (6 params) — using wrong signature
             * causes stack imbalance → crash at Level_ctor during Draw. */
            case 35: /* Catapult_ctor — Stands_ctor family (this, board, mesh)
                     * v55d: Port full Catapult system:
                     *   1. Catapult_ctor calls Stands_ctor → clones spatial trees into obj+0x18
                     *   2. Create collision/render Level via Level_RenderCtor (same as TipperVisual)
                     *      and store at obj+0x10D4 — this is the collision object
                     *   3. Add collision object to board+0x10EC (collision list)
                     *   4. Track in g_catapults[] for per-frame vtable[11] calls
                     *   5. E:CATAPULTBOTTOM collision event triggers Catapult_Launch (0x434290)
                     * Position: obj+0x10D8/0x10DC/0x10E0 (matches Tipper pattern) */
                obj = pfn_operator_new(CATAPULT_SIZE);
                if (!obj) { if (logf) fprintf(logf, "  ROTATER: failed to alloc Catapult\n"); return; }
                memset(obj, 0, CATAPULT_SIZE);
                pfn_Catapult_ctor(obj, (void*)board, mesh);
                *(float*)((char*)obj + 0x10D8) = px;
                *(float*)((char*)obj + 0x10DC) = py;
                *(float*)((char*)obj + 0x10E0) = pz;
                /* v55m_1: Catapult_ctor already creates Level_RenderCtor at obj+0x10D4 internally.
                 * Do NOT create a duplicate — the old code overwrote the ctor's Level,
                 * breaking the TowerCollisionEvents mesh pointer match. */
                /* v55m_43h: RE-ENABLED collision list append.
                 * v55m_42w disabled it because it crashed at 0x478EDD — but that
                 * crash was actually caused by the vtable[18] hook calling 0x45E0E0
                 * during Update, corrupting the mesh buffers that the collision
                 * Level at obj+0x10D4 references. With the g_in_draw_phase guard,
                 * 0x45E0E0 is never called during Update, so the collision Level's
                 * mesh data stays intact. Adding it now makes the catapult solid. */
                {
                    DWORD cat_col_obj = *(DWORD*)((char*)obj + 0x10D4);
                    if (cat_col_obj && !IsBadReadPtr((void*)cat_col_obj, 0x20)) {
                        pfn_AthenaList_Append((DWORD*)(board + BOARD_COLLISION_LIST), (void*)cat_col_obj);
                        DWORD scene_col = *(DWORD*)(board + BOARD_SCENE_OBJ);
                        if (scene_col) {
                            pfn_AthenaList_Append((DWORD*)(scene_col + 0x18), (void*)cat_col_obj);
                        }
                        if (logf) fprintf(logf, "  ROTATER: Catapult collision Level 0x%08X added to lists\n", cat_col_obj);
                    }
                }
                /* v55m_42f: Do NOT add to board+0x43B8 — causes heap corruption on non-Tower levels.
                 * Do NOT use DispatchCollisionEvents SEH trampoline hook — causes Draw crashes.
                 * Use Present-hook radius trigger instead (restored from v55m_28m). */
                if (g_catapult_count < MAX_CATAPULTS) {
                    CatapultState* cs = &g_catapults[g_catapult_count];
                    cs->obj = (DWORD)obj;
                    cs->board = (DWORD)board;
                    cs->x = px;
                    cs->y = py;
                    cs->z = pz;
                    cs->yaw = rot_y;
                    cs->launching = 0;
                    cs->cooldown = 0;
                    cs->was_in_zone = 0;
                    cs->col_obj = 0;
                    cs->collided = 0;
                    cs->countdown = 0;
                    cs->arm_angle = 0.0f;
                    cs->orig_vtable18 = 0;
                    cs->arm_obj = 0;
                    cs->arm_orig_vtable18 = 0;
                    cs->arm_mesh = 0;
                    cs->arm_active = 0;

                    /* v55m_43h: Hook BOTH vtable[18]s for Y-rotation render.
                     * CRITICAL: The catapult's OWN vtable[18] is NOT called
                     * during rendering! The catapult (Stands family) renders
                     * through the collision/render Level at obj+0x10D4, which
                     * is appended to the scene spatial tree (scene_col+0x18).
                     * The scene render calls Level.vtable[18] (0x45E0E0) to
                     * draw it — so we must hook the LEVEL's vtable[18], not
                     * the catapult's. We hook BOTH (the catapult's own too,
                     * harmless, in case the game calls it directly).
                     * v55m_43c: The Catapult vtable (0x4D4F98) is a MULTI-BLOCK
                     * vtable with 200+ entries. The old 0x50-byte copy was too
                     * small — the game reads slots past entry 20 → stack
                     * corruption (0002:00009E75). Copy the FULL vtable
                     * (0x400 bytes = 256 entries). g_in_draw_phase guard
                     * prevents D3D transforms during Update. */
                    {
                        DWORD* orig_vt = *(DWORD**)obj;
                        if (orig_vt && !IsBadReadPtr(orig_vt, 0x400)) {
                            DWORD* new_vt = (DWORD*)pfn_operator_new(0x400);
                            if (new_vt) {
                                memcpy(new_vt, orig_vt, 0x400);
                                cs->orig_vtable18 = new_vt[18];
                                new_vt[18] = (DWORD)cEnt_catapult_render;
                                *(DWORD**)obj = new_vt;
                                if (logf) fprintf(logf, "  ROTATER: Catapult vtable[18] hook installed (0x%08X -> 0x%08X, full 0x400B copy)\n",
                                    cs->orig_vtable18, new_vt[18]);
                            }
                        }
                        /* v55m_43h: ALSO hook the collision Level's vtable[18].
                         * This is the object that actually gets rendered by the
                         * scene spatial tree. The Level is at obj+0x10D4 (created
                         * by Catapult_ctor internally). Its vtable[18] is 0x45E0E0. */
                        DWORD cat_level = *(DWORD*)((char*)obj + 0x10D4);
                        if (cat_level && !IsBadReadPtr((void*)cat_level, 0x20)) {
                            DWORD* level_vt = *(DWORD**)cat_level;
                            if (level_vt && !IsBadReadPtr(level_vt, 0x100)) {
                                DWORD* new_level_vt = (DWORD*)pfn_operator_new(0x100);
                                if (new_level_vt) {
                                    memcpy(new_level_vt, level_vt, 0x100);
                                    cs->arm_orig_vtable18 = new_level_vt[18];
                                    new_level_vt[18] = (DWORD)cEnt_catapult_render;
                                    *(DWORD**)cat_level = new_level_vt;
                                    cs->arm_obj = cat_level;
                                    if (logf) fprintf(logf, "  ROTATER: Catapult Level vtable[18] hook installed (0x%08X -> 0x%08X)\n",
                                        cs->arm_orig_vtable18, new_level_vt[18]);
                                    /* v55m_43h: DUMP the collision Level's mesh structure so we can
                                     * find where the vertex array lives. */
                                    {
                                        DWORD lvl_mw = *(DWORD*)((char*)cat_level + 0x08);
                                        DWORD lvl_so = *(DWORD*)((char*)cat_level + 0x480);
                                        DWORD so_v440 = lvl_so ? *(DWORD*)((char*)lvl_so + 0x440) : 0;
                                        DWORD so_v43c = lvl_so ? *(DWORD*)((char*)lvl_so + 0x43C) : 0;
                                        if (logf) fprintf(logf, "  ROTATER: DUMP colLevel=0x%08X mw=0x%08X sceneObj=0x%08X so+440=0x%08X so+43C=%d\n",
                                            cat_level, lvl_mw, lvl_so, so_v440, so_v43c);
                                        if (lvl_mw && !IsBadReadPtr((void*)lvl_mw, 0x40)) {
                                            DWORD* mwlist = (DWORD*)(lvl_mw + 0x2C);
                                            int mwcount = mwlist ? *(int*)(mwlist + 0x1) : -1;
                                            DWORD* mwitems = mwlist ? *(DWORD**)(mwlist + 0x103) : NULL;
                                            if (logf) fprintf(logf, "  ROTATER: DUMP mw+2C count=%d items=0x%08X\n", mwcount, (DWORD)mwitems);
                                            if (mwcount > 0 && mwcount < 64 && mwitems) {
                                                DWORD mb = mwitems[0];
                                                if (mb && !IsBadReadPtr((void*)mb, 0x50)) {
                                                    DWORD mb_v438 = *(DWORD*)((char*)mb + 0x438);
                                                    DWORD mb_v43c = *(DWORD*)((char*)mb + 0x43C);
                                                    DWORD mb_v4 = *(DWORD*)((char*)mb + 0x4);
                                                    DWORD mb_vc = *(DWORD*)((char*)mb + 0xC);
                                                    DWORD mb_vt = *(DWORD*)((char*)mb + 0x0);
                                                    DWORD mb_v14 = *(DWORD*)((char*)mb + 0x14);
                                                    DWORD mb_v418 = *(DWORD*)((char*)mb + 0x418);
                                                    DWORD mb_v85d = *(DWORD*)((char*)mb + 0x85D);
                                                    DWORD mb_v8 = *(DWORD*)((char*)mb + 0x8);
                                                    DWORD mb_v10 = *(DWORD*)((char*)mb + 0x10);
                                                    DWORD mb_v860 = *(DWORD*)((char*)mb + 0x860);
                                                    DWORD mb_v864 = *(DWORD*)((char*)mb + 0x864);
                                                    DWORD mb_v868 = *(DWORD*)((char*)mb + 0x868);
                                                    DWORD mw_448 = *(DWORD*)((char*)lvl_mw + 0x448);
                                                    DWORD mw_44c = *(DWORD*)((char*)lvl_mw + 0x44C);
                                                    DWORD mw_434 = *(DWORD*)((char*)lvl_mw + 0x434);
                                                    DWORD mw_45c = *(DWORD*)((char*)lvl_mw + 0x45C);
                                                    DWORD mb_v424 = mb + 0x424;  /* embedded list */
                                                    int mb_subcount = 0;
                                                    DWORD* mb_subitems = NULL;
                                                    if (mb && !IsBadReadPtr((void*)(mb + 0x10), 0x840)) {
                                                        mb_subcount = *(int*)(mb_v424 + 0x4);
                                                        mb_subitems = *(DWORD**)(mb_v424 + 0x40C);
                                                    }
                                                    DWORD sub0 = 0;
                                                    DWORD sub0_448 = 0;
                                                    DWORD sub0_444 = 0;
                                                    DWORD sub0_8 = 0;
                                                    if (mb_subcount > 0 && mb_subcount < 64 && mb_subitems && !IsBadReadPtr((void*)mb_subitems, 4)) {
                                                        sub0 = mb_subitems[0];
                                                        if (sub0 && !IsBadReadPtr((void*)sub0, 0x20)) {
                                                            sub0_448 = *(DWORD*)((char*)sub0 + 0x448);
                                                            sub0_444 = *(DWORD*)((char*)sub0 + 0x444);
                                                            sub0_8 = *(DWORD*)((char*)sub0 + 0x8);
                                                        }
                                                    }
                                                    DWORD mb2 = mwcount > 1 ? mwitems[1] : 0;
                                                    if (logf) fprintf(logf, "  ROTATER: DUMP mb[0]=0x%08X vt=0x%08X +4=%d +8=0x%08X +C=%d +10=%d +14=%d\n",
                                                        mb, mb_vt, mb_v4, mb_v8, mb_vc, mb_v10, mb_v14);
                                                    if (logf) fprintf(logf, "  ROTATER: DUMP mb[0] +418=0x%08X +438=0x%08X +43C=%d +85D=%d +860=0x%08X +864=0x%08X +868=0x%08X\n",
                                                        mb_v418, mb_v438, mb_v43c, mb_v85d, mb_v860, mb_v864, mb_v868);
                                                    if (logf) fprintf(logf, "  ROTATER: DUMP mb[0] +424 list=%d items=0x%08X; sub[0]=0x%08X sub+448=0x%08X sub+444=%d sub+8=0x%08X\n",
                                                        mb_subcount, (DWORD)mb_subitems, sub0, sub0_448, sub0_444, sub0_8);
                                                    if (logf) fprintf(logf, "  ROTATER: DUMP mw+448=0x%08X +44C=0x%08X +434=0x%08X +45C=0x%08X\n",
                                                        mw_448, mw_44c, mw_434, mw_45c);
                                                    if (logf) fprintf(logf, "  ROTATER: DUMP mb[1]=0x%08X\n", mb2);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    g_catapult_count++;
                    if (logf) fprintf(logf, "  ROTATER: Catapult tracked in g_catapults[%d] (count=%d)\n",
                        g_catapult_count - 1, g_catapult_count);

                    /* v55m_42f: load dropin sound via BASS for this catapult */
                    cEnt_load_dropin_sample(logf);
                }
                break;
            case 36: /* Mace_ctor */
                obj = pfn_operator_new(MACE_SIZE);
                if (!obj) { if (logf) fprintf(logf, "  ROTATER: failed to alloc Mace\n"); return; }
                memset(obj, 0, MACE_SIZE);
                pfn_Mace_ctor(obj, (void*)board, mesh);
                *(float*)((char*)obj + 0x10D8) = px;
                *(float*)((char*)obj + 0x10DC) = py;
                *(float*)((char*)obj + 0x10E0) = pz;
                break;
            case 37: /* Tipper_ctor — 3 params (this, board, mesh)
                     * v55b: Create TipperVisual + attach, matching native Dizzy_CreateDynamicObjects.
                     * Without TipperVisual, obj+0x10D4=0 (NULL) → vtable[11] state machine
                     * tries AthenaList_Append(board+0x8B0+0x18, NULL) → no collision.
                     * TipperVisual is the object that gets added/removed from the collision
                     * list as the tipper raises/lowers. */
                {
                    obj = pfn_operator_new(TIPPER_SIZE);
                    if (!obj) { if (logf) fprintf(logf, "  ROTATER: failed to alloc Tipper\n"); return; }
                    memset(obj, 0, TIPPER_SIZE);
                    pfn_Tipper_ctor(obj, (void*)board, mesh);
                    *(float*)((char*)obj + 0x10D8) = px;
                    *(float*)((char*)obj + 0x10DC) = py;
                    *(float*)((char*)obj + 0x10E0) = pz;

                    /* Create TipperVisual from the mesh (same as native:
                     * TipperVisual_ctor(alloc, board+0x4370) where board+0x4370 is
                     * Level_RenderCtor result from Level3-Tipper mesh).
                     * We use the already-loaded 'mesh' (Level_MeshWorldCtor result)
                     * as the parent for Level_RenderCtor, then TipperVisual_ctor. */
                    if (mesh && !IsBadReadPtr(mesh, 0x100)) {
                        /* Create render Level from the Tipper mesh */
                        void* render_level = pfn_operator_new(LEVEL_SIZE);
                        if (render_level) {
                            memset(render_level, 0, LEVEL_SIZE);
                            render_level = pfn_Level_RenderCtor(render_level, mesh);
                            if (render_level) {
                                /* Create TipperVisual from the render Level */
                                void* visual = pfn_operator_new(LEVEL_SIZE);
                                if (visual) {
                                    memset(visual, 0, LEVEL_SIZE);
                                    visual = pfn_TipperVisual_ctor2(visual, (int)render_level);
                                    if (visual) {
                                        /* Store TipperVisual at obj+0x10D4 (DWORD index 0x435)
                                         * Native: puVar4[0x435] = this (TipperVisual) */
                                        *(DWORD*)((char*)obj + 0x10D4) = (DWORD)visual;
                                        /* Attach visual to Tipper behavior object */
                                        pfn_TipperVisual_Attach(visual, (int)obj);
                                        if (logf) fprintf(logf, "  ROTATER: TipperVisual created at 0x%08X, attached to Tipper 0x%08X\n",
                                                (DWORD)visual, (DWORD)obj);
                                    } else {
                                        if (logf) fprintf(logf, "  ROTATER: TipperVisual_ctor failed\n");
                                    }
                                }
                            } else {
                                if (logf) fprintf(logf, "  ROTATER: Level_RenderCtor failed for Tipper visual\n");
                            }
                        }
                    } else {
                        if (logf) fprintf(logf, "  ROTATER: Tipper mesh invalid, skipping TipperVisual creation\n");
                    }
                }
                break;
            case 38: /* Lifter_ctor — 7 params (this, board, x, y, z, mesh, lifter_id) */
                obj = pfn_operator_new(LIFTER_SIZE);
                if (!obj) { if (logf) fprintf(logf, "  ROTATER: failed to alloc Lifter\n"); return; }
                memset(obj, 0, LIFTER_SIZE);
                pfn_Lifter_ctor(obj, (void*)board, px, py, pz, mesh, 0);
                break;
            case 39: /* SpeedCylinder_ctor — 7 params (this, board, x, y, z, int_param, mesh)
                      * v55n_2 FIX: Native-parity registration. The ctor creates a
                      * collision/render Level at obj+0x10E0 (via Level_RenderCtor) and
                      * Stands_ctor clones spatial trees into obj+0x18 (collision). To be
                      * SOLID and have BEHAVIOR, we must:
                      *   1. Register obj+0x10E0 collision Level into board+0x10EC + scene tree
                      *   2. Set MeshBuffer+0x47C = entity on all collision mesh MeshBuffers so
                      *      UpRaceCollisionEvents (0x4119B0) finds the entity via [[MeshBuffer]+0x47C]
                      *   3. Set entity+0x47C = entity (self-ref, needed by vtable[1] Rotator_Update)
                      *   4. Track in g_speedcyls[] for the per-frame slot 11 (0x43D8C0) driver */
                obj = pfn_operator_new(SPEEDCYLINDER_SIZE);
                if (!obj) { if (logf) fprintf(logf, "  ROTATER: failed to alloc SpeedCylinder\n"); return; }
                memset(obj, 0, SPEEDCYLINDER_SIZE);
                pfn_SpeedCylinder_ctor(obj, (void*)board, px, py, pz, 0, mesh);
                *(float*)((char*)obj + 0x10D4) = px;
                *(float*)((char*)obj + 0x10D8) = py;
                *(float*)((char*)obj + 0x10DC) = pz;
                {
                    /* 1. Collision Level at obj+0x10E0 (like Catapult's +0x10D4) */
                    DWORD sc_col = *(DWORD*)((char*)obj + 0x10E0);
                    if (sc_col && !IsBadReadPtr((void*)sc_col, 0x20)) {
                        /* 2. Set MeshBuffer+0x47C = entity on ALL MeshBuffers of the collision
                         *    Level so the N:SPEEDCYLINDER handler finds the entity.
                         *    MeshBuffers live in the Level's MeshWorld AthenaList (Level+0x08
                         *    -> MeshWorld, +0x2C = MeshBuffer AthenaList, count+0x04, items+0x40C). */
                        DWORD lvl_mw = *(DWORD*)((char*)sc_col + 0x08);
                        if (lvl_mw && !IsBadReadPtr((void*)(lvl_mw + 0x2C), 0x410)) {
                            DWORD mb_list = lvl_mw + 0x2C;
                            DWORD mb_count = *(DWORD*)(mb_list + 0x04);
                            DWORD mb_items = *(DWORD*)(mb_list + 0x40C);
                            int mi;
                            for (mi = 0; mi < (int)mb_count && mb_items && !IsBadReadPtr((void*)mb_items, mb_count*4); mi++) {
                                DWORD mb = ((DWORD*)mb_items)[mi];
                                if (mb && !IsBadReadPtr((void*)mb, 0x48C)) {
                                    *(DWORD*)((char*)mb + 0x47C) = (DWORD)obj;
                                }
                            }
                        }
                        /* Register collision Level into board+0x10EC + scene tree */
                        pfn_AthenaList_Append((DWORD*)(board + BOARD_COLLISION_LIST), (void*)sc_col);
                        DWORD scene_col2 = *(DWORD*)(board + BOARD_SCENE_OBJ);
                        if (scene_col2) {
                            pfn_AthenaList_Append((DWORD*)(scene_col2 + 0x18), (void*)sc_col);
                        }
                        if (logf) fprintf(logf, "  ROTATER: SpeedCyl collision Level 0x%08X registered\n", sc_col);
                    }
                    /* 3. Entity self-ref +0x47C (needed by vtable[1] Rotator_Update 0x4606D0) */
                    *(DWORD*)((char*)obj + 0x47C) = (DWORD)obj;
                }
                /* 4. Track for per-frame slot 11 driver */
                if (g_speedcyl_count < MAX_SPEEDCYLINDERS) {
                    g_speedcyls[g_speedcyl_count].obj = (DWORD)obj;
                    g_speedcyls[g_speedcyl_count].board = board;
                    g_speedcyls[g_speedcyl_count].x = px;
                    g_speedcyls[g_speedcyl_count].y = py;
                    g_speedcyls[g_speedcyl_count].z = pz;
                    g_speedcyls[g_speedcyl_count].col_level = *(DWORD*)((char*)obj + 0x10E0);
                    g_speedcyls[g_speedcyl_count].mesh_world = mesh ? (DWORD)mesh : 0;
                    g_speedcyl_count++;
                }
                break;
            case 45: /* v55n_3: TimeButton_ctor — 5 params (this, board, x, y, z, mesh).
                      * Native Up race TimeButton (0x436C10, 0x10E8 bytes, vtable 0x4D5830):
                      *   Stands_ctor creates collision/render Level at obj+0x10E0 + tree clones
                      *   at obj+0x18. +0x10E4 = not-yet-pressed latch, +0x10E5 = render-once.
                      * To be SOLID and PRESSABLE we mirror the proven SpeedCylinder(v55n_2) fix:
                      *   1. Register obj+0x10E0 into board+0x10EC + scene tree (solid)
                      *   2. Set MeshBuffer+0x47C = entity on collision MeshBuffers so the
                      *      N:EXTRATIME handler (our DispatchCollisionEvents hook, replicated
                      *      from UpRaceCollisionEvents 0x4119B0) finds the entity
                      *   3. Set entity+0x47C self-ref
                      *   4. Track in g_timebuttons[] for collision dispatch.
                      * The button presses N:EXTRATIME -><- Rotator_TriggerSound (0x436CF0):
                      *   sets +0x10E4=1, +0x10E5=1, +0x10D8 -= 20.0f (sink 20 units), press sound
                      *   channel+0x510. Reward (single-player): timer slot = 500 + "EXTRA TIME:". */
                obj = pfn_operator_new(TIMEBUTTON_SIZE);
                if (!obj) { if (logf) fprintf(logf, "  ROTATER: failed to alloc TimeButton\n"); return; }
                memset(obj, 0, TIMEBUTTON_SIZE);
                /* v55n_22: REMOVED the pre-ctor cEnt_translate_meshworld_verts(mesh,...).
                 * v55n_22 added it and it CRASHED at 0001:000570A4 (EXE, Draw, fcomps
                 * 0x4(%edi)) — the v55n_15 signature. It writes the SOURCE mesh's +0x18
                 * tree items IN PLACE, corrupting a tree the board still walks during
                 * Draw. This is the SAME tree-corruption class as v55n_8/v55n_15.
                 * Solidity is handled by the PROVEN catapult pattern: the Present-driver
                 * calls cEnt_timebutton_translate_tree, which writes the BUILT collision
                 * Level's mw+0x18 from SAVED ORIGINALS (idempotent, never touches source). */
                pfn_TimeButton_ctor(obj, (void*)board, px, py, pz, mesh);
                /* v55n_22 ISOLATION TEST: ZERO registration.
                 * Prior builds 16-21 each had SOME registration active (MeshBuffer+0x47C,
                 * board+0x10EC+scene append, self-ref) and all crashed at spawn
                 * (0001:0004717E = EXE 0x44717E = ScoreDisplay dtor EIP corruption; module 1
                 * mislabeled as ntdll). SpeedCylinder does register and works, so registration
                 * alone isn't the cause — but we have NEVER tested the native ctor+mesh+vtable
                 * completely ALONE with zero mod interference. If THIS still crashes, the
                 * native ctor path itself is the trigger; if it stops, we re-add registration
                 * one piece at a time until the culprit appears. */
                *(float*)((char*)obj + 0x10D4) = px;
                *(float*)((char*)obj + 0x10D8) = py;
                *(float*)((char*)obj + 0x10DC) = pz;
                /* v55n_22: registration block REMOVED for isolation. */
                /* 4. Track for N:EXTRATIME collision dispatch */
                if (g_timebutton_count < MAX_TIMEBUTTONS) {
                    g_timebuttons[g_timebutton_count].obj = (DWORD)obj;
                    g_timebuttons[g_timebutton_count].board = board;
                    g_timebuttons[g_timebutton_count].x = px;
                    g_timebuttons[g_timebutton_count].y = py;
                    g_timebuttons[g_timebutton_count].z = pz;
                    g_timebuttons[g_timebutton_count].col_level = *(DWORD*)((char*)obj + 0x10E0);
                    g_timebuttons[g_timebutton_count].pressed = 0;
                    g_timebuttons[g_timebutton_count].geom_translated = 0;
                    g_timebuttons[g_timebutton_count].orig_vtable18 = 0; /* v55n_22: no private vtable hook */
                    g_timebutton_count++;
                }
                break;
            case 40: /* NeonPlatform_ctor — 6 params (this, board, x, y, z, mesh) — Rotator_ctor_t */
                obj = pfn_operator_new(NEONPLATFORM_SIZE);
                if (!obj) { if (logf) fprintf(logf, "  ROTATER: failed to alloc NeonPlatform\n"); return; }
                memset(obj, 0, NEONPLATFORM_SIZE);
                pfn_NeonPlatform_ctor(obj, (void*)board, px, py, pz, mesh);
                break;
            case 41: /* Trapdoor_ctor — 2 params (this, board) — no mesh or position params
                      * Reads mesh from board+0x878+0x594 (App mesh table).
                      * v53g-5: Guard against NULL App mesh table entry on non-native levels. */
                {
                    DWORD app = *(DWORD*)(board + BOARD_APP);
                    DWORD mesh_table = app ? *(DWORD*)(app + 0x878) : 0;
                    if (!mesh_table || IsBadReadPtr((void*)mesh_table, 0x5A4)) {
                        if (logf) fprintf(logf, "  ROTATER: Trapdoor — App mesh table NULL, skipping\n");
                        return;
                    }
                    DWORD trapdoor_mesh = *(DWORD*)(mesh_table + 0x594);
                    if (!trapdoor_mesh) {
                        if (logf) fprintf(logf, "  ROTATER: Trapdoor — mesh at App+0x878+0x594 is NULL, skipping\n");
                        return;
                    }
                }
                obj = pfn_operator_new(TRAPDOOR_SIZE);
                if (!obj) { if (logf) fprintf(logf, "  ROTATER: failed to alloc Trapdoor\n"); return; }
                memset(obj, 0, TRAPDOOR_SIZE);
                pfn_Trapdoor_ctor(obj, (void*)board);
                /* Position set after construction at +0x10D8/DC/E0 */
                *(float*)((char*)obj + 0x10D8) = px;
                *(float*)((char*)obj + 0x10DC) = py;
                *(float*)((char*)obj + 0x10E0) = pz;
                break;
            case 42: /* Odd_Lifter_ctor (Droplifter) — 5 params (this, board, x, y, z) — no mesh
                      * Reads mesh from board+0x878+0x5C8 (App mesh table).
                      * v53g-5: Guard against NULL App mesh table entry on non-native levels. */
                {
                    DWORD app = *(DWORD*)(board + BOARD_APP);
                    DWORD mesh_table = app ? *(DWORD*)(app + 0x878) : 0;
                    if (!mesh_table || IsBadReadPtr((void*)mesh_table, 0x5CC)) {
                        if (logf) fprintf(logf, "  ROTATER: Droplifter — App mesh table NULL, skipping\n");
                        return;
                    }
                    DWORD lifter_mesh = *(DWORD*)(mesh_table + 0x5C8);
                    if (!lifter_mesh) {
                        if (logf) fprintf(logf, "  ROTATER: Droplifter — mesh at App+0x878+0x5C8 is NULL, skipping\n");
                        return;
                    }
                }
                obj = pfn_operator_new(ODD_LIFTER_SIZE);
                if (!obj) { if (logf) fprintf(logf, "  ROTATER: failed to alloc Droplifter\n"); return; }
                memset(obj, 0, ODD_LIFTER_SIZE);
                pfn_Odd_Lifter_ctor(obj, (void*)board, px, py, pz);
                break;
            case 43: /* Gluebie_ctor — 3 params (this, board, mesh)
                     * v55c: Native Gluebie from Dizzy_CreateDynamicObjects.
                     * vtable[11] (0x43ECC0) handles rendering + animation.
                     * Proximity behavior (ball slowdown) is in DizzyBoard_Update
                     * which iterates board+0x4378 (Gluebie list, Dizzy-only).
                     * Position stored at obj+0x10D4/10D8/10DC (DWORD indices 0x435/436/437). */
                obj = pfn_operator_new(GLUEBIE_SIZE);
                if (!obj) { if (logf) fprintf(logf, "  ROTATER: failed to alloc Gluebie\n"); return; }
                memset(obj, 0, GLUEBIE_SIZE);
                pfn_Gluebie_ctor(obj, (void*)board, mesh);
                /* Position at obj+0x10D4/10D8/10DC (matching native puVar4[0x435/436/437]) */
                *(float*)((char*)obj + 0x10D4) = px;
                *(float*)((char*)obj + 0x10D8) = py;
                *(float*)((char*)obj + 0x10DC) = pz;
                /* v55j_7: Keep Gluebie SOLID (don't clear collision trees).
                 * Native Dizzy Gluebie is solid — ball rolls on its surface.
                 * The 0.95x velocity scaling gently slows the ball as it crosses.
                 * Clearing collision trees (previous approach) made the ball fall
                 * through and get trapped in the velocity zone → stuck + crash. */
                /* v55j_12: Add to Dizzy-specific list if on Dizzy.
                 * Do NOT add to board+0x6080 (generic list) — its +0x414
                 * sorted flag is non-zero, causing AthenaList_SortedInsert crash.
                 * The real fix for Gluebie on non-Dizzy levels is using our own
                 * g_gluebie_particles_created_ball flag instead of ball+0x2BC. */
                {
                    char* board_name = *(char**)((char*)board + 0x868);
                    if (board_name && !IsBadReadPtr(board_name, 12) &&
                        _strnicmp(board_name, "Board (Dizzy)", 13) == 0) {
                        pfn_AthenaList_Append((DWORD*)(board + 0x4378), obj);
                        if (logf) fprintf(logf, "  ROTATER: Gluebie added to Dizzy board+0x4378 (proximity list)\n");
                    } else {
                        if (logf) fprintf(logf, "  ROTATER: Gluebie on non-Dizzy level, using mod proximity check\n");
                    }
                }
                break;
            case 44: /* v55k_1: Tarpit — PopCylinder with _default mesh for visibility.
                     * Tar sinking behavior handled by cEnt_tarpit_proximity_check
                     * in the Present hook (main thread, every frame). */
                obj = pfn_operator_new(POPCYLINDER_SIZE);
                if (!obj) { if (logf) fprintf(logf, "  ROTATER: failed to alloc Tarpit\n"); return; }
                memset(obj, 0, POPCYLINDER_SIZE);
                {
                    /* Load _default mesh for visibility */
                    DWORD app = *(DWORD*)(board + BOARD_APP);
                    DWORD gfx_device = app ? *(DWORD*)(app + APP_GFX_DEVICE) : 0;
                    if (!gfx_device || !mesh) { if (logf) fprintf(logf, "  ROTATER: no gfx/mesh for Tarpit\n"); return; }
                    void* result44 = pfn_PopCylinder_ctor(obj, (void*)board, px, py, pz, mesh);
                    if (!result44) { if (logf) fprintf(logf, "  ROTATER: PopCylinder_ctor failed for Tarpit\n"); return; }
                }
                break;
            case 8:  /* GameLevel_ctor — Wobbly */
                obj = pfn_operator_new(GAMELEVEL_SIZE);
                if (!obj) { if (logf) fprintf(logf, "  ROTATER: failed to alloc GameLevel\n"); return; }
                memset(obj, 0, GAMELEVEL_SIZE);
                pfn_GameLevel_ctor(obj, (void*)board, px, py, pz, mesh);
                break;
            case 9:  /* Glass_Level_ctor — Drawbridge (3 params: this, board, mesh) */
                obj = pfn_operator_new(GLASS_LEVEL_SIZE);
                if (!obj) { if (logf) fprintf(logf, "  ROTATER: failed to alloc Glass_Level\n"); return; }
                memset(obj, 0, GLASS_LEVEL_SIZE);
                pfn_Glass_Level_ctor(obj, (void*)board, mesh);
                /* Position written after construction */
                *(float*)((char*)obj + 0x10D8) = px;
                *(float*)((char*)obj + 0x10DC) = py;
                *(float*)((char*)obj + 0x10E0) = pz;
                break;
            case 10: /* Gear_Level_ctor — Judge (5 params: this, board, x, y, z — no mesh!) */
                obj = pfn_operator_new(GEAR_LEVEL_SIZE);
                if (!obj) { if (logf) fprintf(logf, "  ROTATER: failed to alloc Gear_Level\n"); return; }
                memset(obj, 0, GEAR_LEVEL_SIZE);
                pfn_Gear_Level_ctor(obj, (void*)board, px, py, pz);
                break;
            case 11: /* Secret_ctor — GlassBonus (6 params: this, board, x, y, z, mesh) */
                obj = pfn_operator_new(SECRET_SIZE);
                if (!obj) { if (logf) fprintf(logf, "  ROTATER: failed to alloc Secret\n"); return; }
                memset(obj, 0, SECRET_SIZE);
                pfn_Secret_ctor(obj, (void*)board, px, py, pz, mesh);
                break;
            case 12: /* FlagWaver_Ctor — Flag (2 params: this, gfx_device — code-generated mesh) */
                obj = pfn_operator_new(FLAGWAVER_SIZE);
                if (!obj) { if (logf) fprintf(logf, "  ROTATER: failed to alloc FlagWaver\n"); return; }
                memset(obj, 0, FLAGWAVER_SIZE);
                {
                    DWORD app = *(DWORD*)(board + BOARD_APP);
                    DWORD gfx_device = app ? *(DWORD*)(app + APP_GFX_DEVICE) : 0;
                    if (!gfx_device) { if (logf) fprintf(logf, "  ROTATER: no gfx_device for FlagWaver\n"); return; }
                    pfn_FlagWaver_Ctor(obj, (void*)gfx_device);
                }
                break;
            case 13: /* Sign_ctor — Popup Sign (complex signature) */
                obj = pfn_operator_new(SIGN_SIZE);
                if (!obj) { if (logf) fprintf(logf, "  ROTATER: failed to alloc Sign\n"); return; }
                memset(obj, 0, SIGN_SIZE);
                {
                    DWORD app = *(DWORD*)(board + BOARD_APP);
                    int gfx1 = app ? *(int*)(app + 0x58C) : 0;
                    int gfx2 = app ? *(int*)(app + 0x590) : 0;
                    pfn_Sign_ctor(obj, (void*)board, gfx1, gfx2,
                        *(int*)&px, *(int*)&py, *(int*)&pz, 0, 0, 0);
                }
                break;
            case 14: /* WavyFlag2 — Wavy_ctor copy with custom mesh path.
                      * Wavy_ctor takes a string path (not mesh pointer!) and loads
                      * the mesh internally. Try "levels\Flag", fall back to _default. */
                obj = pfn_operator_new(WAVY_SIZE);
                if (!obj) { if (logf) fprintf(logf, "  ROTATER: failed to alloc WavyFlag2\n"); return; }
                memset(obj, 0, WAVY_SIZE);
                {
                    /* Check if Flag.MESHWORLD exists, else use _default */
                    const char* wavy_path = "levels\\Flag";
                    char check_path[256];
                    snprintf(check_path, 255, "%s.meshworld", wavy_path);
                    if (pfn_check_file_access(check_path, 0) != 0) {
                        /* Flag.MESHWORLD not found — try _default */
                        wavy_path = "levels\\_default";
                        if (logf) fprintf(logf, "  ROTATER: Flag.MESHWORLD not found, using _default\n");
                    }
                    pfn_Wavy_ctor(obj, (void*)board, px, py, pz, wavy_path);
                    /* Configure wave parameters (same as native Wavy1) */
                    pfn_Wavy_Configure(obj, 0x1c, 20.0f, 2.0f, -3.0f);
                }
                break;
            case 15: /* BadBall_ctor — 8ball/BadBall (2 params: this, board)
                      * Position is set after construction at ball+0xC60/0xC64/0xC68
                      * (home position). Mesh is rendered by Ball_Render which reads
                      * from App+0x244[ball+0x754 * 4] — NOT from ball+0x10.
                      * ball+0x10 is the App/parent pointer, not a mesh pointer!
                      *
                      * v54 FIX: Copy 8Ball mesh ptr from App+0x268 to App+0x248
                      * (mesh slot 1), then set ball+0x754=1 so Ball_Render renders
                      * the 8Ball mesh instead of the default Sphere mesh.
                      * This is the same pattern as cEnt_process_custom_tags. */
                obj = pfn_operator_new(BADBALL_SIZE);
                if (!obj) { if (logf) fprintf(logf, "  ROTATER: failed to alloc BadBall\n"); return; }
                memset(obj, 0, BADBALL_SIZE);
                pfn_BadBall_ctor(obj, (void*)board);
                /* Call vtable[1] — Ball_SetupCollisionRender + Ball_SetTrajectory */
                {
                    DWORD* vtable = *(DWORD**)obj;
                    if (vtable && vtable[1]) {
                        typedef void (__thiscall *vtable1_t)(void* this_);
                        ((vtable1_t)vtable[1])(obj);
                    }
                }
                /* v54: Set mesh index to 8Ball.
                 * Copy 8Ball mesh pointer from App+0x268 to App+0x248 (slot 1).
                 * Set ball+0x754 = 1 so Ball_Render renders the 8Ball mesh. */
                {
                    DWORD app = *(DWORD*)(board + BOARD_APP);
                    if (app && !IsBadReadPtr((void*)(app + 0x268), 4)) {
                        DWORD mesh_8ball = *(DWORD*)(app + APP_MESH_8BALL);
                        if (mesh_8ball) {
                            /* Copy to slot 1 (SphereBreak1 position) */
                            *(DWORD*)(app + APP_MESH_ARRAY + 4) = mesh_8ball;
                            /* Set ball mesh index to 1 */
                            *(DWORD*)((char*)obj + BALL_MESH_INDEX_FIELD) = MESH_SLOT_8BALL;
                            if (logf) fprintf(logf, "  ROTATER: 8ball mesh set (App+0x248=0x%08X, ball+0x754=1)\n", mesh_8ball);
                        }
                    }
                }
                /* Set home position (ball+0xC60/C64/C68) */
                *(float*)((char*)obj + BALL_HOME_POS_X) = px;
                *(float*)((char*)obj + BALL_HOME_POS_Y) = py;
                *(float*)((char*)obj + BALL_HOME_POS_Z) = pz;
                break;
            case 16: /* Bridgeslam — isolated Intermediate bridge state machine.
                      * No game _ctor. Does its own init:
                      *   1. Load Level2-Bridge mesh via MeshWorld_ctor
                      *   2. Create render object via Level_RenderCtor
                      *   3. TipperVisual_Attach links them
                      *   4. Init state: angle=45.0, state=0, counter=50
                      * Per-frame update runs from cEnt_bridgeslam_update() in the thread. */
                {
                    DWORD app = *(DWORD*)(board + BOARD_APP);
                    if (!app || IsBadReadPtr((void*)app, 4)) { if (logf) fprintf(logf, "  ROTATER: no app for Bridgeslam\n"); return; }
                    DWORD gfx_device = *(DWORD*)(app + APP_GFX_DEVICE);
                    if (!gfx_device) { if (logf) fprintf(logf, "  ROTATER: no gfx_device for Bridgeslam\n"); return; }

                    /* Load bridge mesh */
                    const char* bridge_path = mesh_path && mesh_path[0] ? mesh_path : "levels\\Level2-Bridge";
                    void* mesh = cEnt_load_mesh_file(gfx_device, bridge_path, &is_mesh_node, logf);
                    if (!mesh) {
                        if (logf) fprintf(logf, "  ROTATER: Bridgeslam mesh load failed, trying Swirl fallback\n");
                        mesh = cEnt_load_mesh_file(gfx_device, g_swirl_mesh_path, &is_mesh_node, logf);
                        if (!mesh) { if (logf) fprintf(logf, "  ROTATER: Bridgeslam Swirl fallback failed\n"); return; }
                    }

                    /* Create render object from mesh */
                    void* render_obj = pfn_operator_new(BRIDGESLAM_SIZE);
                    if (render_obj) {
                        memset(render_obj, 0, BRIDGESLAM_SIZE);
                        render_obj = pfn_Level_RenderCtor(render_obj, mesh);
                    }

                    /* TipperVisual_Attach links render to mesh */
                    if (render_obj) {
                        pfn_TipperVisual_Attach(render_obj, (int)mesh);
                    }

                    /* Allocate a dummy object for board list registration */
                    obj = pfn_operator_new(BRIDGESLAM_SIZE);
                    if (!obj) { if (logf) fprintf(logf, "  ROTATER: failed to alloc Bridgeslam obj\n"); return; }
                    memset(obj, 0, BRIDGESLAM_SIZE);

                    /* Register on board lists */
                    pfn_AthenaList_Append((DWORD*)(board + BOARD_UPDATE_LIST), obj);
                    pfn_AthenaList_Append((DWORD*)(board + BOARD_RENDER_LIST), render_obj);

                    /* Track in bridgeslam state array */
                    if (g_bridgeslam_count < MAX_BRIDGESLAMS) {
                        BridgeslamState* bs = &g_bridgeslams[g_bridgeslam_count];
                        bs->obj = (DWORD)obj;
                        bs->render_obj = (DWORD)render_obj;
                        bs->mesh_world = (DWORD)mesh;
                        bs->board = board;
                        bs->pivot_x = px;
                        bs->pivot_y = py;
                        bs->pivot_z = pz;
                        bs->angle = 45.0f;
                        bs->state = 0;
                        bs->counter = 50;  /* 0x32 */
                        bs->active = 1;
                        g_bridgeslam_count++;
                        if (logf) fprintf(logf, "  ROTATER: Bridgeslam spawned at (%.1f,%.1f,%.1f) obj=0x%08X render=0x%08X mesh=0x%08X\n",
                                px, py, pz, (DWORD)obj, (DWORD)render_obj, (DWORD)mesh);
                    }
                }
                break;
        }
    } else {
        /* AI 0-5, 23-26: Use the correct constructor per AI type.
         * Types 23-26 (Chrome, Funball, Tarbubble, Waterwheel) have no _ctor
         * — they use PopCylinder fallback (board-level behavior only). */
        Rotator_ctor_t ctor_fn = NULL;
        DWORD alloc_sz = 0;
        
        switch (ai_type) {
            case 1:  ctor_fn = pfn_Rotator_ctor;  alloc_sz = ROTATER_SIZE;   break; /* Rotator (Y-axis) */
            case 2:  ctor_fn = pfn_Pendulum_ctor; alloc_sz = PENDULUM_SIZE;  break; /* Pendulum (X-axis) */
            case 3:  ctor_fn = pfn_Looper_ctor;   alloc_sz = LOOPER_SIZE;    break; /* Looper (Z-axis) */
            case 4:  ctor_fn = pfn_Gear_ctor;      alloc_sz = GEAR_SIZE;      break; /* Gear Small */
            case 5:  ctor_fn = pfn_Gear_ctor;      alloc_sz = GEAR_SIZE;      break; /* Gear Big */
            default: /* AI 0: static PopCylinder */
                obj = pfn_operator_new(POPCYLINDER_SIZE);
                if (!obj) { if (logf) fprintf(logf, "  ROTATER: failed to alloc\n"); return; }
                memset(obj, 0, POPCYLINDER_SIZE);
                void* result0 = pfn_PopCylinder_ctor(obj, (void*)board, px, py, pz, mesh);
                if (!result0) { if (logf) fprintf(logf, "  ROTATER: PopCylinder_ctor failed\n"); return; }
                goto spawn_done;
        }
        
        obj = pfn_operator_new(alloc_sz);
        if (!obj) { if (logf) fprintf(logf, "  ROTATER: failed to alloc (%d bytes)\n", alloc_sz); return; }
        memset(obj, 0, alloc_sz);
        void* result = ctor_fn(obj, (void*)board, px, py, pz, mesh);
        if (!result) { if (logf) fprintf(logf, "  ROTATER: ctor failed for AI %d\n", ai_type); return; }
        /* v55m_48d: Reset the initial rotation angle to 0.0 for the
         * Rotator family (AI 1-5). The native SWIRL ctor writes -0.2 in
         * race mode; these Rotator_ctor variants write 0/+1 but we keep
         * the angle at 0.0 for a consistent, unrotated start. */
        if (ai_type >= 1 && ai_type <= 6) {
            *(float*)(obj + 0x10E8) = 0.0f;
        }
    }
    spawn_done:;

    /* 5. Add to board+0x2578 (update list) */
    pfn_AthenaList_Append((DWORD*)(board + BOARD_UPDATE_LIST), obj);

    /* 5a. Type 8 (GameLevel/Wobbly): also add to Scene_Update list (board+0x8B8).
     * Scene_Update iterates this list and calls vtable[1] (Rotator_Update)
     * which performs vertex deformation = the wobble animation.
     * Only type 8 is safe here because it loads its own MESHWORLD with
     * valid vertex data. Other types would crash in Rotator_Update. */
    if (ai_type == 8 || ai_type == 34) {
        pfn_AthenaList_Append((DWORD*)(board + BOARD_SCENE_UPDATE_LIST), obj);
    }

    /* 5b. BadBall also goes into the bad balls list (board+0x29D4) */
    if (ai_type == 15) {
        pfn_AthenaList_Append((DWORD*)(board + BOARD_BAD_BALLS_LIST), obj);
    }

    /* 6. Add to board+0xCD4 (render list). */
    pfn_AthenaList_Append((DWORD*)(board + BOARD_RENDER_LIST), obj);

    /* 7. Add collision object to board+0x10EC.
     * PopCylinder(0) and Rotator(1-6) have collision objects at +0x10D4.
     * Catapult(35): collision is via spatial trees at obj+0x18 (cloned by
     *   Stands_ctor). The render Level at +0x10D4 is for rendering only —
     *   do NOT add it to the collision list (it doesn't have spatial trees).
     * ArenaStands family stores position FLOATS at +0x10D4/+0x10E0 — reading them
     *   as pointers corrupts collision lists → crash in SpatialTree_ctor (0x46333F).
     * Stands_ctor family (35-42) stores position floats at +0x10E0 — same crash.
     * WavyFlag(14) stores position float at +0x10D4 — same crash.
     * Level-family (8,9,10,27,29) also don't have collision objects at these offsets.
     * Fix: col_off=0x10D4 for PopCylinder and Rotator only. */
    {
        DWORD col_off = 0;  /* default: no collision object */
        if (ai_type >= 1 && ai_type <= 6) col_off = 0x10D4;  /* Rotator family */
        /* Catapult(35): collision via obj+0x18 spatial trees, NOT collision list */
        /* All other types: col_off = 0 (no collision object added) */
        DWORD col_obj = *(DWORD*)((char*)obj + col_off);
        if (col_obj && col_off > 0) {
            pfn_AthenaList_Append((DWORD*)(board + BOARD_COLLISION_LIST), (void*)col_obj);

            /* Also add to board+0x8B0+0x18 (scene collision) */
            DWORD scene_col = *(DWORD*)(board + BOARD_SCENE_OBJ);
            if (scene_col) {
                pfn_AthenaList_Append((DWORD*)(scene_col + 0x18), (void*)col_obj);
            }
        }
    }

    /* 8. Add to scene spatial tree (board+0x8AC+0x480+0x1C) */
    {
        DWORD level = cEnt_get_level(board);
        if (level) {
            DWORD sceneobj = *(DWORD*)(level + LEVEL_SCENEOBJECT);
            if (sceneobj) {
                pfn_AthenaList_Append((DWORD*)(sceneobj + 0x1C), obj);
            }
        }
    }

    if (logf) {
        fprintf(logf, "  ROTATER: spawned at (%.1f,%.1f,%.1f) obj=0x%08X mesh='%s' rot=(%.4f,%.4f,%.4f) oc=(%.1f,%.1f,%.1f)\n",
                px, py, pz, (DWORD)obj, path, rot_x, rot_y, rot_z,
                ros_x, ros_y, ros_z);
        fflush(logf);
    }

    /* v55m_48d: snapshot the CONSTRUCTOR-time values once. The constructor
     * assigns a STATIC initial angle (obj+0x10E8) and direction
     * (obj+0x10EC) at spawn. These are frozen here — the per-frame native
     * render later mutates them, but the ctor section must show the static
     * initial values, not the live ones. */
    if (obj && obj >= 0x10000 && !IsBadReadPtr((void*)obj, 0x10F0)) {
        g_dbg_ctor_angle = *(float*)(obj + 0x10E8);
        g_dbg_ctor_direction = *(float*)(obj + 0x10EC);
        if (logf) fprintf(logf, "  ROTATER: ctor snapshot angle=%.2f dir=%.2f\\n",
                g_dbg_ctor_angle, g_dbg_ctor_direction);
    }

    /* Track for despawn + per-frame rotation updates.
     * Skip Level-family entities (30-33: Bell, Fan, SawBlade, Bonk) —
     * they manage their own lifecycle and should NOT be despawned here. */
    if (ai_type >= 30 && ai_type <= 33) return;

    /* v55c: Register Gluebies for cross-level proximity behavior */
    if (ai_type == 43 && g_gluebie_count < MAX_GLUEBIES) {
        g_gluebie_objs[g_gluebie_count] = (DWORD)obj;
        g_gluebie_count++;
        if (logf) fprintf(logf, "  GLUEBIE: registered obj=%p in g_gluebie_objs[%d] (count=%d)\n",
            (void*)obj, g_gluebie_count-1, g_gluebie_count);
    }

    /* v55k_1: Register Tarpits for cross-level tar-sinking behavior */
    if (ai_type == 44 && g_tarpit_count < MAX_TARPITS) {
        g_tarpit_objs[g_tarpit_count] = (DWORD)obj;
        g_tarpit_count++;
        if (logf) fprintf(logf, "  TARPIT: registered obj=%p in g_tarpit_objs[%d] (count=%d)\n",
            (void*)obj, g_tarpit_count-1, g_tarpit_count);
    }

    /* v55m_26: Register Chompers for jaw animation via vtable[18] hook.
     * Re-enabled with FIXED Timer pointers + Gfx_Scale call.
     * The v55m_25 EntityTransform approach didn't work because PopCylinder's
     * vtable[18] (D3DXSkinMesh_CopyStripData) doesn't read EntityTransform.
     * It reads the D3D world matrix set by Gfx_Scale/Gfx_ScaleZ. */
    if (mesh_path && _stricmp(mesh_path, "levels\\Chomper") == 0 &&
        g_chomper_count < MAX_CHOMPERS) {
        ChomperState* cs = &g_chompers[g_chomper_count];
        cs->obj = (DWORD)obj;
        cs->x = px;
        cs->y = py - 20.0f;
        cs->z = pz;
        cs->jaw_angle = 0.25f;
        cs->phase = 0.0f;
        cs->state = 0;
        cs->countdown = 0;
        cs->anim_val = 0.0f;

        /* Create private vtable copy — v55m_44e: must cover the FULL
         * PopCylinder vtable (168 entries = 672 bytes, vtable 0x4D58F0).
         * Old 256-byte copy truncated at 64 entries → game reached slots
         * beyond 64 during level load → garbage EIP → crash 0x40587E7.
         * Same fix as catapult (0x400B). */
        DWORD orig_vtable = *(DWORD*)obj;
        if (orig_vtable && !IsBadReadPtr((void*)orig_vtable, 0x400)) {
            DWORD* new_vtable = (DWORD*)pfn_operator_new(0x400);
            if (new_vtable) {
                memcpy(new_vtable, (void*)orig_vtable, 0x400);
                /* Save original vtable[18] (offset 0x48) */
                cs->orig_vtable2 = new_vtable[18];
                /* Override vtable[18] with our hook */
                new_vtable[18] = (DWORD)&cEnt_chomper_render;
                /* Replace object's vtable pointer */
                *(DWORD*)obj = (DWORD)new_vtable;
            }
        }

        /* v55m_25b: Detailed registration logging */
        DWORD meshworld = *(DWORD*)((char*)obj + 0x08);
        DWORD obj_vtable = *(DWORD*)obj;
        if (logf) fprintf(logf, "  CHOMPER[%d]: registered obj=0x%08X pos=(%.1f,%.1f,%.1f) jaw=0.25\n",
                g_chomper_count, (DWORD)obj, cs->x, cs->y, cs->z);
        if (logf) fprintf(logf, "  CHOMPER[%d]: vtable=0x%08X meshworld=0x%08X (obj+0x08)\n",
                g_chomper_count, obj_vtable, meshworld);
        if (logf) fprintf(logf, "  CHOMPER[%d]: vtable[18] hooked: orig=0x%08X new=0x%08X\n",
                g_chomper_count, cs->orig_vtable2, (DWORD)&cEnt_chomper_render);

        g_chomper_count++;
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
            strncpy(g_rotater_cfg[g_rotater_count].mesh_path, mesh_path, 127);
            g_rotater_cfg[g_rotater_count].mesh_path[127] = 0;
        } else {
            g_rotater_cfg[g_rotater_count].mesh_path[0] = 0;
        }
        g_rotater_count++;
    }
}

/* Despawn all rotater objects — calls vtable[11] (RemoveAndFree) on each */
static void cEnt_despawn_all_rotaters(DWORD board, FILE* logf) {
    int i;
    for (i = 0; i < g_rotater_count; i++) {
        DWORD obj = g_rotater_cfg[i].obj;
        if (!obj || obj < 0x10000) continue;
        if (IsBadReadPtr((void*)obj, 0x10D0)) continue;

        /* Call vtable[11] (offset 0x2C) = RemoveAndFree */
        DWORD vtable = *(DWORD*)obj;
        if (vtable && !IsBadReadPtr((void*)vtable, 0x30)) {
            DWORD remove_fn = *(DWORD*)(vtable + 0x2C);
            if (remove_fn && remove_fn > 0x400000) {
                typedef void (__thiscall *remove_t)(void* this_);
                ((remove_t)remove_fn)((void*)obj);
                if (logf) fprintf(logf, "  ROTATER: removed obj=0x%08X\n", obj);
            }
        }
    }
    g_rotater_count = 0;
    g_gluebie_count = 0;  /* v55c: reset Gluebie tracking on level unload */
    g_tarpit_count = 0;   /* v55k_1: reset Tarpit tracking on level unload */
    g_speedcyl_count = 0; /* v55n_2: reset SpeedCylinder tracking on level unload */
    /* v55n_5: TimeButton despawn. The TimeButton is mod-allocated and is NOT in
     * board+0x2578 (unlike native), so the game's teardown never calls its
     * RemoveAndFree/dtor. We own it. Clean it up exactly like native
     * Rotator_RemoveAndFree (0x436FC0): with +0x10E5==0 (set at spawn) it
     * removes +0x10E0 from board+0x8B0+0x18 + board+0x10EC and frees it, then
     * we free obj via its dtor 0x43DC20. Without this the +0x10E0 Level stays
     * registered in board+0x10EC + scene tree and the game's quit teardown
     * double-frees it -> ntdll crash. */
    if (g_timebutton_count > 0) {
        int ti;
        for (ti = 0; ti < g_timebutton_count; ti++) {
            DWORD tobj = g_timebuttons[ti].obj;
            if (!tobj || tobj < 0x10000) continue;
            if (IsBadReadPtr((void*)tobj, 0x10E8)) continue;
            DWORD tvt = *(DWORD*)tobj;
            if (tvt && !IsBadReadPtr((void*)tvt, 0x10)) {
                /* a) Rotator_RemoveAndFree (0x436FC0) — needs +0x10E5==0 */
                DWORD raf = 0x00436FC0;
                if (*(char*)((char*)tobj + 0x10E5) == 0) {
                    typedef void (__thiscall *raf_t)(void* this_);
                    ((raf_t)raf)((void*)tobj);
                }
                /* b) free obj via its dtor 0x43DC20 (vtable[0]) */
                DWORD dtor = *(DWORD*)tvt;
                if (dtor && dtor > 0x400000) {
                    typedef void (__thiscall *dtor_t)(void* this_, int free_mem);
                    ((dtor_t)dtor)((void*)tobj, 1);
                }
                if (logf) fprintf(logf, "  ROTATER: despawned TimeButton obj=0x%08X\\n", tobj);
            }
        }
    }
    g_timebutton_count = 0; /* v55n_3: reset TimeButton tracking on level unload */
    g_gluebie_particles_created_ball = 0;  /* v55j_12: reset particle flag */
    /* v55n: Free any live decorative TarBubble objects on level unload
     * (they self-free on pop, but must not leak across level changes). */
    {
        int ti;
        for (ti = 0; ti < g_tarbubble_count; ti++) {
            if (g_tarbubble_pos[ti].obj) {
                if (pfn_TarBubble_dtor) {
                    pfn_TarBubble_dtor(g_tarbubble_pos[ti].obj, 1);
                }
                g_tarbubble_pos[ti].obj = 0;
            }
        }
    }
    g_tarbubble_count = 0;  /* v55e: reset TarBubble tracking on level unload */
    /* v55m_44o: WaterWheel cleanup — the mesh is registered in the render
     * list + scene tree and has a private vtable copy (hooked [18]).
     * Restore the original vtable, remove from lists. The mesh object
     * itself is freed by the engine's level teardown (it was operator_new'd
     * but the game's own MeshWorld teardown handles it — the private vtable
     * copy is ours and must be restored first so the engine calls the real
     * vtable functions, not our hook, during teardown). */
    {
        int wi;
        for (wi = 0; wi < g_waterwheel_count; wi++) {
            struct WaterWheelState* ww = &g_waterwheels[wi];
            if (!ww->active || !ww->pc_obj) continue;
            if (ww->orig_vtable18) {
                /* The mesh's vtable is the private copy; restore original
                 * 0x4D8FB0 so engine teardown uses the real functions. */
                DWORD priv_vtbl = *(DWORD*)ww->pc_obj;
                if (priv_vtbl && priv_vtbl > 0x400000 &&
                    !IsBadReadPtr((void*)priv_vtbl, 0x400) &&
                    ww->orig_vtable18) {
                    /* find 0x4D8FB0 as the original — restore slot 18 only */
                    DWORD* priv = (DWORD*)priv_vtbl;
                    priv[18] = ww->orig_vtable18;
                }
            }
            ww->active = 0;
        }
    }
    g_waterwheel_count = 0;  /* v55f: reset WaterWheel tracking on level unload */
    g_catapult_count = 0;  /* v55d: reset Catapult tracking on level unload */
    g_chomper_count = 0;  /* v55m_3: reset Chomper tracking on level unload */

    /* v55m_42q: Arm objects are separate PopCylinder objects. They are
     * registered in the rotater_cfg list via the spawn_done path, so the
     * main rotater loop above will call RemoveAndFree on them automatically.
     * The CatapultState array is zeroed by g_catapult_count=0. */

    /* v55m_42f: free BASS dropin sample on level unload to avoid leak */
    if (g_dropin_sample && real_BASS_SampleFree) {
        real_BASS_SampleFree(g_dropin_sample);
        g_dropin_sample = 0;
    }
    /* v55m_42i: free BASS catapult sample on level unload */
    if (g_catapult_sample && real_BASS_SampleFree) {
        real_BASS_SampleFree(g_catapult_sample);
        g_catapult_sample = 0;
    }

    /* Clear Bonk tracking and uninstall collision hook */
    g_bonk_count = 0;
    uninstall_bonk_collision_hook();
}

/* v55m_42fb: load dropin sound via BASS directly. Called once per level when a
 * Catapult is spawned. Tries .ogg, .wav, .mp3 extensions. Logs BASS error codes.
 * v55m_42i: also loads sounds\\Catapult.ogg for the 50-frame launch sound. */
static void cEnt_load_dropin_sample(FILE* logfile) {
    FILE* lf = logfile;
    if (!lf) lf = fopen("mknp_custom_entities_catapult.log", "a");
    if (!real_BASS_SampleLoad) {
        if (lf) fprintf(lf, "CATAPULT: BASS_SampleLoad not available\n");
        if (lf != logfile) fclose(lf);
        return;
    }
    const char* exts[] = { ".ogg", ".wav", ".mp3" };
    char path[MAX_PATH];

    if (!g_dropin_sample) {
        for (int i = 0; i < 3; i++) {
            snprintf(path, MAX_PATH, "%s\\sounds\\dropin%s", g_game_dir, exts[i]);
            g_dropin_sample = real_BASS_SampleLoad(FALSE, path, 0, 0, 3, 0);
            int err = 0;
            if (real_BASS_ErrorGetCode) err = real_BASS_ErrorGetCode();
            if (lf) fprintf(lf, "CATAPULT: BASS_SampleLoad('%s') handle=0x%08lX err=%d\n", path, g_dropin_sample, err);
            if (g_dropin_sample) {
                if (lf) fprintf(lf, "CATAPULT: loaded dropin sample from %s\n", path);
                break;
            }
        }
    }
    if (!g_dropin_sample && lf) fprintf(lf, "CATAPULT: failed to load dropin sample from %s\\sounds\\dropin\n", g_game_dir);

    if (!g_catapult_sample) {
        for (int i = 0; i < 3; i++) {
            snprintf(path, MAX_PATH, "%s\\sounds\\Catapult%s", g_game_dir, exts[i]);
            g_catapult_sample = real_BASS_SampleLoad(FALSE, path, 0, 0, 3, 0);
            int err = 0;
            if (real_BASS_ErrorGetCode) err = real_BASS_ErrorGetCode();
            if (lf) fprintf(lf, "CATAPULT: BASS_SampleLoad('%s') handle=0x%08lX err=%d\n", path, g_catapult_sample, err);
            if (g_catapult_sample) {
                if (lf) fprintf(lf, "CATAPULT: loaded catapult sample from %s\n", path);
                break;
            }
        }
    }
    if (!g_catapult_sample && lf) fprintf(lf, "CATAPULT: failed to load catapult sample from %s\\sounds\\Catapult\n", g_game_dir);

    if (lf != logfile) fclose(lf);
}

/* v55m_42f: play the loaded dropin sound via BASS directly.
 * Uses older one-shot BASS_SamplePlay if available, else SampleGetChannel+ChannelPlay,
 * else StreamCreateFile+StreamPlay. */
static void cEnt_play_dropin_sound(FILE* logfile) {
    FILE* lf = logfile;
    if (!lf) lf = fopen("mknp_custom_entities_catapult.log", "a");
    if (!g_dropin_sample) {
        if (lf) fprintf(lf, "CATAPULT: no dropin sample loaded\n");
        if (lf != logfile) fclose(lf);
        return;
    }

    /* Primary: old one-shot BASS_SamplePlay */
    if (real_BASS_SamplePlay) {
        DWORD ch = real_BASS_SamplePlay(g_dropin_sample);
        int err = real_BASS_ErrorGetCode ? real_BASS_ErrorGetCode() : -1;
        if (lf) fprintf(lf, "CATAPULT: BASS_SamplePlay(0x%08lX) ch=0x%08lX err=%d\n", g_dropin_sample, ch, err);
        if (ch) {
            if (lf != logfile) fclose(lf);
            return;
        }
    }

    /* Secondary: SampleGetChannel + ChannelPlay */
    if (real_BASS_SampleGetChannel && real_BASS_ChannelPlay) {
        DWORD ch = real_BASS_SampleGetChannel(g_dropin_sample, FALSE);
        int err1 = real_BASS_ErrorGetCode ? real_BASS_ErrorGetCode() : -1;
        if (lf) fprintf(lf, "CATAPULT: BASS_SampleGetChannel(0x%08lX, FALSE) ch=0x%08lX err=%d\n",
            g_dropin_sample, ch, err1);
        if (ch) {
            int ret = real_BASS_ChannelPlay(ch, FALSE);
            int err2 = real_BASS_ErrorGetCode ? real_BASS_ErrorGetCode() : -1;
            if (lf) fprintf(lf, "CATAPULT: BASS_ChannelPlay(0x%08lX) ret=%d err=%d\n", ch, ret, err2);
            if (ret) {
                if (lf != logfile) fclose(lf);
                return;
            }
        }
    }

    /* Tertiary: create a stream from file and play it directly */
    if (real_BASS_StreamCreateFile && real_BASS_StreamPlay) {
        const char* exts[] = { ".ogg", ".wav", ".mp3" };
        char path[MAX_PATH];
        for (int i = 0; i < 3; i++) {
            snprintf(path, MAX_PATH, "%s\\sounds\\dropin%s", g_game_dir, exts[i]);
            DWORD stream = real_BASS_StreamCreateFile(FALSE, path, 0, 0, 0);
            int err = real_BASS_ErrorGetCode ? real_BASS_ErrorGetCode() : -1;
            if (lf) fprintf(lf, "CATAPULT: BASS_StreamCreateFile('%s') stream=0x%08lX err=%d\n", path, stream, err);
            if (stream) {
                DWORD ch = real_BASS_StreamPlay(stream, FALSE, 0);
                int err2 = real_BASS_ErrorGetCode ? real_BASS_ErrorGetCode() : -1;
                if (lf) fprintf(lf, "CATAPULT: BASS_StreamPlay(0x%08lX) ch=0x%08lX err=%d\n", stream, ch, err2);
                if (ch) {
                    if (lf != logfile) fclose(lf);
                    return;
                }
            }
        }
    }

    if (lf) fprintf(lf, "CATAPULT: all BASS play methods failed\n");
    if (lf != logfile) fclose(lf);
}

/* v55m_42i: play the loaded catapult launch sound via BASS directly.
 * Same fallbacks as cEnt_play_dropin_sound. */
static void cEnt_play_catapult_sound(FILE* logfile) {
    FILE* lf = logfile;
    if (!lf) lf = fopen("mknp_custom_entities_catapult.log", "a");
    if (!g_catapult_sample) {
        if (lf) fprintf(lf, "CATAPULT: no catapult sample loaded\n");
        if (lf != logfile) fclose(lf);
        return;
    }

    /* Primary: old one-shot BASS_SamplePlay */
    if (real_BASS_SamplePlay) {
        DWORD ch = real_BASS_SamplePlay(g_catapult_sample);
        int err = real_BASS_ErrorGetCode ? real_BASS_ErrorGetCode() : -1;
        if (lf) fprintf(lf, "CATAPULT: BASS_SamplePlay(catapult 0x%08lX) ch=0x%08lX err=%d\n", g_catapult_sample, ch, err);
        if (ch) {
            if (lf != logfile) fclose(lf);
            return;
        }
    }

    /* Secondary: SampleGetChannel + ChannelPlay */
    if (real_BASS_SampleGetChannel && real_BASS_ChannelPlay) {
        DWORD ch = real_BASS_SampleGetChannel(g_catapult_sample, FALSE);
        int err1 = real_BASS_ErrorGetCode ? real_BASS_ErrorGetCode() : -1;
        if (lf) fprintf(lf, "CATAPULT: BASS_SampleGetChannel(catapult 0x%08lX, FALSE) ch=0x%08lX err=%d\n",
            g_catapult_sample, ch, err1);
        if (ch) {
            int ret = real_BASS_ChannelPlay(ch, FALSE);
            int err2 = real_BASS_ErrorGetCode ? real_BASS_ErrorGetCode() : -1;
            if (lf) fprintf(lf, "CATAPULT: BASS_ChannelPlay(0x%08lX) ret=%d err=%d\n", ch, ret, err2);
            if (ret) {
                if (lf != logfile) fclose(lf);
                return;
            }
        }
    }

    /* Tertiary: create a stream from file and play it directly */
    if (real_BASS_StreamCreateFile && real_BASS_StreamPlay) {
        const char* exts[] = { ".ogg", ".wav", ".mp3" };
        char path[MAX_PATH];
        for (int i = 0; i < 3; i++) {
            snprintf(path, MAX_PATH, "%s\\sounds\\Catapult%s", g_game_dir, exts[i]);
            DWORD stream = real_BASS_StreamCreateFile(FALSE, path, 0, 0, 0);
            int err = real_BASS_ErrorGetCode ? real_BASS_ErrorGetCode() : -1;
            if (lf) fprintf(lf, "CATAPULT: BASS_StreamCreateFile('%s') stream=0x%08lX err=%d\n", path, stream, err);
            if (stream) {
                DWORD ch = real_BASS_StreamPlay(stream, FALSE, 0);
                int err2 = real_BASS_ErrorGetCode ? real_BASS_ErrorGetCode() : -1;
                if (lf) fprintf(lf, "CATAPULT: BASS_StreamPlay(0x%08lX) ch=0x%08lX err=%d\n", stream, ch, err2);
                if (ch) {
                    if (lf != logfile) fclose(lf);
                    return;
                }
            }
        }
    }

    if (lf) fprintf(lf, "CATAPULT: all BASS catapult play methods failed\n");
    if (lf != logfile) fclose(lf);
}
/* Apply rotation direction and oscillation limits to spawned custom_obj objects.
 * Called once at spawn time. Writes ROT_Y to obj+0x10EC (direction field).
 *
 * The native render function (vtable[11] at 0x0043B330) does:
 *   new_angle = direction * 0.004 + angle
 *   obj[0x10E8] = new_angle   (stores back!)
 *   if new_angle > 2.0:  direction = -1.0  (reverse)
 *   if new_angle < -2.0: direction = +1.0  (reverse)
 *
 * ROT_Y is written to the direction field as a speed multiplier.
 * ROS_Y (oscillation range) is stored in config — the native render
 * uses hardcoded ±2.0, so per-object OC requires a per-frame hook to
 * override the direction flip when the custom OC limit is reached.
 * For now, ROS_Y is stored but the native ±2.0 limit applies. */
static void cEnt_apply_rotater_directions(void) {
    int i;
    for (i = 0; i < g_rotater_count; i++) {
        DWORD obj = g_rotater_cfg[i].obj;
        if (!obj || obj < 0x10000) continue;
        if (IsBadReadPtr((void*)obj, 0x10F0)) continue;

        /* Write ROT_Y to the direction field — native render uses it as multiplier */
        *(float*)(obj + 0x10EC) = g_rotater_cfg[i].rot_y;
    }
}

/* Per-frame override for objects with ROS_Y=0 (constant rotation).
 * The native render flips direction at ±2.0 radians. For ROS_Y=0,
 * we rewrite ROT_Y to the direction field every frame to prevent
 * the oscillation reversal, keeping rotation constant. */
static void cEnt_update_constant_rotations(void) {
    int i;
    for (i = 0; i < g_rotater_count; i++) {
        if (g_rotater_cfg[i].ros_y != 0.0f) continue;  /* only for ROS_Y=0 */
        DWORD obj = g_rotater_cfg[i].obj;
        if (!obj || obj < 0x10000) continue;
        if (IsBadReadPtr((void*)obj, 0x10F0)) continue;
        /* Rewrite direction field to prevent native oscillation reversal.
         * The native render (vtable[11] 0x0043B330) does:
         *   angle += direction * 0.004
         *   if angle > 2.0:  direction = -1.0
         *   if angle < -2.0: direction = +1.0
         * By rewriting direction = rot_y EVERY frame (before the native render
         * runs, via the present hook = slot 9, before slot 10 object render),
         * the flip checks never take effect — the native reversal is
         * overwritten before it can be applied. The angle grows unboundedly,
         * but Gfx_Scale uses sin/cos so large angles are fine.
         * NOTE: NO angle clamp here. Clamping to ±1.99 caused a visual snap
         * (angle jumps ~114 degrees when it wraps). direction-only rewrite
         * gives smooth constant rotation. */
        *(float*)(obj + 0x10EC) = g_rotater_cfg[i].rot_y;
        /* v55m_48d: Stop the runaway acceleration, cap at native max.
         * Root cause (Ghidra-verified, render 0x0043B330 + Gfx_ScaleX 0x457C60):
         *   obj+0x10E8 (angle) += direction * 0.004
         *   obj+0x10E4 (accumulator, DEGREES) += old_angle   <-- quadratic!
         *   Gfx_ScaleX(obj+0x10E4) -> matrix angle = accum * 0.01745 (pi/180)
         * With ROS_Y=0 constant rotation the native ±2.0 flip never fires,
         * so the accumulator grows quadratically and the object spins faster
         * and faster. Fixes:
         *  (1) Wrap the accumulator to [-360, 360] deg every frame — it is
         *      only consumed through sin/cos in the rotation matrix, so
         *      wrapping is visually seamless and stops the unbounded growth.
         *  (2) Cap the angle (obj+0x10E8, the per-frame accumulator increment)
         *      at the cEnt Rotator max rate:
         *      max_speed(250.0) * 0.004/frame = 1.0 rad/frame (v55m_50). */
        {
            float accum = *(float*)(obj + 0x10E4);
            /* Sequential wrap: correct only by full 360° turns so the
             * per-frame delta stays continuous — a naive modulo would pop
             * (full reverse spin in one frame when crossing 360°). */
            while (accum >  360.0f) accum -= 720.0f;
            while (accum < -360.0f) accum += 720.0f;
            *(float*)(obj + 0x10E4) = accum;

            float angle = *(float*)(obj + 0x10E8);
            /* 1.0 = cEnt max speed (250.0) × 0.004 rad/frame per unit
             * (v55m_50: raised from 20.0 → 250.0, 0.08 → 1.0) */
            {
                const float max_angle = NATIVE_ROTATOR_MAX_SPEED * 0.004f;
                if (angle >  max_angle) angle =  max_angle;
                if (angle < -max_angle) angle = -max_angle;
            }
            *(float*)(obj + 0x10E8) = angle;
        }
    }
}

/* Scan S1 ref points for Rotater entries with custom rot tags.
 * For each found, search the board's update list for the natively-spawned
 * Rotator object at the matching position, and apply ROT_Y to its direction
 * field (+0x10EC). This does NOT spawn — native game already spawned from S1. */
static void cEnt_apply_s1_rotater_tags(DWORD board, FILE* logf) {
    if (!board) return;
    DWORD sceneobj = cEnt_get_sceneobj(board);
    if (!sceneobj) return;

    /* Read S1 ref points */
    DWORD s1_list = sceneobj + 0x894;
    if (IsBadReadPtr((void*)(s1_list + 0x04), 4)) return;
    int s1_count = *(int*)(s1_list + 0x04);
    if (s1_count <= 0 || s1_count > 1000) return;
    if (IsBadReadPtr((void*)(s1_list + 0x40C), 4)) return;
    DWORD* s1_data = *(DWORD**)(s1_list + 0x40C);
    if (!s1_data || IsBadReadPtr(s1_data, s1_count * 4)) return;

    /* Read board update list (AthenaList at board+0x2578) */
    DWORD update_list = board + 0x2578;
    if (IsBadReadPtr((void*)(update_list + 0x04), 4)) return;
    int update_count = *(int*)(update_list + 0x04);
    if (update_count <= 0 || update_count > 10000) return;
    if (IsBadReadPtr((void*)(update_list + 0x40C), 4)) return;
    DWORD* update_data = *(DWORD**)(update_list + 0x40C);
    if (!update_data || IsBadReadPtr(update_data, update_count * 4)) return;

    /* Known Rotator vtable addresses */
    const DWORD rotator_vtables[] = { 0x004D5518, 0x004D5708, 0 };

    int i, j;
    for (i = 0; i < s1_count; i++) {
        DWORD entry = s1_data[i];
        if (!entry || entry < 0x10000) continue;
        if (IsBadReadPtr((void*)entry, 16)) continue;

        char* name = *(char**)(entry);
        if (!name || IsBadReadPtr(name, 8)) continue;
        if (_strnicmp(name, "cEnt", 4) != 0 &&
            _strnicmp(name, "REF:cEnt", 8) != 0 &&
            _strnicmp(name, "C_entity", 8) != 0 &&
            _strnicmp(name, "REF:C_entity", 13) != 0) continue;

        /* Parse rotation tags from <DAT> block */
        char rot_y_str[32] = {0};
        char ros_y_str[32] = {0};
        cEnt_extract_dat_prop(name, "ROT_Y", rot_y_str, sizeof(rot_y_str));
        cEnt_extract_dat_prop(name, "ROS_Y", ros_y_str, sizeof(ros_y_str));
        if (!rot_y_str[0]) continue;  /* skip if no ROT_Y tag */

        float rot_y = (float)atof(rot_y_str);
        float ros_y = ros_y_str[0] ? (float)fabs(atof(ros_y_str)) : 2.0f;

        /* Get S1 ref point position */
        float px = *(float*)(entry + 0x04);
        float py = *(float*)(entry + 0x08);
        float pz = *(float*)(entry + 0x0C);

        /* Search update list for a Rotator at this position */
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

            /* Check position match (within 2.0 units) */
            float ox = *(float*)(obj + 0x10D4);
            float oy = *(float*)(obj + 0x10D8);
            float oz = *(float*)(obj + 0x10DC);
            float dx = ox - px; if (dx < 0) dx = -dx;
            float dy = oy - py; if (dy < 0) dy = -dy;
            float dz = oz - pz; if (dz < 0) dz = -dz;
            if (dx < 2.0f && dy < 2.0f && dz < 2.0f) {
                /* Found it! Write ROT_Y to direction field */
                *(float*)(obj + 0x10EC) = rot_y;

                /* If ROS_Y=0, store for per-frame direction override
                 * (native render flips direction at ±2.0, we need to
                 * continuously rewrite it to prevent oscillation) */
                if (ros_y == 0.0f && g_rotater_count < MAX_ROTATERS) {
                    g_rotater_cfg[g_rotater_count].obj = obj;
                    g_rotater_cfg[g_rotater_count].rot_y = rot_y;
                    g_rotater_cfg[g_rotater_count].ros_y = 0.0f;
                    g_rotater_cfg[g_rotater_count].angle_y = 0.0f;
                    g_rotater_cfg[g_rotater_count].mesh_path[0] = 0;
                    g_rotater_count++;
                }

                if (logf) {
                    fprintf(logf, "  ROTATER(S1-tag): obj=0x%08X at (%.1f,%.1f,%.1f) ROT_Y=%.4f ROS_Y=%.1f applied\n",
                            obj, px, py, pz, rot_y, ros_y);
                    fflush(logf);
                }
                break;
            }
        }
    }
}

/* Hide original meshbuffers that match C_entity names.
 * The level loader creates meshbuffers for ALL section-3 entries, including
 * C_entity_001. These render as part of the level geometry. We need to hide
 * them so only our custom-spawned Rotator (with the MESH property) is visible.
 *
 * Approach: Zero the render context for matching meshbuffers.
 * The render context array is at MeshWorld+0x28, each entry is 0x50 bytes.
 * The meshbuffer list is at MeshWorld+0x2C (AthenaList).
 * We match by name (at MeshBuffer+0x864) and zero the corresponding
 * render context to make it invisible. */
static void hide_entity_meshbuffers(DWORD board, FILE* logf) {
    if (!board) return;
    DWORD level = cEnt_get_level(board);
    if (!level) return;
    if (IsBadReadPtr((void*)(level + 0x08), 4)) return;
    DWORD meshworld = *(DWORD*)(level + 0x08);
    if (!meshworld || IsBadReadPtr((void*)meshworld, 0x30)) return;

    /* MeshBuffer AthenaList at MeshWorld+0x2C */
    DWORD* mb_list = (DWORD*)(meshworld + 0x2C);
    if (IsBadReadPtr(mb_list, 8)) return;
    int mb_count = *(int*)(mb_list + 1);  /* count at +0x04 */
    if (mb_count <= 0 || mb_count > 10000) return;
    if (IsBadReadPtr((void*)((BYTE*)mb_list + 0x40C), 4)) return;
    DWORD* mb_data = *(DWORD**)((BYTE*)mb_list + 0x40C);
    if (!mb_data || IsBadReadPtr(mb_data, mb_count * 4)) return;

    /* Render context array at MeshWorld+0x28 */
    DWORD rc_array = *(DWORD*)(meshworld + 0x28);
    if (!rc_array || IsBadReadPtr((void*)rc_array, mb_count * 0x50)) {
        /* rc_array might be inline, not a pointer */
        rc_array = meshworld + 0x28;
        if (IsBadReadPtr((void*)rc_array, mb_count * 0x50)) return;
    }

    int hidden = 0;
    int i;
    for (i = 0; i < mb_count; i++) {
        DWORD mb = mb_data[i];
        if (!mb || mb < 0x10000) continue;
        if (IsBadReadPtr((void*)mb, 0x870)) continue;

        /* Read MeshBuffer name at +0x864 */
        char* name = *(char**)(mb + 0x864);
        if (!name || IsBadReadPtr(name, 8)) continue;

        /* Check if name starts with "cEnt" or "REF:cEnt" (case-insensitive) */
        if (_strnicmp(name, "cEnt", 4) != 0 &&
            _strnicmp(name, "C_entity", 8) != 0) continue;

        /* Zero the render context for this meshbuffer (0x50 bytes) */
        DWORD rc_addr = rc_array + i * 0x50;
        if (!IsBadReadPtr((void*)rc_addr, 0x50)) {
            memset((void*)rc_addr, 0, 0x50);
            hidden++;
            if (logf) fprintf(logf, "  HIDE: meshbuffer '%s' (idx=%d, mb=0x%08X) render context zeroed\n", name, i, mb);
        }
    }

    if (logf && hidden > 0) {
        fprintf(logf, "  HIDE: hidden %d original meshbuffer(s)\n", hidden);
        fflush(logf);
    }
}

/* Scan section 3 for REF:Rotater entries and spawn SWIRL at each position.
 * NOTE: Do NOT scan S1 ref points — the game's native vtable[33] handler
 * already spawns Rotators from S1 entries named "Rotater". Scanning S1 would
 * create duplicate objects (double SWIRL). Only section-3 entries are ours. */
/* Case-insensitive substring search (MinGW doesn't have _stristr) */
static char* my_stristr(const char* haystack, const char* needle) {
    if (!haystack || !needle) return NULL;
    size_t nlen = strlen(needle);
    if (nlen == 0) return (char*)haystack;
    for (; *haystack; haystack++) {
        if (_strnicmp(haystack, needle, nlen) == 0)
            return (char*)haystack;
    }
    return NULL;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Entity Definition System
 * Each entity has an onCreate script (runs at spawn) and an onUpdate script
 * (runs every frame). Scripts are loaded from Centities/<name>.txt
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
    /* v46 config variables — parsed from .txt header */
    int   rng_seed;         /* RNG = false/true (unused for now, reserved) */
    float rot_a;            /* ROT_A = initial angle (obj+0x10E8, default 0.0) */
    float rot_d;            /* ROT_D = initial direction (obj+0x10EC, default 0.0) */
    float rot_max;          /* ROT_MAX = max oscillation angle (native 2.0, default 0.0) */
    float rot_min;          /* ROT_MIN = min oscillation angle (native -2.0, default 0.0) */
    int   rot_m;            /* ROT_M = axis: 0=X, 1=Y, 2=Z (default 0=X) */
    entity_cmd_t create_cmds[MAX_ENTITY_CMDS];
    int create_cmd_count;
    entity_cmd_t update_cmds[MAX_ENTITY_CMDS];
    int update_cmd_count;
} entity_def_t;

/* Parse a .txt entity definition file into an entity_def_t */
static int load_entity_def(const char* txt_path, entity_def_t* def, FILE* logf) {
    memset(def, 0, sizeof(entity_def_t));
    def->obj_size = 0x1508;  /* default */

    FILE* ef = NULL;
    fopen_s(&ef, txt_path, "r");
    if (!ef) {
        if (logf) fprintf(logf, "  ENTITY: %s not found\n", txt_path);
        return 0;
    }

    char line[256];
    int section = 0;  /* 0=header, 1=onCreate, 2=onUpdate */

    while (fgets(line, sizeof(line), ef)) {
        /* Strip comments */
        char* hash = strchr(line, '#');
        if (hash) *hash = 0;

        /* Strip whitespace */
        char* p = line;
        while (*p == ' ' || *p == '\t') p++;
        size_t len = strlen(p);
        while (len > 0 && (p[len-1] == '\n' || p[len-1] == '\r' || p[len-1] == ' ' || p[len-1] == '\t'))
            p[--len] = 0;
        if (len == 0) continue;

        /* Check for section headers */
        if (_stricmp(p, "[onCreate]") == 0) { section = 1; continue; }
        if (_stricmp(p, "[onUpdate]") == 0) { section = 2; continue; }

        /* Parse command + args */
        entity_cmd_t* cmd = NULL;
        if (section == 1 && def->create_cmd_count < MAX_ENTITY_CMDS) {
            cmd = &def->create_cmds[def->create_cmd_count++];
        } else if (section == 2 && def->update_cmd_count < MAX_ENTITY_CMDS) {
            cmd = &def->update_cmds[def->update_cmd_count++];
        }
        if (!cmd) {
            /* Header section — parse KEY VALUE */
            char key[32] = {0}, val[128] = {0};
            if (sscanf(p, "%31s %127s", key, val) >= 2) {
                if (_stricmp(key, "MESH") == 0) {
                    strncpy(def->mesh_file, val, 127);
                    def->mesh_file[127] = 0;
                }
                else if (_stricmp(key, "SIZE") == 0) {
                    def->obj_size = (int)strtol(val, NULL, 0);
                }
                else if (_stricmp(key, "RNG") == 0) {
                    /* RNG = false/true (reserved for future use) */
                    if (_stricmp(val, "true") == 0 || _stricmp(val, "1") == 0)
                        def->rng_seed = 1;
                    else
                        def->rng_seed = 0;
                }
                else if (_stricmp(key, "ROT_A") == 0) {
                    def->rot_a = (float)atof(val);
                }
                else if (_stricmp(key, "ROT_D") == 0) {
                    def->rot_d = (float)atof(val);
                }
                else if (_stricmp(key, "ROT_MAX") == 0) {
                    def->rot_max = (float)atof(val);
                }
                else if (_stricmp(key, "ROT_MIN") == 0) {
                    def->rot_min = (float)atof(val);
                }
                else if (_stricmp(key, "ROT_M") == 0) {
                    if (val[0] == 'X' || val[0] == 'x') def->rot_m = 0;
                    else if (val[0] == 'Y' || val[0] == 'y') def->rot_m = 1;
                    else if (val[0] == 'Z' || val[0] == 'z') def->rot_m = 2;
                    else def->rot_m = 0;  /* default X */
                }
            }
            continue;
        }

        /* Tokenize the command line */
        char* tok = strtok(p, " \t");
        if (!tok) continue;
        strncpy(cmd->cmd, tok, 31);
        cmd->cmd[31] = 0;
        cmd->arg_count = 0;
        while ((tok = strtok(NULL, " \t")) && cmd->arg_count < MAX_CMD_ARGS) {
            strncpy(cmd->args[cmd->arg_count], tok, 63);
            cmd->args[cmd->arg_count][63] = 0;
            cmd->arg_count++;
        }
    }
    fclose(ef);
    return 1;
}

/* Execute an onCreate command on an object */
static void exec_create_cmd(DWORD obj, DWORD board, float px, float py, float pz,
                            void* mesh, entity_cmd_t* cmd, FILE* logf) {
    if (_stricmp(cmd->cmd, "SET_VTABLE") == 0 && cmd->arg_count >= 1) {
        DWORD vt = (DWORD)strtol(cmd->args[0], NULL, 0);
        *(DWORD*)obj = vt;
    }
    else if (_stricmp(cmd->cmd, "STORE_BOARD") == 0 && cmd->arg_count >= 1) {
        int off = (int)strtol(cmd->args[0], NULL, 0);
        *(DWORD*)(obj + off) = board;
    }
    else if (_stricmp(cmd->cmd, "STORE_POS") == 0 && cmd->arg_count >= 3) {
        int ox = (int)strtol(cmd->args[0], NULL, 0);
        int oy = (int)strtol(cmd->args[1], NULL, 0);
        int oz = (int)strtol(cmd->args[2], NULL, 0);
        *(float*)(obj + ox) = px;
        *(float*)(obj + oy) = py;
        *(float*)(obj + oz) = pz;
    }
    else if (_stricmp(cmd->cmd, "INIT_LIST") == 0 && cmd->arg_count >= 1) {
        int off = (int)strtol(cmd->args[0], NULL, 0);
        /* AthenaList_Init — simplified: zero the list */
        memset((void*)(obj + off), 0, 0x410);
    }
    else if (_stricmp(cmd->cmd, "CREATE_RENDER_CONTEXT") == 0 && cmd->arg_count >= 1) {
        int off = (int)strtol(cmd->args[0], NULL, 0);
        /* Allocate render context via operator_new + Level_RenderCtor */
        void* rc = pfn_operator_new(0x10D0);
        if (rc) {
            typedef void* (__thiscall *Level_RenderCtor_t)(void*, int);
            Level_RenderCtor_t pfn = (Level_RenderCtor_t)0x00465080;
            rc = pfn(rc, (int)obj);
        }
        *(void**)(obj + off) = rc;
    }
    else if (_stricmp(cmd->cmd, "SET_FLOAT") == 0 && cmd->arg_count >= 2) {
        int off = (int)strtol(cmd->args[0], NULL, 0);
        float val = (float)atof(cmd->args[1]);
        *(float*)(obj + off) = val;
    }
    else if (_stricmp(cmd->cmd, "IF_NOT_PRACTICE") == 0) {
        /* Check practice flag: board+0x878 → App, App+0x237 */
        /* This is a block marker — actual handling is in exec_create_cmds */
    }
    else if (_stricmp(cmd->cmd, "ENDIF") == 0) {
        /* Block end marker */
    }
}

/* Execute all onCreate commands */
static void exec_create_cmds(DWORD obj, DWORD board, float px, float py, float pz,
                             void* mesh, entity_def_t* def, FILE* logf) {
    int i;
    int in_not_practice = 0;
    int is_practice = 0;

    /* Check practice mode: board+0x878 → App, App+0x237 */
    DWORD app = *(DWORD*)(board + 0x878);
    if (app && !IsBadReadPtr((void*)(app + 0x237), 1)) {
        is_practice = (*(char*)(app + 0x237) != 0);
    }

    for (i = 0; i < def->create_cmd_count; i++) {
        entity_cmd_t* cmd = &def->create_cmds[i];

        if (_stricmp(cmd->cmd, "IF_NOT_PRACTICE") == 0) {
            in_not_practice = 1;
            continue;
        }
        if (_stricmp(cmd->cmd, "ENDIF") == 0) {
            in_not_practice = 0;
            continue;
        }
        if (in_not_practice && is_practice) continue;

        exec_create_cmd(obj, board, px, py, pz, mesh, cmd, logf);
    }

    /* v46: Apply ROT_A (initial angle) and ROT_D (initial direction) after onCreate.
     * These write to the same fields the native render function reads:
     *   obj+0x10E8 = angle (param_1[0x43a] in decompiled render at 0x0043B330)
     *   obj+0x10EC = direction (param_1[0x43b])
     * Default is 0.0 for both — the native render will then compute:
     *   new_angle = direction * 0.004 + angle
     * If ROT_D is 0.0 and no onUpdate ROTATE command runs, the object stays still. */
    if (def->rot_a != 0.0f) {
        *(float*)(obj + 0x10E8) = def->rot_a;
    }
    if (def->rot_d != 0.0f) {
        *(float*)(obj + 0x10EC) = def->rot_d;
    }
}

/* Execute all onUpdate commands for a tracked entity */
static void exec_update_cmds(DWORD obj, entity_def_t* def, FILE* logf) {
    int i;
    for (i = 0; i < def->update_cmd_count; i++) {
        entity_cmd_t* cmd = &def->update_cmds[i];

        if (_stricmp(cmd->cmd, "ROTATE_Y") == 0 && cmd->arg_count >= 1) {
            float speed = (float)atof(cmd->args[0]);
            float angle = *(float*)(obj + 0x10E8);
            float dir = *(float*)(obj + 0x10EC);
            angle = dir * speed + angle;
            *(float*)(obj + 0x10E8) = angle;
        }
        else if (_stricmp(cmd->cmd, "OSCILLATE") == 0 && cmd->arg_count >= 1) {
            float limit = (float)atof(cmd->args[0]);
            float angle = *(float*)(obj + 0x10E8);
            float dir = *(float*)(obj + 0x10EC);
            if (angle > limit)  dir = -1.0f;
            if (angle < -limit) dir = 1.0f;
            *(float*)(obj + 0x10EC) = dir;
        }
    }

    /* v46: Per-frame ROT_MAX/ROT_MIN oscillation override.
     * The native render function (0x0043B330) uses hardcoded ±2.0 limits:
     *   if angle > 2.0:  direction = -1.0
     *   if angle < -2.0: direction = +1.0
     * When ROT_MAX/ROT_MIN are non-zero, we override the direction field
     * every frame so the oscillation uses the custom limits instead.
     *
     * Also: ROT_M selects which Gfx_Scale function to call for the
     * "rotation to render matrix" at the end of the update section.
     * Native render calls Gfx_ScaleX(angle). We call the axis-selected
     * function with the current angle (obj+0x10E4 = param_1[0x439]). */
    {
        float rot_max = def->rot_max;
        float rot_min = def->rot_min;
        float angle = *(float*)(obj + 0x10E8);
        float dir = *(float*)(obj + 0x10EC);

        /* Override oscillation limits if configured */
        if (rot_max != 0.0f && angle > rot_max) {
            *(float*)(obj + 0x10EC) = -1.0f;
        }
        if (rot_min != 0.0f && angle < rot_min) {
            *(float*)(obj + 0x10EC) = 1.0f;
        }

        /* ROT_M: call the axis-appropriate Gfx_Scale function with the
         * accumulated angle from obj+0x10E4 (the render matrix angle field).
         * Native render at 0x0043B330 does: Gfx_ScaleX(param_1[0x439])
         * where param_1[0x439] = obj+0x10E4 (byte offset).
         * We replicate this but with the selected axis function. */
        {
            float render_angle = *(float*)(obj + 0x10E4);
            Gfx_ScaleAxis_t scale_fn = pfn_Gfx_ScaleX;  /* default: X */
            if (def->rot_m == 1) scale_fn = pfn_Gfx_ScaleY;
            else if (def->rot_m == 2) scale_fn = pfn_Gfx_ScaleZ;
            scale_fn(render_angle);
        }
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Gluebie proximity behavior — cross-level support
 *
 * Native DizzyBoard_Update iterates board+0x4378 (Gluebie AthenaList, Dizzy-only)
 * and checks ball proximity. We replicate this in the mod's per-frame update so
 * it works on ALL levels.
 *
 * Logic (from decompiled DizzyBoard_Update):
 *   For each ball in board+0x29D4 (ball AthenaList):
 *     For each Gluebie:
 *       dist = 3D distance(gluebie+0x10E0/10E4/10E8, ball+0x164/168/16C)
 *       if dist < gluebie+0x1100 * 60.0:
 *         Read ball velocity from collisionMesh+0xCA4/CA8/CAC
 *           (ball+0x1A4 = collision mesh pointer, set by Ball_ctor)
 *         Scale velocity by 0.95 (slowdown factor)
 *         Write back to collisionMesh+0xCA4/CA8/CAC
 *         Set gluebie+0x1104 = 1 (active flag)
 * ═══════════════════════════════════════════════════════════════════════════ */

/* v55j_9: Present hook infrastructure — runs Gluebie check on main thread.
 * The background thread (entity_thread) is NOT synchronized with the physics
 * loop, so velocity scaling races with the physics engine. The Present hook
 * runs at the end of each frame on the main thread, after physics but before
 * next frame's physics step — correct timing for velocity scaling. */
static BYTE *g_present_cave = NULL;
static int g_present_hook_installed = 0;

/* C helper called from the Present hook cave (main thread, safe for C calls).
 * v55j_8: Skip on Dizzy — native DizzyBoard_Update already handles Gluebie
 * proximity there. Running our check too would double-scale velocity. */
static int gluebie_is_dizzy(DWORD board) {
    if (!board) return 0;
    DWORD app = *(DWORD*)(board + BOARD_APP);
    if (app && !IsBadReadPtr((void*)(app + 0x5FC), 4)) {
        int race_idx = *(int*)(app + 0x5FC);
        if (race_idx == 2 || race_idx == 13) return 1;
    }
    char* level_name = NULL;
    if (!IsBadReadPtr((void*)(board + 0x10), 4)) {
        level_name = *(char**)(board + 0x10);
    }
    if (level_name && !IsBadReadPtr(level_name, 5)) {
        if (my_stristr(level_name, "Dizzy") != NULL) return 1;
    }
    return 0;
}

/* Forward declaration — defined below */
static void cEnt_gluebie_proximity_check(DWORD board);
static void cEnt_tarpit_proximity_check(DWORD board);  /* v55k_1 */

/* Forward decls — defined below */
static void cEnt_chomper_update(DWORD board);
static void cEnt_catapult_present_check(DWORD board);  /* v55m_27i */
/* v55m_28d: Get Player 1 ball pointer from board+0x29D4 (ball AthenaList).
 * v55m_27l tried board+0x2DEC, but that list is empty. The real player ball
 * list is board+0x29D4; we iterate it and select ball+0x18 == 0.
 * Per-frame logging removed to fix lag spikes. */
static DWORD get_ball_ptr(void) {
    DWORD board = get_board();
    if (!board || board < 0x10000 || IsBadReadPtr((void*)board, 0x2E00)) {
        return 0;
    }
    DWORD ball_list = board + 0x29D4;  /* AthenaList: balls */
    if (IsBadReadPtr((void*)ball_list, 0x410)) {
        return 0;
    }
    DWORD count = *(DWORD*)(ball_list + 0x04);
    if (count == 0) return 0;
    DWORD items = *(DWORD*)(ball_list + 0x40C);
    if (!items || items < 0x10000 || IsBadReadPtr((void*)items, count * 4)) {
        return 0;
    }

    /* Find Player 1 ball (player_idx at ball+0x18 == 0) */
    DWORD i;
    DWORD player_ball = 0;
    for (i = 0; i < count; i++) {
        DWORD ball = ((DWORD*)items)[i];
        if (!ball || ball < 0x10000 || IsBadReadPtr((void*)ball, 0x200)) continue;
        DWORD player_idx = *(DWORD*)(ball + 0x18);
        if (player_idx == 0 && player_ball == 0) {
            player_ball = ball;
        }
    }
    return player_ball;
}

/* v55m_27i: Ball position and force-accumulator offsets.
 * Position is at ball+0x164 (used by jump_mod, magnet_mod, ghost mods).
 * Physics force accumulators are at ball+0x170/0x174/0x178 — the engine
 * consumes these in Ball_Update with proper collision/friction response.
 * Writing to ball+0x20 (direct velocity) is ignored by physics. */
#define BALL_POS_X    0x164
#define BALL_POS_Y    0x168
#define BALL_POS_Z    0x16C
#define BALL_FORCE_X  0x170
#define BALL_FORCE_Y  0x174
#define BALL_FORCE_Z  0x178

/* v55m_27j: diagnostic heartbeat + per-frame catapult debug */
static int g_catapult_heartbeat = 0;
static int g_catapult_debug_count = 0;

/* v55m_43h: Rotate the catapult's collision vertex data around its center.
 * The collision Level at obj+0x10D4 has a SceneObject at +0x480 whose vertex
 * array (+0x440, 32-byte stride) feeds BOTH the render AND the spatial-tree
 * collision check. Rotating it in place around the object center keeps the
 * object inside its original AABB (tree bounds stay valid) while the
 * Returns total vertex count rotated, or 0 on failure. */
static int cEnt_catapult_rotate_collision_verts(CatapultState* cs) {
    int b, r;  /* v55m_43h rev9: loop vars for save/rotate passes */
     if (!cs || !cs->obj) return 0;
     DWORD colLevel = *(DWORD*)((char*)cs->obj + 0x10D4);
     if (!colLevel || colLevel < 0x10000) return 0;
     if (IsBadReadPtr((void*)colLevel, 0x490)) return 0;

     /* v55m_43m: TREE-ITEM ROTATION — the actual collision data the query
      * reads. The SpatialTree query (0x4564c0, vtable 0x4D8E14) iterates
      * TWO item lists on the tree: this+0x18 (count +0x4, items +0x40C)
      * and this+0x848 (same layout), reading each item's +0/+4/+8 as a
      * WORLD-space position (compared directly against the ball at
      * 0x456596). These items are baked at tree build from the strips.
      * Rev7's attempt NEVER RAN (early return before it) — tree=0 in that
      * log meant "not measured", not "empty". We measure + rotate BOTH
      * candidate lists here FIRST, before any early return, so we learn
      * the real counts. Rotate around the WORLD pivot (cs->x,y,z) — the
      * items are world-space, unlike the strips which are mesh-local. */
     {
         float tang = cs->arm_angle * 3.14159265f / 180.0f;
         float tc = cosf(tang), ts = sinf(tang);
         DWORD tree_candidates[2];
         tree_candidates[0] = colLevel + 0x18;
         tree_candidates[1] = colLevel + 0x848;
         int tci;
         for (tci = 0; tci < 2; tci++) {
             DWORD treelist = tree_candidates[tci];
             if (IsBadReadPtr((void*)treelist, 0x20)) continue;
             int tcount = *(int*)(treelist + 0x4);
             if (tcount <= 0 || tcount > 4096) continue;
             DWORD* titems = *(DWORD**)(treelist + 0x40C);
             if (!titems || IsBadReadPtr((void*)titems, tcount * 4)) continue;
             /* Save originals lazily (12 bytes/item) for THIS list. */
             DWORD* orig_slot = (tci == 0) ? &cs->tree_orig18 : &cs->tree_orig848;
             int* count_slot = (tci == 0) ? &cs->tree_count18 : &cs->tree_count848;
             if (!*orig_slot) {
                 *orig_slot = (DWORD)malloc(tcount * 12);
                 if (*orig_slot) {
                     float* dst = (float*)*orig_slot;
                     int ti;
                     for (ti = 0; ti < tcount; ti++) {
                         DWORD item = titems[ti];
                         if (!item || IsBadReadPtr((void*)item, 0x10)) continue;
                         float* src = (float*)item;
                         dst[ti * 3 + 0] = src[0];
                         dst[ti * 3 + 1] = src[1];
                         dst[ti * 3 + 2] = src[2];
                     }
                     *count_slot = tcount;
                 }
             }
             if (*orig_slot && *count_slot == tcount) {
                 float* tsrc = (float*)*orig_slot;
                 int ti;
                 for (ti = 0; ti < tcount; ti++) {
                     DWORD item = titems[ti];
                     if (!item || IsBadReadPtr((void*)item, 0x10)) continue;
                     float* o = tsrc + ti * 3;
                     float* p = (float*)item;
                     /* WORLD pivot — tree items are world-space. */
                     float x = o[0] - cs->x;
                     float z = o[2] - cs->z;
                     p[0] = x * tc + z * ts + cs->x;
                     p[1] = o[1];
                     p[2] = -x * ts + z * tc + cs->z;
                 }
                 cs->tree_rotated = 1;
                 if (tci == 0) cs->tree_ok18 = 1; else cs->tree_ok848 = 1;
             }
         }
     }

     DWORD mw = *(DWORD*)((char*)colLevel + 0x08);
     if (!mw || mw < 0x10000) return 0;
     if (IsBadReadPtr((void*)mw, 0x460)) return 0;

     /* v55m_43q: ROTATE THE MESHWORLD TREE ITEMS — the actual collision
      * data. The collision query (0x465ef0) iterates the MeshWorld's
      * collision tree at mw+0x18 (embedded AthenaList: count +0x4, items
      * +0x40C) — NOT colLevel+0x18 (those were empty, tree18=0). Each
      * item's +0/+4/+8 is a WORLD-space position. Rotate them around the
      * WORLD pivot (cs->x,y,z). This is what the broad-phase box test and
      * the exact triangle test read. */
     {
         float tang = cs->arm_angle * 3.14159265f / 180.0f;
         float tc = cosf(tang), ts = sinf(tang);
         DWORD mwlist = mw + 0x18;
         if (!IsBadReadPtr((void*)mwlist, 0x20)) {
             int tcount = *(int*)(mwlist + 0x4);
             if (tcount > 0 && tcount < 65536) {
                 DWORD* titems = *(DWORD**)(mwlist + 0x40C);
                 if (titems && !IsBadReadPtr((void*)titems, tcount * 4)) {
                     /* Save originals lazily (12 bytes/item). */
                     if (!cs->tree_orig_mw) {
                         cs->tree_orig_mw = (DWORD)malloc(tcount * 12);
                         if (cs->tree_orig_mw) {
                             float* dst = (float*)cs->tree_orig_mw;
                             int ti;
                             for (ti = 0; ti < tcount; ti++) {
                                 DWORD item = titems[ti];
                                 if (!item || IsBadReadPtr((void*)item, 0x10)) continue;
                                 float* src = (float*)item;
                                 dst[ti * 3 + 0] = src[0];
                                 dst[ti * 3 + 1] = src[1];
                                 dst[ti * 3 + 2] = src[2];
                             }
                             cs->tree_count_mw = tcount;
                         }
                     }
                     if (cs->tree_orig_mw && cs->tree_count_mw == tcount) {
                         float* tsrc = (float*)cs->tree_orig_mw;
                         int ti;
                         for (ti = 0; ti < tcount; ti++) {
                             DWORD item = titems[ti];
                             if (!item || IsBadReadPtr((void*)item, 0x10)) continue;
                             float* o = tsrc + ti * 3;
                             float* p = (float*)item;
                             /* WORLD pivot — tree items are world-space. */
                             float x = o[0] - cs->x;
                             float z = o[2] - cs->z;
                             p[0] = x * tc + z * ts + cs->x;
                             p[1] = o[1];
                             p[2] = -x * ts + z * tc + cs->z;
                         }
                         cs->tree_ok_mw = 1;
                     }
                 }
             }
         }
     }

    /* v55m_43h (rev 4): The vertex source is the SUB-MESH arrays.
     * MeshWorld_BuildVertexBuffer (0x46f8d0) flow:
     *   - iterates MeshWorld+0x2C (MeshBuffers)
     *   - each MeshBuffer has sub-meshes at +0x424 (AthenaList)
     *   - 0x46faa3: mov 0x448(%edx),%eax reads sub-mesh+0x448 = SOURCE verts
     *   - copies count*32 bytes into the PACKED output at MeshWorld+0x448
     * The spatial tree references the SUB-MESH source arrays (built at
     * load). MeshWorld+0x448 is 0 until the first render (lazy build).
     * So we rotate EACH sub-mesh's +0x448 array (the tree source). */
    DWORD* list = (DWORD*)(mw + 0x2C);
    if (IsBadReadPtr((void*)list, 0x20)) return 0;
    int list_count = *(int*)(list + 0x1);  /* +0x4 */
    if (list_count <= 0 || list_count > 64) return 0;
    DWORD* items = *(DWORD**)(list + 0x103);  /* +0x40C */
    if (!items || IsBadReadPtr((void*)items, list_count * 4)) return 0;

    /* v55m_43k: ROTATE THE COLLISION STRIPS — this is what the collision
     * query (Mesh_FindClosestCollision 0x465d90) actually reads. The query
     * iterates:
     *   MeshWorld+0x2C  → MeshBuffer list (count +0x4, items +0x40C)
     *   MeshBuffer+0xC  → strip list (EMBEDDED AthenaList: count at +0x10,
     *                     items at +0x418)  [dump: mb[0]+10=32 strips,
     *                     +418=items array]
     *   each strip      → 3 CONSECUTIVE vertices, 32 bytes each (8 floats),
     *                     (X,Y,Z) floats at +0/+4/+8
     * The broad-phase AABB test (0x4580d0) checks the ball against these
     * vertex positions; passing verts go to the exact triangle test.
     * The strips are in MESH-LOCAL space (dump v0=(6.2,-20.3,108.1)) —
     * rotate around the mesh origin (0,0,0), matching the arm's visual
     * Y-rotation. Rotate from saved originals (per-MeshBuffer copies). */
    float ang = cs->arm_angle * 3.14159265f / 180.0f;
    float c = cosf(ang), s = sinf(ang);

    /* v55m_43r: ROTATE ONLY THE ARM (LAST MeshBuffer, mb[1]) — bowl stays
     * STATIC. The user reported the bowl becomes non-solid sometimes when
     * it rotates (v43p rotated both). The arm (mb[1], 238 strips) rotates
     * reliably (confirmed v43o). Keeping the bowl static gives SOLID bowl
     * collision while the arm still rotates and knocks the ball.
     * mb[0] (bowl, 32 strips) is left at its original position. */

    /* Save originals once: only the LAST MeshBuffer (the arm) is
     * registered for rotation; the bowl (all others) stays static. */
    int total = 0;
    if (!cs->orig_verts) {
        int bi;
        for (bi = list_count - 1; bi >= 0; bi--) {
            /* v55m_43r: take the LAST MeshBuffer (the arm). */
            DWORD mb = items[bi];
            if (!mb || IsBadReadPtr((void*)mb, 0x840)) continue;
            int strip_count = *(int*)((char*)mb + 0x10);
            if (strip_count <= 0 || strip_count > 4096) continue;
            DWORD* strip_items = *(DWORD**)((char*)mb + 0x418);
            if (!strip_items || IsBadReadPtr((void*)strip_items, strip_count * 4)) continue;
            /* Each strip = 3 verts × 32 bytes = 96 bytes. */
            DWORD save = (DWORD)malloc(strip_count * 96);
            if (!save) continue;
            int si;
            int total_ok = 1;
            for (si = 0; si < strip_count; si++) {
                DWORD strip = strip_items[si];
                if (!strip || IsBadReadPtr((void*)strip, 0x60)) { total_ok = 0; break; }
                memcpy((void*)(save + (DWORD)si * 96), (void*)strip, 96);
            }
            if (!total_ok) { free((void*)save); continue; }
            cs->orig_registry[0] = mb;
            cs->orig_registry[1] = save;
            cs->orig_vert_count = 1;
            cs->arm_mb_strips = strip_count;  /* record arm strip count */
            break;  /* only the last one */
        }
        cs->orig_verts = 1;
    }

    /* Rotate each MeshBuffer's strip vertices from its saved original. */
    {
        int ri;
        for (ri = 0; ri < cs->orig_vert_count; ri++) {
            DWORD mb = cs->orig_registry[ri * 2];
            DWORD orig = cs->orig_registry[ri * 2 + 1];
            if (!mb || !orig || IsBadReadPtr((void*)mb, 0x840)) continue;
            int strip_count = *(int*)((char*)mb + 0x10);
            if (strip_count <= 0 || strip_count > 4096) continue;
            DWORD* strip_items = *(DWORD**)((char*)mb + 0x418);
            if (!strip_items || IsBadReadPtr((void*)strip_items, strip_count * 4)) continue;
            /* v55m_43s: dump first strip vertex of the FIRST registered
             * MeshBuffer (bowl if mb[0] registered, arm if mb[1]) so we
             * can SEE whether the rotation is actually moving it. */
            if (ri == 0 && (cs->rot_dump_ctr++ % 90) == 0) {
                DWORD s0 = strip_items[0];
                if (s0 && !IsBadReadPtr((void*)s0, 0x60)) {
                    float* v0 = (float*)s0;
                    FILE* lf = fopen("mknp_custom_entities_catapult.log", "a");
                    if (lf) {
                        fprintf(lf, "CATAPULT: mb[%d] strip0 v0=(%.1f,%.1f,%.1f) angle=%.1f strips=%d\n",
                            ri, v0[0], v0[1], v0[2], cs->arm_angle, strip_count);
                        fclose(lf);
                    }
                }
            }
            int si;
            for (si = 0; si < strip_count; si++) {
                DWORD strip = strip_items[si];
                if (!strip || IsBadReadPtr((void*)strip, 0x60)) continue;
                float* o = (float*)(orig + (DWORD)si * 96);
                float* p = (float*)strip;
                int v;
                for (v = 0; v < 3; v++) {  /* 3 verts per strip */
                    float ox = o[v * 8 + 0];
                    float oy = o[v * 8 + 1];
                    float oz = o[v * 8 + 2];
                    /* v55m_43u: ROTATE AROUND X AXIS (Y/Z plane, X stays
                     * constant) per user request. Previously Z-axis.
                     * X-axis:
                     *   x' = x
                     *   y' = y*cos - z*sin
                     *   z' = y*sin + z*cos */
                    p[v * 8 + 0] = ox;
                    p[v * 8 + 1] = oy * c - oz * s;
                    p[v * 8 + 2] = oy * s + oz * c;
                }
                total += 3;
            }
        }
    }

    return total;
}

/* v55m_42f: Catapult trigger via Present-hook radius check (restored from v55m_28m).
 * The native E:CATAPULTBOTTOM event only works on Tower (race 4) because only
 * Tower's collision handler checks for it. On custom levels we must detect
 * proximity ourselves and apply launch force directly. */
static void __cdecl cEnt_catapult_present_check(DWORD board) {
    int i;
    FILE* df = NULL;
    DWORD ball = get_ball_ptr();

    g_catapult_heartbeat++;
    int log_now = ((g_catapult_heartbeat % 30) == 0);

    if (!ball || ball < 0x10000 || IsBadReadPtr((void*)ball, 0x200)) {
        if (log_now) {
            df = fopen("mknp_custom_entities_catapult.log", "a");
            if (df) {
                fprintf(df, "CATAPULT: heartbeat=%d ball=0x%08X count=%d (no ball)\n",
                    g_catapult_heartbeat, ball, g_catapult_count);
                fclose(df);
            }
        }
        return;
    }

    float ball_x = *(float*)(ball + BALL_POS_X);
    float ball_y = *(float*)(ball + BALL_POS_Y);
    float ball_z = *(float*)(ball + BALL_POS_Z);

    for (i = 0; i < g_catapult_count; i++) {
        CatapultState* cs = &g_catapults[i];
        if (!cs->obj || cs->obj < 0x10000) continue;
        if (cs->board != board) continue;
        if (IsBadReadPtr((void*)cs->obj, 0x1108)) { cs->obj = 0; continue; }

        /* v55m_43h rev15 (PAUSE FIX v3 — THE REAL ONE): The pause flag is
         * board+0x874. Scene_Update (0x419c00, board vtable[1]) reads
         * board+0x3620/+0x4358 — the BOARD is the 0x4400-byte "Scene".
         * Scene_CreateGameOverMenu (0x40a920) sets board+0x874 = 1, and
         * GameUpdate (0x469cf0) skips updates when it's 1.
         * REV14 checked sceneobj+0x874 (the SceneObject from level+0x480)
         * — WRONG object, that flag is never set. The pause flag is on
         * the BOARD. When paused, board+0x874 == 1 → skip the rotation. */
        {
            if (board && board > 0x10000 &&
                !IsBadReadPtr((void*)(board + 0x878), 0x20)) {
                if (*(BYTE*)(board + 0x874) != 0) continue;  /* paused → skip */
            } else if (log_now) {
                df = fopen("mknp_custom_entities_catapult.log", "a");
                if (df) {
                    fprintf(df, "CATAPULT: PAUSEGATE board=0x%08X (bad read — gate disabled)\n", board);
                    fclose(df);
                }
            }
        }

        /* v55m_43z: Rotation now scaled ×0.1 per user request — the Tower
         * ramp was too strong. The spin numbers (angle increment) are
         * multiplied by 0.1: the arm does a gentler spin. The speed ramp
         * and decay still run at native values so the duration is the
         * same, only the resulting angle is 1/10th. */
        if (cs->rotating) {
            /* update: ramp while countdown active */
            if (cs->countdown > 0) {
                cs->spin_speed *= 1.25f;
                if (cs->spin_speed > 80.0f) cs->spin_speed = 80.0f;
            }
            /* render: angle += speed*0.1; speed -= 2.0 */
            cs->arm_angle += cs->spin_speed * 57.29578f * 0.1f;
            if (cs->arm_angle > 360.0f) cs->arm_angle -= 360.0f;
            cs->spin_speed -= 2.0f;
            if (cs->spin_speed <= 0.0f) {
                cs->spin_speed = 0.0f;
                cs->rotating = 0;  /* rotation done — stop */
            }
        }

        /* v55m_43h: Rotate the collision vertex data to match the render.
         * The render matrix already rotates the visual (user confirmed); this
         * rotates the actual vertex array so the spatial-tree collision
         * follows. Runs every frame; rotation is in-place around center so
         * the tree AABB bounds stay valid. Only when rotating (rotation
         * active). */
        if (cs->rotating) {
            if (log_now) {
                int rot = cEnt_catapult_rotate_collision_verts(cs);
                if (rot > 0) cs->verts_rotating = 1;
                df = fopen("mknp_custom_entities_catapult.log", "a");
                if (df) {
                    if (rot > 0) {
                        fprintf(df, "CATAPULT: rotated %d verts (angle=%.2f) spin=%.3f rot=%d mbufs=%d mw_tree=%d/%d tree18=%d/%d tree848=%d/%d\n",
                            rot, cs->arm_angle, cs->spin_speed, cs->rotating, cs->orig_vert_count,
                            cs->tree_count_mw, cs->tree_ok_mw,
                            cs->tree_count18, cs->tree_ok18, cs->tree_count848, cs->tree_ok848);
                    } else {
                        fprintf(df, "CATAPULT: VERT ROTATION FAILED (0 verts, angle=%.2f) spin=%.3f rot=%d mbufs=%d mw_tree=%d/%d tree18=%d/%d tree848=%d/%d\n",
                            cs->arm_angle, cs->spin_speed, cs->rotating, cs->orig_vert_count,
                            cs->tree_count_mw, cs->tree_ok_mw,
                            cs->tree_count18, cs->tree_ok18, cs->tree_count848, cs->tree_ok848);
                    }
                    fclose(df);
                }
            } else {
                int rot = cEnt_catapult_rotate_collision_verts(cs);
                if (rot > 0) cs->verts_rotating = 1;
            }
        }

        if (cs->cooldown > 0) {
            cs->cooldown--;
            if (cs->cooldown == 0) cs->launching = 0;
        }

        float dx = ball_x - cs->x;
        float dy = ball_y - cs->y;
        float dz = ball_z - cs->z;
        float horiz_sq = dx*dx + dz*dz;
        float horiz = (float)sqrt(horiz_sq);
        int in_trigger_zone = (horiz_sq < 14400.0f && dy > -10.0f && dy < 15.0f);  /* v55m_42g: bowl, radius 120 */
        int in_reset_zone   = (horiz_sq < 22500.0f && dy > -60.0f && dy < 70.0f);

        if (log_now || (horiz_sq < 62500.0f && dy > -180.0f && dy < 120.0f)) {
            df = fopen("mknp_custom_entities_catapult.log", "a");
            if (df) {
                fprintf(df, "CATAPULT: dist ball=(%.1f,%.1f,%.1f) cat=(%.1f,%.1f,%.1f) horiz=%.1f dy=%.1f "
                    "trigger=%d reset=%d launch=%d cd=%d was=%d\n",
                    ball_x, ball_y, ball_z, cs->x, cs->y, cs->z, horiz, dy,
                    in_trigger_zone, in_reset_zone, cs->launching, cs->cooldown, cs->was_in_zone);
                fclose(df);
                df = NULL;
            }
        }

        if (in_trigger_zone && cs->cooldown == 0 && !cs->was_in_zone) {
            cs->launching = 1;
            cs->countdown = 50;  /* v55m_42j: native Tower windup */
            cs->was_in_zone = 1;
            cs->rotating = 0;  /* v55m_43y: rotation starts at LAUNCH, not windup */
            float yaw = cs->yaw;
            cs->launch_dx = -(float)sin(yaw);
            cs->launch_dz = -(float)cos(yaw);
            cEnt_play_dropin_sound(df);  /* v55m_42j: play dropin at windup start */
            df = fopen("mknp_custom_entities_catapult.log", "a");
            if (df) {
                fprintf(df, "CATAPULT: WINDUP ball=(%.1f,%.1f,%.1f) cat=(%.1f,%.1f,%.1f) horiz=%.1f dy=%.1f\n",
                    ball_x, ball_y, ball_z, cs->x, cs->y, cs->z, horiz, dy);
                fclose(df);
                df = NULL;
            }
        }
        if (!in_reset_zone) {
            cs->was_in_zone = 0;
        }

        if (cs->launching) {
            cs->countdown--;
            /* v55m_43h: REMOVED windup arm_angle override — arm_angle is now
             * the CONTINUOUS Y-spin (SWIRL-style), incremented at line 4004.
             * The windup tilt was overriding it every frame, preventing the
             * continuous rotation the user requested. */
            if (cs->countdown > 0) {
                continue;
            }

            float dxz = cs->launch_dx;
            float dzz = cs->launch_dz;

            /* v55m_42j: launch at frame 50, matching native Tower */
            const float launch_horiz = 18.75f;
            const float launch_vert  = -11.25f;
            *(float*)(ball + BALL_FORCE_X) += dxz * launch_horiz;
            *(float*)(ball + BALL_FORCE_Y) += launch_vert;
            *(float*)(ball + BALL_FORCE_Z) += dzz * launch_horiz;

            df = fopen("mknp_custom_entities_catapult.log", "a");
            if (df) {
                fprintf(df, "CATAPULT: LAUNCH! ball=(%.1f,%.1f,%.1f) cat=(%.1f,%.1f,%.1f) dir=(%.2f,%.2f)\n",
                    ball_x, ball_y, ball_z, cs->x, cs->y, cs->z, dxz, dzz);
                fclose(df);
                df = NULL;
            }

            cs->launching = 0;
            cs->cooldown = 60;
            /* v55m_43y: rotation starts NOW — exactly when the catapult
             * sound plays (the user's request). */
            cs->rotating = 1;
            cs->spin_speed = 50.0f;  /* native Tower trigger speed (0x434056) */
            /* v55m_43h: removed arm_angle=0 snap — angle stays continuous */

            /* v55m_42j: play Catapult launch sound at same frame as launch */
            cEnt_play_catapult_sound(df);
        }
    }
}

/* v55m_42w: renderscene_helper — called from Graphics_RenderScene hook.
 * Sets g_in_draw_phase=1 so Stands vtable[18] hooks can do D3D transforms. */
static void __cdecl renderscene_helper(void) {
    g_in_draw_phase = 1;
}

static void __cdecl gluebie_present_helper(void) {
    if (game_is_quitting()) {
        /* v55n_7: One-shot despawn when the game starts quitting. On quit the
         * game's board teardown frees board+0x10EC + scene tree, which still
         * hold our +0x10E0 collision Levels (TimeButton/SpeedCylinder are NOT
         * in board+0x2578 so a normal quit never fires cEnt_despawn_all_rotaters).
         * Unhook them FIRST so the teardown finds clean lists -> no double-free.
         * Guarded so this runs exactly once per process. */
        if (!g_quit_despawn_done) {
            g_quit_despawn_done = 1;
            DWORD bd = get_board();
            if (bd) {
                FILE* lf = NULL;
                fopen_s(&lf, g_log_path, "a");
                cEnt_despawn_all_rotaters(bd, lf ? lf : NULL);
                if (lf) { fflush(lf); fclose(lf); }
            }
        }
        return;
    }
    g_gluebie_ball_in_zone = 0;  /* reset before check */
    DWORD board = get_board();
    if (board && g_gluebie_count > 0 && !gluebie_is_dizzy(board)) {
        cEnt_gluebie_proximity_check(board);
    }
    /* v55k_1: Tarpit proximity check — runs on ALL levels (not just non-Dizzy) */
    if (board && g_tarpit_count > 0) {
        cEnt_tarpit_proximity_check(board);
    }
    /* v55m_42w: Clear draw-phase guard at start of each frame.
     * Graphics_RenderScene sets it to 1 during Draw phase. */
    g_in_draw_phase = 0;

    /* v55m_44u: AI 1 Rotator constant rotation — run BEFORE the native
     * render (vtable[11] 0x0043B330) each frame. The present hook is slot 9
     * (viewport clear), which runs before slot 10 (object render), so this
     * clamps the angle to ±1.99 and rewrites direction to rot_y before the
     * native render's ±2.0 flip check fires. This keeps ROS_Y=0 Rotators
     * spinning in one direction instead of oscillating. */
    if (board && g_rotater_count > 0) {
        cEnt_update_constant_rotations();
    }
    /* v55m_7: Chomper state machine + rendering MUST run on main thread.
     * It calls D3D/Gfx functions (Timer_Init, Gfx_Scale, mesh vtable[7])
     * that need the D3D render context. Running from the background thread
     * causes a crash at 0x499D9D (D3DX SSE2 matrix multiply with NULL param). */
    if (board && g_chomper_count > 0) {
        cEnt_chomper_update(board);
    }
    /* v55n_22: DISABLED timebutton translate -- it's a NO-OP.
     * User log (v55n_22) showed 22x "TBtx tree count=0 (bad)" -- the built
     * collision Level's mw+0x18 tree is EMPTY (or trees built natively at ctor
     * pos). The proven crash-free solid sibling SpeedCylinder (case 39) does NO
     * geometry translate at all -- its solidity comes entirely from registering
     * obj+0x10E0 into board+0x10EC + scene tree + MeshBuffer+0x47C. TimeButton
     * now mirrors that exactly. Leaving this no-op disabled avoids 22x/frame
     * log spam and matches SpeedCylinder. */
#if 0  /* v55n_22: disabled -- no-op, mirrors SpeedCylinder */
    if (board && g_timebutton_count > 0 && *(BYTE*)(board + 0x874) == 0) {
        int ui;
        for (ui = 0; ui < g_timebutton_count; ui++) {
            TimeButtonState* tb = &g_timebuttons[ui];
            if (!tb || !tb->obj || tb->board != board) continue;
            if (tb->geom_translated) continue; /* latched once real translate succeeds */
            FILE* lf = NULL;
            fopen_s(&lf, g_log_path, "a");
            int n = cEnt_timebutton_translate_tree(tb, lf);
            if (lf) { fflush(lf); fclose(lf); }
            if (n > 0) tb->geom_translated = 1;
            /* n==0 -> tree items not built yet; leave latch 0 to retry next frame */
        }
    }
#endif /* v55n_22 */
#if 0  /* v55n_22: replaced by the vertex-source translate above (proven pattern) */
    if (board && g_timebutton_count > 0 && *(BYTE*)(board + 0x874) == 0) {
        int ui;
        for (ui = 0; ui < g_timebutton_count; ui++) {
            TimeButtonState* tb = &g_timebuttons[ui];
            if (!tb->obj || tb->geom_translated) continue;
            DWORD tb_col = tb->col_level ? tb->col_level : *(DWORD*)((char*)tb->obj + 0x10E0);
            if (!tb_col || IsBadReadPtr((void*)tb_col, 0x100)) continue;
            FILE* lf = NULL;
            fopen_s(&lf, g_log_path, "a");
            int n = cEnt_translate_collision_strips(tb_col, tb->x, tb->y, tb->z, lf);
            tb->geom_translated = 1;  /* latch AFTER translate */
            if (lf) { fflush(lf); fclose(lf); }
            if (n == 0) {
                /* 0 verts translated — likely buffers not built yet. Don't latch
                 * so we retry next frame. */
                tb->geom_translated = 0;
            }
        }
    }
#endif

    /* v55m_27i: Catapult trigger + launch force on main thread via Present hook.
     * Background-thread proximity check had wrong ball-list layout and wrong
     * ball offsets, so it never fired. */
    if (board && g_catapult_count > 0) {
        cEnt_catapult_present_check(board);
    }
    /* v55n_2: SpeedCylinder per-frame slot 11 driver.
     * Native SpeedCylinder spin/launch lives in vtable slot 11 (0x43D8C0).
     * The mod's entities are only in board+0x2578 (update list) which calls
     * vtable[1] (Rotator_Update vertex deformation), NOT slot 11. Drive slot 11
     * here per-frame on the main thread, gated on board+0x874 (not paused). */
    if (board && g_speedcyl_count > 0 && *(BYTE*)(board + 0x874) == 0) {
        int si;
        for (si = 0; si < g_speedcyl_count; si++) {
            DWORD sc = g_speedcyls[si].obj;
            if (sc && sc >= 0x10000 && !IsBadReadPtr((void*)sc, 0x60) && pfn_SpeedCyl_slot11) {
                pfn_SpeedCyl_slot11((void*)sc);
            }
        }
    }
    /* v55m_44l: Waterwheel CollisionLevel re-neutralization on MAIN THREAD.
     * 44k ran the re-neutralization only in the background thread, gated on
     * board==g_spawned_board. The 44k crash log (4 nodes neutralized, still
     * crash at 0x465789 during Background/FinishLoad) proves the game can
     * re-register/re-build CollisionLevel nodes between the background
     * thread's 16ms ticks — the renderer reaches a +0x430=0 node first.
     * The Present hook runs synchronously on the main thread EVERY frame
     * before any D3D render, so re-neutralizing here guarantees no
     * CollisionLevel node with broken component meshbuffers (0x7C, no
     * strip arrays at +0x418) is ever rendered. Idempotent + cheap. */
    /* v55m_44o: Manual waterwheel render + rotation on the MAIN THREAD.
     * The mesh is NOT registered in any game list (that triggered the
     * FinishLoad CollisionLevel crash) — we draw it here every frame via
     * the mesh's own vtable[18] (0x470150) with the rotation matrix.
     * This runs synchronously in the Present hook, before D3D Present,
     * so the wheel renders correctly every frame. */
    if (board && g_waterwheel_count > 0) {
        int wi;
        for (wi = 0; wi < g_waterwheel_count; wi++) {
            struct WaterWheelState* ww = &g_waterwheels[wi];
            if (ww->active && ww->pc_obj) {
                cEnt_waterwheel_present_render(ww);
            }
        }
    }
}

/* v55j_9: Ball_Render entry hook — runs AFTER Ball_Update.
 * Ball_Update clears ball+0x260 at 0x407AF4, so the tar splotch visual
 * disappears even though our App_ResetFrame hook set it. This hook
 * re-sets ball+0x260=1 if the ball was in a Gluebie zone this frame.
 * Ball_Render (0x403DB8) prologue: PUSH EBP; MOV EBP,ESP; PUSH ESI; ... 
 * Bytes: 55 8B EC 56 ... (need 5 for JMP) */
#define BALL_RENDER_HOOK_ADDR  0x00403DB8
#define BALL_RENDER_ORIG_BYTES 7
static BYTE *g_ballrender_cave = NULL;
static int   g_ballrender_hook_installed = 0;
static DWORD g_ballrender_ball_ptr = 0;  /* ESI at Ball_Render entry = ball */

static void __cdecl ballrender_helper(void) {
    if (game_is_quitting()) return;  /* v55j_16: check quit flag BEFORE accessing game memory */
    /* v55j_9: No longer needed — we create particles in ball+0x810 instead
     * of setting ball+0x260. Particles persist in the AthenaList and are
     * rendered by Ball_Render's existing particle loop. */
}

/* Function pointer for cave indirection */
static void (__cdecl *g_ballrender_fn_ptr)(void) = NULL;

/* Function pointer for the cave to call (indirection needed since the
 * C helper address isn't known at assembly time). */
static void (__cdecl *g_gluebie_fn_ptr)(void) = NULL;

static void install_present_hook(void) {
    if (g_present_hook_installed) return;
    BYTE *hook_addr = (BYTE*)PRESENT_HOOK_ADDR;
    /* App_ResetFrame prologue: PUSH ESI; MOV ESI,ECX; MOV ECX,[ESI+0x174] */
    BYTE expected[] = { 0x56, 0x8B, 0xF1, 0x8B, 0x8E, 0x74, 0x01, 0x00, 0x00 };
    if (memcmp(hook_addr, expected, 9) != 0) return;

    g_present_cave = (BYTE*)VirtualAlloc(NULL, 512, MEM_COMMIT | MEM_RESERVE,
                                          PAGE_EXECUTE_READWRITE);
    if (!g_present_cave) return;

    g_gluebie_fn_ptr = gluebie_present_helper;

    int p = 0;
    /* PUSHAD + PUSHFD */
    g_present_cave[p++] = 0x60;
    g_present_cave[p++] = 0x9C;
    /* CALL [g_gluebie_fn_ptr] */
    g_present_cave[p++] = 0xFF; g_present_cave[p++] = 0x15;
    *(DWORD*)(g_present_cave + p) = (DWORD)&g_gluebie_fn_ptr; p += 4;
    /* POPFD + POPAD */
    g_present_cave[p++] = 0x9D;
    g_present_cave[p++] = 0x61;
    /* Original 9 bytes: PUSH ESI; MOV ESI,ECX; MOV ECX,[ESI+0x174] */
    g_present_cave[p++] = 0x56;          /* PUSH ESI */
    g_present_cave[p++] = 0x8B; g_present_cave[p++] = 0xF1;  /* MOV ESI,ECX */
    g_present_cave[p++] = 0x8B; g_present_cave[p++] = 0x8E;  /* MOV ECX,[ESI+0x174] */
    *(DWORD*)(g_present_cave + p) = 0x00000174; p += 4;
    /* JMP back to hook_addr + 9 */
    g_present_cave[p++] = 0xE9;
    *(DWORD*)(g_present_cave + p) = (DWORD)(hook_addr + PRESENT_ORIG_BYTES)
                                     - (DWORD)(g_present_cave + p + 4);
    p += 4;

    DWORD old_protect;
    VirtualProtect(hook_addr, PRESENT_ORIG_BYTES, PAGE_EXECUTE_READWRITE, &old_protect);
    DWORD jmp_offset = (DWORD)(g_present_cave - hook_addr - 5);
    hook_addr[0] = 0xE9;
    *(DWORD*)(hook_addr + 1) = jmp_offset;
    /* NOP remaining 4 bytes (9 total - 5 for JMP = 4 NOPs) */
    hook_addr[5] = 0x90; hook_addr[6] = 0x90;
    hook_addr[7] = 0x90; hook_addr[8] = 0x90;
    VirtualProtect(hook_addr, PRESENT_ORIG_BYTES, old_protect, &old_protect);
    FlushInstructionCache(GetCurrentProcess(), hook_addr, PRESENT_ORIG_BYTES);
    g_present_hook_installed = 1;
}

/* v55j_9: Ball_Render entry hook (8 NOPs at 0x403DB8).
 * Runs AFTER Ball_Update (which clears ball+0x260 at 0x407AF4).
 * Re-sets ball+0x260=1 if this ball was in a Gluebie zone this frame.
 * ECX = ball at entry (Ball_Render is __thiscall). */
static void install_ballrender_hook(void) {
    if (g_ballrender_hook_installed) return;
    BYTE *hook_addr = (BYTE*)BALL_RENDER_HOOK_ADDR;
    /* Verify 8 NOPs at hook site */
    BYTE expected[] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
    if (memcmp(hook_addr, expected, 8) != 0) return;

    g_ballrender_cave = (BYTE*)VirtualAlloc(NULL, 512, MEM_COMMIT | MEM_RESERVE,
                                             PAGE_EXECUTE_READWRITE);
    if (!g_ballrender_cave) return;

    g_ballrender_fn_ptr = ballrender_helper;

    int p = 0;
    /* MOV [g_ballrender_ball_ptr], ECX  (6 bytes: 89 0D <addr32>) */
    g_ballrender_cave[p++] = 0x89;
    g_ballrender_cave[p++] = 0x0D;
    *(DWORD*)(g_ballrender_cave + p) = (DWORD)&g_ballrender_ball_ptr; p += 4;
    /* PUSHAD + PUSHFD */
    g_ballrender_cave[p++] = 0x60;
    g_ballrender_cave[p++] = 0x9C;
    /* CALL [g_ballrender_fn_ptr] */
    g_ballrender_cave[p++] = 0xFF; g_ballrender_cave[p++] = 0x15;
    *(DWORD*)(g_ballrender_cave + p) = (DWORD)&g_ballrender_fn_ptr; p += 4;
    /* POPFD + POPAD */
    g_ballrender_cave[p++] = 0x9D;
    g_ballrender_cave[p++] = 0x61;
    /* JMP back to 0x403DC0 (hook_addr + 8) */
    g_ballrender_cave[p++] = 0xE9;
    *(DWORD*)(g_ballrender_cave + p) = (DWORD)(hook_addr + BALL_RENDER_ORIG_BYTES)
                                     - (DWORD)(g_ballrender_cave + p + 4);
    p += 4;

    DWORD old_protect;
    VirtualProtect(hook_addr, BALL_RENDER_ORIG_BYTES, PAGE_EXECUTE_READWRITE, &old_protect);
    DWORD jmp_offset = (DWORD)(g_ballrender_cave - hook_addr - 5);
    hook_addr[0] = 0xE9;
    *(DWORD*)(hook_addr + 1) = jmp_offset;
    /* NOP remaining 3 bytes (8 total - 5 for JMP = 3 NOPs) */
    hook_addr[5] = 0x90; hook_addr[6] = 0x90; hook_addr[7] = 0x90;
    VirtualProtect(hook_addr, BALL_RENDER_ORIG_BYTES, old_protect, &old_protect);
    FlushInstructionCache(GetCurrentProcess(), hook_addr, BALL_RENDER_ORIG_BYTES);
    g_ballrender_hook_installed = 1;
}

static void uninstall_ballrender_hook(void) {
    if (!g_ballrender_hook_installed) return;
    BYTE *hook_addr = (BYTE*)BALL_RENDER_HOOK_ADDR;
    DWORD old_protect;
    if (VirtualProtect(hook_addr, BALL_RENDER_ORIG_BYTES, PAGE_EXECUTE_READWRITE, &old_protect)) {
        /* Restore 8 NOPs */
        memset(hook_addr, 0x90, BALL_RENDER_ORIG_BYTES);
        VirtualProtect(hook_addr, BALL_RENDER_ORIG_BYTES, old_protect, &old_protect);
        FlushInstructionCache(GetCurrentProcess(), hook_addr, BALL_RENDER_ORIG_BYTES);
    }
    g_ballrender_hook_installed = 0;
}

static void uninstall_present_hook(void) {
    if (!g_present_hook_installed) return;
    BYTE *hook_addr = (BYTE*)PRESENT_HOOK_ADDR;
    DWORD old_protect;
    if (VirtualProtect(hook_addr, PRESENT_ORIG_BYTES, PAGE_EXECUTE_READWRITE, &old_protect)) {
        /* Restore: PUSH ESI; MOV ESI,ECX; MOV ECX,[ESI+0x174] */
        hook_addr[0] = 0x56;          /* PUSH ESI */
        hook_addr[1] = 0x8B; hook_addr[2] = 0xF1;  /* MOV ESI,ECX */
        hook_addr[3] = 0x8B; hook_addr[4] = 0x8E;  /* MOV ECX,[ESI+0x174] */
        *(DWORD*)(hook_addr + 5) = 0x00000174;
        VirtualProtect(hook_addr, PRESENT_ORIG_BYTES, old_protect, &old_protect);
        FlushInstructionCache(GetCurrentProcess(), hook_addr, PRESENT_ORIG_BYTES);
    }
    g_present_hook_installed = 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * v55m_44p: In-game text system — draws status text like the score HUD.
 *
 * The existing PRESENT_HOOK_ADDR (0x46C200) is App_ResetFrame = the VIEWPORT
 * CLEAR. Text drawn there is erased. So we hook Graphics_PresentOrEnd
 * (0x455A90) instead, which runs AFTER slot 9 (viewport clear) and BEFORE
 * Present — text drawn here is visible on top of everything, exactly like the
 * score text at the top-left.
 *
 * Prologue (verified): 8A 44 24 04 | 83 EC 20  (MOV AL,[ESP+4]; SUB ESP,0x20)
 * = 7 bytes, two complete instructions → 7-byte trampoline, JMP back to +7.
 * Same PUSHAD/CALL C fn/POPAD + original bytes + JMP back pattern as the
 * existing frame hooks.
 * ═══════════════════════════════════════════════════════════════════════════ */

/* UI_DrawTextShadow_Wrapper (0x409B90): __thiscall, RET 0x3C (15 params).
 * Draws text with a drop shadow at top-left (x,y) — the score-HUD look.
 * Params 6 & 11 are overwritten internally (0x4CF300), pass 0. */
typedef void (__thiscall *UI_DrawTextShadow_t)(
    void* font, char* text, int x, int y,
    int shadow_dx, int shadow_dy, void* ign1,
    float text_r, float text_g, float text_b, float text_a,
    void* ign2, float shadow_r, float shadow_g, float shadow_b, float shadow_a);
static UI_DrawTextShadow_t pfn_UI_DrawTextShadow = (UI_DrawTextShadow_t)0x00409B90;

/* Font_MeasureText (0x456E20): __thiscall, RET 0x04 (1 stack param).
 * Returns the width of a text string in pixels (using the font's scale). */
typedef int (__thiscall *Font_MeasureText_t)(void* font, char* text);
static Font_MeasureText_t pfn_Font_MeasureText = (Font_MeasureText_t)0x00456E20;

#define PRESENTEND_HOOK_ADDR       0x00455A90
#define PRESENTEND_ORIG_BYTES      7   /* 8A 44 24 04 83 EC 20 */
static BYTE *g_presentend_cave = NULL;
static int g_presentend_hook_installed = 0;
static void (__cdecl *g_presentend_fn_ptr)(void) = NULL;

/* Vertical spacing (pixels) between each debug text line. */
static int debugTextSpacing = 24;

/* Runtime debug state — which custom entity the debug table documents.
 * Internal variable (no config file anymore, v55m_48d).
 *
 * v55m_49: Each cEnt found in the MESHWORLD section 3 gets its OWN
 * Custom Entity table. The tables are enumerated at draw time into
 * g_table_names[] (up to MAX_DEBUG_TABLES), and g_which_table indexes
 * into it — so the count of tables equals the count of cEnt entries
 * on the current level, and the numbers don't need to be continuous
 * (cEnt_001 + cEnt_025 => two tables). The A key (VK_A = 0x41) moves
 * to the PREVIOUS table, the D key (VK_D = 0x44) to the NEXT one. */
#define MAX_DEBUG_TABLES 999
static char  g_table_names[MAX_DEBUG_TABLES][64];
static int   g_table_count  = 0;
static char* g_debug_state  = g_table_names[0];  /* current table's cEnt name */
static int   g_which_table  = 0;

/* Whether to show the parenthetical detail text in debug sub-lines. Toggled
 * with the "i" key (rate-limited to 0.5s). */
static int g_show_paren_detail = 1;
static DWORD g_last_paren_toggle_tick = 0;

/* v55m_47q: hex <-> decimal view toggle for the debug value column. When
 * ON, DWORD values shown as "0x%08X" become decimal floats (0x1508 ->
 * 5384.0). v55m_48d: converted to decimal VALUE instead of IEEE-754 bit
 * reinterpretation. Toggled with the "H" key (rate-limited to 0.5s). */
static int   g_hex_to_float = 0;
static DWORD g_last_hex_toggle_tick = 0;

/* v55m_48d: function_display view toggle for the debug body text.
 * State 0 = show the function/subfunction NAME (simplified text).
 * State 1 = show the ADDRESS where the respective function/subfunction
 * is stored (e.g. "Sets Angle" <-> "obj+0x10E8", function addresses
 * shown as hex, field offsets shown as obj+0xNNNN).
 * Toggled with the "K" key (VK_K = 0x4B, rate-limited to 0.5s). */
static int   g_function_display = 0;
static DWORD g_last_func_toggle_tick = 0;

/* v55m_48d: T-key counter — whole string table visibility.
 * 0 = whole string table invisible, 1 = visible. Default 1 at game start. */
static int   g_table_visible = 1;
static DWORD g_last_table_toggle_tick = 0;

/* v55m_49: A/D keys move between the per-cEnt Custom Entity tables
 * (g_table_names[] / g_which_table). Rate-limited to 0.2s.
 * The P key (VK_P = 0x50) still toggles between the TWO property views
 * inside the currently selected Custom Entity table: 0 = the hierarchical
 * docs table, 1 = the live properties table. */
static DWORD g_last_table_prev_tick = 0;   /* A key (previous) */
static DWORD g_last_table_next_tick = 0;   /* D key (next) */
static int   g_which_props = 0;     /* 0 = docs table, 1 = properties table */
static DWORD g_last_props_toggle_tick = 0;

/* v55m_48d: Live values for the entity named by g_debug_state. Resolved on
 * the main thread each Present so the debug table can display the actual
 * runtime rotation fields (angle, direction, mesh path, etc.). A value is
 * "nonzero" (drawn in faint blue) or zero (drawn gray, same as the old
 * "0.0" placeholder). */
static int   g_dbg_entity_found = 0;      /* 1 = a spawned object matched */
static float g_dbg_angle = 0.0f;          /* obj+0x10E8 (per-frame delta) */
static float g_dbg_direction = 0.0f;      /* obj+0x10EC */
/* v55m_51: obj+0x10E4 = the ACCUMULATOR (degrees) that actually drives the
 * visible rotation (Gfx_ScaleX(accum)). The per-frame delta (obj+0x10E8) is
 * capped at 1.0 by v55m_50, so showing only the delta makes every angle
 * "pin" at 1.00. The accumulator is the real live spin value. */
static float g_dbg_accum = 0.0f;          /* obj+0x10E4 (accumulator, deg) */
static int   g_dbg_in_update_list = 0;    /* obj present in board+0x2578 */
static int   g_dbg_in_render_list = 0;    /* obj present in board+0xCD4 */
static int   g_dbg_in_collision_list = 0; /* obj present in board+0x10EC */
static float g_dbg_rot_x = 0.0f, g_dbg_rot_y = 0.0f, g_dbg_rot_z = 0.0f;
static float g_dbg_ros_x = 0.0f, g_dbg_ros_y = 0.0f, g_dbg_ros_z = 0.0f;
static float g_dbg_rot_a = 0.0f, g_dbg_rot_d = 0.0f;
static int   g_dbg_rot_m = 0;
static char  g_dbg_mesh_path[128] = "";
/* v55m_48d: live properties for the 2nd properties table. Position comes
 * straight from the object (obj+0x10D4/10D8/10DC); per-axis accumulated
 * angles + rotation speeds come from the mod-side RotaterConfig. */
static float g_dbg_pos_x = 0.0f, g_dbg_pos_y = 0.0f, g_dbg_pos_z = 0.0f;
static float g_dbg_ang_x = 0.0f, g_dbg_ang_y = 0.0f, g_dbg_ang_z = 0.0f;
static float g_dbg_scl_x = 0.0f, g_dbg_scl_y = 0.0f, g_dbg_scl_z = 0.0f;
/* v55m_48d: pointer to the matched S1 ref-point entry (0x7C bytes).
 * The S1 entry carries the per-axis SCALE at +0x1C/+0x20/+0x24 (default
 * 1.0/1.0/1.0 — hardcoded by the game's ref-point ctor in FUN_004629e0:
 * ppvVar5[7]=ppvVar5[8]=ppvVar5[9]=0x3f800000). The spawned object itself
 * has no stored scale (size is baked into the mesh vertices), so the ref
 * point is the only per-axis scale source. Captured at title-resolve time. */
static DWORD g_dbg_s1_entry = 0;
/* Faint blue used for live values that are != 0.0 (v55m_47z — user asked
 * for "the TARGET blue"; simplified to a plain faint blue). */
#define DEBUG_VALUE_BLUE_R  0.47f
#define DEBUG_VALUE_BLUE_G  0.71f
#define DEBUG_VALUE_BLUE_B  1.00f

/* Debug table vertical scroll. W moves the table up (smaller Y), S moves it
 * down (larger Y). W clamps at 0 = initial position (can't go above the top);
 * S clamps at 300 (enough to bring the last section fully into view). */
static int   g_debug_scroll_y = 0;
static DWORD g_last_scroll_tick = 0;

/* The last string of the Rotator table now sits at line 93 (4 sections with
 * sub-sublines: title + 4 subs × (1+4) + ... + 3 blank separators + title at
 * line 0 + sections start at line 2 → last drawn line 93). Baseline Y at
 * scroll=0 is 12 + 36 + 93*24 = 2280. The S-key (up) limit keeps the last
 * string at least 500px from the top of the screen, so the table can only be
 * scrolled up to scroll_y = 500 - 2280 = -1780. */
#define ROT_LAST_STRING_Y0  2280
#define ROT_MAX_UP_SCROLL   (500 - ROT_LAST_STRING_Y0)   /* -1780 */

/* =====================================================================
 * v55m_48d: resolve live runtime values for the entity named by
 * g_debug_state. Run from the main thread at Present time.
 *
 * The spawned entity is found via the mod's OWN g_rotater_cfg[] tracking
 * array (v55m_48d — much more reliable than scanning board lists): each
 * entry carries the exact spawned object pointer; we match the S3 entry
 * position against the object's spawn position (constructor-dependent
 * offsets, verified via Ghidra), then read live values straight off the
 * object and verify its presence in the update/render/collision lists.
 * ===================================================================== */
static void cEnt_resolve_debug_values(DWORD board, float want_x, float want_y, float want_z) {
    g_dbg_entity_found = 0;
    g_dbg_angle = 0.0f;
    g_dbg_direction = 0.0f;
    g_dbg_accum = 0.0f;
    g_dbg_in_update_list = 0;
    g_dbg_in_render_list = 0;
    g_dbg_in_collision_list = 0;
    g_dbg_rot_x = g_dbg_rot_y = g_dbg_rot_z = 0.0f;
    g_dbg_ros_x = g_dbg_ros_y = g_dbg_ros_z = 0.0f;
    g_dbg_rot_a = g_dbg_rot_d = 0.0f;
    g_dbg_mesh_path[0] = 0;
    g_dbg_pos_x = g_dbg_pos_y = g_dbg_pos_z = 0.0f;
    g_dbg_ang_x = g_dbg_ang_y = g_dbg_ang_z = 0.0f;
    g_dbg_scl_x = g_dbg_scl_y = g_dbg_scl_z = 0.0f;
    if (!board) return;

    /* v55m_48d: The mod tracks every spawned rotater in g_rotater_cfg[].
     * Match the S3 entry position against each cfg entry's spawn position.
     * The cfg's `obj` gives us the EXACT spawned object pointer — no fragile
     * list-scan-by-position needed. We then read live values straight off
     * the object and verify it's present in the update/render/collision lists. */
    DWORD update_list = board + 0x2578;
    DWORD render_list  = board + 0xCD4;
    DWORD coll_list    = board + 0x10EC;

    int ci;
    for (ci = 0; ci < g_rotater_count && ci < MAX_ROTATERS; ci++) {
        DWORD obj = g_rotater_cfg[ci].obj;
        if (!obj || obj < 0x10000) continue;
        if (IsBadReadPtr((void*)obj, 0x10F0)) continue;
        DWORD vtable = *(DWORD*)obj;
        if (vtable < 0x400000) continue;

        /* Position offsets differ by constructor (verified via Ghidra):
         *   PopCylinder (0x4D58F0) / Rotator (0x4D5708):  obj+0x10D4/10D8/10DC
         *   SWIRL (0x4D5518):  obj+0x10D8/10DC/10E0  (0x10D4 is collision ptr)
         * Read the object's spawn position accordingly. */
        float ox, oy, oz;
        if (vtable == 0x004D5518) {
            ox = *(float*)(obj + 0x10D8);
            oy = *(float*)(obj + 0x10DC);
            oz = *(float*)(obj + 0x10E0);
        } else {
            ox = *(float*)(obj + 0x10D4);
            oy = *(float*)(obj + 0x10D8);
            oz = *(float*)(obj + 0x10DC);
        }

        /* Match against the S3 entry position (within 2.0). */
        float dx = ox - want_x; if (dx < 0) dx = -dx;
        float dy = oy - want_y; if (dy < 0) dy = -dy;
        float dz = oz - want_z; if (dz < 0) dz = -dz;
        if (dx > 2.0f || dy > 2.0f || dz > 2.0f) continue;

        /* Found it — this cfg entry is our tracked entity. */
        g_dbg_entity_found = 1;
        g_dbg_angle = *(float*)(obj + 0x10E8);
        g_dbg_direction = *(float*)(obj + 0x10EC);
        /* v55m_51: read the LIVE accumulator (obj+0x10E4, degrees) — the
         * value that actually drives the visible rotation. */
        g_dbg_accum = *(float*)(obj + 0x10E4);
        g_dbg_pos_x = ox; g_dbg_pos_y = oy; g_dbg_pos_z = oz;

        /* Surface the spawn-time config. */
        g_dbg_rot_x = g_rotater_cfg[ci].rot_x;
        g_dbg_rot_y = g_rotater_cfg[ci].rot_y;
        g_dbg_rot_z = g_rotater_cfg[ci].rot_z;
        g_dbg_ros_x = g_rotater_cfg[ci].ros_x;
        g_dbg_ros_y = g_rotater_cfg[ci].ros_y;
        g_dbg_ros_z = g_rotater_cfg[ci].ros_z;
        g_dbg_rot_a = g_rotater_cfg[ci].rot_a;
        g_dbg_rot_d = g_rotater_cfg[ci].rot_d;
        g_dbg_rot_m = g_rotater_cfg[ci].rot_m;
        /* v55m_48d: per-axis angles — obj+0x10E8 drives the native Y-axis
         * render; direction + initial angle feed the other axes. Scale =
         * the rot_x/y/z speeds that drive Gfx_Scale.
         * v55m_51: Angle X now shows the LIVE accumulator (obj+0x10E4,
         * degrees — the value Gfx_ScaleX consumes for the visible spin).
         * Angle Y keeps the per-frame delta (obj+0x10E8), Angle Z the
         * direction multiplier (obj+0x10EC). */
        g_dbg_ang_x = g_dbg_accum;        /* live accumulated angle (deg) */
        g_dbg_ang_y = g_dbg_angle;        /* per-frame delta */
        g_dbg_ang_z = g_dbg_direction;    /* direction multiplier */
        /* v55m_48d: Scale X/Y/Z come from the matched S1 ref-point entry
         * (+0x1C/+0x20/+0x24, default 1.0/1.0/1.0 — hardcoded by the game's
         * ref-point ctor). The spawned object itself has no stored scale
         * (size is baked into the mesh vertices). Guard reads. */
        if (g_dbg_s1_entry && !IsBadReadPtr((void*)g_dbg_s1_entry, 0x28)) {
            g_dbg_scl_x = *(float*)(g_dbg_s1_entry + 0x1C);
            g_dbg_scl_y = *(float*)(g_dbg_s1_entry + 0x20);
            g_dbg_scl_z = *(float*)(g_dbg_s1_entry + 0x24);
        }
        strncpy(g_dbg_mesh_path, g_rotater_cfg[ci].mesh_path, 127);
        g_dbg_mesh_path[127] = 0;

        /* Verify presence in update / render / collision lists (board lists). */
        {
            DWORD lists[3]; int* flags[3];
            lists[0] = update_list;  flags[0] = &g_dbg_in_update_list;
            lists[1] = render_list;  flags[1] = &g_dbg_in_render_list;
            lists[2] = coll_list;    flags[2] = &g_dbg_in_collision_list;
            int li;
            for (li = 0; li < 3; li++) {
                DWORD lst = lists[li];
                if (IsBadReadPtr((void*)(lst + 0x04), 4)) continue;
                int lc = *(int*)(lst + 0x04);
                if (lc <= 0 || lc > 10000) continue;
                if (IsBadReadPtr((void*)(lst + 0x40C), 4)) continue;
                DWORD* ld = *(DWORD**)(lst + 0x40C);
                if (!ld || IsBadReadPtr(ld, lc * 4)) continue;
                int j;
                for (j = 0; j < lc; j++) {
                    if (ld[j] == obj) { *flags[li] = 1; break; }
                }
            }
        }
        return;  /* first match wins */
    }
}


/* Format a DWORD as hex ("0x%08X") or, when the H-key float view is active,
 * as its numeric value in decimal float form. v55m_48d: fixed to convert
 * the hex VALUE to decimal (0x1508 -> 5384.0) instead of reinterpreting
 * the bit pattern as an IEEE-754 float (which gave denormals like 8.6e-42). */
static void cEnt_dbg_format_dword(DWORD v, char* out, int outsz) {
    if (g_hex_to_float) {
        snprintf(out, outsz, "%.1f", (double)v);
    } else {
        snprintf(out, outsz, "0x%08X", v);
    }
}

/* Format the live runtime value for a specific debug-table line.
 * (s,i,k) = (section index, sub index, sub-sub index). Returns a pointer
 * to a static buffer, or NULL if that line has no live value (in which
 * case the draw loop shows the gray "0.0" placeholder).
 * v55m_48d: these are the "respective value of the respective function
 * on their left". */
static const char* cEnt_debug_value(int s, int i, int k) {
    static char vb[160];
    switch (s) {
        case 0:  /* 1 - Constructors */
            if (i == 0) {                 /* Operator_new */
                if (k == 0) { cEnt_dbg_format_dword((DWORD)pfn_operator_new, vb, sizeof(vb)); return vb; }
                if (k == 1) { cEnt_dbg_format_dword((DWORD)ROTATER_SIZE, vb, sizeof(vb)); return vb; }
                if (k == 2) { snprintf(vb, sizeof(vb), g_dbg_entity_found ? "called" : "idle"); return vb; }
                if (k == 3) { snprintf(vb, sizeof(vb), g_dbg_entity_found ? "installed" : "idle"); return vb; }
            }
            if (i == 1) {                 /* Rotator_ctor */
                if (k == 1) { cEnt_dbg_format_dword(0x004D5518u, vb, sizeof(vb)); return vb; }
                if (k == 2) { snprintf(vb, sizeof(vb), "%.2f", g_dbg_ctor_angle); return vb; }
                if (k == 3) { snprintf(vb, sizeof(vb), "%.2f", g_dbg_ctor_direction); return vb; }
            }
            if (i == 2 && k == 0) {       /* cEnt_load_mesh_file */
                snprintf(vb, sizeof(vb), g_dbg_mesh_path[0] ? "%s" : "(default)", g_dbg_mesh_path[0] ? g_dbg_mesh_path : "");
                return vb;
            }
            if (i == 3) {                 /* Mesh fallback */
                if (k == 1) { snprintf(vb, sizeof(vb), g_dbg_mesh_path[0] ? "primary" : "Swirl"); return vb; }
                if (k == 2) { snprintf(vb, sizeof(vb), g_dbg_entity_found ? "used" : "idle"); return vb; }
            }
            return NULL;
        case 1:  /* 2 - Registration */
            if (i == 0 && k == 0) { snprintf(vb, sizeof(vb), g_dbg_in_update_list ? "yes" : "no"); return vb; }
            if (i == 1 && k == 0) { snprintf(vb, sizeof(vb), g_dbg_in_render_list ? "yes" : "no"); return vb; }
            if (i == 2 && k == 0) { snprintf(vb, sizeof(vb), g_dbg_in_collision_list ? "yes" : "no"); return vb; }
            if (i == 3 && k == 0) { snprintf(vb, sizeof(vb), g_dbg_entity_found ? "yes" : "no"); return vb; }
            return NULL;
        case 2:  /* 3 - Updates */
            if (i == 0 && k == 0) {       /* angle += direction * 0.004 */
                snprintf(vb, sizeof(vb), "%.2f", g_dbg_angle);
                return vb;
            }
            if (i == 1 && k == 0) {       /* accumulated angle (obj+0x10E4) */
                /* v55m_51: show the LIVE accumulator — the per-frame delta
                 * (obj+0x10E8) is capped at 1.0 and would pin at 1.00. */
                snprintf(vb, sizeof(vb), "%.2f", g_dbg_accum);
                return vb;
            }
            if (i == 2 && k == 0) {       /* direction multiplier */
                snprintf(vb, sizeof(vb), "%.2f", g_dbg_direction);
                return vb;
            }
            if (i == 4 && k == 0) {       /* constant spin (ROS_Y=0) */
                snprintf(vb, sizeof(vb), g_dbg_ros_y == 0.0f ? "active" : "inactive");
                return vb;
            }
            if (i == 5) {                 /* RotaterConfig struct */
                if (k == 0) { snprintf(vb, sizeof(vb), g_dbg_mesh_path[0] ? "%s" : "(empty)", g_dbg_mesh_path[0] ? g_dbg_mesh_path : ""); return vb; }
                if (k == 1) { snprintf(vb, sizeof(vb), "%.2f / %.2f / %.2f", g_dbg_rot_x, g_dbg_rot_y, g_dbg_rot_z); return vb; }
                if (k == 2) { snprintf(vb, sizeof(vb), "%.2f / %.2f / %.2f", g_dbg_ros_x, g_dbg_ros_y, g_dbg_ros_z); return vb; }
                if (k == 3) { snprintf(vb, sizeof(vb), "%.2f / %.2f / %d", g_dbg_rot_a, g_dbg_rot_d, g_dbg_rot_m); return vb; }
            }
            return NULL;
        case 3:  /* 4 - Lifecycle */
            if (i == 0 && k == 0) { snprintf(vb, sizeof(vb), g_dbg_entity_found ? "yes" : "no"); return vb; }
            if (i == 0 && k == 1) { snprintf(vb, sizeof(vb), "%d", g_rotater_count); return vb; }
            if (i == 1 && k == 2) { snprintf(vb, sizeof(vb), g_dbg_entity_found ? "active" : "idle"); return vb; }
            return NULL;
        default:
            return NULL;
    }
}

/* Draw text with a double shadow, scaled to `scale`. The font's scale
 * (font+0x428) is temporarily set to `scale`, then restored. Shadows:
 * primary bottom-right (2,2) and secondary top-left (-2,-2) — 0.8x the
 * original (3,3)/(-2,-2) to match the scaled-down text. */
static void cEnt_draw_text_double(void* font, const char* text, int x, int y,
                                  float r, float g, float b, float scale) {
    float* fp_scale = (float*)((char*)font + 0x428);
    float orig_scale = *fp_scale;
    *fp_scale = scale;
    /* secondary shadow — top-left (0,-2) — horizontal offset 0 */
    pfn_UI_DrawTextShadow(font, (char*)text, x, y, 0, -2,
                          0, r, g, b, 1.0f,
                          0, 0.0f, 0.0f, 0.0f, 1.0f);
    /* primary shadow — bottom-right (0,3) — horizontal offset 0 */
    pfn_UI_DrawTextShadow(font, (char*)text, x, y, 0, 3,
                          0, r, g, b, 1.0f,
                          0, 0.0f, 0.0f, 0.0f, 1.0f);
    *fp_scale = orig_scale;
}

/* Draw "label" in white, then a colored "value" immediately to its right.
 * Both use the score-HUD font/size and shadow. Returns the total line width. */
static int cEnt_draw_label_value(void* font, const char* label, const char* value,
                                 int x, int y,
                                 float vr, float vg, float vb) {
    int label_w = pfn_Font_MeasureText(font, (char*)label);
    cEnt_draw_text_double(font, label, x, y, 1.0f, 1.0f, 1.0f, 0.9f);
    cEnt_draw_text_double(font, value, x + label_w, y, vr, vg, vb, 0.9f);
    return label_w + pfn_Font_MeasureText(font, (char*)value);
}

/* Draw a debug line where any parenthetical detail is rendered in gray and
 * preceded by 4 spaces before the opening parenthesis. E.g.
 *   "Operator_new    (memory reserve)"
 *                    ^^^^  gray, 4-space pad
 * The base text (before the first '(') is drawn in white; the parenthetical
 * part (from '(' to end) is drawn gray, 4 space-characters after the base.
 * `font`, `x`, `y` are the draw position; `scale` is the font scale. If the
 * parenthetical toggle is off, only the white base is drawn and the trailing
 * spaces are trimmed. */
static int cEnt_draw_paren_line(void* font, const char* text, int x, int y, float scale,
                                int paren_x, const char* value) {
    char base[128];
    char paren[96];
    int i = 0;
    /* Extract base = text up to first '(' (trimmed of trailing spaces). */
    while (text[i] && text[i] != '(' && i < 127) {
        base[i] = text[i];
        i++;
    }
    base[i] = 0;
    /* Remember the position of the '(' (if any) BEFORE trimming. */
    int paren_pos = i;
    /* Trim trailing spaces from base. */
    while (i > 0 && (base[i-1] == ' ' || base[i-1] == '\t')) base[--i] = 0;

    /* Extract the parenthetical part including its '(' char, if any. */
    paren[0] = 0;
    if (paren_pos < 128 && text[paren_pos] == '(') {
        int j = 0;
        int k = paren_pos;
        while (text[k] && j < 95) {
            paren[j++] = text[k++];
        }
        paren[j] = 0;
    }

    /* Draw base in white. */
    cEnt_draw_text_double(font, base, x, y, 1.0f, 1.0f, 1.0f, scale);

    /* Two states toggled with the "i" key:
     *   Status 1 (g_show_paren_detail) — the parenthetical gray text is
     *     visible at the aligned column.
     *   Status 0 (!g_show_paren_detail) — the parens are invisible; instead
     *     draw the live runtime value at the same column. If no value is
     *     supplied (NULL), draw the old gray "0.0" placeholder.
     * All gray/blue texts are shifted 50px further right (v55m_48d).
     * v55m_48d: live values != 0 draw in faint blue; "0.0" stays gray. */
    if (paren_x > 0) {
        if (g_show_paren_detail) {
            if (paren[0]) {
                cEnt_draw_text_double(font, paren, paren_x + 50, y,
                                      0.6f, 0.6f, 0.6f, scale);  /* gray paren */
            }
        } else {
            /* v55m_48d: lines with NO live value (value == NULL) show the
             * gray "N/A" placeholder (same gray as the old "0.0"). Lines
             * whose actual runtime value is 0.0 keep showing "0.0" (gray);
             * nonzero values are drawn in faint blue. */
            if (!value) {
                cEnt_draw_text_double(font, "N/A", paren_x + 50, y,
                                      0.6f, 0.6f, 0.6f, scale);  /* gray */
            } else {
                int nonzero = 0;
                const char* p = value;
                while (*p) {
                    /* nonzero if any char is outside the zero-ish set
                     * (digits 1-9, or letters for yes/no, active/idle,
                     * mesh names, hex, etc.). */
                    if ((*p >= '1' && *p <= '9') ||
                        (*p >= 'a' && *p <= 'z') ||
                        (*p >= 'A' && *p <= 'Z') ||
                        *p == 'x' || *p == 'X') { nonzero = 1; break; }
                    p++;
                }
                if (nonzero) {
                    cEnt_draw_text_double(font, value, paren_x + 50, y,
                                          DEBUG_VALUE_BLUE_R, DEBUG_VALUE_BLUE_G,
                                          DEBUG_VALUE_BLUE_B, scale);  /* faint blue */
                } else {
                    cEnt_draw_text_double(font, value, paren_x + 50, y,
                                          0.6f, 0.6f, 0.6f, scale);  /* gray */
                }
            }
        }
    }
    return 0;
}

/* Measure the width (in pixels) of the base part of a "label (paren)" line,
 * up to but not including the '(' — at the given font scale. Returns 0 if
 * there is no '(' in the text. */
static int cEnt_paren_line_base_width(void* font, const char* text, float scale) {
    char base[128];
    int i = 0;
    while (text[i] && text[i] != '(' && i < 127) {
        base[i] = text[i];
        i++;
    }
    if (text[i] != '(') return 0;
    base[i] = 0;
    while (i > 0 && (base[i-1] == ' ' || base[i-1] == '\t')) base[--i] = 0;

    /* Measure at the given scale (Font_MeasureText reads font+0x428). */
    float* fp_scale = (float*)((char*)font + 0x428);
    float orig_scale = *fp_scale;
    *fp_scale = scale;
    int w = pfn_Font_MeasureText(font, base);
    *fp_scale = orig_scale;
    return w;
}

/* Draw the mod's status text. Runs on the main thread at Present time.
 * Font is read live every frame (it's NULL until the resource loader
 * finishes). The font validity check (App+0x318) is the gate — once the
 * game has a font, it's rendering scenes, so text is visible. (We do NOT
 * gate on get_board(), which returns 0 on Wine setups where the profile
 * is NULL — the native score text draws whenever a font exists.) */
static void __cdecl cEnt_draw_text_helper(void) {
    if (game_is_quitting()) return;  /* v55j_16: check quit flag before touching game memory */

    DWORD app = *(DWORD*)GLOBAL_APP_PTR;
    if (!app || app < 0x10000 || IsBadReadPtr((void*)app, 0x400)) return;
    void* font = *(void**)((char*)app + 0x320);   /* showcardgothic16 — score counter / info text font (same size as the score HUD) */
    if (!font || IsBadReadPtr(font, 0x500)) return;

    /* Only process keybinds while the game window is focused. GetAsyncKeyState
     * reads the GLOBAL keyboard state — without this gate the toggles/scroll
     * fire even when the window is minimized or we alt-tabbed away. Check:
     * (1) the calling thread has an active window, (2) it isn't minimized,
     * (3) it is the foreground (user-facing) window. */
    HWND hwnd_active = GetActiveWindow();
    int window_focused =
        (hwnd_active != NULL) &&
        !IsIconic(hwnd_active) &&
        (GetForegroundWindow() == hwnd_active);

    /* Toggle parenthetical detail text with the "i" key, rate-limited to 0.5s.
     * GetAsyncKeyState bit 0x0001 = key press edge. We gate on elapsed time so
     * one hold = one toggle. */
    if (window_focused) {
        DWORD now = GetTickCount();
        if ((GetAsyncKeyState(0x49) & 0x0001) && (now - g_last_paren_toggle_tick) >= 500) {  /* VK_I = 0x49 */
            g_show_paren_detail = !g_show_paren_detail;
            g_last_paren_toggle_tick = now;
        }
        /* Toggle hex <-> decimal view of the debug value column with the "H"
         * key (VK_H = 0x48), same rate limit. */
        if ((GetAsyncKeyState(0x48) & 0x0001) && (now - g_last_hex_toggle_tick) >= 500) {  /* VK_H = 0x48 */
            g_hex_to_float = !g_hex_to_float;
            g_last_hex_toggle_tick = now;
        }
        /* Toggle function_display (name <-> address) with the "K" key
         * (VK_K = 0x4B), same rate limit. */
        if ((GetAsyncKeyState(0x4B) & 0x0001) && (now - g_last_func_toggle_tick) >= 500) {  /* VK_K = 0x4B */
            g_function_display = !g_function_display;
            g_last_func_toggle_tick = now;
        }
        /* Toggle whole-table visibility with the "T" key (VK_T = 0x54).
         * 0 = invisible, 1 = visible. Default 1 at game start. */
        if ((GetAsyncKeyState(0x54) & 0x0001) && (now - g_last_table_toggle_tick) >= 500) {  /* VK_T = 0x54 */
            g_table_visible = !g_table_visible;
            g_last_table_toggle_tick = now;
        }
        /* v55n_6: Alternate whole-table visibility toggle with the "X" key
         * (VK_X = 0x58) — same behaviour as T, same rate limit. */
        if ((GetAsyncKeyState(0x58) & 0x0001) && (now - g_last_table_toggle_tick) >= 500) {  /* VK_X = 0x58 */
            g_table_visible = !g_table_visible;
            g_last_table_toggle_tick = now;
        }
        /* v55m_49: Move between the per-cEnt Custom Entity tables.
         * A key (VK_A = 0x41) = PREVIOUS table, D key (VK_D = 0x44) = NEXT
         * table. The tables wrap around (after the last comes the first).
         * g_table_count is refreshed every Present from the level's
         * section 3 cEnt entries — a level with cEnt_001 + cEnt_025 has
         * exactly two tables. */
        if ((GetAsyncKeyState(0x41) & 0x0001) && (now - g_last_table_prev_tick) >= 200) {  /* VK_A = 0x41: previous */
            if (g_table_count > 0) {
                g_which_table = (g_which_table - 1 + g_table_count) % g_table_count;
            }
            g_last_table_prev_tick = now;
        }
        if ((GetAsyncKeyState(0x44) & 0x0001) && (now - g_last_table_next_tick) >= 200) {  /* VK_D = 0x44: next */
            if (g_table_count > 0) {
                g_which_table = (g_which_table + 1) % g_table_count;
            }
            g_last_table_next_tick = now;
        }
        /* Toggle the property view inside the current Custom Entity table
         * with the P key (VK_P = 0x50): 0 = hierarchical docs table,
         * 1 = live properties table. Default 0. */
        if ((GetAsyncKeyState(0x50) & 0x0001) && (now - g_last_props_toggle_tick) >= 500) {  /* VK_P = 0x50 */
            g_which_props = !g_which_props;
            g_last_props_toggle_tick = now;
        }
    }

    /* Scroll the debug table with S (up) / W (down), rate-limited to 100ms.
     * Uses the held-state bit (0x8000) so holding the key scrolls smoothly.
     * W (down) is limited to the table's ORIGINAL position: pressing W while
     * the table is already at Y=0 (its start position) does nothing — it can
     * never be pushed below where it started. S (up) is unlimited. */
    if (window_focused) {
        DWORD now2 = GetTickCount();
        if ((now2 - g_last_scroll_tick) >= 10) {
            if (GetAsyncKeyState(0x57) & 0x8000) {          /* VK_W = 0x57: move down */
                if (g_debug_scroll_y < 0) {   /* limit: cannot go below original (0) */
                    g_debug_scroll_y += 12;   /* v55m_48d: 2x scroll speed */
                    g_last_scroll_tick = now2;
                }
            } else if (GetAsyncKeyState(0x53) & 0x8000) {   /* VK_S = 0x53: move up — limited */
                /* S limit: don't let the last string (line 22, baseline 576 at
                 * scroll=0) come within 500px of the top of the screen. */
                if (g_debug_scroll_y - 12 >= ROT_MAX_UP_SCROLL) {   /* last string still >= 500px from top */
                    g_debug_scroll_y -= 12;   /* v55m_48d: 2x scroll speed */
                    g_last_scroll_tick = now2;
                }
            }
        }
    }

    /* On the loading / main menu screen (no active board/race), draw the
     * mod's "Custom Events + version" status table. get_board() returns 0
     * when there's no scene active — i.e. on the menu or during level load.
     * Gated on g_table_visible (T key): 0 hides the whole table. */
    if (!get_board()) {
        if (g_table_visible) {
            cEnt_draw_text_double(font, "Custom Entities Mod v55n_22", 20, 12,
                                  1.0f, 1.0f, 1.0f, 0.9f);
        }
        return;
    }

    /* Resolve the entity's display title: "Custom Entity XXX - (EntityName)".
     * XXX = the number after "cEnt_" in the section 3 object name, EntityName
     * = the string inside <ENTITY>...</ENTITY> in that same object's name.
     * Falls back to "Custom Entity Debug" if the entity can't be resolved.
     *
     * v55m_49: Pass 1 ENUMERATES every section 3 cEnt entry into
     * g_table_names[]/g_table_count — each cEnt gets its OWN Custom Entity
     * table, so a level with cEnt_001 + cEnt_025 has exactly two tables
     * (numbers need not be continuous). Pass 2 resolves the title / live
     * values for the table selected by g_which_table (A/D keys). */
    static char g_debug_title[96] = "";
    static float g_dbg_want_x = 0.0f, g_dbg_want_y = 0.0f, g_dbg_want_z = 0.0f;
    static int   g_dbg_want_set = 0;
    {
        DWORD board2 = get_board();
        DWORD sceneobj = cEnt_get_sceneobj(board2);
        int resolved = 0;
        g_dbg_want_set = 0;

        /* Pass 1 — enumerate all cEnt tables from section 3. */
        g_table_count = 0;
        if (sceneobj && !IsBadReadPtr((void*)(sceneobj + SCENEOBJ_OBJ_COUNT), 4)) {
            int obj_count = *(int*)(sceneobj + SCENEOBJ_OBJ_COUNT);
            if (obj_count > 0 && obj_count <= 1000 &&
                !IsBadReadPtr((void*)(sceneobj + SCENEOBJ_OBJ_ARRAY), 4)) {
                DWORD* obj_array_ptr = *(DWORD**)(sceneobj + SCENEOBJ_OBJ_ARRAY);
                if (obj_array_ptr && !IsBadReadPtr(obj_array_ptr, obj_count * 4)) {
                    int i;
                    for (i = 0; i < obj_count && g_table_count < MAX_DEBUG_TABLES; i++) {
                        DWORD obj_ptr = obj_array_ptr[i];
                        if (!obj_ptr || obj_ptr < 0x10000) continue;
                        if (IsBadReadPtr((void*)obj_ptr, 20)) continue;
                        char* name = *(char**)(obj_ptr);
                        if (!name || IsBadReadPtr(name, 8)) continue;

                        /* Match the cEnt prefix (e.g. "cEnt_001"), optionally
                         * as "REF:cEnt_001". Extract the base name up to the
                         * first separator (space/tab/'<'/NUL). */
                        const char* nm = name;
                        if (_strnicmp(nm, "REF:", 4) == 0) nm += 4;
                        if (_strnicmp(nm, "cEnt", 4) != 0) continue;

                        /* Copy the full "cEnt_XXX" base name into table slot. */
                        size_t nlen = 0;
                        while (nm[nlen] && nm[nlen] != ' ' && nm[nlen] != '\t' &&
                               nm[nlen] != '<' && nlen < 63)
                            nlen++;
                        if (nlen < 5) continue;   /* at least "cEnt_X" */
                        memcpy(g_table_names[g_table_count], nm, nlen);
                        g_table_names[g_table_count][nlen] = 0;
                        g_table_count++;
                    }
                }
            }
        }

        /* Clamp the selected index to the (possibly changed) table count. */
        if (g_table_count <= 0) {
            g_which_table = 0;
            g_debug_state = g_table_names[0];
        } else {
            if (g_which_table >= g_table_count) g_which_table = g_table_count - 1;
            if (g_table_names[g_which_table][0] == 0) g_which_table = 0;
            g_debug_state = g_table_names[g_which_table];
        }

        /* Pass 2 — resolve the currently selected table's entry. */
        if (sceneobj && !IsBadReadPtr((void*)(sceneobj + SCENEOBJ_OBJ_COUNT), 4)) {
            int obj_count = *(int*)(sceneobj + SCENEOBJ_OBJ_COUNT);
            if (obj_count > 0 && obj_count <= 1000 &&
                !IsBadReadPtr((void*)(sceneobj + SCENEOBJ_OBJ_ARRAY), 4)) {
                DWORD* obj_array_ptr = *(DWORD**)(sceneobj + SCENEOBJ_OBJ_ARRAY);
                if (obj_array_ptr && !IsBadReadPtr(obj_array_ptr, obj_count * 4)) {
                    int i;
                    for (i = 0; i < obj_count; i++) {
                        DWORD obj_ptr = obj_array_ptr[i];
                        if (!obj_ptr || obj_ptr < 0x10000) continue;
                        if (IsBadReadPtr((void*)obj_ptr, 20)) continue;
                        char* name = *(char**)(obj_ptr);
                        if (!name || IsBadReadPtr(name, 8)) continue;
                        /* Match the cEnt name exactly (e.g. "cEnt_001"),
                         * optionally as "REF:cEnt_001". The name may have
                         * tags after it — require a separator right after. */
                        int match = 0;
                        const char* nm = name;
                        if (_strnicmp(nm, "REF:", 4) == 0) nm += 4;
                        if (_strnicmp(nm, "cEnt", 4) == 0) {
                            int want_len = (int)strlen(g_debug_state);
                            if (want_len >= 4 &&
                                _strnicmp(nm, g_debug_state, want_len) == 0) {
                                char after = nm[want_len];
                                if (after == ' ' || after == '\t' ||
                                    after == '<' || after == 0)
                                    match = 1;
                            }
                        }
                        if (!match) continue;

                        /* v55m_48d: remember the S3 entry position so we can
                         * find the spawned object in the update list. */
                        g_dbg_want_x = *(float*)(obj_ptr + 0x04);
                        g_dbg_want_y = *(float*)(obj_ptr + 0x08);
                        g_dbg_want_z = *(float*)(obj_ptr + 0x0C);
                        g_dbg_want_set = 1;
                        /* v55m_48d: keep the matched S1 entry pointer — its
                         * per-axis scale (+0x1C/+0x20/+0x24) is displayed in
                         * the 2nd Properties table. */
                        g_dbg_s1_entry = obj_ptr;

                        /* Found it — extract <ENTITY> name */
                        char ent_name[64] = {0};
                        char* ent_start = my_stristr(name, "<ENTITY>");
                        if (ent_start) {
                            ent_start += 8;
                            while (*ent_start == ' ' || *ent_start == '\t') ent_start++;
                            char* ent_end = my_stristr(ent_start, "</ENTITY>");
                            if (!ent_end) ent_end = ent_start + strlen(ent_start);
                            size_t ent_len = ent_end - ent_start;
                            if (ent_len > 0 && ent_len < 64) {
                                strncpy(ent_name, ent_start, ent_len);
                                ent_name[ent_len] = 0;
                                while (ent_len > 0 && (ent_name[ent_len-1] == ' ' || ent_name[ent_len-1] == '\t'))
                                    ent_name[--ent_len] = 0;
                            }
                        }

                        /* Build "Custom Entity XXX - EntityName". v55m_49:
                         * when the level has more than one cEnt, append the
                         * selected table's position — "Custom Entity 001 -
                         * Swirl (1/2)" — so A/D switching is visible. */
                        if (g_table_count > 1) {
                            snprintf(g_debug_title, sizeof(g_debug_title),
                                     "Custom Entity %s - %s (%d/%d)",
                                     g_debug_state + 4,
                                     ent_name[0] ? ent_name : "Unknown",
                                     g_which_table + 1, g_table_count);
                        } else {
                            snprintf(g_debug_title, sizeof(g_debug_title),
                                     "Custom Entity %s - %s",
                                     g_debug_state + 4,
                                     ent_name[0] ? ent_name : "Unknown");
                        }
                        resolved = 1;
                        break;
                    }
                }
            }
        }
        if (!resolved) strcpy(g_debug_title, "Custom Entity Debug");
    }

    /* v55m_48d: resolve live runtime values for the entity named by
     * g_debug_state. Uses the S3 entry position (g_dbg_want_{x,y,z})
     * to find the spawned object in the board's update list. */
    if (g_dbg_want_set) {
        cEnt_resolve_debug_values(get_board(), g_dbg_want_x, g_dbg_want_y, g_dbg_want_z);
    } else {
        g_dbg_entity_found = 0;
    }

    /* Rotator documentation view. It shows a hierarchical breakdown of the
     * entity named by g_debug_state. Line 2 is the section title and
     * sub-lines are indented 40px to the right below it.
     *
     * v55m_48d: g_table_visible (T key) hides the whole table when 0.
     * v55m_49: g_which_props (P key) picks which property view of the
     * SELECTED Custom Entity table is drawn:
     *   0 = this Rotator docs table (the hierarchical docs view),
     *   1 = the live \"properties\" table (runtime values). */
    if (!g_table_visible) return;

    if (g_which_props == 1) {
        /* 2nd properties table — live runtime values of the tracked cEnt
         * rotator. All values are re-resolved each Present (real time).
         * v55m_48d: values are ALIGNED at a fixed X column — the SAME
         * column used by the 1st (Rotator docs) table's paren column.
         * Label spans [20, value_x); value spans [value_x, ...). Nonzero
         * values paint faint blue, zeros stay gray. */
        static const char* const prop_labels[] = {
            "Position X", "Position Y", "Position Z",
            "Angle X", "Angle Y", "Angle Z",
            "Scale X", "Scale Y", "Scale Z",
            "Rotation angle", "Direction",
            "Mesh", "In update list", "In render list", "In collision list",
        };
        int n_props = sizeof(prop_labels) / sizeof(prop_labels[0]);

        /* Compute the common value X = longest label + pad (measured at the
         * same font/scale as the 1st table's paren column). */
        int value_x = 20;
        int p;
        for (p = 0; p < n_props; p++) {
            int w = pfn_Font_MeasureText(font, (char*)prop_labels[p]);
            if (20 + w > value_x) value_x = 20 + w;
        }
        /* Pad so values never touch the longest label. */
        {
            float* fp_scale = (float*)((char*)font + 0x428);
            float orig_scale = *fp_scale;
            *fp_scale = 0.9f;
            value_x += pfn_Font_MeasureText(font, "    ");
            *fp_scale = orig_scale;
        }

        int y0 = 12 + 36 + g_debug_scroll_y;
        int yy = y0;
        char tmp[64];
        cEnt_draw_text_double(font, g_debug_title, 20, yy, 1.0f, 1.0f, 1.0f, 0.9f);
        yy += debugTextSpacing;

        /* Position X / Y / Z (live from obj — SWIRL-aware). */
        cEnt_draw_text_double(font, prop_labels[0], 20, yy, 1.0f, 1.0f, 1.0f, 0.9f);
        snprintf(tmp, sizeof(tmp), "%.2f", g_dbg_pos_x);
        cEnt_draw_text_double(font, tmp, value_x, yy,
                              g_dbg_pos_x != 0.0f ? DEBUG_VALUE_BLUE_R : 0.6f,
                              g_dbg_pos_x != 0.0f ? DEBUG_VALUE_BLUE_G : 0.6f,
                              g_dbg_pos_x != 0.0f ? DEBUG_VALUE_BLUE_B : 0.6f, 0.9f);
        yy += debugTextSpacing;
        cEnt_draw_text_double(font, prop_labels[1], 20, yy, 1.0f, 1.0f, 1.0f, 0.9f);
        snprintf(tmp, sizeof(tmp), "%.2f", g_dbg_pos_y);
        cEnt_draw_text_double(font, tmp, value_x, yy,
                              g_dbg_pos_y != 0.0f ? DEBUG_VALUE_BLUE_R : 0.6f,
                              g_dbg_pos_y != 0.0f ? DEBUG_VALUE_BLUE_G : 0.6f,
                              g_dbg_pos_y != 0.0f ? DEBUG_VALUE_BLUE_B : 0.6f, 0.9f);
        yy += debugTextSpacing;
        cEnt_draw_text_double(font, prop_labels[2], 20, yy, 1.0f, 1.0f, 1.0f, 0.9f);
        snprintf(tmp, sizeof(tmp), "%.2f", g_dbg_pos_z);
        cEnt_draw_text_double(font, tmp, value_x, yy,
                              g_dbg_pos_z != 0.0f ? DEBUG_VALUE_BLUE_R : 0.6f,
                              g_dbg_pos_z != 0.0f ? DEBUG_VALUE_BLUE_G : 0.6f,
                              g_dbg_pos_z != 0.0f ? DEBUG_VALUE_BLUE_B : 0.6f, 0.9f);
        yy += debugTextSpacing;

        /* Angle X / Y / Z (per-axis, live). */
        cEnt_draw_text_double(font, prop_labels[3], 20, yy, 1.0f, 1.0f, 1.0f, 0.9f);
        snprintf(tmp, sizeof(tmp), "%.2f", g_dbg_ang_x);
        cEnt_draw_text_double(font, tmp, value_x, yy,
                              g_dbg_ang_x != 0.0f ? DEBUG_VALUE_BLUE_R : 0.6f,
                              g_dbg_ang_x != 0.0f ? DEBUG_VALUE_BLUE_G : 0.6f,
                              g_dbg_ang_x != 0.0f ? DEBUG_VALUE_BLUE_B : 0.6f, 0.9f);
        yy += debugTextSpacing;
        cEnt_draw_text_double(font, prop_labels[4], 20, yy, 1.0f, 1.0f, 1.0f, 0.9f);
        snprintf(tmp, sizeof(tmp), "%.2f", g_dbg_ang_y);
        cEnt_draw_text_double(font, tmp, value_x, yy,
                              g_dbg_ang_y != 0.0f ? DEBUG_VALUE_BLUE_R : 0.6f,
                              g_dbg_ang_y != 0.0f ? DEBUG_VALUE_BLUE_G : 0.6f,
                              g_dbg_ang_y != 0.0f ? DEBUG_VALUE_BLUE_B : 0.6f, 0.9f);
        yy += debugTextSpacing;
        cEnt_draw_text_double(font, prop_labels[5], 20, yy, 1.0f, 1.0f, 1.0f, 0.9f);
        snprintf(tmp, sizeof(tmp), "%.2f", g_dbg_ang_z);
        cEnt_draw_text_double(font, tmp, value_x, yy,
                              g_dbg_ang_z != 0.0f ? DEBUG_VALUE_BLUE_R : 0.6f,
                              g_dbg_ang_z != 0.0f ? DEBUG_VALUE_BLUE_G : 0.6f,
                              g_dbg_ang_z != 0.0f ? DEBUG_VALUE_BLUE_B : 0.6f, 0.9f);
        yy += debugTextSpacing;

        /* Scale X / Y / Z — v55m_48d: per-axis scale from the matched S1
         * ref-point entry (+0x1C/+0x20/+0x24). Default 1.0/1.0/1.0 —
         * hardcoded by the game's ref-point ctor (FUN_004629e0). The
         * spawned object itself has NO stored scale (mesh size is baked
         * into the vertices), so this is the object's scale source. */
        cEnt_draw_text_double(font, prop_labels[6], 20, yy, 1.0f, 1.0f, 1.0f, 0.9f);
        snprintf(tmp, sizeof(tmp), "%.2f", g_dbg_scl_x);
        cEnt_draw_text_double(font, tmp, value_x, yy,
                              g_dbg_scl_x != 0.0f ? DEBUG_VALUE_BLUE_R : 0.6f,
                              g_dbg_scl_x != 0.0f ? DEBUG_VALUE_BLUE_G : 0.6f,
                              g_dbg_scl_x != 0.0f ? DEBUG_VALUE_BLUE_B : 0.6f, 0.9f);
        yy += debugTextSpacing;
        cEnt_draw_text_double(font, prop_labels[7], 20, yy, 1.0f, 1.0f, 1.0f, 0.9f);
        snprintf(tmp, sizeof(tmp), "%.2f", g_dbg_scl_y);
        cEnt_draw_text_double(font, tmp, value_x, yy,
                              g_dbg_scl_y != 0.0f ? DEBUG_VALUE_BLUE_R : 0.6f,
                              g_dbg_scl_y != 0.0f ? DEBUG_VALUE_BLUE_G : 0.6f,
                              g_dbg_scl_y != 0.0f ? DEBUG_VALUE_BLUE_B : 0.6f, 0.9f);
        yy += debugTextSpacing;
        cEnt_draw_text_double(font, prop_labels[8], 20, yy, 1.0f, 1.0f, 1.0f, 0.9f);
        snprintf(tmp, sizeof(tmp), "%.2f", g_dbg_scl_z);
        cEnt_draw_text_double(font, tmp, value_x, yy,
                              g_dbg_scl_z != 0.0f ? DEBUG_VALUE_BLUE_R : 0.6f,
                              g_dbg_scl_z != 0.0f ? DEBUG_VALUE_BLUE_G : 0.6f,
                              g_dbg_scl_z != 0.0f ? DEBUG_VALUE_BLUE_B : 0.6f, 0.9f);
        yy += debugTextSpacing;

        /* More useful live values. v55m_51: \"Rotation angle\" now shows the
         * LIVE accumulator (obj+0x10E4, degrees) — the per-frame delta
         * (obj+0x10E8) is capped at 1.0 and would pin at 1.00 forever. */
        cEnt_draw_text_double(font, prop_labels[9], 20, yy, 1.0f, 1.0f, 1.0f, 0.9f);
        snprintf(tmp, sizeof(tmp), "%.2f", g_dbg_accum);
        cEnt_draw_text_double(font, tmp, value_x, yy,
                              g_dbg_accum != 0.0f ? DEBUG_VALUE_BLUE_R : 0.6f,
                              g_dbg_accum != 0.0f ? DEBUG_VALUE_BLUE_G : 0.6f,
                              g_dbg_accum != 0.0f ? DEBUG_VALUE_BLUE_B : 0.6f, 0.9f);
        yy += debugTextSpacing;
        cEnt_draw_text_double(font, prop_labels[10], 20, yy, 1.0f, 1.0f, 1.0f, 0.9f);
        snprintf(tmp, sizeof(tmp), "%.2f", g_dbg_direction);
        cEnt_draw_text_double(font, tmp, value_x, yy,
                              g_dbg_direction != 0.0f ? DEBUG_VALUE_BLUE_R : 0.6f,
                              g_dbg_direction != 0.0f ? DEBUG_VALUE_BLUE_G : 0.6f,
                              g_dbg_direction != 0.0f ? DEBUG_VALUE_BLUE_B : 0.6f, 0.9f);
        yy += debugTextSpacing;
        cEnt_draw_text_double(font, prop_labels[11], 20, yy, 1.0f, 1.0f, 1.0f, 0.9f);
        cEnt_draw_text_double(font, g_dbg_mesh_path[0] ? g_dbg_mesh_path : "(empty)",
                              value_x, yy,
                              g_dbg_mesh_path[0] ? DEBUG_VALUE_BLUE_R : 0.6f,
                              g_dbg_mesh_path[0] ? DEBUG_VALUE_BLUE_G : 0.6f,
                              g_dbg_mesh_path[0] ? DEBUG_VALUE_BLUE_B : 0.6f, 0.9f);
        yy += debugTextSpacing;
        cEnt_draw_text_double(font, prop_labels[12], 20, yy, 1.0f, 1.0f, 1.0f, 0.9f);
        cEnt_draw_text_double(font, g_dbg_in_update_list ? "yes" : "no",
                              value_x, yy, 0.0f, 1.0f, 0.0f, 0.9f);
        yy += debugTextSpacing;
        cEnt_draw_text_double(font, prop_labels[13], 20, yy, 1.0f, 1.0f, 1.0f, 0.9f);
        cEnt_draw_text_double(font, g_dbg_in_render_list ? "yes" : "no",
                              value_x, yy, 0.0f, 1.0f, 0.0f, 0.9f);
        yy += debugTextSpacing;
        cEnt_draw_text_double(font, prop_labels[14], 20, yy, 1.0f, 1.0f, 1.0f, 0.9f);
        cEnt_draw_text_double(font, g_dbg_in_collision_list ? "yes" : "no",
                              value_x, yy, 0.0f, 1.0f, 0.0f, 0.9f);
        return;
    }

    {
        cEnt_draw_text_double(font, g_debug_title, 20, 12 + 36 + g_debug_scroll_y,
                              1.0f, 1.0f, 1.0f, 0.9f);
        /* Sections: each main function is a numbered title ("# - Function
         * Name (description)") with its sub-functions as sub-lines. The
         * parenthetical parts get the gray + 4-space treatment, aligned to
         * a common column (the furthest-right base text wins). */
        /* v55m_48d: every line is a {name, addr} pair. `name` is the
         * simplified function/subfunction label (state 0). `addr` is the
         * address / offset where it's stored (state 1, shown when the "K"
         * key toggles g_function_display = 1). Entries that are pure text
         * have addr = "" (same text shown in both states). */
        static const char* const rot_s1_subs[] = {
            "Operator_new (memory reserve)",
            "Rotator_ctor (game constructor)",
            "cEnt_load_mesh_file (mesh loading)",
            "Mesh fallback (tries Swirl if primary mesh fails)",
        };
        static const char* const rot_s1_subsub_names[][4] = {
            { "operator_new", "size ROTATER_SIZE", "malloc wrapper", "PUSHAD/POPAD cave" },
            { "Rotator_ctor_Impossible", "vtable (SWIRL, 384 bytes)", "Sets Angle", "Sets Direction" },
            { "cEnt_load_mesh_file", "tries .MESHWORLD", "or .MESH via MeshNode", "gfx_device as ECX" },
            { "Swirl fallback path", "levels\\Level3-Swirl", "only on primary fail", "cancels if _default missing" },
        };
        static const char* const rot_s1_subsub_addrs[][4] = {
            { "0x004BA57B", "0x1508", "", "" },
            { "0x00435940", "0x4D5518", "obj+0x10E8", "obj+0x10EC" },
            { "cEnt_load_mesh_file", "", "", "" },
            { "Swirl fallback", "levels\\Level3-Swirl", "", "" },
        };
        static const char* const rot_s2_subs[] = {
            "Update list (board+0x2578)",
            "Render list (board+0xCD4)",
            "Collision list (board+0x10EC)",
            "Spatial tree (sceneobj+0x1C)",
        };
        static const char* const rot_s2_subsub_names[][4] = {
            { "AthenaList_Append", "vtable[1] update driver", "flags", "removed on despawn" },
            { "AthenaList_Append", "vtable[18] draw path", "flags", "popcylinder render wrapper" },
            { "AthenaList_Append", "collision object", "flags", "removed on despawn" },
            { "AthenaList_Append", "sceneobject tree", "spatial partitioning", "removed on despawn" },
        };
        static const char* const rot_s2_subsub_addrs[][4] = {
            { "0x00453780", "vtable[1]", "0x201", "" },
            { "0x00453780", "vtable[18]", "0x202", "" },
            { "0x00453780", "obj+0x10D4", "0x203", "" },
            { "0x00453780", "sceneobj+0x1C", "", "" },
        };
        static const char* const rot_s3_subs[] = {
            "Native render vtable[11] (0x0043B330)",
            "obj+0x10E8 (angle field)",
            "obj+0x10EC (direction field)",
            "cEnt_apply_rotater_directions (write rot_y)",
            "cEnt_update_constant_rotations (constant spin)",
            "RotaterConfig struct (mod-side state)",
        };
        static const char* const rot_s3_subsub_names[][4] = {
            { "angle += direction * 0.004", "oscillation upper flip", "oscillation lower flip", "before object render" },
            { "accumulated angle (radians)", "read: render only", "reset on spawn (rot_a)", "default 0.0" },
            { "direction multiplier (rot_y)", "default 0.0", "ROS_Y=0 rewritten", "native flip reversal guard" },
            { "apply directions", "writes direction", "runs once at spawn", "cfg loop" },
            { "only for ROS_Y=0", "rewrites direction", "prevents oscillation", "angle grows unbounded" },
            { "obj + mesh_path[128]", "rot speeds", "rot ranges", "angle / dir / mode" },
        };
        static const char* const rot_s3_subsub_addrs[][4] = {
            { "obj+0x10E8", "obj+0x10EC value 1.0", "obj+0x10EC value -1.0", "slot 9" },
            { "obj+0x10E8", "obj+0x10E8", "rot_a", "0.0" },
            { "obj+0x10EC", "0.0", "obj+0x10EC", "obj+0x10EC" },
            { "cEnt_apply_rotater_directions", "obj+0x10EC", "spawn time", "g_rotater_cfg" },
            { "ROS_Y=0", "obj+0x10EC", "obj+0x10EC", "obj+0x10E8" },
            { "RotaterConfig", "rot_x/rot_y/rot_z", "ros_x/ros_y/ros_z", "rot_a/rot_d/rot_m" },
        };
        static const char* const rot_s4_subs[] = {
            "process_rotaters -> cEnt_spawn_rotater_at",
            "per-frame: vtable[11] native render",
            "cEnt_despawn_all_rotaters (level end)",
        };
        static const char* const rot_s4_subsub_names[][4] = {
            { "scans section 3 objects", "matches cEnt_XXX name", "parses <ENTITY> tag", "calls cEnt_spawn_rotater_at" },
            { "present hook", "slot 9 (viewport clear)", "before slot 10 object render", "cEnt_update_constant_rotations" },
            { "vtable[11] RemoveAndFree", "resets rotater count", "resets gluebie/tarpit/waterwheel", "level unload handler" },
        };
        static const char* const rot_s4_subsub_addrs[][4] = {
            { "", "", "", "" },
            { "0x00455A90", "slot 9", "slot 10", "" },
            { "0x00436FC0", "", "", "" },
        };
        static const struct {
            const char* title;
            const char* const* subs;
            int n_subs;
            const char* const (*subsub_names)[4];
            const char* const (*subsub_addrs)[4];
            int n_subsubs_per_sub[8];
        } rot_sections[] = {
            { "1 - Constructors (object creation)",     rot_s1_subs, 4, rot_s1_subsub_names, rot_s1_subsub_addrs, {4,4,4,4} },
            { "2 - Registration (board wiring)",        rot_s2_subs, 4, rot_s2_subsub_names, rot_s2_subsub_addrs, {4,4,4,4} },
            { "3 - Updates (rotation & behavior)",      rot_s3_subs, 6, rot_s3_subsub_names, rot_s3_subsub_addrs, {4,4,4,4,4,4} },
            { "4 - Lifecycle / Call chain",             rot_s4_subs, 3, rot_s4_subsub_names, rot_s4_subsub_addrs, {4,4,4} },
        };
        int n_sections = sizeof(rot_sections) / sizeof(rot_sections[0]);
        void* sub_font = *(void**)((char*)app + 0x31C);  /* showcardgothic14 — smaller */
        int has_sub_font = sub_font && !IsBadReadPtr(sub_font, 0x500);
        int s, i;

        /* Pass 1 — measure every line's paren column (x + base width) at its
         * own scale and find the max. That becomes the common paren column. */
        int max_paren_col = 0;
        for (s = 0; s < n_sections; s++) {
            int c_title = 20 + cEnt_paren_line_base_width(font, rot_sections[s].title, 0.9f);
            if (c_title > max_paren_col) max_paren_col = c_title;
            if (has_sub_font) {
                for (i = 0; i < rot_sections[s].n_subs; i++) {
                    char subline[128];
                    snprintf(subline, sizeof(subline), "- %s", rot_sections[s].subs[i]);
                    int c_sub = 40 + cEnt_paren_line_base_width(sub_font, subline, 0.81f);
                    if (c_sub > max_paren_col) max_paren_col = c_sub;
                    /* sub-sub lines at double X offset (80px) */
                    int n_subsub = rot_sections[s].n_subsubs_per_sub[i];
                    for (int k = 0; k < n_subsub; k++) {
                        char subsubline[128];
                        /* v55m_48d: K-key toggles function_display. State 0
                         * shows the name, state 1 the address/offset. If the
                         * addr entry is empty, fall back to the name. */
                        const char* cell = g_function_display
                            ? (rot_sections[s].subsub_addrs[i][k][0]
                               ? rot_sections[s].subsub_addrs[i][k]
                               : rot_sections[s].subsub_names[i][k])
                            : rot_sections[s].subsub_names[i][k];
                        snprintf(subsubline, sizeof(subsubline), "  %s", cell);
                        int c_ss = 80 + cEnt_paren_line_base_width(sub_font, subsubline, 0.81f);
                        if (c_ss > max_paren_col) max_paren_col = c_ss;
                    }
                }
            }
        }
        /* Add a 4-space pad (measured at the title scale) so parens don't
         * touch the longest base text. */
        if (max_paren_col > 0) {
            float* fp_scale = (float*)((char*)font + 0x428);
            float orig_scale = *fp_scale;
            *fp_scale = 0.9f;
            max_paren_col += pfn_Font_MeasureText(font, "    ");
            *fp_scale = orig_scale;
        }

        /* Pass 2 — draw everything, parens at the common column. */
        int line = 2;  /* first_line is line 0, section 1 starts at line 2 */
        for (s = 0; s < n_sections; s++) {
            /* Leave an empty line above every numbered section title so the
             * sections are visually separated. Section 1 already has the gap
             * after the "rotator" header, so only skip a line for s > 0. */
            if (s > 0) line++;
            cEnt_draw_paren_line(font, rot_sections[s].title, 20,
                                 12 + 36 + line * debugTextSpacing + g_debug_scroll_y,
                                 0.9f, max_paren_col, NULL);
            line++;
            if (has_sub_font) {
                for (i = 0; i < rot_sections[s].n_subs; i++) {
                    char subline[128];
                    snprintf(subline, sizeof(subline), "- %s", rot_sections[s].subs[i]);
                    cEnt_draw_paren_line(sub_font, subline, 40,
                                         12 + 36 + line * debugTextSpacing + g_debug_scroll_y,
                                         0.81f, max_paren_col, NULL);
                    line++;
                    /* sub-sub lines at double X offset (80px), same font */
                    int n_subsub = rot_sections[s].n_subsubs_per_sub[i];
                    for (int k = 0; k < n_subsub; k++) {
                        char subsubline[128];
                        /* v55m_48d: same name/addr selection as pass 1. */
                        const char* cell = g_function_display
                            ? (rot_sections[s].subsub_addrs[i][k][0]
                               ? rot_sections[s].subsub_addrs[i][k]
                               : rot_sections[s].subsub_names[i][k])
                            : rot_sections[s].subsub_names[i][k];
                        snprintf(subsubline, sizeof(subsubline), "  %s", cell);
                        cEnt_draw_paren_line(sub_font, subsubline, 80,
                                             12 + 36 + line * debugTextSpacing + g_debug_scroll_y,
                                             0.81f, max_paren_col,
                                             cEnt_debug_value(s, i, k));
                        line++;
                    }
                }
            } else {
                for (i = 0; i < rot_sections[s].n_subs; i++) {
                    line += 1 + rot_sections[s].n_subsubs_per_sub[i];
                }
            }
        }
        return;
    }
}

/* Install Graphics_PresentOrEnd hook. */
static void install_presentend_hook(void) {
    if (g_presentend_hook_installed) return;
    BYTE *hook_addr = (BYTE*)PRESENTEND_HOOK_ADDR;
    BYTE expected[] = { 0x8A, 0x44, 0x24, 0x04, 0x83, 0xEC, 0x20 };
    if (memcmp(hook_addr, expected, 7) != 0) return;

    g_presentend_cave = (BYTE*)VirtualAlloc(NULL, 512, MEM_COMMIT | MEM_RESERVE,
                                             PAGE_EXECUTE_READWRITE);
    if (!g_presentend_cave) return;

    g_presentend_fn_ptr = cEnt_draw_text_helper;

    int p = 0;
    /* PUSHAD + PUSHFD */
    g_presentend_cave[p++] = 0x60;
    g_presentend_cave[p++] = 0x9C;
    /* CALL [g_presentend_fn_ptr] */
    g_presentend_cave[p++] = 0xFF; g_presentend_cave[p++] = 0x15;
    *(DWORD*)(g_presentend_cave + p) = (DWORD)&g_presentend_fn_ptr; p += 4;
    /* POPFD + POPAD */
    g_presentend_cave[p++] = 0x9D;
    g_presentend_cave[p++] = 0x61;
    /* Original 7 bytes: MOV AL,[ESP+4]; SUB ESP,0x20 */
    g_presentend_cave[p++] = 0x8A; g_presentend_cave[p++] = 0x44;
    g_presentend_cave[p++] = 0x24; g_presentend_cave[p++] = 0x04;
    g_presentend_cave[p++] = 0x83; g_presentend_cave[p++] = 0xEC;
    g_presentend_cave[p++] = 0x20;
    /* JMP back to hook_addr + 7 */
    g_presentend_cave[p++] = 0xE9;
    *(DWORD*)(g_presentend_cave + p) = (DWORD)(hook_addr + PRESENTEND_ORIG_BYTES)
                                     - (DWORD)(g_presentend_cave + p + 4);
    p += 4;

    DWORD old_protect;
    VirtualProtect(hook_addr, PRESENTEND_ORIG_BYTES, PAGE_EXECUTE_READWRITE, &old_protect);
    DWORD jmp_offset = (DWORD)(g_presentend_cave - hook_addr - 5);
    hook_addr[0] = 0xE9;
    *(DWORD*)(hook_addr + 1) = jmp_offset;
    /* NOP remaining 2 bytes (7 total - 5 for JMP = 2 NOPs) */
    hook_addr[5] = 0x90; hook_addr[6] = 0x90;
    VirtualProtect(hook_addr, PRESENTEND_ORIG_BYTES, old_protect, &old_protect);
    FlushInstructionCache(GetCurrentProcess(), hook_addr, PRESENTEND_ORIG_BYTES);
    g_presentend_hook_installed = 1;
}

static void uninstall_presentend_hook(void) {
    if (!g_presentend_hook_installed) return;
    BYTE *hook_addr = (BYTE*)PRESENTEND_HOOK_ADDR;
    DWORD old_protect;
    if (VirtualProtect(hook_addr, PRESENTEND_ORIG_BYTES, PAGE_EXECUTE_READWRITE, &old_protect)) {
        /* Restore: MOV AL,[ESP+4]; SUB ESP,0x20 */
        hook_addr[0] = 0x8A; hook_addr[1] = 0x44; hook_addr[2] = 0x24; hook_addr[3] = 0x04;
        hook_addr[4] = 0x83; hook_addr[5] = 0xEC; hook_addr[6] = 0x20;
        VirtualProtect(hook_addr, PRESENTEND_ORIG_BYTES, old_protect, &old_protect);
        FlushInstructionCache(GetCurrentProcess(), hook_addr, PRESENTEND_ORIG_BYTES);
    }
    g_presentend_hook_installed = 0;
}

static int g_gluebie_debug_count = 0;  /* limit debug output */
static int g_gluebie_debug_frame = 0; /* frame counter for sampling */

/* v55d: Tar sound queue — set when ball enters Gluebie range, played once per entry.
 * Native plays sound from DizzyBoard_Update (main thread). We play from the mod's
 * background thread — safe because Sound_Play3D just calls Sound_CalcDistAtten +
 * BASS_ChannelPlay, no DirectSound locking issues. */
static int g_gluebie_sound_pending = 0;  /* 1 = need to play tar sound */
static float g_gluebie_snd_x = 0.0f;
static float g_gluebie_snd_y = 0.0f;
static float g_gluebie_snd_z = 0.0f;

/* App+0x484 = tar sound channel (used by native DizzyBoard_Update at 0x41D9BC) */
#define APP_SOUNDFX_TAR  0x484

static void cEnt_gluebie_proximity_check(DWORD board) {
    if (!board) return;

    /* Get ball AthenaList at board+0x29D4 (contains player + AI balls) */
    DWORD ball_list = board + 0x29D4;
    if (IsBadReadPtr((void*)(ball_list + 0x04), 4)) return;
    int ball_count = *(int*)(ball_list + 0x04);
    if (ball_count <= 0 || ball_count > 20) return;
    if (IsBadReadPtr((void*)(ball_list + 0x40C), 4)) return;
    DWORD* ball_data = *(DWORD**)(ball_list + 0x40C);
    if (!ball_data || IsBadReadPtr(ball_data, ball_count * 4)) return;

    g_gluebie_debug_frame++;
    /* Log every 60th frame (~once per second) OR when in range */
    int do_log = (g_gluebie_debug_count < 120) && 
                 ((g_gluebie_debug_frame % 60) == 0);

    int i, j;
    for (j = 0; j < g_gluebie_count; j++) {
        DWORD gluebie = g_gluebie_objs[j];
        if (!gluebie || gluebie < 0x10000) continue;
        if (IsBadReadPtr((void*)gluebie, 0x1110)) continue;

        /* v55j_9: Use native position offsets +0x10E0/10E4/10E8 (NOT +0x10D4).
         * Native DizzyBoard_Update reads Gluebie position from these offsets.
         * +0x10D4 stores spawn position; +0x10E0 stores current physics position. */
        float gx = *(float*)(gluebie + 0x10E0);
        float gy = *(float*)(gluebie + 0x10E4);
        float gz = *(float*)(gluebie + 0x10E8);

        /* v55j_8: Use native outer radius formula: obj+0x1100 * 60.0.
         * Gluebie_ctor initializes +0x1100 to (RNG(25)+75)*0.01 = 0.75-1.0,
         * so native radius is 45-60 units. Fall back to 60.0 if +0x1100 is 0
         * (shouldn't happen since ctor inits it, but safety first). */
        float radius_mult = *(float*)(gluebie + 0x1100);
        float outer_radius = (radius_mult > 0.0f) ? (radius_mult * 60.0f) : 60.0f;
        float inner_radius = 3.0f;  /* tar surface contact (unused by Gluebie — Ball_Update handles it) */

        /* Reset active flag (will be set if any ball is in range) */
        *(BYTE*)(gluebie + 0x1104) = 0;

        for (i = 0; i < ball_count; i++) {
            DWORD ball = ball_data[i];
            if (!ball || ball < 0x10000) continue;
            if (IsBadReadPtr((void*)ball, 0x2D0)) continue;

            /* Check ball is active: ball+0x18 >= 0 (signed) */
            if (*(int*)(ball + 0x18) < 0) continue;

            /* Native also checks ball+0x2CC == 0 */
            if (*(char*)(ball + 0x2CC) != 0) continue;

            /* Read ball position */
            float bx = *(float*)(ball + 0x164);
            float by = *(float*)(ball + 0x168);
            float bz = *(float*)(ball + 0x16C);

            /* 3D distance */
            float dx = gx - bx;
            float dy = gy - by;
            float dz = gz - bz;
            float dist_sq = dx*dx + dy*dy + dz*dz;
            int in_outer = (dist_sq < outer_radius * outer_radius);
            int in_inner = (dist_sq < inner_radius * inner_radius);

            /* Debug: log once per second OR when in outer range */
            if (do_log || (in_outer && g_gluebie_debug_count < 200)) {
                g_gluebie_debug_count++;
                FILE* df = fopen("mknp_custom_entities_debug.log", "a");
                if (df) {
                    float dist = 0.0f;
                    if (dist_sq > 0.0f) dist = sqrtf(dist_sq);
                    fprintf(df, "GLUEBIE f=%d: gpos=(%.1f,%.1f,%.1f) outer=%.1f inner=%.1f ball[%d]=%p bpos=(%.1f,%.1f,%.1f) dist=%.1f in_outer=%d in_inner=%d\n",
                        g_gluebie_debug_frame, gx, gy, gz, outer_radius, inner_radius, i, (void*)ball, bx, by, bz, dist, in_outer, in_inner);
                    DWORD col_mesh = *(DWORD*)(ball + 0x1A4);
                    fprintf(df, "  col_mesh=%p vel=(%.3f,%.3f,%.3f) part_count=%d\n",
                        (void*)col_mesh,
                        col_mesh ? *(float*)(col_mesh + 0xCA4) : 0.0f,
                        col_mesh ? *(float*)(col_mesh + 0xCA8) : 0.0f,
                        col_mesh ? *(float*)(col_mesh + 0xCAC) : 0.0f,
                        *(int*)(ball + 0x814));
                    fclose(df);
                }
            }

            if (in_outer) {
                /* Ball is in outer zone — scale velocity (native Gluebie behavior).
                 * DizzyBoard_Update scales col_mesh velocity by 0.95 each frame.
                 * Ball stays on solid surface, just rolls slower. */
                DWORD col_mesh = *(DWORD*)(ball + 0x1A4);
                if (!col_mesh || IsBadReadPtr((void*)(col_mesh + 0xCB0), 4)) continue;

                *(float*)(col_mesh + 0xCA4) *= 0.95f;
                *(float*)(col_mesh + 0xCA8) *= 0.95f;
                *(float*)(col_mesh + 0xCAC) *= 0.95f;

                /* Set Gluebie active flag */
                *(BYTE*)(gluebie + 0x1104) = 1;

                /* v55j_9: Track ball in zone for Ball_Render hook.
                 * Do NOT set ball+0x260 — that causes the "sweat" sprite overlay,
                 * NOT the real tarsplotch. The real tarsplotch is particles in
                 * ball+0x810 AthenaList, created on first contact. */
                g_gluebie_ball_in_zone = ball;

                /* v55j_12: Create tarsplotch particles (native DizzyBoard_Update behavior).
                 * Use our OWN flag instead of ball+0x2BC (which is UNINITIALIZED garbage
                 * on non-Dizzy levels — no memset after operator_new).
                 * v55j_15: On Dizzy, ball+0x2BC is set to 0 when ball leaves Gluebie
                 * range, then set to 1 on re-entry. So particles ARE created each time
                 * you enter the zone — they stack up to 30 max. We replicate this by
                 * resetting our flag when ball leaves the zone (in_outer=0). */
                {
                    int pc = *(int*)(ball + 0x814);
                    /* If all particles consumed OR ball left zone, reset flag */
                    if (pc == 0 && g_gluebie_particles_created_ball == ball) {
                        g_gluebie_particles_created_ball = 0;
                    }
                }
                {
                    BYTE bc_flag = *(BYTE*)(ball + 0x2BC);
                    int pc = *(int*)(ball + 0x814);
                    /* Debug: log the flags */
                    if (g_gluebie_debug_count < 120) {
                        FILE* df2 = fopen("mknp_custom_entities_debug.log", "a");
                        if (df2) {
                            fprintf(df2, "  >> bc=%d pc=%d list_items=%p ours=%d\n",
                                bc_flag, pc, (void*)*(DWORD*)(ball + 0xC1C),
                                (g_gluebie_particles_created_ball == ball) ? 1 : 0);
                            fclose(df2);
                        }
                    }
                }
                if (g_gluebie_particles_created_ball != ball) {
                    /* Check particle list capacity */
                    int part_count = *(int*)(ball + 0x814);  /* AthenaList count at +0x04 */
                    if (part_count < 30) {
                        int k;
                        for (k = 0; k < 3; k++) {
                            /* Allocate 20 bytes via game's operator_new (not malloc!)
                             * Game's memory system tracks these and Ball_Respawn
                             * will free them via operator_delete. */
                            typedef void* (__cdecl *operator_new_t)(unsigned int);
                            operator_new_t pfn_op_new = (operator_new_t)0x4BA57B;
                            float* particle = (float*)pfn_op_new(0x14);
                            if (!particle) break;
                            /* Random direction */
                            particle[0] = (float)((rand() % 2000) - 1000) / 1000.0f;
                            particle[1] = (float)((rand() % 2000) - 1000) / 1000.0f;
                            particle[2] = (float)((rand() % 2000) - 1000) / 1000.0f;
                            particle[3] = 0.0f;  /* type=6 as int (0x00000006 as float ≈ 8.4e-45) */
                            *(int*)&particle[3] = 6;
                            particle[4] = 0.0f;  /* padding */
                            /* Normalize to unit length */
                            float len_sq = particle[0]*particle[0] + particle[1]*particle[1] + particle[2]*particle[2];
                            if (len_sq > 0.0f) {
                                float inv_len = 1.0f / sqrtf(len_sq);
                                particle[0] *= inv_len;
                                particle[1] *= inv_len;
                                particle[2] *= inv_len;
                            }
                            /* Append to ball+0x810 AthenaList manually.
                             * AthenaList layout: +0x04=count, +0x08..+0x408=iterators,
                             * +0x40C=items ptr, +0x414=sorted flag.
                             * We replicate AthenaList_Append (0x453780) in pure C
                             * to avoid __thiscall inline asm issues. */
                            {
                                DWORD list = ball + 0x810;
                                int *count_ptr = (int*)(list + 0x04);
                                DWORD *items_ptr = (DWORD*)(list + 0x40C);
                                
                                if (*count_ptr == 0) {
                                    /* First item: operator_new(120) for 30 particle ptrs (max).
                                     * No realloc needed — pre-allocate max capacity. */
                                    typedef void* (__cdecl *operator_new_t)(unsigned int);
                                    operator_new_t pfn_op_new2 = (operator_new_t)0x4BA57B;
                                    *items_ptr = (DWORD)pfn_op_new2(120);
                                    if (*items_ptr) {
                                        *(DWORD*)*items_ptr = (DWORD)particle;
                                        *count_ptr = 1;
                                        /* Zero iterator array (+0x08, 0x400 bytes) */
                                        memset((void*)(list + 0x08), 0, 0x400);
                                    }
                                } else {
                                    /* Subsequent: just write to pre-allocated array */
                                    ((DWORD*)*items_ptr)[*count_ptr] = (DWORD)particle;
                                    (*count_ptr)++;
                                }
                            }
                        }
                    }
                    /* Play tar sound — every time particles are created (no cooldown).
                     * Native DizzyBoard_Update plays Sound_Play3D inside the
                     * if(ball+0x2BC==0) block with no cooldown. */
                    {
                        g_gluebie_sound_pending = 1;
                        g_gluebie_snd_x = bx;
                        g_gluebie_snd_y = by;
                        g_gluebie_snd_z = bz;
                    }
                    /* Set first-contact flag so particles+sound only fire once */
                    g_gluebie_particles_created_ball = ball;
                    /* Debug: log particle creation result */
                    {
                        FILE* df3 = fopen("mknp_custom_entities_debug.log", "a");
                        if (df3) {
                            fprintf(df3, "  >> PARTICLES CREATED: part_count after=%d\n",
                                *(int*)(ball + 0x814));
                            fclose(df3);
                        }
                    }
                }
                /* Do NOT set ball+0x260 — that causes "sweat" not tarsplotch */
                /* Do NOT clear any flags when leaving range — native never clears them */
            }
            /* v55j_15: Reset particle flag when ball leaves Gluebie zone.
             * Native Dizzy sets ball+0x2BC=0 when out of range, allowing
             * re-creation of particles on re-entry (stacking up to 30 max). */
            if (!in_outer && g_gluebie_particles_created_ball == ball) {
                g_gluebie_particles_created_ball = 0;
            }
        }
    }
}

/* v55k_2: Tarpit proximity check — replicates native N:TARPIT collision behavior.
 *
 * Native N:TARPIT flow (Ghidra-verified):
 *
 * 1. DispatchCollisionEvents (0x40C5D0) checks mesh name == "N:TARPIT":
 *    - On FIRST entry (ball+0x2CC==0):
 *      a) Play tar sound: Sound_Play3D(board+0x878+0x484, ball.x, ball.y, ball.z)
 *      b) Store entry Y: ball+0x2D0 = ball+0x168
 *    - Set ball+0x2CC = 1 (in_tar)
 *    - Set ball+0x768 = 0 (disable control)
 *
 * 2. Board update (DizzyBoard_Update 0x41D512 or FUN_00420da0 Master):
 *    - When in_tar==1 (ball+0x2CC!=0):
 *      a) ball+0x168 -= 0.25 (sink down, _DAT_004CF380 float)
 *      b) Play tar sound again (Sound_Play3D at board+0x878+0x484)
 *      c) Create tarsplotch particles (3x operator_new(0x14), type=6)
 *      d) Death check: ball+0x168 < ball+0x2D0 - ball+0x284 * 2.5 (double _DAT_004CF378)
 *         If dead: call FUN_00405190 (Ball_Shatter)
 *    - ball+0x2BC (first contact flag) set to 1
 *
 * We replicate ALL of this via proximity check in the Present hook.
 * The sinking, sound, particles, and death check all happen here. */
static void cEnt_tarpit_proximity_check(DWORD board) {
    if (!board || g_tarpit_count <= 0) return;

    /* Get ball AthenaList at board+0x29D4 */
    DWORD ball_list = board + 0x29D4;
    if (IsBadReadPtr((void*)(ball_list + 0x04), 4)) return;
    int ball_count = *(int*)(ball_list + 0x04);
    if (ball_count <= 0 || ball_count > 20) return;
    if (IsBadReadPtr((void*)(ball_list + 0x40C), 4)) return;
    DWORD* ball_data = *(DWORD**)(ball_list + 0x40C);
    if (!ball_data || IsBadReadPtr(ball_data, ball_count * 4)) return;

    /* Get sound channel at board+0x878+0x484 (same as native N:TARPIT handler).
     * board+0x878 is the parent/sound struct, NOT App. */
    DWORD snd_parent = *(DWORD*)(board + 0x878);
    DWORD sound_channel = 0;
    if (snd_parent && !IsBadReadPtr((void*)(snd_parent + 0x484), 4)) {
        sound_channel = *(DWORD*)(snd_parent + 0x484);
    }

    int i, j;
    for (j = 0; j < g_tarpit_count; j++) {
        DWORD tarpit = g_tarpit_objs[j];
        if (!tarpit || tarpit < 0x10000) continue;
        if (IsBadReadPtr((void*)tarpit, 0x10D0)) continue;

        /* Read tarpit position from PopCylinder+0x10D4/10D8/10DC (spawn position) */
        float tx = *(float*)(tarpit + 0x10D4);
        float ty = *(float*)(tarpit + 0x10D8);
        float tz = *(float*)(tarpit + 0x10DC);

        for (i = 0; i < ball_count; i++) {
            DWORD ball = ball_data[i];
            if (!ball || ball < 0x10000) continue;
            if (IsBadReadPtr((void*)ball, 0x2D4)) continue;

            /* Ball position */
            float bx = *(float*)(ball + 0x164);
            float by = *(float*)(ball + 0x168);
            float bz = *(float*)(ball + 0x16C);
            float radius = *(float*)(ball + 0x284);

            /* Check if ball is in tar */
            char in_tar = *(char*)(ball + 0x2CC);

            if (in_tar) {
                /* Ball is sinking — apply tar sinking behavior */
                /* 1. Sink ball Y by 0.25/frame (native _DAT_004CF380) */
                *(float*)(ball + 0x168) = by - 0.25f;

                /* 2. Clear ball+0x2E9 (penetration death flag) — prevents
                 * Ball_Update from seeing the ball "penetrating" the tar mesh
                 * and triggering the dizzy/shatter death. The actual death
                 * happens via our FUN_00405190 call below, not via penetration. */
                *(BYTE*)(ball + 0x2E9) = 0;

                /* 3. Death check: ball+0x168 < ball+0x2D0 - ball+0x284 * 2.5 */
                float entry_y = *(float*)(ball + 0x2D0);
                float death_depth = entry_y - radius * 2.5f;
                if (*(float*)(ball + 0x168) < death_depth) {
                    /* Ball has sunk too deep — call native Ball_Respawn (FUN_00405190).
                     * This is __fastcall(ECX=ball), RET 0.
                     * It: sets ball+0x2F8=1 (death flag), clears velocity,
                     *     clears in_tar (ball+0x2CC=0), finds nearest SAFESPOT,
                     *     teleports ball there. No shatter/dizzy — just respawn. */
                    typedef void (__fastcall *respawn_t)(DWORD);
                    static respawn_t pfn_respawn = NULL;
                    if (!pfn_respawn) {
                        pfn_respawn = (respawn_t)0x00405190;  /* Ball_Respawn */
                    }
                    pfn_respawn(ball);
                }
                continue;  /* Ball is already in tar — skip entry check */
            }

            /* Ball not in tar — check if it entered the tarpit zone */
            /* Horizontal distance (X/Z plane) */
            float dx = tx - bx;
            float dz = tz - bz;
            float dist_sq = dx*dx + dz*dz;

            /* Trigger radius: ball radius + margin = 30 units (same as CollisionFace proximity) */
            /* Also check Y proximity — ball must be near the tar surface */
            float dy = ty - by;
            if (dist_sq < 900.0f && fabsf(dy) < 30.0f) {
                /* Ball entered tarpit — replicate DispatchCollisionEvents N:TARPIT handler */

                /* Store entry Y position (ball+0x2D0 = ball+0x168) */
                *(float*)(ball + 0x2D0) = by;

                /* Set in_tar flag (ball+0x2CC = 1) */
                *(BYTE*)(ball + 0x2CC) = 1;

                /* Clear ball+0x768 (disable control) */
                *(DWORD*)(ball + 0x768) = 0;

                /* Play tar sound (Sound_Play3D at board+0x878+0x484) */
                if (sound_channel && !IsBadReadPtr((void*)sound_channel, 0x20)) {
                    /* Sound_Play3D is __thiscall(ECX=sndChannel, float x, float y, float z, float scale)
                     * RET 0x10 (16 bytes = 4 stack params). Scale defaults to 1.0. */
                    typedef void (__thiscall *play3d_t)(DWORD, float, float, float, float);
                    static play3d_t pfn_play3d = NULL;
                    if (!pfn_play3d) {
                        pfn_play3d = (play3d_t)0x00459860;  /* Sound_Play3D */
                    }
                    pfn_play3d(sound_channel, bx, by, bz, 1.0f);
                }
            }
        }
    }
}

/* v55n: TarBubble decorative bubble driver (was proximity-sink in v55m).
 * The native TarBubble is PURELY DECORATIVE — it occasionally floats a
 * bubble sprite up from the tar. It does NOT sink, slow, or trap the ball
 * (that's Gluebie 43 = slowdown, Tarpit 44 = sinking).
 *
 * Native flow (DizzyBoard_Update 0x41D512 / Master FUN_00420da0):
 *   every frame, RNG()==10 → operator_new(0x1C) + FUN_0044fb50(app, x, y, z)
 *   + AthenaList_Append(board+0x3B00). The board update then drives each
 *   bubble: vtable[1] (update: scale-=5%/frame, lifetime-=1) + vtable[2]
 *   (render: rises scale*60/frame, pops with bubble1 sound when done).
 *
 * Since board+0x3B00 is only driven by Dizzy/Master updates, we self-drive
 * here on ALL boards from the Present hook: occasionally spawn, then
 * update+render each live bubble. The bubble uses the native sprite
 * (app+0x5A4 "Meshes\\tarbubble", loaded by TimerDisplay for every board). */
static void cEnt_tarbubble_update_decoration(DWORD board) {
    if (!board || g_tarbubble_count <= 0) return;
    if (game_is_quitting()) return;

    /* v55m_44d: Pause gate — decorative bubbles must freeze during ESC pause. */
    if (*(BYTE*)(board + 0x874) != 0) return;

    DWORD app = *(DWORD*)(board + BOARD_APP);
    if (!app || IsBadReadPtr((void*)app, 0x5B0)) return;

    int j;
    for (j = 0; j < g_tarbubble_count; j++) {
        TarBubblePos* bp = &g_tarbubble_pos[j];

        /* Bubble expired → clean up (native pops + frees) */
        if (bp->obj) {
            if (IsBadReadPtr((void*)(bp->obj + TARBUBBLE_SIZE), 4) ||
                *(DWORD*)bp->obj == 0) {
                bp->obj = 0;
                continue;
            }
            /* Let the native update run: scale*=0.95, lifetime-=1, pop+free */
            if (pfn_TarBubble_update) {
                pfn_TarBubble_update(bp->obj);
            }
            /* After update the object is freed if lifetime hit 0. Detect by
             * reading lifetime (and render only while alive). */
            if (IsBadReadPtr((void*)(bp->obj + 0x18), 4)) {
                bp->obj = 0;
                continue;
            }
            float lifetime = *(float*)(bp->obj + 0x18);
            if (lifetime <= 0.0f) {
                /* Pop complete — free via native dtor (update only plays the
                 * pop sound; the list owner normally frees). */
                if (pfn_TarBubble_dtor) {
                    pfn_TarBubble_dtor(bp->obj, 1);
                }
                bp->obj = 0;
                continue;
            }
            /* Native render: rises scale*60/frame via Gfx_SetPosition */
            if (pfn_TarBubble_render) {
                pfn_TarBubble_render(bp->obj);
            }
            continue;
        }

        /* No live bubble → countdown to next spawn (native ~10%/frame,
         * but we use a small fixed interval so it's visible but not spammy) */
        if (bp->countdown > 0) {
            bp->countdown--;
            continue;
        }

        /* Spawn a new decorative bubble at the ref point */
        if ((rand() % 100) < 12) {   /* ~12% per frame, native uses RNG==10 */
            void* obj = pfn_operator_new(TARBUBBLE_SIZE);
            if (obj) {
                /* ctor copies pos + sets scale/lifetime; obj lifetime starts 25..50 */
                void* made = pfn_TarBubble_ctor(obj, app, bp->x, bp->y, bp->z);
                if (made) {
                    bp->obj = (DWORD)made;
                    if (logf) fprintf(logf, "  TARBUBBLE: spawned decorative bubble at (%.1f,%.1f,%.1f) obj=0x%08X\n",
                            bp->x, bp->y, bp->z, bp->obj);
                } else {
                    /* ctor failed — don't leak, try again later */
                    bp->obj = 0;
                }
            }
            bp->countdown = 6 + (rand() % 20);  /* 6..25 frames between bubbles */
        } else {
            bp->countdown = 1;
        }
    }
}

/* v55f: WaterWheel per-frame update — rotates the mesh each frame.
 * Native DizzyBoard_Update (0x41D512) does:
 *   angle = board+0x4BBC; angle -= 0.5; board+0x4BBC = angle;
 *   Gfx_RotateY(stack_matrix, angle);          // 0x457C90
 *   Gfx_SetPosition(stack_matrix2, x, y, z);    // 0x457B50  
 *   mesh->vtable[22]();                          // apply transform
 *   mesh->vtable[21](&stack_matrix);             // set rotation
 *
 * We replicate this by calling Gfx_RotateY into a local matrix buffer,
 * then calling the mesh's vtable[22] and vtable[21]. */
static void cEnt_waterwheel_update(DWORD board) {
    if (!board || g_waterwheel_count <= 0) return;

    /* v55m_44d: Pause gate — stop rotation AND sound while the game is
     * paused. Native Dizzy's waterwheel update lives inside
     * DizzyBoard_Update (0x41D512), which does not run while paused
     * (Scene_Update reads board+0x874; Scene_CreateGameOverMenu sets it).
     * Same flag the catapult uses at line ~4473. */
    if (!IsBadReadPtr((void*)(board + 0x874), 1) && *(BYTE*)(board + 0x874) != 0) {
        return;  /* paused → wheel freezes, no creak */
    }

    /* Get ball for force application */
    DWORD ball = *(DWORD*)(board + 0x361C);
    float bx = 0, by = 0, bz = 0;
    int ball_valid = 0;
    if (ball && ball > 0x10000 && !IsBadReadPtr((void*)ball, 0x200)) {
        bx = *(float*)(ball + 0x164);
        by = *(float*)(ball + 0x168);
        bz = *(float*)(ball + 0x16C);
        ball_valid = 1;
    }

    int i;
    for (i = 0; i < g_waterwheel_count; i++) {
        struct WaterWheelState* ww = &g_waterwheels[i];
        if (!ww->active || !ww->pc_obj) continue;
        if (IsBadReadPtr((void*)ww->pc_obj, 0x440)) continue;

        /* v55m_44c: Rotation direction REVERSED per user request.
         * Native Dizzy decrements (angle -= 0.5/frame, constant 0x4CF3F0),
         * user wants the opposite direction → increment instead. */
        ww->angle += 0.5f;

        /* v55m_44c: Play WheelCreak per frame (like native Dizzy at 0x41DC22).
         * Sound_Play3D(channel, x, y, z, scale=1.0) — __thiscall, RET 0x10.
         * Only if a channel was successfully cached at spawn; NULL → silent.
         * Runs on the background thread (same as tar sound — proven safe). */
        if (ww->creak_channel && ww->creak_channel > 0x10000 &&
            !IsBadReadPtr((void*)ww->creak_channel, 0x20) && pfn_Sound_Play3D) {
            pfn_Sound_Play3D((void*)ww->creak_channel, ww->x, ww->y, ww->z, 1.0f);
        }

        /* v55m_26: Waterwheel rotation via vtable[18] hook (same as Chomper).
         * EntityTransform didn't work for PopCylinder.
         * The vtable[18] hook calls Timer + Gfx_Scale + Gfx_ScaleZ.
         * For now, store angle in a global that the hook can read.
         * TODO: implement vtable[18] hook for Waterwheel too.
         * For now, just update the angle — the hook will be added next. */
        /* No EntityTransform writes — doesn't work for PopCylinder */

        /* v55m_1: Apply force to ball when near waterwheel (native DizzyBoard_Update behavior).
         * Native: Ball_ApplyForceV2(ball, dirX, dirY, dirZ, 0.1)
         * Force direction = ball position - waterwheel position (outward push)
         * Guards: ball+0x2F9==0, ball+0x2CC==0, ball+0x808==0, ball+0x2F0<0x51 */
        if (ball_valid) {
            float dx = bx - ww->x;
            float dy = by - ww->y;
            float dz = bz - ww->z;
            float dist_sq = dx*dx + dy*dy + dz*dz;

            /* Check proximity (within 500.0 units — same as Ball_CheckProximity constant) */
            if (dist_sq < 250000.0f) {
                /* v55m_1: Set waterwheel embed mode when ball is very close (< 60 units).
                 * Native: N:WHEELEMBED sets ball+0x808=50 (0x32) which makes the ball
                 * stick to the waterwheel and spin with it. */
                if (dist_sq < 3600.0f) {  /* 60^2 = 3600 */
                    *(DWORD*)(ball + 0x808) = 50;  /* waterwheel embed mode */
                    *(BYTE*)(ball + 0xC3C) = 1;     /* on waterwheel flag */
                }
                /* Check guards */
                BYTE drowning = *(BYTE*)(ball + 0x2F9);
                BYTE in_tar = *(BYTE*)(ball + 0x2CC);
                DWORD water_mode = *(DWORD*)(ball + 0x808);
                DWORD dizzy_timer = *(DWORD*)(ball + 0x2F0);
                if (!drowning && !in_tar && dizzy_timer < 0x51) {
                    /* Normalize direction and apply force 0.1 magnitude.
                     * When in embed mode (ball+0x808=50), force is applied differently —
                     * the ball orbits the waterwheel center. */
                    float dist = sqrtf(dist_sq);
                    if (dist > 0.001f) {
                        float force_mag = 0.1f;
                        float fx = (dx / dist) * force_mag;
                        float fy = (dy / dist) * force_mag;
                        float fz = (dz / dist) * force_mag;
                        /* Accumulate force into ball velocity (ball+0xCA4/CA8/CAC) */
                        *(float*)(ball + 0xCA4) += fx;
                        *(float*)(ball + 0xCA8) += fy;
                        *(float*)(ball + 0xCAC) += fz;
                    }
                }
            }
        }
    }
}

/* v55m_3: Chomper per-frame update — replicates TowerBoard_Update chomper state machine.
 * Ghidra-verified native state machine (Tower_Update 0x41E760):
 * State 0 (IDLE/GROWING): jaw *= 1.2/frame (64-bit double!). When > 25.0: jaw=25.0, state→1, timer=25, anim_val=50.0
 * State 1 (BITING/HOLDING): timer--. When < 1: state→2, play "sounds\chomp"
 * State 2 (CLOSING): jaw *= 0.5/frame. When ≤ 1.0: jaw=0, RNG → state 3 (timer=RNG+100) or back to 0
 * State 3 (OPENING/RECOVERING): anim_val -= 2.0/frame. timer-- → when < 1: state→0
 * Also: phase += 3.0/frame (for Wave_Sin rendering oscillation)
 * E:BITE: when ball touches E:BITE plane → state=0, jaw=25.0 (force restart from full open)
 * Rendering (Scene_RenderWithCamera 0x40DFA0): mesh rendered TWICE (normal + mirrored X scale 180.0, offset -35.0),
 *   jaw oscillates via Wave_Sin(phase) * 10.0, scale 1.15 all axes, Z scale = -jaw_angle */
static void cEnt_chomper_update(DWORD board) {
    if (!board || g_chomper_count <= 0) return;

    DWORD ball = *(DWORD*)(board + 0x361C);
    float bx = 0, by = 0, bz = 0;
    int ball_valid = 0;
    if (ball && ball > 0x10000 && !IsBadReadPtr((void*)ball, 0x200)) {
        bx = *(float*)(ball + 0x164);
        by = *(float*)(ball + 0x168);
        bz = *(float*)(ball + 0x16C);
        ball_valid = 1;
    }

    DWORD app = *(DWORD*)(board + BOARD_APP);

    /* v55m_25b: Open debug log for per-frame Chomper state logging */
    static int chomper_frame_count = 0;
    chomper_frame_count++;
    /* Log every 30 frames (twice per second at 60fps) */
    int do_log = (chomper_frame_count % 30 == 0);
    FILE* chomp_log = NULL;
    if (do_log) {
        chomp_log = fopen("chomper_debug.log", "a");
        if (chomp_log) {
            fprintf(chomp_log, "=== Frame %d (ball_valid=%d, ball_pos=%.1f,%.1f,%.1f) ===\n",
                    chomper_frame_count, ball_valid, bx, by, bz);
        }
    }

    int i;
    for (i = 0; i < g_chomper_count; i++) {
        ChomperState* cs = &g_chompers[i];
        if (!cs->obj) continue;
        if (IsBadReadPtr((void*)cs->obj, 0x100)) {
            if (chomp_log) fprintf(chomp_log, "  CHOMPER[%d]: SKIP obj=0x%08X is bad read ptr\n", i, cs->obj);
            continue;
        }

        /* Update phase (for future jaw animation) */
        cs->phase += 3.0f;

        /* E:BITE proximity check — when ball is very close, force bite restart. */
        int bite_triggered = 0;
        if (ball_valid) {
            float dx = bx - cs->x;
            float dy = by - cs->y;
            float dz = bz - cs->z;
            float dist_sq = dx*dx + dy*dy + dz*dz;
            if (dist_sq < 1600.0f) {  /* 40^2 = 1600 — bite range */
                cs->state = 0;
                cs->jaw_angle = 25.0f;
                bite_triggered = 1;
            }
        }

        int prev_state = cs->state;

        /* State machine — runs on main thread (Present hook).
         * Sound is safe to play here. No Gfx/Timer/D3D calls. */
        switch (cs->state) {
            case 0: /* IDLE/GROWING */
                if (cs->jaw_angle == 0.0f) cs->jaw_angle = 0.25f;
                cs->jaw_angle *= 1.2f;
                if (cs->jaw_angle > 25.0f) {
                    cs->jaw_angle = 25.0f;
                    cs->state = 1;
                    cs->countdown = 25;
                    cs->anim_val = 50.0f;
                }
                break;
            case 1: /* BITING/HOLDING */
                cs->countdown--;
                if (cs->countdown < 1) {
                    cs->state = 2;
                    if (app && pfn_Sound_Play3D) {
                        DWORD snd = *(DWORD*)(app + 0x4A8);
                        if (snd && snd > 0x10000 && !IsBadReadPtr((void*)snd, 0x20)) {
                            pfn_Sound_Play3D((void*)snd, cs->x, cs->y, cs->z, 1.0f);
                        }
                    }
                }
                break;
            case 2: /* CLOSING */
                cs->jaw_angle *= 0.5f;
                if (cs->jaw_angle <= 1.0f) {
                    cs->jaw_angle = 0.0f;
                    if ((rand() % 90) == 0) {
                        if (app && pfn_Sound_Play3D) {
                            DWORD snd = *(DWORD*)(app + 0x4A8);
                            if (snd && snd > 0x10000 && !IsBadReadPtr((void*)snd, 0x20)) {
                                pfn_Sound_Play3D((void*)snd, cs->x, cs->y, cs->z, 1.0f);
                            }
                        }
                    }
                    cs->countdown = (rand() % 100) + 100;
                    cs->state = 3;
                }
                break;
            case 3: /* OPENING/RECOVERING */
                cs->anim_val -= 2.0f;
                if (cs->anim_val < 0.0f) cs->anim_val = 0.0f;
                cs->countdown--;
                if (cs->countdown < 1) {
                    cs->state = 0;
                }
                break;
        }

        /* v55m_25b: Log state machine transitions */
        if (chomp_log) {
            const char* state_names[] = {"GROWING", "HOLDING", "CLOSING", "RECOVERING"};
            fprintf(chomp_log, "  CHOMPER[%d]: obj=0x%08X state=%d(%s)->%d(%s) jaw=%.3f phase=%.1f countdown=%d anim=%.1f bite=%d\n",
                    i, cs->obj,
                    prev_state, (prev_state >= 0 && prev_state <= 3) ? state_names[prev_state] : "?",
                    cs->state, (cs->state >= 0 && cs->state <= 3) ? state_names[cs->state] : "?",
                    cs->jaw_angle, cs->phase, cs->countdown, cs->anim_val, bite_triggered);
        }

        /* v55m_26: Jaw rotation is applied via vtable[18] hook
         * (cEnt_chomper_render) during the render pass.
         * The hook calls Timer_Init + Gfx_Scale(1,1,1) + Gfx_ScaleZ(-jaw)
         * + original render + Timer_Cleanup.
         * No EntityTransform writes needed. */
    }

    if (chomp_log) {
        fprintf(chomp_log, "\n");
        fclose(chomp_log);
    }
}
typedef struct {
    DWORD obj;
    entity_def_t def;
    int active;
} tracked_entity_t;

static tracked_entity_t g_tracked[MAX_ROTATERS];
static int g_tracked_count = 0;

static void process_rotaters(DWORD board, FILE* logf) {
    if (!board) return;

    DWORD sceneobj = cEnt_get_sceneobj(board);
    if (!sceneobj) {
        if (logf) fprintf(logf, "  ROTATER: sceneobj=NULL\n");
        return;
    }

    int found = 0;
    int i;
    
    /* === Scan section 3 objects (sceneobj+0xCA0) for cEnt entries === */
    /* NOTE: S1 ref points and section 3 share the SAME array at sceneobj+0xCA0.
     * Do NOT scan S1 separately — it causes double-spawn. */
    if (IsBadReadPtr((void*)(sceneobj + SCENEOBJ_OBJ_COUNT), 4)) {
        if (logf) fprintf(logf, "  S3: sceneobj+0x898 is bad read ptr\n");
        return;
    }
    int obj_count = *(int*)(sceneobj + SCENEOBJ_OBJ_COUNT);
    if (logf) fprintf(logf, "  S3: obj_count=%d (sceneobj=0x%08X)\n", obj_count, sceneobj);
    if (obj_count <= 0 || obj_count > 1000) return;

    if (IsBadReadPtr((void*)(sceneobj + SCENEOBJ_OBJ_ARRAY), 4)) {
        if (logf) fprintf(logf, "  S3: sceneobj+0xCA0 is bad read ptr\n");
        return;
    }
    DWORD* obj_array_ptr = *(DWORD**)(sceneobj + SCENEOBJ_OBJ_ARRAY);
    if (!obj_array_ptr || IsBadReadPtr(obj_array_ptr, obj_count * 4)) {
        if (logf) fprintf(logf, "  S3: obj_array_ptr bad (0x%08X)\n", (DWORD)obj_array_ptr);
        return;
    }
    if (logf) fprintf(logf, "  S3: array at 0x%08X, scanning %d entries...\n", (DWORD)obj_array_ptr, obj_count);

    for (i = 0; i < obj_count; i++) {
        DWORD obj_ptr = obj_array_ptr[i];
        if (!obj_ptr || obj_ptr < 0x10000) {
            if (logf) fprintf(logf, "    [%d] skip: ptr=0x%08X\n", i, obj_ptr);
            continue;
        }
        if (IsBadReadPtr((void*)obj_ptr, 20)) {
            if (logf) fprintf(logf, "    [%d] skip: bad read ptr 0x%08X\n", i, obj_ptr);
            continue;
        }

        /* Read object name pointer */
        char* name = *(char**)(obj_ptr);
        if (!name || IsBadReadPtr(name, 4)) {
            if (logf) fprintf(logf, "    [%d] skip: name ptr bad (0x%08X)\n", i, (DWORD)name);
            continue;
        }

        /* Log every entry name (first 80 chars) */
        char debug_name[81] = {0};
        strncpy(debug_name, name, 80);
        debug_name[80] = 0;
        if (logf) fprintf(logf, "    [%d] name='%s' ptr=0x%08X pos=(%.1f, %.1f, %.1f)\n",
                i, debug_name, obj_ptr,
                *(float*)(obj_ptr + 0x04), *(float*)(obj_ptr + 0x08), *(float*)(obj_ptr + 0x0C));

        /* Check for cEnt prefix (case-insensitive) */
        if (_strnicmp(name, "cEnt", 4) != 0 &&
            _strnicmp(name, "REF:cEnt", 8) != 0) continue;

        /* Parse <ENTITY> name </ENTITY> */
        char entity_name[64] = {0};
        {
            char* ent_start = my_stristr(name, "<ENTITY>");
            if (ent_start) {
                ent_start += 8;  /* skip "<ENTITY>" (8 chars including '>') */
                while (*ent_start == ' ' || *ent_start == '\t') ent_start++;
                char* ent_end = my_stristr(ent_start, "</ENTITY>");
                if (!ent_end) ent_end = ent_start + strlen(ent_start);
                size_t ent_len = ent_end - ent_start;
                if (ent_len > 0 && ent_len < 64) {
                    strncpy(entity_name, ent_start, ent_len);
                    entity_name[ent_len] = 0;
                    while (ent_len > 0 && (entity_name[ent_len-1] == ' ' || entity_name[ent_len-1] == '\t'))
                        entity_name[--ent_len] = 0;
                }
            }
        }
        if (!entity_name[0]) continue;

        /* Read position */
        float px = *(float*)(obj_ptr + 0x04);
        float py = *(float*)(obj_ptr + 0x08);
        float pz = *(float*)(obj_ptr + 0x0C);

        /* Match entity name to AI list (alphabetically sorted) */
        int ai_type = -1;
        const char* ai_mesh = NULL;

        /* AI List — sorted alphabetically by entity name
         *
         * ctor_type: 0=PopCylinder, 1=Rotator, 2=Pendulum, 3=Looper, 4=Gear, 5=BigGear, 6=Swirl
         *
         * Mesh paths use "levels\\..." for .MESHWORLD files (loaded via MeshWorld_ctor).
         * .MESH-only entities (8ball, Bell, Chomper, etc.) can't be loaded via
         * MeshWorld_ctor — they need MeshNode_ctor (0x471C20, 0x18 byte struct).
         * For those, mesh=NULL and we try loading the .MESH file path directly.
         * MeshWorld_ctor will fail gracefully (returns NULL → spawn skipped).
         *
         * v52 changes:
         *   - Fixed 20+ wrong/NULL mesh paths that caused Swirl fallback
         *   - Added Sign, Windmill, Droplifter AIs
         *   - Lifter now uses Up Race model (LevelUp-Lifter)
         *   - Droplifter uses Odd Race model (Level6-Lifter)
         *   - Rotator uses constant rotation (ROS_Y=0)
         *   - Gear/Looper: AI type changed to 0 (PopCylinder) to prevent crash
         *   - .MESH entities: try loading via MeshWorld_ctor with meshes\\ path
         */
        typedef struct { const char* name; int ctor_type; const char* mesh; } ai_entry_t;
        static const ai_entry_t ai_list[] = {
            /* name */            /* ctor */  /* mesh path */
            { "8ball",            15, "meshes\\8ball" },             /* BadBall_ctor (0x40AFE0, 0xC70 bytes) — 2 params: this, board */
            { "BBridge",          0, "levels\\Level10-Bridge1" },   /* BreakBridge_ctor */
            { "Bell",             0, "meshes\\bell" },              /* PopCylinder — Bell_ctor crashes (Level_ctor has no mesh, vtable update calls LoadMesh) */
            { "Blockdawg",        0, "levels\\Level8-BlockDawg1" }, /* Blockdawg_ctor */
            { "Bonk",             33, "levels\\Level5-Bonk" },       /* Bonk_ctor (0x438850, 0x1200) — self-loads level5-bonk MESHWORLD */
            { "Bridge",            0, "levels\\Level2-Bridge" },     /* BreakBridge_ctor: Pendulum vtable with tilt animation */
            { "Bridgeslam",       16, "levels\\Level2-Bridge" },     /* Alias for Bridge */
            { "Bumper",           0, "levels\\Bumper01" },          /* N:BUMPER tag — no _ctor, _default mesh */
            { "Catapult",         35, "levels\\Level4-Catapult" },   /* Catapult_ctor (0x437E10, 0x1108) */
            { "Chomper",           0, "levels\\Chomper" },          /* v55m_11: PopCylinder with Chomper.MESHWORLD (user-provided) */
            { "Chrome",           23, "meshes\\Sphere" },          /* v55: was _default, now Sphere.MESH for Chrome ball */
            { "Cloudscape",        0, "levels\\Cloudscape" },       /* Cloudscape (Sprite_ctor, 0x45D0C0, 0xD4) — Sky Race clouds */
            { "Drawbridge",        0, "levels\\Level4-Drawbridge" }, /* Glass_Level_ctor (0x4384A0, 0x113C bytes) */
            { "Droplifter",       42, "levels\\Level6-Lifter" },     /* Odd_Lifter_ctor (0x434E60, 0x10F4) */
            { "Fan",              0, "meshes\\fanbody" },           /* PopCylinder — Fan_ctor crashes (Level_ctor has no mesh) */
            { "Flag",              0, "levels\\\\Flag" },             /* Wavy_ctor: same as Flag2, loads Flag.MESHWORLD or _default */
            { "Flag2",             0, "levels\\Flag" },               /* WavyFlag2: Wavy_ctor copy, uses Flag.MESHWORLD or _default fallback */
            { "Flickfloor1",      7,  "levels\\LevelDark-DFloor1" },  /* cEnt_DFloor1_ctor (ArenaStands_ctor, 0x43E450, 0x1104) */
            { "Flickfloor2",     19, "levels\\LevelDark-DFloor4" },  /* cEnt_DFloor4_ctor (ArenaStands + post-config: obj+0x10DC=2, obj+0x10E0=0) */
            { "Flickring",       20, "levels\\LevelDark-Flickring" }, /* cEnt_FlickRing_ctor (ArenaStands_ctor) */
            { "Funball",          24, "meshes\\funball" },           /* Funball_ctor: no _ctor, board-level behavior, PopCylinder fallback */
            { "Gear",              0, "levels\\LevelImpossible-Gear" }, /* Gear_ctor (0x437690, 0x1514, 9 params!) */
            { "Glassbreaker",     11, "meshes\\GlassBonus" },       /* Secret_ctor (0x43DFB0, 0x10EC bytes) */
                                    { "Gluebie",          43, "levels\\Level3-Gluebie" },    /* Gluebie_ctor (0x437CB0, 0x110C) */
            { "Judge",            10, "meshes\\hammyjudge" },       /* Gear_Level_ctor (0x43A150, 0x1100 bytes, no mesh param) */
            { "Lifter",           38, "levels\\LevelUp-Lifter" },    /* Lifter_ctor (0x436920, 0x10F4) */
            { "Looper",            0, "levels\\LevelImpossible-Looper" }, /* Looper_ctor (0x435800, 0x1500, 6 params) */
            { "Mace",              36, "levels\\Level4-Mace" },       /* Mace_ctor (0x438750, 0x110C) */
            { "Mag",              0, "meshes\\magnifyingglass" },   /* .MESH — Magnifier_ctor */
            { "Mousetrap",        0, "levels\\MouseTrap" },        /* MouseTrap_ctor */
            { "Neonplatform",     40, "levels\\LevelDark-NeonPlatform" }, /* NeonPlatform_ctor (0x43E110, 0x1104) */
            { "Pendulum",         2, "levels\\LevelImpossible-Pendulum" }, /* Pendulum_ctor */
            { "Popcylinder",      0, "levels\\Level9-PopCylinder1" }, /* PopCylinder_ctor */
            { "Rotator",          1, "levels\\LevelImpossible-Rotator" }, /* Rotator_ctor (constant rotation) */
            { "Saw",              0, "levels\\Level8-Saw" },        /* Saw_ctor */
            { "Sawblade",         0, "meshes\\sawblade" },         /* PopCylinder — SawBlade_ctor crashes (Level_ctor has no mesh) */
            { "Sign",             13, "levels\\PopupSign" },        /* Sign_ctor (0x443B90, 0x10FC bytes, complex signature) */
            { "Speedcylinder",    39, "levels\\LevelUp-SpeedCylinder" }, /* SpeedCylinder_ctor (0x436A20, 0x150C) */
            { "Spinner",           0, "levels\\Level8-Spinny" },     /* Spinner_Level_ctor (0x4396F0, 0x10FC) */
            { "Swirl",            6, "levels\\\\Level3-Swirl" },      /* Rotator_ctor_Impossible */
            { "Tarbubble",        25, "levels\\_default" },         /* v55n: DECORATIVE floating bubble (native ctor 0x44FB50, vtable 0x4D6E48). Does NOT slow/sink — Gluebie(43) slows, Tarpit(44) sinks. Self-driven on all boards. */
            { "Tarpit",           44, "levels\\_default" },          /* v55k_1: N:TARPIT behavior via proximity, PopCylinder spawn + tar sinking */
            { "Timebutton",       45, "levels\\LevelUp-Button" },    /* TimeButton_ctor (0x436C10, 0x10E8) */
            { "Tipper",            37, "levels\\Level3-Tipper" },     /* Tipper_ctor (0x437960, 0x1104) */
            { "Trapdoor",         41, "levels\\Level4-Trapdoor1" },  /* Trapdoor_ctor (0x438290, 0x10F8) */
            { "Trode",            21, "levels\\LevelDark-Trode" },   /* cEnt_Trode_ctor (ArenaStands_ctor) */
            { "Waterwheel",       26, "levels\\\\Waterwheel" },      /* v55m_44f: user provides Waterwheel.MESHWORLD; falls back to levels\_default if missing */
            { "Wavy",             0, "levels\\Level7-Wavy1" },      /* Wavy_ctor */
            { "Windmill",         0, "levels\\Level4-Windmill" },   /* Tower: Level_RenderCtor + TipperVisual_Attach */
            { "Wobbly",            0, "levels\\Level7-Wobbly1" },    /* GameLevel_ctor (0x4351F0, 0x1524 bytes) */
        };
        static const int ai_list_count = sizeof(ai_list) / sizeof(ai_list[0]);

        for (int j = 0; j < ai_list_count; j++) {
            if (_stricmp(entity_name, ai_list[j].name) == 0) {
                ai_type = ai_list[j].ctor_type;
                ai_mesh = ai_list[j].mesh;
                break;
            }
        }

        if (ai_type < 0) {
            if (logf) fprintf(logf, "  cEnt(S3): '%s' — no matching AI, skipping\n", entity_name);
            continue;
        }

        /* v55: Skip cEnt Swirl (ai_type==6) spawning on levels that natively
         * have SWIRL objects. Dizzy Race (index 2), Master Race (index 13),
         * and Dizzy Arena have native SWIRL ref points. Spawning additional
         * Swirls from cEnt entries causes duplicates.
         * Only skip Swirl — other cEnt types (8ball, Bell, etc.) still spawn. */
        if (ai_type == 6 || cEnt_ci_strstr(entity_name, "Swirl")) {
            int skip_swirl = 0;
            DWORD app = *(DWORD*)(board + BOARD_APP);
            if (app && !IsBadReadPtr((void*)(app + 0x5FC), 4)) {
                int race_idx = *(int*)(app + 0x5FC);
                if (race_idx == 2 || race_idx == 13) {
                    skip_swirl = 1;
                    if (logf) fprintf(logf, "  cEnt(S3): '%s' — SKIPPED (Swirl on Dizzy/Master race_idx=%d)\\n",
                            entity_name, race_idx);
                }
            }
            if (!skip_swirl) {
                char* level_name = NULL;
                if (!IsBadReadPtr((void*)(board + 0x10), 4)) {
                    level_name = *(char**)(board + 0x10);
                }
                if (level_name && !IsBadReadPtr(level_name, 5)) {
                    if (my_stristr(level_name, "Dizzy") != NULL ||
                        my_stristr(level_name, "Arena") != NULL) {
                        skip_swirl = 1;
                        if (logf) fprintf(logf, "  cEnt(S3): '%s' — SKIPPED (Swirl on level '%s')\\n",
                                entity_name, level_name);
                    }
                }
            }
            if (skip_swirl) {
                continue;
            }
        }

        /* v52: Rotator (AI 1) uses constant rotation (ROS_Y=0).
         * The native render function oscillates at ±2.0 by default.
         * With ROS_Y=0, cEnt_update_constant_rotations() rewrites the direction
         * field every frame to prevent oscillation reversal. */
        float spawn_ros_y = 2.0f;
        if (ai_type == 1) spawn_ros_y = 0.0f;  /* Rotator: constant rotation */

        cEnt_spawn_rotater_at(board, px, py, pz, ai_mesh,
                         0.0f, 1.0f, 0.0f,
                         2.0f, spawn_ros_y, 2.0f,
                         ai_type,
                         logf);
        found++;
    }

    if (found > 0 && logf) fprintf(logf, "  Processed %d cEnt entries\n", found);

    if (logf && found > 0) {
        fprintf(logf, "  ROTATER: spawned %d SWIRL object(s)\n", found);
        fflush(logf);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Init: copy testcube.MESHWORLD to levels\ directory
 * ═══════════════════════════════════════════════════════════════════════════ */

static void init_game_dir(void) {
    char dll_path[MAX_PATH] = {0};
    HMODULE hMod = NULL;

    /* Find our own module handle via VirtualQuery */
    VirtualQuery((void*)&init_game_dir, (PMEMORY_BASIC_INFORMATION)&hMod, sizeof(hMod));
    GetModuleFileNameA(hMod, dll_path, MAX_PATH);

    /* Extract directory from DLL path */
    char* p = strrchr(dll_path, '\\');
    if (p) *p = 0;
    strncpy(g_game_dir, dll_path, MAX_PATH - 1);

    /* Copy testcube.MESHWORLD to levels\ directory */
    char src[MAX_PATH], dst[MAX_PATH];
    snprintf(src, MAX_PATH, "%s\\testcube.MESHWORLD", g_game_dir);
    snprintf(dst, MAX_PATH, "%s\\levels\\testcube.MESHWORLD", g_game_dir);

    /* Create levels\ dir if it doesn't exist */
    char levels_dir[MAX_PATH];
    snprintf(levels_dir, MAX_PATH, "%s\\levels", g_game_dir);
    CreateDirectoryA(levels_dir, NULL);

    /* Only copy if destination doesn't exist or source is newer */
    if (!PathFileExistsA(dst) || GetFileAttributesA(src) != INVALID_FILE_ATTRIBUTES) {
        CopyFileA(src, dst, FALSE);
    }
}

/* Load config from mknp_custom_entities.txt */
static void load_config(void) {
    char config_path[MAX_PATH];
    snprintf(config_path, MAX_PATH, "%s\\mknp_custom_entities.txt", g_game_dir);
    FILE* f = NULL;
    fopen_s(&f, config_path, "r");
    if (!f) return;

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        /* Skip comments and blank lines */
        char* p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '#' || *p == '\n' || *p == '\r' || *p == '\0') continue;

        /* Parse grid_speed */
        if (strncmp(p, "grid_speed", 10) == 0) {
            char* eq = strchr(p, '=');
            if (eq) {
                float val = (float)atof(eq + 1);
                if (val > 0.0f) g_grid_speed = val;
            }
        }
    }
    fclose(f);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Main thread — GRID cycling: spawn current GRID, despawn all others
 * ═══════════════════════════════════════════════════════════════════════════ */

static DWORD WINAPI entity_thread(LPVOID param) {
    init_game_dir();
    load_config();

    /* Open log file */
    snprintf(g_log_path, MAX_PATH, "%s\\mknp_custom_entities.log", g_game_dir);
    FILE* logf = NULL;
    fopen_s(&logf, g_log_path, "a");
    if (logf) {
        fprintf(logf, "=== Custom Entities Mod v55n_22 Started ===\n");
        fprintf(logf, "Game dir: %s\n", g_game_dir);
        fprintf(logf, "Mesh path: %s\n", g_mesh_path);
        fprintf(logf, "Grid speed: %.1f seconds\n", g_grid_speed);
        fclose(logf);
    }

    /* Wait for game to fully load */
    Sleep(3000);

    /* v55j_8: Install Present hook for main-thread Gluebie proximity check.
     * This MUST be called after the game has loaded (Sleep(3000) ensures
     * the game's code section is mapped). Without this, gluebie_present_helper
     * never runs and Gluebie proximity check never fires on non-Dizzy levels. */
    install_present_hook();
    /* v55m_44p: Install Graphics_PresentOrEnd hook to draw the in-game text
     * system (score-HUD style). 0x46C200 clears the viewport, so text must be
     * drawn on 0x455A90 (after clear, before Present). */
    install_presentend_hook();
    /* v55m_43h: Restore RenderScene hook — sets g_in_draw_phase=1 during Draw.
     * The catapult vtable[18] hook needs this to skip D3D transforms during
     * Update (calling 0x45E0E0 during Update corrupts mesh data → 0x478EDD). */
    install_renderscene_hook();
    /* v55j_9: Install Ball_Render hook for tar splotch visual.
     * Runs AFTER Ball_Update (which clears ball+0x260), re-sets it if in zone. */
    install_ballrender_hook();

    while (g_running) {
        if (game_is_quitting()) break;  /* v55j_16: stop early when game is exiting */
        /* Per-frame: execute onUpdate scripts for tracked entities */
        {
            int i;
            for (i = 0; i < g_tracked_count; i++) {
                if (!g_tracked[i].active) continue;
                DWORD obj = g_tracked[i].obj;
                if (!obj || obj < 0x10000) { g_tracked[i].active = 0; continue; }
                if (IsBadReadPtr((void*)obj, 0x10F0)) { g_tracked[i].active = 0; continue; }
                exec_update_cmds(obj, &g_tracked[i].def, logf);
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

        /* v55j_9: Gluebie check now runs from Present hook (main thread).
         * Background thread was racing with physics engine — velocity scaling
         * was overwritten before Ball_Update could use it. Present hook runs
         * synchronously on main thread at end of each frame.
         * Sound still played here (background thread) to avoid reentrancy
         * issues with Sound_Play3D inside the Present hook. */

        /* v55n: Per-frame TarBubble DECORATIVE bubble driver (cross-level).
         * Spawns/updates/renders floating bubbles. No trap behavior —
         * Gluebie (slowdown) and Tarpit (sink) are separate entities. */
        {
            DWORD board = get_board();
            if (board && g_tarbubble_count > 0) {
                cEnt_tarbubble_update_decoration(board);
            }
        }

        /* v55f: Per-frame WaterWheel rotation update */
        {
            DWORD board = get_board();
            if (board && g_waterwheel_count > 0) {
                cEnt_waterwheel_update(board);
            }
        }

        /* v55m_3: Chomper state machine + rendering — MOVED to Present hook
         * (gluebie_present_helper) in v55m_7. Running D3D/Gfx functions from
         * the background thread crashes at 0x499D9D (D3DX matrix multiply
         * with NULL param — D3D device state not set up for wrong thread). */

        /* v55m_27i: Catapult trigger + launch force MOVED to Present hook
         * (cEnt_catapult_present_check / gluebie_present_helper). The old
         * background-thread code used the wrong AthenaList layout for the
         * ball list and wrong ball offsets, so detection never fired. */

        /* v55d: Play queued tar sound (deferred from proximity check).
         * Sound_Play3D: __thiscall ECX=[App+0x484], 4 stack params (x, y, z, scale=1.0)
         * App accessed via board+0x878 (same as native DizzyBoard_Update at 0x41D9B3).
         * The v55c crash was NOT from thread-safety — it was a missing 4th param
         * (Sound_Play3D has RET 0x10 = 4 params, old typedef only passed 3). */
        if (g_gluebie_sound_pending) {
            DWORD board = get_board();
            if (board && !IsBadReadPtr((void*)(board + 0x878), 4)) {
                DWORD app = *(DWORD*)(board + 0x878);
                if (app && app > 0x10000 && !IsBadReadPtr((void*)(app + APP_SOUNDFX_TAR), 4)) {
                    DWORD snd = *(DWORD*)(app + APP_SOUNDFX_TAR);
                    if (snd && snd > 0x10000 && !IsBadReadPtr((void*)snd, 0x20)) {
                        pfn_Sound_Play3D((void*)snd, g_gluebie_snd_x, g_gluebie_snd_y, g_gluebie_snd_z, 1.0f);
                    }
                }
            }
            g_gluebie_sound_pending = 0;  /* consumed */
        }

        /* Small sleep to avoid hogging CPU */
        Sleep(16);  /* ~60fps */

        DWORD board = get_board();
        if (!board) {
            /* Debug: log why get_board failed (limited output) */
            static int debug_count = 0;
            if (debug_count < 5) {
                debug_count++;
                FILE* df = NULL;
                fopen_s(&df, g_log_path, "a");
                if (df) {
                    DWORD scene = *(DWORD*)GLOBAL_SCENE_PTR;
                    DWORD app = *(DWORD*)GLOBAL_APP_PTR;
                    fprintf(df, "  DEBUG[%d]: get_board()=0 (g_Scene=0x%08X, app=0x%08X)\n", debug_count, scene, app);
                    if (app && app > 0x10000 && !IsBadReadPtr((void*)(app + 0x220), 4)) {
                        DWORD profile = *(DWORD*)(app + 0x220);
                        fprintf(df, "    profile=0x%08X\n", profile);
                    }
                    fclose(df);
                }
            }
            continue;
        }

        /* Check if board changed (new level loaded) */
        if (board == g_spawned_board) {
            /* v55m_44k: PER-FRAME re-neutralization. The game can re-register
             * or re-build CollisionLevel nodes during FinishLoad (post-init
             * 0x465860 recursively clones children; render sub-list traversal
             * at 0x4657fc calls vtable[18] on every node). 44j's one-shot
             * neutralization ran at spawn but the crash MOVED to
             * Background/FinishLoad — nodes built AFTER the patch still have
             * +0x430=0 and walk the broken component meshbuffers (0x7C, no
             * strip arrays) → AV 0x465789/0x46578C. Re-apply every frame for
             * every active waterwheel so any newly-registered node is
             * neutralized before the renderer reaches it. Cheap: each pass is
             * a bounded tree walk over ≤8 waterwheels. */
            /* v55m_44o: No neutralization — the waterwheel is a plain
             * MeshWorld now (no PopCylinder → no CollisionLevel at
             * +0x10E0). The old per-frame CollisionLevel re-neutralization
             * loop is gone with the crash path. */
            continue;
        }

        /* New level — reset bridgeslams */
        {
            int i;
            for (i = 0; i < g_bridgeslam_count; i++) {
                g_bridgeslams[i].active = 0;
            }
            g_bridgeslam_count = 0;
        }

        /* New board — wait for level to finish loading */
        Sleep(500);

        /* Re-read board in case it changed during sleep */
        board = get_board();
        if (!board) continue;

        /* Verify level is loaded */
        DWORD level = cEnt_get_level(board);
        if (!level) continue;

        logf = NULL;
        fopen_s(&logf, g_log_path, "a");
        if (logf) {
            fprintf(logf, "\n--- Level loaded (board=0x%08X, level=0x%08X) ---\n", board, level);
        }

        /* v55m_44m: new level → stale wheel-node addresses from the previous
         * board. Clear the recorded set; it refills on the next spawn. */
        g_wheel_node_count = 0;
        g_44l_present_logged = 0;

        /* v55: Skip ALL mod processing on Dizzy Race/Arena to test if mod causes double-Swirl.
         * If double-Swirl disappears, mod is the cause. If it persists, it's native. */
        {
            int skip_all = 0;
            DWORD app = *(DWORD*)(board + BOARD_APP);
            if (app && !IsBadReadPtr((void*)(app + 0x5FC), 4)) {
                int race_idx = *(int*)(app + 0x5FC);
                if (race_idx == 2 || race_idx == 13) {
                    skip_all = 1;
                }
            }
            if (!skip_all) {
                char* level_name = NULL;
                if (!IsBadReadPtr((void*)(board + 0x10), 4)) {
                    level_name = *(char**)(board + 0x10);
                }
                if (level_name && !IsBadReadPtr(level_name, 5)) {
                    if (my_stristr(level_name, "Dizzy") != NULL ||
                        my_stristr(level_name, "Arena") != NULL) {
                        skip_all = 1;
                    }
                }
            }
            if (skip_all) {
                if (logf) fprintf(logf, "  SKIP: All mod processing skipped on Dizzy/Arena\\n");
                g_spawned_board = board;
                if (logf) { fflush(logf); fclose(logf); }
                continue;
            }
        }

        /* Process <MESH> and <SPEEDMULT> tags on spawned 8-balls (after CreateBadBall has run) */
        cEnt_process_custom_tags(board, logf);

        /* Process C_entity entries — spawn custom objects at each position */
        process_rotaters(board, logf);

        /* Hide original meshbuffers for C_entity entries */
        hide_entity_meshbuffers(board, logf);

        /* Apply custom rotation directions to spawned rotaters */
        cEnt_apply_rotater_directions();

        /* Apply S1 rot tags to natively-spawned Rotators */
        cEnt_apply_s1_rotater_tags(board, logf);

        /* Find GRID reference points */
        float grid_x[32], grid_y[32], grid_z[32];
        int grid_count = cEnt_find_grid_points(board, grid_x, grid_y, grid_z, 32, logf);

        if (grid_count > 0) {
            g_spawned_board = board;
            if (logf) fprintf(logf, "  Found %d GRID points, starting cycle (speed=%.1fs)\n", grid_count, g_grid_speed);

            /* Start with GRID01 visible */
            int current_grid = 1;
            cEnt_spawn_testcube_at(board, grid_x[0], grid_y[0], grid_z[0], 1, logf);
            if (logf) {
                fprintf(logf, "  Cycle: GRID01 spawned\n");
                fflush(logf);
            }

            /* Cycling loop — keep log file open for entire cycle */
            while (g_running && board == get_board()) {
                /* Wait for grid_speed seconds (check every 100ms for board change) */
                int wait_ms = (int)(g_grid_speed * 1000);
                if (wait_ms < 100) wait_ms = 100;
                int waited = 0;
                while (waited < wait_ms) {
                    Sleep(100);
                    waited += 100;
                    if (!g_running || board != get_board()) break;
                }
                if (!g_running || board != get_board()) break;

                /* Advance to next GRID (1→2→3→4→5→1→...) */
                current_grid++;
                if (current_grid > grid_count) current_grid = 1;

                /* Despawn all spawned objects */
                while (g_spawned_count > 0) {
                    cEnt_despawn_object(board, g_spawned_objs[0], logf);
                    int j;
                    for (j = 0; j < g_spawned_count - 1; j++) {
                        g_spawned_objs[j] = g_spawned_objs[j + 1];
                        strcpy(g_spawned_names[j], g_spawned_names[j + 1]);
                    }
                    g_spawned_count--;
                }

                /* Spawn the current GRID */
                int idx = current_grid - 1;
                cEnt_spawn_testcube_at(board, grid_x[idx], grid_y[idx], grid_z[idx], current_grid, logf);
                if (logf) {
                    fprintf(logf, "  Cycle: GRID%02d spawned\n", current_grid);
                    fflush(logf);
                }
            }

            /* Clean up any remaining spawned objects on level exit */
            while (g_spawned_count > 0) {
                cEnt_despawn_object(board, g_spawned_objs[0], logf);
                int j;
                for (j = 0; j < g_spawned_count - 1; j++) {
                    g_spawned_objs[j] = g_spawned_objs[j + 1];
                    strcpy(g_spawned_names[j], g_spawned_names[j + 1]);
                }
                g_spawned_count--;
            }

            /* Despawn all rotater objects on level exit */
            cEnt_despawn_all_rotaters(board, logf);
        } else {
            /* No GRID points — still mark board as processed.
             * v53g-5 FIX: Do NOT call cEnt_despawn_all_rotaters() here!
             * It destroys all custom entities that were just spawned by
             * process_rotaters(). The despawn was intended for stale objects
             * from a PREVIOUS level, but g_spawned_board hasn't been set yet,
             * so it hits the CURRENT level's objects. Remove this call entirely. */
            g_spawned_board = board;
            if (logf) fprintf(logf, "  No GRID points found\n");
        }

        if (logf) {
            fflush(logf);
            fclose(logf);
        }
    }

    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DLL entry
 * ═══════════════════════════════════════════════════════════════════════════ */

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        load_real_bass();
        g_thread = CreateThread(NULL, 0, entity_thread, NULL, 0, NULL);
    } else if (reason == DLL_PROCESS_DETACH) {
        g_shutting_down = 1;  /* v55j_15: hooks check this before accessing game memory */
        g_running = 0;
        uninstall_present_hook();
        uninstall_presentend_hook();
        uninstall_ballrender_hook();
    }
    return TRUE;
}
