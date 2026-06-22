# SpeedCylinder Global Spawner

## Overview
Spawns a SpeedCylinder (Pendulum) at Player 1's position in **any** level, not just Level 6 (Up). Pure CE AutoAssembler script — **no bass.dll proxy required**.

## Files
- `SpeedCylinderSpawn.CEA` — CE AutoAssembler script (pure CEA, no Lua)

## Installation
1. Copy `SpeedCylinderSpawn.CEA` to your Cheat Engine scripts folder
2. In CE: File → Load → select `SpeedCylinderSpawn.CEA`
3. Enable the script

## Usage
1. Add `SpawnFlag` to CE's address list:
   - Right-click address list → Add Address Manually
   - Description: `SpawnFlag`
   - Address: `SpawnFlag` (the registered symbol)
   - Type: 4 Bytes, Hex
2. Set a hotkey on SpawnFlag:
   - Right-click → Set/Change Hotkeys →
   - Action: Set value to → Value: `1`
3. During gameplay, press the hotkey to spawn a SpeedCylinder at Player 1's position

## How It Works
- Hooks `Graphics_PresentOrEnd` (0x455A90) per-frame via code cave
- When `SpawnFlag=1`:
  1. Loads `levels\levelup-speedcylinder` mesh via `MeshWorld_ctor` (cached after first spawn)
  2. Gets Player 1 ball position from `all_balls_list` (Scene+0x2DEC)
  3. `operator_new(0x150C)` → `Pendulum_ctor(mem, scene, x, y, z, 0, mesh)`
  4. `AthenaList_Append(Scene+0x2578, pendulum)`
  5. Resets `SpawnFlag` to 0

## Verified Addresses (from Ghidra decompilations)
| Address | Function | Calling Convention |
|---------|----------|-------------------|
| 0x005341E0 | App pointer (global) | — |
| App+0x178 | Scene pointer | — |
| App+0x174 | GraphicsDevice pointer | — |
| Scene+0x2578 | Objects AthenaList | — |
| Scene+0x2DEC | All balls AthenaList | — |
| Ball+0x164/168/16C | Position X/Y/Z (floats) | — |
| 0x004BA57B | operator_new | `__cdecl`, RET (caller cleans 4) |
| 0x00461510 | MeshWorld_ctor | `__thiscall`, 2 params, RET 0x08 |
| 0x00436A20 | Pendulum_ctor | `__thiscall`, 6 params, RET 0x18 |
| 0x00453780 | AthenaList_Append | `__thiscall`, 1 param, RET 0x04 |
| 0x00455A90 | Graphics_PresentOrEnd | Hook point (function entry) |

## Original Bytes at Hook Point
```
0x455A90: 8A 44 24 04 83 EC 20 3C 01
```
- `8A 44 24 04` = MOV AL, [ESP+4]
- `83 EC 20` = SUB ESP, 0x20
- `3C 01` = CMP AL, 1

## Version History
- **v2** — Pure CEA script, no Lua, no bass.dll required. All addresses verified from Ghidra decompilations.
- **v1** — Required bass.dll proxy with Lua for address resolution (deprecated).
