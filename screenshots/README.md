# Hamsterball Reimplementation Screenshots

This directory contains organized visual documentation of the reimplementation progress.

## Directory Structure

- **`reimpl/`** — Current reimplementation renders (D3D8 Win32 build, Wine/llvmpipe)
  - `01_warmup_level1.png` — WarmUp (Level 1) with pink tier, ball visible, isometric camera
  - `02_warmup_racing.png` — WarmUp during countdown/race start
  - `03_arena_beginner.png` — Arena/Beginner level with blue tier
  - `04_isometric_view.png` — Isometric camera test showing full track
  - `05_track_geometry.png` — Track geometry visible (early wireframe-style shot)
  - `06_warmup_new.png` — WarmUp latest render with improved lighting

- **`progress/`** — Historical development milestones
  - `01_level_viewer_v2.png` — Standalone level viewer (OpenGL/SDL2)
  - `02_level1_early.png` — Very early Level 1 render
  - `03_level1_no_elimits.png` — Level 1 without E:LIMIT platform markers
  - `04_isometric_early.png` — Early isometric camera prototype

- **`original/`** — *(reserved for side-by-side original game comparisons)*

- **`comparison/`** — *(reserved for before/after comparison images)*

## Generating New Screenshots

The Windows executable can be run headless on Linux via Wine + Xvfb:

```bash
DISPLAY=:99 LIBGL_ALWAYS_SOFTWARE=1 wine hamsterball.exe
```

Captures are done with `import` (ImageMagick) or `xwd` from the virtual framebuffer.

## License

All screenshots in `reimpl/` and `progress/` are of open-source reimplementation code
and may be used freely. Screenshots in `original/` depict copyrighted game content
and are included for comparison purposes only.
