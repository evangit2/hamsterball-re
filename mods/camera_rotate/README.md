# Camera_Rotate Mod

Rotates the camera 90° around the Y axis when the ball touches an `E:ROTCAM` plane.

## Usage

Add a plane to your MESHWORLD level with the name:
- `E:ROTCAM(R)` — rotate right (clockwise, +90°)
- `E:ROTCAM(L)` — rotate left (counter-clockwise, -90°)

The rotation takes 3 seconds and uses a sinusoidal ease-in-out curve:
`angle(t) = startAngle + totalRotation × (1 - cos(π × t)) / 2`

## Installation

1. Rename the original `bass.dll` to `bass_real.dll`
2. Copy this mod's `bass.dll` into the game folder

## Technical Details

- Hooks `DispatchCollisionEvents` (0x40C5D0) to detect collision events
- Reads/writes `scene+0x29BC` (orbit angle in radians, verified from `Scene_SetCamera` disassembly)
- The game reads this value every frame in `Scene_SetCamera` (0x419FA0) to position the camera
- A polling thread updates the angle every 16ms (~60fps) during the 3-second animation
- 90° = π/2 radians ≈ 1.5708

## Build

```
i686-w64-mingw32-gcc -shared -o bass.dll camera_rotate_mod.c -lwinmm \
  -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
  -Wl,--add-stdcall-alias
```

## Log

Diagnostic log writes to `camera_rotate_log.txt` next to `bass.dll`.
