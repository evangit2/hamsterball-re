# Version Changelog

## v54 — MESH File Hotfix

- **Bell, Fan, SawBlade: Now spawnable on any level!**
  - These 3 entities previously crashed because their native constructors call
    `Level_ctor` (creates empty Level with no mesh), and their vtable[1]
    (Rotator_Update) needs vertex data from a loaded mesh.
  - **v54 approach:** Use PopCylinder_ctor to create a properly initialized
    Level with mesh data, then override the vtable to the entity's native
    vtable. This gives the object both valid mesh data AND native behavior.
  - **Bell (type 30):** Loads `meshes\\Bell` (.MESH) via MeshNode_ctor,
    swaps into obj+0x08, sets vtable=0x004D5330.
  - **Fan (type 31):** Loads `meshes\\fanbody` (.MESH) via MeshNode_ctor,
    swaps into obj+0x08, sets vtable=0x004D5180.
  - **SawBlade (type 32):** Loads `levels\\Level8-Saw` (.MESHWORLD) directly
    via PopCylinder_ctor, sets vtable=0x004D5240. Uses .MESHWORLD (not .MESH)
    because it has proper vertex data for Rotator_Update.
- **Also includes all v53g-5 fixes:**
  - Fixed critical despawn bug (entities destroyed on No-GRID levels)
  - Fixed Trapdoor/Odd_Lifter mesh leak + NULL guard
  - All 34 constructor addresses verified via Ghidra
- Crash test: 38.6s OK

## v53g-5

- **CRITICAL FIX: Entities despawned immediately on levels without GRID points**
  - The "No GRID points found" branch called `cEnt_despawn_all_rotaters()` which
    calls `vtable[11]` (RemoveAndFree) on ALL custom entities that were just
    spawned by `process_rotaters()`. This destroyed every custom entity on
    levels that have no GRID reference points.
  - Fix: Removed the `cEnt_despawn_all_rotaters()` call from the No-GRID branch.
    Entities are still properly cleaned up on level exit via the normal path.
- **FIX: Trapdoor/Odd_Lifter mesh leak and NULL mesh guard**
  - Trapdoor_ctor (0x438290) and Odd_Lifter_ctor (0x434E60) read mesh from
    `board+0x878+0x594/0x5C8` (App mesh table), NOT from the mesh parameter.
    The spawn function was loading a mesh file that was never used (memory leak).
  - Fix: Set `path=NULL` for types 41 and 42 so no mesh file is loaded.
  - Added safety guard: checks if App mesh table entry is valid before calling
    the constructor. If NULL, the entity is skipped (logged) instead of crashing.
- **Verified via Ghidra decompilation**:
  - All 34 constructor addresses confirmed correct
  - All 24 alloc size constants confirmed correct
  - All 8 Stands_ctor family calling conventions confirmed (v53g-4 fixes hold)
  - Stands_ctor (0x462850) internally calls SpriteAnim_Ctor, AthenaList_Init,
    Timer_Init, and SpatialTree_CloneToLevel — all verified safe
  - Collision offset logic (col_off=0 for all except Rotator 1-6) confirmed
  - Board list additions (0x2578, 0xCD4, 0x8B8, scene+0x1C) verified correct
  - Despawn logic (skip types 30-33) verified correct
- Crash test: 39.7s, no crash

## v53g-4

- **REVERTED v53f crash regression**: use_board_level_as_mesh and visual mesh swap
  caused 11 entities to crash (Rotator, Pendulum, Looper, Gear, Swirl, Flickfloor1/2,
  Flickring, Spinner, Trode, Bonk). All entities now load their own MESHWORLD files
  directly (back to v53e behavior).
- **Fixed leftover 0x95E0**: Per-frame monitoring code had a leftover board+0x95E0
  reference that corrupted the game's update list, causing crashes at 0x452376.
- **Fixed Flag/Flag2 crashes**: FlagWaver_Ctor creates a global renderer, not a
  per-entity object. Changed Flag from type 12 (FlagWaver) to type 14 (Wavy_ctor)
  with Flag.MESHWORLD path, same as Flag2.
- **Wobbly wobble**: Added type 8 (GameLevel) to board+0x8B8 (Scene_Update list)
  so Rotator_Update (vtable[1]) is called per-frame for vertex deformation.
- **Bridge tilt animation**: Changed Bridge from type 16 (custom PopCylinder) to
  type 34 (BreakBridge_ctor, 0x436D70) with Pendulum vtable and Rotator_Update.
  Added to board+0x8B8 for per-frame vertex deformation.
- **_default.MESHWORLD placeholder**: Entities with no real mesh (Bumper, Tarpit,
  Chrome) use levels\_default as placeholder.
- Known issues: .MESH entities (8ball, Bell, Fan, Funball) show as static Swirl
  due to MeshWorld structure mismatch. Needs further investigation.

## v53f

- **CRITICAL FIX: Crash root cause found and fixed**
  - Constructors like ArenaStands_ctor, Rotator_ctor, Looper_ctor, etc. internally call
    Stands_ctor/Level_RenderCtor which dereference SceneObject+0x440 (vertex data)
  - Separately-loaded .MESHWORLD meshes had NULL vertex data at this offset
  - Fix: pass the board's own Level (board+0x8AC) as the mesh parameter
  - Board's Level has fully-loaded vertex data - no more NULL dereference crashes
- **Visual mesh swap after construction**
  - After constructing with board Level, swap obj+0x08 (MeshWorld*) to desired visual mesh
  - This gives correct appearance while keeping valid collision/vtable from board Level
- **Rotator oscillation fixed**
  - Native render checks "if angle > 2.0" and "if angle < -2.0" to reverse direction
  - Fix: clamp angle to [-1.99, 1.99] every frame to prevent reversal
  - Constant rotation now works correctly

## v53e

- All mod functions renamed with `cEnt_` prefix to separate from game's originals
- Cloned all constructors/wrappers - mod now works with copies, not game functions directly
- README pipe alignment fixed
- README restructured: version changelogs moved to this file, README keeps only reference tables

## v53d

- Fixed 8ball/BadBall: now loads 8ball.MESH via cEnt_MeshNode_ctor and stores at ball+0x10
- Fixed BadBall alloc size: 0xC98 (was 0xC70)
- Added cEnt_Bonk_ctor (type 33, 0x438850, 0x1200) - Warm-Up Bonk, self-loads level5-bonk
- Bell/Fan/SawBlade: reverted to PopCylinder (type 0) - their ctors call Level_ctor (no mesh)
  and crash during board update because the vtable update method calls LoadMesh with invalid state

## v53c

- Isolated and cloned behaviors: Chomper, Chrome, Funball, Tarbubble, Waterwheel
- Fixed cEnt_Gear_ctor (9 params, was using 6-param Rotator typedef - would crash!)
- Fixed Looper size (0x1500, was 0x1508)
- Added cEnt_Spinner_Level_ctor (Expert Race "BRIDGE")
- Added Cloudscape entity (Sky Race clouds, cEnt_Sprite_ctor)
- Named cEnt_ wrappers for all Neon Race objects (cEnt_DFloor1-4, cEnt_FlickRing_ctor, cEnt_Trode_ctor)

## v53b

- **Deep Ghidra decompilation** of every `CreateDynamicObjects` function in the game
- Added 7 new constructor types with correct alloc sizes and calling conventions
- Fixed 10 entities that were using the wrong `_ctor`
- NULL mesh paths now use `levels\_default` as placeholder (your `_default.MESHWORLD` file)

### Entities fixed in v53b

| Entity       | Old _ctor   | New _ctor             | Address  | Size   |
| ------------ | ----------- | --------------------- | -------- | ------ |
| Drawbridge   | PopCylinder | cEnt_Glass_Level_ctor | 0x4384A0 | 0x113C |
| Flag         | PopCylinder | cEnt_FlagWaver_Ctor   | 0x46AF30 | 0x8C   |
| Flickfloor1  | PopCylinder | cEnt_ArenaStands_ctor | 0x43E450 | 0x1104 |
| Flickfloor2  | PopCylinder | cEnt_ArenaStands_ctor | 0x43E450 | 0x1104 |
| Flickring    | PopCylinder | cEnt_ArenaStands_ctor | 0x43E450 | 0x1104 |
| Glassbreaker | PopCylinder | cEnt_Secret_ctor      | 0x43DFB0 | 0x10EC |
| Judge        | PopCylinder | cEnt_Gear_Level_ctor  | 0x43A150 | 0x1100 |
| Sign         | PopCylinder | cEnt_Sign_ctor        | 0x443B90 | 0x10FC |
| Trode        | PopCylinder | cEnt_ArenaStands_ctor | 0x43E450 | 0x1104 |
| Wobbly       | PopCylinder | cEnt_GameLevel_ctor   | 0x4351F0 | 0x1524 |
