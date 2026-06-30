# FPS Unlock Standalone v2 — Fixed Rotation

## Problem
v1 set both `target_fps` (physics tick rate) and `render_fps` to 1000.
Ball_Update advances rotation by a **fixed increment per tick** (no dt scaling),
so running physics at 1000Hz made the ball spin 10x faster than normal.

## Fix (v2)
Only unlock the **render** frame rate cap. Leave physics at the original 100Hz.

| Setting | v1 | v2 |
|---------|----|----|
| App+0x16C (target_fps / physics) | 1000 | **100** (unchanged) |
| App+0x170 (render_fps) | 1000 | 1000 |
| JBE at 0x46BF55 (render-skip) | NOP'd | NOP'd |

## How It Works
The game loop in `App_Run` (0x46BD80) has two independent rates:

1. **Update tick** — `vtable[0x20]` → `App_FrameUpdate` → `GameUpdate` → `Ball_Update`
   - Runs at `target_fps` (App+0x16C, default 100Hz)
   - Advances ball physics, rotation, collision — all using fixed-delta increments
   - Ball_Update at 0x405E00 writes facing angle via `Math_Atan2Angle` and
     spin/roll via the spin friction loop (3 iterations per tick)

2. **Render frame** — `vtable[0x24/0x28/0x2C]` → `Graphics_PresentOrEnd`
   - Runs at `render_fps` (App+0x170, default 75Hz)
   - Reads ball position/rotation (does NOT modify them)
   - Ball_Render at 0x402DE0 calls Ball_GetTransform (0x40AF90) which
     reads ball+0x6C (facing angle) and ball+0xC0/C4 (spin)

By keeping target_fps=100, physics (including rotation) advances at the
correct speed. By unlocking render_fps and NOPing the render-skip JBE,
the game renders as fast as the GPU allows.

## Build
```bash
i686-w64-mingw32-gcc -shared -o bass.dll fps_unlock.c \
  -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
  -Wl,--add-stdcall-alias
```

## Installation
1. Rename original `bass.dll` to `bass_real.dll` in the Hamsterball directory
2. Copy this `bass.dll` (or `fps_unlock.dll`) to the same directory
3. Launch Hamsterball

## Crash Test
Passed via hbtestd (15.57s, no crash, process alive).
