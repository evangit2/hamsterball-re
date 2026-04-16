---
name: hamsterball-re
description: Reverse engineer and recreate the Hamsterball game (2000s Windows game by Raptisoft)
tags: [reverse-engineering, gaming, binary-analysis, c-reimplementation]
---

# Hamsterball Reverse Engineering Skill

## Overview
Reverse engineer and recreate the Hamsterball game (2000s Windows game by Raptisoft).

**Engine Name Note:** The binary only contains one reference to "Athena" — the Win32 window class name "AthenaWindow" at 0x4D9374. There is no "Athena Engine" string, no about box, no internal branding. "Athena" as the engine name is derived from the window class. "Raptisoft" is the company name (Nick Raptis), not the engine name. Use "Athena" as the working name for namespace prefixes (AthenaString, AthenaList, etc.).

**How to prove engine names from window class strings:** In Win32 games, `WNDCLASSEX.lpszClassName` is set during window registration and traditionally carries the engine name. Proof from open-source Quake 3 Arena (`code/win32/win_glimp.c`): `#define WINDOW_CLASS_NAME "Quake 3: Arena"` followed by `wc.lpszClassName = WINDOW_CLASS_NAME; RegisterClass(&wc);`. Similarly, Half-Life/Source uses `"Valve001"`, Unity uses `"UnityWndClass"`, GameMaker uses `"YYGODragonWindow"`. These class names are developer-chosen identifiers visible via Spy++ or GetClassName(). In Hamsterball, at 0x46D218 the code does `mov [esp+0x30], 0x4D9374` ("AthenaWindow") as `lpszClassName` before calling RegisterClassExA. This is standard practice — the window class name IS the engine identifier.

## Prerequisites
- Ghidra 12.0+ at /opt/ghidra_12.0.4_PUBLIC
- GhidraMCP headless server (v5.2.0) on port 8089 — PRIMARY analysis tool
- radare2 (r2) for quick checks (supplementary, but can be primary if GhidraMCP is down)
- User preference: use ALL available tools in parallel — both Ghidra and r2 simultaneously for this difficult project
- Wine 9.0 for execution testing
- Python 3 with pefile
- **CRITICAL**: Reimpl must use same APIs as original (D3D8, DirectInput8, BASS) — NOT SDL2+OpenGL. Target i686-w64-mingw32-gcc, link -ld3d8 -ldinput8 -ldsound -ldxguid. Wine handles D3D8→OpenGL for Linux users. See docs/API_STRATEGY.md for details.

## Key Locations
- Binary: ~/hamsterball-re/originals/installed/extracted/Hamsterball.exe
- Ghidra project: ~/hamsterball-re/analysis/ghidra/HamsterballProject/
- Docs: ~/hamsterball-re/docs/
- C reimplementation: ~/hamsterball-re/reimpl/
- Level viewer: ~/hamsterball-re/reimpl/build/level_viewer

## Critical Function Addresses (CONFIRMED via Ghidra + r2)

| Address | Function | Notes |
|---------|----------|-------|
| 0x4BB4C8 | entry | CRT entry point (GetVersionEx, heap init) |
| 0x4278E0 | WinMain | Calls App_InitFull→App_Run→App_Shutdown |
| 0x429530 | App_Initialize_Full | 26 init steps, debug strings at this+0x208 |
| 0x46BB40 | App_Initialize | 12-step base init (vtable calls + D3D8) |
| 0x46BD80 | App_Run | Game loop: PeekMessage→Update→Render→Present |
| 0x46BA10 | App_Shutdown | Sets this->running=1, vtable jump |
| 0x455380 | Graphics_Initialize | Calls Direct3DCreate8, device creation |
| 0x455A60 | Graphics_Defaults | Set default render states |
| 0x453B50 | Graphics_BeginFrame | Begin frame/render setup |
| 0x455A90 | Graphics_PresentOrEnd | Present frame or end scene |
| 0x45DE30 | LoadMeshWorld | Load .meshworld level file |
| 0x4015b0 | Ball_SetupCollisionRender | Init collision mesh render objects from level data |
| 0x4016f0 | Ball_ApplyForceV2 | Alternate force application (gravity plane, ice/dizzy/tube) |
| 0x402c10 | Ball_RenderWithCollision | Ball render with collision plane check + shadow |
| 0x4280e0 | App_ShowMainMenu | Create MainMenu (0xCDC bytes), store at App+0x224 |
| 0x4288b0 | App_StartTournamentRace | Start tournament: config mirror, create scene, advance race |
| 0x446b80 | RegisterDialog_ValidateSerial | Validate serial via XOR cipher key "54138", registry write |
| 0x459660 | Sound_LoadOggOrWav | Load sound: try .ogg first, then .wav fallback |
| 0x459310 | Sound_LoadOgg | Load OGG Vorbis file into D3D sound buffer |
| 0x459810 | Sound_GetNextChannel | Circular buffer sound channel allocator |
| 0x4706E0 | MeshWorld_ctor | Constructor (0x488 bytes), vtable at 0x4D9CDC |
| 0x470930 | MeshWorld_Parse | Parse text format (*MATERIAL, *MESH tokens) |
| 0x46A020 | LoadMusicFile | BASS_MusicLoad wrapper |
| 0x46A4D0 | LoadJukebox | Parse jukebox.xml |
| 0x42DE50 | MainMenu_ctor | Main menu (LET'S PLAY, HIGH SCORES, OPTIONS, CREDITS, EXIT) |
| 0x442CE0 | OptionsMenu_ctor | Options screen (Resolution, Fullscreen, Color, Volume, Key Remap, Mouse) |
| 0x42B470 | HighScoreEntry_ctor | High score entry screen (name input + score display) |
| 0x42BD40 | HighScoreEntry_Render | Render high score entry UI |
| 0x42E060 | TournamentScreen_ctor | Tournament difficulty selector screen |
| 0x44FD60 | SaveTourneyDialog_ctor | Save tournament dialog |
| 0x4476B0 | RegisterDialog_ctor | Register/purchase dialog |
| 0x4652E0 | CollisionLevel_ctor | Collision-only level (.meshcollision format) |
| 0x465260 | Level_LoadCollision | Load binary collision mesh (planes, objects, AABB) |
| 0x4624C0 | Level_Cleanup | Level destructor (free objects, VBs, textures) |
| 0x413C20 | RumbleBoard_WarmUp_Init | Initialize Warm-Up arena (levels\arena-WarmUp) |
| 0x416F40 | RumbleBoard_Neon_Init | Initialize Neon arena (levels\arena-neon) |
| 0x456E20 | Font_MeasureText | Measure text string width for centering |
| 0x457440 | Font_DrawGlyph | Core glyph rendering (1 call = 1 glyph quad) |
| 0x4013A0 | UI_DrawTextCenteredAbsolute | Draw centered text (x - width/2) |
| 0x409C60 | UI_DrawTextCentered | Draw centered text with shadow |
| 0x4012C0 | UI_DrawTextShadow | Draw text with shadow (offset + main) |
| 0x40A120 | LoadRaceData | Parse racedata.xml |
| 0x413280 | CameraLookAt | Camera look-at targets (CAMERALOOKAT) |
| 0x429200 | ESellerate_Init | eSellerate DRM init |
| 0x4254E0 | CreditsScreen_ctor | Credits scrolling screen (formerly mislabeled Physics_Init) |
| 0x469CF0 | GameUpdate | Main game tick - iterates objects, calls vtable+4 (Update) and vtable+0x3C (Render) |
| 0x428160 | PauseGame | Pause game (RightButtonPause) |
| 0x42FAD0 | QuitRace | Quit current race |
| 0x4298C0 | TimerDisplay | Race timer display (timerblot.png) |
| 0x40FA20 | CreateBumper | BUMPER1/2/3/4 objects |
| 0x40E250 | CreateSawblade | SAWBLADE objects |
| 0x4117B0 | CreateSpeedCylinder | SPEEDCYLINDER |
| 0x412850 | CreateSpinner | N:SPINNER |
| 0x410D00 | CreateLimit | E:LIMIT boundary |
| 0x40BF50 | CreateMouseTrap | MOUSETRAP |
| 0x40C5D0 | GameObject_HandleCollision | Main collision event dispatcher — handles ALL object types: E:NODIZZY, E:SAFESWITCH, E:LIMIT, E:BREAK, E:JUMP, E:ACTION (ONCE/SCORE), E:TRAJECTORY, N:NOCONTROL, N:WATER, N:TARPIT, N:GOAL, N:MOUSETRAP, N:SECRET, N:UNLOCKSECRET, DROPIN, PIPEBONK, POPOUT |
| 0x40E6A0 | Arena_HandleCollision | Arena events: E:CALLHAMMER, E:HAMMERCHASE, E:ALERTSAW1/2, E:ACTIVATESAW1/2, E:ALERTJUDGES, E:SCORE, E:JUMP, E:BELL (+delegates to GameObject_HandleCollision) |
| 0x40DCD0 | Level_HandleCollision | Level events: E:CATAPULTBOTTOM, E:OPENSESAME, N:TRAPDOOR, E:BITE, E:MACETRIGGER, N:MACE (+delegates to GameObject_HandleCollision) |
| 0x434770 | Saw_AlertActivate | Saw blade alert mode (clear flag + 3D sound) |
| 0x434A50 | Saw_Activate | Saw blade full activate (set flag + 3D sound) |
| 0x434C40 | Judge_Reset | Reset judge objects |
| 0x434C80 | ScoreDisplay_SetTime | Set score display time value |
| 0x434E20 | Bell_Activate | Activate bell (+5 seconds extra time) |
| 0x438BB0 | Hammer_ChaseStart | Start hammer chase sequence |
| 0x434290 | Catapult_Launch | Catapult launch (set active + timer) |
| 0x4344D0 | Trapdoor_Open | Open trapdoor (scale 0→1.0) |
| 0x438410 | Trapdoor_Activate | Activate trapdoor (3D sound + timer) |
| 0x459860 | Sound_Play3D | Play 3D positioned sound (set BASS pos + play) |
| 0x4597B0 | Sound_PlayChannel | Play sound channel (pool dispatch) |
| 0x4595B0 | Sound_StartSample | Start BASS sample (vtable: reset, volume, 3D pos) |
| 0x46EC30 | Ball_GetInputForce | 3 | Input→force vector. Mode 1=keyboard (DIK at +0x50C-518, forward/back/left/right), Mode 2=mouse (cursor offset from window center, recenter if App+0x15A), Mode 4-7=joystick (axes at +0x10C/110, normalized). Output: force[0]=X*scale, force[1]=Y*scale. Speed scale at this+0x0C |
| 0x466750 | Sound_CalculateDistanceAttenuation | 3 | 3D audio rolloff. Find nearest listener (this+0x850 count, this+0x854 array of Vec3), linear interpolation between min_rolloff (this+0x914) and max_rolloff (this+0x918). Full volume below min, silent above max |
| 0x44C260 | RaceResultPopup_ctor | Race end popup (rank image + TIME'S UP!/OUT OF TIME!) |
| 0x438B30 | CreateBonkPopup | BONKPOPUP feedback |
| 0x40BAA0 | CreateSecretObjects | Create SECRET and SECRETUNLOCK objects |
| 0x43DFB0 | Secret_ctor | Secret object constructor (0x10EC bytes) |
| 0x4121D0 | CreateLevelObjects | Factory: BRIDGE, TIPPER, BONK, BBRIDGE1/2, POPCYLINDER, BLOCKDAWG1/2, CATAPULT, GLUEBIE |
| 0x4133E0 | CreatePlatformOrStands | Factory: PLATFORM, STANDS |
| 0x417FE0 | CreateMechanicalObjects | Factory: LOOPER, GEAR, BIGGEAR, ROTATOR, PENDULUM |
| 0x37040 | Platform_ctor | Platform constructor (0x10FC bytes) |
| 0x462850 | Stands_ctor | Stands constructor (0x10D0 bytes) |
| 0x435800 | Looper_ctor | Looper constructor (0x1500 bytes) |
| 0x437590 | Gear_ctor | Gear/BigGear constructor (0x1514 bytes) |
| 0x435940 | Rotator_ctor | Rotator constructor (0x1508 bytes) |
| 0x437700 | Pendulum_ctor | Pendulum constructor (0x1504 bytes) |
| 0x437960 | Tipper_ctor | Tipper constructor (0x1104 bytes) |
| 0x4661A0 | TipperVisual_ctor | Tipper visual component |
| 0x465200 | TipperVisual_Attach | Attach visual to tipper |
| 0x438850 | Bonk_ctor | Bonk (hammer) constructor (0x1200 bytes) |
| 0x436D70 | BreakBridge_ctor | Breakable bridge constructor (0x1100 bytes) |
| 0x436EE0 | PopCylinder_ctor | Pop cylinder constructor (0x10E8 bytes) |
| 0x43C310 | Blockdawg_ctor | Blockdawg constructor (0x1154 bytes) |
| 0x437E10 | Catapult_ctor | Catapult constructor (0x1108 bytes) |
| 0x437CB0 | Gluebie_ctor | Gluebie (glue blob) constructor (0x110C bytes) |
| 0x41D060 | BoardLevel3_ctor | BoardLevel3 constructor, vtable at 0x4D0890 |
| 0x40ABA0 | CheckArenaUnlock | Check arena unlock conditions |
| 0x4279F0 | LoadOrSaveConfig | Config load/save dispatcher |
| 0x42AE80 | LoadConfig | Load HS.CFG |
| 0x42B6E0 | SaveConfig | Save HS.CFG |
| 0x433AC0 | TournamentManager | Tournament save/load |
| 0x457130 | LoadFont | Load font.description + PNG glyphs |
| 0x429450 | FinishLoad | Final setup after loading |
| 0x46EE10 | Input_Init | DirectInput8 init (0x438 byte object), stored at App+0x180 |
| 0x46C110 | Input_Create | Creates Input object, stores at param+0x180 |
| 0x46F010 | Input_Cleanup | Input object cleanup/release |
| 0x46F0E0 | Input_dtor | Input destructor (delegates to Input_Cleanup) |
| 0x46E250 | KeyboardDevice_ctor | DI keyboard device (0x524 bytes), vtable at 0x4D9840 |
| 0x46DE60 | KeyboardDevice_dtor | Keyboard cleanup, releases DI device, unbinds keys |
| 0x461510 | Level_ctor | Level object constructor (loads MeshWorld, creates LevelState) |
| 0x461460 | LevelState_ctor | Ball/physics state object (0x10D4 bytes) |
| 0x4629C0 | Level_dtor | Level destructor |
| 0x65080 | Level_Clone | Clone level object for secondary display |
| 0x46F310 | Level_base_ctor | Base level init (before vtable set) |
| 0x19030 | Board_ctor | Base Board/Screen class (vtable at 0x4D0260) |
| 0x453210 | AthenaList_Init | Init 256-entry hash list (used everywhere) |
| 0x453280 | AthenaList_Clear | Clear list contents |
| 0x4532B0 | AthenaList_GetIndex | Get list iteration index |
| 0x4534D0 | AthenaList_Remove | Remove item from list |
| 0x453780 | AthenaList_Append | Append item to dynamic list (malloc/realloc) |
| 0x4532E0 | AthenaList_SortedInsert | Insert with insertion-sort (ascending/descending) |
| 0x402BC0 | AthenaList_SetIndex | Set list iteration index |
| 0x40A020 | AthenaList_GetAt | Get element by index with bounds check; returns 0 if OOB |

### Sound System — Extended (Session 2026-04-13)
| Address | Function | Notes |
|---------|----------|-------|
| 0x458f40 | SoundDevice_LoadWAV | Load WAV file→DirectSound buffer. Validates RIFF/WAVE header, "data" chunk, 6 error paths |
| 0x4668a0 | SoundDevice_dtor | Release all DS buffers, save volume to registry, free memory |
| 0x466570 | SoundDevice_ReadVolume | Read Sound Volume float from registry (default 1.0f) |

### Race Timer System (NEW — Session 2026-04-13)
| Address | Function | Notes |
|---------|----------|-------|
| 0x451df0 | RaceTimer_Tick | Advance frame, adjust speed, accumulate score, determine rank (0-15), load rank sprite |
| 0x44c880 | RaceTimer_ctor | Init timer, score thresholds (100-900), load weasel.png, update high score |

### Registry/GameInfo Utilities (Session 2026-04-13)
| Address | Function | Notes |
|---------|----------|-------|
| 0x473000 | RegKey_WriteDword | Write REG_DWORD (type 3) to registry key |
| 0x475430 | GameInfo_AddSoundEntry | Add "SOUND" resource entry (0x48 bytes) |
| 0x4752f0 | GameInfo_AddSpriteEntry | Add "SPRITE" resource entry (0x48 bytes) |
| 0x475270 | GameInfo_AddCMeshEntry | Add "BCMESH" collision mesh entry (0x48 bytes) |
| 0x46d91d | GameInfo_WriteDSoundTags | Write DSOUND/CURRENTOBJECT/CURRENTOPERATION tags |
| 0x46d230 | GameInfo_BuildFullReport | Build full diagnostic report (PRODUCT, VERSION, D3D8/9, DSOUND, etc.) |
| 0x4764f0 | D3DXERR_ToString | Convert D3DX error code to human-readable string |

### Graphics/UI — Extended (Session 2026-04-13)
| Address | Function | Notes |
|---------|----------|-------|
| 0x453ed0 | Graphics_ReadQualitySettings | Read Texture Quality, ColorMode, SafeMode from registry |
| 0x453f90 | Graphics_WriteQualitySettings | Write Texture Quality, ColorMode, SafeMode to registry |
| 0x4794d0 | SplashScreen_ctor | Brand logo + raptisoftlogo.png + showcardgothic16 font |
| 0x487070 | FontFormatString_WriteFloat | printf-style float formatting (%f, %g, %e, %a) |
| 0x4819ec | D3DXMesh_AttributeSort | Sort mesh faces by attribute ID, rebuild attribute table |
| 0x444880 | AbortConfirm_Render | "REALLY ABORT?" dialog with YES/NO, score reset warning |
| 0x445410 | RollbackConfirm_Render | "REALLY ROLL BACK?" dialog, score zeroing warning |
| 0x4431e0 | KeyRemapMenu_ctor | Keyboard remap menu (Up/Down/Left/Right/Action1/Action2) |
| 0x474900 | LoaderGadget_OK | LoaderGadget OK handler, set operation name |

### Math Utilities (NEW — Session 2026-04-13)
| Address | Function | Notes |
|---------|----------|-------|
| 0x401AA0 | Vec3_NormalizeAndScale | 59 xrefs — most common math utility. Normalizes and scales to length param_1. |
| 0x401D60 | Matrix_TransformVec3 | 15 xrefs. Transform 3D vector by 4x3 matrix. |
| 0x402BF0 | Vec3_Copy | 18 xrefs. Copy 3 floats with self-check. |
| 0x40A050 | Color_RandomRGBA | Generate 32-bit color from 4 random bytes. |
| 0x4580D0 | AABB_ContainsPoint | Test if (x,y,z) inside AABB. Used by collision spatial tree. |
| 0x453150 | Matrix_Scale4x4 | Set 4x4 matrix row scale values. |
| 0x453200 | Matrix_Identity | Set matrix to identity. |
| 0x458B50 | Matrix_ScaleTransform | Create 4x4 by scaling source matrix rows. |

### Rendering Pipeline — Extended (NEW)
| Address | Function | Notes |
|---------|----------|-------|
| 0x4542C0 | Graphics_ctor | Constructor (vtable 0x4D88A0, init render context, texture cache, frustum) |
| 0x455360 | Graphics_dtor | Destructor (cleanup + optional free) |
| 0x454550 | Graphics_Cleanup | Release D3D objects, free texture path, clear cache |
| 0x454000 | Graphics_SetTexturePath | Set custom texture prefix (strdup at +0x7D8) |
| 0x454060 | D3DFMT_ToString | Convert D3DFORMAT enum to debug string |
| 0x454B50 | Graphics_SetViewport | Set viewport dimensions |
| 0x454D30 | Graphics_Reset | Reset device with new params (CreateDevice twice) |
| 0x455D60 | Graphics_DrawScreenRect | Draw 2D rectangle via TLVERTEX tristrip. 63 xrefs. |
| 0x455110 | Graphics_ApplyMaterialAndDraw | Apply material states + draw. 17 xrefs. |
| 0x454190 | Graphics_SetRenderMode | Set shading mode, vertex shader, render states |
| 0x455B80 | Graphics_SetStreamBuffers | Set vertex buffer streams |
| 0x457FA0 | RenderContext_Init | Init 0x50 byte render context (vtable 0x4D8E68) |
| 0x401160 | Graphics_SetViewportClip | Set viewport clip from 4x4 matrix |

### Ball Physics — Extended (NEW)
| Address | Function | Notes |
|---------|----------|-------|
| 0x403100 | Ball_SetTiltedGravity | Gravity plane=1, normal (-1,0,0) |
| 0x403150 | Ball_SetFlatGravity | Gravity plane=2, normal (0,0,1) |
| 0x403850 | Ball_SetTrajectory | Set trajectory direction + force scale |
| 0x403750 | Ball_ApplyTrajectory | Apply trajectory force (normalize, scale, sound, counter=100) |
| 0x403980 | Ball_FindMeshCollision | Mesh_FindClosestCollision wrapper |
| 0x401DD0 | Ball_CreateTrailParticles | Create trail particles (10 iterations, 0x28 byte objects) |
| 0x401920 | Ball_RenderShadow | Render ball shadow at XYZ |
| 0x401DD0 | Ball_CreateTrailParticles | Create trail particles: 9 iterations, spherical camera-relative distribution using RumbleScore objects, velocity=RNG/(RNG+20), appended to scene+0x3b00 |
| 0x413280 | CameraLookAt | Arena camera init: Load arena-spawnplatform + arena-stands meshes, find CAMERALOOKAT target, set distance=45f, height=800f, max_height=800f |
| 0x419FA0 | Scene_SetCamera | 5 camera modes: Default follow, Path rail (spring+sin dampening, dist<700→0, >700→offset-sin), Shake (±50 random offset), Snap (countdown frames→hard position), Orbit (sin/cos rotation at +0x29BC angle, dist +0x29C0). Ball target at +0x758, actual at +0x76C |
| 0x4284C0 | App_SaveAllConfig | Save all config to registry: MouseSensitivity, MirrorTournament, 11 race unlock flags, 9 arena unlock flags, RightButtonPause, BestTime[0x50 bytes], Medals[0x50 bytes], 2PController1-4. Registry key at App+0x54 |
| 0x4279F0 | LoadOrSaveConfig | Config destructor: delete all resources (graphics, meshes, sounds, UI elements), save config, call ShellExecuteA("http://www.raptisoft.com") if !registered, call App_Shutdown |
| 0x446730 | Tourney_SaveTournament | Save tournament: remove DATA\\tournament.sav, call vtable save function |
| 0x446730 | Tourney_SaveTournament | Save tournament: remove DATA\\tournament.sav, call vtable save function |

### Collision System — Extended (NEW)
| Address | Function | Notes |
|---------|----------|-------|
| 0x465EF0 | Collision_TraverseSpatialTree | Recursive octree traversal calling AABB test for each face |
| 0x458000 | Collision_InitDefaultAABB | Set AABB bounds to ±39M (0x4b18967f) |
| 0x4583F0 | AABB_TriangleIntersect2 | Double AABB-triangle test (calls AABB_TriangleTest6Edges twice) |
| 0x458190 | Collision_GradientEval_Stub | Empty stub — called from Ball_Update and Gear_AdvanceAlongPath |
| 0x456D80 | CollisionMesh_ctor | CollisionMesh constructor: 0xCB4 bytes, vtable 0x4D8E10. Initializes triangle list (AthenaList at +0x18), material list (AthenaList at +0x430), vertex list (AthenaList at +0x848), position at +0xCA4, scale factors at +0xC64/0xC68 |
| 0x456120 | CollisionMesh_AddTriangle | Add triangle ref to collision mesh: appends to list at +0x430, sets back-pointer at +0x08 |
| 0x4564C0 | Ball_AdvancePositionOrCollision | **CORE PHYSICS FUNCTION**: Advances ball position with collision detection. Uses SpatialTree for mesh traversal, accumulates velocity (gravity + scale), handles sub-step interpolation with position += direction * (1 - step), collision response via vtable+0x1C callback, material tracking. Offsets: +0x14=hasTrailData, +0x18=trail data, +0x430=triangles, +0x848=vertex normals, +0xC64=scale, +0xC68=friction, +0xC70=max_distance, +0xC74=accumulated_distance, +0xC90-C98=gravity, +0xCA4=position(XYZ), +0xC7C=useGravityCallback flag |
| 0x463330 | SpatialTree_ctor | SpatialTree (octree) constructor for collision spatial partitioning. vtable 0x4D9038. Init values: +0x0C=0.1f (scale), +0x10=6 (max_depth), +0x14=0.9f (0x3F666666, min_extent). Flags +0x18-0x1E=1 (enable all axes) |
| 0x4632E0 | SpatialTree_Free | Free spatial tree leaf nodes + cleanup |

### Board Level Constructors (Session 2026-04-13 Batch 5)
Pattern: Each Board constructor calls Board_ctor, sets vtable, name strings ("Board (Name)"), level unlock flag, Vec3_Init for background color, Matrix_Identity, LoadRaceData, then creates Level_ctor + Level_Clone for split-screen, and sometimes MeshNode_ctor for static geometry.

| Map Case | Level | Board Address | Levels Loaded |
|----------|-------|--------------|---------------|
| 1 | Warm-Up | 0x41CA40 | none (uses existing data) |
| 2 | Intermediate | 0x41CB20 | Level2-Bridge |
| 3 | Dizzy | 0x41D060 (known) | Level3-Swirl |
| 5 | Tower | 0x41E340 | Level4-Catapult, Level4-Drawbridge, Level4-Mace, Level4-Windmill, Level4-Turret + YellowLink, Chomper meshes |
| 8 | Expert | 0x41EA40 | Level5-Bridge + 3x hammyjudge meshes |
| 9 | Odd | 0x41ED80 | none (single level) |
| 12 | Wobbly | 0x41F110 | Level7-Wobbly1 through Level7-Wobbly7 (7 levels!) |

### Input System (NEW)
| Address | Function | Notes |
|---------|----------|-------|
| 0x46E0B0 | Input_IsKeyDown | Multi-device key check (keyboard=0, mouse=1, joystick=3-7) |
| 0x46EC30 | Ball_GetInputForce | Get ball input direction from keyboard/mouse/joystick |
| 0x46EE10 | Input_Init | DirectInput8 init |
| 0x46C110 | Input_Create | Creates Input object, stores at App+0x180 |
| 0x46F010 | Input_Cleanup | Input object cleanup/release |

### Mesh System (Extended)
| Address | Function | Notes |
|---------|----------|-------|
| 0x456390 | Mesh_Clear | Free vertex/index buffers, clear AthenaLists |
| 0x46F340 | MeshBuffer_Cleanup | Cleanup mesh buffer resources, vtable 0x4D9C48 |
| 0x46F670 | Mesh_SaveAndFree | Save mesh data to file (magic 0xBEEF), free buffers |
| 0x46FD60 | Mesh_AddVertex | Add vertex (8 floats: pos+normal+uv), dedup, error "Too many vertices" |
| 0x46C970 | Texture_SetDimensions | Set texture size, compute UV scale factors |

### Game Object Factories (NEW)
| Address | Function | Notes |
|---------|----------|-------|
| 0x418760 | Scene_CreateObject_Gear | Create GEAR/BigGear object (0x1514 bytes) |
| 0x418930 | Gear_AdvanceAlongPath | 8-direction gradient descent path following |

### Game Object Factories — Extended (Session 17)
| Address | Function | Notes |
|---------|----------|-------|
| 0x4143d0 | CreateSpinny | Factory: matches "SPINNY" → Rotator_ctor (0x1508 bytes) |
| 0x414a20 | CreateLifter | Factory: matches "LIFTER" → Lifter_ctor (0x436920, 0x10f4 bytes), falls through to CreatePlatformOrStands |
| 0x415460 | CreateWobbly1 | Factory: matches "WOBBLY1" → GameLevel_ctor (0x1524 bytes), falls through to CreatePlatformOrStands |
| 0x436920 | Lifter_ctor | Lifter object constructor (0x10f4 bytes) |
| 0x413fc0 | RumbleBoard_RenderMultiView | Render 4 dynamic objects with zoom step adjustment |
| 0x4151e0 | RumbleBoard_RenderMultiView5 | Render 5 dynamic objects with zoom step adjustment |
| 0x4155d0 | RumbleBoard_InitScene4 | Init scene objects for 4-player split, assign materials + Level_SetObjectTransform |
| 0x415d90 | RumbleBoard_InitScene5 | Init scene objects for 5-player split, assign materials + Level_SetObjectTransform |

### String/Utility Functions (NEW)
| Address | Function | Notes |
|---------|----------|-------|
| 0x463790 | Vec3_CrossProduct | Cross product: this × param2 → param1 |
| 0x467750 | Array_CopyDWords | Copy N dwords ptr-to-ptr |
| 0x470650 | Array_FillDWords | Fill N dwords with same value |
| 0x472DA0 | Transform_ctor | Transform object constructor |
| 0x4730C0 | Registry_ReadFloat | Read float value from Windows registry |
| 0x473740 | StdString_AppendCharN | Append N copies of a char |
| 0x4738B0 | StdString_AppendN | Append N chars with strncat |
| 0x473B10 | AthenaString_AssignFormatted | Format and assign AthenaString |

### Rendering Pipeline — Extended (Session 16)
| Address | Function | Notes |
|---------|----------|-------|
| 0x456150 | Ray_SetDirection | Set ray direction, normalize, compute length |
| 0x456E80 | Font_WordWrap | Word-wrap text to fit pixel width |
| 0x459610 | Scene_RenderIfVisible | Render scene object only if visible flag set |
| 0x466AC0 | Scene_UpdateChildren | Traverse scene tree, update children |
| 0x4602F0 | Scene_CollectByNameFilter | Collect scene objects by name string filter |
| 0x440DD0 | Graphics_DrawRectAndReset | Draw rect then reset matrix to identity |

### Scene/Board System (NEW)
| Address | Function | Notes |
|---------|----------|-------|
| 0x429520 | Game_SetInProgress | Set game in-progress flag (+0x200=1) |
| 0x44BE80 | ScoreObject_ctor | Score display constructor (vtable 0x4D6C70, score=200000) |
| 0x446B80 | RegisterDialog_ValidateSerial | Validate serial via XOR cipher key "54138" |

### Level Rendering — Extended (NEW)
| Address | Function | Notes |
|---------|----------|-------|
| 0x40B090 | Level_InitScene | Init scene (projection, fog, find CAMERALOCUS) |
| 0x40B420 | Level_RenderDynamicObjects | Render moving objects via Timer positions |
| 0x40B570 | Level_RenderObjects | Iterate objects calling vtable+0x0C |
| 0x40B600 | Level_UpdateAndRender | Full update: merge lists, pre-render, shadow, cleanup |
| 0x40B9C0 | Level_SetObjectTransform | Set world transform from position |
| 0x40ACA0 | Level_SelectCameraProfile | Select camera by difficulty (4-15) |

### App Object Structure (partial, at 0x4FD680)
```
+0x00:  vtable pointer
+0x04:  hInstance (Windows HINSTANCE)
+0x08:  cmdShow (nCmdShow)
+0x54:  registryKey (Config* pointer)
+0x5A:  frameTimeMs (frame delta)
+0x5B:  fpsDenominator
+0x5C:  targetFPS
+0x5D:  renderTarget (render target ptr)
+0x65:  frameCounter (per-second count)
+0x84:  profilingSection (char* current section name)
+0x15A:  someFlag
+0x15C:  width (window width)
+0x158:  windowed (windowed mode flag)
+0x159:  quitFlag (1 = game should quit)
+0x160:  height (window height)
+0x174:  Graphics* pointer (D3D device wrapper)
+0x17C:  audioSystem pointer (BASS audio)
+0x180:  Input* pointer (DirectInput8 subsystem, 0x438 bytes)
+0x1B4:  versionString
+0x1CC:  loadedCount (objects counter)
+0x200:  initialized flag
+0x208:  char[256] debug_string (current init step name)
+0x240:  HCURSOR blank_cursor
+0x278:  void* shadow_texture
+0x534:  MusicPlayer* music handle
+0x538:  int music_channel2
+0x53C:  int music_channel1
+0x550:  gameMode1 (mode value 1)
+0x554:  gameMode2 (mode value 2)
+0x558:  gameMode3 (mode value 4)
+0x55C:  gameMode4 (mode value 5)
+0x914:  int play_count (shareware trial)
```

### App::Initialize Sequence (0x429530) — 26 Steps
Steps 1-12 are in App_Initialize (0x46BB40):
1. vtable+0x94: Parse command line
2. Registry/config init (0x472F50)
3. vtable+0x0C: Window creation (AthenaWindow class)
4. vtable+0x1C: Subsystem init
5. vtable+0x18: Subsystem init
6. vtable+0x30: Subsystem init
7. vtable+0x3C: Subsystem init
8. vtable+0x34: Subsystem init
9. vtable+0x38: Subsystem init
10. vtable+0x40: Subsystem init
11. Graphics_Initialize (Direct3DCreate8 + device creation)
12. Version string extraction

Steps 13-26 are in App_Initialize_Full (0x429530):
13. Set graphics->initialized = true
14. Load "BLANKCURSOR"
15. Set display mode 800x600
16. Configure D3D render states
17. N/A (skipped)
18. Load "shadow.png" texture
19. Load "music\music.mo3"
20. Load "jukebox.xml"
21. Set up music channel 1
22. Set up music channel 2
23. Config::Load + read "PlayCount"
24. Set initialized flag
25-28. Create 4 game mode objects (values 1,2,4,5)
29. Config save
30. vtable+0xA0: final init callback

## Session Progress (From Context Compaction)

| 0x478680 | Texture_dtor | 4 | Destroy texture: free sub-textures, pixel buffers, palette, alpha, vtable release |
| 0x46DB10 | App_Shutdown | 4 | Application cleanup: destroy window, release 5 COM objects, CoUninitialize, free string buffer |
| 0x473640 | AthenaString_Find | 4 | strstr wrapper: returns offset of substring or -1 |
| 0x473600 | AthenaString_Length | 4 | Recalculate and cache string length, clear dirty flag at +0x10 |
| 0x469B20 | UIWidget_HitTest | 3 | Hit-test UI widget list against (x,y) point |
| 0x477670 | Vec3_ClosestPointOnLine | 3 | Project point onto line segment, clamp t to [0,lineLength] |
| 0x475DC0 | CRC32_Compute | 3 | CRC32 using 256-entry lookup table at DAT_004F7534 |
| 0x472990 | Gadget_LabelCtor | 8 | Gadget_Label constructor: vtable 0x4D9E68, name at +0x878, value at +0x87C |
| 0xA00000 | **Key Vtable Addresses** | | |
| 0x4DA65C | D3DX_RegistryGetter_vtable | | DirectX registry path struct vtable |
| 0x4E998C | AthenaList_ctor_vtable | | AthenaList constructor vtable |
| 0x4D9750 | App_Shutdown_vtable | | App shutdown vtable |

### Ball Physics Constants (CONFIRMED from Ghidra decompilation)
- Ball radius = 35.0 at offset +0x284
- Position: +0x164/168/16C (XYZ), Velocity: +0x170/174/178 (XYZ)
- First-frame force modifier at +0x2F0 (1.0→0.25), Ice at +0xC5C, Dizzy at +0xC4C
- Gravity plane at +0x748 (0=XY, 1=tilted, 2=XZ)
- Ball vtable entry +0x04 is dispatcher at 0x405100 that sets defaults: +0x278=0.5, +0x27C=0.2, +0x188=6.0, +0xC6C=600.0, +0xC70=1200.0

### Level::BinaryLoader (0x4629E0) — Confirmed Binary MESHWORLD Format
Opens file with _open(path, 0x8000), reads: material_count → materials (with extended/texture data) → mesh_buffers → game_objects → bounding_box → vertex_array. See SKILL.md for full format spec.

### MESH Parser Status
- v1 (Sphere, 8Ball, Hamster, FunBall): FULLY WORKING
- v2-v5: Partial (finds texture, may miss sub-object vertices)

### OpenGL Level Viewer v2
- Renders object markers from MESHWORLD levels in 3D fly-through
- 86 levels load OK, Level1 has 140+ objects
- Build: gcc -o build/level_viewer_v2 src/level/mesh_parser.c src/level/level_viewer_v2.c -I include -lm $(pkg-config --cflags sdl2 SDL2_image gl glew) $(pkg-config --libs sdl2 SDL2_image gl glew) -lGLU

### TODO Next Steps
- Parse MESHWORLD vertex/face geometry for real mesh rendering
- MESH v2-v5 sub-object support
- Collision plane system RE (Ball_CheckCollisionPlanes 0x402810) — partially done: spatial tree + AABB test documented
- Connect GhidraMCP for interactive decompilation — DONE, working
- Rendering pipeline RE — substantially advanced (ctor, dtor, DrawScreenRect, ApplyMaterialAndDraw, SetRenderMode documented)
- Ball physics gravity + trajectory system documented (SetTiltedGravity, SetFlatGravity, SetTrajectory, ApplyTrajectory)

## Scene Vtable (0x4D0260) — 36 Entries (144 bytes)

| Index | Address | Function | Description |
|-------|---------|----------|-------------|
| 0 | 0x419770 | Scene_DeletingDtor | Destructor (scalar deleting) |
| 1 | 0x419C00 | Scene_Update | Main update tick |
| 2 | 0x41A2E0 | Scene_Render | Main render tick |
| 3 | 0x41C620 | Scene_HandleInput | Input dispatch (menu navigation, ball control) |
| 4 | 0x41C7D0 | Scene_ActivateCurrentItem | Activate selected menu item |
| 6 | 0x41CA00 | Scene_SelectCurrentItem | Select/highlight menu item |
| 7 | 0x409D90 | (3-byte nop stub) | Unused vtable slot |
| 11 | 0x41CB60 | Scene_ClearCurrentItem[1] | Clear item selection variant 1 |
| 12 | 0x41CBF0 | Scene_ClearCurrentItem[2] | Clear item selection variant 2 |
| 14 | 0x419900 | Scene_DestroyScene | Destroy scene + cleanup |
| 15 | 0x419A40 | Scene_NotifyObjects | Notify all scene objects of event |
| 16 | 0x419970 | Scene_SetDestroyed | Set destroyed flag (+0x2C) |
| 17 | 0x4198E0 | Scene_SaveAndCleanup | Save state + cleanup |
| 19 | 0x419E20 | Scene_HandleRaceEnd | Handle race completion |
| 20 | 0x41A180 | Scene_UpdateBallsAndState | Update ball physics + race state |
| 22 | 0x41A050 | Scene_HandleCountdown | 3-2-1-GO countdown logic |
| 23 | 0x419E80 | Scene_HandleBallFinish | Ball finish state machine (5 states) |
| 27 | 0x41A560 | Scene_RenderScoreHUD | Render score HUD overlay |
| 28 | 0x41A680 | Scene_RenderTimerHUD | Render timer HUD overlay |
| 32 | 0x41C5B0 | Scene_SpawnBallsAndObjects | Create game objects (balls, traps, secrets, flags) |
| 35 | 0x41A9A0 | Scene_ComputeLighting | Find closest light, compute direction vector |

### Scene_Update (0x419C00) — Main Game Tick (10 phases)

| Phase | Description | Key Offsets |
|-------|-------------|-------------|
| 1 | Frame counter++ | this+0xD88 (= 0x3620) |
| 2 | Demo timer countdown | this+0x10D6 (active), +0x10D7 (frames left), +0x10D8 (elapsed), +0x10D9 (counter) |
| 3 | ESC check → Scene_CreateGameOverMenu | this+0x10DA (ignore ESC), App+0x5FC (input mode) |
| 4 | Ball position propagation | this+0xA6C (flag), iterate +0xA75 ball list |
| 5 | Gear path follow (single player) | this+0xFC7 (gear enabled), +0xFC8 (path data) |
| 6 | RumbleBoard timer ticks | this+0x221, this+0x226 (two RumbleBoard timers) |
| 7 | Camera shake decay | this+0xE93 (shake active), +0xA6E (magnitude, ±800, decay -10/frame) |
| 8 | Scene object update+render | iterate this+0x22E, vtable[4]=Update, vtable[0]=Render |
| 9 | Physics pipeline | vtable[0x4C/0x50/0x54/0x58] + physics objects at this+0xD8B |
| 10 | Post-physics callback | this+0xEBF+0x04() |

### Scene_HandleBallFinish State Machine (5 states)
```
state 0: start → set counter=150.0f, advance to state 1
state 1: countdown → decrement counter by dt, when ≤0 advance to state 2
state 2: finish → save race time, play "Goal!" music, advance to state 3
state 3: popup → show finish popup, advance to state 4
state 4: done → wait for popup dismissal
```

### Scene Key Offsets
- +0x2C: destroyed flag (set by Scene_SetDestroyed)
- +0x864: current item pointer (menu selection)
- App+0x10EC: race active flag (1=active, set by Scene_SetRaceActive at 0x4366E0, 62 xrefs)

## SceneObject Vtable (0x4D934C) — 10 Entries

| Index | Address | Function | Description |
|-------|---------|----------|-------------|
| 1 | 0x46B560 | SceneObject_SetPosition | Set world position (XYZ) |
| 2 | 0x46B5A0 | SceneObject_SetScale | Set scale vector |
| 3 | 0x46B4B0 | SceneObject_Render | Render scene object |
| 4 | 0x46B4D0 | SceneObject_SetVisible | Toggle visibility flag |
| 7 | 0x46B860 | SceneObject_BaseDtor | Base dtor: iterates child list, calls each child's dtor(1), clears list |
| 8 | 0x46B650 | SceneObject_DeletingDtor | Scalar deleting destructor |

## Tournament & Level System (NEW — Session 2026-04-13)

### Tournament_AdvanceRace (0x427080) — CORRECTED from decompilation
15-case switch creates specific Board constructors. Key discovery: CreateSawblade (0x40E250) is the MASTER ARENA OBJECT FACTORY — creates ALL arena hazard types (Sawblade, TowerLevel, Spinner, Gear/Judge, Tipper/Bell, Bonk/Hammer) by name prefix matching.

| Case | Level | Board Constructor | Size |
|------|-------|-------------------|------|
| 1 | Warm-Up | BoardLevel1_WarmUp_ctor | 0x436C |
| 2 | Beginner | BoardLevel_Beginner_Ctor | 0x644C |
| 3 | Intermediate | BoardLevel2_Intermediate_ctor | 0x438C |
| 4 | Dizzy | BoardLevel3_ctor | 0x4BE0 |
| 5 | Tower | BoardLevel5_Tower_ctor | 0x5418 |
| 6 | Up | BoardLevel_Up_Ctor | 0x4790 |
| 7 | Neon Race | Board_NeonRace_ctor | 0x4394 |
| 8 | Expert | BoardLevel8_Expert_ctor | 0x4FD8 |
| 9 | Odd | BoardLevel9_Odd_ctor | 0x43B0 |
| 10 | Toob Race | BoardLevel_Toob_Ctor | 0x646C |
| 11 | Wobbly | BoardLevel12_Wobbly_ctor | 0x4388 |
| 12 | Glass | Board_Glass_ctor | 0x4390 |
| 13 | Sky | BoardLevel_Sky_Ctor | 0x47F8 |
| 14 | Master | BoardLevel_Master_Ctor | 0x6498 |
| 15 | Impossible | Board_Impossible_ctor | 0x4380 |

Also saves score, computes difficulty time bonus (+1000ms normal, +500ms frenzied), stores race timestamps. When param_1=true, creates TourneyMenu instead (retry current race).

### PracticeMenu_ctor (0x42EA30)
"Practice Menu" scene with 14 race items and 14 thumbnail textures (practice-level1..practice-impossible.png). Lock check via App+0x851-0x865 boolean flags.

### Level Unlock Flags (App offsets)
| Offset | Level |
|--------|-------|
| +0x851 | Level 1 (Warm-Up) |
| +0x852 | Level 2 (Beginner) |
| +0x853 | Level 3 (Intermediate) |
| +0x854 | Level 4 (Dizzy) |
| +0x855 | Level 5 (Tower) |
| +0x856 | Level 6 (King of the Hill) |
| +0x857 | Level 7 (Up) |
| +0x858 | Level 8 (Expert) |
| +0x859 | Level 9 (Odd) |
| +0x85A | Level 10 (Toob Race) |
| +0x85B | Level 11 (Sky) |
| +0x85C | Level 12 (Wobbly) |
| +0x85D | Level 13 (Master) |
| +0x85E | Level 14 (Race of Ages) |
| +0x85F | Level 15 (Impossible) |
| +0x860-0x865 | Arena unlock flags |

### Scene_SpawnBallsAndObjects (vtable[32], 0x41C5B0)
Creates GameObject instances (0xC60 bytes) for each ball, sets start positions via START%d-%d naming, scans SAFESPOT/SAFEPOS objects, creates BadBall/MouseTrap/SecretObjects/Flags/Signs/DynamicObjects.

### App_ShowResults (0x428060)
Creates results screen scene (0x87C bytes). Called after tournament race completion.

### Difficulty_GetTimeModifier (0x428ED0)
Returns time bonus multiplier based on difficulty level.

## RumbleBoard Arena Init Functions (14 total — CORRECTED)

| Arena | Init Function | Level Path | Special Logic |
|-------|---------------|------------|---------------|
| WarmUp | RumbleBoard_WarmUp_Init (0x413C20) | levels\arena-WarmUp | — |
| Beginner | RumbleBoard_Beginner_Init (0x414180) | levels\arena-intermediate | — |
| Intermediate | RumbleBoard_Intermediate_Init (0x414180) | levels\arena-Intermediate | — |
| Dizzy | RumbleBoard_Dizzy_Init (0x414240) | levels\arena-dizzy | Loads bonus level Levels\Level3-Swirl |
| Tower | RumbleBoard_Tower_Init | levels\arena-Tower | — |
| Up | RumbleBoard_Up_Init | levels\arena-Up | — |
| Expert | RumbleBoard_Expert_Init (0x414B10) | levels\arena-expert | — |
| Odd | RumbleBoard_Odd_Init (0x414CE0) | levels\arena-Odd | — |
| Sky | RumbleBoard_Sky_Init (0x4158C0) | levels\arena-Sky | "PILLAR" object collection |
| Neon | RumbleBoard_Neon_Init (0x416F40) | levels\arena-neon | 2x AthenaList transforms + boundary sphere |
| Glass | RumbleBoard_Glass_Init (0x417DF0) | levels\arena-Glass | — |
| Master | RumbleBoard_Master_Init (0x416080) | levels\arena-Master | — |
| Race of Ages | (Cascade) | levels\arena-Cascade | — |
| Impossible | RumbleBoard_Impossible_Init (0x418540) | levels\arena-impossible | — |

### Scene Vtable Discovery Technique
Scene vtable at 0x4D0260 has 36 entries (144 bytes). Read memory at the vtable address, decode as 36 little-endian 32-bit pointers. Some entries may be 3-byte nop stubs (0x409D90) — these are unused slots. Plate comments can ONLY be set at function addresses, not data addresses (failed at 0x4D0260 and 0x4D934C).

### Workflow: High-Xref FUN_* Function Identification
1. Use `search_functions_enhanced` with `has_custom_name=false`, `min_xrefs=15`, `sort_by=xrefs_desc` to find undocumented functions with many callers
2. Batch decompile the top candidates (4-6 at a time via individual decompile_function calls — batch_decompile often fails with "Function not found")
3. Identify patterns: AthenaString_*, Scene_*, Graphics_*, MWParser_*, Vec3*, etc.
4. Cross-reference callers via `get_function_callers` or `get_xrefs_to` on data items to understand usage context
5. Rename with descriptive Module_Action format, set plate comments
6. Key data items can also be labeled: `list_data_items_by_xrefs` reveals high-value globals like `g_renderIndex`, string constants like `s_BACK`

### New Functions (Session 2026-04-13 Batch 4 — Menu System, Scene/Texture, Game Flow)

| Address | Name | Xrefs | Description |
|---------|------|-------|-------------|
| 0x42f810 | TimeTrialMenu_ctor | 3 | "Time Trial Menu" — extends PracticeMenu with race items + lock checks |
| 0x42fc10 | PartyMenu_ctor | 3 | "CHOOSE A PARTY RACE!" — extends PracticeMenu, vtable 0x4D4738 |
| 0x42fc40 | ArenaMenu_ctor | 3 | Arena menu with 14 arena items (Warm-Up to Impossible), lock icons, vtable 0x4D47B8 |
| 0x4326d0 | MPMenu_ctor | 3 | Multiplayer menu: Party Race, Rodent Rumble, controller config (1-4P) |
| 0x44aa40 | Scene_FindTextureByName | 4 | Find texture by case-insensitive name, return ptr+dimensions+width |
| 0x44ab00 | Scene_FindTextureDimensions | 4 | Find texture and measure text width via Font_MeasureText |
| 0x44abf0 | Scene_AddTextureToList | 4 | Add texture reference to scene list by name, mark dirty at +0xCBC |
| 0x443ac0 | SceneObject_RenderScaled | 3 | Render object scaled (ScaleX, SetPosition, vtable callbacks + Timer) |
| 0x4410c0 | SceneObject_FreeStrings | 3 | Free 2 string ptrs, re-init BaseObject, call SceneObject_dtor |
| 0x453c50 | Texture_RemoveRef | 3 | Decrement texture refcount, remove from cache and free when 0 |
| 0x457a50 | Graphics_DisableRenderState | 3 | Thunk → Graphics_SetRenderState (disable mode) |
| 0x428c50 | App_StartPracticeRace | 3 | Start practice/tournament race: calls App_StartRace, PlayerProfile, Tournament_AdvanceRace |
| 0x434580 | Sound_InitChannels | 3 | Allocate sound channels, get next sample, play 3D positioned, set timer 0x140 |
| 0x43b6f0 | ScoreObject_SetScore | 3 | Find score by ID and set to 10, or create new entry with value 10 |
| 0x44bef0 | Timer_Decrement | 4 | Timer tick: value = end - 100, set flag at +0x2A |
| 0x448620 | ScoreDisplay_DeletingDtor | 3 | ScoreDisplay scalar deleting destructor |
| 0x4470d0 | ScoreDisplay_dtor | 3 | Clean up: free strings, timers, 5 BaseObjects, SceneObject_dtor |
| 0x44acb0 | UIList_Clear | 4 | Empty stub (returns 0) |

### Menu System — Arena Unlocks (from ArenaMenu_ctor at 0x42fc40)
App offsets for arena unlock flags (0-based, boolean):
+0x852=Level 2 (Beginner), +0x853=Intermediate, +0x85A=Dizzy, +0x85B=Tower, +0x85C=Up, +0x85D=Expert, +0x85E=Odd, +0x85F=Toob, +0x860=Wobbly, +0x867=Glass, +0x861=Sky, +0x862=Master, +0x868=Impossible

### App_StartPracticeRace (0x428c50) — Key Findings
- Calls App_StartRace first, then resolves scene objects by checking priority values (field+0x525 offsets)
- Sets mirror mode culling: App+0x7D2 (0=normal, 1=mirror), calls Gfx_SetCullMode
- Adjusts viewport dimensions from App+0x27C/280/284 and +0x288/28C/0x290
- Creates 0x528-byte scene object at App+0x90C (Tournament scene manager?)
- Creates PlayerProfile (0x98 bytes) at App+0x220, copies level name from App+0x29B4
- Flags set: App+0x717=1, App+0x7B7=1, App+0x5D7=0, App+0x677 (0 unless mirror)
- When mirror: App+0x677=0, +0x23C=1
- Calls Tournament_AdvanceRace(param_1, 0)

### Batch Workflow Efficiency Notes
- The batch_decompile tool often fails with "Function not found" — prefer individual decompile_function calls
- For menu constructors: string references ("Time Trial Menu", "CHOOSE A PARTY RACE!", etc.) are the most reliable identifiers
- Arena lock flags at App+0x851-0x868 were confirmed from ArenaMenu_ctor decompilation
- Scene texture functions (FindTextureByName, FindTextureDimensions, AddTextureToList) share a common pattern: iterate AthenaList at +0x890 with index tracking at +0x894

| Address | Name | Xrefs | Description |
|---------|------|-------|-------------|
| 0x466C70 | AthenaString_Format | 98 | sprintf wrapper returning internal buffer |
| 0x4BBDFD | AthenaString_Sprintf | - | Internal sprintf using FILE struct trick |
| 0x469990 | Scene_AddObject | 77 | Add SceneObject to scene (uniqueness check, vtable notify) |
| 0x46F390 | Scene_BeginFrame | 39 | Begin rendering frame (Graphics_BeginFrame + vtable callback) |
| 0x460DA0 | Scene_RenderFrame | 38 | Full scene render pipeline with z-buffer interleaving |
| 0x461370 | Scene_RenderOpaque | 38 | Render opaque pass (object vtable[0x28] + mesh buffers) |
| 0x461890 | Scene_LoadMeshWorld | 38 | Load .meshworld from stream (materials, textures, objects) |
| 0x461F00 | Scene_Subdivide | 38 | Subdivide scene space into regular 3D grid |
| 0x462100 | Scene_SubdivideRandom | 38 | Subdivide scene with random grid positions |
| 0x4629E0 | Scene_LoadCached | 37 | Load .cached scene file (binary format) |
| 0x46A750 | Window_Notify | 37 | Send WM_COPYDATA message to window |
| 0x498200 | BitStream_ReadBits | 95 | Read N bits from byte-aligned stream |
| 0x4792FB | D3DX_DetectShaderProfile | 48 | Detect pixel shader version (1.x/2.x/3.x) |
| 0x492BDA | Vertex_Transform | 32 | Transform vertices between coordinate spaces |
| 0x4737F0 | AthenaString_Assign | 52 | String copy/assign operator |
| 0x4605E0 | AthenaHashTable_Lookup | 36 | Case-insensitive hash table lookup |
| 0x4691C0 | SceneObject_dtor | 31 | SceneObject destructor |
| 0x4693C0 | Scene_AddAllObjects | 25 | Batch add all SceneObjects |
| 0x469600 | MWParser_ReadTag | 24 | XML/SGML tag parser for MW files |
| 0x4695D0 | StreamReader_dtor | 24 | Close file handle, free buffer |
| 0x45D450 | Sprite_DrawColoredRect | 23 | Draw colored quad with random vertex colors |
| 0x472AF0 | AthenaString_Init | 18 | Default string constructor |
| 0x472F30 | RegKey_Close | 18 | RegCloseKey wrapper |
| 0x473670 | AthenaString_CopyCtor | 16 | String copy constructor |
| 0x4740D0 | AthenaString_WriteTag | 16 | Build XML tag <tag>content</tag> |
| 0x489610 | Pool_FreeList | 16 | Walk free list, decrement refcounts |
| 0x459B24 | Graphics_InitShaderDispatch | 19 | D3DX shader init dispatch thunk |
| 0x45A439 | Graphics_SetRenderState | 29 | Render state dispatch thunk |
| 0x4AB9B8 | DivCeil | 15 | Ceiling division (a-1+b)/b |
| 0x4D2334 | s_BACK | 15 | "BACK" string constant |
| 0x5341CC | g_renderIndex | - | Global render index counter |

### Documentation Status (2026-04-13 Session 20)
- Total functions: 3,811
- Documented: 2,291 (60.1%)
- User-labeled: 400+ functions
- Progress from baseline: +121 functions documented (was 42.8% at start)
- Session 20 batch: BitStream_ReadBitsSSE2, Matrix_Inverse4x4_SSE2, SSE2_SetFPControlWord, Mem_Zero, Malloc_OrLongjmp, StrCat_Fast, BitStream_CopyToOutput, Noop2, ReturnZero, D3DX_ShaderDispatch0/1/2, AthenaList_Ctor, Audio_StopChannel, App_Shutdown, NetworkConnection_Ctor, Font_RenderToTextureComplex, Gadget_LabelCtor, UIWidget_HitTest, SceneObject_EmptyListCtor, AthenaString_Reserve, AthenaString_Length, AthenaString_Find, D3DX_RegistryGetter, CRC32_Compute, Vec3_ClosestPointOnLine, Texture_StreamRead, Texture_BinarySearch, Texture_LoadFromStream, Texture_dtor, Texture_SetPool, Pool_dtor, Texture_ValidateOrBuild, Mesh_dtor, Pool_Free, Texture_ComputeChecksum, Texture_DestroyBuffers, Mesh_Init, Pool_Alloc, VertexDeclaration_dtor, VertexShader_dtor, Gfx_ResizeBuffers
- Batch workflow: see `references/batch-doc-workflow.md` for efficient decompile→identify→rename→comment→commit cycle

### Session 2026-04-13 Key Findings
- Ball_ApplyForceV2 (0x4016f0): Second force application variant with gravity plane awareness
- Ball_RenderWithCollision (0x402c10): Full collision check + shadow + render pipeline
- Ball_SetupCollisionRender (0x4015b0): Initialize collision mesh render objects from level data
- App_ShowMainMenu (0x4280e0): Creates MainMenu (0xCDC bytes), stores at App+0x224
- App_StartTournamentRace (0x4288b0): Configures mirror mode, creates tournament scene
- RegisterDialog: Complete registration system with serial validation, XOR cipher (key "54138"), writes DXCaps to registry
- Sound_LoadOggOrWav (0x459660): Sound loader with .ogg/.wav fallback pattern
- Sound_GetNextChannel (0x459810): Circular buffer channel allocator
- Graphics_Initialize: Full 27-step D3D8 init sequence documented (adapter enumeration, mode selection, dual device creation, VB setup)
- Graphics object offsets confirmed: +0x7c=D3D8, +0x154=VertexBuffer, +0x164=PrimaryPP, +0x174=SelectedMode, +0x7d3-7d6=ResolutionAvailable flags
- App_StartTournamentRace reveals App offsets: +0x237=flag, +0x717=flag, +0x7b7=flag, +0x5d7=flag, +0x677=flag, +0x236=mirror_mode, +0x27c/280/284=viewport, +0x7d2=mirror_cull_flag

### Session 20 — SSE2/Math/CRT/Texture Functions (NEW)

| Address | Name | Xrefs | Description |
|---------|------|-------|-------------|
| 0x4B22AB | BitStream_ReadBitsSSE2 | 19 | SSE2 bitstream reader: reads N bits in 8-bit chunks, 0xFF marker handling, maintains stream state |
| 0x4A0F3A | Matrix_Inverse4x4_SSE2 | 9 | SSE2 4x4 matrix inverse via cofactor expansion + Newton-Raphson reciprocal. Returns NULL if singular |
| 0x4A6B80 | SSE2_SetFPControlWord | 9 | Store FPU control word to global DAT_00535280 |
| 0x4ABA49 | Mem_Zero | 8 | Optimized memset-to-zero: dwords first (count>>2), then bytes (count&3) |
| 0x4AD576 | Malloc_OrLongjmp | 9 | Safe malloc: calls malloc, longjmps with "Out of Memory" on failure. Returns NULL if ptr/size==0 |
| 0x4BC360 | StrCat_Fast | 9 | Optimized strcat: dword-aligned null detection (0x7efefeff trick), fast copy |
| 0x4B8564 | BitStream_CopyToOutput | 8 | Copy data from bitstream to output buffer with checksum callback. Double-buffered streaming |
| 0x4C183E | Noop2 | 8 | Empty no-op function |
| 0x4C7152 | ReturnZero | 8 | Returns 0 (stub/trap) |
| 0x459D96 | D3DX_ShaderDispatch0 | 3 | D3DX shader dispatch via PTR_FUN_004F7194 vtable |
| 0x459E34 | D3DX_ShaderDispatch1 | 4 | D3DX shader dispatch via PTR_FUN_004F71B8 vtable |
| 0x459ED1 | D3DX_ShaderDispatch2 | 4 | D3DX shader dispatch via PTR_FUN_004F71CC vtable |
| 0x467E40 | AthenaList_Ctor | 3 | Init AthenaList with vtable 0x4E998C, StdString_Substr copy |
| 0x46A0D0 | Audio_StopChannel | 9 | Stop BASS audio channel (BASS_ChannelStop on handle at +0x08) |
| 0x46DB10 | App_Shutdown | 4 | Application cleanup: destroy window, release 5 COM objects, CoUninitialize, free string buffer |
| 0x46DFA0 | NetworkConnection_Ctor | 4 | Init network connection: "Not Connected", id at +0x04, +0x0C=1.0f |
| 0x472340 | Font_RenderToTextureComplex | 4 | Complex text→texture rendering using D3D vertex buffers and shaders |
| 0x472990 | Gadget_LabelCtor | 8 | Gadget_Label constructor: vtable 0x4D9E68, name string at +0x878, value at +0x87C |
| 0x469B20 | UIWidget_HitTest | 3 | Hit-test UI widget list against (x,y) point. Checks +0x420 rect first, else iterates children by bounds |
| 0x46B840 | SceneObject_EmptyListCtor | 3 | Init SceneObject with DeletingDtor vtable 0x4D9368 + empty AthenaList |
| 0x473480 | AthenaString_Reserve | 4 | Reserve string capacity: alloc new buffer, copy old, free old |
| 0x473600 | AthenaString_Length | 4 | Recalculate and cache string length, clear dirty flag at +0x10 |
| 0x473640 | AthenaString_Find | 4 | strstr wrapper: returns offset of substring or -1 |
| 0x477010 | D3DX_RegistryGetter | 4 | Set vtable 0x4DA65C (DirectX registry path struct) |
| 0x475DC0 | CRC32_Compute | 3 | CRC32 using 256-entry lookup table at DAT_004F7534 |
| 0x477670 | Vec3_ClosestPointOnLine | 3 | Project point onto line segment. Clamp t to [0,lineLength] |
| 0x477970 | Texture_StreamRead | 3 | Stream read from bitstream with 0x400-byte blocks and progress callback |
| 0x477AC0 | Texture_BinarySearch | 4 | Binary search in texture archive with checksum and callback |
| 0x477D60 | Texture_LoadFromStream | 4 | Texture loading from stream: init pool, loop with D3DX_Uninit/retry, up to 3 attempts |
| 0x478680 | Texture_dtor | 4 | Destroy texture: free sub-textures, pixel buffers, palette, alpha, vtable release |
| 0x48A160 | Texture_SetPool | 6 | Set texture pool reference (param_2), clear struct |
| 0x48A180 | Pool_dtor | 6 | Free pool list, zero 8 dwords |
| 0x48A560 | Texture_ValidateOrBuild | 5 | Validate/assemble texture from stream chunks, checksum mismatch → -0xF3 |
| 0x48A900 | Mesh_dtor | 6 | Destroy mesh: free D3D vertex buffers, release texture refs, free allocations |
| 0x489DF0 | Pool_Free | 4 | Free pool entries, zero 0x14 dwords |
| 0x489E20 | Texture_ComputeChecksum | 4 | Compute texture checksum from stream bytes (0xFF skip marker, accumulate sum) |
| 0x48A860 | Texture_DestroyBuffers | 5 | Free D3D index/vertex buffers, free allocation ptrs |
| 0x48A8D0 | Mesh_Init | 6 | Zero mesh struct, calloc 0xCA8 bytes for mesh data |
| 0x48B1C0 | Pool_Alloc | 6 | Pool allocator: allocate aligned block, chain old block, return offset |
| 0x48B2A0 | VertexDeclaration_dtor | 3 | Free vertex declaration, release D3D decl, zero struct |
| 0x48B530 | VertexShader_dtor | 3 | Free vertex shader: release D3D shader, free texture/palette/alpha buffers |
| 0x480C4D | Gfx_ResizeBuffers | 5 | D3D texture/surface buffer resize with format detection (D3DFMT checking) |

### High-Xref Utility Functions

| Address | Name | Xrefs | Description |
|---------|------|-------|-------------|
| 0x4BA754 | __ftol2 | 358 | CRT float-to-int64 conversion (compiler intrinsic) — MOST CALLED FUNCTION |
| 0x45DD60 | RNG_Rand | 193 | PRNG with 55-entry circular buf (Mitchell & Moore). Returns (buf[read]+buf[write])>>6 % range. Signed mode: param_2=1 negates 50% of results |
| 0x453250 | Vec3List_Free | 283 | Free Vec3List + member data at +0x103 |
| 0x44B840 | Noop | 280 | Empty stub (vtable placeholder) |
| 0x4736B0 | AthenaString_dtor | 85 | AthenaString destructor — frees buffer, sets vtable to base dtor (0x4D290C) |
| 0x473500 | AthenaString_AssignCStr | 75 | AthenaString assign from C string |
| 0x536A0 | AthenaList_GetSize | 60 | Return count at +4 |
| 0x453640 | AthenaList_FindByValue | - | Linear search, returns index or -1 |
| 0x443990 | AthenaString_Clear | 14 | Free buf, reset to 15-char inline capacity |
| 0x4531E0 | Vec3_Init | 13 | Set Vec3 vtable + zero + w=255.0 |
| 0x4BC0D1 | strtok | 50 | CRT strtok — thread-safe string tokenizer |
| 0x4BAC20 | strstr | 22 | CRT strstr — string search with SIMD optimization |
| 0x4BAE43 | AthenaString_SprintfToBuffer | 71 | sprintf into char buffer via fake FILE struct |

### UI List System (vtable 0x4D6A70, base for ALL menus)

| Address | Name | Xrefs | Description |
|---------|------|-------|-------------|
| 0x448F20 | SimpleMenu_ctor | 15 | "Simple Menu" base with item list + scrollers |
| 0x4490A0 | UIListItem_ctor | - | Init 0x444-byte item (Vec3 + AthenaList) |
| 0x4497F0 | UIList_AddItem | 86 | Add named item (text, subtext, colors, icon, height) |
| 0x449430 | UIList_AddSpacer | 29 | Add empty row with height |
| 0x4494D0 | UIList_ScrollUpdate | 17 | Scroll, mouse wheel, vtable dispatch |
| 0x449B00 | UIList_Cleanup | 27 | Free all items (strings, SceneObjects, Vec3Lists) |
| 0x449C20 | UIList_HandleKeyNav | 18 | Up/down/pgup/pgdn key navigation |
| 0x449D40 | UIList_Render | 18 | Draw items: gradient bars, text, icons, scroll arrows |
| 0x44A570 | UIList_Layout | 18 | Compute widths, position SceneObjects, set scrollers |
| 0x44A8B0 | UIList_SetTextByName | 27 | Find item by subtext, replace display text |
| 0x449750 | UIList_ActivateCurrentItem | 18 | Activate: Back→sound 650, Continue→50, else callback |

UIListItem (0x444 bytes): +0x00=display_text, +0x04=subtext, +0x0C-0x0F=RGBA, +0x1C*4=icon, +0x24*4=height, +0x110=is_icon_row flag, +0x441=highlighted flag

### RumbleBoard System (vtable PTR 0x4D1358, extends Board extends Scene)

| Address | Name | Xrefs | Description |
|---------|------|-------|-------------|
| 0x4217B0 | RumbleBoard_ctor | 15 | Init with "RumbleBoard", timer, base score=6000 |
| 0x421880 | RumbleBoard_dtor | 24 | Cleanup timer, release SceneObjects, Scene_dtor |
| 0x421910 | RumbleBoard_Render | 16 | Timer bar, round ".%d", "TIE BREAKER!" |
| 0x421FE0 | RumbleBoard_Update | 16 | Check round end, resolve ties, "Game Over" music |
| 0x458E60 | RumbleBoard_InitTimer | 12 | Initialize round timer |
| 0x458E80 | RumbleBoard_CleanupTimer | 32 | Cleanup round timer |
| 0x458E90 | RumbleBoard_TickTimer | 12 | Tick countdown |
| 0x44AD50 | RumbleScore_ctor | 12 | Init vtable + difficulty scale [0.02, 0.03, 0.04] |

Key RumbleBoard offsets: +0x47AC=base_score(6000), +0x47C5=is_tie_breaker, +0x47CC=tie_active, +0x47D0=max_rounds(25), +0x11EB=round_end_timer, +0x11F1=game_over_flag, +0x11ED-0x11F0=4 player scores

### Graphics Transform Pipeline (Session 2026-04-13)

| Address | Name | Xrefs | Description |
|---------|------|-------|-------------|
| 0x457B10 | Matrix44_Zero | 12 | Clear 4x4, set diagonals to 1.0 |
| 0x457B50 | Gfx_SetPosition | 69 | D3D SetTransform world translate |
| 0x457BB0 | Gfx_RotateY | 15 | Rotation around Y axis |
| 0x457C60 | Gfx_ScaleX | 40 | Scale X axis |
| 0x457C90 | Gfx_ScaleY | 35 | Scale Y axis |
| 0x457CC0 | Gfx_ScaleZ | 26 | Scale Z axis |
| 0x457FD0 | Matrix4_Identity | 40 | Set identity vtable |
| 0x425FE0 | Gfx_SetAlphaBlendState | 9 | D3DRS_SRCBLEND/DESTBLEND mode 3 |
| 0x427940 | Gfx_SetCullMode | 13 | D3DRS_CULLMODE (none/CW/CCW) |

### Wave Math System

| Address | Name | Xrefs | Description |
|---------|------|-------|-------------|
| 0x457DA0 | Wave_Sin | 38 | sin(time * freq * 2π/360) |
| 0x457DC0 | Wave_Cos | 23 | cos(time * freq * 2π/360) |

### Sprite System (vtable 0x4D8F84)

| Address | Name | Xrefs | Description |
|---------|------|-------|-------------|
| 0x45D0C0 | Sprite_ctor | 30 | Init with texture, RenderContext, material defaults |
| 0x45D660 | Sprite_RenderQuad | 15 | Render textured quad via material + DrawPrimitive |

Sprite (0xD4 bytes): +0x00=vtable, +0x04=gfx, +0x08-0x60=RenderContext, +0x50=texture, +0xC8=width, +0xCC=height, +0xD0=visible(1), +0xD1=flag

### Scene Rendering Extensions (Session 2026-04-13)

| Address | Name | Xrefs | Description |
|---------|------|-------|-------------|
| 0x45E0E0 | Scene_RenderAllObjects | 33 | Main 3D render: BeginFrame → sort (opaque/alpha/shadow) → draw |
| 0x460450 | Scene_RenderBallShadow | 38 | Ball shadow with depth bias pass |
| 0x45DF80 | SceneObject_CallUpdate | 47 | Dispatch +0x434 vtable[1] (Update) |
| 0x45DF90 | SceneObject_CallRender | 47 | Dispatch +0x434 vtable[2] (Render) |
| 0x437130 | Scene_StartCountdown | 11 | Start countdown (3..2..1, 400 or 50 frames) |

Object flags in Scene_RenderAllObjects: +0x85F=shadow, +0x860=alpha, +0x862=deferred, +0x863=skip

### PRNG (RNGState struct)

```c
struct RNGState {
    VTable* vtable;         // +0x00
    int read_ptr;           // +0x04 (wraps at 55)
    int write_ptr;          // +0x08 (wraps at 55)
    uint32_t buffer[55];    // +0x0C..0xE4 (circular buf)
};
// Algorithm: buf[read] = (buf[read] + buf[write]) & 0x3FFFFFFF
// Return: (result >> 6) % range
// Signed: if param_2==1 && Rand(2)==0, negate
```

### Dialog System

| Address | Name | Xrefs | Description |
|---------|------|-------|-------------|
| 0x440E70 | OkayDialog_ctor | 12 | "Okay Dialog" with caption + "OKAY!" button |
| 0x401480 | GameObject_dtor | 9 | Release timers, free Vec3Lists, cleanup matrices |

## Phase 3: System Documentation

After achieving 100% function naming (3781/3781), the project moved to comprehensive
system documentation. Key docs created:

| Doc | Content |
|-----|---------|
| docs/RUMBLEBOARD_SYSTEM.md | 15-race tournament order, all 9 arena asset paths, timer system, HUD layout, menu commands, mirror unlock, time bonus |
| docs/UI_MENU_SYSTEM.md | Menu hierarchy (SimpleMenu/UIList), all dispatch tables (Main/Pause/Difficulty), color system, dialog classes |
| docs/ARENA_HAZARD_SYSTEM.md | 6 hazard types (Sawblade/Tower/Spinner/Gear/Bell/Bonk), CreateSawblade factory, name suffix modifiers, difficulty values |
| docs/LEVEL_OBJECTS.md | Catapult/Trapdoor/ScoreDisplay/HighScore/Damage/JumpPad, RNG (55-element LFSR), Ball trail particles |
| docs/COLLISION_SYSTEM.md | CollisionFace/MeshBuffer structs, .COL binary format, Arena/Level/GameObject event dispatch |

## Quick Analysis Commands

### Using GhidraMCP Headless Server — PRIMARY analysis tool
```bash
# Start the headless server (run once per session):
export GHIDRA_HOME=/opt/ghidra_12.0.4_PUBLIC
MCP_JAR=/home/evan/.config/ghidra/ghidra_12.0.4_PUBLIC/Extensions/GhidraMCP/lib/GhidraMCP-5.2.0.jar
CLASSPATH="$MCP_JAR"
for jar in $GHIDRA_HOME/Ghidra/Framework/*/lib/*.jar; do CLASSPATH="${CLASSPATH}:${jar}"; done
for jar in $GHIDRA_HOME/Ghidra/Features/*/lib/*.jar; do CLASSPATH="${CLASSPATH}:${jar}"; done
for jar in $GHIDRA_HOME/Ghidra/Processors/*/lib/*.jar; do CLASSPATH="${CLASSPATH}:${jar}"; done

java -Xmx4g -XX:+UseG1GC \
    -Dghidra.home=$GHIDRA_HOME -Dapplication.name=GhidraMCP \
    -classpath "$CLASSPATH" \
    com.xebyte.headless.GhidraMCPHeadlessServer \
    --port 8089 --bind 127.0.0.1 &

# Load binary and run analysis:
curl -s -X POST -d "file=/home/evan/hamsterball-re/originals/installed/extracted/Hamsterball.exe" http://127.0.0.1:8089/load_program
curl -s -X POST http://127.0.0.1:8089/run_analysis

# Read operations (GET):
curl -s "http://127.0.0.1:8089/decompile_function?address=0x004278E0"
curl -s "http://127.0.0.1:8089/list_functions?limit=50"
curl -s "http://127.0.0.1:8089/search_strings?search_term=App&limit=10"
curl -s "http://127.0.0.1:8089/get_xrefs_to?address=0x004D9384"
curl -s "http://127.0.0.1:8089/list_imports?limit=200"

# Write operations (POST with JSON body — form-encoded does NOT work):
curl -s -X POST -H "Content-Type: application/json" \
    -d '{"function_address":"0x00455380","new_name":"Graphics_Initialize"}' \
    http://127.0.0.1:8089/rename_function_by_address

# IMPORTANT: API param naming uses underscores (function_address, new_name, search_term)
# IMPORTANT: POST writes require JSON body, form-encoded params silently fail
```

### Using r2 (supplementary — good for quick checks)
```bash
cd ~/hamsterball-re/originals/installed/extracted
r2 -q -c "aaa; afl" Hamsterball.exe | wc -l  # 1869 functions
r2 -q -c "aaa; s 0x429530; pdf" Hamsterball.exe  # App::Initialize
r2 -q -c "aaa; axt @ sym.imp.d3d8.dll_Direct3DCreate8" Hamsterball.exe
```

## MESHWORLD File Format (CONFIRMED)

### Binary Structure
```
[Geometry Section]  — vertex/face data, variable length
[Object Section]    — starts with uint32 count, then objects
[Trailer]           — closing data
```

### Object Format
Each object: `[uint32 str_len][type_string][data...][optional_texture_string]`

**Simple objects** (START, SAFESPOT): 28 bytes after type string
- 12 bytes: position (3 floats: x, y, z)
- 16 bytes: rotation/flags (4 values)

**Complex objects** (FLAG, PLATFORM, BUMPER): variable size
- 12 bytes: position
- 16 bytes: rotation/flags
- 32 bytes: transform matrix (8 floats, identity=1.0)
- 16 bytes: diffuse color (4 floats: RGB + alpha)
- 16 bytes: ambient color (4 floats)
- 4 bytes: size parameter (e.g., 35.0 for platforms)
- 8 bytes: flags (2 uint32)
- [4+N bytes: texture string (length-prefixed)]
- [face index data: pairs of uint32]

### Object Types
START1-1, START2-1, START2-2, START2-3, START2-4 — Player start positions
FLAG02, FLAG04, FLAG06, FLAG07 — Checkpoint flags
SAFESPOT — Safe landing zones
CAMERALOOKAT (CameraLocus) — Camera targets
PLATFORM, N:SINKPLATFORM — Level geometry
N:BUMPER1/2/3/4 — Bumpers
E:NODIZZY<TIME>N</TIME> — Anti-dizzy zones
E:LIMIT — Arena limits
E:GROWSOUND — Sound triggers

### Parsing Approach (IMPORTANT)
The format is NOT straightforward sequential parsing — object data sizes vary per type and can't be determined without knowing the type. **The correct approach** is:
1. Scan entire file for all length-prefixed strings
2. Classify each as "primary type" or "texture"
3. Primary strings define new objects, texture strings attach to the preceding object
4. Read position data from the bytes following each primary string
5. Transform/material data lives between the main type string and the texture string

### Parser Test
```bash
cd ~/hamsterball-re/reimpl
# Build and test (86 levels, 1533 objects confirmed)
./build/level_viewer ~/hamsterball-re/originals/installed/extracted/Levels/Level1.MESHWORLD
# Headless test with Xvfb:
DISPLAY=:99 timeout 3 ./build/level_viewer <level_file>
```

## Running the Game
```bash
cd ~/hamsterball-re/originals/installed/extracted
wine Hamsterball.exe  # Needs X display for rendering
```

## App::Run Game Loop (0x46BD80) — CONFIRMED
```
1. while (!quitFlag):
2.   if PeekMessage(&msg, NULL, 0, 0, PM_REMOVE):
3.     if msg == WM_QUIT: quitFlag = 1; break
4.     TranslateMessage(&msg); DispatchMessage(&msg)
5.   else:
6.     frameStart = GetTickCount()
7.     this->Update()   // vtable call (0x469CF0 = GameUpdate)
8.     this->Render()   // vtable call
9.     this->Present()  // vtable call (0x455A90 = Graphics_PresentOrEnd)
10.    frameEnd = GetTickCount()
11.    elapsed = frameEnd - frameStart
12.    if elapsed < targetFrameTime: Sleep(targetFrameTime - elapsed)
13.    if elapsed > 0: this->fpsDenominator = 33 / elapsed  // frame scaling
```

## Pitfalls / Lessons Learned

### CRITICAL: Always Backup Renames to Git
- The Ghidra project DB is NOT version controlled
- If the project is re-imported or gets corrupted, ALL renames are LOST
- This happened April 2026: dropped from 75.5% to 40% documented
- **After every RE session**: run `ghidra-rename-export` skill to backup to `analysis/ghidra/renames_backup.json`
- **If DB lost**: run `ghidra-restore-renames` skill to restore from `docs/FUNCTION_MAP.md`
- **Server restart**: use `ghidra-mcp-headless` skill with `--program /Hamsterball.exe` (leading slash required)
- Related skills: `ghidra-rename-backup`, `ghidra-restore-renames`, `ghidra-mcp-headless`, `ghidra-rename-export`

### Binary Format RE — String Scanning Works Best
- Sequential parsing of MESHWORLD failed because object sizes are variable per type
- Scanning for all length-prefixed strings first, then building objects from the string list, is the only reliable approach
- The string `[uint32 length][ASCII data]` pattern is very robust for finding object boundaries

### GhidraMCP Headless Server — Key Gotchas
- **POST writes require JSON body**: Form-encoded params (`-d "key=value"`) silently fail with "parameter required" errors. Must use `-H "Content-Type: application/json" -d '{"key":"value"}'`
- **Param names use underscores**: `function_address` not `functionAddress`, `new_name` not `newName`, `search_term` not `searchTerm`
- **Import addresses are EXTERNAL:ordinal**: Not regular program addresses, so `get_xrefs_to` won't work on them. Use the string name (e.g., "BASS_Init") to find the IAT thunk, then find xrefs to the thunk.
- **Ghidra GUI + MCP plugin is fragile**: The plugin requires the GUI CodeBrowser to be open with the project loaded. The headless server (`GhidraMCPHeadlessServer`) is much more reliable for automation.
- **MCP bridge (bridge_mcp_ghidra.py)** discovers instances via Unix domain sockets in `/tmp/ghidra-mcp-{user}/*.sock`. The headless server uses TCP on port 8089 instead. Set `GHIDRA_SERVER_URL` env var or use `connect_instance` with the TCP URL.

### GhidraMCP Struct Creation — JSON Fields Format
- **`create_struct` fields MUST be JSON array format**: ` [{"name":"field_name","offset":0,"type":"float"},...] ` — colon/comma string formats all fail with "No valid fields provided"
- **`add_struct_field` works with explicit offset**: Use for incremental building of large structs
- **`import_data_types` not implemented**: Returns "Import functionality not yet implemented"
- **`run_script_inline` broken**: "BundleHost null" OSGi error since session 47 — use `create_struct`/`add_struct_field` instead
- **Valid type strings**: `float`, `int`, `uint`, `byte`, `char`, `uint32`, `int32`, `byte[20]`, `uint` (pointer), or any Ghidra built-in type name
- **Workflow for large structs**: `create_struct` with initial minimal fields (0-32 bytes), then `add_struct_field` per field with explicit offset — this avoids size conflicts when adding overlapping fields

### Ghidra Issues
- **-readOnly discards all changes**: The `-readOnly` flag in analyzeHeadless discards labels/analysis on exit. Always omit it.
- **Python scripts fail in headless**: "Ghidra was not started with PyGhidra"
- **GUI won't start in headless env**: Xvfb doesn't work reliably for Ghidra's Swing UI
- **Use GhidraMCP headless server instead**: Full 191 REST API endpoints, no GUI needed

### Function Discovery Workflow
1. Search for game-specific strings: `search_strings?search_term=X`
2. Find xrefs to those strings: `get_xrefs_to?address=0x{addr}`
3. Cross-reference to identify functions: each xref tells you which function references it
4. Decompile to confirm: `decompile_function?address=0x{addr}`
5. Rename: `rename_function_by_address` with JSON body

### Vtable Discovery Technique (CRITICAL)
GhidraMCP's `get_xrefs_from` only returns ONE xref per data address (the first entry). To discover ALL vtable method pointers, scan each 4-byte offset:
```
for i in range(0, 0x40, 4):
    result = get_xrefs_from(address=vtable_base + i)
    # Each result links to a function pointer
```
- Dtor is always at +0x00 (first entry)
- The second entry (+0x04) is typically the Update method in this engine
- Some entries may point to addresses Ghidra hasn't defined as functions (thunks/jumps) — these still exist and can be found by searching nearby function addresses
- The Ghidra headless API returns `"No function found"` for undefined thunks — try decompiling nearby addresses to find the real target

### Indirect Object Access Pattern
The App global (0x4FD680) has few direct xrefs because the game typically passes `this` pointers through function calls. To find code accessing specific App fields:
1. Find functions that receive App* as `param_1` or `this`
2. Look for offset accesses matching the App layout (e.g., +0x174 = Graphics, +0x180 = Input)
3. Use string references as anchoring points (debug strings like "App::Initialize(N)")

### MESHWORLD Position Values
- Y is typically negative (below Y=0) in world space
- X and Z span from -200 to +200 for arena levels, -1500 to +1500 for race levels
- Position values for N:SINKPLATFORM/N:BUMPER may be transform-scale (0,0,0 or 1,1,1) rather than world positions

## Input Subsystem (DirectInput8)

### keyboard Device Layout (0x524 bytes, vtable 0x4D9840)
```
+0x00:  vtable pointer
+0x04:  Input* back-pointer (parent Input object)
+0x08:  IDirectInputDevice8* device
+0x0C:  key_state_old[256] (DWORD[0x40] — previous frame key states)
+0x10C:  key_bindings[256] (pointer[0xFF] — DIK code → action object mapping)
+0x143:  KeyDown DIK code (uint8, DIK_* code for Down action)
+0x144:  KeyUp DIK code
+0x145:  KeyLeft DIK code
+0x146:  KeyRight DIK code
+0x147:  KeyAction1 DIK code
+0x148:  KeyAction2 DIK code
+0x184:  ptr to key action object
+0x1CC:  ptr to key action object
+0x1C4:  ptr to key action object
+0x18C:  ptr to key action object
+0x154:  ptr to key action object
+0x194:  ptr to key action object
+0x198:  ptr to key action object
+0x1A0:  ptr to key action object
+0x1A4:  ptr to key action object
+0x1D0:  ptr to key action object
+0x1D4:  ptr to key action object
+0x168:  ptr to key action object
+0x16C:  ptr to key action object
+0x170:  ptr to key action object
+0x14C:  ptr to key action object
+0x158:  ptr to key action object
+0x188:  ptr to key action object
+0x15C:  ptr to key action object
+0x138:  vtable/data pointer
```

### Key Mapping (6 rebindable actions)
- KeyUp (DIK code at KeyboardDevice+0x143)
- KeyDown (DIK code at KeyboardDevice+0x144)
- KeyLeft (DIK code at KeyboardDevice+0x145)
- KeyRight (DIK code at KeyboardDevice+0x146)
- KeyAction1 (DIK code at KeyboardDevice+0x147)
- KeyAction2 (DIK code at KeyboardDevice+0x148)

### Input Subsystem Functions
- Input_Init (0x46EE10): DirectInput8Create with device fallback, EnumDevices for keyboard
- Input_Create (0x46C110): Allocates Input object (0x438 bytes), stores at App+0x180
- KeyboardDevice_ctor (0x46E250): Creates DInput keyboard device (0x524 bytes), sets cooperative level, acquires
- KeyboardDevice_dtor (0x46DE60): Releases DI device, unbinds key mappings
- Mouse input uses DirectInput8 mouse device (not yet fully mapped)

## Level/Object System

### Level Object Layout (0x10D0 bytes, vtable at 0x4D8FB0)
```
+0x00:  vtable pointer
+0x04:  AthenaList* object_list (game objects)
+0x08:  int object_count
+0x18:  AthenaList* (secondary list)
+0x410:  void** object_array (pointer to list data)
+0x41C:  GameObject* tracked_object_1 (e.g., player ball)
+0x420:  GameObject* tracked_object_2
+0x424:  AthenaList* managed_list
+0x428:  AthenaList* (object pool)
+0x430:  uint8 flag_1
+0x431:  uint8 flag_2
+0x4368:  LevelData* (secondary level — clone for split screen)
+0x436C:  Level* level_mesh_1 (MeshWorld)
+0x4370:  Level* level_clone_1
+0x4374:  Level* level_mesh_2 (secondary)
+0x4378:  AthenaList* objects_list_1
+0x4790:  AthenaList* objects_list_2
+0x480:  LevelState* state (ball/physics state, 0x10D4 bytes)
+0x484:  uint8 flag
+0x844:  LevelContext* context (shared rendering state)
+0x868:  char* level_name (e.g., "Board (Dizzy)")
+0x878:  App* back-pointer
```

### LevelState (Ball/Physics) Object (0x10D4 bytes)
```
+0x04:  uint8 enable_flag
+0x14:  int counter
+0x18:  uint32 value (0xf = 15, possibly ball size param)
+0x1C:  AthenaList* (objects)
+0x440:  void* collision_data (freed on cleanup)
+0x444:  IDirect3DVertexBuffer8* vb_1 (freed on cleanup)
+0x448:  IDirect3DVertexBuffer8* vb_2 (freed on cleanup)
+0x44C:  IDirect3DVertexBuffer8* vb_3 (freed on cleanup)
+0x450:  unknown struct (0x14 bytes)
+0x464:  unknown struct (0x14 bytes)
+0x478:  AthenaList* (game entities)
+0x894:  AthenaList* (render entities)
+0xCAC:  AthenaList* (physic entities)
+0x10C4: uint8 flag
+0x10C8: uint32 extra_data (freed on cleanup)
+0x10D0: uint8 flag
```

### Game Object Vtable Layout (inferred from GameUpdate)
```
+0x00:  destructor (called with param=1 for free)
+0x04:  Update() — main update tick (called for active objects)
+0x08:  Release() / IUnknown Release
+0x30:  Render/Cleanup method (called when object removed)
+0x3C:  Render(context) — render with level context param
```

### Game Object Layout (common fields)
```
+0x00:  vtable pointer
+0x0B:  uint8 removed_flag (non-zero = pending removal)
+0x21D: uint8 inactive_flag (non-zero = skip update)
+0x21A: int object_id (used as context during updates)
+0x21C: padding/flags
```

### GameUpdate (0x469CF0) — Object Lifecycle
1. **Active pass**: Iterate object list, call vtable+4 (Update) for each active object (removed_flag=0, inactive_flag=0)
2. **Removal pass**: For objects with removed_flag set:
   - Set LevelContext+0x210 = "Remove Object"
   - If tracked_object_1, call LevelContext+0x68 with (-10000,-10000) and LevelState+0x20C=object_id
   - Call vtable+0x30 (cleanup render)
   - Remove from managed list
   - Call vtable+0x3C (render with level context)
   - Call vtable+0x00 (destructor) with free=1
   - Set LevelContext+0x210 = "Update"

### Board/Screen Hierarchy
- Board_ctor (0x419030): Base class, vtable at 0x4D0260
- BoardLevel3_ctor (0x41D060): "Dizzy" board, vtable at 0x4D0890
- Board types: Beginner, Intermediate, Dizzy, Tower, Expert, Odd, Wobbly, Toob, Sky, Up, Master
- RumbleBoard: Arena variants (Warmup, Beginner, Intermediate, Dizzy, Tower, Up, Expert)

## MESHWORLD Object Types (Complete List)

### N: (Named/Physical) Objects
- N:MOUSETRAP, N:TARPIT, N:WATER, N:NOCONTROL, N:UNLOCKSECRET, N:SECRET
- N:BRIDGE, N:SWIRL, N:WHEELEMBED, N:WATERWHEEL, N:MACE
- N:TRAPDOOR, N:JUMPSECOND, N:JUMPFIRST, N:WAVY, N:SQUAREWOBBLY
- N:BUMPER, N:BUMPER%d (numbered), N:SAWTEETH, N:SPINNER
- N:EXTRATIME, N:SPEEDCYLINDER, N:NEONPLATFORM, N:BUMP
- N:TENBONUS2, N:TENBONUS1, N:GLASS, N:ONGEAR, N:ONROTATOR
- N:BOUNCE, N:SINKPLATFORM (also DN:SINKPLATFORM)

### E: (Event/Trigger) Objects
- E:TRAJECTORY, E:ACTION, E:JUMP, E:BREAK, E:LIMIT
- E:SAFESWITCH, E:NODIZZY, E:MACETRIGGER, E:BITE, E:OPENSESAME
- E:CATAPULTBOTTOM, E:BELL, E:SCORE, E:ALERTJUDGES
- E:ACTIVATESAW1, E:ACTIVATESAW2, E:ALERTSAW1, E:ALERTSAW2
- E:HAMMERCHASE, E:CALLHAMMER, E:LIMITPIPE2, E:LIMITPIPE1
- E:SWALLOW, E:LIMITX, E:LIMITZ, E:PIPERANDOM, E:DROPLIFT
- E:GROW, E:GROWSOUND, E:SHRINK, E:GRAVITY
- E:BRANCH, E:HEATON, E:HEATOFF, E:NOPEGS, E:PEGS
- E:TRAPPOP, E:VACPOPOUT, E:HELPINERTIA, E:UNHELPINERTIA
- E:LAUNCH, E:LIGHTSON, E:LIGHTSOFF, E:ZOOP

## Ball Physics System

### Ball Vtable (0x4CF3A0)
The ball is a GameObject subclass with dedicated physics. Base class is GameObject at 0x4CF314.

| Offset | Address | Function | Description |
|--------|---------|----------|-------------|
| +0x00 | 0x4027F0 | Ball_dtor | Destructor (calls Ball_Cleanup) |
| +0x04 | 0x405100 | (thunk→0x405190) | Update method |
| +0x08 | 0x402DE0 | Ball_CollisionCheck | Per-frame collision check |
| +0x0C | 0x402A70 | (not defined) | Render setup |
| +0x10 | 0x408390 | (not defined) | Unknown |
| +0x14 | 0x401590 | (not defined) | Unknown |
| +0x18 | 0x402650 | Ball_ApplyForce | Apply directional force |
| +0x1C | 0x402C10 | (not defined) | Unknown |
| +0x20 | 0x409480 | (not defined) | Unknown |

### Ball Object Layout (0xC98 bytes)

| Offset | Type | Description |
|--------|------|-------------|
| 0x00 | void** | Vtable pointer (0x4CF3A0) |
| 0x04 | int | Level reference (parent) |
| 0x08 | int | Parent object reference |
| 0x0C | float[6] | Collision planes (4 floats each: ax+by+cz+d) |
| 0x42 | Timer | Per-ball timer |
| 0x59 | float | Ball X position (legacy) |
| 0x5A | float | Ball Y position (legacy) |
| 0x5B | float | Ball Z position (legacy) |
| 0x62 | float | Ball size (default 3.0f = 0x40400000) |
| 0x69 | void* | Sound object |
| 0x96 | int | Sound handle |
| 0x154 | IDirect3DDevice8* | D3D device pointer |
| 0x164 | float | **X position** (authoritative) |
| 0x168 | float | **Y position** (authoritative) |
| 0x16C | float | **Z position** (authoritative) |
| 0x170 | float | **X velocity** (zeroed on collision) |
| 0x174 | float | **Y velocity** (zeroed on collision) |
| 0x178 | float | **Z velocity** (zeroed on collision) |
| 0x17C | float | Acceleration X (zeroed on collision) |
| 0x180 | float | Acceleration Y (zeroed on collision) |
| 0x184 | float | Acceleration Z (zeroed on collision) |
| 0x1A4 | int | Level state reference |
| 0x284 | float | Ball radius / height offset |
| 0x2CC | byte | Force disable flag |
| 0x2DC | float | X position (secondary) |
| 0x2E0 | float | Y position (secondary) |
| 0x2E4 | float | Z position (secondary) |
| 0x2F0 | int | Frame counter (affects force scaling) |
| 0x2F8 | byte | Active flag |
| 0x2F9 | byte | Collision occurred flag |
| 0x300 | int | Value 0x96 (150) - mass/timer constant |
| 0x310 | byte | State flag (1 = active) |
| 0x314 | float | Home X position |
| 0x318 | float | Home Y position |
| 0x324 | byte | Shrunk/special state flag |
| 0x440 | void* | Collision data |
| 0x6FC | int | Render initialized flag |
| 0x700 | byte | Render mode (affects lighting) |
| 0x708 | int | Render parameter |
| 0x70D | byte | Render state initialized |
| 0x734 | byte | Sub-render mode |
| 0x748 | int | Collision mesh pointer |
| 0x76A | byte | Flag (zeroed on collision) |
| 0x7C8 | int | Render call counter |
| 0x808 | int | Freeze counter (skip force if > 0) |
| 0x810 | AthenaList | Force application list |
| 0xA1 | float | Ball radius (from SIZE param) |
| 0xC74 | int | Collision counter |
| 0xC84 | Quaternion | Object rotation |

### Physics Constants (Data Section)

| Address | Value | Description |
|---------|-------|-------------|
| 0x4CF368 | float | Collision radius threshold |
| 0x4CF374 | float | Force multiplier (C5C flag set) |
| 0x4CF378 | float | Force multiplier (shrunk state) |
| 0x4CF380 | float | Force scaling (first frame) |
| 0x4CF3E8 | float | Force direction multiplier |
| 0x4CF48C | float | Gravity/height constant |

### Key Ball Functions

- **Ball_Update (0x405190)**: 18KB main physics loop. Iterates objects, checks collisions, applies forces, handles reset on collision.
- **Ball_ApplyForce (0x402650)**: Applies force vector (x,y,z,magnitude) with state-dependent multipliers.
- **Ball_CheckCollisionPlanes (0x402810)**: Tests ball against 6 collision planes using formula: distance = A*z + B*y + C*x + D. Returns true if all planes pass (ball inside convex volume).
- **Mesh_FindClosestCollision (0x465D90)**: DDA ray traversal through spatial hash to find closest mesh triangle hit point. Returns 3D position.
- **Ball_CollisionCheck (0x402DE0)**: Per-frame collision check against mesh, increments counter on hit.
- **Ball_Render (0x402860)**: D3D8 render: SetRenderState calls, texture setup, DrawPrimitiveUP.
- **Ball_ResetCollisionMesh (0x4030B0)**: Resets collision mesh and orientation.
| 0x40C5D0 | GameObject_HandleCollision | Main collision event dispatcher — handles ALL object types: E:NODIZZY (with TIME param), E:SAFESWITCH (with parenthesized data), E:LIMIT (arena unlock tracking per player 0-3), E:BREAK (ball vtable[0x20] bounce), E:JUMP (plays 3D sound, freeze 10 frames, counter=200), E:ACTION (ONCE flag + SCORE award via Difficulty_GetTimeModifier), E:TRAJECTORY (X/Y/Z params set ball trajectory), N:NOCONTROL (disables input 10 frames), N:WATER (sets water flag + 10 frame counter), N:TARPIT (plays 3D sound, marks tar state, clears velocity), N:GOAL (reached goal! plays "Goal!" music, sets flags), N:MOUSETRAP (redirects ball with trap animation using trajectory dir + DAT_004CF370 speed), N:SECRET (marks Rotator triggered), N:UNLOCKSECRET (calls CheckArenaUnlock). DROPIN (checks trajectory magnitude, plays sound, counter=50), PIPEBONK (random sound from 3, counter=10), POPOUT (sound, counter=50) also handled |
- **Ball_ctor (0x40AFE0)**: Calls GameObject_ctor, sets vtable to 0x4CF3A0, initializes quaternion to identity
### Rendering Pipeline (CONFIRMED from Ghidra)
- Graphics_BeginFrame (0x453B50) → Graphics_RenderScene (0x454BC0) → Graphics_PresentOrEnd (0x455A90)
- Graphics_RenderScene: SetupLights, copy world/view/projection matrices, SetTransform on D3D device, compute frustum for culling, call 8 render callbacks, set Z/stencil/alpha render states
- Graphics_SetViewTransform (0x454A30): Set D3DTS_VIEW matrix, flip for mirror mode (+0x7D2), recompute frustum
- Graphics_SetupFog (0x4539A0): D3DRS_FOGENABLE, FOGVERTEXMODE, FOGSTART, FOGEND, FOGCOLOR — uses linear fog with distance-based scaling  
- Graphics_Clear (0x453900): IDirect3DDevice8::Clear — target+Z (3) or Z+stencil (7)
- Graphics_DrawScreenQuad (0x455F40): DrawPrimitiveUP for 2D overlay quads (UI, menus)
- Matrix_ComputeFrustum (0x4762B0): Build view frustum from projection params
- Graphics_InitRenderStates (0x42C810): Set background color
- Level_Update (0x4606D0): Build collision vertex buffer from mesh faces, assemble into D3D vertex format (32 bytes: pos+norm+uv), create spatial hash
- Level_ExtractByMaterial (0x471830): Extract triangles matching material filter, create new MeshWorld, compute AABB bounds
1. CreateBadBall (0x40BCA0): Creates 0xC98 byte ball object, parses N:BADBALL params (CHASE, HOME, SIZE)
2. Ball_ctor (0x40AFE0): Calls GameObject_ctor, sets vtable to 0x4CF3A0, initializes quaternion to identity
3. GameObject_ctor (0x4039E0): Base class ctor, sets vtable to 0x4CF314

## Athena List Utility Functions

AthenaList (vtable 0x4D875C) is a generic container class used throughout the engine:
- AthenaList_Init (0x453210): Initialize 256-entry hash table list
- AthenaList_Clear (0x453280): Clear list contents
- AthenaList_GetIndex (0x4532B0): Get current iteration index
- AthenaList_Remove (0x4534D0): Remove item from list

List structure (0x418 bytes):
```
+0x000: vtable (PTR_FUN_004D875C)
+0x004: next_id / param
+0x008: count (number of items)
+0x00C: iteration counters (int[256])
+0x100: data area (256 entries, zero-initialized)
+0x410: void** data (pointer to dynamic array)
+0x414: uint32 capacity
```

## Texture System

### Texture Loading Pipeline
The game embeds libpng, libjpeg, and D3DX8 statically — no external DLL dependencies for image loading.

1. **Graphics_LoadTexture (0x455C50)**: Cache-aware texture loader
   - Checks texture cache (Graphics+0x2E4 AthenaList)
   - If cached: increments refcount at texture[4]
   - If not cached: calls Texture_Create

2. **Texture_Create (0x476770)**: Creates 0x74-byte texture object
   - Strips file extension from filename
   - Tries multiple file formats: `%s%s-mip1.%s`, then without "-mip1"
   - Attempts up to 3 format variations (PNG, JPG, BMP)
   - Uses "textures\\" prefix (or custom path if Graphics+0x7D8 set)
   - Calls Image_LoadFromFile to actually load

3. **Image_LoadFromFile (0x489217)**: D3DX image file loader wrapper
   - FUN_00487E5E: Initialize file handle struct (sets to -1,-1,0,0)
   - FUN_00487E70: Read file into memory buffer
   - FUN_004891D7 → FUN_00488B64: Create D3D texture from decoded image

4. **D3DX_CreateTextureFromMemory (0x488B64)**: 15KB D3DX texture creation
   - Parses image header (IJG JPEG or libpng)
   - Creates D3D texture with mipmap chain
   - Returns D3DERR_INVALIDCALL (0x8876086C) or E_FAIL (0x80004005) on error

### Texture Object Layout (0x74 bytes, vtable 0x4DA648)
```
+0x00:  void** vtable (PTR_FUN_004DA648)
+0x04:  IDirect3DTexture8* texture (primary)
+0x08:  IDirect3DTexture8* texture_mip1 (first mip level)
+0x0C:  Graphics* graphics (back-pointer)
+0x18:  char* filename (loaded from)
+0x1C:  uint8 flag_1
+0x1D:  uint8 flag_2
+0x1E:  uint8 flag_3
+0x20:  uint32 width
+0x24:  uint32 height
+0x28:  uint32 mip_levels
+0x2C:  uint32 format (D3DFMT_*)
+0x30:  uint32 refcount (incremented on cache hit)
+0x34:  uint32 pool (D3DPOOL_*)
+0x38-0x70: internal D3D data
```

### Graphics Object Texture Cache
- Graphics+0x2E4: AthenaList* texture_cache (list of loaded textures)
- Graphics+0x6F0: void** texture_array (pointer to cache data)
- Graphics+0x2E8: int texture_count

### Mipmap Support
The game uses explicit mipmap files:
- `texture.png` — base level (mip0)
- `texture-mip1.png` — first mip level
- `texture-mip2.png` — second mip level
- Debug strings confirm: "Create Texture with %d Mip Levels...", "Plotting mip level..."

### Supported Formats
- **PNG**: Primary format for most textures (119 references)
- **BMP**: Used for checker patterns (purplechecker.bmp, brightgreenchecker.bmp, etc.)
- **JPG**: Supported (JPEG error strings in binary)

### Built-in Libraries (Statically Linked)
- libpng: "Incompatible libpng version...", "Width too large..."
- libjpeg: "Invalid SOS parameters...", "Corrupt JPEG data..."
- D3DX8: "D3DXERR_INVALIDMESH", "D3DXERR_CANNOTATTRSORT", etc.

## .MESH Binary Model Format (v1-v5, CONFIRMED)

### Version 1 (Sphere, 8Ball, Hamster, etc.)
```
[4] version (=1)
[4] name_length
[name_length] name string
[88] material data: ambient(4f), diffuse(4f), specular(4f), shine, shadow_bias, shadow_scale
[4] texture_name_length
[texture_name_length] texture filename
[4] vertex_count
[4] unknown (material group index?)
[vertex_count * 32] vertex array:
  [float x, y, z] [float nx, ny, nz] [float u, v] = 32 bytes per vertex
[remaining] face/triangle data
```

### Versions 2-5 (Chomper, FanBody, HammyJudge, etc.)
Same header structure but with additional sub-object sections. Texture string can be found by scanning for ".png"/".jpg" length-prefixed strings. Vertex data may be split into multiple mesh buffers with separate headers.

### Parser
- Code: `reimpl/src/level/mesh_parser.c`, header: `reimpl/include/level/mesh_parser.h`
- Test: `reimpl/src/level/mesh_test.c`
- Successfully parses: Sphere (59 verts), Hamster (119 verts), 8Ball (42 verts), FunBall, Eye, Sawblade, TarBubble, etc.
- v1 fully working, v2-v5 partially working (finds texture, may miss vertices in sub-objects)

## Binary MESHWORLD Format (from Ghidra Decompilation of 0x4629E0)

### Complete Section Layout
```
Section 1: Materials
  [4] material_count
  For each material:
    [4] name_length + [name_length] name
    [4] face_start, [4] face_count, [4x4] unknown (6 uint32 total)
    [4] extended_flag (checked as char, 4 bytes read)
    If extended:
      [4x4] ambient RGBA, [4x4] diffuse RGBA, [4x4] specular RGBA
      [4] shine, [4] has_reflective, [4] has_texture
      If has_texture: [4] tex_name_length + [tex_name_length] texture filename

Section 2: Mesh Buffers
  [4] mesh_buffer_count
  For each buffer:
    [4] name_length + [name_length] name
    [4] face_count
    For each face: [3 reads of 4 bytes] + FUN_004685e0 call

Section 3: Game Objects
  [4] object_count
  For each object:
    [4] object_type (0=straight, others=special)
    If type==0: [3x4] position, [3x4] size, [3x4] scale, material init

Section 4: Bounding Box
  [6x4] min_x, min_y, min_z, max_x, max_y, max_z

Section 5: Vertex Array
  [4] vertex_count
  [vertex_count * 32] vertex data (pos+norm+uv, 32 bytes each)

Section 6: Post-load
  level+0x43C = 0 (vertex_data_ready)
  level+0x47C = this (self_ptr)
  Timer_Init, vtable callback, __close
```

### Level Object Offsets (LevelState/MeshWorld)
| Offset | Field | Description |
|--------|-------|-------------|
| +0x438 | vertex_count | Number of vertices |
| +0x440 | vertex_array | Vertex data (32 bytes each) |
| +0x454 | min_x | Bounding box min X |
| +0x458 | min_y | Bounding box min Y |
| +0x45C | min_z | Bounding box min Z |
| +0x468 | max_x | Bounding box max X |
| +0x46C | max_y | Bounding box max Y |
| +0x470 | max_z | Bounding box max Z |
| +0x894 | materials | Material list (AthenaList) |
| +0xCAC | mesh_buffers | Mesh buffer list (AthenaList) |

## Ball Physics Constants (from Ball_Update dispatcher 0x405100)

| Value | Usage |
|-------|-------|
| radius = 35.0 (0x420C0000) | Ball collision radius |
| max_speed = 600.0 (0x44160000) | Maximum velocity |
| max_speed2 = 1200.0 (0x44960000) | Maximum velocity (secondary) |
| friction1 = 0.5 (0x3F000000) | Surface friction |
| friction2 = 0.2 (0x3E4CCCCD) | Low friction |
| damping = 0.2 (0x3E4CCCCD) | Velocity damping |

### Ball ApplyForce Multipliers
| Condition | Multiplier | Address |
|-----------|-----------|---------|
| First frame | 1.0 → 0.25 | 0x4CF380 |
| In tube | 0.0 (no force!) | 0x4CF378 |
| On ice | 0.2 | 0x4CF374 |
| Is dizzy | 0.75 | 0x4CF36C |

## Level Vtable (0x4D8FB0) — 16 Entries

| Index | Offset | Address | Function |
|-------|--------|---------|----------|
| 0 | +0x00 | 0x4629C0 | Level_dtor |
| 1 | +0x04 | 0x4606D0 | Level_Update (233 lines) |
| 2 | +0x08 | 0x472770 | Level_vtbl2 |
| 3 | +0x0C | 0x471750 | Level_LoadMesh |
| 4 | +0x10 | 0x470440 | Level_vtbl4 |
| 5 | +0x14 | 0x46F3B0 | Level_vtbl5 |
| 6 | +0x18 | 0x470150 | Level_vtbl6 |
| 7 | +0x1C | 0x46F390 | Level_vtbl7 |
| 8 | +0x20 | 0x471830 | Level_DrawBuffer |
| 9 | +0x24 | 0x45DFD0 | Level_vtbl9 |
| 10 | +0x28 | 0x461370 | Level_vtbl10 |
| 11 | +0x2C | 0x44ACB0 | Level_vtbl11 |
| 12 | +0x30 | 0x45DE30 | Level_LoadMeshWorld |
| 13 | +0x34 | 0x461890 | Level_vtbl13 |
| 14 | +0x38 | 0x4629E0 | Level_BinaryLoader |
| 15 | +0x3C | 0x460DA0 | Level_vtbl15 |

## Level Viewer v2

### Build & Run
```bash
cd ~/hamsterball-re/reimpl
gcc -o build/level_viewer_v2 \
    src/level/mesh_parser.c src/level/level_viewer_v2.c \
    -I include -lm \
    $(pkg-config --cflags sdl2 SDL2_image gl glew) \
    $(pkg-config --libs sdl2 SDL2_image gl glew) -lGLU

# Run with a level:
DISPLAY=:99 ./build/level_viewer_v2 ~/hamsterball-re/originals/installed/extracted/Levels/Level1.MESHWORLD

# Controls: WASD=move, mouse=look, Space=up, Shift=down, L=wireframe, T=textures, ESC=quit
```

### Features
- Loads .MESHWORLD files, scans for all object types (START, FLAG, SAFESPOT, PLATFORM, BUMPER, etc.)
- Renders with OpenGL: grid, axes, start positions (green crosshairs), flag markers (yellow), other objects (blue)
- Loads .MESH models for ball (Sphere.MESH) rendering at start position
- Camera centered on level bounding box

### .MESH Parser Test
```bash
cd ~/hamsterball-re/reimpl
gcc -o build/mesh_test src/level/mesh_parser.c src/level/mesh_test.c -I include -lm
./build/mesh_test ~/hamsterball-re/originals/installed/extracted/Meshes/Sphere.MESH
# Output: Name: Sphere01, Vertices: 59, Texture: HamsterBall.png, BBox: X[-4.88,25.00]...
```

### .MESH Python Renderer (Visual Identification)
Located at `~/hamsterball-re/tools/render_meshes.py`. Uses numpy + matplotlib to render .MESH files as PNG images for visual identification.

**Workflow:**
1. Parse .MESH binary format (v1 working, v2-v5 partial)
2. Render multi-view (3 angles: 30°, 120°, 210°) with face data + scatter overlay
3. Save to `~/hamsterball-re/analysis/screenshots/meshes/{name}.png`
4. Vision-analyze the rendered images to identify game objects
5. Use identified names for Ghidra function/structure renaming

**Usage:**
```bash
source ~/hb_venv/bin/activate
python ~/hamsterball-re/tools/render_meshes.py
```

**Current Issues:**
- v2-v5 meshes (HammyJudge, DawgShoe, MagnifyingGlass, SkyPillar) parse with 0 vertices — sub-object format not yet handled
- Face/index data not extracted — only vertex scatter rendering for v1 meshes
- NaN errors on some meshes (Bell, FanBody, GlassBonus-Smashed) — need bounds checking
- The v1 format after texture name: vertex_count (uint32), unknown (uint32), then vertices (32 bytes each)

**v1 Format Details (confirmed from Sphere.MESH hex analysis):**
```
offset 0: version (1)
offset 4: name_length (9)
offset 8: name ("Sphere01\0")
offset 17: material data (88 bytes):
  - ambient RGBA (4 floats)
  - diffuse RGBA (4 floats) 
  - specular RGBA (4 floats)
  - shine (1 float, e.g. 25.0)
  - shadow_bias (1 float, 0.0)
  - shadow_scale (1 float, stored as int 1)
offset 105: texture_name_length (16)
offset 109: texture name ("HamsterBall.png\0")
offset 125: after texture, two uint32s:
  - 0x3B = 59 (vertex_count!)
  - then vertex data starts (32 bytes each: xyz + normals + uv)
```

**v2+ Format (needs more RE — from Chomper.MESH hex analysis):**
- Version 2+: sub-objects with separate mesh buffers
- After name: position(3 floats), then material(88 bytes)
- Sub-objects: count, name+texture per sub-object, transform data per sub-object
- Each sub-object has its own vertex/face data section
- Need to match decompiled FormatReader (0x4629E0) logic for full parsing

## C Reimplementation (Milestone 6 — In Progress)

### Module Structure (~/hamsterball-re/reimpl/src/)
- `main.c` — Game loop: SDL init, load level, physics tick, render, input polling
- `physics.c` / `physics.h` — Ball physics: gravity, acceleration, collision planes, position/velocity
- `input.c` / `input.h` — Keyboard polling: up/down/left/right/action1/action2
- `renderer.c` / `renderer.h` — OpenGL render stub (D3D8→GL translation layer)
- `audio.c` / `audio.h` — Audio stub (BASS→SDL_mixer)
- `level.c` / `level.h` — Level management stub
- `config.c` / `config.h` — Config management, types (Vec3, App, GameObject)
- `ui.c` / `ui.h` — UI stub

### Build System
CMake-based. Key fix: SDL2 must be found via pkg-config, not FindSDL2.cmake:
```cmake
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
target_include_directories(hamsterball PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_directories(hamsterball PRIVATE ${SDL2_LIBRARY_DIRS})
target_link_libraries(hamsterball ${SDL2_LIBRARIES} GL m pthread)
```

### Type Conflict Fix
config.h and game.h both defined bool, Vec3, App, GameObject — resolved by using unique
type names (GameApp, GameVec3, etc.) or #include guards with #ifndef checks.

### Current Status
- Builds and runs successfully on Linux (SDL2 + OpenGL)
- Loads MESHWORLD levels (57+ levels parse OK)
- Game loop runs: init → load → update → render → shutdown (clean exit)
- Input: keyboard polling works, 6 game keys mapped
- Physics: gravity + acceleration + velocity applied per frame, collision planes checked
- Rendering: stub — needs actual mesh rendering pipeline (GL draw calls from MeshWorld data)
- Audio: stub — needs BASS→SDL_mixer bridge

### D3D8/DInput8/DSound8 Cross-Compilation (32-bit Windows)

The reimplementation uses the original Windows APIs, not SDL2+OpenGL. This allows
faithful recreation and Wine compatibility on Linux.

**Toolchain Requirements:**
- `i686-w64-mingw32-gcc` (32-bit MinGW cross-compiler)
- D3D8 headers: `/usr/share/mingw-w64/include/d3d8.h` (standard MinGW install)
- DInput8 headers: `/usr/share/mingw-w64/include/dinput.h`
- DSound8 headers: `/usr/share/mingw-w64/include/dsound.h`

**Critical Fixes for Compilation (FIXED 2026-04-15):**

1. **WAVEFORMATEX requires mmsystem.h** — Include before dsound.h:
   ```c
   #include <windows.h>
   #include <mmsystem.h>  /* Needed before dsound.h for WAVEFORMATEX */
   #include <d3d8.h>
   #include <dinput.h>
   #include <dsound.h>
   ```

2. **D3DPRESENT_PARAMETERS field names** — Use correct field names:
   - `PresentationInterval` does NOT exist in D3D8 headers
   - Use `FullScreen_PresentationInterval` instead:
   ```c
   g_d3dpp.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
   ```

3. **D3D8 has no D3DX math functions** — D3DXVECTOR3, D3DXMATRIX, D3DX_PI are in D3DX8,
   not D3D8. Use plain math:
   - Replace `D3DXVECTOR3` with `struct { float x,y,z; }` or floats
   - Replace `D3DX_PI` with `3.14159265f`
   - Build view matrices manually (see win32_main.c SetMatrices function)

4. **No D3DXCreateTextureFromFile** — D3D8 doesn't have this. Use D3D8 API directly:
   - `IDirect3DTexture8` creation via device methods
   - Or link d3dx8.lib if using D3DX (separate DLL)

**Working Compilation Command:**
```bash
i686-w64-mingw32-gcc -std=c11 -m32 -O2 -g -Wall \
  -Iinclude -DWIN32_LEAN_AND_MEAN \
  src/core/win32_main.c src/level/meshworld_parser.c src/level/mesh_parser.c \
  -o hamsterball.exe \
  -ld3d8 -ldinput8 -ldsound -ldxguid -lole32 -lwinmm -mwindows
```

**Link Libraries:**
- `-ld3d8` — Direct3D8 API
- `-ldinput8` — DirectInput8 API
- `-ldsound` — DirectSound8 API
- `-ldxguid` — DirectInput/DirectSound GUIDs
- `-lole32` — COM initialization
- `-lwinmm` — Windows Multimedia (for timer functions)
- `-mwindows` — Windows GUI mode (no console)

**Testing with Wine:**
```bash
# On Linux, Wine translates D3D8 → OpenGL automatically
wine hamsterball.exe
```

**Important:** The original game ships `bass.dll` alongside the exe. For the open-source
reimplementation, BASS.dll can be:
1. Purchased separately and placed alongside the exe (eventually)
2. Replaced with SDL_mixer or similar (interim, not faithful to original API)

The current approach uses D3D8/DInput8/DSound8 for authenticity, BASS can be stubbed
for now until licensing is resolved.
SDL2 headers use `#include <SDL2/SDL.h>` but SDL_image/mixer headers include `<SDL.h>`
internally. You need BOTH `-I` (for `SDL2/SDL.h` style) AND `-isystem` (for bare `SDL.h`
style from within SDL2_image/mixer headers):

```bash
cd ~/hamsterball-re/reimpl
bash build/build_win64.sh   # Uses the saved build script
```

The build script is at `reimpl/build/build_win64.sh`. Key flags:
- `-I` paths for `SDL2/SDL.h`, `SDL2/SDL_image.h`, `SDL2/SDL_mixer.h`
- `-isystem` paths for bare `SDL.h` (included from within SDL2_image/mixer)
- Links: `-lopengl32 -lglu32 -lsetupapi -lole32 -loleaut32 -limm32 -lversion -lwinmm`
- Output: `build-win64/hamsterball.exe` (~15MB static)

**IMPORTANT**: Use `opendir()` instead of `fopen()` for directory existence checks.
On Windows/Wine, `fopen("path/to/dir", "rb")` returns NULL even if the directory exists.
`opendir("path/to/dir")` works correctly on both Linux and Windows.

### Run (Linux)
```bash
cd ~/hamsterball-re/reimpl/build
./hamsterball  # Runs with default level, 3-sec timeout in headless mode
```

### Run (Wine — test Windows build)
```bash
cd ~/hamsterball-re/reimpl/build-win64
DISPLAY=:99 WINEDEBUG=-all timeout 10 wine hamsterball.exe 2>/dev/null
# Copies of game assets (Levels/, Textures/, etc.) must be alongside hamsterball.exe
```

### Legacy Level Viewer (still works)
```bash
cd ~/hamsterball-re/reimpl
gcc -o build/level_viewer \
    src/level/meshworld_parser.c src/level/level_viewer.c \
    -I include \
    $(pkg-config --cflags sdl2 SDL2_image SDL2_mixer gl glew) \
    $(pkg-config --libs sdl2 SDL2_image SDL2_mixer gl glew) -lm
```