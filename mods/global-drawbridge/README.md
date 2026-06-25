# Global Drawbridge Spawner

Spawns Tower Race drawbridges at Player 1's position in any level via hotkey.

## Usage
1. Enable script in Cheat Engine
2. Press hotkey (set `SpawnDrawbridge` to 1) to spawn a drawbridge at player position
3. Drawbridges animate (open/close) automatically via vtable[0x2C] update

## Root Cause of Original Crash

**board+0x4378 is a REUSED SLOT across levels:**

| Level | board+0x4378 contains |
|-------|----------------------|
| Tower (Level4) | `Levels\Level4-Mace` mesh (set by BoardLevel5_Tower_ctor) |
| Beginner (LevelCascade) | 0 (unused) |
| Other races | varies — may contain other mesh data |

The original script wrote the drawbridge mesh to board+0x4378 permanently and never restored it. In Beginner Race the slot was 0 (unused) so it worked. In Tower Race and other races, the slot already contained another level's mesh, causing:
- Wrong mesh used for drawbridge (Mace mesh instead of Drawbridge mesh)
- Board slot corrupted for original objects → crash when level objects try to use the slot

## Fix: JIT Mesh Injection

Save board+0x4378 → inject drawbridge mesh → call constructor → restore board+0x4378.

This is the same pattern as the universal ref loader (see skill `hamsterball-dll-modding`).

## Functions Traced

| Address | Name | Description |
|---------|------|-------------|
| 0x4396F0 | Spinner_Level_ctor | `__thiscall`, RET 0x14 (5 params). Reads board+0x4378 for mesh. |
| 0x461510 | MeshWorld_ctor | `__thiscall`, RET 0x8. Loads MESHWORLD file. |
| 0x462850 | Stands_ctor | Clones SpatialTree from mesh source. |
| 0x453810 | AthenaList_Append | `__thiscall`, RET 4. Adds to list. |
| 0x439870 | Impossible_Level_Update | vtable[0x2C]. Drawbridge animation: timer, rotation, sound. |
| 0x4BA57B | operator_new | `__cdecl`. Allocates memory. |
| 0x40E250 | CreateSawblade | Game factory. Creates BONK, TOWER, SAWBLADE, BRIDGE, JUDGE, BELL. |
| 0x41E340 | BoardLevel5_Tower_ctor | Tower board init. Loads meshes into board+0x43xx slots. |

## Board Slot Layout (Tower Race)

| Offset | Mesh | Set by |
|--------|------|--------|
| board+0x436C | Levels\Level4-Catapult | BoardLevel5_Tower_ctor |
| board+0x4370 | Levels\Level4-Drawbridge | BoardLevel5_Tower_ctor |
| board+0x4374 | Meshes\YellowLink | BoardLevel5_Tower_ctor |
| board+0x4378 | Levels\Level4-Mace | BoardLevel5_Tower_ctor |
| board+0x437C | Levels\Level4-Windmill | BoardLevel5_Tower_ctor |
| board+0x4390 | Meshes\Chomper | BoardLevel5_Tower_ctor |
| board+0x43B4 | Levels\Level4-Turret | BoardLevel5_Tower_ctor |

## Object Registration

Drawbridges are registered in:
- `board+0x2578` — active objects list (update + render)
- `board+0xCD4` — collision objects list
- `board+0x10EC` — collision level list
- `board+0x8B0+0x18` — collision dispatch list
- `board+0x8AC→+0x480→+0x1C` — render list (post-alpha)

## Object Structure (Spinner_Level / Drawbridge)

| Offset | Type | Description |
|--------|------|-------------|
| +0x000 | int* | vtable (0x4D51E0) |
| +0x10D0 | int | board pointer |
| +0x10D4 | float | position X |
| +0x10D8 | float | position Y |
| +0x10DC | float | position Z |
| +0x10E0 | float | rotation angle |
| +0x10E4 | int | state (0=initialized) |
| +0x10E8 | int | unknown (0) |
| +0x10F0 | int | timer countdown (100) |
| +0x10F4 | void* | CollisionLevel pointer |
| +0x10F8 | float | rotation direction (1.0 or -1.0) |
