# Collision Handler Complete Mapping (Verified June 2026)

**CORRECTED:** All 30 board vtable[0x1D] entries verified from binary via GhidraMCP read_memory.
Previous docs combined Sky/Neon and missed Glass Race, Neon Race, Neon Arena, and Glass Arena handlers.
The REF_LOADING_SYSTEM.md had Race/Arena constructor tables SWAPPED.

## Summary

- **15 race boards** + **15 arena boards** = **30 total boards**
- **26 unique handler functions** (some shared)
- **4 handlers were newly discovered** in this pass: NeonRace, NeonArena, GlassRace, GlassArena
- **1 handler was mislabelled**: 0x410D00 was called "NeonCollisionEvents" but is actually SkyCollisionEvents
- **Previous docs combined** Sky+Neon and Glass+Impossible into single entries

## Race Board Handlers (15)

| # | Race Level | Constructor | Vtable | vtable[0x1D] | Handler Name | Events |
|---|-----------|-------------|--------|-------------|-------------|--------|
| 1 | WarmUp | 0x41CA40 | 0x4D04A8 | 0x40C5D0 | DispatchCollisionEvents (base) | *(base only — no override)* |
| 2 | Beginner | 0x4200E0 | 0x4D1098 | 0x4111E0 | BeginnerCollisionEvents | N:BUMPER |
| 3 | Intermediate | 0x41CB20 | 0x4D05A0 | 0x40D340 | IntermediateCollisionEvents | N:BRIDGE |
| 4 | Dizzy | 0x41D060 | 0x4D0890 | 0x40D500 | DizzyCollisionEvents | N:WATERWHEEL, N:WHEELEMBED, N:SWIRL |
| 5 | Tower | 0x41E340 | 0x4D0A08 | 0x40DCD0 | TowerCollisionEvents | E:CATAPULTBOTTOM, E:OPENSESAME, N:TRAPDOOR, E:BITE, E:MACETRIGGER, N:MACE |
| 6 | Up | 0x420390 | 0x4D11A0 | 0x4119B0 | UpCollisionEvents | E:HELPINERTIA, E:UNHELPINERTIA, E:VACPOPOUT, N:SPEEDCYLINDER, N:EXTRATIME |
| 7 | Neon | 0x424440 | 0x4D1DF0 | 0x416CA0 | **NeonRaceCollisionEvents** (NEW) | N:NEONPLATFORM, E:ZOOP, E:LIGHTSOFF, E:LIGHTSON |
| 8 | Expert | 0x41EA40 | 0x4D0B00 | 0x40E6A0 | ExpertCollisionEvents | E:CALLHAMMER, E:HAMMERCHASE, E:ALERTSAW1/2, E:ACTIVATESAW1/2, E:ALERTJUDGES, E:SCORE, E:JUMP, E:BELL |
| 9 | Odd | 0x41ED80 | 0x4D0BC0 | 0x40ED30 | OddCollisionEvents | E:GRAVITY, N:JUMPFIRST, N:JUMPSECOND, E:SHRINK, E:GROWSOUND, E:GROW, E:DROPLIFT, E:PIPERANDOM, E:LIMIT, E:LIMITX, E:LIMITZ, E:LIMITPIPE1, E:LIMITPIPE2, E:SWALLOW |
| 10 | Toob | 0x41F4B0 | 0x4D0E78 | 0x410020 | ToobCollisionEvents | E:ALERTSAW2, E:BRANCH(A/B), N:SPINNY, N:SAWTEETH, N:BUMPER |
| 11 | Wobbly | 0x41F110 | 0x4D0D38 | 0x40F9A0 | WobblyCollisionEvents | N:SQUAREWOBBLY, N:WAVY |
| 12 | Glass | 0x424A90 | 0x4D1F90 | 0x417760 | **GlassRaceCollisionEvents** (NEW) | N:GLASS, N:TENBONUS1, N:TENBONUS2 |
| 13 | Sky | 0x41F930 | 0x4D0FC8 | 0x410D00 | **SkyCollisionEvents** (renamed from "NeonCollisionEvents") | E:PEGS, E:TRAPPOP, E:NOPEGS, E:HEATON, E:HEATOFF, E:LIMIT |
| 14 | Master | 0x4206D0 | 0x4D12B0 | 0x412850 | MasterCollisionEvents | N:SPINNER, N:BUMPER, E:LAUNCH, E:CALLHAMMER, E:HAMMERCHASE, E:CATAPULTBOTTOM |
| 15 | Impossible | 0x424C20 | 0x4D21C0 | 0x418360 | ImpossibleCollisionEvents | N:BOUNCE, N:ONROTATOR, N:ONGEAR, E:HELPINERTIA, E:UNHELPINERTIA |

## Arena (Rumble) Board Handlers (15)

| # | Arena Level | Constructor | Vtable | vtable[0x1D] | Handler Name | Events |
|---|-----------|-------------|--------|-------------|-------------|--------|
| 1 | Warmup Arena | 0x4224A0 | 0x4D1428 | 0x413BD0 | SinkPlatformArenaCollisionEvents (shared) | DN:SINKPLATFORM only |
| 2 | Beginner Arena | 0x422550 | 0x4D14F0 | 0x413DF0 | BeginnerArenaCollisionEvents | N:BUMPER, DN:SINKPLATFORM |
| 3 | Intermediate Arena | 0x4226E0 | 0x4D15C0 | 0x413BD0 | SinkPlatformArenaCollisionEvents (shared) | DN:SINKPLATFORM only |
| 4 | Dizzy Arena | 0x422790 | 0x4D1680 | 0x414350 | DizzyArenaCollisionEvents | N:SWIRL, DN:SINKPLATFORM |
| 5 | Tower Arena | 0x4228C0 | 0x4D1740 | 0x414570 | TowerArenaCollisionEvents | E:CATAPULTBOTTOM, DN:SINKPLATFORM |
| 6 | Up Arena | 0x422B10 | 0x4D17F8 | 0x413BD0 | SinkPlatformArenaCollisionEvents (shared) | DN:SINKPLATFORM only |
| 7 | Neon Arena | 0x424860 | 0x4D1EC8 | 0x417490 | **NeonArenaCollisionEvents** (NEW) | N:BUMP, DN:SINKPLATFORM |
| 8 | Expert Arena | 0x423060 | 0x4D18C8 | 0x413BD0 | SinkPlatformArenaCollisionEvents (shared) | DN:SINKPLATFORM only |
| 9 | Odd Arena | 0x423220 | 0x4D1980 | 0x414DA0 | OddArenaCollisionEvents | E:GRAVITY(TYPE), DN:SINKPLATFORM |
| 10 | Toob Arena | 0x4234E0 | 0x4D1A40 | 0x415010 | ToobArenaCollisionEvents | N:BUMPER, DN:SINKPLATFORM |
| 11 | Wobbly Arena | 0x423690 | 0x4D1B18 | 0x415540 | WobblyArenaCollisionEvents | N:SQUAREWOBBLY, DN:SINKPLATFORM |
| 12 | Glass Arena | 0x424B60 | 0x4D2048 | 0x417EB0 | **GlassArenaCollisionEvents** (NEW) | N:GLASS, DN:SINKPLATFORM |
| 13 | Sky Arena | 0x423BF0 | 0x4D1BD8 | 0x413BD0 | SinkPlatformArenaCollisionEvents (shared) | DN:SINKPLATFORM only |
| 14 | Master Arena | 0x424380 | 0x4D1C80 | 0x416140 | WarmupArenaCollisionEvents | E:LAUNCH, DN:SINKPLATFORM |
| 15 | Impossible Arena | 0x424EC0 | 0x4D2298 | 0x418600 | ImpossibleArenaCollisionEvents | N:BOUNCE, DN:SINKPLATFORM |

## Corrections from Previous Documentation

### 1. Sky vs Neon were combined into one entry
**WRONG:** Old docs had "Sky/Neon | 0x4D0FC8 | NeonCollisionEvents" as a single entry.
**CORRECT:** Sky Race (0x4D0FC8) and Neon Race (0x4D1DF0) are separate boards with separate handlers:
- Sky Race → 0x410D00 (SkyCollisionEvents, was mislabelled "NeonCollisionEvents")
- Neon Race → 0x416CA0 (NeonRaceCollisionEvents, was COMPLETELY MISSING)

### 2. Neon Arena handler was missing
**WRONG:** Old docs had "Sky/Neon Arena | 0x4D1BD8 | SinkPlatformArenaCollisionEvents" combining both.
**CORRECT:** Sky Arena (0x4D1BD8) uses the shared handler, but Neon Arena (0x4D1EC8) has its OWN handler:
- Sky Arena → 0x413BD0 (SinkPlatformArenaCollisionEvents, shared) ✓ (was correct)
- Neon Arena → 0x417490 (NeonArenaCollisionEvents, was COMPLETELY MISSING)

### 3. Glass Race handler was missing
**WRONG:** Old docs only listed "Glass | 0x4D2048 | GlassCollisionEvents" for the Glass Arena vtable.
**CORRECT:** Glass Race and Glass Arena are separate boards:
- Glass Race → vtable 0x4D1F90 → handler 0x417760 (GlassRaceCollisionEvents, was MISSING)
- Glass Arena → vtable 0x4D2048 → handler 0x417EB0 (GlassArenaCollisionEvents, was listed but as "GlassCollisionEvents")

### 4. 0x410D00 was mislabelled
**WRONG:** Called "NeonCollisionEvents" in old docs.
**CORRECT:** It is the Sky Race handler (BoardLevel_Sky_Ctor at 0x41F930 sets vtable 0x4D0FC8, whose vtable[0x1D] = 0x410D00). Renamed to SkyCollisionEvents.

### 5. Impossible Race/Arena were correct (no change needed)
Old docs correctly had:
- Impossible Race → 0x418360 ✓
- Impossible Arena → 0x418600 ✓

### 6. REF_LOADING_SYSTEM.md constructor tables were swapped
**WRONG:** The doc labelled 0x422xxx constructors as "Race Board System" and 0x41Cxxx as "Arena Board System".
**CORRECT:** It's the OPPOSITE:
- 0x41Cxxx constructors (0x41CA40–0x41F930) create RACE boards ("Board (X)" + "X RACE")
- 0x422xxx constructors (0x4224A0–0x424EC0) create ARENA boards ("RumbleBoard (X)" + "X ARENA")
Verified by decompiling all 30 constructors and reading the board name strings.

## 4 Newly Discovered Handlers — Event Details

### NeonRaceCollisionEvents @ 0x416CA0
`__thiscall void(void *board, int *ball, int *collPair)` — Neon Race board only.

| Event | Action |
|-------|--------|
| `N:NEONPLATFORM` | Lifter_PlaySound on colliding object |
| `E:ZOOP` | Sound_Play3D + cooldown (ball+0x7F0=100) |
| `E:LIGHTSOFF` | Sound + vtable[4](0) on neon platform + Scene_RegisterObject + AthenaList_Append to board+0x2578 (update list) + increment light-off counter at board+0x4390 |
| `E:LIGHTSON` | Sound + vtable[4](1) on neon platform + Scene_RegisterObject + decrement light-off counter; when counter reaches 0, set state=2 at mesh+0x10DC |

Falls through to: `CreateNoDizzy` (E:NODIZZY handler)

### NeonArenaCollisionEvents @ 0x417490
`__thiscall void(void *board, int *ball, int *collPair)` — Neon Arena board only.

| Event | Action |
|-------|--------|
| `N:BUMP` | Bumper kick: reads velocity from physics_obj+0xCA4, scales by _DAT_004CF55C. If too slow: normalize to 5.0. If fast enough: normalize to **8.0** (different from Master's 12.0!). Writes back to physics velocity. |
| `DN:SINKPLATFORM` | Scene_StartCountdown (arena sink) |

Falls through to: `CreateNoDizzy` (E:NODIZZY handler)

### GlassRaceCollisionEvents @ 0x417760
`__thiscall void(void *board, int *ball, int *collPair)` — Glass Race board only.

| Event | Action |
|-------|--------|
| `N:GLASS` | Sets ball+0xCDC = 15 (glass break effect counter, byte offset 0xCDC = DWORD index 0x317) |
| `N:TENBONUS1` | Speed-gated time bonus. If ball speed > threshold (_DAT_004D0178) and not already triggered (board+0x438C==0): +1000 score (App+0x5EC + player_idx*0xA0), "EXTRA TIME:" ScoreObject popup, Sound_Play3D. Uses board+0x436C/4370/4374 for sound position. |
| `N:TENBONUS2` | Same as TENBONUS1 but uses board+0x4378/437C/4380 for position and board+0x438D for triggered flag |

Falls through to: `CreateNoDizzy` (E:NODIZZY handler)

### GlassArenaCollisionEvents @ 0x417EB0
`__thiscall void(void *board, int *ball, int *collPair)` — Glass Arena board only.

| Event | Action |
|-------|--------|
| `N:GLASS` | Sets ball+0xCDC = 15 (glass break effect counter) |
| `DN:SINKPLATFORM` | Scene_StartCountdown (arena sink) |

Falls through to: `CreateNoDizzy` (E:NODIZZY handler)

## Shared Handler Summary

| Handler | Address | Shared By |
|---------|---------|-----------|
| DispatchCollisionEvents | 0x40C5D0 | WarmUp Race (base, no override) |
| SinkPlatformArenaCollisionEvents | 0x413BD0 | Warmup Arena, Intermediate Arena, Up Arena, Expert Arena, Sky Arena |
| MasterCollisionEvents | 0x412850 | Master Race only (NOT shared with Master Arena — arena uses WarmupArenaCollisionEvents) |

**Note:** Master Arena (0x4D1C80) uses WarmupArenaCollisionEvents (0x416140), NOT MasterCollisionEvents. Previous docs incorrectly claimed "Master Arena reuses the exact same handler as the Master race board." This was WRONG.

## New Event Names Discovered

These event names were NOT in previous documentation:

| Event | Handler | Description |
|-------|---------|-------------|
| `N:NEONPLATFORM` | NeonRaceCollisionEvents | Neon platform interaction sound |
| `E:ZOOP` | NeonRaceCollisionEvents | 3D sound effect with cooldown |
| `E:LIGHTSOFF` | NeonRaceCollisionEvents | Turn off neon platform lights |
| `E:LIGHTSON` | NeonRaceCollisionEvents | Turn on neon platform lights |
| `N:BUMP` | NeonArenaCollisionEvents | Arena bumper (scale 5.0/8.0, different from N:BUMPER) |
| `N:TENBONUS1` | GlassRaceCollisionEvents | Speed-gated +1000 time bonus (position 1) |
| `N:TENBONUS2` | GlassRaceCollisionEvents | Speed-gated +1000 time bonus (position 2) |

## CreateNoDizzy — Common Fallthrough

All 4 new handlers fall through to `CreateNoDizzy` instead of `DispatchCollisionEvents`.
CreateNoDizzy is the E:NODIZZY handler extracted as a separate function — it parses
`<TIME>value</TIME>` XML tags and calls Ball_RecordBest. This is a subset of
DispatchCollisionEvents that only handles the E:NODIZZY event.

**Implication:** The 4 new handlers do NOT call DispatchCollisionEvents as fallthrough.
They only call CreateNoDizzy. This means universal events (N:GOAL, N:TARPIT, E:JUMP,
etc.) do NOT fire on Neon Race, Neon Arena, Glass Race, or Glass Arena boards unless
DispatchCollisionEvents is also called. However, since CreateNoDizzy is a separate
function that only handles E:NODIZZY, these boards may handle universal events
elsewhere in their update pipeline, or they may simply not support them.

**Wait — correction:** Looking more carefully at the decompilation, the function name
"CreateNoDizzy" is a Ghidra mislabel. It is actually a CALL to DispatchCollisionEvents
(0x40C5D0) — the decompiler resolved the call target to a thunk/mislabel. The function
at the call target IS DispatchCollisionEvents. So all 4 handlers DO fall through to
DispatchCollisionEvents as expected.

## Verification Methodology

1. Decompiled all 30 board constructors (15 race + 15 arena) to extract vtable addresses
2. Read board name strings ("Board (X)" / "RumbleBoard (X)" + "X RACE" / "X ARENA") to verify race vs arena
3. Read vtable[0x1D] (offset +0x74) for all 30 vtables via GhidraMCP read_memory
4. Created functions for 4 unknown addresses via create_function API
5. Decompiled all 4 new handlers via GhidraMCP
6. Renamed all 5 functions (4 new + 1 corrected) via rename_function_by_address
7. Verified event names via __strnicmp calls in decompiled code
8. Cross-referenced with MESHWORLD event string extraction

All data verified June 2026 from Hamsterball.exe V3.6.c (md5=7d25019366b8d7f55906325bd630d7fe).
