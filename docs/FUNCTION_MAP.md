# Hamsterball - Function Map
**Binary:** Hamsterball.exe (MD5: 7d25019366b8d7f55906325bd630d7fe)
**Total functions:** 3,781 (Ghidra analysis)
**Documented:** 2,553 (67.5%)
**User-labeled:** 932+
**Sessions:** 14-17 (50%→60%), 18 (60→64%), 19 (64→66%), 20 (66→71%), 21 (71→72.9%), 22 (72.9→74.4%), 23 (74.4→75.5%), 24 (96% of 3958), 25 (63.8%→67.5% of 3781)

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
| 0x00454190 | Graphics_SetRenderMode | Set shading mode, reset vertex shader, re-apply render states |
| 0x00455B80 | Graphics_SetStreamBuffers | Set vertex buffer stream sources for rendering |
| 0x00457FA0 | RenderContext_Init | Initialize render context struct (0x50 bytes, vtable 0x4D8E68) |
| 0x00401160 | Graphics_SetViewportClip | Set viewport clipping bounds from 4x4 matrix |

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
| 0x00453150 | Matrix_Scale4x4 | Set 4x4 matrix row scale values. |
| 0x00453200 | Matrix_Identity | Set matrix to identity (vtable pointer). |
| 0x004532E0 | AthenaList_SortedInsert | Insert with insertion-sort (ascending/descending). |
| 0x00458B50 | Matrix_ScaleTransform | Create 4x4 matrix by scaling source rows. |
| 0x0040A050 | Color_RandomRGBA | Generate 32-bit color from 4 random bytes. |
| 0x004580D0 | AABB_ContainsPoint | Test if point inside AABB (6 floats). |
| 0x00401010 | Vec3_Copy | Copy 4 floats (Vec3+padding) from source, preserves vtable |
| 0x00401040 | Vec3_Init | Init Vec3: set vtable 0x4CF300 + copy 4 floats from source |
| 0x00401070 | Vec3_dtor | Vec3 deleting destructor: reset to identity, optionally free |
| 0x004016c0 | Vec3_Scale | Multiply Vec3 by scalar: out = scalar * this |
| 0x00401890 | Vec3_DivideByScalar | In-place Vec3 division via reciprocal multiply with g_one (1.0f) |
| 0x004018c0 | Vec3_AddTwo | Add two Vec3s: out = this + rhs |
| 0x004018f0 | Vec3_AddInPlace | In-place Vec3 addition: this += rhs |
| 0x00401d20 | Vec3_Distance | Euclidean distance between two 3D points (6 float params) |
| 0x0040a020 | AthenaList_GetAt | Get element by index with bounds check; returns 0 if OOB |

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
| 32 | +0x80 | 0x41C5B0 | Scene_SpawnBallsAndObjects | vmethod[32]: spawn balls (GameObject_ctor), scan SAFESPOT, CreateBadBall/MouseTrap/Secret/Flags/Signs/DynamicObjects |
| 33 | +0x84 | 0x419750 | Scene_method33 | Near Scene_dtor helper |
| 34 | +0x88 | 0x44B840 | NoOp_return | Empty stub (default) |
| 35 | +0x8C | 0x41A9A0 | Scene_ComputeLighting | Find closest light source from array, compute direction/scale, camera-relative vector |

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

## Tournament and Menu System

| Address | Name | Description |
|---------|------|-------------|
| 0x00427080 | Tournament_AdvanceRace | Advance to next race in tournament; 15-case switch creates Board_ctor for each level (1-15), saves score, difficulty time bonus, saves race timestamps |
| 0x00428060 | App_ShowResults | Create results screen (FUN_426030 ctor), save to App+0x228, dispatch to scene manager |
| 0x0042EA30 | PracticeMenu_ctor | "Practice Menu" scene, "CHOOSE A PRACTICE RACE!", 14 race items with thumbnail textures (practice-level1.png through practice-impossible.png), lock check via App+0x851-0x865 flags |
| 0x004366E0 | Scene_SetRaceActive | Sets +0x10EC=1 (race active flag), 62 xrefs |

### Tournament Level Mapping (from Tournament_AdvanceRace switch)

| Case | Level | Board Constructor | Size |
|------|-------|-------------------|------|
| 1 | Warm-Up | FUN_41CA40 | 0x436C |
| 2 | Beginner | FUN_4200E0 | 0x644C |
| 3 | Intermediate | FUN_41CB20 | 0x438C |
| 4 | Dizzy | BoardLevel3_ctor | 0x4BE0 |
| 5 | Tower | FUN_41E340 | 0x5418 |
| 6 | Up | FUN_420390 | 0x4790 |
| 7 | Expert | FUN_424440 | 0x4394 |
| 8 | Odd | FUN_41EA40 | 0x4FD8 |
| 9 | Neon | FUN_41ED80 | 0x43B0 |
| 10 | Toob | FUN_41F4B0 | 0x646C |
| 11 | Wobbly | FUN_41F110 | 0x4388 |
| 12 | Glass | FUN_424A90 | 0x4390 |
| 13 | Sky | FUN_41F930 | 0x47F8 |
| 14 | Master | FUN_4206D0 | 0x6498 |
| 15 | Impossible | FUN_424C20 | 0x4380 |

### Level Unlock Flags (App+0x851-0x865)

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

## RNG / Random System

| Address | Name | Xrefs | Description |
|---------|------|-------|-------------|
| 0x0045dd60 | RNG_Rand | 193 | PRNG with 55-entry circular buffer; returns (buf[read]+buf[write])>>6 % range |
| 0x0045dd60 | RNG_Rand(signed) | - | When param_2=1 and RNG_Rand(2)==1, returns negative value |

## Graphics Transform Pipeline

| Address | Name | Xrefs | Description |
|---------|------|-------|-------------|
| 0x00457b10 | Matrix44_Zero | 12 | Clear 4x4 matrix; zero all 16 entries, set diagonals to 1.0 |
| 0x00457b50 | Gfx_SetPosition | 69 | Set world position via D3D SetTransform |
| 0x00457bb0 | Gfx_RotateY | 15 | Rotate around Y axis (negate + look-at construction) |
| 0x00457c60 | Gfx_ScaleX | 40 | Scale X axis with render-state multiplier |
| 0x00457c90 | Gfx_ScaleY | 35 | Scale Y axis with render-state multiplier |
| 0x00457cc0 | Gfx_ScaleZ | 26 | Scale Z axis with render-state multiplier |
| 0x00457fd0 | Matrix4_Identity | 40 | Set identity vtable + zero (Vec3 base init) |
| 0x00425fe0 | Gfx_SetAlphaBlendState | 9 | Set D3D alpha blend render states (0xD, 0xE → mode 3) |
| 0x00427940 | Gfx_SetCullMode | 13 | Set D3D cull mode (none/CW/CCW) based on +0x708 and specular flag |

## Wave / Math System

| Address | Name | Xrefs | Description |
|---------|------|-------|-------------|
| 0x00457da0 | Wave_Sin | 38 | sin(time * frequency * 2π/360) |
| 0x00457dc0 | Wave_Cos | 23 | cos(time * frequency * 2π/360) |

## UI List System (vtable 0x4D6A70)

| Address | Name | Xrefs | Description |
|---------|------|-------|-------------|
| 0x00448f20 | SimpleMenu_ctor | 15 | "Simple Menu" base ctor with item list, up/down scrollers |
| 0x004490a0 | UIListItem_ctor | - | Init 0x444-byte item with Vec3 + AthenaList |
| 0x004497f0 | UIList_AddItem | 86 | Add named item (text, subtext, colors, SceneObject, height) |
| 0x00449430 | UIList_AddSpacer | 29 | Add empty spacer item with height only |
| 0x004494d0 | UIList_ScrollUpdate | 17 | Scroll logic + mouse wheel + vtable dispatch |
| 0x00449b00 | UIList_Cleanup | 27 | Free all items (strings, SceneObjects, Vec3Lists) |
| 0x00449c20 | UIList_HandleKeyNav | 18 | Handle up/down/pgup/pgdn key navigation |
| 0x00449d40 | UIList_Render | 18 | Draw all items: gradient bars, text, icons, scroll arrows |
| 0x0044a570 | UIList_Layout | 18 | Compute text widths, position SceneObjects, set scrollers |
| 0x0044a8b0 | UIList_SetTextByName | 27 | Find item by subtext, replace display text |
| 0x00449750 | UIList_ActivateCurrentItem | 18 | Activate selected item: Back→650, Continue→50, else callback |
| 0x0044ad50 | RumbleScore_ctor | 12 | Init RumbleScore vtable + difficulty scale (0.02/0.03/0.04) |

## Rumble Board System

| Address | Name | Xrefs | Description |
|---------|------|-------|-------------|
| 0x004217b0 | RumbleBoard_ctor | 15 | Init board with "RumbleBoard" string, timer, base score=6000 |
| 0x00421880 | RumbleBoard_dtor | 24 | Cleanup timer, release SceneObjects, call Scene_dtor |
| 0x00421910 | RumbleBoard_Render | 16 | Draw timer bar, round info, difficulty status, "TIE BREAKER!" |
| 0x00421fe0 | RumbleBoard_Update | 16 | Check round end, resolve ties, play "Game Over" music |
| 0x00458e60 | RumbleBoard_InitTimer | 12 | Initialize round timer |
| 0x00458e80 | RumbleBoard_CleanupTimer | 32 | Cleanup round timer |
| 0x00458e90 | RumbleBoard_TickTimer | 12 | Tick round timer countdown |

## Scene / Rendering Pipeline

| Address | Name | Xrefs | Description |
|---------|------|-------|-------------|
| 0x0045e0e0 | Scene_RenderAllObjects | 33 | Main render: Graphics_BeginFrame → sort objects (opaque/alpha/shadow) → draw |
| 0x00460450 | Scene_RenderBallShadow | 38 | Render ball shadow: Ball_Render + depth bias pass |
| 0x0045df80 | SceneObject_CallUpdate | 47 | Dispatch +0x434 vtable[1] (Update) |
| 0x0045df90 | SceneObject_CallRender | 47 | Dispatch +0x434 vtable[2] (Render) |
| 0x00437130 | Scene_StartCountdown | 11 | Start race countdown (3..2..1..GO with param=400 or 50 frames) |

## Sprite System (vtable 0x4D8F84)

| Address | Name | Xrefs | Description |
|---------|------|-------|-------------|
| 0x0045d0c0 | Sprite_ctor | 30 | Init with texture, RenderContext, material defaults |
| 0x0045d660 | Sprite_RenderQuad | 15 | Render textured quad using material + draw primitive |

## Dialog System

| Address | Name | Xrefs | Description |
|---------|------|-------|-------------|
| 0x00440e70 | OkayDialog_ctor | 12 | "Okay Dialog" with caption text + "OKAY!" button |

## Core Utility Functions

| Address | Name | Xrefs | Description |
|---------|------|-------|-------------|
| 0x004531e0 | Vec3_Init | 13 | Set Vec3 vtable + zero position + w=255.0 |
| 0x00453250 | Vec3List_Free | 283 | Free Vec3List + member data at +0x103 |
| 0x004536a0 | AthenaList_GetSize | 60 | Return list count at +4 |
| 0x00453640 | AthenaList_FindByValue | - | Linear search for value, returns index or -1 |
| 0x00443990 | AthenaString_Clear | 14 | Free string buffer, reset to 15-char inline capacity |
| 0x0044b840 | Noop | 280 | Empty stub function (vtable placeholder) |
| 0x00401480 | GameObject_dtor | 9 | Release timers, free Vec3Lists, cleanup matrices |
| 0x00453610 | AthenaList_ContainsValue | 10 | Check if value exists in list (returns count>0) |
| 0x00453820 | AthenaList_MergeSorted | 10 | Merge sorted list param into this list |
| 0x004598c0 | Float_IsBetween | 10 | Test if param1 minus param2 is between two global bounds |
| 0x00469510 | AthenaString_Set | 10 | Copy string param into AthenaString (free old, strdup, set length) |
| 0x0046a140 | MusicPlayer_SetTempoScale | 10 | Set tempo scaling based on music BPM and delta time |
| 0x00477860 | FileStream_SeekRead | 10 | File stream seek and read data |
| 0x00460530 | Level_FindObjectByName | 9 | Find level game object by string name (stricmp), returns ptr or 0 |
| 0x00467d30 | Buffer_Free | 9 | Free buffer: free ptr+4, zero ptr+4/+8/+C |
| 0x00480032 | CRT startup helper | 5 | C runtime init |
| 0x00480c4d | CRT helper | 5 | C runtime helper |
| 0x0048a560 | Math/alloc helper | 5 | Runtime helper |

## Session 2026-04-13 Additions — Ball Physics & Rendering

| Address | Name | Xrefs | Description |
|---------|------|-------|-------------|
| 0x004015b0 | Ball_SetupCollisionRender | 4 | Initialize collision mesh render objects from level collision data |
| 0x004016f0 | Ball_ApplyForceV2 | 4 | Alternate force application (gravity plane aware, ice/dizzy/tube multipliers) |
| 0x00402c10 | Ball_RenderWithCollision | 10 | Ball render: check collision planes, render shadow, apply scaling, end frame |
| 0x00425340 | RumbleBoard_DeletingDtor | 4 | RumbleBoard destructor (calls RumbleBoard_dtor, then free) |
| 0x004280e0 | App_ShowMainMenu | 4 | Create MainMenu (0xCDC bytes) and store at App+0x224 |
| 0x004288b0 | App_StartTournamentRace | 4 | Start tournament race: config mirror/mode, create level scene, advance race |
| 0x0042c7c0 | Graphics_SetScaleAndPosition | 4 | Set identity scale matrix then set position (x,y) on Graphics object |
| 0x00443e30 | QuitDialog_ctor | 4 | Quit Dialog constructor — "Quit Dialog" with "YES"/"NO" buttons |
| 0x004468c0 | RumbleBoard_TickTimerWrapper | 4 | Wrapper: calls RumbleBoard_TickTimer at offset +0x110C |
| 0x00446b80 | RegisterDialog_ValidateSerial | 5 | Validate serial number using XOR cipher with key "54138", write DXCaps to registry |

## Session 2026-04-13 Additions — Register/Security Dialog

| Address | Name | Xrefs | Description |
|---------|------|-------|-------------|
| 0x004475a0 | RegisterDialog_HandleInput | 4 | Handle keyboard input for register dialog (char add, length check) |
| 0x00447920 | RegisterDialog_Render | 4 | Render "REGISTER HAMSTERBALL!" screen, name/serial fields, buy button |
| 0x00448890 | RegisterDialog_HandleKey | 4 | Handle key navigation (Tab name/serial, arrows, Enter=validate, Esc=close) |
| 0x00448a40 | UITimer_dtor | 4 | UI timer destructor (set vtable, cleanup) |
| 0x00449240 | UIList_AddIconItem | 4 | UI list add item with icon (0x444-byte UIListItem, text+subtext+RGBA+icon+height) |

## Session 2026-04-13 Additions — Sound System

| Address | Name | Xrefs | Description |
|---------|------|-------|-------------|
| 0x00459660 | Sound_LoadOggOrWav | 10 | Load sound file: try .ogg first, then .wav fallback |
| 0x00459310 | Sound_LoadOgg | - | Load OGG Vorbis file, create D3D sound buffer, add to channel list |
| 0x00458ee0 | Sound_Play3DAtPosition | - | Play 3D positioned sound (get channel, call vtable+0x3C with position) |
| 0x00459810 | Sound_GetNextChannel | 10 | Get next sound channel from circular buffer (with wrap-around) |

## Documentation Progress

| Session | Total Functions | Documented | % | New Labels |
|---------|----------------|------------|---|------------|
| Initial | 3,988 | 1,700 | 42.8% | — |
| Session 1 | 3,888 | 1,768 | 45.5% | 37 |
| Session 2026-04-13a | 3,811 | 1,870 | 49.1% | 25+ |
| Session 2026-04-13b | 3,811 | 1,892 | 49.6% | 27+ |
| Session 2026-04-13c | 3,811 | 1,908 | 50.1% | 39 |

## Session 2026-04-13b Additions — Core Engine & Scene Systems

| Address | Name | Xrefs | Description |
|---------|------|-------|-------------|
| 0x00498200 | BitStream_ReadBits | 95 | Bitstream reader - reads N bits from byte-aligned stream with bit-level positioning |
| 0x004737f0 | AthenaString_Assign | 52 | String copy/assign operator (copies src into dest buffer) |
| 0x004792fb | D3DX_DetectShaderProfile | 48 | Detect pixel shader version (1.x/2.x/3.x) based on D3DX runtime |
| 0x00492bda | Vertex_Transform | 32 | Vertex coordinate transformation between spaces (mode 1/2/3) |
| 0x00466c70 | AthenaString_Format | 98 | String format wrapper - calls Sprintf with object's internal buffer, returns ptr |
| 0x004bbdfd | AthenaString_Sprintf | - | In-memory sprintf using FILE struct trick for vsnprintf |
| 0x00469990 | Scene_AddObject | 77 | Add SceneObject to Scene - checks uniqueness, appends, sets back-ref, vtable notify |
| 0x0046f390 | Scene_BeginFrame | 39 | Begin scene frame - Graphics_BeginFrame + vtable[6] callback |
| 0x00460da0 | Scene_RenderFrame | 38 | Full frame render pipeline - iterates scenes/objects, assigns render indices |
| 0x00461370 | Scene_RenderOpaque | 38 | Opaque render pass - iterates objects calling vtable[0x28], renders mesh entries |
| 0x00461890 | Scene_LoadMeshWorld | 38 | Load mesh world from stream - creates MeshBuffers with materials/textures/flags |
| 0x00461f00 | Scene_Subdivide | 38 | Create 3D grid of mesh objects by dividing bounding box, test visibility per cell |
| 0x00462100 | Scene_SubdivideRandom | 38 | Random grid subdivision using seeded RNG, tests cell visibility |
| 0x004629e0 | Scene_LoadCached | 37 | Load .cached scene file - materials, meshes, transforms, objects, bounding box |
| 0x0046a750 | Window_Notify | 37 | Send WM_COPYDATA (0x4A) message to window with formatted string |
| 0x004605e0 | AthenaHashTable_Lookup | 36 | Hash table case-insensitive string lookup with bucket iteration |
| 0x004691c0 | SceneObject_dtor | 31 | SceneObject destructor - frees Vec3Lists, restores base vtable |
| 0x004693c0 | Scene_AddAllObjects | 25 | Batch-add all SceneObjects from internal list |
| 0x00469600 | MWParser_ReadTag | 24 | XML/SGML tag parser - finds <tag>...</tag> pairs from stream |
| 0x004695d0 | StreamReader_dtor | 24 | Stream/file reader destructor - close handle, free buffer |
| 0x004694f0 | Sprite_DrawColoredRect | (was 0x45d450) 23 | Draw colored rectangle with random RGBA vertex colors |
| 0x00472af0 | AthenaString_Init | 18 | Default string constructor - set vtable, zero fields, flags |
| 0x00472f30 | RegKey_Close | 18 | Close registry key handle via RegCloseKey |
| 0x00473670 | AthenaString_CopyCtor | 16 | String copy constructor from source string object |
| 0x004740d0 | AthenaString_WriteTag | 16 | Build XML tag string: <tag>content</tag> concatenation |
| 0x00489610 | Pool_FreeList | 16 | Memory pool free list traversal with reference counting |
| 0x00459b24 | Graphics_InitShaderDispatch | 19 | Shader init dispatch thunk (D3DX detect + indirect jump) |
| 0x0045a439 | Graphics_SetRenderState | 29 | Render state dispatch thunk (profile detect + indirect jump) |
| 0x004ab9b8 | DivCeil | 15 | Ceiling division utility: (a-1+b)/b |

## Key Data Items

| Address | Name | Description |
|---------|------|-------------|
| 0x004D2334 | s_BACK | String "BACK" |
| 0x005341CC | g_renderIndex | Global render index counter |
| 0x004F7360 | PTR_OBJ_VTABLE | Pointer to object vtable (used by Ball, GameObject, Scene, etc.) |

## Session 2026-04-13c Additions — Scene, Path, Game, Registry

| Address | Name | Xrefs | Description |
|---------|------|-------|-------------|
| 0x00467bf0 | Path_GetPosition | 14 | Interpolate X/Z from path splines at time t, Y=0 |
| 0x004690f0 | Gadget_ctor | 11 | Generic Gadget constructor (vtable 0x4d9170, SceneObject-derived) |
| 0x00469240 | Gadget_Activate | 9 | Activate gadget callback (vtable[2] with RNG params) |
| 0x00457de0 | Math_Atan2Angle | 13 | Atan2 angle with quadrant adjust, degrees-per-unit scaling |
| 0x004561e0 | Material_Init | 11 | Zero-initialize 0x64-byte material/transform array |
| 0x004562b0 | Material_Copy | 8 | Copy 0x64 bytes (4x4 matrix + extras) |
| 0x00458020 | AABB_Update | 12 | Expand axis-aligned bounding box by 3D point |
| 0x00458130 | Math_FastDistance2D | 8 | Approximate integer distance (Bresenham weights 102/246) |
| 0x0045d300 | Sprite_DrawRect | 12 | Draw rect with position+size, random RGBA, 2-triangle strip |
| 0x0046b2a0 | MeshBuffer_dtor | 11 | Destroy mesh buffer list items + free Vec3List |
| 0x00472fd0 | RegKey_WriteDword | 13 | Write DWORD via RegSetValueExA |
| 0x00473080 | RegKey_ReadDword | 13 | Read DWORD via RegQueryValueExA |
| 0x00471c20 | MeshNode_ctor | 9 | Load mesh file into scene graph node (vtable 0x4d9c48) |
| 0x00472c70 | Math_Lerp | 9 | Linear interpolation: a + (b-a)*t |
| 0x0042c870 | Font_DrawCentered | 8 | Draw text centered at (x,y) position |
| 0x004351f0 | GameLevel_ctor | 8 | Game level constructor (Stands_ctor, Level_Clone, sound channel) |
| 0x00402400 | Ball_RecordBest | 7 | Reset +0x2EC, update max at +0x2F4 if param exceeds current |
| 0x00425f90 | App_CompleteRace | 7 | Complete race - increment counter, trigger state transitions, clear flag |
| 0x00426b30 | String_AllocBuffer | 7 | Allocate string buffer with size | 0xF rounding |
| 0x0042b190 | ConfirmMenu_ctor | 6 | Confirmation menu (BACK/BACK2TOURNAMENT, DONE), vtable 0x4d39d0 |
| 0x00473500 | AthenaString_AssignCStr | 75 | AthenaString assign from C string (free old, alloc new, copy) |
| 0x004736b0 | AthenaString_dtor | 85 | AthenaString destructor - frees buffer, sets vtable to base dtor |
| 0x00473a50 | AthenaString_AssignCRLF | 21 | AthenaString assign CRLF ("\\r\\n") |
| 0x004bae43 | AthenaString_SprintfToBuffer | 71 | sprintf into char buffer via fake FILE struct |
| 0x00473050 | RegKey_WriteBool | 30 | Write boolean to registry via RegSetValueExA (REG_BINARY) |
| 0x00473130 | RegKey_ReadBool | 28 | Read boolean from registry via RegQueryValueExA |
| 0x00473170 | RegKey_ReadString | 23 | Read string from registry with fallback attempts |
| 0x00470150 | SceneObject_RenderFull | 40 | Full render with ball+material+strips, alpha-aware material application |
| 0x00470440 | SceneObject_RenderSingleObj | 39 | Render a single SceneObject with material/strip dispatch |
| 0x0046fbb0 | SceneObject_ComputeCollisionSphere | 38 | Compute bounding sphere from AABB, call Ball_CheckCollisionPlanes |
| 0x0045dfd0 | SceneObject_CheckCollision | 38 | Thunk: compute bounding sphere + check collision planes |
| 0x0049336b | D3DDevice_SetFPUControl | 29 | Set FPU control word from device state |
| 0x0049338e | Mesh_InitTexture | 32 | Initialize texture object from surface desc (D3D texture init) |
| 0x00493671 | Mesh_DrawWithTransform | 31 | Draw mesh with temporary transform override |
| 0x0049373d | Mesh_ClearColorVertices | 30 | Zero out vertices matching the clear color (transparency hack) |
| 0x0047dfb9 | Graphics_DrawIndexedPrimitive | 25 | D3D DrawIndexedPrimitive wrapper via vtable |
| 0x004a458c | longjmp_with_cleanup | 34 | CRT longjmp with optional cleanup callback |
| 0x004a45aa | seh_filter_invoke | 21 | Invoke SEH exception filter callback at offset 0x44 |
| 0x004c02e7 | LeaveCriticalSection_indexed | 23 | LeaveCriticalSection by index into global array |
| 0x0046f3b0 | Scene_BeginFrameThenRender | 39 | Begin graphics frame then invoke render callback |
| 0x0046f3d0 | MeshWorld_ctor | 39 | MeshWorld constructor from filename and strip count |
| 0x00472770 | SceneObject_BuildStrips | 39 | Builds triangle strips for SceneObject rendering |
| 0x004ba754 | __ftol2 | 358 | CRT float-to-int64 conversion (compiler intrinsic) |
| 0x004bc7c8 | __errno | 40 | CRT __errno - returns thread-local errno pointer |
| 0x004bc7d1 | __doserrno | 23 | CRT __doserrno - returns thread-local DOS errno pointer |
| 0x004bcda8 | __security_init_cookie | 34 | CRT security cookie initialization |
| 0x004bac20 | strstr | 22 | CRT strstr - string search with SIMD optimization |
| 0x004bc0d1 | strtok | 50 | CRT strtok - thread-safe string tokenizer |
| 0x00461740 | Level_ctor | 12 | Level constructor: inits base, vtable 0x4D8FB0, 4 AthenaLists, Timer + LevelState 0x10D4 bytes |
| 0x004694f0 | SceneObject_sub1_ctor | 12 | Simple SceneObject sub-ctor: set vtable 0x4D91BC, null +0x08/+0x10, visible +0x14=true |
| 0x0049721e | MeshNode_DeletingDtor | 30 | MeshNode deleting destructor: calls base dtor, then frees if param_1 & 1 |
| 0x00472c20 | AthenaHashTable_ctor | 15 | Athena hash table constructor: set vtable, call internal init, then set final vtable 0x4CF584 |
| 0x00472ec0 | RegKey_Open | 15 | Open registry key: HKLM first, fall back HKCU, create if needed |
| 0x0047df9a | Graphics_DrawIndexedPrimitiveUP | 14 | D3D DrawIndexedPrimitiveUP wrapper via vtable dispatch at offset 0x2C |
| 0x00489710 | AthenaList_IterateNext | 15 | Advance linked list iterator to next node |
| 0x00489bd0 | AthenaList_Sort_14 | 15 | Sort list elements (comparator index 0xE / 14) |
| 0x0048a1a0 | AthenaList_FreeAll | 10 | Free all list nodes: Pool_FreeList on head and items |
| 0x0048cd87 | D3D8_DebugSetMute | 14 | D3D8 DebugSetMute: load d3d8.dll/d3d8d.dll, call DebugSetMute |
| 0x004c8ff7 | _check_file_access | 14 | Check if file path is accessible (directory vs file), uses GetFileAttributesA |
| 0x004ab4a0 | __libm_sse2_tan | 14 | SSE2 tan() implementation with range reduction and polynomial approx |
| 0x004ad5ac | Zlib_FreeIf | 12 | Conditional free: only frees if both params non-null |
| 0x004ad63c | Zlib_UpdateHash | 14 | Zlib deflate hash update (calls update + insert) |
| 0x004adf0c | Zlib_FlushWithCRC | 14 | Zlib flush with CRC validation, chunked writes |
| 0x004b5ae1 | CRT_Noop2 | 14 | Empty function (no-op) |
| 0x004ba61e | CRT_ThrowStringTooLong | 10 | Throw C++ exception "string too long" |
| 0x004bacc0 | strchr | 11 | CRT strchr - find char in string with SIMD optimization |
| 0x004bc350 | strcpy | 12 | CRT strcpy - string copy with SIMD optimization |
| 0x004a167b | Matrix4x4_Multiply_SSE2 | 12 | 4x4 matrix multiply using SSE2 packed float ops |
| 0x00402a20 | Ball_SetVec3AtOffset | 12 | Set 3 floats at offset 0xca4 in ball (camera/force vector) |
| 0x00402a50 | GameObject_sub2_dtor | 12 | GameObject deleting destructor variant 2 |
| 0x00402a70 | Ball_DrawRumbleScoreText | 25 | Draw rumble score text at viewport using Graphics_SetViewport + UI_DrawTextCenteredAbsolute |
| 0x00405100 | Ball_InitPhysicsDefaults | 14 | Set ball physics defaults: radius=0.5, friction=0.2, max_speed=35.0, gravity=6.0 |
| 0x00405d90 | GameObject_sub_ctor | 14 | GameObject subclass constructor: vtable 0x4CF494, scale=1.0, visibility flag, +0x80c=0x32 |
| 0x00405dd0 | GameObject_sub_dtor | 12 | GameObject deleting destructor variant 1 |
| 0x00405e00 | Ball_Update | 400+ | Main ball physics tick: timer, collision, velocity integration, reflection, sound, camera tilt, spin. Core function! |
| 0x00408390 | Ball_AI_ChaseNearest | 60 | AI steering: finds nearest opponent ball, applies force toward it, sine wave wandering fallback |
| 0x00408830 | Ball_FallUpdate | 40 | Ball update when fallen: shrinks ball, handles scale change, trail cleanup |
| 0x00408d10 | Ball_Split_ctor | 14 | Split ball constructor: vtable 0x4CF560, +0xc60=5, calls scene function |
| 0x00408d70 | Ball_SplitIntoThree | 50 | Creates 3 split balls with different trajectories, copies collision direction, sets ball IDs 1/2/4 |
| 0x00409480 | Ball_SplitAndExplode | 70 | Creates 2 split balls + circular RumbleScore explosion pattern (0-360 degrees) |
| 0x0040a920 | Scene_CreateGameOverMenu | 25 | Creates game-over UI based on game state: single quit, multiplayer quit, or menu mode |
| 0x0040ae50 | Sprite_DrawCentered | 12 | Sprite draw centered at position: Sprite_DrawRect with center offset |
| 0x0040b960 | SceneObj_SetBounds | 14 | SceneObject bounds setter: sets +0x14-20, identity matrix if scale != 1.0 |
| 0x0040d280 | Scene_LoadLevel2 | 15 | Load level 2: "levels\\level2", clone, init scene |
| 0x0040d390 | Scene_LoadLevel3 | 25 | Load level 3: "levels\\level3", clone, init scene, collect TarBubble objects |
| 0x0040d6d0 | Scene_LoadLevel4 | 20 | Load level 4: "levels\\level4", clone, init scene, special setup (flag +0x80=1, camera bounds) |
| 0x00401590 | SceneObj_CallVtable18 | 8 | SceneObject vtable call at offset 0x18 |
| 0x00401a60 | Vec3_Length | 8 | Vec3 length: sqrt(x*x + y*y + z*z), min 1.0 |

## New Functions (Session 2026-04-13)

| Address | Name | Xrefs | Description |
|---------|------|-------|-------------|
| 0x4531b0 | Vec3_SetScalar | 6 | Set all 3 components to same value, w=1.0 |
| 0x4536b0 | AthenaList_InsertAt | 7 | Insert value at index, reallocate array |
| 0x453e90 | Gfx_TransformY | 7 | Screen Y transform: y * viewMatrix + viewport |
| 0x453eb0 | Gfx_TransformZ | 7 | Screen Z transform: z * viewMatrix + viewport |
| 0x453970 | Graphics_SetCullMode2 | 5 | Set D3D cull mode via vtable call |
| 0x44aca0 | SimpleList_vtbl_Init | 7 | SimpleList vtable pointer init |
| 0x44b850 | UIListItem_vtbl_Init | 6 | UIListItem vtable pointer init |
| 0x426d20 | StdString_Erase | 6 | String erase: delete substring at position |
| 0x426de0 | StdString_Substr | 5 | String substring extraction |
| 0x426f30 | PlayerProfile_ctor | 5 | Player profile constructor (0x280 bytes) |
| 0x443a10 | StdString3_Clear | 5 | Clear string, free heap buffer |
| 0x443f50 | GameObject2_dtor | 5 | GameObject destructor variant |
| 0x44fda0 | TourneyMenu_ctor | 6 | Tournament menu constructor |
| 0x458220 | AABB_TriangleIntersect | 6 | AABB vs triangle edge intersection test |
| 0x458320 | AABB_TriangleTest6Edges | 6 | Full 6-edge AABB-triangle intersection |
| 0x457f10 | Collision_PointInTriangle | 5 | Barycentric point-in-triangle test |

## New Functions (Session 2026-04-13 Part 2 - Vec3/Ball/Scene)

| Address | Name | Description |
|---------|------|-------------|
| 0x00401090 | Scene_SetSoundMode | Set scene audio mode: dispatches vtable[50](0x16, mode), stores at +0x708 |
| 0x00401100 | Gfx_PackColorRGB | Pack 3 float RGB channels to 24-bit color via __ftol2 + shift/OR |
| 0x004011c0 | SceneObj_SetScale | Set uniform scale factor on SceneObject, create scale matrix |
| 0x00401220 | Scene_ZoomIn | Increase scene viewport zoom by g_zoomStep, call SetProjection |
| 0x00401270 | Scene_ZoomOut | Decrease scene viewport zoom by g_zoomStep, call SetProjection |
| 0x00401640 | Timer_dtor | Timer deleting destructor: calls Timer_Cleanup, optionally frees |
| 0x00401660 | Ball_SetName | Set ball display name at +0xC28, copies string, sets type ID=200 |
| 0x00402030 | Ball_SetTargetPos | Set/interpolate target position with smooth damping threshold |
| 0x00402150 | Ball_CheckProximity | Check position proximity radius, store integer distance result |
| 0x00402200 | Ball_StartFall | Mark ball fallen: set flag+0xC4C=1, shrink radius to 13.0, play 3D sound |
| 0x00402270 | Ball_EndFall | Reset from fallen: clear flag, set radius=26.0, physics=5.0 |
| 0x00402290 | GameObject_Render | Render game obj: scale, depth layer toggle, Sprite_RenderQuad, copy result |
| 0x004027f0 | Ball_dtor | Ball deleting destructor: calls Ball_Cleanup, optionally frees |
| 0x004029c0 | Ball_SetSpeed | Set ball speed, computes velocity = speed * direction unit vec |
| 0x457b80 | Gfx_SetPositionAndRender | 5 | Set transform + render state + draw |
| 0x458f10 | Vtable_CallOffset48 | 7 | Generic vtable call at offset 0x48 |
| 0x473940 | FontFormatString_Parse | 9 | Parse %-formatted string for font rendering |
| 0x4749b0 | StdString_CompareSubstr | 8 | Compare substring with byte-by-byte comparison |
| 0x47c830 | StreamReaderVtbl_Init | 8 | StreamReader vtable init |
| 0x489660 | D3DX_SurfaceClipBlit | 9 | D3DX surface blit with clipping |
| 0x489da0 | Pool_Reset | 8 | Pool object reset/cleanup |
| 0x48a820 | D3DX_Uninit | 8 | D3DX library shutdown |
| 0x42e220 | DifficultyMenu_ctor | 5 | Difficulty selection menu (Pipsqueak/Normal/Frenzied) |
| 0x42e840 | OptionsMenu_RenderControls | 5 | Render control icons in options |
| 0x42f550 | OptionsMenu_dtor | 5 | Options menu destructor |
| 0x4363f0 | Platform_ctor | 5 | Platform object constructor |
| 0x465260 | Level_LoadCollision | 6 | Load binary .meshcollision file |
| 0x465d90 | Mesh_FindClosestCollision | 10 | Ray-mesh collision via spatial tree |
| 0x465ef0 | Collision_TraverseSpatialTree | 3 | Octree traversal for collision faces |
| 0x492bc4 | D3DX_AssemblyOp8 | 8 | D3DX shader assembly operation |
| 0x498200 | BitStream_ReadBits | 95 | Read N bits from bitstream |

## New Functions (Session 2026-04-13 Continued)

| Address | Name | Xrefs | Description |
|---------|------|-------|-------------|
| 0x4531b0 | Vec3_SetScalar | 6 | Set all 3 components to same value, w=1.0 |
| 0x4536b0 | AthenaList_InsertAt | 7 | Insert value at index, reallocate array |
| 0x453e90 | Gfx_TransformY | 7 | Screen Y transform: y * viewMatrix + viewport |
| 0x453eb0 | Gfx_TransformZ | 7 | Screen Z transform: z * viewMatrix + viewport |
| 0x453970 | Graphics_SetCullMode2 | 5 | Set D3D cull mode via vtable call |
| 0x44aca0 | SimpleList_vtbl_Init | 7 | SimpleList vtable pointer init |
| 0x44b850 | UIListItem_vtbl_Init | 6 | UIListItem vtable pointer init |
| 0x426d20 | StdString_Erase | 6 | String erase: delete substring at position |
| 0x426de0 | StdString_Substr | 5 | String substring extraction |
| 0x426f30 | PlayerProfile_ctor | 5 | Player profile constructor (0x280 bytes) |
| 0x443a10 | StdString3_Clear | 5 | Clear string, free heap buffer |
| 0x443f50 | GameObject2_dtor | 5 | GameObject destructor variant |
| 0x44fda0 | TourneyMenu_ctor | 6 | Tournament menu constructor |
| 0x458220 | AABB_TriangleIntersect | 6 | AABB vs triangle edge intersection test |
| 0x458320 | AABB_TriangleTest6Edges | 6 | Full 6-edge AABB-triangle intersection |
| 0x457f10 | Collision_PointInTriangle | 5 | Barycentric point-in-triangle test |
| 0x463790 | Vec3_CrossProduct | 7 | Vector cross product: this x param2 -> param1 |
| 0x46b1a0 | AthenaListObj_ctor | 7 | AthenaList object constructor |
| 0x4738b0 | StdString_AppendN | 7 | Append N chars from string with strncat |
| 0x47c840 | Sprite_CalcTexCoords | 8 | Calculate sprite UV coordinates |
| 0x4602f0 | Scene_CollectByNameFilter | 6 | Collect scene objects by name filter |
| 0x46ec30 | Ball_GetInputForce | 7 | Get ball input force (keyboard/mouse/joystick) |
| 0x46f670 | Mesh_SaveAndFree | 5 | Save mesh to file, free buffers |
| 0x473b10 | AthenaString_AssignFormatted | 6 | Format and assign AthenaString |
| 0x46a440 | Audio_PlayMusicAtSpeed | 6 | Play music with tempo/speed adjustment |
| 0x466ac0 | Scene_UpdateChildren | 5 | Traverse scene tree, update children |
| 0x467750 | Array_CopyDWords | 5 | Copy N dwords from src to dst |
| 0x429520 | Game_SetInProgress | 5 | Mark game in progress (sets +0x200=1) |
| 0x440dd0 | Graphics_DrawRectAndReset | 5 | Draw rect, then reset matrix to identity |
| 0x44be80 | ScoreObject_ctor | 5 | Score display object constructor |
| 0x44fd40 | SimpleList_dtor | 5 | SimpleList destructor |
| 0x459610 | Scene_RenderIfVisible | 5 | Render scene object if visible flag set |
| 0x470650 | Array_FillDWords | 5 | Fill N dwords with same value |
| 0x472da0 | Transform_ctor | 5 | Transform object constructor |
| 0x418760 | Scene_CreateObject_Gear | 6 | Create GEAR object in scene |
| 0x418930 | Gear_AdvanceAlongPath | 6 | Gear path following (8-dir gradient descent) |
| 0x4730c0 | Registry_ReadFloat | 5 | Read float from Windows registry |
| 0x473740 | StdString_AppendCharN | 5 | Append N copies of a char |
| 0x46e0b0 | Input_IsKeyDown | 5 | Check input state (keyboard/mouse/joystick) |
| 0x456150 | Ray_SetDirection | 3 | Set ray direction, normalize, compute length |
| 0x456390 | Mesh_Clear | 3 | Clear mesh: free vertex/index buffers |
| 0x456e80 | Font_WordWrap | 3 | Word-wrap text to fit width |
| 0x458000 | Collision_InitDefaultAABB | 3 | Initialize default collision AABB bounds |
| 0x4583f0 | AABB_TriangleIntersect2 | 4 | Double AABB-triangle test wrapper |
| 0x4581d0 | Vec2_Distance | 3 | 2D distance: sqrt(dx²+dy²) |
| 0x458190 | Collision_GradientEval_Stub | 4 | Empty stub for collision gradient evaluation |
| 0x46f340 | MeshBuffer_Cleanup | 3 | Cleanup mesh buffer, free resources |
| 0x46fd60 | Mesh_AddVertex | 4 | Add vertex to mesh buffer (8 floats) |
| 0x46c970 | Texture_SetDimensions | 3 | Set texture dimensions and UV scale |

## Menu System (Session 2026-04-13 Batch 4)

| Address | Name | Xrefs | Description |
|---------|------|-------|-------------|
| 0x42f810 | TimeTrialMenu_ctor | 3 | "Time Trial Menu" → extends PracticeMenu with race items + lock checks |
| 0x42fc10 | PartyMenu_ctor | 3 | "Time Trial Menu" ( Party Race! ) → extends PracticeMenu, vtable 0x4D4738 |
| 0x42fc40 | ArenaMenu_ctor | 3 | Arena menu with 14 arena items (Warm-Up to Impossible), lock icons, vtable 0x4D47B8 |
| 0x4326d0 | MPMenu_ctor | 3 | Multiplayer menu: Party Race, Rodent Rumble, controller config (1-4P) |

## Scene/Texture/Rendering (Session 2026-04-13 Batch 4)

| Address | Name | Xrefs | Description |
|---------|------|-------|-------------|
| 0x44aa40 | Scene_FindTextureByName | 4 | Find texture by case-insensitive name, return ptr+dimensions+width |
| 0x44ab00 | Scene_FindTextureDimensions | 4 | Find texture and measure text width via Font_MeasureText |
| 0x44abf0 | Scene_AddTextureToList | 4 | Add texture reference to scene list by name, mark dirty at +0xCBC |
| 0x443ac0 | SceneObject_RenderScaled | 3 | Render object scaled (ScaleX, SetPosition, vtable calls + Timer) |
| 0x4410c0 | SceneObject_FreeStrings | 3 | Free 2 string pointers, re-init BaseObject, call SceneObject_dtor |
| 0x453c50 | Texture_RemoveRef | 3 | Decrement texture refcount, remove from cache and free when 0 |
| 0x457a50 | Graphics_DisableRenderState | 3 | Thunk → Graphics_SetRenderState (disable mode) |

## Game Flow (Session 2026-04-13 Batch 4)

| Address | Name | Xrefs | Description |
|---------|------|-------|-------------|
| 0x428c50 | App_StartPracticeRace | 3 | Start practice/tournament race: calls App_StartRace, sets up PlayerProfile, calls Tournament_AdvanceRace |
| 0x434580 | Sound_InitChannels | 3 | Allocate sound channels for object, get next sample, play 3D positioned sound, set timer 0x140 |
| 0x43b6f0 | ScoreObject_SetScore | 3 | Find score by ID, set to 10, or create new score entry with value 10 |
| 0x44bef0 | Timer_Decrement | 4 | Timer tick: value = end_value - 100, set flag at +0x2A |
| 0x448620 | ScoreDisplay_DeletingDtor | 3 | ScoreDisplay scalar deleting destructor |
| 0x4470d0 | ScoreDisplay_dtor | 3 | Clean up ScoreDisplay: free strings, timers, BaseObjects (5), SceneObject_dtor |
| 0x44acb0 | UIList_Clear | 4 | Empty stub (returns 0), clear/reset UI list |

## Board Level Constructors (Session 2026-04-13 Batch 5)

| Address | Name | Xrefs | Description |
|---------|------|-------|-------------|
| 0x41CA40 | BoardLevel1_WarmUp_ctor | 1 | Board constructor for Warm-Up race (Level 1), calls Board_ctor, sets "Board (Warm-Up)", loads BEGINNERRACE data |
| 0x41CB10 | BoardLevel1_WarmUp_dtor | 1 | Board destructor for Warm-Up race, sets vtable to 0x4D04A8, calls Scene_dtor |
| 0x41CB20 | BoardLevel2_Intermediate_ctor | 1 | Board constructor for Intermediate race (Level 2), loads Level2-Bridge, creates level clone, TipperVisual_Attach |
| 0x41CC80 | BoardLevel2_Intermediate_dtor | 1 | Board destructor for Intermediate race |
| 0x41D450 | BoardLevel3_Dizzy_dtor | 1 | Board destructor for Dizzy race (Level 3), frees Vec3Lists at +0x11E4 and +0x10DE |
| 0x41E340 | BoardLevel5_Tower_ctor | 1 | Board constructor for Tower race (Level 5), loads 6 levels: Level4-Catapult, Level4-Drawbridge, Level4-Mace, Level4-Windmill, Level4-Turret, plus YellowLink and Chomper meshes |
| 0x41E640 | BoardLevel5_Tower_dtor | 1 | Board destructor for Tower race, frees Vec3Lists at multiple offsets |
| 0x41EA40 | BoardLevel8_Expert_ctor | 1 | Board constructor for Expert race (Level 8), loads Level5-Bridge, creates clone, 3x hammyjudge meshes |
| 0x41EC90 | BoardLevel8_Expert_dtor | 1 | Board destructor for Expert race |
| 0x41ED80 | BoardLevel9_Odd_ctor | 1 | Board constructor for Odd race (Level 9), sets "Board (Odd)", ODDRACE data |
| 0x41EE70 | BoardLevel9_Odd_dtor | 1 | Board destructor for Odd race |
| 0x41F110 | BoardLevel12_Wobbly_ctor | 1 | Board constructor for Wobbly race (Level 12), loads 7 levels: Level7-Wobbly1 through Level7-Wobbly7 |
| 0x41F3C0 | BoardLevel12_Wobbly_dtor | 1 | Board destructor for Wobbly race |
| 0x41F720 | BoardLevel_Toob_dtor | 1 | Board destructor for Toob race |

## Collision System (Session 2026-04-13 Batch 5)

| Address | Name | Xrefs | Description |
|---------|------|-------|-------------|
| 0x456D80 | CollisionMesh_ctor | 1 | CollisionMesh constructor: initializes triangle list for mesh collision detection, sets up AthenaLists |
| 0x456120 | CollisionMesh_AddTriangle | 1 | Add triangle to collision mesh: appends to list at +0x430, sets back-pointer at +0x08 |
| 0x4564C0 | Ball_AdvancePositionOrCollision | 1 | Ball physics: advance position with collision detection. Uses spatial tree for mesh traversal, handles material accumulation, collision response. Core physics function. |
| 0x463330 | SpatialTree_ctor | 1 | SpatialTree (octree) constructor for collision spatial partitioning |
| 0x4632E0 | SpatialTree_Free | 1 | Free spatial tree nodes and cleanup |

## CRT/Misc

| Address | Name | Xrefs | Description |
|---------|------|-------|-------------|
| 0x4C5F5A | CRT_amsg_exit | 10 | CRT error message exit (__amsg_exit) |

## Newly Documented Functions (Session 2026-04-13)

| Address | Name | Xrefs | Description |
|---------|------|-------|-------------|
| 0x440390 | Scene_UpdateArenaPhysics | 1 | Arena physics update: wave motion, collision detection, sound triggers |
| 0x466cf0 | CollisionNode_ctor | 1 | Collision node constructor: sets friction values (0.1), vtable |
| 0x477020 | CollisionNode_BaseInit | 1 | Collision node base initialization |
| 0x459aba | Triangle_Interpolate2D | 1 | 2D triangle interpolation (barycentric coords) |
| 0x453c00 | Graphics_InitShaderProfileThunk | 1 | Shader profile init thunk |
| 0x441150 | UI_CheckKeyCombo | 2 | UI key combo handler (calls vtable on press) |
| 0x41f7e0 | Scene_HandleRaceEnd_ClampZoom | 1 | Clamps camera zoom levels after race end |
| 0x430330 | PauseRumbleMenu_ctor | 2 | Pause Rumble menu constructor (RESUME, OPTIONS, QUIT) |
| 0x4601a0 | Scene_MarkDirty | 2 | Recursively marks scene objects as dirty (tree traversal) |
| 0x470680 | MeshWorld_ClearObjectLists | 3 | Clears AthenaLists on marked objects during cleanup |
| 0x474e70 | Menu_dtor | 1 | Menu destructor: frees 10 Vec3List entries, clears AthenaLists |
| 0x475000 | Menu_DeletingDtor | 1 | Menu scalar deleting destructor |
| 0x441190 | OkayDialog_Render | 2 | Okay dialog render: draws gradient bars, "OKAY!" button |
| 0x4415b0 | UI_SetQuarterViewport | 1 | Sets quarter-screen viewport dimensions (50x50) |
| 0x446710 | UI_ConfirmYes | 1 | UI confirm handler ("YES" dialog response) |
| 0x424c10 | RumbleBoard_DeletingDtor | 1 | RumbleBoard scalar deleting destructor |
| 0x460220 | Scene_ResetObjectSlots | 1 | Resets scene object slots, re-registers objects |
| 0x453100 | Rect_ContainsPoint | 1 | Point-in-rectangle test (AABB containment) |
| 0x4415f0 | UI_ResetViewportToQuarter | 1 | Resets viewport to quarter-screen (4x 0x32 values) |
| 0x442540 | UI_SetPauseRightButtonText | 1 | Sets "PAUSE W/RIGHT BUTTON: %s" text in UIList |
| 0x4431c0 | UIList_DeletingDtor | 1 | UIList scalar deleting destructor |
| 0x441660 | UIList_dtor | 1 | UIList destructor: initializes 5 AthenaHashTables, cleanup |
| 0x4502f0 | RumbleBoard_Menu_dtor | 1 | RumbleBoard menu destructor: vtable, timer, UIList cleanup |
| 0x445230 | Scene_StartTournament | 1 | Starts tournament mode: creates TourneyMenu, sets music tempo |
| 0x446730 | Tourney_SaveTournament | 1 | Saves tournament to "DATA\\tournament.sav" |
| 0x450960 | Tourney_AdvanceRound | 1 | Advances tournament to next round, creates new TourneyMenu |
| 0x4508f0 | Tourney_SetCurrentLevel | 1 | Sets tournament current level, material color |
| 0x4610e0 | MeshWorld_CollectTrianglesInAABB | 3 | Recursive AABB triangle collection (collision mesh query) |
| 0x467730 | Array_Fill | 1 | Simple array fill (memset-like) |
| 0x468050 | Vector_Init | 1 | Vector/array initializer (allocates dword array) |
| 0x469090 | SpatialNode_SwapBuffers | 1 | Spatial tree node buffer swap (copy/move semantics) |
| 0x4707b0 | MeshObject_dtor | 1 | Mesh object destructor: releases textures, frees mesh data |
| 0x471c00 | MeshObject_DeletingDtor | 1 | MeshObject scalar deleting destructor |
| 0x475020 | MeshWorld_AddTexture | 2 | Adds texture to mesh world (creates 0x48 byte texture object) |
| 0x476009 | Bit_ShiftByte | 1 | Bit shift helper for byte values |
| 0x4471e0 | StdString_Replace | 1 | String replace operation (std::string-like) |
| 0x448410 | UI_TextEdit_PasteFromClipboard | 2 | Pastes text from Windows clipboard to UI |
| 0x4492d0 | UIList_AddItem | 1 | Adds item to UIList (creates UIListItem, copies string) |
| 0x441640 | SceneObject_DeletingDtor | 1 | SceneObject scalar deleting destructor |
| 0x4722e0 | AthenaVector_Init | 1 | Athena vector/list initializer |
| 0x473030 | Registry_SetValue | 4 | Windows registry value setter (RegSetValueExA wrapper) |
| 0x451b90 | TourneyMenu_ctor | 1 | Tournament menu constructor: difficulty, music, scoring thresholds |
| 0x462380 | SpatialTree_CloneToLevel | 2 | Recursively clones spatial tree to Level object |
| 0x456870 | Mesh_DeletingDtor | 1 | Mesh scalar deleting destructor |
| 0x479000 | Sound_EnumerateDevices | 1 | Sound device enumeration callback registration |
| 0x480032 | D3DTexture_ResizeAndValidate | 5 | D3D texture resize and compression format validation |

## Session 17 - SSE2/Math/Texture/AthenaString/Gfx Functions

| Address | Name | Xrefs | Description |
|---------|------|-------|-------------|
| 0x4b22ab | BitStream_ReadBitsSSE2 | 19 | SSE2 bitstream reader: reads N bits from compressed stream, skips 0xFF markers |
| 0x4a0f3a | Matrix_Inverse4x4_SSE2 | 8 | SSE2 4x4 matrix inverse using cofactor expansion + Newton-Raphson reciprocal |
| 0x4a6b80 | SSE2_SetFPControlWord | 9 | Sets FPU control word from param, stores to global DAT_00535280 |
| 0x4aba49 | Mem_Zero | 8 | Optimized memset-to-zero: dword loop + byte tail |
| 0x4ad576 | Malloc_OrLongjmp | 9 | Safe malloc wrapper, longjmps on failure with "Out of Memory" |
| 0x4bc360 | StrCat_Fast | 9 | Optimized strcat with dword-aligned null detection (0x7efefeff trick) |
| 0x4b8564 | BitStream_CopyToOutput | 8 | Bitstream copy-to-buffer with checksum callback and double-buffer |
| 0x4c183e | Noop2 | 8 | Empty no-op function stub |
| 0x4c7152 | ReturnZero | 8 | Stub function returning 0 |
| 0x459d96 | D3DX_ShaderDispatch0 | 3 | D3DX shader dispatch: calls D3DX_DetectShaderProfile then vtable[0x4F7194] |
| 0x459e34 | D3DX_ShaderDispatch1 | 4 | D3DX shader dispatch via PTR 0x4F71B8 |
| 0x459ed1 | D3DX_ShaderDispatch2 | 4 | D3DX shader dispatch via PTR 0x4F71CC |
| 0x467e40 | AthenaList_Ctor | 4 | AthenaList constructor: sets vtable 0x4E998C, calls StdString_Substr |
| 0x46a0d0 | Audio_StopChannel | 4 | BASS_ChannelStop on channel at this+0x08 |
| 0x46b840 | SceneObject_EmptyListCtor | 3 | Sets vtable to SceneObject_DeletingDtor(0x4D9368), inits AthenaList |
| 0x46db10 | App_Shutdown | 4 | App shutdown: destroys window, releases 5 COM objects, CoUninitialize |
| 0x46dfa0 | NetworkConnection_Ctor | 3 | NetworkConnection init: sets "Not Connected", +0x0C=1.0f |
| 0x472990 | Gadget_LabelCtor | 4 | UI label widget ctor: Gadget+label, vtable 0x4D9E68, name at +0x878 |
| 0x69b20 | UIWidget_HitTest | 3 | UI hit-test: checks +0x420 rect or iterates children for point containment |
| 0x73480 | AthenaString_Reserve | 4 | String reserve: allocates new buffer, copies old string, free old |
| 0x73600 | AthenaString_Length | 3 | Recalculates string length from buffer, caches at +0x14 |
| 0x73640 | AthenaString_Find | 4 | strstr wrapper: finds substring, returns offset or -1 |
| 0x77010 | D3DX_RegistryGetter | 4 | Sets vtable from DirectX registry path struct at 0x4DA65C |
| 0x75dc0 | CRC32_Compute | 3 | CRC32 using 256-entry lookup table at 0x4F7534 |
| 0x77670 | Vec3_ClosestPointOnLine | 3 | Projects point onto line segment, clamps t, returns nearest point |
| 0x77970 | Texture_StreamRead | 3 | Texture streaming read with 1KB chunks and progress callback |
| 0x77ac0 | Texture_BinarySearch | 4 | Texture binary search for load position optimization |
| 0x77d60 | Texture_LoadFromStream | 4 | Texture load from stream with D3D validation and retry |
| 0x78680 | Texture_dtor | 4 | Texture destructor: frees sub-textures, pixel buffers, refcount release |
| 0x45ace5 | D3DX_ShaderDispatch_4 | 4 | D3DX shader dispatch with 4 params via PTR 0x4F7214 |
| 0x45ad75 | D3DX_ShaderDispatch_4b | 4 | D3DX shader dispatch with 4 params via PTR 0x4F71F8 |
| 0x45ae05 | D3DX_ShaderDispatch_2a | 4 | D3DX shader dispatch with 2 params via PTR 0x4F7208 |
| 0x45aea1 | D3DX_ShaderDispatch_2b | 4 | D3DX shader dispatch with 2 params via PTR 0x4F720C |
| 0x45af3e | D3DX_ShaderDispatch_2c | 4 | D3DX shader dispatch with 2 params via PTR 0x4F7210 |
| 0x45afdc | D3DX_ShaderDispatch_3 | 4 | D3DX shader dispatch with 3 params via PTR 0x4F7238 |
| 0x45b521 | D3DX_ShaderDispatch_noarg | 4 | D3DX shader dispatch with no args via PTR 0x4F71FC |
| 0x45b7a8 | D3DX_ShaderDispatch_noarg2 | 4 | D3DX shader dispatch via PTR table |
| 0x45baeb | D3DX_ShaderDispatch_noarg3 | 4 | D3DX shader dispatch via PTR table |
| 0x45bc10 | D3DX_ShaderDispatch_noarg4 | 4 | D3DX shader dispatch via PTR table |
| 0x45bf20 | D3DX_ShaderDispatch_noarg5 | 4 | D3DX shader dispatch via PTR table |
| 0x8a160 | Texture_SetPool | 6 | Sets texture pool on struct (calls init then assigns param) |
| 0x8a180 | Pool_dtor | 6 | Pool destructor: frees pool list, zeroes 8 dwords |
| 0x8a560 | Texture_ValidateOrBuild | 5 | Validates texture generation or rebuilds from source data |
| 0x8a860 | Texture_DestroyBuffers | 5 | Destroys indexed buffer list (frees each entry then array) |
| 0x8a8d0 | Mesh_Init | 3 | Mesh initializer: zeroes 8 dwords, callocs 0xCA8 byte sub-struct |
| 0x8a900 | Mesh_dtor | 6 | Mesh destructor: frees vertex/index/shader buffers, zeroes 8 dwords |
| 0x89df0 | Pool_Free | 4 | Frees two pool lists and zeroes 0x14 dwords |
| 0x89e20 | Texture_ComputeChecksum | 4 | Iterates byte stream summing values, marks 0x80000000 on non-0xFF terminator |
| 0x8b1c0 | Pool_Alloc | 6 | Pool allocator: 8-byte aligned alloc with linked-list overflow |
| 0x8b2a0 | VertexDeclaration_dtor | 3 | Vertex declaration destructor: frees buffer at +0x4C, zeroes 0x18 dwords |
| 0x8b530 | VertexShader_dtor | 3 | Vertex shader destructor: frees vertex/index buffers, releases DX objects |
| 0x80c4d | Gfx_ResizeBuffers | 5 | D3D render target resize: recreates vertex/index buffers, validates device |
| 0x472340 | Font_RenderToTextureComplex | 4 | Complex font rendering to texture with vertex buffers and shaders |

## Session 18 - RumbleBoard/BoardLevel Scalar Destructors

| Address | Name | Description |
|---------|------|-------------|
| 0x425360 | RumbleBoard_CollSlices_scalar_dtor | Scalar deleting destructor for CollSlices board |
| 0x425380 | RumbleBoard_CollSlices_dtor | Destructor for CollSlices board |
| 0x4253e0 | RumbleBoard_Expert_Arena_scalar_dtor | Scalar deleting destructor for Expert Arena board |
| 0x425400 | RumbleBoard9_PopCylinder_scalar_dtor | Scalar deleting destructor for RumbleBoard9 PopCylinder |
| 0x425420 | BoardLevel14_RaceOfAges_Scene_scalar_dtor | Scalar deleting destructor for RaceOfAges scene |
| 0x425440 | RumbleBoard_Odd_Arena_scalar_dtor | Scalar deleting destructor for Odd Arena board |
| 0x425460 | BoardLevel_Glass_scalar_dtor | Scalar deleting destructor for Glass board level |
| 0x425480 | RumbleBoard_scalar_dtor | Scalar deleting destructor for RumbleBoard |
| 0x4254c0 | RumbleBoard_Level_scalar_dtor | Scalar deleting destructor for RumbleBoard level |

## Session 18 - CreditsScreen, MusicPlayer, TourneyMenu

| Address | Name | Description |
|---------|------|-------------|
| 0x425920 | CreditsScreen_dtor | CreditsScreen destructor |
| 0x425ac0 | CreditsScreen_Render | CreditsScreen render/layout |
| 0x425d80 | CreditsScreen_Layout | CreditsScreen layout calculation |
| 0x425f70 | CreditsScreen_scalar_dtor | CreditsScreen scalar deleting destructor |
| 0x426030 | MusicPlayer_ctor | MusicPlayer constructor |
| 0x4260c0 | MusicPlayer_dtor | MusicPlayer destructor |
| 0x426130 | MusicPlayer_scalar_dtor | MusicPlayer scalar deleting destructor |
| 0x426150 | MusicPlayer_Render | MusicPlayer render/update |
| 0x4264a0 | TourneyMenu_GetRaceName | Get race name string for tournament |
| 0x4264b0 | TourneyMenu_WriteSave | Write tournament save data |
| 0x4265a0 | TourneyMenu_LoadSaveAndShow | Load tournament save and display |
| 0x4266f0 | TourneyMenu_Advance | Advance tournament to next round |
| 0x426780 | TourneyMenu_CreateBoard | Create board for tournament race |
| 0x426af0 | TourneyRaceEntry_scalar_dtor | TourneyRaceEntry scalar deleting destructor |
| 0x431d00 | TourneyRaceEntry_scalar_dtor2 | TourneyRaceEntry alternate scalar deleting destructor |
| 0x432d20 | TourneyMenu_Render | TourneyMenu render function |

## Session 18 - AthenaString

| Address | Name | Description |
|---------|------|-------------|
| 0x426bae | AthenaString_MoveAssign | AthenaString move assignment operator |
| 0x426c50 | AthenaString_Reserve | AthenaString buffer reserve/capacity |

## Session 18 - Pendulum/Rotator/CollisionFace

| Address | Name | Description |
|---------|------|-------------|
| 0x434030 | Rotator_PlayCollisionSound | Play collision sound for Rotator |
| 0x434070 | Rotator_Render | Rotator render function |
| 0x4362c0 | Pendulum_Cleanup_vtable | Pendulum vtable cleanup thunk |
| 0x4362d0 | Pendulum_Render | Pendulum render function |
| 0x436390 | Pendulum_AddIndex | Add index to Pendulum object |
| 0x436530 | Rotator_Cleanup_vtable | Rotator vtable cleanup thunk |
| 0x4366f0 | Rotator_ctor | Rotator constructor |
| 0x4367d0 | Rotator_Cleanup | Rotator cleanup/release |
| 0x4367e0 | Rotator_StartSound | Rotator start sound playback |
| 0x436860 | CollisionFace_ctor | CollisionFace constructor |
| 0x436910 | CollisionFace_dtor_vtable | CollisionFace vtable destructor thunk |
| 0x436920 | Rotator_ctor_sound | Rotator constructor (sound variant) |
| 0x436a10 | Rotator_Cleanup_vtable2 | Rotator vtable cleanup thunk variant 2 |
| 0x436a20 | Pendulum_ctor | Pendulum constructor |
| 0x436b10 | Pendulum_Cleanup | Pendulum cleanup/release |
| 0x436b70 | Pendulum_PlayCollisionSound | Play collision sound for Pendulum |
| 0x436c10 | Rotator_ctor_nosound | Rotator constructor (no sound variant) |
| 0x436ce0 | Rotator_Cleanup_vtable3 | Rotator vtable cleanup thunk variant 3 |
| 0x436cf0 | Rotator_TriggerSound | Rotator trigger/start sound |
| 0x436e40 | Pendulum_scalar_dtor | Pendulum scalar deleting destructor |
| 0x436e60 | Pendulum_Cleanup_vtable2 | Pendulum vtable cleanup thunk variant 2 |
| 0x436e70 | Pendulum_ResetAndFire | Reset and fire Pendulum |
| 0x436fb0 | Rotator_Cleanup_vtable4 | Rotator vtable cleanup thunk variant 4 |
| 0x436fc0 | Rotator_RemoveAndFree | Remove and free Rotator object |
| 0x4371a0 | Rotator_Cleanup_vtable5 | Rotator vtable cleanup thunk variant 5 |
| 0x4371f0 | Rotator_MarkTriggered | Mark Rotator as triggered |
| 0x4372f0 | Rotator_Cleanup_vtable6 | Rotator vtable cleanup thunk variant 6 |

## Session 18 - Level_Cleanup Variants

| Address | Name | Description |
|---------|------|-------------|
| 0x433f00 | Level_Cleanup_vtable1 | Level vtable cleanup thunk variant 1 |

## Session 21 - Scene Rendering, Mesh, Font, Huffman, D3DTexture, Sound, CRT

| Address | Name | Description |
|---------|------|-------------|
| 0x0045ec30 | Scene_Render3DObjects | Main 3D object renderer: transforms vertices, frustum culls, submits triangle strips with D3D draw calls |
| 0x00468600 | Path_ComputeSegmentLengths | Compute segment lengths from point pairs using sqrt(dx²+dy²) |
| 0x0045afdc | D3DX_ShaderThunk_Profile1 | Thunk: calls D3DX_DetectShaderProfile(1) then indirect via PTR_FUN_004f7238 |
| 0x0045b521 | D3DX_ShaderThunk_Profile1b | Thunk: calls D3DX_DetectShaderProfile(1) then indirect via PTR_FUN_004f71fc |
| 0x0045bf20 | D3DX_ShaderThunk_Profile1c | Thunk: calls D3DX_DetectShaderProfile(1) then indirect via PTR_FUN_004f71e8 |
| 0x0047c990 | Mesh_Dtor | Mesh destructor: set vtable, free ptrs +0x20/+0x28/+0x30, clear AthenaList, free Vec3Lists |
| 0x0047d160 | Mesh_FindElement | Search array of 8-float elements (stride 0x424) for matching entry, returns index or -1 |
| 0x0047d2a0 | Mesh_AddElement | If not found, append 8 floats + 0 at stride 0x424 and increment count |
| 0x0047d020 | Mesh_ConnectElements | Build adjacency between two mesh elements (param_2, param_3) via edge list |
| 0x0047d680 | FileHandle_Dtor | File handle destructor: close handle, free buffer |
| 0x00479be0 | App_GetProductVersion | Uses Version API (GetFileVersionInfoSizeA/A/VerQueryValueA) to extract ProductVersion string |
| 0x00478780 | BitStream_ReadValue | Read int64 from bitstream array with stride 0x10; can sum all values |
| 0x004ab9c8 | Math_AlignUp | Round up: ((a-1+b)/b)*b — align value to multiple of second arg |
| 0x004ab9e0 | Array_CopyElements | Copy param_5 elements of param_6 bytes each from src to dst array-of-pointers |
| 0x004abfaf | DSound_SetVolume | Set volume on sound object via vtable call (opcode 0x35) |
| 0x004ad569 | CRT_FreeIfNotNull | Free ptr if non-null, simple null-check wrapper |
| 0x004b2101 | Huffman_BuildTable | Build canonical Huffman decode table from 16-byte code length table (deflate-style) |
| 0x004b2a13 | BitStream_FlushAndReset | Flush accumulated bits, zero the history buffer, reset position, set timestamp |
| 0x004b670c | Sound_DecodeFrame | Decode a sound frame: handles types 4 (stop note), 5 (callback), 6 (sub-decode) |
| 0x0048300c | D3DTexture_Init | Create D3D texture object with vtable PTR_004db360, init format/flags, register with parent |
| 0x00483a44 | D3DTexture_InitLocked | Same as Init but with locked vtable PTR_004db3d0 (render target variant) |
| 0x00486f30 | D3DTexture_NullDtor | Trivial destructor that just sets vtable to PTR_LAB_004db4c8 |
| 0x004b0e6b | CRT_FreeParam2 | Callback wrapper: just frees param_2 (2nd arg), ignores param_1 |
| 0x004ad716 | Font_DecodeGlyphBits | Decode glyph bitmap from compressed font data based on bit depth (1/2/4/8) |

## Session 21 (continued) - Level ctors/dtors, more naming

| Address | Name | Description |
|---------|------|-------------|
| 0x004383f0 | Glass_Level_scalar_dtor | Glass level scalar deleting destructor |
| 0x004384a0 | Glass_Level_ctor | Glass level constructor: Stands init, clone level, init timers, lookup Chain1/2 Bridge/Wall hashtables |
| 0x00438730 | Impossible_Level_scalar_dtor | Impossible level scalar deleting destructor |
| 0x00438830 | Cascade_Level_scalar_dtor | Cascade level scalar deleting destructor |
| 0x00438b10 | WarmUp_Level_scalar_dtor | WarmUp level scalar deleting destructor |
| 0x00438f30 | Tower_Level_scalar_dtor | Tower level scalar deleting destructor |
| 0x004396f0 | Spinner_Level_ctor | Spinner level constructor: Stands init, position/velocity, timer, scale X/Z, clone level, 100 score, 1.0 factor |
| 0x00439850 | Intermediate_Level_scalar_dtor | Intermediate level scalar deleting destructor |
| 0x00439870 | Impossible_Level_Update | Impossible level update: countdown timer, gravity, scale/position, sound |
| 0x00439b90 | Sawblade_Level_scalar_dtor | Sawblade level scalar deleting destructor |
| 0x0043a150 | Gear_Level_ctor | Gear level constructor: RNG for speed/offset, 400ms time, ScoreDisplay init |
| 0x00430430 | TourneyRaceEntry_Dtor | TourneyRaceEntry destructor: set vtable, call UIList_Cleanup |

## Session 21 (continued 2) - Level dtors, UI, String, RaceResults

| Address | Name | Description |
|---------|------|-------------|
| 0x00446860 | QuitToDesktop_scalar_dtor | QuitToDesktop scalar deleting destructor |
| 0x00446880 | GameObject2_scalar_dtor | GameObject2 scalar deleting destructor |
| 0x004468a0 | TourneyContinueDialog_scalar_dtor | TourneyContinueDialog scalar deleting destructor |
| 0x004469e0 | App_CreateConfirmMenu | Create ConfirmMenu dialog, add to scene |
| 0x00446a60 | App_CreateHighScoreEntry | Create HighScoreEntry dialog, add to scene |
| 0x00446ae0 | QuitToDesktop_Execute | Execute quit: set cull mode, start race, show results, show main menu |
| 0x00447370 | ScoreDisplay_scalar_dtor | ScoreDisplay scalar deleting destructor |
| 0x00447390 | StdString_Insert | Insert count bytes from ptr at position in std::string (SSO-aware) |
| 0x00447500 | StdString_TruncateToWidth | Erase chars from end until Font_MeasureText width <= 0x13F (319px) |
| 0x00447570 | StdString_InsertCStr | Insert C string at position (strlen + call StdString_Insert) |
| 0x0044af60 | RaceResults_scalar_dtor | RaceResults scalar deleting destructor |
| 0x0044af80 | RaceResults_Init | RaceResults init: set vtable, Matrix_Identity |
| 0x0044b8a0 | RaceResults_ctor | RaceResults constructor: init timers, random congratulatory text, score thresholds |
| 0x0044bfc0 | RaceResults_Render | Render race results: colored rect, shadow text for title/place/time |
| 0x0044c450 | RaceResults_Reset | Reset: free sub-object, cleanup timer, restore vtable |
| 0x0044c7d0 | RaceResults_Update | Update race results: advance timers, check completion, sound/voice |
| 0x0044ca80 | RaceResultsMenu_scalar_dtor | RaceResultsMenu scalar deleting destructor |
| 0x0044cb10 | RaceResultsMenu_ctor | RaceResultsMenu constructor: title, subtitle, player entries, timer |

## Session 22 - Path/Sprite/Gfx/Math Functions

| Address | Name | Description |
|---------|------|-------------|
| 0x00457370 | FontList_Dtor | FontList destructor: set vtable PTR_004d8e30, iterate/remove textures, clear list, null 0x100 entries, Vec3List_Free |
| 0x0045cb88 | Matrix_BuildRotationZYX | Build 4x4 rotation matrix from Euler ZYX angles (sin/cos with global factor 0x4d5c84) |
| 0x0045d8f0 | Ball_RenderWithMaterial | Render ball with material: Ball_Render, Graphics_ApplyMaterialAndDraw, vtable 0x120 call |
| 0x0045dfe0 | Gfx_SetupAlphaRenderState | Setup alpha render state: cull mode, texture stage 0x152, alpha test 0x16, blend 0x1d/7/0x89, texture 0x1b |
| 0x0045dcf0 | RNG_SeedSmall | Seed small RNG: 53-entry additive PRNG with state at +0xc (mask 0x3fffffff), 0x35 iterations |
| 0x0045dde0 | Gfx_SetBlendState | Set D3D blend state: vtable 0xfc calls (0, 0xd, 2) and (0, 0xe, 2), state check at +0x704 |
| 0x0045d030 | Sprite_Reset | Sprite reset: set vtable PTR_004d8f84, remove texture ref, Matrix_Identity |
| 0x0045d0a0 | Sprite_ScalarDtor | Sprite scalar destructor: calls Sprite_Reset, then free if flag bit 0 set |
| 0x0045d1d0 | Sprite_Ctor | Sprite constructor: vtable PTR_004d8f84, RenderContext_Init, sets 3 scale pairs from image dims |
| 0x0045dab0 | Sprite_DrawRotatedQuad | Draw rotated quad: 5-point star pattern via sin/cos waves around center, 4 RGBA corner colors |
| 0x00468780 | Path_BuildVertexStrips | Build vertex strips from path array: interleaved pos/uv/color quads, stride-7 format, vertex count = (count-1)*4 |
| 0x00468f30 | Path_ComputeSegDeltas | Compute segment deltas from path: outputs 4-float {0,0,delta,start} per segment |
| 0x00469580 | FileHandle_Open | Open file: close prev handle, free buffer, _open with 0x8000 flags, set status flag |
| 0x00467c60 | Array_CopyDWordsThunk | Thunk: just calls Array_CopyDWords |
| 0x00467e00 | DualBuffer_Free | Free dual buffer: frees two pointers at +0x14 and +0x04, zeros 6 DWORDs |
| 0x00468510 | PathGroup_Init | PathGroup init: zero 15 DWORDs across 5 groups of 3 |
| 0x00466cd0 | Transform_SetDefaultScale | Set transform default scale: NoOp + write 0.1f at +0xc and +0x10 |
| 0x00466d50 | Transform_ScalarDtor | Transform scalar destructor: calls sub_466cc0 then free if flag bit 0 set |
| 0x00467780 | Array_CopyBackward | Copy DWORDs backward: src→dest in reverse, count in bytes |
| 0x004677b0 | Matrix_SolveGaussElim | Gaussian elimination with partial pivoting: solves Ax=b for matrix at param_1 |
| 0x00467cc0 | Array_FillAndAdvance | Fill DWORD array and advance: Array_FillDWords, returns ptr + count |
| 0x00467cf0 | Vector_InsertRange | Insert range into vector: copy [param_2, param_3) into this container, update end ptr |
| 0x004685e0 | PathGroup_PushPair | PathGroup push pair: Vector_PushBack of 4 then 8 onto two vectors offset by 0x10 |
| 0x004692d0 | SceneObject_ScalarDtor | SceneObject scalar destructor: calls SceneObject_dtor then free if flag bit 0 set |

## Session 22 (continued) - App Lifecycle, Input, Gfx Lighting

| Address | Name | Description |
|---------|------|-------------|
| 0x0046c050 | App_CreateInputDevice | Allocate 0x91c bytes, call FUN_00466620 (input device ctor), store at App+0x178 |
| 0x0046c0b0 | App_CreateAudioDevice | Allocate 0x424 bytes, call Audio_Init, store at App+0x17c |
| 0x0046c170 | App_FrameUpdate | Frame update: GetCursorPos, WindowFromPoint, poll input/audio/collision, GameUpdate |
| 0x0046c200 | App_ResetFrame | Reset frame: Scene_ResetCameraAndFrameCount, then Graphics_Clear |
| 0x0046c260 | App_TickGameUpdate | Thunk: call FUN_00469a60(App+0x184) - tick game update |
| 0x0046c290 | App_OnMouseDown | Mouse down handler: SetCapture, set button flags, UIWidget_HitTest, dispatch to widget |
| 0x0046f100 | Gfx_ApplyLightingState | Set D3D lighting state: specular enable, light enable, material emission, 0x39 state |
| 0x0046f1e0 | Gfx_ResetLighting | Reset lighting: disable specular, set render state 0x1b=0, material type 0, state 0x39 |
| 0x0046ca20 | App_ParseGUID | Parse GUID from string: MultiByteToWideChar + CLSIDFromString, store at this+0x44 |
| 0x0046dc20 | App_ScalarDtor | App scalar destructor: calls App_Shutdown then free if flag bit 0 |
| 0x0046dc40 | App_Ctor | App constructor: vtable, size 640/480, cursors, CoInitialize, input device 0x848 |
| 0x0046e910 | KeyboardDevice_ScalarDtor | KeyboardDevice scalar destructor: calls KeyboardDevice_dtor then free |
| 0x0046ebd0 | InputDevice_PollAndRelease | Poll DInput device, acquire on error, release 4 sub-devices |
| 0x00471c60 | Vector_FillResize | Fill vector with count DWORDs, update end pointer

## Session 23 — Math, Sound, D3D, Stream, Spatial (74.4→75.2%)

| Address | Name | Xrefs | Description |
|---------|------|-------|-------------|
| 0x4637f0 | Vec3_Abs | 2 | Compute absolute value of 3-component vector (fabs each) |
| 0x45b345 | Matrix_BuildLookAt | 1 | Compute view matrix from eye, target, up vectors (cross products) |
| 0x45b48d | Matrix_BuildPerspectiveFOV | 1 | Compute perspective projection matrix from FOV, aspect, near, far |
| 0x45b1f9 | D3D_Thunk4 | 1 | D3D dispatch thunk through PTR_FUN_004f7230 (4 params) |
| 0x45b227 | D3D_Thunk0 | 1 | D3D dispatch thunk through PTR_FUN_004f722c (0 params) |
| 0x45b238 | D3D_Thunk5 | 1 | D3D dispatch thunk through PTR_FUN_004f7234 (5 params) |
| 0x45be69 | D3D_ThunkIndirect | 1 | D3D dispatch thunk through DAT_004f71e4 |
| 0x457690 | Font_DrawGlyph3D | 1 | Render 3D text — iterate chars, render with Ball_RenderWithMaterial |
| 0x476270 | D3DX_ErrorHandler_Ctor | 1 | Init D3DX error handler — set vtable 0x4DA194 + store param |
| 0x466060 | Level_CloneTree | 1 | Recursively clone Level tree — create CollisionLevel for each child |
| 0x466320 | SoundChannel_Ctor | 1 | Init sound channel vtable 0x4D90E8 with 7 zero fields |
| 0x466480 | SpatialTree_ComputeOffset | 1 | Compute vector offset with cross-product area for spatial tree |
| 0x4664f0 | Sound_LoadAndAppend | 1 | Create AthenaList entry, load sound via Sound_LoadOggOrWav |
| 0x466620 | SoundDevice_ctor | 1 | Sound device constructor — vtable 0x4D911C, 16 channels, DS init |
| 0x466a10 | SoundDevice_UpdateChannels | 1 | Tick sound channels, play on timer expire, remove expired |
| 0x466b80 | SoundDevice_Play3DAll | 1 | Iterate all sound channels, call Sound_Play3DAtPosition per entry |
| 0x466c50 | SoundDevice_DeletingDtor | 1 | Scalar deleting destructor for SoundDevice |
| 0x466c90 | SoundBuffer_ScalarDtor | 1 | Free buffer at +4, scalar deleting destructor |
| 0x466cc0 | D3DX_RegistryGetter_Ctor | 1 | Init D3DX registry getter — set vtable 0x4D9128 |
| 0x468700 | D3DXSkinMesh_Init | 1 | Initialize D3DX skin mesh — two vector resizes |
| 0x4632f0 | SpatialTree_SetDefaults | 1 | Set spatial tree defaults: scale=0.1, depth=6, min=0.9 |
| 0x4633b0 | SpatialTree_DeletingDtor | 1 | Scalar deleting destructor — free then delete |
| 0x463880 | SpatialTree_ForEach | 1 | Iterate tree children calling vtable[0x20] on each |
| 0x4694a0 | StreamReader_DeletingDtor | 1 | Scalar deleting destructor calling StreamReader_dtor |
| 0x4694c0 | StreamReader_dtor | 2 | Free 2 buffers at +4/+8, set vtable 0x4D91B8 |
| 0x4698d0 | FileStream_DeletingDtor | 1 | Close file handle +0x10, free buffer +0x8 |
| 0x469920 | MeshWorld_ctor_v2 | 1 | MeshWorld constructor v2 — vtable 0x4D91C4 |

| 0x4699d0 | MeshWorld_RemoveObject | 1 | Remove object from mesh list, clear current references, call dtor |
| 0x469a40 | MeshWorld_ClearCurrent | 1 | Clear current item by calling vtable+0x30 |
| 0x469a80 | MeshWorld_CallNext | 1 | Call vtable+0x24 on next item at +0x424 |
| 0x469ac0 | MeshWorld_SkipOrAdvance | 1 | Skip current (param_2!=0) or advance to next if matches param_1 |
| 0x469c30 | MeshWorld_dtor2 | 1 | Clear list, iterate items calling dtor(1), Vec3List_Free |
| 0x469ec0 | MeshWorld_ActiveUpdate | 1 | Iterate objects, skip inactive (+0xB), set obj ID, call vtable+8 |
| 0x469f50 | MeshWorld_CallVtable34 | 1 | Iterate objects calling vtable+0x34 |
| 0x469fc0 | MeshWorld_DeletingDtor2 | 1 | Scalar deleting destructor for MeshWorld variant |
| 0x469fe0 | MusicChannel_Ctor | 1 | Init vtable 0x4D91D8, AthenaList, volume=1.0, flags=0 |
| 0x46a180 | MusicChannel_FadeUpdate | 1 | Fade BASS volume up/down with BASS_ChannelSetAttributes |
| 0x46a250 | MusicChannel_Cleanup | 1 | Free all channel buffers, clear AthenaList, Vec3List_Free |
| 0x46a4b0 | MusicChannel_DeletingDtor | 1 | Scalar deleting destructor for MusicChannel |

## Session 25 - Mesh/Vertex/D3D thunks, Array utilities (63.8%)

| Address | Name | Description |
|---------|------|-------------|
| 0x0047ead8 | DynArray_Grow | Dynamic array grow: realloc with doubled capacity |
| 0x0047ebae | FindInSmallIntArray | Search small int array for matching value |
| 0x00480c4d | MeshBuffer_Allocate | Allocate mesh vertex/index buffer |
| 0x00489217 | SetFileSecurityW | IAT thunk for SetFileSecurityW |
| 0x0048a560 | AthenaList_SortMerge | AthenaList merge sort implementation |
| 0x0048a860 | MeshGroup_dtor | MeshGroup destructor |
| 0x0048ce30 | TextureCache_RecursiveFree | TextureCache recursive free (tree traversal) |
| 0x0048eb20 | BuildSDFGrid | Build signed distance field grid |
| 0x004912e4 | ComputeScanlineZBuffer | Compute scanline Z-buffer for software rendering |
| 0x00496e13 | Mesh_SetVertexFormat | Set mesh vertex format/declaration |
| 0x0045b72c | D3DX_Thunk3Param | D3D dispatch thunk through PTR_FUN_004f7204 (3 params) |
| 0x0047e026 | WideString_MatchSlot | Match ushort value against 3-slot array, return index (0/1/2) |
| 0x0047e152 | WideArray_Grow | Realloc wchar array with capacity doubling |
| 0x0047e359 | ShortArray_FindMatch3 | Search short array of size 3 for matching value |
| 0x0047e5bd | Mesh_BuildAttributeIndices | Build mesh attribute/index remapping tables (vertex→attrib, 0xFFFF sentinel) |
| 0x0047e725 | Mesh_CollectUniqueAttributes | Collect unique attribute IDs from mesh into dynamic array (doubles capacity) |
| 0x0047e855 | UshortArray_PushBack | Push ushort element with doubling capacity |
| 0x0047e914 | Mesh_RemapVertexIndices | Remap vertex/adjacency indices via lookup tables (stride 0xC) |
| 0x0047ea05 | Mesh_SwapVertices | Swap two mesh vertices + adjacency/attribute/blend data |
| 0x0047eb76 | MeshData_Init | Init mesh data struct: set 3 params, zero counts at +0x18/+0x2C |
| 0x0047ec49 | Mesh_SwapVertexData | Swap vertex data between indices using alloca temp buffer (stride from +0x2C) |
| 0x0047edc2 | MeshSubMesh_Init | Init sub-mesh struct: set 3 params, zero counts at +0x10/+0x20 |
| 0x0047ee1b | MeshEdge_GetFirstVertex | Get first valid vertex from 4-entry edge list (+0x14 base offset) |
| 0x0047ef1e | MeshAttribute_FindIndex | Search attribute short array for matching value, return found+index |
| 0x0047f02a | Mesh_UnlinkVertex | Unlink vertex from doubly-linked adjacency list (0xC stride) |
| 0x0047f17e | Mesh_LinkVertexToHead | Link vertex to head of hash bucket (stride-6 linked list) |
| 0x0047f363 | VertexDecl_CopyVertexData | Copy vertex data between declarations: blend weights, position, normal, texcoords |
| 0x0047f58e | VertexDecl_CreateFromFVF | Create vertex declaration from FVF (Flexible Vertex Format) code |
| 0x0047e263 | DWord6Array_Grow | Grow DWORD-6-stride array with realloc (init 0x28 bytes, security cookie) |

## Session 25 (continued) - Mesh Pipeline Functions

| Address | Name | Description |
|---------|------|-------------|
| 0x0047f6c7 | Mesh_RenderSubset | Render mesh subset by attribute ID via D3D DrawIndexedPrimitive (HW or SW vertex processing) |
| 0x0047f993 | Mesh_WeldVertices | Weld duplicate vertices: build adjacency hash, split edges, remap indices, fill adjacency |
| 0x0047fdc0 | Mesh_RemoveEdgeFromHash | Remove edge entry from hash bucket linked list (find matching v0/v1/v2, unlink) |
| 0x0047fe0e | Mesh_FindBestDuplicateVertex | Find best matching duplicate vertex by comparing cross-product magnitudes with epsilon |
| 0x0047feec | Mesh_ComputeTriCrossProduct | Compute scalar triple product of two 3D triangles for vertex welding comparison |
| 0x00480501 | Mesh_ValidateAdjacency | Validate mesh adjacency: check face refs, vertex bounds, edge integrity, attribute matching |

## Session 25 (continued 2) - Mesh Pipeline, Rendering, Validation

| Address | Name | Description |
|---------|------|-------------|
| 0x00480501 | Mesh_ValidateAdjacency | Validate mesh adjacency: check face refs, vertex bounds, edge integrity, attribute matching |
| 0x004806da | Mesh_DrawSubsetIndexed | Draw mesh subset by attribute ID using D3D DrawIndexedPrimitive (indexed path) |
| 0x00480813 | Mesh_WeldVertices16 | 16-bit variant of WeldVertices: build adjacency, weld duplicate verts, split edges |
| 0x00481101 | Mesh_ValidateAdjacency16 | 16-bit variant of ValidateAdjacency: check 16-bit face/edge/vertex refs |
| 0x0048130c | Mesh_ValidateAttributes | Check attribute IDs match face ranges in attribute table |

## Session 25 (continued 3) - Mesh Copy, Adjacency, Optimize

| Address | Name | Description |
|---------|------|-------------|
| 0x00481432 | Mesh_CopyFrom | Copy mesh data from source: indices, adjacency, attributes, vertices, blend data |
| 0x00481637 | Mesh_CopyFrom16 | 16-bit index variant of Mesh_CopyFrom (ushort adjacency stride 6) |
| 0x0048183d | Mesh_AdjacencyIteratorNext | Advance adjacency iterator across mesh edges (32-bit), handle boundary/interior |
| 0x00481900 | Mesh_AdjacencyIteratorNext16 | 16-bit variant of Mesh_AdjacencyIteratorNext |
| 0x00482010 | Mesh_OptimizeFaces | Optimize mesh face order for vertex cache, compact vertex remapping |

## Session 25 (continued 4) - Mesh Optimization Pipeline

| Address | Name | Description |
|---------|------|-------------|
| 0x00482137 | Mesh_OptimizeVertices | Vertex cache optimization: swap/reorder vertices for cache locality, compact index remap |
| 0x00482218 | Mesh_SortByAttribute | Sort mesh by attribute ID: build attr table, remap vertices/faces, D3DX ID3DXMesh Optimize style |

## Session 25 (continued 5) - Mesh Helper Functions

| Address | Name | Description |
|---------|------|-------------|
| 0x0047dffd | FindInSmallIntArray32 | Find int value in 3-element int array, return index or (2+not_found) |
| 0x0047e8f6 | Mesh_RemapVertexAttribute32 | Remap vertex attribute through lookup table (if flag bit 2 set at +0x0C) |
| 0x0047e96b | Mesh_SwapVertices32 | Swap vertex data between two indices using alloca temp buffer (32-bit stride) |

## Session 25 (continued 6) - Vec/Math, Level, Sound, WaterRipple, App, Input, Vector STL

| Address | Name | Description |
|---------|------|-------------|
| 0x0045c17b | Vec2_Normalize | Normalize 2D vector (length-squared check, divide by sqrt) |
| 0x0045c32f | Vec3_Normalize | Normalize 3D vector (length-squared check, divide by sqrt) |
| 0x0045c208 | Matrix_TransformVec4x3 | Transform vector by 4x3 matrix (last row implicit [0,0,0,1]) |
| 0x0045c48e | Gfx_ProjectToViewport | Project 3D coords to viewport UV using render state flags |
| 0x0045c61b | Gfx_UnprojectToNDC | Unproject screen coords back to normalized device coords via viewport rect |
| 0x0045c7c1 | D3D_ThunkShaderDispatch4 | D3D thunk dispatch with 4 params |
| 0x0045caae | Matrix_BuildOuterProductScale | Build 4x4 matrix from outer product of vec scaled and subtracted from identity |
| 0x00461460 | SceneObject_BaseInit | Base init for scene objects — AthenaList_Init, Vec3_Init, string buffer |
| 0x00461680 | SceneObject_BaseClear | Clear/reset scene object base — Vec3List_Free, Matrix_Identity, free string |
| 0x004629c0 | Level_DeletingDtor | Scalar deleting destructor for Level (calls Level_Cleanup, free if flag) |
| 0x004650e0 | Level_dtor | Level destructor — set vtable 0x4D9068, call Level_Cleanup |
| 0x00465240 | Level_DeletingDtor2 | Scalar deleting destructor variant 2 for Level |
| 0x00465860 | Level_LoadMeshes | Create MeshWorld, create MeshBuffers, parse (NOCOLLIDE)/N:/E: prefixes, create CollisionLevels |
| 0x00466570 | Level_ReadSoundVolume | Read "Sound Volume" float from registry, default 1.0 |
| 0x004665e0 | Audio_ClampPanValue | Convert float to int pan value, clamp minimum at -2000/-10000 |
| 0x004668a0 | SoundDevice_dtor | SoundDevice destructor — save volume to registry, free AthenaLists, vec3list |
| 0x00467b40 | Spline_EvalCubic | Evaluate cubic spline (t^3*a + t^2*b + t*c + d), clamp t to array bounds |
| 0x00467c30 | Array_CopyDWords3 | Copy dwords from begin to end into destination |
| 0x00467c80 | Array_CopyDWords4 | Thunk to Array_CopyDWords |
| 0x00467d60 | MultiBuffer_Free | Free 6 allocated buffers with size/capacity fields at 0x10-byte intervals |
| 0x00467ea0 | Exception_DeletingDtor | Scalar deleting destructor for std::exception |
| 0x00467ec0 | Exception_dtor | Exception destructor — free SSO string, call base ~exception |
| 0x00467f00 | Exception_AssignCStr | Exception constructor from C string — StdString_Assign |
| 0x00467f40 | Exception_ThrowVectorLength | Throw "vector<T> too long" exception |
| 0x00467fc0 | Exception_CopyCtor | Exception copy constructor — base copy + StdString_Substr |
| 0x004680b0 | Vector_InsertN | STL vector::insert — insert N elements at position (realloc/memmove) |
| 0x00468350 | Vector_Assign | STL vector::assign — copy from another vector, realloc if needed |
| 0x00468490 | Vector_Resize | STL vector::resize — grow or trim to count |
| 0x00468570 | Vector_PushBack | STL vector::push_back — append element, realloc via InsertN if needed |
| 0x00469a60 | UIWidget_CallVtable20 | Call vtable+0x20 on object at +0x424 if non-null |
| 0x00469aa0 | UIWidget_CallVtable28 | Call vtable+0x28 on object at +0x424 if non-null |
| 0x00469b20 | UIWidget_HitTest | Find widget under point — check rect bounds, iterate back-to-front |
| 0x00469be0 | UIWidget_UpdateHover | Update hover — find widget under point, call vtable+0x30 leave / vtable+0x2c enter |
| 0x0046a0e0 | RegKeyList_AppendStr | Allocate string+value pair, append to AthenaList at +0xC |
| 0x0046a3c0 | RegKeyList_CopyFromSibling | Iterate sibling list entries, call RegKeyList_AppendStr for each |
| 0x0046a6e0 | RaptisoftUtil_Ctor | Constructor — FindWindowA("Raptisoft Utility"), set vtable |
| 0x0046a7f0 | RaptisoftUtil_DeletingDtor | Destructor — Window_Notify "*** END RAPTISOFT SESSION ***" |
| 0x0046a820 | WaterRipple_dtor | Free vertex buffer, release D3D resource, identity matrix |
| 0x0046a8a0 | WaterRipple_AllocBuffers | Allocate vertex buffer array, create D3D vertex buffer |
| 0x0046a930 | WaterRipple_AdvancePhase | Increment wave phase offset at +0x28 |
| 0x0046a940 | WaterRipple_DeletingDtor | Scalar deleting destructor for WaterRipple |
| 0x0046a960 | WaterRipple_UpdateVertices | Compute water wave positions (sin/cos), normals, vertex averaging for smooth surface |
| 0x0046af30 | WaterRipple_Ctor | Constructor — vtable 0x4D9344, 10 segments, wave params, alloc buffers |
| 0x0046b070 | WaterRipple_Render | Render water ripple — update vertices, test ball intersection, draw with vertex buffer |
| 0x0046b200 | RenderList_AppendCopy | Allocate new RenderContext and copy data, or append existing to list |
| 0x0046b360 | RenderList_FreeAndClear | Iterate list calling dtor(1), then AthenaList_Free |
| 0x0046b3d0 | MeshBuffer_DeletingDtor | Scalar deleting destructor for MeshBuffer |
| 0x0046bca0 | App_WriteDisplaySettings | Write Fullscreen/ScreenWidth/ScreenHeight to registry |
| 0x0046bd00 | App_ReadDisplaySettings | Read Fullscreen/ScreenWidth/ScreenHeight from registry |
| 0x0046bff0 | App_CreateGraphics | Allocate Graphics object (0x7DC), store at App+0x174 |
| 0x0046c110 | App_CreateInputHandler | Allocate InputHandler (0x438), store at App+0x180 |
| 0x0046c3c0 | Input_OnMouseUpCapture | Decrement capture count, ReleaseCapture if zero, notify interceptor "MouseUp" |
| 0x0046c430 | Input_OnMouseUp | ReleaseCapture, notify interceptor or widget with vtable+0x18, clear button flags |
| 0x0046c760 | Input_OnMouseDown | Hit-test widget, call vtable+0x1c with button param |
| 0x0046c7c0 | App_SetFullScreen | Toggle fullscreen/windowed, Graphics_Reset, AdjustWindowRect, SetWindowPos |
| 0x0046c9e0 | App_FrameTick | If not paused, render + update scene |
| 0x0046cb00 | App_CreateScoreDisplay | Allocate 0x8A4 score display object, add to scene |
| 0x0046cb70 | App_SetTitleString | Free and replace title string at App+0x1B4 |

## Session 25 (cont7) — 44 renames — 67.5% documented

| Address | Name | Description |
|---------|------|-------------|
| 0x0046f010 | SceneObject_dtor | Destroys scene object, iterates list calling dtor(1), frees sub-objects |
| 0x0046f0e0 | SceneObject_DeletingDtor | Deleting dtor for SceneObject |
| 0x0046f310 | SpriteAnim_Ctor | Initializes sprite animation with vtable 004d9c48 |
| 0x0046f7c0 | SpriteAnim_SetRange | Sets animation range (prev=current, new=param) |
| 0x0046f7d0 | SpriteAnim_InitFromCalcTexCoords | Initializes sprite anim from texture coords (8 calls) |
| 0x0046f8d0 | MeshWorld_BuildVertexBuffer | Builds vertex buffer from mesh object lists, creates D3D VB |
| 0x0046fb50 | SpriteAnim_DeletingDtor | Deleting dtor for SpriteAnim, frees callback and data |
| 0x0046fcc0 | MeshWorld_CollectRenderLists | Collects render lists from mesh objects into target list |
| 0x0046ff50 | SafeCallDtor | Calls vtable[2] (dtor) on object ptr if non-null |
| 0x0046ff60 | MeshWorld_OptimizeAll | Optimizes all meshes, builds vertex buffers |
| 0x00471ce0 | Throw_VectorTooLong | Throws "vector<T> too long" exception |
| 0x00472570 | MeshWorld_BuildFontMeshes | Builds font meshes via Font_RenderToTextureComplex |
| 0x00472a30 | RumbleBoard_dtor | RumbleBoard destructor, frees sub-object and calls SceneObject_dtor |
| 0x00472a50 | RumbleBoard_TickDown | Decrements countdown, triggers vtable callbacks at zero |
| 0x00472a80 | RumbleBoard_RenderThenFree | Calls render (vtable+0x48) then free (vtable+0x40) |
| 0x00472ad0 | RumbleBoard_DeletingDtor | Deleting dtor for RumbleBoard |
| 0x00472b20 | HitBox_PointInBounds | AABB point-in-bounds check with float comparisons |
| 0x00472b80 | D3DTexture_Ctor | Initializes D3D texture object with vtable 004d9ecc |
| 0x00472c00 | D3DTexture_DeletingDtor | Deleting dtor for D3D texture |
| 0x00472d80 | BaseObject_DeletingDtor | Deleting dtor for BaseObject |
| 0x00472e20 | AthenaHashTable_DeletingDtor | Deleting dtor for hash table (calls ctor then frees) |
| 0x00472ea0 | RegKey_Ctor | Initializes registry key object with vtable 004d9f08 |
| 0x00472f50 | RegKey_SetSoftwarePath | Builds "SOFTWARE\%s" registry path |
| 0x00473000 | RegKey_WriteDWORD | Writes DWORD value to registry via RegSetValueExA |
| 0x00473100 | RegKey_QueryValue | Queries registry value via RegQueryValueExA |
| 0x00473220 | RegKey_DeletingDtor | Deleting dtor for RegKey, frees path string |
| 0x00473260 | eSellerate_ExtractDLL | Extracts eSellerateEngine.dll from resources to Windows dir |
| 0x00473355 | eSellerate_ExtractDLLNull | Calls eSellerate_ExtractDLL with NULL module |
| 0x0047335d | eSellerate_ReadAffiliateKey | Reads affiliate key from SOFTWARE\eSellerate registry |
| 0x00473460 | StdString_FreeBuffer | Frees internal buffer, zeroes capacity/size/ptr |
| 0x00473480 | StdString_Reserve | Reallocates buffer to param_1+1 size |
| 0x00473580 | StdString_AssignN | strncpy-assigned string with length param |
| 0x00473600 | StdString_RecalcLen | Recalculates string length if dirty flag set |
| 0x00473640 | StdString_FindSubstr | strstr wrapper returning index or -1 |
| 0x004736f0 | AthenaString_AssignCStrFree | Assign C string then free source |
| 0x00473a70 | AthenaString_AssignCStrFree2 | Variant of assign-CStr-then-free |
| 0x00473ac0 | AthenaString_AssignFree | Assign string then free source |
| 0x00473ba0 | AthenaString_Substr | Substring extraction by index/length |
| 0x00473cd0 | AthenaString_EraseRange | Erase characters in range |
| 0x00473e20 | AthenaString_EraseSubstr | Find and erase substring |
| 0x00474000 | AthenaString_Truncate | Truncate string to N characters |
| 0x004742b0 | MWParser_DumpTags | Dump tags from MW parser to string |
| 0x004743f0 | MusicChannel_LoadAndAppend | Create MusicChannel, load file, append to list |
| 0x00474480 | MusicDevice_SetVolume | Set BASS config volume |
| 0x004744b0 | MusicDevice_ReadVolume | Read "Music Volume" from registry |
| 0x00474510 | MusicDevice_MuteToggle | Toggle BASS mute on/off |
| 0x004746a0 | MusicDevice_dtor | Destroy music device, free channels, BASS_Stop/Free |
| 0x00474780 | MusicDevice_FadeAll | Update fade on all music channels |
| 0x004747e0 | MusicDevice_DeletingDtor | Deleting dtor for MusicDevice |
| 0x00474800 | Menu_Ctor | Menu class constructor, inits 8 AthenaLists |
| 0x00474900 | LoaderGadget_OK | "LoaderGadget::OK" - handles OK button |
| 0x00474930 | Menu_SetDirty | Sets dirty flag (+0x2d64) |
| 0x00474940 | Menu_MergeAllLists | Merge all 7 category lists into main list |
| 0x00474a30 | LoaderGadget_Tick | Load progress tick, updates count/percentage |
