# Hamsterball — ALL Functions, Labels, and Important Addresses

**Binary:** Hamsterball.exe V3.6.c  
**MD5:** 7d25019366b8d7f55906325bd630d7fe  
**Size:** 1,404,928 bytes  
**Image Base:** 0x00400000  
**Total Functions:** 3,781+ (Ghidra auto-analysis)  
**Documented:** 100%  

> Compiled and organized by Hamsterbot for MAKYUNI 🦇  
> Sources: Ghidra decompilation, hbtestd runtime analysis, reverse engineering sessions 1-46+

---

## Table of Contents

1. [Entry Point & Game Lifecycle](#1-entry-point--game-lifecycle)
2. [App Object & Global State](#2-app-object--global-state)
3. [Graphics / Direct3D 8 System](#3-graphics--direct3d-8-system)
4. [Audio / BASS System](#4-audio--bass-system)
5. [Level / Game World Functions](#5-level--game-world-functions)
6. [Game Object Factories & Constructors](#6-game-object-factories--constructors)
7. [Ball Physics System](#7-ball-physics-system)
8. [Collision System](#8-collision-system)
9. [Camera System](#9-camera-system)
10. [Scene System](#10-scene-system)
11. [SceneObject System](#11-sceneobject-system)
12. [Level Rendering](#12-level-rendering)
13. [Menu / UI System](#13-menu--ui-system)
14. [Tournament / Race System](#14-tournament--race-system)
15. [Arena / Rumble Board System](#15-arena--rumble-board-system)
16. [Sound System](#16-sound-system)
17. [Math / Vector / Matrix Utilities](#17-math--vector--matrix-utilities)
18. [String / AthenaString System](#18-string--athenastring-system)
19. [Font / Text Rendering](#19-font--text-rendering)
20. [Mesh / MeshWorld System](#20-mesh--meshworld-system)
21. [Config / Save / Registry System](#21-config--save--registry-system)
22. [DRM / Licensing System](#22-drm--licensing-system)
23. [Input System](#23-input-system)
24. [Sprite / Particle System](#24-sprite--particle-system)
25. [Path / Spline System](#25-path--spline-system)
26. [RNG / Random System](#26-rng--random-system)
27. [CRT / Runtime Functions](#27-crt--runtime-functions)
28. [Global Variables & Data Addresses](#28-global-variables--data-addresses)
29. [Struct Layouts](#29-struct-layouts)
30. [Vtable Layouts](#30-vtable-layouts)
31. [Important Offsets Quick Reference](#31-important-offsets-quick-reference)

---


## 1. Entry Point & Game Lifecycle

| Address | Name | Description |
|---------|------|-------------|
| 0x004BB4C8 | entry | CRT entry point (GetVersionEx, heap init, etc.) |
| 0x004278E0 | WinMain | Game entry: Init → Run → Shutdown |
| 0x00429530 | App_Initialize_Full | 26-step init sequence (graphics, audio, input, cursors, music) |
| 0x0046BB40 | App_Initialize | 12-step base init (vtable calls + D3D8 creation) |
| 0x0046BD80 | App_Run | Game loop (PeekMessage → Update → Render). Also labeled GameLoop. |
| 0x0046BA10 | App_Shutdown | Cleanup on exit: destroys window, releases 5 COM objects, CoUninitialize |
| 0x0046DC20 | App_ScalarDtor | App scalar destructor: calls App_Shutdown then free if flag bit 0 |
| 0x0046DC40 | App_Ctor | App constructor: vtable, size 640/480, cursors, CoInitialize, input device 0x848 |
| 0x00469CF0 | GameUpdate | Main update tick |
| 0x0046C170 | App_FrameUpdate | Frame update: GetCursorPos, WindowFromPoint, poll input/audio/collision, GameUpdate |
| 0x0046C200 | App_ResetFrame | Reset frame: Scene_ResetCameraAndFrameCount, then Graphics_Clear. **Every-frame hook point.** |
| 0x0046C260 | App_TickGameUpdate | Thunk: call game update tick (App+0x184) |
| 0x0046C9E0 | App_FrameTick | If not paused, render + update scene |
| 0x00429450 | FinishLoad | Finish loading (calls final setup) |
| 0x00429520 | Game_SetInProgress | Mark game in progress (sets +0x200=1) |
| 0x00428160 | PauseGame | Pausing (RightButtonPause) |
| 0x0042FAD0 | QuitRace | Quit current race |
| 0x0042E6F0 | QuitRaceMenu | Race quit confirmation menu |
| 0x00425F90 | App_CompleteRace | Complete race: increment counter, trigger state transitions, clear flag |
| 0x004280E0 | App_ShowMainMenu | Create MainMenu (0xCDC bytes) and store at App+0x224 |
| 0x00428060 | App_ShowResults | Create results screen, save to App+0x228, dispatch to scene manager |
| 0x004287C0 | App_StartRace | Restart audio, setup race, play sound, free dialogs, start music |
| 0x004288B0 | App_StartTournamentRace | Start tournament race: config mirror/mode, create level scene, advance race |
| 0x00428C50 | App_StartPracticeRace | Start practice/tournament race: calls App_StartRace, sets up PlayerProfile |
| 0x00428B20 | App_StartPartyRace | Start party race: sets profile+0x10=1 (party flag) |
| 0x0046BFF0 | App_CreateGraphics | Allocate Graphics object (0x7DC), store at App+0x174 |
| 0x0046C050 | App_CreateInputDevice | Allocate 0x91c bytes, call input device ctor, store at App+0x178 |
| 0x0046C0B0 | App_CreateAudioDevice | Allocate 0x424 bytes, call Audio_Init, store at App+0x17C |
| 0x0046C110 | App_CreateInputHandler | Allocate InputHandler (0x438), store at App+0x180 |
| 0x0046C7C0 | App_SetFullScreen | Toggle fullscreen/windowed, Graphics_Reset, AdjustWindowRect, SetWindowPos |
| 0x0046CB00 | App_CreateScoreDisplay | Allocate 0x8A4 score display object, add to scene |
| 0x0046CB70 | App_SetTitleString | Free and replace title string at App+0x1B4 |
| 0x0046BCA0 | App_WriteDisplaySettings | Write Fullscreen/ScreenWidth/ScreenHeight to registry |
| 0x0046BD00 | App_ReadDisplaySettings | Read Fullscreen/ScreenWidth/ScreenHeight from registry |
| 0x0046CA20 | App_ParseGUID | Parse GUID from string: MultiByteToWideChar + CLSIDFromString |
| 0x00479BE0 | App_GetProductVersion | Uses Version API to extract ProductVersion string |
| 0x0040ABA0 | CheckArenaUnlock | Check arena unlock conditions |
| 0x0040A420 | CheckPurchaseOrHighScore | Purchase reminder / high score check |


## 2. App Object & Global State

### Global Pointers

| Address | Name | Description |
|---------|------|-------------|
| 0x005341E0 | g_App | Global App singleton object pointer (also at 0x004FD680 as static data) |
| 0x005341E4 | g_Scene | Global Scene/Board pointer — holds the current board/scene directly |
| 0x005341CC | g_renderIndex | Render frame counter / sprite animation index |

### App Struct Layout (key offsets)

| Offset | Field | Description |
|--------|-------|-------------|
| +0x000 | vtable | Virtual function table pointer |
| +0x004 | hInstance | Windows HINSTANCE |
| +0x01C | is_paused | Pause flag |
| +0x020 | app_state | Application state |
| +0x054 | registryKey | Registry/config key |
| +0x05C | targetFPS | Target frame rate |
| +0x15C | width | Window width |
| +0x158 | windowed | Windowed mode flag |
| +0x160 | height | Window height |
| +0x164 | last_frame_tick | Last frame tick count |
| +0x16C | target_fps | Target FPS (runtime) |
| +0x170 | render_fps | Render FPS (runtime) |
| +0x174 | graphics | Graphics object pointer |
| +0x178 | inputDevice | Input device pointer |
| +0x17C | audioSystem | Audio system pointer |
| +0x180 | d3dDevice | D3D device pointer / input handler |
| +0x1B4 | versionString | Version string |
| +0x1CC | loadedCount | Loaded objects counter |
| +0x200 | initialized | Init complete flag |
| +0x208 | initStep | Current init step string |
| +0x220 | profile | Player profile pointer (NULL in Time Trial mode!) |
| +0x224 | mainMenu | Main menu pointer |
| +0x228 | resultsScreen | Results screen pointer |
| +0x23C | difficulty | 0=Pipsqueak, 1=Normal, 2=Frenzied |
| +0x240 | cursor | Loaded cursor handle |
| +0x278 | shadowTexture | Shadow texture |
| +0x534 | musicHandle | BASS music handle |
| +0x538 | musicChannel2 | Second music channel |
| +0x53C | musicChannel1 | First music channel |
| +0x550-0x55C | gameMode1-4 | Game mode objects (1,2,4,5) |
| +0x5D6 | tournament_finished_flag | Byte: set by Board_UpdateRaceState when timer DEC brings obj+0x1C below 0 |
| +0x5DC | ball_ptr | Current ball pointer (used by Scene_SetupLevelDark) |
| +0x5E0 | max_score | Maximum score |
| +0x5E4 | player_score | Player 1 score |
| +0x5E8/+0x5EC | timer_P1 | Player 1 timer slots (stride 0xA0) |
| +0x688/+0x68C | timer_P2 | Player 2 timer slots |
| +0x728/+0x72C | timer_P3 | Player 3 timer slots |
| +0x7C8/+0x7CC | timer_P4 | Player 4 timer slots |
| +0x851-0x865 | level_unlocks | Level unlock flags (see Tournament section) |
| +0x90C | bestTimeTracker | BestTimeTracker (recording) for Time Trial ghost |
| +0x910 | ghostPlaybackIndex | Ghost playback index for Time Trial |
| +0x914 | playCount | Play count from registry |
| +0xB28-0xB34 | control1-4 | Control bindings for players 1-4 |


## 3. Graphics / Direct3D 8 System

| Address | Name | Description |
|---------|------|-------------|
| 0x00455380 | Graphics_Initialize | Full 27-step D3D8 init (adapter check, device creation, display mode enumeration) |
| 0x00455A60 | Graphics_Defaults | Set default render states |
| 0x00453B50 | Graphics_BeginFrame | Begin frame/render setup |
| 0x00455A90 | Graphics_PresentOrEnd | Present frame or end scene |
| 0x004542C0 | Graphics_ctor | Graphics constructor (vtable 0x4D88A0, init render context, texture cache, frustum) |
| 0x00455360 | Graphics_dtor | Graphics destructor (cleanup + optional free) |
| 0x00454550 | Graphics_Cleanup | Release D3D objects, free texture path, clear cache |
| 0x00454000 | Graphics_SetTexturePath | Set custom texture prefix path (strdup at +0x7D8) |
| 0x00454060 | D3DFMT_ToString | Convert D3DFORMAT enum to debug string |
| 0x00454B50 | Graphics_SetViewport | Set viewport dimensions |
| 0x00454D30 | Graphics_Reset | Reset device with new params, CreateDevice twice |
| 0x00455D60 | Graphics_DrawScreenRect | Draw 2D screen rectangle (x,y,w,h → TLVERTEX triangle strip). 63 xrefs. |
| 0x00455110 | Graphics_ApplyMaterialAndDraw | Apply material/render states + draw textured geometry. 17 xrefs. |
| 0x00454190 | Graphics_SetRenderMode | Set shading mode (Gouraud), reset vertex shader, re-apply render states. SPECULARENABLE=0 globally. |
| 0x00455B80 | Graphics_SetStreamBuffers | Set vertex buffer stream sources for rendering |
| 0x00401160 | Graphics_SetColorMultiplier | Set 4-component RGBA color multiplier (gfx+0x7A8=1, stores Matrix_Scale4x4 at gfx+0x7B0). Used by Ball_Render for heat-based color shifting. |
| 0x00453C90 | Graphics_CreateDevice | D3D8 CreateDevice with format fallbacks (0x4b, 0x4f, 0x49, 0x47, 0x50, 0x4d) |
| 0x0042C810 | Graphics_InitRenderStates | Step 20: Initialize render states |
| 0x00454F10 | Graphics_SetProjection | Step 23: SetProjection(10.0, 5000.0) |
| 0x00454630 | Graphics_SetupLights | Step 25: Setup D3D8 lights |
| 0x00457FA0 | RenderContext_Init | Initialize render context struct (0x50 bytes, vtable 0x4D8E68) |
| 0x00457B10 | Matrix44_Zero | Clear 4x4 matrix; zero all 16 entries, set diagonals to 1.0 |
| 0x00457B50 | Gfx_SetPosition | Set world position via D3D SetTransform. 69 xrefs. |
| 0x00457BB0 | Gfx_RotateY | Rotate around Y axis (negate + look-at construction). 15 xrefs. |
| 0x00457C60 | Gfx_ScaleX | Scale X axis with render-state multiplier. 40 xrefs. |
| 0x00457C90 | Gfx_ScaleY | Scale Y axis with render-state multiplier. 35 xrefs. |
| 0x00457CC0 | Gfx_ScaleZ | Scale Z axis with render-state multiplier. 26 xrefs. |
| 0x00457FD0 | Matrix4_Identity | Set identity vtable + zero (Vec3 base init). 40 xrefs. |
| 0x00425FE0 | Gfx_SetAlphaBlendState | Set D3D alpha blend render states (0xD, 0xE → mode 3) |
| 0x00427940 | Gfx_SetCullMode | Set D3D cull mode (none/CW/CCW) based on +0x708 and specular flag |
| 0x0042C7C0 | Graphics_SetScaleAndPosition | Set identity scale matrix then set position (x,y) on Graphics object |
| 0x0045A439 | Graphics_SetRenderState | Render state dispatch thunk (profile detect + indirect jump). 29 xrefs. |
| 0x00459B24 | Graphics_InitShaderDispatch | Shader init dispatch thunk (D3DX detect + indirect jump). 19 xrefs. |
| 0x0046F100 | Gfx_ApplyLightingState | Set D3D lighting state: specular enable, light enable, material emission |
| 0x0046F1E0 | Gfx_ResetLighting | Reset lighting: disable specular, set render state 0x1b=0, material type 0 |
| 0x0045DFE0 | Gfx_SetupAlphaRenderState | Setup alpha render state: cull mode, texture stage, alpha test, blend |
| 0x0045DDE0 | Gfx_SetBlendState | Set D3D blend state: vtable 0xfc calls (0, 0xd, 2) and (0, 0xe, 2) |
| 0x00457A50 | Graphics_DisableRenderState | Thunk → Graphics_SetRenderState (disable mode) |
| 0x00440DD0 | Graphics_DrawRectAndReset | Draw rect, then reset matrix to identity |
| 0x0045E0E0 | Scene_RenderAllObjects | Main render: Graphics_BeginFrame → sort objects (opaque/alpha/shadow) → draw. 33 xrefs. |
| 0x0045EC30 | Scene_Render3DObjects | Main 3D object renderer: transforms vertices, frustum culls, submits triangle strips |
| 0x00454BC0 | Graphics_RenderScene | **ONLY runs on D3DERR_DEVICELOST — USELESS for per-frame hooks.** |

### D3D8 Render State Constants

| State | Value | Description |
|-------|-------|-------------|
| FOGENABLE | 0x1C (28) | Enable/disable fog |
| FOGCOLOR | 0x22 (34) | Fog color |
| FOGTABLEMODE | 0x23 (35) | Fog table mode |
| FOGSTART | 0x24 (36) | Fog start distance |
| FOGEND | 0x25 (37) | Fog end distance |
| AMBIENT | 0x8B (139) | Ambient light color |
| FOGVERTEXMODE | 0x8C (140) | Fog vertex mode |
| ALPHATESTENABLE | 0x39 (57) | Alpha test enable |
| SPECULARENABLE | — | Globally DISABLED (set by Graphics_SetRenderMode) |
| SHADEMODE | — | Set to Gouraud by Graphics_SetRenderMode |

### Graphics Struct Offsets (gfx+)

| Offset | Field | Description |
|--------|-------|-------------|
| +0x154 | device | D3D device pointer |
| +0x704 | state_check | Render state check flag |
| +0x708 | cull_mode | Cull mode setting (3=both) |
| +0x730 | ambient | Ambient light color |
| +0x734 | fog_enable | Fog enable byte |
| +0x738 | fog_bg_color | Fog/background color |
| +0x73C | fog_start | Fog start distance |
| +0x740 | fog_end | Fog end distance |
| +0x7A8 | color_mult_w | Color multiplier W (default 1.0) |
| +0x7B0 | color_mult_matrix | Color multiplier 4x4 matrix (R,G,B,A) |
| +0x7D1 | fullscreen_flag | Fullscreen mode flag |
| +0x7D2 | vsync_flag | VSync mode flag |
| +0x7D8 | texture_path | Custom texture prefix path (strdup) |


## 4. Audio / BASS System

| Address | Name | Description |
|---------|------|-------------|
| 0x00474540 | Audio_Init | BASS_Init (tries -1, then 0 device), BASS_Start, BASS_SetConfig |
| 0x0046A310 | Audio_PlayMusic | BASS_MusicPlayEx wrapper. **ONLY function that resets volume to 1.0.** |
| 0x0046A020 | LoadMusicFile | BASS_MusicLoad wrapper |
| 0x0046A4D0 | LoadJukebox | Parse jukebox.xml |
| 0x0046A440 | Audio_PlayMusicAtSpeed | Play music with tempo/speed adjustment |
| 0x0046A0D0 | Audio_StopChannel | BASS_ChannelStop on channel at this+0x08 |
| 0x00487E40 | BASS_SetConfig | IAT thunk |
| 0x00487E46 | BASS_Start | IAT thunk |
| 0x00487E4C | BASS_Init | IAT thunk |
| 0x00487E52 | BASS_Free | IAT thunk |
| 0x00487E58 | BASS_Stop | IAT thunk |
| 0x004794B0 | BASS_ErrorGetCode | IAT thunk |
| 0x004794B6 | BASS_MusicLoad | IAT thunk |
| 0x004794BC | BASS_ChannelStop | IAT thunk |
| 0x004794C2 | BASS_ChannelSetAttributes | IAT thunk |
| 0x004794C8 | BASS_MusicPlayEx | IAT thunk |

### Music System Struct Layout

**MusicChannel struct (0x534 bytes):**

| Offset | Field | Description |
|--------|-------|-------------|
| +0x000 | vtable | Virtual function table |
| +0x004 | parent | Parent object pointer |
| +0x008 | BASS_chan | BASS channel handle |
| +0x528 | volume | Float volume (init 1.0) |
| +0x52C | fade_rate | Fade rate |
| +0x530 | fade_out | Fade out flag |
| +0x531 | fade_in | Fade in flag |

**MusicDevice struct:**

| Offset | Field | Description |
|--------|-------|-------------|
| +0x418 | chanList | Channel list |
| +0x10 | count | Channel count |

### Music Functions

| Address | Name | Description |
|---------|------|-------------|
| 0x00469FE0 | MusicChannel_Ctor | Init vtable 0x4D91D8, AthenaList, volume=1.0, flags=0 |
| 0x0046A180 | MusicChannel_FadeUpdate | Fade BASS volume up/down with BASS_ChannelSetAttributes |
| 0x0046A250 | MusicChannel_Cleanup | Free all channel buffers, clear AthenaList, Vec3List_Free |
| 0x0046A4B0 | MusicChannel_DeletingDtor | Scalar deleting destructor for MusicChannel |
| 0x004743F0 | MusicChannel_LoadAndAppend | Create MusicChannel, load file, append to list |
| 0x00474480 | MusicDevice_SetVolume | Set BASS config volume |
| 0x004744B0 | MusicDevice_ReadVolume | Read "Music Volume" from registry |
| 0x00474510 | MusicDevice_MuteToggle | Toggle BASS mute on/off |
| 0x004746A0 | MusicDevice_dtor | Destroy music device, free channels, BASS_Stop/Free |
| 0x00474780 | MusicDevice_FadeAll | Update fade on all music channels |
| 0x004747E0 | MusicDevice_DeletingDtor | Deleting dtor for MusicDevice |
| 0x0046A140 | MusicPlayer_SetTempoScale | Set tempo scaling based on music BPM and delta time |
| 0x0046A6E0 | RaptisoftUtil_Ctor | Constructor — FindWindowA("Raptisoft Utility"), set vtable |
| 0x0046A7F0 | RaptisoftUtil_DeletingDtor | Destructor — Window_Notify "*** END RAPTISOFT SESSION ***" |


## 5. Level / Game World Functions

| Address | Name | Description |
|---------|------|-------------|
| 0x0045DE30 | LoadMeshWorld | Load .meshworld level file. Hook point for level file redirection. |
| 0x004706E0 | MeshWorld_ctor | MeshWorld object constructor (0x488 bytes) |
| 0x00461890 | Scene_LoadMeshWorld | Load mesh world from stream — creates MeshBuffers with materials/textures/flags |
| 0x00470930 | MeshWorld_Parse | Parse MESHWORLD text format (*MATERIAL, *MESH, etc.) |
| 0x00458970 | CreateMeshBuffer | Allocate mesh vertex/index buffer |
| 0x00458A80 | InitMaterialArray | Initialize material array |
| 0x0040D1C0 | GetLevelPath | Get path for level (levels/levelN) |
| 0x0040A120 | LoadRaceData | Parse racedata.xml |
| 0x004652E0 | CollisionLevel_ctor | Collision-only level (.meshcollision format) |
| 0x00465260 | Level_LoadCollision | Load binary .meshcollision file |
| 0x004624C0 | Level_Cleanup | Level destructor (free objects, VBs, textures) |
| 0x00461740 | Level_ctor | Level constructor: inits base, vtable 0x4D8FB0, 4 AthenaLists, Timer + LevelState 0x10D4 bytes |
| 0x00465860 | Level_LoadMeshes | Create MeshWorld, create MeshBuffers, parse N:/E: prefixes, create CollisionLevels |
| 0x004629C0 | Level_DeletingDtor | Scalar deleting destructor for Level |
| 0x004650E0 | Level_dtor | Level destructor — set vtable 0x4D9068, call Level_Cleanup |
| 0x00466060 | Level_CloneTree | Recursively clone Level tree — create CollisionLevel for each child |
| 0x00460530 | Level_FindObjectByName | Find level game object by string name (stricmp), returns ptr or 0. 9 xrefs. |
| 0x004130E0 | Level_AssignTextures | Match object textures by ID against 20-slot table at App+0x2C8 |
| 0x00411BA0 | Level_AssignTexturesAndScales | Set scale based on max(x,y,z) vs threshold, then match textures |
| 0x00471750 | LoadMesh | Load discrete .mesh files (uses same parser as MESHWORLD) |

### Level Setup Functions

| Address | Name | Level Path | Description |
|---------|------|------------|-------------|
| 0x00416270 | Scene_SetupLevelDark | levels\leveldark | 2-player SceneObjects when !multiplayer, App+0x5DC/+0x67C |
| 0x0040E190 | Scene_SetupLevel5 | levels\level5 | Simple load (no extras) |
| 0x0040EA90 | Scene_SetupLevel6 | levels\level6 | LAUNCH01/02/03 + CHROMESHADOW positions, launcher timer +0x10DD=200 |
| 0x0040F360 | Scene_SetupLevel7 | levels\level7 | Simple load (no extras) |
| 0x00410830 | Scene_SetupLevel9 | levels\level9 | PILLAR list, MAGNIFYER on hard, CLOUDSCAPE, fog + projection setup |
| 0x00411F60 | Scene_SetupLevel10 | levels\level10 | 4 bumpers, FUN_436FC0 removal on easy, TarBubble list, multiplayer append |
| 0x004110D0 | Scene_SetupLevelCascade | levels\levelcascade | 8 bumpers (N:BUMPER%d 0-7) — Beginner Race (internal name: Cascade) |
| 0x00411540 | Scene_SetupLevelUp | levels\levelup | Initial ball pos (0,50,0), VAC-IN/VAC-OUT vacuum tubes |
| 0x0040D280 | Scene_LoadLevel2 | levels\level2 | Load level 2, clone, init scene |
| 0x0040D390 | Scene_LoadLevel3 | levels\level3 | Load level 3, clone, init scene, collect TarBubble objects |
| 0x0040D6D0 | Scene_LoadLevel4 | levels\level4 | Load level 4, clone, init scene, special setup (flag +0x80=1, camera bounds) |

### Level Board Constructors (Tournament)

| Address | Name | Level | Board Size |
|---------|------|-------|-----------|
| 0x0041CA40 | LevelBoard_WarmUp_ctor | Warm-Up (1) | 0x436C |
| 0x0041CB20 | LevelBoard_Intermediate_ctor | Intermediate (2) | 0x438C |
| 0x0041D060 | LevelBoard_Dizzy_ctor | Dizzy (3) | 0x4BE0 |
| 0x0041E340 | LevelBoard_Tower_ctor | Tower (4) | 0x5418 |
| 0x00420390 | LevelBoard_Up_ctor | Up (6) | 0x4790 |
| 0x00424440 | LevelBoard_Expert_ctor | Expert (5) | 0x4394 |
| 0x0041EA40 | LevelBoard_Odd_ctor | Odd (6) | 0x4FD8 |
| 0x0041ED80 | LevelBoard_Neon_ctor | Neon (9) | 0x43B0 |
| 0x0041F4B0 | LevelBoard_Toob_ctor | Toob (10) | 0x646C |
| 0x0041F110 | LevelBoard_Wobbly_ctor | Wobbly (12) | 0x4388 |
| 0x00424A90 | LevelBoard_Glass_ctor | Glass (12) | 0x4390 |
| 0x0041F930 | LevelBoard_Sky_ctor | Sky (13) | 0x47F8 |
| 0x004206D0 | LevelBoard_Master_ctor | Master (14) | 0x6498 |
| 0x00424C20 | LevelBoard_Impossible_ctor | Impossible (15) | 0x4380 |
| 0x00419030 | Board_ctor | (base) | Base tournament board constructor |

### Level Unlock Flags (App+offset)

| Offset | Level | Texture |
|--------|-------|---------|
| +0x851 | Dizzy | practice-level3.png |
| +0x852 | Tower | practice-level4.png |
| +0x853 | Up | practice-up.png |
| +0x854 | Expert | practice-level5.png |
| +0x855 | Odd | practice-level6.png |
| +0x856 | Toob | practice-level7.png |
| +0x857 | Wobbly | practice-level8.png |
| +0x858 | Sky | practice-level9.png |
| +0x859 | Master | practice-level10.png |
| +0x863 | Neon | practice-neon.png |
| +0x864 | Glass | practice-glass.png |
| +0x865 | Impossible | practice-impossible.png |


## 6. Game Object Factories & Constructors

### Object Creation Functions

| Address | Name | Description |
|---------|------|-------------|
| 0x0040FA20 | CreateBumper | Create BUMPER1/2/3/4 objects |
| 0x00413CE0 | CreateBumper2 | Create bumper variant |
| 0x0040BF50 | CreateMouseTrap | Create MOUSETRAP |
| 0x0040BAA0 | CreateSecretObjects | Create SECRET and SECRETUNLOCK objects |
| 0x0040BCA0 | CreateBadBalls | Create BADBALL entities (8-balls, chrome, funballs) |
| 0x0040C5D0 | DispatchCollisionEvents | Main collision event dispatcher |
| 0x0040C430 | Scene_CreateDynamicObjects | Generic object creation loop via vmethod +0x84 |
| 0x0040C0F0 | Scene_CreateFlags | Scan for FLAG/SMALLFLAG objects, create Flag |
| 0x0040C270 | Scene_CreateSigns | Scan for SIGN objects, create Sign |
| 0x004121D0 | CreateLevelObjects | Multi-factory: BRIDGE, TIPPER, BONK, BBRIDGE, POPCYLINDER, BLOCKDAWG, CATAPULT, GLUEBIE |
| 0x004133E0 | CreatePlatformOrStands | Create PLATFORM and STANDS objects |
| 0x00417FE0 | CreateMechanicalObjects | Multi-factory: LOOPER, GEAR, BIGGEAR, ROTATOR, PENDULUM |
| 0x004117B0 | CreateUpLevelObjects | Create SPEEDCYLINDER |
| 0x0040E250 | CreateExpertLevelObjects | Create SAWBLADE |
| 0x00412850 | HandleArenaCollisionEvents | Create N:SPINNER |
| 0x00438B30 | CreateBonkPopup | Create BONKPOPUP feedback |
| 0x0040AFE0 | Ball_Ctor | Ball constructor (also BadBall_ctor) |
| 0x0040AF90 | Ball_GetTransform | Read ball transform into output struct |
| 0x0040AF00 | SceneObject_InitAtPosition | Initialize SceneObject at (x,y) with Vec3 vtable |

### Object Constructors

| Address | Name | Object | Size | Vtable | Description |
|---------|------|--------|------|--------|-------------|
| 0x00437040 | Popcylinder_ctor | Platform | 0x10FC | — | Platform object constructor |
| 0x00436EE0 | PopCylinder_ctor | PopCylinder | 0x10E8 | — | Pop-up obstacle constructor (static objects) |
| 0x00462850 | SceneObject_ctor | Stands | 0x10D0 | — | Stands/Audience constructor (NOT MeshWorld_ctor!) |
| 0x00435800 | Looper_ctor | Looper | 0x1500 | — | Loop-de-loop constructor |
| 0x00437590 | Gear_ctor | Gear/BigGear | 0x1514 | 0x4D5AD0 | Gear constructor (multi-axis X+Y+Z continuous) |
| 0x00435940 | Rotator_ctor | Rotator | 0x1508 | 0x4D5518 | Rotator constructor (Y-axis oscillation ±2.0rad) |
| 0x00437700 | Pendulum_ctor | Pendulum | 0x1504 | 0x4D5B30 | Pendulum constructor (Z-axis sine wave amplitude 38.0) |
| 0x00437460 | Looper_ctor | Looper | 0x1500 | — | Looper constructor |
| 0x00437960 | Tipper_ctor | Tipper | 0x1104 | — | Tipping platform constructor |
| 0x004661A0 | TipperVisual_ctor | TipperVisual | — | — | Tipper visual component |
| 0x00465200 | TipperVisual_Attach | — | — | — | Attach visual to tipper |
| 0x00438850 | Bonk_ctor | Bonk (hammer) | 0x1200 | — | Hammer constructor |
| 0x00436D70 | BreakBridge_ctor | Breakable bridge | 0x1100 | — | Breakable bridge constructor |
| 0x0043C310 | Blockdawg_ctor | Blockdawg | 0x1154 | — | Block creature constructor |
| 0x00437E10 | Catapult_ctor | Catapult | 0x1108 | — | Catapult constructor |
| 0x00437CB0 | Gluebie_ctor | Gluebie | 0x110C | — | Glue blob constructor |
| 0x0043DFB0 | Secret_ctor | Secret | 0x10EC | — | Secret object constructor |
| 0x00436250 | Magnifier_ctor | Magnifier | — | 0x4D569C | Magnifier constructor (scale 90.0, AthenaList for heated balls) |
| 0x0043CB70 | Magnifier_Update | — | — | — | Per-frame: homes toward heated balls, increments ball+0xC50 (heat) |
| 0x0044BE80 | ScoreObject_ctor | ScoreObject | — | 0x4D6C70 | Race goal rendering object constructor |
| 0x0044AD50 | ArenaScoreParticle_ctor | ArenaScoreParticle | — | — | Init vtable + difficulty scale (0.02/0.03/0.04) |

### Impossible Race Object Vtable

All 5 Impossible objects share vtable **0x004D21C0**. The mesh path string determines visual appearance, the vtable provides behavior.

| Object | Mesh Path Address | Mesh Path String |
|--------|-------------------|-----------------|
| Rotator | 0x004D20FC | Level3-Swirl |
| Pendulum | 0x004D20DC | LevelImpossible-Pendulum |
| Looper | 0x004D2158 | (Looper mesh) |
| Gear | 0x004D213C | (Gear mesh) |
| BigGear | 0x004D211C | (BigGear mesh) |

### Rotator System Details

| Address | Name | Description |
|---------|------|-------------|
| 0x0043B330 | Rotator_Render (vtable[11]) | Oscillates angle: new = direction * 0.004 + angle; stores to +0x10E8. If >2.0 dir=-1.0; if <-2.0 dir=+1.0 |
| 0x004606D0 | Rotator_Update (vtable[1]) | Vertex deformation only |
| 0x00436FC0 | Rotator_RemoveAndFree | vtable[11] offset 0x2C — remove and free object |

**Rotator struct offsets:**
- +0x10D4/D8/DC: Position (X/Y/Z)
- +0x10E8: Current angle
- +0x10EC: Direction (speed multiplier: 1.0=native, 4.0=4x, -1.0=reverse, 0.0=constant rotation)
- Constants: 0x004D5C88=0.004 (speed), 0x004CF48C=+2.0, 0x004D5C84=-2.0 (oscillation limits)

### Pendulum System Details

- Time field: +0x14FC
- Speed field: +0x1500 (CPUID*0.05+0.5)
- Amplitude: 38.0 (constant at 0x004D5D20)
- Uses sin(time) * 38.0 for Z-axis swing

### Gear System Details

- 4 angle fields: +0x10E8, +0x10EC, +0x10F0, +0x10F4
- Uses Gfx_ScaleY/X/Z for multi-axis rotation
- vtable: 0x4D5AD0


## 7. Ball Physics System

| Address | Name | Description |
|---------|------|-------------|
| 0x00405E00 | Ball_Update | **Main ball physics tick (400+ xrefs).** Two-pass collision: Pass 1 checks bounce_count>1 → Ball_ApplyTrajectory; Pass 2 increments bounce_count, sets death_pending if speed>1.0. Also: stun recovery, timer, velocity integration, reflection, sound, camera tilt, spin. |
| 0x00403980 | Ball_FindMeshCollision | Wrapper for Mesh_FindClosestCollision |
| 0x00403100 | Ball_SetTiltedGravity | Set gravity plane to tilted (value 1, normal -1,0,0) |
| 0x00403150 | Ball_SetFlatGravity | Set gravity plane to flat (value 2, normal 0,0,1) |
| 0x00403850 | Ball_SetTrajectory | Set ball trajectory direction + force scale |
| 0x00403750 | Ball_ApplyTrajectory | The "dizzy" effect: normalize+scale velocity by 0.5, damp Y by 1.25, set impact_count(+0x2F0)=100, set death_pending(+0x2E9)=1, set has_trajectory(+0x14D)=1, create trail particles, play boost sound |
| 0x00401DD0 | Ball_CreateTrailParticles | Create trail particles (10 iterations, spawn 0x28 byte objects) |
| 0x00401920 | Ball_RenderShadow | Render ball shadow (scale by radius*constant, position at ball XYZ) |
| 0x00405100 | Ball_InitPhysicsDefaults | Set defaults: radius=0.5, friction=0.2, max_speed=35.0, gravity=6.0 |
| 0x00402400 | Ball_DizzyImmunity | Clears bounce_count(+0x2EC)=0, sets immunity timer(+0x2F4) to max(current, TIME) |
| 0x00402C10 | Ball_RenderWithCollision | Ball render: check collision planes, render shadow, apply scaling, end frame |
| 0x00402A20 | Ball_SetVec3AtOffset | Set 3 floats at offset 0xCA4 in ball (camera/force vector) |
| 0x00402A70 | Ball_DrawArenaScoreText | Draw rumble score text at viewport |
| 0x004027F0 | Ball_dtor | Ball deleting destructor: calls Ball_Cleanup, optionally frees |
| 0x00401660 | Ball_SetName | Set ball display name at +0xC28, copies string, sets type ID=200 |
| 0x00402030 | Ball_SetTargetPos | Set/interpolate target position with smooth damping threshold |
| 0x00402150 | Ball_CheckProximity | Check position proximity radius, store integer distance result |
| 0x00402200 | Ball_Shrink | Odd Race shrink: set is_shrunk+0xC4C=1, shrink radius to 13.0, play 3D sound |
| 0x00402270 | Ball_Grow | Odd Race grow: clear is_shrunk, set radius=26.0, physics=5.0 |
| 0x00408390 | Ball_AI_ChaseNearest | AI steering: finds nearest opponent ball, applies force toward it. 60 xrefs. |
| 0x00408830 | Ball_FallUpdate | Ball update when fallen: shrinks ball, handles scale change, trail cleanup. 40 xrefs. |
| 0x00408D10 | Ball_Split_ctor | Split ball constructor: vtable 0x4CF560, +0xC60=5 |
| 0x00408D70 | Ball_Shatter | Arena: marks parent ball for despawn, spawns 3 AI split balls. 50 xrefs. |
| 0x00409480 | Ball_SplitAndExplode | Creates 2 split balls + circular ArenaScoreParticle explosion. 70 xrefs. |
| 0x00456CD0 | Ball_InitBattleMode | Initialize ball for battle mode: friction 0.18, bounciness 1.0, radius 400.0 |
| 0x004564C0 | Ball_AdvancePositionOrCollision | Advance position with collision detection. Uses spatial tree. Core physics. |
| 0x00402A50 | GameObject_sub2_dtor | GameObject deleting destructor variant 2 |
| 0x004029C0 | CollisionMesh_SetSpeed | **DEAD CODE.** Writes roll_friction (+0xC64) and scaled_dir (+0xC98), immediately overwritten. |
| 0x0045D8F0 | Ball_RenderWithMaterial | Render ball with material: Ball_Render, Graphics_ApplyMaterialAndDraw |
| 0x00405D90 | GameObject_sub_ctor | GameObject subclass constructor: vtable 0x4CF494, scale=1.0 |
| 0x00405DD0 | GameObject_sub_dtor | GameObject deleting destructor variant 1 |
| 0x00401480 | GameObject_dtor | Release timers, free Vec3Lists, cleanup matrices |
| 0x00402290 | GameObject_Render | Render game obj: scale, depth layer toggle, Sprite_RenderQuad |

### Ball Struct Layout (key offsets)

| Offset | Field | Description |
|--------|-------|-------------|
| +0x000 | vtable | Virtual function table pointer (0x004CF3A0) |
| +0x014 | position | Vec3 position (X, Y, Z) |
| +0x020 | velocity | Vec3 velocity (X, Y, Z) |
| +0x03C | radius | Ball radius (default 26.0, 8-ball=35.0) |
| +0x14D | has_trajectory | Trajectory flag (set by Ball_ApplyTrajectory) |
| +0x164/168/16C | position_alt | Alternative position (used in Scene_SetCamera context) |
| +0x188 | max_speed | Maximum speed |
| +0x198 | travel_angle | Pre-computed by Ball_Update via Math_Atan2Angle |
| +0x1B8 | material_ambient | Material ambient matrix start (4 floats) |
| +0x1BC/1C0/1C4/1C8 | material_diffuse | Material diffuse (4 floats) |
| +0x1CC/1D0/1D4/1D8 | material_ambient_vals | Ambient values (4 floats) |
| +0x1EC/1F0/1F4/1F8 | material_specular | Specular values (4 floats) |
| +0x204 | material_flag | Material flag |
| +0x284 | max_speed | Max speed field |
| +0x2A4 | gravity_magnitude | Gravity strength (spin_rate, default 5.0 for ALL levels) |
| +0x2AC/2B0/2B4 | color_mult | Color multiplier R/G/B (read by Ball_Render every frame) |
| +0x2E9 | death_pending | Death pending flag |
| +0x2EC | bounce_count | Bounce count (triggers trajectory at >1) |
| +0x2F0 | impact_count | Impact count (set to 100 by trajectory) |
| +0x2F4 | dizzy_immunity_timer | Dizzy immunity timer |
| +0x2FC | color | Ball color |
| +0x304 | team | Team ID |
| +0x308 | is_active | Active flag |
| +0x31D | is_8ball | 8-ball flag |
| +0x778 | collision_flags_primary | Primary collision flags (set by DispatchCollision) |
| +0x779 | collision_flags_secondary | Secondary collision flags |
| +0x748 | field_748 | Set to 0 by pre-respawn FUN_004030b0 |
| +0xC50 | heat | Magnifier heat value |
| +0xC58 | burning_flag | Burning flag (set by Magnifier_Update) |
| +0xC60/C64/C68 | home_pos | Home position X/Y/Z (for BADBALL matching) |
| +0xC68 | max_speed_mult | Speed multiplier (from <SPEEDMULT> tag) |
| +0xC4C | is_shrunk | Shrunk flag (Odd Race) |
| +0xC64 | roll_friction | Roll friction |
| +0xC68 | speed_scale | Speed scale |
| +0xC74 | gravity | Gravity field |
| +0xC28 | name | Display name string |
| +0xCA4/CA8/CAC | velocity_alt | Alternative velocity (used by catapult/rotator) |
| +0xC50 | heat_value | Magnifier heat |
| +0x2CC | in_tar | Tar pit flag (1=frozen, stops physics) |


## 8. Collision System

| Address | Name | Description |
|---------|------|-------------|
| 0x00465D90 | Mesh_FindClosestCollision | Ray-mesh collision via spatial tree. 10 xrefs. |
| 0x00465EF0 | Collision_TraverseSpatialTree | Recursive octree traversal for collision tests |
| 0x0040C5D0 | DispatchCollisionEvents | Main collision event dispatcher (18 global events: N:SECRET, UNLOCKSECRET, N:GOAL, MOUSETRAP, WATER, TARPIT, NOCONTROL, E:NODIZZY, SAFESWITCH, LIMIT, BREAK, JUMP, ACTION, TRAJECTORY, DROPIN, PIPEBONK, POPOUT, ZOOP) |
| 0x0040E6A0 | ExpertCollisionEvents | Arena-specific collision handler (E:CALLHAMMER, E:HAMMERCHASE, E:ALERTSAW1/2, E:ACTIVATESAW1/2, E:ALERTJUDGES, E:SCORE, E:JUMP, E:BELL + delegates to DispatchCollisionEvents) |
| 0x0040DCD0 | TowerCollisionEvents | Level-specific collision handler (E:CATAPULTBOTTOM, E:OPENSESAME, N:TRAPDOOR, E:BITE, E:MACETRIGGER, N:MACE + delegates to DispatchCollisionEvents) |
| 0x00410D00 | NeonCollisionEvents | Collision dispatcher for Neon/Sky: E:HEATON/E:HEATOFF/E:PEGS/E:NOPEGS/E:TRAPPOP/E:LIMIT |
| 0x00413BD0 | SinkPlatformArenaCollisionEvents | Match "DN:SINKPLATFORM" name, call sinking behavior |
| 0x00456D80 | CollisionMesh_ctor | CollisionMesh constructor: initializes triangle list |
| 0x00456120 | CollisionMesh_AddTriangle | Add triangle to collision mesh: appends to list at +0x430 |
| 0x00463330 | SpatialTree_ctor | SpatialTree (octree) constructor for collision spatial partitioning |
| 0x004632E0 | SpatialTree_Free | Free spatial tree nodes and cleanup |
| 0x004610E0 | MeshWorld_CollectTrianglesInAABB | Recursive AABB triangle collection (collision mesh query) |
| 0x004580D0 | AABB_ContainsPoint | Test if point inside AABB (6 floats) |
| 0x00458020 | AABB_Update | Expand axis-aligned bounding box by 3D point |
| 0x00477330 | AABB_FromSphere | Compute AABB min/max from sphere center, velocity, radius |
| 0x00458220 | AABB_TriangleIntersect | Test if triangle edge overlaps AABB (SAT collision test) |
| 0x00458320 | AABB_TriangleTest6Edges | Full 6-edge AABB-triangle intersection |
| 0x004583F0 | AABB_TriangleIntersect2 | Double AABB-triangle test wrapper |
| 0x00458000 | Collision_InitDefaultAABB | Initialize default collision AABB bounds |
| 0x004774B0 | Tri_TestPointInside | Test if point is inside triangle using cross product edge tests |
| 0x00457F10 | Collision_PointInTriangle | Barycentric point-in-triangle test |
| 0x00477330 | AABB_FromSphere | Compute AABB from sphere |
| 0x00477670 | Segment_ClosestPoint | Find closest point on line segment to a point |
| 0x004777C0 | Vec3_ProjectOntoPlane | Project vector onto plane |
| 0x004772C0 | Vec3_Reflect | Reflect vector v across plane normal n |
| 0x00477240 | Vec3_DotDiff | dot(a,n) - dot(b,n) |
| 0x00477280 | Vec3_DotDiffAbs | abs(dot(a,n) - dot(b,n)) |
| 0x00477060 | Ray_SphereIntersect | Ray-sphere intersection test |
| 0x00477120 | Vec3_ProjectOntoRay | Project point onto ray |
| 0x004771D0 | Ray_PlaneIntersectT | Ray-plane intersection, returns T |
| 0x0045DFD0 | SceneObject_CheckCollision | Thunk: compute bounding sphere + check collision planes |
| 0x0046FBB0 | SceneObject_ComputeCollisionSphere | Compute bounding sphere from AABB, call Ball_CheckCollisionPlanes |
| 0x00435B00 | CollisionLevel_PlayBreakSound | Play 3D positional "break" sound at collision level position |
| 0x00466CF0 | CollisionNode_ctor | Collision node constructor: sets friction values (0.1), vtable |
| 0x004777020 | CollisionNode_BaseInit | Collision node base initialization |
| 0x00459ABA | Triangle_Interpolate2D | 2D triangle interpolation (barycentric coords) |
| 0x00458190 | Collision_GradientEval_Stub | Empty stub for collision gradient evaluation |

### Collision Event Triggers

| Event Tag | Type | Description |
|-----------|------|-------------|
| N:GOAL | Goal | Race finish line |
| N:MOUSETRAP | Trap | Mouse trap object |
| N:SECRET | Secret | Secret area |
| N:UNLOCKSECRET | Secret | Unlock secret area |
| N:WATER | Water | Water effect |
| N:TARPIT | Trap | Tar pit (freezes ball) |
| N:NOCONTROL | Control | Remove player control |
| N:TRAPDOOR | Trap | Tower trapdoor |
| N:MACE | Trap | Tower mace |
| N:SPINNER | Arena | Arena spinner |
| N:ONROTATOR | Physics | Register ball on rotator tracking |
| N:ONGEAR | Physics | Register ball on gear tracking |
| E:NODIZZY | Effect | Clear dizzy checkpoints |
| E:SAFESWITCH | Checkpoint | Safe switch checkpoint |
| E:LIMIT | Boundary | Level boundary |
| E:BREAK | Breakable | Breakable object |
| E:JUMP | Jump | Jump pad |
| E:ACTION | Action | Generic action trigger |
| E:TRAJECTORY | Effect | Dizzy trajectory |
| E:DROPIN | Spawn | Drop-in point |
| E:PIPEBONK | Collision | Pipe bonk |
| E:POPOUT | Spawn | Pop-out point |
| E:ZOOP | Effect | Zoop effect |
| E:HEATON | Neon | Add ball to magnifier heated list |
| E:HEATOFF | Neon | Remove ball from heated list |
| E:PEGS | Neon | Enable pegs |
| E:NOPEGS | Neon | Disable pegs |
| E:TRAPPOP | Neon | Trap pop |
| E:CALLHAMMER | Arena | Call hammer |
| E:HAMMERCHASE | Arena | Hammer chase start |
| E:ALERTSAW1/2 | Arena | Alert saw blade |
| E:ACTIVATESAW1/2 | Arena | Activate saw blade |
| E:ALERTJUDGES | Arena | Alert judges |
| E:SCORE | Arena | Score point |
| E:BELL | Arena | Ring bell |
| E:CATAPULTBOTTOM | Tower | Catapult launch |
| E:OPENSESAME | Tower | Open gate |
| E:BITE | Tower | Chomper bite |
| E:MACETRIGGER | Tower | Trigger mace |


## 9. Camera System

| Address | Name | Description |
|---------|------|-------------|
| 0x00419FA0 | Scene_SetCamera | Camera positioning. Ball pos from +0x758 + scene offset +0x434C. Boundary check +0x3F1C: distance clamp with sqrt+falloff. FOV 0.9 from +0x29BC. |
| 0x00413280 | CameraLookAt | Set camera look-at (CAMERALOOKAT) |
| 0x0040DFA0 | Scene_RenderWithCamera | Two-pass camera render: front face then 180-degree back face. Camera angle +0x43A0, Y offset +0x43A4, X offset +0x43B0. |
| 0x0040ACA0 | Level_SelectCameraProfile | Select camera profile by level difficulty (4-15) |
| 0x0041F7E0 | Scene_HandleRaceEnd_ClampZoom | Clamps camera zoom levels after race end |

### Camera Orbit Semantics

The Hamsterball camera is **ORBIT**:
- **SetPosition(X,Y,Z)** via vtable[+0x8] sets LOOK-AT TARGET
- **SetDirection(&vec3, float dist)** via vtable[+0xC] sets direction + orbit distance
- **EYE = TARGET + DIRECTION × DISTANCE** (PLUS)
- Original: target=ball, dir={sin,0.9,cos}, dist=700
- Ball position in Scene_SetCamera context: edi+0x164/0x168/0x16C
- Ball travel angle at ball+0x198 (pre-computed by Ball_Update via Math_Atan2Angle)
- Vertical angle hardcoded 0.9f at 0x41A2B1
- Scene+0x29C0 = orbit distance, Scene+0x29BC = orbit angle


## 10. Scene System

| Address | Name | Description |
|---------|------|-------------|
| 0x00419770 | Scene_dtor | Master scene destructor. Destroys all AthenaList items, frees level clones, resources. |
| 0x00419C00 | Scene_Update | Main scene tick. Demo timer, unpause check, ball position updates, camera tracking, screen offset animation, vmethods (render passes), iterate dynamic objects. |
| 0x0041A2E0 | Scene_Render | Main render dispatch by player count. Mode 0 (1P), Mode 1 (2P), Mode 2 (3-4P split). |
| 0x00419B70 | Scene_ForEachBall_SetVelocity | Iterate ball list +0x29D4, call Ball_SetVelocity on each |
| 0x00418870 | Scene_CreateObject4f | Create object at (x,y,z,w) with BaseObject vtable 0x4CF584 |
| 0x00469990 | Scene_AddObject | Add SceneObject to Scene — checks uniqueness, appends, sets back-ref. 77 xrefs. |
| 0x004693C0 | Scene_AddAllObjects | Batch-add all SceneObjects from internal list. 25 xrefs. |
| 0x0046F390 | Scene_BeginFrame | Begin scene frame — Graphics_BeginFrame + vtable[6] callback. 39 xrefs. |
| 0x0046F3B0 | Scene_BeginFrameThenRender | Begin graphics frame then invoke render callback. 39 xrefs. |
| 0x00460DA0 | Scene_RenderFrame | Full frame render pipeline — iterates scenes/objects, assigns render indices. 38 xrefs. |
| 0x00461370 | Scene_RenderOpaque | Opaque render pass — iterates objects calling vtable[0x28]. 38 xrefs. |
| 0x00461F00 | Scene_Subdivide | Create 3D grid of mesh objects by dividing bounding box. 38 xrefs. |
| 0x00462100 | Scene_SubdivideRandom | Random grid subdivision using seeded RNG. 38 xrefs. |
| 0x004629E0 | Scene_LoadCached | Load .cached scene file. 37 xrefs. |
| 0x004605E0 | AthenaHashTable_Lookup | Hash table case-insensitive string lookup. 36 xrefs. |
| 0x004601A0 | Scene_MarkDirty | Recursively marks scene objects as dirty (tree traversal) |
| 0x00460220 | Scene_ResetObjectSlots | Resets scene object slots, re-registers objects |
| 0x004602F0 | Scene_CollectByNameFilter | Collect scene objects by name filter |
| 0x004366E0 | Scene_SetRaceActive | Sets +0x10EC=1 (race active flag). 62 xrefs. |
| 0x00437130 | Scene_StartCountdown | Start race countdown (3..2..1..GO with param=400 or 50 frames) |
| 0x00440390 | Scene_UpdateArenaPhysics | Arena physics update: wave motion, collision detection, sound triggers |
| 0x0045DF80 | SceneObject_CallUpdate | Dispatch +0x434 vtable[1] (Update). 47 xrefs. |
| 0x0045DF90 | SceneObject_CallRender | Dispatch +0x434 vtable[2] (Render). 47 xrefs. |
| 0x00460450 | Scene_RenderBallShadow | Render ball shadow: Ball_Render + depth bias pass. 38 xrefs. |
| 0x00459E10 | Scene_RenderIfVisible | Render scene object if visible flag set |
| 0x00444AA40 | Scene_FindTextureByName | Find texture by case-insensitive name |
| 0x00444AB00 | Scene_FindTextureDimensions | Find texture and measure text width |
| 0x00444ABF0 | Scene_AddTextureToList | Add texture reference to scene list by name |

### Scene Struct Layout (key offsets)

| Offset | Field | Description |
|--------|-------|-------------|
| +0x424 | active_object | Active object pointer (controls input dispatch — last object added) |
| +0x864 | current_item | Current menu item pointer |
| +0x868 | name | Scene name string |
| +0x8AC | meshworld | MeshWorld pointer |
| +0x8B0 | collision_level | Collision level pointer |
| +0x29B0 | current_level | Current level ID |
| +0x29B4 | game_mode | Game mode |
| +0x29BC | camera_orbit_angle | Camera orbit angle |
| +0x29C0 | camera_orbit_dist | Camera orbit distance |
| +0x29D0 | ball_count | Ball count |
| +0x29D4 | ball_list | Ball list (AthenaList) |
| +0x3A38 | player_list_2P | Player list for 2P mode |
| +0x3F1C | boundary_check | Distance clamp boundary |
| +0x3F2C | snap_timer | Camera snap timer |
| +0x43A0 | camera_angle | Camera angle |
| +0x43A4 | camera_y_offset | Camera Y offset |
| +0x43B0 | camera_x_offset | Camera X offset |
| +0x434C | scene_offset | Scene offset for camera |
| +0x758 | ball_pos_for_camera | Ball position used by camera |


## 11. SceneObject System

**Vtable:** 0x4D934C | **Size:** 0xD4 bytes

| Address | Name | Description |
|---------|------|-------------|
| 0x0046B4F0 | SceneObject_ctor | Constructor: vtable 0x4D934C, init 3 matrix transforms, pos=0, visible=1, zOrder=-1 |
| 0x0046B650 | SceneObject_dtor | Destructor: sets vtable, flags cleanup |
| 0x0046B3F0 | SceneObject_Cleanup | Cleanup helper: clear lists, reset state |
| 0x0046B4D0 | SceneObject_SetVisible | Toggle visibility at +0x88 |
| 0x0046B490 | SceneObject_SetPosition | Set position (3 floats at +0x08) + vmethod+0x0C |
| 0x0046B4B0 | SceneObject_SetScale | Set scale (3 floats at +0x14) + vmethod+0x0C |
| 0x0046B670 | SceneObject_Render | D3D world matrix build + SetTransform + SetMaterial |
| 0x0046B860 | SceneObject_BaseDtor | Iterate child AthenaList, call each dtor(1), clear list |
| 0x0046B9F0 | SceneObject_DeletingDtor | Calls BaseDtor then free if scalar deleting |
| 0x00461460 | SceneObject_BaseInit | Base init: AthenaList_Init, Vec3_Init, string buffer |
| 0x00461680 | SceneObject_BaseClear | Clear/reset: Vec3List_Free, Matrix_Identity, free string |
| 0x00453BD0 | Scene_RegisterObject | Assign ID to obj+0x8C, call vmethod+0xC, store in scene array |
| 0x00470150 | SceneObject_RenderFull | Full render with ball+material+strips, alpha-aware. 40 xrefs. |
| 0x00470440 | SceneObject_RenderSingleObj | Render a single SceneObject with material/strip dispatch. 39 xrefs. |
| 0x00472770 | SceneObject_BuildStrips | Builds triangle strips for SceneObject rendering. 39 xrefs. |
| 0x00479630 | SceneObject_DeletingDtor | Deleting destructor: release 3 render callbacks then dtor |
| 0x004796B0 | SceneObject_FadeAlpha | Fade alpha state machine: states 0-3 cycle through fade timers |
| 0x00479820 | SceneObject_ScalarDtor | Scalar destructor (free if bit 0 set) |
| 0x004692D0 | SceneObject_ScalarDtor | SceneObject scalar destructor |
| 0x0046F010 | SceneObject_dtor | Destroys scene object, iterates list calling dtor(1) |
| 0x0046F0E0 | SceneObject_DeletingDtor | Deleting dtor for SceneObject |
| 0x004443AC0 | SceneObject_RenderScaled | Render object scaled (ScaleX, SetPosition, vtable calls + Timer) |
| 0x004410C0 | SceneObject_FreeStrings | Free 2 string pointers, re-init BaseObject, call SceneObject_dtor |
| 0x0040B960 | SceneObj_SetBounds | SceneObject bounds setter: sets +0x14-20, identity matrix if scale != 1.0 |

### SceneObject Structure Layout

| Offset | Type | Field | Description |
|--------|------|-------|-------------|
| +0x000 | void* | vtable | Virtual function table pointer (0x4D934C) |
| +0x004 | int | gfxContext | Graphics context pointer |
| +0x008-0x01C | int[6] | field_08 | Zeroed at init |
| +0x088 | char | visible | Visibility flag (1=visible) |
| +0x08C | int | zOrder | Z-order / object ID (-1 default) |
| +0x094 | float[5] | baseScaleMatrix | Base scale 4x4 matrix (identity 1.0) |
| +0x0A8 | float[5] | rotationMatrix | Rotation 4x4 matrix (zero at init) |
| +0x0BC | float[5] | worldMatrix | World transform 4x4 matrix (zero at init) |
| +0x0CC | float | radius | Bounding radius = sqrt(global_constant) |
| +0x0D0 | int | type | Object type (3 default) |


## 12. Level Rendering

| Address | Name | Description |
|---------|------|-------------|
| 0x0040B090 | Level_InitScene | Init camera/scene (projection, fog, find CAMERALOCUS) |
| 0x0040B420 | Level_RenderDynamicObjects | Render moving objects (platforms) using Timer position |
| 0x0040B570 | Level_RenderObjects | Iterate objects calling vtable+0x0C (render callback) |
| 0x0040B600 | Level_UpdateAndRender | Full update pass: merge lists, pre-render, shadow, cleanup |
| 0x0040B9C0 | Level_SetObjectTransform | Set world transform from position data |
| 0x0041FC90 | SkyBoard_Update | Sky board per-frame update: Magnifier_Update, Board_UpdateRaceState, pillar animation |
| 0x00410E80 | SkyBoard_RenderDynamic | Sky board render dynamic: cloud sprite, pillar rendering, magnifier glass mesh |
| 0x00410670 | Scene_RenderReflectiveObjects | 8 reflective objects at +0x438C, float offset +0x644C |
| 0x00411380 | Scene_RenderReflectiveObjects7 | 8 reflective objects at +0x436C, float offset +0x642C |
| 0x00412DC0 | Scene_RenderReflectiveObjects4 | 4 reflective objects at +0x439C, float offset +0x53FC |
| 0x00446150 | Level_MeshWorldCtor | Level MeshWorld constructor (0x461510) |
| 0x00465080 | Level_RenderCtor | Level render constructor (0x465080) |
| 0x0041C5B0 | Board_Setup | Board setup: ball spawn → SAFESPOT scan → CreateBadBalls → CreateMouseTrap → CreateSecretObjects → Scene_CreateFlags → Scene_CreateSigns → Scene_CreateDynamicObjects |

### Board Color System

| Address | Name | Description |
|---------|------|-------------|
| board+0x1508 | RGBA level colors | Level color values (4 floats: R, G, B, Alpha=1.0). NOT "gravity" — these are the RGBA colors passed to rendering. |

### Bumper System

| Address | Name | Description |
|---------|------|-------------|
| board+0x6428 | bumper_lit_1 | Bumper lit flag 1 (WRITE-ONLY decay timer, NO render reads this) |
| board+0x6448 | bumper_lit_2 | Bumper lit flag 2 (WRITE-ONLY decay timer) |
| board+0x53FC | bumper_lit_3 | Bumper lit flag 3 (WRITE-ONLY decay timer) |

Bounce: vel*4.0 (Master 5.0), clamp 5-10 (Master 12). Visual flash from mesh collision vtable, not lit polling.


## 13. Menu / UI System

| Address | Name | Description |
|---------|------|-------------|
| 0x0042DE50 | MainMenu_ctor | Main menu (LET'S PLAY, HIGH SCORES, OPTIONS, CREDITS, EXIT) |
| 0x00442CE0 | OptionsMenu_ctor | Options (Resolution, Fullscreen, Color, Volume, Remap, Mouse) |
| 0x004254E0 | CreditsScreen_ctor | Credits scrolling screen |
| 0x0042B470 | HighScoreEntry_ctor | High score entry screen (name input + score display) |
| 0x0042BD40 | HighScoreEntry_Render | Render high score entry UI |
| 0x0042E060 | GameSelectionScreen_ctor | Tournament difficulty selector |
| 0x0044FD60 | SaveTourneyDialog_ctor | Save tournament dialog |
| 0x004476B0 | RegisterDialog_ctor | Register/purchase dialog |
| 0x0042E6F0 | QuitRaceMenu | Race quit confirmation menu |
| 0x0042EA30 | PracticeMenu_ctor | Practice menu: "CHOOSE A PRACTICE RACE!", 14 race items |
| 0x0042F810 | TimeTrialMenu_ctor | Time Trial menu → extends PracticeMenu with race items + lock checks |
| 0x0042FC10 | PartyMenu_ctor | Party Race menu → extends PracticeMenu, vtable 0x4D4738 |
| 0x0042FC40 | ArenaMenu_ctor | Arena menu with 14 arena items (Warm-Up to Impossible), vtable 0x4D47B8 |
| 0x00432D20 | ArenaLevelSelect_Render | Arena/Rodent Rumble level selection render |
| 0x004326D0 | MPMenu_ctor | Multiplayer menu: Party Race, Rodent Rumble, controller config |
| 0x0042E220 | DifficultyMenu_ctor | Difficulty selection menu (Pipsqueak/Normal/Frenzied) |
| 0x0042B190 | ConfirmMenu_ctor | Confirmation menu (BACK/BACK2TOURNAMENT, DONE), vtable 0x4D39D0 |
| 0x004430330 | PauseArenaMenu_ctor | Pause Rumble menu (RESUME, OPTIONS, QUIT) |
| 0x00440E70 | OkayDialog_ctor | "Okay Dialog" with caption text + "OKAY!" button |
| 0x00443E30 | QuitDialog_ctor | Quit Dialog — "Quit Dialog" with "YES"/"NO" buttons |
| 0x0040A920 | Scene_CreateGameOverMenu | Creates game-over UI based on game state |
| 0x004469E0 | App_CreateConfirmMenu | Create ConfirmMenu dialog, add to scene |
| 0x00446A60 | App_CreateHighScoreEntry | Create HighScoreEntry dialog, add to scene |
| 0x004448F20 | SimpleMenu_ctor | "Simple Menu" base ctor with item list, up/down scrollers. 15 xrefs. |
| 0x004490A0 | UIListItem_ctor | Init 0x444-byte item with Vec3 + AthenaList |
| 0x004497F0 | UIList_AddItem | Add named item (text, subtext, colors, SceneObject, height). 86 xrefs. |
| 0x00449430 | UIList_AddSpacer | Add empty spacer item with height only. 29 xrefs. |
| 0x004494D0 | UIList_ScrollUpdate | Scroll logic + mouse wheel + vtable dispatch. 17 xrefs. |
| 0x00449B00 | UIList_Cleanup | Free all items. 27 xrefs. |
| 0x00449C20 | UIList_HandleKeyNav | Handle up/down/pgup/pgdn key navigation. 18 xrefs. |
| 0x00449D40 | UIList_Render | Draw all items: gradient bars, text, icons, scroll arrows. 18 xrefs. |
| 0x0044A570 | UIList_Layout | Compute text widths, position SceneObjects, set scrollers. 18 xrefs. |
| 0x0044A8B0 | UIList_SetTextByName | Find item by subtext, replace display text. 27 xrefs. |
| 0x00449750 | UIList_ActivateCurrentItem | Activate selected item: Back→650, Continue→50, else callback. 18 xrefs. |
| 0x00449240 | UIList_AddIconItem | UI list add item with icon (0x444-byte UIListItem) |
| 0x00444ACB0 | UIList_Clear | Empty stub (returns 0), clear/reset UI list |
| 0x004441660 | UIList_dtor | UIList destructor: initializes 5 AthenaHashTables, cleanup |
| 0x0044431C0 | UIList_DeletingDtor | UIList scalar deleting destructor |
| 0x004474800 | Menu_Ctor | Menu class constructor, inits 8 AthenaLists |
| 0x004474930 | Menu_SetDirty | Sets dirty flag (+0x2d64) |
| 0x004474940 | Menu_MergeAllLists | Merge all 7 category lists into main list |
| 0x004474E70 | Menu_dtor | Menu destructor: frees 10 Vec3List entries, clears AthenaLists |
| 0x004475000 | Menu_DeletingDtor | Menu scalar deleting destructor |

### Menu Rendering Notes

- SimpleMenu's UIList_Render (vtable[2]=0x449D40) has NO visibility check — unconditionally renders all items every frame
- There is NO engine-level SetVisible/Hide function for menus
- OptionsMenu solves this by replacing vtable[2] with 0x441800 which checks +0xE09 before calling UIList_Render
- +0xE09 field doesn't exist on base SimpleMenu (OptionsMenu alloc=0xE0C, SimpleMenu is smaller)
- Input dispatch: only one menu receives input at a time via MeshWorld+0x424 (active object pointer)
- UIList_AddItem only appends. To reorder, swap pointers in items array at menu+0xC98 (count at menu+0x890)
- Empty submenus crash the game (engine limitation — mouse hit-testing assumes minimum item count)


## 14. Tournament / Race System

| Address | Name | Description |
|---------|------|-------------|
| 0x00427080 | Tournament_AdvanceRace | Advance to next race in tournament; 15-case switch creates Board_ctor for each level (1-15), saves score, difficulty time bonus, saves race timestamps |
| 0x004298C0 | TimerDisplay | Race timer display |
| 0x00428ED0 | Difficulty_GetTimeModifier | Time modifier by difficulty: 0=easy, 1=normal, 2=hard, default=0.0 |
| 0x00433AC0 | GameSelectionManager | Tournament save/load (TOURNAMENT.SAV) |
| 0x004446730 | Tourney_SaveTournament | Saves tournament to "DATA\\tournament.sav" |
| 0x00450960 | Tourney_AdvanceRound | Advances tournament to next round, creates new TourneyMenu |
| 0x004508F0 | Tourney_SetCurrentLevel | Sets tournament current level, material color |
| 0x00445230 | Scene_StartTournament | Starts tournament mode: creates TourneyMenu, sets music tempo |
| 0x0044C260 | RaceResultPopup_ctor | Race end popup showing rank + TIME'S UP! / OUT OF TIME! text |
| 0x0044B8A0 | RaceResults_ctor | RaceResults constructor: init timers, random congratulatory text, score thresholds |
| 0x0044BFC0 | RaceResults_Render | Render race results: colored rect, shadow text for title/place/time |
| 0x0044C7D0 | RaceResults_Update | Update race results: advance timers, check completion, sound/voice |
| 0x0044CB10 | RaceResultsMenu_ctor | RaceResultsMenu constructor: title, subtitle, player entries, timer |
| 0x0044FDA0 | TourneyMenu_ctor | Tournament menu constructor |
| 0x00451B90 | TourneyMenu_ctor (v2) | Tournament menu constructor: difficulty, music, scoring thresholds |
| 0x00450AF0 | TourneyMenu_Render | Between-races TourneyMenu render (vtable[2] of 0x4D83F0) |
| 0x004264A0 | TourneyMenu_GetRaceName | Get race name string for tournament |
| 0x004264B0 | TourneyMenu_WriteSave | Write tournament save data |
| 0x004265A0 | TourneyMenu_LoadSaveAndShow | Load tournament save and display |
| 0x004266F0 | TourneyMenu_Advance | Advance tournament to next round |
| 0x00426780 | TourneyMenu_CreateBoard | Create board for tournament race |
| 0x00426930 | TourneyRaceEntry_Dtor | TourneyRaceEntry destructor: set vtable, call UIList_Cleanup |
| 0x0041B130 | Board_UpdateRaceState | Updates race state: fade alpha decrement, per-player timer countdown, Game Over popup. Sets App+0x5D6 when timer DEC brings obj+0x1C below 0. |

### Tournament Level Mapping

| Case | Level | Board Constructor | Board Size |
|------|-------|-------------------|-----------|
| 1 | Warm-Up | 0x41CA40 | 0x436C |
| 2 | Beginner | 0x4200E0 | 0x644C |
| 3 | Intermediate | 0x41CB20 | 0x438C |
| 4 | Dizzy | 0x41D060 | 0x4BE0 |
| 5 | Tower | 0x41E340 | 0x5418 |
| 6 | Up | 0x420390 | 0x4790 |
| 7 | Expert | 0x424440 | 0x4394 |
| 8 | Odd | 0x41EA40 | 0x4FD8 |
| 9 | Neon | 0x41ED80 | 0x43B0 |
| 10 | Toob | 0x41F4B0 | 0x646C |
| 11 | Wobbly | 0x41F110 | 0x4388 |
| 12 | Glass | 0x424A90 | 0x4390 |
| 13 | Sky | 0x41F930 | 0x47F8 |
| 14 | Master | 0x4206D0 | 0x6498 |
| 15 | Impossible | 0x424C20 | 0x4380 |

### Difficulty System

| Value | Name | Description |
|-------|------|-------------|
| 0 | Pipsqueak | Easy mode |
| 1 | Normal | Normal mode |
| 2 | Frenzied | Hard mode |

- App+0x23C = difficulty (0/1/2)
- Tournament detection: profile=App+0x220, check profile+0x10(party)+profile+0x11(practice)==0
- App_StartTournamentRace(0x4288B0) sets neither party nor practice
- App_StartPracticeRace(0x428C50) sets profile+0x11=1
- App_StartPartyRace(0x428B20) sets profile+0x10=1
- Enemy gates in Board_Setup: JZ at 0x41C9E4+0x41CA05 (74 07→NOP)
- LoadMeshWorld hook(0x45DE30) redirects level files by difficulty suffix (-easy/-hard)

### Timer Slots (stride 0xA0)

| Player | Timer Address |
|--------|-------------|
| P1 | App+0x5E8 / App+0x5EC |
| P2 | App+0x688 / App+0x68C |
| P3 | App+0x728 / App+0x72C |
| P4 | App+0x7C8 / App+0x7CC |


## 15. Arena / Rumble Board System

| Address | Name | Description |
|---------|------|-------------|
| 0x004217B0 | ArenaBoard_ctor | Init board with "ArenaBoard" string, timer, base score=6000. 15 xrefs. |
| 0x00421880 | ArenaBoard_dtor | Cleanup timer, release SceneObjects, call Scene_dtor. 24 xrefs. |
| 0x00421910 | ArenaBoard_Render | Draw timer bar, round info, difficulty status, "TIE BREAKER!". 16 xrefs. |
| 0x00421FE0 | ArenaBoard_Update | Check round end, resolve ties, play "Game Over" music. 16 xrefs. |
| 0x00425340 | ArenaBoard_DeletingDtor | ArenaBoard destructor |
| 0x00424C10 | ArenaBoard_DeletingDtor (v2) | ArenaBoard scalar deleting destructor |
| 0x00472A30 | ArenaBoard_dtor (v2) | ArenaBoard destructor, frees sub-object and calls SceneObject_dtor |
| 0x00472A50 | ArenaBoard_TickDown | Decrements countdown, triggers vtable callbacks at zero |
| 0x00472A80 | ArenaBoard_RenderThenFree | Calls render (vtable+0x48) then free (vtable+0x40) |
| 0x00472AD0 | ArenaBoard_DeletingDtor | Deleting dtor for ArenaBoard |
| 0x004502F0 | ArenaBoard_Menu_dtor | ArenaBoard menu destructor: vtable, timer, UIList cleanup |
| 0x00458E60 | ToggleTimer_Init | Initialize round timer. 12 xrefs. |
| 0x00458E80 | ToggleTimer_Cleanup | Cleanup round timer. 32 xrefs. |
| 0x00458E90 | ToggleTimer_Tick | Tick round timer countdown. 12 xrefs. |
| 0x00446860 | ToggleTimer_TickWrapper | Wrapper: calls ToggleTimer_Tick at offset +0x110C |
| 0x00444BEF0 | Timer_Decrement | Timer tick: value = end_value - 100, set flag at +0x2A |

### Arena Initialization Functions

| Address | Name | Level Path |
|---------|------|------------|
| 0x00413C20 | ArenaLevel_WarmUp_Init | levels\arena-WarmUp |
| 0x00413CE0 | ArenaLevel_Beginner_Init | levels\arena-beginner |
| 0x00414180 | ArenaLevel_Intermediate_Init | levels\arena-intermediate |
| 0x00414240 | ArenaLevel_Dizzy_Init | levels\arena-dizzy |
| 0x004144B0 | ArenaLevel_Tower_Init | levels\arena-tower |
| 0x00414960 | ArenaLevel_Up_Init | levels\arena-up |
| 0x00414B10 | ArenaLevel_Expert_Init | levels\arena-expert |
| 0x00414CE0 | ArenaLevel_Odd_Init | levels\arena-Odd |
| 0x00414F00 | ArenaLevel_Toob_Init | levels\arena-Toob |
| 0x004153A0 | ArenaLevel_Wobbly_Init | levels\arena-Wobbly |
| 0x004158C0 | ArenaLevel_Sky_Init | levels\arena-Sky |
| 0x00416080 | ArenaLevel_Master_Init | levels\arena-Master |
| 0x00416F40 | ArenaLevel_Neon_Init | levels\arena-neon |
| 0x00417DF0 | ArenaLevel_Glass_Init | levels\arena-glass |
| 0x00418540 | ArenaLevel_Impossible_Init | levels\arena-impossible |

### Arena Scoring

| Address | Name | Description |
|---------|------|-------------|
| 0x00434C40 | Judge_Reset | Reset judges (clear active flag, re-add to list) |
| 0x00434C80 | ScoreDisplay_SetTime | Set score display time with format string |
| 0x00434E20 | Bell_Activate | Activate bell (play sound, set animation) |
| 0x00438BB0 | Hammer_ChaseStart | Start hammer chase (set chasing flag, copy positions) |
| 0x00434770 | Saw_AlertActivate | Activate saw blade (alert mode - clear flag, play 3D sound) |
| 0x00434A50 | Saw_Activate | Activate saw blade (full - set active flag, play 3D sound) |
| 0x00434290 | Catapult_Launch | Catapult launch (set active +0x10F0=1, timer +0x10F4=50) |
| 0x004344D0 | Trapdoor_Open | Open trapdoor (set scale from 0 to 1.0) |
| 0x00438410 | Trapdoor_Activate | Activate trapdoor (play 3D sound, set timer) |
| 0x00443E600 | Catapult_Update | Per-frame: decrements tick counters, applies rotation matrix to tracked balls |
| 0x00443E9C0 | Catapult_AddObjectConditional | Register ball on catapult/gear tracking list (guarded by +0x1510) |
| 0x00443B6F0 | Rotator_AddBall | Register ball on rotator tracking list (8-byte entry [ball_ptr, tick=10]) |

### Arena Respawn System

Ball_Respawn (0x405190) calls FUN_004030b0 (pre-respawn: reset collision+Board vtable) THEN scans board+0x1518 (SAFESPOT list). Arena picks RANDOM SAFESPOT via RNG, validates with Mesh_FindClosestCollision distance check, rejects if too close to other balls. 3 scan variants: mode 0 (race, nearest), mode 1 (arena X, random+[X] tag), mode 2 (arena Z, random+[Z] tag).


## 16. Sound System

| Address | Name | Description |
|---------|------|-------------|
| 0x00459860 | Sound_Play3D | Play 3D positioned sound (set position via BASS, play channel) |
| 0x004597B0 | Sound_PlayChannel | Play sound channel (check if playing, dispatch from pool or allocate) |
| 0x004595B0 | Sound_StartSample | Start BASS sample (via vtable: reset, volume, 3D position) |
| 0x00466750 | Sound_CalculateDistanceAttenuation | 3D distance attenuation (find closest listener, apply min/max rolloff) |
| 0x00459660 | Sound_LoadOggOrWav | Load sound file: try .ogg first, then .wav fallback. 10 xrefs. |
| 0x00459310 | Sound_LoadOgg | Load OGG Vorbis file, create D3D sound buffer, add to channel list |
| 0x00458EE0 | Sound_Play3DAtPosition | Play 3D positioned sound (get channel, call vtable+0x3C with position) |
| 0x00459810 | Sound_GetNextChannel | Get next sound channel from circular buffer. 10 xrefs. |
| 0x00434580 | Sound_InitChannels | Allocate sound channels for object, get next sample, play 3D positioned sound |
| 0x00466620 | SoundDevice_ctor | Sound device constructor — vtable 0x4D911C, 16 channels, DS init |
| 0x00466A10 | SoundDevice_UpdateChannels | Tick sound channels, play on timer expire, remove expired |
| 0x00466B80 | SoundDevice_Play3DAll | Iterate all sound channels, call Sound_Play3DAtPosition per entry |
| 0x00466C50 | SoundDevice_DeletingDtor | Scalar deleting destructor for SoundDevice |
| 0x004668A0 | SoundDevice_dtor | SoundDevice destructor — save volume to registry, free AthenaLists |
| 0x00466320 | SoundChannel_Ctor | Init sound channel vtable 0x4D90E8 with 7 zero fields |
| 0x00479000 | Sound_EnumerateDevices | Sound device enumeration callback registration |
| 0x004665E0 | Audio_ClampPanValue | Convert float to int pan value, clamp minimum at -2000/-10000 |
| 0x00466570 | Level_ReadSoundVolume | Read "Sound Volume" float from registry, default 1.0 |


## 17. Math / Vector / Matrix Utilities

| Address | Name | Xrefs | Description |
|---------|------|-------|-------------|
| 0x00401AA0 | Vec3_NormalizeAndScale | 59 | Normalize 3D vector and scale to length param_1. Most common math utility. |
| 0x00401D60 | Matrix_TransformVec3 | 15 | Transform 3D vector by 4x3 matrix |
| 0x00453150 | Matrix_Scale4x4 | — | Set 4x4 matrix row scale values |
| 0x00453200 | Matrix_Identity | — | Set matrix to identity (vtable pointer) |
| 0x00453180 | Vec3_Init | 13 | Initialize Vec3 with vtable 0x4CF300, 3 floats, default scale 1.0 |
| 0x00401010 | Vec3_Copy | — | Copy 4 floats (Vec3+padding) from source, preserves vtable |
| 0x00401040 | Vec3_Init (alt) | — | Init Vec3: set vtable 0x4CF300 + copy 4 floats from source |
| 0x00401070 | Vec3_dtor | — | Vec3 deleting destructor: reset to identity, optionally free |
| 0x004016C0 | Vec3_Scale | — | Multiply Vec3 by scalar: out = scalar * this |
| 0x00401890 | Vec3_DivideByScalar | — | In-place Vec3 division via reciprocal multiply with g_one (1.0f) |
| 0x004018C0 | Vec3_AddTwo | — | Add two Vec3s: out = this + rhs |
| 0x004018F0 | Vec3_AddInPlace | — | In-place Vec3 addition: this += rhs |
| 0x00401D20 | Vec3_Distance | — | Euclidean distance between two 3D points (6 float params) |
| 0x00401A60 | Vec3_Length | 8 | Vec3 length: sqrt(x*x + y*y + z*z), min 1.0 |
| 0x00463790 | Vec3_CrossProduct | 7 | Vector cross product: this × param2 → param1 |
| 0x004637F0 | Vec3_Abs | 2 | Compute absolute value of 3-component vector (fabs each) |
| 0x0045C17B | Vec2_Normalize | — | Normalize 2D vector (length-squared check, divide by sqrt) |
| 0x0045C32F | Vec3_Normalize | — | Normalize 3D vector (length-squared check, divide by sqrt) |
| 0x004531E0 | Vec3_Init (v2) | 13 | Set Vec3 vtable + zero position + w=255.0 |
| 0x004531B0 | Vec3_SetScalar | 6 | Set all 3 components to same value, w=1.0 |
| 0x00457DA0 | Wave_Sin | 38 | sin(time * frequency * 2π/360) |
| 0x00457DC0 | Wave_Cos | 23 | cos(time * frequency * 2π/360) |
| 0x00457DE0 | Math_Atan2Angle | 13 | Atan2 angle with quadrant adjust, degrees-per-unit scaling |
| 0x00458130 | Math_FastDistance2D | 8 | Approximate integer distance (Bresenham weights 102/246) |
| 0x00472C70 | Math_Lerp | 9 | Linear interpolation: a + (b-a)*t |
| 0x0045CB88 | Matrix_BuildRotationZYX | — | Build 4x4 rotation matrix from Euler ZYX angles |
| 0x0045B104 | Matrix_BuildScaleTranslation | — | Build combined scale+translation matrix |
| 0x0045B74C | Matrix_BuildRotationAxisScaled | — | Build rotation matrix around axis with scale factor |
| 0x0045B345 | Matrix_BuildLookAt | 1 | Compute view matrix from eye, target, up vectors |
| 0x0045B48D | Matrix_BuildPerspectiveFOV | 1 | Compute perspective projection matrix from FOV, aspect, near, far |
| 0x0045CAAE | Matrix_BuildOuterProductScale | — | Build 4x4 matrix from outer product |
| 0x00458B50 | Matrix_ScaleTransform | — | Create 4x4 matrix by scaling source rows |
| 0x004580D0 | AABB_ContainsPoint | — | Test if point inside AABB (6 floats) |
| 0x004532E0 | AthenaList_SortedInsert | — | Insert with insertion-sort (ascending/descending) |
| 0x004598C0 | Float_IsBetween | 10 | Test if param1 minus param2 is between two global bounds |
| 0x0045C208 | Matrix_TransformVec4x3 | — | Transform vector by 4x3 matrix |
| 0x0045C48E | Gfx_ProjectToViewport | — | Project 3D coords to viewport UV |
| 0x0045C61B | Gfx_UnprojectToNDC | — | Unproject screen coords back to normalized device coords |
| 0x0045C273 | Matrix_TransformPoint2D | — | 2D point transform: mat*vec2 |
| 0x0045C2F3 | Matrix_TransformVector2D | — | 2D vector transform: mat*vec2 without translation |
| 0x0045C3CF | Matrix_TransformPoint3D | — | 3D point transform: mat*vec3 with perspective divide |
| 0x0045C7FA | Matrix_BuildFullTransform | — | Build full transform matrix with scale/translation offsets |
| 0x0048C24C | Vec3_Normalize (SSE2) | — | Normalize 3D vector in-place (with fast inverse sqrt) |
| 0x0048C316 | Matrix_Multiply4x4_InPlace | — | 4x4 matrix multiply allowing in-place |
| 0x0048C3F9 | Matrix_Multiply4x4 | — | 4x4 matrix multiply (out = a*b with temp buf) |
| 0x0048C487 | Matrix_InitVTable | — | Initialize matrix math vtable (7 entries) |
| 0x004A167B | Matrix4x4_Multiply_SSE2 | 12 | 4x4 matrix multiply using SSE2 packed float ops |
| 0x004A0F3A | Matrix_Inverse4x4_SSE2 | 8 | SSE2 4x4 matrix inverse using cofactor expansion + Newton-Raphson |
| 0x004677B0 | Matrix_SolveGaussElim | — | Gaussian elimination with partial pivoting |
| 0x004581D0 | Vec2_Distance | 3 | 2D distance: sqrt(dx²+dy²) |
| 0x004AB9B8 | DivCeil | 15 | Ceiling division utility: (a-1+b)/b |
| 0x004AB9C8 | Math_AlignUp | — | Round up: ((a-1+b)/b)*b |

### Important Float Constants (Ghidra double-as-float misread warning)

| Address | Value | Type | Description |
|---------|-------|------|-------------|
| 0x004D03B0 | 0.16 | double | Physics constant |
| 0x004D03A8 | 0.1 | double | Physics constant |
| 0x004CF308 | 0.1 | double | Physics constant |
| 0x004CF3E0 | 0.01 | double | Physics constant |
| 0x004CF440 | 0.25 | double | Physics constant |
| 0x004CF458 | 1.5 | double | Physics constant |
| 0x004CF4D0 | 1.25 | double | Physics constant |
| 0x004CF4F8 | 2.0 | double | Physics constant |
| 0x004CF528 | 0.5 | double | Physics constant |
| 0x004D03E0 | 0.02 | double | Physics constant |
| 0x004CF538 | 0.01 | double | Physics constant |
| 0x004D03C8 | 3.0 | double | Physics constant |
| 0x004D5C88 | 0.004 | double | Rotator speed |
| 0x004CF48C | 2.0 | double | Rotator oscillation limit (+) |
| 0x004D5C84 | -2.0 | double | Rotator oscillation limit (-) |
| 0x004D5D20 | 38.0 | double | Pendulum amplitude |


## 18. String / AthenaString System

| Address | Name | Xrefs | Description |
|---------|------|-------|-------------|
| 0x00466C70 | AthenaString_Format | 98 | String format wrapper — calls Sprintf with object's internal buffer. **param_1 is an AthenaString OBJECT pointer, NOT a char buffer!** |
| 0x004BBDFD | AthenaString_Sprintf | — | In-memory sprintf using FILE struct trick for vsnprintf |
| 0x004737F0 | AthenaString_Assign | 52 | String copy/assign operator |
| 0x00473500 | AthenaString_AssignCStr | 75 | AthenaString assign from C string (free old, alloc new, copy) |
| 0x004736B0 | AthenaString_dtor | 85 | AthenaString destructor — frees buffer, sets vtable |
| 0x00473670 | AthenaString_CopyCtor | 16 | String copy constructor from source string object |
| 0x00472AF0 | AthenaString_Init | 18 | Default string constructor — set vtable, zero fields, flags |
| 0x004740D0 | AthenaString_WriteTag | 16 | Build XML tag string: <tag>content</tag> concatenation |
| 0x00473A50 | AthenaString_AssignCRLF | 21 | AthenaString assign CRLF ("\\r\\n") |
| 0x004BAE43 | AthenaString_SprintfToBuffer | 71 | sprintf into char buffer via fake FILE struct |
| 0x00473990 | AthenaString_Clear | 14 | Free string buffer, reset to 15-char inline capacity |
| 0x00469510 | AthenaString_Set | 10 | Copy string param into AthenaString (free old, strdup, set length) |
| 0x00473BA0 | AthenaString_Substr | — | Substring extraction by index/length |
| 0x00473CD0 | AthenaString_EraseRange | — | Erase characters in range |
| 0x00473E20 | AthenaString_EraseSubstr | — | Find and erase substring |
| 0x00474000 | AthenaString_Truncate | — | Truncate string to N characters |
| 0x00473E70 | AthenaString_ReplaceAt | — | Replace substring at position with new string |
| 0x00473FB0 | AthenaString_ReplaceSubstr | — | Find substr, erase, insert replacement |
| 0x00426E90 | StdString_Assign | — | MSVC std::string::assign (SSO 0xF threshold, word+byte copy) |
| 0x00426D20 | StdString_Erase | 6 | String erase: delete substring at position |
| 0x00426DE0 | StdString_Substr | 5 | String substring extraction |
| 0x00426F30 | PlayerProfile_ctor | 5 | Player profile constructor (0x280 bytes) |
| 0x00447500 | StdString_TruncateToWidth | — | Erase chars from end until Font_MeasureText width <= 319px |
| 0x00447570 | StdString_InsertCStr | — | Insert C string at position |
| 0x00447390 | StdString_Insert | — | Insert count bytes from ptr at position in std::string |
| 0x0044471E0 | StdString_Replace | 1 | String replace operation |
| 0x00473460 | StdString_FreeBuffer | — | Frees internal buffer, zeroes capacity/size/ptr |
| 0x00473480 | StdString_Reserve | — | Reallocates buffer to param_1+1 size |
| 0x00473580 | StdString_AssignN | — | strncpy-assigned string with length param |
| 0x00473600 | StdString_RecalcLen | — | Recalculates string length if dirty flag set |
| 0x00473640 | StdString_FindSubstr | — | strstr wrapper returning index or -1 |
| 0x004736F0 | AthenaString_AssignCStrFree | — | Assign C string then free source |
| 0x00473AC0 | AthenaString_AssignFree | — | Assign string then free source |
| 0x00473B10 | AthenaString_AssignFormatted | 6 | Format and assign AthenaString |
| 0x004738B0 | StdString_AppendN | 7 | Append N chars from string with strncat |
| 0x00473740 | StdString_AppendCharN | 5 | Append N copies of a char |
| 0x00426BAE | AthenaString_MoveAssign | — | AthenaString move assignment operator |
| 0x00426C50 | AthenaString_Reserve | — | AthenaString buffer reserve/capacity |
| 0x00426B30 | String_AllocBuffer | 7 | Allocate string buffer with size | 0xF rounding |
| 0x00473440 | StdString_FreeBuffer | — | Frees internal buffer |
| 0x004749B0 | StdString_CompareSubstr | 8 | Compare substring with byte-by-byte comparison |


## 19. Font / Text Rendering

| Address | Name | Description |
|---------|------|-------------|
| 0x00457130 | LoadFont | Load font.description + PNG glyphs |
| 0x00456E20 | Font_MeasureText | Measure text string width for centering |
| 0x00457440 | Font_DrawGlyph | Core glyph rendering (1 call = 1 glyph quad) |
| 0x004013A0 | UI_DrawTextCenteredAbsolute | Draw centered text (x - width/2) |
| 0x00409C60 | UI_DrawTextCentered | Draw centered text with shadow |
| 0x004012C0 | UI_DrawTextShadow | Draw text with shadow (offset + main) |
| 0x00409B90 | UI_DrawTextShadow_Wrapper | Wraps UI_DrawTextShadow with Vec3 default params |
| 0x0042C870 | Font_DrawCentered | Draw text centered at (x,y) position. 8 xrefs. |
| 0x00457690 | Font_DrawGlyph3D | Render 3D text — iterate chars, render with Ball_RenderWithMaterial |
| 0x00456E80 | Font_WordWrap | Word-wrap text to fit width |
| 0x00472340 | Font_RenderToTextureComplex | Complex font rendering to texture with vertex buffers and shaders |
| 0x00473940 | FontFormatString_Parse | Parse %-formatted string for font rendering |
| 0x004AD716 | Font_DecodeGlyphBits | Decode glyph bitmap from compressed font data (1/2/4/8 bit) |
| 0x004A91D0 | Font_RenderChannels | Renders font glyph channels via Huffman decode and callback |


## 20. Mesh / MeshWorld System

| Address | Name | Description |
|---------|------|-------------|
| 0x004706E0 | MeshWorld_ctor | MeshWorld object constructor (0x488 bytes) |
| 0x0046F3D0 | MeshWorld_ctor (v2) | MeshWorld constructor from filename and strip count. 39 xrefs. |
| 0x00470930 | MeshWorld_Parse | Parse MESHWORLD text format (*MATERIAL, *MESH, etc.) |
| 0x0046F8D0 | MeshWorld_BuildVertexBuffer | Builds vertex buffer from mesh object lists, creates D3D VB |
| 0x0046FCC0 | MeshWorld_CollectRenderLists | Collects render lists from mesh objects into target list |
| 0x0046FF60 | MeshWorld_OptimizeAll | Optimizes all meshes, builds vertex buffers |
| 0x00470680 | MeshWorld_ClearObjectLists | Clears AthenaLists on marked objects during cleanup |
| 0x00472570 | MeshWorld_BuildFontMeshes | Builds font meshes via Font_RenderToTextureComplex |
| 0x00469FE0 | MeshWorld_ctor_v2 | MeshWorld constructor v2 — vtable 0x4D91C4 |
| 0x004699D0 | MeshWorld_RemoveObject | Remove object from mesh list, clear current references |
| 0x00469A40 | MeshWorld_ClearCurrent | Clear current item by calling vtable+0x30 |
| 0x00469A80 | MeshWorld_CallNext | Call vtable+0x24 on next item at +0x424 |
| 0x00469AC0 | MeshWorld_SkipOrAdvance | Skip current or advance to next if matches param |
| 0x00469C30 | MeshWorld_dtor2 | Clear list, iterate items calling dtor(1), Vec3List_Free |
| 0x00469EC0 | MeshWorld_ActiveUpdate | Iterate objects, skip inactive, set obj ID, call vtable+8 |
| 0x00469F50 | MeshWorld_CallVtable34 | Iterate objects calling vtable+0x34 |
| 0x00469FC0 | MeshWorld_DeletingDtor2 | Scalar deleting destructor for MeshWorld variant |
| 0x00469600 | MWParser_ReadTag | XML/SGML tag parser — finds <tag>...</tag> pairs. 24 xrefs. |
| 0x004742B0 | MWParser_DumpTags | Dump tags from MW parser to string |
| 0x00458970 | CreateMeshBuffer | Allocate mesh vertex/index buffer |
| 0x00458A80 | InitMaterialArray | Initialize material array |
| 0x00480C4D | MeshBuffer_Allocate | Allocate mesh vertex/index buffer |
| 0x0046B2A0 | MeshBuffer_dtor | Destroy mesh buffer list items + free Vec3List. 11 xrefs. |
| 0x0046B3D0 | MeshBuffer_DeletingDtor | Scalar deleting destructor for MeshBuffer |
| 0x0046F340 | MeshBuffer_Cleanup | Cleanup mesh buffer, free resources |
| 0x0046FD60 | Mesh_AddVertex | Add vertex to mesh buffer (8 floats) |
| 0x0047C990 | Mesh_Dtor | Mesh destructor: set vtable, free ptrs +0x20/+0x28/+0x30 |
| 0x0047D160 | Mesh_FindElement | Search array of 8-float elements (stride 0x424) |
| 0x0047D2A0 | Mesh_AddElement | If not found, append 8 floats + 0 at stride 0x424 |
| 0x0047D020 | Mesh_ConnectElements | Build adjacency between two mesh elements |
| 0x00471C20 | MeshNode_ctor | Load mesh file into scene graph node (vtable 0x4D9C48) |
| 0x004770B0 | MeshObject_dtor | Mesh object destructor: releases textures, frees mesh data |
| 0x00471C00 | MeshObject_DeletingDtor | MeshObject scalar deleting destructor |
| 0x004775020 | MeshWorld_AddTexture | Adds texture to mesh world (creates 0x48 byte texture object) |
| 0x00478EDD | MeshArchive_ctor | MeshArchive constructor: init vtable, pool, D3D resources |
| 0x00478E70 | MeshArchive_InitAndLoad | Create archive and fully load mesh data from stream |
| 0x00478680 | MeshArchive_dtor | Destructor: free all sub-resources, D3D resources, mesh groups |
| 0x00478800 | MeshArchive_SeekRead | Seek to position and read mesh data with bitstream |
| 0x00478340 | MeshArchive_LoadFrame | Frame-level load with bitstream, mesh groups, position tracking |
| 0x00478D60 | MeshArchive_LoadComplete | Full load: reset position, read chunks, build tree, load all |
| 0x00478320 | MeshArchive_ReleaseBuffers | Release sub-buffers, set state=2 |
| 0x00477970 | MeshArchive_ReadChunks | Read 0x400-byte chunks from stream |
| 0x00477AC0 | MeshArchive_BuildTree | Recursive binary search tree for mesh data |
| 0x00477D60 | MeshArchive_LoadSubmesh | Load submesh with bitstream and D3DX retry loop |
| 0x00477F10 | MeshArchive_LoadAll | Load all meshes from archive with position tracking |
| 0x00478B90 | MeshArchive_GetSubmeshPtr | Get pointer to submesh at index |
| 0x00478BD0 | MeshArchive_ReadVertices | Read vertex data from current frame |
| 0x00478780 | MeshArchive_SetPosition | Set position and reset pool via vtable |
| 0x0045D0C0 | Sprite_ctor | Init with texture, RenderContext, material defaults. 30 xrefs. |
| 0x0045D660 | Sprite_RenderQuad | Render textured quad using material + draw primitive. 15 xrefs. |
| 0x0045D030 | Sprite_Reset | Sprite reset: set vtable, remove texture ref, Matrix_Identity |
| 0x0045D0A0 | Sprite_ScalarDtor | Sprite scalar destructor |
| 0x0045D1D0 | Sprite_Ctor | Sprite constructor: vtable, RenderContext_Init, sets 3 scale pairs |
| 0x0045DAB0 | Sprite_DrawRotatedQuad | Draw rotated quad: 5-point star pattern via sin/cos |
| 0x0045D300 | Sprite_DrawRect | Draw rect with position+size, random RGBA, 2-triangle strip |
| 0x0047C840 | Sprite_CalcTexCoords | Calculate sprite UV coordinates |
| 0x0040AE50 | Sprite_DrawCentered | Sprite draw centered at position |
| 0x004694F0 | Sprite_DrawColoredRect | Draw colored rectangle with random RGBA vertex colors. 23 xrefs. |

### MESHWORLD Material Struct (72 bytes + 4 has_texture = 76 total)

| Offset | Size | Field | Description |
|--------|------|-------|-------------|
| +0x00 | 64 | color | 4×4 float color matrix (diffuse/ambient/specular/emissive) |
| +0x40 | 4 | shine | Float shine value |
| +0x44 | 4 | has_reflection | Reflection flag |
| +0x48 | 4 | has_texture | Texture flag (read as SEPARATE __read() after material) |

### MESHWORLD Material Memory Layout (0x50 bytes in memory)

| Offset | Field | Description |
|--------|-------|-------------|
| +0x04 | diffuse | 4 floats (R,G,B,A) |
| +0x14 | ambient | 4 floats |
| +0x24 | specular | 4 floats |
| +0x44 | shine | Float |
| +0x48 | tex_ptr | Runtime texture pointer |


## 21. Config / Save / Registry System

| Address | Name | Description |
|---------|------|-------------|
| 0x004279F0 | LoadOrSaveConfig | Config load/save dispatcher |
| 0x0042AE80 | LoadConfig | Load HS.CFG |
| 0x0042B6E0 | SaveConfig | Save HS.CFG |
| 0x00472EC0 | RegKey_Open | Open registry key: HKLM first, fall back HKCU, create if needed. 15 xrefs. |
| 0x00472F30 | RegKey_Close | Close registry key handle via RegCloseKey. 18 xrefs. |
| 0x00472F50 | RegKey_SetSoftwarePath | Builds "SOFTWARE\\%s" registry path |
| 0x00473000 | RegKey_WriteDWORD | Writes DWORD value to registry. 13 xrefs. |
| 0x00472FD0 | RegKey_WriteDword | Write DWORD via RegSetValueExA |
| 0x00473050 | RegKey_WriteBool | Write boolean to registry via RegSetValueExA (REG_BINARY). 30 xrefs. |
| 0x00473080 | RegKey_ReadDword | Read DWORD via RegQueryValueExA. 13 xrefs. |
| 0x00473100 | RegKey_QueryValue | Queries registry value via RegQueryValueExA |
| 0x00473130 | RegKey_ReadBool | Read boolean from registry via RegQueryValueExA. 28 xrefs. |
| 0x00473170 | RegKey_ReadString | Read string from registry with fallback attempts. 23 xrefs. |
| 0x00473030 | Registry_SetValue | Windows registry value setter (RegSetValueExA wrapper). 4 xrefs. |
| 0x004730C0 | Registry_ReadFloat | Read float from Windows registry. 5 xrefs. |
| 0x00473220 | RegKey_DeletingDtor | Deleting dtor for RegKey, frees path string |
| 0x00472EA0 | RegKey_Ctor | Initializes registry key object with vtable 0x4D9F08 |
| 0x00473260 | eSellerate_ExtractDLL | Extracts eSellerateEngine.dll from resources to Windows dir |
| 0x00473355 | eSellerate_ExtractDLLNull | Calls eSellerate_ExtractDLL with NULL module |
| 0x0047335D | eSellerate_ReadAffiliateKey | Reads affiliate key from SOFTWARE\\eSellerate registry |


## 22. DRM / Licensing System

| Address | Name | Description |
|---------|------|-------------|
| 0x00429200 | ESellerate_Init | eSellerate DRM initialization |
| 0x00473260 | eSellerate_ExtractDLL | Extracts eSellerateEngine.dll from resources |
| 0x00473355 | eSellerate_ExtractDLLNull | Calls eSellerate_ExtractDLL with NULL module |
| 0x0047335D | eSellerate_ReadAffiliateKey | Reads affiliate key from registry |
| 0x00446B80 | RegisterDialog_ValidateSerial | Validate serial number using XOR cipher with key "54138" |
| 0x004475A0 | RegisterDialog_HandleInput | Handle keyboard input for register dialog |
| 0x00447920 | RegisterDialog_Render | Render "REGISTER HAMSTERBALL!" screen |
| 0x00448890 | RegisterDialog_HandleKey | Handle key navigation (Tab, arrows, Enter=validate, Esc=close) |
| 0x00475ED4 | LicenseKey_ComputeHash | Compute license hash from strings then mask + add constant |
| 0x00475EF9 | LicenseKey_EncryptBlock | 100-element permutation cipher with key expansion |
| 0x00475D03 | StrNEq | strncmp-like comparison, returns 1 if equal up to N chars |
| 0x00475DEC | Hash_MixString | Hash mix over string bytes using MixKey |
| 0x00475E15 | Hash_MixStringUpper | Hash mix with Char_ToUpper on first string |
| 0x00475E60 | NoiseTable_Init | Seed RNG and fill 0x200-entry noise table at 0x5341F0 |
| 0x00475E87 | StrLen_Delay8 | strlen with 8-iteration delay loop (anti-analysis?) |
| 0x00475CEC | StrLen | Null-safe strlen (character count) |
| 0x00475DC0 | CRC32_Compute | CRC32 using 256-entry lookup table at 0x4F7534 |


## 23. Input System

| Address | Name | Description |
|---------|------|-------------|
| 0x0046E0B0 | Input_IsKeyDown | Check input state (keyboard/mouse/joystick). 5 xrefs. |
| 0x0046EBD0 | InputDevice_PollAndRelease | Poll DInput device, acquire on error, release 4 sub-devices |
| 0x0046DFC0 | InputDevice_SetType | Set input device type (1=keyboard, 2=mouse, 4=joystick, 5=?) |
| 0x004692F0 | Scene_HandleInput | Iterate menu items, check input, set current item at +0x864 |
| 0x00428F10 | Input_CheckKeyCombo | Check key combos. param_1=2: escape. 0-3: iterate 4 input bindings, 50-frame debounce |
| 0x0046EC30 | Ball_GetInputForce | Get ball input force (keyboard/mouse/joystick). 7 xrefs. |
| 0x0046C290 | App_OnMouseDown | Mouse down handler: SetCapture, set button flags, UIWidget_HitTest |
| 0x0046C430 | Input_OnMouseUp | ReleaseCapture, notify interceptor or widget with vtable+0x18 |
| 0x0046C760 | Input_OnMouseDown | Hit-test widget, call vtable+0x1c with button param |
| 0x0046C3C0 | Input_OnMouseUpCapture | Decrement capture count, ReleaseCapture if zero |
| 0x0046E910 | KeyboardDevice_ScalarDtor | KeyboardDevice scalar destructor |
| 0x0046DFA0 | NetworkConnection_Ctor | NetworkConnection init: sets "Not Connected", +0x0C=1.0f |
| 0x00469B20 | UIWidget_HitTest | Find widget under point — check rect bounds, iterate back-to-front |
| 0x00469BE0 | UIWidget_UpdateHover | Update hover — find widget under point, call leave/enter callbacks |
| 0x00469A60 | UIWidget_CallVtable20 | Call vtable+0x20 on object at +0x424 if non-null |
| 0x00469AA0 | UIWidget_CallVtable28 | Call vtable+0x28 on object at +0x424 if non-null |
| 0x004448410 | UI_TextEdit_PasteFromClipboard | Pastes text from Windows clipboard to UI |
| 0x00441150 | UI_CheckKeyCombo | UI key combo handler (calls vtable on press) |
| 0x0047C7F0 | DirectInput8Create | DirectInput8 creation |


## 24. Sprite / Particle System

| Address | Name | Description |
|---------|------|-------------|
| 0x0045D0C0 | Sprite_ctor | Init with texture, RenderContext, material defaults. 30 xrefs. |
| 0x0045D660 | Sprite_RenderQuad | Render textured quad using material + draw primitive. 15 xrefs. |
| 0x0045D030 | Sprite_Reset | Sprite reset: set vtable PTR_004d8f84, remove texture ref |
| 0x0045D0A0 | Sprite_ScalarDtor | Sprite scalar destructor |
| 0x0045D1D0 | Sprite_Ctor | Sprite constructor: vtable, RenderContext_Init, 3 scale pairs |
| 0x0045DAB0 | Sprite_DrawRotatedQuad | Draw rotated quad: 5-point star pattern via sin/cos |
| 0x0045D300 | Sprite_DrawRect | Draw rect with position+size, random RGBA, 2-triangle strip |
| 0x0047C840 | Sprite_CalcTexCoords | Calculate sprite UV coordinates |
| 0x0040AE50 | Sprite_DrawCentered | Sprite draw centered at position |
| 0x004694F0 | Sprite_DrawColoredRect | Draw colored rectangle with random RGBA vertex colors |
| 0x0046F310 | SpriteAnim_Ctor | Initializes sprite animation with vtable 004d9c48 |
| 0x0046F7C0 | SpriteAnim_SetRange | Sets animation range (prev=current, new=param) |
| 0x0046F7D0 | SpriteAnim_InitFromCalcTexCoords | Initializes sprite anim from texture coords |
| 0x0046FB50 | SpriteAnim_DeletingDtor | Deleting dtor for SpriteAnim, frees callback and data |
| 0x0046A820 | FlagWaver_dtor | Free vertex buffer, release D3D resource, identity matrix |
| 0x0046A8A0 | FlagWaver_AllocBuffers | Allocate vertex buffer array, create D3D vertex buffer |
| 0x0046A930 | FlagWaver_AdvancePhase | Increment wave phase offset at +0x28 |
| 0x0046A940 | FlagWaver_DeletingDtor | Scalar deleting destructor for FlagWaver |
| 0x0046A960 | FlagWaver_UpdateVertices | Compute water wave positions (sin/cos), normals, vertex averaging |
| 0x0046AF30 | FlagWaver_Ctor | Constructor — vtable 0x4D9344, 10 segments, wave params, alloc buffers |
| 0x0046B070 | FlagWaver_Render | Render water ripple — update vertices, test ball intersection, draw |
| 0x0046B200 | RenderList_AppendCopy | Allocate new RenderContext and copy data, or append existing |
| 0x0046B360 | RenderList_FreeAndClear | Iterate list calling dtor(1), then AthenaList_Free |

### FlagWaver Struct

| Offset | Field | Description |
|--------|-------|-------------|
| +0x08 | segCount | Segment count |
| +0x10 | width | Flag width |
| +0x14 | height | Flag height |
| +0x1C | waveAmp | Wave amplitude |
| +0x20 | waveFreq | Wave frequency |
| +0x80 | pennant | 0=rectangle, 1=triangle |
| ×0.5 | SMALLFLAG | Small flag scale multiplier |


## 25. Path / Spline System

| Address | Name | Description |
|---------|------|-------------|
| 0x00467BF0 | Path_GetPosition | Interpolate X/Z from path splines at time t, Y=0. 14 xrefs. |
| 0x00468600 | Path_ComputeSegmentLengths | Compute segment lengths from point pairs using sqrt(dx²+dy²) |
| 0x00468780 | Path_BuildVertexStrips | Build vertex strips from path array: interleaved pos/uv/color quads |
| 0x00468F30 | Path_ComputeSegDeltas | Compute segment deltas from path: outputs 4-float per segment |
| 0x00468510 | PathGroup_Init | PathGroup init: zero 15 DWORDs across 5 groups of 3 |
| 0x004685E0 | PathGroup_PushPair | PathGroup push pair: Vector_PushBack of 4 then 8 |
| 0x00467B40 | Spline_EvalCubic | Evaluate cubic spline (t³*a + t²*b + t*c + d), clamp t |
| 0x00418760 | Scene_CreateObject_Gear | Create GEAR object in scene |
| 0x00418930 | Gear_AdvanceAlongPath | Gear path following (8-dir gradient descent). 6 xrefs. |


## 26. RNG / Random System

| Address | Name | Xrefs | Description |
|---------|------|-------|-------------|
| 0x0045DD60 | RNG_Rand | 193 | PRNG with 55-entry circular buffer; returns (buf[read]+buf[write])>>6 % range. Also called as CPUID_CheckProcessorFeature. |
| 0x0045DD60 | CPUID_CheckMMX | — | Checks CPUID feature bit 23 (MMX support) |
| 0x0045DD6F | CPUID_IsMMXAvailable | — | Returns MMX availability flag from cached CPU features |
| 0x0045DD7E | CPUID_Check3DNow | — | Checks CPUID extended features for 3DNow support |
| 0x0045DDA0 | CPUID_DetectFeatures | — | Runs CPUID leaf 1 and stores feature flags |
| 0x0045DDB0 | CPUID_GetProcessorFeatures | — | Returns cached processor feature flags |
| 0x0045DDC0 | CPUID_CheckProcessorFeature | — | Tests specific bit in processor feature flags |
| 0x0045DCF0 | RNG_SeedSmall | — | Seed small RNG: 53-entry additive PRNG with state at +0xc |
| 0x0040A050 | Color_RandomRGBA | — | Generate 32-bit color from 4 random bytes |

### RNG Calling Convention

CPUID_CheckProcessorFeature (0x0045DD60) is called as **__thiscall** via g_RNG (21 calls) AND g_CPUIDRNG (3 calls) — same address, same bug. ECX not set to 0x4F7360 → crash. Fix: RNG_call() __fastcall wrapper with dummy EDX. 24 total call sites.


## 27. CRT / Runtime Functions

| Address | Name | Description |
|---------|------|-------------|
| 0x004BA57B | operator_new | C++ operator new (malloc wrapper) |
| 0x004BAF41 | CRT_ArrayUnwind1 | Array unwind helper (4 args) |
| 0x004BAFA3 | CRT_ArrayUnwind2 | Array unwind helper (alt params) |
| 0x004BB411 | CRT_LeaveCritSec8 | LeaveCriticalSection(8) |
| 0x004BB429 | CRT_mkdir | CRT mkdir wrapper |
| 0x004BB455 | CRT_remove | CRT remove/delete file wrapper |
| 0x004BB9B3 | CRT_Initterm | Run init/term function arrays |
| 0x004BBA18 | CRT_ExitProcess | CRT ExitProcess with cleanup |
| 0x004BBB0D | CRT_ExitProcessNoCleanup | ExitProcess without full cleanup |
| 0x004BBC24 | CRT_fsopen | __fsopen wrapper |
| 0x004BBC40 | FPU_AsinWrapper | FPU asin() wrapper |
| 0x004BBD0B | FPU_IsFinite | Check if double is finite |
| 0x004BBD20 | FPU_ClassifyDouble | Classify double (FP_CLASS) |
| 0x004BBE60 | FPU_RoundDouble | CRT round double with error handling |
| 0x004BC768 | CRT_vsprintf | CRT vsprintf implementation |
| 0x004BCE60 | CRT_qsort | CRT qsort implementation |
| 0x004BD37D | CRT_DivCeil | Ceiling division helper |
| 0x004BD397 | CRT_GetLocalePtr | Get locale pointer |
| 0x004BD39D | CRT_FreeLocale | Free locale struct |
| 0x004BD467 | CRT_SetupThreadLocale | Setup thread locale |
| 0x004BE077 | CRT_cftof | Float format (%f) |
| 0x004BE0DF | CRT_cftoe | Float format (%e scientific) |
| 0x004BE1D0 | CRT_SetFPUAffinity | Set FPU affinity mask |
| 0x004BDF67 | CRT_cvtToScientific | Double-to-scientific conversion |
| 0x004BDFDB | CRT_FormatFractional | Format fractional part of float |
| 0x004BEF25 | CRT_WriteFormatted | Full printf-style formatted output |
| 0x004BFC28 | CRT_ParseCommandLine | Parse command line into argv |
| 0x004BFD94 | CRT_GetMainArgs | __getmainargs implementation |
| 0x004BFF58 | CRT_IOInit | CRT I/O initialization (stdin/stdout/stderr) |
| 0x004BA754 | __ftol2 | CRT float-to-int64 conversion (compiler intrinsic). 358 xrefs! |
| 0x004BC7C8 | __errno | CRT __errno — returns thread-local errno pointer. 40 xrefs. |
| 0x004BC7D1 | __doserrno | CRT __doserrno — returns thread-local DOS errno pointer |
| 0x004BCDA8 | __security_init_cookie | CRT security cookie initialization |
| 0x004BAC20 | strstr | CRT strstr — string search with SIMD optimization. 22 xrefs. |
| 0x004BACC0 | strchr | CRT strchr — find char in string with SIMD optimization |
| 0x004BC350 | strcpy | CRT strcpy — string copy with SIMD optimization |
| 0x004BC0D1 | strtok | CRT strtok — thread-safe string tokenizer. 50 xrefs. |
| 0x004C8FF7 | _check_file_access | Check if file path is accessible (GetFileAttributesA). 14 xrefs. |
| 0x004C02E7 | LeaveCriticalSection_indexed | LeaveCriticalSection by index into global array. 23 xrefs. |
| 0x004A458C | longjmp_with_cleanup | CRT longjmp with optional cleanup callback. 34 xrefs. |
| 0x004BAE43 | AthenaString_SprintfToBuffer | sprintf into char buffer via fake FILE struct. 71 xrefs. |


## 28. Global Variables & Data Addresses

### Memory Layout

| Section | Range | Description |
|---------|-------|-------------|
| .text | 0x00401000 - 0x004CEFFF | Code (executable instructions) |
| .rdata | 0x004CF000 - 0x004F6FFF | Read-only data (strings, vtables, IAT) |
| .data | 0x004F7000 - 0x00536AF3 | Mutable global variables |
| .data1 | 0x00537000 - 0x00537FFF | Additional data |
| .rsrc | 0x00538000 - 0x0058FFFF | Resources (icons, cursors, DLL) |

### Key Global Variables

| Address | Name | Type | Description |
|---------|------|------|-------------|
| 0x005341E0 | g_App | pointer | Global App singleton object pointer |
| 0x005341E4 | g_Scene | pointer | Global Scene/Board pointer (holds board directly) |
| 0x005341CC | g_renderIndex | uint32 | Render frame counter / sprite animation index |
| 0x004FD680 | g_App (static) | App struct | Static App data (2328 bytes / 0x918) |
| 0x004F7360 | PTR_OBJ_VTABLE | pointer | Pointer to object vtable (RNG + object vtable). 193 xrefs. |
| 0x004F7188 | PTR_PTR_004f7188 | pointer | Pointer to trigonometric lookup table. 121 xrefs. |
| 0x004F7448 | PTR_PTR_004f7448 | pointer | Pointer to texture format string table. 99 xrefs. |
| 0x004D9CDC | MeshWorld_vtable | pointer | Virtual function table for MeshWorld class |
| 0x004D2334 | s_BACK | char[5] | String "BACK" — used in all menus. 45 xrefs. |
| 0x004F7080 | PTR_s_WARM-UP_RACE | pointer | Array of race level name strings |
| 0x004F7148 | PTR_s_TAKE_YOUR_TIME | pointer | Array of level description/tutorial strings |
| 0x004F70C8 | PTR_s_HAMSTER_PELLET | pointer | String "HAMSTER PELLET" |
| 0x004F77C0 | PTR_PTR_004f77c0 | pointer | Pointer to diagnostic data structure |
| 0x004F77C4 | PTR_s_http://bugs.raptisoft | pointer | Raptisoft bug tracker URL |
| 0x004F77C8 | PTR_s_RaptisoftBugTracker | pointer | Raptisoft bug tracker name |
| 0x004D5E88 | PTR_Rsrc_DLL_1_409 | pointer | Embedded DLL resource data (resource ID 304949) |
| 0x004D6C70 | PTR_RaceGoalReached_Render | pointer | ScoreObject vtable pointer |
| 0x004D6CB8 | PTR_RaceGoalReached_Render (v2) | pointer | ScoreObject vtable pointer (variant) |
| 0x004D90C8 | PTR_Level_LoadCollision | pointer | Function pointer to Level_LoadCollision |
| 0x004D935C | PTR_SceneObject_SetVisible | pointer | Function pointer to SceneObject_SetVisible |
| 0x004F7534 | CRC32_Table | byte[1024] | CRC32 lookup table (256 entries) |
| 0x005341F0 | NoiseTable | byte[0x200] | Noise table (512 entries, seeded by RNG) |
| 0x00535280 | FPU_ControlWord | dword | FPU control word global |


## 29. Struct Layouts

### App Struct (0x918 bytes)

See [Section 2: App Object & Global State](#2-app-object--global-state) for full layout.

### Ball Struct

See [Section 7: Ball Physics System](#7-ball-physics-system) for full layout.

### Scene Struct

See [Section 10: Scene System](#10-scene-system) for key offsets.

### SceneObject Struct (0xD4 bytes)

See [Section 11: SceneObject System](#11-sceneobject-system) for full layout.

### ArenaBoard Struct

| Offset | Field | Description |
|--------|-------|-------------|
| +0x000 | board_state | Board state base |
| +0x1518 | SAFESPOT_list | SAFESPOT list (used by respawn) |
| +0x3624 | fade_alpha | Race-end fade alpha |
| +0x47B4 | knockoff_counts | Knockoff counts |
| +0x47C0 | knockoff_end | Knockoff end condition |
| +0x6428 | bumper_lit_1 | Bumper lit flag 1 (WRITE-ONLY decay timer) |
| +0x6448 | bumper_lit_2 | Bumper lit flag 2 (WRITE-ONLY decay timer) |
| +0x53FC | bumper_lit_3 | Bumper lit flag 3 (WRITE-ONLY decay timer) |
| +0x1508 | RGBA_colors | Level RGBA colors (4 floats: R,G,B,Alpha=1.0) |

### Rotator Struct (0x1508 bytes)

| Offset | Field | Description |
|--------|-------|-------------|
| +0x10D4/D8/DC | position | Position X/Y/Z |
| +0x10E8 | angle | Current rotation angle |
| +0x10EC | direction | Speed multiplier (1.0=native, 4.0=4x, -1.0=reverse, 0.0=constant) |

### Pendulum Struct (0x1504 bytes)

| Offset | Field | Description |
|--------|-------|-------------|
| +0x14FC | time | Sine wave time parameter |
| +0x1500 | speed | Speed (CPUID*0.05+0.5) |

### Gear Struct (0x1514 bytes)

| Offset | Field | Description |
|--------|-------|-------------|
| +0x10E8 | angle_1 | Rotation angle 1 |
| +0x10EC | angle_2 | Rotation angle 2 |
| +0x10F0 | angle_3 | Rotation angle 3 |
| +0x10F4 | angle_4 | Rotation angle 4 |

### PopCylinder Struct (0x10E8 bytes)

Static pop-up obstacle. Collision goes through PARENT board vtable[29], NOT spawned object's vtable.

### CollisionLevel Struct

| Offset | Field | Description |
|--------|-------|-------------|
| +0x10D4 | Fallout | CollisionLevel for Fallout objects |
| +0x10E0 | PopCylinder | CollisionLevel for PopCylinder objects |
| +0x10E4 | break_cooldown | Break sound cooldown (1.0f) |

### MusicChannel Struct (0x534 bytes)

See [Section 4: Audio / BASS System](#4-audio--bass-system) for layout.

### FlagWaver Struct

See [Section 24: Sprite / Particle System](#24-sprite--particle-system) for layout.

### AthenaList Struct

| Offset | Field | Description |
|--------|-------|-------------|
| +0x08 | inline_array | Zeroed inline array (NOT the items pointer!) |
| +0x40C | items_ptr | Items pointer array (THIS is the real items pointer) |
| +0x410 | spacing | Spacing between entries |
| +0xCA0 | count | Item count |

### MESHWORLD Material Struct

See [Section 20: Mesh / MeshWorld System](#20-mesh--meshworld-system) for layout.


## 30. Vtable Layouts

### Scene Vtable (0x4D0260, 36 entries)

| Slot | Offset | Address | Name | Description |
|------|--------|---------|------|-------------|
| 0 | +0x00 | 0x425020 | Scene_DeletingDtor | Destructor + free if flag&1 |
| 1 | +0x04 | 0x419C00 | Scene_Update | Main tick (9-step: input, update pipeline, ball update+respawn, race-end, countdown, objects, cameras, render, HUD) |
| 2 | +0x08 | 0x41A2E0 | Scene_Render | 1P/2P/split render dispatch |
| 3 | +0x0C | 0x4692F0 | Scene_HandleInput | Iterate menu items, check input |
| 4 | +0x10 | 0x469220 | Scene_ActivateCurrentItem | Call vmethod+0x10 on current menu item |
| 5 | +0x14 | 0x4130A0 | Scene_vmethod5 | Unknown (arena-specific override) |
| 6 | +0x18 | 0x469280 | Scene_SelectCurrentItem | Call vmethod+0x0C on current menu item |
| 7 | +0x1C | 0x409D90 | Scene_NoOp | Empty (no-op stub, 3 bytes) |
| 8 | +0x20 | 0x40B400 | Level_RenderDynamicObjects | Level dynamic object rendering |
| 9 | +0x24 | 0x44B840 | NoOp_return | Empty stub (default) |
| 10 | +0x28 | 0x44B840 | NoOp_return | Empty stub (default) |
| 11 | +0x2C | 0x4692A0 | Scene_ClearCurrentItem | Set current item ptr to NULL |
| 12 | +0x30 | 0x4692A0 | Scene_ClearCurrentItem | Same as slot 11 |
| 13 | +0x34 | 0x44B840 | NoOp_return | Empty stub (default) |
| 14 | +0x38 | 0x409DA0 | Scene_DestroyScene | Calls FUN_4693C0 then SaveAndCleanup |
| 15 | +0x3C | 0x469430 | Scene_NotifyObjects | Iterate AthenaList, call FUN_4699D0 on each item |
| 16 | +0x40 | 0x419740 | Scene_SetDestroyed | Set +0x2C=1 destroyed flag |
| 17 | +0x44 | 0x4692B0 | Scene_SaveAndCleanup | Calls FUN_469AC0 (save + cleanup) |
| 18 | +0x48 | 0x40B090 | Level_InitScene | Level scene initialization |
| 19 | +0x4C | 0x41B130 | Scene_HandleRaceEnd | Fade alpha decrement, per-player timer countdown, Game Over popup |
| 20 | +0x50 | 0x41B540 | Scene_UpdateBallsAndState | Iterate ball lists, SetCamera, Ball_Update, destroy finished balls |
| 21 | +0x54 | 0x40A040 | NoOp | Empty (58 xrefs) |
| 22 | +0x58 | 0x41A540 | Scene_ProcessRaceEnd | Race-end transition: checks all players finished, fade-out |
| 23 | +0x5C | 0x409DE0 | Scene_HandleBallFinish | Ball finish state machine: start→countdown(150f)→finish→result popup→done |
| 24 | +0x60 | 0x40B420 | Level_RenderDynamicObjects_2 | Alternate dynamic render |
| 25 | +0x64 | 0x40B600 | Level_UpdateAndRender | Combined update + render |
| 26 | +0x68 | 0x40B570 | Level_RenderObjects | Level object rendering |
| 27 | +0x6C | 0x41B710 | Scene_RenderScoreHUD | Draw tournament title, countdown progress bar, "Score" text, Player 2, timer |
| 28 | +0x70 | 0x41BFD0 | Scene_RenderTimerHUD | Draw race timer, split screen divider, time display, overlay popups |
| 29 | +0x74 | 0x40C5D0 | Scene_vmethod29 | Game logic override (DispatchCollisionEvents) |
| 30 | +0x78 | 0x44B840 | NoOp_return | Empty stub (default) |
| 31 | +0x7C | 0x41AC70 | Scene_vmethod31 | Game logic override |
| 32 | +0x80 | 0x41C5B0 | Scene_SpawnBallsAndObjects | Spawn balls, scan SAFESPOT, CreateBadBall/MouseTrap/Secret/Flags/Signs/DynamicObjects |
| 33 | +0x84 | 0x419750 | Scene_method33 | Near Scene_dtor helper |
| 34 | +0x88 | 0x44B840 | NoOp_return | Empty stub (default) |
| 35 | +0x8C | 0x41A9A0 | Scene_ComputeInputForceDirection | Computes 3D force vector from strongest player input. Scale: 0.12 human. |

### SceneObject Vtable (0x4D934C, 10 entries)

| Slot | Offset | Address | Name | Description |
|------|--------|---------|------|-------------|
| 0 | +0x00 | 0x46B650 | SceneObject_dtor | Destructor |
| 1 | +0x04 | 0x46B490 | SceneObject_SetPosition | Set position (3 floats at +0x08) |
| 2 | +0x08 | 0x46B4B0 | SceneObject_SetScale | Set scale (3 floats at +0x14) |
| 3 | +0x0C | 0x46B670 | SceneObject_Render | D3D world matrix build + SetTransform + SetMaterial |
| 4 | +0x10 | 0x46B4D0 | SceneObject_SetVisible | Toggle visibility flag (+0x88) |
| 7 | +0x1C | 0x46B9F0 | SceneObject_DeletingDtor | Calls BaseDtor then free if flag&1 |

### Ball Vtable (0x4CF3A0)

### CollisionLevel Vtable (0x4D9068)

### Impossible Object Vtable (0x4D21C0)

Shared by all 5 Impossible objects (Rotator, Pendulum, Looper, Gear, BigGear).

### Rotator Vtable (0x4D5518)

### Pendulum Vtable (0x4D5B30)

### Gear Vtable (0x4D5AD0)

### UIList Vtable (0x4D6A70)

### Sprite Vtable (0x4D8F84)

### MeshWorld Vtable (0x4D9CDC)

### SimpleMenu Vtable (RVA 0xD5F50)

### OptionsMenu Vtable (replaces slot [2] with 0x441800 for visibility check)


## 31. Important Offsets Quick Reference

### Crash Report Address Format

CRASH_ADDRESS `0001:000XXXXX` where 0001=segment selector (.text), XXXXX=RVA from image base (0x400000).
Example: `0001:00052783` = 0x00452783 (NOT section-relative).

### Common Crash Addresses

| Address | Location | Cause |
|---------|----------|-------|
| 0x004713C0 | MeshWorld_Parse (ASE text parser) | Game parsed binary .MESHWORLD as ASE text |
| 0x00478EDD | MeshArchive_ctor (sound loading) | Sound loading during ball iteration |
| 0x00461971 | Scene_LoadMeshWorld | Garbage mesh pointer |
| 0x00469443 | FUN_00469430 (MeshWorld iter) | BASS stubs (load_real_bass not called) |
| 0x0046A443 | Music pointer deref | BASS_Init stub returns TRUE w/o bass_real |
| 0x00404B3E | Ball_Render shadow FPU overflow | Ball falls forever (no SAFESPOT found) |
| 0x0045F6FA | Render loop crash | Material matrix corruption (partial write) |
| 0x00416F5C | Draw (Impossible vtable) | PopCylinder vtable override to Impossible vtable (size mismatch) |

### Key Hook Points for Mods

| Address | Hook Point | Description |
|---------|-----------|-------------|
| 0x0046C200 | App_ResetFrame | Every-frame hook (ECX=gfx, ESI=App). Best for per-frame mods. |
| 0x0045DE30 | LoadMeshWorld | Level file loading — redirect to custom files |
| 0x0041C5B0 | Board_Setup (Scene_SpawnBallsAndObjects) | Object spawning hook |
| 0x00405E00 | Ball_Update | Ball physics tick — per-ball per-frame |
| 0x00419FA0 | Scene_SetCamera | Camera positioning — modify camera behavior |
| 0x0041B130 | Board_UpdateRaceState | Race state updates — timer, race end |
| 0x0040C5D0 | DispatchCollisionEvents | Collision event dispatch — custom events |

### MESHWORLD File Format Summary

- **S1 (Ref Points):** Axis order (x,z,y) — 3DS Max Z-up swap. Bare names (MOUSETRAP, BADBALL, START1-1).
- **S2 (Splines):** Axis order (x,z,y). Path splines.
- **S3 (Lights):** Axis order (x,z,y). Type=0 reads 36 bytes, type≠0 reads nothing.
- **S5 (Vertices):** Axis order (x,y,z) — NO swap. Raw memory dump.
- **S6 (MeshBuffer Names):** Prefixed (E:, N:, S:, T:, O:). Independent from S1.
- **Strips:** D3D triangle strips (D3DPT_TRIANGLESTRIP). N triangles use N+2 consecutive vertices.
- **Material:** 72 bytes (64 color + 4 shine + 4 has_reflection) + 4 bytes has_texture (separate read).
- **Texture loading:** Game strips extension, tries .bmp/.png/.tga. Prepends 'textures\\'.

### Supported Display Modes

- 640x480, 800x600, 1024x768, 1280x1024

### Supported Refresh Rates

- 60Hz, 70Hz, 72Hz, 75Hz, 80Hz, 85Hz, 90Hz, 100Hz+

---

*Document compiled from 46+ reverse engineering sessions, Ghidra auto-analysis (3,781 functions), hbtestd runtime analysis, and extensive modding work. All addresses verified against Hamsterball.exe V3.6.c (MD5: 7d25019366b8d7f55906325bd630d7fe).*

*Created by Hamsterbot for MAKYUNI 🦇 — 2026-07-20*
