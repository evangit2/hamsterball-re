# Global Blockdawg Spawner

Spawns a Blockdawg at player 1's position on any level. Based on the working Global Saw pattern.

## Usage
1. Enable the script in Cheat Engine
2. Set `[SpawnDawg]` to 1
3. Blockdawg spawns at player 1's position

## How It Works

### Blockdawg in Toob Race (traced from binary)

**Creation** (`FUN_0040FC29`):
```
operator_new(0x1154) → Blockdawg_ctor(0x43C310)
Mesh: "Levels\Level8-Blockdawg1" loaded via FUN_00461510
Path: DAWGPATH1 found via Level_FindObjectByName
Post: AthenaList_Append(board+0x2578, blockdawg)
```

**Blockdawg_ctor** (`0x43C310`):
- `Stands_ctor(this, mesh)` — same base as Saw
- Sets vtable = `0x4D5638`
- Stores board, position, path
- Creates mesh clone at `+0x10D4` via `FUN_00465080`
- Initializes fields (speed=1.0, Y offset=25.0, state=0)

**Blockdawg_Update** (`0x43C4E0` = `ArenaObject_Update`):
- State 0 (active):
  - Follows DAWGPATH (Path_GetPosition)
  - Spins blade (`Gfx_ScaleZ(3.0)`)
  - Checks ball proximity via `Math_FastDistance2D`
  - When ball within range → state = 1 (activate!)
- State 1 (falling):
  - Y offset decreases
  - Spawns particles
  - Plays sound

### Global Spawn (same approach as Global Saw v9)

| Component | Value |
|---|---|
| Object size | 0x1154 bytes |
| Mesh | `Levels\Level8-Blockdawg1` (fresh load) |
| Path | Dummy 1-vertex (no movement) |
| Direction init | (0, 0, 1.0) — prevents render corruption |
| Post-processing | SpeedCylinder pattern (SpatialTree, render, collision) |

### Field Layout (byte offsets)

| Offset | Field | Type |
|---|---|---|
| +0x10D0 | board | ptr |
| +0x10D4 | mesh clone | ptr |
| +0x10D8 | X position | float |
| +0x10DC | Y position | float |
| +0x10E0 | Z position | float |
| +0x10E4 | dir X | float |
| +0x10E8 | dir Y | float |
| +0x10EC | dir Z | float |
| +0x10F0 | path object | ptr |
| +0x10F4 | Timer | ptr |
| +0x1138 | path counter | float |
| +0x113C | speed | float (1.0) |
| +0x1144 | sound toggle | byte |
| +0x1148 | sound timer | int |
| +0x114C | Y offset | float (25.0) |
| +0x1150 | state | byte (0=active, 1=falling) |
| +0x1151 | particle flag | byte |
| +0x1152 | blockdawg3 flag | byte |
