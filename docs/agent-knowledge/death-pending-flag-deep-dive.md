# Deep-Dive: Ball+0x2E9 (death_pending)

## Overview

`Ball+0x2E9` is a **death-pending flag** that marks the ball as being in a terminal state. Once set, the ball cannot go dizzy again (Ball_ApplyTrajectory is blocked), and hitting a wall while moving the wrong direction triggers Ball_FallDeath (shatter). It is only cleared by respawn (Ball_InitPhysicsDefaults) or full reconstruction (Ball_ctor2) — there is no mid-game recovery path.

## Naming History

Previous documentation labeled this field as `on_ramp`, `on_surface`, `flag2`, `is_teleporting`, `impact_shatter`, or `dizzy_lock`. **All of these labels are wrong or misleading.** The name `dizzy_lock` was backwards — it sounds like "prevents dizziness" when really it means "the ball has been marked for death (via dizzy or limit boundary) and is now in a terminal state." The correct name is `death_pending` because the ball is awaiting death.

## Initialization

| Function | Address | Action | Byte Offset |
|----------|---------|--------|-------------|
| `Ball_ctor2` | 0x004039E0 | `*(byte*)((int)this + 0x2E9) = 0` | 0x2E9 (correct, uses `(int)` cast) |

## Setting

**Set in `Ball_Update` (0x00405E00), at L798 (collision type 5, speed > 1.0):**

```c
// Inside collision type 5 (floor) handler:
if (piVar16[0x15] > 1.0  &&  is_shrunk == 0) {
    *(byte*)((int)param_1 + 0x2E9) = 1;   // SET (uses (int) cast = byte offset 0x2E9)
    Scene_SetCamera(param_1[5], param_1, 1);
    Graphics_SetViewport(..., param_1[0x59], param_1[0x5A]);
    // Viewport bounds check → may set swallow flag (param_1[0xBA] = 1)
}
```

**Also set by the following events in `DispatchCollisionEvents` (0x0040C5D0):**
- `E:LIMIT` — sets `ball+0x2E9 = 1`
- `E:LIMITX` — sets `ball+0x2E9 = 1`
- `E:LIMITZ` — sets `ball+0x2E9 = 1`
- `E:LIMITPIPE1` — sets `ball+0x2E9 = 1`
- `E:LIMITPIPE2` — sets `ball+0x2E9 = 1`
- `E:SWALLOW` — sets `ball+0x2E8 = 1` (is_falling/shattered flag)

**Also set by `Ball_ApplyTrajectory` (0x00403750) itself:** when the dizzy effect fires, it sets `death_pending=1` to prevent itself from re-firing on subsequent frames.

**Trigger conditions (Ball_Update L798):**
1. Ball is colliding with a type-5 (floor) surface
2. Surface speed (`piVar16[0x15]`, collision_obj+0x54) > 1.0
3. `is_shrunk` (ball+0xC4C) == 0 (ball is NOT in shrunk state)

When these conditions are met, the flag is set AND:
- `Scene_SetCamera` is called (camera follows ball)
- `Graphics_SetViewport` sets up the viewport around the ball position
- A viewport bounds check may set the swallow flag (`ball+0xBA = 1`), which triggers respawn

## Clearing

`Ball_InitPhysicsDefaults` (0x00405100) clears the flag at address 0x00405262:

```asm
00405262: C6 86 E9 02 00 00 00    MOV byte [ESI+0x2E9], 0
```

The Ghidra decompilation shows `*(undefined1 *)(param_1 + 0x2e9) = 0` without an `(int)` cast. Previous documentation (ball-ground-detection.md) claimed this was `int*` arithmetic writing to `+0xBA4` instead of `+0x2E9`. **This was wrong.** The actual disassembly confirms it is `MOV byte [ESI+0x2E9], 0` — a direct byte write to offset `0x2E9` via the ModRM `9E` encoding (`[ESI+disp32]`). The flag IS properly cleared on respawn.

`Ball_ctor2` (0x004039E0) also initializes it to 0:

```asm
; At ctor2+0x1FE: 88 9E E9 02 00 00 = MOV [ESI+0x2E9], BL  (BL pre-loaded with 0)
```

**Result: `death_pending` is cleared both on respawn (`Ball_InitPhysicsDefaults`) and on full reconstruction (`Ball_ctor2`).** There is no mid-game recovery path — once set, the ball is in a terminal state until it dies and respawns.

## Effects When Set (death_pending = 1)

### 1. Skip Ball_ApplyTrajectory (L498)

```c
if (piVar16[0] == 1) {   // type 1 = surface/wall collision
    if (piVar16[0x19] == unaff_EBP) {   // belongs to this board
        if (param_1[0xBB] > 1 && death_pending == 0) {
            Ball_ApplyTrajectory(param_1);   // BOUNCE/REFLECT
        }
```

When `death_pending = 1`: `Ball_ApplyTrajectory` is **skipped**. The ball cannot go dizzy again — it's already in the death pipeline.

### 2. Trigger Ball_Shatter on Wall Hit (L502)

```c
if (death_pending == 1) {
    if (velocity_check_based_on_axis) {
        (*vtable[8])();   // calls Ball_FallDeath = Ball_Shatter!
    }
}
```

The velocity check depends on `ball+0x1D2` (axis selector):
- **Axis 0 (Y):** `board+0xCA8 >= 0.0` (ball moving up or stationary)
- **Axis 1 (X):** `board+0xCA4 < 0.0` (ball moving in negative X)
- **Axis 2 (Z):** `board+0xCAC >= 0.0` (ball moving in positive Z or stationary)

If the ball is moving in the "wrong" direction for the current axis AND hits a wall, the ball **shatters** (breaks into debris pieces, plays sound, triggers respawn).

**`Ball_FallDeath` (0x00409480, vtable[8]):**
- This is the death/respawn handler called when the player ball STOPS after falling (velocity reverses or ball lands). NOT a "ramp variant" of Ball_Shatter.
- Sets `ball+0x2E8 = 1` (shattered/needs_respawn flag)
- Checks `is_shrunk` (0xC4C) to select which shatter sound to play
- Creates 3 `Ball_Split_ctor` debris pieces at the ball's position
- Copies velocity, position, radius to each debris piece
- Creates `ArenaScoreParticle` objects (score popup particles)
- Note: Ball_Shatter (0x00408D70) at 0x408CD5 also sets +0x2E8=1, but ONLY for balls hit by Breaker/Bonkbash crusher objects (arena/level objects), NOT for player fall death

### 3. Dead Code: Position-Match Shatter (L642)

```c
if (death_pending != 0 && param_1[0xC9] == 0) {
    if (ABS(ball_pos - ramp_entry_pos) < _DAT_004cf4f8) {   // threshold = 0.0
        (*vtable[8])();   // Ball_Shatter
    }
}
```

`_DAT_004cf4f8 = 0.0`, and `ABS(x) < 0.0` is **always false** for any float. This code path **never executes**.

## Interaction with is_shrunk (0xC4C)

The `death_pending` is only SET when `is_shrunk == 0`. If the ball is in shrunk state (our half-size mod), `death_pending` cannot be set by type-5 floor collisions. This means:

- Shrunk balls never get their trajectory limited
- Shrunk balls never trigger the shatter-on-wall-hit behavior
- The camera/viewport setup at L798 is also skipped for shrunk balls

This is likely intentional game design: in Odd Race, when the ball is shrunk inside the shrink pipes, it shouldn't shatter from floor contact.

## Summary Table

| Aspect | Detail |
|--------|--------|
| **Field** | `Ball+0x2E9` (byte) |
| **Name** | `death_pending` |
| **Init** | 0 (by `Ball_ctor2` at 0x4039E0+0x1FE) |
| **Set by** | Ball_ApplyTrajectory (0x403750), speed>1.0 collision (0x407391), E:LIMIT (0x40C767), E:LIMITX, E:LIMITZ, E:LIMITPIPE1, E:LIMITPIPE2 |
| **Cleared by** | `Ball_InitPhysicsDefaults` (0x00405262: `MOV byte [ESI+0x2E9],0`) and `Ball_ctor2` (0x4039E0+0x1FE) — only on respawn or full reconstruction |
| **Effect 1** | Blocks `Ball_ApplyTrajectory` from firing (Pass 1 checks `death_pending == 0`) — ball cannot go dizzy again |
| **Effect 2** | When set by speed>1.0: camera change + viewport setup |
| **Effect 3** | Enables wall-hit shatter path (vtable[8] = Ball_FallDeath). Ball is in terminal state — hitting a wall while moving wrong direction = death |
| **Effect 4** | Dead code: position-match shatter (threshold = 0.0, never triggers) |
| **Recovery** | NONE — no mid-game clear. Ball must die and respawn to clear this flag |
| **Interaction with `is_shrunk`** | `is_shrunk=1` prevents `death_pending` from being set by speed>1.0 collision |
| **Related fields** | `ball+0x2EC` (bounce_count), `ball+0x14D` (has_trajectory), `ball+0x2F0` (impact_count) |
| **End screen** | When set by Ball_ApplyTrajectory: increments `App+pIdx×0xA0+0x5F8` (dizzy counter) |
| **Full analysis** | See [dizzy-stun-system.md](dizzy-stun-system.md) for the complete dizzy system including two-pass collision architecture |
