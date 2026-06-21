# jump_mod

Press SPACE to jump (Player 1 only, raycast-based ground check).

## Behavior

- **SPACE** applies an upward velocity impulse (500.0) to Player 1's ball.
- **Raycast ground detection:** A background thread probes downward from the ball
  center each ~10ms using the engine's own `Mesh_FindClosestCollision` (0x00465D90).
  If geometry is within `(radius + 2.0)` units below the ball, `g_on_ground` is set
  and the jump is allowed. No cooldown timer.
- **Instant re-jump:** The player can jump again the exact frame they touch ground
  after a jump arc. No artificial 1-second lockout.
- **Ramp-friendly:** No velocity threshold — you can jump while rolling down ramps,
  on slopes, or moving at any speed. The raycast detects ground regardless of speed.
- **Death state excluded:** `Ball+0xC4C` (`fall_mode`) is checked — the ball cannot
  jump while in the fall-off-level/respawn state.
- **Edge detection:** SPACE uses rising-edge detection (one jump per keypress).

## Architecture (Pattern 4: volatile flag + polling thread)

This mod uses the **code cave + background thread** pattern, which is the only
safe way to call game functions from a DLL mod (calling C functions directly
from a hand-assembled code cave corrupts the stack/FPU/SEH state):

1. **Code cave** (in Ball_Update epilogue, per frame):
   - Stores ball pointer in `g_ball_ptr` (for the background thread)
   - Edge-detects SPACE press
   - Checks `g_on_ground` flag (set by background thread)
   - If grounded + pressed: writes jump velocity to `ball+0x174`

2. **Background thread** (Sleep 10ms loop, ~100 checks/sec):
   - Reads `g_ball_ptr` → Ball → Scene (Ball+0x14) → CollisionLevel (Scene+0x8B0)
   - Reads ball position (Ball+0x164/0x168/0x16C) and radius (Ball+0x284)
   - Calls `Mesh_FindClosestCollision(cl, &out, pos, {0,-1,0}, radius+0.5)`
   - Sets `g_on_ground = (|out.y - pos.y| <= radius + 2.0) ? 1 : 0`

The background thread is safe because level collision geometry is static during
gameplay and `Mesh_FindClosestCollision` creates a temp CollisionMesh per call
(no shared mutable state with the main game loop).

## Raycast API

```c
// __thiscall: ECX=collision_level, rest on stack
// ret 0x20 (32 bytes = 8 DWORDs)
typedef Vec3* (__thiscall *MeshRaycast_t)(
    void* collision_level,  // ECX = Scene+0x8B0
    Vec3* out,             // output hit point
    Vec3 origin,           // ray start (ball center)
    Vec3 direction,        // ray direction (0,-1,0 for downward)
    float max_dist         // sphere radius for AABB broad-phase
);
// Address: 0x00465D90
```

Key details:
- `max_dist = radius + 0.5f` (matches engine's own ground probes)
- Direction is normalized internally, then scaled to 99999, clamped to ~994
- On **hit**: `out` = intersection point (floor directly below ball)
- On **miss**: `out` = endpoint ~994 units along direction (NOT origin)
- Always check `|out - origin|` distance after the call
- The `__thiscall` keyword in the typedef is **critical** — without it,
  `collision_level` goes on the stack instead of ECX and all params shift

## Files
- `jump_mod.c` — C source code (BASS proxy + code cave + background thread)
- `bass.dll` — Compiled DLL (PE32 i386, 93KB)
- `jump_mod.zip` — Packaged zip

## Previous approaches that didn't work

1. **`Ball+0x281` (`is_falling`):** Legacy/init flag, not read during Ball_Update.
2. **`Ball+0x2E9` (`on_surface`):** Sticky limit/trajectory flag, never cleared in
   Ball_Update. Once set to 1, stays 1 until respawn — inverted behavior.
3. **Velocity threshold:** Would block jumping on ramps.
4. **60-frame cooldown timer (previous version):** Worked but was imprecise —
   player had to wait the full 1 second even if they landed earlier. Replaced
   by raycast which gives instant ground contact detection.

## Proxy Type
BASS.dll proxy. Installation:
1. Rename original `bass.dll` → `bass_real.dll` in the Hamsterball game folder
2. Copy the mod's `bass.dll` into the game folder
3. Launch Hamsterball
