# First-Person Ball Cam (CEA)

## Version 4 — Simple Approach (Don't fight the engine, feed it)

### What it does
Places the camera at the ball's exact position by modifying the INPUTS the original camera code uses, then letting the game's own SetPosition + SetDirection calls run unmodified.

### Why v2/v3 failed
v2 and v3 tried to call `camera->vtable[2](X,Y,Z)` and `camera->vtable[3](&dir, dist)` ourselves. But we kept getting the orbit math wrong — the sign of `eye = target ± direction × distance` was never confirmed, and the direction vector's normalization behavior inside vtable[3] was unknown. Result: camera placed at random positions, swinging wildly.

### v4 approach: Modify inputs, not calls
Instead of calling vtable functions ourselves, v4:
1. **Overwrites the camera target** (stack locals at ESP+0xC/0x10/0x14) with the ball's exact position (edi+0x14/0x18/0x1C)
2. **Sets orbit distance** (Scene+0x29C0) to 1.0 — so the camera is essentially AT the target
3. **Sets orbit angle** (Scene+0x29BC) to 0 — sin(0)=0, cos(0)=1, direction = {0, 0.9, 1}
4. Then **lets the original code run** — it calls SetPosition and SetDirection with our modified values

No vtable calls, no FPU operations, no orbit math. The game does it all.

### Result
Camera should be at the ball's exact position, looking forward (+Z). Not a true first-person ball cam yet (no velocity-facing direction), but should be STABLE and at the RIGHT POSITION.

### Tunable parameters (CE address table)
| Symbol | Default | Description |
|--------|---------|-------------|
| fpCamHeight | 0.0 | Extra height above ball center (not yet wired) |
| fpCamDist | 1.0 | Orbit distance (1.0 = at ball, lower = closer) |

### Hook point
- **Address:** 0x41A254 (Scene_SetCamera epilogue)
- **Original bytes:** `8B 44 24 14 8B`
- **Jump-back:** 0x41A25E (original SetPosition + SetDirection + ApplyCamera)

### Next steps
- If position is correct: add velocity-based angle (atan2) to Scene+0x29BC
- If position is wrong: verify ESP offsets match original code's stack layout
