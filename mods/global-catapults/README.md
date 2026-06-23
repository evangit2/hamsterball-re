# Global Catapults (CEA)

Spawns functional catapults on any level/arena. Ball is launched via the game's native collision system (`E:CATAPULTBOTTOM`), not proximity detection.

## Usage

1. Enable the script in Cheat Engine.
2. Set `SpawnCatapult` to 1 in the address list.
3. Enter any race or arena.
4. A catapult spawns at the ball's position (20 units below).
5. Roll over the catapult's `E:CATAPULTBOTTOM` surface — the ball gets launched.

## What Changed from v1

| Issue | v1 (Original) | v2 (Fixed) |
|-------|--------------|------------|
| Board+0x43B8 crash | Uninitialized AthenaList on non-Tower levels → `_realloc(garbage)` crash | Checks vtable vs 0x4D875C, calls `AthenaList_Init(0x00453210)` if needed |
| Board+0x584C crash | Same uninitialized list crash | Same fix |
| Proximity launch | Checks XZ distance < 9.0 and manually calls Catapult_Launch | **Removed entirely** — collision system handles launch via `E:CATAPULTBOTTOM` |
| `float_9` | 9.0 proximity constant | Removed (unused) |

## How Launch Works (Original Game)

1. Ball touches `E:CATAPULTBOTTOM` collision triangle on the catapult's CollisionLevel mesh
2. Collision dispatch fires:
   - **Race mode:** `Level_HandleCollision` (0x0040DCD0) iterates `Board+0x43B8`
   - **Arena mode:** `CreateSpinner` (0x00412850) iterates `Board+0x584C`
3. For each catapult in the list, checks `catapult+0x10D4 == *collision_entry`
4. On match: sets `catapult+0x10EC = ball_ptr`, calls `Catapult_Launch(catapult)`
5. `Catapult_Launch` sets `+0x10F0 = 1` (active), `+0x10F4 = 50` (launch timer frames)
6. `Catapult_Update` (0x0043E600) applies matrix transform each frame, physically launching the ball

## Board AthenaList Initialization

`Board_ctor` (0x00419030) initializes these AthenaLists:
```
0x08B8, 0x0CD4, 0x10EC, 0x1518, 0x1930, 0x1D48,
0x2160, 0x2578, 0x29D4, 0x2DEC, 0x3204, 0x362C
```

But `Board+0x43B8` and `Board+0x584C` are **only** initialized by level-specific ctors:
- `BoardLevel5_Tower_ctor` (0x0041E340) → inits +0x43B8, +0x584C
- `BoardLevel_Master_Ctor` (0x004206D0) → inits +0x584C

On all other levels these offsets contain garbage from malloc. The fix checks the vtable pointer (first 4 bytes) against `0x4D875C` (the AthenaList vtable set by `AthenaList_Init`). If it doesn't match, the list is uninitialized and `AthenaList_Init` is called before appending.

## Catapult Struct Layout (0x1108 bytes)

| Offset | Type | Description |
|--------|------|-------------|
| +0x0000 | void* | vtable (0x4D4F98 after Catapult_ctor) |
| +0x10D0 | void* | parent Board pointer |
| +0x10D4 | void* | CollisionLevel pointer (matched in collision dispatch) |
| +0x10D8 | float | position X |
| +0x10DC | float | position Y |
| +0x10E0 | float | position Z |
| +0x10E4 | float | rotation |
| +0x10E8 | float | launch direction Y (-1.0 = upward) |
| +0x10EC | void* | ball pointer (set by collision dispatch on launch) |
| +0x10F0 | byte | triggered flag (1 = launch active) |
| +0x10F4 | int | launch timer (50 frames) |
| +0x10F8 | AthenaList | tracked objects (0x418 bytes) |
| +0x1100 | byte | active state (1 = active) |
| +0x1104 | float | launch power (17.0) |
