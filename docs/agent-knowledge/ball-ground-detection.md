# Ball Ground Detection — Verified Offsets

## TL;DR

| Offset | Name | Reliable for ground check? | Notes |
|--------|------|---------------------------|-------|
| `Ball+0x281` | `is_falling` | ❌ NO | Legacy/init flag, NOT read during Ball_Update physics tick |
| `Ball+0xC4C` | `fall_mode` | ⚠ Partial | Only tracks death/respawn (fall-off-level), NOT ground contact |
| `Ball+0x2E9` | `limit_flag` | ❌ **BROKEN** | Sticky flag, never cleared in Ball_Update. **DO NOT USE.** |
| `Ball+0x260` | `is_airborne` | ⚠ Partial | Set by speed/friction thresholds, not true ground contact |

**For jump mods:** Use a cooldown timer (60 frames). No ground flag needed.
**For mods needing true ground contact:** Use the engine's raycast (`Mesh_FindClosestCollision`) from a background thread.

---

## ⚠ CRITICAL: Ball+0x2E9 is NOT a ground-contact flag

Previous versions of multiple docs (BALL_OBJECT.md, BALL_UPDATE_DECOMP.md, etc.) labeled `Ball+0x2E9` as:
- `on_ramp` (slope/ramp flag)
- `on_surface` (ground-contact flag)
- `flag2` (reset to 0 each frame)
- `is_teleporting`

**All of these labels are WRONG.**

### What 0x2E9 actually is

`Ball+0x2E9` is a **sticky limit/trajectory flag**:

- **Set to 1** by:
  - `E:LIMIT` arena events (finish line reached)
  - Type-5 floor collision with deep penetration (`piVar16[0x15] > _DAT_004cf310`)
- **NEVER cleared within `Ball_Update` (0x405E00)**

### Why the "cleared each frame" myth exists

The clears in `Ball_FindClosestRespawnPoint` (0x405190) and decompiled `Ball_Update` appear as:
```c
*(undefined1 *)(param_1 + 0x2e9) = 0;
```

But `param_1` is declared as `int*` (int pointer). `int* + 0x2e9` = **byte offset `0x2e9 * 4 = 0xBA4`**, NOT byte offset `0x2E9`.

The actual byte at `Ball+0x2E9` is ONLY cleared by `Ball_ctor2` (full constructor, called on respawn). The `Ball_Update` code that writes to it:
```c
// In decomp_0x405E00_Ball_Update.c, line 804:
*(undefined1 *)((int)param_1 + 0x2e9) = 1;  // SET: uses (int) cast = byte offset 0x2E9
```

Note the `(int)` cast — this IS a byte offset. But the clears do NOT use this cast:
```c
// In decomp_ball_update.c, line 81:
*(undefined1 *)(param_1 + 0x2e9) = 0;  // CLEAR: uses int* arithmetic = byte offset 0xBA4
```

**This makes `0x2E9` a sticky flag** — once set to 1, it stays 1 until the ball is fully reconstructed by `Ball_ctor2`.

### Real-world bug caused by this

The jump_mod used `Ball+0x2E9` as a ground check, reading it at the end of `Ball_Update`. Symptoms:
- **Can't jump on flat ground** (flag hasn't been set yet from a deep collision)
- **CAN jump midair** (flag is stuck at 1 from a previous frame's deep collision)

Fix: replaced with a 60-frame cooldown timer after each jump.

---

## What DOES work for ground detection

### Option 1: Cooldown timer (simplest, for jump mods)

After applying jump velocity, set a 60-frame counter. While >0, block jumping.
- Jump velocity 500, gravity ~15/frame
- Apex at ~33 frames, landing at ~66 frames
- 60-frame cooldown covers most of the arc
- **Pros:** Simple, no external calls, works on ramps/slopes
- **Cons:** Ball can technically double-jump in the last ~6 frames before landing (frame 60-66). Can increase to 80 to be safe.

### Option 2: Engine raycast (for true ground contact)

Call `Mesh_FindClosestCollision` from a **background thread** (NOT from a code cave):

```c
// Ball_FindMeshCollision @ 0x00403980
// thiscall: this=Ball*, param_1=Vec3* out
// Reads ball's CollisionMesh (ball+0x1A4), uses ball position + gravity
// direction (CollisionMesh+0xC8C), writes closest hit point to out.

typedef void (__attribute__((thiscall)) *BallFindMeshCollision_t)(
    void* ball, Vec3* out);

// Call from background thread:
Vec3 out = {0};
find_collision(ball_ptr, &out);
// Compare out to ball position — if close, ball is grounded
```

**⚠ DO NOT call from a code cave** — calling C functions from hand-assembled code caves inside `Ball_Update` corrupts stack/FPU/SEH state and crashes the game. Use Pattern 4 (volatile flag + polling thread) from the modding skill.

### Key offsets for raycast

| Offset | Name | Type | Purpose |
|--------|------|------|---------|
| `Ball+0x164` | position | Vec3 | Current ball position (X/Y/Z) |
| `Ball+0x1A4` | collision_mesh | ptr | CollisionMesh pointer |
| `Ball+0x284` | radius | float | Ball radius (~35.0) |
| `CollisionMesh+0xC8C` | gravity_dir | Vec3 | Gravity direction (normalized) |
| `Scene+0x8B0` | collision_level | ptr | Level CollisionMesh (for Mesh_FindClosestCollision) |

---

## ⚠ is_falling (Ball+0x281) is also NOT reliable

- Documented as `is_falling` in BALL_OBJECT.md
- **NOT read during `Ball_Update` (0x405E00)** physics tick
- Legacy/init flag only
- Setting it to 0 in water mods works as a side effect (clears a stale state), but it does NOT reflect current ground contact

## fall_mode (Ball+0xC4C) is death-state only

- Set by `Ball_Shrink` (0x402200) when ball falls off the level edge
- Cleared by `Ball_Grow` (0x402270) after respawn animation
- **Does NOT track per-frame ground contact** — ball can be midair (from a jump) with fall_mode=0
- Safe to use as a **secondary gate** (don't allow jumping while dying), but NOT as a primary ground check
