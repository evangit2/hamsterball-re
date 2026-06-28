# Ball Ground Detection — Verified Offsets

## TL;DR

| Offset | Name | Reliable for ground check? | Notes |
|--------|------|---------------------------|-------|
| `Ball+0x281` | `unused_init_flag` | ❌ NO | DEAD: set by ctor, NEVER read by any function |
| `Ball+0xC4C` | `is_shrunk` | ⚠ Partial | Only set by Odd Race E:SHRINK/E:GROW, NOT ground contact |
| `Ball+0x2E9` | `dizzy_lock` | ❌ **BROKEN** | Sticky flag, never cleared in Ball_Update. **DO NOT USE for ground detection.** |
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

### Ball+0x2DC/0x2E0/0x2E4 — Last Grounded Position (LGP) ✅ VERIFIED

| Offset | Name | Type | Description |
|--------|------|------|-------------|
| `Ball+0x2DC` | `lgp_x` | float | X coordinate of last type-2 surface collision |
| `Ball+0x2E0` | `lgp_y` | float | Y coordinate of last type-2 surface collision |
| `Ball+0x2E4` | `lgp_z` | float | Z coordinate of last type-2 surface collision |

The LGP is the ball's position **snapshot at the moment of its last type-2 surface collision**.
It is written inside `Ball_Update` (0x405E00) whenever the ball's own collision mesh registers a
type-2 surface hit. Type-2 entries are created by `SpatialTree_TestFace` (0x463E20), which has
**no backface culling** — it uses an absolute distance check and a velocity gate (`dot <= 0`)
that passes for wall slides (dot ≈ 0) and clip-throughs while falling (dot ≈ 0). See
`docs/physics/BALL_RESPAWN_SYSTEM.md` for the full gate analysis.

```c
param_1[0xB7] = param_1[0x59];  // ball+0x2DC = ball+0x164 (current X → LGP X)
param_1[0xB8] = param_1[0x5A];  // ball+0x2E0 = ball+0x168 (current Y → LGP Y)
param_1[0xB9] = param_1[0x5B];  // ball+0x2E4 = ball+0x16C (current Z → LGP Z)
```

The respawn system (`Ball_FindClosestRespawnPoint` at 0x405190) uses the LGP — **not** the live
position — for all its distance and height calculations. This is so the ball respawns near where
it fell off the track, not wherever it landed in the void.

**For mods:** LGP is a reliable indicator of where the ball was last grounded. If you need a "last
safe position" for any custom respawn/teleport logic, read these fields. They are updated every
frame the ball is touching ground via type-2 collision, and remain stale (holding the last grounded
position) while the ball is airborne or falling.

**Previous labels:** These fields were previously named `checkpoint_x/y/z` and described as
"Last safe position" or "Last collision/bump position." The correct terminology is **Last Grounded
Position (LGP)** — they are specifically the position from the last type-2 ground collision, not
a checkpoint save or a generic bump position.

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
