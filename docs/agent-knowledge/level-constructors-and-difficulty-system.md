# Hamsterball Level Constructors & Difficulty System

## App+0x23C — Difficulty Level

**NOT a "practice mode" flag.** It is the tournament difficulty level:

| Value | Name | Effect |
|-------|------|--------|
| 0 | Pipsqueak | Extra time, **all enemies removed**, half scoring |
| 1 | Normal | Standard time, enemies on, normal scoring |
| 2 | Frenzied | Less time, enemies on, double scoring |

### Write Sites

| Address | Value | Context |
|---------|-------|---------|
| `0x42E3A6` | 0 | Difficulty menu → Pipsqueak → calls App_StartTournamentRace |
| `0x42E3DA` | 1 | Difficulty menu → Normal → calls App_StartTournamentRace |
| `0x42E40E` | 2 | Difficulty menu → Frenzied → calls App_StartTournamentRace |
| `0x428BB2` | 1 | App_StartPartyRace (0x428B20) — forces Normal |
| `0x428DCC` | 1 | App_StartPracticeRace (0x428C50) — forces Normal |

**App+0x23C is NOT persisted** to the Windows Registry. It is set per-race.

### App+0x237 — Party/2P Mode Flag

Byte flag. Set to 0 by all race start functions. Used as an OR condition with difficulty:
`if (difficulty != 0 || party_mode)` → spawn enemies

This means enemies spawn in Party mode regardless of difficulty.

## Difficulty_GetTimeModifier (0x428ED0)

```c
int difficulty = App->difficulty;  // +0x23C
if (difficulty == 0) return time * 0.5;   // Pipsqueak: half time bonus
if (difficulty == 1) return time;          // Normal: unchanged
if (difficulty == 2) return time + time;   // Frenzied: double time bonus
return 0.0;
```

Called from: Ball_Update, DispatchCollisionEvents, BadBall_Update, TourneyMenu_ctor.

## Level Constructor Architecture

### Universal 4-Step Pattern (all 15 levels)

Every `Scene_LoadLevel<Race>` (vtable[18]) follows this skeleton:

1. **operator_new(0x10D0)** → `Level_MeshWorldCtor(0x461510)(obj, gfx, "levels\\<file>")`
   - Stores at `board+0x22B` (board+0x8AC)
   - Parses the .MESHWORLD binary file via `LoadMeshWorld`
   - Initializes 4 AthenaLists, a Timer, and a SceneObject_Base

2. **operator_new(0x10D0)** → `Level_RenderCtor(0x465080)(obj, meshWorld)`
   - Stores at `board+0x22C` (board+0x8B0)
   - Calls `Level_ctor` then `Level_LoadMeshes` — builds collision meshes

3. **Level_InitScene(0x40B090)(board)**
   - Creates SoundChannel, sets up camera (CAMERALOCUS), projection, fog
   - Starts level music via `Audio_PlayMusicAtSpeed`

4. **vtable[32]()** — calls `Board_Setup(0x41C5B0)` (shared)

### Board_Setup (0x41C5B0) — Shared vtable[32]

After setting up balls and safe spots:

```c
// Global enemies — gated by difficulty/party
if ((App+0x23C != 0) || (App+0x237 != 0))  CreateBadBalls(board);     // 0x40BCA0
if ((App+0x23C != 0) || (App+0x237 != 0))  CreateMouseTrap(board);     // 0x40BF50

// Always created (terrain/scenery):
CreateSecretObjects(board);
Scene_CreateFlags(board);
Scene_CreateSigns(board);
Scene_CreateDynamicObjects(board);  // dispatches to vtable[33] per mesh object
```

### CreateDynamicObjects (vtable[33]) — Per-Level Enemy Spawning

Called once per mesh object in the level. Each level has its own implementation that
checks mesh object names and dispatches to the appropriate constructor.

**Enemies gated by `App+0x23C != 0` (non-Pipsqueak):**

| Race | Mesh Name | Constructor | Size |
|------|-----------|-------------|------|
| Dizzy | TIPPER | Tipper_ctor | 0x1104 |
| Dizzy | GLUEBIE | Gluebie_ctor | 0x110C |
| Tower | MACE | Mace_ctor (0x438750) | 0x110C |
| Expert | BONK | Bonk_ctor | 0x1200 |
| Expert | FAN | Fan_ctor (0x438C20) | 0x1188 |
| Expert | SAWBLADE | SawBlade_ctor (0x434660) | 0x111C |
| Toob | SAW | Saw_ctor (0x43B780) | 0x1110 |
| Toob | SAW2 | Saw2_ctor (0x43BE20) | 0x1118 |
| Toob | BLOCKDAWG1 | Blockdawg_ctor | 0x1154 |
| Toob | BLOCKDAWG2 | Blockdawg_ctor | 0x1154 |
| Toob | BLOCKDAWG3 | Blockdawg_ctor | 0x1154 |
| Sky | POPCYLINDER | Popcylinder_ctor | 0x10F4 |
| Sky | MAGNIFYER | Magnifier_ctor | 0x444 |
| Master | TIPPER | Tipper_ctor | 0x1104 |
| Master | BONK | Bonk_ctor | 0x1200 |
| Master | BLOCKDAWG1 | Blockdawg_ctor | 0x1154 |
| Master | BLOCKDAWG2 | Blockdawg_ctor | 0x1154 |
| Master | GLUEBIE | Gluebie_ctor | 0x110C |
| All | BADBALL | BadBall_ctor (0x40AFE0) | 0xC98 |
| All | MOUSETRAP | MouseTrap_ctor (0x437880) | 0x10F8 |

**Master Rotators:** Removed via `Rotator_RemoveAndFree(0x436FC0)` in `Scene_LoadLevelMaster`
when `App+0x23C == 0` (Pipsqueak). Iterates `board+0x150A` AthenaList.

**NOT gated (terrain/physics objects, appear in all difficulties):**

| Race | Objects |
|------|---------|
| Tower | CATAPULT, DRAWBRIDGE, WINDMILL, TRAPDOOR, CHOMPER, TURRET |
| Expert | BRIDGE, JUDGE, BELL |
| Toob | SPINNY, FALLOUT1 |
| Odd | LIFTER |
| Up | LIFTER, SPEEDCYLINDER, TIMEBUTTON |
| Master | BRIDGE, BBRIDGE1, BBRIDGE2, POPCYLINDER, CATAPULT |
| Neon | NEONPLATFORM, DFLOOR1-4, TRODE |
| Glass | SMASHER1, SMASHER2 |
| Wobbly | WOBBLY1-7, WAVY1 |
| Impossible | LOOPER, GEAR, BIGGEAR, ROTATOR, PENDULUM |

## Enemy Constructor Pattern

The Create* functions are **scanners/dispatchers** — they iterate MESHWORLD mesh objects
and dispatch to individual constructors:

```
Board_Setup
  └─ CreateBadBalls(board)           ← scans for "BADBALL" mesh objects
       └─ for each BADBALL found:
            ├─ operator_new(0xC98)   ← allocate
            ├─ BadBall_ctor(obj, board) ← initialize defaults
            ├─ set position from mesh data
            ├─ parse CHASE/HOME/SIZE/SPINDISTANCE tags
            └─ AthenaList_Append to ball + collision lists
```

8-balls, chrome balls, and funballs are all BADBALL objects with different mesh appearances.

## Level File → Race Mapping

| # | Race | MW File | Constructor | Custom Logic |
|---|------|---------|-------------|--------------|
| 0 | Warm-Up | level1 | 0x0040D1C0 | None |
| 1 | Beginner | levelcascade | 0x004110D0 | 8 bumpers |
| 2 | Intermediate | level2 | 0x0040D280 | None |
| 3 | Dizzy | level3 | 0x0040D390 | TarBubble scan |
| 4 | Tower | level4 | 0x0040D6D0 | FlagWaver (pennant) |
| 5 | Expert | level5 | 0x0040E190 | None |
| 6 | Odd | level6 | 0x0040EA90 | 3 launchers + chromeshadow |
| 7 | Wobbly | level7 | 0x0040F360 | None |
| 8 | Toob | level8 | 0x0040FA20 | 8 bumpers (before vtable[32]) |
| 9 | Sky | level9 | 0x00410830 | Pillars + Magnifier + camera |
| 10 | Up | levelup | 0x00411540 | Vacuum tubes (VAC-IN/OUT/VEC) |
| 11 | Master | level10 | 0x00411F60 | 4 bumpers + rotator removal + TarBubble + party |
| 12 | Neon | leveldark | 0x00416270 | Ball materials + SceneObjects |
| 13 | Glass | levelglass | 0x00417640 | Glass breakable flag |
| 14 | Impossible | levelimpossible | 0x00417F20 | None |

## Rotator System

Three constructor variants sharing `Rotator_Update(0x4606D0)`:

### Rotator_ctor_Impossible (0x435940)
- Used by: Toob "SPINNY", Impossible "ROTATOR"
- vtable: 0x4D5518, struct size: 0x1508 bytes
- Pos: +0x10D8/DC/E0, Angle: +0x10E8, Speed: +0x10EC
- Party mode: angle=0, speed=1.0 (spinning)
- Non-party: angle=-0.2, speed=0.0 (frozen at slight offset)
- Ball tracking list at +0x10F0 (via Rotator_AddBall)

### Rotator_ctor (0x4366F0)
- Used by: Sky "TRAPDOOR"
- vtable: 0x4D5708
- Pos: +0x10D4/D8/DC (different layout!)

### Rotator_Update (0x4606D0)
- Vertex buffer deformation engine
- Allocates dynamic buffer at sceneObj+0x10C8 (size = vertCount * 0x60)
- Copies mesh vertices every frame, alternating triangle winding
- Uploads via Font_RenderToTextureComplex (D3D vertex buffer upload)

### Rotator_RemoveAndFree (0x436FC0)
- Called for each rotator in Master Pipsqueak
- Sinks Y position by 40.0 (0x4CF484) — moves below floor
- Removes from render list (board+0x8B0 → +0x18)
- Removes from collision list (board+0x10EC)
- Frees child render object
- Guard flag at +0x10E5 prevents double-removal

## Ghidra Function Names (Session 11593)

| Address | Name |
|---------|------|
| 0x00461510 | Level_MeshWorldCtor |
| 0x00465080 | Level_RenderCtor |
| 0x0041C5B0 | Board_Setup |
| 0x0040BCA0 | CreateBadBalls |
| 0x0040AFE0 | BadBall_ctor |
| 0x00408390 | BadBall_Update |
| 0x00428B20 | App_StartPartyRace |
| 0x00437880 | MouseTrap_ctor |
| 0x00438750 | Mace_ctor |
| 0x00438C20 | Fan_ctor |
| 0x00434660 | SawBlade_ctor |
| 0x00434D70 | Bell_ctor |
| 0x0043BBC0 | Fallout_ctor |
| 0x0043B780 | Saw_ctor |
| 0x0043BE20 | Saw2_ctor |
| 0x0043E110 | NeonPlatform_ctor |
| 0x0043AD40 | Wavy_ctor |
| 0x00434E60 | Odd_Lifter_ctor |
| 0x00438290 | Trapdoor_ctor |
| 0x00402BF0 | Vec3_Copy |
| 0x00434AB0 | SawBlade_SetVariant |
| 0x00435440 | Wavy_Configure |
| 0x0040FC29 | Toob_CreateDynamicObjects_Inner |
| (renamed) | Popcylinder_ctor (was Platform_ctor) |
