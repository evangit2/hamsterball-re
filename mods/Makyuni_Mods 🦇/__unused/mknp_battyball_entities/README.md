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
- `behaviour` = behaviour name.
  - `Woodbridge` (v1bj+): sinks below home while the ball is inside the
    proximity ellipsoid, rises back to home when it leaves. Per-def keys:
    `low_Y` = sink depth below home (def 50), `speed_Y` = seconds for full
    home-low travel (def 0.5), `proximity` = base zone size (def 150),
    `proximity_scaleX/Y/Z` = per-axis stretch, 1 = original (def 1/1/1).
    Zone = ellipsoid semi-axes proximity*scale around home; Y measured
    from fixed home so sinking never retriggers itself.
    Non-positive values keep defaults. Smooth both ways, frozen while
    paused. Move uses the native reposition path (`obj+0x10D8` +
    `+0x10E4` dirty flag, consumed by update `0x43DED0`).
  - REF rotation (v1bw+): the ref angle (THIRD S1 rot float:
    file rot2->ry, DAT ROT_Y wins) pitches about X, mirrored
    (M = Rx(-a)*T, D3DX rows [1,0,0],[0,c,-s],[0,s,c]), actuated
    every frame from slot-11 + slots 21/22 backstop, direct
    renderLevel+4 writes (model-space mesh, zero device touches).
    Trigger zone follows (stored angle negated into the X slot).
    NO native X/Z draw path exists (0x45AF3E dead stub, NEVER call).
    KNOWN LIMIT: collision flat.
  - Woodbridge_area gate (v1bz+): `"Woodbridge_area",
    { "behaviour": "Woodbridge_area", "mesh": "vertices" }` loads a
    trigger quad from the VERTICES of the level's S6 geom(s) whose
    name contains the def name (01/02/... each one quad, up to 8).
    Never rendered, spawns nothing (S1 REF:s are markers only).
    Ball inside any quad (XZ column, Y ignored) = proximity runs;
    outside = all bridges hold home. No quads found = gate open
    (old levels unchanged). Log: `AREA: quad <name>: N verts (...)`
    + `gate OPEN/CLOSED` edges. S1 refs hold one point only, never
    verts — the quad always comes from the S6 geom.
    Hiding the plane (v1ca+): the ENGINE renders every plain geom
    itself, so name it `E:Woodbridge_area01(NOCOLLIDE)` — `E:` is the
    native invisible flag, `(NOCOLLIDE)` keeps it non-solid. Loader
    matches by substring, unaffected. `"visible": false` (bare bool,
    default true) is read per def; on areas without the `E:` prefix
    the log warns `plane shows`.
  - Distance falloff (v1ca+): sink depth scales with closeness —
    target = -low_Y × (1 - dist): ball centered = full depth,
    zone edge = 0. Neighbours dip progressively, never constant.
  - Creak (v1cb+): native Wobbly replication — shared channel from
    App+0x4E0 via Sound_GetChannel 0x459810, primed 0.0 (0x459610),
    replayed every frame with vol=min(60×travel,1) (0x458EE0, silent
    at 0). Acquire-once lazy, never released. Log: `SND: creak ch=`
    + start/stop edges. Pause-safe (no calls while paused).
    v1cc anti-chop: fire ONLY while audible (silent re-fires restarted
    the sample); still = hands off, tail rings out; ~12f smoothed
    release bridges flicker.
    v1cd: `"sound_sensitivity"` per def (bare float, default 1, >0) —
    multiplies that def's travel before the vol mapping. Lower = only
    big dips trigger; higher = every tremor creaks. Logged as
    `sndsens=` when != 1.
    v1ce: `"sound": "<Name>"` per def loads `Sounds\<Name>.ogg`
    (ext auto-tried, 10 buffers) through native Sound_LoadAndAppend
    0x4664F0 (thiscall-shape typedef! fastcall would corrupt on RET 8).
    Exactly-once per def (leaf appends even when missing — no retry);
    fail = creakyplatform fallback, logged. Per-def channels + travel.
    Static meshes rotate about their own center via direct renderLevel+4
    matrix writes (v1bt, slot-11 every frame + slots 21+22 wrappers);
    collision stays flat (known limit). The Woodbridge zone rotates with the same rotation.
    Normals stay baked. AI-native types log rotation only.
  - AI behaviours (v1bg+, names verbatim from mknp_custom_entities AI list):
    native ctor + your mesh, motion via game lists. Rotator, Pendulum, Swirl,
    Flickfloor1, Flickfloor2, Flickring, Trode, Glassbreaker, Judge, Sign,
    8ball, Bonk, Catapult, Mace, Tipper, Lifter, Speedcylinder,
    Neonplatform, Trapdoor, Droplifter, Gluebie, Timebutton, 6ball.
    Catapult/Speedcylinder register solid collision; types without
    per-frame drivers stay visible (some non-solid) until v1bh+.
  - static fallback (PopCylinder, solid, no motion): Bridgeslam, Chrome,
    Funball, Tarbubble, Waterwheel + any other name.
  - `Tarpit` (v1ch+): Dizzy/Master tar, bass-mod port — touch-only
    zone (horiz dist2 < 1600 = r40 + |dy| < 40 around the S1 point,
    fixed, no keys). Entry: `+0x2D0=Y`, `+0x2CC=1`,
    DWORD `+0x768=0`, one-shot 3D splash (custom `"sound"` or native
    `gluestuck` via `board+0x878+0x484`). In-tar per frame: Y -= 0.25,
    `+0x2E9=0`; sunk past entryY - radius*2.5 calls Ball_Respawn
    0x405190 (SAFESPOT tp). Sink-then-die, never freeze. All balls via
    `board+0x29D4`. Logs `entry tar=1` + `sunk -> respawn`.
  - `Mouse` (v1df+): ping-pong on X — `home -> home+100 -> home`,
    repeat. Static PopCylinder (solid), mesh from `mesh`, spawn at each
    `REF:MouseXX`. Per-def keys (bare floats): `speed_X` = seconds
    per leg like Woodbridge speed_Y (def 2, rate = high/speed),
    `high_X` = travel units (def 100), `moveX_delay` = hold seconds at
    each end incl. base (def 0). Each forward start plays the def's
    `sound` one-shot at the mouse pos (unset = silent). Same native
    reposition
    path as Woodbridge (`obj+0x10D4` + `+0x10E4`), pause-frozen.
    Exact `behaviour: Mouse` match (never catches `Mousetrap`).
    Logs `ENT MouseN: reached +100/home` on turns.
  - Death bubbles (v1da+): three top-level set keys (NOT per def):
    `"death_bubbles": false` = no bubble spawns at all (default true).
    `"death_bubbles_tex": "<Name>"` loads bubble skin from
    `Textures\<Name>` (default + fallback `"Tar"`). Parsed + logged;
    texture swap ships after the mesh material-walk RE.
    `"death_bubbles_sfx": "<base>"` loads pop sounds from
    `Sounds\<base>1` (+ `<base>2`..`<base>9` while present) and pops
    pick round-robin among loaded (default `"bubble"` = native
    bubble1/bubble2 pops, no hook). Other bases install a Play3D
    0x459860 detour swapping the native pop lists (prologue verified,
    mismatch = native). Log: `BUBCFG:` + `BUBSFX:` lines.
- `mesh` = meshworld file, always loaded from the game's `Levels` folder,
  except `Tarpit`: `"vertices"` (or empty/missing/bad file) = invisible
  trigger-only zone at the S1 point, no spawn. A real file = visible
  solid + the same tar trigger on top.

At level start each `REF:<Name>` spawns its mesh via the same native chain as
GRID (`operator_new` -> `MeshWorld_ctor` -> `PopCylinder_ctor` -> lists) and
stays visible (solid, never cycled). Missing or bad mesh files log
`ENT <Name>: MISSING/BAD` and skip without crashing (except `Tarpit`,
which falls back to an invisible trigger). Full destroy at level
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

### v1dv
- 6ball behaviour: BadBall body + custom `mesh` skin. Same
  BadBall_ctor+v[1]+home as 8ball, custom `Levels/<mesh>` MeshWorld
  into App slot 2 (754=2, FunBall pattern). Slot 1 stays 8ball-only.
  Bad mesh = 8ball fallback, never invisible. Joins bad-balls list.
  Set: `"6ball", { "behaviour": "6ball", "mesh": "6ball.MESHWORLD" }`.

### v1dr
- Mouse forward start plays its `sound` one-shot (Tarpit splash
  pattern, positioned at the mouse, vol 1.0). Unset/failed = silent,
  never gluestuck fallback. Log: forward snd=custom/none.

### v1dq
- speed_X now seconds-per-leg (Woodbridge speed_Y style):
  rate = high_X/speed_X. Your 2/1/500 = 2s legs, 1s holds.
  Old u/s reading made 250s legs (looked dead). Def speed 2.

### v1dp
- Mouse per-def motion: `speed_X` (def 50 u/s), `high_X` travel
  (def 100u), `moveX_delay` hold at both ends incl. base (def 0s,
  pause-frozen). Missing keys = old behavior. Log shows parsed
  spX/dly/hiX per def + wait on each arrival.

### v1do
- Mouse normals inverted in-game: mesh authored inside-out
  (your editor check); mod negates all S5 normals in a temp
  copy at load, ctors from temp, deletes after. Orig never
  written. Fail = stock load. Log: normals inverted N.

### v1dn
- Y-flip generalized: base matrix (all behaviours) negates
  the Y row too. Only Y is ever flipped, X/Z normal.

### v1dm
- Mouse Y-flip on the composed matrix (Y row negated),
  Mouse-only. Log: ENT Mouse: rot ON.

### v1dl
- Mouse rotation fixed: flips reverted, axes normal; Mouse
  installs on any-axis rotation (incl. Z-only) and composes Z
  into the matrix (R=Rx*Rz, det +1). Log: ENT Mouse: rot ON,
  now gated on real install.

### v1dk
- Mouse Z-flip: Z row negated on top of XY+X-unflip,
  Mouse-only. Log: ENT Mouse: XY+X-unflip+Z-flip ON.

### v1dj
- Mouse X-unflip: X row negated on top of XY-swap, Mouse-only
  (now a true rotation, det +1). Log: ENT Mouse: XY+X-unflip ON.

### v1di
- Mouse XY-swap probe: X/Y basis rows swapped, Mouse-only
  (Y-flip reverted). Log: ENT Mouse: XY-swap ON.

### v1dh
- Mouse Y-flip probe: render matrix negates the Y row for Mouse
  only (Woodbridge path untouched). Log: ENT Mouse: Y-flip ON.

### v1dg
- Rotated Mouse fix: render matrix now reads live X (obj+0x10D4),
  not home X. Rotated Woodbridges unaffected (their X never moves,
  so live==home for them).

### v1df
- Mouse behaviour: `behaviour: Mouse` spawns its `mesh` at each
  `REF:MouseXX` as a solid static and ping-pongs +100 X from home
  at 50 u/s (`obj+0x10D4`/`+0x10E4` path, pause-frozen, per-instance).

### v1de
- Ambient bubble properties: `bubbles` bool gate (default true),
  `bubbles_tex` parsed+logged with `TarBlot` fallback (swap pending
  with death tex), `bubbles_sfx` base+N probe with its OWN pop path —
  ambient bubbles are ctor'd on a proxy app carrying their lists
  (native update plays them, death redirect never matches). Logs
  `BUBACFG:`/`BUBASFX:`.

### v1dd
- Ambient spread: rejection-sampled over the whole cover quad bbox
  (point-in-quad tested, home+-25 fallback) — no more single cluster
  around the one S1 point.

### v1dc
- Ambient height fix: spawns were ~29u buried (S1 home sits below the
  ridden surface). Surface now learned per instance from first entry
  ball Y (log `BUB: surfN learned`), ambient = surface-20; pre-touch
  fallback home+10.

### v1db
- Ambient TarBubbles (Dizzy/Master look): every Tarpit instance bubbles
  constantly from its surface (~8%/tick, +-25u square, same -20 height
  offset as death bubbles), player-independent, not gated by
  `death_bubbles`. Death + ambient share one demand queue in bub_frame.

### v1da
- Death-bubble properties: `death_bubbles` bool gate (default true),
  `death_bubbles_tex` parsed+logged with `Tar` fallback (swap pending
  material-walk RE), `death_bubbles_sfx` base+N probe with Play3D-detour
  pop redirect (`bubble` = native, no hook). Logs `BUBCFG:`/`BUBSFX:`.

### v1cz
- Bubbles 160 lower than probe per test: spawn `ball-20`.

### v1cy
- Bubbles 170 lower than probe per test: spawn `ball-30`.

### v1cx
- Bubbles 150 lower than probe per test: spawn `ball-10` (visual ≈ `ball-82`, likely under the deck — say the word if invisible).

### v1cw
- Bubbles 100 lower than probe per test: spawn `ball+40`.

### v1cv
- Bubbles 50 lower per test: spawn `ball+90` (v1cu proved 1:1).

### v1cu
- Bubble height probe: spawn `ball+140` (visual must jump way-high if it follows spawn 1:1 — v1ct's 10u was unjudgeable). Decides the position math.

### v1ct
- Bubbles smaller + lower: scale `25.0` → `12.0` (tests if draw size follows), spawn `ball+62` so the visual emerges ~`ball-10` and rises.

### v1cs
- Bubble experiment: append to `board+0x3B00`, native drives (no mod update/render calls). Decides whether Warm-Up drives the bubble list.

### v1cr
- Bubbles visible: render draws at spawn−scale−60 (disassembled, doc note wrong), so spawns move to ball+85 (visual starts at the ball, rises as it shrinks). Render call now passes the dummy arg (`ret 4` exact).

### v1cq
- Bubble crash fix: drive bubbles from `game_update` (update stage, like native board updates). v1cp drove the sprite render from Draw and died at `0001:00078EDD`.

### v1cp
- Tarpit drowning bubbles: real native bubble objects (ctor `0x44FB50` RET `0x10` objdump-verified, update/render/dtor) spawn around the sinking ball (~20%/frame, cap 24), self-driven from `text_render`. New `source/entbub.h`, `BUB:` log lines.

### v1co
- Tarpit cover binds the S6 geom at the S1 point (name hit > home-inside > nearest, GOAL never picked) — works when the plane geom has any name. Plus `S1[N]=` name+pos dump every scan. No quads/geoms = sphere fallback.

### v1cn
- Tarpit diag: when 0 quads match, log lists the level's S6 geom names (`TARQ: S6[N]=`) so the real plane name shows. No behavior change.

### v1cm
- Tarpit covers the whole plane: entry XZ now tests the level S6 quad(s) matching the def name (ball-radius rim samples) instead of the r30 S1 sphere; no quads = sphere fallback. New `source/enttar.h`, `TARQ:` log lines.

### v1cl
- Tarpit zone back to r30/`|dy|`30 (v1cg feel). v1ch r40 caught the deck 38u out (`dx=-32 dz=-21 dy=22`), sinking through the roll-in = the dead second.

### v1ck
- Tarpit onset diagnostics only (no behavior change): entry logs ball offset from S1 (`dx/dy/dz`); in-tar traces `Y/g` every 30f. Decides zone-late vs sink-stalled.

### v1cj
- Tarpit sink starts instantly: in-tar re-asserts `+0x2CC=1` + DWORD `+0x768=0` every frame (standing on solid re-grounded the ball after entry, stalling onset ~1s).

### v1ch
- Tarpit trigger earlier per test: zone r30/|dy|30 → r40/|dy|40 around S1.

### v1cg
- Tarpit fixed per your 2 bullets: zone now touch-only (r30 + |dy|<30, was 150u bubble that fired early); sink-then-die replaces freeze (Y -= 0.25/frame, `+0x2E9=0`, Ball_Respawn 0x405190 at entryY - radius*2.5 = Dizzy SAFESPOT tp). Sound parent fixed to `board+0x878+0x484`. Bass-mod verbatim port.

### v1cf
- Tarpit behaviour: native `N:TARPIT` replication (`ball+0x2CC=1`, `+0x2D0=entryY`, `+0x768=0`, one-shot 3D splash custom-`sound`-or-`gluestuck`, permanent). `"vertices"`/empty/missing/bad mesh = invisible trigger-only at the S1 point; real file = visible solid + trigger. Zone = `proximity` ellipsoid (def 150/1/1/1). Your `Cheesepit` line works as-is in LevelQ.

### v1bf
- Woodbridge behaviour: sinks 50u while ball within 150u, rises back after. Smooth 100 u/s, pause-frozen, native `+0x10D8`/`+0x10E4` reposition path (disasm-verified against `PopCylinder_ctor`/`0x43DED0`/`0x46FBB0`). Edge-only logs (`near -> sinking`, `far -> rising`, `reached`).

### v1be
- Named entities: set jsonc top-level `"Name", { "behaviour", "mesh" }` pairs spawn `REF:<Name>` refs with `Levels/<mesh>` meshes (persistent static solids, same native chain as GRID). Behaviour stored only for now. INIT log now lists `log=` + `set=` paths.

### v1
- Initial release: standalone HB+ GRID-object cycling system, ported from `mknp_custom_entities`.
