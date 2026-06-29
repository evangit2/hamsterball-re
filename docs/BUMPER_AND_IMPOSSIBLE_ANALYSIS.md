# Global Bumper & Impossible Objects — Reverse Engineering Analysis

## 1. Bumpers in Beginner Race

### What Are Bumpers?
Bumpers are bouncy collision objects embedded in MESHWORLD geometry. They are NOT standalone game objects with their own vtable — they are **mesh triangles tagged with `N:BUMPER%d` event names** in the MESHWORLD octree (Section 6).

### Where Do They Come From?
- **Beginner Race** (`LevelCascade.MESHWORLD`): Contains `N:BUMPER1` through `N:BUMPER6` ref points
- **Toob Race** (`Level8.MESHWORLD`): Contains `N:BUMPER1` through `N:BUMPER8`
- The bumper *visual mesh* comes from the level's own MESHWORLD file
- The bumper *collision event* is processed by the board's collision dispatch

### How Are Bumpers Created?

#### Step 1: Board Initialization
```
BoardLevel_Beginner_Ctor (0x004200E0)
  → Board_ctor(this, param_1)
  → Sets vtable = 0x4D1098 (Beginner board)
  → LoadRaceData(this, "CASCADERACE")
  → vtable[0x12] = Scene_SetupLevelCascade (0x004110D0)
```

#### Step 2: Scene Setup (vtable[0x12] call)
```
Scene_SetupLevelCascade (0x004110D0)
  1. operator_new(0x10D0) → allocates MeshWorld
  2. MeshWorld_ctor(mesh, App+0x174, "levels\levelcascade")
     → Parses LevelCascade.MESHWORLD binary
     → Builds octree with all geometry including N:BUMPER%d tagged triangles
  3. Stores mesh at Board+0x8AC (param_1[0x22B])
  4. operator_new(0x10D0) → allocates CollisionLevel
  5. CollisionLevel_ctorWithLevel(colLevel, mesh)
     → Clones collision geometry from mesh
  6. Stores at Board+0x8B0 (param_1[0x22C])
  7. Level_InitScene(board)
     → Iterates Section 1 ref points
     → For each ref, dispatches to CreateLevelObjects/CreateMechanicalObjects/CreateSpeedCylinder
     → N:BUMPER%d refs do NOT match any factory → fall through to CreatePlatformOrStands
     → But the bumper TRIANGLES are already in the octree from MeshWorld_ctor
  8. vtable[0x80]() — post-init callback
  9. Loops 8 times (i=1..8):
     - AthenaString_Format("N:BUMPER%d", i) → generates "N:BUMPER1", "N:BUMPER2", etc.
     - Scene_CollectByNameFilter(mesh, nameFilter, &bumperSlot[i])
       → Searches the MeshWorld octree for triangles tagged with this event name
       → Stores collected triangle groups in Board+0x43E4 + i*0x418 (bumper slot array)
     - Sets Board+0x642C + i*4 = 0 (bumper activation state)
```

**Key insight:** Bumpers are NOT created by a factory function. They are **pre-baked into the level's MESHWORLD file** as tagged collision triangles. The `Scene_CollectByNameFilter` call gathers them into slot arrays for the collision system.

#### Step 3: Collision Dispatch (vtable[0x1D])
When the ball collides with any triangle in the level:
```
Beginner Collision Dispatch (0x004111E0) — vtable[0x1D]
  1. Checks if event name starts with "N:BUMPER"
  2. If yes:
     a. Sound_Play3D — plays bumper bounce sound at ball position
     b. Reads ball velocity from Ball+0xCA4 (velX), +0xCA8 (velY), +0xCAC (velZ)
     c. Scales velocity by _DAT_004CF41C (a float multiplier)
     d. If speed < threshold: Vec3_NormalizeAndScale(&vel, 5.0) — small bounce
     e. If speed > threshold: Vec3_NormalizeAndScale(&vel, 10.0) — big bounce
     f. Writes scaled velocity back to Ball+0xCA4/+0xCA8/+0xCAC
     g. Reads bumper index from event name: _atol(name + 8) → "N:BUMPER3" → index 3
     h. Sets Board+0x6428 + index*4 = 1.0 (0x3F800000) — bumper activation flag
  3. Falls through to DispatchCollisionEvents (0x0040C5D0) for shared event handling
```

### Bumper Data Structure
| Offset | Type | Description |
|--------|------|-------------|
| Board+0x43E4 + i*0x418 | struct | Bumper slot[i] (collected triangle groups) |
| Board+0x6428 + i*4 | float | Bumper activation state (0=inactive, 1=just hit) |
| Board+0x642C + i*4 | int | Bumper state flag |

### Why Bumpers Are Hard to Clone Globally
Bumpers are **embedded collision triangles**, not standalone objects. To spawn them globally:
1. You must load the Level8.MESHWORLD (or LevelCascade.MESHWORLD) file which contains the bumper geometry
2. Create a Stands object from that mesh (clones the octree including collision triangles)
3. Position it at the player
4. Register it in the board's collision lists (Board+0x10EC and Board+0x8B0→+0x18)

The collision dispatch (vtable[0x1D]) automatically handles "N:BUMPER" events regardless of which board type you're on, because ALL boards inherit from the same base collision dispatch that calls `DispatchCollisionEvents`.

---

## 2. Impossible Race Objects

### Overview
Impossible Race (`LevelImpossible.MESHWORLD`) contains 5 types of mechanical objects, each loaded from a separate sub-MESHWORLD file:

| Object | Mesh File | String Address | Board Slot | Alloc Size | Ctor Address | RET N |
|--------|-----------|---------------|------------|------------|-------------|-------|
| LOOPER | LevelImpossible-Looper | 0x4D2158 | +0x436C | 0x1500 | 0x435800 | 0x14 (5 params) |
| GEAR | LevelImpossible-Gear | 0x4D213C | +0x4370 | 0x1514 | 0x437590 | 0x20 (8 params) |
| BIGGEAR | LevelImpossible-BigGear | 0x4D211C | +0x4374 | 0x1514 | 0x437590 | 0x20 (8 params) |
| ROTATOR | LevelImpossible-Rotator | 0x4D20FC | +0x4378 | 0x1508 | 0x435940 | 0x14 (5 params) |
| PENDULUM | LevelImpossible-Pendulum | 0x4D20DC | +0x437C | 0x1504 | 0x436A20 | 0x18 (6 params) |

### How They Are Created

#### Step 1: Board_Impossible_ctor (0x00424C20)
```c
Board_ctor(this, param_1);
this->vtable = 0x4D21C0;  // Impossible board
this->name = "Board (Impossible)";
this->race_name = "IMPOSSIBLE RACE";
LoadRaceData(this, "IMPOSSIBLERACE");

// Pre-load 5 sub-meshes into board slots:
this->mesh_Looper   = MeshWorld_ctor(new(0x10D0), App+0x174, "Levels\LevelImpossible-Looper");   // +0x436C
this->mesh_Gear     = MeshWorld_ctor(new(0x10D0), App+0x174, "Levels\LevelImpossible-Gear");     // +0x4370
this->mesh_BigGear  = MeshWorld_ctor(new(0x10D0), App+0x174, "Levels\LevelImpossible-BigGear");  // +0x4374
this->mesh_Rotator  = MeshWorld_ctor(new(0x10D0), App+0x174, "Levels\LevelImpossible-Rotator");  // +0x4378
this->mesh_Pendulum = MeshWorld_ctor(new(0x10D0), App+0x174, "Levels\LevelImpossible-Pendulum"); // +0x437C
```

#### Step 2: LevelImpossible_InitScene (0x00417F20) — vtable[0x12]
```c
mesh = MeshWorld_ctor(new(0x10D0), App+0x174, "levels\levelimpossible");
board->sceneMesh = mesh;        // +0x8AC
board->collisionMesh = CollisionLevel_ctorWithLevel(new(0x10D0), mesh); // +0x8B0
Level_InitScene(board);         // Creates dynamic objects from MESHWORLD refs
board->vtable[0x80]();         // Post-init
```

#### Step 3: Scene_CreateDynamicObjects (0x0040C430)
Iterates all Section 1 ref points in the MESHWORLD octree. For each ref, calls the board's vtable[0x84]:
```
vtable[0x84](refName, &outObj, &outCol, refParams)
```
This dispatches to:
- **CreateLevelObjects** (0x004121D0): BRIDGE, TIPPER, BONK, BBRIDGE1-2, POPCYLINDER, BLOCKDAWG1-2, CATAPULT, GLUEBIE
- **CreateMechanicalObjects** (0x00417FE0): LOOPER, GEAR, BIGGEAR, ROTATOR, PENDULUM
- **CreateSpeedCylinder** (0x004117B0): LIFTER, SPEEDCYLINDER, TIMEBUTTON
- **Scene_CreateObject_Gear** (0x00418760): GEAR (arena variant)
- **CreatePlatformOrStands** (0x004133E0): Fallback for anything else

#### Step 4: CreateMechanicalObjects (0x00417FE0) — Detailed

For each ref name, uses `__strnicmp` to match:

**LOOPER** (strnicmp "LOOPER", 6):
```c
obj = operator_new(0x1500);
Looper_ctor(obj, board, posX, posY, posZ, board->mesh_Looper);  // +0x436C
collisionLevel = obj->CollisionLevel;  // +0x10D4
AthenaList_Append(board+0x2578, obj);  // general objects
```

**GEAR** (strnicmp "GEAR", 4):
```c
obj = operator_new(0x1514);
Gear_ctor(obj, board, posX, posY, posZ, dirX, dirY, dirZ, board->mesh_Gear);  // +0x4370
AthenaList_Append(board+0x2578, obj);
```

**BIGGEAR** (strnicmp "BIGGEAR", 7):
```c
obj = operator_new(0x1514);
Gear_ctor(obj, board, posX, posY, posZ, dirX, dirY, dirZ, board->mesh_BigGear);  // +0x4374
obj->scale = 0.5;       // +0x10F4 = 0x3F000000
if (strstr(name, "TOUCH")) obj->touchFlag = 1;  // +0x1510
AthenaList_Append(board+0x2578, obj);
```

**ROTATOR** (strnicmp "ROTATOR", 7):
```c
obj = operator_new(0x1508);
Rotator_ctor(obj, board, posX, posY, posZ, board->mesh_Rotator);  // +0x4378
obj->spinDir = 1.0;     // +0x10E8 = 0x3F800000
if (RNG_Rand(0, 2, 0) == 0) obj->spinDir = -1.0;  // 0xBF800000 — 50% chance reverse
AthenaList_Append(board+0x2578, obj);
```

**PENDULUM** (strnicmp "PENDULUM", 8):
```c
obj = operator_new(0x1504);
Pendulum_ctor(obj, board, posX, posY, posZ, phase, board->mesh_Pendulum);  // +0x437C
collisionLevel = obj->CollisionLevel;  // +0x10D4
AthenaList_Append(board+0x2578, obj);
```

#### Step 5: Constructor Internals

All 5 ctors follow the same pattern:
```c
// 1. Call SceneObject_ctor(this, mesh)
//    → SpriteAnim_Ctor, sets vtable=Level_DeletingDtor, Timer_Init
//    → Clones SpatialTree from mesh (geometry + collision)
//    → AthenaList_Init on +0x18, +0x488, +0x8A0, +0xCB8

// 2. Set object-specific vtable:
//    Looper  → vtable = 0x4D54B8 (PopCylinder_DeletingDtor)
//    Gear    → vtable = 0x4D5AD0 (Gear_Vec3List_DeletingDtor)
//    Rotator → vtable = 0x4D5518 (Lifter_DeletingDtor)
//    Pendulum→ vtable = 0x4D57D0 (Pendulum_DeletingDtor2)

// 3. AthenaList_Init on object-specific list (+0x10E8/+0x10F0/+0x10F8)

// 4. Set board pointer: obj+0x10D0 = board

// 5. Set position: obj+0x10D8/+0x10DC/+0x10E0 = posX/posY/posZ

// 6. Create CollisionLevel:
//    operator_new(0x10D0) → CollisionLevel_ctorWithLevel(col, this)
//    obj->CollisionLevel = col  (+0x10D4)
//    col->field_434 = obj->field_434  (difficulty flag propagation)
//    col->field_431 = 0  (collision enabled)
```

### Impossible Race Collision Dispatch (0x00418360)

When the ball touches any collision triangle:
```c
if (ball->is_active) {  // ball+0x2E8 (param_2[0x1DA] in Ghidra int* indexing = 0x768)
    // N:BOUNCE — bounce physics (same as Impossible Gear bumpers)
    if (strnicmp(event, "N:BOUNCE", 8) == 0) {
        // Scale velocity, normalize, apply bounce force
    }
    // N:ONROTATOR — score tracking
    if (strnicmp(event, "N:ONROTATOR", 11) == 0) {
        ScoreObject_SetScore(board->scoreObj, ball);
    }
    // N:ONGEAR — catapult-style object attachment
    if (strnicmp(event, "N:ONGEAR", 8) == 0) {
        Catapult_AddObjectConditional(board->scoreObj, ball);
    }
    // E:HELPINERTIA — increase ball inertia (0x40200000 = 2.5)
    if (stricmp(event, "E:HELPINERTIA") == 0) {
        ball->inertia = 0x40200000;  // +0x2A4 (param_2[0xA9])
    }
    // E:UNHELPINERTIA — decrease ball inertia (0x40A00000 = 5.0)
    if (stricmp(event, "E:UNHELPINERTIA") == 0) {
        ball->inertia = 0x40A00000;
    }
}
// Always calls shared handler:
DispatchCollisionEvents(board, ball, collision);
```

### DispatchCollisionEvents (0x0040C5D0) — Shared Event Handler
Processes ALL common collision events:
- `N:SECRET` → Rotator_MarkTriggered
- `N:UNLOCKSECRET` → CheckArenaUnlock
- `E:NODIZZY<TIME>N</TIME>` → Ball_RecordBest (TIME checkpoint clear — NOT dizzy-related)
- `E:SAFESWITCH` → copy switch data to ball+0xC2C
- `E:LIMIT` → track arena completions
- `E:JUMP` → play sound + SetForce(0.1, 1) + impact=10
- `E:ACTION(ONCE/SCORE)` → score tracking
- `E:TRAJECTORY(X,Y,Z)` → set ball trajectory
- `N:WATER` → water flag + timer=10
- `N:TARPIT` → tar sound + tar flag
- `N:GOAL` → finish race
- `DROPIN` → sound + score + 200
- `PIPEBONK` → random sound + score + 100
- `POPOUT` → sound + score + 100
- `N:MOUSETRAP` → deflect + rotator collision

---

## 3. Global Spawn Pattern

Both CEA scripts use the same proven pattern as the SpeedCylinder spawn script:

### Hook Point
```
Ball_Update @ 0x00405E22 (original: mov eax, [esi+0x0C5C])
→ jmp SpawnCode
```
- `esi` = Ball pointer
- `ball+0x14` = Board pointer
- `ball+0x18` = state (0 = active racing)
- `ball+0x164/+0x168/+0x16C` = position X/Y/Z

### Spawn Flow
1. Save player position every frame (when `ball+0x18 == 0`)
2. Check spawn flag (set via CE address list)
3. If flag set: `pushad` → spawn → `popad`
4. Spawn process:
   a. Get board from `ball+0x14`
   b. Get App+0x174 (D3D device) from `board+0x878`
   c. `operator_new(0x10D0)` → `MeshWorld_ctor(mesh, D3D, "filename")`
   d. `operator_new(objSize)` → `Object_ctor(obj, board, posX, posY, posZ, mesh)`
   e. Set position at `obj+0x10D8/+0x10DC/+0x10E0`
   f. `AthenaList_Append(board+0x2578, obj)` — general objects
   g. `AthenaList_Append(board+0x10EC, collisionLevel)` — collision registration
   h. `AthenaList_Append(board+0x8B0→+0x18, collisionLevel)` — spatial tree
   i. `Timer_Init` → `vtable[0x58]` (CallUpdate) → `vtable[0x54]` (CallRender)
   j. Track in array for board-change cleanup

### Key Addresses
| Address | Function |
|---------|----------|
| 0x004BA57B | operator_new |
| 0x00461510 | MeshWorld_ctor (RET 0x8) |
| 0x00465080 | CollisionLevel_ctorWithLevel (RET 0x4) |
| 0x00462850 | SceneObject_ctor (RET 0x4) |
| 0x00453810 | AthenaList_Append |
| 0x00453210 | AthenaList_Init (RET 0x4) |
| 0x00457AD0 | Timer_Init |
| 0x00457A40 | Timer_Cleanup |
| 0x0040B090 | Level_InitScene |

### String Constants
| Address | String |
|---------|--------|
| 0x4CFCA4 | "levels\level8" |
| 0x4D2158 | "Levels\LevelImpossible-Looper" |
| 0x4D213C | "Levels\LevelImpossible-Gear" |
| 0x4D211C | "Levels\LevelImpossible-BigGear" |
| 0x4D20FC | "Levels\LevelImpossible-Rotator" |
| 0x4D20DC | "Levels\LevelImpossible-Pendulum" |

### Ctor Calling Conventions (verified via RET N)
| Ctor | RET N | Stack Params | Signature |
|------|-------|-------------|----------|
| Looper_ctor (0x435800) | 0x14 | 5 | (this, board, posX, posY, posZ, mesh) |
| Gear_ctor (0x437590) | 0x20 | 8 | (this, board, posX, posY, posZ, dirX, dirY, dirZ, mesh) |
| Rotator_ctor (0x435940) | 0x14 | 5 | (this, board, posX, posY, posZ, mesh) |
| Pendulum_ctor (0x436A20) | 0x18 | 6 | (this, board, posX, posY, posZ, phase, mesh) |
| SceneObject_ctor (0x462850) | 0x4 | 1 | (this, mesh) |
| MeshWorld_ctor (0x461510) | 0x8 | 2 | (this, D3D, filename) |
