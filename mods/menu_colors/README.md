# Menu Colors Mod

Reads `menu_colors.txt` and applies custom colors to menu UI elements in Hamsterball.

## Installation

1. Rename original `bass.dll` to `bass_real.dll`
2. Copy this `bass.dll` + `menu_colors.txt` to the game folder
3. Edit `menu_colors.txt` to change colors at runtime (mod re-reads every ~2 seconds)

## Config Format

```
- LOADING SCREEN -
"Click here" button - Off = #FFFFFF
"Click here" button - On = #FFFFFF

- MAIN MENU -
HBversion = #FFFFFF, 1.0
```

- **Off** = dark/unhovered button state
- **On** = highlighted/hovered button state
- **HBversion** = "HB" version text color + alpha
- RGB colors are hex: `#RRGGBB` (e.g. `#FF0000` = red, `#00FF00` = green)
- Alpha is a float: `1.0` = opaque, `0.5` = semi-transparent, `0.0` = invisible

## How It Works

The mod patches `Vec3_Init` and `Matrix_Scale4x4` call sites with code caves that read from global float variables updated by the config reader thread.

### Addresses

| Element | State | Address | Original Value | Args |
|---------|-------|---------|----------------|------|
| "Click here" button | Off (dark) | `0x0042d5fd` | B=1.0, G=0.0, R=0.0 | RGB (3) |
| "Click here" button | On (highlighted) | `0x0042d624` | B=0.5, G=0.0, R=0.0 | RGB (3) |
| HB version text | — | `0x00426433` | A=1.0, B=0.0, G=1.0, R=0.0 | RGBA (4) |

### Code Cave Structure

Each original PUSH sequence is replaced with:
- 5-byte `JMP` to code cave
- NOP padding to fill original instruction size

The code cave pushes float values from global variables (`PUSH dword ptr [addr]`) then jumps back.

- 3-arg cave (RGB): 3× `PUSH [mem]` + `JMP` = 23 bytes
- 4-arg cave (RGBA): 4× `PUSH [mem]` + `JMP` = 29 bytes

## Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll menu_colors.c -lwinmm \
  -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
  -Wl,--add-stdcall-alias
```

## Current Sections

- **LOADING SCREEN** — "Click here" button Off/On colors ✅
- **MAIN MENU** — HB version text color + alpha ✅
- TOURNEY MENU — soon
- MIRROR MENU — soon
- TIME TRIALS MENU — soon
- 2P MENU — soon
- 4P MENU — soon
- OTHER — soon
