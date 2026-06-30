# Hamsterball Global Variables

This document catalogs every global variable in the Hamsterball.exe binary,
derived from Ghidra analysis. Variables are organized by category.

## Memory Layout

| Section | Range | Description |
|---------|-------|-------------|
| .text | 0x00401000 - 0x004CEFFF | Code (executable instructions) |
| .rdata | 0x004CF000 - 0x004F6FFF | Read-only data (strings, vtables, IAT) |
| .data | 0x004F7000 - 0x00536AF3 | Mutable global variables |
| .data1 | 0x00537000 - 0x00537FFF | Additional data |
| .rsrc | 0x00538000 - 0x0058FFFF | Resources (icons, cursors, DLL) |

---

## 1. Game Engine Globals

These are the primary game state variables used by the Hamsterball engine.

### g_App
- **Address**: `0x004FD680`
- **Type**: `App` struct (2328 bytes / 0x918 bytes)
- **Description**: The central application object. Contains all global game state including
  the D3D device, window handle, scene pointer, player data, input state, difficulty settings,
  tournament data, and rendering context. This is the single most important global in the game.
  Passed to `App_Initialize_Full()`, `App_Run()`, and `App_Shutdown()` from WinMain.
- **Used in**:
  - `WinMain+0x0A` (DATA) — `App_Initialize_Full(&g_App, ...)`
  - `WinMain+0x14` (DATA) — `App_Run((int*)&g_App)`
  - `WinMain+0x1E` (DATA) — `App_Shutdown((int*)&g_App)`
  - `FUN_004ce4e0` (DATA) — `LoadOrSaveConfig(&g_App.dwVtable)`
  - `Unwind@004ce3e6+0x1A` (DATA) — exception handler cleanup

### g_renderIndex
- **Address**: `0x005341CC`
- **Type**: `uint32` (4 bytes, signed integer)
- **Description**: Render frame counter / sprite animation index. Incremented once per
  rendered object during `Scene_RenderFrame`. Used to index sprite animation ranges via
  `SpriteAnim_SetRange()`. Reset to 0 at the start of each frame when the scene is active.
- **Used in**:
  - `Scene_RenderFrame+0x3F` (WRITE) — `g_renderIndex = 0;` (reset per frame)
  - `Scene_RenderFrame+0x181` (READ) — `SpriteAnim_SetRange(param_1, g_renderIndex);`
  - `Scene_RenderFrame+0x18E` (READ_WRITE) — `g_renderIndex = g_renderIndex + 1;`

### MeshWorld_vtable
- **Address**: `0x004D9CDC` (.rdata)
- **Type**: `pointer` (4 bytes, pointer to vtable array)
- **Description**: Virtual function table for the `MeshWorld` class. Assigned to all MeshWorld
  instances during construction (`*(undefined***)this = &MeshWorld_vtable;`). Contains
  virtual destructor and other MeshWorld virtual methods.
- **Used in**:
  - `MeshWorld_ctor+0x1A` (DATA) — `*this = &MeshWorld_vtable;`
  - `MeshObject_dtor+0x1F` (DATA) — vtable lookup during destruction

### PTR_OBJ_VTABLE
- **Address**: `0x004F7360` (.data)
- **Type**: `pointer` (4 bytes, function pointer table)
- **Description**: Pointer to the base engine object vtable (at `0x004D8F88` in .rdata).
  Initialized at startup by `FUN_004ce500`. Used extensively (193 xrefs) for:
  - RNG operations (`RNG_Rand(&PTR_OBJ_VTABLE, ...)`) — contains the random number generator state
  - Object construction/destruction (sets vtable on new objects)
  - Scene camera setup, ball spawning, collision setup
  - Board level constructors, rumble board setup
  - Sound channel initialization
- **Initialized by**: `FUN_004ce500` → `PTR_OBJ_VTABLE = &PTR_LAB_004d8f88;`
- **Used in** (193 locations, key ones):
  - `Ball_ctor2+0x126` — `RNG_Rand(&PTR_OBJ_VTABLE, 3, '\0')` for random ball properties
  - `Ball_Update+0xFF` — per-frame ball physics
  - `Ball_Shatter+0x21F` — ball splitting mechanic
  - `Scene_SetCamera+0x1EA` — camera setup
  - `Scene_SpawnBallsAndObjects+0x174` — level object spawning
  - `Scene_UpdateBallsAndState+0x3E4` — main game logic update
  - `Scene_ComputeArenaLighting+0x202` — arena lighting
  - `Board_Master_Update+0x42` — master board update
  - `CreateBumper+0x7F3`, `HandleArenaCollisionEvents+0x3C0`, `DispatchCollisionEvents+0x7AC` — object creation
  - `ArenaBoard_Update+0x331` — arena mode update
  - `Sound_InitChannels+0x90` — audio system init
  - `LoadingScreenGadget_Ctor+0x2E2` — loading screen
  - `ScoreDisplay_SetTime+0x09` — score display
  - Many level dtor functions (LevelBoard_Dizzy_dtor, LevelBoard_Tower_dtor, etc.)

### PTR_PTR_004f7188
- **Address**: `0x004F7188` (.data)
- **Type**: `pointer` (4 bytes, pointer to math lookup table)
- **Description**: Pointer to the engine's trigonometric lookup table (at `0x004D8E5C` in .rdata).
  Used as the first argument to `Math_Atan2Angle()`, `Wave_Sin()`, and `Wave_Cos()` for fast
  angle and wave computations. Contains pre-computed sine/cosine/atan2 tables for
  performance (avoids FPU transcendental calls). 121 xrefs across physics, rendering, and camera.
- **Initialized by**: `FUN_004ce4f0` → `PTR_PTR_004f7188 = &PTR_LAB_004d8e5c;`
- **Used in** (121 locations, key ones):
  - `Ball_ApplyForceV2+0x12F` — `Math_Atan2Angle(&PTR_PTR_004f7188, x, z, 0, 0)` for movement direction
  - `Ball_ApplyForceWithMultipliers+0x12F` — same with force multipliers
  - `Ball_FallUpdate+0x44F` — falling physics
  - `Ball_Update+0x1F95` — main ball physics tick
  - `Scene_SetCamera+0x111` — `Wave_Sin(&PTR_PTR_004f7188, ...)` for camera orbit
  - `Scene_RenderWithCamera+0xBF` — render camera transform
  - `Scene_ComputeLighting+0xA5` — lighting calculation
  - `Sprite_DrawRotatedQuad+0x152` — sprite rotation
  - `Gear_AdvanceAlongPath+0x49A` — gear path following
  - `WaterRipple_UpdateVertices+0x48` — water effect

### PTR_PTR_004f7448
- **Address**: `0x004F7448` (.data)
- **Type**: `pointer` (4 bytes, pointer to string format table)
- **Description**: Pointer to a level/texture name format string table. Used by
  `AthenaString_Format(0x4f7448, &DAT_004d03f8)` to generate texture filenames for levels.
  Initialized to `&PTR_SoundBuffer_ScalarDtor_004d9124` by `FUN_004ce510` (static init).
  99 xrefs — primarily in UI/menu constructors and render functions for texture loading.
- **Initialized by**: `FUN_004ce510` → `PTR_PTR_004f7448 = &PTR_SoundBuffer_ScalarDtor_004d9124;`
- **Used in** (99 locations, key ones):
  - `TimeTrialMenu_ctor+0x6E` — `AthenaString_Format(0x4f7448, ...)` for level textures
  - `HighScoreMenu_Render+0x291` — high score display textures
  - `ArenaBoard_Render+0x218` — arena rendering
  - `DifficultyMenu_Render+0x252` — difficulty selection
  - `RaceGoalReached_Tick+0x297` — race completion
  - `Scene_SetupLevel10+0xBB` — Master level setup
  - `Scene_SetupLevelCascade+0xCA` — Beginner Race setup
  - `TourneyMenu_ctor+0x26F` — tournament menu construction
  - `MPMenu_ctor+0x356` — multiplayer menu
  - `Options_Menu_dtor+0x1C8` — options cleanup
  - `Ball_Update+0x1219` — ball rendering

### PTR_PTR_004f77c0
- **Address**: `0x004F77C0` (.data)
- **Type**: `pointer` (4 bytes, pointer to diagnostic data)
- **Description**: Pointer to a diagnostic data structure used by `App_BuildDiagnosticReport`.
  Initialized to `&PTR_LAB_004da74c` by `FUN_004ce550`. Used when building crash/error reports.
  Contains system information for the diagnostic report.
- **Initialized by**: `FUN_004ce550` → `PTR_PTR_004f77c0 = &PTR_LAB_004da74c;`
- **Used in**:
  - `App_BuildDiagnosticReport+0x4EF` (DATA)
  - `App_BuildDiagnosticReport+0x56C` (DATA)
  - `App_BuildDiagnosticReport+0x5E9` (DATA)
  - `App_BuildDiagnosticReport+0x666` (DATA)
  - `App_BuildDiagnosticReport+0x6E3` (DATA)

### s_BACK
- **Address**: `0x004D2334` (.rdata)
- **Type**: `char[5]` (string literal "BACK\0", Ghidra labels as `undefined1`)
- **Description**: The string literal `"BACK"`. Used as the label for the 'Back' / 'Previous Menu'
  button across all menus and UI screens. 45 xrefs — appears in every menu constructor and handler.
- **Used in** (45 locations, key ones):
  - `ArenaMenu_ctor+0x6BD` — arena selection back button
  - `TourneyMenu_ctor+0x2C2` — tournament menu back button
  - `KeyRemapMenu_Ctor+0x1E5` — key remapping back button
  - `ConfirmMenu_ctor+0x7A` — confirmation dialog back button
  - `CreditsScreen_ctor+0x2B7` — credits screen back button
  - `PracticeMenu_ctor+0xAF4` — practice level selection back button
  - `OptionsMenu_ctor+0x312` — options menu back button
  - `PauseMenu_Ctor+0xD6` — pause menu back button
  - `DifficultyMenu_ctor+0x10B` — difficulty menu back button
  - `PauseMenu_HandleButtonClick+0xB2` — back button click handler
  - `OptionsMenu_HandleButtonClick+0x481` — options back click handler
  - `QuitRace+0x10B` — quit race back button

---

## 2. D3D / Graphics Function Pointer Globals

These are function pointers set up at runtime for graphics operations. The D3D thunk system
uses indirect calls through these pointers to support multiple code paths (e.g., MMX vs scalar).

### PTR_D3DX_ShaderDispatch0
- **Address**: `0x004F7194`
- **Type**: `pointer` (4 bytes, function pointer)
- **Description**: Function pointer for D3DX shader dispatch (shader profile 0). Set by
  `D3DX_DetectShaderProfile()` based on CPU capabilities. Called indirectly to execute
  vertex/pixel shader operations.
- **Used in**:
  - `D3DX_DetectShaderProfile+0x22` (WRITE) — assignment
  - `D3DX_DetectShaderProfile+0x51` (WRITE) — assignment
  - `D3DX_ShaderDispatch0+0x0` (INDIRECTION) — indirect call
  - `D3DX_ShaderDispatch0+0x7` (INDIRECTION) — indirect call

### PTR_D3DX_ShaderDispatch1
- **Address**: `0x004F71B8`
- **Type**: `pointer` (4 bytes, function pointer)
- **Description**: Function pointer for D3DX shader dispatch profile 1.
- **Used in**: `D3DX_ShaderDispatch1` (2 refs)

### PTR_D3DX_ShaderDispatch2
- **Address**: `0x004F71CC`
- **Type**: `pointer` (4 bytes, function pointer)
- **Description**: Function pointer for D3DX shader dispatch profile 2.
- **Used in**: `D3DX_ShaderDispatch2` (2 refs)

### PTR_D3DX_ShaderDispatch_noarg
- **Address**: `0x004F71FC`
- **Type**: `pointer` (4 bytes, function pointer)
- **Description**: No-argument variant of D3DX shader dispatch.
- **Used in**: shader dispatch functions (2 refs)

### PTR_D3DX_ShaderDispatch_noarg2
- **Address**: `0x004F71F4`
- **Type**: `pointer` (4 bytes, function pointer)
- **Description**: Second no-argument variant of D3DX shader dispatch.
- **Used in**: shader dispatch functions (2 refs)

### PTR_D3DX_ShaderDispatch_noarg4
- **Address**: `0x004F721C`
- **Type**: `pointer` (4 bytes, function pointer)
- **Description**: Fourth no-argument variant of D3DX shader dispatch.
- **Used in**: shader dispatch functions (2 refs)

### PTR_D3DX_ShaderDispatch_noarg5
- **Address**: `0x004F71E8`
- **Type**: `pointer` (4 bytes, function pointer)
- **Description**: Fifth no-argument variant of D3DX shader dispatch.
- **Used in**: shader dispatch functions (2 refs)

### PTR_D3DX_ShaderDispatch_2a / 2b / 2c
- **Addresses**: `0x004F7208` / `0x004F720C` / `0x004F7210`
- **Type**: `pointer` (4 bytes each, function pointers)
- **Description**: D3DX shader dispatch variants 2a, 2b, 2c. Different parameter configurations
  for the shader execution pipeline.
- **Used in**: respective ShaderDispatch_2a/2b/2c functions (2 refs each)

### PTR_D3DX_ShaderDispatch_3
- **Address**: `0x004F7238`
- **Type**: `pointer` (4 bytes, function pointer)
- **Description**: D3DX shader dispatch variant 3.
- **Used in**: `D3DX_ShaderDispatch_3` (2 refs)

### PTR_D3DX_ShaderDispatch_4
- **Address**: `0x004F7214`
- **Type**: `pointer` (4 bytes, function pointer)
- **Description**: D3DX shader dispatch variant 4.
- **Used in**: `D3DX_ShaderDispatch_4` (2 refs)

### PTR_D3DX_ShaderDispatch_4b
- **Address**: `0x004F71F8`
- **Type**: `pointer` (4 bytes, function pointer)
- **Description**: D3DX shader dispatch variant 4b (alternative parameter format).
- **Used in**: shader dispatch functions (2 refs)

### PTR_Graphics_SetRenderState
- **Address**: `0x004F719C`
- **Type**: `pointer` (4 bytes, function pointer)
- **Description**: Function pointer for `Graphics_SetRenderState`. Called indirectly via
  `(*(code*)PTR_Graphics_SetRenderState_004f719c)();` to set D3D render states.
- **Used in**: `Graphics_SetRenderState` (2 refs)

### PTR_Graphics_InitShaderDispatch
- **Address**: `0x004F71AC`
- **Type**: `pointer` (4 bytes, function pointer)
- **Description**: Function pointer for graphics shader dispatch initialization.
- **Used in**: `Graphics_InitShaderDispatch` (2 refs)

### PTR_Matrix_TransformVec4x3
- **Address**: `0x004F7278`
- **Type**: `pointer` (4 bytes, function pointer)
- **Description**: Function pointer for 4x3 matrix vector transformation. Used by
  `D3DX_DetectShaderProfile()` to select between CPU-specific implementations.
- **Used in**: `D3DX_DetectShaderProfile` (4 refs — read and indirection)

### PTR_D3D_Thunk0 through PTR_D3D_Thunk_C
- **Addresses**: `0x004F722C` (Thunk0), `0x004F7230` (Thunk4), `0x004F7234` (Thunk5),
  `0x004F71D4` (Thunk_6), `0x004F724C` (Thunk_7), `0x004F7250` (Thunk_8),
  `0x004F7258` (Thunk_9), `0x004F725C` (Thunk_A), `0x004F7264` (Thunk_B),
  `0x004F7268` (Thunk_C)
- **Type**: `pointer` (4 bytes each, function pointers)
- **Description**: D3D device thunk function pointers. Each thunk detects the shader profile
  (`D3DX_DetectShaderProfile`) then calls through the stored pointer. Used to dispatch D3D
  device operations through the shader-accelerated code path. Each has 1 xref.

### PTR_D3DX_Thunk3Param
- **Address**: `0x004F7204`
- **Type**: `pointer` (4 bytes, function pointer)
- **Description**: D3DX thunk for 3-parameter operations.
- **Used in**: 1 xref (thunk dispatch)

### PTR_D3DThunk_DrawIndexedPrimitiveUP
- **Address**: `0x004F723C`
- **Type**: `pointer` (4 bytes, function pointer)
- **Description**: Function pointer for `DrawIndexedPrimitiveUP` D3D device operation.
  Dispatches indexed primitive drawing through the shader pipeline.
- **Used in**: 1 xref (thunk dispatch)

### PTR_D3DX_BoxFilter2x_Init / PTR_D3DX_BoxFilter2x_Init16
- **Addresses**: `0x004FA638` / `0x004FA63C`
- **Type**: `pointer` (4 bytes each, function pointers)
- **Description**: Function pointers for 2x box filter operations (texture downsampling).
  Set by `D3DX_BoxFilter2x_Init()` based on MMX availability:
  - No MMX: `D3DX_BoxFilter2x_Scalar` / `D3DX_BoxFilter2x_NoMMX`
  - With MMX: `D3DX_BoxFilter2x_MMX` / `D3DX_BoxFilter2x_MMX`
- **Used in**:
  - `D3DX_BoxFilter2x_Init+0x11/0x1D/0x32` (WRITE) — sets the function pointer
  - `D3DX_BoxFilter2x_Init16+0x11/0x18/0x27` (WRITE) — sets the 16-bit variant
  - `D3DX_BoxFilter2x_Dispatch+0x26` (READ) — calls through pointer
  - `D3DX_BoxFilter2x_Dispatch16+0x26` (READ) — calls through pointer

### PTR_D3DDevice_Reset (two entries)
- **Addresses**: `0x004DB360` / `0x004DB3D0` (.rdata)
- **Type**: `pointer` (4 bytes each)
- **Description**: D3D device vtable entries for the `Reset` method. These are entries
  in the COM vtable for the D3D9 device interface, used by `D3D_GetAdapterMode`.
- **Used in**: `D3D_GetAdapterMode` (2 refs each)

### PTR_COM_QueryInterface
- **Address**: `0x004DBEAC` (.rdata)
- **Type**: `pointer` (4 bytes)
- **Description**: COM `QueryInterface` vtable entry. Used by `COM_QueryInterface()` to
  query D3D COM interfaces for specific GUIDs.
- **Used in**: `COM_QueryInterface` (2 refs)

---

## 3. Resource Pool & Audio Globals

### PTR_PTR_004fa7d8
- **Address**: `0x004FA7D8`
- **Type**: `pointer` (4 bytes, pointer to vtable array)
- **Description**: Pointer to a vtable array for D3D resource pool objects. Used by
  `D3DResourcePool_Release()` to call virtual destructors on pooled D3D resources.
  Indexed by resource type (`puVar2[-0x40]`). Also used by the Vorbis decoder.
- **Used in**:
  - `D3DResourcePool_Release+0x58` (DATA) — `(**(code**)((&PTR_PTR_004fa7d8)[type] + 8))(obj);`
  - `Vorbis_InitDecodeChannel+0x175` (DATA)
  - `Vorbis_FreeDecoder+0x97` (DATA)
  - `Vorbis_ReadSetupHeader+0x1A6` (DATA)
  - `Vorbis_DecodeAudioFrame+0x139` (DATA)

### PTR_PTR_004fa7c4 / PTR_PTR_004fa7cc
- **Addresses**: `0x004FA7C4` / `0x004FA7CC`
- **Type**: `pointer` (4 bytes each)
- **Description**: Additional vtable arrays for D3D resource pool objects. Used alongside
  `PTR_PTR_004fa7d8` by `D3DResourcePool_Release()` to dispatch virtual destructors for
  different resource types. Also used by Vorbis decoder and channel freeing.
- **Used in**:
  - `D3DResourcePool_Release+0x88/+0xB8` (DATA)
  - `Vorbis_ReadSetupHeader+0xDD/+0x146` (DATA)
  - `Channel_FreePairArray+0x11C/+0x15C` (DATA)

---

## 4. Game Content / Level Data Globals

### PTR_s_WARM-UP_RACE_004f7080
- **Address**: `0x004F7080`
- **Type**: `pointer` (4 bytes, pointer to string array)
- **Description**: Pointer to an array of race level name strings. Indexed by level number
  in `TourneyMenu_GetRaceName`: `return (&PTR_s_WARM_UP_RACE_004f7080)[level_index];`
  Contains strings like "WARM-UP RACE", "BEGINNER RACE", etc.
- **Used in**: `TourneyMenu_GetRaceName+0x03` (1 xref)

### PTR_s_TAKE_YOUR_TIME_ON_THE_WARM-UP_RA_004f7148
- **Address**: `0x004F7148`
- **Type**: `pointer` (4 bytes, pointer to string array)
- **Description**: Pointer to an array of level description/tutorial strings. Contains
  helpful text like "TAKE YOUR TIME ON THE WARM-UP RACE" displayed when selecting levels.
- **Used in**: 1 xref (level selection display)

### PTR_s_HAMSTER_PELLET_004f70c8
- **Address**: `0x004F70C8`
- **Type**: `pointer` (4 bytes, pointer to string)
- **Description**: Pointer to the string "HAMSTER PELLET". Used in tournament/menu rendering
  for displaying hamster pellet-related UI elements (likely score or item display).
- **Used in**:
  - `ArenaLevelSelect_Render+0xB07` (DATA)
  - `ConfirmMenu_Render+0x75E` (DATA)
  - `TourneyMenu_TickWithRank+0x11F2` (DATA)
  - `HighScoreEntry_DeletingDtor+0x69E` (DATA)

### PTR_Rsrc_DLL_1_409[304949]
- **Address**: `0x004D5E88` (.rdata)
- **Type**: `pointer` (4 bytes, pointer to resource data)
- **Description**: Pointer to embedded DLL resource data (resource ID 304949). Used by
  `OptionsMenu_ApplySettings` for applying graphics/display settings. The embedded DLL
  is likely the eSellerate licensing engine or a D3D helper.
- **Used in**:
  - `OptionsMenu_ApplySettings+0x2B7/+0x30F/+0x496/+0x4EE` (DATA)

### PTR_RaceGoalReached_Render (two entries)
- **Addresses**: `0x004D6C70` / `0x004D6CB8` (.rdata)
- **Type**: `pointer` (4 bytes each)
- **Description**: Vtable pointers for the `ScoreObject` class (race goal objects).
  Assigned in `ScoreObject_ctor`: `*this = &PTR_RaceGoalReached_Render_004d6c70;`
  Contains virtual render function for race finish/goal markers.
- **Used in**:
  - `ScoreObject_ctor+0x??` (1 xref each) — vtable assignment

### PTR_Level_LoadCollision
- **Address**: `0x004D90C8` (.rdata)
- **Type**: `pointer` (4 bytes, function pointer)
- **Description**: Function pointer to `Level_LoadCollision`. Stored in a vtable for
  indirect calling during level collision mesh setup.
- **Used in**: `Level_LoadCollision` (1 xref)

### PTR_SceneObject_SetVisible
- **Address**: `0x004D935C` (.rdata)
- **Type**: `pointer` (4 bytes, function pointer)
- **Description**: Function pointer to `SceneObject_SetVisible`. Stored in a vtable for
  indirect calling when toggling object visibility.
- **Used in**: `SceneObject_SetVisible` (1 xref)

---

## 5. CRT / Runtime Globals

These are MSVC C Runtime (CRT) global variables used for CRT initialization,
exception handling, and locale management.

### PTR_PTR_004fc664
- **Address**: `0x004FC664`
- **Type**: `pointer` (4 bytes, pointer to `lconv` structure)
- **Description**: Pointer to the CRT locale conversion (`lconv`) structure. Used by
  `___free_lconv_num` and `___free_lconv_mon` to compare locale string pointers before
  freeing — only frees strings that are NOT the static defaults from the `lconv` struct.
  Also used by `CRT_GetLocalePtr`.
- **Used in**:
  - `___free_lconv_num+0x0B/+0x27` (READ)
  - `___free_lconv_mon+0x10/+0x2D/+0x4A/+0x67/+0x84/+0xA1/+0xBE` (READ)
  - `CRT_GetLocalePtr+0x00` (READ)

### PTR_PTR_004fc6bc
- **Address**: `0x004FC6BC`
- **Type**: `pointer` (4 bytes)
- **Description**: Additional locale structure pointer. Used by `___updatetlocinfo`
  (CRT thread locale info update).
- **Used in**: `___updatetlocinfo` (1 xref)

### PTR_s_R6009_-_not_enough_space_for_env_004fc85c
- **Address**: `0x004FC85C`
- **Type**: `pointer` (4 bytes, pointer to error string)
- **Description**: Pointer to the CRT runtime error message string for error R6009
  ("not enough space for environment"). Used by `CRT_RuntimeError` to display the
  appropriate error message when the runtime encounters an out-of-memory condition.
- **Used in**:
  - `CRT_RuntimeError+0xDE/+0x11B/+0x141/+0x147/+0x150` (READ/DATA)

### PTR_s_(null)_004fc840
- **Address**: `0x004FC840`
- **Type**: `pointer` (4 bytes, pointer to string)
- **Description**: Pointer to the string `"(null)"`. Used by CRT string formatting
  functions as a fallback for NULL string pointers during printf/sprintf operations.
- **Used in**: 2 xrefs in CRT string formatting

### PTR_ReturnZero_004fce88
- **Address**: `0x004FCE88`
- **Type**: `pointer` (4 bytes, function pointer)
- **Description**: Function pointer to a function that returns zero. Used by
  `FPU_WriteMathError` as a handler for FPU exception processing. Called as
  `iVar1 = (*(code*)PTR_ReturnZero_004fce88)(&local_28);`
- **Used in**:
  - `FPU_WriteMathError+0x11D/+0x1D8/+0x26B` (READ)

### PTR_terminate_004fc82c
- **Address**: `0x004FC82C`
- **Type**: `pointer` (4 bytes, function pointer)
- **Description**: C++ `std::terminate` handler function pointer. Used by the CRT
  exception handling system (`___InternalCxxFrameHandler`) when an unhandled exception occurs.
- **Used in**: `___InternalCxxFrameHandler` (1 xref)

### PTR_CRT_amsg_exit (6 entries)
- **Addresses**: `0x004FC810` through `0x004FC824`
- **Type**: `pointer` (4 bytes each)
- **Description**: CRT abort message exit handlers. Array of function pointers for
  different CRT abort error codes. Called when the CRT needs to terminate with a specific error.
- **Used in**: 2 xrefs each (CRT abort dispatch)

### PTR_CRT_InitSecurityCookie_004f7004
- **Address**: `0x004F7004`
- **Type**: `pointer` (4 bytes, function pointer)
- **Description**: CRT security cookie initialization function pointer. Part of the
  MSVC stack buffer overrun protection mechanism.
- **Used in**: 1 xref (CRT init)

### PTR____onexitinit_004f7030
- **Address**: `0x004F7030`
- **Type**: `pointer` (4 bytes, function pointer)
- **Description**: CRT `atexit`/`onexit` initialization function pointer.
- **Used in**: 1 xref (CRT init)

### PTR____endstdio_004f704c
- **Address**: `0x004F704C`
- **Type**: `pointer` (4 bytes, function pointer)
- **Description**: CRT stdio cleanup function pointer. Called during CRT shutdown
  to flush and close standard I/O streams.
- **Used in**: 1 xref (CRT shutdown)

### PTR_CRT_SetUnhandledExceptionFilter_004f7058
- **Address**: `0x004F7058`
- **Type**: `pointer` (4 bytes, function pointer)
- **Description**: CRT unhandled exception filter setup function pointer.
- **Used in**: 1 xref (CRT init)

### PTR_CRT_InitFPState_004fc458
- **Address**: `0x004FC458`
- **Type**: `pointer` (4 bytes, function pointer)
- **Description**: CRT floating-point unit state initialization function pointer.
- **Used in**: 1 xref (CRT init)

### PTR___exit_004fc484
- **Address**: `0x004FC484`
- **Type**: `pointer` (4 bytes, function pointer)
- **Description**: CRT exit function pointer.
- **Used in**: 1 xref (CRT shutdown)

### s_0123456789ABCDEF_004f780c
- **Address**: `0x004F780C`
- **Type**: `char[17]` (string literal "0123456789ABCDEF\0")
- **Description**: Hexadecimal digit lookup table (uppercase). Used by `CRT_FormatInteger`
  when format specifier is `%X` (uppercase hex). Each character's index = its hex value.
- **Used in**:
  - `CRT_FormatInteger+0x0F` (DATA)
  - `CRT_FormatInteger+0x55` (DATA)
  - `CRT_FormatInteger+0x7A` (DATA)

### s_0123456789abcdef_004f77f8
- **Address**: `0x004F77F8`
- **Type**: `char[17]` (string literal "0123456789abcdef\0")
- **Description**: Hexadecimal digit lookup table (lowercase). Used by `CRT_FormatInteger`
  when format specifier is `%x` (lowercase hex).
- **Used in**:
  - `CRT_FormatInteger+0x55` (DATA)
  - `CRT_FormatInteger+0x7A` (DATA)

### s__004f76b8
- **Address**: `0x004F76B8`
- **Type**: `byte[49]` (lookup table)
- **Description**: Base32 decoding lookup table. Used by `Base32_Decode` to map
  Base32 characters to their decoded values: `bVar2 = s__004f76b8[*param_2];`
  Contains values 0x20 for invalid characters (whitespace). Part of the license key
  validation system (`LicenseKey_Validate`).
- **Used in**:
  - `Base32_Decode+0x1E` (DATA)
  - `Base32_Encode` (1 xref)

---

## 6. Licensing / eSellerate Globals

These globals support the eSellerate DRM/licensing system embedded in the game.

### s_\\eSellerateEngine.dll_004f74b0
- **Address**: `0x004F74B0`
- **Type**: `char[22]` (string literal)
- **Description**: Filename string `"\eSellerateEngine.dll"`. Used by
  `eSellerate_ExtractDLLNull` to locate and extract the eSellerate licensing DLL.
- **Used in**: `eSellerate_ExtractDLL` (1 xref)

### s_Software\\eSellerate\\Affiliates\\%_004f74cc
- **Address**: `0x004F74CC`
- **Type**: `char[37]` (string literal)
- **Description**: Registry path template `"Software\eSellerate\Affiliates\%s"`.
  Used for reading/writing eSellerate affiliate data in the Windows registry.
- **Used in**: `eSellerate_ExtractDLL` (1 xref)

### s_RegCloseKey / s_RegQueryValueExA / s_RegOpenKeyExA / s_advapi32
- **Addresses**: `0x004F74F4` / `0x004F7500` / `0x004F7514` / `0x004F7524`
- **Type**: `char[]` (string literals)
- **Description**: Windows API function name strings used by `eSellerate_ExtractDLL`
  to dynamically load registry functions from advapi32.dll via `GetProcAddress`.
  Strings: `"RegCloseKey"`, `"RegQueryValueExA"`, `"RegOpenKeyExA"`, `"advapi32"`.
- **Used in**: `eSellerate_ExtractDLL` (1 xref each)

### s_MVESD_Entry2_004f77d4 / s_eSellerateEngine_004f77e4
- **Addresses**: `0x004F77D4` / `0x004F77E4`
- **Type**: `char[]` (string literals)
- **Description**: eSellerate engine entry point and module name strings.
  `"MVESD_Entry2"` is the DLL export function name; `"eSellerateEngine"` is the module name.
- **Used in**: 1 xref each (eSellerate init)

### PTR_s_http://bugs.raptisoft.com / PTR_s_RaptisoftBugTracker
- **Addresses**: `0x004F77C4` / `0x004F77C8`
- **Type**: `pointer` (4 bytes each)
- **Description**: Pointers to the Raptisoft bug tracker URL string and name.
  Used when building diagnostic/crash reports to include the bug submission URL.
- **Used in**: 1 xref each (diagnostic report)

---

## 7. RTTI / Exception Handling Globals

### vftable @ 0x004e9b44
- **Address**: `0x004E9B44` (.rdata)
- **Type**: `pointer` (4 bytes, RTTI vtable)
- **Description**: RTTI type info vtable. Used by `TypeInfo_Dtor` for destroying
  C++ type_info objects. Referenced by multiple RTTI Type Descriptor structures.
- **Used in**:
  - `TypeInfo_Dtor+0x5` (DATA)
  - RTTI Type Descriptor entries at `0x004F7450`, `0x004F7468`, `0x004F7488`,
    `0x004FC414`, `0x004FC434`, `0x004FC4A0`

### PTR_RTTI_Type_Descriptor entries
- **Addresses**: `0x004EE664`, `0x004EE67C`, `0x004EE6C4`, `0x004EE710` (.rdata)
- **Type**: `pointer` (4 bytes each)
- **Description**: RTTI Type Descriptor pointers. Used by the C++ runtime for
  `dynamic_cast` and `typeid` operations. Point to TypeDescriptor structures that
  describe class type information.
- **Used in**: 1-3 xrefs each (RTTI lookup)

### PTR_PTR_004f7820
- **Address**: `0x004F7820`
- **Type**: `pointer` (4 bytes)
- **Description**: Pointer to a CRT string table entry. Initialized to
  `&PTR_LAB_004db4dc` by `FUN_004ce570`. Used by `AthenaString_AssignCRLF`
  for CRLF string assignment operations.
- **Initialized by**: `FUN_004ce570` → `PTR_PTR_004f7820 = &PTR_LAB_004db4dc;`
- **Used in**:
  - `AthenaString_AssignCRLF+0x??` (1 xref)
  - `FUN_004ce570` (WRITE, static init)

---

## 8. Miscellaneous Globals

### DWORD_004f5b80
- **Address**: `0x004F5B80`
- **Type**: `dword` (4 bytes, signed integer)
- **Description**: Referenced by `sub_004001a0` (CRT startup code in the PE header region).
  Likely a CRT initialization flag or security cookie value.
- **Used in**: `sub_004001a0` (1 xref)

### PTR_s_Bogus_message_code_%d_004e42d8
- **Address**: `0x004E42D8` (.rdata)
- **Type**: `pointer` (4 bytes)
- **Description**: Pointer to format string `"Bogus message code %d"`. Used for
  displaying unknown/invalid Windows message codes in the window procedure.
- **Used in**: 1 xref (window message handler)

### u_null)_004e9e86
- **Address**: `0x004E9E86` (.rdata)
- **Type**: `wchar16[6]` (Unicode string)
- **Description**: Unicode string `"(null)"`. Used as a fallback for NULL wide-string
  pointers in CRT wide-character formatting functions.
- **Used in**: 1 xref (CRT wide string formatting)

---

## Summary Table

| # | Name | Address | Type | Size | Xrefs | Category |
|---|------|---------|------|------|-------|----------|
| 1 | PTR_OBJ_VTABLE | 0x004F7360 | pointer | 4 | 193 | Engine (RNG/object vtable) |
| 2 | PTR_PTR_004f7188 | 0x004F7188 | pointer | 4 | 121 | Engine (trig lookup table) |
| 3 | PTR_PTR_004f7448 | 0x004F7448 | pointer | 4 | 99 | Engine (texture format table) |
| 4 | s_BACK | 0x004D2334 | char[5] | 1* | 45 | UI (back button label) |
| 5 | PTR_PTR_004fc664 | 0x004FC664 | pointer | 4 | 11 | CRT (locale lconv) |
| 6 | PTR_PTR_004f77c0 | 0x004F77C0 | pointer | 4 | 6 | Engine (diagnostic data) |
| 7 | PTR_D3DX_BoxFilter2x_Init | 0x004FA638 | pointer | 4 | 6 | Graphics (box filter) |
| 8 | PTR_PTR_004fa7d8 | 0x004FA7D8 | pointer | 4 | 5 | Audio/Graphics (vtable array) |
| 9 | PTR_s_R6009 | 0x004FC85C | pointer | 4 | 5 | CRT (error string) |
| 10 | g_App | 0x004FD680 | App struct | 2328 | 5 | Engine (main app object) |
| 11 | PTR_s_HAMSTER_PELLET | 0x004F70C8 | pointer | 4 | 4 | UI (string pointer) |
| 12 | PTR_D3DX_ShaderDispatch0 | 0x004F7194 | pointer | 4 | 4 | Graphics (shader dispatch) |
| 13 | PTR_Matrix_TransformVec4x3 | 0x004F7278 | pointer | 4 | 4 | Graphics (matrix ops) |
| 14 | PTR_D3DX_BoxFilter2x_Init16 | 0x004FA63C | pointer | 4 | 4 | Graphics (box filter 16) |
| 15 | s_0123456789ABCDEF | 0x004F780C | char[17] | 17 | 3 | CRT (hex lookup) |
| 16 | PTR_PTR_004fa7c4 | 0x004FA7C4 | pointer | 4 | 3 | Audio (vtable array) |
| 17 | PTR_PTR_004fa7cc | 0x004FA7CC | pointer | 4 | 3 | Audio (vtable array) |
| 18 | PTR_ReturnZero | 0x004FCE88 | pointer | 4 | 3 | CRT (FPU error handler) |
| 19 | g_renderIndex | 0x005341CC | uint32 | 4 | 3 | Engine (render counter) |
| 20 | PTR_Graphics_SetRenderState | 0x004F719C | pointer | 4 | 2 | Graphics (render state) |
| 21 | PTR_Graphics_InitShaderDispatch | 0x004F71AC | pointer | 4 | 2 | Graphics (shader init) |
| 22 | PTR_D3DX_ShaderDispatch1 | 0x004F71B8 | pointer | 4 | 2 | Graphics (shader dispatch) |
| 23 | PTR_D3DX_ShaderDispatch2 | 0x004F71CC | pointer | 4 | 2 | Graphics (shader dispatch) |
| 24 | PTR_D3DX_ShaderDispatch_noarg5 | 0x004F71E8 | pointer | 4 | 2 | Graphics (shader dispatch) |
| 25 | PTR_D3DX_ShaderDispatch_noarg2 | 0x004F71F4 | pointer | 4 | 2 | Graphics (shader dispatch) |
| 26 | PTR_D3DX_ShaderDispatch_4b | 0x004F71F8 | pointer | 4 | 2 | Graphics (shader dispatch) |
| 27 | PTR_D3DX_ShaderDispatch_noarg | 0x004F71FC | pointer | 4 | 2 | Graphics (shader dispatch) |
| 28 | PTR_D3DX_ShaderDispatch_2a | 0x004F7208 | pointer | 4 | 2 | Graphics (shader dispatch) |
| 29 | PTR_D3DX_ShaderDispatch_2b | 0x004F720C | pointer | 4 | 2 | Graphics (shader dispatch) |
| 30 | PTR_D3DX_ShaderDispatch_2c | 0x004F7210 | pointer | 4 | 2 | Graphics (shader dispatch) |
| 31 | PTR_D3DX_ShaderDispatch_4 | 0x004F7214 | pointer | 4 | 2 | Graphics (shader dispatch) |
| 32 | PTR_D3DX_ShaderDispatch_noarg4 | 0x004F721C | pointer | 4 | 2 | Graphics (shader dispatch) |
| 33 | PTR_D3DX_ShaderDispatch_3 | 0x004F7238 | pointer | 4 | 2 | Graphics (shader dispatch) |
| 34 | s__004f76b8 | 0x004F76B8 | byte[49] | 49 | 2 | Licensing (Base32 table) |
| 35 | s_0123456789abcdef | 0x004F77F8 | char[17] | 17 | 2 | CRT (hex lookup) |
| 36 | PTR_PTR_004f7820 | 0x004F7820 | pointer | 4 | 2 | CRT (string table) |
| 37 | PTR_s_(null) | 0x004FC840 | pointer | 4 | 2 | CRT (null string fallback) |
| 38 | PTR_s_WARM-UP_RACE | 0x004F7080 | pointer | 4 | 1 | Game (level names) |
| 39 | PTR_s_TAKE_YOUR_TIME | 0x004F7148 | pointer | 4 | 1 | Game (level descriptions) |
| 40 | PTR_D3D_Thunk_6 | 0x004F71D4 | pointer | 4 | 1 | Graphics (D3D thunk) |
| 41 | PTR_D3DX_Thunk3Param | 0x004F7204 | pointer | 4 | 1 | Graphics (D3D thunk) |
| 42 | PTR_D3D_Thunk0 | 0x004F722C | pointer | 4 | 1 | Graphics (D3D thunk) |
| 43 | PTR_D3D_Thunk4 | 0x004F7230 | pointer | 4 | 1 | Graphics (D3D thunk) |
| 44 | PTR_D3D_Thunk5 | 0x004F7234 | pointer | 4 | 1 | Graphics (D3D thunk) |
| 45 | PTR_D3DThunk_DrawIndexedPrimUP | 0x004F723C | pointer | 4 | 1 | Graphics (D3D thunk) |
| 46 | PTR_D3D_Thunk_7 through _C | 0x4F724C-4F7268 | pointer | 4 each | 1 each | Graphics (D3D thunks) |
| 47 | s_\eSellerateEngine.dll | 0x004F74B0 | char[22] | 22 | 1 | Licensing (DLL name) |
| 48 | s_Software\eSellerate\... | 0x004F74CC | char[37] | 37 | 1 | Licensing (reg path) |
| 49 | s_RegCloseKey | 0x004F74F4 | char[12] | 12 | 1 | Licensing (API name) |
| 50 | s_RegQueryValueExA | 0x004F7500 | char[17] | 17 | 1 | Licensing (API name) |
| 51 | s_RegOpenKeyExA | 0x004F7514 | char[14] | 14 | 1 | Licensing (API name) |
| 52 | s_advapi32 | 0x004F7524 | char[9] | 9 | 1 | Licensing (DLL name) |
| 53 | PTR_s_http://bugs.raptisoft | 0x004F77C4 | pointer | 4 | 1 | Misc (bug tracker URL) |
| 54 | PTR_s_RaptisoftBugTracker | 0x004F77C8 | pointer | 4 | 1 | Misc (bug tracker name) |
| 55 | s_MVESD_Entry2 | 0x004F77D4 | char[13] | 13 | 1 | Licensing (entry point) |
| 56 | s_eSellerateEngine | 0x004F77E4 | char[17] | 17 | 1 | Licensing (module name) |
| 57 | PTR_PTR_004fc6bc | 0x004FC6BC | pointer | 4 | 1 | CRT (locale update) |
| 58 | PTR_terminate | 0x004FC82C | pointer | 4 | 1 | CRT (std::terminate) |
| 59 | MeshWorld_vtable | 0x004D9CDC | pointer | 4 | 2 | Engine (MeshWorld vtable) |
| 60 | vftable | 0x004E9B44 | pointer | 4 | 7 | RTTI (type info vtable) |
| 61 | PTR_Rsrc_DLL_1_409 | 0x004D5E88 | pointer | 4 | 4 | Resource (embedded DLL) |
| 62 | PTR_RaceGoalReached_Render (x2) | 0x4D6C70/4D6CB8 | pointer | 4 each | 1 each | Game (ScoreObject vtable) |
| 63 | PTR_Level_LoadCollision | 0x004D90C8 | pointer | 4 | 1 | Game (collision func ptr) |
| 64 | PTR_SceneObject_SetVisible | 0x004D935C | pointer | 4 | 1 | Game (visibility func ptr) |
| 65 | PTR_D3DDevice_Reset (x2) | 0x4DB360/4DB3D0 | pointer | 4 each | 2 each | Graphics (D3D vtable) |
| 66 | PTR_COM_QueryInterface | 0x004DBEAC | pointer | 4 | 2 | Graphics (COM vtable) |
| 67 | DWORD_004f5b80 | 0x004F5B80 | dword | 4 | 1 | CRT (init flag) |
| 68 | PTR_s_Bogus_message_code | 0x004E42D8 | pointer | 4 | 1 | Misc (error string) |
| 69 | u_null) | 0x004E9E86 | wchar16[6] | 12 | 1 | CRT (unicode null) |
| 70 | PTR_CRT_InitSecurityCookie | 0x004F7004 | pointer | 4 | 1 | CRT (security cookie) |
| 71 | PTR____onexitinit | 0x004F7030 | pointer | 4 | 1 | CRT (atexit init) |
| 72 | PTR____endstdio | 0x004F704C | pointer | 4 | 1 | CRT (stdio cleanup) |
| 73 | PTR_CRT_SetUnhandledExFilter | 0x004F7058 | pointer | 4 | 1 | CRT (exception filter) |
| 74 | PTR_CRT_InitFPState | 0x004FC458 | pointer | 4 | 1 | CRT (FPU init) |
| 75 | PTR___exit | 0x004FC484 | pointer | 4 | 1 | CRT (exit) |
| 76 | PTR_CRT_amsg_exit (x6) | 0x4FC810-4FC824 | pointer | 4 each | 1-2 each | CRT (abort handlers) |
| 77 | PTR_RTTI_Type_Descriptor (x4) | 0x4EE664-4EE710 | pointer | 4 each | 1-3 each | RTTI (type descriptors) |

> *s_BACK is labeled as 1 byte by Ghidra but is actually a 5-byte string ("BACK\0").

---

## Excluded from this document

The following categories of global data were excluded as they are not game-specific:
- **TEB (Thread Environment Block)**: ~70 entries at `0xFFDFF000-0xFFDFFFFF` (system TEB fields)
- **IAT (Import Address Table)**: ~177 entries at `0x004CF000-0x004CF2FF` (Windows API import thunks)
- **String constants**: ~1666 string literals in `.rdata` (level names, object names, error messages, etc.)
- **Vtable destructor pointers**: ~218 `PTR_*_Dtor` entries in `.rdata` (C++ virtual destructor function pointers)
- **FuncInfo/UnwindMapEntry**: ~400+ entries (MSVC exception handling tables)
- **Resource data**: Rsrc_ entries (icons, cursors, embedded DLL data)
- **TypeDescriptor (RTTI)**: 6 entries in `.data` (C++ RTTI type descriptors)
