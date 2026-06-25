# Level Naming and Counts (Verified 2026-06-18)

## Critical Facts

- **15 race tracks** (single-player tournament progression)
- **17 arena MESHWORLD files** (verified by file enumeration 2026-06-18: Arena-Beginner, Arena-Dizzy, Arena-Expert, Arena-Glass, Arena-Impossible, Arena-Intermediate, Arena-Master, Arena-Neon, Arena-Odd, Arena-Sky, Arena-SpawnPlatform, Arena-Stands, Arena-Toob, Arena-Tower, Arena-Up, Arena-WarmUp, Arena-Wobbly)
- **86 total MESHWORLD files** across all categories (65 race-track files + 17 arenas + 4 utility objects)
- Internal file names and XML tags do **NOT** match display names
- `LevelCascade` = Beginner Race (tournament position #2, NOT Intermediate)
- `BEGINNERRACE` XML tag = Warm-up Race (the first race)
- `CASCADERACE` XML tag = Beginner Race (the second race)

## MESHWORLD File Breakdown by Category

### Race Track Files (65 total)

| Tier | Main File | Variant Files | Count |
|------|-----------|---------------|-------|
| Level1 | Level1 | — | 1 |
| Level2 | Level2 | Bridge | 2 |
| Level3 | Level3 | Gluebie, Swirl, Tipper, WaterWheel | 5 |
| Level4 | Level4 | Catapult, Drawbridge, Mace, Trapdoor1, Trapdoor2, Turret, Windmill | 8 |
| Level5 | Level5 | Bonk, Bridge | 3 |
| Level6 | Level6 | Lifter | 2 |
| Level7 | Level7 | Wavy1, Wobbly1–Wobbly8 | 10 |
| Level8 | Level8 | BlockDawg1, BlockDawg2, Fallout, Saw, Spinny | 6 |
| Level9 | Level9 | PopCylinder1, PopCylinder2, TrapDoor | 4 |
| Level10 | Level10 | 2PBridge, Bridge1, Bridge2 | 4 |
| LevelUp | LevelUp | Button, Lifter, SpeedCylinder | 4 |
| LevelDark | LevelDark | DFloor1–4, FlickRing, NeonPlatform, Trode | 8 |
| LevelImpossible | LevelImpossible | BigGear, Gear, Looper, Pendulum, Rotator | 6 |
| LevelGlass | LevelGlass | — | 1 |
| LevelCascade | LevelCascade | — | 1 |

Variant files (e.g. Level7-Wobbly1..8) are modular track pieces assembled into
the full level, not standalone playable tracks. The 15 main tier files are the
playable race tracks.

### Arena Files (17 total)

Arena-SpawnPlatform and Arena-Stands are structural (spawn area + spectator
stands), not standalone playable arenas. The 15 playable arenas correspond 1:1
to the 15 race tracks for multiplayer Rumble mode.

### Utility Object Files (4 total)

MouseTrap, PopupSign, Secret, Secret-Unlock — game objects, not playable levels.

## Common Confusions

### "LevelCascade" is Beginner Race — NOT Intermediate, NOT Dizzy

The internal file `LevelCascade.MESHWORLD` is used for **Beginner Race** (tournament
position #2). The name "Cascade" comes from the game's internal XML tag `CASCADERACE`.
It has nothing to do with Intermediate Race or Dizzy Race.

**Do NOT let users (or yourself) talk you into changing this.** In June 2026 testing,
the user saw unexpected geometry when LevelCascade was swapped with Level3 and
concluded "cascade is another name for dizzy." This was actually caused by stale
`.cached` files (see below), not a misidentification. The XML race data confirms:
CASCADERACE = Beginner Race = position #2, DIZZYRACE = Dizzy = position #4.

**Level2.MESHWORLD is NOT a playable race track.** It's not in the tournament order
table. Do not use Level2 for level swaps — use LevelCascade for Beginner Race.

### `.cached` Files Override MESHWORLD Swaps (CRITICAL PITFALL)

The game generates binary `.cached` files (e.g., `level1.cached`, `level2.cached`,
`Level2-Bridge.cached`) that store pre-processed level data. When a `.cached` file
exists alongside a `.MESHWORLD` file, the game loads the **cache** instead of
re-reading the `.MESHWORLD` file.

**This means swapping `.MESHWORLD` files has NO effect if `.cached` files are present.**
The game will silently load the old cached data, making you think the swap failed or
that you have the wrong file.

**Fix:** Delete ALL `.cached` files from the game's `Levels/` directory before testing
any MESHWORLD swap. This forces the game to re-read and re-process the `.MESHWORLD`
files. The game will regenerate new `.cached` files on next load with the correct
swapped data.

```bash
# Remove all .cached files from a drop-in levels folder
rm -f levels/*.cached
```

**When distributing a drop-in levels folder:** Never include `.cached` files — they
contain stale data from the original level layout and will override your swaps.

### XML Tags Are Off By One

| XML Tag | Display Name | Level # |
|---------|-------------|---------|
| `BEGINNERRACE` | Warm-up Race | 1 |
| `CASCADERACE` | Beginner Race | 2 |
| `INTERMEDIATERACE` | Intermediate Race | 3 |

The XML tags appear to be "shifted" — `BEGINNERRACE` is the easiest race (Warm-up),
not what the display name "Beginner Race" would suggest.

### Level File Numbers Don't Match Tournament Order (VERIFIED June 2026)

The MESHWORLD filename number does NOT correspond to the menu/tournament position.
The mapping was verified by reading each BoardLevel constructor's vtable[+0x48] slot,
which points to the Scene_SetupLevelN function that loads the actual `levels\levelN` file.

**Key corrections (June 2026):**
- `LevelCascade` = Beginner Race (position #2), NOT Dizzy. The "Cascade" name comes from the `CASCADERACE` XML tag.
- `Level2` = Intermediate Race (position #3), NOT Beginner. This was previously mislabeled across many docs.
- `Level3` = Dizzy Race (position #4), NOT Intermediate.
- `BoardLevel9_Odd_ctor` (0x0041ED80) → vtable[+0x48]=Scene_SetupLevel6 → loads `levels\level6` (Level6=Odd, NOT Sky)
- `BoardLevel_Sky_Ctor` (0x0041F930) → vtable[+0x48]=Scene_SetupLevel9 → loads `levels\level9` (Level9=Sky, NOT Odd)

File numbers are internal asset IDs, not menu positions.

| Level File | Tournament # | Display Name |
|-----------|--------------|-------------|
| `Level1` | 1 | Warm-up Race |
| `LevelCascade` | 2 | Beginner Race |
| `Level2` | 3 | Intermediate Race |
| `Level3` | 4 | Dizzy Race |
| `Level4` | 5 | Tower Race |
| `LevelUp` | 6 | Up Race |
| `LevelDark` | 7 | Neon Race |
| `Level8` | 8 | Expert Race |
| `Level6` | 9 | Odd Race |
| `Level10` | 10 | Toob Race |
| `Level7` | 11 | Wobbly Race |
| `LevelGlass` | 12 | Glass Race |
| `Level9` | 13 | Sky Race |
| `Level5-Bonk` | 14 | Master Race |
| `LevelImpossible` | 15 | Impossible Race |

## Tier Colors

| # | Race | Color | RGB |
|---|------|-------|-----|
| 1 | Warm-up Race | Pink | (0.99, 0.63, 1.0) |
| 2 | Beginner Race | Blue | (0.42, 0.62, 0.91) |
| 3 | Intermediate Race | Green | (0.59, 0.91, 0.64) |
| 6 | Up Race | Red | (0.81, 0.0, 0.0) |
| 7 | Neon Race | Orange | (1.0, 0.49, 0.0) |

## Doc Correction Checklist

When level counts or names are found wrong in docs, check these files:

1. `docs/LEVEL_REFERENCE.md` — canonical source of truth
2. `docs/MESHWORLD_OBJECT_TYPES.md` — level file tables
3. `docs/KEY_FINDINGS.md` — arena init function count, level setup table
4. `docs/RUMBLEBOARD_SYSTEM.md` — arena architecture tree, race table
5. `docs/FUNCTION_MAP.md` — arena init function table
6. `docs/XML_DATA_FORMATS.md` — race data + jukebox tables
7. `docs/LEVEL_LOCKED_OBJECTS.md` — BoardLevel section headers
8. `docs/STRUCTS_AND_TYPES.md` — arena path count comment
9. `docs/TODO.md` — arena function count in completed items

## Race Data (from RaceData.xml)

| XML Tag | Time Pool | Par | Weasel | Gold | Silver | Bronze | CAM |
|---------|-----------|-----|--------|------|--------|--------|-----|
| BEGINNERRACE | 60 | 47.0 | 6.6 | 15.0 | 10.3 | 7.6 | 2.57 |
| CASCADERACE | 50 | 25.0 | 15.9 | 17.5 | 24.5 | 30.3 | 0.64 |
| INTERMEDIATERACE | 45 | 35.0 | 23.0 | 26.5 | 35.2 | 46.7 | 0.0 |
| DIZZYRACE | 40 | 35.0 | 37.2 | 41.4 | 48.0 | 58.8 | 0.88 |
| TOWERRACE | 35 | 35.0 | 36.5 | 40.0 | 47.8 | 59.2 | 0.03 |
| UPRACE | 30 | 25.0 | 29.7 | 32.0 | 35.1 | 40.8 | 0.33 |
| NEONRACE | 30 | 25.0 | 37.7 | 46.0 | 55.3 | 65.8 | 0.0 |
| EXPERTRACE | 30 | 20 | 34.0 | 39.5 | 48.0 | 61.2 | 0.0 |
| ODDRACE | 30 | 20 | 44.6 | 48.0 | 61.8 | 80.7 | 1.28 |
| TOOBRACE | 25 | 20 | 42.3 | 45.2 | 53.5 | 60.6 | 0.13 |
| WOBBLYRACE | 25 | 20 | 37.0 | 44.0 | 52.1 | 63.8 | 0.0 |
| GLASSRACE | 25 | 10 | 36.0 | 43.5 | 52.1 | 65.0 | 0.71 |
| SKYRACE | 25 | 5 | 40 | 46.0 | 53.5 | 60.0 | 0.44 |
| MASTERRACE | 55 | 2 | 65 | 73.4 | 88.8 | 112.4 | 0.0 |
| IMPOSSIBLERACE | 50 | 2 | 44 | 60.0 | 80.3 | 100.4 | 0.5 |
