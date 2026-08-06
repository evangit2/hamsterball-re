# Hamsterball Water Physics Mod v7.5

Custom water physics for Hamsterball via bass.dll proxy.

## What's New in v7.5

**Running water currents reworked as a subset of `E:WATER`.**

Place a collision plane named `E:WATERFLOW-N`, `E:WATERFLOW-S`,
`E:WATERFLOW-E`, or `E:WATERFLOW-W` in a custom level. It is handled as a
**subset of `E:WATER`** — it does everything calm water does (set the flag,
damp velocity on entry, capture the surface Y), then sets a per-ball flow
direction flag:

- **N** = −Z, **S** = +Z, **E** = +X, **W** = −X
- Flow direction flags: 0 = none (calm), 1 = N, 2 = S, 3 = E, 4 = W

While in running water, each frame the mod adds a **constant force**
(`CurrentStrength`, default `0.18`) to the ball's **force accumulators**
(`ball+0x170/174/178`) in the flow direction — the same accumulator
mechanism the game uses for movement. The physics engine consumes these with
proper collision response, so the current pushes/carries the ball smoothly
and is naturally capped by drag, and you can fight it by inputting against
it.

- Touching plain `E:WATER` (calm) → flow direction 0, no force
- Touching `E:WATERFLOW-<dir>` → does all the `E:WATER` entry, then sets direction 1-4
- Touching `E:WATEREXIT` → water off entirely (no grace period)

## What's New in v7.3

**Added `E:WATEREXIT` event plane.**

Place a collision plane named `E:WATEREXIT` in a custom level. When the ball
touches it, the water flag is turned OFF entirely — all water physics stop
immediately, with **no grace period**:

- `in_water = 0` (stops drag / buoyancy instantly)
- surface + submersion tracking reset
- grace period = 0 (the exit is final — no lingering death suppression)

Repeated `E:WATEREXIT` triggers are harmless (idempotent), and re-entering
water via `E:WATER` right after re-activates the flag normally.

`E:WATEREXIT` is checked before the `E:WATER` prefix match in the collision
hook, so a plane named `E:WATEREXIT` is never mistaken for `E:WATER`.

## What's New in v7.2

**Surgical death-flag block (replaces the whole-block skip).**

Previously, Hook 3 skipped the ENTIRE type-5 death block while the ball was in
water or in the grace period — including the camera snap, viewport setup, and
split-flag logic. Now it skips only the ONE instruction that matters:
the `ball+0x2E9` (death/falling flag) write at `0x407391`.

In practice: while submerged or within the 5-second grace period after leaving
water, the game never marks the ball as "falling to its death" — so death
checks #1/#2 can't fire and the ball won't shatter at the apex of a bounce-out.
But the camera still snaps to follow the falling ball, the viewport still
updates, and the split logic still runs — the fall *looks* and *feels* exactly
as before, it just can't kill you.

E:LIMIT (fell out of the level / kill plane) is a separate writer via
DispatchCollisionEvents and is NOT blocked — falling out of the map still kills
you, as intended.

## What's New in v7

**Feature: Dizzy immunity while submerged.**

Two additions:

1. **On water entry** (Hook 1 `trigger_water_contact`): Clears `ball+0x2EC` (bounce counter) to 0 — same effect as E:NODIZZY calling `Ball_DizzyImmunity`. The 2-strike dizzy counter resets when the ball hits water.

2. **Every frame while in water** (Hook 2 `apply_water_physics`): Clears bounce counter AND sets `ball+0x2F4` (dizzy_immunity_timer) to `GRACE_PERIOD_FRAMES` (120 frames ~5s). Uses the same max-only logic as `Ball_DizzyImmunity` (0x402400) — only increases the timer, never shortens existing immunity.

This means the ball cannot go dizzy while submerged, and when it exits, the remaining grace period immunity also covers the bounce-out arc.

## What's New in v6

**Fix: Ball shatters at apex of bounce-out after a long fall.**

Two-part root cause:

1. During the fall, type 5 mesh-penetration sets `ball+0x2E9` (falling flag) = 1 **before** the ball reaches water. The v4 grace period only delayed death — it didn't clear the flag.

2. After the ball exits water, type 5 collision **re-sets** `0x2E9` when the ball clips through the mesh on the way up. Hook 3 only checked `in_water` (now 0 after exit), so it fell through and the type 5 block set `0x2E9=1`. At the apex, death check #2 (`0x2E9==1 + ABS(position_delta) < 2.0`) fired through `vtable[8]`.

**Three fixes applied:**
- Clear `ball+0x2E9` on water entry (in `trigger_water_contact`)
- Clear `ball+0x2E9` **every frame** in `apply_water_physics` while `in_water` or `grace_frames > 0`
- Extend Hook 3's `is_ball_in_water` to also return true during grace period — suppresses type 5 collision entirely during the bounce-out arc

## What's New in v5

**Fix: FPU state corruption crash at 0x407BC6.**

The Phase 15 code cave was calling `apply_water_physics()` without saving the x87 FPU register stack. The C function's float operations corrupted the FPU state that `Ball_Update` depends on for collision/render math. This caused a crash when entering water (the only time the full FPU-heavy physics path runs).

v5 adds `FNSAVE`/`FRSTOR` (108-byte full x87 state save/restore) around the C function call in the code cave. Additionally, the mod is now compiled with `-msse2 -mfpmath=sse` so the C function uses SSE registers instead of x87, providing belt-and-suspenders protection.

## Architecture (v4+)

Four hooks working together:

| Hook | Address | Type | Purpose |
|------|---------|------|---------|
| 1 | 0x40C5D0 | Trampoline (8B) | DispatchCollisionEvents — E:WATER entry (flag + damp + capture Y); E:WATERFLOW-<dir> = same + flow_dir flag 1-4; E:WATEREXIT turns water OFF |
| 2 | 0x407BB4 | Code cave | Phase 15 — per-frame drag/buoyancy while in_water (with FPU save/restore, clear 0x2E9, dizzy immunity) |
| 3 | 0x407377 | Code cave | Type 5 suppressor — block ONLY the 0x2E9 death-flag write while submerged/in grace (camera snap + split logic intact) |
| 4 | 0x4CF3C0+8 | Vtable swap | Ball_FallDeath — suppress death during in_water + grace period |

### Collision-Event-Driven Detection

No MeshWorld scanning, no background threads. The game's own collision system tells the mod when the ball touches water:

1. **Hook 1** (DispatchCollisionEvents trampoline) intercepts all collision events. When the collision object's name starts with `E:WATER`, it fires the trigger: set `in_water` flag, reduce velocity by `entry_damping`, capture ball Y as `water_surface_y`, clear `ball+0x2E9` (falling flag), and clear `ball+0x2EC` (bounce counter). `E:WATERFLOW-<dir>` is a subset: it does all of the above, then sets `flow_dir` (1-4) parsed from the name. When the name is `E:WATEREXIT`, it instead turns the water flag OFF entirely and immediately, with no grace period. `E:WATEREXIT` is matched before `E:WATER` so the prefix match never swallows it.

2. **Hook 2** (Phase 15 code cave) runs every frame. If `in_water` is set, applies drag (all velocity axes), horizontal drag (extra on X/Z), and buoyancy (upward acceleration proportional to submersion depth). Also clears `ball+0x2E9`, clears `ball+0x2EC` (bounce counter), and sets `ball+0x2F4` (dizzy_immunity_timer) to `GRACE_PERIOD_FRAMES` every frame while submerged. If `flow_dir` is non-zero, adds a constant force (`CurrentStrength`) to the force accumulators (`ball+0x170/174/178`) in that direction. Saves/restores full FPU state via FNSAVE/FRSTOR.

3. **Hook 3** (Type 5 collision suppressor) prevents geometric mesh-penetration from writing the `ball+0x2E9` (falling/death flag) while submerged or within the grace period. It skips only the flag-write instruction, so the camera snap and viewport still follow the falling ball. E:LIMIT events still set 0x2E9 through DispatchCollisionEvents, so the ball can still die from level boundaries.

4. **Hook 4** (vtable[8] Ball_FallDeath) suppresses death while in water or within the grace period (120 frames ~5s) after leaving water. Covers the bounce-out scenario.

## Installation

1. In your Hamsterball game folder, rename the original `bass.dll` to `bass_real.dll`
2. Copy the mod `bass.dll` and `hamsterball_water.ini` into the game folder
3. Place `E:WATER` collision objects in custom levels
4. Run Hamsterball.exe normally

## Uninstall

1. Delete the mod `bass.dll`
2. Rename `bass_real.dll` back to `bass.dll`

## Level Setup

In the Raptisoft level editor, add a collision mesh object named `E:WATER`.
The object needs at least one face (triangle). The Y coordinate of the ball
at the moment of contact determines the water surface height.

## Configuration

See `hamsterball_water.ini` for all options:

| Parameter | Default | Description |
|-----------|---------|-------------|
| EntryDamping | 0.90 | Velocity multiplier on first contact (0-1) |
| Drag | 0.02 | Per-frame velocity drag on all axes (0-1) |
| HorizontalDrag | 0.04 | Extra drag on X/Z axes (0-1) |
| BuoyancyStrength | 1.0 | Upward acceleration at full submersion |
| Debug | 1 | Write log file (water_mod_log.txt) |

## Debug Log

With `Debug=1` in the INI, the mod writes `water_mod_log.txt` next to bass.dll.

## Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll hamsterball_water_mod.c -lwinmm \
  -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
  -Wl,--add-stdcall-alias -msse2 -mfpmath=sse
```
