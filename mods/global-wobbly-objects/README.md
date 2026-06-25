# Global Wobbly Race Object Spawner — WAVY1, WOBBLY1-4

Spawns Wobbly Race objects at Player 1's position in any level.

## Usage

1. Set `ObjectType` (1-5):
   - 1 = WAVY1 (wavy floor — wave-based physics surface)
   - 2 = WOBBLY1 (wobbly platform 1)
   - 3 = WOBBLY2 (wobbly platform 2)
   - 4 = WOBBLY3 (wobbly platform 3)
   - 5 = WOBBLY4 (wobbly platform 4)
2. Set `SpawnObject` to 1 to spawn at player position

## How Each Object Works

### WAVY1 (Wavy Floor)

The actual wavy floor from Wobbly Race — a large physics surface that creates wave-based deformation.

- **Constructor**: `Stands_CtorWithCollisionLevel` (0x43AD40), alloc **0x1AE7C** (110,204 bytes — huge!)
- **RET**: 0x14 (5 params: board, X, Y, Z, mesh_path_string)
- **Mesh**: `"Levels\Level7-Wavy1"` (string at 0x4CFC54)
- **vtable**: 0x4D5458
- **Update** (vtable[0x0B/+0x2C] = 0x440390, `Scene_UpdateArenaPhysics`):
  - Increments counter at +0x10F8 each frame
  - When counter > 2: begins wave physics
  - Uses `Wave_Sin` for wave motion deformation across multiple segments
  - Calls `Mesh_FindClosestCollision` for ball collision
  - Transforms ball positions riding on the surface
  - **No path access, no external dependencies → safe for global spawn**

**Key difference from WOBBLY**: Takes a mesh **path string** (not MeshWorld pointer). The constructor internally calls `MeshWorld_ctor(this, device, path)` to create its own mesh. No need to create a separate MeshWorld.

### WOBBLY1-4 (Wobbly Platforms)

Wobbly platforms that create a wave-like motion for balls riding on them.

- **Constructor**: `GameLevel_ctor` (0x4351F0), alloc 0x1524
- **RET**: 0x14 (5 params: board, X, Y, Z, mesh_pointer)
- **vtable**: 0x4D53F8 (Spinner vtable)
- **Update** (vtable[0x0B/+0x2C] = 0x43A700, `Stands_Update`):
  - Reads wave amplitudes at +0x10F0/+0x10F4
  - Reads position at +0x10D8/+0x10DC/+0x10E0
  - Reads wave scale at +0x1100 (=150.0)
  - Reads max amplitude at +0x10E8/+0x10EC (=10.0)
  - Iterates balls on platform, transforms positions (wave motion)
  - Uses `Gfx_ScaleY/Z` for deformation
  - **No path access, no crash risks → safe for global spawn**

**Key difference from WAVY1**: Takes a MeshWorld **pointer** (not path string). Must create a `MeshWorld` first via `MeshWorld_ctor`, then pass the pointer.

## Factory Chain

```
BoardLevel12_Wobbly_ctor (0x41F110):
  LoadRaceData("WOBBLYRACE")
  board+0x436C = MeshWorld("Levels\Level7-Wobbly1")
  board+0x4370 = MeshWorld("Levels\Level7-Wobbly2")
  board+0x4374 = MeshWorld("Levels\Level7-Wobbly3")
  board+0x4378 = MeshWorld("Levels\Level7-Wobbly4")
  board+0x437C = MeshWorld("Levels\Level7-Wobbly5")
  board+0x4380 = MeshWorld("Levels\Level7-Wobbly6")
  board+0x4384 = MeshWorld("Levels\Level7-Wobbly7")

Factory Dispatch (0x40F420, vtable[0x21]/+0x84):
  "WOBBLY1" → GameLevel_ctor, mesh=board+0x436C
  "WOBBLY2" → GameLevel_ctor, mesh=board+0x4370
  "WAVY1"   → Stands_CtorWithCollisionLevel, mesh_path="Levels\Level7-Wavy1"
  "WOBBLY3" → GameLevel_ctor, mesh=board+0x4374
  "WOBBLY4" → GameLevel_ctor, mesh=board+0x4378
  Fallback  → CreatePlatformOrStands (PLATFORM, STANDS)
```

## Object Field Layout

### WAVY1 — vtable 0x4D5458

| Offset | Type | Description |
|--------|------|-------------|
| +0x000 | int* | vtable (0x4D5458) |
| +0x10D0 | int | board pointer |
| +0x10D4 | void* | CollisionLevel pointer |
| +0x10E0 | float | position X |
| +0x10E4 | float | position Y |
| +0x10E8 | float | position Z |
| +0x10D8 | int | 0 (unused?) |
| +0x10F8 | int | frame counter (increments, wave physics starts at >2) |
| +0x1104 | array | Vec3List array (0x32 entries × 0x418 bytes) |
| +0xDDB4 | array | Vec3List array (0x32 entries × 0x418 bytes) |
| +0x1AA64 | AthenaList | ball list |

### WOBBLY1-4 — vtable 0x4D53F8

| Offset | Type | Description |
|--------|------|-------------|
| +0x000 | int* | vtable (0x4D53F8) |
| +0x10D0 | int | board pointer |
| +0x10D4 | void* | CollisionLevel pointer |
| +0x10D8 | float | position X |
| +0x10DC | float | position Y |
| +0x10E0 | float | position Z |
| +0x10E4 | byte | flag (1 = active) |
| +0x10E8 | float | max amplitude X (10.0) |
| +0x10EC | float | max amplitude Z (10.0) |
| +0x10F0 | float | wave amplitude X |
| +0x10F4 | float | wave amplitude Z |
| +0x1100 | float | wave scale (150.0) |
| +0x1104 | byte | freeze X flag |
| +0x1105 | byte | freeze Z flag |
| +0x1108 | AthenaList | ball list |
| +0x1520 | int | sound channel |

## Mesh String Addresses

| Address | String |
|---------|--------|
| 0x4D0CF4 | `Levels\Level7-Wobbly1` |
| 0x4D0CDC | `Levels\Level7-Wobbly2` |
| 0x4D0CC4 | `Levels\Level7-Wobbly3` |
| 0x4D0CAC | `Levels\Level7-Wobbly4` |
| 0x4CFC54 | `Levels\Level7-Wavy1` |

## String Table (0x4CFC40+)

| Address | String |
|---------|--------|
| 0x4CFC44 | WOBBLY4 |
| 0x4CFC4C | WOBBLY3 |
| 0x4CFC54 | Levels\Level7-Wavy1 |
| 0x4CFC68 | WAVY1 |
| 0x4CFC70 | WOBBLY2 |
| 0x4CFC78 | WOBBLY1 |
| 0x4CFC80 | N:WAVY |
| 0x4CFC88 | N:SQUAREWOBBLY |
