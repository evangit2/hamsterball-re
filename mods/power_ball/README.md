# Power Ball Mod (bass.dll proxy)

Makes the player ball stronger than 8-Ball and Fun Ball.

## What it does

- **Radius** increased to 55.0 (player default 26.0, 8-ball 35.0, fun ball 26.0)
- **Max speed** increased to 10.0 (default 6.0)
- Runs automatically — no buttons needed

### Why 55.0 radius?

Knockout formula from `Ball_Update` (0x405E00): `if (otherRadius < myRadius * 0.7) → knockout`

| Ball | Radius | Can KO player? | Player can KO? |
|------|--------|----------------|----------------|
| Player (modded) | 55.0 | — | ✓ everyone |
| 8-Ball | 35.0 | 24.5 < 55.0 → NO | 38.5 > 35.0 → YES |
| Fun Ball | 26.0 | 18.2 < 55.0 → NO | YES |

## Install

1. Rename original `bass.dll` → `bass_real.dll` in the game folder
2. Copy this `bass.dll` into the game folder
3. Launch the game!

## Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll power_ball.c \
  -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
  -Wl,--add-stdcall-alias
```
