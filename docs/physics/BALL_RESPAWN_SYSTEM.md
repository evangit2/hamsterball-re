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

## Last Grounded Position (LGP) — Respawn Search Position Source

The respawn point search does **NOT** use the ball's current live position (`ball+0x164/0x168/0x16C`).
Instead, it uses the **Last Grounded Position (LGP)** stored at:

| Offset | Name | Type | Description |
|--------|------|------|-------------|
| `Ball+0x2DC` | `lgp_x` | float | X coordinate of last ground collision |
| `Ball+0x2E0` | `lgp_y` | float | Y coordinate of last ground collision |
| `Ball+0x2E4` | `lgp_z` | float | Z coordinate of last ground collision |

The LGP is a **snapshot of the ball's position at the moment of its last type-2 (ground) collision**.
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

**Note:** Type-2 collision = ground/surface collision. This is the collision type where the ball's
own collision mesh (`piVar16[0x19] == unaff_EBP`) hits a walkable surface. The previous labels
`checkpoint_x/y/z` and "Last safe position" were partially correct but misleading — these are
specifically the last grounded position, not a checkpoint save.

## Related Fields (NOT part of respawn)

- **Ball+0xC4C** = is_shrunk (Odd Race only, set by Ball_Shrink/Ball_Grow)
- **Ball+0x281** = init_flag (WRITE-ONLY dead field — set to 1 by ctor, never read)
- **Ball+0xC60** = debris lifetime (5.0→0.0, counts down for debris fragments)
- **Ball+0x2EC** = fall counter (incremented by `INC dword [esi+0x2EC]` at 0x4075C9)
