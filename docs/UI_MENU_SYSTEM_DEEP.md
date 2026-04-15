# UI & Menu System

## Overview
Hamsterball's menu system uses a class hierarchy:
- **SimpleMenu** (base, 0x870 bytes) — title, UIList, button handling
- **MainMenu** — vtable 0x4D3F30, title "Main Menu"
- **DifficultyMenu** — vtable 0x4D40A8, title "CHOOSE A DIFFICULTY!"
- **PracticeMenu, TimeTrialMenu, PartyMenu** — level selection
- **TourneyMenu** — tournament progression
- **OptionsMenu** — settings with control rendering
- **PauseMenu** — in-race pause
- **QuitRaceMenu** — quit confirmation
- **HighScoreMenu** — score display
- **ConfirmMenu** — generic confirmation dialog

## Menu Class Hierarchy

```
Gadget (0x870 bytes)
  └─ SimpleMenu (derived from Gadget)
       ├─ MainMenu (vtable 0x4D3F30)
       ├─ DifficultyMenu (vtable 0x4D40A8)
       ├─ PracticeMenu
       ├─ TimeTrialMenu
       ├─ PartyMenu
       ├─ TourneyMenu
       ├─ OptionsMenu
       ├─ PauseMenu
       └─ ConfirmMenu
```

## MainMenu Items (0x42DE50)

Menu items are added via `UIList_AddItem(menu, "Display Text", "ID_CODE", ...)`:

| Display Text | ID Code | Condition | Scale Factor |
|-------------|---------|-----------|--------------|
| LET'S PLAY! | PLAY | Always | (0.75, 1.0, 0.75, 1.0) |
| (spacer 10) | - | Always | - |
| HIGH SCORES | HS | Always | (1.0, 1.0, 1.0, 1.0) |
| OPTIONS | OP | Always | (1.0, 1.0, 1.0, 1.0) |
| CREDITS | CR | Always | (1.0, 1.0, 1.0, 1.0) |
| REGISTER GAME | RG | If App+0x200==0 (not registered) | (1.0, 1.0, 0, 1.0) |
| (spacer 10) | - | Always | - |
| (mini-game) | MG | If App+0x918 && App+0xB24 | (width, height) |
| (spacer 10) | - | If mini-game shown | - |
| EXIT TO DESKTOP | EXIT | Always | (1.0, 0.75, 0.75, 1.0) |

## DifficultyMenu Items (0x42E220)

| Display Text | ID Code | Scale Factor |
|-------------|---------|--------------|
| PIPSQUEAK | EASY | (0.7, 1.0, 0.7, 1.0) |
| NORMAL | NORMAL | (1.0, 1.0, 0.7, 1.0) |
| FRENZIED! | HARD | (1.0, 0.5, 0.5, 1.0) |
| (spacer 10) | - | - |
| PREVIOUS | BACK | (0.75, 0.75, 1.0, 1.0) |

### Difficulty Mapping
- EASY (0): MouseSensitivity default, slower AI
- NORMAL (1): Default game speed
- HARD (2): Frenzied mode — faster objects, harder tracks

## SimpleMenu Layout (0x870 bytes, extends Gadget)

Key offsets specific to SimpleMenu:
| Offset | Type | Description |
|--------|------|-------------|
| +0x44C | AthenaList | Font string list |
| +0x868 | char* | Menu title string |
| +0x86C | AthenaList | Item list (alternate) |
| +0x88C | AthenaList | UIListItem list |
| +0xCAC | int | Min width |
| +0xCB0 | int | Max width |
| +0xCB4 | int | Total height |
| +0xCBC | byte | Needs recalculation flag |

## UIList_AddItem (0x449820)

Creates UIListItem (0x444 bytes) per menu entry:
1. Allocate UIListItem (0x444 bytes)
2. Copy display text string → item+0x00 (strdup)
3. Copy ID code string → item+0x04 (strdup)
4. Store matrix transform → item+0x0C..0x18
5. Create AthenaString (0x1C bytes) for rendering → item+0x1C
6. Store font reference → item+0x20
7. Append to font list (SimpleMenu+0x44C)
8. Append to item list (SimpleMenu+0x88C)
9. Calculate text width via Font_MeasureText
10. Update menu total height (SimpleMenu+0xCB4)
11. Update menu max width (SimpleMenu+0xCB0)

## UIListItem Structure (0x444 bytes)

| Offset | Type | Description |
|--------|------|-------------|
| +0x00 | char* | Display text (e.g., "LET'S PLAY!") |
| +0x04 | char* | ID code (e.g., "PLAY") |
| +0x0C | float[4] | Matrix scale row (X1, X2, Y1, Y2) |
| +0x1C | AthenaString* | Rendered text object |
| +0x20 | int | Font data reference |
| +0x24 | int | Item height |
| +0x110 | byte | Hidden flag |
| +0x441 | byte | Disabled flag |

## Menu Command Dispatch

When a menu item is selected, the ID code string is dispatched through
App_ShowMainMenu (0x4280E0) which uses a strcmp chain:

```
"PLAY"    → Show difficulty selection
"HS"      → Show high scores
"OP"      → Show options menu
"CR"      → Show credits
"RG"      → Show registration
"MG"      → Launch mini-game
"EXIT"    → Quit to desktop
"EASY"    → Start practice race (difficulty 0)
"NORMAL"  → Start practice race (difficulty 1)
"HARD"    → Start practice race (difficulty 2)
"BACK"    → Return to previous menu
```

## Pause System

### PauseMenu_Ctor (0x42E4B0)
In-race pause menu with:
- RESUME → continue racing
- QUIT → quit race menu

### QuitRaceMenu (0x42E6F0)
Confirmation dialog:
- YES, QUIT → end race
- NO, CONTINUE → resume racing

## TourneyMenu System

### TourneyMenu_Advance (0x42E210)
Handles tournament progression after race completion.

### TourneyMenu_GetRaceName (0x4264A0)
Returns race name string for current tournament position.

### TourneyMenu_WriteSave (0x4264B0)
Saves tournament progress to registry.

### TourneyMenu_LoadSaveAndShow (0x4265A0)
Loads saved tournament state and displays current race.

## GraphicsOptionsMenu (0x42E840)

OptionsMenu_RenderControls renders control mapping:
- Lists all DirectInput devices
- Shows key bindings (up/down/left/right/action)
- Supports keyboard, mouse, joystick remapping

## LoadingScreenGadget (0x3628 bytes)

Created by TimerDisplay (0x4298C0 — actually App_ResourceLoader).
Manages the loading screen during initial asset loading.
See ASSET_MANIFEST.md for complete list of loaded resources.

## Menu Input Handling

Scene_HandleInput (0x4692F0):
1. Checks each gadget in scene's gadget list (Scene+0x858)
2. If gadget is input active (+0x16 != 0):
   - Dispatches input through gadget vtable[1]
   - If gadget captures input (+0x14 != 0): marks as active
3. Player 1 uses App+0x1E4 input channel
4. Player 2 uses App+0x1E8 input channel
5. Plays sound effect via App+0x1DC sound channel

## Key Menu Functions

| Address | Function | Description |
|---------|----------|-------------|
| 0x42DE50 | MainMenu_ctor | "Main Menu" — PLAY/HS/OP/CR/RG/MG/EXIT |
| 0x42E220 | DifficultyMenu_ctor | "CHOOSE A DIFFICULTY!" — EASY/NORMAL/HARD/BACK |
| 0x42EA30 | PracticeMenu_ctor | Level selection for practice mode |
| 0x42F810 | TimeTrialMenu_ctor | Level selection for time trials |
| 0x42FC10 | PartyMenu_ctor | Level selection for party/multiplayer |
| 0x42E4B0 | PauseMenu_Ctor | In-race pause: RESUME/QUIT |
| 0x42E6F0 | QuitRaceMenu | Quit confirmation: YES/NO |
| 0x42B190 | ConfirmMenu_ctor | Generic yes/no dialog (6 xrefs) |
| 0x42B890 | HighScoreMenu_Render | High score display |
| 0x449820 | UIList_AddItem | Add menu item with display text + ID code |
| 0x4497F0 | UIList_AddSpacer | Add vertical spacing |
| 0x4280E0 | App_ShowMainMenu | Menu command dispatcher |
| 0x4284C0 | App_SaveAllConfig | Save settings to registry on menu exit |
| 0x4279F0 | LoadOrSaveConfig | Cleanup + save on game exit |