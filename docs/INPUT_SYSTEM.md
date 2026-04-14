# Input System

## Architecture Overview

Hamsterball uses DirectInput8 (DInput8.dll) for all input:
1. **Keyboard** — 4-direction control (left/right/forward/backward)
2. **Mouse/Trackball** — Position-relative control (offset from screen center)
3. **Joystick/Gamepad** — Up to 4 controllers, analog X/Y axes

All input is converted to a 2D force vector by `Ball_GetInputForce` (0x46EC30).

## InputDevice Structure

| Offset | Type | Description |
|--------|------|-------------|
| +0x00 | char* | Display name ("Keyboard", "Mouse/Trackball", "Not Connected", or joystick name from +0xC) |
| +0x04 | App* | Back-pointer to App |
| +0x08 | int | Input mode (0=None, 1=Keyboard, 2=Mouse, 4-7=Joystick) |
| +0x0C | float | Speed scale / sensitivity (default 1.0) |
| +0x10 | void* | Joystick device pointer (modes 4-7 only) |

## Input Modes (InputDevice_SetType 0x46DFC0)

| Mode | Name | Description |
|------|------|-------------|
| 0 | None | Not Connected |
| 1 | Keyboard | Uses KeyboardDevice at App+0x434 |
| 2 | Mouse/Trackball | Uses cursor position relative to center |
| 4 | Joystick 1 | Device at App+0x424 |
| 5 | Joystick 2 | Device at App+0x428 |
| 6 | Joystick 3 | Device at App+0x42C |
| 7 | Joystick 4 | Device at App+0x430 |

If a joystick device pointer is NULL, mode falls back to 0 (Not Connected).

## Ball_GetInputForce (0x46EC30)

### Keyboard Mode (Case 1)
```
Ball_GetInputForce(this, output):
  kb = App->KeyboardDevice  // App+0x434
  
  // Read key states (DirectInput DIK codes, & 0x80 for key-down)
  left  = kb[0x50C] & 0x80
  right = kb[0x510] & 0x80
  fwd   = kb[0x514] & 0x80  // Fast forward (boost)
  back  = kb[0x518] & 0x80  // Slow/reverse
  
  // Compute X force (left/right)
  if left:  force_x = -1.0
  if right: force_x = +1.0
  
  // Compute Y force (forward/backward)
  // Default forward = 0.0 (DAT_004CF368 = no auto-forward on keyboard)
  // Fast forward adds boost speed
  // Backward adds slow speed
  
  // Apply sensitivity
  output[0] = speed_scale * force_x
  output[1] = speed_scale * force_y
```

**Key offsets in KeyboardDevice**:
| Offset | DIK Key | Default Action |
|--------|---------|---------------|
| +0x50C | key_left | Steer left |
| +0x510 | key_right | Steer right |
| +0x514 | key_forward | Fast forward (boost speed = DAT_004D0250) |
| +0x518 | key_backward | Slow/reverse (speed = DAT_004CF310) |

### Mouse Mode (Case 2)
```
Ball_GetInputForce(this, output):
  GetCursorPos(&cursor_pos)
  center_x = App->window_width / 2    // App+0x15C / 2
  center_y = App->window_height / 2   // App+0x160 / 2
  
  force_x = cursor_pos.x - center_x
  force_y = cursor_pos.y - center_y
  
  // Re-center mouse if flag set
  if App->recenter_mouse (App+0x15A):
    SetCursorPos(center_x, center_y)
  
  output[0] = speed_scale * force_x
  output[1] = speed_scale * force_y
```

Mouse controls ball direction by cursor offset from screen center.
When "recenter mouse" is enabled, cursor snaps back to center each frame.

### Joystick Mode (Cases 4-7)
```
Ball_GetInputForce(this, output):
  joystick = this->joystick_device  // this+0x10
  
  // Read analog axes
  raw_x = joystick[0x10C] / 100.0  // Divided by 100, cast to float
  raw_y = joystick[0x110] / 100.0
  
  // Normalize and scale
  Vec3_NormalizeAndScale(&point, 1.0)
  force_x = point.x
  force_y = point.y
  
  output[0] = speed_scale * force_x
  output[1] = speed_scale * force_y
```

## Multi-Controller Support

The game supports up to 4 simultaneous controllers for multiplayer:
- `2PController1` at App+0x424 (mode 4)
- `2PController2` at App+0x428 (mode 5)
- `2PController3` at App+0x42C (mode 6)
- `2PController4` at App+0x430 (mode 7)

Controller mapping strings stored in registry under `CONTROL1` through `CONTROL4`.

## Key Remapping

"REMAP KEYBOARD CONTROLS" dialog (at `Remap Keyboard` string 0x4D622C):
- Players can remap the 4 keyboard direction keys
- Key bindings stored in registry per controller slot
- CONTROL%d format string for registry keys

## DirectInput8 Initialization

- DLL: `DINPUT8.dll` (linked at 0x4F6140)
- IDirectInput8 COM interface created at startup
- Keyboard acquired via DirectInput device enumeration
- Joysticks enumerated and connected at App+0x424-0x430

## Input In Ball Update Loop

Ball input is processed in the 18-phase update:
- **Phase 0**: Read Ball_GetInputForce → apply as acceleration
- Input force is applied to ball velocity through physics system
- Speed scale affects how strongly input moves the ball
- Maximum speed clamped by `Ball+0x188` (max_speed = 5000.0f)

## N:NOCONTROL Collision Event

Special collision event `N:NOCONTROL` at string address 0x4CF840:
- Disables player control temporarily
- Likely used for trapdoor/mousetrap animations where ball is out of control
- Ball re-enables control after event duration timer expires

## Key Address Map

| Address | Function | Description |
|---------|----------|-------------|
| 0x46DFC0 | InputDevice_SetType | Configure input mode (kb/mouse/joystick) |
| 0x46EC30 | Ball_GetInputForce | Convert input to 2D force vector |

## Reimplementation Notes (SDL2)

### Input System Replacement
```cpp
enum class InputMode { None=0, Keyboard=1, Mouse=2, 
                       Joystick1=4, Joystick2=5, Joystick3=6, Joystick4=7 };

class InputDevice {
    InputMode mode = InputMode::None;
    float speed_scale = 1.0f;
    SDL_GameController* gamepad = nullptr;
    
    void SetType(InputMode m);
    void GetInputForce(float& out_x, float& out_y);
};

class KeyboardDevice {
    SDL_Scancode key_left = SDL_SCANCODE_LEFT;
    SDL_Scancode key_right = SDL_SCANCODE_RIGHT;
    SDL_Scancode key_forward = SDL_SCANCODE_UP;
    SDL_Scancode key_backward = SDL_SCANCODE_DOWN;
};

void InputDevice::GetInputForce(float& out_x, float& out_y) {
    float fx = 0, fy = 0;
    
    switch (mode) {
    case InputMode::Keyboard: {
        auto* kb = app->keyboard;
        const Uint8* keys = SDL_GetKeyboardState(nullptr);
        if (keys[kb->key_left])  fx = -1.0f;
        if (keys[kb->key_right]) fx = +1.0f;
        // Forward/backward use boost/slow speeds
        if (keys[kb->key_forward])  fy = forward_speed;
        if (keys[kb->key_backward]) fy = backward_speed;
        break;
    }
    case InputMode::Mouse: {
        int mx, my;
        SDL_GetMouseState(&mx, &my);
        fx = mx - app->window_width/2;
        fy = my - app->window_height/2;
        if (app->recenter_mouse)
            SDL_WarpMouseInWindow(app->window, app->window_width/2, app->window_height/2);
        break;
    }
    case InputMode::Joystick1:
    case InputMode::Joystick2:
    case InputMode::Joystick3:
    case InputMode::Joystick4: {
        if (gamepad) {
            fx = SDL_GameControllerGetAxis(gamepad, SDL_CONTROLLER_AXIS_LEFTX) / 32767.0f;
            fy = SDL_GameControllerGetAxis(gamepad, SDL_CONTROLLER_AXIS_LEFTY) / 32767.0f;
            float mag = sqrtf(fx*fx + fy*fy);
            if (mag > 1.0f) { fx /= mag; fy /= mag; }
        }
        break;
    }
    default: break;
    }
    
    out_x = speed_scale * fx;
    out_y = speed_scale * fy;
}
```

### Key Remapping
- Store key bindings in INI/JSON config file (replace Windows Registry)
- SDL2 supports scancode remapping natively via SDL_Scancode
- Controller mapping: SDL_GameControllerMapping() handles most gamepads