# mknp_animated_textures

A Hamsterball Plus mod. Cycles numbered texture variants at a custom
framerate. Install: copy `mknp_animated_textures.dll` into the game's
`Mods/` folder (Hamsterball Plus required).

## How it works

1. `mknp_animated_textures_set.jsonc` (next to the dll) lists which
   textures animate, with per-texture settings
2. Background thread scans the Graphics texture cache for those
   `baseNN.png` frames (e.g. `Circleanim02.png`). Names NOT in the
   set are ignored, so stock textures like `Title02.png` are never
   touched
3. Remaining frames load via the game's own texture loader
4. The `IDirect3DTexture8*` pointer in the texture object swaps at the
   configured framerate. Original texture restores on level exit.

## Any-frame start

The level may reference ANY frame (`Circleanim01`, `02`, ... `NN`).
Each material's texture animates phased from its own frame: an
object wearing `A01` cycles 1-2-3, one wearing `A02` cycles 2-3-1.
Files on disk must still be `01..NN` contiguous, 2-digit, no underscore.

## Set file format

One `mknp_animated_textures_set.jsonc` next to the dll (`//` and
`/* */` comments allowed, trailing commas allowed):

```
[
    "Circleanim", { "framerate": 0.25, "looptype": 1 },

    "WaterFlow",  { "framerate": 0.5, "looptype": 3, "proximity": 450 },
]
```

- **framerate**: seconds between frame swaps (float, default 0.5)
- **looptype**: `0` = play once, stop on last frame.
  `1` = loop forever (default). `2` = ping-pong (reverse direction
  at ends). `3` = proximity gate (see below).
- **paused**: `true` (default) = freeze on the current frame while the
  Esc menu is open, resume cleanly on close. `false` = ignore pause,
  keep animating.
- **proximity**: ball distance that counts as "near" (float, loop 3
  only, default 300).

Re-read on every level enter: edit the file, replay the level, no
restart needed. Syntax error = previous set kept + `SET parse error`
in the log. Missing file = no animations + `SET missing`.

## Looptype 3 (proximity gate)

Zero setup: anchors = positions of the meshes wearing the texture —
found from the level file (`levels/*.MESHWORLD`, S1-count fingerprint,
strip-vertex averages, up to 8 per animation), no ref points needed.
Ball inside `proximity` radius = hold frame 01. Ball leaves = play
02..NN once, then hold the last frame. Re-enter = hold 01 again.
Exit edge = 1.15x radius (anti-flicker at the boundary).
Optional manual extras: S1 ref points named `PROX:<base>`. No anchor
at all = plain loop plus one log line. Transitions log as `NEAR` /
`FAR`. Anchor lines in the log: `MATCH` (materials scanned/hit) +
`FILE` (level file + geoms) + `ANCHOR` (auto + manual).

## File naming

- Set: `Mods/mknp_animated_textures_set.jsonc` (next to the dll)
- Frames: `Textures/Circleanim01.png`, `Circleanim02.png`, ...
- Level material: point at any one of the frames

## Log

`mknp_animated_textures.log` is written next to this dll (the `Mods/`
folder): INIT + ENTER/EXIT + every cache hit + skip reasons (no txt /
few files / load fail) + setup line + first 8 swaps. Send it back if an
animation sits still.

## Technical details

- HB+ v2.1, MinGW build: nocrt + manual 17-entry vtable, KERNEL32 only
- Board/ball via HB+ GetScene/GetPlayer (manual vtable, same as
  mknp_battyball_entities), direct-memory fallback
- Texture loader at 0x455C50, cache at Graphics+0x2E4/+0x6F0
- Swaps `IDirect3DTexture8*` at texture object +0x04, 16 ms tick thread
- Up to 32 animations (one per texture object), 32 frames each
- `source/` holds `.cpp` (HB+ build, shipped) + `.c` (retired bass-proxy
  variant — kept for reference, does NOT work in `Mods/`)

## Build

```sh
cd source && bash build.sh
```
