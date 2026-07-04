# FreeCam

Freecam mod for Hamsterball Plus. Press **F7** to toggle between the game's normal camera and a free-flying camera you control.

## Controls (freecam active)

| Key | Action |
|-----|--------|
| F7 | Toggle freecam on/off |
| W/S | Move forward/backward |
| A/D | Strafe left/right |
| E/Q | Move up/down |
| Arrow Keys | Look (yaw/pitch) |
| Left Shift | Hold for 3× movement speed |

## How it works

- On activation, the camera spawns at the player ball's position (offset back/up) and looks at the ball.
- `onRenderApply` replaces the view matrix each frame using `BuildCustomViewMatrix` with the computed eye position and forward vector.
- On deactivation (F7 again, level change, or scene end), the game's normal camera resumes automatically.
- A green "FREECAM" indicator is drawn in the top-left corner while active.

## Build

Visual Studio (32-bit / x86 DLL). Place `FreeCam.dll` in the game's `Mods\` folder.
