# Ball Fall & Shatter System — Complete Analysis

Verified July 2026 via GhidraMCP decompilation. Supersedes all prior documentation about
ball+0xC60, Ball_FallUpdate, Ball_Shatter callers, and player ball fall death.

## Correction History

Previous docs (BALL_RESPAWN_SYSTEM.md, dizzy-stun-system.md, KEY_FINDINGS.md, hbremap)
contained multiple errors:

- "Ball_Shatter is called from FollowBall_Update when ball falls off edges" → **WRONG**.
  Ball_Shatter is called from Breaker_Update and Bonkbash_Update (arena crusher objects).
- "ball+0xC60 is a fall timer for the player ball" → **WRONG**. 0xC60 is a bad-ball-only timer.
- "Ball_FallUpdate (0x408830) is active code for the player ball" → **WRONG**. It's vtable[65]
  of the bad ball vtable (0x4CF494), not the player ball vtable (0x4CF3A0).
- "ball+0x768 = is_active/cam_active/race_active" → **WRONG**. It's the on-ground flag.

## Two Separate Death Systems

The game has **two completely independent ball death systems**: one for the player ball,
one for bad balls (AI balls). They use different vtables, different fields, and different
mechanisms.

### Player Ball (vtable 0x4CF3A0)

Constructor: `Ball_ctor` (0x4040AFE0) → calls `Ball_ctor2` (0x4039E0)

Fields set at construction:
- `ball+0x768 = 1` (on ground)
- `ball+0x80C = 0` (no pre-fall counter)
- `ball+0xC60` = **not set** (uninitialized — not used for player)
- `ball+0x2E8 = 0` (not dead)
- `ball+0x2E9 = 0` (not falling)

Update: `Ball_Update` (0x405E00, vtable[1]) — single massive function that handles
physics, collision, falling, and death. Ball_FallUpdate is NOT called for the player.

### Bad Ball / AI Ball (vtable 0x4CF494)

Constructor: `BadBall_ctor` (0x405D90, formerly "GameObject_sub_ctor") → calls
`Ball_ctor2` (0x4039E0) first, then sets:
- `ball+0x768 = 1` (on ground)
- `ball+0x80C = 50` (pre-fall grace counter)
- `ball+0xC60 = 1.0f` (fall timer)

Spawned by: `Scene_SpawnBadBall` (0x41EE86)

Update: `Ball_FallUpdate` (0x408830, vtable[65] at offset 0x104) — separate physics
update that uses a timer-based death mechanism.

## Player Ball Fall Death (NOT timer-based)

The player ball does NOT die from a fall timer. It dies when it **stops moving** after
going off an edge. The mechanism:

### Step 1: Edge detection (collision type 5)

When the ball rolls past the edge of a surface, the collision system generates a
type-5 collision entry. The entry's `+0x54` field (`piVar17[0x15]`) stores the
penetration depth — how far past the edge the ball has traveled.

In `Ball_Update` (0x405E00), the type-5 handler:
1. If `piVar17[0x15] > ~0` (tiny threshold): force surface normal to UP (0,1,0),
   reduce spin
2. If `piVar17[0x15] > 1.0` AND `ball+0x313 == 0` (not in special mode):
   - Set `ball+0x2E9 = 1` (falling mode)
   - Call `Scene_SetCamera(scene, ball, 1)` — camera follows the fall
   - Call `Graphics_SetViewport` — adjust viewport
   - Viewport bounds check may set `ball+0xBA = 1` (out of bounds → immediate respawn)
3. For multiplayer: raycast down to detect edge-hanging, increment `ball+0xBB` counter

### Step 2: Death trigger (stopped-moving check)

Each frame while `ball+0x2E9` is set (falling) and `ball+0xC9 == 0` (not in special
mode), `Ball_Update` checks if the ball has stopped moving on its primary axis:

```c
if (ball+0x2E9 != 0 && ball+0xC9 == 0) {
    if (ABS(ball_pos[axis] - prev_ball_pos[axis]) < threshold) {
        (*vtable[8])();  // Ball_FallDeath → death/respawn
    }
}
```

The primary axis depends on `ball+0x1D2` (gravity plane selector):
- Axis 0: Y position (`ball+0x168`)
- Axis 1: X position (`ball+0x164`)
- Axis 2: Z position (`ball+0x16C`)

The ball dies when motion on the primary axis stops. This happens when:
- **Ball lands on ground** — collision zeros velocity, position stops changing
- **Ball reaches apex of arc** (going up) — vertical velocity decelerates to zero
- **Ball hits a wall** — stopped on primary axis

**This is NOT a timer.** The ball can fall forever as long as it keeps moving. It only
dies when motion on the primary axis ceases.

### Step 3: vtable[8] → Ball_FallDeath (0x409480)

Called from the stopped-moving check. This is the death/respawn handler that sets
`ball+0x2E8 = 1` (needs_respawn) and triggers `Ball_FindClosestRespawnPoint`.

## Bad Ball Fall Death (timer-based)

Bad balls use a completely different death mechanism via `Ball_FallUpdate` (0x408830):

### Phase 1: Pre-fall counter (ball+0x80C)

- Initial value: 50 (set by `BadBall_ctor` at 0x405D90)
- Decrements by 1 per frame in Ball_FallUpdate, floored at 0
- Duration: 50 frames

### Phase 2: Fall timer (ball+0xC60)

- Initial value: 1.0f (set by `BadBall_ctor`)
- `BadBall_StartFallCountdown` (0x402390, vtable[69]) nudges 1.0 → 0.99 to start
  the countdown (only fires if c60 == 1.0)
- `Ball_FallUpdate` decrements by 0.02 per frame when:
  `(c60 != 0.0) OR (ball+0x768 == 0)` — i.e., timer active OR airborne
- When `c60 < 0.0`: sets `ball+0x2E8 = 1` (death)
- Duration: 1.0 ÷ 0.02 = 50 frames

### ball+0x768 (on-ground flag) — bad ball context

- Set to 1 by `Ball_ctor2` (0x4039E0) and `Ball_FindClosestRespawnPoint` (0x405190)
- Set to 0 by `OddBoard_CollisionHandler` (0x40ED30) when ball goes off edge
- When `768 != 0` (on ground) AND `c60 == 0.0`: timer stops (ball is safe)
- When `768 == 0` (airborne): timer decrements regardless of c60 value

### Total bad ball fall time: ~100 frames

| FPS | Phase 1 | Phase 2 | Total |
|-----|---------|---------|-------|
| 30  | 1.67s   | 1.67s   | 3.3s  |
| 60  | 0.83s   | 0.83s   | 1.7s  |
| 144 | 0.35s   | 0.35s   | 0.7s  |

Both phases are frame-rate dependent (per-frame decrements, not real-time).

## Ball_Shatter (0x408D70) — Arena Object Death

`Ball_Shatter` is NOT called when the player falls off edges. It's called from
arena/level objects that crush balls:

### Caller 1: Breaker_Update (0x43F3C0)

- Vtable: 0x4D50E0, slot[3]
- 4-state machine: active → cooldown → retract → cooldown
- State 0: scales up, iterates ALL balls in scene's ball list
- For each ball: computes 3D distance from breaker position
- If distance < threshold: creates 3 trajectory points, calls `Ball_Shatter(ball, trajectory)`
- Uses `AthenaHashTable_Lookup` with key `"BREAKER"` for position offset
- After shattering: calls `Scene_ForEachBall_SetVelocity` to push other balls away

### Caller 2: Bonkbash_Update (0x43F930)

- Vtable: 0x4D5140, slot[3]
- 3-state machine: arming → strike → cooldown
- State 0: uses `Collision_PointInTriangle` to find target ball in two triangle areas
- Raycasts along ball velocity to find impact point
- State 1: iterates ALL balls, checks distance from impact point
- For each ball within range: creates 3 trajectory points at 90.0 units spread,
  calls `Ball_Shatter(ball, trajectory)`
- Uses `"BONKBASH"` string for scene event dispatch
- Radius: 35.0 (36.5 on Impossible difficulty)

### What Ball_Shatter does to the ball

1. Checks `ball+0x324` — if already shattered, bail out
2. Frees `ball+0xC28` (allocated buffer)
3. Calls `board->vtable[0x78]()` — sets velocity on all other balls
4. Sets `ball+0x2E8 = 1` (death flag)
5. Checks `ball+0x744` (mode) — if 0: plays sound + creates fragments
6. Creates 3 `Ball_Split_ctor` fragments (0xC64 bytes each)
7. Copies velocity from original ball's physics struct
8. Sets fragment positions from trajectory param
9. Calls `Ball_SetTrajectory` on each fragment
10. Sets `fragment+0xC9 = 1` (is_split_ball), `fragment+0xCA = 1/2/4` (index)
11. Appends each fragment to `board+0x3204` (ball list)
12. Calls `Ball_CreateTrailParticles` for visual effect

### Ball_Split_ctor (0x408D10) — fragment constructor

Sets `ball+0xC60 = 5` (raw integer). As float, 5 is ~7e-45 (essentially zero),
so fragments die immediately in Ball_FallUpdate (one frame). Fragments are
visual-only debris.

## All Writers of ball+0x2E8 (death flag)

| Address | Function | Context |
|---------|----------|---------|
| 0x408CD5 | Ball_Shatter (0x408D70) | Balls hit by Breaker/Bonkbash |
| 0x406244 | Ball_Update (0x405E00) | Post-shatter timer (ball+0x2FC) expiry, gated by ball+0x324 |
| 0x407436 | Ball_Update (0x405E00) | Second death path (dynamic threshold) |
| 0x40F2E7 | OddBoard_CollisionHandler (0x40ED30) | Odd Race edge collision |
| (E:SWALLOW) | DispatchCollisionEvents (0x40C5D0) | Pipe swallow event |

Cleared by `Ball_FindClosestRespawnPoint` (0x405190) at 0x40525B.

## Key Field Reference

| Offset | Type | Name | Player Ball | Bad Ball |
|--------|------|------|-------------|----------|
| +0x2E8 | byte | needs_respawn | Set by Ball_Update (stopped after fall), Ball_Shatter, E:SWALLOW | Set by Ball_FallUpdate (timer expiry) |
| +0x2E9 | byte | dizzy_lock / falling | Set by collision type 5 (edge detection) | Not used |
| +0x324 | byte | shattered | Set by Ball_Shatter | Set by Ball_Shatter |
| +0x768 | byte | on_ground | Set to 1 by ctor/respawn, cleared by OddBoard | Same |
| +0x80C | int | pre_fall_counter | 0 (not used) | 50 (counts down to 0) |
| +0xC60 | float | fall_timer | NOT SET (uninitialized) | 1.0f (counts down by 0.02/frame) |
| +0x2FC | float | alpha / post_shatter_timer | Counts down when 0x324 set | Same |

## Vtable Reference

| Vtable | Address | Used by | Update Function |
|--------|---------|---------|-----------------|
| Ball (player) | 0x4CF3A0 | Player balls | Ball_Update (0x405E00) at slot[1] |
| BadBall | 0x4CF494 | AI/bad balls | Ball_FallUpdate (0x408830) at slot[65] (+0x104) |
| Breaker | 0x4D50E0 | Breaker objects | Breaker_Update (0x43F3C0) at slot[3] |
| Bonkbash | 0x4D5140 | Bonkbash objects | Bonkbash_Update (0x43F930) at slot[3] |

## Function Address Reference

| Address | Name | Role |
|---------|------|------|
| 0x405E00 | Ball_Update | Player ball physics/collision/death (vtable[1]) |
| 0x408830 | Ball_FallUpdate | Bad ball physics/timer death (vtable[65] of bad ball) |
| 0x408D70 | Ball_Shatter | Arena object crush death (Breaker/Bonkbash) |
| 0x409480 | Ball_FallDeath | vtable[8] — death/respawn handler for stopped ball |
| 0x405D90 | BadBall_ctor | Bad ball constructor (sets 0xC60=1.0, 0x80C=50) |
| 0x402390 | BadBall_StartFallCountdown | Nudges 0xC60 from 1.0 to 0.99 |
| 0x405190 | Ball_FindClosestRespawnPoint | Respawn: teleports, clears 0x2E8/0x2E9, sets stun |
| 0x4039E0 | Ball_ctor2 | Base ball constructor (sets 0x768=1, 0x80C=0) |
| 0x40AFE0 | Ball_ctor | Player ball constructor (sets vtable to 0x4CF3A0) |
| 0x41EE86 | Scene_SpawnBadBall | Spawns bad balls using BadBall_ctor |
| 0x43F3C0 | Breaker_Update | Crusher object that calls Ball_Shatter |
| 0x43F930 | Bonkbash_Update | Hammer object that calls Ball_Shatter |
| 0x40ED30 | OddBoard_CollisionHandler | Odd Race: sets 0x768=0, 0x2E9=1, 0x2E8=1 |
