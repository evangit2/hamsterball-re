# Version Changelog

## v53g

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
