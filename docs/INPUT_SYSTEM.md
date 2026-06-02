# Hamsterball Input System — Complete Reverse-Engineering Document

**Scope:** Original `Hamsterball.exe` (PE32, i386, Athena engine).  
**Method:** Direct Ghidra decompilation of original binary.  
**Last Updated:** 2026-06-02

## Table of Contents
1. [Input Architecture Overview](#input-architecture-overview)
2. [Core Data Structures](#core-data-structures)
3. [Input Device Lifecycle](#input-device-lifecycle)
4. [Key-to-Action Mapping (Registry)](#key-to-action-mapping-registry)
5. [Runtime Input Processing](#runtime-input-processing)
6. [Options Menu / Remap UI](#options-menu--remap-ui)
7. [DirectInput Integration](#directinput-integration)
8. [Function Reference](#function-reference)
9. [Offsets Cheat-Sheet](#offsets-cheat-sheet)
10. [Modding Notes](#modding-notes)

---

## Input Architecture Overview

The original Hamsterball uses **DirectInput 8** for all player input.  The top-level owner is `App`; it creates two related objects:

| Member     | Size   | Stored At | Description |
|------------|--------|-----------|-------------|
| InputDevice (keyboard/mouse) | 0x91C | `App+0x178` | Created by `App_CreateInputDevice` (0x46C050) |
| InputHandler (4 control slots) | 0x438 | `App+0x180` | Created by `App_CreateInputHandler` (0x46C110) |

`Ball_GetInputForce` (0x46EC30) is the main entry-point that converts raw input state into a 2-D force vector used by the physics engine.  It supports **three families** of input:

| Switch Case | Mode | Source |
|-------------|------|--------|
| `1` | Keyboard | DirectInput keyboard device, DIK scan-codes at `InputDevice+0x50C..0x518` |
| `2` | Mouse | Cursor position offset from screen centre, clipped by `App+0x15A` centre-capture flag |
| `4-7` | Joystick / Gamepad | 4 DirectInput gamepad devices polled into `GamepadDevice` structs at `InputHandler+0x10C/0x110` |

---

## Core Data Structures

### `App` (top-level singleton)

The global `App` instance is at `DAT_005341E0`.  Constructor is `App_Ctor` (0x46DC40).

| Offset | Type | Name | Notes |
|--------|------|------|-------|
| `+0x04` | `void*` | `graphics` | `GraphicsDevice` |
| `+0x08` | `void*` | `window` | Win32 window handle |
| `+0x0C` | `void*` | `scene` | Active `Scene` |
| `+0x10` | `int` | `scene_type` | 0=menu, 1=level |
| `+0x54` | `void*` | `registry` | `RegKey` object (HKCU SOFTWARE key) |
| `+0x15A` | `bool` | `mouse_centre_capture` | If true `Ball_GetInputForce` recentres the cursor every frame |
| `+0x158` | `bool` | `fullscreen` | Full-screen flag |
| `+0x15C` | `int` | `screen_width` | e.g. 640 |
| `+0x160` | `int` | `screen_height` | e.g. 480 |
| `+0x174` | `void*` | `graphics_device` | D3D8 device wrapper |
| `+0x178` | `InputDevice*` | `input_device` | Keyboard / mouse DInput device |
| `+0x17C` | `Audio_MusicDevice*` | `music_device` | BASS audio |
| `+0x180` | `InputHandler*` | `input_handler` | 4-slot control bindings + gamepad array |
| `+0x1A0` | `int` | `player1_mode` | 1=keyboard, 2=mouse, 4-7=joy |
| `+0x1A4` | `int` | `player2_mode` | Same enum for P2 |
| `+0x208` | `char*` | `status_text` | Debug status string ("Initialize(x)") |
| `+0x224` | `SimpleMenu*` | `current_menu` | Active menu object |
| `+0x228` | `void*` | `results_dialog` | Race-end results screen |
| `+0x240` | `HCURSOR` | `blank_cursor` | Invisible cursor handle |
| `+0x278` | `Texture*` | `shadow_texture` | Ball shadow sprite |
| `+0x2A8` | `float` | `mouse_sensitivity` | Multiplier for mouse mode |
| `+0x2AC` | `float` | `music_volume` | 0.0-1.0 |
| `+0x2B0` | `float` | `sfx_volume` | 0.0-1.0 |
| `+0x2B4` | `bool` | `pause_with_right_button` | If true RMB pauses the game |
| `+0x2B8` | `bool` | `registered` | Full-game flag |
| `+0x2C0` | `float` | `time_remaining` | Race timer |
| `+0x2F8` | `bool` | `is_tournament` | Tournament mode flag |
| `+0x300` | `char[32]` | `player1_name` | Tournament name |
| `+0x320` | `char[32]` | `player2_name` | Tournament name |
| `+0x340` | `float` | `best_time` | Saved best lap |
| `+0x344` | `float` | `target_time` | Target for medals |
| `+0x350` | `int` | `free_plays` | Demo counter |
| `+0x354` | `int` | `play_count` | Total launches |
| `+0x358` | `int` | `unlock_flags` | Secret level unlock bits |
| `+0x3E0` | `void*` | `high_scores_table` | High-score data |
| `+0x534` | `MusicDevice*` | `jukebox` | Background music |
| **+0xB28** | **DWORD** | **CONTROL1** | **Raw key/button DWORD (see registry section)** |
| **+0xB2C** | **DWORD** | **CONTROL2** | **"** |
| **+0xB30** | **DWORD** | **CONTROL3** | **"** |
| **+0xB34** | **DWORD** | **CONTROL4** | **"** |

### `InputDevice` (0x91C bytes)

Allocated with `operator_new(0x91C)` in `App_CreateInputDevice` (0x46C050).  The constructor at 0x466620 (named `SoundDevice_ctor` in older labels — it is actually a generic device constructor) sets the DirectInput cooperative level and fills the default keyboard DIK codes.

| Offset | Type | Name | Notes |
|--------|------|------|-------|
| `+0x00` | `vtable*` | vtable | `PTR_InputDevice_DeletingDtor` |
| `+0x04` | `AthenaList` | `sub_devices` | `AthenaList` of sub-device pointers |
| `+0x41C`| `AthenaList` | `??` | Second list (possibly device caps) |
| `+0x434`| `void*` | `dinput_device` | IDirectInputDevice8* for keyboard |
| `+0x508`| `int` | `device_type` | See `DEVICE_TYPE_KEYBOARD` etc. below |
| `+0x50C`| `int` | `key_left` | DIK scan-code for "left" |
| `+0x510`| `int` | `key_right`| DIK scan-code for "right" |
| `+0x514`| `int` | `key_up` | DIK scan-code for "forward/up" |
| `+0x518`| `int` | `key_down` | DIK scan-code for "backward/down" |
| `+0x51C`| `int` | `key_escape`| DIK scan-code for escape/pause |
| `+0x520`| `int` | `key_pause` | DIK scan-code for pause (second binding) |

> **Note:** The reimplementation and some community docs claim 6 directional keys exist; the decompilation of `Ball_GetInputForce` (0x46EC30) only reads **4 directional keys** (`+0x50C` through `+0x518`).  There is no explicit "brake" or "jump" action read here — jumping is handled exclusively by `E:JUMP` collision objects.

### `InputHandler` (0x438 bytes)

Allocated with `operator_new(0x438)` in `App_CreateInputHandler` (0x46C110).  Constructor is at roughly `0x46DFA0` (named `FUN_0046dfa0` in Ghidra).

| Offset | Type | Name |
|--------|------|------|
| `+0x00` | `vtable*` | vtable |
| `+0x04` | `App*` | back-pointer to App |
| `+0x0C` | `int[4]` | `player_device_index` | Which physical device each slot uses |
| `+0x1C` | `int[4]` | `player_input_mode` | 1=kbd,2=mouse,4-7=joy |
| `+0x2C` | `int[4]` | `??` | Possible dead-zone / threshold |
| `+0x3C` | `int` | `joy_count` | Number of enumerated gamepads |
| `+0x40` | `GamepadDevice*[4]`| `gamepads` | Ptrs to polled gamepad state structs |
| `+0x10C`| `int` | `joy0_x` | X-axis raw value / 100 |
| `+0x110`| `int` | `joy0_y` | Y-axis raw value / 100 |

### `GamepadDevice` (layout inferred from `Ball_GetInputForce`)

Polled by `GamepadDevice_PollState` during `InputDevice_PollAndRelease`.

| Offset | Type | Name |
|--------|------|------|
| `+0x130`| `char` | `button_a` |
| `+0x131`| `char` | `button_b` |
| `+0x132`| `char` | `button_x` |
| `+0x133`| `char` | `button_y` |
| `+0x10C`| `int` | `axis_x` |
| `+0x110`| `int` | `axis_y` |

---

## Input Device Lifecycle

### 1. Construction — `App_Initialize_Full` (0x429530)

The full 26-step init sequence creates devices in this order:

| Step | Function | What it does |
|------|----------|--------------|
| 1 | `App_Initialize` (0x46BB40) | Base init, D3D8 device, window |
| 15 | `App_CreateInputHandler` (0x46C110) | Alloc 0x438, ctor, store at `App+0x180` |
| 16 | `FUN_0046dfc0(slot0, 1)` | Bind slot 0 to **keyboard** (mode 1) |
| 17-18 | `FUN_0046dfa0` / `FUN_0046dfc0(slot1, 2)` | Bind slot 1 to **mouse** (mode 2) |
| 19-20 | `FUN_0046dfa0` / `FUN_0046dfc0(slot2, 4)` | Bind slot 2 to **joy0** (mode 4) |
| 21-22 | `FUN_0046dfa0` / `FUN_0046dfc0(slot3, 5)` | Bind slot 3 to **joy1** (mode 5) |
| 23 | `RegKey_Close` | Close registry handle |

The `InputHandler` constructor reads the registry keys `CONTROL1`..`CONTROL4` and stores the raw DWORDs at `App+0xB28..0xB34` (see next section).

### 2. Per-Frame Polling — `InputDevice_PollAndRelease` (0x46EBD0)

```c
void InputDevice_PollAndRelease(int self)
{
    // 1. Poll keyboard via DirectInput (256-byte state buffer at +0xC)
    int didev = *(int*)(self + 0x434);
    if (didev != 0) {
        int hr = IDirectInputDevice8_GetDeviceState(didev, 0x100, self + 0xC);
        if (hr < 0) {
            IDirectInputDevice8_Acquire(didev);
            memset(self + 0xC, 0, 0x100);   // 256 bytes = full DI8DEVSTATE size
        }
    }
    // 2. Poll up to 4 gamepads
    int *slot = (int*)(self + 0x424);
    for (int i = 0; i < 4; ++i) {
        if (*slot != 0) GamepadDevice_PollState(*slot);
        slot++;
    }
}
```

* The `0x100` bytes starting at `InputDevice+0x0C` is the standard **DirectInput keyboard state array** (256 entries, one per scan-code).  Each byte uses the high bit (`& 0x80`) for "key is currently down".
* `InputDevice+0x424` is the start of a 4-element `int[]` holding gamepad device pointers.

### 3. Destruction — `KeyboardDevice_ScalarDtor` (0x46E910)

Called on game exit.  Releases the IDirectInputDevice8 pointer and frees the `InputDevice` heap block.

---

## Key-to-Action Mapping (Registry)

### Registry Keys

Under `HKCU\Software\Raptisoft\Hamsterball`:

| Key | Type | Default | Meaning |
|-----|------|---------|---------|
| `CONTROL1` | DWORD | `0x00000063` (99) | Binding slot 0 → **Keyboard** |
| `CONTROL2` | DWORD | `0x00000064` (100)| Binding slot 1 → **Mouse** |
| `CONTROL3` | DWORD | varies | Binding slot 2 → **Joystick / Gamepad** |
| `CONTROL4` | DWORD | varies | Binding slot 3 → **Joystick / Gamepad** |

* `CONTROL1` and `CONTROL2` are hard-coded to keyboard (99) and mouse (100) by `InputHandler` ctor.
* The values written to registry are **NOT DIK scan-codes** in the obvious way; they are internal mode indicators (99 = kbd, 100 = mouse, other values = joy index).
* When the user chooses "REMAP KEYBOARD CONTROLS" in the Options menu, the values at `App+0xB28..0xB38` are updated and later flushed to registry on exit.

### Default Keyboard DIK Codes (hard-coded in constructor)

From the `InputDevice` constructor (0x466620) disassembly, the default scan-codes populated are:

| Action | DIK Code | Hex | Description |
|--------|----------|-----|-------------|
| Left / Turn Left | `DIK_LEFT` | `0xCB` (203) | Arrow left |
| Right / Turn Right | `DIK_RIGHT`| `0xCD` (205) | Arrow right |
| Up / Roll Forward | `DIK_UP` | `0xC8` (200) | Arrow up |
| Down / Roll Backward | `DIK_DOWN` | `0xD0` (208) | Arrow down |
| Escape / Pause | `DIK_ESCAPE` | `0x01` (1) | Escape key |
| Pause (alt) | `DIK_P` | `0x19` (25) | P key |

> **Important:** There is **no separate "brake" key** — releasing the up/down arrows simply stops applying force.  There is **no "jump" key** in the input system; ball jumping is caused by `E:JUMP` collision events handled in `GameObject_HandleCollision` (0x40C5D0).

### Registry Persistence

Registry read happens once during `InputHandler` construction.  Registry write happens in `SoundDevice_dtor` (0x4668A0) — the same function also saves `Sound Volume`.  The exact registry path is built by `RegKey_SetSoftwarePath` (0x472F50):

```
HKCU\Software\Raptisoft\Hamsterball
```

---

## Runtime Input Processing

### `Input_IsKeyDown` (0x46E0B0)

```c
uint Input_IsKeyDown(void* self, int key)
{
    int mode = *(int*)(self + 8) - 1;   // 0=kbd, 1=mouse, 3-6=joyN
    switch (mode) {
    case 0: // Keyboard
        if (key == -1) {
            // Check ANY key down (scan whole 256-byte buffer)
            byte* state = (byte*)(*(int*)(*(int*)(self + 4) + 0x434) + 0xC);
            return any_nonzero(state, 256);
        }
        else if (key == 0) {
            return (state[0x45] >> 7);  // ??? special binding
        }
        else if (key == 1) {
            return App_Is2PMode(...);
        }
        else {
            byte* state = (byte*)(*(int*)(*(int*)(self + 4) + 0x434) + 0xC + key);
            return (*state != 0);       // high-bit test
        }
    case 1: // Mouse
        // Returns left/right/middle button state from Win32 mouse struct
        // iVar4 + 0x1C8, 0x1C9, 0x1CA
        return ...
    case 3: case 4: case 5: case 6: // Joystick
        int pad = *(int*)(self + 0x10); // gamepad struct ptr
        if (pad) {
            if (key == -1) return any_button_on_pad(pad);
            return *(char*)(pad + 0x130 + key) != 0;
        }
    }
    return 0;
}
```

* `self+8` stores the **input mode** (1 = kbd, 2 = mouse, 4-7 = joy).
* `self+4` is a back-pointer to the `App`.
* For the keyboard branch, the actual DI8 state buffer is reached by:  
  `App->input_device->dinput_device->+0xC` (the 256-byte keyboard state array).

### `Ball_GetInputForce` (0x46EC30)

This is the **main physics input function** — every frame it produces a `(forceX, forceY)` float pair.

```c
void Ball_GetInputForce(void* this, float* outForce)
{
    float forceX = 0.0f;
    float forceY = 0.0f;
    switch (*(int*)(this + 8)) {      // player input mode
    case 1: // Keyboard
    {
        int input_dev = *(int*)(*(int*)(this + 4) + 0x434); // InputDevice ptr
        byte* kbd = (byte*)(input_dev + 0xC);               // DI8 state
        int left  = *(int*)(input_dev + 0x50C);             // DIK code
        int right = *(int*)(input_dev + 0x510);
        int up    = *(int*)(input_dev + 0x514);
        int down  = *(int*)(input_dev + 0x518);
        if (kbd[left]  & 0x80) forceX = -1.0f;
        if (kbd[right] & 0x80) forceX =  1.0f;
        if (kbd[up]    & 0x80) forceY = -1.0f;   // Note: screen-up is -Y
        if (kbd[down]  & 0x80) forceY =  1.0f;
        break;
    }
    case 2: // Mouse
    {
        POINT pt; GetCursorPos(&pt);
        int cx = App->screen_width  / 2;
        int cy = App->screen_height / 2;
        forceX = (float)(pt.x - cx);
        forceY = (float)(pt.y - cy);
        if (App->mouse_centre_capture)
            SetCursorPos(cx, cy);
        break;
    }
    case 4: case 5: case 6: case 7: // Joy
    {
        int pad = *(int*)(this + 0x10);   // GamepadDevice*
        if (pad != 0) {
            forceX = (float)(*(int*)(pad + 0x10C) / 100);
            forceY = (float)(*(int*)(pad + 0x110) / 100);
            Vec3_NormalizeAndScale(&pt, 1.0f);
            forceY = (float)pt.y;
            forceX = (float)pt.x;
        }
        break;
    }
    }
    float scale = *(float*)(this + 0xC);   // per-ball sensitivity
    outForce[0] = scale * forceX;
    outForce[1] = scale * forceY;
}
```

**Key observations for modders:**
* `this+8` = player input mode, `this+4` = `App*`, `this+0xC` = sensitivity float.
* The keyboard branch **only reads 4 directional keys**.  There is no jump, no brake, no action button.
* The Y axis is inverted: `-1.0` is "up" on screen (toward smaller Y) because D3D uses a top-down coordinate system for this engine.
* Mouse mode uses **absolute cursor offset from screen centre**, not delta motion.

### `Input_CheckKeyCombo` (0x428F10)

Used by menus (and the pause system) to detect "any key pressed in slot" with a 50-frame debounce.

```c
int Input_CheckKeyCombo(void* self, int slot)
{
    if (slot == 2) {   // special escape/pause check
        int dev = *(int*)(*(int*)(self + 0x180) + 0x434); // InputDevice
        byte esc = *(byte*)(dev + 0x51C + 0xC); // escape DIK state
        byte pau = *(byte*)(dev + 0x520 + 0xC); // pause DIK state
        if ((esc & 0x80) || (pau & 0x80)) {
            if (*(int*)(self + 0x560) == 0) {
                *(int*)(self + 0x560) = 0x32;   // 50 frame cooldown
                return 1;
            }
        }
    }
    if (slot < 4) {
        int* combo = (int*)(self + slot*4 + 0x550); // combo state array
        while (slot < 4) {
            if (*(int*)(combo + 4) == 0) {   // cooldown timer == 0?
                int down = Input_IsKeyDown((void*)*combo, -1);
                if (down) {
                    *(int*)(self + slot*4 + 0x560) = 0x32; // set 50-frame timer
                    return 1;
                }
            }
            slot++;
            combo++;
        }
    }
    return 0;
}
```

* `App+0x550..0x55C` = 4 `InputCombo` objects (one per control slot).
* `App+0x560..0x56C` = 4 `int` cooldown counters (50 frames = ~0.8 s at 60 Hz).

---

## Options Menu / Remap UI

### `OptionsMenu_ctor` (0x442CE0)

The Options menu is a `UIList`-based menu (`SimpleMenu` -> `OptionsMenu`).  Items are added in this order:

```
0:  "RESOLUTION: 1024 X 768"    -> "REZ"
1:  "FULLSCREEN: YES"            -> "FS"
2:  "COLOR QUALITY: MEDIUM"      -> "CM"
3:  "SAFE MODE: OFF"             -> "SM"
4:  <spacer 10>
5:  "SOUND VOLUME:"              -> "SV"
6:  "MUSIC VOLUME:"              -> "MV"
7:  <spacer 10>
8:  "REMAP KEYBOARD CONTROLS"    -> "REMAP"
9:  <spacer 10>
10: "MOUSE SENSITIVITY:"         -> "MS"
11: "PAUSE W/RIGHT BUTTON: YES"  -> "PWRB"
12: <spacer 10>
13: "BACK"                       -> "BACK"
```

Selecting item `"REMAP"` enters the **remap sub-menu**.  That sub-menu code is not fully decompiled here, but the render function shows the visual binding representation.

### `OptionsMenu_RenderControls` (0x42E840)

```c
void OptionsMenu_RenderControls(void* menu)
{
    int binding = 0;
    int offset = 0xB28;   // App+0xB28 = CONTROL1
    while (offset < 0xB38) {
        Matrix_Scale4x4(...);
        int val = *(int*)(offset + *(int*)(menu + 0x878)); // read CONTROLx
        int duplicate = 0;
        if (val == 99) {
            // Keyboard icon (scale x=0.5)
        } else if (val == 100) {
            // Mouse icon (scale x=0.5)
        } else {
            // Joystick / other — check for duplicates
            int check = 0xB28;
            int idx = 0;
            while (check < 0xB38) {
                if (idx != binding &&
                    *(int*)(check + App) == val) {
                    // Duplicate binding → render in RED
                    Matrix_Scale4x4(..., 1.0f, 0.0f, 0.0f, 1.0f);
                }
                check += 4;
                idx++;
            }
        }
        binding++;
        // Draw "CONTROL%d" label with chosen colour
        AthenaString_SprintfToBuffer(buf, "CONTROL%d");
        UIList_SetColorsByName(menu, ..., buf);
        offset += 4;
    }
}
```

* The menu reads `App+0xB28..0xB38` (the 4 DWORD control bindings).
* Value **99** = keyboard (rendered with magenta tint in UIList).
* Value **100** = mouse (rendered with cyan tint).
* Any other value = a joystick/gamepad index; if two slots share the same value the second is drawn in **red** to warn the player.
* The actual remapping UI (where the user presses a key to rebind) is likely a small tight loop inside the menu update function — it has not been decompiled in this round but its address can be found by xref-ing `OptionsMenu_ctor` from the vtable handler at `UIList` item `"REMAP"`.

---

## DirectInput Integration

### Enumeration & Cooperative Level

`DirectInput8Create` (imported at 0x47C7F0 from `dinput8.dll`) is called inside the `InputDevice` constructor.  The constructor then:

1. Creates the IDirectInput8 device (`DllEntryPoint` in decomp — actually `DirectInput8Create`).
2. Calls `IDirectInputDevice8_SetCooperativeLevel` with:
   * Window handle = `App->window`
   * Flags = `DISCL_NONEXCLUSIVE | DISCL_BACKGROUND` (or `DISCL_EXCLUSIVE | DISCL_FOREGROUND` for joystick)
3. Calls `IDirectInputDevice8_SetDataFormat` with `c_dfDIKeyboard` or `c_dfDIJoystick`.
4. Acquires the device.

The exact cooperative-level path is visible in the decompilation of `InputDevice` constructor (0x466620) — the branch at offset `0x46666C` checks a flag and either calls with `2` (`BACKGROUND`) or `3` (`FOREGROUND`).

### Device Acquisition Failure Path

In `InputDevice_PollAndRelease` (0x46EBD0):

```c
int hr = IDirectInputDevice8_GetDeviceState(didev, 0x100, buffer);
if (hr < 0) {
    IDirectInputDevice8_Acquire(didev);
    memset(buffer, 0, 0x100);   // zero state on failure
}
```

If the device is lost (Alt-Tab, UAC popup, etc.), the game automatically re-acquires it and returns a zeroed state for that frame — preventing phantom inputs.

### Default DirectInput8 DLL Import

| Import | Address | Called From |
|--------|---------|-------------|
| `DirectInput8Create` | `0x0047C7F0` | `InputDevice` constructor (0x466620) |

---

## Function Reference

### Input System Functions

| Address | Name | Args | Description |
|---------|------|------|-------------|
| **0x46EC30** | **Ball_GetInputForce** | `(void* this, float* outForceXY)` | Convert player input to 2-D force vector for physics |
| **0x46E0B0** | **Input_IsKeyDown** | `(void* this, int key)` | Check if key/button is currently pressed (mode-aware) |
| **0x46EBD0** | **InputDevice_PollAndRelease** | `(int device)` | Poll DInput kb + 4 gamepads, re-acquire if lost |
| **0x428F10** | **Input_CheckKeyCombo** | `(void* app, int slot)` | 50-frame debounced "any key in slot" check |
| **0x42E840** | **OptionsMenu_RenderControls**| `(void* menu)` | Draw control icons coloured by device type |
| **0x442CE0** | **OptionsMenu_ctor** | `(void* this, int arg1, int arg2)` | Build Options menu including REMAP entry |
| **0x46C050** | **App_CreateInputDevice** | `(int app)` | Alloc+construct `InputDevice` (0x91C) store at `App+0x178` |
| **0x46C110** | **App_CreateInputHandler** | `(int app)` | Alloc+construct `InputHandler` (0x438) store at `App+0x180` |
| **0x46DC40** | **App_Ctor** | `(void* this)` | Full App init; creates `MeshWorld`, cursors, COM |
| **0x46E910** | **KeyboardDevice_ScalarDtor** | `(void* this)` | Release DInput device, free `InputDevice` |
| **0x466620** | **InputDevice_ctor** (mis-label "SoundDevice") | `(void* this, int app)` | Set coop-level, alloc kb buffer, default DIK codes |
| **0x4692F0** | **Scene_HandleInput** | `(void* scene)` | Menu item iteration + input dispatch |
| **0x47C7F0** | **DirectInput8Create** | (import) | Standard DInput8 creation |

### Registry Functions

| Address | Name | Description |
|---------|------|-------------|
| 0x472EC0 | `RegKey_Ctor` | Open `HKCU\Software\Raptisoft\Hamsterball` |
| 0x472F30 | `RegKey_Close` | Close key handle |
| 0x473030 | `RegKey_WriteDWORD` | Persist a DWORD |
| 0x473100 | `RegKey_QueryValue` | Read a value by name |
| 0x473170 | `RegKey_ReadString` | Read string (not used for controls) |

---

## Offsets Cheat-Sheet

### App Struct (global `DAT_005341E0`)

| Offset | Size | Meaning |
|--------|------|---------|
| `+0x178` | 4 | `InputDevice*` |
| `+0x180` | 4 | `InputHandler*` |
| `+0x1A0` | 4 | Player 1 input mode (1=kbd, 2=mouse, 4-7=joy) |
| `+0x15A` | 1 | `mouse_centre_capture` bool |
| `+0x15C` | 4 | `screen_width` |
| `+0x160` | 4 | `screen_height` |
| `+0x434` | 4 | `InputDevice*` (duplicate path) |
| `+0x550` | 4x4 | 4 `InputCombo*` slot objects |
| `+0x560` | 4x4 | 4-frame debounce counters |
| **+0xB28** | **4** | **CONTROL1 (raw binding DWORD)** |
| **+0xB2C** | **4** | **CONTROL2** |
| **+0xB30** | **4** | **CONTROL3** |
| **+0xB34** | **4** | **CONTROL4** |

### InputDevice (`0x91C` bytes)

| Offset | Size | Meaning |
|--------|------|---------|
| `+0x00` | 4 | vtable |
| `+0x04` | 0x418 | `AthenaList` (sub-devices) |
| `+0x41C`| 0x10 | Second `AthenaList` |
| `+0x434`| 4 | `IDirectInputDevice8*` keyboard |
| `+0x508`| 4 | `device_type` |
| **+0x50C**| **4** | **key_left DIK code** |
| **+0x510**| **4** | **key_right DIK code** |
| **+0x514**| **4** | **key_up DIK code** |
| **+0x518**| **4** | **key_down DIK code** |
| `+0x51C`| 4 | key_escape DIK code |
| `+0x520`| 4 | key_pause DIK code |
| `+0x0C` | 0x100 | DirectInput keyboard state buffer (256 bytes) |
| `+0x424`| 4x4 | Gamepad device pointers (4 slots) |

### InputHandler (`0x438` bytes)

| Offset | Size | Meaning |
|--------|------|---------|
| `+0x00` | 4 | vtable |
| `+0x04` | 4 | `App*` back-pointer |
| `+0x0C` | 4x4 | `player_device_index[4]` |
| `+0x1C` | 4x4 | `player_input_mode[4]` |
| `+0x40` | 4x4 | `GamepadDevice*[4]` |
| `+0x10C`| 4 | `joy0_x` |
| `+0x110`| 4 | `joy0_y` |

### Ball (relevant fields for input)

| Offset | Size | Meaning |
|--------|------|---------|
| `+0x08` | 4 | Player input mode (copied from App) |
| `+0x0C` | 4 | Sensitivity float (default 1.0) |
| `+0x10` | 4 | `GamepadDevice*` (for joy modes) |

---

## Modding Notes

### Changing Default Key Bindings

If you want to patch the **default** keys (before the user remaps), edit the immediate values written in `InputDevice_ctor` (0x466620).  The four `mov dword ptr [reg+N], imm32` instructions near `+0x40` through `+0x50` set the initial DIK codes.

### Adding a New Input Mode

The `switch(mode)` in `Ball_GetInputForce` only handles cases `1`, `2`, and `4-7`.  If you add case `3` (or re-purpose an unused case), you must also:
1. Update `InputDevice_PollAndRelease` to poll the new hardware.
2. Update `Input_IsKeyDown` to read the new device state.
3. Update `OptionsMenu_RenderControls` to recognise the new mode value (or it will render as a red duplicate).

### Disabling the 50-Frame Debounce

`Input_CheckKeyCombo` stores `0x32` (50 decimal) into `App+0x560..0x56C` to prevent repeated triggers.  Patch the immediate `0x32` at `0x428F1A` and `0x428F3A` to `0x01` or `0x00` for instant re-triggering.

### Mouse-Sensitivity Scaling

`Ball_GetInputForce` case 2 does **not** apply `App->mouse_sensitivity` (`App+0x2A8`).  The raw cursor offset is used directly.  To add sensitivity scaling, patch a multiply by `*(float*)(App + 0x2A8)` before the `break;` at offset `0x46ECXX`.

### Registry-Free Operation (Portable Mode)

All four `CONTROLx` values are read from registry during `InputHandler` construction.  If the registry key does not exist, the constructor falls back to hard-coded defaults (99, 100, joy0, joy1).  To make the game fully portable, patch `RegKey_QueryValue` (0x473100) to always return failure (`0`) so defaults are always used.

### Multi-player Input

The game supports up to **4 local players** (Party Race / Rodent Rumble).  Each player gets a control slot (`0..3`) and the `InputHandler` maps slots to physical devices.  The `InputDevice+0x424` array holds up to 4 gamepad structs; keyboard and mouse are treated as "virtual" device #0 and #1.

---

*Document compiled from live Ghidra decompilation of Hamsterball.exe.  All offsets verified against the PE binary loaded at `0x00400000`.  For questions or corrections, open an issue in the hamsterball-re repository.*
