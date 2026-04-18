# Reference Screenshots & Visual Comparison

Screenshots captured from the **original** Hamsterball.exe running under Wine via d3d8to9 + llvmpipe software rendering.

## Method

- Built [d3d8to9](https://github.com/crosire/d3d8to9) proxy DLL (D3D8→D3D9 translation)
- Placed `d3d8.dll` next to original `Hamsterball.exe`
- Ran on Xvfb (800x600x16) with `LIBGL_ALWAYS_SOFTWARE=1` + llvmpipe
- Navigated menus with xdotool mouse clicks
- Captured screenshots with scrot

## Screenshots

| File | Description |
|------|-------------|
| `original/01_title.png` | Title screen — "CLICK HERE TO PLAY!" |
| `original/02_menu.png` | Main menu — LET'S PLAY!, HIGH SCORES, OPTIONS, CREDITS, EXIT |
| `original/03_choose.png` | Game mode select — TOURNAMENT, TIME TRIALS, MIRROR, PARTY GAMES |
| `original/04_races.png` | Race list — WARM-UP RACE highlighted |
| `original/05_warmup.png` | Warm-Up gameplay — ball on starting pad, timer at 10.2s |
| `original/06_warmup2.png` | Warm-Up after some time passed |

## Side-by-Side Comparison Template

Use `compare/` folder for stitched left/right images for vision model analysis:
- Left: original game screenshot
- Right: reimplementation screenshot
- Same frame/angle for meaningful diff

## Key Visual Findings (Warm-Up Level)

### Original Game Appearance
- **Floor checkers**: white + light lavender/blue (NOT pink+white)
- **Sky/background**: dark blue (~RGB 22,37,85)
- **Platform side walls**: medium blue (directional lit) / slate blue (shadow)
- **Starting pad**: bright green circle on floor
- **Ball**: transparent sphere with hamster model inside, specular highlight
- **Lighting**: directional from upper-left, blue-tinted ambient
- **"GO!" arrow**: red arrow pointing forward from start
- **HUD**: timer top-center (~10.2s format), "WARM-UP RACE" text

### PinkChecker.bmp + Lighting Interaction
- Raw texture: RGB(255,192,192) pink + RGB(255,255,255) white (2x2 pixels)
- D3D render: MODULATE(texture, material_diffuse) under blue-tinted ambient light
- Result: pink × blue_tint → lavender, white × blue_tint → light blue-white
- This is why the original shows lavender checkers despite PinkChecker being pink

### Reimpl Differences (as of session 53)
- Floor: grey+pink (need blue-tinted ambient/diffuse light)
- Sky: different shade of blue (need to match ~22,37,85)
- Ball: grey sphere + green block (need transparent sphere + hamster model)
- Starting pad: missing entirely
- Lighting: flat/neutral (need blue-tinted directional + ambient)