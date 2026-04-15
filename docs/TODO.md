# Hamsterball RE - TODO List

## Milestone 1: Executable Environment and Binary/Resource Inventory ✅
- [x] Download and preserve original binaries (installer + installed folder)
- [x] Compute SHA256 hashes of all 450 files
- [x] Extract installed game folder (1,404,928 byte EXE + 450 asset files)
- [x] Document file formats (MESHWORLD, MESH, MO3, OGG, XML, CFG)
- [x] Document DLL dependencies (d3d8, dinput8, dsound, bass, etc.)
- [x] Install Wine 9.0, test game launch (partial success - no display)

## Milestone 2: Subsystem Map and First Labeled Ghidra Database ✅
- [x] Run Ghidra headless analysis (3,781 functions analyzed)
- [x] Use r2 for initial function mapping (1,869 functions found)
- [x] Identify entry point (0x004BB4C8)
- [x] Identify Graphics::Initialize (0x00455380)
- [x] Identify window class "AthenaWindow"
- [x] Map import table (9 DLLs, 177 imports)
- [x] Map key string addresses (App::Initialize, etc.)
- [x] Connect Ghidra MCP for interactive decompilation (headless server on port 8089)
- [x] Label top 50 functions in Ghidra (57 labeled, now 975+)
- [x] Map all subsystem addresses (graphics, audio, input, physics, UI, save, DRM)
- [x] **100% DOCUMENTED: All 3,781 functions identified and renamed (0 FUN_* remaining)**

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

## Milestone 6: First Playable Open-Source Build 🔄 — Phase 2 Deep Docs Done
- [x] Create C project scaffold with SDL2 + OpenGL
- [x] Implement MESH file loader
- [x] Implement MESHWORLD file loader
- [ ] Implement basic ball rendering and movement
- [ ] Implement first test level loading
- [ ] Build and run on Linux
- [ ] Implement collision plane system (Level_LoadCollision RE in progress)

## Phase 2 Deep Documentation (Complete)
- PARTICLE_SYSTEM.md: Ball_CreateTrailParticles (9-particle ring, RumbleScore allocator)
- CAMERA_SYSTEM.md: 5-mode camera (follow/path/shake/snap/orbit), Scene_SetCamera offsets
- SAVE_CONFIG_REGISTRY_SYSTEM.md: 30+ registry fields, BestTime/Medals binary blobs, App struct
- AUDIO_SYSTEM_SFX.md: BASS music + DirectSound 3D SFX, 55 sound effects with channel counts
- LEVEL_OBJECT_FACTORY.md: 25+ Create* functions, struct sizes, scene offsets, full factory map
- ASSET_MANIFEST.md: Complete resource loading manifest (5 fonts, 40+ textures, 14 meshes, 55 sounds)
- GAME_STATE_RACE_LIFECYCLE.md: App state machine, start/end race, tournament progression, timer
- D3D8_RENDERING_PIPELINE.md: 8-pass render system, z-buffer interleaving, alpha blending, lighting
- SCENE_STRUCT.md: 50+ Scene struct offsets, creation flow, camera config, object lists
- INPUT_SYSTEM: Ball_GetInputForce (3-mode: keyboard/mouse/joystick), Scene_HandleInput dispatch
- Ball physics: Ball_AdvancePositionOrCollision 6-phase pipeline (damping, collision, gravity, trail)
- Race completion: RaceGoalReached ctor, medal thresholds, best time tracking
- Decompilations: camera/, particles/, save/, input/, audio/, scene/, physics/

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

## Milestone 9: Scene System Documentation ✅
- [x] Decompile Scene_dtor (0x419770) - master scene destructor
- [x] Decompile Scene_Update (0x419C00) - main tick function
- [x] Decompile Scene_Render (0x41A2E0) - 1P/2P/split render
- [x] Decompile Scene_SetCamera (0x419FA0) - camera positioning (5 modes)
- [x] Decompile Scene_RenderWithCamera (0x40DFA0) - two-pass render
- [x] Decompile Scene_UpdateBallsAndState (0x41B540) - per-ball physics + respawn
- [x] Decompile Scene_HandleRaceEnd (0x41B130) - race finish conditions
- [x] Decompile Scene_HandleCountdown (0x41A540) - race countdown
- [x] Decompile Gear_AdvanceAlongPath (0x418930) - 8-sample gradient descent path follower
- [x] Document Ball_Update (0x405E00) - 23-step physics pipeline
- [x] Document Ball vtable (0x4CF3A0, 9 entries) and App_Run loop (0x46BD80)
- [x] Document Scene vtable (0x4D0260, 36 entries, all named)
- [x] Document SceneObject class (vtable 0x4D934C, 0xD4 bytes)
- [x] Document 8 level setup functions (leveldark through levelup)
- [x] Document 3 reflection render passes
- [x] Document scene object lists (6 lists at known offsets)
- [x] Document all 14 RumbleBoard arena init functions
- [x] Document Board (Tournament) constructors (Toob/Rodenthood level8)
- [x] Deep doc: SCENE_SYSTEM_DECOMP.md with full tick order, render pipeline, camera modes
- [x] Document remaining level setup functions (if they exist as Scene_Setup)
- [x] Document Level_UpdateAndRender (0x40B600) - 6-phase level render pipeline
- [x] Document Level_RenderObjects (0x40B570) - transparent pass renderer
- [x] Document Scene_CheckPath (0x57EC0) - 359-cell ring pathfinder
- [x] Document Scene_SpawnBallsAndObjects (0x41C5B0) - level startup spawner
- [x] Document Scene_RenderAllObjects (0x45E0E0) - 3-pass object render
- [x] Document Scene_RenderFrame (0x60DA0) - vertex buffer construction + font render

## Milestone 10: Rendering Pipeline Documentation ✅
- [x] Document SceneObject_RenderFull (0x470150) - full render with alpha path
- [x] Document SceneObject_RenderSingleObj (0x470440) - single object render
- [x] Document SceneObject_BuildStrips (0x472770) - triangle strip builder
- [x] Document SceneObject_CheckCollision (0x45dfd0) - collision thunk
- [x] Document SceneObject_ComputeCollisionSphere (0x46fbb0) - bounding sphere
- [x] Document MeshWorld_ctor (0x46f3d0) - mesh world construction
- [x] Document Mesh_InitTexture (0x49338e) - D3D texture init
- [x] Document Mesh_DrawWithTransform (0x493671) - draw with temp transform
- [x] Document Mesh_ClearColorVertices (0x49373d) - zero matching vertices
- [x] Document Graphics_DrawIndexedPrimitive (0x47dfb9) - D3D draw wrapper
- [x] Document Scene_BeginFrameThenRender (0x46f3b0) - frame begin wrapper
- [x] Document D3D vtable dispatch map (9 offsets: 0x28-0x200)
- [x] Document Graphics_ApplyMaterialAndDraw (0x455110) - material system
- [x] Document Graphics_RenderScene (0x454BC0) - full 3D render pipeline
- [x] Document D3D texture system (Ctor/Dtor/Init/Release 13 functions)
- [x] Document material structure layout (0x50 bytes, 8 fields)
- [x] Document graphics subsystem functions (25+ functions cataloged)

## Milestone 11: AthenaString System Documentation ✅
- [x] Document AthenaString_AssignCStr (0x473500) - C string assign (75 xrefs)
- [x] Document AthenaString_dtor (0x4736b0) - destructor (85 xrefs)
- [x] Document AthenaString_AssignCRLF (0x473a50) - CRLF assign (21 xrefs)
- [x] Document AthenaString_SprintfToBuffer (0x4bae43) - sprintf to buffer (71 xrefs)
- [x] Document AthenaString_Sprintf (0x4bbdfd) - already known
- [x] Document AthenaString_Format (0x466c70) - already known
- [x] Document AthenaString_Assign (0x4737f0) - already known
- [x] Document AthenaString_Init/CopyCtor/WriteTag - already known

## New Documentation Created
- `BALL_PHYSICS_DECOMP.md` - Full ball physics system decompilation
- `MESHWORLD_OBJECT_TYPES.md` - N:/E: object type reference
- Updated `FUNCTION_MAP.md` with Input, Texture, Ball systems
- Updated `KEY_DECOMPILATIONS.md`
