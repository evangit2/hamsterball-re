# mkn_plus_bumper_colors

**Author:** MAKYUNI 🦇  
**Contributors:** Hamsterbot  
**API Version:** HB+ v2.1 (HAMSTERBALL_API_VERSION 3)  
**Mod Version:** 1.0

## Description

When a ball hits a bumper, instead of flashing hardcoded white (1, 1, 1), each bumper flashes its own emissive color from the level MESHWORLD material data. The emissive color is read from the bumper's material struct at runtime.

## How It Works

1. **Collision:** Ball hits `N:BUMPER%d` mesh → `board+0x53FC[index]` set to 1.0 (unchanged)
2. **Decay:** Per-frame, lit value decreases by 0.05 (unchanged, ~0.33s at 60fps)
3. **Render (PATCHED):** Instead of writing hardcoded `0x3F800000` (1.0) to all 4 emissive channels, a code cave reads the bumper's material emissive RGBA from:
   - `level = *(board + 0x8AC)`
   - `material_array = *(level + 0x28)`
   - `material_index = *(bumper_sceneobj + 0x04)`
   - `material = material_array + material_index * 0x50`
   - `emissive_R/G/B/A = *(float*)(material + 0x34/0x38/0x3C/0x40)`

## Installation

1. Place `mkn_plus_bumper_colors.dll` in the game's `Mods\` folder
2. Ensure HB+ (bass.dll proxy) is installed
3. Launch the game
4. Enable "Bumper Colors" in the options menu

## Technical Details

- **Hook point:** 0x00412EA6 (inside `Scene_RenderReflectiveObjects4` at 0x00412DC0)
- **Hook range:** 0x00412EA6 to 0x00412ECB (37 bytes: JMP + 32 NOPs)
- **Code cave:** VirtualAlloc'd executable memory, reads material emissive each render frame
- **Fallback:** If material pointer chain is null, falls back to original white (1,1,1,1)
- **Toggle:** Can be enabled/disabled via options menu toggle button

## Source Files

- `source/mkn_plus_bumper_colors.cpp` — Main mod source
- `source/hbplus_api.h` — MinGW IModAPI vtable wrapper (includes PatchMemory + GetGameBaseAddress)
- `source/nocrt.cpp` — No-CRT runtime (no msvcrt dependency)
- `source/nocrt.h` — No-CRT header
- `source/HamsterballAPI.h` — HB+ v2.1 API header (static_asserts disabled for MinGW)
- `source/build.sh` — Cross-compilation build script (MinGW 32-bit)
