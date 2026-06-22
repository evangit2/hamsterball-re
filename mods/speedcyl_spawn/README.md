# SpeedCylinder Spawner

Spawns SpeedCylinder objects globally in any race or arena at Player 1's position.

## What it does

- Hooks `Graphics_PresentOrEnd` (per-frame callback)
- When `g_spawn_flag` is set to 1 (via CE hotkey), spawns a SpeedCylinder Pendulum at Player 1's position
- Auto-resets the flag to 0 after spawning
- SpeedCylinder mesh is loaded once (cached) from `levels\levelup-speedcylinder`

## Installation

1. In your Hamsterball game folder, rename `bass.dll` → `bass_real.dll`
2. Copy `bass.dll` from this mod into the game folder
3. Launch game, attach Cheat Engine
4. Enable the CEA script → "spawnSpeed" appears in address list
5. Right-click `spawnSpeed` → Set/Change Hotkeys → Action: Set value to 1
6. Press hotkey during gameplay to spawn a SpeedCylinder

## Files

| File | Description |
|------|-------------|
| `speedcyl_spawn.c` | Source code (bass.dll proxy) |
| `bass_exports.def` | DLL export definitions |
| `bass.dll` | Compiled mod |
| `SpeedCylinderSpawn.CEA` | CE AutoAssembler script |

## How it works internally

1. On first spawn: loads MeshWorld via `MeshWorld_ctor(mem, graphics, "levels\\levelup-speedcylinder")` (0x461510)
2. On each spawn: allocates Pendulum via `operator_new(0x150C)` + `Pendulum_ctor(mem, scene, x, y, z, 0, mesh)` (0x436A20)
3. Adds to Scene object list via `AthenaList_Append(Scene+0x2578, pendulum)` (0x453780)
4. Player 1 position found by iterating `Scene+0x2DEC` (all_balls_list), matching `ball+0x018 == 0`

## Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll speedcyl_spawn.c bass_exports.def \
    -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc -Wl,--add-stdcall-alias
```
