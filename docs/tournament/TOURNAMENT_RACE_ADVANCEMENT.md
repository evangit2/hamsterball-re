# Tournament Race Advancement System

Reverse-engineered analysis of how Hamsterball advances from one race to the next in Tournament mode.

All addresses verified via Ghidra decompilation of Hamsterball.exe V3.6.c.

## Overview

Tournament mode flows through 15 races in a fixed order. The game uses a **PlayerProfile** struct to track race index and scores, a **TourneyMenu** screen between races, and a central `Tournament_AdvanceRace` function that creates each board. The race-end transition is handled by `Scene_HandleCountdown`, which checks a "last race" flag on the board to decide whether to end the tournament.

## Key Functions

| Function | Address | Purpose |
|----------|---------|---------|
| `GameSelectionManager` | `0x433AC0` | Main menu command dispatcher (handles "1PT", "1PMT", "1PP", etc.) |
| `App_StartTournamentRace` | `0x4288B0` | Sets up App flags, creates PlayerProfile, calls AdvanceRace |
| `Tournament_AdvanceRace` | `0x427080` | **Core function**: increments race index, creates next board via switch |
| `Scene_HandleCountdown` | `0x41A540` | Race-end logic: checks timer, last-race flag, routes to advance/menu |
| `TourneyMenu_Advance` | `0x4266F0` | "PLAY!" button handler on between-races screen |
| `TourneyMenu_CreateBoard` | `0x426780` | Arena mode variant of the board creation switch |
| `TourneyMenu_WriteSave` | `0x4264B0` | Saves tournament state to `DATA\TOURNAMENT.SAV` |
| `TourneyMenu_LoadSaveAndShow` | `0x4265A0` | Loads tournament save and resumes |
| `TourneyMenu_GetRaceName` | `0x4264A0` | Returns race name string from pointer table |
| `App_StartRace` | `0x4287C0` | Cleanup: destroys old scene objects, resets gfx state |
| `App_ShowResults` | `0x428060` | Creates MusicPlayer (results jingle) object |

## The Two TourneyMenu Constructors

There are two distinct TourneyMenu constructors that serve different roles:

| Address | Signature | Purpose |
|---------|-----------|---------|
| `0x0044FDA0` | `TourneyMenu_ctor(this, app, mode_flag)` | Between-races menu (PLAY!, ROLLBACK, MAIN MENU) OR win screen |
| `0x00451B90` | `TourneyMenu_ctor(this, app)` | Final win screen (after Impossible Race) |

### Between-Races Menu (0x0044FDA0)

Called by `Tournament_AdvanceRace`, `TourneyMenu_Advance`, and `TourneyMenu_LoadSaveAndShow`. Has a `param_2` mode flag (1 = play tournament music, 0 = don't).

Checks `profile+0x96` (won flag):
- **Won flag NOT set (normal between-races):** Shows "PLAY!" button, "ROLLBACK" button (label "ROLL" or "RXLL" depending on race index), "MAIN MENU" button, and a race description (word-wrapped text from the race name pointer table at 0x4F7148). Also calls `TourneyMenu_WriteSave` to save progress.
- **Won flag IS set (win screen):** No "PLAY!" button. Instead, calculates the player's **rank** by comparing `App+0x5E4` (total score) against threshold values at `0x4F710C`. Loads a rank badge texture (`textures\ranks\%d.jpg` via `AthenaString_Format`). Shows only "MAIN MENU" button.

### Final Win Screen (0x00451B90)

Called directly from `Scene_HandleCountdown` when the Impossible Race ends in tournament mode. This is the actual win screen:
1. Sets `profile+0x96 = 1` (won flag — marks tournament as won)
2. Calls `CRT_remove("DATA\\tournament.sav")` — **deletes the save file** (tournament is over, can't resume)
3. Plays "Main Theme - No Intro" music at 2x speed (victory fanfare)
4. Sets up rank badge animations and timer fields
5. Sets `App+0x850 = 1` (marks tournament completed on the App struct)
6. Does NOT add any "PLAY!" button — only the win screen content is shown

The user clicks "MAIN MENU" to return to the title screen. There is no separate "advance to next race" step — the tournament is over.

## The Race Order

The switch statement in `Tournament_AdvanceRace` (0x427080) defines the fixed race order:

| Race Idx | Switch Case | Board Constructor | Race Name | MESHWORLD File |
|----------|-------------|-------------------|-----------|-----------------|
| 1 | 1 | `BoardLevel1_WarmUp_ctor` | WARM-UP RACE | Level1 |
| 2 | 2 | `FUN_004200e0` (Beginner) | BEGINNER RACE | LevelCascade |
| 3 | 3 | `BoardLevel2_Intermediate_ctor` | INTERMEDIATE RACE | Level2 |
| 4 | 4 | `BoardLevel3_ctor` (Dizzy) | DIZZY RACE | Level3 |
| 5 | 5 | `BoardLevel5_Tower_ctor` | TOWER RACE | Level4 |
| 6 | 6 | `FUN_00420390` (Up) | UP RACE | LevelUp |
| 7 | 7 | `FUN_00424440` (Neon) | NEON RACE | LevelDark |
| 8 | 8 | `BoardLevel8_Expert_ctor` | EXPERT RACE | Level5 |
| 9 | 9 | `BoardLevel9_Odd_ctor` | ODD RACE | Level6 |
| 10 | 10 | `FUN_0041f4b0` (Toob) | TOOB RACE | Level8 |
| 11 | 0xB | `BoardLevel12_Wobbly_ctor` | WOBBLY RACE | Level7 |
| 12 | 0xC | `FUN_00424a90` (Glass) | GLASS RACE | LevelGlass |
| 13 | 0xD | `FUN_0041f930` (Sky) | SKY RACE | Level9 |
| 14 | 0xE | `FUN_004206d0` (Master) | MASTER RACE | Level10 |
| 15 | 0xF | `FUN_00424c20` (Impossible) | IMPOSSIBLE RACE | LevelImpossible |

Race name strings are stored in a pointer table at `0x4F7080` (15 entries, 4 bytes each). `TourneyMenu_GetRaceName` indexes this table with the current race index to display the **upcoming** race name on the between-races screen.

## PlayerProfile Struct Layout

The PlayerProfile (0x98 bytes, allocated by `PlayerProfile_ctor`) is the central tournament state object. Stored at `App+0x220`.

```
Offset  Size  Field
------  ----  -----
0x00    4     vtable pointer
0x04    4     App pointer
0x08    4     race index (int32, 0-based — 0 = no races completed, 1 = Warm-Up done, etc.)
0x0C    4     current board pointer
0x10    1     is_practice flag (byte) — set for practice mode, not tournament
0x11    1     is_party flag (byte) — set for multiplayer party mode
0x14    60    scores array (float[15], one per race — stores App+0x5E8 race time)
0x50    60    times array (int[15], stores __ftol2 conversion of score float)
0x8C    4     (unknown)
0x90    4     (unknown int32)
0x94    1     (unknown byte)
0x95    1     (unknown byte — set to 1 by TourneyMenu_Advance)
```

## Complete Tournament Flow

### Phase 1: Starting a Tournament

1. **Main menu** → User selects "1PT" (1-Player Tournament) or "1PMT" (Mirror)
2. `GameSelectionManager` (0x433AC0) sets App flags:
   - `App+0x5D4 = 1` (tournament active)
   - `App+0x235 = 1` (is tournament mode)
   - `App+0x236 = 0 or 1` (mirror mode flag)
3. Checks for save file `DATA\TOURNAMENT.SAV`:
   - **No save** → creates `DifficultyMenu` → user picks Normal/Frenzied
   - **Save exists** → creates `TourneyContinueDialog` (Resume? or Restart?)
4. Difficulty selection triggers `App_StartTournamentRace` (0x4288B0)

### Phase 2: App_StartTournamentRace (0x4288B0)

```c
App_StartRace(app);           // Cleanup old scene, reset gfx
app->0x237 = 0;               // Not arena mode
app->0x717 = 1;               // Player 1 active
app->0x7B7 = 1;               // Player 2 active (AI)
app->0x5D7 = 0;
app->0x677 = 1;               // (unless difficulty != 0)
if (app->0x236) {             // Mirror mode
    gfx->0x7D2 = 1;           // Reverse culling
}
// Create new PlayerProfile
profile = PlayerProfile_ctor(new(0x98), app, app->0x234);
app->0x220 = profile;
// Start the first race!
Tournament_AdvanceRace(profile, 1);
```

### Phase 3: Tournament_AdvanceRace (0x427080) — The Core Function

**Parameters:**
- `this` = PlayerProfile
- `param_1` = flag: `1` = show between-races menu, `0` = advance and create next board

**When `param_1 == 1` (show TourneyMenu):**
1. Save current score: `profile->scores[raceIdx] = App->0x5E8`
2. Create a `TourneyMenu` (between-races screen showing results + "PLAY!" button)
3. Return — game is now paused showing the menu

**When `param_1 == 0` (advance to next race):**
1. Destroy old board if it exists (call board's destructor via vtable)
2. Clear board pointer: `profile->board = NULL`
3. **Check arena mode**: if `App+0x237` is set, call `TourneyMenu_CreateBoard` instead (arena variant)
4. **Increment race index**: `profile->raceIdx++`
5. Save current score to the score array
6. **Switch on new race index** → create the appropriate board constructor
7. If no case matches (raceIdx > 15) → tournament is over, return without creating a board
8. If a board was created:
 - Store it: `profile->board = newBoard`
 - Call `board->vtable[0x12]()` (init/load function)
 - Add board to scene via `Scene_AddObject`
 - Set up score timers based on difficulty:
   - Normal (difficulty 0): `targetTime += 1000`, score bonus `_DAT_004cf6f4`
   - Frenzied (difficulty 1): `targetTime += 500`, score bonus `_DAT_004cf3d8`
 - Initialize 4 player score slots (App+0x5E8/5EC, 0x688/68C, 0x728/72C, 0x7C8/7CC)
   - For races 1-2 (index < 3): both "current" and "previous" are set to the same value (starting the tournament fresh)
   - For races 3+ (index >= 3): only "previous" is updated, while "current" carries forward from the previous race's end state
 - **All race scores are saved identically to the profile's scores array** — the `index < 3` distinction only affects the App-level display slots, not the profile score array which is always written: `profile->scores[raceIdx] = App->0x5E8`
 - For each player slot (4 players, 0xA0 stride):
   - Set race index: `playerData->0x60C = raceIdx - 1`
   - Copy race name from `board->0x29B4` to `playerData->0x610`

### Phase 4: Race Ends — Scene_HandleCountdown (0x41A540)

This function is called when a race ends (all balls finished or timer expires).

**Key flow:**

```
1. Check if all balls have finished (iterate ball list at board+0x362C)
2. If not all finished → return (keep playing)

3. Check ball count (board+0x3628) — if 0, return

4. CHECK "LAST RACE" FLAG:
   board+0x4348 (byte, set by Impossible Race constructor)

   IF IS LAST RACE (board+0x4348 != 0):
     ├── Check profile+0x10 (is_practice)
     │   └── If set → go to TIMER PATH (normal race end)
     ├── Check profile+0x11 (is_party)
     │   └── If set → go to TIMER PATH (normal race end)
     └── Neither set (tournament mode):
         → Set board+0x874 = 1 (PAUSE)
         → Create TourneyMenu (0x00451B90) — final results screen
         → Return (game paused, showing final tournament results)

   IF NOT LAST RACE (board+0x4348 == 0):
     → Go to TIMER PATH

5. TIMER PATH (0x41A6BC):
   - Increment countdown timer (board+0x3624)
   - If tournament mode (profile+0x10 != 0): add extra time
   - If timer < threshold (0x4CF310): return (keep counting)
   - Timer expired → race is truly over

6. CLEANUP + SCORE SAVE:
   - Call vtable[0] on all scene objects (destroy them)
   - Copy current scores to "previous" slots:
     App+0x5EC ← App+0x5E8  (Player 1)
     App+0x68C ← App+0x688  (Player 2)
     App+0x72C ← App+0x728  (Player 3)
     App+0x7CC ← App+0x7C8  (Player 4)

7. MODE DISPATCH:
   ├── profile+0x11 (is_party) SET → Party mode path:
   │   ├── App_StartRace(app)
   │   ├── App_ShowResults(app)
   │   └── Show PartyMenu / ArenaMenu / TimeTrialMenu
   │
   ├── profile+0x10 (is_practice) SET → Practice mode path:
   │   ├── App_ShowResults(app)
   │   ├── Create PracticeMenu
   │   └── App_StartRace(app)
   │
   └── NEITHER set → Tournament mode:
       └── Tournament_AdvanceRace(profile, 1)
           → Creates TourneyMenu (between-races screen)
           → User clicks "PLAY!" → eventually calls AdvanceRace(profile, 0)
           → Creates next board → next race begins
```

### Phase 5: The Between-Races Screen (TourneyMenu)

When `Tournament_AdvanceRace(profile, 1)` is called, it creates a TourneyMenu object (0x111C bytes) that:
- Shows the results of the just-completed race
- Displays the name of the **upcoming** race (via `TourneyMenu_GetRaceName`)
- Has a "PLAY!" button

When "PLAY!" is clicked:
1. `TourneyMenu_Advance` (0x4266F0) is called
2. Sets `profile+0x95 = 1`
3. Decrements `profile->raceIdx` (correcting for the increment that will happen in AdvanceRace)
4. Creates a **new** TourneyMenu with param=0
5. This eventually triggers `Tournament_AdvanceRace(profile, 0)` which:
   - Increments raceIdx back
   - Creates the next board
   - The race begins

### Phase 6: Tournament End — Win Screen

After the Impossible Race (race 15, the last race):

1. `Scene_HandleCountdown` detects `board+0x4348` (last race flag, set by Impossible's constructor at `0x424C20`)
2. In tournament mode (profile+0x10=0, profile+0x11=0):
   - Sets pause flag (`board+0x874 = 1`)
   - Calls the **win screen constructor** at `0x00451B90` (NOT the between-races `0x0044FDA0`)
   - The win screen constructor:
     - Sets `profile+0x96 = 1` (won flag)
     - Deletes `DATA\TOURNAMENT.SAV` (tournament over, no resume)
     - Plays "Main Theme - No Intro" at 2x speed (victory fanfare)
     - Sets `App+0x850 = 1` (tournament completed)
   - Game pauses showing the win screen with rank badge
3. User clicks "MAIN MENU" → returns to title screen
4. **Tournament is over.** No `Tournament_AdvanceRace` is called — there is no "PLAY!" button and no attempt to advance to race 16.

## Tournament Save File Format

`DATA\TOURNAMENT.SAV` — written by `TourneyMenu_WriteSave` (0x4264B0):

| Offset | Size | Source | Description |
|--------|------|--------|-------------|
| 0 | 4 | profile+0x08 | Race index (current progress) |
| 4 | 4 | profile+0x14 | First race score (float) |
| 8 | 60 | profile+0x18 | Remaining race scores (float[15]) |
| 68 | 60 | profile+0x54 | Race times (int[15]) |
| 128 | 4 | profile+0x90 | Unknown |
| 132 | 1 | profile+0x94 | Unknown flag |
| 133 | 1 | profile+0x95 | Unknown flag |
| 134 | 1 | App+0x236 | Mirror mode flag |
| 135 | 4 | App+0x23C | Difficulty enum (0=Normal, 1=Frenzied) |
| 139 | 4 | App+0x5E8 | Current race time |
| 143 | 4 | App+0x5E4 | Current score |
| 147 | 4 | App+0x5F4 | Unknown |

Total: 151 bytes

Loading: `TourneyMenu_LoadSaveAndShow` (0x4265A0) reads the same format, then creates a TourneyMenu with `param=1` (resume mode).

## Key App Struct Offsets (Tournament-Related)

```
App+0x174   gfx_device (D3D8)
App+0x178   sound_device
App+0x184   scene manager
App+0x220   PlayerProfile pointer (current tournament state)
App+0x234   difficulty enum (0=Normal, 1=Frenzied, 2=?)
App+0x235   is_tournament (byte)
App+0x236   is_mirror (byte)
App+0x237   arena_mode (byte)
App+0x5D4   tournament_active (byte)
App+0x5D8   player_active[0] (byte, per-player at 0xA0 stride)
App+0x5E4   current_score (int32)
App+0x5E8   current_race_time (int32)
App+0x5EC   previous_race_time (int32)
App+0x5F4   unknown (int32)
App+0x60C   player_race_index[0] (int32, per-player at 0xA0 stride)
App+0x610   player_race_name[0] (string, per-player at 0xA0 stride)
App+0x688   player2_current_score
App+0x68C   player2_previous_score
App+0x717   player1_active (byte)
App+0x728   player3_current_score
App+0x72C   player3_previous_score
App+0x7B7   player2_active (byte)
App+0x7C8   player4_current_score
App+0x7CC   player4_previous_score
App+0x90C   unknown object pointer (freed on tournament start)
App+0x910   unknown object pointer (freed on tournament start)
```

## Key Board Struct Offsets (Tournament-Related)

```
Board+0x874    pause_flag (byte, 1=paused)
Board+0x878    App pointer
Board+0x29B4   race_name_string_pointer
Board+0x2990   unknown (set by AdvanceRace)
Board+0x2994   unknown (set by AdvanceRace)
Board+0x2998   unknown timer/score value (read by AdvanceRace)
Board+0x3624   countdown_timer (float)
Board+0x3628   ball_count (int)
Board+0x362C   ball_AthenaList
Board+0x4348   is_last_race (byte — set ONLY by Impossible Race constructor)
Board+0x8B8    scene_object_AthenaList
```

## The "Last Race" Flag

`board+0x4348` is the critical flag that determines tournament end behavior. It is set to `1` by the **Impossible Race constructor** (`FUN_00424c20` at `0x424C20`). No other board constructor sets this flag.

When this flag is set and the race ends:
- **Tournament mode**: Pauses the game and calls the **final win screen** constructor `0x00451B90` directly (NOT the between-races `0x0044FDA0`). This sets `profile+0x96 = 1` (won flag), deletes `DATA\TOURNAMENT.SAV`, plays victory music, and shows the win screen with rank badge. No "PLAY!" button — only "MAIN MENU". **Tournament is over.**
- **Practice mode** (`profile+0x10` set): Skips the win screen, goes through the normal timer/cleanup path
- **Party mode** (`profile+0x11` set): Same — skips win screen, goes to party mode menu

## Difficulty Bonuses

After creating each board, `Tournament_AdvanceRace` adjusts the target time and score based on difficulty:

| Difficulty | Name | Time Bonus | Score Bonus (float constant) |
|------------|------|------------|-------------------------------|
| 0 | Normal | +1000 ms | `_DAT_004cf6f4` |
| 1 | Frenzied | +500 ms | `_DAT_004cf3d8` |
| 2 | (?) | (no bonus) | (no bonus) |

The time bonus is added to `board+0x2998` (the board's target time). The score bonus is added to `profile+0x14` (the overall tournament score float).

For races 1-2 (index < 3): both "current" and "previous" display slots are set to the same value.
For races 3+ (index >= 3): only "previous" is updated, while "current" carries forward.
**All race scores are saved identically to the profile's scores array** regardless of race index — the `index < 3` check only affects the App-level display state, not the persistent profile score data.
