> **⚠️ Status: Untested** — This is an HB+ API port that has not yet been crash-tested or verified in-game.

# Water Mod (HB+ v2.1)

Water physics mod for Hamsterball — HB+ API version of the bass.dll proxy water_mod v7.

## Features

- **E:WATER collision event** triggers water entry (velocity damping + surface Y capture)
- **Per-frame physics**: drag, horizontal drag, and buoyancy while submerged
- **Dizzy immunity** while submerged (clears bounce counter + sets immunity timer)
- **Fall death suppression** during water + 120-frame grace period after exit
- **Type 5 mesh-penetration suppression** while submerged/in grace period

## HB+ Sliders

All parameters adjustable in the Options menu:

| Slider | Default | Range | Description |
|--------|---------|-------|-------------|
| Entry Damping | 0.90 | 0.0-1.0 | Velocity multiplier on water contact |
| Drag | 0.02 | 0.0-0.1 | Per-frame velocity drag on all axes |
| Horizontal Drag | 0.04 | 0.0-0.1 | Extra drag on X/Z axes |
| Buoyancy | 1.0 | 0.0-2.0 | Upward acceleration at full submersion |

Toggle on/off with the "Water Physics" button.

## Level Setup

Place `E:WATER` collision objects in custom levels. The object needs at least one face (triangle). The Y coordinate of the ball at contact determines the water surface height.

## Differences from bass.dll proxy version

| Component | bass.dll (v7) | HB+ (this) |
|---|---|---|
| E:WATER detection | DispatchCollisionEvents trampoline (asm) | `onEventPlaneCollide` callback |
| Per-frame physics | Phase 15 code cave + FPU save/restore (asm) | `onBallUpdate` callback (no FPU issues!) |
| Type 5 suppression | Code cave (needed — runs before onBallUpdate) | Code cave (same) |
| Fall death suppression | vtable[8] swap | vtable[8] swap (same) |
| Config | INI file | HB+ sliders |
| BASS proxy | ~200 lines | Removed |

Author: RodentRacer / Hamsterbot
