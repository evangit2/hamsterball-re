# Dual Platform Arena v2

## Overview
Custom arena replacement for Warm-Up Arena (Arena-WarmUp.MESHWORLD). Features two large circular platforms with C-shaped railings, connected by a wide bridge.

## v2 Fixes (from v1)
| Issue in v1 | Fix in v2 |
|-------------|-----------|
| Players 1-4 cramped together (single START1-1) | START2-1 through START2-4 spread across platform at 150-unit intervals |
| 8-ball stuck | Removed BADBALL ref points (game spawns 8-balls automatically in arena mode) |
| Platforms too small (r=200) | Bigger platforms: r=350 |
| No railings → balls fall off | C-shaped railings on each platform (40 units high, 60° gap toward bridge) |
| Bridge too narrow (80 units) | Wider bridge: 240 units total width |
| Wrong camera ref name (CameraLocus1) | CAMERALOOKAT (matches original game format) |
| Missing STANDS/SAFESPOT | Added STANDS and 3 SAFESPOT ref points |

## Layout
```
       Platform A (pink, r=350)        Bridge (brown, 240 wide)        Platform B (blue, r=350)
       center: (-600, 0, 0)                                              center: (+600, 0, 0)
       
  START2-4(-750,67,+150)                                              START2-4(+750,815,+150)
       |                                                                     |
  START2-1(-750,67,-150)    [C-railing gap → bridge → gap← C-railing]   START2-3(+750,67,-150)
       |                                                                     |
  START2-3(-450,67,-150)                                              START2-3(+450,815,-150)
       |                                                                     |
  START2-2(-450,67,+150)                                              START2-2(+450,815,+150)
       
  C-shaped railing: 40 units high, gap faces bridge (60° opening)
```

## Geometry
| Component | Vertices | Triangles | Material |
|-----------|----------|-----------|----------|
| Platform A (pink cylinder) | 384 | 128 | (0.99, 0.63, 1.0) |
| Platform B (blue cylinder) | 384 | 128 | (0.42, 0.62, 0.91) |
| Railing A (C-shape, gray) | 900 | 252 | (0.7, 0.7, 0.8) |
| Railing B (C-shape, gray) | 900 | 252 | (0.7, 0.7, 0.8) |
| Bridge (brown box) | 24 | 12 | (0.55, 0.42, 0.30) |
| **Total** | **2384** | **772** | **5 geoms** |

## Ref Points (Section 1)
| Name | Position | Purpose |
|------|----------|---------|
| START2-1 | (-750, 67, -150) | Player 1 spawn (back-left of Platform A) |
| START2-2 | (-450, 67, +150) | Player 2 spawn (front-right of Platform A) |
| START2-3 | (-450, 67, -150) | Player 3 spawn (back-right of Platform A) |
| START2-4 | (-750, 67, +150) | Player 4 spawn (front-left of Platform A) |
| CAMERALOOKAT | (0, 0, 0) | Camera orbit center between platforms |
| PLATFORM × 10 | Various | Arena platform spawn positions |
| SAFESPOT × 3 | On each platform + bridge | Respawn safety points |
| STANDS | (0, -776, 0) | Arena spectator stands |

## C-Shaped Railings
Each platform has a curved railing that covers 300° of the circumference, with a 60° gap facing the bridge. The railing is:
- Height: 40 units above platform surface
- Radius: platform_radius + 8 (slightly outside edge)
- Thickness: 10 units (inner r-5 to outer r+5)
- Gap direction: Platform A gap faces +X (toward bridge), Platform B gap faces -X
- Color: Light gray (0.7, 0.7, 0.8)

## Installation
```bash
# Backup original
cp Levels/Arena-WarmUp.MESHWORLD Levels/Arena-WarmUp.MESHWORLD.bak

# Install custom arena
cp DualPlatformArenaV2.MESHWORLD Levels/Arena-WarmUp.MESHWORLD
```

## Testing Results
Tested in original Hamsterball.exe on Wine/llvmpipe (Xvfb display :99):
- ✅ Level loads correctly — binary format parses without errors (83158 bytes)
- ✅ Race starts — timer counting, ball on platform, TARGET: 15 displayed
- ✅ Ball stays on platform — physics working, ball doesn't fall through
- ✅ 3D geometry renders — platform surface visible
- ⚠️ Materials render dark on Wine/llvmpipe (known D3D8 lighting issue — colors will show on real Windows GPU)
- ⚠️ Camera follows ball closely in race mode — can't see both platforms simultaneously (arena mode uses wider camera)

## Regeneration
```bash
python3 tools/create_dual_arena_v2.py DualPlatformArenaV2.MESHWORLD
```
