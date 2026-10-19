# mknp_player_colors (HB+ v2.1)

Per-player ball colors for Hamsterball Plus.

## What It Does

Tints player balls to hex colors from `mknp_player_colors.set` (jsonc — `//` and `/* */` comments allowed). Supports all 4 players across 1P/2P/4P modes. Config re-read ~every 2 seconds — edit at runtime, no restart needed.

## Installation (Hamsterball Plus required)

1. Place `mknp_player_colors.dll` in the game's `Mods\` folder.
2. Launch the game. The mod auto-loads via HB+ and auto-generates `mknp_player_colors.set` next to the DLL.
3. Edit the 7 color keys (see below).

## Configuration

Edit `mknp_player_colors.set` (next to the DLL in `Mods\`, NOT the game root):

```jsonc
{
  "p1_single": "FFFFFF",   // P1 in 1-player mode
  "p1_2p":     "4A90D9",   // P1 in 2-player mode
  "p1_4p":     "2ECC71",   // P1 in 4-player mode (Arena/Rumble)
  "p2_2p":     "FF6B35",   // P2 in 2-player mode
  "p2_4p":     "E74C3C",   // P2 in 4-player mode (Arena/Rumble)
  "p3_4p":     "9B59B6",   // P3 in 4-player mode (Arena/Rumble)
  "p4_4p":     "F1C40F"    // P4 in 4-player mode (Arena/Rumble)
}
```

Hex RGB (`#`/`0x` prefixes and 3-digit shorthand accepted). Missing keys keep their current value, so partial sets are fine.

## Technical Details

- **Board color table**: `board+0x3AB0` (P1) `+0x3AC4` (P2) `+0x3AD8` (P3) `+0x3AEC` (P4), 4 RGBA floats each, `0x14` stride. Board reached via HB+ `GetScene()` (Scene == Board), player count via `Scene.player_count`.
- **No threads** — per-tick apply in `onGameUpdate()`.

## Files

- `mknp_player_colors.dll` — compiled HB+ mod
- `mknp_player_colors.cpp` — source code (MinGW, nocrt, manual 17-entry vtable)
- `mknp_player_colors.set` — auto-generated config file (lives next to the DLL)

## Building

- **MinGW (Linux):** `cd source && ./build.sh` → `mknp_player_colors.dll`

## Testing note

The HB+ loading chain (scanning `Mods\`, calling `CreateModInstance`, firing `Initialize()`) cannot be verified under Wine/hbtestd. Only real Windows with HB+ installed can confirm the full load + in-game behavior. Parser (`strip_jsonc`/`find_key_str`/`parse_hex`) verified with a native gcc harness: comment stripping, `#`/3-digit hex, missing-key and ghost-key cases all pass.

## Author

MAKYUNI / Hamsterbot
