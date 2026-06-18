# Hamsterball Water Physics Mod (bass.dll proxy)

Adds realistic buoyancy physics for custom `E:WATER` event planes in the original Windows game.

## Features

- **Entry damping**: when the ball first falls into `E:WATER`, it loses 30% of its vertical speed.
- **Drag**: overall velocity damping while in water, with extra horizontal damping.
- **Depth-scaled buoyancy**: the deeper the ball is submerged, the stronger the upward force.
- **Stable floating**: at zero vertical velocity the ball settles half-submerged at the water surface.
- **Horizontal control preserved**: X/Z movement works normally with a slight drag.
- **No vertical speed cap**: the mod does not impose its own maximum vertical velocity.

## Installation

1. Copy the following files into your **Hamsterball game folder** (next to `Hamsterball.exe`):
   - `bass.dll`
   - `hamsterball_water.ini`
2. **Rename the original `bass.dll` to `bass_real.dll`**.
3. Run `Hamsterball.exe` normally.

Or run `install_water_mod.bat` from the game folder (it renames the original bass.dll for you).

## Level design

Place an invisible collision plane in your level and name it exactly:

```
E:WATER
```

The `E:` prefix marks the object as an invisible event trigger. When the ball touches the plane, the water physics activate.

## Tuning

Edit `hamsterball_water.ini` in the game folder:

```ini
[WaterPhysics]
EntryDamping=0.70          ; vertical speed multiplier on first entry while falling
Drag=0.03                  ; overall water drag per frame (0.03 = 3%)
HorizontalDrag=0.04        ; extra horizontal drag per frame
GravityEquivalent=0.45     ; effective gravity per frame; buoyancy balances at half-submerged
TimerFrames=10             ; frames physics persists after leaving the water plane

[Debug]
Debug=0                    ; set to 1 to write Hamsterball_water_mod.log
```

## How it works

This mod is a proxy for `bass.dll`. The game loads it instead of the real BASS audio library. The proxy forwards all BASS calls to `bass_real.dll` and patches two game functions in memory:

- `Ball vtable[4]` (the wrapper at `0x408390`around `Ball_Update` at `0x405E00`) — applies buoyancy, drag, and damping each frame.
- CALL sites to `CreateNoDizzy` in the level/arena collision handlers — detects `E:WATER` collisions and records the water surface height.

Physics changes use the CollisionMesh persistent velocities (`Ball + 0x1A4 -> +0xCA4/CA8/CAC`) and the engine's gravity multiplier (`CollisionMesh + 0xC7C`). No changes are made to the ephemeral `Ball + 0x170` velocity accumulator.

## Documentation correction

This mod clarified an earlier documentation mistake: the function at `0x405190` is **not** `Ball_Update`; it is the respawn/break cleanup routine that zeros velocities when a ball breaks. The actual main physics tick is `Ball_Update` at **`0x405E00`**.

## Uninstall

Run `uninstall_water_mod.bat`, or manually delete the proxy `bass.dll` and rename `bass_real.dll` back to `bass.dll`.

## Build (from Linux with MinGW)

```bash
make
make package   # creates hamsterball-water-mod.zip
```

## Troubleshooting

- **No water effect**: make sure the collision object is named exactly `E:WATER` (case-insensitive). The `E:` prefix is required for it to be loaded as an invisible event trigger.
- **Game crashes on startup**: verify `bass_real.dll` exists and is the original BASS library.
- **Strange physics**: adjust `GravityEquivalent` and `EntryDamping` in the INI file.
