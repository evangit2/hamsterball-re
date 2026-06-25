# Global Toob Race Object Spawner — SAW, SAW2, FALLOUT1, SPINNY, BLOCKDAWGS

Spawns Toob Race objects at Player 1's position in any level.

## Usage

1. Set `ObjectType` (1-6):
   - 1 = SPINNY (rotating platform)
   - 2 = SAW (big saw blade)
   - 3 = SAW2 (small saw blade)
   - 4 = FALLOUT1 (fallout floor)
   - 5 = BLOCKDAWG1 (block dog enemy 1)
   - 6 = BLOCKDAWG2 (block dog enemy 2)
2. Set `SpawnObject` to 1 to spawn at player position

## How Each Object Is Created and Works

### Factory Function (0x40FB28)

All Toob Race mechanical objects are created by a single unlabeled factory function at 0x40FB28. It uses `__strnicmp` to match ref names from the MESHWORLD file:

| Ref Name | Constructor | Alloc | RET | Mesh | Mesh Slot | Path |
|----------|-------------|-------|-----|------|-----------|------|
| SPINNY | Rotator_ctor (0x435940) | 0x1508 | 0x14 | Level8-Spinny | board+0x436C | — |
| SAW | Stands_CtorCollision (0x43B780) | 0x1110 | 0x18 | Level8-Saw | board+0x4370 | SAWPATH |
| SAW2 | Stands_CtorSpeedCylinder (0x43BE20) | 0x1118 | 0x18 | Level8-Saw | board+0x4370 | SMALLSAWPATH |
| FALLOUT1 | Stands_CtorCollisionV2 (0x43BBC0) | 0x10E8 | 0x14 | Level8-Fallout | board+0x4374 | — |
| BLOCKDAWG1 | Blockdawg_ctor (0x43C310) | 0x1154 | 0x18 | Level8-Blockdawg1 | board+0x4378 | DAWGPATH1 |
| BLOCKDAWG2 | Blockdawg_ctor (0x43C310) | 0x1154 | 0x18 | Level8-Blockdawg1 | board+0x4378 | DAWGPATH2 |

### SPINNY (Rotator_ctor)

- **Constructor**: `Rotator_ctor(this, board, X, Y, Z, mesh)` — 5 stack params, RET 0x14
- **Mesh**: `Levels\Level8-Spinny` (string at 0x4D0E38)
- **vtable**: 0x4D5518 (Lifter vtable)
- **CollisionLevel**: +0x10D4
- **Position**: +0x10D8 (X), +0x10DC (Y), +0x10E0 (Z)
- **Update** (vtable[0x0B/+0x2C]): Catapult_Update (0x43E600) — rotates platform, transforms balls riding on it
- **No path needed** — safe for global spawn

### SAW (Stands_CtorCollision)

- **Constructor**: `Stands_CtorCollision(this, board, X, Y, Z, mesh, pathId)` — 6 stack params, RET 0x18
- **Mesh**: `Levels\Level8-Saw` (string at 0x4D0E24)
- **vtable**: 0x4D5578 (Button vtable)
- **pathId**: stored at +0x10D8
- **Update** (0x43B8E0):
  - Reads `+0x110C` (byte flag). If 0 → **skips entire update** (safe, no path access)
  - If +0x110C != 0: reads `+0x10F4` (state=2), uses `+0x10D8` (pathId) → **crashes if 0**
- **CRASH FIX**: Set `+0x110C = 0` after construction → update does nothing, object renders but stays static

### SAW2 (Stands_CtorSpeedCylinder)

- **Constructor**: `Stands_CtorSpeedCylinder(this, board, X, Y, Z, mesh, pathId)` — 6 params, RET 0x18
- **Calls Stands_CtorCollision first**, then overrides:
  - vtable → 0x4D5CA0 (Button variant)
  - +0x10F0 = 0xC2C80000 (float)
  - +0x1110 = 1 (flag)
  - +0x1114 = 0x3F800000 (1.0)
- **Update** (0x43BEB0): Same `+0x110C` check → if 0, skips (safe)
- **CRASH FIX**: Set `+0x110C = 0` after construction

### FALLOUT1 (Stands_CtorCollisionV2)

- **Constructor**: `Stands_CtorCollisionV2(this, board, X, Y, Z, mesh)` — 5 params, RET 0x14
- **Mesh**: `Levels\Level8-Fallout` (string at 0x4D0E0C)
- **vtable**: 0x4D55D8 (SpeedCylinder vtable)
- **Update** (0x43BD10): Reads `+0x10D8` (pathId pointer), **if null → skips path code gracefully**
- **No crash fix needed** — handles null pathId automatically

### BLOCKDAWG1/2 (Blockdawg_ctor)

- **Constructor**: `Blockdawg_ctor(this, board, X, Y, Z, mesh, pathId)` — 6 params, RET 0x18
- **Mesh**: `Levels\Level8-Blockdawg1` (string at 0x4D0DF0, same for both)
- **vtable**: 0x4D5638 (RumbleObject vtable)
- **pathId**: stored at +0x10F0
- **Update** (0x43C4E0, RumbleObject_Update):
  - If `+0x1150 == 0` (awake): follows path via `Path_GetPosition(+0x10F0)` → **crashes if 0**
  - If `+0x1150 != 0` (sleeping): plays wake-up sound + creates particle ring → **safe**
- **CRASH FIX**: Set `+0x1150 = 1` (sleeping mode)
- **BLOCKDAWG2 difference**: Also sets `+0x1152 = 1` (is_blockdawg2 flag, set by factory)

## Rendering

All constructors call `Stands_ctor(this, mesh)` which clones the SpatialTree from the mesh. The mesh is added to the board's SpatialTree automatically. The board's render pipeline renders it through normal opaque/translucent passes — **no manual render list registration needed**.

This is why the user's Judge script (using `Gear_Level_ctor` → `Level_ctor` → empty mesh) showed nothing: Level_ctor doesn't clone a SpatialTree, so the board has no mesh to render. The Toob objects use Stands_ctor which properly sets up the mesh.

## Registration

Factory code adds each object to `board+0x2578` (active objects list) via `AthenaList_Append` (0x453810). The board's update loop iterates this list and calls `vtable[0x0B]` (+0x2C) for each object each frame.

## String Table (0x4CFC80+)

| Address | String | Purpose |
|---------|--------|---------|
| 0x4CFCB4 | DAWGPATH3 | Path for BLOCKDAWG3 |
| 0x4CFCC0 | BLOCKDAWG3 | Ref name (unused in standard Toob) |
| 0x4CFCCC | DAWGPATH2 | Path for BLOCKDAWG2 |
| 0x4CFCD8 | BLOCKDAWG2 | Ref name |
| 0x4CFCE4 | DAWGPATH1 | Path for BLOCKDAWG1 |
| 0x4CFCF0 | BLOCKDAWG1 | Ref name |
| 0x4CFCFC | FALLOUT1 | Ref name |
| 0x4CFD08 | SMALLSAWPATH | Path for SAW2 |
| 0x4CFD18 | SAW2 | Ref name |
| 0x4CFD20 | SAWPATH | Path for SAW |
| 0x4CFD28 | SAW | Ref name |
| 0x4CFD2C | SPINNY | Ref name |
