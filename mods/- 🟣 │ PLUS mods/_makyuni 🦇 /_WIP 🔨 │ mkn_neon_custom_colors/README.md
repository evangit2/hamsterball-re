# Neon Custom Colors

Customizable Neon Race RGB colors from a **`.txt` config file**.  
Change the player ball outline and emitter glow to any color — for both P1 and P2!

## How it works

A detour hook on `Scene_SetupLevelDark` (0x00416270) intercepts the Neon level setup. After the original function writes the default yellow colors to the phys/emitter structs, the hook overwrites the R/G/B values with values read from `neon_colors.txt`. This approach bypasses the 2-byte `PUSH imm8` limitation that prevents patching the B channel via static byte patches.

## Config file (`neon_colors.txt`)

```
NEON PLAYER1 OUTLINE:
* R = 1.0
* G = 1.0
* B = 0.0

NEON PLAYER1 GLOW:
* R = 10.0
* G = 10.0
* B = 0.0

NEON PLAYER2 OUTLINE:
* R = 1.0
* G = 1.0
* B = 0.0

NEON PLAYER2 GLOW:
* R = 10.0
* G = 10.0
* B = 0.0
```

- **OUTLINE** = material colors (applied to Diffuse, Ambient, and Emissive)
- **GLOW** = emitter light Diffuse color
- Values are **floats** (e.g. `1.0`, `0.0`, `10.0`, `0.5`, `2.0`)
- If `neon_colors.txt` is missing, a default one is **auto-generated** with original yellow values
- Config is re-read every time the Neon level loads (edit and replay to see changes!)

### Color examples

| Color | R | G | B |
|-------|------|------|------|
| Yellow (default) | 1.0 | 1.0 | 0.0 |
| Green | 0.0 | 1.0 | 0.0 |
| Red | 1.0 | 0.0 | 0.0 |
| Blue | 0.0 | 0.0 | 1.0 |
| White | 1.0 | 1.0 | 1.0 |
| Magenta | 1.0 | 0.0 | 1.0 |
| Cyan | 0.0 | 1.0 | 1.0 |

For glow, use higher values (10.0 = default brightness, 20.0 = brighter, 5.0 = dimmer).

## Installation

1. Rename your original `bass.dll` to `bass_real.dll`
2. Copy the modded `bass.dll` and `neon_colors.txt` into the game folder
3. Launch Hamsterball — the mod auto-generates a config file if missing
4. Edit `neon_colors.txt` to set your desired colors, then restart the level

## Technical details

| Component | Value |
|-----------|-------|
| Hook target | `Scene_SetupLevelDark` (0x00416270) |
| Hook type | JMP detour (7-byte prologue → trampoline) |
| P1 phys | App+0x5DC (materials: +0x1BC/+0x1CC/+0x1EC) |
| P2 phys | App+0x67C (same offsets) |
| P1 emitter | Scene+0x436C (color: +0x94) |
| P2 emitter | Scene+0x4370 (color: +0x94) |
| Mesh nodes | Scene+0x2DEC (AthenaList of neon platform/border meshes) |

## Crash test

Tested via hbtestd on Wine/Xvfb: 38.6s runtime, no crash, verdict OK.
