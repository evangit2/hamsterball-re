/*
 * custom_entities.c — Hamsterball Custom Entities Mod v55
 *
 * bass.dll proxy mod. Spawns testcube meshes at S1 GRID reference points.
 *
 * v13 REWRITE: Uses the proven CEA spawning pattern instead of direct
 * MeshBuffer injection. Loads testcube.MESHWORLD via the game's own
 * MeshWorld_ctor, then creates PopCylinder objects that reference it.
 * This is the same pattern used by XRow's "Press S to spawn red bridge"
 * CEA script — load mesh + create object + register in board lists.
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll custom_entities.c \
 *     -I../shared -lwinmm -Wl,--enable-stdcall-fixup -O2 -static \
 *     -static-libgcc -Wl,--add-stdcall-alias -msse2 -mfpmath=sse -lshlwapi
 */

#include "bass_proxy.h"
#include <shlwapi.h>
#include <math.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

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

/* v55j_9: Present hook for main-thread Gluebie proximity check.
 * Graphics_PresentOrEnd (0x455A90) runs at end of each frame on the main thread.
 * Original 7 bytes: 8A 44 24 04 83 EC 20 (MOV AL,[ESP+4]; SUB ESP,0x20)
 * Same pattern as magnet_mod — PUSHAD/CALL C fn/POPAD + original bytes + JMP back. */
#define PRESENT_HOOK_ADDR       0x00455A90
#define PRESENT_ORIG_BYTES      7

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
static StandsCtor_t pfn_Catapult_ctor     = (StandsCtor_t)0x00437E10;
static StandsCtor_t pfn_Mace_ctor         = (StandsCtor_t)0x00438750;
static StandsCtor_t pfn_Tipper_ctor       = (StandsCtor_t)0x00437960;
static StandsCtor_t pfn_Gluebie_ctor      = (StandsCtor_t)0x00437CB0;
static LifterCtor_t pfn_Lifter_ctor       = (LifterCtor_t)0x00436920;
static SpeedCylCtor_t pfn_SpeedCylinder_ctor = (SpeedCylCtor_t)0x00436A20;
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
    DWORD obj;       /* Catapult object pointer */
    DWORD board;     /* board pointer */
    float x, y, z;   /* position */
} CatapultState;
static CatapultState g_catapults[MAX_CATAPULTS];
static int g_catapult_count = 0;

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

/* Timer_Init / Timer_Cleanup — timer context for render transforms */
typedef void (__fastcall *Timer_Init_t)(void* out);
static Timer_Init_t pfn_Timer_Init = (Timer_Init_t)0x00457AD0;
typedef void (__fastcall *Timer_Cleanup_t)(void* out);
static Timer_Cleanup_t pfn_Timer_Cleanup = (Timer_Cleanup_t)0x00457A40;

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

/* v55e: TarBubble tracking — no entity spawned, just position markers.
 * Native game stores TarBubble S1 ref points in board+0x4790 AthenaList,
 * then DizzyBoard_Update creates a collision traversal object (0x44FA90)
 * that sinks the ball 0.25/frame when inside the tar radius.
 * We replicate this without spawning any entity. */
#define MAX_TARBUBBLES 64
typedef struct {
    float x, y, z;  /* position from S1 ref point */
} TarBubblePos;
static TarBubblePos g_tarbubble_pos[MAX_TARBUBBLES];
static int g_tarbubble_count = 0;

/* v55f: WaterWheel tracking — mesh object + position + rotation.
 * Native game stores waterwheel mesh at board+0x4BA8, position at board+0x4BB0,
 * angle at board+0x4BBC. DizzyBoard_Update rotates angle -= 0.5/frame and
 * applies transform via mesh vtable[22]+[21]. N:WHEELEMBED collision embeds ball.
 * We replicate by creating the mesh via MeshWorld_ctor and rotating each frame. */
#define MAX_WATERWHEELS 8
typedef struct {
    DWORD mesh_obj;    /* Level/MeshWorld object (0x10D0 bytes) */
    float x, y, z;     /* position from S1 ref point */
    float angle;       /* current rotation angle (degrees, decremented 0.5/frame) */
    int active;
} WaterWheelState;
static WaterWheelState g_waterwheels[MAX_WATERWHEELS];
static int g_waterwheel_count = 0;

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
#define NEONPLATFORM_SIZE     0x1104
#define TRAPDOOR_SIZE         0x10F8
#define ODD_LIFTER_SIZE       0x10F4

/* Level3-Swirl mesh path (game .data at 0x004CFFE0) */
static const char* g_swirl_mesh_path = (const char*)0x004CFFE0;

/* Gfx_Scale function pointers — used by ROT_M axis selection.
 * Native render (0x0043B330) calls Gfx_ScaleX(angle) to build the
 * rotation-to-render matrix. ROT_M lets the .txt config choose which
 * axis function to call instead. */
typedef void (__cdecl *Gfx_Scale_t)(float);
static Gfx_Scale_t pfn_Gfx_ScaleX = (Gfx_Scale_t)0x00457C60;
static Gfx_Scale_t pfn_Gfx_ScaleY = (Gfx_Scale_t)0x00457C90;
static Gfx_Scale_t pfn_Gfx_ScaleZ = (Gfx_Scale_t)0x00457CC0;

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

    /* Check if we have any tracked Bonk objects */
    if (g_bonk_count <= 0) return;

    /* Get event name from collision_data[1]+0x864 */
    if (!collision_data || IsBadReadPtr((void*)collision_data, 8)) return;
    int meshbuf = collision_data[1];
    if (!meshbuf || IsBadReadPtr((void*)meshbuf, 0x868)) return;
    char* event_name = *(char**)(meshbuf + 0x864);
    if (!event_name || IsBadReadPtr(event_name, 1)) return;

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

    /* v55i: TarBubble (ai_type 25) — spawn PopCylinder with tarbubble mesh
     * for visibility, AND store position for proximity tar sinking. */
    if (ai_type == 25) {
        if (g_tarbubble_count < MAX_TARBUBBLES) {
            g_tarbubble_pos[g_tarbubble_count].x = px;
            g_tarbubble_pos[g_tarbubble_count].y = py;
            g_tarbubble_pos[g_tarbubble_count].z = pz;
            g_tarbubble_count++;
            if (logf) fprintf(logf, "  TARBUBBLE: stored position (%.1f,%.1f,%.1f) [%d]\n",
                    px, py, pz, g_tarbubble_count - 1);
        }
        /* Fall through to spawn PopCylinder with tarbubble mesh.
         * The mesh path is already set to meshes\tarbubble. */
        /* Don't return — let the normal spawn path handle it. */
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
            WaterWheelState* ww = &g_waterwheels[g_waterwheel_count];
            const char* ww_path = mesh_path && mesh_path[0] ? mesh_path : "levels\\Level3-WaterWheel";

            /* Create mesh object via MeshWorld_ctor */
            DWORD mesh = (DWORD)pfn_operator_new(MESHWORLD_SIZE);
            if (!mesh) {
                if (logf) fprintf(logf, "  WATERWHEEL: failed to alloc mesh\n");
                return;
            }
            memset((void*)mesh, 0, MESHWORLD_SIZE);
            void* result = pfn_MeshWorld_ctor((void*)mesh, (void*)gfx_device, ww_path);
            if (!result) {
                if (logf) fprintf(logf, "  WATERWHEEL: MeshWorld_ctor failed for '%s'\n", ww_path);
                return;
            }

            ww->mesh_obj = mesh;
            ww->x = px;
            ww->y = py;
            ww->z = pz;
            ww->angle = 0.0f;
            ww->active = 1;
            g_waterwheel_count++;

            /* Spawn PopCylinder with the waterwheel mesh for proper rendering */
            void* pc_obj = pfn_operator_new(POPCYLINDER_SIZE);
            if (pc_obj) {
                memset(pc_obj, 0, POPCYLINDER_SIZE);
                pfn_PopCylinder_ctor(pc_obj, (void*)board, px, py, pz, (void*)mesh);
                /* Add to board lists */
                pfn_AthenaList_Append((DWORD*)(board + BOARD_UPDATE_LIST), pc_obj);
                pfn_AthenaList_Append((DWORD*)(board + BOARD_RENDER_LIST), pc_obj);
                /* Add to scene spatial tree */
                DWORD level = cEnt_get_level(board);
                if (level) {
                    DWORD sceneobj = *(DWORD*)(level + LEVEL_SCENEOBJECT);
                    if (sceneobj) {
                        pfn_AthenaList_Append((DWORD*)(sceneobj + 0x1C), pc_obj);
                    }
                }
                /* Track for despawn */
                if (g_rotater_count < MAX_ROTATERS) {
                    g_rotater_cfg[g_rotater_count].obj = (DWORD)pc_obj;
                    g_rotater_cfg[g_rotater_count].rot_y = 0.0f;
                    g_rotater_count++;
                }
            }

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
        ai_type == 16 || ai_type == 28 ||
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
    } else if ((ai_type >= 7 && ai_type <= 14) || (ai_type >= 17 && ai_type <= 22) || (ai_type >= 27 && ai_type <= 43)) {
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
            case 22: /* Chomper_ctor — Tower Race Chomper (MeshNode_ctor, 0x18 bytes)
                      * Loads "Meshes\\Chomper" as a small mesh node. */
                {
                    DWORD app = *(DWORD*)(board + BOARD_APP);
                    DWORD gfx_device = app ? *(DWORD*)(app + APP_GFX_DEVICE) : 0;
                    if (!gfx_device) { if (logf) fprintf(logf, "  ROTATER: no gfx_device for Chomper\n"); return; }
                    obj = pfn_operator_new(MESHNODE_SIZE);
                    if (!obj) { if (logf) fprintf(logf, "  ROTATER: failed to alloc Chomper\n"); return; }
                    memset(obj, 0, MESHNODE_SIZE);
                    pfn_MeshNode_ctor(obj, (void*)gfx_device, "Meshes\\Chomper");
                }
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
                /* Track Bonk for collision event hook (E:CALLHAMMER/E:HAMMERCHASE) */
                if (g_bonk_count < MAX_BONKS) {
                    g_bonk_objs[g_bonk_count] = (DWORD)obj;
                    g_bonk_count++;
                }
                /* Collision dispatch hook disabled in v53g-2 — trampoline caused
                 * stack corruption. Will re-add in future version with proper
                 * detour library (MinHook/etc). */
                /* install_bonk_collision_hook(); */
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
                /* Create collision/render Level via Level_RenderCtor, same as TipperVisual.
                 * Stands_ctor creates the behavior object but the collision/render Level
                 * must be created separately and stored at obj+0x10D4.
                 * This is the object that gets added to the collision list (board+0x10EC). */
                if (mesh && !IsBadReadPtr(mesh, 0x100)) {
                    void* render_level = pfn_operator_new(LEVEL_SIZE);
                    if (render_level) {
                        memset(render_level, 0, LEVEL_SIZE);
                        render_level = pfn_Level_RenderCtor(render_level, mesh);
                        if (render_level) {
                            /* Store collision/render Level at obj+0x10D4 */
                            *(DWORD*)((char*)obj + 0x10D4) = (DWORD)render_level;
                            if (logf) fprintf(logf, "  ROTATER: Catapult collision Level created at 0x%08X\n", (DWORD)render_level);
                        } else {
                            if (logf) fprintf(logf, "  ROTATER: Level_RenderCtor failed for Catapult\n");
                        }
                    }
                } else {
                    if (logf) fprintf(logf, "  ROTATER: Catapult mesh invalid, no collision Level created\n");
                }
                /* Track for per-frame Catapult_vtable11 calls */
                if (g_catapult_count < MAX_CATAPULTS) {
                    g_catapults[g_catapult_count].obj = (DWORD)obj;
                    g_catapults[g_catapult_count].board = (DWORD)board;
                    g_catapults[g_catapult_count].x = px;
                    g_catapults[g_catapult_count].y = py;
                    g_catapults[g_catapult_count].z = pz;
                    g_catapult_count++;
                    if (logf) fprintf(logf, "  ROTATER: Catapult tracked in g_catapults[%d] (count=%d)\n",
                        g_catapult_count - 1, g_catapult_count);
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
            case 39: /* SpeedCylinder_ctor — 7 params (this, board, x, y, z, int_param, mesh) */
                obj = pfn_operator_new(SPEEDCYLINDER_SIZE);
                if (!obj) { if (logf) fprintf(logf, "  ROTATER: failed to alloc SpeedCylinder\n"); return; }
                memset(obj, 0, SPEEDCYLINDER_SIZE);
                pfn_SpeedCylinder_ctor(obj, (void*)board, px, py, pz, 0, mesh);
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
                     * Gluebie_ctor calls Stands_ctor (clones spatial trees from mesh).
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
                /* Add to board+0x4378 (Gluebie list) for DizzyBoard_Update proximity check.
                 * Only safe on Dizzy (where it's an AthenaList). On other levels,
                 * board+0x4378 may be a Level pointer (Tower/Expert/Toob) — skip to avoid crash.
                 * Check: Dizzy board name is "Board (Dizzy)" at board+0x868. */
                {
                    char* board_name = *(char**)((char*)board + 0x868);
                    if (board_name && !IsBadReadPtr(board_name, 12) &&
                        _strnicmp(board_name, "Board (Dizzy)", 13) == 0) {
                        pfn_AthenaList_Append((DWORD*)(board + 0x4378), obj);
                        if (logf) fprintf(logf, "  ROTATER: Gluebie added to Dizzy board+0x4378 (proximity list)\n");
                    } else {
                        if (logf) fprintf(logf, "  ROTATER: Gluebie on non-Dizzy level, proximity behavior not available\n");
                    }
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
    g_tarbubble_count = 0;  /* v55e: reset TarBubble tracking on level unload */
    g_waterwheel_count = 0;  /* v55f: reset WaterWheel tracking on level unload */
    g_catapult_count = 0;  /* v55d: reset Catapult tracking on level unload */

    /* Clear Bonk tracking and uninstall collision hook */
    g_bonk_count = 0;
    uninstall_bonk_collision_hook();
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
        /* Rewrite direction field to prevent native oscillation reversal */
        *(float*)(obj + 0x10EC) = g_rotater_cfg[i].rot_y;
        /* Clamp angle to prevent the native "if angle > 2.0" / "if angle < -2.0" checks
         * from triggering. By keeping angle within [-1.99, 1.99], the native code
         * never reverses direction, giving constant rotation. */
        float angle = *(float*)(obj + 0x10E8);
        if (angle > 1.99f) *(float*)(obj + 0x10E8) = -1.99f;
        else if (angle < -1.99f) *(float*)(obj + 0x10E8) = 1.99f;
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
            Gfx_Scale_t scale_fn = pfn_Gfx_ScaleX;  /* default: X */
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

/* C helper called from the Present hook cave (main thread, safe for C calls). */
static void __cdecl gluebie_present_helper(void) {
    DWORD board = get_board();
    if (board && g_gluebie_count > 0) {
        cEnt_gluebie_proximity_check(board);
    }
}

/* Function pointer for the cave to call (indirection needed since the
 * C helper address isn't known at assembly time). */
static void (__cdecl *g_gluebie_fn_ptr)(void) = NULL;

static void install_present_hook(void) {
    if (g_present_hook_installed) return;
    BYTE *hook_addr = (BYTE*)PRESENT_HOOK_ADDR;
    BYTE expected[] = { 0x8A, 0x44, 0x24, 0x04, 0x83, 0xEC, 0x20 };
    if (memcmp(hook_addr, expected, 7) != 0) return;

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
    /* Original 7 bytes */
    g_present_cave[p++] = 0x8A; g_present_cave[p++] = 0x44;
    g_present_cave[p++] = 0x24; g_present_cave[p++] = 0x04;
    g_present_cave[p++] = 0x83; g_present_cave[p++] = 0xEC;
    g_present_cave[p++] = 0x20;
    /* JMP back to hook_addr + 7 */
    g_present_cave[p++] = 0xE9;
    *(DWORD*)(g_present_cave + p) = (DWORD)(hook_addr + PRESENT_ORIG_BYTES)
                                     - (DWORD)(g_present_cave + p + 4);
    p += 4;

    DWORD old_protect;
    VirtualProtect(hook_addr, PRESENT_ORIG_BYTES, PAGE_EXECUTE_READWRITE, &old_protect);
    DWORD jmp_offset = (DWORD)(g_present_cave - hook_addr - 5);
    hook_addr[0] = 0xE9;
    *(DWORD*)(hook_addr + 1) = jmp_offset;
    hook_addr[5] = 0x90;
    hook_addr[6] = 0x90;
    VirtualProtect(hook_addr, PRESENT_ORIG_BYTES, old_protect, &old_protect);
    FlushInstructionCache(GetCurrentProcess(), hook_addr, PRESENT_ORIG_BYTES);
    g_present_hook_installed = 1;
}

static void uninstall_present_hook(void) {
    if (!g_present_hook_installed) return;
    BYTE *hook_addr = (BYTE*)PRESENT_HOOK_ADDR;
    DWORD old_protect;
    if (VirtualProtect(hook_addr, PRESENT_ORIG_BYTES, PAGE_EXECUTE_READWRITE, &old_protect)) {
        hook_addr[0] = 0x8A; hook_addr[1] = 0x44;
        hook_addr[2] = 0x24; hook_addr[3] = 0x04;
        hook_addr[4] = 0x83; hook_addr[5] = 0xEC;
        hook_addr[6] = 0x20;
        VirtualProtect(hook_addr, PRESENT_ORIG_BYTES, old_protect, &old_protect);
        FlushInstructionCache(GetCurrentProcess(), hook_addr, PRESENT_ORIG_BYTES);
    }
    g_present_hook_installed = 0;
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

        /* Use visual position (obj+0x10D4/10D8/10DC) — always valid */
        float gx = *(float*)(gluebie + 0x10D4);
        float gy = *(float*)(gluebie + 0x10D8);
        float gz = *(float*)(gluebie + 0x10DC);

        /* Read detection radius — use native outer zone for slowdown.
         * Native has TWO proximity checks:
         *   1. DizzyBoard_Update: obj+0x1100 * 60.0 = 45-60 units (center-to-center, velocity slowdown)
         *   2. Ball_Update: 3.0 units (distance to tar SURFACE, sets tar flag)
         * We use the outer zone (obj+0x1100 * 60.0) for velocity slowdown,
         * and the inner zone (3.0) for tar splotch + tar flag. */
        /* v55j_9: Use hardcoded outer radius 60.0 (native Gluebie default).
         * obj+0x1100 reads 0.0 on cross-level spawn (ctor doesn't init it),
         * which triggers the fallback. Use 60.0 directly for consistent
         * detection radius matching native Dizzy behavior. */
        float outer_radius = 60.0f;
        float inner_radius = 3.0f;  /* tar surface contact */

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
                FILE* df = fopen("custom_entities_debug.log", "a");
                if (df) {
                    float dist = 0.0f;
                    if (dist_sq > 0.0f) dist = sqrtf(dist_sq);
                    fprintf(df, "GLUEBIE f=%d: gpos=(%.1f,%.1f,%.1f) outer=%.1f inner=%.1f ball[%d]=%p bpos=(%.1f,%.1f,%.1f) dist=%.1f in_outer=%d in_inner=%d\n",
                        g_gluebie_debug_frame, gx, gy, gz, outer_radius, inner_radius, i, (void*)ball, bx, by, bz, dist, in_outer, in_inner);
                    DWORD col_mesh = *(DWORD*)(ball + 0x1A4);
                    fprintf(df, "  col_mesh=%p vel=(%.3f,%.3f,%.3f)\n",
                        (void*)col_mesh,
                        col_mesh ? *(float*)(col_mesh + 0xCA4) : 0.0f,
                        col_mesh ? *(float*)(col_mesh + 0xCA8) : 0.0f,
                        col_mesh ? *(float*)(col_mesh + 0xCAC) : 0.0f);
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

                /* Play tar sound — 1 sec cooldown. */
                {
                    static int g_gluebie_sound_cooldown = 0;
                    if (g_gluebie_sound_cooldown > 0) g_gluebie_sound_cooldown--;
                    if (g_gluebie_sound_cooldown == 0) {
                        g_gluebie_sound_pending = 1;
                        g_gluebie_snd_x = bx;
                        g_gluebie_snd_y = by;
                        g_gluebie_snd_z = bz;
                        g_gluebie_sound_cooldown = 60;
                    }
                }

                /* v55j_8: Set tar render flag (ball+0x260).
                 * Native DizzyBoard_Update sets ball+0x260 which makes Ball_Render
                 * draw the tar splotch mesh (App+0x264) on the ball automatically.
                 * Only set if App+0x264 (tar mesh) is loaded — on non-Dizzy levels
                 * it may be NULL, which would cause Ball_Render to skip drawing. */
                {
                    DWORD app = *(DWORD*)(board + BOARD_APP);
                    if (app && app > 0x10000 &&
                        !IsBadReadPtr((void*)(app + 0x264), 4)) {
                        DWORD tar_mesh = *(DWORD*)(app + 0x264);
                        if (tar_mesh && tar_mesh > 0x10000) {
                            *(BYTE*)(ball + 0x260) = 1;
                        }
                    }
                }
            } else {
                /* Ball NOT in range — clear tar render flag */
                *(BYTE*)(ball + 0x260) = 0;
            }
        }
    }
}

/* v55e: TarBubble proximity check — replicates native DizzyBoard_Update tar behavior.
 * Native flow: TarBubble S1 ref points stored in board+0x4790 AthenaList.
 * DizzyBoard_Update (0x41D512) creates a collision traversal object (0x44FA90)
 * that iterates balls and sinks any inside the tar radius.
 *
 * Sinking: ball+0x168 (Y position) -= 0.25/frame (constant at 0x4CF380).
 * When ball+0x2CC (in_tar) is set, Ball_Update uses ball's own position for
 * distance calc (can't be pushed), spin decays 0.85x/frame.
 * Ball dies when sunk past radius * 2.5 (depth check in Board_Setup).
 *
 * We replicate this without any spawned entity — just position markers. */
static void cEnt_tarbubble_proximity_check(DWORD board) {
    if (!board || g_tarbubble_count <= 0) return;

    /* Get ball AthenaList at board+0x29D4 */
    DWORD ball_list = board + 0x29D4;
    if (IsBadReadPtr((void*)(ball_list + 0x04), 4)) return;
    int ball_count = *(int*)(ball_list + 0x04);
    if (ball_count <= 0 || ball_count > 20) return;
    if (IsBadReadPtr((void*)(ball_list + 0x40C), 4)) return;
    DWORD* ball_data = *(DWORD**)(ball_list + 0x40C);
    if (!ball_data || IsBadReadPtr(ball_data, ball_count * 4)) return;

    int i, j;
    for (j = 0; j < g_tarbubble_count; j++) {
        float tx = g_tarbubble_pos[j].x;
        float ty = g_tarbubble_pos[j].y;
        float tz = g_tarbubble_pos[j].z;

        for (i = 0; i < ball_count; i++) {
            DWORD ball = ball_data[i];
            if (!ball || ball < 0x10000) continue;
            if (IsBadReadPtr((void*)ball, 0x2D0)) continue;

            /* Skip ball if already in tar (native checks ball+0x2CC) */
            if (*(char*)(ball + 0x2CC) != 0) continue;

            /* Ball position */
            float bx = *(float*)(ball + 0x164);
            float by = *(float*)(ball + 0x168);
            float bz = *(float*)(ball + 0x16C);

            /* 3D distance to tarbubble center */
            float dx = tx - bx;
            float dy = ty - by;
            float dz = tz - bz;
            float dist_sq = dx*dx + dy*dy + dz*dz;

            /* Native tar radius: ball sinks when close to tarbubble center.
             * The native collision traversal object uses the mesh bounds,
             * but for a position-only marker we use a fixed radius.
             * Native Ball_Update uses 3.0 units for tar surface contact. */
            float radius = 3.0f;
            if (dist_sq < radius * radius) {
                /* Sink ball: ball+0x168 (Y) -= 0.25/frame (native constant 0x4CF380) */
                *(float*)(ball + 0x168) = by - 0.25f;

                /* Set in_tar flag — disables ball control, decays spin 0.85x */
                *(BYTE*)(ball + 0x2CC) = 1;

                /* Mark ball tar flag for render (draws tar splotch mesh) */
                *(BYTE*)(ball + 0x2BC) = 1;
            }
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

    int i;
    for (i = 0; i < g_waterwheel_count; i++) {
        WaterWheelState* ww = &g_waterwheels[i];
        if (!ww->active || !ww->mesh_obj) continue;
        if (IsBadReadPtr((void*)ww->mesh_obj, 0x440)) continue;

        /* Decrement angle by 0.5 degrees per frame (native constant at 0x4CF3F0) */
        ww->angle -= 0.5f;

        /* Build rotation matrix on stack (4x4 = 64 bytes, but Gfx_RotateY writes to buf+4) */
        float rot_matrix[16];   /* 4x4 float matrix */
        memset(rot_matrix, 0, sizeof(rot_matrix));

        /* Gfx_RotateY(matrix_buf, angle_degrees) — writes rotation to buf+0x04 */
        pfn_Gfx_RotateY(rot_matrix, ww->angle);

        /* Get mesh vtable */
        DWORD vtable = *(DWORD*)ww->mesh_obj;
        if (IsBadReadPtr((void*)vtable, 0x60)) continue;

        /* vtable[22] (offset 0x58) = SetTransform — no params, __thiscall 0 params
         * This calls mesh->sceneobj->vtable[1] (update vertex buffer from rotation) */
        typedef void (__thiscall *MeshSetTransform_t)(DWORD this_);
        MeshSetTransform_t pfn_setTransform = *(MeshSetTransform_t*)(vtable + 0x58);
        if (pfn_setTransform) {
            pfn_setTransform(ww->mesh_obj);
        }

        /* vtable[21] (offset 0x54) = SetPosition — 1 param (&matrix), __thiscall RET 4
         * This calls mesh->sceneobj->vtable[3] with &rotation_matrix */
        typedef void (__thiscall *MeshSetPosition_t)(DWORD this_, float* matrix);
        MeshSetPosition_t pfn_setPosition = *(MeshSetPosition_t*)(vtable + 0x54);
        if (pfn_setPosition) {
            pfn_setPosition(ww->mesh_obj, rot_matrix);
        }
    }
}

/* Tracked entity for per-frame updates */
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
            { "Chomper",          22, "meshes\\chomper" },           /* Chomper_ctor (MeshNode_ctor, 0x471C20, 0x18 bytes) */
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
            { "Tarbubble",        25, "meshes\\tarbubble" },         /* v55i: PopCylinder + tar proximity */
            { "Tarpit",           0, "levels\\\\_default" },          /* N:TARPIT tag — no _ctor, _default mesh */
            { "Timebutton",       0, "levels\\LevelUp-Button" },    /* TimeButton_ctor */
            { "Tipper",            37, "levels\\Level3-Tipper" },     /* Tipper_ctor (0x437960, 0x1104) */
            { "Trapdoor",         41, "levels\\Level4-Trapdoor1" },  /* Trapdoor_ctor (0x438290, 0x10F8) */
            { "Trode",            21, "levels\\LevelDark-Trode" },   /* cEnt_Trode_ctor (ArenaStands_ctor) */
            { "Waterwheel",       26, "levels\\Level3-WaterWheel" }, /* v55f: mesh loaded + rotated by mod, no entity spawned */
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

/* Load config from custom_entities.txt */
static void load_config(void) {
    char config_path[MAX_PATH];
    snprintf(config_path, MAX_PATH, "%s\\custom_entities.txt", g_game_dir);
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
    char log_path[MAX_PATH];
    snprintf(log_path, MAX_PATH, "%s\\custom_entities.log", g_game_dir);
    FILE* logf = NULL;
    fopen_s(&logf, log_path, "a");
    if (logf) {
        fprintf(logf, "=== Custom Entities Mod v55 Started ===\n");
        fprintf(logf, "Game dir: %s\n", g_game_dir);
        fprintf(logf, "Mesh path: %s\n", g_mesh_path);
        fprintf(logf, "Grid speed: %.1f seconds\n", g_grid_speed);
        fclose(logf);
    }

    /* Wait for game to fully load */
    Sleep(3000);

    while (g_running) {
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

        /* v55e: Per-frame TarBubble proximity check (cross-level behavior) */
        {
            DWORD board = get_board();
            if (board && g_tarbubble_count > 0) {
                cEnt_tarbubble_proximity_check(board);
            }
        }

        /* v55f: Per-frame WaterWheel rotation update */
        {
            DWORD board = get_board();
            if (board && g_waterwheel_count > 0) {
                cEnt_waterwheel_update(board);
            }
        }

        /* v55d: Per-frame Catapult state machine update.
         * Catapult_vtable11 (0x437F10) is the per-frame state machine
         * (wind-up + release). Native game calls this via Scene_Update
         * (board vtable[1] iterating board+0x8B8), but Catapult is NOT
         * in that list. We call it manually here for each tracked Catapult. */
        {
            int i;
            for (i = 0; i < g_catapult_count; i++) {
                DWORD obj = g_catapults[i].obj;
                if (!obj || obj < 0x10000) {
                    /* Stale entry — clear it */
                    g_catapults[i].obj = 0;
                    continue;
                }
                if (IsBadReadPtr((void*)obj, 0x1108)) {
                    g_catapults[i].obj = 0;
                    continue;
                }
                /* Call Catapult's vtable[11] (state machine) directly */
                pfn_Catapult_vtable11((void*)obj);
            }
        }

        /* v55d: Per-frame Catapult trigger check.
         * E:CATAPULTBOTTOM collision event fires when ball touches the
         * catapult's bottom plate. Native game only checks this on Tower
         * Race (race 4). We use per-frame proximity check instead of
         * hooking DispatchCollisionEvents (SEH trampoline crash risk).
         *
         * Catapult_Launch (0x434290) sets launching flag + 50-tick countdown.
         * Called when ball is within trigger radius of catapult base.
         * The bottom plate is below the catapult pivot — check at Y offset -10. */
        {
            DWORD board = get_board();
            if (board && g_catapult_count > 0) {
                /* Get ball position from board+0x361C (ball pointer) */
                DWORD ball_ptr = *(DWORD*)(board + 0x361C);
                if (ball_ptr && ball_ptr > 0x10000 &&
                    !IsBadReadPtr((void*)ball_ptr, 0x200)) {
                    float ball_x = *(float*)(ball_ptr + 0x164);
                    float ball_y = *(float*)(ball_ptr + 0x168);
                    float ball_z = *(float*)(ball_ptr + 0x16C);
                    int i;
                    for (i = 0; i < g_catapult_count; i++) {
                        DWORD obj = g_catapults[i].obj;
                        if (!obj || obj < 0x10000) continue;
                        if (IsBadReadPtr((void*)obj, 0x1108)) continue;
                        /* Check if ball is near catapult base (bottom plate area).
                         * Catapult base is at obj+0x10D8/0x10DC/0x10E0.
                         * Bottom plate is ~10 units below pivot. */
                        float cx = *(float*)((char*)obj + 0x10D8);
                        float cy = *(float*)((char*)obj + 0x10DC);
                        float cz = *(float*)((char*)obj + 0x10E0);
                        float dx = ball_x - cx;
                        float dy = ball_y - (cy - 10.0f);
                        float dz = ball_z - cz;
                        float dist_sq = dx*dx + dy*dy + dz*dz;
                        /* Trigger radius ~40 units (covers the bottom plate area).
                         * Only trigger if not already launched (check launching flag
                         * at obj+0x10F0 — set to 1 by Catapult_Launch). */
                        DWORD launching = *(DWORD*)((char*)obj + 0x10F0);
                        if (dist_sq < 1600.0f && !launching) {
                            /* Ball is on the bottom plate — launch the catapult! */
                            pfn_Catapult_Launch((void*)obj);
                        }
                    }
                }
            }
        }

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
                fopen_s(&df, log_path, "a");
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
        if (board == g_spawned_board) continue;

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
        fopen_s(&logf, log_path, "a");
        if (logf) {
            fprintf(logf, "\n--- Level loaded (board=0x%08X, level=0x%08X) ---\n", board, level);
        }

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
        g_running = 0;
    }
    return TRUE;
}
