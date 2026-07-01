# neon_arena_lightfix

Fixes the missing yellow diffuse light effect on Neon Arena.

## Problem

The ball glow render path at 0x402F0E in `Ball_Render` hardcodes **white** emissive (1,1,1) via three `PUSH 0x3F800000` calls to `Gfx_PackColorRGB`. It ignores the ball's material colors entirely. In Neon Race, the ball appears yellow because of the emitter D3D light positioned at the ball's location. In Neon Arena:

1. `ArenaLevel_Neon_Init` loop 1 forgets to set `+0xC80 = 1` (glow flag) on P1 balls
2. 8-balls spawn dynamically during gameplay after init has run — they never get the glow flag

## Fix (two parts)

**1. Byte-patch glow emissive B-channel** (0x402F51): Change `PUSH 0x3F800000` (1.0f) to `PUSH 0x00000000` (0.0f), making the hardcoded emissive yellow (R=1, G=1, B=0) instead of white (R=1, G=1, B=1). This affects all glowing balls — both Neon Race and Neon Arena.

**2. Background polling thread**: Detects Neon Arena (board+0x47E4 non-zero) and sets `+0xC80 = 1` on all balls in both AthenaLists (board+0x29D4 for P1, board+0x2DEC for 8-balls). Re-applies every 500ms to catch dynamically spawned 8-balls.

## Installation

1. Rename original `bass.dll` to `bass_real.dll`
2. Copy this `bass.dll` to the game folder

## Build

```
i686-w64-mingw32-gcc -shared -o bass.dll neon_arena_lightfix.c \
  -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
  -Wl,--add-stdcall-alias
```
