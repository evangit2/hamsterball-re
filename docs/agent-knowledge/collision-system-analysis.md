# Ball Collision System Analysis

## Overview

Hamsterball's ball-ball collision system is **asymmetric**: each collision
event generates an entry in only ONE ball's collision list, not both. When
ball A hits ball B, only one of them receives a collision entry for that
contact. This was confirmed by runtime dump data showing:

- **No two entries share the same tick** — if symmetric, paired entries would
  appear at the same frame (~33ms apart at 30fps). The smallest gap between
  any two entries is 453ms (~14 frames).
- **Unequal counts**: 4 PLAYER→8BALL entries vs 7 8BALL→PLAYER entries (1:1.8
  ratio). If symmetric, counts would be equal.
- **Each tick is a separate collision event** — not the same collision seen
  from two perspectives.

The 8-ball receives collision entries ~1.8x more often than the player. This
likely reflects which ball is the "moving" one in each contact — the ball whose
physics step detects the overlap creates the entry.

## Key Addresses

| Address | Name | Description |
|---------|------|-------------|
| 0x405E00 | `Ball_Update` | Per-frame physics tick for ALL balls (player ID=0 AND 8-balls ID=-1). Called via Ball vtable[4]. Scene_UpdateBallsAndState iterates all balls and calls this for each. |
| 0x408390 | `Ball_AI_ChaseNearest` | 8-ball AI function. NOT the 8-ball physics tick — 8-balls use the same 0x405E00 as the player. |

## Collision Entry Struct (0x80 bytes)

Each entry is 0x80 (128) bytes. Fields below are verified against the
decompiled `Ball_Update` at 0x405E00. "Verified" means the game reads or writes
the field at that offset; "Unread" means Ball_Update never accesses it.

### Confirmed Fields

| Offset | Type | Name | Evidence |
|--------|------|------|----------|
| +0x00 | int | **type** | `*piVar16 == 1` (ball-ball), `== 2` (wall), `== 5` (floor). Directly compared in Ball_Update lines 494, 502, 686, 794. |
| +0x04 | int | **sub_type / flags** | Always 4 for ball-ball, 1 for wall, 0 for floor. **Never read by Ball_Update** — the game ignores this field entirely. Filtering on `e[1] == 4` works by coincidence, not design. |
| +0x08 | float | **contact_age** (estimated) | Small float (0.01–0.11). Never read by Ball_Update. Likely set by collision detection system. |
| +0x0C | ptr | **other_ball** | `piVar16[3]` → then reads `piVar2[0x59]`, `piVar2[0x5a]`, `piVar2[0x5b]` (position), `piVar2[0xa1]` (radius), `piVar2[6]` (player_index). Verified at line 518. |
| +0x10 | ptr | **material_ptr** | Only for type==2 (wall). `*(int*)(piVar16[4] + 0x434)` reads material vtable. NULL for type==1. Verified at line 723. |
| +0x14 | float | **collision_point_x** | World-space X of contact point. Matches ball position at collision time. Not read by Ball_Update but populated by collision system. |
| +0x18 | float | **collision_point_y** | World-space Y (vertical). Typically near ball center height. |
| +0x1C | float | **collision_point_z** | World-space Z of contact point. |
| +0x20 | float | **normal_x** | Contact normal X component. Used in dot product with gravity (line 557) and wall reflection (line 689). |
| +0x24 | float | **normal_y** | Contact normal Y component. Same. |
| +0x28 | float | **normal_z** | Contact normal Z component. Same. |
| +0x2C | float | **contact_distance** (estimated) | ~0.14–0.80 range. Never read by Ball_Update. |
| +0x30 | float | **rel_vel_x** | Relative velocity X at contact point. Magnitude checked vs 0.003 threshold for collision sound (lines 560–562). |
| +0x34 | float | **rel_vel_y** | Relative velocity Y. Same. |
| +0x38 | float | **rel_vel_z** | Relative velocity Z. Same. |
| +0x3C–0x50 | — | **unused** | Always zero for ball-ball entries. Never read. |
| +0x54 | float | **penetration_depth** | Only for type==5 (floor). Compared against 0.1, 0.05, 0.003 thresholds (lines 795, 803, 818, 846). Always 0 for ball-ball. |
| +0x58 | float | **prev_point_x** | Previous-frame contact point X. Close to +0x14 values. Never read by Ball_Update. |
| +0x5C | float | **prev_point_y** | Previous-frame contact point Y. |
| +0x60 | float | **prev_point_z** | Previous-frame contact point Z. |
| +0x64 | ptr | **source_ref** | Compared against `unaff_EBP` and `unaff_ESI` (ball pointer in registers) at lines 494, 503, 517, 686. **NOT a ball pointer** — dump values (0x0F3C7FD8 etc.) don't match ball addresses. Likely a CollisionNode or internal tracking object. Acts as an ownership/processing filter: only the ball whose register matches `+0x64` processes the entry. |
| +0x68–0x7C | — | **metadata** | Contains pointers and flags. Pattern varies between player and 8-ball entries. Never read by Ball_Update. |

## Asymmetry Evidence

Hooking `Ball_Update` (0x405E00) for ALL balls and logging ball-ball entries
reveals that collisions are NOT paired — each event creates a single entry in
one ball's list only:

### All Recorded Entries (chronological)

```
tick 47852375  PLAYER -> 8-BALL   (14.4s gap to next)
tick 47866750  8-BALL -> PLAYER   (48.5s gap to next)
tick 47915281  8-BALL -> PLAYER   (42.8s gap to next)
tick 47958093  8-BALL -> PLAYER   (2.4s gap to next)
tick 47960468  8-BALL -> PLAYER   (2.4s gap to next)
tick 47962890  PLAYER -> 8-BALL   (1.4s gap to next)
tick 47964281  PLAYER -> 8-BALL   (2.0s gap to next)
tick 47966265  8-BALL -> PLAYER   (2.9s gap to next)
tick 47969125  8-BALL -> PLAYER   (0.6s gap to next)
tick 47969765  PLAYER -> 8-BALL   (0.5s gap to next)
tick 47970218  8-BALL -> PLAYER
```

### Key Observations

1. **No paired ticks**: Zero duplicate ticks. If symmetric, each collision would
   produce two entries at the same tick (or within ~33ms = one frame). The
   smallest gap is 453ms (~14 frames apart).

2. **Unequal distribution**: 4 PLAYER→8BALL vs 7 8BALL→PLAYER. If symmetric,
   counts would be 1:1. The 8-ball receives ~1.8x more collision entries.

3. **Each tick is a separate event**: The entries represent individual collision
   detections, not the same collision from two perspectives. The normals and
   contact points differ because the balls are at different positions across
   these separate events.

### Example: NOT a pair

```
[tick 47852375] PLAYER 0FD680C8 -> 8-BALL 0FD699E8
  +0x0C: 0x0FD699E8  (other = 8-ball)
  +0x14: 2144.48      (contact X)
  +0x20: 0.7018       (normal X)
  +0x64: 0x0F3C7FD8   (source_ref)

[tick 47866750] 8-BALL 0FD699E8 -> PLAYER 0FD680C8
  +0x0C: 0x0FD680C8  (other = player)
  +0x14: 2324.83      (contact X — 180 units away, different location!)
  +0x20: -0.2247      (normal X — not simply opposite)
  +0x64: 0x0F3C8118   (source_ref — different node)
```

These are **two separate collisions at two different locations** (2144 vs 2325
on X axis), 14 seconds apart — NOT the same contact seen from both sides.

## How Ball_Update Processes Collisions

The collision loop in `Ball_Update` (0x405E00) iterates entries at
`ball->collision_mesh + 0x424` with count at `+0x1C`:

### Phase 1: Trajectory/bounce check (lines 502–516)

```c
if (*piVar16 == 1) {                                    // type = ball-ball
    if ((undefined4 *)piVar16[0x19] == unaff_EBP) {     // +0x64 == THIS ball?
        // → trajectory bounce, sticky flag, camera focus
    }
}
```

### Phase 2: Push-apart / velocity exchange (lines 517–631)

```c
if ((*piVar16 == 1) && ((float)piVar16[0x19] == unaff_ESI)) {
    piVar2 = (int *)piVar16[3];                          // +0x0C = other ball
    // → compute relative position, push apart, exchange velocity
    // → award 500-point bonus if one ball is much smaller
}
```

Both `unaff_EBP` and `unaff_ESI` are "unaffected" registers that Ghidra lost
track of, but both were set to `param_1` (the current ball) early in the
function. Both phases check the SAME condition (`+0x64 == this ball`):
- Phase 1 handles trajectory/bounce for entries owned by this ball
- Phase 2 handles push-apart physics for entries owned by this ball

Because collisions are asymmetric, only one ball's list contains the entry for
any given contact. That ball performs BOTH the trajectory bounce AND the
push-apart/velocity exchange. The other ball never sees this collision.

## Collision List Location

| Offset from Ball | Offset from PhysicsObject | Field |
|------------------|---------------------------|-------|
| Ball + 0x69 (DWORD) | — | `collision_mesh_ptr` (pointer to PhysicsObject) |
| — | collision_mesh + 0x1C | `collision_count` (int) |
| — | collision_mesh + 0x424 | `collision_arr` (pointer to array of entry pointers) |
| — | collision_mesh + 0x20 | AthenaList index array (internal iteration) |

**Important**: `collision_mesh` is NOT the PhysicsObject itself. The Ball
struct stores `param_1[0x69]` which is a pointer to a collision mesh object.
The collision count and array are at offsets within that mesh object.

In the modding framework, `ecx_player->physics_object` maps to
`ball[0x69]`, and `physics_object->collision_count` / `collision_arr` map
to `+0x1C` / `+0x424` within that object.

## Stale Entry Problem

The AthenaList does not zero out entries when balls separate. Old collision
entries remain in the list with stale `+0x0C` pointers. These stale entries
have invalid `playerID` values at `other_ball + 0x18` (e.g., 30, 47, 99).

### Fix

Validate the other ball's `playerID` before processing:

```c
int other_id = *(int*)((char*)other_ball + 0x18);
if (other_id != 0 && other_id != -1) continue;  // skip stale
```

Only `0` (player) and `-1` (8-ball) are valid ball IDs in the game.

## Recommended Detection Pattern

Because collisions are asymmetric, a mod hooking only the player's Ball_Update
will MISS ~64% of collisions (only 4 of 11 entries were PLAYER→8BALL). To catch
all ball-ball contacts, hook Ball_Update for ALL balls and scan each ball's
collision list:

```c
void __fastcall Hooked_BallUpdate(Ball* ball, void* edx) {
    Original_BallUpdate(ball, edx);

    PhysicsObject* phys = ball->physics_object;
    int count = phys->collision_count;
    if (count <= 0) return;

    void** entries = (void**)phys->collision_arr;
    if (!entries) return;

    for (int i = 0; i < count; i++) {
        DWORD* e = (DWORD*)entries[i];
        if (!e || IsBadReadPtr(e, 0x80)) continue;
        if (e[0] != 1) continue;  // ball-ball only

        DWORD other_ptr = e[3];  // +0x0C
        if (other_ptr <= 0x10000) continue;
        if (IsBadReadPtr((void*)other_ptr, 0x20)) continue;

        int other_id = *(int*)((char*)other_ptr + 0x18);
        if (other_id != 0 && other_id != -1) continue;  // skip stale

        // Collision detected: ball hit other_ball
        for (HamsterballAPI* mod : g_Mods) {
            mod->onBallBump(ball, (Ball*)other_ptr);
        }
    }
}
```

### Catching Collisions From the Other Side

Since only one ball receives the entry per collision, you can also check the
OTHER ball's list. When ball A's list has an entry pointing to ball B, ball B
was also involved in this collision even though B's list has no entry for it.
For full coverage, fire `onBallBump` for both `ball` and `(Ball*)other_ptr`:

```c
// Fire for both balls involved — only one has the entry,
// but both are participants in the collision
mod->onBallBump(ball, (Ball*)other_ptr);
mod->onBallBump((Ball*)other_ptr, ball);
```

### Contact Deduplication

The collision list persists across frames while balls remain in contact,
so `onBallBump` fires every frame (~30fps) for the same pair. For one-shot
detection, track currently-touching pointers:

```c
static DWORD touching[16] = {0};

// At start of each frame for this ball:
//   1. Scan collision list, build new-touching set
//   2. Fire onBallBump only for NEW contacts
//   3. Update touching[] array

// Alternative: clear touching[] when collision_count drops to 0
```

## vtable[4] Clarification

Previous analysis incorrectly identified:
- `0x405E00` as player-only → **WRONG**: runs for ALL balls
- `0x408390` as 8-ball vtable[4] → **WRONG**: it's a separate AI function
  (`Ball_AI_ChaseNearest`), not the 8-ball's physics tick

Confirmed via `printf("player type: %d\n", ball->playerID)` in the hook,
which outputs alternating `0, -1, 0, -1, ...` — proving `0x405E00` runs
for both player and 8-balls every frame.

## Field Summary: What Ball_Update Actually Reads

| Field | Read by Ball_Update? | Purpose |
|-------|---------------------|---------|
| +0x00 (type) | ✅ Yes | Dispatch: 1=ball-ball, 2=wall, 5=floor |
| +0x04 | ❌ No | Sub-type/flags — ignored by physics |
| +0x08 | ❌ No | Contact age — ignored by physics |
| +0x0C (other_ball) | ✅ Yes | Reads position, radius, player_index from other ball |
| +0x10 (material) | ✅ Yes (type 2 only) | Reads vtable at material+0x434 |
| +0x14–0x1C (point) | ❌ No | Contact point — set by collision system, not read |
| +0x20–0x28 (normal) | ✅ Yes | Dot product with gravity, wall reflection |
| +0x2C | ❌ No | Contact distance — ignored |
| +0x30–0x38 (rel_vel) | ✅ Yes | Magnitude check for collision sound |
| +0x3C–0x50 | ❌ No | Always zero — padding/reserved |
| +0x54 (penetration) | ✅ Yes (type 5 only) | Floor collision depth thresholds |
| +0x58–0x60 (prev_point) | ❌ No | Previous contact point — ignored |
| +0x64 (source_ref) | ✅ Yes | Ownership filter: only ball matching +0x64 processes entry |
| +0x68–0x7C | ❌ No | Metadata — ignored |

---

*Document created from decompiled Ball_Update (0x405E00) analysis and
runtime collision dumps. All offsets verified against Ghidra decompilation
output. Dump data captured via MinGW-compiled bass.dll proxy hook.
Asymmetry confirmed by tick analysis: zero paired entries, unequal counts
(4P vs 7B), all gaps >14 frames.*
