/* Ball Input & Audio System - Deep Documentation
 *
 * Ball_GetInputForce (0x46EC30)  - Input→force conversion for ball control
 * Sound_CalculateDistanceAttenuation (0x466750) - 3D sound distance falloff
 *
 * ═══════════════════════════════════════════════════════════════
 * Ball_GetInputForce (0x46EC30)
 * ═══════════════════════════════════════════════════════════════
 *
 * Signature: void __thiscall Ball_GetInputForce(BallInput* this, float* output)
 *
 * Converts raw input (keyboard/mouse/joystick) into a 2D force vector
 * that drives ball movement. Returns force in output[0] (X) and output[1] (Y).
 *
 * INPUT MODES (switch on this+0x08):
 *
 * CASE 1: KEYBOARD
 *   - this+0x04 → InputDevice → KeyboardDevice at +0x434
 *   - Key state bytes at KeyboardDevice+0x50C/+0x510/+0x514/+0x518 (DIK)
 *   - & 0x80 tests key-down state
 *   - Forward: KeyboardDevice+0x514 = fast forward (DAT_004D0250)
 *   - Backward: KeyboardDevice+0x518 = slow/reverse (DAT_004CF310)
 *   - Left: KeyboardDevice+0x50C → local_10 = -1.0
 *   - Right: KeyboardDevice+0x510 → local_10 = +1.0
 *   - Default forward speed: DAT_004CF368 (0.0 = no auto-forward)
 *
 * CASE 2: MOUSE
 *   - GetCursorPos to get cursor position
 *   - this+0x04 → App* at +4
 *   - Center X = App+0x15C / 2 (half window width)
 *   - Center Y = App+0x160 / 2 (half window height)
 *   - force_x = cursor_x - center_x
 *   - force_y = cursor_y - center_y
 *   - If App+0x15A != 0 (recenter mouse): SetCursorPos to center
 *   - This means mouse controls ball direction by offset from center
 *
 * CASE 4-7: JOYSTICK/CONTROLLER
 *   - this+0x10 = joystick device pointer
 *   - X axis: joystick+0x10C (divided by 100, cast to float)
 *   - Y axis: joystick+0x110 (divided by 100, cast to float)
 *   - Vec3_NormalizeAndScale(&point, 1.0) normalizes the joystick vector
 *   - local_10 = normalized Y, fVar3 = normalized X
 *
 * FINAL OUTPUT:
 *   speed_scale = *(float*)(this + 0x0C)  // Ball speed sensitivity
 *   output[0] = speed_scale * force_x
 *   output[1] = speed_scale * force_y
 *
 * OBJECT OFFSETS:
 *   this+0x04 = InputDevice* (keyboard) or App* (mouse)
 *   this+0x08 = input_mode (1=keyboard, 2=mouse, 4-7=joystick)
 *   this+0x0C = speed_scale (sensitivity multiplier)
 *   this+0x10 = joystick_device (only for mode 4-7)
 *
 * KEYBOARD OFFSETS:
 *   InputDevice+0x434 = KeyboardDevice* base
 *   KeyboardDevice+0x50C = key_left (DIK code, &0x80 for down)
 *   KeyboardDevice+0x510 = key_right (DIK code)
 *   KeyboardDevice+0x514 = key_forward_fast (boost speed)
 *   KeyboardDevice+0x518 = key_backward_slow (reverse speed)
 *
 * ═══════════════════════════════════════════════════════════════
 * Sound_CalculateDistanceAttenuation (0x466750)
 * ═══════════════════════════════════════════════════════════════
 *
 * Signature: float __thiscall Sound_CalculateDistanceAttenuation(
 *     SoundSystem* this, float x, float y, float z)
 *
 * Calculates volume attenuation for 3D positioned sounds based on
 * distance from the nearest "listener" (ball/camera position).
 *
 * ALGORITHM:
 *   1. Initialize min_dist = DAT_004D9120 (very large float, ~infinity)
 *   2. Iterate this+0x850 (listener_count) listeners starting at this+0x854
 *      - Each listener is 3 floats: (x, y, z)
 *      - Compute Euclidean distance: sqrt(dx²+dy²+dz²)
 *      - If distance < min_dist: update min_dist
 *   3. Apply rolloff curve:
 *      - If min_dist <= min_rolloff (this+0x914): return 1.0 (full volume)
 *      - If min_dist > max_rolloff (this+0x918): return 0.0 (silence)
 *      - Otherwise: return 1.0 - (min_dist - min_rolloff) / (max_rolloff - min_rolloff)
 *   4. Guard: if max_rolloff == min_rolloff, set range to 1.0 (avoid div/0)
 *
 * ROLLOFF CURVE: Linear interpolation
 *
 *   Volume  1.0 ─────╲
 *                    ╲
 *           0.5      ╲
 *                     ╲
 *           0.0 ───────╲─────
 *               min    max   Distance →
 *
 * SOUND SYSTEM OFFSETS:
 *   +0x850 = listener_count (number of 3D listener positions)
 *   +0x854 = listeners[] (array of Vec3[3] floats each)
 *   +0x914 = min_rolloff_dist (below this = full volume)
 *   +0x918 = max_rolloff_dist (above this = silent)
 */