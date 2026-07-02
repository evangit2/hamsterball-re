# Odd Race — Ball Shrink/Grow Mechanic

## Overview

The Odd Race (menu position 9) features a unique mechanic where the player's ball **shrinks to half size** when entering a shrink zone, then **grows back** when reaching a grow zone. This is used in the pipe maze section of the level.

## Level File Mapping

| Menu Name | Board Constructor | MESHWORLD File | Scene Setup Function |
|-----------|-------------------|----------------|----------------------|
| Odd Race | `LevelBoard_Odd_ctor` @ `0x0041ED80` | `Level6.MESHWORLD` | `Scene_SetupLevel6` @ `0x0040EA90` |

> **Note:** The file is named `Level6.MESHWORLD` but the board is `LevelBoard_Odd`. The numbering schemes don't match — file numbers are internal, board numbers are menu order. The Odd board's vtable (`0x4D0BC0`, slot `+0x48`) points to `Scene_SetupLevel6` which loads `levels\level6`.

## The Shrink/Grow Sequence

### Three collision events drive the mechanic

All three are event-name-triggered (`E:` prefix) collision geometry inside `Level6.MESHWORLD`:

| Event Name | Handler Code | What It Does |
|------------|-------------|-------------|
| `E:SHRINK` | `0x0040EF80` in `OddCollisionEvents` | Shrinks the ball |
| `E:GROWSOUND` | `0x0040F04E` in `OddCollisionEvents` | Plays grow sound (100-frame cooldown) |
| `E:GROW` | `0x0040F09E` in `OddCollisionEvents` | Restores ball to full size |

### `E:SHRINK` — Ball_Shrink (0x00402200)

```c
void __fastcall Ball_Shrink(int ball) {
    ball[0xC4C] = 1;           // is_shrunk flag = ON (ball is shrunk)
    ball[0x284] = 0x41500000;  // radius = 13.0 (was 26.0)
    ball[0x188] = 0x40200000;  // physics_scale = 2.5 (was 5.0)
    Sound_Play3D(board->sound_device->fall_sound, ball->pos);
}
```

Then the Odd handler:
1. Looks up `SHRINKCENTER` refpoint position from the hash table
2. Teleports the ball there: `(-285.2, -2210.5, -1068.8)`
3. Sets downward trajectory: velocity = `(0.0, -1.0, 0.0)` (Y=-1.0, pushes ball down)

### `E:GROWSOUND` — Sound Only

```c
if (ball[0x1FE] == 0) {  // cooldown counter
    Sound_PlayChannel(board->sound_device->grow_sound);
}
ball[0x1FE] = 100;  // 100-frame cooldown (4 seconds at 25fps)
```

### `E:GROW` — Ball_Grow (0x00402270)

```c
void __fastcall Ball_Grow(int ball) {
    ball[0xC4C] = 0;           // is_shrunk flag = OFF (ball is normal)
    ball[0x284] = 0x41D00000;  // radius = 26.0 (restored)
    ball[0x188] = 0x40A00000;  // physics_scale = 5.0 (restored)
}
```

## Field Changes Summary

| Ball Offset | Field Name | Normal Value | Shrunk Value | Ratio |
|------------|-----------|-------------|-------------|-------|
| `+0xC4C` | `is_shrunk` (byte) | `0` | `1` | — |
| `+0x284` | `radius` (float) | `26.0` | `13.0` | 50% |
| `+0x188` | `physics_scale` (float) | `5.0` | `2.5` | 50% |

## Physics Effects of Shrunk State

The `is_shrunk` flag (`ball+0xC4C`) gates several behaviors in `Ball_Update` (0x00405E00):

### 1. Speed Penalty (line ~858)
When the ball exceeds max speed and is shrunk:
```c
if (speed > max_speed) {
    speed = max_speed;
    if (ball[0xC4C] != 0) {      // is shrunk
        speed *= 0.25;           // _DAT_004cf380 = 0.25 → 75% speed reduction
    }
}
```
The shrunk ball moves at **25% of max speed** — significantly slower than normal.

### 2. Camera Skip (line ~797)
When the ball is on a fast-moving surface and shrunk:
```c
if (speed > 1.0 && ball[0xC4C] == 0) {  // only when NOT shrunk
    ball[0x2E9] = 1;           // on_ramp flag
    Scene_SetCamera(ball->board, ball, 1);
    // ... viewport setup
}
```
Camera tracking is **skipped** when shrunk — prevents camera jerk during teleport.

### 3. Bumper Counter Skip (line ~839)
```c
if (bumper_count != 0 && ball[0xC4C] == 0 && speed >= threshold) {
    bumper_count++;  // only increments when NOT shrunk
}
```

### 4. Ball_FallUpdate (vtable[65], 0x00408830)
When `is_stunned=1`, the game calls `Ball_FallUpdate` instead of normal physics. This function:
- Copies `ball+0x188` (physics_scale) to `physics_obj+0xC70`
- Uses `ball+0x284` (radius) for collision detection via `SpatialTree`
- Decays `ball+0xC60` (a float, starts at 1.0, decremented by 0.02/frame) — when it hits 0, sets `ball+0x2E8=1` (event flag, likely triggers respawn/end)

## Rendering Effects

### Ball_Render (0x00402DE0)
The ball's visual scale is derived from `ball+0x284` (radius):
```c
// Mesh scale = radius * 0.037037 (1/27)
vtable[0x38](ball->radius * _DAT_004cf39c);  // Gfx_Scale = 26.0/27 ≈ 0.963

// Shadow scale = radius * 0.028571 (1/35) * int_factor
Sprite_RenderQuad(..., ball->radius * _DAT_004cf398 * _DAT_004cf390, ...);
```

When shrunk: `13.0/27 ≈ 0.481` — the ball renders at **half visual size**.

## Refpoint Locations (Level6.MESHWORLD)

| Refpoint | Position (X, Y, Z) | Purpose |
|----------|-------------------|---------|
| `SHRINKCENTER` | `(-285.2, -2210.5, -1068.8)` | Where ball teleports after shrinking |
| `JUMPPIPE1` | `(-848.5, -1770.7, -645.6)` | First jump pipe exit (Y velocity = +16.0) |
| `JUMPPIPE2` | `(-566.3, -2113.6, -645.6)` | Second jump pipe exit (Y velocity = +16.0) |
| `PIPERANDOM1` | `(-379.8, -2620.7, -1825.9)` | Random pipe destination 1 |
| `PIPERANDOM2` | `(-268.2, -2598.9, -1805.9)` | Random pipe destination 2 |

## Complete Odd Collision Event List

The Odd board's collision dispatch handler is at `0x0040ED30` (vtable[0x1D], `+0x74`).

| Event | Effect |
|-------|--------|
| `E:GRAVITY(TYPE)` | Changes gravity: `NORMAL`/`X`/`Z` |
| `N:JUMPFIRST` | Teleport to JUMPPIPE1, launch up (Y=+16.0) |
| `N:JUMPSECOND` | Teleport to JUMPPIPE2, launch up (Y=+16.0) |
| **`E:SHRINK`** | **Shrink ball to 50% size, teleport to SHRINKCENTER** |
| `E:GROWSOUND` | Play grow sound (100-frame cooldown) |
| **`E:GROW`** | **Restore ball to full size** |
| `E:DROPLIFT` | Play break sound on mesh at board+0x436C |
| `E:PIPERANDOM` | Random teleport to PIPERANDOM1 or PIPERANDOM2 |
| `E:LIMIT` | Qualify for limit gate (axis 0) |
| `E:LIMITX` | Qualify for limit gate (axis 1) |
| `E:LIMITZ` | Qualify for limit gate (axis 2) |
| `E:LIMITPIPE1` | Qualify for limit (pipe 1 flag) |
| `E:LIMITPIPE2` | Qualify for limit (pipe 2 flag) |
| `E:SWALLOW` | Set swallow flag (`ball+0x2E8 = 1`, is_falling/shattered flag — NOT death_pending) |

## Related Functions

| Function | Address | Role |
|----------|---------|------|
| `Ball_Shrink` | `0x00402200` | Shrinks ball (radius 26→13, physics 5→2.5) |
| `Ball_Grow` | `0x00402270` | Restores ball (radius 13→26, physics 2.5→5) |
| `Ball_FallUpdate` | `0x00408830` | Physics update when shrunk (vtable[65]) |
| `Ball_Update` | `0x00405E00` | Normal physics (checks is_stunned flag) |
| `Ball_Render` | `0x00402DE0` | Visual scaling from radius |
| `OddCollisionEvents` | `0x0040ED30` | Event dispatch for Odd race |
| `LevelBoard_Odd_ctor` | `0x0041ED80` | Odd board constructor |
| `Scene_SetupLevel6` | `0x0040EA90` | Loads `levels\level6` MESHWORLD for Odd |
