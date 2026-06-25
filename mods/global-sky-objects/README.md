# Global Sky Race Object Spawner — FUNBALL, PILLAR, MAGNIFYER

Spawns Sky Race objects at Player 1's position in any level via hotkey.

## Usage

1. Set `ObjectType` (1-3):
   - 1 = FUNBALL (PopCylinder — blue ball that pops up near goal)
   - 2 = PILLAR (Platform — pillar that pops up and blocks path)
   - 3 = MAGNIFYER (CollisionLevel_Spatial — magnifying glass lens effect)
2. Set `SpawnObject` to 1 to spawn at player position

## How Each Object Works

### FUNBALL (PopCylinder)

The blue ball that pops up near the goal in Sky Race.

- **Constructor**: `PopCylinder_ctor` (0x436EE0), alloc **0x10E8**
- **RET**: 0x14 (5 params: board, X, Y, Z, mesh)
- **vtable**: 0x4D58F0
- **Mesh**: `"levels\level9-popcylinder1"` (string at 0x4D0F5C)
- **Stands_ctor** → renders automatically
- **Update** (vtable[0x0B/+0x2C] = 0x43DED0):
  - If `+0x10E4` flag set → position update via vtable, then clear flag
  - No path access, no external dependencies → SAFE for global spawn
- **Fields**: +0x10D0=board, +0x10D4=X, +0x10D8=Y, +0x10DC=Z, +0x10E0=CollisionLevel, +0x10E4=1(active)

### PILLAR (Platform)

The pillars that suddenly pop up and block your path in Sky Race and Sky Arena.

- **Constructor**: `Platform_ctor` (0x4363F0), alloc **0x10F4**
- **RET**: 0x14 (5 params: board, X, Y, Z, mesh)
- **vtable**: 0x4D56A8
- **Mesh**: `"levels\level9-popcylinder1"` (string at 0x4D0F5C)
- **Stands_ctor** → renders automatically
- **Update** (vtable[0x0B/+0x2C] = 0x436540):
  - State machine: 0=idle, 1=descending, 2=waiting, 3=ascending
  - Pops up/down using `Gfx_SetPosition` for vertical movement
  - Timer at +0x10F0, state at +0x10EC
  - Reads +0x10E4 (pos offset), +0x10E8 (target offset)
  - No path access → SAFE for global spawn
- **Fields**: +0x10D0=board, +0x10D4=X, +0x10D8=Y, +0x10DC=Z, +0x10E0=CollisionLevel, +0x10E4=45.0, +0x10E8=-1.0

**Original PILLAR behavior**: In the original Sky Race, PILLAR refs are collected into `board+0x4394` (AthenaList) during `Scene_SetupLevel9` (0x410830). The board's update loop then randomly pops them up using the `MeshNode("meshes\skypillar")` mesh from `board+0x436C`. Our global spawn uses Platform_ctor instead, which provides a similar popping-up behavior via the state machine.

### MAGNIFYER (Magnifying Glass)

The magnifying glass lens effect in Sky Race.

- **Constructor**: `CollisionLevel_Spatial_Ctor` (0x436250), alloc **0x444**
- **RET**: 0x10 (4 params: board, X, Y, Z) — **NO mesh parameter!**
- **vtable**: 0x4D569C (Pendulum variant)
- **NO Stands_ctor** — does NOT render a visible mesh!
- This is an **invisible collision sphere** (radius 90.0 at +0x24/+0x28)
- Creates a lens effect that magnifies the ball when it passes through
- **Gate**: Only created if `difficulty != 0` (App+0x23C)
- **Fields**: +0x04=board, +0x08=X, +0x0C=Y, +0x10=Z, +0x24=90.0, +0x28=90.0

## Factory Chain

```
BoardLevel_Sky_Ctor (0x41F930):
  LoadRaceData("SKYRACE")
  board+0x436C = MeshNode("meshes\skypillar")         → PILLAR mesh
  board+0x4370 = MeshNode("meshes\magnifyingglass")    → MAGNIFYER mesh
  board+0x4384 = MeshWorld("levels\level9-popcylinder1") → FUNBALL mesh
  board+0x4388 = MeshWorld("levels\level9-popcylinder2") → FUNBALL2 mesh
  board+0x438C = MeshWorld("levels\level9-trapdoor")    → TRAPDOOR mesh

Scene_SetupLevel9 (0x410830):
  "PILLAR"     → collected into board+0x4394 (AthenaList) for later popping
  "MAGNIFYER"  → CollisionLevel_Spatial_Ctor(alloc 0x444, board, X, Y, Z)
                 Stored at board+0x46AC. Gate: difficulty != 0.
  "CLOUDSCAPE" → position lookup via AthenaHashTable

Sky Factory Dispatch (0x410AD0, vtable[0x21]/+0x84):
  "POPCYLINDER" → Platform_ctor(alloc 0x10F4, board, X, Y, Z, board+0x4384)
  "TRAPDOOR"    → Rotator_ctor(alloc 0x10F4, board, X, Y, Z, board+0x438C)

CreateLevelObjects (0x4121D0):
  "POPCYLINDER" → PopCylinder_ctor(alloc 0x10E8, board, X, Y, Z, mesh)
```

## Mesh String Addresses

| Address | String |
|---------|--------|
| 0x4D0F5C | `levels\level9-popcylinder1` |
| 0x4D0F40 | `levels\level9-popcylinder2` |
| 0x4D0F90 | `meshes\skypillar` |
| 0x4D0F78 | `meshes\magnifyingglass` |
| 0x4D0F28 | `levels\level9-trapdoor` |

## String Table (0x4CFD88+)

| Address | String |
|---------|--------|
| 0x4CFD88 | CLOUDSCAPE |
| 0x4CFD94 | MAGNIFYER |
| 0x4CFDA0 | PILLAR |
| 0x4CFDA8 | levels\level9 |
| 0x4CFDB8 | POPCYLINDER |
| 0x4CFDC4 | E:HEATOFF |
| 0x4CFDD0 | E:HEATON |
| 0x4CFDDC | E:NOPEGS |
| 0x4CFDE8 | E:TRAPPOP |
| 0x4CFDF4 | E:PEGS |

## Object Field Layout

### FUNBALL (PopCylinder) — vtable 0x4D58F0

| Offset | Type | Description |
|--------|------|-------------|
| +0x000 | int* | vtable (0x4D58F0) |
| +0x10D0 | int | board pointer |
| +0x10D4 | float | position X |
| +0x10D8 | float | position Y |
| +0x10DC | float | position Z |
| +0x10E0 | void* | CollisionLevel pointer |
| +0x10E4 | byte | active flag (1 = active) |
| +0x10E5 | byte | unused flag (0) |

### PILLAR (Platform) — vtable 0x4D56A8

| Offset | Type | Description |
|--------|------|-------------|
| +0x000 | int* | vtable (0x4D56A8) |
| +0x10D0 | int | board pointer |
| +0x10D4 | float | position X |
| +0x10D8 | float | position Y |
| +0x10DC | float | position Z |
| +0x10E0 | void* | CollisionLevel pointer |
| +0x10E4 | float | position offset (45.0) |
| +0x10E8 | float | target offset (-1.0) |
| +0x10EC | int | state (0=idle, 1=descend, 2=wait, 3=ascend) |
| +0x10F0 | int | timer countdown |

### MAGNIFYER (CollisionLevel_Spatial) — vtable 0x4D569C

| Offset | Type | Description |
|--------|------|-------------|
| +0x000 | int* | vtable (0x4D569C) |
| +0x004 | int | board pointer |
| +0x008 | float | position X |
| +0x00C | float | position Y |
| +0x010 | float | position Z |
| +0x014 | float | secondary X (0.0) |
| +0x018 | float | secondary Y (0.0) |
| +0x01C | float | secondary Z (0.0) |
| +0x020 | int | flag (0) |
| +0x024 | float | collision radius X (90.0) |
| +0x028 | float | collision radius Z (90.0) |
| +0x02C | AthenaList | ball list |

## Vtable Comparison

### PopCylinder (FUNBALL) vtable 0x4D58F0

| Slot | Address | Description |
|------|---------|-------------|
| [00] | 0x43DEB0 | dtor |
| [01] | 0x4606D0 | render |
| [0B] | 0x43DED0 | update (flag-based position update) |

### Platform (PILLAR) vtable 0x4D56A8

| Slot | Address | Description |
|------|---------|-------------|
| [00] | 0x43D120 | dtor |
| [01] | 0x4606D0 | render |
| [0B] | 0x436540 | update (state machine: idle→descend→wait→ascend) |

### CollisionLevel_Spatial (MAGNIFYER) vtable 0x4D569C

| Slot | Address | Description |
|------|---------|-------------|
| [00] | 0x43CB50 | dtor |
| [01] | 0x43CB70 | render (no visible mesh) |
| [0B] | 0x471830 | update (collision sphere check) |
