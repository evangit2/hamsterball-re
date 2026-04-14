# Game State Machine & Tournament System

## Architecture Overview

Hamsterball's game flow is a menu-driven state machine where each "screen" is a SceneObject added to the active scene. Menu transitions work by deleting the old menu object and creating a new one — there is no explicit state enum. Instead, game mode is tracked via boolean flags in the App object.

## Menu Hierarchy (State Machine)

```
MainMenu (0x42DE50, vtable 0x4D3F30)
  ├── LET'S PLAY! → TournamentScreen (0x42E0C0, vtable 0x4D4010)
  │     ├── TOURNAMENT → TournamentManager (0x433C00) "1PT"
  │     │     ├── DifficultyMenu → Normal/Frenzied selection
  │     │     ├── TourneyContinueDialog → Resume saved tournament
  │     │     ├── Start → App_StartTourneyRace (0x428A40)
  │     │     └── Result → TourneyMenu (0x450070, vtable 0x4D83F0)
  │     ├── TIME TRIALS → TimeTrialMenu "1PTT"
  │     ├── MIRROR TOURNAMENT → "1PMT" (if unlocked, else "LOCKED")
  │     ├── PARTY GAMES → MPMenu (0x42E1A0) "PARTY"
  │     └── PREVIOUS → back to MainMenu
  ├── HIGH SCORES → "HS"
  ├── OPTIONS → OptionsMenu "OP"
  ├── CREDITS → "CR"
  ├── REGISTER GAME → "RG" (if not registered)
  ├── MINI GAMES → "MG" (if unlocked)
  └── EXIT TO DESKTOP → "EXIT"
```

## Game Mode Flags (App Object)

| Offset | Type | Description |
|--------|------|-------------|
| App+0x234 | bool | is_multiplayer (from menu selection) |
| App+0x235 | bool | is_tournament_mode |
| App+0x236 | bool | is_mirror_mode (mirror tournament) |
| App+0x237 | bool | (cleared on race start) |
| App+0x23C | bool | race_active (set to 1 when race starts) |
| App+0x5D4 | bool | is_single_player |
| App+0x5D5 | bool | (cleared on tourney entry) |
| App+0x5D7 | bool | (cleared on race start, set 1 during) |
| App+0x677 | bool | (set 1 = default, cleared if MP) |
| App+0x717 | bool | (set 1 on race start) |
| App+0x7B7 | bool | (set 1 on race start) |
| App+0x850 | bool | mirror_tournament_unlocked |
| App+0x851-0x865 | bool[15] | Per-race visited flags (sound triggers on first visit) |

## Tournament System

### Tournament Flow

1. **Main Menu** → "LET'S PLAY!" → **Game Menu (TournamentScreen)**
2. **Game Menu** → "TOURNAMENT" (button code "1PT") → **TournamentManager**
3. **TournamentManager** checks for `DATA\TOURNAMENT.SAV`
4. If save exists: **TourneyContinueDialog** — Continue or New?
5. If no save or New: **DifficultyMenu** — Normal or Frenzied?
6. Difficulty selected → **App_StartTourneyRace** (0x428A40)
7. Race completes → **TourneyMenu** (0x450070) — shows results
8. TourneyMenu → "PLAY!" → **Tournament_AdvanceRace** → next race
9. After all 14 races: "TOURNAMENT WINNER!" or "LOST TOURNAMENT"

### TournamentManager (0x433C00)
Central button handler for game mode selection. Receives button codes as strings:

| Button Code | Action | Target Menu |
|------------|--------|-------------|
| "BACK" | Return to MainMenu | MainMenu_ctor (0x42DE50) |
| "LOCKED" | Mirror tournament locked | OkayDialog with unlock message |
| "1PT" | 1P Tournament | DifficultyMenu or TourneyContinueDialog |
| "1PMT" | 1P Mirror Tournament | DifficultyMenu (mirror flag set) |
| "1PP" | Practice | PracticeMenu_ctor |
| "1PTT" | Time Trials | TimeTrialMenu_ctor |
| "PARTY" | Party/Multiplayer | MPMenu_ctor (0x42E1A0) |

For "1PT" and "1PMT", the manager:
1. Sets `App+0x235 = 1` (tournament flag)
2. Sets `App+0x236` = 1 for mirror, 0 for normal
3. Clears `App+0x5D5, 0x5D7` 
4. Sets `App+0x5D4 = 1` (single player)
5. Checks for `DATA\TOURNAMENT.SAV` file
6. If save exists AND `Game_SetInProgress()` returns true → TourneyContinueDialog
7. Else → DifficultyMenu (fresh start)

### DifficultyMenu
Presented when starting a new tournament:
- **NORMAL** — Standard time pool
- **FRENZIED** — Reduced time pool ("Experts only! At Frenzied Difficulty, you get less time")

### App_StartTourneyRace (0x428A40)
Entry point for tournament race initialization:
```
App_StartTourneyRace(app):
  App_StartRace(app)
  app.is_237 = 0
  app.is_717 = 1
  app.is_7b7 = 1
  app.is_5d7 = 0
  app.is_677 = 1
  
  // Create PlayerProfile (0x98 bytes)
  profile = PlayerProfile_ctor(app, app.is_234)
  app.player_profile = profile  // +0x220
  
  // Load tournament save and show menu
  TourneyMenu_LoadSaveAndShow(profile, "DATA\\TOURNAMENT.SAV")
  
  // Set graphics cull mode based on mirror flag
  if !app.is_mirror:
    gfx.reversed = 0;  cull = default
  else:
    gfx.reversed = 1;  cull = mirror mode
  
  // Clean up old UI objects
  delete app.old_ui_0x90c
  delete app.old_ui_0x910
```

### App_StartMPRace (0x428B20)
Multiplayer race initialization:
```
App_StartMPRace(app, param_1):
  App_StartRace(app)
  app.is_mp = 1 (0x717, 0x7b7)
  app.is_5d7 = 0
  app.is_677 = 1 (cleared if MP flag set)
  
  // Create PlayerProfile
  profile = PlayerProfile_ctor(app, app.is_234)
  app.player_profile = profile  // +0x220
  profile[2] = param_1
  profile.is_active = 1  // +0x10
  
  // Start tournament advancement
  Tournament_AdvanceRace(profile, 0)
  
  // Clean up old UI objects
  delete app.old_ui_0x90c
  delete app.old_ui_0x910
```

### TourneyMenu (0x450070, vtable 0x4D83F0)
Post-race tournament screen showing:
- **PLAY!** — Advance to next race
- **ROLLBACK** — Re-do previous race (only if current_race > 1, shows "RXLL" if race <= 1)
- **MAIN MENU** — Exit tournament

If tournament is in progress (not warm-up race):
- Shows rank badge from `textures\ranks\%d.jpg`
- Rank computed by comparing accumulated time against 16 thresholds at `DAT_004F710C`
- Warm-up race text: "TAKE YOUR TIME ON THE WARM-UP RACE! This easy little race lets you get the hang of various hamsterball tricks without affecting your time pool."

Race visit tracking (per-race "first visit" sound triggers):
| Race # | App Flag Offset | Race |
|--------|-----------------|------|
| 3 | App+0x851 | Race 3 |
| 4 | App+0x852 | Race 4 |
| 5 | App+0x853 | Race 5 |
| 6 | App+0x863 | Race 6 |
| 7 | App+0x854 | Race 7 |
| 8 | App+0x855 | Race 8 |
| 9 | App+0x856 | Race 9 |
| 10 | App+0x857 | Race 10 |
| 11 | App+0x864 | Race 11 |
| 12 | App+0x858 | Race 12 |
| 13 | App+0x859 | Race 13 |
| 14 | App+0x865 | Race 14 |

On first visit to a race, `this+0x1108 = 1` and a "first visit" sound plays.

### PlayerProfile Structure (0x98 bytes)
Created during tournament/race init:
| Offset | Type | Description |
|--------|------|-------------|
| +0x00 | vtable* | PlayerProfile vtable |
| +0x04 | App* | Back-pointer to App |
| +0x08 | int | Current race number (1-14) |
| +0x10 | bool | is_active (set 1 for MP) |
| +0x90 | int | Accumulated time for save |
| +0x95 | bool | has_rollback_available |
| +0x96 | bool | tournament_completed |
| +0x5E4 | float | Total accumulated time (for rank calculation) |

### Tournament Save File
- Path: `DATA\TOURNAMENT.SAV`
- Written via `TourneyMenu_WriteSave(profile, path)` after each race
- Loaded via `TourneyMenu_LoadSaveAndShow(profile, path)` at tournament start
- Contains: current race number, accumulated times, visited flags

### Unlock System
**Race unlock**: Each race is unlocked by reaching it during a Normal or Frenzied tournament game:
```
"THIS RACE ISN'T UNLOCKED YET! TO UNLOCK %s RACE, YOU NEED TO REACH IT
 WHILE PLAYING A NORMAL OR FRENZIED TOURNAMENT GAME!"
```

**Arena unlock**: Each arena requires finding a secret unlock spot in a specific race:
```
"THIS ARENA ISN'T UNLOCKED YET! TO UNLOCK %s ARENA, YOU NEED TO FIND THE
 SECRET UNLOCK SPOT IN THE %s RACE DURING A NORMAL OR FRENZIED TOURNAMENT GAME!"
```

**Mirror Tournament unlock**: Win a tournament at Normal or Frenzied difficulty:
```
"THE MIRROR TOURNAMENT ISN'T UNLOCKED YET! TO UNLOCK THE MIRROR TOURNAMENT,
 YOU NEED TO WIN A TOURNAMENT AT NORMAL OR FRENZIED DIFFICULTY!"
```
- Tracked by `App+0x850` (mirror_tournament_unlocked bool)

### Game End States
- **TOURNAMENT WINNER!** — Won all 14 races within time pool
- **LOST TOURNAMENT** — Time pool depleted
  - "YOU HAVE LOST THE TOURNAMENT. IN ORDER TO RESUME, YOU NEED TO SELECT 'ROLLBACK' ON THE MENU!"
  - ROLLBACK option lets player redo the previous race with their remaining time

## Key Address Map

| Address | Function | Description |
|---------|----------|-------------|
| 0x42DE50 | MainMenu_ctor | Main menu (PLAY/HS/OP/CR/RG/MG/EXIT) |
| 0x42E0C0 | TournamentScreen_ctor | Game menu (Tourney/TT/Mirror/Party/Back) |
| 0x433C00 | TournamentManager | Mode selection button handler |
| 0x428A40 | App_StartTourneyRace | Tournament race init + save load |
| 0x428B20 | App_StartMPRace | Multiplayer race init |
| 0x450070 | TourneyMenu_ctor | Post-race tournament results menu |
| 0x42E1A0 | MPMenu_ctor | Multiplayer menu |

## Reimplementation Notes

### State Machine (SDL2)
- Replace menu system with a simple state stack or scene graph
- Each "menu" is a screen class with Update/Render/HandleInput methods
- Button codes become enum values or string IDs
- Replace `operator_new(size) + ctor()` with modern factory methods
- The boolean-flag approach is fragile — consider an explicit `GameMode` enum:
  ```cpp
  enum class GameMode { MainMenu, Tournament, TimeTrial, MirrorTournament, 
                        Practice, PartyGames, Multiplayer };
  ```

### Tournament Save
- Use JSON or simple binary format instead of raw struct dump
- PlayerProfile is only 0x98 bytes — small enough for JSON
- Rank thresholds at `DAT_004F710C` should be extracted as constants

### Mirror Mode
- Mirror flag at `App+0x236` reverses cull mode (D3DRS_CULLMODE)
- Equivalent to: `glFrontFace(GL_CW)` vs `glFrontFace(GL_CCW)` in OpenGL
- Also flips camera/track direction — need to verify if track geometry is mirrored or just rendering