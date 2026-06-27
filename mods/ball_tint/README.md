# Ball Tint Mod

Tints player 1's ball to any hex color, read from a text file at runtime.

## How It Works

The mod creates `ball_tint.txt` next to `bass.dll` on first launch. Edit the file with any hex color (e.g. `FF6B35` for orange) and the ball recolors within ~60ms — no restart needed.

**Mechanism (v2):** Writes RGBA floats directly into the board's player ball color table at `board+0x3AB0`. These are the same color entries initialized by `Board_ctor` (0x419030) via four `Vec3_Init` calls — one per player:

| Board Offset | Player | Default Color |
|---|---|---|
| +0x3AB0 | Player 1 | (1.0, 1.0, 1.0) white |
| +0x3AC4 | Player 2 | (0.0, 0.5, 1.0) light blue |
| +0x3AD8 | Player 3 | (1.0, 0.25, 0.25) salmon |
| +0x3AEC | Player 4 | (1.0, 1.0, 0.0) yellow |

Each entry is 4 floats (R, G, B, A) = 16 bytes, spaced 0x14 bytes apart.

This is a much cleaner approach than v1: instead of writing to the ball's per-ball material fields and setting a gfx override, we simply overwrite the board-level color value that the game's own rendering pipeline reads when drawing each player's ball.

The board is found via `App+0x220 → PlayerProfile+0xC → board`, with a fallback scanner that looks for a valid AthenaList at `+0x29D4`.

## Files

- `bass.dll` — the mod (replace game's bass.dll)
- `ball_tint.txt` — auto-created config file
- `ball_tint.c` — source code

## Usage

1. Backup your original `bass.dll`
2. Copy `bass.dll` to the game folder
3. Make sure `bass_real.dll` (the original renamed) is in the same folder
4. Launch the game — `ball_tint.txt` is created automatically
5. Edit `ball_tint.txt` with a hex color (e.g. `4A90D9` for blue)
6. Save the file — ball recolors instantly

## Config Format

```
FFFFFF
# Ball Tint Color (hex RGB, no alpha)
# Examples: FF6B35 (orange), 4A90D9 (blue), 2ECC71 (green)
# Lines starting with # are ignored
```

Supported formats: `FF6B35`, `#FF6B35`, `0xFF6B35` (case-insensitive).

## Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll ball_tint.c -lwinmm \
  -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
  -Wl,--add-stdcall-alias
```

## Tested

- ✅ Crash test: 35s on Wine/Xvfb, process alive
- ⚠️ Visual testing on real Windows required (Wine/llvmpipe renders black)
