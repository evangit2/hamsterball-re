# Global Sky Race Object Spawner — FUNBALL, PILLAR, MAGNIFYER

Spawns Sky Race objects at Player 1's position in any level via hotkey.

## Usage

1. Set `ObjectType` (1-3):
   - 1 = FUNBALL (PopCylinder — blue ball that pops up near goal)
   - 2 = PILLAR (Platform — pillar that pops up and blocks path)
   - 3 = MAGNIFYER (CollisionLevel_Spatial — magnifying glass lens effect)
2. Set `SpawnObject` to 1 to spawn at player position

## Object Details

### FUNBALL (PopCylinder)

The blue ball that pops up near the goal in Sky Race.

- **Constructor**: `PopCylinder_ctor` (0x436EE0), alloc **0x10E8**
- **RET**: 0x14 (5 params: board, X, Y, Z, mesh)
- **vtable**: 0x4D58F0
- **Mesh**: `"levels\level9-popcylinder1"` (string at 0x4D0F5C)
- **Stands_ctor** → renders automatically
- **Update** (vtable[0x0B/+0x2C] = 0x43DED0): if `+0x10E4` flag set → position update, then clear flag. SAFE.
- **Fields**: +0x10D0=board, +0x10D4=X, +0x10D8=Y, +0x10DC=Z, +0x10E0=CollisionLevel, +0x10E4=1(active)

**Note on "Meshes\FunBall"**: The string `"Meshes\FunBall"` (0x4D3474) is the ball sphere MESH MODEL loaded by the resource loader (App_ResourceLoader at 0x4298C0) into `App+0x26C` as a MeshNode (0x18 bytes). This is NOT a level object — it's the ball's sphere model used for rendering. It's loaded alongside Sphere (0x244), 8Ball (0x268), Bell (0x270), and Dizzy (0x274). The visible "funball" object in Sky Race that pops up near the goal is actually a **POPCYLINDER** created from the MESHWORLD file `"levels\level9-popcylinder1"`.

### PILLAR (Platform)

The pillars that suddenly pop up and block your path in Sky Race and Sky Arena.

- **Constructor**: `Platform_ctor` (0x4363F0), alloc **0x10F4**
- **RET**: 0x14 (5 params: board, X, Y, Z, mesh)
- **vtable**: 0x4D56A8
- **Mesh**: `"levels\level9-popcylinder1"` (string at 0x4D0F5C)
- **Stands_ctor** → renders automatically
- **Update** (vtable[0x0B/+0x2C] = 0x436540): state machine (0=idle, 1=descending, 2=waiting, 3=ascending)
  - Pops up/down using `Gfx_SetPosition` for vertical movement
  - Timer at +0x10F0, state at +0x10EC. SAFE.
- **Fields**: +0x10D0=board, +0x10D4=X, +0x10D8=Y, +0x10DC=Z, +0x10E0=CollisionLevel, +0x10E4=45.0, +0x10E8=-1.0

### MAGNIFYER (Magnifying Glass)

The magnifying glass lens effect in Sky Race.

- **Constructor**: `CollisionLevel_Spatial_Ctor` (0x436250), alloc **0x444**
- **RET**: 0x10 (4 params: board, X, Y, Z) — **NO mesh parameter!**
- **vtable**: 0x4D569C (Pendulum variant)
- **INVISIBLE** — creates a collision sphere (radius 90.0), NOT a visible mesh
- Creates a lens effect that magnifies the ball when it passes through
- **Gate**: Only created if `difficulty != 0` (App+0x23C)
- **Fields**: +0x04=board, +0x08=X, +0x0C=Y, +0x10=Z, +0x24=90.0, +0x28=90.0

## Factory Chain

```
BoardLevel_Sky_Ctor (0x41F930):
  LoadRaceData("SKYRACE")
  board+0x436C = MeshNode("meshes\skypillar")
  board+0x4370 = MeshNode("meshes\magnifyingglass")
  board+0x4384 = MeshWorld("levels\level9-popcylinder1")
  board+0x4388 = MeshWorld("levels\level9-popcylinder2")
  board+0x438C = MeshWorld("levels\level9-trapdoor")

Scene_SetupLevel9 (0x410830):
  "PILLAR"     → collected into board+0x4394 (AthenaList) for later popping
  "MAGNIFYER"  → CollisionLevel_Spatial_Ctor → board+0x46AC (difficulty gate)
  "CLOUDSCAPE" → position lookup via AthenaHashTable

Sky Factory (0x410AD0, vtable[0x21]):
  "POPCYLINDER" → Platform_ctor with board+0x4384 mesh
  "TRAPDOOR"    → Rotator_ctor with board+0x438C mesh

CreateLevelObjects (0x4121D0):
  "POPCYLINDER" → PopCylinder_ctor with mesh param
```

## Mesh String Addresses

| Address | String | Usage |
|---------|--------|-------|
| 0x4D0F5C | `levels\level9-popcylinder1` | FUNBALL mesh (MESHWORLD) |
| 0x4D0F40 | `levels\level9-popcylinder2` | FUNBALL2 mesh (MESHWORLD) |
| 0x4D0F90 | `meshes\skypillar` | PILLAR mesh (MeshNode) |
| 0x4D0F78 | `meshes\magnifyingglass` | MAGNIFYER mesh (MeshNode) |
| 0x4D0F28 | `levels\level9-trapdoor` | TRAPDOOR mesh (MESHWORLD) |
| 0x4D3474 | `Meshes\FunBall` | Ball sphere model (MeshNode, App+0x26C) |

## Registration Pattern (same as SpeedCylinder script)

1. Create MeshWorld via `MeshWorld_ctor` (alloc 0x10D0)
2. Store mesh in board slot `board+0x4788`
3. Call factory: `CreateSpeedCylinder` (0x4117B0) for FUNBALL
4. Register in `board+0xCD4` (post-alpha render list)
5. Register in `board+0x10EC` (collision list)
6. Call `vtable[0x16]` (update) and `vtable[0x15]` (render) on new object
