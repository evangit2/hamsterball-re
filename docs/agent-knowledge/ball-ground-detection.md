# Ball Ground Detection — Verified Offsets

## TL;DR

| Offset | Name | Reliable for ground check? | Notes |
|--------|------|---------------------------|-------|
| `Ball+0x281` | `unused_init_flag` | ❌ NO | DEAD: set by ctor, NEVER read by any function |
| `Ball+0xC4C` | `is_shrunk` | ⚠ Partial | Only set by Odd Race E:SHRINK/E:GROW, NOT ground contact |
| `Ball+0x2E9` | `impact_shatter` | ❌ **BROKEN** | Sticky flag, never cleared in Ball_Update. **DO NOT USE.** |
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

`Ball+0x2E9` is a **limit/trajectory flag**:

- **Set to 1** by:
  - `E:LIMIT` arena events (finish line reached)
  - Type-5 floor collision with deep penetration (`piVar16[0x15] > _DAT_004cf310`)
- **Cleared to 0** by `Ball_FindClosestRespawnPoint` (at 0x00405262: `MOV byte [ESI+0x2E9],0`) and `Ball_ctor2`

### ⚠ CORRECTION: Previous "sticky flag" claim was wrong

Previous versions of this doc claimed `Ball_FindClosestRespawnPoint`'s clear was `int*` arithmetic writing to `+0xBA4` instead of `+0x2E9`, making the flag "sticky." **This was wrong.** The actual disassembly at 0x00405262 is:

```asm
00405262: C6 86 E9 02 00 00 00    MOV byte [ESI+0x2E9], 0
```

This is a direct byte write to offset `0x2E9` via the ModRM `9E` encoding (`[ESI+disp32]`). The Ghidra decompilation's `param_1 + 0x2e9` uses `param_1` declared as `int` (not `int*`), so the arithmetic IS byte offset `0x2E9`. The flag IS properly cleared on respawn.

However, the flag is still **not cleared per-frame within `Ball_Update`** — it persists between frames within a single life. It's only cleared on respawn/teleport, not every physics tick.

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

## ⚠ unused_init_flag (Ball+0x281) is DEAD CODE

- Documented as `unused_init_flag` (formerly is_falling) in BALL_OBJECT.md
- **NOT read during `Ball_Update` (0x405E00)** physics tick
- Legacy/init flag only
- Setting it to 0 in water mods works as a side effect (clears a stale state), but it does NOT reflect current ground contact

## is_shrunk (Ball+0xC4C) is Odd Race shrink mechanic only

- Set by `Ball_Shrink` (0x402200) during Odd Race E:SHRINK collision event
- Cleared by `Ball_Grow` (0x402270) during Odd Race E:GROW collision event
- **Does NOT track per-frame ground contact** — ball can be midair (from a jump) with is_shrunk=0
- NOT related to falling off edges — Ball_Shatter (0x408D70) handles falling and never writes to 0xC4C
