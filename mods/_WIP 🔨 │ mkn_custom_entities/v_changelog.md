# Version Changelog

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
