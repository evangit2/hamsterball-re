# N:TARPIT — Tar Pit Collision Event (Verified June 2026)

## Summary

N:TARPIT is a collision event handled by `DispatchCollisionEvents` (0x40C5D0), the shared base collision dispatcher. It freezes the ball in place, plays a splash sound, blocks goal completion, and eventually triggers the ball's shatter/fall sequence after ~2 seconds. The effect is permanent — there is no cleanup timer or escape mechanic.

## String Location

- `"N:TARPIT"` at 0x4CF82C (ASCII, null-terminated)
- Referenced only from `DispatchCollisionEvents` at 0x40CC20

## Dispatcher Architecture

**ALL 30 boards** have custom `vtable[0x1D]` (+0x74) collision handlers. **WarmUp Race** is the only board that uses `DispatchCollisionEvents` directly as its vtable entry. Every other board has a custom handler that:

1. Processes board-specific events first (e.g., N:SWIRL on Dizzy, E:CATAPULTBOTTOM on Tower)
2. Falls through to `DispatchCollisionEvents` (0x40C5D0) as the last call

**This means N:TARPIT fires on ALL boards** — the base dispatcher is always reached via fallthrough.

Full 30-board mapping: [COLLISION_HANDLER_COMPLETE_MAPPING.md](./COLLISION_HANDLER_COMPLETE_MAPPING.md)

## Handler Code

### DispatchCollisionEvents (0x40C5D0) — N:TARPIT block

Decompiled C (Ghidra):

```c
iVar3 = __stricmp(*(char **)(param_2[1] + 0x864), "N:TARPIT");
if (iVar3 == 0) {
    // Only on first contact (tar flag not already set)
    if ((char)param_1[0xB3] == '\0') {   // ball+0x2CC == 0
        // Save entry position for sound
        fStack_38 = (float)param_1[0x5A];    // ball+0x168 (pos Y)
        local_3c = (float)param_1[0x59];     // ball+0x164 (pos X)
        param_1[0xB4] = param_1[0x5A];       // ball+0x2D0 = entry Y pos
        fStack_34 = (float)param_1[0x5B];    // ball+0x16C (pos Z)
        // Play 3D splash/squish sound
        Sound_Play3D(
            *(void **)(*(int *)((int)this + 0x878) + 0x484),  // board→scene→sound_ptr
            local_3c,     // X
            fStack_38,    // Y
            fStack_34     // Z
        );
    }
    // SET TAR FLAG (permanent, no timer)
    *(undefined1 *)(param_1 + 0xB3) = 1;   // ball+0x2CC = 1 (tar active)
    // CLEAR GROUNDED FLAG
    *(undefined1 *)(param_1 + 0x1DA) = 0;  // ball+0x768 = 0 (not grounded)
}
```

### Ghidra Index → Byte Offset Conversion

Ghidra decompiles struct access as `param_1[N]` where param_1 is `int*` (4-byte stride).
To convert: `byte_offset = N × 4`.

| Ghidra Index | Byte Offset | Field | Type | Description |
|-------------|-------------|-------|------|-------------|
| param_1[0xB3] | ball+0x2CC | tar_flag | byte | 0 = normal, 1 = tarred (permanent) |
| param_1[0xB4] | ball+0x2D0 | tar_entry_y | float | Y position when tar triggered |
| param_1[0x1DA] | ball+0x768 | grounded | byte | 0 = airborne/ungrounded, cleared by tar |
| param_1[0x59] | ball+0x164 | pos_x | float | Ball position X |
| param_1[0x5A] | ball+0x168 | pos_y | float | Ball position Y (vertical) |
| param_1[0x5B] | ball+0x16C | pos_z | float | Ball position Z |

## Per-Frame Physics Effects

### Ball_FallUpdate (0x408830)

The tar flag overrides the physics body's computed position with the ball's own stored position, effectively freezing movement:

```c
// Normal: get position from physics integration (includes velocity/movement)
pos = physics->vtable[1]();   // computed position with movement applied

// TAR OVERRIDE: use ball's current position, ignoring physics movement
if (ball+0x2CC != 0 || ball+0x2F9 != 0) {
    pos = ball.pos;  // use stored position (no movement)
}
```

### Ball_Update (0x405E00)

Same pattern — tar flag causes the physics integration to use the ball's current position instead of the computed movement target:

```c
if (ball+0x2CC != 0 || ball+0x2F9 != 0 || ball+0x2F8 != 0) {
    // Override computed position with ball's current position
    pos = ball.pos;
}
```

### Fall Timer Decay (Ball_FallUpdate)

Because tar clears `ball+0x768` (grounded) to 0, the fall timer decays:

```c
// _DAT_004cf3c8 = 0.0f (threshold)
// _DAT_004cf448 = 0.02f (decay rate per frame)
if (ball+0xC60 != 0.0f || ball+0x768 == 0) {
    ball+0xC60 -= 0.02f;           // decay from 1.0 (0x3F800000)
    if (ball+0xC60 < 0.0f) {
        ball+0x2E8 = 1;            // set fall-timeout flag
    }
}
```

At 0.02/frame and 25fps, `ball+0xC60` reaches 0 after **~50 frames (~2 seconds)**, then sets the fall-timeout flag.

## Gameplay Consequences

1. **Ball freezes** — tar overrides physics movement with the ball's current position
2. **Cannot reach goal** — N:GOAL handler checks `ball+0x768 != 0` as a prerequisite; tar clears it to 0
3. **Fall timer starts** — grounded=false causes ball+0xC60 to decay from 1.0 → 0.0 over ~2 seconds
4. **Permanent** — no cleanup timer anywhere in Ball_Update or Ball_FallUpdate; flag persists until ball destroy/respawn

## Comparison with N:WATER

| Property | N:TARPIT | N:WATER |
|----------|----------|---------|
| Flag location | ball+0x2CC (byte) | ball+0x2D5 (byte) |
| Timer | None (permanent) | 10 frames (ball+0x2D8, auto-decrements) |
| Clears grounded | Yes (ball+0x768=0) | No |
| Blocks goal | Yes | No |
| Sound | 3D splash (board+0x484) | None |
| Recovery | Ball respawn only | Auto-clears after 10 frames |

## Related Constants

| Address | Value | Type | Description |
|---------|-------|------|-------------|
| 0x4CF448 | 0.02 | float | Fall timer decay rate per frame |
| 0x4CF3C8 | 0.0 | float | Zero threshold (ball+0xC60 == 0) |
| 0x4CF368 | 0.0 | float | Velocity squared threshold |
| 0x4CF418 | 3.0 | float | Speed threshold |

## Verification

- Decompiled `DispatchCollisionEvents` (0x40C5D0) via GhidraMCP
- Decompiled `Ball_FallUpdate` (0x408830) and `Ball_Update` (0x405E00)
- Verified all 30 board vtable[0x1D] entries — all custom handlers call base as fallthrough
- Read constant values from .rdata via GhidraMCP read_memory
- Verified June 2026 from Hamsterball.exe V3.6.c (md5=7d25019366b8d7f55906325bd630d7fe)
