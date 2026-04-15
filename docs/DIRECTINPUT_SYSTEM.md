# DirectInput & Input Handling System

## Overview
Hamsterball uses DirectInput8 (DirectInput8Create at 0x47C7F0) for input from
keyboard, mouse, and joystick devices. The input system supports up to 4 players
with different device types: 1=keyboard, 2=mouse, 4=joystick1, 5=joystick2.

## InputDevice Structure (0x91C bytes)

Created by App_CreateInputDevice (0x46C050), actually calls SoundDevice_ctor
(this appears to be an error in naming — it's a generic Device base class).

Key offsets (based on App+0x1E4, 0x1E8 player device pointers):
| Offset | Type | Description |
|--------|------|-------------|
| +0x04 | int | Device type (1=keyboard, 2=mouse, 4=joystick) |
| +0x10 | void* | DirectInput device handle |
| +0x18C | DIDEVICEINSTANCE | Device instance data |
| +0x10C | int | Joystick X axis value |
| +0x110 | int | Joystick Y axis value |
| +0x114 | int | Joystick Z axis value |
| +0x434 | void* | Keyboard key state array |

## Input Device Initialization

### App_Initialize_Full (0x429530) — Input Setup Steps 15-22
```
Step 15: InputDevice_SetType(App+0x1E4, 1)     // Player 1 = keyboard
Step 16: InputDevice_SetType(App+0x1E8, 1)     // Player 2 = keyboard
Step 17: InputDevice_SetType(App+0x1E4, 2)     // Player 1 = mouse
Step 18: InputDevice_SetType(App+0x1E8, 2)     // Player 2 = mouse
Step 19: InputDevice_SetType(App+0x1E4, 4)     // Player 1 = joystick 1
Step 20: InputDevice_SetType(App+0x1E8, 4)     // Player 2 = joystick 1
Step 21: InputDevice_SetType(App+0x1E4, 5)     // Player 1 = joystick 2
Step 22: InputDevice_SetType(App+0x1E8, 5)     // Player 2 = joystick 2
```

The game tries all device types during initialization. The final type
is set based on which devices are detected/connected.

### DirectInput8Create (0x47C7F0)
Standard DirectInput COM entry point:
```c
result = DirectInput8Create(hinst, 0x800, IID_IDirectInput8W, &di, NULL);
```

## InputDevice Functions

| Address | Function | Description |
|---------|----------|-------------|
| 0x46C050 | App_CreateInputDevice | Allocate 0x91C device struct |
| 0x46C110 | App_CreateInputHandler | Create input handler |
| 0x6DFC0 | InputDevice_SetType | Set device type (1=kb, 2=mouse, 4=joy1, 5=joy2) |
| 0x46E0B0 | Input_IsKeyDown | Check if DIK key is pressed |
| 0x46EBD0 | InputDevice_PollAndRelease | Poll device, release buffers |
| 0x46EC30 | Ball_GetInputForce | Get 2D force vector from input |
| 0x46EE10 | InputHandler_Ctor | Input handler constructor |

## Ball Input Force (Ball_GetInputForce 0x46EC30)

Maps raw device input to ball movement force:

### Mode 1 — Keyboard
```
Key state array at: *(int*)(ball + 4) + 0x434
DIK codes at: +0x50C (left), +0x510 (right), +0x514 (up), +0x518 (down)
Check: key_state[DIK_code + 0xC] & 0x80 (high bit = pressed)

UP:   force_y = -0.5  // forward
DOWN: force_y = 1.0   // backward  
LEFT: force_x = -1.0
RIGHT: force_x = 1.0
```

### Mode 2 — Mouse
```
center_x = gfx->width / 2
center_y = gfx->height / 2
GetCursorPos(&cursor)
force_x = cursor.x - center_x
force_y = cursor.y - center_y
If mouse capture active: SetCursorPos(center_x, center_y) // recenter
```

### Mode 3-6 — Joystick (4 devices)
```
pos.x = joystick->axis_x / 100
pos.y = joystick->axis_y / 100
NormalizeAndScale to unit circle
force_x = normalized.x
force_y = normalized.y
```

Output is multiplied by per-ball speed scale (ball+0xC).

## Input Checking

### Input_CheckKeyCombo (0x428F10)
14 xrefs — checks for keyboard shortcut combos:
- Checks multiple key codes simultaneously
- Returns TRUE if all keys in combo are pressed

### Input_CheckJoystickButtons (0x428FB0)
1 xref — checks joystick button states.

## Input Dispatch (Scene_HandleInput 0x4692F0)

Main input routing (51 xrefs):
```
1. Clear active_input_object at Scene+0x864
2. Iterate gadgets in Scene+0x858 (AthenaList)
3. For each gadget where input_active (+0x16 != 0):
   - Call gadget->vtable[1](msg1, msg2) // input handler
   - If gadget captures (+0x14 != 0): active = gadget
   - If player1 (+0x19 != 0): channel = App+0x1E4
   - If player2 (+0x15 != 0): channel = App+0x1E8
4. Play sound via App+0x1DC
```

## Mouse Input

### Input_OnMouseDown (0x46C760)
2 xrefs — mouse button down event.

### Input_OnMouseUp (0x46C430)
2 xrefs — mouse button up event.

### Input_OnMouseUpCapture (0x46C3C0)
2 xrefs — mouse up with cursor capture/release.

## Key Codes (DIK Constants)

Keyboard uses DirectInput key constants (DIK_*):
| DIK Code | Key |
|---------|-----|
| DIK_UP | Arrow Up |
| DIK_DOWN | Arrow Down |
| DIK_LEFT | Arrow Left |
| DIK_RIGHT | Arrow Right |
| DIK_SPACE | Spacebar |
| DIK_ESCAPE | Escape |

These are stored in the InputDevice structure and looked up by
Ball_GetInputForce to check key states.

## Input in Loading Screen

### LoadingScreen_HandleInput (0x42D020)
1 xref — handles input during the loading screen (resource loading phase).
Allows user to skip or cancel during initial load.

## Registration Dialog

### RegisterDialog_HandleInput (0x4475A0)
4 xrefs — handles input in the registration dialog.
Used for entering registration keys.

## Key Input Constants

DirectInput keyboard scan codes used:
- 0x50C (DIK_LEFT) — stored at InputDevice+0x50C
- 0x510 (DIK_RIGHT) — stored at InputDevice+0x510
- 0x514 (DIK_UP) — stored at InputDevice+0x514
- 0x518 (DIK_DOWN) — stored at InputDevice+0x518

The key state array format:
```
key_state = *(int*)(device + 0x434)
key_pressed = key_state[DIK_code + 0xC] & 0x80
```

## Input Device Detection

The game attempts to detect and configure input devices during startup.
App_CreateInputHandler (0x46C110) manages device enumeration.

Multiple input types can be configured simultaneously:
- Player 1 can use keyboard, mouse, or joystick
- Player 2 can use keyboard, mouse, or joystick  
- Each player's device is stored at App+0x1E4 and App+0x1E8

## Input Sound Feedback

Input events trigger sound effects via App+0x1DC:
- Menu navigation sounds
- Button click sounds
- Race start countdown sounds