# Multi-Player System

How Hamsterball handles 2-4 players across Party Race and Rodent Rumble.

## Game Modes

| Mode | Players | Split-screen | Timer HUD | Function |
|------|---------|-------------|-----------|----------|
| Party Race | 2 (human only) | Yes, 2-way (left/right) | None | `App_StartPracticeRace` (0x428C50) |
| Rodent Rumble | 1-4 (human or CPU) | No, single screen | Arena timer | `ArenaBoard_Render` (0x421910) |
| Tournament | 1 (P1 only) | No | Yes | `App_StartTournamentRace` (0x4288B0) |
| Time Trial | 1 | No | Yes | `App_StartPracticeRace` (0x428C50) |

## Player Slots

The App struct has 4 player_data slots at stride 0xA0:

| Player | Data offset | Active flag | Joined flag | Control slot |
|--------|------------|-------------|-------------|-------------|
| P1 | App+0x5CC | App+0x5D7 (byte) | App+0x5D4 (byte) | App+0xB28 |
| P2 | App+0x66C | App+0x677 (byte) | App+0x674 (byte) | App+0xB2C |
| P3 | App+0x70C | App+0x717 (byte) | App+0x714 (byte) | App+0xB30 |
| P4 | App+0x7AC | App+0x7B7 (byte) | App+0x7B4 (byte) | App+0xB34 |

Control slot values: 0-3 = device index, 99 = OFF (no player), 100 = COMPUTER (AI).

Strings exist for all 4 players: "PLAYER 1-4: %s", "PLAYER 1-4: OFF", "PLAYER 1-4: COMPUTER".

The Options menu (`OptionsMenu_RenderControls` at 0x42E840) iterates all 4 control slots (0xB28-0xB38) and renders CONTROL1-4 settings. Registry keys CONTROL1-4 persist these assignments.

## Menu Flow (FUN_00430DA0)

The main multiplayer menu handler at 0x430DA0 processes button actions:

### Party Race ("2PRACE")

```
if (control2 < 99) {   // P2 is human (not OFF)
    // ALLOW — go to PartyMenu
    // Zeros P3/P4 slots (0xB38-0xB44 = 0)
} else {
    // BLOCK — show error dialog:
    // "THE PARTY RACE REQUIRES THAT PLAYER 1 AND PLAYER 2 BE HUMAN PLAYERS!"
}
```

**Only checks P2.** Does not check P3/P4 at all. If P2 is human, the party race starts regardless of P3/P4 status. However, P3/P4 slots are zeroed before entering PartyMenu, so they are effectively ignored.

Menu label: "PARTY RACE (2P ONLY)" (string at 0x4D4D98, referenced by `MPMenu_ctor`).

### Rodent Rumble ("RUMBLE")

```
if (control2 == 100 && control3 == 100 && control4 == 100) {
    // BLOCK — all three are OFF → error:
    // "THE RODENT RUMBLE REQUIRES AT LEAST TWO HUMAN OR COMPUTER PLAYERS!"
} else {
    // ALLOW — go to ArenaMenu
    // Zeros P3/P4 slots (0xB38-0xB44 = 0)
}
```

Checks P2, P3, P4 — only blocks if ALL are OFF (value 100). Accepts any mix of human (0-3) and computer (100) players. Also zeros the P3/P4 slots before entering ArenaMenu.

Menu label: "RODENT RUMBLE (1-4P)" (string at 0x4D4D80).

### Tournament ("2PT" / "2PMT")

Hardcodes P1 and P2 as the only active players:
```
App+0x5D5 = 0;  App+0x5D7 = 0;  App+0x5D4 = 1;  // P1 joined, active
App+0x675 = 0;  App+0x677 = 0;  App+0x674 = 1;  // P2 joined, active
App+0x235 = 1;  // party mode flag
App+0x236 = 0/1; // mirror flag (0=normal "2PT", 1=mirror "2PMT")
```

No P3/P4 activation. Goes to DifficultyMenu.

## Player Activation (App_StartPracticeRace @ 0x428C50)

When starting a party race or time trial:

```
App+0x717 = 1;  // P3 active
App+0x7B7 = 1;  // P4 active
App+0x5D7 = 0;  // P1... (see below)
App+0x677 = 1;  // P2 active
if (App+0x234) {  // party mode flag
    App+0x677 = 0;  // P2 inactive in party mode
}
App+0x23C = 1;  // difficulty/practice flag
```

The `App+0x234` flag distinguishes party mode (true) from time trial (false). In party mode, P2 is deactivated after initial setup.

## Split-Screen Rendering (Scene_Render @ 0x41A2E0)

`Scene_Render` checks `AthenaList_GetSize(scene+0x362C)` for the number of viewports:

| Count | Behavior |
|-------|----------|
| 0 | Single screen, no camera target (menu/demo mode) |
| 1 | Single screen, camera follows P1 ball (time trial, tournament) |
| 2 | **Two-way split** — iterates 2 entries, calls `Graphics_SetViewport` with different params per half, camera follows each player's ball |
| 3+ | **No case exists** — function returns without rendering |

### 2-way split details

For the 2-way split:
1. Gets AthenaList iterator from scene+0x362C
2. Iterates through entries (each entry has a ball pointer at +0x10)
3. For each entry:
   - Converts float viewport params via `__ftol2` 
   - Calls `Graphics_SetViewport(gfx, param1, param2)` with viewport coordinates
   - Sets ball pointer: `scene+0x29D0 = entry->ball`
   - Calls `Scene_SetCamera(scene, ball, 1)`
   - Renders 3D scene + reflective objects
4. After loop: `Graphics_SetViewport(gfx, 0, 0)` (reset to full screen)
5. Renders UI overlay on full screen

The viewport parameters come from float values on the stack — the 2-way split uses:
- Left half: viewport at (0, 0) covering left portion
- Right half: viewport covering right portion

### No 4-way split exists

There is **no case for 4 viewports** in `Scene_Render`. The function checks for 0, 1, and 2 only. After the `CMP EAX, 0x2 / JNZ` at 0x41A41B, if count != 2, execution falls through to the function epilogue.

A `UI_SetQuarterViewport` function exists at 0x4415B0 but is never called during gameplay — it sets 50px borders (0x32) and appears to be leftover from planned 4-player support that was never completed for race modes.

## Rodent Rumble Rendering

Rodent Rumble uses `ArenaBoard_Render` (0x421910) which renders all players on a **single screen** — no split-screen needed since all 4 balls share the same arena platform.

`ArenaBoard_Render` handles up to 4 player scores:
- P1 score: drawn at x=0x4B, reads from board+0x47B4
- P2 score: drawn at x=0x4B, reads from board+0x47B8 (if App+0x677 is active)
- P3 score: drawn at x=0x2D5, reads from board+0x47BC (if App+0x717 is active)
- P4 score: drawn at x=0x2D5, reads from board+0x47C0 (if App+0x7B7 is active)

The arena timer (board+0x47AC) is drawn in the center-top area.

## Ball Storage

| Slot | Location | Used by |
|------|----------|---------|
| P1 ball (direct ptr) | scene+0x29D0 | All modes |
| P2 ball (AthenaList) | scene+0x3204 | Party Race, Tournament |
| All balls (AthenaList) | scene+0x29D4 | Rodent Rumble (up to 4) |

## What Would 4-Player Party Race Require

Since the engine already handles 4 balls and 4 players in Rodent Rumble, the main missing piece is **split-screen rendering for 4 viewports**:

1. **Scene_Render**: Add a `size==4` case with 4 quadrant viewports (top-left, top-right, bottom-left, bottom-right), each calling `Graphics_SetViewport` with quadrant coordinates and `Scene_SetCamera` for each player's ball
2. **Ball spawning**: Party Race currently spawns 2 balls; would need 4 (the engine already supports this in Rodent Rumble)
3. **Camera**: 4 independent cameras following 4 balls (already done in Rodent Rumble for single-screen, just needs per-viewport version)
4. **HUD**: Party Race has no timer HUD, so no HUD changes needed. Score display could reuse ArenaBoard_Render's 4-player score layout, but Party Race uses race-style levels not arena levels.

The `UI_SetQuarterViewport` function (0x4415B0) may have been intended for this purpose but was never wired into the render pipeline.

## Key Addresses

| Address | Function | Purpose |
|---------|----------|---------|
| 0x430DA0 | FUN_00430DA0 | Multiplayer menu handler (2PRACE/RUMBLE/2PT/2PMT/2PP) |
| 0x428C50 | App_StartPracticeRace | Starts party race or time trial |
| 0x4288B0 | App_StartTournamentRace | Starts tournament race |
| 0x41A2E0 | Scene_Render | Split-screen renderer (0/1/2 viewports) |
| 0x421910 | ArenaBoard_Render | Arena mode renderer (1-4 players, single screen) |
| 0x4415B0 | UI_SetQuarterViewport | Unused quarter-viewport setup (50px borders) |
| 0x4415F0 | UI_ResetViewportToQuarter | Unused quarter-viewport reset |
| 0x42E840 | OptionsMenu_RenderControls | Renders 4 control slots in options |
| 0x42FC10 | PartyMenu_ctor | Party race level selection menu |
| 0x42E220 | DifficultyMenu_ctor | Difficulty selection (Pipsqueak/Normal/Frenzied) |

## Menu Strings

| Address | String | Context |
|---------|--------|---------|
| 0x4D4D98 | "PARTY RACE (2P ONLY)" | Menu label |
| 0x4D4D80 | "RODENT RUMBLE (1-4P)" | Menu label |
| 0x4D4A90 | "THE PARTY RACE REQUIRES THAT PLAYER 1 AND PLAYER 2 BE HUMAN PLAYERS!" | Error dialog |
| 0x4D4A30 | "THE RODENT RUMBLE REQUIRES AT LEAST TWO HUMAN OR COMPUTER PLAYERS!" | Error dialog |
| 0x4D48C0-0x4D497C | "PLAYER 1-4: %s/OFF/COMPUTER" | Player status strings |
| 0x4D4260 | "CONTROL%d" | Registry key format |
