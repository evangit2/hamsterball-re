# Half Size All Balls

Shrinks ALL balls (player + AI + split balls) to half their normal size.

## What it does

Patches 3 sites in the game executable:

1. **Ball_ctor2 default radius**: 27.0 → 13.5 (AI/bad balls)
2. **Player ball spawn radius**: 26.0 → 13.0
3. **CreateBadBall SIZE tag**: Halves the FPU value via code cave (MESHWORLD-specified sizes)

Ball_Shatter (split balls) copies the parent's radius, so split balls are automatically half size too.

## Installation

1. Go to your Hamsterball game folder
2. Rename `bass.dll` → `bass_real.dll`
3. Copy this `bass.dll` into the game folder
4. Run the game — all balls will be half size!

## Build

```
i686-w64-mingw32-gcc -shared -o bass.dll half_size_balls.c \
    -lwinmm -Wl,--enable-stdcall-fixup \
    -O2 -static -static-libgcc -Wl,--add-stdcall-alias
```

## Crash Test

✅ Passed — 38.7s runtime, no crash (hbtestd, June 2026)
