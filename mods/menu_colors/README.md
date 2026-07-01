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
```

- **Off** = dark/unhovered button state
- **On** = highlighted/hovered button state
- Colors are hex RGB: `#RRGGBB` (e.g. `#FF0000` = red, `#00FF00` = green)

## How It Works

The mod patches two `Vec3_Init` call sites in the title screen renderer (`FUN_0042d270`) with code caves that read from global float variables updated by the config reader thread.

### Addresses

| Element | State | Address | Original Value |
|---------|-------|---------|----------------|
| "Click here" button | Off (dark) | `0x0042d5fd` | Blue=1.0, Green=0.0, Red=0.0 |
| "Click here" button | On (highlighted) | `0x0042d624` | Blue=0.5, Green=0.0, Red=0.0 |

### Code Cave Structure

Each original 9-byte PUSH sequence (3 consecutive PUSH instructions for R/G/B) is replaced with:
- 5-byte `JMP` to code cave
- 4× `NOP` padding

The code cave pushes 3 float values from global variables (`PUSH dword ptr [addr]`) then jumps back.

## Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll menu_colors.c -lwinmm \
  -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
  -Wl,--add-stdcall-alias
```

## Current Sections

- **LOADING SCREEN** — "Click here" button Off/On colors ✅
- MAIN MENU — soon
- TOURNEY MENU — soon
- MIRROR MENU — soon
- TIME TRIALS MENU — soon
- 2P MENU — soon
- 4P MENU — soon
- OTHER — soon
