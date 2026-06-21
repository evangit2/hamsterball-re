# UI/Menu System

Hamsterball's UI is a hierarchical menu system built on top of the SceneObject
framework. All menus are SceneObjects added to the scene graph. The system uses
a SimpleMenu base class with UIList widget for vertical scrolling lists.

## Menu Class Hierarchy

```
SceneObject (base)
└── SimpleMenu (0x448F20) — base menu with UIList
    ├── MainMenu (0x4D3F30) — "LET'S PLAY!", HIGH SCORES, OPTIONS, CREDITS, EXIT
    ├── DifficultyMenu (0x4D40A8) — PIPSQUEAK, NORMAL, FRENZIED!
    ├── PauseMenu (0x4D4120) — RESUME, OPTIONS, ABORT RACE
    ├── PauseRumbleMenu (0x430330) — pause for RumbleBoard modes
    ├── PracticeMenu (0xD18 bytes) — level select for practice
    ├── TimeTrialMenu (0xD18 bytes) — level select for time trial
    ├── ArenaMenu (0x42FC40) — RumbleBoard arena select
    ├── MPMenu (0x4D3F30, 0xCDC bytes) — multiplayer party setup
    ├── HighScoreMenu (0x42B290) — high scores display
    ├── OptionsMenu — graphics/sound/controls
    ├── KeyRemapMenu (0x4D4340) — key binding screen
    ├── GraphicsOptionsMenu (0x441E70) — quality settings
    └── TournamentScreen (0x42E060) — tournament results
```

## Dialog Classes

```
SceneObject (base)
├── OkayDialog (0x440E70, 0x8A8 bytes) — simple message + OK button
├── ConfirmMenu (0x4C780, 0x8A8 bytes) — YES/NO confirmation
├── QuitDialog (0x43E30) — quit race confirmation + continue tourney
├── QuitAbortDialog (0x45320) — abort race dialog
├── QuitToDesktopDialog (0x44790) — exit to desktop confirmation
├── SaveTourneyDialog (0x4FD60) — save tournament progress
├── TourneyContinueDialog (0x45E60, 0x8BC bytes) — resume saved tournament?
└── RegisterDialog (0x476B0) — serial key registration
```

## SimpleMenu Base (0x448F20)

The fundamental menu class. All game menus inherit from this.

### Key Offsets (SimpleMenu and subclasses)

| Offset | Type | Field |
|--------|------|-------|
| +0x868 | char* | Title text (e.g. "Main Menu", "Pause Menu") |
| +0x878 | App* | App pointer (set via SimpleMenu_ctor param) |

### SimpleMenu_ctor Flow

1. Call SceneObject base constructor
2. Set vtable pointer for subclass
3. Set title text at `this+0x868`
4. Add menu items via `UIList_AddItem(this, display_text, command_text, colors...)`
5. Add spacers via `UIList_AddSpacer(this, pixel_height)`

## UIList Widget System

### UIList Functions

| Address | Function | Description |
|---------|----------|-------------|
| 0x4492D0 | UIList_AddItem | Add selectable item to list |
| 0x4497F0 | UIList_AddItem (alt) | Add item with format string |
| 0x4490F0 | UIList_AddItemWithFormat | Add formatted item |
| 0x449430 | UIList_AddSpacer | Add vertical spacer |
| 0x449750 | UIList_ActivateCurrentItem | Click current selection |
| 0x449C20 | UIList_HandleKeyNav | Keyboard/gamepad navigation |
| 0x44A570 | UIList_Layout | Recalculate item positions |
| 0x449D40 | UIList_Render | Draw all items |
| 0x4494D0 | UIList_ScrollUpdate | Handle scroll animation |
| 0x44A970 | UIList_SetColorsByName | Set item colors by command |
| 0x44A8B0 | UIList_SetTextByName | Set item text by command |

### UIListItem (0x4490A0)

Each item in the list stores:
- Display text (shown on screen)
- Command text (dispatched on click, e.g. "PLAY", "BACK", "1PT")
- Color matrix (4x4 Scale for tinting)
- Spacer flag + height

## Menu Dispatch Tables

### Main Menu (0x42DE50)

| Display | Command | Handler |
|---------|---------|---------|
| "LET'S PLAY!" | "PLAY" | TournamentManager_ctor |
| "HIGH SCORES" | "HS" | HighScoreMenu_ctor |
| "OPTIONS" | "OP" | OptionsMenu |
| "CREDITS" | "CR" | Credits screen |
| "REGISTER GAME" | "RG" | RegisterDialog (only if not registered, App+0x200==0) |
| (dynamic) | "MG" | Mini-game button (only if App+0x918 && App+0xB24) |
| "EXIT TO DESKTOP" | "EXIT" | QuitToDesktopDialog |

### Difficulty Menu (0x42E220)

| Display | Command | Difficulty Value |
|---------|---------|-----------------|
| "PIPSQUEAK" | "EASY" | 0 (easiest, +1000ms bonus) |
| "NORMAL" | "NORMAL" | Standard |
| "FRENZIED!" | "HARD" | 1 (hardest, +500ms bonus) |
| (spacer) | | |
| "PREVIOUS" | "BACK" | Return to previous menu |

Title: "CHOOSE A DIFFICULTY!"

### Pause Menu (0x42E4B0)

| Display | Command | Handler |
|---------|---------|---------|
| "RESUME" | "RESUME" | Unpause game |
| "OPTIONS" | "OP" | Options submenu |
| (spacer) | | |
| "ABORT RACE" | "BACK" | Quit current race |

Title: "Pause Menu"

### RumbleBoard Menu (0x433AC0 — TournamentManager)

See [RUMBLEBOARD_SYSTEM.md](RUMBLEBOARD_SYSTEM.md) for full dispatch table.

### Play Mode Selection (from TournamentManager)

| Display | Command | Mode |
|---------|---------|------|
| 1-Player Tournament | "1PT" | Tournament, is_mirror=0 |
| 1-Player Mirror Tournament | "1PMT" | Tournament, is_mirror=1 |
| 1-Player Practice | "1PP" | Practice, is_tournament=0 |
| 1-Player Time Trial | "1PTT" | TimeTrial, is_tournament=0 |
| Multiplayer Party | "PARTY" | MP mode |

## Menu Color System

Each UIList_AddItem takes a 4x4 matrix for color tinting. Common color patterns:

| Pattern | Hex Values | Usage |
|---------|-----------|-------|
| Highlight (bright) | 1.0, 1.0, 0.7, 1.0 | Selected item |
| Normal | 1.0, 1.0, 1.0, 1.0 | Standard item |
| Dimmed | 0.75, 0.75, 0.5, 1.0 | Less important item |
| Warning (red) | 1.0, 0.5, 0.5, 1.0 | Destructive action (Abort/Exit) |
| Dim red | 0.75, 0.5, 0.5, 1.0 | Less important destructive |
| Pipsqueak | 0.7, 1.0, 0.7, 1.0 | Green tint (easiest) |
| Frenzied | 1.0, 0.5, 0.5, 1.0 | Red tint (hardest) |
| Register | 1.0, 1.0, 0.0, 1.0 | Yellow (limited time) |

## App Menu State

| App Offset | Type | Field |
|-----------|------|-------|
| +0x184 | void* | Scene object list (for Scene_AddObject) |
| +0x224 | void* | Current main menu pointer |
| +0x235 | byte | is_tournament (0=practice/trial, 1=tournament) |
| +0x236 | byte | is_mirror (0=normal tracks, 1=mirror tracks) |
| +0x237 | byte | tournament_complete flag |
| +0x200 | byte | is_registered (1=registered, show "REGISTER" if 0) |
| +0x5E8 | int | P1 time display |
| +0x688 | int | P2 time display |
| +0x728 | int | P3 time display |
| +0x7C8 | int | P4 time display |
| +0x918 | byte | has_mini_game (show "MG" button) |
| +0xB24 | char* | mini_game_label (dynamic button text) |

## App Frame Lifecycle

### App_FrameTick (0x46C9E0)

Called each frame when game is NOT paused (App+0x159 == 0):
1. `vtable[0x60]()` — SceneObject::Tick (update current scene objects)
2. `MeshWorld_CallVtable34(scene, param)` — Update mesh world
3. `MusicDevice_MuteToggle(param)` — Handle audio focus

### App_CompleteRace (0x425F90)

Called when a race finishes:
1. If `App+0x704 != 0` (race in progress):
   - Increment `App+0x7C8` (race completion counter)
   - Call `scene->vtable[0xFC](0, 13, 1)` — D3D surface operation 13
   - Call `scene->vtable[0xFC](0, 14, 1)` — D3D surface operation 14
   - Clear `App+0x704` (race no longer in progress)

### App_Is2PMode (0x427910)

Checks if 2-player mode is active:
```c
bool App_Is2PMode(App *app) {
    return ((app[app->p1_offset + 0xC] | app[app->p2_offset + 0xC]) >> 7) & 1;
}
```

## Dialog Details

### RegisterDialog (0x476B0)

Serial key registration dialog. Features:
- Text input for serial key
- `RegisterDialog_ValidateSerial(0x46B80)` — key validation
- `RegisterDialog_HandleInput(0x475A0)` — character input processing
- `RegisterDialog_HandleKey(0x48890)` — keyboard handler
- "REGISTER GAME" appears on Main Menu only when `App+0x200 == 0`

### OkayDialog (0x440E70)

Simple message display dialog:
```c
OkayDialog_ctor(new(0x8A8), app, title_text, message_text, display_time_ms);
```
Used for error messages, unlock notifications, etc.

### TourneyContinueDialog (0x45E60)

Appears when tournament save file exists (`DATA\TOURNAMENT.SAV`):
- "Continue saved tournament?" prompt
- On continue: loads saved state
- On new game: starts fresh tournament

## Related Functions

| Address | Name | Purpose |
|---------|------|---------|
| 0x448F20 | SimpleMenu_ctor | Base menu constructor |
| 0x42DE50 | MainMenu_ctor | Main menu setup |
| 0x42E220 | DifficultyMenu_ctor | Difficulty selection |
| 0x42E4B0 | PauseMenu_Ctor | In-game pause |
| 0x433AC0 | TournamentManager | Title screen dispatcher |
| 0x4492D0 | UIList_AddItem | Add menu item |
| 0x449430 | UIList_AddSpacer | Add vertical gap |
| 0x449C20 | UIList_HandleKeyNav | Input navigation |
| 0x449D40 | UIList_Render | Draw menu |
| 0x440E70 | OkayDialog_ctor | Message dialog |
| 0x443E30 | QuitDialog_ctor | Race quit dialog |
| 0x44790 | QuitToDesktopDialog_Ctor | Exit confirmation |
| 0x45E60 | TourneyContinueDialog_Ctor | Resume tournament |
| 0x476B0 | RegisterDialog_ctor | Serial key entry |
| 0x46B80 | RegisterDialog_ValidateSerial | Key validation |