---
name: hamsterball-first-d3d8-build
description: First D3D8-based Hamsterball reimplementation build - proof of concept running on Wine
category: reimplementation
tags: [d3d8, win32, game-engine, hamsterball]
---

# First D3D8 Build Success - Proof of Concept

## Current State
- ✅ D3D8 device creation (1024x768)
- ✅ DInput8 keyboard + mouse initialization  
- ✅ WinMain (0x4278E0) mirrored exactly
- ✅ MESHWORLD parser loading Arena-WarmUp (17 objects)
- ✅ Ball spawns at correct position (184.9, 26.4, 183.4)
- ✅ Game loop running at 30fps fixed timestep
- ✅ FPS counter working (~30K FPS)
- ✅ Wine D3D8→OpenGL translation active

## What's NOT Implemented (Skeleton Only)
- ❌ Loading screen menu
- ❌ Full MESHWORLD geometry rendering (binary format not fully parsed - only object list extracted, not vertices/faces/materials)
- ❌ Texture loading (D3DX)
- ❌ Collision detection (ball falls through floor)
- ❌ Real ball physics (6-phase pipeline)
- ❌ Scene system with render passes
- ❌ UI/menu system
- ❌ Audio (DirectSound stubbed)
- ❌ Any menus or title screen

## Architecture (WinMain mirroring)
```
WinMain (0x4278E0)
├── D3D8 device creation
├── DInput8 initialization
├── Load Arena-WarmUp.mesworld
├── Parse 17 scene objects
├── Enter game loop:
│   ├── Clear screen
│   ├── Draw ball (sphere placeholder)
│   ├── Draw object markers
│   └── Present
```

## To Get "Playable":
1. **MESHWORLD geometry rendering** - Parse vertex/face data, build vertex buffers
2. **Texture loading** - Parse PNG, create D3D textures, UV mapping
3. **Collision geometry** - Parse .COL files, ball collision detection
4. **Real physics** - Ball_AdvancePositionOrCollision 6-phase pipeline
5. **UI scaffolding** - Menu system (bare minimum)

## Compilation
```bash
cd ~/hamsterball-re/reimpl
i686-w64-mingw32-gcc -std=c11 -m32 -O2 -Iinclude \
  src/core/win32_main.c src/level/*.c \
  -o hamsterball.exe \
  -ld3d8 -ldinput8 -ldsound -ldxguid -lole32 -lwinmm -mwindows
```

## Testing
```bash
wine ./hamsterball.exe
```

## CRITICAL: MESHWORLD Binary Format (v7 verified)

1. **MeshWorld_Parse** (0x4039E0 area) = TEXT/ASE format parser — WRONG ONE for .MESHWORLD files
2. **FUN_004629e0** (decomp_binary_meshloader.c) = BINARY loader — CORRECT ONE

Binary format (v7, verified against WarmUp data):
- Section1: u32 count → per item: name_len+name, 6×u32 (floats: pos3+pad3), ext_flag, if ext: amb(4f)+dif(4f)+spec(4f)+**EMIS(4f)**+shine(f)+refl(u32)+hastex(u32==1)+texname
- Items include BOTH materials AND game objects (PLATFORM,SAFESPOT,START,CAMERALOOKAT etc.)
- Section2: u32 meshbuf_count → data_len+data+face_count+faces(3×4B each)
- Section3: u32 obj_count → type(i32)+pos/rot/scale  
- Section4: bbox(6f) — in struct has 12-byte gap between min_z(+0x45c) and max_x(+0x468)
- Section5: u32 vtx_count + vtx_count×32B (pos3+normal3+uv2)
- SpawnPlatform: 0/0/0 counts → 34 vertices parsed correctly
- WarmUp: 16 items (5 ext PLATFORM), meshbuf=0, obj=1
- u32 mat_count → per mat: u32 name_len+name bytes, 6xu32 fields, u32 ext_flag
  - if extended: ambient[4], diffuse[4], specular[4] (RGBA floats), shine float, has_texture u32
  - if has_texture: u32 texlen + texname bytes
- u32 meshbuf_count → per buf: u32 name_len+name, u32 data_len+raw_data, i32 face_count, per face 3xfloat
- u32 obj_count → per obj: i32 type (0=straight), pos[3], rot[3], scale[3] as u32 values + virtual calls
- 6xfloat bounding box (min xyz, max xyz)
- u32 vertex_count → vertex_count * 0x20 bytes vertex data (pos[3] + normal[3] + uv[2] = 8 floats = 32 bytes)

Ref: ~/hamsterball-re/analysis/ghidra/decompilations/graphics/decomp_binary_meshloader.c

## Next Steps for Real Build
1. Implement binary MESHWORLD parser (use FUN_004629e0 decomp, NOT the text parser)
2. Build D3D8 vertex buffer from parsed vertex data (FVF=D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1)
3. Build D3D8 index buffer per mesh buffer from face data
4. Apply materials (SetMaterial with diffuse/ambient/specular from parsed data)
5. Load textures from parsed texture filenames (DDS/TGA)
6. Implement collision mesh loading from .COL files
