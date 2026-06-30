# Verified MESHWORLD Ref Points by Race Level

**Source**: Parsed directly from the original game's MESHWORLD binary files  
**Location**: `originals/installed/extracted/Levels/*.MESHWORLD`  
**Method**: Binary parser using the official Raptisoft MESHWORLD format spec  
**Date**: 2026-06-23  

## What This Document Is

This is the **ground-truth** list of Section 1 ref point names found in each race level's MESHWORLD file. Every name below was extracted by parsing the actual binary data — not inferred from decompilation or guesswork.

**46 unique object types** exist across the 15 race levels. This corrects earlier documentation that claimed "75 refs" — that number was inflated by incorrectly counting Section 6 entity names (N:/E:/T: geometry behavior modifiers) as if they were Section 1 ref points, and by inventing names that do not exist in any file.

## MESHWORLD Format — Two Separate Systems

A MESHWORLD file has two distinct name systems that should not be conflated:

### Section 1: Ref Points (Object Spawn Points)
- Stored at the top of the file as `[u32 count][count × ref_entry]`
- Each entry: `[string name][float pos.x,z,y][float rot.x,z,y][u32 has_material][material?]`
- These are **object spawn markers** — the game reads them and creates game objects via `Board->vtable[33]` factory dispatch
- Examples: `SPEEDCYLINDER`, `BONK`, `GEAR`, `TIPPER`, `CATAPULT`
- Names are stored **without** N:/E: prefix — bare names only

### Section 6: Entity Names (Geometry Behavior Modifiers)
- Stored inside the octree leaf nodes, attached to individual geometry strips
- Format: `[string name]` per geom inside a leaf cube
- These are **not objects** — they modify the behavior of the mesh geometry they're attached to
- Examples: `N:GOAL`, `E:LIMIT`, `E:JUMP`, `T:NEONARROW(NOCOLLIDE)`, `E:LIGHTSOFF`
- Only Neon Race has Section 6 entity names in its race MESHWORLD file. Other race levels have empty Section 6 entity lists (entity names are more common in Arena MESHWORLD files).

## Utility Refs (Present in Most/All Levels, Not Game Objects)

These refs appear in Section 1 but are **not** game objects — they are structural/utility markers:

| Utility Ref | Purpose |
|-------------|---------|
| `START1-1`, `START2-1`, `START2-2` | Ball spawn points (player 1, player 2 positions) |
| `FLAG02`–`FLAG18` | Checkpoint flags along the race track |
| `SAFESPOT` / `SAFEPOS` | Respawn safe spots (with variants like `SAFESPOT(A)`, `SAFESPOT(B)`, `SAFESPOT[X]`, `SAFESPOT[Z]`) |
| `SAFESPOT\t` | Tab-terminated safespots (same purpose, formatting variant) |
| `SECRET` / `SECRETUNLOCK` | Secret level unlock triggers |
| `BADBALL <CHASE>...</CHASE><HOME>...</HOME>` | AI ball spawn with XML parameters |
| `START-DEBUG01`–`06` / `START-DEBUGX` | Debug spawn points |
| `NOTHING` | Null placeholder ref |
| `CameraLocus1` | Camera focus point |
| `FollowBallSpot` | Camera follow target |
| `SIGN` / `SIGN-*` | Sign objects (handled by separate `Scene_CreateSigns` dispatch) |

## Verified Object Refs Per Race Level

### L1 — Warm-up Race (Level1.MESHWORLD)
*(no object refs — only START, SAFESPOT, and utility refs)*

### L2 — Beginner Race (LevelCascade.MESHWORLD)
*(no object refs — only START, SAFESPOT, FLAG, BADBALL)*

### L3 — Intermediate Race (Level2.MESHWORLD)
- BRIDGE
- MOUSETRAP

### L4 — Dizzy Race (Level3.MESHWORLD)
- SIGN-TARPIT
- TARBUBBLE
- GLUEBIE
- TIPPER
- WATERWHEEL
- SWIRL

### L5 — Tower Race (Level4.MESHWORLD)
- CATAPULT
- TRAPDOOR
- DRAWBRIDGE
- MACE
- WINDMILL
- CHOMPER
- TURRET

### L6 — Up Race (LevelUp.MESHWORLD)
- SPEEDCYLINDER
- LIFTER
- TIMEBUTTON

### L7 — Neon Race (LevelDark.MESHWORLD)
- DFLOOR
- TRODE
- NEONPLATFORM

### L8 — Expert Race (Level5.MESHWORLD)
- BONK
- FAN
- FANSLOW
- SAWBLADE
- BRIDGE
- SAW-BREAK
- JUDGE
- BELL

### L9 — Odd Race (Level6.MESHWORLD)
- LIFTER
- LAUNCH

### L10 — Toob Race (Level8.MESHWORLD)
- SPINNY
- SAW
- FALLOUT1
- SAW2
- BLOCKDAWG

### L11 — Wobbly Race (Level7.MESHWORLD)
- WOBBLY
- WAVY

### L12 — Glass Race (LevelGlass.MESHWORLD)
- SMASHER

### L13 — Sky Race (Level9.MESHWORLD)
- PILLAR
- MAGNIFYER
- POPCYLINDER
- TRAPDOOR

### L14 — Master Race (Level10.MESHWORLD)
- BBRIDGE
- BLOCKDAWG
- BONK
- BRIDGE
- CATAPULT
- GLUEBIE
- MOUSETRAP
- POPCYLINDER
- TIPPER
- TARBUBBLE

### L15 — Impossible Race (LevelImpossible.MESHWORLD)
- LOOPER
- GEAR
- BIGGEAR
- ROTATOR
- PENDULUM

## Reverse Index — Object → Levels

| Object | Race Levels Found In | Count |
|--------|---------------------|-------|
| BBRIDGE | Master | 1 |
| BELL | Expert | 1 |
| BIGGEAR | Impossible | 1 |
| BLOCKDAWG | Toob, Master | 2 |
| BONK | Expert, Master | 2 |
| BRIDGE | Intermediate, Expert, Master | 3 |
| CATAPULT | Tower, Master | 2 |
| CHOMPER | Tower | 1 |
| DFLOOR | Neon | 1 |
| DRAWBRIDGE | Tower | 1 |
| FALLOUT1 | Toob | 1 |
| FAN | Expert | 1 |
| FANSLOW | Expert | 1 |
| GEAR | Impossible | 1 |
| GLUEBIE | Dizzy, Master | 2 |
| JUDGE | Expert | 1 |
| LAUNCH | Odd | 1 |
| LIFTER | Up, Odd | 2 |
| LOOPER | Impossible | 1 |
| MACE | Tower | 1 |
| MAGNIFYER | Sky | 1 |
| MOUSETRAP | Intermediate, Master | 2 |
| NEONPLATFORM | Neon | 1 |
| PENDULUM | Impossible | 1 |
| PILLAR | Sky | 1 |
| POPCYLINDER | Sky, Master | 2 |
| ROTATOR | Impossible | 1 |
| SAW | Toob | 1 |
| SAW-BREAK | Expert | 1 |
| SAW2 | Toob | 1 |
| SAWBLADE | Expert | 1 |
| SIGN-TARPIT | Dizzy | 1 |
| SMASHER | Glass | 1 |
| SPEEDCYLINDER | Up | 1 |
| SPINNY | Toob | 1 |
| SWIRL | Dizzy | 1 |
| TARBUBBLE | Dizzy, Master | 2 |
| TIMEBUTTON | Up | 1 |
| TIPPER | Dizzy, Master | 2 |
| TRAPDOOR | Tower, Sky | 2 |
| TRODE | Neon | 1 |
| TURRET | Tower | 1 |
| WATERWHEEL | Dizzy | 1 |
| WAVY | Wobbly | 1 |
| WINDMILL | Tower | 1 |
| WOBBLY | Wobbly | 1 |

**Total: 46 unique object types across 15 race levels.**

## Refs That Do NOT Exist in Any Race Level File

The following names were listed in earlier documentation as ref points but **do not exist** in any original race MESHWORLD Section 1 data. Some are Section 6 entity names (geometry modifiers), others were fabricated:

- `BUMP`, `N:BUMP`, `N:BUMPER` — do not exist as Section 1 refs (BUMPER is not a ref point; bumper behavior is set via Section 6 entity names)
- `N:GLASS`, `N:TENBONUS1`, `N:TENBONUS2` — Section 6 entity names, not Section 1 refs
- `N:SPINNER`, `N:ONGEAR`, `N:ONROTATOR`, `N:BOUNCE` — Section 6 entity names
- `N:SQUAREWOBBLY`, `N:WAVY`, `N:SPINNY`, `N:SAWTEETH` — Section 6 entity names
- `N:JUMPFIRST`, `N:JUMPSECOND` — do not exist in any file
- `N:WATER`, `N:TARPIT`, `N:GOAL` — Section 6 entity names (N:GOAL appears only in Neon Race Section 6)
- `E:LAUNCH`, `E:JUMP`, `E:ACTION`, `E:LIMIT`, `E:BREAK` — Section 6 entity names
- `EDGECYLINDER` — does not exist in any race file
- `T:SPEEDARROW` — does not exist as a Section 1 ref
- `FLICKNING` — does not exist in any race file (appears in Arena Neon only)
- `N:NEONPLATFORM` — Section 6 entity name
- `VAC-IN`, `VAC-OUT`, `VAC-VEC` — utility refs for camera/vacuum zones in Up Race, not game objects
