# neon_arena_lightfix

Fixes the missing yellow diffuse light effect on Neon Arena.

## Problem

Neon Arena should have a yellow glow effect on balls (like Neon Race), but it doesn't work. Two bugs in `ArenaLevel_Neon_Init` (0x416F40):

1. **Missing glow flag**: Loop 1 (P1 balls) writes yellow material colors but forgets `+0xC80 = 1` (glow render flag). Loop 2 (8-balls) sets it correctly.
2. **Dynamic 8-ball spawns**: 8-balls are created during gameplay, AFTER `ArenaLevel_Neon_Init` has already run. New 8-balls never receive neon materials at all.

## Fix

A background polling thread detects Neon Arena (board+0x47E4 non-zero) and continuously applies:
- Glow flag (`+0xC80 = 1`)
- Yellow ambient/diffuse/emissive materials (R=1.0, G=1.0, B=0.0, A=1.0)

to ALL balls in both AthenaLists (board+0x29D4 for P1, board+0x2DEC for 8-balls). Re-applies every 500ms to catch dynamically spawned 8-balls.

No phys struct pointer dereferencing — only direct ball struct writes with vtable verification (0x4CF3A0).

## Installation

1. Rename original `bass.dll` to `bass_real.dll`
2. Copy this `bass.dll` to the game folder

## Build

```
i686-w64-mingw32-gcc -shared -o bass.dll neon_arena_lightfix.c \
  -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
  -Wl,--add-stdcall-alias
```
