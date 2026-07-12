# Direction Detect Mod

Detects which direction the hamster ball is facing and displays it as an on-screen compass overlay.

## Features

- **On-screen compass**: Shows facing direction (N/NE/E/SE/S/SW/W/NW) and heading angle in degrees
- **Velocity display**: Shows current XZ velocity vector and speed magnitude
- **Engine facing fields**: Cross-references the engine's own `facing_direction` cos/sin fields at ball+0x194/0x198
- **Toggle**: Press **D** to show/hide the overlay
- **Log file**: Writes direction data to `direction_log.txt` in the game directory for debugging

## How It Works

The mod uses two mechanisms:

1. **Background polling thread** (~60fps): Finds Player 1's ball by scanning the Scene's ball list (Scene+0x29D4 AthenaList), then reads:
   - Ball velocity X/Z (ball+0x170/0x178) → computes heading via `atan2f(vz, vx)`
   - Ball speed (ball+0x188)
   - Engine's facing direction cos/sin (ball+0x194/0x198)

2. **Render hook**: Hooks `Graphics_PresentOrEnd` (0x455A90) via a code cave. Draws text **before** calling the original Present function — this is the correct timing (after viewport clear, before Present/EndScene).

## Compass Mapping

The Hamsterball coordinate system is Y-up, with X and Z as the horizontal plane:
- `atan2(vz, vx)` gives the heading angle
- 0° = East (+X), 90° = North (+Z), 180° = West (-X), 270° = South (-Z)

## Installation

1. Rename original `bass.dll` → `bass_real.dll` in the Hamsterball directory
2. Copy this `bass.dll` to the same directory
3. Launch `Hamsterball.exe`

## Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll direction_detect.c -lwinmm \
  -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc -Wl,--add-stdcall-alias -lm
```

## Controls

| Key | Action |
|-----|--------|
| D   | Toggle direction overlay on/off |

## Display

Three lines of text appear at top-center of the screen (x=360, y=20):

```
Facing: NE (47 deg)
Vel: (123, -45)  Spd: 131.0
Engine: cos=0.731 sin=0.682
```

## Struct Offsets Used

| Offset | Type | Field |
|--------|------|-------|
| ball+0x018 | int | player_index (0 = Player 1) |
| ball+0x164 | float | position X |
| ball+0x168 | float | position Y |
| ball+0x16C | float | position Z |
| ball+0x170 | float | velocity X |
| ball+0x174 | float | velocity Y |
| ball+0x178 | float | velocity Z |
| ball+0x188 | float | speed (velocity magnitude) |
| ball+0x194 | float | facing_direction cos (engine-computed) |
| ball+0x198 | float | facing_direction sin (engine-computed) |

## Architecture

- **Pattern 4** (volatile flag + polling thread): The background thread reads ball state and stores it in `volatile` globals. The render hook's code cave calls a C function (`draw_direction_overlay`) that reads these globals and draws text using `UI_DrawTextShadow_Wrapper` (0x409B90).
- The code cave at `Graphics_PresentOrEnd` (0x455A90) does: PUSHFD/PUSHAD → CALL draw_overlay → POPAD/POPFD → execute original 7 bytes → JMP back.
- Text is drawn using `UI_DrawTextShadow_Wrapper` (0x409B90, 15 params, `RET 0x3C`) with white text and black shadow for readability.
