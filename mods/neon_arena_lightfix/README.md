# neon_arena_lightfix

Fixes the missing yellow diffuse light effect on Neon Arena.

## Problem

Neon Arena should have a yellow glow effect on balls similar to Neon Race, but it doesn't render. The root cause is in `ArenaLevel_Neon_Init` (0x416F40):

- **Loop 1** (P1 balls at board+0x29D4): writes yellow ambient/diffuse/emissive material colors, but **forgets to set `+0xC80 = 1`** (the glow render flag). Without this flag, the emissive material is written but the game never renders the glow.
- **Loop 2** (P2/badballs at board+0x2DEC): correctly sets `+0xC80 = 1` — which is why only P1 balls lack the glow.

Compare with `Scene_SetupLevelDark` (0x416270, Neon Race) where both loops set `+0xC80 = 1`.

## Fix

A background polling thread detects when Neon Arena is active (board+0x47E4 non-zero) and sets `+0xC80 = 1` on every ball in both AthenaLists. No material writes needed — the game already wrote the yellow materials; they just weren't being rendered.

## Installation

1. Rename original `bass.dll` to `bass_real.dll`
2. Copy this `bass.dll` to the game folder

## Build

```
i686-w64-mingw32-gcc -shared -o bass.dll neon_arena_lightfix.c \
  -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
  -Wl,--add-stdcall-alias
```
