# Hamsterball — Level Reference

The authoritative list of all 15 race tracks and 15 arenas in Hamsterball.

## Key Facts

- **15 race tracks** (single-player tournament progression)
- **15 arenas** (1 per race track, used in multiplayer Rumble mode)
- Internal file names and XML tags do **NOT** match display names
- `LevelCascade` = Beginner Race (NOT Intermediate, NOT Level 3)
- `BEGINNERRACE` XML tag = Warm-up Race (the very first race)
- `CASCADERACE` XML tag = Beginner Race (the second race)

## Race Tracks (in tournament order)

| # | Display Name | XML Tag | Level File | Checker Color | Board Ctor |
|---|-------------|---------|-----------|---------------|------------|
| 1 | Warm-up Race | `BEGINNERRACE` | `Level1.MESHWORLD` | Pink | `LevelBoard_WarmUp_ctor` (0x41CA40) |
| 2 | Beginner Race | `CASCADERACE` | `LevelCascade.MESHWORLD` | Orange | `LevelBoard_Beginner_ctor` (0x4200E0) |
| 3 | Intermediate Race | `INTERMEDIATERACE` | `Level2.MESHWORLD` | Blue | `LevelBoard_Intermediate_ctor` |
| 4 | Dizzy Race | `DIZZYRACE` | `Level3.MESHWORLD` | Green | `LevelBoard_Dizzy_ctor` |
| 5 | Tower Race | `TOWERRACE` | `Level4.MESHWORLD` | Yellow | `LevelBoard_Tower_ctor` |
| 6 | Up Race | `UPRACE` | `LevelUp.MESHWORLD` | Purple | `LevelBoard_Up_ctor` (0x420390) |
| 7 | Neon Race | `NEONRACE` | `LevelDark.MESHWORLD` | Yellow* | `Board_NeonRace_ctor` |
| 8 | Expert Race | `EXPERTRACE` | `Level5.MESHWORLD` | Red | `LevelBoard_Expert_ctor` |
| 9 | Odd Race | `ODDRACE` | `Level6.MESHWORLD` | Orange | `LevelBoard_Odd_ctor` |
| 10 | Toob Race | `TOOBRACE` | `Level8.MESHWORLD` | Blue | `LevelBoard_Toob_ctor` (0x41F4B0) |
| 11 | Wobbly Race | `WOBBLYRACE` | `Level7.MESHWORLD` | Green | `LevelBoard_Wobbly_ctor` |
| 12 | Glass Race | `GLASSRACE` | `LevelGlass.MESHWORLD` | Pink | `Board_Glass_ctor` |
| 13 | Sky Race | `SKYRACE` | `Level9.MESHWORLD` | Light Blue | `LevelBoard_Sky_ctor` |
| 14 | Master Race | `MASTERRACE` | `Level10.MESHWORLD` | Gray** | `LevelBoard_Master_ctor` |
| 15 | Impossible Race | `IMPOSSIBLERACE` | `LevelImpossible.MESHWORLD` | Red | `Board_Impossible_ctor` |

\* Neon: Technically black and white textures, but yellow lighting makes it appear yellow.
\** Master: Gray floor, brownish walls, black sky.

## Arenas (in tournament order)

| # | Display Name | Arena File | Init Function |
|---|-------------|-----------|---------------|
| 1 | Warm-up Arena | `Arena-WarmUp.MESHWORLD` | `ArenaLevel_WarmUp_Init` (0x413C20) |
| 2 | Beginner Arena | `Arena-Beginner.MESHWORLD` | `ArenaLevel_Beginner_Init` (0x413CE0) |
| 3 | Intermediate Arena | `Arena-Intermediate.MESHWORLD` | `ArenaLevel_Intermediate_Init` (0x414180) |
| 4 | Dizzy Arena | `Arena-Dizzy.MESHWORLD` | `ArenaLevel_Dizzy_Init` (0x414240) |
| 5 | Tower Arena | `Arena-Tower.MESHWORLD` | `ArenaLevel_Tower_Init` (0x4144B0) |
| 6 | Up Arena | `Arena-Up.MESHWORLD` | `ArenaLevel_Up_Init` (0x414960) |
| 7 | Neon Arena | `Arena-Neon.MESHWORLD` | `ArenaLevel_Neon_Init` (0x416F40) |
| 8 | Expert Arena | `Arena-Expert.MESHWORLD` | `ArenaLevel_Expert_Init` (0x414B10) |
| 9 | Odd Arena | `Arena-Odd.MESHWORLD` | `ArenaLevel_Odd_Init` (0x414CE0) |
| 10 | Toob Arena | `Arena-Toob.MESHWORLD` | `ArenaLevel_Toob_Init` (0x414F00) |
| 11 | Wobbly Arena | `Arena-Wobbly.MESHWORLD` | `ArenaLevel_Wobbly_Init` (0x4153A0) |
| 12 | Glass Arena | `Arena-Glass.MESHWORLD` | `ArenaLevel_Glass_Init` (0x417DF0) |
| 13 | Sky Arena | `Arena-Sky.MESHWORLD` | `ArenaLevel_Sky_Init` (0x4158C0) |
| 14 | Master Arena | `Arena-Master.MESHWORLD` | `ArenaLevel_Master_Init` (0x416080) |
| 15 | Impossible Arena | `Arena-Impossible.MESHWORLD` | `ArenaLevel_Impossible_Init` (0x418540) |

**Non-playable arena files:** `Arena-SpawnPlatform.MESHWORLD` (spawn platform),
`Arena-Stands.MESHWORLD` (audience stands) — not counted in the 15.

## Common Confusions

### "LevelCascade" is Beginner Race, NOT Intermediate

The internal file `LevelCascade.MESHWORLD` is used for **Beginner Race** (tournament
position #2). The name "Cascade" comes from the game's internal XML tag `CASCADERACE`.
It has nothing to do with Intermediate Race.

### XML Tags Are Off By One

| XML Tag | Display Name | Level # |
|---------|-------------|---------|
| `BEGINNERRACE` | Warm-up Race | 1 |
| `CASCADERACE` | Beginner Race | 2 |
| `INTERMEDIATERACE` | Intermediate Race | 3 |

The XML tags appear to be "shifted" — `BEGINNERRACE` is the easiest race (Warm-up),
not what we'd call "Beginner" in the display.

### Level File Numbers Don't Match Tournament Order

| Level File | Tournament Position | Display Name |
|-----------|---------------------|-------------|
| `Level1` | 1 | Warm-up Race |
| `LevelCascade` | 2 | Beginner Race |
| `Level2` | 3 | Intermediate Race |
| `Level3` | 4 | Dizzy Race |
| `Level4` | 5 | Tower Race |
| `LevelUp` | 6 | Up Race |
| `LevelDark` | 7 | Neon Race |
| `Level5` | 8 | Expert Race |
| `Level6` | 9 | Odd Race |
| `Level8` | 10 | Toob Race |
| `Level7` | 11 | Wobbly Race |
| `LevelGlass` | 12 | Glass Race |
| `Level9` | 13 | Sky Race |
| `Level10` | 14 | Master Race |
| `LevelImpossible` | 15 | Impossible Race |

## Sub-Levels (Object Prefabs)

These are not standalone race tracks — they are 3D object meshes loaded by race levels:

| File | Purpose | Used By |
|------|---------|---------|
| `Level2-Bridge` | Bridge section | Intermediate Race |
| `Level3-Gluebie` | Glue trap object | Dizzy Race |
| `Level3-Swirl` | Swirl vortex | Dizzy Race / Dizzy Arena |
| `Level3-Tipper` | Tipping platform | Dizzy Race |
| `Level3-WaterWheel` | Water wheel | Dizzy Race |
| `Level4-Catapult` | Catapult launcher | Tower Race |
| `Level4-Drawbridge` | Drawbridge | Tower Race |
| `Level4-Mace` | Swinging mace | Tower Race |
| `Level4-Trapdoor1/2` | Trapdoor variants | Tower Race |
| `Level4-Turret` | Turret | Tower Race |
| `Level4-Windmill` | Windmill | Tower Race |
| `Level5-Bridge` | Collapsible bridge | Expert Race |
| `Level6-Lifter` | Lifting platform | Odd Race |
| `Level7-Wobbly1-8` | Wobbly platforms | Wobbly Race |
| `Level8-BlockDawg1/2` | Block-dawg obstacle | Toob Race |
| `Level8-Fallout` | Falling obstacle | Toob Race |
| `Level8-Saw` | Sawblade obstacle | Toob Race |
| `Level8-Spinny` | Spinning obstacle | Toob Race |
| `Level9-PopCylinder1/2` | Pop cylinder | Sky Race |
| `Level9-TrapDoor` | Trapdoor | Sky Race |
| `Level10-Bridge1/2` | Bridge variants | Master Race |
| `Level10-2PBridge` | 2-player bridge | Master Race |
| `LevelDark-DFloor1-4` | Dark floor sections | Neon Race |
| `LevelDark-FlickRing` | Flickering ring | Neon Race |
| `LevelDark-NeonPlatform` | Neon platform | Neon Race |
| `LevelDark-Trode` | Electrode | Neon Race |
| `LevelImpossible-BigGear` | Big gear | Impossible Race |
| `LevelImpossible-Gear` | Gear | Impossible Race |
| `LevelImpossible-Looper` | Looper | Impossible Race |
| `LevelImpossible-Pendulum` | Pendulum | Impossible Race |
| `LevelImpossible-Rotator` | Rotator | Impossible Race |
| `LevelUp-Button` | Button trigger | Up Race |
| `LevelUp-Lifter` | Lifting platform | Up Race |
| `LevelUp-SpeedCylinder` | Speed boost | Up Race |
| `Secret` | Secret area | Various |
| `Secret-Unlock` | Unlock spot | Various |
| `PopupSign` | Popup sign object | Various |
| `MouseTrap` | Mousetrap object | Various |
