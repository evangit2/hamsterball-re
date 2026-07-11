# Magnifying Glass Heat System (Sky Race)

## Overview

The magnifying glass is a Sky Race (Level 9) hazard. It floats above the track,
homes toward balls in its trigger zone, and heats them with a "burning beam."
When a ball's heat counter exceeds 1.1, it explodes (instant death).

The visual "burning" effect is a **white→red color shift** on the ball border sprite,
NOT a size change. The ball's 3D mesh is completely unaffected by heat.

## Key Functions

| Address | Name | Description |
|---------|------|-------------|
| 0x0043CB70 | `Magnifier_Update` | Per-frame magnifier update (vtable slot 1). Homes toward heated balls, increments heat counter, spawns spark particles. |
| 0x00436250 | `Magnifier_ctor` | Constructor. Initializes vtable (0x004D569C), position, scale (90.0), AthenaList for heated balls. |
| 0x00410D00 | `NeonCollisionEvents` | Collision dispatcher handling `E:HEATON`/`E:HEATOFF` events (adds/removes balls from heated list). |
| 0x00436390 | `Pendulum_AddIndex` | Adds ball to magnifier's heated list. Flips magnifier direction on first ball. |
| 0x004534D0 | `AthenaList_RemoveValue` | Removes ball from heated list (on `E:HEATOFF`). |
| 0x00401160 | `Graphics_SetColorMultiplier` | Sets 4-component RGBA color multiplier matrix. NOT a viewport clip. |
| 0x00403DB8 | `Ball_Render` | Reads heat counter and burning flag to apply color shift + burner overlay. |

## Ball Heat Fields

| Offset | Type | Name | Description |
|--------|------|------|-------------|
| `ball+0xC50` | `float` | `heat_counter` | Accumulates while in the magnifier beam. +0.025/frame. 0.0=normal, 1.1=death. |
| `ball+0xC58` | `byte` | `burning_flag` | Set to 1 each frame the magnifier is actively zapping (within 60 units 2D). Cleared at start of each Magnifier_Update pass. |

## Magnifier Object Layout (0x444 bytes)

| Offset | Type | Description |
|--------|------|-------------|
| +0x00 | `void**` | vtable → 0x004D569C |
| +0x04 | `int` | Board pointer (parent scene) |
| +0x08 | `float` | Position X |
| +0x0C | `float` | Position Y |
| +0x10 | `float` | Position Z |
| +0x14 | `float` | Velocity X |
| +0x18 | `float` | Velocity Y |
| +0x1C | `float` | Velocity Z |
| +0x20 | `float` | Phase counter (increments by 1.0/frame) |
| +0x24 | `float` | Current scale X (starts at 90.0) |
| +0x28 | `float` | Target scale (starts at 90.0) |
| +0x2C | `AthenaList` | Heated balls list (ball pointers currently being tracked) |

## Sky Board Offsets

| Offset | Description |
|--------|-------------|
| `board+0x47AC` | Magnifier object pointer |
| `board+0x4370` | Magnifying glass mesh (`meshes\magnifyingglass`) |
| `board+0x4394` | Pillar list (AthenaList for skypillar objects) |

## How It Works

### 1. Trigger: `E:HEATON` / `E:HEATOFF`

When a ball enters a trigger zone tagged `E:HEATON`, `NeonCollisionEvents` adds the
ball to the magnifier's heated list via `Pendulum_AddIndex`. If this is the first ball
added, the magnifier flips its movement direction.

When a ball exits (`E:HEATOFF`), it's removed from the heated list.

### 2. Magnifier Update (0x0043CB70)

Each frame, the magnifier's update runs:

1. **Clear burning flags** on all balls in the scene (`ball+0xC58 = 0`)
2. **If no heated balls**: Drift freely with velocity damping (×0.95/frame)
3. **If heated balls exist**: Home toward the first heated ball at max speed 6.0
4. **When within 60 units (2D distance)**:
   - `ball+0xC50 += 0.025` (increment heat counter)
   - `ball+0xC58 = 1` (set burning flag for this frame)
   - Spawn a spark particle at the ball position (scaled by `1.0 - heat_counter`)
   - If `heat_counter > 1.1`: Call ball death, spawn 24 explosion particles in a ring
5. **Animate scale**: Oscillate between current and target scale values

### 3. Ball Rendering (0x00403DB8) — The Color Shift

The heat counter affects rendering through `Graphics_SetColorMultiplier` (0x00401160),
which sets a 4-component RGBA multiplier matrix on the graphics object:

```c
void Graphics_SetColorMultiplier(gfx, float R, float G, float B, float A) {
    gfx->field_0x7A8 = 1;  // enable custom color multiplier
    Matrix_Scale4x4(&local, R, G, B, A);
    gfx->matrix_0x7B0 = local;
}
```

**Render order in Ball_Render:**

1. 3D ball mesh rendered (NOT affected by heat — drawn before any heat multiplier is set)
2. If heated: `SetColorMultiplier(gfx, ball_R, ball_G - heat, ball_B - heat, scale)`
   → Applied to attached objects (hamster model, gear)
3. `SetColorMultiplier(gfx, 1.0, 1.0, 1.0, scale)` → reset
4. If heated: `SetColorMultiplier(gfx, 1.0, 1.0-heat, 1.0-heat, scale)`
   → Applied to **ballborder.png** sprite
5. `SetColorMultiplier(gfx, 1.0, 1.0, 1.0, scale)` → reset
6. If `burning_flag == 1`: Render **ballburner.png** at identity (1.0, 1.0, 1.0)
   → Additive yellowish-white glow overlay, no color shift

### Color Gradient (ballborder.png as heat increases)

| Heat (ball+0xC50) | R | G | B | Visible Color |
|--------------------|---|---|---|---------------|
| 0.0 | 1.0 | 1.0 | 1.0 | White (normal) |
| 0.25 | 1.0 | 0.75 | 0.75 | Pinkish |
| 0.5 | 1.0 | 0.5 | 0.5 | Red |
| 0.75 | 1.0 | 0.25 | 0.25 | Deep red |
| 1.0 | 1.0 | 0.0 | 0.0 | Pure red |

The green and blue channels are reduced by the heat counter value, while red stays
at full. This creates the white→red gradient seen when the ball is being burned.

### Important Misconception Corrected

`Graphics_SetColorMultiplier` (originally named `Graphics_SetViewportClip` in Ghidra)
does **NOT** set viewport clipping bounds. It sets a 4-component color multiplier
matrix. The 4 parameters are RGBA color multipliers (0.0–1.0), not XYZ geometry
scale + viewport dimensions. The ball never shrinks during heating — only its color
shifts toward red.

## Using This for Ball Color Mods

The heat system reveals two methods for changing ball color at runtime:

### Method 1: Direct color fields (ball+0x20C–0x218)
Write RGBA floats directly to the ball's color fields. This affects the 3D mesh
material. See `ball+0x20C` (R), `+0x210` (G), `+0x214` (B), `+0x218` (A).

### Method 2: Graphics_SetColorMultiplier (0x00401160)
Call `Graphics_SetColorMultiplier(gfx, R, G, B, A)` before rendering sprites. This
multiplies the sprite/material colors by the given RGBA values. Used by the native
game for:
- Heat-based white→red shift on ballborder.png and attached objects
- Alpha transparency (via `SceneObj_SetAlpha` at 0x004011C0, which calls
  `SetColorMultiplier(gfx, 1.0, 1.0, 1.0, alpha)`)

The multiplier stays active until reset with `SetColorMultiplier(gfx, 1.0, 1.0, 1.0, scale)`
and `gfx+0x7A8 = 0` (disable flag).

## Textures

| Texture | Scene Offset | Purpose |
|---------|-------------|---------|
| `ballborder.png` | scene+0x33C | Ball border sprite (color-shifted by heat) |
| `ballburner.png` | scene+0x340 | Burning overlay sprite (yellowish-white glow, no color shift) |
| `sweat.png` | scene+0x344 | Sweat droplet sprite (grip-climbing effect, unrelated to heat) |
