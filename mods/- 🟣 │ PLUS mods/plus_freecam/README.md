# FreeCam

Freecam mod for Hamsterball Plus. Press **F7** to toggle between the game's normal camera and a free-flying camera you control. Press **F8** to hide/show all game UI text.

## Controls

| Key | Action |
|-----|--------|
| F7 | Toggle freecam on/off |
| F8 | Toggle all UI text on/off |
| W/S | Move forward/backward |
| A/D | Strafe left/right |
| E/Q | Move up/down |
| Arrow Keys | Look (yaw/pitch) |
| Left Shift | Hold for 3× movement speed |

## How it works

- On activation, the camera spawns at the player ball's position (offset back/up) and looks at the ball.
- `onRenderApply` replaces the view matrix each frame using `BuildCustomViewMatrix` with the computed eye position and forward vector.
- F8 hooks `Font_DrawGlyph` (RVA 0x57440) and early-returns to suppress all text rendering. The FREECAM indicator temporarily re-enables the hook to draw itself, then re-hides.
- On deactivation (F7 again, level change, or scene end), the game's normal camera resumes automatically.
- A green "FREECAM" indicator is drawn at x=200 while active.

## Build

Visual Studio (32-bit / x86 DLL). Place `FreeCam.dll` in the game's `Mods\` folder.
