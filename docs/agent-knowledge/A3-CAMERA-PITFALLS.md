# A3 - Camera System Pitfalls

`Scene+0x3F1C` (`path_follow_mode`) is **not** a camera smoothing toggle. It is a pointer check for a `CAMERALOCUS` object; if the object exists, the camera target blends along a spline. If absent, the camera target is the ball. The perceived "smooth follow" comes from the ball's display position interpolation, not this flag.

Actual camera parameters:

| Offset | Name | Effect |
|--------|------|--------|
| 0x29BC | camera_orbit_angle | Fixed orbit direction `(cos, 0.9, sin)` |
| 0x29C0 | camera_orbit_dist | Distance from target to eye |

Original `Scene_SetCamera` places the camera at:
```cpp
orbit_dir = (cos_a, 0.9, sin_a);
eye = target + normalize(orbit_dir) * orbit_dist;
```

In reimpl, Level1/2 work with this sign; Level3 requires the opposite sign because its geometry lies mostly at `Z < ball.Z`. An adaptive sign heuristic or level-specific override is needed.
