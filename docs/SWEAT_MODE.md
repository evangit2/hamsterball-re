# Sweat Mode — Ball Physics Assist & Visual Indicator

## Overview

When the ball rolls up a hill and begins to lose momentum, **sweat bubbles** (the `sweat.png` sprite) appear above the hamster. This is not just a visual effect — it's tied to a **physics assist system** that gives the ball a minimum velocity to keep it climbing slopes.

## Trigger Condition

The sweat flag (`ball+0x260`) is set inside `Ball_Update` (0x405E00) during the collision response pass:

```
if (ball is NOT in full ground contact) {    // partial contact on slope
    if (target_speed <= 0.0) {                // always true (see note below)
        ball+0x260 = 1;                       // SET SWEAT FLAG
        target_speed = 0.25;                  // minimum slope speed
    }
}
else {  // ball is on flat ground
    if (target_speed <= 0.0) {
        target_speed = 1.25;                   // minimum ground speed (no sweat)
    }
}
```

### Why the speed check always passes

The target speed formula multiplies by `_DAT_004cf3e0` (at 0x4CF3E0), which is **0.0**. This zeroes out the dynamic speed calculation, making the computed target speed always 0.0. Since `0.0 <= 0.0` is true, the minimum speed floor always kicks in.

This appears to be intentional — the dynamic speed scaling was disabled (set to 0.0), and the minimum speed constants (1.25 ground, 0.25 slope) are the actual effective values.

### The "not on ground" check

The condition `pvStack_8c4 + fStack_904 == 0.0` checks whether the ball has **full surface contact**. On flat ground, the collision normal is straight up and the penetration depth sums to zero → "on ground" branch. On a slope, the tilted collision normal causes the sum to be non-zero → "not on ground" (slope) branch, triggering sweat.

## Physics Effect

When sweat mode is active:

| State | Minimum Speed | Sweat Flag |
|-------|--------------|------------|
| Flat ground | 1.25 | Off |
| Slope/incline | 0.25 | **On** |
| Slope + shrunk (Odd Race) | 0.0625 (0.25 × 0.25) | **On** |

The 0.25 minimum speed is a **struggle assist** — it prevents the ball from stopping completely on uphill sections by maintaining a small forward velocity even when gravity deceleration exceeds momentum.

When the ball is shrunk (`ball+0xC4C` = is_shrunk, Odd Race pipe mechanic), the sweat speed is multiplied by 0.25 again, making the shrunk ball struggle even more on slopes.

## Flag Clearing

The sweat flag **persists between frames** — it is NOT auto-cleared each tick. It is only cleared when:

```
if (target_speed > max_speed) {   // max_speed = 5.0 (ball+0x2A4)
    target_speed = max_speed;
    ball+0x260 = 0;                // CLEAR SWEAT FLAG
}
```

This means the sweat flag stays set while the ball is struggling at low speed on slopes, and only clears when the ball reaches high speed (flat/downhill acceleration to 5.0+).

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

### Flashing effect (RumbleBoard)

The second condition (`ball+0x268 != 0`) is the **RumbleBoard.active** toggle flag. The RumbleBoard struct (at `ball+0x264`) toggles its `active` flag every `period` ticks:

| Offset | Field | Type | Default | Description |
|--------|-------|------|---------|-------------|
| +0x264 | vtable | dword | — | RumbleBoard vtable pointer |
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
ball+0x260  byte   sweat_flag        — 1 when struggling on slope, 0 when at high speed
ball+0x264  dword  RumbleBoard.vtable
ball+0x268  byte   RumbleBoard.active — toggles 0↔1 every 20 ticks (controls flashing)
ball+0x26C  int    RumbleBoard.period — toggle interval (default 20)
ball+0x270  int    RumbleBoard.counter — tick counter
ball+0x274  byte   RumbleBoard.signal — toggle event flag
```

## Key Constants

| Address | Value | Description |
|---------|-------|-------------|
| 0x4CF3E0 | 0.0 | Speed scaling multiplier (disables dynamic calc) |
| 0x4CF440 | 0.0 | Air/slope speed threshold |
| 0x4CF4D0 | 0.0 | Ground speed threshold |
| 0x4CF434 | 1.25 | Minimum ground speed |
| 0x4CF380 | 0.25 | Minimum slope speed (applied with sweat flag) |
| 0x4CF408 | 0.0385 | Sweat sprite size scale |
| 0x4CF46C | 65.0 | Sweat sprite Y offset multiplier |
| 0x4CF368 | 0.0 | Zero constant (for comparisons) |
| 0x4CF310 | 1.0 | One constant |

## Functions Involved

| Function | Address | Role |
|----------|---------|------|
| `Ball_Update` | 0x405E00 | Sets/clears sweat flag in collision response |
| `Ball_RenderAI` | 0x403DC0 | Renders sweat sprite when flag + RumbleBoard active |
| `RumbleBoard_TickTimer` | 0x458E90 | Toggles active flag every period ticks |
| `RumbleBoard_InitTimer` | 0x458E60 | Initializes RumbleBoard struct |
| `Ball_ctor2` | 0x4039E0 | Initializes ball+0x260 = 0 |
| `App_ResourceLoader` | 0x4298C0 | Loads sweat.png texture at App+0x344 |

## Analysis Notes

- The `_DAT_004cf3e0 = 0.0` constant effectively disables the dynamic target speed calculation, making the minimum speed floors (1.25 ground, 0.25 slope) the actual effective speeds. This may have been a tuning decision during development — the dynamic scaling was turned off by zeroing the multiplier.
- The sweat flag is set whenever the ball has partial surface contact (slopes) and the target speed is ≤ 0 (always). This means it can also trigger during brief airborne moments, but the tiny sprite size and flashing make it nearly invisible during fast movement.
- The RumbleBoard at `ball+0x264` was previously documented as "rumble_timer" — it's actually a periodic toggle timer that controls the sweat sprite's flashing animation.
