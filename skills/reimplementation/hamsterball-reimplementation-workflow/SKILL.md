---
name: hamsterball-reimplementation-workflow
description: Iterative build-test-commit workflow for Hamsterball D3D8 reimplementation
category: reimplementation
tags: [d3d8, game-dev, iterative, workflow]
---

# Hamsterball Reimplementation Workflow

## Session State (From Session 52)
- **Current**: First D3D8 build running on Wine. Original game EXE available for API comparison.
- **Original game**: D3DERR_NOTAVAILABLE on Xvfb (needs real GPU for HARDWARE_VERTEXPROCESSING)
- **Reimpl**: D3D8 device OK with SOFTWARE_VERTEXPROCESSING fallback, clear screen, no geometry yet
- **Commit Point**: win32_main.c + meshw parser + basic loop all working
- **Ghidra Docs**: 50 docs, 21,551 lines total
- **Test script**: scripts/test_api_compare.sh

## Iterative Development Loop
```
1. Identify next subsystem (e.g., MESHWORLD geometry rendering)
2. Write targeted decompilation from Ghidra
3. Implement in reimpl/src/
4. Compile with MinGW
5. Test with Wine
6. Commit + push to both repos
7. Save Ghidra
```

## Subsystem Priority (for next sessions)
1. **MESHWORLD geometry rendering** — Parse vertex/face data, build D3D8 vertex buffers
2. **Texture loading** — TGA/PNG loader, D3D texture creation, UV mapping
3. **Collision detection** — Parse .COL files, ball collision with surfaces
4. **Real ball physics** — Ball_AdvancePositionOrCollision 6-phase pipeline
5. **UI/menu system** — Main menu, pause menu, race menus
6. **Loading screen** — TimerDisplay, App_ResourceLoader
7. **Audio** — DirectSound/BASS initialization

## Key Ghidra Functions for Each Subsystem
- MESHWORLD geometry: decomp_meshworld_parse.c (0x470930) — handles .ASE vertices/faces
- Collision: Ball_AdvancePositionOrCollision (0x4564C0), Collision_TraverseSpatialTree (0x465EF0)
- Textures: LoadTexture (0x4146D0), TGA loader in decompilations/graphics/
- Menu: MainMenu_ctor (0x42DE50), UIList_AddItem (0x4497F0)

## Testing Commands
```bash
# Build
cd ~/hamsterball-re/reimpl
i686-w64-mingw32-gcc -std=c11 -m32 -O2 -Iinclude \
  src/core/win32_main.c src/level/*.c \
  -o hamsterball.exe -ld3d8 -ldinput8 -ldsound -ldxguid -lole32 -lwinmm -mwindows

# Run
wine ./hamsterball.exe

# Debug Wine D3D8 translation
WINEDEBUG=+d3d8 wine ./hamsterball.exe 2>&1 | head -100
```

## Commit Messages Pattern
- "D3D8 build: [subsystem] [action]"
- Example: "D3D8 build: MESHWORLD geometry rendering + vertex buffers"
- Always commit to both origin (public) and priv (private)

## Memory to Save for Next Session
- 3781/3781 functions documented (100% complete)
- 50 docs, 21,551 lines of documentation
- First D3D8 build working (skeleton)
- Next focus: Fix WarmUp obj section parsing for bbox+vertex extraction, then test on real GPU

## Common Issues & Fixes
- D3DX math not in D3D8: Use manual math or d3dx8.lib
- PresentationInterval: Use FullScreen_PresentationInterval in MinGW
- Header order: mmsystem.h before dsound.h
- Wine translation: D3D8→OpenGL happens automatically, no extra code needed
