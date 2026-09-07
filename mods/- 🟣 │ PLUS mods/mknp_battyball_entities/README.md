# mknp_battyball_entities (HB+ v2.1)

First mod in the **battyball** series — a set of HB+ mods designed to work together.

This mod recreates the **GRID object system** from `mknp_custom_entities` (the bass.dll proxy mod) from scratch as a standalone HB+ mod. It does **NOT** include the custom-entity (cEnt) system — only the Grid objects.

## What it does

Scans the active level's **S1 reference points** for any name containing `GRID` (e.g. `GRID01`, `GRID02`, `testcube(GRID01)`). Each GRID ref point marks a position where the `testcube` mesh is placed.

Only **one** grid cube is visible at a time. The visible cube cycles:

```
GRID01 -> GRID02 -> ... -> GRIDnn -> GRID01 -> ...
```

Every **`grid_speed`** seconds, the current cube is despawned (removed from the update/render/collision lists, collision object destroyed) and the next one is spawned.

All GRID objects are **preloaded once at level start** (mesh load + construct).
Switches only move list membership (hide old, show next): no disk, no
rebuild, so high speeds stay smooth. Full destroy happens at level quit,
toggle-off, or board change.

## Own meshes (v1m+)

Each `GRIDxx` ref point spawns its mesh from a file: ref `GRID01`
(or `REF:GRID01`, `GRID01(NOCOLLIDE)`, ...) loads `levels\Grid01.MESHWORLD`,
`GRID02` loads `levels\Grid02.MESHWORLD`, and so on. The number is the
digits right after `GRID` in the S1 ref name. Whatever the file holds
(mesh, affixes) applies natively at load. Old `mknp_grid<N>` temps are
deleted at level start (no longer used).

If `GridNN.MESHWORLD` is missing or fails validation (must parse, >=1
geom), the point falls back to `testcube` (if present in `levels\`), else
it is skipped with a log line. Missing meshes never crash.

- name has `(NOCOLLIDE)` → visual only, ball passes through
- otherwise → solid, ball stands on it

If no matching geom is found in the current level file, the point is
skipped with a log line. Missing meshes never crash. (The old `testcube`
fallback is disabled — level geoms only.)

## Per-level speed (`mknp_battyball_entities_set.jsonc`)

The mod auto-creates `mknp_battyball_entities_set.jsonc` next to the DLL on
first run (default: every level `1.0`). Each `levelN` value **multiplies** the
HB+ `grid_speed` slider for that level file:

```jsonc
"grid_speed", { "level1": 1.0, "level2": 2.0, ... }
```

`level2 = 2.0` with slider `3.0s` = `6.0s` per switch on that level.
Level identity is an S1 fingerprint, so renamed and swapped files still match.
Extra keys (e.g. `levelup`, `levelcascade`) work if you add them. Reloaded
every level start, so edits apply on next level load.

## Features

- **Native spawning mechanics** identical to `mknp_custom_entities`:
  - `operator_new` (`0x4BA57B`) for allocation
  - `MeshWorld_ctor` (`0x461510`) loads `levels\testcube.MESHWORLD`
  - `PopCylinder_ctor` (`0x436EE0`) creates the object
  - `AthenaList_Append/Remove` (`0x453810`/`0x4534D0`) wire it into the game lists
- **HB+ callback loop** (`onGameUpdate`) instead of a background thread — the cycle uses `GetTickCount()` timing, so nothing blocks.
- **Auto-installs the mesh**: copies `testcube.MESHWORLD` (ships beside the DLL) into the game's `levels\` folder on load, so you don't have to.
- **Config via HB+ options** (no `.txt` file to edit).

## HB+ Options

| Control | ID | Default | Range | Description |
|---|---|---|---|---|
| Toggle | `BATTY_ENTITIES` | ON | - | Master switch. When OFF, despawns all grid cubes. |
| Slider | `BATTY_GRID_SPEED` | 3.0 | 0.5–30.0 | Seconds each GRID point stays visible before cycling. |
| Toggle | `BATTY_LIGHTS` | ON | - | Master switch for native point lights. |
| Slider | `BATTY_LIGHT_RANGE` | 400.0 | 50–3000 | Light range (native default 400). |

## Point lights (v1p+)

Each level file's S3 `DISTANTLIGHT`s become native D3D point lights,
plus any S1 ref named `PointlightXX` (`POINTXX` legacy) becomes a light at
its position (S3 first, refs fill free slots). Ref color = the S6 mesh
material diffuse (mesh name containing the ref name, e.g.
`mat_pointlight01...`) x fixed intensity 5.0 (`LIGHT_INTENSITY`); white if
no mesh found.
at level start the mod matches the on-disk file (S1 fingerprint), reads
light position (file x,z,y → game x,y,z) + color, creates native
`SceneObject`s exactly like `Scene_SetupLevelDark` does for Neon
(`+0xD0=1` POINT, emitter +0x94, range +0xCC), and registers them in gfx
slots 4–7. Position is set via native `vtable[1]` SetPosition (like
`Scene_SetupLevelDark` — never bare `+0x08` writes alone). RefreshLight
`0x46B670` builds the D3DLIGHT8 in-object at `+0x20`: true Range field is
`+0x68` (written by mod — the game never does), `+0xB8` is specular
(never touched). RefreshLight bakes `Att2=0.04` every call — the mod
zeroes that constant once (`Att2 patch`), else `1/(1+0.04d^2)` attenuation
eats the light (0.0003 at 288 units). The engine itself drives D3D — the mod never calls D3D
functions. Slots re-assert ~every 2s (race retry wipes them). Ambient is never touched (darkness stays file-side).
`E:LIGHTSOFF` / `E:LIGHTSON` toggle the mod lights like the native ones.
First 4 lights win; extras are logged and skipped.
`Light Intensity` slider rescales POINT pools live (S3 file lights keep
designer values); output trim x20 so mat 1.0 = emitter 100 at default gain.
Felt range = shown slider (D3D cuts light beyond Range).
Disasm audit (0x453BD0/0x46B4F0/0x46B670): register, ctor and refresh are
strictly per-object/per-slot — the mod cannot alter native slots 0-1.

## Level setup

Place S1 reference points with `GRID` in the name in your custom level. The order they cycle in follows their order in the S1 list (`GRID01` first, then the next matching entry, etc.). The `testcube.MESHWORLD` you ship with the mod can be swapped for any mesh you'd like to appear at each point.

## Installation

1. Place `mknp_battyball_entities.dll` in the game's `Mods\` folder (HB+ required).
2. Ship `testcube.MESHWORLD` in the same folder (or directly in the game root) — the mod copies it into `levels\testcube.MESHWORLD` automatically.
3. Launch the game. The mod auto-loads via HB+.

## Building

- **MinGW (Linux):** `cd source && ./build.sh` → `mknp_battyball_entities.dll`
- Uses `nocrt` + manual 17-entry vtable + `hbplus_api.h` (MinGW-safe pattern).

## Testing note

The HB+ loading chain (scanning `Mods\`, calling `CreateModInstance`, firing `Initialize()`) cannot be verified under Wine/hbtestd (that environment uses a plain bass proxy, not the HB+ framework). Only real Windows with HB+ installed can confirm the full load + in-game behavior.

## Author

MAKYUNI / Hamsterbot

## Changelog

### v1
- Initial release: standalone HB+ GRID-object cycling system, ported from `mknp_custom_entities`.
