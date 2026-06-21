# Dual Platform Arena

## Overview
Custom arena replacement for Warm-Up Arena (Arena-WarmUp.MESHWORLD). Features two circular platforms connected by a bridge, with one 8-ball spawn on each platform.

## Layout
```
     Platform A          Bridge           Platform B
     (pink, r=200)    (brown, 300×80)    (blue, r=200)
     center: (-350,0,0)                   center: (+350,0,0)
     
     [Player Start]                      [8-Ball B]
                    [8-Ball A]
                        |
                    [Bridge connects inner edges]
```

## Geometry
- **Platform A**: Cylinder, radius=200, thickness=30, center=(-350,0,0), pink material (0.99,0.63,1.0)
- **Platform B**: Cylinder, radius=200, thickness=30, center=(+350,0,0), blue material (0.42,0.62,0.91)
- **Bridge**: Box from x=-160 to x=160, z=-40 to z=40, y_top=15, y_bot=-5, brown material (0.55,0.42,0.30)
- **Total**: 600 vertices, 204 triangles, 3 geoms

## Ref Points (Section 1)
| Name | Position | Purpose |
|------|----------|---------|
| START1-1 | (-350, 67, 0) | Player spawn on Platform A |
| CameraLocus1 | (0, 50, 0) | Camera orbit center between platforms |
| BADBALL | (-270, 67, 80) | 8-ball spawn on Platform A |
| BADBALL | (+270, 67, -80) | 8-ball spawn on Platform B |

## Testing Results
Tested in original Hamsterball.exe on Wine/llvmpipe (Xvfb display :99):
- ✅ Level loads correctly — binary format parses without errors
- ✅ Race starts — "WARM-UP RACE" banner, SET!/GO! countdown, timer at 70.0
- ✅ Ball spawns on Platform A — player ball visible on dark platform surface
- ✅ Physics works — ball stays on platform, doesn't fall through
- ✅ 3D geometry renders — platform surface, bridge, and second platform visible
- ⚠️ Materials render dark on Wine/llvmpipe (known D3D8 lighting issue — colors will show correctly on real Windows GPU)
- ⚠️ 8-balls not visible in race mode (BADBALL spawns only work in Arena mode, which requires tournament unlock)

## Installation
```bash
# Backup original
cp Levels/Arena-WarmUp.MESHWORLD Levels/Arena-WarmUp.MESHWORLD.bak

# Install custom arena
cp DualPlatformArena.MESHWORLD Levels/Arena-WarmUp.MESHWORLD
```

## Regeneration
```bash
python3 tools/create_dual_arena.py DualPlatformArena.MESHWORLD
```

## Notes
- The platforms are thick cylinders (top + bottom + side wall) for robust collision
- Each platform uses 24 segments for a smooth circle
- The bridge overlaps slightly with both platforms for seamless collision
- CameraLocus is centered between platforms for a good overview angle
