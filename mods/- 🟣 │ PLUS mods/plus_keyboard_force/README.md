# Keyboard Force

HB+ v2.1 mod that adds a slider to adjust keyboard and controller input force.

## What It Does

The game uses different force scaling for mouse vs keyboard/controller:

- **Mouse:** `sensitivity × 0.16 + 0.1` (ranges from 0.10 to 0.26)
- **Keyboard/Controller:** Fixed at 0.12

This mod lets you adjust the keyboard/controller force scale via an in-game
slider. Mouse input is completely unaffected.

## Slider Settings

| Setting | Value |
|---------|-------|
| Default | 0.12 (original game value) |
| Min | 0.10 (matches mouse at 0% sensitivity) |
| Max | 0.26 (matches mouse at 100% sensitivity) |
| Step | 0.01 |

## How It Works

Patches the float at `0x4D03B8` (KeyboardForceScale) in Hamsterball.exe every
frame via `onGameUpdate()`. This value is read by `Board_GetInputForce3D`
(0x41A9A0) only when the active input handler is NOT mouse (mode ≠ 2).

The mouse path ignores this value entirely and uses
`App+0x84C (sensitivity) × 0.16 + 0.1` instead.

## Installation

1. Place `plus_keyboard_force.dll` in the game's `Mods\` folder
2. Launch the game
3. Find "Keyboard/Controller Force" in the HB+ options menu

## Source

- `KeyboardForce.cpp` — Visual Studio (MSVC) source, inherits from HamsterballAPI
- `KeyboardForce_MinGW.cpp` — MinGW cross-compile source, manual 17-entry vtable
- `nocrt.h/cpp` — Minimal CRT (MinGW only)
- `hbplus_api.h` — Manual IModAPI dispatch (MinGW only)
- `build.sh` — MinGW build script

## Author

BookwormKevin · Hamsterbot
