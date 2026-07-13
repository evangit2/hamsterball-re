# Hamsterball Race Construction System

## Overview

Races are constructed through a layered pipeline:

1. **Menu Selection** → `App_StartTournamentRace` or `App_StartPracticeRace`
2. **Race Initialization** → `App_StartRace` (cleanup + graphics + music)
3. **Level Dispatch** → `Tournament_AdvanceRace` (switch on race index)
4. **Board Construction** → `LevelBoard_X_ctor` (per-level vtable + meshes + race data)
5. **Scene Setup** → `vtable[0x48]()` (loads level mesh, initializes scene)
6. **Registration** → `Scene_AddObject`

## Entry Points

### Tournament Mode
- `Scene_StartTournament` (0x445230) — creates TourneyMenu (0x111C bytes)
- `App_StartTournamentRace` (0x4288B0) — creates PlayerProfile (0x98 bytes)
- `Tournament_AdvanceRace` (0x427080) — core level dispatcher

### Practice/Time Trial Mode
- `App_StartPracticeRace` (0x428C50) — creates PlayerProfile + BestTimeTracker (0x528 bytes)
- Sets `App+0x23C = 1` (Normal difficulty)
- Sets `profile+0x11 = 1` (practice flag — enables ghost recording)

### Arena Mode
- `ArenaBoard_ctor` (0x4217B0) extends `Board_ctor` with arena-specific fields
- Separate constructor hierarchy: `ArenaBoard_WarmUp_ctor`, `ArenaBoard_Dizzy_ctor`, etc.
- Arena unlocks handled by `CheckArenaUnlock` (0x40ABA0)

## Tournament_AdvanceRace (0x427080)

### Two Modes
- **param_1 != 0** (same-level restart): saves carry-over time → creates TourneyMenu → returns
- **param_1 == 0** (advance): increments race index → dispatches to LevelBoard constructor

### Level Dispatch Table

| Race # | Level | Alloc Size | Constructor | racedata.xml Key |
|--------|-------|-----------|-------------|------------------|
| 1 | Warm-Up | 0x436C | LevelBoard_WarmUp_ctor (0x41CA40) | BEGINNERRACE |
| 2 | Beginner (Cascade) | 0x644C | LevelBoard_Beginner_ctor (0x4200E0) | CASCADERACE |
| 3 | Intermediate | 0x438C | LevelBoard_Intermediate_ctor (0x41CB20) | INTERMEDIATERACE |
| 4 | Dizzy | 0x4BE0 | LevelBoard_Dizzy_ctor (0x41D060) | DIZZYRACE |
| 5 | Tower | 0x5418 | LevelBoard_Tower_ctor (0x41E340) | TOWERRACE |
| 6 | Up | 0x4790 | LevelBoard_Up_ctor (0x420390) | UPRACE |
| 7 | Neon | 0x4394 | LevelBoard_Neon_ctor (0x424440) | NEONRACE |
| 8 | Expert | 0x4FD8 | LevelBoard_Expert_ctor (0x41EA40) | EXPERTRACE |
| 9 | Odd | 0x43B0 | LevelBoard_Odd_ctor (0x41ED80) | ODDRACE |
| 10 | Toob | 0x646C | LevelBoard_Toob_ctor (0x41F4B0) | TOOBRACE |
| 11 | Wobbly | 0x4388 | LevelBoard_Wobbly_ctor (0x41F110) | WOBBLYRACE |
| 12 | Glass | 0x4390 | LevelBoard_Glass_ctor (0x424A90) | GLASSRACE |
| 13 | Sky | 0x47F8 | LevelBoard_Sky_ctor (0x41F930) | SKYRACE |
| 14 | Master | 0x6498 | LevelBoard_Master_ctor (0x4206D0) | MASTERRACE |
| 15 | Impossible | 0x4380 | LevelBoard_Impossible_ctor (0x424C20) | IMPOSSIBLERACE |

### Difficulty Bonus (Timer)
After constructing the board, `Tournament_AdvanceRace` adds bonus time to the base timer:
- **App+0x23C == 0** (Easy): +1000 ms (10 seconds)
- **App+0x23C == 1** (Normal): +500 ms (5 seconds)
- Practice/Time Trial mode uses Normal difficulty (App+0x23C=1)

### Timer Distribution
The timer is distributed to 4 player slots (stride 0xA0):
- Player 1: App+0x5E8 (primary), App+0x5EC (carry-over)
- Player 2: App+0x688, App+0x68C
- Player 3: App+0x728, App+0x72C
- Player 4: App+0x7C8, App+0x7CC

For races 1-2 (index < 3), both primary and carry-over are set.
For races 3+, only the carry-over slot is set.

## LevelBoard Constructor Pattern

Every LevelBoard constructor follows this sequence:
1. **`Board_ctor(this, app)`** — base class init (14 AthenaLists, 2 ToggleTimers, 4 player slots, ghost ball)
2. **Set vtable** — level-specific vtable pointer
3. **Set board name** (+0x868) — debug string (e.g. "Board (Warm-Up)")
4. **Set race name** (+0x29B4) — display string (e.g. "WARM-UP RACE")
5. **Set +0x870** — reference to App child field (skipped by Neon, Glass, Impossible)
6. **Set level color** (+0x1508-0x1514) — Vec3 RGBA via Vec3_Init (R, G, B, A=1.0)
7. **`LoadRaceData(this, "RACENAME")`** — parse racedata.xml for timer/medal thresholds
8. **Set music track** (+0x4344) — string name of background music
9. **Load level-specific meshes** — MeshWorld (0x10D0) and MeshNode (0x18) objects
10. **Set unlock flags** — if practice && !party, set App+0x85X = 1 (see Unlock System)

## Level Color Table (board+0x1508)

These are RGBA float values used for visual elements (likely timer blots), NOT gravity.
All levels use the same downward Y-axis gravity.

| Level | R | G | B | Color |
|-------|---|---|---|-------|
| WarmUp | 1.0 | 0.0 | 1.0 | Magenta |
| Beginner | 1.0 | 0.75 | 0.25 | Orange |
| Intermediate | 0.0 | 0.0 | 1.0 | Blue |
| Dizzy | 0.0 | 1.0 | 0.0 | Green |
| Tower | 1.0 | 0.75 | 0.0 | Yellow-Orange |
| Up | 1.0 | 0.0 | 1.0 | Magenta |
| Neon | 1.0 | 1.0 | 0.0 | Yellow |
| Expert | 1.0 | 0.0 | 0.0 | Red |
| Odd | 1.0 | 0.5 | 0.0 | Orange |
| Toob | 0.5 | 0.5 | 1.0 | Light Blue |
| Wobbly | 0.62 | 0.84 | 0.30 | Yellow-Green |
| Glass | 1.0 | 0.0 | 1.0 | Magenta |
| Sky | 0.0 | 0.5 | 1.0 | Cyan |
| Master | 0.5 | 0.5 | 0.5 | Gray |
| Impossible | 1.0 | 0.0 | 0.0 | Red |

The 4th component (Alpha, +0x1514) is always 1.0, set by Vec3_Init.

## Per-Level Mesh Objects

| Level | MeshWorld (0x10D0) | MeshNode (0x18) | Sprite | Total | Notable Meshes |
|-------|-------|---------|--------|-------|-------|
| WarmUp | 0 | 0 | 0 | 0 | None (mesh loaded by Scene_LoadLevel) |
| Beginner | 0 | 0 | 0 | 0 | None (8 bumper AthenaLists initialized) |
| Intermediate | 2 | 0 | 0 | 2 | Level2-Bridge (tipper) |
| Dizzy | 7 | 0 | 0 | 7 | Tipper, WaterWheel, Swirl, Gluebie + sound channel |
| Tower | 6 | 2 | 0 | 8 | Catapult, Drawbridge, Mace, Windmill, Turret, Chomper, YellowLink |
| Up | 3 | 0 | 0 | 3 | Lifter, SpeedCylinder, Button |
| Neon | 6 | 0 | 0 | 6 | NeonPlatform, DFloor1-4, Trode |
| Expert | 2 | 3 | 0 | 5 | Level5-Bridge + 3x hammyjudge |
| Odd | 0 | 0 | 0 | 0 | None |
| Toob | 5 | 0 | 0 | 5 | Spinny, Saw, Fallout, Blockdawg1-2 |
| Wobbly | 7 | 0 | 0 | 7 | Wobbly1-7 |
| Glass | 0 | 0 | 0 | 0 | None |
| Sky | 3 | 2 | 1 | 6 | SkyPillar, MagnifyingGlass, PopCylinder1-2, Trapdoor, Clouds.png |
| Master | 14 | 0 | 0 | 14 | Reuses meshes from 5 other levels (see below) |
| Impossible | 5 | 0 | 0 | 5 | Looper, Gear, BigGear, Rotator, Pendulum |

### Master Race Mesh Reuse
Master (race 14) reuses meshes from 5 other levels:
- Level2-Bridge (from Intermediate)
- Level3-Tipper (from Dizzy)
- Level9-PopCylinder1-2 (from Sky)
- Level8-Blockdawg1-2 (from Toob)
- Level4-Catapult (from Tower)
- Level3-Gluebie (from Dizzy)
- Level10-Bridge1, Level10-2PBridge, Level10-Bridge2 (new, Master-exclusive)

Master is the only constructor that calls `Level_AssignTexturesAndScales` — needed because
reused meshes would otherwise have wrong textures for the Master level theme.

## Beginner (Cascade) Bumper System

The Beginner race constructor initializes 8 AthenaList slots (each 0x418 bytes)
at board+0x436C via `_eh_vector_constructor_iterator_`. These are NOT water animation
vertex lists — they are **bumper collection lists**.

`Beginner_SetupScene` (vtable[18], 0x4110D0, formerly `Scene_SetupLevelCascade`):
- Loads the level mesh from `"levels\levelcascade"`
- Calls `Level_InitScene` and `vtable[0x80]()` (Board_Setup)
- Loops 8 times, calling `Scene_CollectByNameFilter` with `"N:BUMPER%d"` (BUMPER1-BUMPER8)
- Stores results in the 8 AthenaList slots at board+0x436C (stride 0x418)

## Unlock Flag System

### Registry Persistence
Unlock flags are persisted to the Windows Registry via two functions:
- **`App_SaveSettings`** (0x4284C0, formerly `FUN_004284C0`) — writes all flags to registry
- **`App_LoadSettings`** (0x428160, formerly `PauseGame`) — reads all flags from registry

### Race Unlock Flags

| App Offset | Registry Key | Level | Set By |
|-----------|-------------|-------|--------|
| +0x850 | MirrorTournament | Mirror tournament | Unknown |
| +0x851 | DizzyRace | Dizzy | LevelBoard_Dizzy_ctor |
| +0x852 | TowerRace | Tower | LevelBoard_Tower_ctor |
| +0x853 | UpRace | Up | LevelBoard_Up_ctor |
| +0x854 | ExpertRace | Expert | LevelBoard_Expert_ctor |
| +0x855 | OddRace | Odd | LevelBoard_Odd_ctor |
| +0x856 | ToobRace | Toob | LevelBoard_Toob_ctor |
| +0x857 | WobblyRace | Wobbly | LevelBoard_Wobbly_ctor |
| +0x858 | SkyRace | Sky | LevelBoard_Sky_ctor |
| +0x859 | MasterRace | Master | LevelBoard_Master_ctor |
| +0x863 | NeonRace | Neon | NOT set in constructor (set elsewhere) |
| +0x864 | GlassRace | Glass | NOT set in constructor (set elsewhere) |
| +0x865 | ImpossibleRace | Impossible | NOT set in constructor (set elsewhere) |

### Arena Unlock Flags

| App Offset | Registry Key | Level |
|-----------|-------------|-------|
| +0x85A | DizzyArena | Dizzy arena |
| +0x85B | TowerArena | Tower arena |
| +0x85C | UpArena | Up arena |
| +0x85D | ExpertArena | Expert arena |
| +0x85E | OddArena | Odd arena |
| +0x85F | ToobArena | Toob arena |
| +0x860 | WobblyArena | Wobbly arena |
| +0x861 | SkyArena | Sky arena |
| +0x862 | MasterArena | Master arena |
| +0x866 | NeonArena | Neon arena |
| +0x867 | GlassArena | Glass arena |
| +0x868 | ImpossibleArena | Impossible arena |

### Unlock Condition (Levels 4-14)
The 9 levels with unlock flags in their constructors (Dizzy through Master) use this condition:
```c
if (App+0x23C != 0 && profile+0x10 == 0) {
    App+0x85X = 1;  // unlock next level
}
```
- `App+0x23C != 0`: practice/normal difficulty (not easy tournament)
- `profile+0x10 == 0`: not party mode

This means: completing a race in practice mode (not party) unlocks the next level.

### Neon, Glass, Impossible Unlock Mechanism
Neon (0x863), Glass (0x864), and Impossible (0x865) do NOT set their unlock flags in their
LevelBoard constructors. Their flags ARE read by PracticeMenu_ctor and TourneyMenu_ctor
to show "LOCKED NEON", "LOCKED GLASS", "LOCKED IMPOSSIBLE" when the flag is 0.

The mechanism that sets these three flags is currently unknown. They are not set via
simple byte-write patterns (C6 ?? 63/64/65 08 00 00 01 returns no matches for Glass and
Impossible). They may be set through:
- Medal threshold achievement
- Tournament completion
- `CheckArenaUnlock` (0x40ABA0) for arena variants
- Some other indirect mechanism

### CheckArenaUnlock (0x40ABA0)
Sets arena unlock flags at runtime based on the current race index:
```c
switch (race_index) {
  case 4:  unlock App+0x85A (DizzyArena)
  case 5:  unlock App+0x85B (TowerArena)
  case 6:  unlock App+0x85C (UpArena)
  case 7:  unlock App+0x866 (NeonArena)
  case 8:  unlock App+0x85D (ExpertArena)
  case 9:  unlock App+0x85E (OddArena)
  case 10: unlock App+0x85F (ToobArena)
  case 11: unlock App+0x860 (WobblyArena)
  case 12: unlock App+0x867 (GlassArena)
  case 13: unlock App+0x861 (SkyArena)
  case 14: unlock App+0x862 (MasterArena)
  case 15: unlock App+0x868 (ImpossibleArena)
}
```
Plays a sound and sets `Ball_SetName("ARENA UNLOCKED!")` when a new arena is unlocked.

## PlayerProfile Struct (0x98 bytes)

| Offset | Type | Description |
|--------|------|-------------|
| +0x00 | ptr | vtable |
| +0x04 | ptr | App* |
| +0x08 | int | race_index |
| +0x0C | ptr | current_board_ptr |
| +0x10 | byte | party mode flag |
| +0x11 | byte | practice flag (1=time trial, 0=tournament) |
| +0x14 | int[16] | carry_over_time array (64 bytes) |
| +0x50 | int[16] | score array (64 bytes) |

## LoadRaceData (0x40A120)

Parses `racedata.xml` for per-level configuration:

| Board Offset | XML Tag | Description |
|-------------|---------|-------------|
| +0x2998 | TIME | Base timer (seconds, converted to ms) |
| +0x2990 | (difficulty) | Difficulty bonus (9 - value) |
| +0x2994 | WEASEL | Weasel threshold |
| +0x29A0 | GOLD | Gold medal time |
| +0x29A4 | SILVER | Silver medal time |
| +0x29A8 | BRONZE | Bronze medal time |
| +0x299C | (camera) | Camera locus override (float, -1.0 = none) |
| +0x29AC | (derived) | Active medal threshold (computed from unlock state) |

## Music Tracks

| Level | Music Track |
|-------|-------------|
| WarmUp | Hamster Nation |
| Beginner | Cascade Race |
| Intermediate | Gerbil Groove |
| Dizzy | Dizzy! |
| Tower | Happy Rush |
| Up | Up Race |
| Neon | Neon Theme |
| Expert | Fight! |
| Odd | Ninja Hamster |
| Toob | Rodenthood |
| Wobbly | Hamster Chase |
| Glass | Glass Theme |
| Sky | Bucky Break |
| Master | Master Theme |
| Impossible | Impossible Theme |

## Key Functions

| Function | Address | Description |
|----------|---------|-------------|
| App_StartRace | 0x4287C0 | Base race initialization (cleanup + gfx + music) |
| App_StartTournamentRace | 0x4288B0 | Tournament race entry |
| App_StartPracticeRace | 0x428C50 | Practice/Time Trial entry |
| Tournament_AdvanceRace | 0x427080 | Level dispatcher (switch on race index) |
| Tourney_SetCurrentLevel | 0x4508F0 | Sets current tournament level + timer |
| Board_ctor | 0x419030 | Base board constructor (14 AthenaLists, player slots) |
| Gadget_ctor | 0x4690F0 | Parent class constructor (SceneObject → Gadget → Board) |
| LoadRaceData | 0x40A120 | Parses racedata.xml |
| Level_InitScene | 0x40B090 | Scene setup (sound, camera, music, CAMERALOCUS) |
| Level_AssignTextures | 0x4130E0 | vtable[0x90] — assigns textures from main level mesh |
| Level_AssignTexturesAndScales | 0x411BA0 | Extended texture assignment (used by Master) |
| Scene_SetRaceActive | 0x4366E0 | Sets scene+0x10EC = 1 |
| Scene_ProcessRaceEnd | 0x41A540 | vtable[22] — race end transition |
| Board_UpdateRaceState | 0x41B130 | vtable[19] — per-frame race state machine |
| App_SaveSettings | 0x4284C0 | Writes all settings + unlock flags to registry |
| App_LoadSettings | 0x428160 | Reads all settings + unlock flags from registry |
| CheckArenaUnlock | 0x40ABA0 | Sets arena unlock flags based on race index |
| Beginner_SetupScene | 0x4110D0 | vtable[18] for Beginner — loads bumpers BUMPER1-8 |

## Ghidra Renames Applied This Session

| Old Name | New Name | Address |
|----------|----------|---------|
| Scene_SetupLevelCascade | Beginner_SetupScene | 0x4110D0 |
| FUN_004284C0 | App_SaveSettings | 0x4284C0 |
| PauseGame | App_LoadSettings | 0x428160 |
