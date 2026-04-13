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

## Milestone 9: Scene System Documentation 🔄
- [x] Decompile Scene_dtor (0x419770) - master scene destructor
- [x] Decompile Scene_Update (0x419C00) - main tick function
- [x] Decompile Scene_Render (0x41A2E0) - 1P/2P/split render
- [x] Decompile Scene_SetCamera (0x419FA0) - camera positioning
- [x] Decompile Scene_RenderWithCamera (0x40DFA0) - two-pass render
- [x] Document SceneObject class (vtable 0x4D934C, 0xD4 bytes)
- [x] Document 8 level setup functions (leveldark through levelup)
- [x] Document 3 reflection render passes
- [x] Document scene object lists (6 lists at known offsets)
- [x] Document Scene vtable layout (36 entries at 0x4D0260)
- [x] Document SceneObject vtable layout (10 entries at 0x4D934C)
- [x] Document all 14 RumbleBoard arena init functions
- [x] Document Board (Tournament) constructors (Toob/Rodenthood level8)
- [ ] Document remaining level setup functions (level11-15, if they exist as Scene_Setup)
- [ ] Reconstruct full Scene structure layout (all +0x864 offsets and beyond)
- [ ] Document collision event dispatch per level

## New Documentation Created
- `BALL_PHYSICS_DECOMP.md` - Full ball physics system decompilation
- `MESHWORLD_OBJECT_TYPES.md` - N:/E: object type reference
- Updated `FUNCTION_MAP.md` with Input, Texture, Ball systems
- Updated `KEY_DECOMPILATIONS.md`
