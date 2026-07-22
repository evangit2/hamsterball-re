/*
 * custom_entities.c — Hamsterball Custom Entities Mod v53d
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
 *   3  = Looper_ctor (0x437460, size 0x1500) — Z-axis rotation
 *   4  = Gear_ctor (0x437690, size 0x1514) — multi-axis
 *   5  = BigGear_ctor (same as 4)
 *   6  = Swirl (Rotator_ctor, constant rotation)
 *   7  = DFloor1_ctor (ArenaStands_ctor, 0x43E450, 0x1104) — Neon DFLOOR1
 *   17 = DFloor2_ctor (ArenaStands_ctor) — Neon DFLOOR2
 *   18 = DFloor3_ctor (ArenaStands_ctor) — Neon DFLOOR3
 *   19 = DFloor4_ctor (ArenaStands_ctor + post-config) — Neon DFLOOR4 (obj+0x10DC=2, obj+0x10E0=0)
 *   20 = FlickRing_ctor (ArenaStands_ctor) — Neon Arena FLICKRING
 *   21 = Trode_ctor (ArenaStands_ctor) — Neon TRODE
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
 *   25 = Tarbubble_ctor (no _ctor, board-level behavior, PopCylinder fallback)
 *   26 = Waterwheel_ctor (no _ctor, position-only storage, PopCylinder fallback)
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
static Rotator_ctor_t pfn_Looper_ctor = (Rotator_ctor_t)0x437460;
static Rotator_ctor_t pfn_Gear_ctor = (Rotator_ctor_t)0x437690;

/* ArenaStands_ctor — Neon Race DFLOOR, FLICKRING, TRODE (all use the same _ctor) */
typedef void* (__thiscall *ArenaStands_ctor_t)(void* this_, void* board, float x, float y, float z, void* mesh);
static ArenaStands_ctor_t pfn_ArenaStands_ctor = (ArenaStands_ctor_t)0x0043E450;

/* Named _ctor wrappers — all call ArenaStands_ctor internally.
 * DFloor4 has extra post-construction config from Neon_CreateDynamicObjects. */
static void* DFloor1_ctor(void* obj, void* board, float x, float y, float z, void* mesh) {
    return pfn_ArenaStands_ctor(obj, board, x, y, z, mesh);
}
static void* DFloor2_ctor(void* obj, void* board, float x, float y, float z, void* mesh) {
    return pfn_ArenaStands_ctor(obj, board, x, y, z, mesh);
}
static void* DFloor3_ctor(void* obj, void* board, float x, float y, float z, void* mesh) {
    return pfn_ArenaStands_ctor(obj, board, x, y, z, mesh);
}
static void* DFloor4_ctor(void* obj, void* board, float x, float y, float z, void* mesh) {
    void* result = pfn_ArenaStands_ctor(obj, board, x, y, z, mesh);
    /* Post-construction config from Neon_CreateDynamicObjects (DFLOOR4 case):
     *   obj+0x10DC = 2  (sets collision flag)
     *   obj+0x10E0 = 0  (clears collision object) */
    *(DWORD*)((char*)obj + 0x10DC) = 2;
    *(DWORD*)((char*)obj + 0x10E0) = 0;
    return result;
}
static void* FlickRing_ctor(void* obj, void* board, float x, float y, float z, void* mesh) {
    return pfn_ArenaStands_ctor(obj, board, x, y, z, mesh);
}
static void* Trode_ctor(void* obj, void* board, float x, float y, float z, void* mesh) {
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

/* TipperVisual_Attach — links visual object to behavior object */
typedef void (__cdecl *TipperVisual_Attach_t)(void* visual, int behavior);
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

/* Sound_Play3D — plays a 3D positioned sound */
typedef void (__thiscall *Sound_Play3D_t)(void* soundChannel, float x, float y, float z);
static Sound_Play3D_t pfn_Sound_Play3D = (Sound_Play3D_t)0x00459860;

/* Gfx_ScaleZ — apply Z-axis rotation to gfx device (thiscall with gfx ptr) */
typedef void (__thiscall *Gfx_ScaleFn_t)(void* gfx, float val);
static Gfx_ScaleFn_t pfn_Gfx_ScaleZ_Bridge = (Gfx_ScaleFn_t)0x00457CC0;

/* Gfx_SetPosition — set gfx device position */
typedef void (__thiscall *Gfx_SetPosition_t)(void* gfx, float x, float y, float z);
static Gfx_SetPosition_t pfn_Gfx_SetPosition_Bridge = (Gfx_SetPosition_t)0x00457B50;

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

/* Per-frame update for a single bridgeslam object */
static void bridgeslam_update(BridgeslamState* bs) {
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
            /* Play bridgeslam sound */
            if (app && pfn_Sound_Play3D) {
                DWORD snd = *(DWORD*)(app + APP_SOUNDFX_BRIDGESLAM);
                if (snd) {
                    pfn_Sound_Play3D((void*)snd, bs->pivot_x, bs->pivot_y, bs->pivot_z);
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

static DWORD get_level(DWORD board) {
    if (!board) return 0;
    if (IsBadReadPtr((void*)(board + BOARD_LEVEL), 4)) return 0;
    return *(DWORD*)(board + BOARD_LEVEL);
}

/* Get the SceneObject from the level */
static DWORD get_sceneobj(DWORD board) {
    DWORD level = get_level(board);
    if (!level) return 0;
    if (IsBadReadPtr((void*)(level + LEVEL_SCENEOBJECT), 4)) return 0;
    return *(DWORD*)(level + LEVEL_SCENEOBJECT);
}

/* Find S1 reference points by scanning the sceneobj's S1 list */
static int find_grid_points(DWORD board, float* out_x, float* out_y, float* out_z, int max_points, FILE* logf) {
    DWORD sceneobj = get_sceneobj(board);
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
static int ci_strstr(const char* haystack, const char* needle) {
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
static int extract_dat_prop(const char* name, const char* key, char* out_buf, int out_size) {
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
static int extract_tag(const char* name, const char* tag_name, char* out_buf, int out_size) {
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
static void process_custom_tags(DWORD board, FILE* logf) {
    if (!board) return;

    if (logf) {
        fprintf(logf, "  TAGS: process_custom_tags called (board=0x%08X)\n", board);
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
    DWORD sceneobj = get_sceneobj(board);
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
        /* int has_mesh = extract_tag(name, "MESH", mesh_value, sizeof(mesh_value)); */
        int has_mesh = 0;  /* MESH tag inactive */

        /* Check for <SPEEDMULT> tag */
        char speed_value[64] = {0};
        int has_speed = extract_tag(name, "SPEEDMULT", speed_value, sizeof(speed_value));

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
                     * initialized yet when process_custom_tags runs.
                     * Ball_Render reads ball+0x10 as App, but it's set by
                     * the game's per-frame loop, not during CreateBadBalls. */
                    DWORD app = *(DWORD*)GLOBAL_APP_PTR;
                    if (app && !IsBadReadPtr((void*)app, 0x280)) {
                        int target_slot = MESH_SLOT_8BALL;  /* default */
                        DWORD src_mesh = 0;

                        if (ci_strstr(mesh_value, "funball")) {
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
                                    ci_strstr(mesh_value, "funball") ? APP_MESH_FUNBALL : APP_MESH_8BALL);
                            mesh_changes++;
                        } else if (logf) {
                            fprintf(logf, "  TAGS: ball 0x%08X → mesh src invalid (App+0x%X=0x%08X)\n",
                                    ball, ci_strstr(mesh_value, "funball") ? APP_MESH_FUNBALL : APP_MESH_8BALL, src_mesh);
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

static void spawn_testcube_at(DWORD board, float px, float py, float pz, int grid_num, FILE* logf) {
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
    DWORD level = get_level(board);
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

static void despawn_object(DWORD board, DWORD obj, FILE* logf) {
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

    /* 4. Remove obj from board+0xCD4 (render list) */
    pfn_AthenaList_Remove((DWORD*)(board + BOARD_RENDER_LIST), (int)obj);

    /* 5. Remove obj from sceneobj+0x1C (scene tree) */
    DWORD level = get_level(board);
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

static void despawn_by_name(const char* target_name, DWORD board, FILE* logf) {
    int i;
    for (i = 0; i < g_spawned_count; i++) {
        if (strstr(g_spawned_names[i], target_name) != NULL) {
            if (logf) fprintf(logf, "  DESPAWN: found '%s' at index %d, obj=0x%08X\n",
                    g_spawned_names[i], i, g_spawned_objs[i]);
            despawn_object(board, g_spawned_objs[i], logf);
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
 * load_mesh_file — loads a .MESH or .MESHWORLD file
 *
 * For .MESHWORLD files: uses Level_MeshWorldCtor (0x461510).
 * For .MESH files: uses MeshNode_ctor (0x471C20) to load the .MESH binary.
 *
 * Returns a Level pointer (0x10D0 bytes, .MESHWORLD) or MeshNode pointer
 * (0x18 bytes, .MESH). Caller must check is_mesh_file to know which.
 * Returns NULL on failure (file not found, parse error, etc).
 * ═══════════════════════════════════════════════════════════════════════════ */
static void* load_mesh_file(DWORD gfx_device, const char* path, int* out_is_mesh_node, FILE* logf) {
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

static void spawn_rotater_at(DWORD board, float px, float py, float pz,
                              const char* mesh_path,
                              float rot_x, float rot_y, float rot_z,
                              float ros_x, float ros_y, float ros_z,
                              int ai_type,
                              FILE* logf) {
    if (!board) return;

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
        /* FlagWaver: code-generated mesh, no file needed */
        path = NULL;
    } else if (ai_type == 13) {
        /* Sign_ctor: loads its own mesh internally */
        path = NULL;
    } else if (ai_type == 14) {
        /* WavyFlag2: Wavy_ctor loads mesh from string path internally.
         * Skip mesh loading here — the path string is passed directly to Wavy_ctor. */
        path = NULL;
    } else if (ai_type == 15) {
        /* BadBall: BadBall_ctor doesn't take a mesh param.
         * Mesh is handled by the BadBall system (App mesh array). */
        path = NULL;
    } else if (ai_type == 16) {
        /* Bridgeslam: loads its own mesh inside the case block.
         * Skip external mesh load to avoid double-loading. */
        path = NULL;
    } else if (ai_type == 22) {
        /* Chomper: MeshNode_ctor loads mesh from string path internally */
        path = NULL;
    } else if (ai_type == 28) {
        /* Cloudscape: Sprite_ctor takes a string path, not a mesh pointer.
         * Path is determined inside the case block. */
        path = NULL;
    } else if (ai_type >= 30 && ai_type <= 33) {
        /* Bell/Fan/SawBlade/Bonk: constructors don't take a mesh parameter.
         * Bell/Fan/SawBlade call Level_ctor (no mesh).
         * Bonk calls Level_MeshWorldCtor (self-loads level5-bonk). */
        path = NULL;
    } else {
        path = g_swirl_mesh_path;
    }
    
    /* If mesh path is "levels\\_default", use _default.MESHWORLD as placeholder.
     * This file is provided by the user and serves as a null mesh fallback
     * for entities that don't have a real mesh file (Bumper, Tarpit, Chrome). */
    if (path && _stricmp(path, "levels\\_default") == 0) {
        /* _default.MESHWORLD is the user-provided placeholder */
        /* Keep the path as-is — the game's MeshWorld_ctor will try to load it */
    }

    /* 3. Load mesh file — handles both .MESHWORLD and .MESH formats.
     * NULL path is valid for entities with no mesh file (FlagWaver, Sign). */
    int is_mesh_node = 0;
    void* mesh = NULL;
    if (path) {
        mesh = load_mesh_file(gfx_device, path, &is_mesh_node, logf);
        if (!mesh) {
            /* Fallback: try Swirl mesh */
            if (logf) fprintf(logf, "  ROTATER: load_mesh_file failed for '%s', trying Swirl fallback\n", path);
            is_mesh_node = 0;
            mesh = load_mesh_file(gfx_device, g_swirl_mesh_path, &is_mesh_node, logf);
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
        void* swirl_mesh = load_mesh_file(gfx_device, g_swirl_mesh_path, &swirl_is_node, logf);
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
        DWORD level = get_level(board);
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
    } else if ((ai_type >= 7 && ai_type <= 13) || (ai_type >= 17 && ai_type <= 22) || (ai_type >= 27 && ai_type <= 33)) {
        /* New constructor types (7-13) — each with specific signature */
        switch (ai_type) {
            case 7:  /* DFloor1_ctor — Neon DFLOOR1 */
                obj = pfn_operator_new(ARENASTANDS_SIZE);
                if (!obj) { if (logf) fprintf(logf, "  ROTATER: failed to alloc DFloor1\n"); return; }
                memset(obj, 0, ARENASTANDS_SIZE);
                DFloor1_ctor(obj, (void*)board, px, py, pz, mesh);
                break;
            case 17: /* DFloor2_ctor — Neon DFLOOR2 */
                obj = pfn_operator_new(ARENASTANDS_SIZE);
                if (!obj) { if (logf) fprintf(logf, "  ROTATER: failed to alloc DFloor2\n"); return; }
                memset(obj, 0, ARENASTANDS_SIZE);
                DFloor2_ctor(obj, (void*)board, px, py, pz, mesh);
                break;
            case 18: /* DFloor3_ctor — Neon DFLOOR3 */
                obj = pfn_operator_new(ARENASTANDS_SIZE);
                if (!obj) { if (logf) fprintf(logf, "  ROTATER: failed to alloc DFloor3\n"); return; }
                memset(obj, 0, ARENASTANDS_SIZE);
                DFloor3_ctor(obj, (void*)board, px, py, pz, mesh);
                break;
            case 19: /* DFloor4_ctor — Neon DFLOOR4 (with post-construction config) */
                obj = pfn_operator_new(ARENASTANDS_SIZE);
                if (!obj) { if (logf) fprintf(logf, "  ROTATER: failed to alloc DFloor4\n"); return; }
                memset(obj, 0, ARENASTANDS_SIZE);
                DFloor4_ctor(obj, (void*)board, px, py, pz, mesh);
                break;
            case 20: /* FlickRing_ctor — Neon Arena FLICKRING */
                obj = pfn_operator_new(ARENASTANDS_SIZE);
                if (!obj) { if (logf) fprintf(logf, "  ROTATER: failed to alloc FlickRing\n"); return; }
                memset(obj, 0, ARENASTANDS_SIZE);
                FlickRing_ctor(obj, (void*)board, px, py, pz, mesh);
                break;
            case 21: /* Trode_ctor — Neon TRODE */
                obj = pfn_operator_new(ARENASTANDS_SIZE);
                if (!obj) { if (logf) fprintf(logf, "  ROTATER: failed to alloc Trode\n"); return; }
                memset(obj, 0, ARENASTANDS_SIZE);
                Trode_ctor(obj, (void*)board, px, py, pz, mesh);
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
                      * Calls Level_ctor (no mesh file). Mesh loaded by vtable[1] call. */
                obj = pfn_operator_new(BELL_SIZE);
                if (!obj) { if (logf) fprintf(logf, "  ROTATER: failed to alloc Bell\n"); return; }
                memset(obj, 0, BELL_SIZE);
                pfn_Bell_ctor(obj, (void*)board, px, py, pz);
                break;
            case 31: /* Fan_ctor — Expert Race Fan (6 params: this, board, x, y, z, float)
                      * Calls Level_ctor (no mesh file). Mesh loaded by vtable[1] call. */
                obj = pfn_operator_new(FAN_SIZE);
                if (!obj) { if (logf) fprintf(logf, "  ROTATER: failed to alloc Fan\n"); return; }
                memset(obj, 0, FAN_SIZE);
                pfn_Fan_ctor(obj, (void*)board, px, py, pz, 0.0f);
                break;
            case 32: /* SawBlade_ctor — Expert Race SawBlade (5 params: this, board, x, y, z)
                      * Calls Level_ctor (no mesh file). Mesh loaded by vtable[1] call. */
                obj = pfn_operator_new(SAWBLADE_SIZE);
                if (!obj) { if (logf) fprintf(logf, "  ROTATER: failed to alloc SawBlade\n"); return; }
                memset(obj, 0, SAWBLADE_SIZE);
                pfn_SawBlade_ctor(obj, (void*)board, px, py, pz);
                break;
            case 33: /* Bonk_ctor — Warm-Up Race Bonk (5 params: this, board, x, y, z)
                      * Self-loads "levels\\level5-bonk" MESHWORLD via Level_MeshWorldCtor. */
                obj = pfn_operator_new(BONK_SIZE);
                if (!obj) { if (logf) fprintf(logf, "  ROTATER: failed to alloc Bonk\n"); return; }
                memset(obj, 0, BONK_SIZE);
                pfn_Bonk_ctor(obj, (void*)board, px, py, pz);
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
                      * (home position). Mesh is loaded by calling vtable[1] after ctor,
                      * same as CreateBadBalls does. */
                obj = pfn_operator_new(BADBALL_SIZE);
                if (!obj) { if (logf) fprintf(logf, "  ROTATER: failed to alloc BadBall\n"); return; }
                memset(obj, 0, BADBALL_SIZE);
                pfn_BadBall_ctor(obj, (void*)board);
                /* Call vtable[1] — this loads the 8ball mesh (same as CreateBadBalls) */
                {
                    DWORD* vtable = *(DWORD**)obj;
                    if (vtable && vtable[1]) {
                        typedef void (__thiscall *vtable1_t)(void* this_);
                        ((vtable1_t)vtable[1])(obj);
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
                      * Per-frame update runs from bridgeslam_update() in the thread. */
                {
                    DWORD app = *(DWORD*)(board + BOARD_APP);
                    if (!app || IsBadReadPtr((void*)app, 4)) { if (logf) fprintf(logf, "  ROTATER: no app for Bridgeslam\n"); return; }
                    DWORD gfx_device = *(DWORD*)(app + APP_GFX_DEVICE);
                    if (!gfx_device) { if (logf) fprintf(logf, "  ROTATER: no gfx_device for Bridgeslam\n"); return; }

                    /* Load bridge mesh */
                    const char* bridge_path = mesh_path && mesh_path[0] ? mesh_path : "levels\\Level2-Bridge";
                    void* mesh = load_mesh_file(gfx_device, bridge_path, &is_mesh_node, logf);
                    if (!mesh) {
                        if (logf) fprintf(logf, "  ROTATER: Bridgeslam mesh load failed, trying Swirl fallback\n");
                        mesh = load_mesh_file(gfx_device, g_swirl_mesh_path, &is_mesh_node, logf);
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

    /* 5b. BadBall also goes into the bad balls list (board+0x29D4) */
    if (ai_type == 15) {
        pfn_AthenaList_Append((DWORD*)(board + BOARD_BAD_BALLS_LIST), obj);
    }

    /* 6. Add to board+0xCD4 (render list) */
    pfn_AthenaList_Append((DWORD*)(board + BOARD_RENDER_LIST), obj);

    /* 7. Add collision object to board+0x10EC */
    {
        DWORD col_off = 0x10E0;  /* default for PopCylinder */
        if (ai_type >= 1 && ai_type <= 6) col_off = 0x10D4;  /* Rotator family */
        else if (ai_type == 7 || (ai_type >= 17 && ai_type <= 21)) col_off = 0x10E0;  /* ArenaStands family */
        else if (ai_type == 8) col_off = 0x10D4;  /* GameLevel (Wobbly) */
        else if (ai_type == 9) col_off = 0x10D4;  /* Glass_Level (Drawbridge) */
        else if (ai_type == 10) col_off = 0x10D4; /* Gear_Level (Judge) */
        else if (ai_type == 11) col_off = 0x10E0; /* Secret (GlassBonus) */
        else if (ai_type == 12) col_off = 0;      /* FlagWaver — no collision obj */
        else if (ai_type == 13) col_off = 0x10EC; /* Sign — collision at 0x43B*4=0x10EC */
        else if (ai_type == 14) col_off = 0x10D4; /* WavyFlag2 — same as GameLevel */
        else if (ai_type == 15) col_off = 0;      /* BadBall — no collision obj (uses board+0x29D4 list) */
        else if (ai_type == 16) col_off = 0;     /* Bridgeslam — no collision obj (visual only) */
        else if (ai_type == 22) col_off = 0;     /* Chomper — tiny MeshNode, no collision */
        else if (ai_type == 27) col_off = 0x10D4; /* Spinner — same as Rotator family */
        else if (ai_type == 28) col_off = 0;      /* Cloudscape — background sprite, no collision */
        else if (ai_type == 29) col_off = 0x10D4; /* Gear — same as Rotator family */
        else if (ai_type == 30) col_off = 0x10D4; /* Bell — Level family */
        else if (ai_type == 31) col_off = 0x10D4; /* Fan — Level family */
        else if (ai_type == 32) col_off = 0x10D4; /* SawBlade — Level family */
        else if (ai_type == 33) col_off = 0x10D4; /* Bonk — Level family */
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
    DWORD level = get_level(board);
    if (level) {
        DWORD sceneobj = *(DWORD*)(level + LEVEL_SCENEOBJECT);
        if (sceneobj) {
            pfn_AthenaList_Append((DWORD*)(sceneobj + 0x1C), obj);
        }
    }

    if (logf) {
        fprintf(logf, "  ROTATER: spawned at (%.1f,%.1f,%.1f) obj=0x%08X mesh='%s' rot=(%.4f,%.4f,%.4f) oc=(%.1f,%.1f,%.1f)\n",
                px, py, pz, (DWORD)obj, path, rot_x, rot_y, rot_z,
                ros_x, ros_y, ros_z);
        fflush(logf);
    }

    /* Track for despawn + per-frame rotation updates */
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
static void despawn_all_rotaters(DWORD board, FILE* logf) {
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
static void apply_rotater_directions(void) {
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
static void update_constant_rotations(void) {
    int i;
    for (i = 0; i < g_rotater_count; i++) {
        if (g_rotater_cfg[i].ros_y != 0.0f) continue;  /* only for ROS_Y=0 */
        DWORD obj = g_rotater_cfg[i].obj;
        if (!obj || obj < 0x10000) continue;
        if (IsBadReadPtr((void*)obj, 0x10F0)) continue;
        *(float*)(obj + 0x10EC) = g_rotater_cfg[i].rot_y;
    }
}

/* Scan S1 ref points for Rotater entries with custom rot tags.
 * For each found, search the board's update list for the natively-spawned
 * Rotator object at the matching position, and apply ROT_Y to its direction
 * field (+0x10EC). This does NOT spawn — native game already spawned from S1. */
static void apply_s1_rotater_tags(DWORD board, FILE* logf) {
    if (!board) return;
    DWORD sceneobj = get_sceneobj(board);
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
        extract_dat_prop(name, "ROT_Y", rot_y_str, sizeof(rot_y_str));
        extract_dat_prop(name, "ROS_Y", ros_y_str, sizeof(ros_y_str));
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
    DWORD level = get_level(board);
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

    DWORD sceneobj = get_sceneobj(board);
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
            { "Bell",             30, NULL },                        /* Bell_ctor (0x434D70, 0x10E8) — Level_ctor, no mesh param */
            { "Blockdawg",        0, "levels\\Level8-BlockDawg1" }, /* Blockdawg_ctor */
            { "Bonk",             33, "levels\\Level5-Bonk" },       /* Bonk_ctor (0x438850, 0x1200) — self-loads level5-bonk MESHWORLD */
            { "Bridge",           16, "levels\\Level2-Bridge" },     /* Bridgeslam: isolated Intermediate bridge state machine */
            { "Bridgeslam",       16, "levels\\Level2-Bridge" },     /* Alias for Bridge */
            { "Bumper",           0, "levels\\_default" },          /* N:BUMPER tag — no _ctor, _default mesh */
            { "Catapult",         0, "levels\\Level4-Catapult" },   /* Catapult_ctor */
            { "Chomper",          22, "meshes\\chomper" },           /* Chomper_ctor (MeshNode_ctor, 0x471C20, 0x18 bytes) */
            { "Chrome",           23, "levels\\_default" },          /* Chrome_ctor: no _ctor, board-level behavior, PopCylinder fallback */
            { "Cloudscape",       28, "levels\\Cloudscape" },       /* Cloudscape (Sprite_ctor, 0x45D0C0, 0xD4) — Sky Race clouds */
            { "Drawbridge",       9, "levels\\Level4-Drawbridge" }, /* Glass_Level_ctor (0x4384A0, 0x113C bytes) */
            { "Droplifter",       0, "levels\\Level6-Lifter" },     /* Odd Race model */
            { "Fan",              31, NULL },                        /* Fan_ctor (0x438C20, 0x1188) — Level_ctor, no mesh param */
            { "Flag",             12, NULL },                        /* FlagWaver_Ctor (0x46AF30, 0x8C bytes) — code-generated mesh */
            { "Flag2",            14, "levels\\Flag" },               /* WavyFlag2: Wavy_ctor copy, uses Flag.MESHWORLD or _default fallback */
            { "Flickfloor1",      7,  "levels\\LevelDark-DFloor1" },  /* DFloor1_ctor (ArenaStands_ctor, 0x43E450, 0x1104) */
            { "Flickfloor2",     19, "levels\\LevelDark-DFloor4" },  /* DFloor4_ctor (ArenaStands + post-config: obj+0x10DC=2, obj+0x10E0=0) */
            { "Flickring",       20, "levels\\LevelDark-Flickring" }, /* FlickRing_ctor (ArenaStands_ctor) */
            { "Funball",          24, "meshes\\funball" },           /* Funball_ctor: no _ctor, board-level behavior, PopCylinder fallback */
            { "Gear",             29, "levels\\LevelImpossible-Gear" }, /* Gear_ctor (0x437690, 0x1514, 9 params!) */
            { "Glassbreaker",     11, "meshes\\GlassBonus" },       /* Secret_ctor (0x43DFB0, 0x10EC bytes) */
            { "Gluebie",          0, "levels\\Level3-Gluebie" },    /* Gluebie_ctor */
            { "Judge",            10, "meshes\\hammyjudge" },       /* Gear_Level_ctor (0x43A150, 0x1100 bytes, no mesh param) */
            { "Lifter",           0, "levels\\LevelUp-Lifter" },    /* Up Race model */
            { "Looper",           3, "levels\\LevelImpossible-Looper" }, /* Looper_ctor (0x437460, 0x1500, 6 params) */
            { "Mace",             0, "levels\\Level4-Mace" },       /* Mace_ctor */
            { "Mag",              0, "meshes\\magnifyingglass" },   /* .MESH — Magnifier_ctor */
            { "Mousetrap",        0, "levels\\MouseTrap" },        /* MouseTrap_ctor */
            { "Neonplatform",     0, "levels\\LevelDark-NeonPlatform" }, /* NeonPlatform_ctor */
            { "Pendulum",         2, "levels\\LevelImpossible-Pendulum" }, /* Pendulum_ctor */
            { "Popcylinder",      0, "levels\\Level9-PopCylinder1" }, /* PopCylinder_ctor */
            { "Rotator",          1, "levels\\LevelImpossible-Rotator" }, /* Rotator_ctor (constant rotation) */
            { "Saw",              0, "levels\\Level8-Saw" },        /* Saw_ctor */
            { "Sawblade",         32, NULL },                        /* SawBlade_ctor (0x434660, 0x111C) — Level_ctor, no mesh param */
            { "Sign",             13, "levels\\PopupSign" },        /* Sign_ctor (0x443B90, 0x10FC bytes, complex signature) */
            { "Speedcylinder",    0, "levels\\LevelUp-SpeedCylinder" }, /* SpeedCylinder_ctor */
            { "Spinner",          27, "levels\\Level8-Spinny" },     /* Spinner_Level_ctor (0x4396F0, 0x10FC) */
            { "Swirl",            6, "levels\\Level3-Swirl" },      /* Rotator_ctor_Impossible */
            { "Tarbubble",        25, "meshes\\tarbubble" },         /* Tarbubble_ctor: no _ctor, board-level behavior, PopCylinder fallback */
            { "Tarpit",           0, "levels\\_default" },          /* N:TARPIT tag — no _ctor, _default mesh */
            { "Timebutton",       0, "levels\\LevelUp-Button" },    /* TimeButton_ctor */
            { "Tipper",           0, "levels\\Level3-Tipper" },     /* Tipper_ctor */
            { "Trapdoor",         0, "levels\\Level4-Trapdoor1" },  /* Trapdoor_ctor */
            { "Trode",            21, "levels\\LevelDark-Trode" },   /* Trode_ctor (ArenaStands_ctor) */
            { "Waterwheel",       26, "levels\\Level3-WaterWheel" }, /* Waterwheel_ctor: no _ctor, position-only storage, PopCylinder fallback */
            { "Wavy",             0, "levels\\Level7-Wavy1" },      /* Wavy_ctor */
            { "Windmill",         0, "levels\\Level4-Windmill" },   /* Tower: Level_RenderCtor + TipperVisual_Attach */
            { "Wobbly",           8, "levels\\Level7-Wobbly1" },    /* GameLevel_ctor (0x4351F0, 0x1524 bytes) */
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

        /* v52: Skip Swirl entity spawning on levels that natively have SWIRL
         * objects. Dizzy Race, Master Race, and all Arena levels already spawn
         * SWIRL via the game's own vtable[33] handler. Spawning additional
         * Swirls from cEnt entries causes duplicate SWIRL objects.
         * Also skip "Rotater" entries on Dizzy/Master/Arena — native game
         * already spawns Rotators from S1 entries named "Rotater". */
        if (ai_type == 6) {
            /* Swirl entity — check if level natively has SWIRL */
            DWORD app = *(DWORD*)(board + BOARD_APP);
            if (app && !IsBadReadPtr((void*)(app + 0x23C), 4)) {
                /* Check level name from board */
                char* level_name = NULL;
                if (!IsBadReadPtr((void*)(board + 0x10), 4)) {
                    level_name = *(char**)(board + 0x10);
                }
                if (level_name && !IsBadReadPtr(level_name, 5)) {
                    if (_strnicmp(level_name, "Dizzy", 5) == 0 ||
                        _strnicmp(level_name, "Master", 6) == 0 ||
                        _strnicmp(level_name, "Board (Arena", 12) == 0 ||
                        _strnicmp(level_name, "Arena", 5) == 0) {
                        if (logf) fprintf(logf, "  cEnt(S3): '%s' — SKIPPED (native SWIRL on %s)\n",
                                entity_name, level_name);
                        continue;
                    }
                }
            }
        }

        /* v52: Rotator (AI 1) uses constant rotation (ROS_Y=0).
         * The native render function oscillates at ±2.0 by default.
         * With ROS_Y=0, update_constant_rotations() rewrites the direction
         * field every frame to prevent oscillation reversal. */
        float spawn_ros_y = 2.0f;
        if (ai_type == 1) spawn_ros_y = 0.0f;  /* Rotator: constant rotation */

        spawn_rotater_at(board, px, py, pz, ai_mesh,
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
        fprintf(logf, "=== Custom Entities Mod v53d Started ===\n");
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
                    bridgeslam_update(&g_bridgeslams[i]);
                }
            }
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
        DWORD level = get_level(board);
        if (!level) continue;

        logf = NULL;
        fopen_s(&logf, log_path, "a");
        if (logf) {
            fprintf(logf, "\n--- Level loaded (board=0x%08X, level=0x%08X) ---\n", board, level);
        }

        /* Process <MESH> and <SPEEDMULT> tags on spawned 8-balls (after CreateBadBall has run) */
        process_custom_tags(board, logf);

        /* Process C_entity entries — spawn custom objects at each position */
        process_rotaters(board, logf);

        /* Hide original meshbuffers for C_entity entries */
        hide_entity_meshbuffers(board, logf);

        /* Apply custom rotation directions to spawned rotaters */
        apply_rotater_directions();

        /* Apply S1 rot tags to natively-spawned Rotators */
        apply_s1_rotater_tags(board, logf);

        /* Find GRID reference points */
        float grid_x[32], grid_y[32], grid_z[32];
        int grid_count = find_grid_points(board, grid_x, grid_y, grid_z, 32, logf);

        if (grid_count > 0) {
            g_spawned_board = board;
            if (logf) fprintf(logf, "  Found %d GRID points, starting cycle (speed=%.1fs)\n", grid_count, g_grid_speed);

            /* Start with GRID01 visible */
            int current_grid = 1;
            spawn_testcube_at(board, grid_x[0], grid_y[0], grid_z[0], 1, logf);
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
                    despawn_object(board, g_spawned_objs[0], logf);
                    int j;
                    for (j = 0; j < g_spawned_count - 1; j++) {
                        g_spawned_objs[j] = g_spawned_objs[j + 1];
                        strcpy(g_spawned_names[j], g_spawned_names[j + 1]);
                    }
                    g_spawned_count--;
                }

                /* Spawn the current GRID */
                int idx = current_grid - 1;
                spawn_testcube_at(board, grid_x[idx], grid_y[idx], grid_z[idx], current_grid, logf);
                if (logf) {
                    fprintf(logf, "  Cycle: GRID%02d spawned\n", current_grid);
                    fflush(logf);
                }
            }

            /* Clean up any remaining spawned objects on level exit */
            while (g_spawned_count > 0) {
                despawn_object(board, g_spawned_objs[0], logf);
                int j;
                for (j = 0; j < g_spawned_count - 1; j++) {
                    g_spawned_objs[j] = g_spawned_objs[j + 1];
                    strcpy(g_spawned_names[j], g_spawned_names[j + 1]);
                }
                g_spawned_count--;
            }

            /* Despawn all rotater objects on level exit */
            despawn_all_rotaters(board, logf);
        } else {
            /* No GRID points — still mark board as processed */
            g_spawned_board = board;
            /* Still process rotaters even without GRID points */
            despawn_all_rotaters(board, logf);
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
