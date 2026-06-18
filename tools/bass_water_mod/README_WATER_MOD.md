# Hamsterball Water Physics Mod (bass.dll proxy)

Adds realistic buoyancy physics for custom `E:WATER` event planes in the original Windows game.

## Features

- **Entry damping**: when the ball first hits an `E:WATER` plane while falling, it loses 30% of its vertical speed.
- **Depth-scaled buoyancy**: the deeper the ball is submerged, the stronger the upward force.
- **Stable floating**: at zero vertical velocity the ball settles half-submerged at the water surface.
- **Horizontal control preserved**: X/Z movement works normally with a slight drag and reduced top speed.
- **No vertical speed cap**: buoyancy can push the ball up as fast as physics allows.

## Installation

1. Copy the following files into your **Hamsterball game folder** (next to `Hamsterball.exe`):
   - `bass.dll`
   - `hamsterball_water.ini`
2. **Rename the original `bass.dll` to `bass_real.dll`**.
3. Run `Hamsterball.exe` normally.

Or run `install_water_mod.bat` from the game folder (it performs the rename automatically).

## Level design

Place an invisible collision plane in your level and name it:

```
E:WATER
```

The `E:` prefix makes the object invisible and non-rendered. When the ball touches the plane, the water physics activate.

You can tune behavior by editing `hamsterball_water.ini`:

```ini
[WaterPhysics]
EntryDamping=0.70          ; vertical speed kept on entry (0.7 = lose 30%)
Drag=0.03                  ; overall water drag per frame
HorizontalDrag=0.04        ; extra horizontal drag per frame
GravityEquivalent=0.45     ; effective gravity per frame; buoyancy balances at half-submerged
AccelerationDamp=0.92      ; max horizontal speed in water = 92% of normal
TimerFrames=3              ; frames water physics persists after leaving the plane

[Debug]
Debug=0                    ; set to 1 to write water_mod.log
```

## How it works

This mod is a proxy for `bass.dll`. The game loads it instead of the real BASS audio library. The proxy forwards all BASS calls to `bass_real.dll` and patches two game functions in memory:

- `Ball vtable[4]` (Ball_Update) — applies buoyancy, drag, and damping each frame.
- CALL sites to `CreateNoDizzy` in the level/arena collision handlers — detects `E:WATER` collisions and records the water surface height.

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
