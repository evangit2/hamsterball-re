# Ball Collision System Analysis

## Overview

Hamsterball's collision system is **symmetric**: each ball maintains its own
collision list and creates its own entries for every contact. When ball A hits
ball B, both A's and B's collision lists receive separate entries. This was
confirmed by hooking `Ball_Update` (0x405E00) — which runs for ALL balls, not
just the player — and observing paired entries from both perspectives.

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
| +0x64 | ptr | **source_ref** | Compared against `unaff_EBP` and `unaff_ESI` (ball pointer in registers) at lines 494, 503, 517, 686. **NOT a ball pointer** — dump values (0x0F3C7FD8 etc.) don't match ball addresses. Likely a CollisionNode or internal tracking object. Determines which ball "owns" the entry. |
| +0x68–0x7C | — | **metadata** | Contains pointers and flags. Pattern varies between player and 8-ball entries. Never read by Ball_Update. |

## Symmetry Evidence

Hooking `Ball_Update` (0x405E00) for ALL balls and logging ball-ball entries
reveals paired collision events from both perspectives:

### Example Pair 1

```
[tick 47852375] PLAYER 0FD680C8 -> 8-BALL 0FD699E8
  +0x0C: 0x0FD699E8  (other ball = 8-ball)
  +0x20: 0x3F33A673  [0.7018]   (normal X)
  +0x24: 0xBE14D0E1  [-0.1453]  (normal Y)
  +0x28: 0x3F328B10  [0.6974]   (normal Z)
  +0x64: 0x0F3C7FD8             (source_ref — player's node)

[tick 47866750] 8-BALL 0FD699E8 -> PLAYER 0FD680C8
  +0x0C: 0x0FD680C8  (other ball = player)
  +0x20: 0xBE660BC8  [-0.2247]  (normal X — roughly opposite)
  +0x24: 0x3E17EEBD  [0.1484]   (normal Y — roughly opposite)
  +0x28: 0xBF768C2F  [-0.9631]  (normal Z — roughly opposite)
  +0x64: 0x0F3C8118             (source_ref — 8-ball's node, DIFFERENT)
```

### Example Pair 2

```
[tick 47962890] PLAYER 0FD680C8 -> 8-BALL 0FD699E8
  +0x20: 0xBF41E475  [-0.7574]  (normal)
  +0x64: 0x0F3C7FF8             (player's node)

[tick 47966265] 8-BALL 0FD699E8 -> PLAYER 0FD680C8
  +0x20: 0xBE5C5AD9  [-0.2152]  (normal — different but same collision)
  +0x64: 0x0F3C7DD8             (8-ball's node, DIFFERENT)
```

### Key Observations

1. **Separate entries**: `+0x64` (source_ref) differs between paired entries,
   proving they are distinct CollisionNode objects in separate lists.

2. **Opposite normals**: The contact normals are roughly opposite (dot product
   ~-0.47 to -0.85, not exactly -1.0 because the balls are at different
   positions when their respective Ball_Update runs).

3. **Different timestamps**: Paired entries often have slightly different tick
   values because Ball_Update runs sequentially for each ball within the same
   frame. The tick difference (~1–14ms) represents the time between each ball's
   physics tick within the frame.

4. **Both balls process the collision**: The game code at lines 502–631
   (trajectory/bounce + push apart + exchange velocity) runs for both the
   player and the 8-ball when they process their respective entries.

## How Ball_Update Processes Collisions

The collision loop in `Ball_Update` (0x405E00) iterates entries at
`ball->collision_mesh + 0x424` with count at `+0x1C`:

### Phase 1: Self-entry check (lines 502–516)

```c
if (*piVar16 == 1) {                                    // type = ball-ball
    if ((undefined4 *)piVar16[0x19] == unaff_EBP) {     // +0x64 == THIS ball?
        // → trajectory bounce, sticky flag, camera focus
    }
}
```

### Phase 2: Other-ball entry check (lines 517–631)

```c
if ((*piVar16 == 1) && ((float)piVar16[0x19] == unaff_ESI)) {
    piVar2 = (int *)piVar16[3];                          // +0x0C = other ball
    // → compute relative position, push apart, exchange velocity
    // → award 500-point bonus if one ball is much smaller
}
```

Both `unaff_EBP` and `unaff_ESI` are "unaffected" registers that Ghidra lost
track of, but both were set to `param_1` (the current ball) early in the
function. The two checks fire for different entries in the same list:

- Phase 1 fires when `+0x64` matches the current ball (its own entry)
- Phase 2 fires when `+0x64` matches the current ball (also its own entry)

**Correction**: Both phases check the SAME condition (`+0x64 == this ball`).
Phase 1 handles trajectory/bounce, Phase 2 handles push-apart physics. They
fire for the same entry, not different entries. The "other ball" entry (where
`+0x64` does NOT match) is simply skipped by both checks.

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

For mods that need to detect ball-ball collisions (e.g., player bumping an
8-ball), hook `Ball_Update` (0x405E00) and scan the collision list after the
original function returns:

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
| +0x64 (source_ref) | ✅ Yes | Ownership check: "is this MY entry?" |
| +0x68–0x7C | ❌ No | Metadata — ignored |

---

*Document created from decompiled Ball_Update (0x405E00) analysis and
runtime collision dumps. All offsets verified against Ghidra decompilation
output. Dump data captured via MinGW-compiled bass.dll proxy hook.*
