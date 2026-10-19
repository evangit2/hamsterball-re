# mknp_player_colors (HB+ v2.1)

Per-player ball colors for Hamsterball Plus.

## What It Does

Tints player balls to hex colors from `mknp_player_colors.set` (jsonc — `//` and `/* */` comments allowed). Player1 ball follows the current mode (Tournament and Time Trials both run 1-player). Supports all 4 players across 1P/2P/4P modes. Config re-read ~every 2 seconds — edit at runtime, no restart needed.

## Installation (Hamsterball Plus required)

1. Place `mknp_player_colors.dll` in the game's `Mods\` folder.
2. Launch the game. The mod auto-loads via HB+ and auto-generates `mknp_player_colors.set` next to the DLL.
3. Edit the 7 color keys (see below).

## Configuration

Edit `mknp_player_colors.set` (next to the DLL in `Mods\`, NOT the game root):

```jsonc
[
  "Player1_ball_color",    { "1P_mode": "FFFFFF", "2P_mode": "FFFFFF", "4P_mode": "FFFFFF" },
  "Player1_ball_specular", { "1P_mode": "FFFFFF", "2P_mode": "FFFFFF", "4P_mode": "FFFFFF" },
  "Player1_ball_outline",  { "1P_mode": "FFFFFF", "2P_mode": "FFFFFF", "4P_mode": "FFFFFF" },
  "Player1_ball_outline_E",{ "1P_mode": "FFFFFF", "2P_mode": "FFFFFF", "4P_mode": "FFFFFF" },
  "Player1_UI", { "1P_mode": "FFFFFF", "2P_mode": "FFFFFF", "4P_mode": "FFFFFF" }
]
```

- `Player1_ball_color`: Player 1 ball tint. `1P_mode` = Tournament and Time Trials, `2P_mode` = 2-player mode, `4P_mode` = 4-player arena (Rodent Rumble).
- `Player1_ball_specular`: Player 1 ball specular color, same modes.
- `Player1_ball_outline`: Player 1 ball ring/outline color, same modes.
- `Player1_ball_outline_E`: Player 1 ball ring/outline emission color, same modes.
- `Player1_UI`: Hammy1/Hammy2/Hammy3 portrait tint. `1P_mode` = Tournament and Time Trials menus (incl. Tournament win screen), `2P_mode` = 2-player menu, `4P_mode` = arena menu + in-game arena. (UI tint lands in the next build — this build wires the file + ball tint, and logs what the UI hook needs.)

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
