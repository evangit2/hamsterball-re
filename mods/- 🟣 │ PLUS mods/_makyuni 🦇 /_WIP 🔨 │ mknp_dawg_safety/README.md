# mkn_plus_dawg_safety

Force sleeping mode on pathless Blockdawgs to prevent crashes.

## What It Does

When a Blockdawg entity has no DAWGPATH to follow (path pointer at +0x10F0 is NULL), `ArenaObject_Update` (0x43C4E0) crashes trying to dereference it. This mod detours the update function and sets `+0x1150 = 1` (sleeping state) before the crash can happen.

## How It Works

- **Hook target**: `ArenaObject_Update` (0x0043C4E0) — called every frame for Blockdawg objects
- **Check**: If vtable matches Blockdawg (0x4D5638) AND path (+0x10F0) is NULL AND state (+0x1150) is 0 (active)
- **Action**: Sets state to 1 (sleeping) — Blockdawg stays in place playing idle animation
- **Pass-through**: Calls original update function after the safety check

## Installation

1. Copy `bass.dll` and rename your original `bass.dll` to `bass_real.dll`
2. Place this `bass.dll` in the Hamsterball game folder

## Technical Details

| Field | Value |
|-------|-------|
| Object size | 0x1154 bytes |
| Vtable | 0x4D5638 (RumbleObject) |
| Path pointer | +0x10F0 |
| State byte | +0x1150 (0=active, 1=sleeping) |
| Hook type | Detour (7-byte trampoline) |
| Hook address | 0x0043C4E0 |

## Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll mkn_plus_dawg_safety.c \
  bass.def -O2 -static -static-libgcc \
  -Wl,--enable-stdcall-fixup -Wl,--add-stdcall-alias -msse2 -mfpmath=sse
```

Author: Hamsterbot for MAKYUNI 🦇
