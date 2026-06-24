# No-Pause Mod v2

Prevents the pause menu from appearing via **any** input method — ESC key, right-click, or Win32 message pump. The game continues running normally with no pause overlay, no physics freeze, and no camera stop.

## Why v1 Didn't Work

v1 only patched **one of three** code paths that trigger pause. Pressing ESC still paused the game through the Win32 message pump path, and right-clicking paused through the mouse event handler path.

## The Three Pause Paths

`Scene_CreateGameOverMenu` (0x40a920) creates the pause overlay and sets `scene+0x874 = 1` (pause flag). When this flag is set, `GameUpdate` (0x469cf0) skips calling `Scene_Update` on the scene — freezing all physics and game logic.

There are **three** independent code paths that call `Scene_CreateGameOverMenu`:

### Path 1: DirectInput ESC Poll (Scene_Update)
```
Scene_Update (0x419c00)
  → Input_CheckKeyCombo(app, 2)    ; checks ESC via DirectInput
  → if pressed: Scene_CreateGameOverMenu(scene, 1)
```
Gating conditions: game state not in {3,4}, `scene+0x220 == 0`, demo timer inactive.

### Path 2: Right-Click Mouse Handler (vtable[5])
```
App_OnMouseDown(param_3=1=right button)
  → UIWidget_HitTest → vtable[5] on Scene
  → thunk 0x4130A0 checks param_3==1, App+0x238, profile+0x95
  → if conditions met: Scene_CreateGameOverMenu
```
This thunk appears in **32 scene-object vtables** — all scene types inherit this pause-on-right-click behavior.

### Path 3: Win32 Message Pump ESC (vtable[8])
```
WndProc → key message dispatch → vtable[8] on Scene
  → thunk 0x40B400 checks param == 0x1B (VK_ESCAPE = 27)
  → if match: JMP Scene_CreateGameOverMenu (tail call)
```
This is a **separate ESC detection path** from Path 1. Path 1 uses DirectInput polling; Path 3 uses the Win32 message pump. Both fire on ESC press. This thunk also appears in 32 vtables.

## The Patches

Three single-byte patches, one per path:

| Path | Address | Original | Patched | Effect |
|------|---------|----------|---------|--------|
| 1 — DirectInput ESC | 0x419d5b | `74 09` (JZ) | `EB 09` (JMP) | Always skip pause creation in Scene_Update |
| 2 — Right-click | 0x4130b5 | `74 17` (JZ) | `EB 17` (JMP) | Always skip pause in vtable[5] right-click thunk |
| 3 — Message pump ESC | 0x40b405 | `75 0D` (JNZ) | `EB 0D` (JMP) | Always skip pause in vtable[8] message handler thunk |

All three convert conditional jumps to unconditional jumps, causing the pause creation code to always be skipped.

## How Pause Works (for reference)

1. `Scene_CreateGameOverMenu` (0x40a920) creates a `PauseMenu` or `PauseRumbleMenu` overlay and sets `scene+0x874 = 1`
2. `GameUpdate` (0x469cf0) iterates scene objects each frame. For each object, it checks `obj[0x21d]` (byte at scene+0x874). When the flag is 1, it **skips calling `Scene_Update`** (vtable[1]) — freezing all game logic
3. When the player clicks "RESUME" in the pause menu, `PauseMenu_HandleButtonClick` sets `scene+0x874 = 0` — unfreezing the game

## Installation

1. Extract the zip into your Hamsterball game folder (next to `Hamsterball.exe`)
2. Run `install.bat`
3. Launch the game

## Uninstallation

Run `uninstall.bat` to restore the original `bass.dll`.

## Technical Details

- **Mod type:** BASS.dll proxy (v3 lazy loader pattern)
- **Patch type:** Three single-byte patches — conditional → unconditional jumps
- **Patch addresses:** RVA 0x19d5b, 0x130b5, 0x0b405 (VA 0x419d5b, 0x4130b5, 0x40b405)
- **Side effects:** None — ESC and right-click are simply ignored during gameplay. No menu, no pause, no freeze.
