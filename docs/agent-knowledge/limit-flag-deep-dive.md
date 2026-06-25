# Deep-Dive: Ball+0x2E9 (impact_shatter)

## Overview

`Ball+0x2E9` is a **sticky limit/trajectory flag** that, once set, permanently alters ball collision behavior until the ball is fully reconstructed. It is NOT an "on_ramp" flag, NOT a ground-contact flag, and NOT cleared per-frame.

## Naming History

Previous documentation labeled this field as `on_ramp`, `on_surface`, `flag2`, or `is_teleporting`. **All of these labels are wrong.** The correct name is `impact_shatter` because it limits trajectory application and can trigger ball shatter.

## Initialization

| Function | Address | Action | Byte Offset |
|----------|---------|--------|-------------|
| `Ball_ctor2` | 0x004039E0 | `*(byte*)((int)this + 0x2E9) = 0` | 0x2E9 (correct, uses `(int)` cast) |

## Setting

**Only set in `Ball_Update` (0x00405E00), at L798:**

```c
// Inside collision type 5 (floor) handler:
if (piVar16[0x15] > 1.0  &&  is_shrunk == 0) {
    *(byte*)((int)param_1 + 0x2E9) = 1;   // SET (uses (int) cast = byte offset 0x2E9)
    Scene_SetCamera(param_1[5], param_1, 1);
    Graphics_SetViewport(..., param_1[0x59], param_1[0x5A]);
    // Viewport bounds check → may set swallow flag (param_1[0xBA] = 1)
}
```

**Trigger conditions:**
1. Ball is colliding with a type-5 (floor) surface
2. Surface speed (`piVar16[0x15]`, collision_obj+0x54) > 1.0
3. `is_shrunk` (ball+0xC4C) == 0 (ball is NOT in shrunk state)

When these conditions are met, the flag is set AND:
- `Scene_SetCamera` is called (camera follows ball)
- `Graphics_SetViewport` sets up the viewport around the ball position
- A viewport bounds check may set the swallow flag (`ball+0xBA = 1`), which triggers respawn

## Clearing

`Ball_FindClosestRespawnPoint` (0x00405190) clears the flag at address 0x00405262:

```asm
00405262: C6 86 E9 02 00 00 00    MOV byte [ESI+0x2E9], 0
```

The Ghidra decompilation shows `*(undefined1 *)(param_1 + 0x2e9) = 0` without an `(int)` cast. Previous documentation (ball-ground-detection.md) claimed this was `int*` arithmetic writing to `+0xBA4` instead of `+0x2E9`. **This was wrong.** The actual disassembly confirms it is `MOV byte [ESI+0x2E9], 0` — a direct byte write to offset `0x2E9` via the ModRM `9E` encoding (`[ESI+disp32]`). The flag IS properly cleared on respawn.

`Ball_ctor2` (0x004039E0) also initializes it to 0:

```asm
; At ctor2+0x1FE: 88 9E E9 02 00 00 = MOV [ESI+0x2E9], BL  (BL pre-loaded with 0)
```

**Result: `impact_shatter` is cleared both on respawn (`Ball_FindClosestRespawnPoint`) and on full reconstruction (`Ball_ctor2`).** It is NOT sticky — the previous "sticky flag" claim was based on an incorrect `int*` arithmetic assumption that the disassembly disproves.

## Effects When Set (impact_shatter = 1)

### 1. Skip Ball_ApplyTrajectory (L498)

```c
if (piVar16[0] == 1) {   // type 1 = surface/wall collision
    if (piVar16[0x19] == unaff_EBP) {   // belongs to this board
        if (param_1[0xBB] > 1 && impact_shatter == 0) {
            Ball_ApplyTrajectory(param_1);   // BOUNCE/REFLECT
        }
```

When `impact_shatter = 1`: `Ball_ApplyTrajectory` is **skipped**. The ball does not bounce off walls. This means the ball slides along surfaces instead of reflecting.

### 2. Trigger Ball_Shatter on Wall Hit (L502)

```c
if (impact_shatter == 1) {
    if (velocity_check_based_on_axis) {
        (*vtable[8])();   // calls Ball_OnRampEvent = Ball_Shatter!
    }
}
```

The velocity check depends on `ball+0x1D2` (axis selector):
- **Axis 0 (Y):** `board+0xCA8 >= 0.0` (ball moving up or stationary)
- **Axis 1 (X):** `board+0xCA4 < 0.0` (ball moving in negative X)
- **Axis 2 (Z):** `board+0xCAC >= 0.0` (ball moving in positive Z or stationary)

If the ball is moving in the "wrong" direction for the current axis AND hits a wall, the ball **shatters** (breaks into debris pieces, plays sound, triggers respawn).

**`Ball_OnRampEvent` (0x00409480, vtable[8]):**
- Sets `ball+0x2E8 = 1` (shattered flag)
- Checks `is_shrunk` (0xC4C) to select which shatter sound to play
- Creates 3 `Ball_Split_ctor` debris pieces at the ball's position
- Copies velocity, position, radius to each debris piece
- Creates `RumbleScore` objects (score popup particles)
- This is essentially the same as `Ball_Shatter` (0x00408D70) but called from a different path

### 3. Dead Code: Position-Match Shatter (L642)

```c
if (impact_shatter != 0 && param_1[0xC9] == 0) {
    if (ABS(ball_pos - ramp_entry_pos) < _DAT_004cf4f8) {   // threshold = 0.0
        (*vtable[8])();   // Ball_Shatter
    }
}
```

`_DAT_004cf4f8 = 0.0`, and `ABS(x) < 0.0` is **always false** for any float. This code path **never executes**.

## Interaction with is_shrunk (0xC4C)

The `impact_shatter` is only SET when `is_shrunk == 0`. If the ball is in shrunk state (our half-size mod), `impact_shatter` cannot be set by type-5 floor collisions. This means:

- Shrunk balls never get their trajectory limited
- Shrunk balls never trigger the shatter-on-wall-hit behavior
- The camera/viewport setup at L798 is also skipped for shrunk balls

This is likely intentional game design: in Odd Race, when the ball is shrunk inside the shrink pipes, it shouldn't shatter from floor contact.

## Summary Table

| Aspect | Detail |
|--------|--------|
| **Field** | `Ball+0x2E9` (byte) |
| **Name** | `impact_shatter` |
| **Init** | 0 (by `Ball_ctor2`) |
| **Set by** | Type-5 floor collision, speed > 1.0, `is_shrunk == 0` |
| **Cleared by** | `Ball_FindClosestRespawnPoint` (0x00405262: `MOV byte [ESI+0x2E9],0`) and `Ball_ctor2` (0x004039E0+0x1FE: `MOV [ESI+0x2E9],BL`) |
| **Effect 1** | Skip `Ball_ApplyTrajectory` — no wall bouncing |
| **Effect 2** | Trigger `Ball_Shatter` on wall hit if moving in "wrong" direction |
| **Effect 3** | Dead code: position-match shatter (threshold = 0.0, never triggers) |
| **vtable[8]** | `Ball_OnRampEvent` (0x00409480) — shatter function called from this path |
| **Interaction with `is_shrunk`** | `is_shrunk=1` prevents `impact_shatter` from being set |
