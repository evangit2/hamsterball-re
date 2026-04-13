# Hamsterball RE Project - Build Notes

## Environment Setup

### System
- OS: Linux (Ubuntu)
- Architecture: x86_64
- Wine: 9.0 installed, game launches but no display

### Tools Installed
- Ghidra 12.0.4 at /opt/ghidra
- radare2 (r2) - CLI disassembler/analyzer
- pefile - Python PE parser
- Wine 9.0 - Windows compatibility layer
- Java 21 JDK
- GCC toolchain

### Project Layout
```
~/hamsterball-re/
├── originals/
│   ├── installer/
│   │   ├── setup_hamsterball.exe    (6.9 MB)
│   │   └── share_download.zip      (15.4 MB)
│   └── installed/
│       ├── Hamsterball.zip          (8.5 MB)
│       └── extracted/               (full game, 450 files)
├── analysis/
│   └── ghidra/
│       ├── HamsterballProject/     (Ghidra project, 20MB)
│       ├── functions.txt           (r2 function list)
│       └── r2_functions.txt        (r2 full function dump)
├── docs/
│   ├── RESEARCH_LOG.md
│   ├── FILE_HASHES.json
│   ├── FILE_FORMATS.md
│   ├── FUNCTION_MAP.md
│   ├── STRUCTS_AND_TYPES.md
│   ├── RUNTIME_ENVIRONMENT.md
│   └── BUILD_NOTES.md (this file)
├── reimpl/
│   ├── src/
│   ├── include/
│   ├── assets/
│   └── tools/
└── notes/
```

## Key Findings So Far

### Binary Analysis
- PE32 i386 executable, MSVC compiled
- Entry point: 0x004BB4C8
- 5 sections: .text, .rdata, .data, .data1, .rsrc
- 9 imported DLLs with ~200 function imports
- 1,869 functions identified by r2
- Window class: "AthenaWindow" (Raptisoft engine name)
- Uses Direct3D 8 for rendering
- Uses BASS library for audio (MO3 format music)

### Key Function Addresses (CONFIRMED via r2)
- `fcn.00455380` - Graphics::Initialize (calls Direct3DCreate8)
  - Sets debug strings at offset +0x8 of object: "Graphics::Initialize(1)" through "Graphics::Initialize(23)"
- `entry0 (0x40BB4C8)` - CRT entry point
- `fcn.00453ed0` - Graphics initialization sub-function
- Cross-references to "AthenaWindow" at 0x46ba62, 0x46bb00, 0x46d218 (window creation)

### Game Engine Architecture
- Engine name: "Athena" (derived from window class "AthenaWindow")
- Built by Raptisoft (John Raptis)
- C++ with MSVC, DirectX 8 era (2010 timestamp)
- Custom mesh format (not standard ASE, despite having ASE-style tokens)
- Debug strings preserved in release build

### Asset Format Summary
- MESHWORLD: Binary level format with type strings
- MESH: Binary 3D mesh with embedded texture references
- MO3: Tracker music via BASS
- OGG: Sound effects
- Font: Custom bitmap font system with atlas PNGs
- CFG: Binary save format with 64-byte player name records
- XML: Jukebox.xml and RaceData.xml for game configuration

### Next Steps
1. Connect Ghidra MCP for interactive decompilation
2. Map the game's main loop (WinMain → App::Initialize → game loop)
3. Reverse Graphics::Initialize in detail
4. Reverse the level loader (MESHWORLD parser)
5. Begin C reimplementation scaffold