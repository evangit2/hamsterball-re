# Hamsterball FPS Mod

A drop-in mod for the original **Hamsterball** Windows game that lets you set a custom FPS cap.

## What it does

- Reads `TargetFPS` and `RenderFPS` from `hamsterball_fps.ini`
- Writes those values into the game's live `App` struct at `App+0x16C` and `App+0x170`
- Optionally enables the old "uncap" mode (`Uncap=1`) which NOPs the render-skip throttle and forces immediate swap intervals

## Installation (Windows)

1. Copy these files into your Hamsterball game folder:
   - `bass.dll` (the proxy)
   - `hamsterball_fps.ini`
   - `install_fps_mod.bat` (optional helper)
2. Double-click `install_fps_mod.bat`, **OR** manually:
   - Rename the original `bass.dll` → `bass_real.dll`
   - Rename `hamsterball_fps_mod.dll` → `bass.dll`
3. Edit `hamsterball_fps.ini` and set your desired FPS.
4. Launch the game normally.

## Uninstallation

Run `uninstall_fps_mod.bat`, or manually delete the proxy `bass.dll` and rename `bass_real.dll` back to `bass.dll`.

## Configuration

```ini
[FPS]
TargetFPS=144
RenderFPS=144

[Uncap]
Uncap=0
```

- `TargetFPS`: update tick rate cap (default in original game: 88)
- `RenderFPS`: render frame cap (default in original game: 75)
- `Uncap=1`: additionally remove the render-skip check and vsync limits (may affect physics/timing)

## Building from source (Linux + MinGW)

```bash
cd tools/bass_fps_proxy
make
make package   # creates hamsterball-fps-mod.zip
```

## How it works

The game loads `bass.dll` at startup. This proxy forwards every BASS call to the real `bass_real.dll` and, after the game has initialized, patches the live `App` singleton via the pointer at `0x005341E0`.

Offsets used:

| Field        | Offset | Original value |
|--------------|--------|----------------|
| target_fps   | 0x16C  | 88             |
| render_fps   | 0x170  | 75             |

## Warning

Very high FPS can change physics behavior (ball friction, collision response, timer drift). Test carefully. The uncap mode in particular can cause the well-known "white screen" timing issue on some systems.
