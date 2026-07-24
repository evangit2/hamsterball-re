# Self-Destruct Mod

**The most pointless Hamsterball mod ever created.**

## What It Does

A few seconds after the game opens, it automatically closes the game.

That's it. That's the whole mod.

## How It Works

- A background thread sleeps for 3 seconds
- Then sets `App+0x159 = 1` (the quit flag)
- This breaks `App_Run`'s main loop
- Control returns to `WinMain`, which calls `App_Dtor` → `DestroyWindow` → clean exit
- The game closes with exit code 0

## Installation

1. Rename your original `bass.dll` to `bass_real.dll`
2. Put this `bass.dll` in the game folder
3. Open the game
4. Enjoy the 3 seconds you get

## Configuration

Edit `#define MIN_DELAY_MS` and `#define MAX_DELAY_MS` in `mod.c` and recompile to change the random delay range. Defaults: 500ms-3000ms.

## Why?

Because we can.
