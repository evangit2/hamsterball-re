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

## Own meshes (v1a+)

Each `REF:GRIDxx` point spawns **its own mesh**, taken from the level files —
no shared `testcube` needed. At level start the mod scans `levels\*.MESHWORLD`
for a geom named like the ref (`REF:GRID01`, else stripped `GRID01`), extracts
it into `levels\mknp_grid<N>.MESHWORLD` (centroid at origin), and spawns that.
The geom name is kept verbatim, so **affix rules apply natively**:

- name has `(NOCOLLIDE)` → visual only, ball passes through
- otherwise → solid, ball stands on it

If no matching geom is found, the point falls back to `testcube` (if present),
else it is skipped with a log line. Missing meshes never crash.

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
