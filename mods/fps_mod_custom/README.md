# fps_mod_custom

Custom FPS mod (writes App+0x16C/0x170)

## Files
- `hamsterball_fps_mod.c` — C source code
- `hamsterball_fps_mod.dll` — Compiled DLL (PE32 i386)
- `hamsterball-fps-mod.zip` — Packaged zip

## Proxy Type
BASS.dll proxy. Installation:
1. Rename original `bass.dll` → `bass_real.dll` in the Hamsterball game folder
2. Copy the mod's `bass.dll` (or renamed DLL) into the game folder
3. Launch Hamsterball
