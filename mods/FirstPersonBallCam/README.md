# First-Person Ball Cam (CEA)

## Version 3 — Orbit Camera Fix + Smoothing

### What it does
Places the camera directly above the ball and faces the direction the ball is moving. Works in both arenas and race modes.

### v3 Fixes (from v2)
1. **EYE = TARGET + DIRECTION × DISTANCE (not minus!)** — v2 had the orbit formula backwards. The original game uses `eye = target + direction × distance`, meaning positive Y in direction = eye ABOVE target. v2 negated Y, putting the eye underground.
2. **Added frame-to-frame smoothing** — velocity direction changes every frame, causing sideways wiggling. v3 lerps the direction (20% per frame) to stabilize.
3. **Re-normalization after smoothing** — lerping can denormalize the vector, so v3 re-normalizes after smoothing.
4. **FPU stack bug fixed** — re-norm zero-magnitude path now properly pops the FPU stack before jumping to skipCamera.

### Camera model
```
SetPosition(X, Y, Z) = look-at TARGET (where camera looks toward)
SetDirection(&vec, dist) = direction + orbit distance
EYE = TARGET + DIRECTION × DISTANCE

Target = ball + (velDir.x × lookAhead, 0, velDir.z × lookAhead)
Direction = (ball.x - targetX, height, ball.z - targetZ)
           = (-velDir.x × lookAhead, height, -velDir.z × lookAhead)
Distance = 1.0
EYE = target + direction = ball + (0, height, 0)  ← directly above ball ✓
```

### Tunable parameters (CE address table)
| Symbol | Default | Description |
|--------|---------|-------------|
| fpCamHeight | 40.0 | Height above ball center (radius ~26) |
| fpCamLookAhead | 300.0 | How far ahead to look |
| fpCamSmoothing | 0.2 | Lerp factor (0=instant, 1=no movement) |

### Installation
Load `FirstPersonBallCam.CEA` in Cheat Engine, enable it.

### Hook point
- **Address:** 0x41A254 (Scene_SetCamera epilogue)
- **Original bytes:** `8B 44 24 14 8B`
- **Jump-back:** 0x41A2BF (ApplyCamera)
