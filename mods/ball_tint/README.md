# Ball Tint Mod (v3)

Tints Player 1's ball to a custom hex color. Supports **three separate color settings** for different player counts.

## Installation

1. Rename your original `bass.dll` to `bass_real.dll`
2. Copy the modded `bass.dll` into your Hamsterball game folder
3. Run the game — `ball_tint.txt` is auto-created next to the DLL on first launch

## Configuration

Edit `ball_tint.txt` (next to `bass.dll`) with any text editor:

```
FFFFFF
4A90D9
2ECC71
```

| Line | Setting                           | Example |
|------|-----------------------------------|---------|
| 1    | Player 1 color in **1-player** mode  | `FFFFFF` (white) |
| 2    | Player 1 color in **2-player** mode  | `4A90D9` (blue) |
| 3    | Player 1 color in **4-player** mode  | `2ECC71` (green) |

- Hex RGB format: `RRGGBB` (e.g. `FF6B35` = orange)
- `#RRGGBB`, `0xRRGGBB`, and 3-digit shorthand (`F63`) also accepted
- Lines starting with `#` are comments
- Invalid/missing values default to white (`FFFFFF`)
- File is re-read every 60ms — change colors at runtime without restarting

## How It Works

The mod detects the active player count by scanning the App struct's player data slots (`App+0x5CC`, stride `0xA0`). Each slot's active flag at `+0x0B` indicates whether that player is participating. The mod then selects the matching color from the config file and writes it into the board's player ball color table at `board+0x3AB0` (4 floats: R, G, B, A).

## Files

- `bass.dll` — proxy DLL (rename original to `bass_real.dll`)
- `ball_tint.c` — source code
- `ball_tint.txt` — color configuration

## Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll ball_tint.c -lwinmm \
  -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
  -Wl,--add-stdcall-alias
```
