# Global Toob SAW / Fallout / Blockdawg Spawner

Spawns Toob Race SAWs, Fallout floors, and Blockdawgs at Player 1's position in any level.

## Usage

1. Set `ObjectType` (1-4):
   - 1 = SAW (spinning saw blade, mesh: Level8-Saw)
   - 2 = FALLOUT (fallout floor, mesh: Level8-Fallout, rotates at half speed)
   - 3 = BLOCKDAWG1 (block dog enemy 1, mesh: Level8-Blockdawg1)
   - 4 = BLOCKDAWG2 (block dog enemy 2, mesh: Level8-Blockdawg2)
2. Set `SpawnObject` to 1 to spawn at player position

## How Each Object Works

### SAW (Gear_ctor)
- **Constructor**: Gear_ctor (0x437590), alloc 0x1514, RET 0x20 (8 params)
- **Mesh**: `Levels\Level8-Saw` (string at 0x4D0E24, loaded into board+0x4370 by BoardLevel_Toob_Ctor)
- **Update**: vtable[0x0B/+0x2C] = Catapult_Update (0x43E600)
  - Decrements timer (+0x10F0) by rotation speed (+0x10F4 = 1.0)
  - Uses Gfx_ScaleY/X/Z to rotate the platform
  - Transforms ball positions when balls ride on the gear (matrix math)
  - Ball list at +0x10F8 (AthenaList, starts empty → safe)
- **No external dependencies**: Safe to spawn globally

### FALLOUT (Gear_ctor, different mesh + slower speed)
- **Same constructor as SAW** but with different mesh
- **Mesh**: `Levels\Level8-Fallout` (string at 0x4D0E0C, loaded into board+0x4374)
- **Difference from SAW**: +0x10F4 = 0x3F000000 (0.5 rotation speed instead of 1.0)
  - The game factory (CreateMechanicalObjects) sets this for BIGGEAR objects
- **No external dependencies**: Safe to spawn globally

### BLOCKDAWG1/2 (Blockdawg_ctor)
- **Constructor**: Blockdawg_ctor (0x43C310), alloc 0x1154, RET 0x18 (6 params)
- **Mesh**: `Levels\Level8-Blockdawg1/2` (strings at 0x4D0DF0/0x4D0DD4)
- **Update**: vtable[0x0B/+0x2C] = RumbleObject_Update (0x43C4E0)
  - If +0x1150 == 0 (awake): follows path via Path_GetPosition(pathId, ...)
    - **CRASHES if pathId=0 and no DAWGPATH ref in the level!**
  - If +0x1150 != 0 (sleeping): plays wake-up sound + creates particle ring
    - Safe, no path needed
- **FIX**: Script sets +0x1150 = 1 after construction → sleeping mode → no crash
- **BLOCKDAWG2 difference**: +0x1152 = 1 (is_blockdawg2 flag, set by game factory)

## Factory Chain

```
BoardLevel_Toob_Ctor (0x41F4B0):
  LoadRaceData("TOOBRACE")
  board+0x4370 = MeshWorld("Levels\Level8-Saw")
  board+0x4374 = MeshWorld("Levels\Level8-Fallout")
  board+0x4378 = MeshWorld("Levels\Level8-Blockdawg1")
  board+0x437C = MeshWorld("Levels\Level8-Blockdawg2")

Level loading processes MESHWORLD refs:
  CreateMechanicalObjects (0x417FE0):
    "GEAR" → Gear_ctor(..., board+0x4370)         [SAW]
    "BIGGEAR" → Gear_ctor(..., board+0x4374)      [FALLOUT, sets +0x10F4=0.5]
  CreateLevelObjects (0x4121D0):
    "BLOCKDAWG1" → Blockdawg_ctor(..., board+0x5840, pathId)
    "BLOCKDAWG2" → Blockdawg_ctor(..., board+0x5844, pathId), sets +0x1152=1
```

## Object Field Layout

### Gear (SAW/FALLOUT) — vtable 0x4D5AD0

| Offset | Type | Description |
|--------|------|-------------|
| +0x000 | int* | vtable (0x4D5AD0) |
| +0x10D0 | int | board pointer |
| +0x10D4 | void* | CollisionLevel pointer |
| +0x10D8 | float | position X |
| +0x10DC | float | position Y |
| +0x10E0 | float | position Z |
| +0x10E4 | float | X2 (rotation axis / second pos) |
| +0x10E8 | float | Y2 |
| +0x10EC | float | Z2 |
| +0x10F0 | int | rotation timer (decrements each frame) |
| +0x10F4 | float | rotation speed (1.0=normal, 0.5=slow/fallout) |
| +0x10F8 | AthenaList | ball list (balls riding on gear) |
| +0x1510 | byte | TOUCH flag (0=no touch trigger) |

### Blockdawg (BLOCKDAWG1/2) — vtable 0x4D5638

| Offset | Type | Description |
|--------|------|-------------|
| +0x000 | int* | vtable (0x4D5638) |
| +0x10D0 | int | board pointer |
| +0x10D4 | void* | CollisionLevel pointer |
| +0x10D8 | float | position X |
| +0x10DC | float | position Y |
| +0x10E0 | float | position Z |
| +0x10F0 | int | pathId (from Level_FindObjectByName) |
| +0x1138 | float | random rotation offset |
| +0x1150 | byte | sleeping flag (0=awake/follow path, 1=sleeping) |
| +0x1151 | byte | unused (0) |
| +0x1152 | byte | is_blockdawg2 flag (0=DAWG1, 1=DAWG2) |
