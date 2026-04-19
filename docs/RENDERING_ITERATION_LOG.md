# Rendering Iteration Log (Sessions 50+)

## Goal
Pixel-perfect visual parity between D3D8 reimplementation and original Hamsterball (2004).

## Reference Screenshots
- `reimpl/reference-screenshots/original/05_level2_intermediate.png` — Level2 (Intermediate)
- `reimpl/reference-screenshots/original/06_warmup2.png` — Level1 (Warm Up)

## Iteration History

### v5 (pre-session)
- Wall shadow too dark (deep blue), walls too dim overall
- Floor too dark, sky close but slightly off

### v6 — Wall shadow lightened, lit_factor increased
- Wall lit=(0.80,0.92,1.0), shadow=(0.35,0.50,0.73)
- lit_factor = 0.85*NdotL1 + 0.15*NdotL2
- Vision result: walls (125,155,170) vs original (185,230,255) — STILL TOO DARK

### v7 — Target-matched colors approach
- Rewrote to define output lit/shadow colors directly matching original
- Wall lit=(0.73,0.90,1.0), shadow=(0.35,0.50,0.73) → (89,128,186)
- Floor lit=(0.96,0.96,1.0), shadow=(0.47,0.55,0.78)
- Vision: walls (125,155,170) still too dark, floor too blue

### v8 — Gamma curve added
- Added pow(lit_factor, 0.55) for steeper falloff
- Vision: contrast 2/10, wall shadow (110,140,175) vs orig (65,95,145) — too bright
- Root cause: fill light (L2) always adds brightness even to shadow faces

### v9 — Fill light removed, deeper shadows
- Set fill light weight to 0.0 (shadow color encodes ambient)
- Wall shadow=(0.18,0.25,0.38), lit_factor=pow(x,0.45)
- Floor shadow=(0.70,0.78,1.0) — pale lavender
- Sky: (85,120,215) deep royal blue
- Vision: shadow (82,116,153) — closer but room for improvement

### v10 — Shadow values tuned for zero-fill-light model
- Wall shadow=(0.24,0.35,0.49) → at lit_factor=0: (61,89,125) ≈ original (62,88,124) ✓
- lit_factor=pow(x,0.35) — aggressive gamma
- Floor bright nearly white, floor shadow lavender
- Pink checker: warm pink alternate squares
- Blue checker: light blue alternate squares
- Vision: contrast 4/10, shadows still reported as (110,140,175)

### v11-v14 — Checker pattern investigation
- Checker code WAS running but pattern invisible in screenshots
- Root cause: per-vertex Gouraud shading cannot show square checker patterns
  because D3D interpolates smoothly between vertex colors
- If all 3 vertices of a triangle are on the same checker cell → uniform triangle
- Tried 2x UV scale (u*2.0) — still invisible
- Also found: D3DTSS_COLOROP was set to MODULATE for textured geoms,
  which meant the (broken) BMP texture was overriding vertex colors
  → Changed to SELECTARG1+DIFFUSE for all geoms
- Still no visible checker pattern because per-vertex approach is fundamentally limited

### v15 — Programmatic checker textures (BREAKTHROUGH)
- Created 128x128 D3D textures via CreateTexture+LockRect at runtime
- PinkChecker: white + pale pink (0xFFE0D8EA / 0xFFFFF8FF)
- BlueChecker: white + light blue (0xFFD0E0FF / 0xFFFFF8FF)
- Set D3DTSS_COLOROP=MODULATE, COLORARG1=TEXTURE, COLORARG2=DIFFUSE
  → Per-pixel texture × per-vertex lighting gives visible checker!
- Removed per-vertex checker code (no longer needed)
- Vision: **checkerboard IS now visible** on floor surfaces!

## Current Color Targets (v15)

### Walls (non-textured geoms)
| State | RGB | Float values |
|-------|-----|-------------|
| Lit | (172,222,254) target → actual ~(185,230,255) | (0.72,0.90,1.0) |
| Shadow | (62,88,124) target → actual ~(61,89,125) | (0.24,0.35,0.49) |

### Floor (textured geoms)
| State | RGB | Float values |
|-------|-----|-------------|
| Lit | (245,245,255) | (0.96,0.96,1.0) |
| Shadow | (180,200,255) | (0.70,0.78,1.0) |

### Sky
- Clear color: D3DCOLOR_RGBA(85,120,215) — deep royal blue

### Lighting formula
```
NdotL1 = 0.35*nx + 0.85*ny + 0.35*nz  (primary: above-left)
NdotL2 = -0.3*nx + -0.6*ny + -0.3*nz  (fill: unused, weight=0)
raw_factor = 1.0 * NdotL1 + 0.0 * NdotL2
lit_factor = pow(raw_factor, 0.35)
final_color = shadow + (lit - shadow) * lit_factor
```

### Checker textures
- Programmatic 128×128, 8×8 grid (16px cells)
- MODULATE mode: texture_RGB × vertex_RGB per pixel
- Pink: alternate cells are pale pink (0xE0D8EA) vs near-white (0xFFF8FF)
- Blue: alternate cells are light blue (0xD0E0FF) vs near-white (0xFFF8FF)

## Remaining Issues

1. **Wall shadow mid-tones still slightly bright** — vision reports (100,135,175) for
   partially-shadowed faces vs original (62,88,124). The gamma curve pow(x,0.35)
   may need further tuning, or the shadow base values need slight reduction.

2. **Floor checker contrast** — pattern IS visible at v15 but very subtle.
   Need to increase color difference between checker squares.
   Pink and blue alternate squares should be more distinctly colored.

3. **Missing visual elements**:
   - GO! arrow (red arrow decal)
   - HUD timer (showcardgothic72 font + timerblot.png)
   - Start pad (green circle: Decal-Start.png)
   - Railing/pipes (purple pipe geometry)
   - Ball transparency (hamster inside glass sphere)

4. **Level2-specific**: bluebrick.png geom isn't getting checker texture
   (texture name detection only matches "Checker" not "brick").
   Need to decide if bluebrick wall geoms should get any special treatment.

## Key Architectural Decision
**Target-matching approach**: Instead of simulating the full D3D material×lighting
pipeline (which produces wrong colors on llvmpipe), we define lit/shadow output
colors that directly match the original game's appearance. The lit_factor interpolates
between them using a steep gamma curve. This is the inverse of standard rendering
but produces correct visual results.

## llvmpipe Texture Bug
Wine/llvmpipe cannot render BMP/PNG-loaded D3D8 textures. Workaround: create
programmatic D3D textures (CreateTexture+LockRect) at runtime which DO render.
This is used for checker patterns via MODULATE blending.