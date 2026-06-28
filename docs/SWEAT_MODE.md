# Sweat Mode — Ball Grip Assist & Visual Indicator

## Overview

When the ball rolls up a steep hill and begins to lose momentum, **sweat bubbles** (the `sweat.png` sprite) appear above the hamster. This is not just a visual effect — it's tied to a **physics grip assist system** that reduces the ball's collision force, allowing it to stick to steep surfaces and climb angles that would otherwise be impossible.

## Two Climbing Regimes

The ball physics has **two distinct climbing modes**, determined by whether the ball has full ground contact:

### Regime 1: Speed Climbing (grounded, no sweat)
- Ball maintains full surface contact (collision normal sums to zero)
- Minimum force: **1.25** (grounded minimum)
- Pass 1 multiplier: **0.5** (double at 0x4CF3E0)
- Pass 2 multiplier: **1.5** (double at 0x4CF458)
- Works up to ~**37°** (tan⁻¹(0.75) = 36.87°)
- Above 37°, the ball's gravity exceeds the driving force, ball starts to bounce

### Regime 2: Sweat/Grip Climbing (airborne on slope, sweat active)
- Ball loses full contact on steep slope (bounces slightly)
- Sweat flag set → force drops to **0.25** (5× weaker than grounded)
- Gentle force → ball doesn't launch off surface
- Collision reflection keeps ball near surface
- **Can climb any angle, including ~90° (vertical walls)**
- Tradeoff: very slow speed, sweat visual appears

### Why sweat mode enables steep climbing

The grounded force (1.25) produces violent collision reflections on steep surfaces, launching the ball off the surface. When the ball loses contact, there are no more collisions, and gravity pulls it down — making slopes above ~37° impossible.

Sweat mode's 0.25 force produces gentle reflections that keep the ball near the surface. Each frame:
1. Gravity pulls ball into surface
2. Collision reflects velocity along surface (upward on steep slopes)
3. Gentle force (0.25) keeps reflection small → ball stays on surface
4. Input direction drives ball up along the surface
5. Ball crawls up slowly but can traverse any angle

The 37° limit applies **only to sustained-speed climbing**. Sweat mode enables **slow grip-climbing** at any angle.

## Trigger Condition

The sweat flag (`ball+0x260`) is set inside `Ball_Update` (0x405E00) during the first collision response pass:

```
if (ball is NOT in full ground contact) {    // partial contact on slope
    if (computed_force < 0.25) {             // airborne threshold (double at 0x4CF440)
        ball+0x260 = 1;                       // SET SWEAT FLAG
        target_speed = 0.25;                  // minimum slope speed (float at 0x4CF380)
    }
}
else {  // ball is on flat ground
    if (computed_force < 1.25) {              // grounded threshold (double at 0x4CF4D0)
        target_speed = 1.25;                   // minimum ground speed (float at 0x4CF434)
    }
}
```

### The grounded check

The condition `(float)puStack_8c4 + fStack_904 == 0.0` checks whether the ball has **full surface contact** (checked against `_DAT_004cf368`, a 4-byte float = 0.0). On flat ground, the collision normal is straight up and the penetration depth sums to zero → "on ground" branch. On a slope, the tilted collision normal causes the sum to be non-zero → "not on ground" (slope) branch, triggering sweat.

### The collision-resolved check

The collision loop breaks when `dot(reflected_velocity, up_vector) >= _DAT_004cf4e0` (float = -0.01 at 0x4CF4E0). This means: if the velocity is now pointing upward or horizontal (dot product ≥ -0.01), stop processing collisions. This works at any slope angle because the reflection math redirects velocity along whatever surface the ball hit.

## Physics Effect

When sweat mode is active:

| State | Minimum Speed | Sweat Flag | Max Climbable Angle |
|-------|--------------|------------|---------------------|
| Flat ground (grounded) | 1.25 | Off | ~37° (speed climbing) |
| Slope/incline (airborne) | 0.25 | **On** | **~90° (grip climbing)** |
| Slope + shrunk (Odd Race) | 0.0625 (0.25 × 0.25) | **On** | ~90° (very slow) |

When the ball is shrunk (`ball+0xC4C` = is_shrunk, Odd Race pipe mechanic), the sweat speed is multiplied by 0.25 again, making the shrunk ball struggle even more on slopes (but still able to grip).

## Flag Clearing

The sweat flag **persists between frames** — it is NOT auto-cleared each tick. It is only cleared in the **second collision pass** when:

```
// Second pass (after collision loop):
target_speed = formula * speed * (float)1.5 * input   // pass 2 multiplier

if (target_speed > max_speed) {   // max_speed = 6.0 (ball+0x188)
    target_speed = max_speed;
    ball+0x260 = 0;                // CLEAR SWEAT FLAG (ball going fast enough)
    if (ball+0xC4C != 0) {         // if dizzy/shrunk
        target_speed *= 0.25;       // reduce by sweat multiplier
    }
}
```

This means: when the ball reaches high speed (force exceeds max_speed), the sweat flag is cleared — the ball is going fast enough to not need grip assist. When the ball is slow (can't reach max_speed), sweat stays set and grip mode continues.

## Visual Rendering

The sweat sprite is rendered in `Ball_RenderAI` (0x403DC0):

```c
if (ball+0x260 != 0 && ball+0x268 != 0) {
    float scale = ball_radius * 0.0385;  // _DAT_004cf408
    float y_offset = scale * 65.0;       // _DAT_004cf46c
    Sprite_RenderQuad(
        App+0x344,                      // sweat.png texture
        ball_x,                         // ball+0x164
        ball_y + y_offset,              // ball+0x168 + 65×scale (ABOVE ball)
        ball_z,                         // ball+0x16C
        scale,                          // size
        ball+0x208                      // color/render context
    );
}
```

### Flashing effect (ToggleTimer)

The second condition (`ball+0x268 != 0`) is the **ToggleTimer.active** toggle flag. The ToggleTimer struct (at `ball+0x264`) toggles its `active` flag every `period` ticks:

| Offset | Field | Type | Default | Description |
|--------|-------|------|---------|-------------|
| +0x264 | vtable | dword | — | ArenaBoard vtable pointer |
| +0x268 | active | byte | 0 | Toggles 0↔1 every period ticks |
| +0x26C | period | int | 20 | Toggle interval (~0.8s at 25fps) |
| +0x270 | counter | int | 0 | Increments each tick, resets at period |
| +0x274 | signal | byte | 0 | Set to 1 when toggle occurs |

This makes the sweat sprite **flash on/off** rather than display continuously, creating the "bubbling" appearance.

### Sprite details

- **Texture**: `sweat.png` loaded at `App+0x344` (by `App_ResourceLoader` at 0x4298C0)
- **Position**: Ball X/Z, Y + 65×(radius×0.0385) — small offset above ball center
- **Size**: radius × 0.0385 (very small, ~1px for default radius of 26)
- **Color**: From `ball+0x208` render context

## Struct Layout Summary

```
ball+0x260  byte   sweat_flag        — 1 when grip-climbing slope (slow + airborne on surface), 0 when at high speed
ball+0x264  dword  ToggleTimer.vtable
ball+0x268  byte   ToggleTimer.active — toggles 0↔1 every 20 ticks (controls flashing)
ball+0x26C  int    ToggleTimer.period — toggle interval (default 20)
ball+0x270  int    ToggleTimer.counter — tick counter
ball+0x274  byte   ToggleTimer.signal — toggle event flag
```

## Key Constants

All constants verified from .rdata memory inspection (June 2026). Doubles were confirmed by x86 instruction encoding: `DC` opcode prefix = 8-byte double operations, `D8`/`D9` = 4-byte float operations.

| Address | Type | Value | Bytes | Description |
|---------|------|-------|-------|-------------|
| 0x4CF368 | float (4B) | 0.0 | `00 00 00 00` | Grounded check: collision normal sum == 0.0 means grounded |
| 0x4CF3E0 | **double (8B)** | **0.5** | `00 00 00 00 00 00 E0 3F` | Pass 1 force multiplier (`DC 1D` = FCOMP qword) |
| 0x4CF380 | float (4B) | 0.25 | `00 00 80 3E` | Minimum slope/sweat speed |
| 0x4CF434 | float (4B) | 1.25 | `00 00 A0 3F` | Minimum grounded speed |
| 0x4CF440 | **double (8B)** | **0.25** | `00 00 00 00 00 00 D0 3F` | Airborne threshold: if force < 0.25, set sweat flag |
| 0x4CF458 | **double (8B)** | **1.5** | `00 00 00 00 00 00 F8 3F` | Pass 2 force multiplier (`DC 0D` = FMUL qword) |
| 0x4CF4D0 | **double (8B)** | **1.25** | `00 00 00 00 00 00 F4 3F` | Grounded threshold: if force < 1.25, use grounded min |
| 0x4CF4E0 | float (4B) | -0.01 | `0A D7 23 BC` | Collision-resolved: dot(vel, up) ≥ -0.01 → break collision loop |
| 0x4CF408 | float (4B) | 0.0385 | — | Sweat sprite size scale |
| 0x4CF46C | float (4B) | 65.0 | — | Sweat sprite Y offset multiplier |
| 0x4CF310 | float (4B) | 1.0 | — | One constant |

> **Important:** Addresses 0x4CF3E0, 0x4CF440, 0x4CF458, and 0x4CF4D0 store **8-byte IEEE 754 doubles**, not 4-byte floats. Ghidra's decompiler shows them as `_DAT` (4-byte float) with a `(float)` cast, but the actual x86 instructions use `DC` opcodes (qword/double operations). The low 4 bytes of these doubles happen to be `00 00 00 00` (float 0.0), which is misleading if you only read 4 bytes. Always read 8 bytes from these addresses.

## Functions Involved

| Function | Address | Role |
|----------|---------|------|
| `Ball_Update` | 0x405E00 | Sets/clears sweat flag in collision response |
| `Ball_RenderAI` | 0x403DC0 | Renders sweat sprite when flag + ArenaBoard active |
| `ToggleTimer_Tick` | 0x458E90 | Toggles active flag every period ticks |
| `ToggleTimer_Init` | 0x458E60 | Initializes ToggleTimer struct |
| `Ball_ctor2` | 0x4039E0 | Initializes ball+0x260 = 0 |
| `App_ResourceLoader` | 0x4298C0 | Loads sweat.png texture at App+0x344 |

## Analysis Notes

- `_DAT_004cf3e0` was previously documented as 0.0 (float). This is **wrong**. The address stores an 8-byte double with value 0.5. The first 4 bytes (`00 00 00 00`) read as float 0.0, but the x86 instruction at 0x406E71 uses `DC 1D` (FCOMP qword), confirming an 8-byte double read. The same applies to 0x4CF440 (double 0.25), 0x4CF458 (double 1.5), and 0x4CF4D0 (double 1.25). Always read 8 bytes from these addresses.
- The sweat flag is set when the ball has partial surface contact (slopes) and the computed force is below the airborne threshold (0.25). This means it can also trigger during brief airborne moments, but the tiny sprite size and flashing make it nearly invisible during fast movement.
- The ToggleTimer at `ball+0x264` was previously documented as "toggle_timer" — it's actually a periodic toggle timer that controls the sweat sprite's flashing animation.
- The two-regime climbing system (speed climbing vs grip climbing) explains why the ball can climb nearly vertical surfaces in-game despite the 37° force-balance limit. Sweat mode trades speed for grip — the 0.25 force keeps the ball on the surface through gentle collision reflections, allowing continuous upward drive along steep surfaces.
