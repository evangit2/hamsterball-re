# Hamsterball - Function Map

Binary: Hamsterball.exe (MD5: 7d25019366b8d7f55906325bd630d7fe)
Total functions: 3,958 (Ghidra auto-analysis)
User-labeled: 80+

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
- 100Hz+ (0x17 fallback)
