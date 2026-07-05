# always_active_mode

Keeps Hamsterball running when you tab out of fullscreen mode.

## What it does

Prevents the game from going black / stopping when you Alt-Tab or click outside the game window in fullscreen mode.

## How it works

Two byte patches in `bass.dll`:

1. **WM_ACTIVATEAPP handler** (0x46CD1A): Patches `setnz al` → `mov al, 1; nop` so the active flag is always 1, even when the window loses focus.

2. **App_Run render check** (0x46BF6F): Patches `jnz +0x39` → `nop nop` so the game always renders frames, bypassing the fullscreen+inactive check.

## Installation

1. Rename the original `bass.dll` in your Hamsterball folder to `bass_real.dll`
2. Copy this `bass.dll` into the game folder
3. Launch the game

## Compatibility

- Safe for Android/Wine (no threads, no IAT hooks, just byte patches)
- No gameplay changes — purely window management behavior
