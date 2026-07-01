# Hamsterball Water Physics Mod v6

Custom water physics for Hamsterball via bass.dll proxy.

## What's New in v6

**Fix: Ball shatters at apex of bounce-out after a long fall.**

During a long fall, type 5 mesh-penetration sets `ball+0x2E9` (falling flag) to 1 **before** the ball reaches water. Hook 3 prevents *new* type 5 sets while submerged, but the flag set during the fall was never cleared. Death check #2 at `0x40721F` (`0x2E9` set + `ABS(position_delta) < 2.0`) fires at the apex of the bounce-out — exactly when the ball's velocity drops to zero — shattering the ball.

The v4 grace period (120 frames ≈ 5s of death suppression after leaving water) only delayed the inevitable. Any subsequent slowdown would trigger the death.

**Fix:** Clear `ball+0x2E9` in `trigger_water_contact()` the moment the ball enters water. This eliminates the root cause — the grace period remains as a safety net.

## What's New in v5

**Fix: FPU state corruption crash at 0x407BC6.**

The Phase 15 code cave was calling `apply_water_physics()` without saving the x87 FPU register stack. The C function's float operations corrupted the FPU state that `Ball_Update` depends on for collision/render math. This caused a crash when entering water (the only time the full FPU-heavy physics path runs).

v5 adds `FNSAVE`/`FRSTOR` (108-byte full x87 state save/restore) around the C function call in the code cave. Additionally, the mod is now compiled with `-msse2 -mfpmath=sse` so the C function uses SSE registers instead of x87, providing belt-and-suspenders protection.

## Architecture (v4+)

Four hooks working together:

| Hook | Address | Type | Purpose |
|------|---------|------|---------|
| 1 | 0x40C5D0 | Trampoline (8B) | DispatchCollisionEvents — detect E:WATER, trigger 3-step (flag + damp + capture Y) |
| 2 | 0x407BB4 | Code cave | Phase 15 — per-frame drag/buoyancy while in_water (with FPU save/restore) |
| 3 | 0x407377 | Code cave | Type 5 suppressor — skip 0x2E9 death block while submerged |
| 4 | 0x4CF3C0+8 | Vtable swap | Ball_FallDeath — suppress death during in_water + grace period |

### Collision-Event-Driven Detection

No MeshWorld scanning, no background threads. The game's own collision system tells the mod when the ball touches water:

1. **Hook 1** (DispatchCollisionEvents trampoline) intercepts all collision events. When the collision object's name starts with `E:WATER`, it fires a 3-step trigger: set `in_water` flag, reduce velocity by `entry_damping`, capture ball Y as `water_surface_y`.

2. **Hook 2** (Phase 15 code cave) runs every frame. If `in_water` is set, applies drag (all velocity axes), horizontal drag (extra on X/Z), and buoyancy (upward acceleration proportional to submersion depth). Saves/restores full FPU state via FNSAVE/FRSTOR.

3. **Hook 3** (Type 5 collision suppressor) prevents geometric mesh-penetration from setting ball+0x2E9 (falling flag) while submerged. E:LIMIT events still set 0x2E9 through DispatchCollisionEvents, so the ball can still die from level boundaries.

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
| EntryDamping | 0.70 | Velocity multiplier on first contact (0-1) |
| Drag | 0.03 | Per-frame velocity drag on all axes (0-1) |
| HorizontalDrag | 0.04 | Extra drag on X/Z axes (0-1) |
| BuoyancyStrength | 0.45 | Upward acceleration at full submersion |
| Debug | 1 | Write log file (water_mod_log.txt) |

## Debug Log

With `Debug=1` in the INI, the mod writes `water_mod_log.txt` next to bass.dll.

## Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll hamsterball_water_mod.c -lwinmm \
  -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
  -Wl,--add-stdcall-alias -msse2 -mfpmath=sse
```
