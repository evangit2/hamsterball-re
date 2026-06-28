# Ball Respawn & Stun System

Complete RE of the respawn/stun state machine. Verified June 2026 via GhidraMCP decompilation.

## Correction History

Previous docs had MULTIPLE errors. Original mislabels (now corrected everywhere):
- Ball+0x2F9 was called "Collision occurred flag" → WRONG, then "is_falling" → STILL WRONG
- Ball+0x2FC was called "fall_timer / render_scale" → WRONG, it's alpha/opacity
- Ball+0x2F8 was called "respawn_in_progress" → WRONG, it's show_stars
- Ball+0x300 was called "respawn_timer" → WRONG, it's stun_timer

## The Four Respawn Fields

| Offset | Type | Name | Set by | Cleared by | Purpose |
|--------|------|------|--------|------------|---------|
| +0x2E8 | byte | needs_respawn (shattered) | Ball_Shatter, Ball_FallUpdate | FindClosestRespawnPoint (→0) | Triggers respawn search |
| +0x2F8 | byte | show_stars | FindClosestRespawnPoint (→1) | Ball_Update when stun_timer=0 (→0) | Enables star-circling render effect |
| +0x2F9 | byte | **is_stunned** | FindClosestRespawnPoint (→1) | Ball_Update when alpha≥1.0 (→0) | Blocks force, changes collision, excludes from AI |
| +0x2FC | float | **alpha** | FindClosestRespawnPoint (→0) | Counts up to 1.0 in Ball_Update | Opacity: 0=invisible→1=opaque. Ball fades in |
| +0x300 | int | stun_timer | FindClosestRespawnPoint (→150) | Ball_Update (decrements to 0) | Duration of stunned state. AI checks ==0 |

## Lifecycle

1. **Ball falls off edge** → Ball_FallUpdate detects ball below threshold → sets +0x2E8=1 (needs respawn)
2. **Respawn triggered** → Ball_FindClosestRespawnPoint (0x405190) teleports ball, sets:
   - +0x2F9=1 (is_stunned), +0x2FC=0 (alpha=invisible), +0x300=150 (stun_timer), +0x2F8=1 (show_stars)
3. **During stun** — Ball fading in (alpha 0→1 over ~150 frames), forces blocked, AI can't target,
   8 stars circle ball converging inward as alpha increases, collision uses raw position
4. **Alpha reaches 1.0** — Ball_Update clears +0x2F9=0 (stunned done). Stun_timer reaches 0,
   clears +0x2F8=0 (stars stop). Ball fully visible and interactive.

## Key Functions

| Function | Address | Purpose |
|----------|---------|---------|
| Ball_FindClosestRespawnPoint | 0x405190 | Teleports ball to nearest respawn point, sets stun fields |
| Ball_Update | 0x405E00 | 23-phase physics tick. Contains stun recovery (alpha increment, clear is_stunned when ≥1.0) |
| Ball_ApplyForceWithMultipliers | 0x402650 | Force application. FIRST guard: +0x2F9 (is_stunned) — if set, NO force |
| Ball_AI_ChaseNearest | 0x408390 | AI targeting. Filters out stunned balls (+0x2F9!=0, +0x300!=0) |
| Ball_FallUpdate | 0x408830 | Active code — sets +0x2E8=1 at 0x408CD5 when fall timer (+0xC60) expires |
| Ball_RenderAI | 0x403DC0 | Ball render. Contains star-circling effect, alpha rendering, impact wobble |
| SceneObj_SetAlpha | 0x4011C0 | MISNAMED "SceneObj_SetScale" by Ghidra. Sets alpha via W-component, NOT scale |
| Scene_UpdateBallsAndState | 0x41B540 | Respawn dispatcher. Detects +0x2E8 flag, calls FindClosestRespawnPoint |

## Star-Circling Effect (Ball_RenderAI, 0x403DC0)

Discovered June 2026 via GhidraMCP decompilation.

When `show_stars` (+0x2F8) is set, the ball's render function draws:
- **8 star sprites** at 45° intervals using sin/cos positioning
- Star distance from ball = `(1.0 - alpha) * spread + base`
- Stars start far away (alpha=0, just teleported) and **converge inward** as ball fades in
- **Aura/ring sprite** on ground beneath ball
- Uses sprites at `ball+0x3A0` (stars) and `ball+0x3A8` (aura)
- Temporarily adjusts projection (gfx+0x790/0x794) for zoom effect

## Alpha Rendering (NOT Scale)

When `alpha` (+0x2FC) != 1.0:
- `SceneObj_SetAlpha(0x4011C0)` sets `gfx+0x7A8=1` (custom alpha flag)
- `Graphics_SetViewportClip` receives +0x2FC as W-component of 4x4 matrix
- X/Y/Z components are all 1.0 — ball NEVER changes size, only transparency
- After render: `gfx+0x7A8=0` (reset to opaque)

**SceneObj_SetScale is a misnomer.** Ghidra auto-generated this name for 0x4011C0.
The function passes its parameter as the W-component (alpha) of a transformation matrix
while X/Y/Z are hardcoded to 1.0. Renamed to `SceneObj_SetAlpha` in Ghidra June 2026.

## Where is_stunned (+0x2F9) is READ

### 1. Ball_ApplyForceWithMultipliers (0x402650) — force guard
```c
if ((*(char*)(this + 0x2F9) == 0) &&    // is_stunned must be 0
    (*(char*)(this + 0x2CC) == 0) &&    // block_flag must be 0
    (*(int*)(this + 0x808) == 0) &&      // state must be 0
    (*(int*)(this + 0x2F0) < 0x51)) {   // impact_count < 81
    // apply force to velocity accumulators
}
```
If is_stunned=1, NO force is applied — ball can't be pushed, bumped, or steered.

### 2. Ball_FallUpdate (0x408830) — collision position override
```c
if ((*(char*)(ball + 0x2CC) != 0) || (*(char*)(ball + 0x2F9) != 0)) {
    // Use ball's CURRENT position instead of collision-derived position
}
```
Prevents snapping to surfaces during respawn.

### 3. Ball_AI_ChaseNearest (0x408390) — target exclusion filter
```c
if ((*(char*)(target + 0x768) != 0) &&     // active
    (*(char*)(target + 0x2F9) == 0) &&      // NOT stunned
    (*(int*)(target + 0x300) == 0) &&       // stun_timer == 0
    (*(char*)(target + 0x324) == 0) &&      // not teleporting
    (*(char*)(scene + 0x3A4C) != 0) &&      // countdown finished
    (*(int*)(target + 0x18) != -1))         // has valid player
```

### 4. Ball_Update (0x405E00) — display position
```c
if ((*(char*)(ball + 0x2F9) != 0)) {
    // Use ball's current position for rendering, not collision-adjusted
}
```

## Stun Recovery in Ball_Update (0x405E00)

Phase 7 of 23-phase physics tick (the "tar-state machine"):
```c
if (*(char*)(ball + 0x2F9) != 0) {
    alpha += _DAT_004cf524;          // increment alpha per frame
    if (alpha > 1.0) {               // _DAT_004cf310 = 1.0
        alpha = 1.0;                  // clamp
        *(byte*)(ball + 0x2F9) = 0; // CLEAR is_stunned
    }
}
```

Stun timer (+0x300) is decremented by 1 each frame, clamped at 0.
When it reaches 0, show_stars (+0x2F8) is cleared.

## Collision Entry Types

Hamsterball's physics system uses three collision entry types, stored in the first DWORD
(`*piVar16`) of each entry in the PhysicsBody's entry list (`ball+0x1A4` → `+0x18`):

| Type | Name | Created by | Key Effect |
|------|------|------------|------------|
| 1 | Ball-Ball | `CollisionNode_CollisionResponse` (0x467360) | Push-apart velocity exchange, collision sound, scoring (500pts × difficulty) |
| 2 | Wall/Floor (SpatialTree) | SpatialTree triangle intersection in substep (0x463E20) | **Writes LGP**, surface normal reflection, friction, up-vector updates |
| 5 | Deep Floor (penetration) | `Ball_AdvancePositionOrCollision` (0x4564C0) directly | Kill confirm, camera change, out-of-bounds detection |

Type 2 is the "normal walking on the track" collision. Type 5 is the "ball has penetrated through
the floor surface" collision — essentially death/fall detection.

**No types 3 or 4 exist** as entry types. The value `4` appears at `entry+0x04` as a *subtype* set
by `CollisionNode_IntersectTest`, but the main entry type (`entry+0x00`) is only ever 1, 2, or 5.

See `references/collision-detection-pipeline.md` for the full vtable map and entry creation details.

### Type-2 Entry: Velocity Gate (NO Backface Culling)

Type-2 entries are created by `SpatialTree_TestFace` (0x463E20), the per-face swept-sphere test.
**There is NO backface culling.** The function has one directional filter — a velocity gate —
and it does not discriminate which side of the triangle the ball approaches from.

**Step 1 — Distance check (symmetric):** `Vec3_DotDiffAbs` computes the **absolute value** of
the distance from the ball to the triangle plane. If `|distance| > 1.0`, skip. This is symmetric —
it passes regardless of which side the ball is on.

**Step 2 — Ray-sphere intersection:** Finds the contact point on the ball's sphere closest to
the triangle. Side-agnostic.

**Step 3 — Velocity gate** (the only directional check):
```c
fVar7 = Vec3_DotDiff(param_4, contact_pos, contact_normal);
if (fVar7 <= 0.0f) {  // proceed
    // ... point-in-triangle test, then create type-2 entry
}
// else: return 0 (ball moving away → no entry)
```
- `param_4` = ball velocity direction
- Passes when `dot(velocity, contact_normal) <= 0` — ball moving into or parallel to surface
- **Fails only when `dot > 0`** — ball moving *away* from surface

**Step 4 — Point-in-triangle test:** Checks whether the contact point projects inside the
triangle via edge dot products. Geometric, side-agnostic.

**Consequence for wall slides:** When the ball slides along a wall, velocity is perpendicular
to the wall normal → `dot ≈ 0` → `0 <= 0` → **PASSES**. Type-2 entries are created every frame
during wall contact. LGP updates continuously.

**Consequence for clip-through:** When the ball clips through a wall from the wrong side while
falling, velocity is downward and the wall normal is horizontal → `dot ≈ 0` → **PASSES**.
The distance check is absolute (side-agnostic), and the point-in-triangle test is geometric.
A type-2 entry is created → **LGP resets to the wall contact position.**

The game does NOT have double-sided walls. Level geometry is single-sided. The swept sphere
simply does not check which side the ball approaches from — it only checks whether the ball
is moving away (dot > 0 → skip) vs. into/parallel (dot <= 0 → pass).

### Type-5 Entry: Penetration Depth Gates

Type-5 entries are created by `Ball_AdvancePositionOrCollision` when the substep reports no surface
collision (`local_25 == 0`). The entry stores accumulated penetration time at `+0x54` (piVar16[0x15]).
Three sub-gates in `Ball_Update`'s entry processing handle different penetration depths:

1. **`piVar16[0x15] > _DAT_004cf420`** (deep): Resets up-vector to (0,1,0), damps speed
2. **`piVar16[0x15] > _DAT_004cf310`** (>1.0, AND not NPC): Sets `dizzy_lock` flag (ball+0x2E9=1),
   calls `Scene_SetCamera`, checks out-of-bounds → sets `needs_respawn` (ball+0xBA=1)
3. **`piVar16[0x15] >= _DAT_004cf4e8`** (moderate, AND player ball, AND no surface contact):
   Downward raycast via `Mesh_FindClosestCollision` — if distance exceeds `radius × _DAT_004cf418`,
   increments `surface_proximity_counter` (ball+0xBB). This is "about to fall off" detection.

## Last Grounded Position (LGP) — Respawn Search Position Source

The respawn point search does **NOT** use the ball's current live position (`ball+0x164/0x168/0x16C`).
Instead, it uses the **Last Grounded Position (LGP)** stored at:

| Offset | Name | Type | Description |
|--------|------|------|-------------|
| `Ball+0x2DC` | `lgp_x` | float | X coordinate of last type-2 surface collision |
| `Ball+0x2E0` | `lgp_y` | float | Y coordinate of last type-2 surface collision |
| `Ball+0x2E4` | `lgp_z` | float | Z coordinate of last type-2 surface collision |

The LGP is a **snapshot of the ball's position at the moment of its last type-2 surface collision**.
It is written inside `Ball_Update` (0x405E00) in the type-2 surface collision handler:

```c
// When ball's own collision mesh registers a type-2 surface hit:
param_1[0xB7] = param_1[0x59];  // ball+0x2DC = ball+0x164 (current X → LGP X)
param_1[0xB8] = param_1[0x5A];  // ball+0x2E0 = ball+0x168 (current Y → LGP Y)
param_1[0xB9] = param_1[0x5B];  // ball+0x2E4 = ball+0x16C (current Z → LGP Z)
```

*(Indices are `int*` stride — 0xB7 × 4 = 0x2DC, 0x59 × 4 = 0x164, etc.)*

**Why LGP instead of live position?** By the time the ball dies and `Ball_FindClosestRespawnPoint`
fires, the ball has typically fallen far below the track. Using the live position would find respawn
points near wherever the ball landed in the void. Instead, the game uses the LGP — the last place the
ball was touching solid ground — so it respawns you near where you fell off, not wherever your corpse
ended up.

The height gate in Mode 0 also uses LGP: `ball+0x2E0 (lgp_y) - (radius + 2.0) < respawn_point.Y` —
only considers respawn points above where the ball last touched ground (plus a small tolerance),
preventing it from teleporting you downward.

Distance formula using LGP:
```
dist = sqrt((lgp_x - point_x)² + (lgp_y - point_y)² + (lgp_z - point_z)²)
```

### LGP Behavior During Wall Slides — RESOLVED (v3)

**The LGP write in Ball_Update has NO surface-normal filter of its own** — it blindly writes LGP
for every type-2 entry from the ball's SpatialTree. The filtering happens upstream in the entry
*creation* path via `SpatialTree_TestFace` (0x463E20).

**There is NO backface culling gate.** The swept-sphere collision system does not discriminate
which side of a triangle the ball approaches from:

1. **Distance check** (`Vec3_DotDiffAbs`) uses **absolute value** — symmetric, passes regardless
   of which side the ball is on
2. **Velocity gate** uses `<= 0` — passes when `dot ≈ 0` (falling perpendicular to wall normal)
3. **Point-in-triangle test** — geometric, side-agnostic

The game does NOT have double-sided walls (user-corrected). Level geometry is single-sided.
The swept sphere simply doesn't check which side the ball is on — it only checks whether the
ball is moving away (dot > 0 → skip) vs. into/parallel (dot <= 0 → pass).

When the ball clips through a wall from the wrong side while falling:
- Velocity is downward, wall normal is horizontal → `dot ≈ 0` → **PASSES**
- Distance check is absolute → **PASSES** regardless of side
- Type-2 entry created → **LGP resets to the wall contact position**

**Wall slides DO create type-2 entries every frame.** LGP updates continuously during wall
contact — it does NOT freeze. The wall-slide shortcut works because the LGP follows the ball
down the wall to a position *below* the track surface, giving a respawn point that's lower
than where the ball originally fell from.

**Ball_FallUpdate (0x408830) uses the SAME pipeline** — creates a temporary SpatialTree from
Scene+0x8B0 and calls Ball_AdvancePositionOrCollision (vtable[1]), running the same
SpatialTree_TestFace. No separate LGP update path exists during falls.

**⚠ RE methodology lesson (3 iterations to get right):**
1. First claimed a "backface culling gate" at 0x4640b4 blocks back-side hits — WRONG
2. Then claimed double-sided walls explain it — WRONG (user: "game does not have
   double-sided walls")
3. Finally decompiled the full function via Ghidra `create_function` + `decompile_function`
   and found the distance check uses `Vec3_DotDiffAbs` (ABS) and velocity gate uses `<= 0`
   **Always `create_function` + decompile via Ghidra FIRST, before drawing conclusions
   from raw ASM.** The ASM dot-product patterns were misread across two iterations
   without the full decompiled context.

**Previous labels corrected:** `checkpoint_x/y/z` and "Last safe position" were partially correct
but misleading. These are specifically the last grounded position from a type-2 surface collision,
not a checkpoint save. Renamed to `lgp_x/y/z` (Last Grounded Position) June 2026.

## Related Fields (NOT part of respawn)

- **Ball+0xC4C** = is_shrunk (Odd Race only, set by Ball_Shrink/Ball_Grow)
- **Ball+0x281** = init_flag (WRITE-ONLY dead field — set to 1 by ctor, never read)
- **Ball+0xC60** = debris lifetime (5.0→0.0, counts down for debris fragments)
- **Ball+0x2EC** = fall counter (incremented by `INC dword [esi+0x2EC]` at 0x4075C9)
