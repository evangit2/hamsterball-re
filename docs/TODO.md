# Hamsterball RE - TODO List

## Milestone 1: Executable Environment and Binary/Resource Inventory ✅
- [x] Download and preserve original binaries (installer + installed folder)
- [x] Compute SHA256 hashes of all 450 files
- [x] Extract installed game folder (1,404,928 byte EXE + 450 asset files)
- [x] Document file formats (MESHWORLD, MESH, MO3, OGG, XML, CFG)
- [x] Document DLL dependencies (d3d8, dinput8, dsound, bass, etc.)
- [x] Install Wine 9.0, test game launch (partial success - no display)

## Milestone 2: Subsystem Map and First Labeled Ghidra Database ✅
- [x] Run Ghidra headless analysis (3,958 functions found)
- [x] Use r2 for initial function mapping (1,869 functions found)
- [x] Identify entry point (0x004BB4C8)
- [x] Identify Graphics::Initialize (0x00455380)
- [x] Identify window class "AthenaWindow"
- [x] Map import table (9 DLLs, 177 imports)
- [x] Map key string addresses (App::Initialize, etc.)
- [x] Connect Ghidra MCP for interactive decompilation (headless server on port 8089)
- [x] Label top 50 functions in Ghidra (57 labeled, now 80+)
- [x] Map all subsystem addresses (graphics, audio, input, physics, UI, save, DRM)

## Milestone 3: Documented Runtime Dependencies and Launch Procedure 🔄
- [x] Document DLL dependencies (9 DLLs)
- [x] Document registry keys (ADVAPI32 functions identified)
- [x] Document file paths (DATA\HS.CFG, textures, levels, meshes)
- [ ] Test full game launch with Xvfb + Wine
- [ ] Install DirectX 8 runtime via winetricks
- [ ] Document all registry writes during launch
- [ ] Test multiplayer / tournament save

## Milestone 4: Reconstructed Core Loop and Resource Loading ✅ (mostly)
- [x] Decompile WinMain function
- [x] Decompile App::Initialize steps 1-26
- [x] Map game loop (PeekMessageA → Update → Render)
- [x] Reverse MESHWORLD file format parser (MeshWorld_Parse at 0x470930)
- [x] Reverse MESH file format parser (same parser, different loading)
- [x] Reverse texture loading (Graphics_LoadTexture, D3DX embedded)
- [x] Reverse audio initialization (BASS_Init, BASS_MusicLoad)
- [x] Reverse input initialization (DirectInput8Create, Input_Init)

## Milestone 5: Reconstructed Input/Render/Physics/Menu Pipeline ✅
- [x] Reverse D3D8 device creation and render pipeline
- [x] Reverse DirectInput keyboard/gamepad input (Input_Init, KeyboardDevice)
- [x] Reverse ball physics (Ball_Update 18KB, Ball_ApplyForce, Ball_CheckCollisionPlanes)
- [x] Reverse camera system (CAMERALOOKAT)
- [x] Reverse level object system (N:/E: prefixes)
- [x] Reverse AthenaList container class (0x53210)
- [x] Document ball vtable and physics constants

## Milestone 6: First Playable Open-Source Build 🔄
- [x] Create C project scaffold with SDL2 + OpenGL
- [x] Implement MESH file loader
- [x] Implement MESHWORLD file loader
- [ ] Implement basic ball rendering and movement
- [ ] Implement first test level loading
- [ ] Build and run on Linux
- [ ] Implement collision plane system (Level_LoadCollision RE in progress)

## Milestone 7: Behavior-Polish and Bug Reduction
- [ ] Match physics feel of original
- [ ] Match camera behavior
- [ ] Match audio triggers
- [ ] Match menu flow
- [ ] Match scoring system

## Milestone 8: Selective Machine-Code Matching
- [ ] Identify critical functions for codegen matching
- [ ] Set up MSVC 6/7 build environment
- [ ] Match specific function signatures
- [ ] Verify with binary diff

## New Documentation Created
- `BALL_PHYSICS_DECOMP.md` - Full ball physics system decompilation
- `MESHWORLD_OBJECT_TYPES.md` - N:/E: object type reference
- Updated `FUNCTION_MAP.md` with Input, Texture, Ball systems
- Updated `KEY_DECOMPILATIONS.md`
