# Ball Color Cycle Mod

## Info
- **File**: `bass.dll` (proxy)
- **Controls**: Press **F2** in-game to cycle through 10 colors
- **Android-safe**: No IAT hooks, no code caves, no D3D API calls from thread

## Colors (10 total)
White → Orange → Blue → Green → Pink → Yellow → Purple → Cyan → Red → Dark Orange → (loops back)

## What it does
Tints player 1's hamster ball to a selected color. Sets the ball's material
diffuse/ambient/emissive RGBA values and activates the game's material override
system (gfx+0x7C0) so the 3D sphere mesh uses the custom color.

## v3 Crash Fix
v2 crashed after finishing a race because `gfx+0x7C0` was set to `ball+0x208`
but never cleared. When the ball was destroyed at race end, the pointer became
dangling — the render thread read freed memory → crash.

**Fix**: When no ball is found (scene transition/race end), `gfx+0x7C0` is set
to 0 (NULL), making the game use its default material. The pointer is re-set
to the new ball's render context on the next frame when the ball is found again.

## Build
```
i686-w64-mingw32-gcc -shared -o bass.dll ball_color_cycle.c -lwinmm \
  -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
  -Wl,--add-stdcall-alias
```

## Installation
1. Rename original `bass.dll` to `bass_real.dll`
2. Copy mod `bass.dll` to game folder
3. On Android/Wine: set DLL override to `native` for `bass.dll`
