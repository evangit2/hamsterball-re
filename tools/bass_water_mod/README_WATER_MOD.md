# Hamsterball Water Physics Mod v2

Custom water physics for Hamsterball via bass.dll proxy.

## What's New in v2

v2 is a complete rewrite that fixes fundamental physics issues in v1:

| Problem in v1 | Fix in v2 |
|---|---|
| Modified position delta, not velocity → drag/buoyancy didn't actually affect momentum | Modifies velocity directly in physics struct (phys+0xCA4/CA8/CAC) |
| Buoyancy was a position offset → ball oscillated, never floated stable | Buoyancy is now acceleration (added to velocity) → ball decelerates, stops, floats |
| Entry damping was a one-frame teleport → visual stutter | Entry damping reduces velocity → smooth deceleration that persists |
| ball+0x14 mislabeled as Scene → wrong pointer reads | Correctly identified as Board (verified via Ghidra) |
| Vtable hook with save-call-modify pattern | Phase 15 code cave (proven approach from jump mod + power bounce mod) |
| Per-frame constants with no FPS independence | Same issue remains (per-frame constants), but velocity modification means the effect is more stable across framerates |

## Installation

1. In your Hamsterball game folder, rename the original `bass.dll` to `bass_real.dll`
2. Copy the mod `bass.dll` and `hamsterball_water.ini` into the game folder
3. Place `E:WATER` collision planes in custom levels (see below)
4. Run Hamsterball.exe normally

## Uninstall

1. Delete the mod `bass.dll`
2. Rename `bass_real.dll` back to `bass.dll`

Or run `uninstall_water_mod.bat`.

## How It Works

### Hook Architecture

Uses a **Phase 15 code cave** at `0x407BB4` in Ball_Update — the same proven hook point used by the jump mod and power bounce mod. The cave:

1. Saves all registers (PUSHAD + PUSHFD)
2. Calls a C function `apply_water_physics(ball)` that modifies velocity
3. Restores all registers (POPFD + POPAD)
4. Executes the original 6 bytes
5. Jumps back

### Velocity-Based Physics

The C function reads the ball's **velocity** from the physics struct (ball+0x1A4 → +0xCA4/CA8/CAC) and modifies it directly:

1. **Entry Damping**: On first contact while falling, `vel_y *= 0.70` (30% velocity reduction). This persists — the ball actually slows down.

2. **Drag**: All velocity axes scaled by `(1 - drag)` per frame. The ball decelerates over time.

3. **Horizontal Drag**: Extra scaling on X/Z velocity, making horizontal movement sluggish.

4. **Buoyancy**: Upward acceleration `= buoyancy_strength × submersion × 2.0`, added to `vel_y`. At half-submerged, it roughly cancels gravity. At full submersion, net upward force. The ball decelerates going down, stops, then accelerates upward — reaching a stable float.

### Water Plane Discovery

Scans the Board's collision mesh for objects named `E:WATER`:
- Board is at ball+0x14 (NOT Scene — v1 had this wrong)
- CollisionLevel is at Board+0x8B0 → +0x08 for the MeshWorld
- Iterates AthenaList at MeshWorld+0x2C
- For each MeshBuffer, reads name at +0x864
- If name starts with `E:WATER`, reads Y from first collision face vertex
- A background thread re-scans on board/level changes

**Fallback**: If no E:WATER objects found, uses Y coordinates from the INI file.

### Level Setup

In the Raptisoft level editor, add a collision mesh object named `E:WATER`.
The object needs at least one face (triangle). The Y coordinate of the first
vertex of the first face determines the water surface height.

## Configuration

See `hamsterball_water.ini` for all options with descriptions.

## Debug Log

With `Debug=1` in the INI, the mod writes `water_mod_log.txt` next to bass.dll.
