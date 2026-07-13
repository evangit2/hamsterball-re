# Global Saw Spawner

## What This Does

Spawns a working Saw (from Toob Race / Level8) at the player's position on any level. Set `[SpawnSaw]` to 1 in Cheat Engine to trigger.

## How the Saw Works in Toob Race (Full Trace)

### 1. LevelBoard_Toob_ctor (0x41F4B0) — Board Setup

Loads five meshes for Toob Race:
- `Levels\Level8-Spinny` → board+0x436C (spinner objects)
- `Levels\Level8-Saw` → board+0x4370 (the saw blade mesh)
- `Levels\Level8-Fallout` → board+0x4374 (fallout zone)
- `Levels\Level8-Blockdawg1` → board+0x4378
- `Levels\Level8-Blockdawg2` → board+0x437C

Sets board name to "TOOB RACE", vtable to 0x4D0E78.

### 2. SAW Creation — Toob_CreateDynamicObjects_Inner (Toob Board Object Factory)

Part of the Toob board's object creation dispatch. When the engine encounters a "SAW1" object name in the MESHWORLD:

1. Calls `Level_FindObjectByName(scene, "SAWPATH")` → gets the path object
2. Calls `Saw_ctor (0x43B780)` with: board, position (X,Y,Z), mesh (board+0x4370), path
3. `AthenaList_Append(board+0x2578, saw)` — registers in update list
4. Stores saw at `board+0x4380` (Saw1 slot)

If the object is "SAW2" (hard difficulty only):
1. Calls `Level_FindObjectByName(scene, "SMALLSAWPATH")` → small saw path
2. Calls `SmallSaw_ctor (0x43BE20)` — same as Saw_ctor but sets different vtable (0x4D5CA0) and faster speed (0xC2C80000 = -100.0)
3. Stores at `board+0x4384` (Saw2 slot)

### 3. Saw_ctor — Saw_ctor (0x43B780)

```c
Saw_ctor(this, board, posX, posY, posZ, mesh, path) {
    Stands_ctor(this, mesh);           // base constructor
    this->vtable = 0x4D5578;           // Saw vtable
    this->board = board;               // +0x10D0
    this->posX = posX;                 // +0x10DC
    this->posY = posY;                 // +0x10E0
    this->posZ = posZ;                 // +0x10E4
    this->path = path;                 // +0x10D8 (SAWPATH)
    this->field_10EC = 0;             // path position counter
    this->field_10F0 = 0;             // Y offset
    this->visual = clone(mesh);        // +0x10D4
    this->field_10F4 = 2;             // initial state
    this->field_10FC = 0;             // timer
    Timer_Init(timer);
    SceneObject_CallUpdate(this);      // initial update
    SceneObject_CallRender(this);     // initial render
    this->field_110C = 0;             // active flag (inactive)
}
```

### 4. Saw_Update — FUN_0043B8E0 (vtable[0x0B], 0x43B8E0)

4-state movement FSM. Only runs when `saw+0x110C` (active flag) is set.

**States:**
- **State 0** (moving forward, descending):
  - `Gfx_ScaleZ(180.0)` — scale the saw blade
  - Increment path counter: `saw+0x10EC += 0.02f` (_DAT_004CF448)
  - `Path_GetPosition(path, &pos, counter)` — get position along SAWPATH
  - Calculate direction, normalize, set facing via `Gfx_RotateY`
  - Decrement Y offset: `saw+0x10F0 -= 2.75` (double _DAT_004D5C90)
  - When Y offset < -100.0 (float _DAT_004D0248) → **state 1** (pause, timer=2)

- **State 1** (pausing at bottom):
  - Count down timer (`saw+0x10F8 -= 1`)
  - When timer expires → **state 2** (ascending)
  - On entering state 2: plays "sawspeedy" sound (App+0x4E8)

- **State 2** (moving forward, ascending):
  - Same path following as state 0
  - Increment Y offset: `saw+0x10F0 += 3.75` (double _DAT_004D0178)
  - When Y offset > 0.0 (_DAT_004CF368) → **state 3** (pause, timer=1)

- **State 3** (pausing at top):
  - Count down timer
  - When timer expires → **state 0** (cycle repeats)

### 5. Activation — Toob Race vs Expert Race

**Toob Race** does NOT use `Saw_Activate`. The ToobBoard collision handler
(`0x410020`) activates the saw by **directly writing** `saw+0x110C = 1` (byte):

- **E:BRANCH(A)**: Sets `board+0x4380->0x110C = 1` (activates Saw1)
- **E:ALERTSAW2**: Sets `board+0x4384->0x110C = 1` (activates Saw2)

`Saw_Update` checks `saw+0x110C` at `0x43B8FB` (`MOV AL, byte [ESI+0x110C]`)
— if non-zero, the 4-state FSM runs.

**Expert Race** uses `Saw_Activate (0x434A50)` which sets `saw+0x1114` — a
DIFFERENT flag. This is the wrong flag for Toob Race.

### 6. Saw_AlertActivate — 0x434770 (Expert Race only)

Clears alert flag `saw+0x110D` and plays idle hum. Used by Expert Race,
NOT Toob Race.

### 7. Collision System — ToobBoard CollisionHandler (0x410020)

The Toob board's collision handler processes saw-related events:

- **E:ALERTSAW2**: Sets `board+0x4384->0x110C = 1` (activates Saw2)
- **E:BRANCH(A)**: Sets `board+0x4380->0x110C = 1` (activates Saw1)
- **N:SAWTEETH**: If `saw->field_10F4 == 0` (saw is in first-moving state):
  - Reads break direction from `saw+0x1100` (SAW1-BREAK position)
  - Normalizes and scales by 3.0
  - Sets ball velocity to knockback direction
  - Calls `Ball_ApplyTrajectory(ball)`
- **E:ACTIVATESAW1** (from ExpertCollisionEvents, 0x40E6A0):
  Calls `Saw_Activate(board+0x4370)` — the mesh pointer, NOT the saw object.
  Wait — this is `board+0x4370` which is the saw *mesh*, but `Saw_Activate` takes the saw *object*. Looking more carefully at the code, `board+0x4370` on the Toob board stores the mesh, but on the Expert board (which shares the collision handler), `board+0x4370` stores the saw *object*. The Toob board uses `board+0x4380` for the saw object.

### 8. Saw SetBreak — SawBlade_SetVariant (0x434AB0)

Sets the saw's break direction (knockback direction when ball touches teeth):
```c
void Saw_SetBreak(this, sawNumber) {
    this->field_10F8 = sawNumber;  // which saw (1 or 2)
    if (sawNumber == 1) {
        // Look up "SAW1-BREAK" position from MESHWORLD
        pos = AthenaHashTable_Lookup(scene, "SAW1-BREAK");
        this->breakPos = pos;     // +0x1100, +0x1104, +0x1108
    }
    if (sawNumber == 2) {
        this->field_10EC = 0xC2B40000;  // -90.0f (rotation offset)
        pos = AthenaHashTable_Lookup(scene, "SAW2-BREAK");
        this->breakPos = pos;
    }
}
```

## Key Functions

| Address | Name | Purpose |
|---|---|---|
| 0x41F4B0 | LevelBoard_Toob_ctor | Toob board setup, loads saw mesh |
| 0x40FC29 | ToobBoard_CreateObjects | Creates SAW1/SAW2 from MESHWORLD objects |
| 0x43B780 | Saw_ctor | Constructs saw object (Stands_ctor + vtable) |
| 0x43B8E0 | Saw_Update (vtable[0x0B]) | 4-state movement FSM |
| 0x434A50 | Saw_Activate | Sets active flag, plays startup sound |
| 0x434770 | Saw_AlertActivate | Sets alert flag, plays idle hum |
| 0x434AB0 | Saw_SetBreak | Sets knockback direction (SAW1-BREAK/SAW2-BREAK) |
| 0x410020 | ToobBoard_CollisionHandler | Handles N:SAWTEETH, E:BRANCH, E:ALERTSAW2 |
| 0x40E6A0 | ExpertCollisionEvents | Handles E:ACTIVATESAW1/2, E:ALERTSAW1/2 |
| 0x467BF0 | Path_GetPosition | Gets position along a path |
| 0x460530 | Level_FindObjectByName | Finds named object in scene |
| 0x4BA57B | operator_new | C++ malloc wrapper |
| 0x453810 | AthenaList_Append | Add item to linked list |
| 0x461510 | Level_MeshWorldCtor | Mesh loader (from file) |

## Saw Vtable (0x4D5578)

| Index | Offset | Address | Function |
|---|---|---|---|
| 0x00 | +0x00 | 0x43BE90 | scalar dtor |
| 0x0B | +0x2C | 0x43B8E0 | Update (movement FSM) |
| 0x18 | +0x60 | 0x43BCF0 | dtor |

## Saw Struct Layout (0x1118 bytes)

| Offset | Type | Field |
|---|---|---|
| +0x000 | ptr | vtable (0x4D5578) |
| +0x434 | ptr | collision object (from Stands_ctor) |
| +0x435 | float[3] | position (X, Y, Z) — base SceneObject |
| +0x437 | float | current X position |
| +0x438 | float | current Y position |
| +0x439 | float | current Z position |
| +0x43B | float | path position counter |
| +0x43C | float | Y offset (vertical movement) |
| +0x43D | int | FSM state (0-3) |
| +0x43E | int | pause countdown timer |
| +0x440 | float[3] | facing direction (rotation vector) |
| +0x10D0 | ptr | board pointer |
| +0x10D4 | ptr | visual mesh clone |
| +0x10D8 | ptr | SAWPATH object |
| +0x10DC | float[3] | spawn position (X, Y, Z) |
| +0x10EC | float | path progress |
| +0x10F0 | float | Y offset |
| +0x10F4 | int | sub-state / mode (init=2) |
| +0x10F8 | int | saw number (1 or 2) |
| +0x10FC | int | timer |
| +0x1100 | float[3] | break direction (SAW1-BREAK) |
| +0x110C | byte | active flag (0=inactive, 1=active) |
| +0x110D | byte | alert flag |
| +0x1114 | byte | activated flag (set by Saw_Activate) |

## Toob Board Vtable (0x4D0E78)

| Index | Offset | Address | Function |
|---|---|---|---|
| 0x00 | +0x00 | 0x425120 | dtor |
| 0x01 | +0x04 | 0x419C00 | Update (Scene_Update) |
| 0x12 | +0x48 | 0x40FA20 | SetupLevel (loads MESHWORLD) |
| 0x1D | +0x74 | 0x410020 | CollisionHandler |

## SmallSaw (SAW2)

SmallSaw_ctor (0x43BE20) calls Saw_ctor then overrides:
- vtable → 0x4D5CA0 (SmallSaw vtable)
- +0x1110 = 1 (small saw flag)
- +0x10F0 = 0xC2C80000 (-100.0, Y offset)
- +0x10F4 = 2 (state)
- +0x1114 = 0x3F800000 (1.0, active)

Uses `SMALLSAWPATH` instead of `SAWPATH`. Only spawns on hard difficulty.

## MESHWORLD Named Objects (Toob Race)

| Name | Type | Purpose |
|---|---|---|
| SAWPATH | path | Movement path for Saw1 |
| SMALLSAWPATH | path | Movement path for Saw2 (small saw) |
| SAW1-BREAK | position | Knockback direction when ball touches Saw1 teeth |
| SAW2-BREAK | position | Knockback direction when ball touches Saw2 teeth |
| N:SAWTEETH | collision | Collision trigger for saw teeth knockback |
| E:ACTIVATESAW1 | event | Activates Saw1 (sets active flag) |
| E:ACTIVATESAW2 | event | Activates Saw2 |
| E:ALERTSAW1 | event | Alerts Saw1 (plays idle hum) |
| E:ALERTSAW2 | event | Alerts Saw2 |
| E:BRANCH(A) | event | Activates Saw1 via branch path |

## CEA Usage

1. Enable the script in Cheat Engine
2. Set `[SpawnSaw]` to 1 to spawn a saw at the player's position
3. The saw loads the `Levels\Level8-Saw` mesh, follows any SAWPATH found in the scene (if available), and is activated immediately
4. The flag auto-resets to 0 after each spawn
5. On non-Toob levels: the saw will render and spin but won't move (no SAWPATH). Collision with saw teeth won't work either (ToobBoard collision handler is needed).
