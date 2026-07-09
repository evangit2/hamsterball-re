# Hamsterball Level Completion, Menu Transition, and Level Loading System

Reverse-engineered from Hamsterball.exe (V3.6.c) via Ghidra decompilation.
All offsets verified against decompiled source.

## Overview

The game has three distinct game modes that follow different completion paths:
- **Tournament Mode**: Sequential progression through 15 races (Warm-Up → Impossible)
- **Practice/Time Trial Mode**: Single race, returns to selection menu
- **Arena/Party Mode**: Multiplayer, returns to arena/party menu

---

## 0. How the Game Loads a Level From a Menu (Menu → Level Transition)

### 0a. Menu System Overview

All menus inherit from `SimpleMenu` (ctor at 0x448F20). Each menu is a scene object
that gets added to the scene graph via `Scene_AddObject(scene+0x184, menu)`. Menu items
are added via `UIList_AddItem(menu, displayText, actionTag, ...)` — the `actionTag` string
is what gets passed to the menu's callback when the user selects an item.

**Menu hierarchy:**
```
MainMenu_ctor (0x42DE50) — "LET'S PLAY!" / "HIGH SCORES" / "OPTIONS" / "CREDITS" / "EXIT"
  ├── "LET'S PLAY!" → DifficultyMenu_ctor (0x42E220)
  │     ├── "TOURNAMENT" → App_StartTournamentRace (0x4288B0)
  │     ├── "TIME TRIALS" → TimeTrialMenu_ctor (0x42F810)
  │     └── "PARTY" → PartyMenu_ctor (0x42FC10)
  │           └── "ARENA" → ArenaMenu_ctor (0x42FC40)
  │
  ├── "HIGH SCORES" → (high scores screen)
  ├── "OPTIONS" → OptionsMenu_ctor (0x42CE0)
  └── "CREDITS" → (credits screen)
```

### 0b. Menu Item Selection → Level Loading

When a menu item is selected (via keyboard/mouse), the menu's vtable callback fires
with the item's `actionTag` string. The callback then dispatches to the appropriate
action. For level-starting menus, the flow is:

**Practice/Time Trial path:**
1. `PracticeMenu_ctor` (0x42EA30) builds the level selection list
2. For each level, it checks the **unlock flag** at `scene+0x851` through `scene+0x865`
3. If unlocked: adds the level with its thumbnail sprite + number tag ("0" through "14")
4. If locked: adds the item with `"LOCKED DIZZY"` etc. text, uses a generic locked sprite
5. When user selects an unlocked level → callback calls `App_StartPracticeRace(app, levelIndex)`

**Tournament path:**
1. `TourneyMenu_ctor` (0x44FDA0) shows the tournament menu with "PLAY!" button
2. When user clicks "PLAY!" → callback calls `Tournament_AdvanceRace(profile, 0)`
   - This increments the race index and creates the next `LevelBoard_*_ctor`
   - The board constructor calls `LoadRaceData` and then `vtable[0x12]()` for initialization

### 0c. `App_StartPracticeRace` (0x428C50) — Practice/Time Trial Level Start

This is the bridge between menu selection and level loading:

1. `App+0x237 = 0` (clear arena flag)
2. `App_StartRace(app)` — cleans up previous race state (see §2c)
3. Manages BestTimeTracker recording/playback buffers (App+0x90C / App+0x910)
   - These are 0x528-byte objects that store per-frame ball position snapshots
   - App+0x90C = recording buffer (current race), App+0x910 = playback buffer (best run = ghost)
   - If both exist, compares finish times: promotes recording to playback if better
   - Entries with `9999999` at offset +0x149 (dword = +0x524) = never-finished sentinel
   - See [TIME_TRIAL_GHOST_SYSTEM.md](TIME_TRIAL_GHOST_SYSTEM.md) for full details
4. Sets up player slots:
   - `App+0x717 = 1` (P3 active)
   - `App+0x7B7 = 1` (P4 active)
   - `App+0x5D7 = 0` (P1 inactive — gets set later by tournament)
   - `App+0x677 = 1` (P2 active)
   - If party mode (`App+0x234 != 0`): `App+0x677 = 0`
5. `App+0x23C = 1` (set difficulty to Normal)
6. Creates `PlayerProfile` (0x98 bytes, via `PlayerProfile_ctor`)
   - Stores at `App+0x220`
   - Sets `profile+0x08 = levelIndex` (the selected level)
   - Sets `profile+0x11 = 1` (time-trial flag)
7. **Calls `Tournament_AdvanceRace(profile, 0)`** — this is the key call
   - With `param_1=0`, AdvanceRace takes the "create next board" path
   - Switches on `profile+0x08` (race index) to create the right `LevelBoard_*_ctor`
   - The board constructor does the actual level loading
8. Copies the race name from `board+0x29B4` into the score-tracking object at `App+0x90C+0x424` (used to compare which level the recording was made on)
9. Checks if the new race name matches the previous race's recording — if the level changed, may destroy old playback buffer

### 0d. `App_StartTournamentRace` (0x4288B0) — Tournament Level Start

1. `App_StartRace(app)` — cleanup
2. `App+0x237 = 0` (not arena)
3. Sets up player slots (same as practice, but P2 logic differs for party mode)
4. Creates `PlayerProfile` and stores at `App+0x220`
5. **Calls `Tournament_AdvanceRace(profile, 1)`** — with `param_1=1` (results mode)
   - This creates a `TourneyMenu` showing the tournament status
   - Does NOT immediately create a board — the board is created when user clicks "PLAY!"
   - When "PLAY!" is clicked → `Tournament_AdvanceRace(profile, 0)` → creates the board

### 0e. Board Constructor → Level Data Loading

Each `LevelBoard_*_ctor` (e.g. `LevelBoard_WarmUp_ctor` at 0x41CA40) does:

1. `Board_ctor(this, app)` — base board initialization
2. Sets the board's vtable (each level has its own vtable with custom collision/update handlers)
3. Sets `board+0x868` = board name (e.g. "Board (Warm-Up)")
4. Sets `board+0x29B4` = race display name (e.g. "WARM-UP RACE")
5. Initializes light direction matrix at `board+0x1508` via `Vec3_Init`
6. **Calls `LoadRaceData(this, raceName)`** — loads race parameters from XML
7. Sets `board+0x4344` = music track name (e.g. "Hamster Nation")

### 0f. `LoadRaceData` (0x40A120) — Race Parameter Loading

Parses `racedata.xml` and loads per-race parameters:

1. Opens `racedata.xml` via `FileHandle_Open`
2. Uses `MWParser_ReadTag` to iterate XML tags (same parser as MESHWORLD)
3. Finds the tag matching the race name (e.g. "BEGINNERRACE", "WARMUPRACE")
4. Reads sub-tags:

| Tag | Board Offset | Purpose |
|-----|-------------|---------|
| `TIME` | +0x2998 | Race time limit (int, from atof) |
| (difficulty) | +0x2990 | Difficulty rating (9 - value) |
| `WEASEL` | +0x2994 | Weasel score (9 - value) |
| `GOLD` | +0x29A0 | Gold medal threshold (9 - value) |
| `SILVER` | +0x29A4 | Silver medal threshold (9 - value) |
| `BRONZE` | +0x29A8 | Bronze medal threshold (9 - value) |
| (camstart) | +0x299C | Camera start position (float) |

5. After parsing, adjusts medal threshold based on player's difficulty flags:
   - Checks `scene+0x8BC + raceIndex*4` → upgrade to Silver
   - Checks `scene+0x8BD + raceIndex*4` → upgrade to Gold
   - Checks `scene+0x8BE + raceIndex*4` → upgrade to Weasel
   - Stores final threshold at `board+0x29AC`

### 0g. Board Initialization: `vtable[0x12]()` and `Level_InitScene`

After the board constructor returns, `Tournament_AdvanceRace` calls:
1. `(*piVar7)[0x48]()` — this is `vtable[0x12]` (offset 0x48 = 18th entry, 0-indexed)
   - This is the board's "initialize" virtual method
   - For most boards, this calls into the level loading chain

2. `Scene_AddObject(scene+0x184, board)` — adds the board to the scene

3. `GetLevelPath` (0x40D1C0) — called from the board vtable or scene setup:
   - Creates a `Level` object (0x10D0 bytes) via `FUN_00461510`
   - Passes the level path string (e.g. "levels\\level1") and graphics device
   - Stores the Level at `scene+0x22B` (offset 0x22B*4 = +0x8AC)
   - Creates a collision data object (0x10D0 bytes) via `FUN_00465080`
   - Stores at `scene+0x22C` (offset +0x8B0)
   - Calls `Level_InitScene(scene)` to initialize the 3D scene

4. `Level_InitScene` (0x40B090) sets up the 3D world:
   - Creates a `SoundChannel` (0x80 bytes) for 3D audio → stored at `board+0x87C`
   - Configures audio position from `board+0x29C0`
   - If level has collision data (`board+0x8AC != 0`):
     - Resets object slots via `Scene_ResetObjectSlots`
     - Finds "CAMERALOCUS" object for camera positioning
   - Sets up camera: copies distance/projection from scene globals
   - Calls `Level_SelectCameraProfile(board)` for per-level camera setup
   - Configures graphics: cull mode, projection matrix, near/far planes
   - Sets camera start position from `board+0x299C` (if != -4.0)
   - Plays level music:
     - Practice mode: appends "_No_Intro" to music track name, plays at 4x speed
     - Tournament mode: plays music track at 2x speed

### 0h. Complete Menu → Level Loading Sequence

```
User selects level in PracticeMenu / TimeTrialMenu
    │
    ▼
App_StartPracticeRace(app, levelIndex)     [0x428C50]
    ├── App_StartRace(app)                  — cleanup previous state
    ├── Create PlayerProfile                — set race index, mode flags
    ├── Tournament_AdvanceRace(profile, 0)  [0x427080]
    │     ├── Increment race index (profile+0x08)
    │     ├── switch(raceIndex):
    │     │     case 1: LevelBoard_WarmUp_ctor(board, app)
    │     │     case 2: LevelBoard_Beginner_ctor(board, app)
    │     │     ... (15 level constructors)
    │     │     Each ctor:
    │     │       ├── Board_ctor(this, app)
    │     │       ├── Set vtable, names, light direction
    │     │       ├── LoadRaceData(this, "RACENAME")  — parse racedata.xml
    │     │       └── Set music track name
    │     ├── vtable[0x12]()                 — board initialization
    │     │     └── GetLevelPath(scene)
    │     │           ├── Create Level object (0x10D0)  — load MESHWORLD
    │     │           ├── Create Collision object (0x10D0) — load collision
    │     │           └── Level_InitScene(board)
    │     │                 ├── Create SoundChannel
    │     │                 ├── Find CAMERALOCUS object
    │     │                 ├── Set up camera (distance, projection, cull)
    │     │                 └── Play level music
    │     └── Scene_AddObject(scene, board)  — add to scene graph
    │
    └── Race begins (countdown sequence handled by Board_UpdateRaceState)
```

### 0i. `QuitRaceMenu` (0x42E6F0) / `QuitRace` (0x42FAD0) — In-Race Pause Menu

When ESC is pressed during a race, `Scene_CreateGameOverMenu` (0x40A920) creates a pause menu.
The `QuitRaceMenu` variant adds:
- "RESUME" → clears pause flag (`board+0x874 = 0`)
- "RESTART RACE" → clears pause, clears split-screen flag, calls `App_StartPracticeRace` with current level
- "OPTIONS" → opens `OptionsMenu` with "PAUSETT" context
- "QUIT THIS RACE" → calls `App_StartRace` + `App_ShowResults`, then creates TimeTrialMenu or PartyMenu

The callback handler at `FUN_00431520` processes these actions by string-comparing the action tag.

---

## 1. How a Level Ends (Race Completion)

### 1a. Per-Frame Race State Machine: `Board_UpdateRaceState` (0x41B130)

Called every frame by 6 Board vtable methods. This is the master race state machine.

**Key board offsets:**
| Offset | Type | Purpose |
|--------|------|---------|
| +0x3620 | int | Race phase/state (0x19=25 = race in progress) |
| +0x3624 | float | Fade alpha (1.0→0.0 fade-in at race start, 0.0→1.0 fade-out at race end; NOT the race timer) |
| +0x3A38 | ptr | Finish-line trigger objects array |
| +0x3A4C | byte | Countdown done (0=counting, 1=active) |
| +0x3A50 | int | Countdown phase (0-3: GET READY→countdown→GO→start) |
| +0x3A54 | float | Countdown phase timer |
| +0x8B4 | int | Gate counter (incremented while results shown) |
| +0x8B8 | AthenaList | Active UI objects (ScoreObject, RaceResultPopup) |
| +0x878 | ptr | Scene pointer |
| +0x880 | byte | Race-end flag |

**Countdown sequence** (when `+0x3A4C == 0` AND `+0x8B4 == 0` AND `+0x3620 > 25`):
1. Phase 0: "GET READY" (timer increments by 0.01/frame)
2. Phase 1: Countdown beep
3. Phase 2: Second beep (at timer ≥ 2.0), threshold = 3.0 + 2.0 = 5.0
4. Phase 3: Race start — sets `+0x3A4C = 1`

**Finish detection** (when `+0x3A4C != 0`, race active):
- Iterates finish-line trigger objects at `+0x3A38`
- For each trigger object (which are pointers into App player slots at `App+PID*0xA0+0x5CC`):
  - If `obj+0x0A == 0` (not yet finished, = `App+PID*0xA0+0x5D6`):
    - **Single-player**: decrements `obj+0x1C` (time remaining, = `App+PID*0xA0+0x5E8`). When `< 0` AND `obj+0x20 < 1`:
      - Sets `obj+0x1C = 0`, `obj+0x0A = 1` (finished)
      - Sets ball `+0x14C = 1` (ball finished flag)
      - Calls `Scene_UpdateChildren(scene+0x178)`
      - Sets `board+0x880 = 1` (race ended)
      - Creates `RaceResultPopup` (0x60 bytes, ctor at 0x44C260) — uses `obj+0x18` (= `App+PID*0xA0+0x5E4`, the **score**) to determine rank
      - Appends popup to `board+0x8B8` (UI objects list)
      - Stops current music channel, plays "Game Over" music
    - **Multiplayer**: increments `obj+0x1C` instead (counts up to threshold)

**Timer stop mechanism**: The timer is stopped *passively* — no function actively stops it.
`Board_UpdateRaceState` checks `obj+0x0A` (the finished flag at `App+PID*0xA0+0x5D6`) each frame.
If set to 1 (by N:GOAL or by timer expiry), the `DEC obj+0x1C` instruction is skipped via `JNZ` at `0x41B3E5`.
The flag is set by N:GOAL in `DispatchCollisionEvents` (0x40C5D0).

**Score vs Time**: `obj+0x18` (= `App+PID*0xA0+0x5E4`) is the **score** (float, accumulated by E:ACTION
collision events). `obj+0x1C` (= `App+PID*0xA0+0x5E8`) is the **time remaining** (int, countdown from
par time). The rank threshold table at `0x4F710C` contains DWORD score thresholds (6000→350000),
not time values. RaceResultPopup_ctor reads the score as a float and compares against these
thresholds to pick the rank image (`textures\ranks\N.jpg`).

### 1b. `RaceResultPopup` (ctor at 0x44C260)

Created when a ball finishes. Shows the race result popup on screen.
Appended to `board+0x8B8` AthenaList for lifecycle management.

### 1c. `App_CompleteRace` (0x425F90)

Called from: `FUN_0042D270`, `FUN_00431720`, `FUN_004319A0`, `Graphics_SetupLights`, `MusicPlayer_Render`, `VertexDecl_WriteBlendWeights`

```
if (App+0x704 != 0):          // race-complete flag
    App+0x7C8++               // increment completed-race counter
    vtable[0xFC](gfx, 0, 0xD, 1)   // graphics state change
    vtable[0xFC](gfx, 0, 0xE, 1)   // graphics state change
    App+0x704 = 0             // clear flag
```

---

## 2. Transition Back to Level Menu

### 2a. `Scene_ProcessRaceEnd` (0x41A540)

Called via scene-object vtables (32 DATA references from vtable tables at 0x4D02B8–0x4D22F0).
This is the **central transition function** — it determines what menu to show after a race.

**Flow:**

1. **Check if all balls finished**: Iterates ball list, calls `vtable[0x5C]` on each ball, checks if state == 5 (finished). If any ball not finished, returns early.

2. **Check ball count**: If `scene+0xD8A < 1` (no balls), returns early.

3. **Tournament end check** (`scene+0x10D2 != 0` AND `profile+0x10 == 0` AND `profile+0x11 == 0`):
   - Sets `scene+0x21D = 1`
   - Calls cleanup on scene objects (iterates `scene+0x22E` list, calls vtable[0] with arg=1)
   - Creates `TourneyMenu` (0xCFC bytes, ctor at 0x44FDA0)
   - Adds to scene via `Scene_AddObject(scene+0x184, menu)`
   - **→ Shows tournament progression menu**

4. **Non-tournament path** (fade-based delay):
   - If `board+0x3624 == 0.0` (fade complete) and music player exists: `MusicPlayer_SetTempoScale(2.0)` (slow down music)
   - Increments `board+0x3624` (fade alpha) by 0.025/frame (+0.01 extra if `profile+0x10 != 0`, party mode)
   - When `board+0x3624 >= 1.0` (fade-out complete, threshold `_DAT_004CF310` = 1.0):
     - Copies current scores to "best" slots:
       - `App+0x5E8 → App+0x5EC` (P1 current → P1 best)
       - `App+0x688 → App+0x68C` (P2)
       - `App+0x728 → App+0x72C` (P3)
       - `App+0x7C8 → App+0x7CC` (P4)
     - Calls cleanup on scene objects
     - Branches based on `profile+0x11` (is_tournament_flag) and `profile+0x10` (practice_flag):

     **If `profile+0x11 == 0` (not tournament/time-trial):**
       - If `profile+0x10 != 0` (practice):
         - `App_ShowResults(app, 0)` — creates MusicPlayer results screen (0x87C bytes)
         - Creates `PracticeMenu` (0xD18 bytes, ctor at 0x42EA30)
         - Calls `App_StartRace(app)` — starts next practice race
         - **→ Shows practice menu + starts next race**
       - If `profile+0x10 == 0` (tournament main):
         - `Tournament_AdvanceRace(profile, 1)` — advance with results flag
         - **→ Shows tournament results menu**

     **If `profile+0x11 != 0` (tournament/time-trial):**
       - `App_StartRace(app)` — restarts/prepares race
       - `App_ShowResults(app, 0)` — shows results screen
       - Branches on game mode:
         - `App+0x234 == 0` → Creates `TimeTrialMenu` (0xD18, ctor at 0x42F810)
         - `App+0x234 != 0` AND `App+0x237 != 0` → Creates `ArenaMenu` (0xCDC, ctor at 0x42FC40)
         - `App+0x234 != 0` AND `App+0x237 == 0` → Creates `PartyMenu` (0xD18, ctor at 0x42FC10)
       - Adds menu to scene
       - **→ Shows mode-appropriate menu**

### 2b. `App_ShowResults` (0x428060)

Creates a `MusicPlayer` object (0x87C bytes) that displays the results screen.
- Calls `Scene_UpdateChildren(app+0x178)` first
- Stores result player at `App+0x228`
- Adds to scene

### 2c. `App_StartRace` (0x4287C0)

Cleans up previous race state and prepares for next:
- Calls `Scene_UpdateChildren(app+0x178)`
- Sets cull mode on graphics device
- Sets `gfx+0x708 = 3` (state transition)
- Destroys previous objects:
  - `App+0x21C` (vtable[0x40] call = destructor)
  - `App+0x228` (results music player)
  - `App+0x224` (another menu/object)
  - `App+0x220` (PlayerProfile — calls vtable[0] with arg=1)
- Resets music tempo

### 2d. `RaceResultsMenu` (ctor at 0x44CB10)

The results display menu:
- Sets vtable to `PTR_FUN_004D6CFC`
- Stores board pointer at `this+0x0C`
- **Increments `board+0x8B4`** (gate counter — prevents Board_UpdateRaceEnd from processing while results are shown)
- Sets up timing: `this+0x1C = 800` (initial delay), `this+0x14 = 500` (per-entry delay)

### 2e. `RaceResults_Update` (0x44C7D0)

Per-frame update for results screen:
- Checks for key press (`Input_CheckKeyCombo(app, 0)` = Enter/Space)
- If pressed and timer < 1: sets skip flag `this+0x20 = 1`
- Iterates through result entries with timing
- When all entries shown:
  - Sets `App+0x5FC = 5` and `App+0x69C = 5` (state transition values)
  - Decrements `board+0x8B4` (gate counter) when done, clamps to 0
  - Clearing `board+0x8B4` allows Scene_ProcessRaceEnd to proceed

---

## 3. How New Levels Open Up (Level Progression)

### 3a. `Tournament_AdvanceRace` (0x427080) — `__thiscall(this=PlayerProfile, param_1=char)`

**The core tournament progression function.** Called from:
- `App_StartPracticeRace` (0x428C50)
- `App_StartTournamentRace` (0x4288B0)
- `FUN_00428B20`, `FUN_00429230`, `FUN_00450720`
- `Scene_ProcessRaceEnd` (0x41A540)

**PlayerProfile struct layout (0x98 bytes, allocated at 0x44FDA0):**
| Offset | Type | Purpose |
|--------|------|---------|
| +0x00 | vtable | Virtual function table |
| +0x04 | ptr | App pointer |
| +0x08 | int | Current race index (0-based, 0=Warm-Up, 14=Impossible) |
| +0x0C | ptr | Current Board pointer |
| +0x10 | byte | Practice mode flag |
| +0x11 | byte | Tournament/time-trial flag |
| +0x14 | float[] | Per-race score array (15 entries × 4 bytes = 0x3C bytes) |
| +0x50 | int[] | Per-race time array (15 entries × 4 bytes = 0x3C bytes) |
| +0x90 | int | Total score |
| +0x94 | byte | Save flag |
| +0x95 | byte | Won flag / advance flag |
| +0x96 | byte | Results display mode (0=show PLAY button, 1=show rank image) |

**When `param_1 != 0` (advance with results — called from Scene_ProcessRaceEnd):**
1. Save current race score: `this + raceIndex*4 + 0x14 = App+0x5E8` (current P1 score)
2. Save current race time: `this + raceIndex*4 + 0x50 = __ftol2(score)`
3. Create `TourneyMenu` (0x111C bytes) with `param_2=1` (results mode)
4. Add to scene → **shows tournament results screen**

**When `param_1 == 0` (advance to next race — called from PLAY button):**
1. Increment race index: `this+0x08++`
2. Save current race score and time
3. **Big switch on new race index** — creates the next level board:

| case | Race | Board Constructor | Alloc Size |
|------|------|-------------------|------------|
| 1 | Warm-Up | `LevelBoard_WarmUp_ctor` (0x41CA40) | 0x436C |
| 2 | Beginner | `LevelBoard_Beginner_ctor` (0x4200E0) | 0x644C |
| 3 | Intermediate | `LevelBoard_Intermediate_ctor` (0x41CB20) | 0x438C |
| 4 | Dizzy | `LevelBoard_Dizzy_ctor` (0x41D060) | 0x4BE0 |
| 5 | Tower | `LevelBoard_Tower_ctor` (0x41E340) | 0x5418 |
| 6 | Up | `LevelBoard_Up_ctor` (0x420390) | 0x4790 |
| 7 | Neon | `LevelBoard_Neon_ctor` (0x424440) | 0x4394 |
| 8 | Expert | `LevelBoard_Expert_ctor` (0x41EA40) | 0x4FD8 |
| 9 | Odd | `LevelBoard_Odd_ctor` (0x41ED80) | 0x43B0 |
| 10 | Toob | `LevelBoard_Toob_ctor` (0x41F4B0) | 0x646C |
| 0xB | Wobbly | `LevelBoard_Wobbly_ctor` (0x41F110) | 0x4388 |
| 0xC | Glass | `LevelBoard_Glass_ctor` (0x424A90) | 0x4390 |
| 0xD | Sky | `LevelBoard_Sky_ctor` (0x41F930) | 0x47F8 |
| 0xE | Master | `LevelBoard_Master_ctor` (0x4206D0) | 0x6498 |
| 0xF | Impossible | `LevelBoard_Impossible_ctor` (0x424C20) | 0x4380 |
| default | — | Falls through (no new board, tournament complete?) |

4. Call `vtable[0x12]()` on new board (initialization)
5. Add board to scene via `Scene_AddObject`
6. Set up score thresholds based on difficulty:
   - Difficulty 0 (Easy): `+1000` to target, `+0.25` to score multiplier
   - Difficulty 1 (Normal): `+500` to target, `+0.15` to score multiplier
7. Update player score slots for 4 players (App+0x5E8/688/728/7C8)
   - For races < 3 (Warm-Up, Beginner, Intermediate): write to "current" slot
   - For races ≥ 3: write to "best" slot
8. Iterate player data (4 players, stride 0xA0):
   - If player not active (`App+0x5D8 == 0`):
     - Set `App+0x60C = raceIndex - 1` (current race for this player)
     - Copy race name from `board+0x29B4` to player's name string (`App+0x610`)

### 3b. `TourneyMenu_ctor` (0x44FDA0) — The Tournament Menu

**This is where level unlocking happens.** When the tournament menu is constructed after a race:

1. If `profile+0x96 == 0` (not results-display mode):
   - Shows "PLAY!" button (triggers `Tournament_AdvanceRace(profile, 0)`)
   - Shows "ROLLBACK" button (triggers `TourneyMenu_Advance`)
   - Shows "MAIN MENU" button

2. If `profile+0x96 != 0` (results-display mode):
   - Shows rank image based on score
   - Looks up rank from score threshold table at 0x4F710C
   - Loads rank texture from `textures\ranks\%d.jpg`

3. **Level unlock logic** (when `App+0x23C != 0`, difficulty is set):
   - Switch on `profile+0x08` (current race index):

| Race Index | Unlock Flag Offset | Level Unlocked |
|------------|-------------------|----------------|
| 3 (Intermediate) | scene+0x851 | Intermediate completed |
| 4 (Dizzy) | scene+0x852 | Dizzy completed |
| 5 (Tower) | scene+0x853 | Tower completed |
| 6 (Up) | scene+0x863 | Up completed |
| 7 (Neon) | scene+0x854 | Neon completed |
| 8 (Expert) | scene+0x855 | Expert completed |
| 9 (Odd) | scene+0x856 | Odd completed |
| 10 (Toob) | scene+0x857 | Toob completed |
| 0xB (Wobbly) | scene+0x864 | Wobbly completed |
| 0xC (Glass) | scene+0x858 | Glass completed |
| 0xD (Sky) | scene+0x859 | Sky completed |
| 0xE (Master) | scene+0x865 | Master completed |

   - If the flag was `0` (not yet completed):
     - Sets `menu+0x1108 = 1` (newly-unlocked flag)
     - Sets the completion flag to `1`
   - If any level was newly unlocked:
     - Plays unlock sound (`scene+0x518`)

4. **Save game**: Calls `TourneyMenu_WriteSave(profile, "DATA\\TOURNAMENT.SAV")` every time the menu is constructed.

5. Shows race description text from table at 0x4F7148 (indexed by race index).

### 3c. `CheckArenaUnlock` (0x40ABA0) — Arena Unlock Notification

Called from: `DispatchCollisionEvents` (0x40C5D0)

This is a **one-time notification** that fires when you first play a level whose arena hasn't been unlocked yet. It's separate from the tournament progression.

- Reads `board+0x878` = scene, `scene+0x220` = PlayerProfile, `profile+0x08` = race index
- Switch on race index maps to scene "arena unlocked" flags:

| Race Index | Arena Unlock Flag | Level |
|------------|------------------|-------|
| 4 | scene+0x85A | Dizzy arena |
| 5 | scene+0x85B | Tower arena |
| 6 | scene+0x85C | Up arena |
| 7 | scene+0x866 | Neon arena |
| 8 | scene+0x85D | Expert arena |
| 9 | scene+0x85E | Odd arena |
| 10 | scene+0x85F | Toob arena |
| 0xB | scene+0x860 | Wobbly arena |
| 0xC | scene+0x867 | Glass arena |
| 0xD | scene+0x861 | Sky arena |
| 0xE | scene+0x862 | Master arena |
| 0xF | scene+0x868 | Impossible arena |

- If the arena flag is `0` (not yet unlocked):
  - Plays unlock sound (`scene+0x518`)
  - Sets arena flag to `1`
  - Copies some matrix/graphics data
  - Sets ball name to "ARENA UNLOCKED!" (displays on screen)

**Note:** The "race completed" flags (0x851–0x865) and "arena unlocked" flags (0x85A–0x868) are **different sets of bytes** in the scene struct. Completing a race in tournament mode sets the completion flag; the arena unlock flag is set the first time you play that level's arena.

### 3d. `TourneyMenu_CreateBoard` (0x426780)

Creates an Arena board (not a race board) for arena mode. Switch on race index (1–0xF):
- Creates `ArenaBoard_*_ctor` for each level (different alloc sizes from race boards)
- Calls `vtable[0x12]()` for initialization
- Adds to scene

### 3e. `TourneyMenu_Advance` (0x4266F0)

Triggered by the "ROLLBACK" button on the tournament menu:
- Sets `profile+0x95 = 1` (advance flag)
- Decrements `profile+0x08` (race index — goes back one race)
- Creates new `TourneyMenu` with `param_2=0` (non-results mode)
- Adds to scene

### 3f. `Tourney_AdvanceRound` (0x450960)

Advances to the next round in multiplayer tournament:
- If `profile+0x95 == 0`: decrements `profile+0x08` (race index)
- If `profile+0x95 != 0`: clears `profile+0x95`
- Sets `profile+0x94 = 1`
- Calls `vtable[0x40]()` (destructor on current board)
- Creates new `TourneyMenu` with `param_2=1` (results mode)

---

## 4. Save Game System

### 4a. `TourneyMenu_WriteSave` (0x4264B0) — `__thiscall(this=PlayerProfile, param_1=filename)`

Writes tournament save file with the following layout:

| Offset in file | Source | Size | Purpose |
|----------------|--------|------|---------|
| 0x00 | profile+0x08 | 4 bytes | Current race index |
| 0x04 | profile+0x14 | 4 bytes | Current race score |
| 0x08 | profile+0x18 | 0x3C (60) | Per-race score array (15 × 4) |
| 0x44 | profile+0x54 | 0x3C (60) | Per-race time array (15 × 4) |
| 0x80 | profile+0x90 | 4 bytes | Total score |
| 0x84 | profile+0x94 | 1 byte | Save flag |
| 0x85 | profile+0x95 | 1 byte | Won/advance flag |
| 0x86 | App+0x236 | 1 byte | Split-screen mode |
| 0x87 | App+0x23C | 4 bytes | Difficulty (0/1/2) |
| 0x8B | App+0x5E8 | 4 bytes | P1 score |
| 0x8F | App+0x5E4 | 4 bytes | P1 best score |
| 0x93 | App+0x5F4 | 4 bytes | (additional score data) |

File: `DATA\TOURNAMENT.SAV`

### 4b. `TourneyMenu_LoadSaveAndShow` (0x4265A0)

Reads the save file back into the PlayerProfile and App struct, then creates a `TourneyMenu` with `param_2=1` (results mode) to show the loaded tournament state.

### 4c. `Tourney_SaveTournament` (0x446730)

- Calls `vtable[0x40]()` (destructor)
- Removes `DATA\TOURNAMENT.SAV`
- Calls save write method via vtable

---

## 5. Complete Flow Diagram

```
RACE IN PROGRESS
    │
    ▼
Board_UpdateRaceState (per-frame)
    │ Ball crosses finish line
    ▼
RaceResultPopup created + "Game Over" music
    │
    ▼
Scene_ProcessRaceEnd (via scene vtable)
    │ Checks all balls finished (state==5)
    │
    ├─── Tournament mode (profile+0x10==0, +0x11==0):
    │        └── Tournament_AdvanceRace(profile, 1)
    │             └── Creates TourneyMenu (results mode)
    │                  ├── Unlocks next level (sets scene+0x85x flag)
    │                  ├── Plays unlock sound if newly unlocked
    │                  ├── Writes save file (TOURNAMENT.SAV)
    │                  └── Shows "PLAY!" button
    │                       └── User clicks PLAY!
    │                            └── Tournament_AdvanceRace(profile, 0)
    │                                 ├── Increments race index
    │                                 ├── Creates next LevelBoard_*_ctor
    │                                 ├── Calls board vtable[0x12]() (init)
    │                                 ├── Sets score thresholds
    │                                 └── App_StartRace()
    │                                      └── NEXT RACE BEGINS
    │
    ├─── Practice mode (profile+0x10!=0):
    │        ├── App_ShowResults() → MusicPlayer results screen
    │        ├── Creates PracticeMenu
    │        └── App_StartRace() → next practice race
    │
    ├─── Time Trial (profile+0x11!=0, App+0x234==0):
    │        ├── App_StartRace()
    │        ├── App_ShowResults()
    │        └── Creates TimeTrialMenu
    │
    ├─── Arena mode (App+0x234!=0, App+0x237!=0):
    │        ├── App_StartRace()
    │        ├── App_ShowResults()
    │        └── Creates ArenaMenu
    │
    └─── Party mode (App+0x234!=0, App+0x237==0):
             ├── App_StartRace()
             ├── App_ShowResults()
             └── Creates PartyMenu
```

## 6. Key Differences: Race Board vs Arena Board

`Tournament_AdvanceRace` creates **race boards** (`LevelBoard_*_ctor`) for tournament progression.
`TourneyMenu_CreateBoard` creates **arena boards** (`ArenaBoard_*_ctor`) for arena mode.

These are different classes with different allocation sizes — arena boards are generally smaller than race boards (e.g. Beginner race = 0x644C bytes, Beginner arena = 0x5850 bytes).

## 7. Level Unlock Flag Summary

Two separate flag systems in the Scene struct:

**Race Completion Flags** (set by TourneyMenu_ctor after winning in tournament mode):
- scene+0x851: Intermediate completed
- scene+0x852: Dizzy completed
- scene+0x853: Tower completed
- scene+0x854: Neon completed
- scene+0x855: Expert completed
- scene+0x856: Odd completed
- scene+0x857: Toob completed
- scene+0x858: Glass completed
- scene+0x859: Sky completed
- scene+0x863: Up completed
- scene+0x864: Wobbly completed
- scene+0x865: Master completed

**Arena Unlock Flags** (set by CheckArenaUnlock on first play):
- scene+0x85A: Dizzy arena unlocked
- scene+0x85B: Tower arena unlocked
- scene+0x85C: Up arena unlocked
- scene+0x85D: Expert arena unlocked
- scene+0x85E: Odd arena unlocked
- scene+0x85F: Toob arena unlocked
- scene+0x860: Wobbly arena unlocked
- scene+0x861: Sky arena unlocked
- scene+0x862: Master arena unlocked
- scene+0x866: Neon arena unlocked
- scene+0x867: Glass arena unlocked
- scene+0x868: Impossible arena unlocked

Note: Warm-Up (index 1) and Beginner (index 2) have no unlock flags — they are always available.
Impossible (index 0xF) only has an arena unlock flag (0x868), no race completion flag in TourneyMenu_ctor (the switch ends at case 0xE for Master).
