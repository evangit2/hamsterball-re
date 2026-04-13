# Hamsterball - Function Map

|Binary: Hamsterball.exe (MD5: 7d25019366b8d7f55906325bd630d7fe)
|Total functions: 3,797 (Ghidra analysis)
||Documented: 1,758 (46.3%)
||User-labeled: 180+

## Entry Point and Lifecycle

| Address | Name | Description |
|---------|------|-------------|
| 0x004BB4C8 | entry | CRT entry point (GetVersionEx, heap init, etc.) |
| 0x004278E0 | WinMain | Game entry: Init → Run → Shutdown |
| 0x00429530 | App_Initialize_Full | 26-step init sequence |
| 0x0046BB40 | App_Initialize | 12-step base init (vtable calls + D3D8) |
| 0x0046BD80 | App_Run | Game loop (PeekMessage → Update → Render) |
| 0x0046BA10 | App_Shutdown | Cleanup on exit |

## Graphics Subsystem (Direct3D 8)

| Address | Name | Description |
|---------|------|-------------|
| 0x00455380 | Graphics_Initialize | D3D8 init, adapter check, device creation |
| 0x00455A60 | Graphics_Defaults | Set default render states |
| 0x00453B50 | Graphics_BeginFrame | Begin frame/render setup |
|| 0x00455A90 | Graphics_PresentOrEnd | Present frame or end scene |
|| 0x004542C0 | Graphics_ctor | Graphics constructor (vtable 0x4D88A0, init render context, texture cache, frustum) |
|| 0x00455360 | Graphics_dtor | Graphics destructor (cleanup + optional free) |
|| 0x00454550 | Graphics_Cleanup | Release D3D objects, free texture path, clear cache |
|| 0x00454000 | Graphics_SetTexturePath | Set custom texture prefix path (strdup at +0x7D8) |
|| 0x00454060 | D3DFMT_ToString | Convert D3DFORMAT enum to debug string |
|| 0x00454B50 | Graphics_SetViewport | Set viewport dimensions |
|| 0x00454D30 | Graphics_Reset | Reset device with new params, CreateDevice twice |
|| 0x00455D60 | Graphics_DrawScreenRect | Draw 2D screen rectangle (x,y,w,h → TLVERTEX triangle strip). 63 xrefs. |
|| 0x00455110 | Graphics_ApplyMaterialAndDraw | Apply material/render states + draw textured geometry. 17 xrefs. |
|| 0x00454190 | Graphics_SetRenderMode | Set shading mode, reset vertex shader, re-apply render states |
|| 0x00455B80 | Graphics_SetStreamBuffers | Set vertex buffer stream sources for rendering |
|| 0x00457FA0 | RenderContext_Init | Initialize render context struct (0x50 bytes, vtable 0x4D8E68) |
|| 0x00401160 | Graphics_SetViewportClip | Set viewport clipping bounds from 4x4 matrix |

## Audio Subsystem (BASS)

| Address | Name | Description |
|---------|------|-------------|
| 0x0046A020 | LoadMusicFile | BASS_MusicLoad wrapper |
| 0x0046A4D0 | LoadJukebox | Parse jukebox.xml |
| 0x00442CE0 | OptionsMenu_ctor | Options screen (Resolution, Fullscreen, Color, Safe Mode, Volume, Key Remap, Mouse, Pause) |
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

## Level/Game World System

| Address | Name | Description |
|---------|------|-------------|
| 0x0045DE30 | LoadMeshWorld | Load .meshworld level file |
| 0x004706E0 | MeshWorld_ctor | MeshWorld object constructor (0x488 bytes) |
| 0x00470930 | MeshWorld_Parse | Parse MESHWORLD text format (*MATERIAL, *MESH, etc.) |
| 0x00458970 | CreateMeshBuffer | Allocate mesh vertex/index buffer |
| 0x00458A80 | InitMaterialArray | Initialize material array |
| 0x0040D1C0 | GetLevelPath | Get path for level (levels/levelN) |
| 0x0040A120 | LoadRaceData | Parse racedata.xml |

## Game Object Factories

| Address | Name | Description |
|---------|------|-------------|
| 0x0040FA20 | CreateBumper | Create BUMPER1/2/3/4 objects |
| 0x00413CE0 | CreateBumper2 | Create bumper variant |
| 0x00410D00 | CreateLimit | Create E:LIMIT boundary |
| 0x004117B0 | CreateSpeedCylinder | Create SPEEDCYLINDER |
| 0x00412850 | CreateSpinner | Create N:SPINNER |
| 0x0040E250 | CreateSawblade | Create SAWBLADE |
| 0x0040BF50 | CreateMouseTrap | Create MOUSETRAP |
| 0x0040C5D0 | CreateNoDizzy | Create E:NODIZZY |
| 0x00438B30 | CreateBonkPopup | Create BONKPOPUP feedback |
| 0x0041D060 | BoardLevel3_ctor | BoardLevel3 constructor |
| 0x0040BAA0 | CreateSecretObjects | Create SECRET and SECRETUNLOCK objects |
| 0x0043DFB0 | Secret_ctor | Secret object constructor (0x10EC bytes) |
| 0x004121D0 | CreateLevelObjects | Multi-factory: BRIDGE, TIPPER, BONK, BBRIDGE1/2, POPCYLINDER, BLOCKDAWG1/2, CATAPULT, GLUEBIE |
| 0x004133E0 | CreatePlatformOrStands | Create PLATFORM and STANDS objects |
| 0x00417FE0 | CreateMechanicalObjects | Multi-factory: LOOPER, GEAR, BIGGEAR, ROTATOR, PENDULUM |
| 0x00437040 | Platform_ctor | Platform object constructor (0x10FC bytes) |
| 0x00462850 | Stands_ctor | Stands/Audience constructor (0x10D0 bytes) |
| 0x00435800 | Looper_ctor | Looper (loop-de-loop) constructor (0x1500 bytes) |
| 0x00437590 | Gear_ctor | Gear/BigGear constructor (0x1514 bytes) |
| 0x00435940 | Rotator_ctor | Rotator constructor (0x1508 bytes) |
| 0x00437700 | Pendulum_ctor | Pendulum constructor (0x1504 bytes) |
| 0x00437960 | Tipper_ctor | Tipper (tipping platform) constructor (0x1104 bytes) |
| 0x004661A0 | TipperVisual_ctor | Tipper visual component |
| 0x00465200 | TipperVisual_Attach | Attach visual to tipper |
| 0x00438850 | Bonk_ctor | Bonk (hammer) constructor (0x1200 bytes) |
| 0x00436D70 | BreakBridge_ctor | Breakable bridge constructor (0x1100 bytes) |
| 0x00436EE0 | PopCylinder_ctor | Pop cylinder (pop-up obstacle) constructor (0x10E8 bytes) |
| 0x0043C310 | Blockdawg_ctor | Blockdawg (block creature) constructor (0x1154 bytes) |
| 0x00437E10 | Catapult_ctor | Catapult constructor (0x1108 bytes) |
| 0x00437CB0 | Gluebie_ctor | Gluebie (glue blob) constructor (0x110C bytes) |

## Camera System

| Address | Name | Description |
|---------|------|-------------|
| 0x00413280 | CameraLookAt | Set camera look-at (CAMERALOOKAT) |

## Menu/UI Screens

| Address | Name | Description |
|---------|------|-------------|
| 0x0042DE50 | MainMenu_ctor | Main menu (LET'S PLAY, HIGH SCORES, OPTIONS, CREDITS, EXIT) |
| 0x00442CE0 | OptionsMenu_ctor | Options (Resolution, Fullscreen, Color, Volume, Remap, Mouse) |
| 0x004254E0 | CreditsScreen_ctor | Credits scrolling screen (formerly Physics_Init) |
| 0x0042B470 | HighScoreEntry_ctor | High score entry screen (name input + score display) |
| 0x0042BD40 | HighScoreEntry_Render | Render high score entry UI |
| 0x0042E060 | TournamentScreen_ctor | Tournament difficulty selector |
| 0x0044FD60 | SaveTourneyDialog_ctor | Save tournament dialog |
| 0x004476B0 | RegisterDialog_ctor | Register/purchase dialog |
| 0x0042E6F0 | QuitRaceMenu | Race quit confirmation menu |

## Ball Physics System

| Address | Name | Description |
|---------|------|-------------|
| 0x00403100 | Ball_SetTiltedGravity | Set gravity plane to tilted (value 1, normal -1,0,0) |
| 0x00403150 | Ball_SetFlatGravity | Set gravity plane to flat (value 2, normal 0,0,1) |
| 0x00403850 | Ball_SetTrajectory | Set ball trajectory direction + force scale |
| 0x00403750 | Ball_ApplyTrajectory | Apply trajectory force (normalize+scale direction, play sound, frame counter=100) |
| 0x00403980 | Ball_FindMeshCollision | Wrapper for Mesh_FindClosestCollision |
| 0x00401DD0 | Ball_CreateTrailParticles | Create trail particles (10 iterations, spawn 0x28 byte objects) |
| 0x00401920 | Ball_RenderShadow | Render ball shadow (scale by radius*constant, position at ball XYZ) |
| 0x00402BC0 | AthenaList_SetIndex | Set list iteration index |

## Collision System

| Address | Name | Description |
|---------|------|-------------|
| 0x00465EF0 | Collision_TraverseSpatialTree | Recursive octree traversal for collision tests |
| 0x00453780 | AthenaList_Append | Append item to dynamic AthenaList |

## Level Rendering

| Address | Name | Description |
|---------|------|-------------|
| 0x0040B090 | Level_InitScene | Init camera/scene (projection, fog, find CAMERALOCUS) |
| 0x0040B420 | Level_RenderDynamicObjects | Render moving objects (platforms) using Timer position |
| 0x0040B570 | Level_RenderObjects | Iterate objects calling vtable+0x0C (render callback) |
| 0x0040B600 | Level_UpdateAndRender | Full update pass: merge lists, pre-render, shadow, cleanup |
| 0x0040B9C0 | Level_SetObjectTransform | Set world transform from position data |
| 0x0040ACA0 | Level_SelectCameraProfile | Select camera profile by level difficulty (4-15) |

## Level/Collision Loading

| Address | Name | Description |
|---------|------|-------------|
| 0x004652E0 | CollisionLevel_ctor | Collision-only level (.meshcollision format) |
| 0x00465260 | Level_LoadCollision | Load binary collision mesh (planes, objects, AABB) |
| 0x004624C0 | Level_Cleanup | Level destructor (free objects, VBs, textures) |

## Arena Initialization

| Address | Name | Description |
|---------|------|-------------|
| 0x00413C20 | RumbleBoard_WarmUp_Init | Initialize Warm-Up arena (levels\arena-WarmUp) |
| 0x00416F40 | RumbleBoard_Neon_Init | Initialize Neon arena (levels\arena-neon) |

## Game Logic

| Address | Name | Description |
|---------|------|-------------|
| 0x00469CF0 | GameUpdate | Main update tick |
| 0x00428160 | PauseGame | Pausing (RightButtonPause) |
| 0x0042FAD0 | QuitRace | Quit current race |
| 0x0042E6F0 | QuitRaceMenu | Race quit menu |
| 0x004298C0 | TimerDisplay | Race timer display |
| 0x004254E0 | CreditsScreen_ctor | Credits scrolling screen (formerly mislabeled Physics_Init) |
| 0x0040ABA0 | CheckArenaUnlock | Check arena unlock conditions |
| 0x0040A420 | CheckPurchaseOrHighScore | Purchase reminder / high score check |

## Config/Save System

| Address | Name | Description |
|---------|------|-------------|
| 0x004279F0 | LoadOrSaveConfig | Config load/save dispatcher |
| 0x0042AE80 | LoadConfig | Load HS.CFG |
| 0x0042B6E0 | SaveConfig | Save HS.CFG |
| 0x00433AC0 | TournamentManager | Tournament save/load (TOURNAMENT.SAV) |

## Font/Text Rendering

| Address | Name | Description |
|---------|------|-------------|
| 0x00457130 | LoadFont | Load font.description + PNG glyphs |
| 0x00456E20 | Font_MeasureText | Measure text string width for centering |
| 0x00457440 | Font_DrawGlyph | Core glyph rendering (1 call = 1 glyph quad) |
| 0x004013A0 | UI_DrawTextCenteredAbsolute | Draw centered text (x - width/2) |
| 0x00409C60 | UI_DrawTextCentered | Draw centered text with shadow |
| 0x004012C0 | UI_DrawTextShadow | Draw text with shadow (offset + main) |

## DRM System

| Address | Name | Description |
|---------|------|-------------|
| 0x00429200 | ESellerate_Init | eSellerate DRM initialization |

## Math Utilities

| Address | Name | Description |
|---------|------|-------------|
| 0x00401AA0 | Vec3_NormalizeAndScale | Normalize 3D vector and scale to length param_1. 59 xrefs — most common math utility. |
| 0x00401D60 | Matrix_TransformVec3 | Transform 3D vector by 4x3 matrix. 15 xrefs. |
| 0x00402BF0 | Vec3_Copy | Copy 3 floats (12 bytes) with self-check. 18 xrefs. |
| 0x00453150 | Matrix_Scale4x4 | Set 4x4 matrix row scale values. |
| 0x00453200 | Matrix_Identity | Set matrix to identity (vtable pointer). |
| 0x004532E0 | AthenaList_SortedInsert | Insert with insertion-sort (ascending/descending). |
| 0x00458B50 | Matrix_ScaleTransform | Create 4x4 matrix by scaling source rows. |
| 0x0040A050 | Color_RandomRGBA | Generate 32-bit color from 4 random bytes. |
| 0x004580D0 | AABB_ContainsPoint | Test if point inside AABB (6 floats). |

## Misc/Utility

| Address | Name | Description |
|---------|------|-------------|
| 0x00429450 | FinishLoad | Finish loading (calls final setup) |
| 0x00457AD0 | Timer_Init | High-resolution timer init |
| 0x00457A40 | Timer_Cleanup | Timer cleanup |

## Key Global Data

| Address | Name | Description |
|---------|------|-------------|
| 0x004FD680 | g_App | Global App singleton object |
| 0x004D9CDC | MeshWorld_vtable | MeshWorld virtual function table |

## App Object Layout (from App_Initialize_Full)

Offset | Field | Description
-------|-------|------------
0x000 | vtable | Virtual function table pointer
0x004 | hInstance | Windows HINSTANCE
0x008 | cmdShow | nCmdShow parameter
0x054 | registryKey | Registry/config key
0x05C | targetFPS | Target frame rate
0x15C | width | Window width
0x158 | windowed | Windowed mode flag
0x160 | height | Window height
0x174 | graphics | Graphics object pointer
0x17C | audioSystem | Audio system pointer
0x180 | d3dDevice | D3D device pointer
0x200 | initialized | Init complete flag
0x208 | initStep | Current init step string
0x240 | cursor | Loaded cursor handle
0x278 | shadowTexture | Shadow texture
0x534 | musicHandle | BASS music handle
0x538 | musicChannel2 | Second music channel
0x53C | musicChannel1 | First music channel
0x550 | gameMode1 | Game mode object (1)
0x554 | gameMode2 | Game mode object (2)
0x558 | gameMode3 | Game mode object (4)
0x55C | gameMode4 | Game mode object (5)
0x914 | playCount | Play count from registry
0x159 | quitFlag | Game quit flag
0x15A | someFlag | Another control flag
0x156 | updateDisabled | Update disabled flag
0x5A | frameTimeMs | Frame delta time
0x5B | fpsDenominator | FPS denominator
0x5D | renderTarget | Render target ptr
0x65 | frameCounter | Per-second frame counter
0x84 | profilingSection | Current profiling section name
0x1B4 | versionString | Version string
0x1CC | loadedCount | Loaded objects counter


## Texture System

| Address | Name | Description |
|---------|------|-------------|
| 0x00471750 | LoadMesh | Load discrete .mesh files (uses same parser as MESHWORLD) |

## Graphics Pipeline (27-step init)

| Address | Name | Description |
|---------|------|-------------|
| 0x00455380 | Graphics_Initialize | Full 27-step D3D8 init |
| 0x00453C90 | Graphics_CreateDevice | D3D8 CreateDevice with format fallbacks (0x4b, 0x4f, 0x49, 0x47, 0x50, 0x4d) |
| 0x0042C810 | Graphics_InitRenderStates | Step 20: Initialize render states |
| 0x00454F10 | Graphics_SetViewport | Step 22: SetViewport(0,0,w,h) |
| 0x00454AB0 | Graphics_SetProjection | Step 23: SetProjection(10.0, 5000.0) |
| 0x00454630 | Graphics_SetupLights | Step 25: Setup D3D8 lights |

## Collision Event System

| Address | Name | Description |
|---------|------|-------------|
| 0x0040C5D0 | GameObject_HandleCollision | Main collision event dispatcher (E:NODIZZY, E:SAFESWITCH, E:LIMIT, E:BREAK, E:JUMP, E:ACTION, E:TRAJECTORY, N:NOCONTROL, N:WATER, N:TARPIT, N:GOAL, N:MOUSETRAP, N:SECRET, N:UNLOCKSECRET, DROPIN, PIPEBONK, POPOUT) |
| 0x0040E6A0 | Arena_HandleCollision | Arena-specific collision handler (E:CALLHAMMER, E:HAMMERCHASE, E:ALERTSAW1/2, E:ACTIVATESAW1/2, E:ALERTJUDGES, E:SCORE, E:JUMP, E:BELL + delegates to GameObject_HandleCollision) |
| 0x0040DCD0 | Level_HandleCollision | Level-specific collision handler (E:CATAPULTBOTTOM, E:OPENSESAME, N:TRAPDOOR, E:BITE, E:MACETRIGGER, N:MACE + delegates to GameObject_HandleCollision) |
| 0x00434770 | Saw_AlertActivate | Activate saw blade (alert mode - clear flag, play 3D sound) |
| 0x00434A50 | Saw_Activate | Activate saw blade (full - set active flag, play 3D sound) |
| 0x00434C40 | Judge_Reset | Reset judges (clear active flag, re-add to list) |
| 0x00434C80 | ScoreDisplay_SetTime | Set score display time with format string |
| 0x00434E20 | Bell_Activate | Activate bell (play sound, set animation) |
| 0x00438BB0 | Hammer_ChaseStart | Start hammer chase (set chasing flag, copy positions) |
| 0x00434290 | Catapult_Launch | Catapult launch (set active + timer) |
| 0x004344D0 | Trapdoor_Open | Open trapdoor (set scale from 0 to 1.0) |
| 0x00438410 | Trapdoor_Activate | Activate trapdoor (play 3D sound, set timer) |

## Sound System (BASS)

| Address | Name | Description |
|---------|------|-------------|
| 0x00459860 | Sound_Play3D | Play 3D positioned sound (set position via BASS, play channel) |
| 0x004597B0 | Sound_PlayChannel | Play sound channel (check if playing, dispatch from pool or allocate) |
| 0x004595B0 | Sound_StartSample | Start BASS sample (via vtable: reset, volume, 3D position) |
| 0x00466750 | Sound_CalculateDistanceAttenuation | 3D distance attenuation (find closest listener, apply min/max rolloff) |

## Race/Timer System

| Address | Name | Description |
|---------|------|-------------|
| 0x0044C260 | RaceResultPopup_ctor | Race end popup showing rank + TIME'S UP! / OUT OF TIME! text |
| 0x004298C0 | TimerDisplay | Race timer display |

## Audio Pipeline

| Address | Name | Description |
|---------|------|-------------|
| 0x00474540 | Audio_Init | BASS_Init (tries -1, then 0 device), BASS_Start, BASS_SetConfig |
| 0x0046A310 | Audio_PlayMusic | BASS_MusicPlayEx wrapper |
| 0x0046A020 | LoadMusicFile | BASS_MusicLoad wrapper (already labeled) |

## Supported Display Modes (from Graphics_Initialize)

- 640x480 (0x280 x 0x1E0)
- 800x600 (0x320 x 0x258)
- 1024x768 (0x400 x 0x300)
- 1280x1024 (0x500 x 0x400)

## Supported Refresh Rates

- 60Hz (0x14)
- 70Hz (0x15) 
- 72Hz (0x16)
- 75Hz (0x17)
- 80Hz (0x18)
- 85Hz (0x19)
- 90Hz (0x1a)
|- 100Hz+ (0x17 fallback)|

## Scene System (Core Lifecycle)

| Address | Name | Description |
|---------|------|-------------|
| 0x00419770 | Scene_dtor | Master scene destructor. Destroys all AthenaList items (dynamic +0x335, sub-objects +0x43B, static +0x22E), frees level clones (+0x22B/+0x22C), resources (+0x21F, +0xE92), clears 8+ AthenaLists, destroys matrix arrays, cleanup |
| 0x00419C00 | Scene_Update | Main scene tick. Demo timer (buy dialog), unpause check, ball position updates from +0xA75 list, camera tracking from App+0x5DC, screen offset animation (+0xA6E, -10/frame to 0 or -800), update+delete static objects in +0x22E, vmethods +0x4C/0x50/0x54/0x58 (render passes), iterate dynamic objects +0xD8B |
| 0x0041A2E0 | Scene_Render | Main render dispatch by player count. Mode 0 (1P): render3D/renderObjects/renderOverlay then HUD. Mode 1 (2P): setup camera from player list +0x3A38, same. Mode 2 (3-4P split): per-viewport Graphics_SetViewport, camera per player |
| 0x00419FA0 | Scene_SetCamera | Camera positioning. Ball pos from +0x758 + scene offset +0x434C. Boundary check +0x3F1C: distance clamp with sqrt+falloff. Object+0x744 noise randomizer. Timer +0x3F2C: snap to ball for N frames. FOV 0.9 from +0x29BC |
| 0x0040DFA0 | Scene_RenderWithCamera | Two-pass camera render: front face then 180-degree back face. Camera angle +0x43A0, Y offset +0x43A4, X offset +0x43B0, iterator vtable +0x4390 |
| 0x00418870 | Scene_CreateObject4f | Create object at (x,y,z,w) with BaseObject vtable 0x4CF584, set position via FUN_45D450 |
| 0x0040C0F0 | Scene_CreateFlags | Scan for FLAG/SMALLFLAG objects, create Flag (0x8C size), append to +0x2160, scale SMALLFLAG by constant |
| 0x0040C270 | Scene_CreateSigns | Scan for SIGN objects, create Sign (0x10FC size), SIGN-TARPIT gets tar texture, added to +0xCD4 and render lists |
| 0x0040C430 | Scene_CreateDynamicObjects | Generic object creation loop via vmethod +0x84, append to +0x335 (dynamic) and level lists, also +0x43B sub-objects |
| 0x00419B70 | Scene_ForEachBall_SetVelocity | Iterate ball list +0x29D4, call Ball_SetVelocity on each |

## SceneObject Class (vtable 0x4D934C, size 0xD4)

| Address | Name | Description |
|---------|------|-------------|
| 0x0046B4F0 | SceneObject_ctor | Constructor. Sets vtable, initializes 3 matrix transforms (base scale +0x94, rotation +0xA8, world +0xBC), pos=0, visible +0x88=1, zOrder +0x8C=-1, radius +0xCC=sqrt(const), type +0xD0=3 |
| 0x0046B650 | SceneObject_dtor | Destructor: calls Cleanup then optionally frees |
| 0x0046B3F0 | SceneObject_Cleanup | Reset vtable, unlink from scene list (+0x710-0x730), reset 3 identity matrices |
| 0x0046B4D0 | SceneObject_SetVisible | Set visibility flag +0x88, call vmethod +0xC |
| 0x00453BD0 | Scene_RegisterObject | Assign ID to obj+0x8C, call vmethod +0xC (init), store in scene array +0x710+id*4 |

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

## Level Setup Functions

| Address | Name | Level Path | Special Features |
|---------|------|------------|-----------------|
| 0x00416270 | Scene_SetupLevelDark | levels\leveldark | 2-player SceneObjects when !multiplayer, App+0x5DC/+0x67C |
| 0x0040E190 | Scene_SetupLevel5 | levels\level5 | Simple load (no extras) |
| 0x0040EA90 | Scene_SetupLevel6 | levels\level6 | LAUNCH01/02/03 + CHROMESHADOW positions, launcher timer +0x10DD=200 |
| 0x0040F360 | Scene_SetupLevel7 | levels\level7 | Simple load (no extras) |
| 0x00410830 | Scene_SetupLevel9 | levels\level9 | PILLAR list, MAGNIFYER on hard, CLOUDSCAPE, fog + projection setup |
| 0x00411F60 | Scene_SetupLevel10 | levels\level10 | 4 bumpers, FUN_436FC0 removal on easy, TarBubble list, multiplayer append |
| 0x004110D0 | Scene_SetupLevelCascade | levels\levelcascade | 8 bumpers (N:BUMPER%d 0-7) |
| 0x00411540 | Scene_SetupLevelUp | levels\levelup | Initial ball pos (0,50,0), VAC-IN/VAC-OUT vacuum tubes |

## Reflection Rendering Passes

| Address | Name | Object Count | Offset Base | Float Offset |
|---------|------|-------------|-------------|-------------|
| 0x00410670 | Scene_RenderReflectiveObjects | 8 | +0x438C | +0x644C |
| 0x00411380 | Scene_RenderReflectiveObjects7 | 8 | +0x436C | +0x642C |
| 0x00412DC0 | Scene_RenderReflectiveObjects4 | 4 | +0x439C | +0x53FC |

## Extended Utility Functions

| Address | Name | Description |
|---------|------|-------------|
| 0x00453180 | Vec3_Init | Initialize Vec3 with vtable 0x4CF300, 3 floats, default scale 1.0 |
| 0x0040A0B0 | Matrix_TransformVec3 | Transform 3D vector by 4x4 matrix: result = M * v |
| 0x00426E90 | StdString_Assign | MSVC std::string::assign (SSO 0xF threshold, word+byte copy) |
| 0x0040A040 | NoOp | Empty function (58 xrefs, default/no-op handler) |
| 0x00409AC0 | BaseObject_Init | Set vtable pointer to 0x4CF584 (54 xrefs) |
| 0x00409D60 | RepeatCall | Call function pointer N times (34 xrefs) |

## Ball Extended Functions

| Address | Name | Description |
|---------|------|-------------|
| 0x0040AF90 | Ball_GetTransform | Read ball transform (+0x6C/+0x70/+0xC0/+0xC4) into output struct |
| 0x0040AF00 | SceneObject_InitAtPosition | Initialize SceneObject at (x,y) with Vec3 vtable |

## UI and Input (Extended)

| Address | Name | Description |
|---------|------|-------------|
| 0x00409B90 | UI_DrawTextShadow_Wrapper | Wraps UI_DrawTextShadow with Vec3 default params |
| 0x00428F10 | Input_CheckKeyCombo | Check key combos. param_1=2: escape. 0-3: iterate 4 input bindings, 50-frame debounce |

## Game State Management (Extended)

| Address | Name | Description |
|---------|------|-------------|
| 0x004287C0 | App_StartRace | Restart audio, setup race, play sound, free dialogs, start music (vol 1.0/0.5) |
| 0x00428ED0 | Difficulty_GetTimeModifier | Time modifier by difficulty: 0=easy, 1=normal, 2=hard, default=0.0 |
| 0x00413BD0 | SinkPlatform_OnCollision | Match "DN:SINKPLATFORM" name, call sinking behavior, then base handler |

## Level Texture/Scale Assignment

| Address | Name | Description |
|---------|------|-------------|
| 0x004130E0 | Level_AssignTextures | Match object textures by ID against 20-slot table at App+0x2C8 |
| 0x00411BA0 | Level_AssignTexturesAndScales | Set scale based on max(x,y,z) vs threshold, then match textures |

## Scene Vtable Layout (0x4D0260, 36 entries)

| Slot | Offset | Address | Name | Description |
|------|--------|---------|------|-------------|
| 0 | +0x00 | 0x425020 | Scene_DeletingDtor | Destructor + free if flag&1 |
| 1 | +0x04 | 0x419C00 | Scene_Update | Main tick (9-step: input, bumpers, physics, update, level, objects, cameras, render, HUD) |
| 2 | +0x08 | 0x41A2E0 | Scene_Render | 1P/2P/split render dispatch |
| 3 | +0x0C | 0x4692F0 | Scene_HandleInput | Iterate menu items, check input, set current item at +0x864, play sound |
| 4 | +0x10 | 0x469220 | Scene_ActivateCurrentItem | Call vmethod+0x10 on current menu item (+0x864) |
| 5 | +0x14 | 0x4130A0 | Scene_vmethod5 | Unknown (arena-specific override) |
| 6 | +0x18 | 0x469280 | Scene_SelectCurrentItem | Call vmethod+0x0C on current menu item (+0x864) |
| 7 | +0x1C | 0x409D90 | Scene_NoOp | Empty (no-op stub, 3 bytes) |
| 8 | +0x20 | 0x40B400 | Level_RenderDynamicObjects | Level dynamic object rendering |
| 9 | +0x24 | 0x44B840 | NoOp_return | Empty stub (default) |
| 10 | +0x28 | 0x44B840 | NoOp_return | Empty stub (default) |
| 11 | +0x2C | 0x4692A0 | Scene_ClearCurrentItem | Set current item ptr (+0x864) to NULL |
| 12 | +0x30 | 0x4692A0 | Scene_ClearCurrentItem | Same as slot 11 |
| 13 | +0x34 | 0x44B840 | NoOp_return | Empty stub (default) |
| 14 | +0x38 | 0x409DA0 | Scene_DestroyScene | Calls FUN_4693C0 then SaveAndCleanup |
| 15 | +0x3C | 0x469430 | Scene_NotifyObjects | Iterate AthenaList, call FUN_4699D0 on each item |
| 16 | +0x40 | 0x419740 | Scene_SetDestroyed | Set +0x2C=1 destroyed flag |
| 17 | +0x44 | 0x4692B0 | Scene_SaveAndCleanup | Calls FUN_469AC0 (save + cleanup) |
| 18 | +0x48 | 0x40B090 | Level_InitScene | Level scene initialization |
| 19 | +0x4C | 0x41B130 | Scene_HandleRaceEnd | Race timer decrement, lap 3->4 transition, Game Over, RaceResultPopup |
| 20 | +0x50 | 0x41B540 | Scene_UpdateBallsAndState | Iterate ball lists, SetCamera, Ball_Update, destroy finished balls |
| 21 | +0x54 | 0x40A040 | NoOp | Empty (58 xrefs) |
| 22 | +0x58 | 0x41A540 | Scene_HandleCountdown | Countdown timer, check all balls finished, scene transition on expire |
| 23 | +0x5C | 0x409DE0 | Scene_HandleBallFinish | Ball finish state machine: start→countdown(150f)→finish→result popup→done |
| 24 | +0x60 | 0x40B420 | Level_RenderDynamicObjects_2 | Alternate dynamic render |
| 25 | +0x64 | 0x40B600 | Level_UpdateAndRender | Combined update + render |
| 26 | +0x68 | 0x40B570 | Level_RenderObjects | Level object rendering |
| 27 | +0x6C | 0x41B710 | Scene_RenderScoreHUD | Draw tournament title, countdown progress bar, "Score" text, Player 2, timer |
| 28 | +0x70 | 0x41BFD0 | Scene_RenderTimerHUD | Draw race timer, split screen divider, time display, overlay popups |
| 29 | +0x74 | 0x40C5D0 | Scene_vmethod29 | Game logic override |
| 30 | +0x78 | 0x44B840 | NoOp_return | Empty stub (default) |
| 31 | +0x7C | 0x41AC70 | Scene_vmethod31 | Game logic override |
| 32 | +0x80 | 0x41C5B0 | Scene_vmethod32 | Called by RumbleBoard virtual dispatch |
| 33 | +0x84 | 0x419750 | Scene_method33 | Near Scene_dtor helper |
| 34 | +0x88 | 0x44B840 | NoOp_return | Empty stub (default) |
| 35 | +0x8C | 0x41A9A0 | Scene_vmethod35 | Game logic override |

## SceneObject Vtable Layout (0x4D934C, 10 entries)

| Slot | Offset | Address | Name | Description |
|------|--------|---------|------|-------------|
| 0 | +0x00 | 0x46B650 | SceneObject_dtor | Destructor (sets vtable, flags cleanup) |
| 1 | +0x04 | 0x46B490 | SceneObject_SetPosition | Sets position (this+0x08..0x10) then calls vmethod+0x0C |
| 2 | +0x08 | 0x46B4B0 | SceneObject_SetScale | Sets scale (this+0x14..0x1C) then calls vmethod+0x0C |
| 3 | +0x0C | 0x46B670 | SceneObject_Render | Build world matrix from base+rotation+scale transforms, call D3D SetTransform+SetMaterial |
| 4 | +0x10 | 0x46B4D0 | SceneObject_SetVisible | Toggle visibility flag (+0x88) |
| 5 | +0x14 | - | (padding) | Non-code sentinel value |
| 6 | +0x18 | - | (padding) | Non-code sentinel value |
| 7 | +0x1C | 0x46B9F0 | SceneObject_DeletingDtor | Calls BaseDtor then free if flag&1 |
| 8 | +0x20 | 0x46B910 | SceneObject_BaseDtor | (function exists but not created) |
| 9 | +0x24 | 0x46B980 | SceneObject_vmethod9 | (function exists but not created) |

## Rumble/Arena Board Initialization Functions

| Address | Name | Level Path | Special Features |
|---------|------|------------|-----------------|
| 0x00413C20 | RumbleBoard_WarmUp_Init | levels\arena-WarmUp | Simple load + clone |
| 0x00413CE0 | RumbleBoard_Beginner_Init | levels\arena-beginner | 4 bumpers (N:BUMPER%d), Level_ctor + Clone |
| 0x00414180 | RumbleBoard_Intermediate_Init | levels\arena-intermediate | Simple load + clone |
| 0x00414240 | RumbleBoard_Dizzy_Init | levels\arena-dizzy | Extra Level3-Swirl loaded, no bumpers |
| 0x004144B0 | RumbleBoard_Tower_Init | levels\arena-tower | Simple load + clone |
| 0x00414960 | RumbleBoard_Up_Init | levels\arena-up | Simple load + clone |
| 0x00414B10 | RumbleBoard_Expert_Init | levels\arena-expert | Simple load + clone |
| 0x00414CE0 | RumbleBoard_Odd_Init | levels\arena-Odd | Simple load + clone |
| 0x00414F00 | RumbleBoard_Toob_Init | levels\arena-Toob | 5 bumpers (N:BUMPER%d) |
| 0x004153A0 | RumbleBoard_Wobbly_Init | levels\arena-Wobbly | Simple load + clone |
| 0x004158C0 | RumbleBoard_Sky_Init | levels\arena-Sky | PILLAR name scan via __strnicmp, append to +0x11FB |
| 0x00416080 | RumbleBoard_Master_Init | levels\arena-Master | Simple load + clone |
| 0x00416F40 | RumbleBoard_Neon_Init | levels\arena-neon | Scale matrix setup for dynamic objects, SceneObject at +0x11F9 |
| 0x00417DF0 | RumbleBoard_Glass_Init | levels\arena-glass | Simple load + clone |
| 0x00418540 | RumbleBoard_Impossible_Init | levels\arena-impossible | Simple load + clone |

**Common RumbleBoard pattern:** Level_ctor → Level_Clone → CameraLookAt → vmethod+0x80 (post-init). Simple arenas have just load+clone; complex ones add bumper objects, pillar scans, or SceneObject decorations.

## Board (Tournament) Constructors

| Address | Name | Tournament | Sub-Levels |
|---------|------|-----------|-----------|
| 0x00419030 | Board_ctor | (base) | Base tournament board constructor |
| 0x0041F4B0 | Board_ctor (Toob) | "Rodenthood" | Level8-Spinny, Level8-Saw, Level8-Fallout, Level8-Blockdawg1, Level8-Blockdawg2 |
| 0x0041D060 | BoardLevel3_ctor | (tournament) | Tournament level 3 board |

## Key SceneObject Methods

| Address | Name | Description |
|---------|------|-------------|
| 0x0046B4F0 | SceneObject_ctor | Constructor (0xD4 bytes, vtable 0x4D934C, init pos/scale/rot matrices) |
| 0x0046B650 | SceneObject_dtor | Destructor (sets vtable, flags cleanup) |
| 0x0046B3F0 | SceneObject_Cleanup | Cleanup helper (clears lists, resets state) |
| 0x0046B4D0 | SceneObject_SetVisible | Toggle visibility at +0x88 |
| 0x0046B490 | SceneObject_SetPosition | Set position (3 floats at +0x08) + vmethod+0x0C |
| 0x0046B4B0 | SceneObject_SetScale | Set scale (3 floats at +0x14) + vmethod+0x0C |
| 0x0046B670 | SceneObject_Render | D3D world matrix build + SetTransform + SetMaterial |
| 0x0046B860 | SceneObject_BaseDtor | Iterate child AthenaList, call each dtor(1), clear list |
| 0x0046B9F0 | SceneObject_DeletingDtor | Calls BaseDtor then free if scalar deleting |
