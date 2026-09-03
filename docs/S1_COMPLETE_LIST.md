# Complete S1 Object List — Every Race & Arena Level

**Source:** Binary-parsed from `originals/installed/extracted/Levels/*.MESHWORLD` (Section 1 ref points)  
**Method:** `struct.unpack <I` + length-prefixed strings, 6 floats + `has_material` per ref (spec: `docs/VERIFIED_REFS_BY_LEVEL.md` + `docs/REF_LOADING_SYSTEM.md`)  
**Date:** 2026-08-31  
**Scope:** All 15 race levels + 15 arena levels (+ 2 utility meshes). Count = exact S1 entry occurrences, numbered suffixes kept verbatim.

> **S1 vs S6:** Section 1 = object spawn points (bare names like `BRIDGE`, `TIPPER`). Section 6 = geometry behavior modifiers (`N:GOAL`, `E:JUMP` etc.) — **not** listed here. See `MESHWORLD_TECHNICAL_SPEC.md` for the distinction.

---

## 1) Race Levels — Summary (46 unique object types, collapsed)

| # | Race Level | File | Object types (collapsed) | Total object instances |
|---|-------------|------|--------------------------|------------------------|
| L1 | Warm-up | `Level1.MESHWORLD` | *(none — only START/SAFESPOT/FLAG)* | 0 |
| L2 | Beginner | `LevelCascade.MESHWORLD` | *(none)* | 0 |
| L3 | Intermediate | `Level2.MESHWORLD` | `BRIDGE` ×1, `MOUSETRAP` ×3 | 4 |
| L4 | Dizzy | `Level3.MESHWORLD` | `SIGN-TARPIT`×1, `TARBUBBLE`×36 (TarBubble + 01-35), `GLUEBIE`×14 (08,11-13,16,18-26), `TIPPER`×3, `WATERWHEEL`×1, `SWIRL`×1 | 56 |
| L5 | Tower | `Level4.MESHWORLD` | `CATAPULT`×1, `TRAPDOOR`×1, `DRAWBRIDGE`×1, `MACE`×3 (MACE/01/02), `WINDMILL`×1, `CHOMPER`×1, `TURRET`×8 (TURRET/01-05/09 + XTURRET07) + `Point01`×1 | 17 |
| L6 | Up | `LevelUp.MESHWORLD` | `SPEEDCYLINDER`×2, `LIFTER`×3 (LIFTER×2 + LIFTER225), `TIMEBUTTON`×1, `VAC-*`×6 (IN/OUT/VEC ×2) | 12 |
| L7 | Neon | `LevelDark.MESHWORLD` | `DFLOOR`×5 (DFLOOR1×2 + 2/3/4), `TRODE`×14 (02-10,13-15,17), `NEONPLATFORM`×4 (base +01-03) | 23 |
| L8 | Expert | `Level5.MESHWORLD` | `BONK`×1, `FAN`×6 (FAN, FANSLOW, FAN02/04, FAN(SUPER)(UP), FAN(UP)×2), `SAWBLADE`×2 (1/2), `BRIDGE`×4 (1/2 × NEG/POS), `SAW-BREAK`×2, `JUDGE`×5, `BELL`×1, `HAMMERAREA`×5 | 26 |
| L9 | Odd | `Level6.MESHWORLD` | `LIFTER`×1, `JUMPPIPE`×2, `PIPERANDOM`×2, `LAUNCH`×3 (01-03), `SMALLFLAG`×3, `BIGNOTHING`/`SHRINKCENTER`/`CHROMESHADOW`/`TUBE(NOCOLLIDE)01` | 15 |
| L10 | Toob | `Level8.MESHWORLD` | `SPINNY`×1, `SAW`×1, `FALLOUT1`×1, `SAW2`×1, `BLOCKDAWG`×3 (1/2/3), `BRANCH`×8 (A/B × POS01/02 + VECTOR01/02) | 15 |
| L11 | Wobbly | `Level7.MESHWORLD` | `WOBBLY`×8 (1,2×2,3-7), `WAVY`×1 (WAVY1) | 9 |
| L12 | Glass | `LevelGlass.MESHWORLD` | `SMASHER`×2 (1/2) | 2 |
| L13 | Sky | `Level9.MESHWORLD` | `PILLAR`×52 (PILLAR +01-51), `MAGNIFYER`×1, `POPCYLINDER`×17 (1/2 +03-16), `TRAPDOOR`×1, `CLOUDSCAPE`×1 | 72 |
| L14 | Master | `Level10.MESHWORLD` | `BBRIDGE`×2 (1/2), `BLOCKDAWG`×2 (1/2), `BONK`×1, `BRIDGE`×1, `CATAPULT`×1, `GLUEBIE`×7 (base +01-06), `MOUSETRAP`×2, `POPCYLINDER`×10 (0×4,1×2,06-11), `TIPPER`×4, `TARBUBBLE`×54 (TarBubble +01-53), + `EXPLODEHELPER`/`HAMMERAREA`×5/`LAUNCHPOINT` | 90 |
| L15 | Impossible | `LevelImpossible.MESHWORLD` | `LOOPER`×1, `GEAR`×25 (GEAR +01-24), `BIGGEAR`×15 (BIGGEAR +01-14), `ROTATOR`×1, `PENDULUM`×3 (base +01/02), `BIGGEAR-TOUCH`×3 | 48 |

**46 unique base types** across races (matches `VERIFIED_REFS_BY_LEVEL.md`).

---

## 2) Race Levels — Full Instance List (exact S1 strings + counts)

### L1 — Warm-up (`Level1.MESHWORLD`) — 22 S1 refs, 0 objects
Only utility: `START1-1`, `CameraLocus1`, `FLAG`×n, `SAFESPOT`×n. No object refs. *(Current `Level1.MESHWORLD` on disk is tampered with a `cEnt_001 <ENTITY>Chomper</ENTITY>` — not in original file, excluded here.)*

### L2 — Beginner (`LevelCascade.MESHWORLD`) — 63 S1 refs, 0 objects
`START1-1`, `FLAG02/05/06/07`, `BADBALL <CHASE>200</CHASE><HOME>300</HOME><SPINDISTANCE>45</SPINDISTANCE>`×1, `START2-1/2-2`, rest `SAFESPOT` (incl. tab-terminated `SAFESPOT\t` variants).

### L3 — Intermediate (`Level2.MESHWORLD`) — 68 S1 refs
- `BRIDGE`: 1
- `MOUSETRAP`: 3
- Utilities: `FLAG02`×2/`03`/`04`/`05`/`06`/`07`, `BADBALL <CHASE>300</CHASE><HOME>375</HOME>`, `START1-1`, `START2-1/2-2`, `SAFESPOT`/`SAFEPOS`/`SAFESPOT\t` × ~54

### L4 — Dizzy (`Level3.MESHWORLD`) — 144 S1 refs
- `SIGN-TARPIT`: 1
- `TarBubble`: 1, `TarBubble01`–`TarBubble35`: 1 each (35) → **36 total TARBUBBLE**
- `Gluebie08/11/12/13/16/18/19/20/21/22/23/24/25/26`: 1 each (14)
- `Tipper`: 3
- `WaterWheel`: 1
- `SWIRL`: 1
- Utilities: `START1-1`, `BADBALL<CHASE>100</CHASE><HOME>1000</HOME>`, `FLAG02-08`, `START2-1/2-2`, `SAFESPOT`×~75, `SECRET`×2 + `SECRETUNLOCK`×1

### L5 — Tower (`Level4.MESHWORLD`) — 132 S1 refs
- `Catapult`: 1
- `Trapdoor`: 1
- `DRAWBRIDGE`: 1
- `MACE`, `MACE01`, `MACE02`: 1 each
- `WINDMILL`: 1
- `CHOMPER`: 1
- `TURRET`, `TURRET01`, `TURRET02`, `TURRET03`, `TURRET04`, `TURRET05`, `TURRET09`, `XTURRET07`: 1 each (8)
- `Point01`: 1
- Utilities: `FLAG02-15`, `BADBALL <CHASE>150</CHASE><HOME>225</HOME>`, `START1-1`, `START2-1/2-2`, `SAFESPOT`×~100 (incl. `SAFESPOT(A)`/`(B)` variants), `SECRET`×3 + `SECRETUNLOCK`×1

### L6 — Up (`LevelUp.MESHWORLD`) — 85 S1 refs
- `SPEEDCYLINDER`: 1, `SPEEDCYLINDER01`: 1
- `LIFTER`: 2, `LIFTER225`: 1
- `TIMEBUTTON`: 1
- `VAC-IN-1/02`, `VAC-OUT-1/02`, `VAC-VEC-1/02`: 1 each (6 vacuum markers)
- Utilities: `FLAG02/03`, `START1-1`, `START2-1/2-2`, `SAFESPOT`×~70, `SECRETUNLOCK`×1

### L7 — Neon (`LevelDark.MESHWORLD`) — 147 S1 refs
- `DFLOOR1`: 2, `DFLOOR2/3/4`: 1 each (5)
- `TRODE02/03/04/05/06/07/08/09/10/13/14/15/17`: 1 each (13, note TRODE01/11/12/16 missing)
- `NEONPLATFORM`, `NEONPLATFORM01/02/03`: 1 each (4)
- Utilities: `START1-1`, `BADBALL <CHASE>200</CHASE><HOME>250</HOME>`, `START2-1/2-2`, `SAFESPOT`×~120, `SECRET`/`SECRETUNLOCK`

### L8 — Expert (`Level5.MESHWORLD`) — 128 S1 refs
- `Bonk`: 1
- `FAN(SUPER)(UP)`: 1, `FAN(UP)`: 2, `FAN02`: 1, `FAN04`: 1, `FANSLOW`: 1 (6 FAN variants)
- `SAWBLADE1/2`: 1 each
- `BRIDGE1(NEG)/(POS)`, `BRIDGE2(NEG)/(POS)`: 1 each (4)
- `SAW1-BREAK`/`SAW2-BREAK`: 1 each
- `JUDGE`, `JUDGE01-04`: 1 each (5)
- `BELL`: 1
- `HAMMERAREA1/2/3/4` + `HAMMERSAFESMACK`: 1 each (5, hammer system markers)
- Utilities: `START1-1`, `FLAG02/03/04/06/07/08/09/10/11/12/13/14`, `START-DEBUG01-06`, `START2-1/2-2`, `SAFESPOT`×~80 (incl. `(B)/(C)/(D)/(E)` variants), `SECRETUNLOCK`

### L9 — Odd (`Level6.MESHWORLD`) — 170 S1 refs
- `LIFTER`: 1
- `JUMPPIPE1/2`: 1 each
- `PIPERANDOM1/2`: 1 each
- `LAUNCH01/02/03`: 1 each
- `SMALLFLAG01/02/03`: 1 each
- `BIGNOTHING`, `SHRINKCENTER`, `CHROMESHADOW`, `TUBE(NOCOLLIDE)01`: 1 each
- Utilities: `START1-1`, `FLAG02/03/04/06/07/08/09/10`, `BADBALL`×2 with `<SIZE>` tags, `START2-1/2-2`, `SAFESPOT`×~140 (incl. `[X]`/`[Z]`/`(A)/(B)/(C)` rail markers), `SECRET`/`SECRETUNLOCK`

### L10 — Toob (`Level8.MESHWORLD`) — 105 S1 refs
- `SPINNY`: 1
- `SAW`: 1, `SAW2`: 1, `FALLOUT1`: 1
- `BLOCKDAWG1/2/3`: 1 each
- `BRANCH(A)-POS01/02`, `BRANCH(A)-VECTOR01/02`, `BRANCH(B)-POS01/02`, `BRANCH(B)-VECTOR01/02`: 1 each (8 branch teleport markers)
- Utilities: `START1-1`, `BADBALL <CHASE>250</CHASE><HOME>400</HOME><SPINDISTANCE>45</SPINDISTANCE>`, `FLAG02/04/05/06/07/08`, `START2-1/2-2`, `SAFESPOT`×~80, `SECRET`/`SECRETUNLOCK`

### L11 — Wobbly (`Level7.MESHWORLD`) — 82 S1 refs
- `WOBBLY1`/`2`/`2`/`3`/`4`/`5`/`6`/`7`: 1 each except `WOBBLY2` appears twice (8 total)
- `WAVY1`: 1
- Utilities: `START1-1`, `NOTHING`×2, `FLAG02/03/04/05/06/07/08/09/10/11/12/13/14/15/16/17/18` (incl. `START-DEBUGX`), `START2-1/2-2`, `SAFESPOT`×~55, `SECRETUNLOCK`

### L12 — Glass (`LevelGlass.MESHWORLD`) — 87 S1 refs
- `SMASHER1/2`: 1 each (2, note `PSAFESPOT\t` with leading P is a typo entry, not an object)
- Utilities: `START1-1`, `START2-1/2-2`, `BADBALL<CHASE>200</CHASE><HOME>300</HOME>`, `FLAG02/03/04/05/06/09/10`, `START-DEBUGX`, `SAFESPOT`×~70, `SECRET`×2 + `SECRETUNLOCK`

### L13 — Sky (`Level9.MESHWORLD`) — 157 S1 refs
- `PILLAR`, `PILLAR01-51`: 1 each (52)
- `MAGNIFYER`: 1
- `POPCYLINDER1/2`, `POPCYLINDER03-16`: 1 each (16)
- `TRAPDOOR`: 1
- `CLOUDSCAPE`: 1
- Utilities: `Start1-1` (lowercase s), `FLAG02/04/05/06/07`, `FollowBallSpot`, `Start2-1/2-2`, `SAFESPOT`×~80 (incl. `(B)/(C)/(D)/(E)/(F)/(G)/(H)`), `SECRETUNLOCK`

### L14 — Master (`Level10.MESHWORLD`) — 224 S1 refs (largest race file)
- `BBRIDGE1/2`: 1 each
- `BLOCKDAWG1/2`: 1 each
- `Bonk`: 1
- `BRIDGE`: 1
- `Catapult`: 1
- `Gluebie`, `Gluebie01-06`: 1 each (7)
- `MOUSETRAP`: 2
- `POPCYLINDER0`×4, `POPCYLINDER1`×2, `POPCYLINDER06-11`: 1 each (10)
- `Tipper`: 4
- `TarBubble`, `TarBubble01-53`: 1 each (54)
- `EXPLODEHELPER`, `HAMMERAREA1-4`+`HAMMERSAFESMACK`, `LAUNCHPOINT`, `NOTHING01/02`: extras
- Utilities: `START1-1`, `BADBALL`, `FLAG02-08`, `START2-1/2-2`, `SAFESPOT`×~110, `SECRETUNLOCK`

### L15 — Impossible (`LevelImpossible.MESHWORLD`) — 107 S1 refs
- `LOOPER`: 1
- `GEAR`, `GEAR01-24`: 1 each (25)
- `BIGGEAR`, `BIGGEAR01-14`: 1 each (15)
- `ROTATOR`: 1
- `PENDULUM`, `PENDULUM01/02`: 1 each (3)
- `BIGGEAR-TOUCH`×3: 1 each (3, touch triggers)
- Utilities: `START1-1`, `FLAG02/03/04/06/07/08/09/10/11/12/13/14/15/16/17`, `START2-1/2-2`, `SAFESPOT`×~40

---

## 3) Arena Levels — Full List

All arenas contain the arena scaffold: `PLATFORM` (floor tiles, 4-8 instances), `STANDS` (seating, 1 instance, absent in a few), `CAMERALOOKAT` (1), `SAFESPOT` (4-8), `START2-1` through `START2-4` (4 spawns). Object refs below are *additional* to that scaffold.

| Arena File | Total S1 | Base scaffold | Extra object instances |
|------------|----------|---------------|------------------------|
| `Arena-WarmUp.MESHWORLD` | 16 | PLATFORM×5, STANDS×1, CAMERALOOKAT×1, SAFESPOT×5, START2×4 | *(none)* |
| `Arena-Beginner.MESHWORLD` | 17 | PLATFORM×5, STANDS×1, CAMERALOOKAT×1, SAFESPOT×5, START2×4 | `BADBALL<CHASE>100</CHASE><HOME>400</HOME>`×1 |
| `Arena-Intermediate.MESHWORLD` | 26 | PLATFORM×8, STANDS×1, CAMERALOOKAT×1, SAFESPOT×8, START2×4 | `MOUSETRAP`×4 |
| `Arena-Dizzy.MESHWORLD` | 17 | PLATFORM×5, STANDS×1, CAMERALOOKAT×1, SAFESPOT×5, START2×4 | `SPINNY`×1 |
| `Arena-Tower.MESHWORLD` | 23 | PLATFORM×4, SAFESPOT×4, CAMERALOOKAT×1, START2×4 | `MACE`×4, `CATAPULT`×1, `TURRET`×5 (base +01-04) — STANDS absent (tower arena uses no stands) |
| `Arena-Up.MESHWORLD` | 15 | PLATFORM×4, STANDS×1, CAMERALOOKAT×1, SAFESPOT×4, START2×4 | `LIFTER86`×1 |
| `Arena-Neon.MESHWORLD` | 17 | PLATFORM×5, SAFESPOT×5, CAMERALOOKAT×1, START2×4 | `FLICKRING`×1, `BADBALL`×1 — STANDS absent |
| `Arena-Expert.MESHWORLD` | 19 | PLATFORM×4, STANDS×1, CAMERALOOKAT×1, SAFESPOT×4+`SAFESPOT\t`×1, START2×4 | `FAN`×1 + `FAN01/02/03`×1 each (4) |
| `Arena-Odd.MESHWORLD` | 16 | PLATFORM×5, STANDS×1, CAMERALOOKAT×1, SAFESPOT×5, START2×4 | *(none)* |
| `Arena-Toob.MESHWORLD` | 16 | PLATFORM×5, STANDS×1, CAMERALOOKAT×1, SAFESPOT×5, START2×4 | *(none)* |
| `Arena-Wobbly.MESHWORLD` | 17 | PLATFORM×5, STANDS×1, CAMERALOOKAT×1, SAFESPOT×5, START2×4 | `WOBBLY1`×1 |
| `Arena-Glass.MESHWORLD` | 16 | PLATFORM×5, SAFESPOT×5, CAMERALOOKAT×1, START2×4 | `BADBALL<CHASE>100</CHASE><HOME>400</HOME>`×1 — STANDS absent |
| `Arena-Sky.MESHWORLD` | 87 | PLATFORM×5, STANDS×1, CAMERALOOKAT×1, SAFESPOT×5, START2×4 | `POPCYLINDER01-36`×1 each (36) + `EDGECYLINDER01-24`×1 each (24) + `PILLAR`+`PILLAR01-10`×1 each (11) — *by far the densest arena* |
| `Arena-Master.MESHWORLD` | 16 | PLATFORM×5, STANDS×1, CAMERALOOKAT×1, SAFESPOT×5, START2×4 | *(none)* |
| `Arena-Impossible.MESHWORLD` | 18 | PLATFORM×5, STANDS×1, CAMERALOOKAT×1, SAFESPOT×5, START2×4 | `GEAR`×1 + `GEAR01`×1 |
| `Arena-SpawnPlatform.MESHWORLD` | 0 | — | *(utility mesh, no S1 refs)* |
| `Arena-Stands.MESHWORLD` | 0 | — | *(stands mesh, no S1 refs)* |

---

## 4) Reverse Index — Object → Levels (base names, collapsed)

| Object (base) | Race levels | Arena levels |
|---------------|-------------|--------------|
| BBRIDGE | Master | — |
| BELL | Expert | — |
| BIGGEAR | Impossible | — |
| BLOCKDAWG | Toob, Master | — |
| BONK | Expert, Master | — |
| BRIDGE | Intermediate, Expert, Master | — |
| CATAPULT | Tower, Master | Tower |
| CHOMPER | Tower | — |
| CLOUDSCAPE | Sky | — |
| DFLOOR | Neon | — |
| DRAWBRIDGE | Tower | — |
| EDGECYLINDER | — | Sky |
| FALLOUT1 | Toob | — |
| FAN / FANSLOW | Expert | Expert |
| FLICKRING | — | Neon |
| GEAR | Impossible | Impossible |
| GLUEBIE | Dizzy, Master | — |
| JUDGE | Expert | — |
| LAUNCH | Odd | — |
| LIFTER | Up, Odd | Up (LIFTER86) |
| LOOPER | Impossible | — |
| MACE | Tower | Tower |
| MAGNIFYER | Sky | — |
| MOUSETRAP | Intermediate, Master | Intermediate |
| NEONPLATFORM | Neon | — |
| PENDULUM | Impossible | — |
| PILLAR | Sky | Sky |
| POPCYLINDER | Sky, Master | Sky |
| ROTATOR | Impossible | — |
| SAW / SAW2 | Toob | — |
| SAW-BREAK | Expert | — |
| SIGN-TARPIT | Dizzy | — |
| SMASHER | Glass | — |
| SPINNY | Toob | Dizzy, — |
| SWIRL | Dizzy | — |
| TARBUBBLE | Dizzy, Master | — |
| TIMEBUTTON | Up | — |
| TIPPER | Dizzy, Master | — |
| TRAPDOOR | Tower, Sky | — |
| TRODE | Neon | — |
| TURRET | Tower | Tower |
| WATERWHEEL | Dizzy | — |
| WAVY | Wobbly | — |
| WINDMILL | Tower | — |
| WOBBLY | Wobbly | Wobbly |

Plus utility/system markers present everywhere: `START*`, `SAFESPOT`/`SAFEPOS`/`SAFESPOT(A/B/C/D/E/F/G/H/Q/X/Z/\t)`, `FLAG02-18`, `START-DEBUG*`, `SECRET`/`SECRETUNLOCK`, `BADBALL` (AI balls, 1-2 per level), `CAMERALOOKAT`+`PLATFORM`+`STANDS` (arena scaffold).

---

## 5) Notes & Caveats

- `Level1.MESHWORLD` on disk is currently overwritten with a custom build (87 KB, 23 refs including `cEnt_001`). Original Warm-up has 0 object refs — the tampered entry is excluded above. Restore from a clean install to re-verify.
- `Arena-Beginner`/`Glass` BADBALL and `Arena-Neon` BADBALL/`FLICKRING` are literal S1 strings including XML — they spawn via the arena board factory but are not drawable objects.
- `VAC-IN/OUT/VEC`, `BRANCH-*-POS/VECTOR`, `HAMMERAREA`, `EXPLODEHELPER`, `LAUNCHPOINT`, `BIGNOTHING`, `Point01` etc. are system markers used by level-specific logic, listed under object instances for completeness.
- `SAFESPOT` has many formatting variants (`\t` suffix, `(A)`, `[X]`, `SAFEPOS` without T, `PSAFESPOT\t` typo) — all counted as the same respawn system.
