# Unbreakable + 6x Size Mod

## Info
- **File**: `bass.dll` (proxy)
- **Effects**: No fall damage + 6x ball size + no pause
- **Android-safe**: No IAT hooks, no GetTickCount, no threads

## What it does
Merges three mods into one DLL:

### 1. No Pause (3 patches)
Disables ESC/right-click pause via 3 single-byte JZ→JMP patches.

### 2. 6x Ball Size (3 patches)
- Ball_ctor2 default radius: 27.0 → 162.0
- Player ball spawn radius: 26.0 → 156.0
- CreateBadBall SIZE: code cave multiplies by 6.0f (pure FPU asm, no C calls)

### 3. Unbreakable Ball (8 patches from XRow's CEA)
- 3× early RET (Ball_Shatter, variant, Ball_Shatter_OnRamp)
- 5× NOP (prevent shatter flag + is_active + fall timer writes)

## Winlator Safety
- No IAT hooks (GetTickCount crashes Android)
- No background threads — all patches applied in DllMain
- Code cave is pure FPU assembly (FSTP/FLD/FMUL/RET)
- All patches restored on DLL_PROCESS_DETACH

## Build
```
i686-w64-mingw32-gcc -shared -o bass.dll unbreakable_6x.c \
  -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
  -Wl,--add-stdcall-alias
```

## Installation
1. Rename original `bass.dll` to `bass_real.dll`
2. Copy mod `bass.dll` to game folder
3. On Android/Wine: set DLL override to `native` for `bass.dll`
