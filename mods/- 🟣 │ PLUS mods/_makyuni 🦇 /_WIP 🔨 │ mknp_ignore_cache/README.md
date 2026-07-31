# mknp_ignore_cache

Disables the game's `.cached` mesh cache system.

## What it does

The game has a master cache flag stored as a byte at **Graphics + 0x7D1**. When this flag is `1`, the game will load existing `.cached` files and write new `.cached` files after building meshes. When it is `0`, the game never loads or creates `.cached` files.

This mod forces that flag to stay `0` for the entire session.

## How it works

1. **Patch the initial enable**: In `App_Initialize_Full` at `0x42956A` the game executes:
   ```
   mov byte ptr [eax+0x7D1], 1
   ```
   The mod patches the immediate value from `1` to `0`, so the flag is never turned on.

2. **Frame-by-frame enforcement**: A `Graphics_PresentOrEnd` hook also writes the flag to `0` every frame, overriding any other code path that might set it.

## Files

- `mknp_ignore_cache.dll` — the compiled mod
- `source/mknp_ignore_cache.c` — source code
- `source/bass_proxy.h` — shared mknp_ignore_cache.dll proxy boilerplate
- `README.md` — this file

## Install

Place `mknp_ignore_cache.dll` next to `Hamsterball.exe`. Rename the original `mknp_ignore_cache.dll` to `bass_real.dll` first.
