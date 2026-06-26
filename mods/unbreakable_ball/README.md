# Unbreakable Ball Mod

## Info
- **File**: `bass.dll` (proxy)
- **Effect**: Ball never shatters — no fall damage
- **Android-safe**: No IAT hooks, no code caves, no threads for byte patches
- **Translated from**: XRow's CEA script

## What it does
Patches 8 points in Hamsterball.exe to prevent the ball from shattering:

| # | Address | Original | Patch | Effect |
|---|---------|----------|-------|--------|
| 1 | 0x408D70 | `6A FF 64 A1` (SEH frame) | `C3 90 90` (RET) | Ball_Shatter early return |
| 2 | 0x409050 | `6A FF 64 A1` (SEH frame) | `C3 90 90` (RET) | Shatter variant early return |
| 3 | 0x409480 | `6A FF 64 A1` (SEH frame) | `C3 90 90` (RET) | Ball_Shatter_OnRamp early return |
| 4 | 0x40C761 | `88 85 68 07 00 00` | 6× NOP | Prevent is_active=0 write |
| 5 | 0x40C767 | `C6 85 E9 02 00 00 01` | 7× NOP | Prevent shatter flag=1 write |
| 6 | 0x40F226 | `C6 86 68 07 00 00 00` | 7× NOP | Prevent is_active=0 write |
| 7 | 0x40F22D | `C6 86 E9 02 00 00 01` | 7× NOP | Prevent shatter flag=1 write |
| 8 | 0x4075C9 | `FF 86 EC 02 00 00` | 6× NOP | Prevent fall timer increment |

## How it works
- Uses `VirtualProtect` to make code pages writable
- Writes byte patches directly (NOP or RET)
- Verifies original bytes before patching (won't double-patch)
- Restores original bytes on DLL unload
- All pointer accesses guarded by `IsBadReadPtr`

## Build
```
i686-w64-mingw32-gcc -shared -o bass.dll unbreakable_ball.c -lwinmm \
  -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
  -Wl,--add-stdcall-alias
```

## Installation
1. Rename original `bass.dll` to `bass_real.dll`
2. Copy mod `bass.dll` to game folder
3. On Android/Wine: set DLL override to `native` for `bass.dll`
