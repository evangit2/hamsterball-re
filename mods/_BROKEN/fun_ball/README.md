# Fun Ball Mod

## Info
- **File**: `bass.dll` (proxy)
- **Effect**: Spawns player 1's ball as the FunBall mesh every race
- **Android-safe**: No IAT hooks, no code caves, no D3D API calls

## What it does
The game loads the FunBall mesh (index 10) into the ball mesh array at
`board+0x244` but never assigns it — `Ball_ctor2` always sets `ball+0x754=0`
(Sphere). This mod writes `ball+0x754=10` every frame (20fps) so the game
renders the FunBall mesh for player 1's ball at the start of every race.

No button activation needed — it's automatic.

## Build
```
i686-w64-mingw32-gcc -shared -o bass.dll fun_ball.c -lwinmm \
  -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
  -Wl,--add-stdcall-alias
```

## Installation
1. Rename original `bass.dll` to `bass_real.dll`
2. Copy mod `bass.dll` to game folder
3. On Android/Wine: set DLL override to `native` for `bass.dll`
