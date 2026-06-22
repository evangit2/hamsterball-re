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
1. Add the address `SpawnFlag` to your CE address list (Advanced → Add Address, or type "SpawnFlag" in the address bar after enabling the script)
2. Enter a race or arena (any level works)
3. Set `SpawnFlag` to `1` — a SpeedCylinder spawns at Player 1's current position
4. The flag auto-resets to `0` after spawning
5. Set to `1` again to spawn another

## How It Works
The script hooks `Scene_UpdateBallsAndState` (0x41B540) which runs every frame. When `SpawnFlag` is set:
1. Checks if a SpeedCylinder mesh is already loaded (Board+0x4788, only set on Up Race)
2. If not, loads the mesh from `levels\levelup-speedcylinder` via `MeshWorld_ctor`
3. Finds Player 1's ball in the all_balls_list and reads its position
4. Allocates a Pendulum struct (0x150C bytes) and calls `Pendulum_ctor`
5. Registers the new SpeedCylinder in the Board's mechanical objects list (Board+0x2578)

## Verified Addresses (Ghidra, 2026-06-22)
| Address | Function | Convention |
|---------|----------|-----------|
| 0x41B540 | Scene_UpdateBallsAndState | `__thiscall` (ECX=Board) |
| 0x4BA57B | operator_new | `__cdecl` (RET) |
| 0x461510 | MeshWorld_ctor | `__thiscall` (RET 0x8, 2 params) |
| 0x436A20 | Pendulum_ctor | `__thiscall` (RET 0x18, 6 params) |
| 0x453810 | AthenaList_Append | `__thiscall` (RET 0x4, 1 param) |
| 0x4D1140 | "levels\levelup-speedcylinder" | String constant |
| 0x5341E0 | App global pointer | Runtime global |
