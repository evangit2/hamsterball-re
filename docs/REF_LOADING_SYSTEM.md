# Hamsterball Reference Node (Ref) Loading System

## Overview

Hamsterball levels load "reference nodes" (refs) from MESHWORLD files. Each ref has a name (e.g. `SPEEDCYLINDER`, `BONK`, `GEAR`) that determines what game object gets created. The game uses a **multi-pass code-gated dispatch system** — not a file-driven registry — to decide which refs to instantiate. The full pipeline runs in the master level loading function (~0x0041C8xx), which makes sequential passes over the ref point list.

**Key finding:** Adding a ref name to a MESHWORLD file is **necessary but not sufficient**. The ref must ALSO be handled by one of the dispatch passes (SAFESPOT extractor, SIGN handler, or vtable[33] factory), or the game will silently skip it. To load any object ref into any level, you must patch the Board's vtable[33] to point to a factory that handles the desired ref names, since each level's factory only recognizes its own subset of ref names.

### Complete Loading Pipeline

```
Board constructor
  ├─ 1. Creates LevelData (board+0x8AC, size 0x10D0) with MESHWORLD file path
  │     LevelData parses MESHWORLD binary into AthenaLists:
  │     • Section 1 ref points → LevelData+0x480+0x894
  │     • Section 3 scene objects → mesh entities with name at +0x864
  │
  └─ 2. Loads sub-mesh MESHWORLD files into board+0x436C..0x4390

Master loading function (~0x0041C8xx)
  ├─ 3a. Iterates ref points, extracts SAFESPOT and START-DEBUG refs
  ├─ 3b. Conditional quality-dependent setup (scene+0x237 byte)
  ├─ 3c. Additional setup (0x0040BAA0, 0x0040C0F0)
  ├─ 3d. Scene_CreateSigns (0x0040C270) — "SIGN" prefix refs → StandsTipper objects
  ├─ 3e. Scene_CreateDynamicObjects (0x0040C430) — object refs via vtable[33] factory
  │     ├─ Factory matches ref name via __strnicmp
  │     ├─ Creates game object (Pendulum, Bonk, Gear, etc.)
  │     ├─ Appends to board+0xCD4 and scene object lists
  │     └─ Factory calls N: handler (0x0040C5D0) on created mesh entities
  └─ 3f. N: handler processes entity names from Section 3:
        • N:GOAL, N:TARPIT, N:WATER, N:SECRET, E:JUMP, E:BREAK, etc.
        • Sets behavioral flags on board/scene/mesh
```

---

## Architecture

### 1. MESHWORLD Ref Points (Section 1)

Each MESHWORLD file contains a Section 1 with "reference points" — named position markers stored as:
- `name` (string, e.g. `SPEEDCYLINDER`, `BONK`, `GEAR` — stored WITHOUT `N:` prefix)
- `position` (3 floats: x, y, z at offsets +0x04, +0x08, +0x0C)
- `rotation/scale` data (at offset +0x10)
- `extra float` (at offset +0x14, used for scale/size calculations)
- Additional flags and color data

The ref points are parsed from the MESHWORLD binary and stored in an `AthenaList` at:
- `board+0x8AC` → LevelData object (0x10D0 bytes, created in Board constructor)
- `LevelData+0x480` → SceneObject
- `SceneObject+0x894` → AthenaList of ref points (count at +0x898, array at +0x40C)

Each ref entry in the array has the structure:
```
+0x00: char* name          // pointer to ref name string
+0x04: float posX          
+0x08: float posY
+0x0C: float posZ
+0x10: rotation/scale data
+0x14: float extraScale    // used in Timer_Init and position calculations
```

### 2. Scene_CreateDynamicObjects (0x0040C430) — The Central Dispatch

```
void __thiscall Scene_CreateDynamicObjects(int *board)  // ECX = board
```

This function iterates over all ref points in the loaded MESHWORLD level data. For each ref:

1. Reads the ref name string from `ref_entry+0x00` (`*puVar4`)
2. Reads position/scale data from `ref_entry+0x04` through `ref_entry+0x14`
3. Calls `board->vtable[33](refName, &out_obj, &out_col, ref_entry)` — i.e. `(**(code**)(*board + 0x84))(name, &out1, &out2, refPtr)` at address `0x0040C4BA`
4. If the factory returns a non-null object (`out_obj != NULL`):
   - Initializes a `Timer` (via `Timer_Init` at 0x00457AD0) using a float from `ref_entry+0x14`
   - Sets graphics position/scale from the ref entry position data
   - Appends the object to `board+0xCD4` (board-specific object list) via `AthenaList_Append` at 0x00453810
   - Appends to the scene's SceneObject list (`LevelData+0x480+0x1C`)
   - Calls the object's `vtable[0x58]` (Update setup) at 0x0040C531
   - Calls the object's `vtable[0x54]` (Render setup) with the timer at 0x0040C53F
   - If a collision object was returned, appends it to `board+0x10EC` and `board+0x8B0+0x18`

**Ref entry structure** (param_5 / puVar4):
- `[0x00]` = name string pointer (char*)
- `[0x04],[0x08],[0x0C]` = position (x, y, z as floats)
- `[0x10]` = rotation data
- `[0x14]` = extra float parameter (used by Timer_Init and some factories)

### 3. Board Vtable Slot 33 (+0x84) — The Factory Method

Each Board subclass overrides vtable[33] with its own factory function. This function receives:
- `this` = Board pointer
- `param_2` (char*) = ref name string (e.g. "SPEEDCYLINDER", "BONK", "GEAR")
- `param_3` (out) = pointer to receive created visual object
- `param_4` (out) = pointer to receive created collision object
- `param_5` (int) = pointer to the full ref entry (for position/scale extraction)

The factory uses `__strnicmp(refName, "KEYWORD", len)` to match ref names to object constructors. **Unmatched refs are silently ignored** — the factory simply returns null pointers.

### 4. The `N:` Prefix — TWO Separate Systems

There are **two independent dispatch systems** for MESHWORLD data:

**System A: Ref Points (Section 1) → vtable[33] Factory Dispatch**
- Ref names stored WITHOUT `N:` prefix: `SPEEDCYLINDER`, `BONK`, `GEAR`, `LIFTER`, etc.
- Processed by `Scene_CreateDynamicObjects` → `board->vtable[33]` factory
- Factory uses `__strnicmp(refName, "SPEEDCYLINDER", len)` — bare name comparison
- Creates game OBJECTS (visual + collision entities)

**System B: Entity Names (Section 3) → N:/E: Prefix Handler (0x0040C5D0)**
- Entity names stored WITH `N:` or `E:` prefix: `N:GOAL`, `N:TARPIT`, `E:JUMP`, `E:BREAK`
- Entity name is stored at `mesh_entity+0x864` during `Level_LoadMeshes` / `CreateMeshBuffer`
- Processed by the N:/E: handler at `0x0040C5D0`, which is called from within the vtable[33] factories
- Sets behavioral FLAGS on the board/scene (e.g. tar pit behavior, water effects, jump zones, breakable surfaces)
- Does NOT create objects — modifies existing mesh entities

**The N: handler (0x0040C5D0)** processes these prefixes:
- `N:SECRET`, `N:UNLOCKSECRET` — secret level unlock flags
- `N:GOAL` — goal/finish marker
- `N:TARPIT` — tar pit collision behavior
- `N:WATER` — water surface effect
- `N:NOCONTROL` — disables ball control in this area
- `N:BRIDGE`, `N:SWIRL`, `N:WHEELEMBED`, `N:WATERWHEEL` — mesh behavior modifiers
- `N:MACE`, `N:TRAPDOOR` — object-specific behaviors
- `N:JUMPFIRST`, `N:JUMPSECOND` — jump pad sequencing
- `N:WAVY`, `N:SQUAREWOBBLY` — wavy surface behavior
- `N:BUMPER`, `N:BUMPER%d` — bumper assignment
- `N:SAWTEETH`, `N:SPINNY` — saw/spinner behavior
- `N:EXTRATIME`, `N:SPEEDCYLINDER` — time/speed zone markers
- `N:SPINNER`, `N:NEONPLATFORM` — platform behavior
- `N:BUMP`, `N:TENBONUS1`, `N:TENBONUS2` — bonus point markers
- `N:GLASS` — breakable glass surface
- `N:ONGEAR`, `N:ONROTATOR` — gear/rotator attachment
- `N:BOUNCE` — bounce surface
- `N:MOUSETRAP` — mouse trap behavior
- `E:JUMP`, `E:BREAK`, `E:ACTION`, `E:LIMIT`, `E:TRAJECTORY` — event triggers
- `ONCE`, `TRUE`, `SCORE`, `X`, `Y`, `Z`, `PIPEBONK`, `POPOUT`, `ZIP` — modifiers

The handler is called from 25 sites within the vtable[33] factory functions. Each factory calls the handler after creating the object, passing the mesh entity as `this` (ECX) and two additional arguments. The handler reads the entity name from `mesh_entity+4 → +0x864` (the entity name set during `CreateMeshBuffer` when parsing MESHWORLD Section 3).

**Call site example** (at 0x0040D380, Dizzy factory):
```asm
MOV EAX, [ESP+0x0C]    ; load ref entry from stack
PUSH EDI               ; push board pointer (param_3)
PUSH EAX               ; push ref entry (param_2)
MOV ECX, ESI           ; ECX = mesh entity (this for __thiscall)
CALL 0x0040C5D0        ; N: handler(mesh_entity, ref_entry, board)
```

### 5. The `Scene+0x23C` Quality Gate

Many (but not all) factory branches check `*(int *)(*(int *)(board + 0x878) + 0x23c) != 0` before creating objects. This field is the **graphics quality setting** — objects like TIPPER, BONK, MACE, SAW, SAW2, BLOCKDAWG, GEARS, etc. are only created when quality is non-zero (high quality). Objects like BRIDGE, SPEEDCYLINDER, LIFTER, POPCYLINDER, TRAPDOOR are created regardless of quality.

---

## Race vs Arena — Two Separate Board Systems

Hamsterball has **TWO independent Board systems**: one for Race mode and one for Arena mode. Each uses different Board constructors, different vtables, and different vtable[33] factories.

### RACE Board System (0x422xxx constructors)

Race Board constructors are at `0x4224A0`–`0x424EC0`. Each is called from a jump table at `0x426AB0` (15 entries, indexed by `level_number - 1`). The jump table is reached via `JMP [EAX*4 + 0x426AB0]` at `0x4267A0`.

Race Board vtables are at `0x4D1428`–`0x4D2298`. The Race factories form an **inheritance chain**: each level-specific factory checks its own refs, then falls through to call the base factory `0x4133E0` (which handles `PLATFORM` and `STANDS`).

| # | Race Level | Board Vtable | Factory Addr | Refs Handled (factory-specific) |
|---|-----------|-------------|-------------|-------------------------------|
| 1 | Warm-up | 0x4D1428 | 0x4133E0 | PLATFORM, STANDS (base only) |
| 2 | Beginner | 0x4D14F0 | 0x4133E0 | PLATFORM, STANDS (base only) |
| 3 | Intermediate | 0x4D15C0 | 0x4133E0 | PLATFORM, STANDS (base only) |
| 4 | Dizzy | 0x4D1680 | 0x4143D0 | SPINNY, MACE, CATAPULT, TURRET, LIFTER + base |
| 5 | Tower | 0x4D1740 | 0x414680 | MACE, CATAPULT, TURRET, LIFTER, FAN, E:GRAVITY + base |
| 6 | Up | 0x4D17F8 | 0x414A20 | LIFTER, FAN, E:GRAVITY, N:BUMPER + base |
| 7 | Neon | 0x4D1EC8 | 0x4173B0 | FLICKNING, N:BUMP, N:GLASS, N:TENBONUS1-2 + base |
| 8 | Expert | 0x4D18C8 | 0x414BD0 | FAN, E:GRAVITY, N:BUMPER + base |
| 9 | Odd | 0x4D1980 | 0x4133E0 | PLATFORM, STANDS (base only) |
| 10 | Toob | 0x4D1A40 | 0x4133E0 | PLATFORM, STANDS (base only) |
| 11 | Wobbly | 0x4D1B18 | 0x415460 | WOBBLY1, N:SQUAREWOBBLY, PILLAR, POPCYLINDER, EDGECYLINDER + base |
| 12 | Glass | 0x4D2048 | 0x4133E0 | PLATFORM, STANDS (base only) |
| 13 | Sky | 0x4D1BD8 | 0x415A30 | POPCYLINDER, EDGECYLINDER, E:LAUNCH + base |
| 14 | Master | 0x4D1C80 | 0x4133E0 | PLATFORM, STANDS (base only) |
| 15 | Impossible | 0x4D2298 | 0x418760 | GEAR + base |

**Key insight**: Levels 1, 2, 3, 9, 10, 12, 14 use ONLY the base factory — they handle only `PLATFORM` and `STANDS` in race mode. Their level-specific objects (tippers, bridges, etc.) are NOT created by vtable[33] in race mode — they are either part of the static level geometry or loaded via the Arena Board system.

### ARENA Board System (0x41Cxxx constructors)

Arena Board constructors are at `0x41CB20`–`0x424C20`. Each is called from a sequential switch at `0x427140` (sequential `MOV [ESP+0x18], level_id; JZ skip; CALL constructor`).

Arena Board vtables are at `0x4D05A0`–`0x4D21C0`. The Arena factories handle **many more ref types** than Race factories — they create ALL interactive objects for Arena mode.

| # | Arena Level | Board Vtable | Factory Addr | Refs Handled |
|---|-----------|-------------|-------------|--------------|
| 1 | Warm-up Arena | 0x4D1098 | 0x419750 | (none — NoOp) |
| 2 | Beginner Arena | 0x4D05A0 | 0x40A550 | BRIDGE, TIPPER, WATERWHEEL, SWIRL, GLUEBIE, SMASHER1-2 |
| 3 | Intermediate Arena | 0x4D0890 | 0x40A5F0 | TIPPER, WATERWHEEL, SWIRL, GLUEBIE, SMASHER1-2 |
| 4 | Dizzy Arena | 0x4D0A08 | 0x40D7C0 | CATAPULT, MACE, DRAWBRIDGE, WINDMILL, TRAPDOOR, CHOMPER, TURRET, BONK, FAN |
| 5 | Tower Arena | 0x4D11A0 | 0x4117B0 | LIFTER, SPEEDCYLINDER, TIMEBUTTON, TarBubble, BRIDGE, TIPPER, BONK |
| 6 | Up Arena | 0x4D1DF0 | 0x416910 | NEONPLATFORM, DFLOOR1-4, TRODE, N:NEONPLATFORM, E:ZOOP, E:LIGHTSOFF, E:LIGHTSON, FLICKNING, N:BUMP |
| 7 | Expert Arena | 0x4D0B00 | 0x40E250 | BONK, FAN, SAWBLADE, BRIDGE, JUDGE, BELL, E:SCORE, E:BELL, LIFTER, E:GRAVITY |
| 8 | Odd Arena | 0x4D0BC0 | 0x40EC40 | LIFTER, E:GRAVITY, WOBBLY1-5, WAVY1 |
| 9 | Toob Arena | 0x4D0E78 | 0x40FB30 | SPINNY, FALLOUT1, BLOCKDAWG1-3, E:ALERTSAW2, E:BRANCH, N:SPINNY, N:SAWTEETH, N:BUMPER |
| 10 | Glass Arena | 0x4D0D38 | 0x40F420 | WOBBLY1-7, WAVY1, N:SQUAREWOBBLY, N:WAVY, SPINNY, FALLOUT1, BLOCKDAWG1-3 |
| 11 | Wobbly Arena | 0x4D1F90 | 0x40AD80 | SMASHER1, SMASHER2 |
| 12 | Sky Arena | 0x4D0FC8 | 0x410AD0 | POPCYLINDER, TRAPDOOR, N:BUMPER, VAC-in |
| 13 | Master Arena | 0x4D12B0 | 0x4121D0 | BRIDGE, TIPPER, BONK, BBRIDGE1-2, POPCYLINDER, BLOCKDAWG1-2, CATAPULT, GLUEBIE, N:SPINNER, N:BUMPER, E:LAUNCH |
| 14 | Impossible Arena | 0x4D21C0 | 0x417FE0 | LOOPER, GEAR, BIGGEAR, ROTATOR, PENDULUM, N:BOUNCE, N:ONROTATOR, N:ONGEAR |

### Level File → Board Mapping

The game's internal level numbers map to file paths differently for race vs arena:

| Race Level | File Path | Arena Level | File Path |
|-----------|----------|-----------|----------|
| L1 (Warm-up) | levels\level1 | L1 (Warm-up Arena) | levels\arena1 |
| L2 (Beginner) | levels\levelcascade | L2 (Beginner Arena) | levels\arena2 |
| L3 (Intermediate) | levels\level3 | L3 (Intermediate Arena) | levels\arena3 |
| L4 (Dizzy) | levels\level4 | L4 (Dizzy Arena) | levels\arena4 |
| L5 (Tower) | levels\level5 | L5 (Tower Arena) | levels\arena5 |
| L6 (Up) | levels\levelup | L6 (Up Arena) | levels\arena6 |
| L7 (Neon) | levels\leveldark | L7 (Neon Arena) | levels\arena7 |
| L8 (Expert) | levels\level8 | L8 (Expert Arena) | levels\arena8 |
| L9 (Odd) | levels\level9 | L9 (Odd Arena) | levels\arena9 |
| L10 (Toob) | levels\level10 | L10 (Toob Arena) | levels\arena10 |
| L11 (Wobbly) | levels\level7 | L11 (Wobbly Arena) | levels\arena11 |
| L12 (Glass) | levels\levelglass | L12 (Glass Arena) | levels\arena12 |
| L13 (Sky) | levels\level9 | L13 (Sky Arena) | levels\arena13 |
| L14 (Master) | levels\level10 | L14 (Master Arena) | levels\arena14 |
| L15 (Impossible) | levels\levelimpossible | L15 (Impossible Arena) | levels\arena15 |

### Complete Ref Name → Constructor Mapping

| Ref Name | Constructor | Alloc Size | Quality-Gated? | Levels Found In |
|----------|------------|-----------|----------------|-----------------|
| BRIDGE | (returns pre-loaded mesh) | 0 | No | Intermediate, Expert, Master |
| TIPPER | Tipper_ctor | 0x1104 | Yes | Dizzy, Master |
| WATERWHEEL | (returns pre-loaded mesh) | 0 | No | Dizzy |
| SWIRL | (returns pre-loaded mesh) | 0 | No | Dizzy |
| GLUEBIE | Gluebie_ctor | 0x110C | Yes | Dizzy, Master |
| CATAPULT | Catapult_ctor | 0x1108 | No | Tower, Master |
| MACE | CascadeStands_Ctor | 0x110C | Yes | Tower |
| DRAWBRIDGE | Glass_Level_ctor | 0x113C | No | Tower |
| WINDMILL | (returns pre-loaded mesh + collision) | 0 | No | Tower |
| TRAPDOOR | GlassStands_Ctor / Rotator_ctor | 0x10F8 / 0x10F4 | No | Tower, Sky |
| CHOMPER | (position update only) | 0 | No | Tower |
| TURRET | Stands_ctor + CollisionLevel | 0x10D0 | No | Tower |
| LIFTER | Stands_CtorWithCollision / Rotator_ctor_sound | 0x10FC / 0x10F4 | No | Odd, Up |
| SPEEDCYLINDER | Pendulum_ctor | 0x150C | No | Up |
| TIMEBUTTON | Rotator_ctor_nosound | 0x10E8 | No | Up |
| NEONPLATFORM | Stands_CtorRotator | 0x10EC | No | Neon |
| DFLOOR1-4 | RumbleBoard_Stands_ctor | 0x1104 | No | Neon |
| TRODE | RumbleBoard_Stands_ctor | 0x1104 | No | Neon |
| BONK | Bonk_ctor | 0x1200 | Yes | Expert, Master |
| SAWBLADE | Sawblade_Level_Ctor | 0x111C | Yes | Expert |
| JUDGE | Gear_Level_ctor | 0x1100 | No | Expert |
| BELL | Tipper_Level_Ctor | 0x10E8 | No | Expert |
| SPINNY | Rotator_ctor | 0x1508 | No | Toob |
| SAW | Stands_CtorCollision | 0x1110 | Yes | Toob |
| SAW2 | Stands_CtorSpeedCylinder | 0x1118 | Yes | Toob |
| FALLOUT1 | Stands_CtorCollisionV2 | 0x10E8 | No | Toob |
| BLOCKDAWG1-3 | Blockdawg_ctor | 0x1154 | Yes | Toob, Master |
| WOBBLY1-7 | GameLevel_ctor | 0x1524 | No | Wobbly |
| WAVY1 | Stands_CtorWithCollisionLevel | 0x1AE7C | No | Wobbly |
| SMASHER1-2 | (position update only) | 0 | No | Glass |
| POPCYLINDER | PopCylinder_ctor / Platform_ctor | 0x10E8 / 0x10F4 | Conditional | Sky, Master |
| LOOPER | Looper_ctor | 0x1500 | No | Impossible |
| GEAR | Gear_ctor | 0x1514 | No | Impossible |
| BIGGEAR | Gear_ctor | 0x1514 | No | Impossible |
| ROTATOR | Rotator_ctor | 0x1508 | No | Impossible |
| PENDULUM | Pendulum_ctor | 0x1504 | No | Impossible |
| BBRIDGE1-2 | BreakBridge_ctor | 0x1100 | No | Master |

### Special Ref Modifiers

Some refs support suffix modifiers checked via `strstr()`:
- `(NOCOLLIDE)` — on BRIDGE refs, suppresses collision object creation
- `SLOW` — on Expert's [TOW] refs, sets slow flag
- `SUPER` — on Expert's [TOW] refs, sets super flag  
- `UP` — on Expert's [TOW] refs, initializes sound channels
- `1`, `2` — on SAWBLADE and BRIDGE refs, assigns to specific board slots
- `NEG` — on BRIDGE refs in Expert, sets negative rotation
- `TOUCH` — on BIGGEAR refs, sets touch-activated flag

### Verified Ref Names per Level

Confirmed by binary-grepping the original MESHWORLD files:

| MESHWORLD File | Object Refs (Section 1, bare names) | Entity Names (Section 3, N:/E: prefixed) |
|----------------|-------------------------------------|------------------------------------------|
| Level1 (Warm-up) | START | (none) |
| LevelCascade (Beginner) | BUMP, BUMPER | N:GOAL, N:BUMP, N:BUMPER |
| Level2 (Intermediate) | BONK, BRIDGE | N:GOAL, E:LIMIT |
| Level3 (Dizzy) | SWIRL | N:GOAL, N:TARPIT |
| Level4 (Tower) | BRIDGE, MACE, DRAWBRIDGE, WINDMILL, CHOMPER, TURRET | N:GOAL, E:JUMP, E:ACTION |
| Level5 (Expert) | SAWBLADE, BRIDGE, JUDGE, BELL, SAW, SAW2 | N:GOAL, E:JUMP |
| Level6 (Odd) | LIFTER | N:GOAL, N:JUMPFIRST, N:JUMPSECOND |
| LevelUp (Up) | SPEEDCYLINDER, LIFTER, TIMEBUTTON | N:GOAL, T:SPEEDARROW |
| LevelDark (Neon) | NEONPLATFORM, DFLOOR, TRODE | N:GOAL |
| Level7 (Wobbly) | BONK, WOBBLY, WAVY | N:GOAL |
| Level8 (Toob) | SPINNY, SAW, SAW2, FALLOUT, BLOCKDAWG, BUMP, BUMPER | N:GOAL, N:BUMP, N:BUMPER |
| LevelGlass (Glass) | BONK, SMASHER | N:GOAL, N:GLASS, N:TENBONUS1, N:TENBONUS2 |
| Level9 (Sky) | TRAPDOOR, POPCYLINDER | N:GOAL |
| Level10 (Master) | BRIDGE, BLOCKDAWG, POPCYLINDER, BBRIDGE | N:GOAL, N:TARPIT |
| LevelImpossible (Impossible) | BONK, LOOPER, GEAR, BIGGEAR, ROTATOR, PENDULUM | N:GOAL |

Common refs in ALL levels: START (spawn point), SAFESPOT (respawn point), FLAG (checkpoint), E:LIMIT (boundary kill plane).

---

## How to Load Any Ref Into Any Level

### Method 1: MESHWORLD-Only (Limited)

Adding a ref name to a MESHWORLD file alone will NOT cause an object to be created. The ref name must match a `__strnicmp` check in the Board's vtable[33] factory. If the factory doesn't handle that name, the ref is silently ignored.

**Exception:** Master Race's `CreateLevelObjects` (0x4121D0) factory handles the most ref types (BRIDGE, TIPPER, BONK, BBRIDGE1-2, POPCYLINDER, BLOCKDAWG1-2, CATAPULT, GLUEBIE). If the target level uses this factory, many refs will work from MESHWORLD alone.

### Method 2: DLL Mod — Vtable Patch (Recommended)

Patch the target Board's vtable[33] entry (at vtable_addr + 0x84) to point to a combined factory function. The combined factory should:
1. First try the original level's factory (call through the saved original pointer)
2. If the original returns null, try factories from other levels

```c
// Pseudo-code for combined factory
void __thiscall UniversalFactory(void* board, char* refName, void** outObj, void** outCol, int* refEntry) {
    // Try original factory first
    originalFactory(board, refName, outObj, outCol, refEntry);
    if (*outObj != NULL) return;
    
    // Try other level factories
    CreateUpLevelObjects(board, refName, outObj, outCol, refEntry);
    if (*outObj != NULL) return;
    
    CreateExpertLevelObjects(board, refName, outObj, outCol, refEntry);
    if (*outObj != NULL) return;
    
    CreateMechanicalObjects(board, refName, outObj, outCol, refEntry);
    // ... etc
}
```

**Critical requirement:** The Board must have the necessary sub-meshes pre-loaded. Each Board constructor loads specific MeshWorld files (e.g. `Levels\Level3-Tipper`, `Levels\Level4-Catapult`) into board+0x436C..0x4390 slots. Without the corresponding mesh, the factory may crash or produce invisible objects.

### Method 3: DLL Mod — Hook Scene_CreateDynamicObjects

Hook the central dispatch at 0x0040C4BA. Before the original call to `vtable[33]`, intercept the ref name and call any factory function directly. This is the most flexible approach:

```c
// Hook at 0x0040C4BA, before the vtable[33] call
// The call site is at offset ~0x?? from function entry:
//   (**(code **)(*param_1 + 0x84))(*puVar4, &local_58, &local_54, puVar4);
// Replace this indirect call with a direct call to your dispatcher
```

### Method 4: Binary Patch — Swap Vtable Entry

Overwrite the 4 bytes at `(board_vtable_addr + 0x84)` with the address of a different level's factory. For example, to give Warm-up Race the Impossible factory:
- Patch `0x4D04A8 + 0x84` = `0x4D04EC` from `0x00419750` to `0x00417FE0`

This is the simplest method but only gives you ONE level's factory at a time.

---

## Sub-Mesh Preloading

Each Board constructor preloads specific MeshWorld files into board struct slots. The table below shows which sub-mesh files each Board constructor loads (verified by tracing string references in the binary):

### Board Constructor → Sub-Mesh File Mapping

| Board | Constructor Area | Sub-Meshes Loaded |
|-------|-----------------|-------------------|
| Warm-up (L1) | 0x004197xx | (none — no factory) |
| Beginner (L2) | 0x004197xx | (none — no factory) |
| Intermediate (L3) | 0x0041CBxx | Level2-Bridge |
| Dizzy (L4) | 0x0041D0xx-0x0041D3xx | Level3-Tipper, Level3-WaterWheel, Level3-Swirl, Level3-Gluebie |
| Tower (L5) | 0x0041E3xx-0x0041E6xx | Level4-Catapult, Level4-Drawbridge, Level4-Mace, Level4-Windmill, Level4-Turret, Level4-Trapdoor1-2 |
| Up (L6) | 0x0042A0xx-0x0042A1xx | Level6-Lifter, LevelUp-SpeedCylinder, LevelUp-Button, LevelUp |
| Neon (L7) | 0x004245xx-0x004249xx | LevelDark-NeonPlatform, LevelDark-DFloor1-4, LevelDark-Trode, LevelDark-Flickning |
| Expert (L8) | 0x0041EBxx | Level5-Bridge |
| Odd (L9) | 0x0042A1xx | Level6-Lifter |
| Toob (L10) | 0x0041F5xx-0x0041F6xx | Level8-Spinny, Level8-Saw, Level8-Fallout, Level8-Blockdawg1-2 |
| Wobbly (L11) | 0x0041F2xx-0x0040F5xx | Level7-Wobbly1-7, Level7-Wavy1 |
| Glass (L12) | 0x004177xx | (position-only refs, minimal mesh loading) |
| Sky (L13) | 0x00429Fxx | Level4-Trapdoor1-2 (shared with Tower), Level9-PopCylinder |
| Master (L14) | 0x004207xx-0x00420Bxx | Level2-Bridge, Level3-Tipper, Level3-Gluebie, Level3-Swirl, Level3-WaterWheel, Level4-Catapult, Level4-Mace, Level4-Windmill, Level4-Turret, Level4-Trapdoor1-2, Level7-Wobbly8, Level8-Blockdawg1-2, Level10-Bridge1-2, Level10-2PBridge |
| Impossible (L15) | 0x00424Cxx-0x00424Fxx | LevelImpossible-Looper, LevelImpossible-Pendulum, LevelImpossible-Gear, LevelImpossible-BigGear, LevelImpossible-Rotator |

### Board Struct Slots for Sub-Meshes

| Board Offset | Typical Use | Example |
|-------------|-------------|---------|
| +0x436C | Primary sub-mesh (e.g. Tipper mesh, Bridge mesh) | Dizzy: Level3-Tipper |
| +0x4370 | Primary collision level or secondary mesh | Dizzy: CollisionLevel(Tipper) |
| +0x4374 | Secondary sub-mesh | Dizzy: Level3-Gluebie |
| +0x4378 | Tertiary sub-mesh | Tower: Level4-Mace |
| +0x437C | Quaternary sub-mesh | Tower: Level4-Windmill |
| +0x4380-0x4390 | Additional sub-meshes | Neon: DFloor1-4, Trode |

**When a factory tries to create an object that references a sub-mesh slot, the slot must contain a valid MeshWorld pointer.** If the Board constructor didn't load that mesh, the factory will either:
- Crash (dereferencing null)
- Create an object with no visual mesh (invisible)
- Silently fail

### Runtime Sub-Mesh Loading (DLL Mod Approach)

To load additional sub-meshes at runtime from a DLL mod:

```c
// Load a mesh world into a free board slot at runtime
// MeshWorld_ctor address: find from Board constructor code
// operator_new: 0x00449E70
typedef void* (__cdecl *operator_new_t)(size_t);
typedef void* (__thiscall *MeshWorld_Load_t)(void* mesh, void* graphics, const char* path);

void load_submesh(void* board, int slot_offset, const char* path) {
    operator_new_t op_new = (operator_new_t)0x00449E70;
    // Allocate and load the mesh
    void* mesh = op_new(0x10D0);  // MeshWorld struct size
    if (mesh) {
        // Call MeshWorld constructor with graphics device and path
        // The graphics device is at board+0x8AC (Scene pointer) → scene+0x480
        void* scene = *(void**)((char*)board + 0x878);
        void* graphics = *(void**)((char*)scene + 0x480);
        // MeshWorld_Load(mesh, graphics, path);
        // Store in board slot
        *(void**)((char*)board + slot_offset) = mesh;
    }
}
```

### Master Race as the Universal Level

Master Race's Board constructor is the most inclusive — it preloads sub-meshes from **5 other levels** (Tower, Toob, Dizzy, Intermediate, Wobbly) plus its own meshes. This is why Master's `CreateLevelObjects` factory (0x4121D0) can create 9 different object types. The universal ref loader DLL mod will work most reliably on Master Race levels, or on any level where you manually preload the required sub-meshes.

---

## Scene_CreateSigns (0x0040C400)

A separate dispatch function handles `SIGN` refs. This function is **not** vtable-gated — it runs for all levels and creates `StandsTipper` objects for any ref starting with "SIGN". Special case: `SIGN-TARPIT` gets additional tar-pit data from `scene+0x27C`.

---

## Summary

1. **Two independent Board systems**: RACE Boards (0x422xxx constructors, vtable 0x4D14xx-0x4D22xx) and ARENA Boards (0x41Cxxx constructors, vtable 0x4D05xx-0x4D21xx). Each has its own set of vtable[33] factories. Race factories handle fewer ref types; Arena factories handle the full set.
2. **Two independent dispatch systems**: (A) vtable[33] factory creates objects from bare-name ref points in MESHWORLD Section 1; (B) the N:/E: handler at 0x0040C5D0 processes entity names from Section 3 meshes to set behavioral flags.
3. **The `N:` prefix is NOT stripped** — it's part of the entity name in Section 3, not the ref point name in Section 1. Ref points use bare names.
4. **Race factory inheritance chain**: Dizzy → Tower → Up → Expert all share the same base factory (0x4133E0). Each adds its own refs on top. Levels 1,2,3,9,10,12,14 use ONLY the base factory.
5. **Arena factories are more inclusive**: Master Arena factory (0x4121D0) handles 9+ ref types from other levels.
6. **To load any ref into any level**, you need: (a) patch the Board's vtable[33] to a combined factory that calls multiple level factories, AND (b) preload the required sub-meshes into board struct slots. A MESHWORLD-only approach is insufficient — the factory must recognize the ref name.
7. **Quality gating** (`scene+0x23C`) blocks creation of complex visual objects on low quality settings.
8. **The N:/E: handler is called from within the factories** — it runs as a sub-step of object creation, not as a separate top-level dispatch.
