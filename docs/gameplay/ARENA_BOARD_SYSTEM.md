# ArenaBoard Arena System

The multiplayer arena battle mode in Hamsterball. Players compete on floating
platforms ("boards") with hazards, trying to knock each other off or survive
the longest.

## Architecture Overview

```
ArenaBoard (base class)
├── ArenaBoard_Warmup_Ctor        → Warm-up Arena (ID 1)
├── ArenaBoard_Beginner_Ctor      → Beginner Arena (ID 2)
├── ArenaBoard_Intermediate_Ctor  → Intermediate Arena (ID 3)
├── ArenaBoard_Dizzy_Ctor         → Dizzy Arena (ID 4)
├── ArenaBoard_Tower_Ctor         → Tower Arena (ID 5)
├── ArenaBoard_Up_Ctor       → Up Arena (ID 6)
├── ArenaBoard_Neon_Ctor     → Neon Arena (ID 7)
├── ArenaBoard_Expert_Ctor   → Expert Arena (ID 8)
├── ArenaBoard_Odd_Ctor      → Odd Arena (ID 9)
├── ArenaBoard_Toob_Ctor     → Toob Arena (ID 10)
├── ArenaBoard_Wobbly_Ctor   → Wobbly Arena (ID 11)
├── BoardLevel_Glass_ctor          → Glass Arena (ID 12)
├── ArenaBoard_Sky_Ctor      → Sky Arena (ID 13)
├── ArenaBoard_WarmupArena_Ctor   → Master Arena (ID 14)
├── ArenaBoard_Impossible_Ctor    → Impossible Arena (ID 15)
└── CollSlices variants (collision mesh slicing)
```

## Tournament Race Order (from Tournament_AdvanceRace 0x427080)

The 15-race tournament sequence, revealed by the switch statement:

| Race | Ctor | Level Name | Size (bytes) | Asset Path |
|------|------|------------|-------------|------------|
| 1 | BoardLevel1_WarmUp_ctor | Warm Up | 0x436C | — |
| 2 | BoardLevel_Beginner_Ctor | Beginner | 0x644C | — |
| 3 | BoardLevel2_Intermediate_ctor | Intermediate | 0x438C | — |
| 4 | BoardLevel3_ctor | Dizzy | 0x4BE0 | — |
| 5 | BoardLevel5_Tower_ctor | Tower | 0x5418 | — |
| 6 | BoardLevel_Up_Ctor | Up | 0x4790 | — |
| 7 | Board_NeonRace_ctor | Neon Race | 0x4394 | — |
| 8 | BoardLevel8_Expert_ctor | Expert | 0x4FD8 | — |
| 9 | BoardLevel9_Odd_ctor | Odd | 0x43B0 | — |
| 10 | BoardLevel_Toob_Ctor | Toob | 0x646C | — |
| 11 | BoardLevel12_Wobbly_ctor | Wobbly | 0x4388 | — |
| 12 | Board_Glass_ctor | Glass | 0x4390 | — |
| 13 | BoardLevel_Sky_Ctor | Sky | 0x47F8 | — |
| 14 | BoardLevel_Master_Ctor | Master | 0x6498 | — |
| 15 | Board_Impossible_ctor | Impossible | 0x4380 | — |

## ArenaBoard Arena Asset Paths

Each arena loads a MeshWorld and CollisionLevel pair from these paths:

| Arena Init Function | Arena Mesh Path | Extra Level |
|---------------------|-----------------|-------------|
| ArenaBoard_Beginner (0x22550) | "ArenaBoard (Beginner Arena)" | Race: "Beginner Race" (internal: Cascade Race) |
| ArenaLevel_Intermediate_Init (0x414180) | levels\arena-intermediate | — |
| ArenaLevel_Dizzy_Init (0x414240) | levels\arena-dizzy | Levels\Level3-Swirl |
| ArenaLevel_Expert_Init (0x414B10) | levels\arena-expert | — |
| ArenaLevel_Neon_Init (0x416F40) | levels\arena-neon | 2x AthenaList transforms + boundary sphere |
| ArenaLevel_Glass_Init (0x417DF0) | levels\arena-Glass | — |
| ArenaLevel_Impossible_Init (0x418540) | levels\arena-impossible | — |
| ArenaLevel_Sky_Init (0x4158C0) | levels\arena-Sky | "PILLAR" object collection |
| ArenaLevel_Master_Init (0x416080) | levels\arena-Master | — |
| ArenaLevel_Toob_Init (0x414F00) | (in ctor) | — |
| ArenaLevel_Odd_Init (0x414CE0) | levels\arena-Odd | — |
| ArenaLevel_Toob_Init (0x414F00) | levels\arena-Toob | 5x "N:BUMPER%d" objects |

### Special Arena Init Logic

**ArenaLevel_Dizzy_Init**: Loads TWO mesh worlds — the arena
(levels\arena-dizzy) and a bonus level (Levels\Level3-Swirl) at `this+0x11F8`.

**ArenaLevel_Sky_Init**: Iterates all objects in the arena MeshWorld looking
for "PILLAR" name prefix (case-insensitive via `__strnicmp`). Adds matching
objects to a pillar list at `this+0x11FB`.

**ArenaLevel_Toob_Init**: Collects 5 bumper objects named "N:BUMPER1" through
"N:BUMPER5" via `Scene_CollectByNameFilter`. Each bumper stored at `this+0x11F8`
(stride 0x106). Bumper count array at `this+0x1716`.

**ArenaLevel_Neon_Init**: Most complex init — loads arena, then applies
Matrix transformations to all objects in two AthenaLists (first at
`this+0xA75`, second at `this+0xB7B`). Each object gets 3 matrix transforms
applied to its position/orientation, with a "visible" check based on whether
the object's float value differs from `DAT_004CF3C8`. Also creates a SceneObject
with 10x10 scale and 400.0 radius for the arena boundary.

## ToggleTimer System

### Timer Structure (0x14 bytes)

| Offset | Type | Field | Description |
|--------|------|-------|-------------|
| +0x00 | vtable* | vtable | Timer vtable (0x4D8E74) |
| +0x04 | byte | active | Timer is running |
| +0x08 | int32 | period | Total ticks before expiry (default: 100) |
| +0x0C | int32 | count | Current tick count |
| +0x10 | byte | expired | Set to 1 when count >= period |

### ToggleTimer_Init (0x458E60)

```c
ToggleTimer_Init(Timer *t) {
    t->vtable = &Timer_Vtable;
    t->active = 0;
    t->period = 100;
    t->count = 0;
}
```

### ToggleTimer_Tick (0x458E90)

```c
ToggleTimer_Tick(Timer *t) {
    t->expired = 0;
    t->count++;
    if (t->count >= t->period) {
        t->count = 0;
        t->active = !t->active;  // Toggle on/off
        t->expired = 1;           // Signal expiry
    }
}
```

### ToggleTimer_Cleanup (0x458E80)

```c
ToggleTimer_Cleanup(Timer *t) {
    t->vtable = &Timer_Vtable;  // Reset to default (deactivate)
}
```

### ArenaBoard_TickDown (0x472A50)

Decrements the tick counter at `this+0x21F*4` (object lifetime counter).
When it reaches < 1, calls `vtable[0x4C]` (cleanup) and `vtable[0x40]` (destructor).

## ArenaObject System

### ArenaSceneObj_Tick (0x42B660)

Per-frame update for arena objects (hammers, saws, etc.):

```c
ArenaSceneObj_Tick(Object *obj) {
    // Animate vertical position (float up)
    obj->float_offset += DAT_004cf524;  // small upward velocity
    if (obj->float_offset > DAT_004cf438)  // max float height
        obj->float_offset = 0.75f;         // clamp

    // Animate cooldown timer (count down)
    obj->cooldown -= DAT_004cf370;  // decrement
    if (obj->cooldown < DAT_004cf368)  // zero threshold
        obj->cooldown = 0;

    // Tick two embedded timers (offset 0x888 and 0x89C)
    ToggleTimer_Tick(obj + 0x888);
    ToggleTimer_Tick(obj + 0x89C);
}
```

Object offsets:
- `+0x880`: float position (vertical bob)
- `+0x884`: cooldown timer (float)
- `+0x888`: Timer struct (0x14 bytes) — primary animation timer
- `+0x89C`: Timer struct (0x14 bytes) — secondary animation timer

## ArenaBoard Render System (0x421910)

### HUD Layout (4-player split screen)

```
+--------+--------+   Screen: 800x600
| P1     | P2     |   Quadrants: 400x300 each
| 75,75  | 725,75 |   Timer font at app->timer_font (offset 0x318/0x328)
+--------+--------+   Shadow font at app->shadow_font (offset 0x318)
| P3     | P4     |
| 75,525 | 725,525|
+--------+--------+
```

### Timer Display

- Main timer: centered at `screen_center - 88`, drawn with large font
- Sub-seconds: `".N"` format (tenths of a second)
- Flashing red when time < 1100 ticks (with expired flag)
- Flashing blue when time < 600 ticks
- "TIE BREAKER!" text at (400, 40) when both `timer_started` and `tie_breaker` flags set

### Player Visibility Flags (in App struct)

| Offset | Flag | Meaning |
|--------|------|---------|
| +0x677 | p2_hidden | Hide P2 quadrant |
| +0x717 | p3_hidden | Hide P3 quadrant |
| +0x7B7 | p4_hidden | Hide P4 quadrant |

## Menu Command System (from GameSelectionManager 0x433AC0)

The title screen dispatches menu commands as string comparisons:

| Command | Description | Menu Created |
|---------|-------------|-------------|
| "BACK" | Return to title | MainMenu_ctor |
| "LOCKED" | Mirror locked dialog | OkayDialog ("NOT UNLOCKED!") |
| "1PT" | 1-Player Tournament | DifficultyMenu or TourneyContinueDialog |
| "1PMT" | 1-Player Mirror Tournament | DifficultyMenu or TourneyContinueDialog |
| "1PP" | 1-Player Practice | PracticeMenu_ctor |
| "1PTT" | 1-Player Time Trial | TimeTrialMenu_ctor |
| "PARTY" | Multiplayer Party | MPMenu_ctor |

### Mirror Tournament Unlock

The mirror tournament requires winning a regular tournament at Normal or
Frenzied difficulty. The unlock check message:

> "THE MIRROR TOURNAMENT ISN'T UNLOCKED YET! TO UNLOCK THE MIRROR TOURNAMENT,
> YOU NEED TO WIN A TOURNAMENT AT NORMAL OR FRENZIED DIFFICULTY!"

### Tournament Save System

- Save file: `DATA\TOURNAMENT.SAV`
- Uses `_check_file_access()` to detect existing save
- If save exists and no game in progress: shows TourneyContinueDialog ("Resume?")
- If no save or new game: shows DifficultyMenu

### App Tournament Flags (App struct offsets)

| Offset | Flag | Meaning |
|--------|------|---------|
| +0x234 | (set 0) | Clear on menu entry |
| +0x235 | is_tournament | 1=tournament mode, 0=practice/time trial |
| +0x236 | is_mirror | 1=mirror tracks (reverse direction) |
| +0x237 | tournament_complete | 1=all 15 races done |
| +0x5D4 | player_count_flag | Set to 1 for single player |
| +0x5D5 | (cleared) | Cleared on tournament start |
| +0x5D7 | (cleared) | Cleared on tournament start |

## Tournament Time Bonus System

### Time Award Per Difficulty (from Tournament_AdvanceRace)

| Difficulty | Base Bonus | Increment |
|-----------|-----------|-----------|
| Normal (0) | +1000 ms | DAT_004CF6F4 (per-race increment) |
| Frenzied (1) | +500 ms | DAT_004CF3D8 (smaller per-race increment) |

### Player Time Tracking

Each player slot (stride 0xA0, up to 4 players at App+0x5E8):

| Player | Current Time | Extra Time | Race Index | Level Name | Active |
|--------|-------------|-----------|------------|------------|--------|
| P1 | App+0x5E8 | App+0x5EC | App+0x60C | App+0x610 | App+0x5D8 |
| P2 | App+0x688 | App+0x68C | App+0x6CC | App+0x6D0 | App+0x678 |
| P3 | App+0x728 | App+0x72C | App+0x76C | App+0x770 | App+0x718 |
| P4 | App+0x7C8 | App+0x7CC | App+0x80C | App+0x810 | App+0x7B8 |

For races 1-2: `current_time = base_time`, `extra_time = 0`
For races 3+: `extra_time = base_time` (adds to existing accumulated time)

### Tournament Object Structure (Tournament_AdvanceRace `this`)

| Offset | Type | Field |
|--------|------|-------|
| +0x04 | int* | App pointer (scene state) |
| +0x08 | int | current_race_index (1-15) |
| +0x0C | void* | current_board_object |
| +0x10 | byte | skip_bonus_flag |
| +0x11 | byte | no_time_flag |
| +0x14 | float[N] | race_times_array (time at start of each race) |
| +0x50 | int[N] | race_times_ms_array (millisecond timestamps) |

## Related Functions

| Address | Name | Purpose |
|---------|------|---------|
| 0x427080 | Tournament_AdvanceRace | Create next race in tournament |
| 0x433AC0 | GameSelectionManager | Title screen command dispatcher |
| 0x422550 | ArenaBoard_Beginner_Ctor | Beginner arena constructor |
| 0x4226E0 | ArenaBoard_Intermediate_Ctor | Intermediate arena ctor |
| 0x422790 | ArenaBoard_Dizzy_Ctor | Dizzy arena ctor |
| 0x423060 | ArenaBoard_Expert_Ctor | Expert arena ctor |
| 0x424860 | ArenaBoard_Neon_Ctor | Neon arena ctor |
| 0x423BF0 | ArenaBoard_Sky_Ctor | Sky arena ctor |
| 0x4234E0 | ArenaBoard_Toob_Ctor | Toob arena ctor |
| 0x423220 | ArenaBoard_Odd_Ctor | Odd arena ctor |
| 0x424EC0 | ArenaBoard_Impossible_Ctor | Impossible arena ctor |
| 0x421910 | ArenaBoard_Render | HUD/timer rendering |
| 0x42B660 | ArenaSceneObj_Tick | Arena hazard animation |
| 0x458E60 | ToggleTimer_Init | Timer struct initializer |
| 0x458E90 | ToggleTimer_Tick | Timer tick + toggle logic |
| 0x458E80 | ToggleTimer_Cleanup | Timer deactivation |
| 0x472A50 | ArenaBoard_TickDown | Lifetime countdown |
| 0x4288B0 | App_StartTournamentRace | Tournament launch entry |
| 0x445230 | Scene_StartTournament | Scene tournament setup |
| 0x446730 | Tourney_SaveTournament | Save tournament state |
| 0x42E060 | GameSelectionScreen_ctor | Tournament results screen |