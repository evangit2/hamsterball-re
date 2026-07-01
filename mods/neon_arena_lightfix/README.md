# neon_arena_lightfix

Fixes the missing yellow diffuse light effect on Neon Arena.

## Problem

`ArenaLevel_Neon_Init` (0x416F40) creates a yellow D3D point light (R=10, G=10, B=0) but:
1. Places it at `(0,0,0)` — world origin — and never moves it to follow the ball
2. `Scene_RegisterObject` calls `vtable[3]` only ONCE to submit the light to the D3D device. After that, position changes to the SceneObject struct are never re-submitted — the light stays frozen at its initial position forever.

In Neon Race, `Scene_SetupLevelDark` positions the emitter at the ball's location `(ball.x, ball.y+30, ball.z)`. Additionally, loop 1 forgets to set the glow flag (`+0xC80=1`) on P1 balls, and 8-balls spawn dynamically during gameplay.

## Fix (three parts)

1. **Glow flag**: Background thread sets `+0xC80=1` on all balls in both AthenaLists (board+0x29D4 for P1, board+0x2DEC for 8-balls). Re-applies every 100ms.

2. **Emitter position**: Background thread writes `(ball.x, ball.y+30, ball.z)` to `emitter+0x08/+0x0C/+0x10` every 100ms, following the P1 ball.

3. **D3D light re-submission**: After writing the position, calls `vtable[3]` (SceneObject_vtable3 at 0x46B670) via inline asm to re-submit the updated light to the D3D device. Without this, `SetLight` is never called again and the light stays frozen at `(0,0,0)`.

## Installation

1. Rename original `bass.dll` to `bass_real.dll`
2. Copy this `bass.dll` to the game folder

## Build

```
i686-w64-mingw32-gcc -shared -o bass.dll neon_arena_lightfix.c \
  -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
  -Wl,--add-stdcall-alias
```
