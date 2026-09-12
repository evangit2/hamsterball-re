# mknp_entities_plus (HB+ v2.1)

Safe merge of `mknp_battyball_entities` (GRID + named entities + point
lights + BallBorder ring + P1 glow) and `mknp_battyball_events` (custom
`E:` event-plane sounds) into ONE DLL. Drop-in replacement for running
both: install this DLL **instead of** the two old ones, never alongside
them (two drivers for the same `E:` planes would double-play sounds).

## What it does (v1i)

- Everything `mknp_battyball_entities` v1dx does (same S1 `GRID` cycle,
  `ENTITIES` defs, `grid_speed`, neon ring/glow, lights, TRAJ + LIGHTSON /
  LIGHTSOFF handling).
- Plus everything `mknp_battyball_events` v1k does (`E:name` sound events,
  `trigger_type` 0/1/2/3, `loop_sound`, `loop_delay`, `pause`, native +
  custom sounds, 1s re-fire gate).
- v1b prefix split: `REF:<name>` keys are entities (S1 ref-point spawn),
  `E:<name>` keys are events (event quads/planes). Renames: `e_Launch` to
  `E:Launch`, `Woodbridge_area` to `E:Woodbridge_area`, `Cheesepit` to
  `E:Cheesepit`. Behaviour match is prefix-blind (bare/`REF:`/`E:` identical,
  old sets keep working).
- v1c: numbered `E:` driver variants (`E:Woodbridge_area0`) route to the
  drivers too (prefix match on both scanners).
- v1d: scan logs EVERY S1 name + `area=`/`def=` match (`S1[i]=<name>
  area=<d> def=<d>`), so unmatched refs are visible instead of silent.
- v1e: `E:`/`REF:` prefixes are routing labels only. Stored/matched names
  are prefix-stripped (`E:Cheesepit` matches S1 `REF:Cheesepit`,
  `E:Cheesepit01`, bare `Cheesepit`). `E:` blocks route on content:
  with `behaviour` = entity-driver, without = sound event.
- v1f: trailing digits in def names are wildcards: `REF:MouseA0` loads
  S1 `REF:MouseA1`..`REF:MouseA9` (any digit, literal incl), two-digit
  `..00` loads `01`..`99`. Same rule for S6 quad names (area/tar/launch).
- v1i: `flip_normals` removed entirely (never fixed the lighting).
  Mouse auto-flip restored as the only normals path.
- One DLL, one log (`mknp_entities_plus.log`), one set file
  (`mknp_entities_plus_set.jsonc`, re-read every level start, same
  tolerant parsers: `E:` blocks with `behaviour` are entity-drivers,
  without are sound events; stored names are prefix-stripped).

## Migrating

1. Delete `mknp_battyball_entities.dll` and `mknp_battyball_events.dll`
   from `Mods\` (keep their set files as backup).
2. Copy `mknp_entities_plus.dll` + `mknp_entities_plus_set.jsonc` into
   `Mods\`.
3. Copy your old `"Name", {...}` entity defs and `"E:...", {...}` event
   entries into the new set file (same syntax, verbatim).

## HB+ Options

| Control | ID                    | Default | Range    | Description                                |
|---------|-----------------------|---------|----------|--------------------------------------------|
| Toggle  | `BATTY_ENTITIES`      | ON      | -        | Master switch for GRID/entities/lights.    |
| Slider  | `BATTY_GRID_SPEED`    | 3.0     | 0.5–30.0 | Seconds each GRID point stays visible.     |
| Toggle  | `BATTY_LIGHTS`        | ON      | -        | Master switch for native point lights.     |
| Slider  | `BATTY_LIGHT_RANGE`   | 400.0   | 50–3000  | Light range (native default 400).          |
| Slider  | `BATTY_LIGHT_INTENSITY` | 5.0   | 0–100    | Material-color multiplier.                 |
| Toggle  | `BATTY_EVENTS`        | ON      | -        | Master switch. When OFF, events are quiet. |

## Log

```
INIT Battyball Entities Plus v1i log=... set=...
SET init: N sound events
NEWBOARD ...
EV LEVEL start
...
```

## Build

```bash
cd source && ./build.sh
```

MinGW `i686-w64-mingw32-g++`, same flags as the parents.
Merge script: `/tmp/merge_plus.py` (entities base + transplanted events
section with `ev_enabled` / `ev_prev_tick` renames, merged callbacks).
