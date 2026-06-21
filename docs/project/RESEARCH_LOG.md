# Hamsterball Reverse Engineering Project

## Project Overview

**Goal:** Create a documented, legally clean, open-source recreation of Hamsterball (2000s Windows game) that is playable, buildable, and behaviorally faithful to the original.

**Permission:** Original developer has granted permission for reverse engineering and recreation.

## Binary Inventory

| File | Size | MD5 | SHA256 |
|------|------|-----|--------|
| Hamsterball.exe | 1,404,928 | 7d25019366b8d7f55906325bd630d7fe | 3379e9041c7ab83abd07da1bcf974529... |
| bass.dll | 97,336 | fd5ec122f4dd201b3c3ef19e3058af81 | 11075ca0a1a3064bd971a3faa3856595... |
| unins000.exe | 871,194 | aa765b9cb2afb116f955d063bb7a2b36 | 53da811094504adcfaf6c220bfbc2d65... |

## Dependencies Identified

### Runtime DLLs (bundled)
- `bass.dll` - BASS audio library

### System DLLs (Windows)
- d3d8.dll, d3d9.dll - DirectX 8/9
- ddraw.dll - DirectDraw
- dinput8.dll - DirectInput 8
- dsound.dll - DirectSound
- kernel32.dll, user32.dll, gdi32.dll - Windows core
- advapi32.dll - Windows API
- shell32.dll - Windows Shell
- ws2_32.dll, wsock32.dll - Networking (for eSellerate DRM)
- comctl32.dll - Common Controls
- ole32.dll, oleaut32.dll - OLE Automation
- riched32.dll - Rich Edit
- version.dll - Version Info

### External Requirements
- DirectX 8 or higher
- Windows 2000/XP/Vista/7

## File Formats

### Custom Formats
- `.MESHWORLD` - Level format (custom binary format)
- `.MESH` - 3D mesh format
- `.MESHCOLLISION` - Collision mesh data

### Standard Formats
- `.ogg` - Ogg Vorbis audio (sounds)
- `.mo3` - MO3 audio (music, MOD/tracker format with BASS)
- `.png` - Textures (with mipmaps)
- `.bmp` - Textures
- `.xml` - Configuration (Jukebox.xml, RaceData.xml)
- `.CFG` - Save/config (HS.CFG)
- `.SAV` - Tournament save (TOURNAMENT.SAV)

## Asset Inventory

- **Levels:** 80+ MESHWORLD files (Arena-*, Level*, Secret-*, etc.)
- **Meshes:** 40+ custom 3D mesh files
- **Textures:** 250+ texture files (PNG, BMP with mipmaps)
- **Sounds:** 80+ OGG sound effects
- **Music:** 1 MO3 music file
- **Fonts:** Custom bitmap fonts (ArialNarrow, ShowcardGothic)

## Identified Subsystems

1. **Graphics Engine** - DirectX 8/9 based renderer
2. **Audio Engine** - BASS library for music and sound effects
3. **Input System** - DirectInput8 for keyboard/gamepad
4. **Physics** - Custom physics for ball movement
5. **Level System** - Custom MESHWORLD format loader
6. **UI/Menu System** - In-game menus and UI
7. **Game Logic** - Tournament, race, time trial modes
8. **Save System** - HS.CFG and TOURNAMENT.SAV
9. **DRM System** - eSellerate activation

## Development Notes

- Game is PE32 executable for Windows
- Uses C++ with likely custom game engine
- Class structure visible in symbols: App, Graphics, BoardLevel3
- Custom collision detection system
- Supports both single player and 2-player modes

## Status

- [x] Original files acquired and hashed
- [x] Asset inventory complete
- [ ] Wine execution environment tested
- [ ] Ghidra analysis started
- [ ] Core subsystems documented
- [ ] Reimplementation started