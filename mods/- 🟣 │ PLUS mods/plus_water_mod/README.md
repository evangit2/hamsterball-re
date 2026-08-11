> **Status: Buildable + shippable.** Both MSVC (class) and MinGW (`_MinGW` variant) build targets provided. The MinGW DLL is verified ship-safe (nocrt + manual 17-entry vtable + only KERNEL32 import). Full in-game gameplay verification still pending (see note below).

# Water Mod (HB+ v2.1)

Water physics mod for Hamsterball — HB+ API version of the bass.dll proxy water_mod, now at **v7.9 parity**.

## v7.9 Fixes (parity with bass v7.9)

1. **Hook 4 now patches the PLAYER ball vtable** slot 8 @ `0x4CF334`
   (base `0x4CF314`, `Ball_SplitDeath`). Previously `0x4CF3A0` (BadBall
   vtable) was hooked, so the player was never actually protected by the
   fall-death suppression — only BadBalls were.
2. **Water-state table recycling** — the 32-slot `g_states` table now
   recycles dead slots (ball not in water, no grace) instead of filling
   up forever and silently killing water physics mid-session. Hook 4 uses
   a lookup-only getter that never allocates.
3. **No entry-frame double-damp** — water entry sets `prev_submersion=1.0`
   so the same-frame surface-crossing check doesn't damp Y velocity a
   second time (effective damping was `entry_damping²`).


## Features

- **E:WATER collision event** triggers water entry (velocity damping + surface Y capture)
- **E:WATEREXIT** turns water OFF entirely with **no grace period** (checked before the E:WATER prefix so it's never swallowed)
- **E:WATERFLOW(N)** = E:WATER subset + running-water current: constant per-frame force into the force accumulators (`ball+0x170/174/178`) in the flow direction. N is 1-8 **clockwise from North** (1=N −Z, 2=NE, 3=E +X, 4=SE, 5=S +Z, 6=SW, 7=W −X, 8=NW; diagonals split by 1/√2). Direction **switches immediately** when touching a different `E:WATERFLOW(N)` plane inside the same body of water; plain `E:WATER` clears the flow.
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
| Current Strength | 0.18 | 0.0-1.0 | Per-frame force of E:WATERFLOW running water (force accumulators) |

Toggle on/off with the "Water Physics" button.

## Level Setup

Place `E:WATER` collision objects in custom levels. The object needs at least one face (triangle). The Y coordinate of the ball at contact determines the water surface height.

- `E:WATERFLOW(N)` — running water, N = flow direction 1-8 clockwise from North
- `E:WATEREXIT` — turns water fully off, no grace period (checked before E:WATER)

## Differences from bass.dll proxy version

| Component | bass.dll (v7.8) | HB+ (this) |
|---|---|---|
| E:WATER detection | DispatchCollisionEvents trampoline (asm) | `onEventPlaneCollide` callback |
| Per-frame physics | Phase 15 code cave + FPU save/restore (asm) | `onBallUpdate` callback (no FPU issues!) |
| Type 5 suppression | Code cave (needed — runs before onBallUpdate) | Code cave (same) |
| Fall death suppression | vtable[8] swap | vtable[8] swap (same) |
| Config | INI file | HB+ sliders |
| BASS proxy | ~200 lines | Removed |

Author: RodentRacer / Hamsterbot

## Building

- **MinGW (Linux):** `./build.sh` → produces `plus_water_mod.dll` from `WaterMod_MinGW.cpp` (nocrt + manual vtable + hbplus_api.h). Verified: 17-entry vtable, CreateModInstance exported, only KERNEL32 import, no msvcrt/.eh_frame.
- **Visual Studio (Windows):** Create an HB+ project from `HBmodTemplate.zip`, replace `MainModFile.cpp` with `WaterMod.cpp` (class-based), build as x86 DLL.

Either way, drop the resulting `.dll` in the game's `Mods\` folder.

## Testing note

The HB+ loading chain (scanning `Mods\`, calling `CreateModInstance`, firing `Initialize()`) cannot be verified under Wine/hbtestd — that test env uses a plain bass proxy, not the HB+ framework. Only real Windows with HB+ installed can confirm the full load + in-game behavior. The raw memory hooks (type-5 code cave at 0x407377, vtable[8] at 0x4CF3A0→0x409480) were byte-verified against Hamsterball.exe.
