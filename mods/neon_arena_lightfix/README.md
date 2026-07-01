# neon_arena_lightfix

Fixes the missing yellow diffuse light effect on Neon Arena.

## Problem

`ArenaLevel_Neon_Init` (0x416F40) creates a yellow D3D emitter light (Vec3 R=10, G=10, B=0) but places it at `(0,0,0)` — world origin — and never moves it to follow the ball. In Neon Race, `Scene_SetupLevelDark` positions the emitter at the ball's location `(ball.x, ball.y+30, ball.z)`. Additionally, loop 1 forgets to set the glow flag (`+0xC80=1`) on P1 balls, and 8-balls spawn dynamically during gameplay so they never get the flag.

## Fix (two parts)

1. **Glow flag**: Background thread sets `+0xC80=1` on all balls in both AthenaLists (board+0x29D4 for P1, board+0x2DEC for 8-balls). Re-applies every 100ms to catch dynamically spawned 8-balls.

2. **Emitter follow**: Background thread continuously moves the emitter (board+0x47E4) to the P1 ball's position: writes `(ball.x, ball.y+30, ball.z)` to `emitter+0x08/+0x0C/+0x10`. This casts the yellow D3D point light onto the ball and surrounding floor, making the checker texture visible.

## Installation

1. Rename original `bass.dll` to `bass_real.dll`
2. Copy this `bass.dll` to the game folder

## Build

```
i686-w64-mingw32-gcc -shared -o bass.dll neon_arena_lightfix.c \
  -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
  -Wl,--add-stdcall-alias
```
