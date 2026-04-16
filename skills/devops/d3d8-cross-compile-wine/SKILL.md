---
name: d3d8-cross-compile-wine
description: Cross-compile D3D8-based game with MinGW, test on Wine with OpenGL translation
category: devops
tags: [d3d8, mingw, wine, cross-compile, windows-gaming]
---

# D3D8 Cross-Compilation and Wine Testing Workflow

## Toolchain Setup
```bash
# Install MinGW-w64 cross-compiler
sudo apt install mingw-w64

# Install Wine for testing
sudo apt install wine
```

## Compilation Command
```bash
i686-w64-mingw32-gcc -std=c11 -m32 -O2 -Iinclude \
  src/core/win32_main.c src/level/*.c \
  -o hamsterball.exe \
  -ld3d8 -ldinput8 -ldsound -ldxguid -lole32 -lwinmm -mwindows
```

## D3D8 Build Fixes Required

### 1. Header Order (Critical)
```c
// MUST include before dsound.h
#include <mmsystem.h>
#include <dsound.h>
```

### 2. D3DPRESENT_PARAMETERS Field Name
```c
// MinGW uses FullScreen_PresentationInterval, not PresentationInterval
present.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
```

### 3. D3DX Math Not in D3D8
```c
// D3DX math functions are NOT in d3d8.lib
// Use standard math:
#define D3DX_PI 3.14159265358979323846
// or manually implement:
D3DXVECTOR3 v = {x, y, z};
```

### 4. Link Flags
```bash
-ld3d8 -ldinput8 -ldsound -ldxguid -lole32 -lwinmm -mwindows
```

## Wine Testing

### Basic Test
```bash
wine ./hamsterball.exe
```

### With Debug Output
```bash
WINEDEBUG=+d3d8 wine ./hamsterball.exe 2>&1 | grep -i "d3d"
```

### Xvfb for Headless (CI/CD)
```bash
Xvfb :99 -screen 0 1024x768x24 &
export DISPLAY=:99
wine ./hamsterball.exe
```

## Verification Steps
1. ✅ D3D8 device creation succeeds
2. ✅ DInput8 keyboard/mouse initialized  
3. ✅ Level loads from asset path
4. ✅ Game loop runs at ~30fps
5. ✅ Wine translates D3D8 → OpenGL silently

## Original Game API Comparison

### Comparison Test Script
```bash
./scripts/test_api_compare.sh :99
```
Runs both original Hamsterball.exe and reimpl hamsterball.exe under Wine with
WINEDEBUG=+d3d8,+dinput,+dsound tracing, captures traces and screenshots.

### Key Finding: Original Game Fails on Xvfb
- Original `Hamsterball.exe` (1.4MB PE32 i386) requires **D3DCREATE_HARDWARE_VERTEXPROCESSING**
- On Xvfb (no GPU): `D3DERR_NOTAVAILABLE` — HAL device can't create without real GPU
- Our reimpl uses **SOFTWARE_VERTEXPROCESSING fallback** → succeeds on Xvfb
- For full original game testing: need real Windows GPU or GPU passthrough

### What Each Shows on Xvfb
| Build | D3D8 Device | Window | 3D Scene |
|-------|-------------|--------|----------|
| Original (Hamsterball.exe) | ❌ D3DERR_NOTAVAILABLE | Error dialog only | None |
| Reimpl (hamsterball.exe) | ✅ SW vertex processing | Clear color screen | Blank (no geometry yet) |

### Screenshots
Screenshots and comparison docs: `analysis/screenshots/comparison/`

## Pitfalls
- **Original game fails on Xvfb**: D3D8 HAL needs real GPU, use real hardware or GPU passthrough
- **D3DX functions unavailable**: Use manual math or link d3dx8.lib separately
- **PresentationInterval field name**: MinGW uses FullScreen_PresentationInterval
- **Header order**: mmsystem.h before dsound.h for WAVEFORMATEX
- **32-bit only**: Must use -m32 for D3D8 COM interface compatibility
- **EXE not in git**: Original Hamsterball.exe and bass.dll gitignored from public repo.
  Get from your share link (files.rsks.lol) and place in originals/installed/extracted/
