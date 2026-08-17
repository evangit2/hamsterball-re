# Hamsterball Arena Scoring System — Modder's Reference

> **Document version:** 2026-06-13  
> **Based on:** Hamsterball.exe decompilation via Ghidra + GhidraMCP  
> **Target audience:** Modders, trainers, reverse engineers  

---

## Table of Contents

1. [Overview](#overview)
2. [Data Structures](#data-structures)
3. [Score Lifecycle](#score-lifecycle)
4. [Timer System](#timer-system)
5. [Win Condition Logic](#win-condition-logic)
6. [How to Edit Scores](#how-to-edit-scores)
7. [Function Reference](#function-reference)
8. [Offsets Quick Reference](#offsets-quick-reference)

---

## Overview

Hamsterball's multiplayer **Arena Mode** (ToggleTimer) uses a per-player scoring system where:

- Each player has a **score value** tracked inside the `ArenaBoard` object
- A **countdown timer** limits round duration (default: 60 seconds)
- When time expires, the player with the **highest score wins**
- If multiple players tie for highest score, a **tie-breaker** round begins
- Falling off the board typically awards points to surviving players

The scoring system is completely separate from the single-player race timer system. Arena scores are stored in the `ArenaBoard` struct, while race times are stored in the `App` struct.

---

## Data Structures

### ArenaBoard Scoring Fields

The `ArenaBoard` inherits from `Board` → `Gadget` → `Scene`. Within the ArenaBoard, these offsets control scoring:

| Offset | Type | Field | Description |
|--------|------|-------|-------------|
| `+0x11ED` | `int32` | `p1_score` | Player 1 current score |
| `+0x11EE` | `int32` | `p2_score` | Player 2 current score |
| `+0x11EF` | `int32` | `p3_score` | Player 3 current score |
| `+0x11F0` | `int32` | `p4_score` | Player 4 current score |
| `+0x47AC` | `int32` | `time_limit` | Countdown duration in ticks (default: 6000 ≈ 60s) |
| `+0x47B0` | `byte` | `timer_started` | 1 = countdown is running |
| `+0x47C4` | `byte` | `tie_breaker` | 1 = tie-breaker active |
| `+0x47C5` | `byte` | `game_over` | 1 = round ended, results showing |
| `+0x11EB` | `int32` | `countdown` | Frames remaining until check |
| `+0x11EC` | `byte` | `timer_toggle` | Internal timer flip-flop |
| `+0x11F1` | `byte` | `results_shown` | 1 = RaceResultsMenu already created |

> **Verified in:** `ArenaBoard_Update` (0x421FE0) and `ArenaBoard_Render` (0x421910)

### App Player Slots

Each player's tournament/race data lives in the `App` struct with a stride of `0xA0`:

| Player | Active Flag | Score/Time | Extra Time | Race Index | Level Name String |
|--------|-------------|-----------|-----------|------------|------------------|
| P1 | `+0x5D8` | `+0x5E8` | `+0x5EC` | `+0x60C` | `+0x610` |
| P2 | `+0x678` | `+0x688` | `+0x68C` | `+0x6CC` | `+0x6D0` |
| P3 | `+0x718` | `+0x728` | `+0x72C` | `+0x76C` | `+0x770` |
| P4 | `+0x7B8` | `+0x7C8` | `+0x7CC` | `+0x80C` | `+0x810` |

In **arena mode**, the `+0x5E8` slot is repurposed as a **score accumulator** rather than a race time counter. The ArenaBoard copies scores into these slots at round end. In race modes, `+0x5E4` (float) is the score accumulated by E:ACTION events, and `+0x5E8` (int centiseconds) is the current race time, counting **up** from 0 (frozen at N:GOAL as the finish time).

> **Verified in:** `PlayerProfile_ctor` (0x426F30), `Tournament_AdvanceRace` (0x427080)

### ScoreObject Structure

Pop-up score notifications (like "EXTRA TIME:") use this struct:

| Offset | Type | Field |
|--------|------|-------|
| `+0x00` | `vtable*` | ScoreObject vtable = 0x4D6C70 |
| `+0x04` | `App*` | app pointer |
| `+0x08` | `void*` | player data pointer |
| `+0x0C` | `App*` | app (duplicate) |
| `+0x10` | `int32` | display value |
| `+0x14` | `int32` | lifetime timer |
| `+0x18` | `byte` | active flag |
| `+0x1C` | `float` | x position |
| `+0x20` | `float` | y position |
| `+0x24` | `float` | scale |
| `+0x2C` | `char*` | label string (e.g., `"EXTRA TIME:"`) |

> **Size:** 0x30 bytes  
> **Ctor:** `ScoreObject_ctor` at 0x44BE80

---

## Score Lifecycle

### 1. Score Initialization

When a Arena board arena is constructed (`ArenaBoard_ctor`, 0x4217B0):

- All four player scores (`+0x11ED`..`+0x11F0`) are initialized to **0**
- `time_limit` (`+0x47AC`) is set to **6000** (60 seconds)
- `timer_started` (`+0x47B0`) is **0** initially

### 2. Score Increment Events

During gameplay, scores increase through these mechanisms:

#### A. Ball Falls Off Board
When a ball falls off the arena platform:
1. `Ball_Shrink` (0x402200) triggers — sets airborne flag, plays sound
2. `Ball_FallUpdate` (0x408830) runs physics until ball hits void
3. The ArenaBoard detects the fallen player and awards **+10 points** to all surviving players via `Rotator_AddBall` (0x43B6F0)

#### B. Bell Collision Bonus
When a ball hits an `E:BELL` event plane:
1. `Bell_Activate` plays the bell sound
2. If not in a race/demo: **+500 bonus time** added to the player's timer (`App+0x5EC`)
3. A `ScoreObject` popup with `"EXTRA TIME:"` text is created and appended to the scene score list (`ArenaBoard+0x8B8`)

> **Source:** `ExpertCollisionEvents` (0x40E6A0), lines 90-108

#### C. Direct ScoreObject Creation
Any code can create a score popup:
```c
void* scoreObj = operator_new(0x30);
ScoreObject_ctor(scoreObj, app, player_data_ptr, "MY LABEL:");
Timer_Decrement(scoreObj);
AthenaList_Append(rumbleBoard + 0x8B8, scoreObj);
```

### 3. Score Storage

`Rotator_AddBall` (0x43B6F0) manages a linked list of score entries:

```c
// Each entry in the list is 8 bytes:
struct ScoreEntry {
    int32_t player_id;   // +0x00
    int32_t score;       // +0x04 (always set to 10)
};
```

- If `player_id` already exists in the list: update its score to **10**
- If new: allocate 8 bytes and append to `ScoreObject+0x10F0` list

> **Note:** This function always sets score = 10. It does not accumulate.

---

## Timer System

### Countdown Timer

The ArenaBoard uses a **tick-based countdown** (not real-time seconds):

| Value | Meaning |
|-------|---------|
| `time_limit = 6000` | ~60 seconds at 100 ticks/sec |
| `time_limit < 1100` | Final 11 seconds — HUD flashes **red** |
| `time_limit < 600` | Final 6 seconds — HUD flashes **blue** |
| `time_limit == 0` | Round over — trigger win calculation |

### Timer Display

`ArenaBoard_Render` (0x421910) draws the timer at screen center:

1. Background quad at `(screen_center - 88, 10, 180, 105)`
2. Main time string in `MM:SS` format (large font)
3. Decimal fraction `".N"` (tenths of a second)
4. Color shifts to red/blue during final seconds

### Tie-Breaker Display

When `tie_breaker` flag (`+0x47C4`) is set:
- `"TIE BREAKER!"` text renders at screen position `(400, 40)`
- Timer continues counting down
- First player to score wins

---

## Win Condition Logic

### Round End Trigger

`ArenaBoard_Update` (0x421FE0) checks every frame:

```c
// Pseudocode from decompilation:
if (timer_started && countdown_expired) {
    // Find maximum score among active players
    max_score = MAX(p1_score, p2_score, p3_score, p4_score);
    
    // Count how many players have max_score
    tie_count = 0;
    for each active player:
        if (player_score == max_score) tie_count++;
    
    if (tie_count >= 2) {
        // TIE — enable tie-breaker mode
        tie_breaker = 1;
        timer_started = 0;  // Actually continues in tie-breaker
    } else {
        // WINNER — show results
        game_over = 0;
        winner_idx = player_with_max_score;
        CreateRaceResultsMenu(rumbleBoard, winner_idx);
        results_shown = 1;
    }
}
```

### Important Logic Details

From the decompilation at 0x421FE0:

1. **Inactive players are skipped** — checks `App+0x5D7`, `+0x677`, `+0x717`, `+0x7B7` (hidden flags)
2. **Tie requires ≥2 players tied** — if only 1 player has max, they win immediately
3. **RaceResultsMenu** is created once — protected by `results_shown` flag (`+0x11F1`)
4. **Audio cue** — plays `"Game Over"` music when results appear

---

## How to Edit Scores

### Method 1: Direct Memory Patching (Recommended for Trainers)

The simplest way to manipulate scores is writing directly to the ArenaBoard fields:

```cpp
// Get App pointer from global
App* app = *(App**)0x004FD680;

// Get current ArenaBoard from active scene
// The ArenaBoard is the current Scene object in arena mode
Scene* scene = app->currentScene;  // App+0x178

// Verify we're in arena mode (ArenaBoard vtable = 0x4D1358)
if (*(uint32_t*)scene == 0x004D1358) {
    // Edit scores directly
    *(int32_t*)((char*)scene + 0x11ED) = 999;  // P1 score = 999
    *(int32_t*)((char*)scene + 0x11EE) = 0;     // P2 score = 0
    *(int32_t*)((char*)scene + 0x11EF) = 0;     // P3 score = 0
    *(int32_t*)((char*)scene + 0x11F0) = 0;     // P4 score = 0
}
```

### Method 2: Hook Rotator_AddBall

Intercept score changes as they happen:

```cpp
// Original: 0x43B6F0 __thiscall Rotator_AddBall(void* this, int player_id)
void __fastcall Hook_Rotator_AddBall(void* scoreObj, int player_id) {
    // Force P1 to always get +100 instead of +10
    if (player_id == 0) {
        // Write directly to the score entry after original processes
        Original_Rotator_AddBall(scoreObj, player_id);
        // Now patch the stored value from 10 to 100
        // (Requires walking the list at scoreObj+0x10F0)
        return;
    }
    Original_Rotator_AddBall(scoreObj, player_id);
}
```

### Method 3: Freeze Timer

Prevent round from ending:

```cpp
// In your per-frame hook (e.g., App_FrameUpdate at 0x46C170):
Scene* scene = app->currentScene;
if (*(uint32_t*)scene == 0x004D1358) {  // ArenaBoard vtable
    *(int32_t*)((char*)scene + 0x47AC) = 6000;  // Reset time_limit to max
    *(int32_t*)((char*)scene + 0x11EB) = 100;    // Reset countdown
}
```

### Method 4: Force Win (Single Player Arena)

End the round immediately with P1 as winner:

```cpp
Scene* scene = app->currentScene;
if (*(uint32_t*)scene == 0x004D1358) {
    // Set P1 to winning score, others to 0
    *(int32_t*)((char*)scene + 0x11ED) = 100;
    *(int32_t*)((char*)scene + 0x11EE) = 0;
    *(int32_t*)((char*)scene + 0x11EF) = 0;
    *(int32_t*)((char*)scene + 0x11F0) = 0;
    
    // Force timer to expire
    *(int32_t*)((char*)scene + 0x47AC) = 0;
    *(int32_t*)((char*)scene + 0x11EB) = 0;
    *(char*)((char*)scene + 0x47B0) = 1;   // timer_started = true
}
```

### Method 5: Edit App Tournament Data

For tournament mode, scores are copied to App struct at round end:

```cpp
// App player data offsets (P1 example):
// +0x5E8 = race time (in arena: repurposed as score)
// +0x5EC = extra_time (bonus accumulates here)

*(int32_t*)((char*)app + 0x5E8) = 0;      // P1 score = 0
*(int32_t*)((char*)app + 0x688) = 99999;  // P2 score = 99999 (win)
```

---

## Function Reference

### Core Scoring Functions

| Address | Name | Signature | Description |
|---------|------|-----------|-------------|
| `0x43B6F0` | `Rotator_AddBall` | `__thiscall (void* this, int player_id)` | Set/add score entry for a player |
| `0x44BE80` | `ScoreObject_ctor` | `__thiscall (void* this, int app, int player_data, char* label)` | Create score popup object |
| `0x434C80` | `ScoreDisplay_SetTime` | `__thiscall (void* this, int time)` | Set displayed time string |
| `0x421FE0` | `ArenaBoard_Update` | `__fastcall (int* this)` | Per-frame update + win check |
| `0x421910` | `ArenaBoard_Render` | `__thiscall (void* this, undefined4)` | Draw HUD/timer |
| `0x4217B0` | `ArenaBoard_ctor` | `__thiscall (void* this, int app)` | Initialize arena with time_limit=6000 |

### Ball / Death Functions

| Address | Name | Description |
|---------|------|-------------|
| `0x402200` | `Ball_Shrink` | Odd Race E:SHRINK collision — shrink ball |
| `0x408830` | `Ball_FallUpdate` | Physics update while falling |
| `0x405190` | `Ball_FindClosestRespawnPoint` | Find respawn after fall |

### Arena Collision

| Address | Name | Description |
|---------|------|-------------|
| `0x40E6A0` | `ExpertCollisionEvents` | Master collision dispatcher |
| `0x434E20` | `Bell_Activate` | Bell hit → bonus time |
| `0x438BB0` | `Hammer_ChaseStart` | Start hammer chase sequence |
| `0x434A50` | `Saw_Activate` | Activate saw blade |
| `0x434770` | `Saw_AlertActivate` | Saw warning ping |

### Results / Menu

| Address | Name | Description |
|---------|------|-------------|
| `0x44CB10` | `RaceResultsMenu_ctor` | Create race results overlay |
| `0x44C260` | `RaceResultPopup_ctor` | "TIME'S UP!" / "OUT OF TIME!" popup |
| `0x44C4B0` | `RaceResults_Tick` | Animate results screen |
| `0x451DF0` | `TourneyMenu_TickWithRank` | Tournament rank calculation |

---

## Offsets Quick Reference

### ArenaBoard Scoring Offsets

```
+0x11ED  int32  p1_score
+0x11EE  int32  p2_score
+0x11EF  int32  p3_score
+0x11F0  int32  p4_score
+0x11EB  int32  countdown_timer
+0x11EC  byte   timer_toggle
+0x11F1  byte   results_shown
+0x47AC  int32  time_limit (default 6000)
+0x47B0  byte   timer_started
+0x47C4  byte   tie_breaker
+0x47C5  byte   game_over
+0x8B8   void*  score_object_list (AthenaList)
```

### App Player Data Offsets

```
// Player 1
+0x5D8  byte   p1_active
+0x5E4  float  p1_score (accumulated by E:ACTION, determines rank)
+0x5E8  int32  p1_race_time (current race time, counts up from 0; frozen at N:GOAL — not a countdown from par)
+0x5EC  int32  p1_extra_time
+0x60C  int32  p1_race_index
+0x610  char*  p1_level_name

// Player 2 (+= 0xA0)
+0x678  byte   p2_active
+0x688  int32  p2_current_time
+0x68C  int32  p2_extra_time

// Player 3 (+= 0x140)
+0x718  byte   p3_active
+0x728  int32  p3_current_time
+0x72C  int32  p3_extra_time

// Player 4 (+= 0x1E0)
+0x7B8  byte   p4_active
+0x7C8  int32  p4_current_time
+0x7CC  int32  p4_extra_time
```

### Vtables for Type Checking

```
0x004D1358  ArenaBoard vtable
0x004CE400  App vtable
0x004D0260  Scene vtable
0x004CF3A0  Ball vtable
```

---

## Modding Tips

### Tip 1: Detect Arena Mode

Check if current scene is a ArenaBoard:
```cpp
Scene* scene = *(Scene**)(app + 0x178);
uint32_t vtable = *(uint32_t*)scene;
bool is_arena = (vtable == 0x004D1358);
```

### Tip 2: Find ArenaBoard from App

In arena mode, `App->currentScene` IS the ArenaBoard:
```cpp
void* rumbleBoard = *(void**)(app + 0x178);
```

### Tip 3: Score Multiplier

Hook `Rotator_AddBall` and multiply all scores:
```cpp
void __fastcall Hook_SetScore(void* this, int player_id) {
    Original_SetScore(this, player_id);
    // Walk the list at this+0x10F0 and multiply by 10
}
```

### Tip 4: Disable Tie-Breaker

Force instant win even on ties:
```cpp
// In ArenaBoard_Update, nop out the tie_count check
// Or simply never let timer expire:
*(int32_t*)(rumbleBoard + 0x47AC) = 999999;
```

### Tip 5: Visual Score Popups

Create custom score notifications:
```cpp
void* popup = operator_new(0x30);
ScoreObject_ctor(popup, app, player_ptr, "CUSTOM BONUS:");
*(int32_t*)(popup + 0x10) = 500;  // Display value
Timer_Decrement(popup);
AthenaList_Append(rumbleBoard + 0x8B8, popup);
```

---

## Files Referenced

| File | Description |
|------|-------------|
| `analysis/ghidra/decompilations/collision/decomp_expert_collisionevents.c` | Arena collision events |
| `analysis/ghidra/decompilations/tournament/decomp_rumbleboard_render.c` | HUD rendering |
| `analysis/ghidra/decompilations/scene/decomp_scene_updateballs.c` | Ball update logic |
| `analysis/ghidra/structs/rumbleboard_struct.h` | ArenaBoard C struct |
| `docs/ARENA_BOARD_SYSTEM.md` | Arena architecture overview |
| `docs/ARENA_HAZARD_SYSTEM.md` | Hazard object documentation |
| `docs/APP_OBJECT.md` | App struct full reference |

---

*Document compiled from live Ghidra decompilation. All offsets verified against Hamsterball.exe build. For questions or corrections, check the raw decompilation files in `analysis/ghidra/decompilations/`.*
