# mknp_entities_plus (HB+ v2.1)

Safe merge of `mknp_battyball_entities` (GRID + named entities + point
lights + BallBorder ring + P1 glow) and `mknp_battyball_events` (custom
`E:` event-plane sounds) into ONE DLL. Drop-in replacement for running
both: install this DLL **instead of** the two old ones, never alongside
them (two drivers for the same `E:` planes would double-play sounds).

## What it does (v1av)

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
- v1j: `e_Mousepush` behaviour (type 48, static). Touch sphere
  (`push_radius`, def 70u XZ, |dy| < 60) kicks the ball away from pad
  home like a bumper: accumulators += `push` (def 3.0) along
  (ball-home) XZ + `push`*0.5 up, one-shot custom def `sound` (else
  silent), 0.4s cooldown, rising-edge. Needs S1 `E:Mousepush` refs
  (any level) + `"E:Mousepush", { "behaviour": "e_Mousepush",
  `"mesh": "Mouse" }` (shipped in the set already).
- v1k: mesh-file S1 refs. After any static instance spawns from mesh file
  F, F's own S1 is scanned: refs matching a type-48 (`e_Mousepush`) def
  spawn invisible follow-pads at carrier_home + (s1pos - S5centroid),
  riding the carrier live pos (obj+0x10D4) per-frame (translation only).
  Put S1 `E:Mousepush` in the MESH file (editor coords) — no level ref
  needed. Log shows centroid + local + pad home to verify placement.
- v1m: E:Mousepush quad push (event-side, replaces the bumper idea).
  The quad's facing is read from the level file at level start (Newell
  normal, dominant axis + sign); touching the quad adds force 20 along
  that axis every frame (accumulators, continuous while touching). Your
  quad faces +X exactly, so touch = +X push 20. No quad in file =
  fallback +X. Log: PUSHQ scan line per level + throttled PUSHQ hit
  lines on touch.
- v1p: PUSHQ proximity volume. Ellipsoid `proximity * proximity_scaleX/Y/Z`
  (from your Mousepush def) around the quad centroid; inside = force 20
  along facing every frame. Runs without touching the quad, needs no
  entity instances. Touch event now fires only when no quad found (no
  double force). Log: scan line shows c + prox + sc; enter/leave vol
  lines on transitions.
- v1q: volume center RIDES the live Mouse instance (obj+0x10D4) when one
  exists, static quad centroid otherwise. Facing still from the file
  quad. Enter line shows c + ride/static.
- v1r: shove jump. Mouse moving 5+ frames straight + ball in vol = one
  Y impulse 20.0 (jump_mod proven value), once per motion leg (30 still
  frames re-arms). Log: PUSHQ shove jump line.
- v1w: shove jump 150x8 frames (single-frame impulses clamp —
  20/150/1000 all read the same, sign never the issue).
- v1y: NEWBOARD clears a carried-over paused flag (restart-from-pause
  no longer starts frozen; log: was paused, cleared). Mid-game pausing
  untouched.
- v1x: shove grants E:JUMP-identical landing immunity (+0x2EC=0,
  +0x2F4=2000, log shows immune=2000) — high landings never dizzy,
  height untouched. Restart fix: EV LEVEL forces full rescan (same-
  address board reuse no longer skips it); S1-not-ready retries ~10s
  instead of latching dead (log: S1 not ready, waiting...).
- v1av: `Cloudscape` fog kill UNCONDITIONAL (fogentry read suspect) +
  4-state diag `SKY: rs7/rs22/rs27/rs28` to verify the GET slot.
- v1au: `Cloudscape` `blend` knob (def 0 = opaque; 1 = alpha blend,
  SRCALPHA/INVSRCALPHA, entry states saved+restored). Soft edges blend in.
  NOTE: fogentry=0 on LevelS (level fog already off; fog knob correctly idle).
- v1at: `Cloudscape` `fog` knob (def 1 = level fog; 0 = unfogged).
  Entry FOGENABLE saved via GetRenderState (vtbl[0x33]) + restored exactly.
- v1as: `Cloudscape` knobs (def keys verbatim): `scale` (quad size x,
  def 1), `ambient` (base brightness +, def 1), `emission` (added floor,
  def 0). Brightness = ambient + emission (def = v1ar look).
- v1ar: `Cloudscape` BLACK fixed (Ghidra: RenderQuad forces LIGHTING=1 via
  0x402860; ctor material Diffuse=white+Ambient=black = black on lightless
  levels). Ours now Diffuse=black + Emissive=white (pure texture, all races).
- v1aq: `Cloudscape` Sky-race native test (draw native board+0x4374 sprite
  from hook + dump native-vs-ours fields; native alloc 0xD4, same ctor/path).
- v1ap: `Cloudscape` NO state touches (kills+restore-to-1 were black
  suspect #1) + Sky-slot side-by-side with native + sprite mat dump.
- v1ao: `Cloudscape` native order (quad FIRST, level overdraws) + double-quad
  prime (2nd finds stages hot via own 0x402860 bind) + minimal dance.
- v1an: `Cloudscape` double-render (orig, quad, orig): painter-order
  buries quad, zero depth dependence. Base has no Clear, per-frame-pure.
- v1am: `Cloudscape` DIAGNOSTIC (same visual): + `744=` basis ptr
  pre/post (view-swap proof; fix writes itself).
- v1al: `Cloudscape` DIAGNOSTIC (same visual as v1ak): logs
  `SKY: pre/post 7a8/7b0/7c0/700/70c/70d/s4/gfx` (find the prime delta).
- v1ak: `Cloudscape` ZENABLE ON around post-base quad (base leaves it
  OFF; left ON after, native-Sky exit state).
- v1aj: `Cloudscape` post-base draw (invoke-wrapper: orig first, then
  quad; depth level-fresh, stages hot; base-only, `orig=` in log).
- v1ai: `Cloudscape` + fog kill around early quad (entry fog blacks it).
- v1ah: `Cloudscape` minimal early (bisect result: quad-only positions
  right, full state dance broke frame): early = quad + LIGHTING kill
  only. Intermediate entry is lit, Sky entry is not.

- v1af: `Cloudscape` early-path lighting: v1ae forced LIGHTING=1 after
  the quad, blackening the level it was meant to sit behind (native
  never touches RS137, entry is already unlit). Early path now does
  exactly the native ops, nothing more.
- v1ae: `Cloudscape` draw-FIRST (native order): slot-24 vtable wrap draws
  the quad before the level render (native `0x410E80` calls quad, then
  base `0x40B420`), transparent naked wrapper + per-class originals.
  Late onRenderApply path retired when wrap live. Native Sky Level9
  CLOUDSCAPE sits at the same coords you use, so S1 pos was never the
  bug, order was.
- v1ad: `Cloudscape` layer + pause: onRenderApply fires with ZENABLE off
  (giant quad overdraws level) -> force Z on around quad, restore off;
  paused skips draw (entity stages dead = black quad).
- v1ac: `Cloudscape` lighting (black-rect candidate): onRenderApply fires in
  entity phase (LIGHTING=1, sprite renders black); native Sky draws in
  level phase (unlit). RS 137 off/on around the quad + `SKY: sprdims`
  and `SKY: draws=N` diag lines.
- v1ab: `Cloudscape` crash fix (v1aa called the state fn thiscall-shaped:
  wrong args + ESP off by 4 = d3d8 crash. Native pushes 3 args and ECX
  is leftover garbage at both sites, so the fn is stdcall(dev,st,val).
  Sprite alloc 0x10F4 = native size).
- v1aa: `Cloudscape` black-rect fix: render-state toggle now calls the
  `gfx+0x154` render-device vtable (native `0x410E80` shape, binary-
  verified) instead of the gfx vtable, exact `textures\clouds.png` path,
  + `SKY: tex=ok/MISSING` probe line proving the file exists in the
  Battyball install.
- v1z: `Cloudscape` behaviour — Sky Race's cloud skybox on any level.
  S1 `REF:Cloudscape` point sets the position (native reads it from the
  `CLOUDSCAPE` file entry). Load is `Sprite_ctor` (0x45D0C0, op_new
  0x110, `textures\clouds.png`); every frame `render_apply` replays
  `SkyBoard_RenderDynamic` (0x410E80) exactly: projection 20/50000,
  cull off, `RenderQuad` layer 23.0, cull on, projection restored.
  No instance, no collision, `mesh` ignored (any non-empty value —
  the set parser drops mesh-less defs). One global sprite reused
  across levels. Sky slot 13 keeps native clouds (mod idle).
  `visible=false` supported. Log: `SKY:` lines.
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
INIT Battyball Entities Plus v1av log=... set=...
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
