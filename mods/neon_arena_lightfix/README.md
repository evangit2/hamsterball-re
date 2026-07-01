# neon_arena_lightfix

Fixes the missing yellow diffuse light effect on Neon Arena.

## Problem

Neon Arena should have a yellow glow effect on balls similar to Neon Race, but it doesn't render. The root cause is two bugs in `ArenaLevel_Neon_Init` (0x416F40) compared to `Scene_SetupLevelDark` (0x416270, Neon Race):

1. **Missing glow flag**: The first loop iterates P1 balls (board+0x29D4 AthenaList) and writes ambient/diffuse/emissive material colors, but forgets to set `+0xC80 = 1` (the glow render flag). Without this flag, the emissive material is written but never rendered. The second loop (P2/badballs at board+0x2DEC) correctly sets `+0xC80 = 1`, which is why only P1 balls lack the glow.

2. **Missing phys struct writes**: After the ball loops, `Scene_SetupLevelDark` writes 3 material blocks directly to the global phys struct (`App+0x5DC` for P1, `App+0x67C` for P2). `ArenaLevel_Neon_Init` omits these writes entirely.

## Fix

A background polling thread detects when Neon Arena is active (board+0x47E4 non-zero — the emitter SceneObject pointer set during init) and:

- Sets `+0xC80 = 1` (glow flag) on every ball in the P1 AthenaList
- Writes yellow material colors (R=1.0, G=1.0, B=0.0, A=1.0) to ambient, diffuse, and emissive on each ball
- Also writes to `App+0x5DC` and `App+0x67C` phys structs as belt-and-suspenders
- Re-applies every 2 seconds while in Neon Arena

## Installation

1. Rename original `bass.dll` to `bass_real.dll`
2. Copy this `bass.dll` to the game folder

## Build

```
i686-w64-mingw32-gcc -shared -o bass.dll neon_arena_lightfix.c \
  -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
  -Wl,--add-stdcall-alias
```
