# Heavy 8-Ball Mod

Play as the **Heavy 8-Ball** — your player ball becomes the black 8-ball with 4x mass, knocking other balls around like a bowling ball!

## What It Does

| Change | Offset | Value | Effect |
|--------|--------|-------|--------|
| Ball mesh → 8Ball | `ball+0x754` | `9` | Your ball looks like the Arena 8-ball |
| Mass → Heavy | `ball+0xC78` | `100.0` | 4x normal Arena mass (25.0) — you barely get knocked back, and send other balls flying |
| Battle physics ON | `ball+0xC7C` | `1` | Enables collision response physics |

## Installation

1. Go to your Hamsterball game folder
2. Rename `bass.dll` → `bass_real.dll`
3. Copy the modded `bass.dll` into the game folder
4. Launch the game — your ball is now the heavy 8-ball!

## Android (GameHub/Wine)

1. Same as above — rename original `bass.dll` to `bass_real.dll`
2. Copy modded `bass.dll` into the game folder
3. Set Wine DLL override to `native` for `bass.dll`
4. Launch — works automatically, no button press needed

## Safety

- ✅ No IAT hooks
- ✅ No code caves / VirtualProtect
- ✅ Background thread does memory writes only
- ✅ All pointer accesses guarded by IsBadReadPtr
- ✅ Android/GameHub safe
- ✅ Crash-tested (37s, no crash)

## Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll heavy_8ball.c -lwinmm \
  -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
  -Wl,--add-stdcall-alias
```
