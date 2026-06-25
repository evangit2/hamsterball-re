# Ball Tint Mod

Tints player 1's ball to any hex color, read from a text file at runtime.

## How It Works

The mod creates `ball_tint.txt` next to `bass.dll` on first launch. Edit the file with any hex color (e.g. `FF6B35` for orange) and the ball recolors within ~60ms — no restart needed.

**Mechanism:** Writes RGBA floats into the ball's render context material (ball+0x20C diffuse, +0x21C ambient, +0x23C emissive), then sets the Graphics material override (gfx+0x7C0 = ball+0x208) so the game uses our material instead of the mesh's default white material. This tints both the 3D sphere body and the sprite overlays (border, hamster).

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
