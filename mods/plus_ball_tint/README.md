# Ball Tint — Hamsterball Plus Mod

**Author:** Hamsterbot  
**Ported from:** bass.dll ball_tint v4  
**API:** Hamsterball Plus v1

## What It Does

Adds 6 RGB sliders to the options menu for customizing player ball colors:
- **P1 Red / Green / Blue** — Player 1's ball color (default: white 1.0/1.0/1.0)
- **P2 Red / Green / Blue** — Player 2's ball color (default: blue 0.0/0.5/1.0)

Colors apply in real-time — adjust sliders in the options menu and see changes immediately in-game.

## How It Works

- Uses `onGameUpdate()` to write RGBA floats to the board's player color table every frame
- Board found via `App → +0x220 (PlayerProfile) → +0x0C (Board)`
- Color table at `board+0x3AB0` (P1) and `board+0x3AC4` (P2) — 4 floats (R,G,B,A) per player, 0x14 stride
- `IsBadWritePtr` guards prevent crashes during level transitions

## Improvements over bass.dll v4

| Old (bass.dll proxy) | New (Plus API) |
|---|---|
| Required renaming bass.dll to bass_real.dll | Just drop in Mods folder |
| Background thread polling every 60ms | Uses onGameUpdate callback (no thread) |
| Config file (ball_tint.txt) with hex values | In-game sliders, no file editing |
| 5 colors hardcoded per player-count mode | 6 RGB sliders, adjust live |
| Player count detection logic needed | Colors always written for P1+P2 |

## Building

1. Open Visual Studio with Hamsterball Plus mod template
2. Replace `MainModFile.cpp` with `BallTint.cpp`
3. Build as DLL
4. Place `.dll` in game's `Mods\` folder

## Slider Reference

| Slider ID | Display | Default | Range |
|---|---|---|---|
| TINT_P1_R | P1 Red | 1.0 | 0.0-1.0 |
| TINT_P1_G | P1 Green | 1.0 | 0.0-1.0 |
| TINT_P1_B | P1 Blue | 1.0 | 0.0-1.0 |
| TINT_P2_R | P2 Red | 0.0 | 0.0-1.0 |
| TINT_P2_G | P2 Green | 0.5 | 0.0-1.0 |
| TINT_P2_B | P2 Blue | 1.0 | 0.0-1.0 |
