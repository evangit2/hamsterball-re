# Up Race Vacuum System

Verified July 2026 from Hamsterball.exe V3.6.c (md5=7d25019366b8d7f55906325bd630d7fe)
via GhidraMCP decompilation.

## Overview

Up Race (Level 6, file `LevelUp.MESHWORLD`) features vacuum tubes that suck the ball
in at one location and shoot it out at another. The system is a **3-phase state machine**
driven by `CollisionFace_Update`, with `E:VACPOPOUT` as a cleanup trigger at the exit.

## MESHWORLD Reference Points

Level designers place three types of utility markers in Section 1 of `LevelUp.MESHWORLD`:

| Ref | Purpose |
|-----|---------|
| `VAC-IN` | Entry point — where the ball gets sucked in |
| `VAC-OUT` | Exit point — where the ball gets launched out |
| `VAC-VEC` | Direction vector — the launch trajectory direction |

These are **not game objects** (no `N:`/`E:` prefix, no collision). They are pure
coordinate markers. Two pairs exist in `LevelUp.MESHWORLD`:
- `VAC-IN-1` / `VAC-OUT-1` / `VAC-VEC-1`
- `VAC-IN-02` / `VAC-OUT-02` / `VAC-VEC-02`

## Level Setup — `Scene_SetupLevelUp` (0x411540)

At level load, this function:

1. Creates MeshWorld from `levels\levelup`
2. Sets initial ball position to **(0, 50.0, 0)** — starting high up
3. Sets camera distance to **900.0** (Scene+0xA70)
4. Iterates the MW object list looking for any object whose name starts with `"VAC-IN"` (via `__strnicmp`)
5. For each VAC-IN found:
   - Extracts the suffix (e.g. `"-1"`, `"-02"`)
   - Constructs `"VAC-OUT%s"` and `"VAC-VEC%s"` format strings
   - Looks up matching VAC-OUT and VAC-VEC objects via `AthenaHashTable_Lookup`
   - Allocates a **CollisionFace** object (0x38 bytes) via `operator_new(0x38)`
   - Passes IN/OUT/VEC positions to `CollisionFace_ctor` (0x436860)
   - Appends the CollisionFace to an **AthenaList at Scene+0x10DB**

The AthenaList at Scene+0x10DB is iterated per-frame by the scene update loop,
calling `CollisionFace_Update` (vtable[0]) on each CollisionFace.

## CollisionFace Struct (0x38 bytes, vtable 0x4D5768)

| Offset | Type | Field | Source |
|--------|------|-------|--------|
| +0x00 | void* | vtable | 0x4D5768 |
| +0x04 | void* | scene ptr | parent board/scene |
| +0x08 | float | VAC-IN X | entry point position |
| +0x0C | float | VAC-IN Y | entry point position |
| +0x10 | float | VAC-IN Z | entry point position |
| +0x14 | float | VAC-OUT X | exit point position |
| +0x18 | float | VAC-OUT Y | exit point position |
| +0x1C | float | VAC-OUT Z | exit point position |
| +0x20 | float | direction X (normalized) | computed from VAC-VEC |
| +0x24 | float | direction Y (normalized) | computed from VAC-VEC |
| +0x28 | float | direction Z (normalized) | computed from VAC-VEC |
| +0x2C | void* | current ball ptr | 0 = idle, nonzero = ball being transported |
| +0x30 | int | phase | 0 = sucking, 1 = shooting |
| +0x34 | int | timer | countdown frames |

### CollisionFace_ctor (0x436860)

```c
CollisionFace_ctor(this, scene, inX, inY, inZ, outX, outY, outZ,
                   vecX, vecY, vecZ)
{
    this->scene = scene;
    this->vtable = &CollisionFace_vtable;  // 0x4D5768
    this->inPos  = {inX, inY, inZ};        // +0x08/+0x0C/+0x10
    this->outPos = {outX, outY, outZ};     // +0x14/+0x18/+0x1C
    // Compute direction = VAC-VEC - VAC-OUT (edge vector)
    this->direction = {vecX - outX, vecY - outY, vecZ - outZ};
    Vec3_NormalizeAndScale(&this->direction, 1.0);  // normalize to unit vector
    this->ballPtr = 0;     // +0x2C — idle
}
```

Note: The direction vector is computed from the VEC marker position relative to the
OUT marker position, then normalized. This becomes the exit velocity direction.

### CollisionFace vtable (0x4D5768)

| Slot | Address | Name |
|------|---------|------|
| [0] | 0x0043D160 | CollisionFace_Update — per-frame state machine |
| [1] | 0x00000000 | (null) |
| [2] | 0x0043D400 | Scalar deleting destructor (calls Rotator_Cleanup_vtable2 + _free) |
| [3] | 0x004606D0 | Rotator_Update (shared vtable slot) |

## CollisionFace_Update (0x0043D160) — The Vacuum State Machine

This is the per-frame update called on each CollisionFace. It's a 3-phase state machine.

### Key Constants

| Address | Value | Description |
|---------|-------|-------------|
| 0x4D5D10 | 50.0 (double) | Y-axis proximity threshold for ball detection |
| 0x4CF440 | 0.25 (double) | Y increment per frame during SUCK phase |
| 0x4CF3E8 | 6.0 (float) | Y increment per frame during SHOOT phase |
| 0x4CF9F8 | 10.0 (float) | Exit velocity scale (direction × 10.0) |
| — | 0x1E (30) | Math_FastDistance2D threshold (fixed-point, XZ plane) |
| — | 0x74 (116) | SUCK phase timer (frames) |
| — | 0x2D (45) | SUCK phase early-exit threshold |
| — | 0x0F (15) | SHOOT phase timer (frames) |
| — | 0x41A00000 | 20.0f — ball radius during launch |
| — | 0x41D00000 | 26.0f — ball radius restored by E:VACPOPOUT |

### Phase: IDLE (ball ptr == 0)

Every frame, scans the ball list (Scene+0x29D4) using AthenaList iteration:

1. For each ball in the list:
   - Computes `Math_FastDistance2D(ball, VAC-IN)` in the XZ plane
   - If distance < 30 (0x1E) **AND** `|ball.Y - VAC-IN.Y| < 50.0`:
     - Ball is captured!
2. On capture:
   - Stores ball pointer at `CollisionFace+0x2C`
   - Plays `vac-o-sux` 3D sound (App+0x504, 3 channels)
   - Sets `ball+0x808 = 1000` (impact/freeze counter — blocks `Ball_ApplyForce`)
   - Sets `ball+0x2CC = 1` (in-tar flag — also blocks `Ball_ApplyForce`)
   - Sets phase = 0, timer = 116 (0x74)

> **Correction (July 2026):** This doc previously claimed three fields were set:
> `ball+0x808` (called "input disable timer"), `ball+0x2D4` (called "no-control
> flag"), and `ball+0x2CC` (called "control flag"). Verification against decompiled
> `Ball_ApplyForce` (0x402650) and `Ball_Update` (0x405E00) showed:
>
> - `ball+0x808` (`[0x202]` as int array): **impact/freeze counter**. Counts down
>   each frame via `Ball_Update` (L223). When non-zero, `Ball_ApplyForce` skips
>   force application. Also set by `N:NOCONTROL` (=10) and `E:CATAPULTBOTTOM` (=1000).
> - `ball+0x2CC` (`[0xB3]` as byte): **in-tar flag**. Also set by `N:TARPIT`.
>   Does NOT auto-decrement — must be manually cleared. Blocks `Ball_ApplyForce`.
> - `ball+0x2D4`: **NOT a ball field.** Only appears in `TimerDisplay_0x004298c0`
>   loading `greenchecker.bmp` texture. Does not appear in any ball-related
>   decompilation. The prior claim was an error.
>
> Both `+0x808` and `+0x2CC` gate `Ball_ApplyForce` via an AND condition in the
> guard clause at 0x402650:
> ```c
> if ((*(char*)(ball+0x2F9) == 0) &&     // not teleporting
>     (*(char*)(ball+0x2CC) == 0) &&     // not in tar
>     (*(int*)(ball+0x808) == 0) &&      // not frozen
>     (*(int*)(ball+0x2F0) < 0x51))      // impact counter < 81
> ```

### Phase 0: SUCK (phase == 0, timer counting down from 116)

Each frame:
- If timer > 45 frames remaining (timer >= 0x2D):
  - Keeps `ball+0x2CC = 1` (in-tar flag — blocks force application)
  - Moves ball **upward**: `ball.Y += 0.25` (double at 0x4CF440)
- If timer <= 45:
  - Stops moving the ball (just waits for timer to expire)
- When timer hits 0:
  - Transitions to **Phase 1** with timer = 15 (0x0F)
  - Clears `ball+0x2CC = 0` (re-enable force application)

### Phase 1: SHOOT (phase == 1, timer counting down from 15)

Each frame:
- **Teleports ball XZ** to VAC-IN position: `ball.X = VAC-IN.X`, `ball.Z = VAC-IN.Z`
- Moves ball **upward**: `ball.Y += 6.0` (float at 0x4CF3E8)
- When timer hits 0 (after 15 frames): **LAUNCH**
  1. Clears `ball+0x808 = 0` (clear freeze counter — re-enable force application)
  2. **Teleports ball to VAC-OUT position**: copies VAC-OUT X/Y/Z to ball+0x164/+0x168/+0x16C
  3. Clears `ball+0x2CC = 0` (clear in-tar flag)
  4. Sets `ball+0x284 = 20.0` (0x41A00000) — shrinks ball radius during launch
  5. **Sets exit velocity**: writes normalized direction × 10.0 to CollisionMesh+0xCA4/CA8/CAC
     (the ball's physics velocity vector)
  6. Resets to IDLE (ball ptr = 0, ready to grab next ball)

### Total transport time

116 (suck) + 15 (shoot) = **131 frames** ≈ **5.2 seconds** at 25fps.

## E:VACPOPOUT — Ball Radius Reset (UpRaceCollisionEvents @ 0x4119B0)

`E:VACPOPOUT` is an `E:` event trigger placed at the **exit** of the vacuum tube.
It fires **after** the CollisionFace has transported and launched the ball.

### What it does

```c
if (__stricmp(eventName, "E:VACPOPOUT") == 0) {
    ball[0xA1] = 0x41D00000;  // ball+0x284 = 26.0f — restore normal radius
    Sound_Play3D(App+0x468, ball.x, ball.y, ball.z);
}
```

It sets `ball+0x284` (radius) to **26.0f** and plays a 3D sound.

### Why 26.0?

The player ball's normal radius is set to **27.0f** by `Ball_ctor2` (0x4039E0) at
`this+0x284 = 0x41D80000`. However, 26.0 is close to this value and likely the
"intended gameplay radius" after any per-level adjustments. The prior documentation
claiming "sets ball[0xA1] = 20.0 (vacuum popout force)" was **wrong in every detail**:

| Claim | Wrong value | Correct value |
|-------|-------------|----------------|
| Value | 20.0 | 26.0 |
| Field | "popout force" | radius (+0x284) |
| Effect | applies a force | restores normal ball size |

The CollisionFace_Update shrinks the ball to **20.0** during the launch phase.
E:VACPOPOUT fires at the exit to **restore the radius** back to 26.0.

### UpRaceCollisionEvents full event table

The Up Race collision handler at 0x4119B0 handles these events, then falls through
to the base `DispatchCollisionEvents` (0x40C5D0):

| Event | Action |
|-------|--------|
| `E:HELPINERTIA` | Sets `ball+0x2A4 = 2.5f` — reduces inertia for easier control |
| `E:UNHELPINERTIA` | Sets `ball+0x2A4 = 5.0f` — restores normal inertia |
| `E:VACPOPOUT` | Sets `ball+0x284 = 26.0f` — restores normal radius after vacuum exit. Plays 3D sound. |
| `N:SPEEDCYLINDER` | Calls `Pendulum_PlayCollisionSound` — speed boost sound |
| `N:EXTRATIME` | Awards +500 time (single-player only, one-time via rotator+0x10E4 flag). Creates "EXTRA TIME:" ScoreObject popup. |

## Other Up Race Mechanics

### Board Constructor (LevelBoard_Up_ctor @ 0x420390)

Pre-loads three sub-meshes:

| Board Offset | Mesh File | Mechanic |
|-------------|-----------|----------|
| +0x4784 | `levelup-lifter` | **Lifter** — oscillating elevator platform (bounces between +2.0/−2.0 bounds) |
| +0x4788 | `levelup-speedcylinder` | **SpeedCylinder** — speed boost (plays collision sound) |
| +0x478C | `levelup-button` | **Button** — big red button, awards +5 seconds extra time |

### Board Vtable

- Board vtable: 0x4D11A0
- Collision handler (vtable[0x1D]): 0x4119B0 (UpRaceCollisionEvents)
- Race end handler: 0x420660 (iterates CollisionFace list at +0x436C, calls vtable[0] to reset)
- Destructor: 0x420550 (frees CollisionFace list at +0x10DB via CollisionFace_dtor + _free)

## Ball Radius Fields — Clarification

The ball struct has **two** radius-related fields:

| Offset | Set By | Value | Used For |
|--------|--------|-------|----------|
| +0x284 | Ball_ctor2 (0x4039E0) | 27.0f (0x41D80000) | Constructor default — player ball |
| +0x284 | Ball_InitPhysicsDefaults (0x405100) | 35.0f (0x420C0000) | Physics override — used by 8-balls (CreateBadBall calls this) |
| +0x284 | E:VACPOPOUT (0x4119B0) | 26.0f (0x41D00000) | Vacuum exit — restores normal player size |
| +0x284 | CollisionFace_Update (0x43D160) | 20.0f (0x41A00000) | Vacuum launch — temporary shrink |
| +0x284 | E:SHRINK (Odd Race) | 13.0f (0x41500000) | Odd Race shrink mechanic |
| +0xC78 | Ball_InitBattleMode (0x456CD0) | 25.0f (0x41C80000) | Arena/battle mode radius (different field) |

**Key distinction:** `+0x284` is the race-mode radius (player + 8-balls).
`+0xC78` is the battle-mode radius (arena only). These are separate fields.

## Function Reference

| Address | Name | Description |
|---------|------|-------------|
| 0x411540 | Scene_SetupLevelUp | Level setup: loads meshworld, creates CollisionFace objects from VAC-IN/OUT/VEC |
| 0x420390 | LevelBoard_Up_ctor | Board constructor: loads lifter/speedcylinder/button sub-meshes |
| 0x420550 | BoardLevel_Up_Dtor | Destructor: frees CollisionFace list |
| 0x420660 | Board_Up_HandleRaceEnd | Race end: resets CollisionFace objects |
| 0x4119B0 | UpRaceCollisionEvents | Collision handler: HELPINERTIA, VACPOPOUT, SPEEDCYLINDER, EXTRATIME |
| 0x436860 | CollisionFace_ctor | Constructor: stores IN/OUT/VEC positions, computes normalized direction |
| 0x0043D160 | CollisionFace_Update | Per-frame state machine: suck → shoot → launch ball through tube |
| 0x436910 | CollisionFace_dtor_vtable | Trivial destructor: resets vtable pointer |

## Ghidra Renames Applied

| Address | Old Name | New Name |
|---------|----------|----------|
| 0x004119B0 | UpRace_DispatchCollisionEvents | UpRaceCollisionEvents |
| 0x0043D160 | FUN_0043d160 | CollisionFace_Update |

## Verification

All data verified July 2026 from decompiled Ghidra output:
- `UpRaceCollisionEvents` decompiled via GhidraMCP `decompile_function`
- `CollisionFace_Update` decompiled via GhidraMCP `decompile_function` (created function first via `create_function`)
- Float constants read via `read_memory` and decoded with `struct.unpack`
- VACPOPOUT presence in LevelUp.MESHWORLD confirmed from `meshworld_names_full.json`
- VAC-IN/OUT/VEC marker names confirmed (2 pairs in LevelUp.MESHWORLD)
