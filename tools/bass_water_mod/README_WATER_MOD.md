# Hamsterball Water Physics Mod

Custom water physics for Hamsterball via bass.dll proxy.

## Installation

1. In your Hamsterball game folder, rename the original `bass.dll` to `bass_real.dll`
2. Copy the mod `bass.dll` and `hamsterball_water.ini` into the game folder
3. Place `E:WATER` collision planes in custom levels (see below)
4. Run Hamsterball.exe normally

## Uninstall

1. Delete the mod `bass.dll`
2. Rename `bass_real.dll` back to `bass.dll`

Or run `uninstall_water_mod.bat`.

## How Water Planes Work

### Level Setup

In the Raptisoft level editor, add a collision mesh object named `E:WATER`.
The object needs at least one face (triangle). The Y coordinate of the first
vertex of the first face determines the water surface height.

The mod scans both the collision MeshWorld and visual MeshWorld for objects
named `E:WATER` when a level loads.

### Fallback: INI Water Planes

If your level's collision data doesn't expose E:WATER objects (or you want
to test without editing a level), you can specify water plane Y coordinates
in `hamsterball_water.ini`:

```ini
[WaterPlanes]
Count=1
Y0=100.0
```

## Physics Behavior

When the ball touches a water plane, the following physics apply:

1. **Entry Damping**: On first contact while falling, vertical speed is
   reduced by 30% (configurable via `EntryDamping`).

2. **Drag**: A small per-frame velocity reduction on all axes (`Drag`).

3. **Buoyancy**: An upward force proportional to how deep the ball is
   submerged. The force increases linearly from 0 (just touching surface)
   to 2× gravity (fully submerged).

4. **Equilibrium**: At half-submerged, buoyancy exactly cancels gravity,
   so the ball floats with zero net vertical force at the surface.

5. **Horizontal Dampening**: Extra drag on X/Z axes, slightly lowering
   the maximum horizontal speed in water (`HorizontalDrag`).

6. **No Vertical Speed Cap**: The engine's own max velocity remains
   unchanged — no artificial cap is added.

## Configuration

See `hamsterball_water.ini` for all options with descriptions.

## Debug Log

With `Debug=1` in the INI, the mod writes a log file (`water_mod.log` or
`Hamsterball_water_mod.log`) showing water plane discovery and per-frame
physics data.
