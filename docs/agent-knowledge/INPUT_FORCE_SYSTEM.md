# Hamsterball Input Force System

## Overview

When the player provides input (keyboard, mouse, or gamepad), the game converts that input into a 3D force vector that is accumulated into the ball's physics each frame. This document traces the complete pipeline from raw input to force accumulation, with all values verified against the original binary.

## Force Pipeline

```
Ball_GetInputForce (0x46EC30)
  └→ returns 2D force vector (force_mult × direction)
       └→ Board_GetInputForce3D (0x41A9A0)
            └→ aggregates up to 4 input handlers, converts 2D→3D via camera
            └→ normalizes direction, scales by force_scale
            └→ returns 3D force = normalized_direction × force_scale
                 └→ Ball_ApplyForceV2 (0x4016F0)
                      └→ applies state multipliers
                      └→ accumulates to ball+0x170/174/178
                           └→ Ball_Update (0x405E00)
                                └→ reads & clears accumulators
                                └→ integrates into position/velocity
```

## Stage 1: Ball_GetInputForce (0x46EC30)

**`__thiscall void Ball_GetInputForce(void* this, float* outForce)`**

Reads raw directional input from one input handler and returns a 2D force vector.

### Input Handler Struct (pointed to by entries at Board+0x550+i*4)

| Offset | Type | Name | Description |
|--------|------|------|-------------|
| +0x00 | void* | vtable | Virtual table pointer |
| +0x04 | void* | app_ptr | Pointer to App/Scene |
| +0x08 | int | mode | Input mode: 1=keyboard, 2=mouse, 4-7=gamepad |
| +0x0C | float | force_mult | Scalar multiplier for raw input direction |
| +0x10 | void* | gamepad_state | Joystick state (modes 4-7 only) |

### Mode 1: Keyboard

Reads 4 DIK codes from `InputDevice+0x50C..0x518`:
- Forward (up) key → fVar3 = -1.0 (`_DAT_004D0250`)
- Backward (down) key → fVar3 = +1.0 (`_DAT_004CF310`)
- Left key → local_10 = -1.0
- Right key → local_10 = +1.0

Output: `(force_mult × ±1.0, force_mult × ±1.0)`

### Mode 2: Mouse

```c
GetCursorPos(&cursor);
center_x = screen_width / 2;   // App+0x15C
center_y = screen_height / 2;   // App+0x160
dx = cursor.x - center_x;       // pixel offset from center
dy = cursor.y - center_y;
// Recenter if enabled (App+0x15A byte):
if (recenter_enabled) SetCursorPos(center_x, center_y);
```

Output: `(force_mult × dx_pixels, force_mult × dy_pixels)`

### Mode 4-7: Gamepad

```c
raw_x = gamepad_state[0x10C] / 100;  // joystick axis
raw_y = gamepad_state[0x110] / 100;
Vec3_NormalizeAndScale(&dir, 1.0);
```

Output: `(force_mult × normalized_x, force_mult × normalized_y)`

### Final Scaling

All modes end with:
```c
outForce[0] = force_mult * direction_x;
outForce[1] = force_mult * direction_y;
```

## Stage 2: Board_GetInputForce3D (0x41A9A0)

**`__thiscall void Board_GetInputForce3D(void* this, float* outForce3D, int playerIndex)`**

Aggregates input from up to 4 input handlers and converts to a 3D world-space force.

### Process

1. **Iterate input handlers** at `Board+0x550 + playerIndex * 4`
2. **Call Ball_GetInputForce** for each handler
3. **Find max |force|** via `Vec2_Distance(0, 0, force_x, force_y)`
4. **Track if winner is mouse** (mode == 2 → `bVar3 = true`)
5. **Convert 2D → 3D**: Projects the 2D force through the camera using `Graphics_SetStreamBuffers`, then subtracts camera position to get world-space direction
6. **Normalize**: `fVar2 = 1.0 / sqrt(direction_x² + direction_y² + direction_z²)`
7. **Scale by force_scale**:
   - Keyboard/Gamepad: `force_scale = KeyboardForceScale` (0x4D03B8) = **0.12** (float)
   - Mouse: `force_scale = App+0x84C × MouseSensitivityMultiplier + MouseSensitivityOffset`
8. **Output**: `outForce3D = normalized_direction × force_scale`

### Force Scale Constants

| Constant | Address | Type | Value | Description |
|----------|---------|------|-------|-------------|
| KeyboardForceScale | 0x4D03B8 | float (4 bytes) | 0.12 | Default force scale for keyboard/gamepad |
| MouseSensitivityMultiplier | 0x4D03B0 | **double** (8 bytes) | 0.16 | Mouse sensitivity slope |
| MouseSensitivityOffset | 0x4D03A8 | **double** (8 bytes) | 0.1 | Mouse sensitivity baseline |

### ⚠️ CRITICAL PITFALL: Double-as-Float Misread

Ghidra's decompiler shows the mouse constants as `(float)` casts:
```c
fVar1 = *(float*)(scene + 0x84c) * (float)_DAT_004d03b0 + (float)_DAT_004d03a8;
```

But the actual x86 instructions use **QWORD (8-byte) double operations**:
```asm
0x41AC0F: fld dword [ecx + 0x84c]       ; sensitivity (genuine 4-byte float)
0x41AC15: fmul qword [0x4d03b0]          ; × 0.16 (DOUBLE, 8 bytes!)
0x41AC1B: fadd qword [0x4d03a8]          ; + 0.1  (DOUBLE, 8 bytes!)
```

Reading only 4 bytes at 0x4D03B0 gives 89128.96 (garbage). The correct value is 0.16.

This is the same pattern as:
- `_DAT_004CF4F8` (double 2.0, Ghidra shows float 0.0)
- `SliderStepSize` at 0x4CF308 (double 0.1, Ghidra shows float -1.59e-23)

### Mouse Force Scale Formula

```
force_scale = mouse_sensitivity × 0.16 + 0.1
```

| Sensitivity | Force Scale |
|-------------|-------------|
| 0.0 (min) | 0.10 |
| 0.5 (default) | 0.18 |
| 1.0 (max) | 0.26 |

### Caller: Board_GetPlayerInputForce (0x422C70)

This is a Board vtable method that wraps Board_GetInputForce3D. When the player's input mode is keyboard (99), it instead computes AI target-seeking force for the ball. Otherwise it delegates to Board_GetInputForce3D.

A second variant exists at 0x423800 (Board_GetPlayerInputForce2) with identical logic but a different AI force constant (`_DAT_004D1BAC` vs `_DAT_004CF4D8`).

## Stage 3: Ball_ApplyForceV2 (0x4016F0)

**`__thiscall void Ball_ApplyForceV2(void* this, float dirX, float dirY, float dirZ, float magnitude)`**

Accumulates directional force into the ball's physics force accumulators.

### Guards (all must pass to apply force)

| Offset | Condition | Meaning |
|--------|-----------|---------|
| ball+0x2F9 | == 0 | Not drowning |
| ball+0x2CC | == 0 | Not force-disabled (set by N:TARPIT, vacuum) |
| ball+0x808 | == 0 | Not impact-frozen (set by N:NOCONTROL, E:CATAPULTBOTTOM, vacuum) |
| ball+0x2F0 | < 0x51 (81) | Timer state counter below threshold |

### State Multipliers

| Condition | Offset | Multiplier | Constant | Value |
|-----------|--------|------------|----------|-------|
| Timer active (ball+0x2F0 ≠ 0) | ball+0x2F0 | ×0.25 | `_DAT_004CF380` | TimerStateMult |
| Sweat mode | ball+0x324 | ×0.0 | `_DAT_004CF378` | SweatForceMult |
| 8-ball mode | ball+0xC5C | ×0.20 | `_DAT_004CF374` | EightBallForceMult |
| C4C flag | ball+0xC4C | ×0.75 | `_DAT_004CF36C` | C4CFlagMult |

### Accumulation

```c
ball->force_x (0x170) += dirX * magnitude;  // after state multipliers
ball->force_y (0x174) += dirY * magnitude;
ball->force_z (0x178) += dirZ * magnitude;
```

Also sets `ball+0x198 = Math_Atan2Angle(dirX, dirZ/dirY depending on gravity axis)` and `ball+0x19C = 1` (has_direction flag).

## Stage 4: Ball_Update (0x405E00)

Reads and clears the force accumulators (`ball+0x170/174/178` → zeroed after read), then integrates them into ball position through the collision system and velocity calculations.

The force accumulators correspond to `param_1[0x5C]`, `param_1[0x5D]`, `param_1[0x5E]` in Ball_Update (since param_1 is `int*`, offset 0x5C×4 = 0x170).

## Force Magnitudes Summary

| Input Source | Formula | Value | Notes |
|---|---|---|---|
| Keyboard | fixed | **0.12** | Per frame, via Ball_ApplyForceV2 |
| Mouse (default, sens=0.5) | 0.5×0.16+0.1 | **0.18** | Per frame |
| Mouse (max, sens=1.0) | 1.0×0.16+0.1 | **0.26** | Per frame |
| Mouse (min, sens=0.0) | 0.0×0.16+0.1 | **0.10** | Per frame |
| Jump mod | hardcoded | **20.0** | One-shot, direct to ball+0x174 (bypasses Ball_ApplyForceV2) |

## Mouse Sensitivity Settings

### Storage
- **Live value**: `App+0x84C` (float)
- **Registry**: `HKCU\Software\Raptisoft\Hamsterball`, key `"MouseSensitivity"`
- **Default**: 0.5 (`0x3F000000`), set in `PauseGame` (0x428160) when registry key absent

### Slider (OptionsMenu_AdjustSlider, 0x442680)

OptionsMenu vtable[19]. Handles left/right key presses on slider items.

| Property | Value | Source |
|----------|-------|--------|
| Min | 0.0 | `_DAT_004CF368` |
| Max | 1.0 | `_DAT_004CF310` |
| Step | 0.1 | `SliderStepSize` (0x4CF308, **DOUBLE** — same double-as-float pitfall) |
| Storage | OptionsMenu+0xDF0 | During menu |

On menu close, `FUN_004284C0` writes OptionsMenu+0xDF0 to App+0x84C and persists to registry.

## Key Function Addresses

| Address | Name | Description |
|---------|------|-------------|
| 0x46EC30 | Ball_GetInputForce | Reads raw directional input, returns 2D force |
| 0x41A9A0 | Board_GetInputForce3D | Aggregates input, converts 2D→3D, applies force scale |
| 0x422C70 | Board_GetPlayerInputForce | Board vtable method, wraps GetInputForce3D with AI fallback |
| 0x423800 | Board_GetPlayerInputForce2 | Second variant (different AI force constant) |
| 0x4016F0 | Ball_ApplyForceV2 | Accumulates force into ball physics |
| 0x405E00 | Ball_Update | Consumes accumulators, integrates into position |
| 0x442680 | OptionsMenu_AdjustSlider | Slider adjustment handler |
| 0x428160 | PauseGame | Reads mouse sensitivity from registry (default 0.5) |
| 0x4284C0 | (save settings) | Writes mouse sensitivity to registry |
| 0x46EE10 | InputHandler_ctor | InputHandler constructor (allocates 0x438 bytes) |

## Key Data Addresses

| Address | Name | Type | Value | Description |
|---------|------|------|-------|-------------|
| 0x4D03B8 | KeyboardForceScale | float | 0.12 | Keyboard/gamepad force scale |
| 0x4D03B0 | MouseSensitivityMultiplier | **double** | 0.16 | Mouse force scale slope |
| 0x4D03A8 | MouseSensitivityOffset | **double** | 0.1 | Mouse force scale baseline |
| 0x4CF308 | SliderStepSize | **double** | 0.1 | OptionsMenu slider step |
| 0x4CF380 | TimerStateMult | float | 0.25 | Ball_ApplyForceV2 timer multiplier |
| 0x4CF378 | SweatForceMult | float | 0.0 | Ball_ApplyForceV2 sweat multiplier |
| 0x4CF374 | EightBallForceMult | float | 0.20 | Ball_ApplyForceV2 8-ball multiplier |
| 0x4CF36C | C4CFlagMult | float | 0.75 | Ball_ApplyForceV2 c4c flag multiplier |
| 0x4CF368 | ZeroFloat | float | 0.0 | Generic zero constant |
| 0x4CF310 | OneFloat | float | 1.0 | Generic one constant |

## Ghidra Double-as-Float Pitfall

This is a recurring pattern in the Hamsterball binary. The Ghidra decompiler shows `(float)` casts on values that are actually 8-byte doubles in the x86 code. The giveaway is always in the disassembly: `fmul qword` / `fadd qword` = 8-byte double, `fmul dword` / `fadd dword` = 4-byte float.

Known instances:

| Address | Ghidra shows (4 bytes) | Actual (8 bytes) | Used by |
|---------|----------------------|-------------------|---------|
| 0x4D03B0 | 89128.96 | **0.16** | Board_GetInputForce3D mouse force |
| 0x4D03A8 | -1.08e-19 | **0.1** | Board_GetInputForce3D mouse force |
| 0x4CF308 | -1.59e-23 | **0.1** | OptionsMenu_AdjustSlider step |
| 0x4CF4F8 | 0.0 | **2.0** | Ball_Update death distance check |
| 0x4CF3E0 | 0.0 | **0.01** | Ball_Update collision velocity factor |
| 0x4CF440 | ~0 | **0.01** | Ball_Update trail particle threshold |
| 0x4CF528 | 0.0 | **0.5** | Ball_Update Y offset |
| 0x4CF3C8 | 0.0 | **(varies)** | Ball constructor default check |
| 0x4D03E0 | 89128.96 | **0.02** | Board_UpdateRaceState race timer decrement/frame |
| 0x4CF538 | 89128.96 | **0.01** | Board_UpdateRaceState countdown timer increment/frame |
| 0x4D03C8 | 0.0 | **3.0** | Board_UpdateRaceState countdown phase threshold (seconds) |

**Rule**: When a physics constant from Ghidra's decompiler produces a nonsensical value (extremely large or tiny), ALWAYS check the x86 disassembly for `QWORD` vs `DWORD` qualifiers. Read 8 bytes and decode as `struct.unpack('<d', ...)` to get the true double value.
