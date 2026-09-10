# mknp_textures_plus

A Hamsterball Plus mod. Per-texture behaviors from a set file.
Install: copy `mknp_textures_plus.dll` + `mknp_textures_plus_set.jsonc`
into the game's `Mods/` folder (Hamsterball Plus required).

## Set file format

One `mknp_textures_plus_set.jsonc` next to the dll (`//` and
`/* */` comments allowed, trailing commas allowed):

```
[
    // Checker Textures
    "tex_checker_01",     { "texture_type": "checker", "texture": "checker01" },

    // Custom Goal Textures
    "tex_goal-darkround", { "texture_type": "goal",
                            "goal_off_tex": "goal-darkround",
                            "goal_on_tex": "goal-darkround-lit" },
]
```

- First string per entry = label (log identifier only).
- Names accept bare (`checker01`) or pathed (`Textures/checker01`)
  form, with or without extension — matched case-insensitive
  against the game's texture cache.
- Re-read on every level enter: edit the file, replay the level, no
  restart needed. Syntax error = previous set kept + `SET parse error`
  in the log. Missing file = mod idle + `SET missing`.

## texture_type

- **checker**: forces POINT (nearest) MAG/MIN/MIP filtering for the
  `texture` — stays sharp even when super small, same treatment as
  vanilla checker textures. Only listed textures are touched.
- **goal**: works like vanilla goal textures. Shows `goal_off_tex`
  normally, swaps to `goal_on_tex` when the player touches `N:GOAL`
  (native parity: same D3D-pointer swap the game does for
  `goal.png` -> `goal-lit.png`). Restores on level start/end.

## Log

`mknp_textures_plus.log` is written next to this dll (the `Mods/`
folder): INIT + SET lines + per-texture resolve (`GOAL ...: ready`,
`CHECKER ...: not found`) + swap lines (`GOAL ...: ON`, `GOAL ...:
restored`). Send it back if a texture sits still.

## Technical details

- HB+ v2.1, MinGW build: nocrt + manual 17-entry vtable, KERNEL32 only
- Checker hook: D3D8 device vtable SetTexture[61]/SetTextureStageState[63]
  (same technique as mknp_sharp_textures; non-listed pass through)
- Goal swap: texture cache (Graphics+0x2E8/+0x6F0, D3D ptr at +0x04),
  loader at 0x455C50, trigger = onEventPlaneCollide `N:GOAL` +
  board+0xCD0 poll fallback
- `source/` holds `.cpp` (HB+ build, shipped)

## Build

```sh
cd source && bash build.sh
```
