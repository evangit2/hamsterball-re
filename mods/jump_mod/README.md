# jump_mod

Press SPACE to jump (Player 1 only, raycast-based ground check).

## Behavior

- **SPACE** adds an upward velocity impulse (+500.0) to Player 1's ball.
  The impulse is **added** to the existing accumulated velocity via x87 FPU
  (`FLD`/`FADD`/`FSTP`), so horizontal momentum is fully preserved.
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

## v3: Phase 15 Hook (Horizontal Momentum Fix)

Previous versions hooked at `0x004082B6` (end of Ball_Update, Phase 23). The jump
impulse was written to `ball+0x174` AFTER position was already finalized. Next frame,
the ball was still on the ground, so **floor collision (type 5) fired and zeroed
the horizontal velocity** — the jump killed all XZ momentum.

**v3 hooks at `0x00407BB4` (Phase 15)** — the `Ball_ApplyForce` call site. This is
BEFORE position finalization. The jump impulse enters the force pipeline and
`Ball_ApplyForce` accumulates it into the position delta. The ball lifts off the
ground in the **same frame** as the jump, so next frame's floor collision (type 5)
never fires. Horizontal momentum is fully preserved.

### Ball_Update physics pipeline context

| Phase | What happens | Hook impact |
|-------|-------------|-------------|
| 8     | Velocity accumulators zeroed | — |
| 9-13  | Collision tree built, gravity reflected, floor/wall collisions processed | If ball on ground: type 5 zeroes XZ |
| 14    | Roll physics: surface gradient → velocity direction | Computes roll direction |
| **15**| **Ball_ApplyForce called (0x407BB4)** ← **HOOK HERE** | **Jump impulse added before position finalize** |
| 16    | Position finalized from velocity + lerp | Ball lifts off THIS frame |
| 23    | End of Ball_Update (old hook site) | Too late — position already done |

## Architecture (Pattern 4: volatile flag + polling thread)

This mod uses the **code cave + background thread** pattern, which is the only
safe way to call game functions from a DLL mod (calling C functions directly
from a hand-assembled code cave corrupts the stack/FPU/SEH state):

1. **Code cave** (in Ball_Update Phase 15, per frame):
   - Stores ball pointer in `g_ball_ptr` (for the background thread)
   - Edge-detects SPACE press
   - Checks `g_on_ground` flag (set by background thread)
   - If grounded + pressed: FADD's jump velocity to `ball+0x174`

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

## v21: Controls-Gated (Countdown + Goal Freeze)

The game disables directional controls during the Ready-Set-Go countdown at race
start and after the ball reaches the goal. This is done via `ball+0x14C` — a byte
flag set to 1 by `Scene_StartCountdown` (0x437130) and `Scene_HandleRaceEnd`
(0x41B130). `Ball_Update` checks this at `0x4060A1` and skips the entire
movement/input block when non-zero.

The jump mod now also checks `ball+0x14C` before allowing a jump, preventing
jumps during the countdown and after finishing the race.

## Files
- `jump_mod.c` — C source code (BASS proxy + code cave + background thread)
- `bass.dll` — Compiled DLL (PE32 i386, 96KB)
- `jump_mod_v21.zip` — Packaged zip

## Previous approaches that didn't work

1. **`Ball+0x281` (`is_falling`):** Legacy/init flag, not read during Ball_Update.
2. **`Ball+0x2E9` (`on_surface`):** Sticky limit/trajectory flag, never cleared in
   Ball_Update. Once set to 1, stays 1 until respawn — inverted behavior.
3. **Velocity threshold:** Would block jumping on ramps.
4. **60-frame cooldown timer (v1):** Worked but was imprecise — player had to wait
   the full 1 second even if they landed earlier. Replaced by raycast.
5. **Phase 23 hook with FADD (v2):** Hooked at end of Ball_Update. The FADD
   preserved vel.x/z in the accumulator, but next frame's floor collision (type 5)
   zeroed XZ before the jump could lift the ball. Fixed in v3 by moving hook to
   Phase 15 (before position finalization).

## Proxy Type
BASS.dll proxy. Installation:
1. Rename original `bass.dll` → `bass_real.dll` in the Hamsterball game folder
2. Copy the mod's `bass.dll` into the game folder
3. Launch Hamsterball
