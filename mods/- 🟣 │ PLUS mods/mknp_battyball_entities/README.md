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
Parsed pool = 64 (S3 + POINT refs, no cap in practice); only the nearest 7
to the ball own gfx slots 1-7 at any moment, re-picked every frame, rest
stay defined-but-dark — walk the level and the light window follows you.
Slot 0 stays native (P1 follower, never touched): 7 mod + 1 native = D3D8's 8.
`REF:PlayerlightXX` (max 4) parses exactly like a Pointlight (same mesh-color
+ `(Rnnn)` rules) but snaps to the ball every frame — d2=0, so it always wins
a slot and behaves as a Neon-style follower light.

## BallBorder (v1ax+)

P1's `ballborder.png` ring uses a ball-local `D3DMATERIAL8` (Ball_Render passes
`ball+0x1B8`, consumed at `+4`). Native leaves diffuse+emissive black, so the
ring goes dark on dim levels.

## Per-level ring color (v1ay+)

`neon_ballring_player1` in `mknp_battyball_entities_set.jsonc` maps each race
slot to `[r,g,b,a]` (P1 only). Keys are slot IDs, NOT filenames:
1 WarmUp, 2 Beginner/Cascade, 3 Intermediate, 4 Dizzy, 5 Tower, 6 Up, 7 Neon,
8 Expert, 9 Odd, 10 Toob, 11 Wobbly, 12 Glass, 13 Sky, 14 Master,
15 Impossible. Present key = ring painted with that RGBA every frame (diffuse
RGBA + emissive RGB); commented-out/absent key = ring forced back to native
black (no glow, no stale color from the last level). Slot = board vtable
(`board+0x0`, set by `LevelBoard_X_ctor` — Ghidra-verified all 15, immune to
file swaps and renames; names are never read). Unknown
boards (slot 0: Pinball, menus) are left fully native — never touched. Off
with the master `BATTY_ENTITIES` toggle. Same slot keys drive `grid_speed`.

Neon (slot 7) needs no special case: `Ball_Render` (0x403DB8) only READS
`ball+0x1B8` at its 3 `Sprite_RenderQuad` sites and never writes it, so the
mod's per-frame write (update + render loop) always wins over the exe when
the `level7` key exists. Absent `level7` forces native black, killing the
stock Neon glow too.

## P1 emitter glow (v1bb+)

`neon_glow_player1` does exactly the Neon_colors mod's Player GLOW half, from
jsonc: P1 emitter SceneObject (`scene+0x436C`, set up by `0x416270`) gets RGB
at `+0x94` and A at `+0xA0`, every frame. Same `levelN` slot keys (slot 7 =
Neon); absent/commented key = emitter untouched (native glow). The OUTLINE
half of Neon_colors is already `neon_ballring_player1` above.
Slots 1-3 are P2-P4 follower slots: empty in solo, and native-priority
yield defers ours the moment a native claims one (reclaims when freed).
Naming rules: S1 ref `PointlightNN[(Rrange)]` (e.g. `Pointlight02(R200)`,
slider-proof range; suffix strips for lookup); fixture material name MUST
contain the ref name (`mat_pointlight02...` glows for `Pointlight02`).
Emissive RGB = color, emissive alpha = per-light gain (1.0 = same).
Missing material -> unity white + `no-mat` tag, never blinding.
`Light Intensity` slider is relative (5.0 = x10 base, 10 = x20, to 100).
Emitter = emissive x emissive-alpha x Intensity/5 x10 trim.
Unity emitters (~0.3) are dim past ~50 units under native 0.04 falloff:
raise Intensity (or emissive alpha, to 10) for far-reaching pools.
Felt range = shown slider (D3D cuts light beyond Range).
Disasm audit (0x453BD0/0x46B4F0/0x46B670): register, ctor and refresh are
strictly per-object/per-slot — the mod cannot alter native slots 0-1.

## Named entities (v1be+)

Top-level set pairs next to `grid_speed` define named entities:

    "Woodbridge", { "behaviour": "Woodbridge", "mesh": "Woodbridge.MESHWORLD" },

- Key = entity name, also the S1 ref substring (`REF:Woodbridge` in the level).
- `behaviour` = behaviour name. v1be stores it only; every entity is static.
- `mesh` = meshworld file, always loaded from the game's `Levels` folder.

At level start each `REF:<Name>` spawns its mesh via the same native chain as
GRID (`operator_new` -> `MeshWorld_ctor` -> `PopCylinder_ctor` -> lists) and
stays visible (solid, never cycled). Missing or bad mesh files log
`ENT <Name>: MISSING/BAD` and skip without crashing. Full destroy at level
quit via the shared despawn path.

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

### v1be
- Named entities: set jsonc top-level `"Name", { "behaviour", "mesh" }` pairs spawn `REF:<Name>` refs with `Levels/<mesh>` meshes (persistent static solids, same native chain as GRID). Behaviour stored only for now. INIT log now lists `log=` + `set=` paths.

### v1
- Initial release: standalone HB+ GRID-object cycling system, ported from `mknp_custom_entities`.
