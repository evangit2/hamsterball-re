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

| # | Race Level | Board Vtable | Factory Addr | Refs Handled (verified from binary) |
|---|-----------|-------------|-------------|-------------------------------------|
| 1 | Warm-up | 0x4D1428 | 0x4133E0 | PLATFORM, STANDS, N:BUMPER (base) |
| 2 | Beginner | 0x4D14F0 | 0x4133E0 | PLATFORM, STANDS, N:BUMPER (base) |
| 3 | Intermediate | 0x4D15C0 | 0x4133E0 | PLATFORM, STANDS, N:BUMPER (base) |
| 4 | Dizzy | 0x4D1680 | 0x4143D0 | SPINNY, MACE, CATAPULT, TURRET, LIFTER, FAN, E:GRAVITY + base |
| 5 | Tower | 0x4D1740 | 0x414680 | MACE, CATAPULT, TURRET, LIFTER, FAN, E:GRAVITY, N:BUMPER + base |
| 6 | Up | 0x4D17F8 | 0x414A20 | LIFTER, FAN, E:GRAVITY, N:BUMPER, WOBBLY1, N:SQUAREWOBBLY + base |
| 7 | Neon | 0x4D1EC8 | 0x4173B0 | FLICKNING, N:BUMP, N:GLASS, N:TENBONUS1-2 + base |
| 8 | Expert | 0x4D18C8 | 0x414BD0 | FAN, E:GRAVITY, N:BUMPER, WOBBLY1, N:SQUAREWOBBLY + base |
| 9 | Odd | 0x4D1980 | 0x4133E0 | PLATFORM, STANDS, N:BUMPER (base) |
| 10 | Toob | 0x4D1A40 | 0x4133E0 | PLATFORM, STANDS, N:BUMPER (base) |
| 11 | Wobbly | 0x4D1B18 | 0x415460 | WOBBLY1, N:SQUAREWOBBLY, PILLAR, POPCYLINDER, EDGECYLINDER + base |
| 12 | Glass | 0x4D2048 | 0x4133E0 | PLATFORM, STANDS, N:BUMPER (base) |
| 13 | Sky | 0x4D1BD8 | 0x415A30 | POPCYLINDER, EDGECYLINDER, E:LAUNCH + base |
| 14 | Master | 0x4D1C80 | 0x4133E0 | PLATFORM, STANDS, N:BUMPER (base) |
| 15 | Impossible | 0x4D2298 | 0x418760 | GEAR + base |

**Key insight**: Levels 1, 2, 3, 9, 10, 12, 14 use ONLY the base factory — they handle only `PLATFORM` and `STANDS` in race mode. Their level-specific objects (tippers, bridges, etc.) are NOT created by vtable[33] in race mode — they are either part of the static level geometry or loaded via the Arena Board system.

### ARENA Board System (0x41Cxxx constructors)

Arena Board constructors are at `0x41CB20`–`0x424C20`. Each is called from a sequential switch at `0x427140` (sequential `MOV [ESP+0x18], level_id; JZ skip; CALL constructor`).

Arena Board vtables are at `0x4D05A0`–`0x4D21C0`. The Arena factories handle **many more ref types** than Race factories — they create ALL interactive objects for Arena mode.

| # | Arena Level | Board Vtable | Factory Addr | Refs Handled (verified from binary) |
|---|-----------|-------------|-------------|-------------------------------------|
| 1 | Warm-up Arena | 0x4D1098 | 0x419750 | (none — NoOp) |
| 2 | Beginner Arena | 0x4D05A0 | 0x40A550 | BRIDGE, TIPPER, WATERWHEEL, SWIRL, GLUEBIE, SMASHER1, SMASHER2 |
| 3 | Intermediate Arena | 0x4D0890 | 0x40A5F0 | TIPPER, WATERWHEEL, SWIRL, GLUEBIE, SMASHER1, SMASHER2 |
| 4 | Dizzy Arena | 0x4D0A08 | 0x40D7C0 | CATAPULT, MACE, DRAWBRIDGE, WINDMILL, TRAPDOOR, CHOMPER, BONK, FAN, SAWBLADE, BRIDGE, JUDGE, BELL |
| 5 | Tower Arena | 0x4D11A0 | 0x4117B0 | LIFTER, SPEEDCYLINDER, TIMEBUTTON, TarBubble, BRIDGE, TIPPER, BONK, BBRIDGE1-2, POPCYLINDER, BLOCKDAWG1-2, CATAPULT |
| 6 | Neon Arena | 0x4D1DF0 | 0x416910 | NEONPLATFORM, DFLOOR1-4, TRODE, N:NEONPLATFORM, E:ZOOP, E:LIGHTSOFF, E:LIGHTSON, FLICKNING, N:BUMP, N:GLASS, N:TENBONUS1 |
| 7 | Expert Arena | 0x4D0B00 | 0x40E250 | BONK, FAN, SAWBLADE, BRIDGE, JUDGE, BELL, E:SCORE, E:BELL, LIFTER, E:GRAVITY |
| 8 | Odd Arena | 0x4D0BC0 | 0x40EC40 | LIFTER, E:GRAVITY, WOBBLY1-7, WAVY1, N:SQUAREWOBBLY, N:WAVY, SPINNY |
| 9 | Toob Arena | 0x4D0E78 | 0x40FB30 | SPINNY, FALLOUT1, BLOCKDAWG1-3, E:ALERTSAW2, E:BRANCH, N:SPINNY, N:SAWTEETH, N:BUMPER, PILLAR, MAGNIFYER |
| 10 | Glass Arena | 0x4D0D38 | 0x40F420 | WOBBLY1-7, WAVY1, N:SQUAREWOBBLY, N:WAVY, SPINNY, FALLOUT1, BLOCKDAWG1-3, E:ALERTSAW2, E:BRANCH, N:SPINNY |
| 11 | Wobbly Arena | 0x4D1F90 | 0x40AD80 | SMASHER1, SMASHER2, SECRETUNLOCK, SECRET, BADBALL |
| 12 | Sky Arena | 0x4D0FC8 | 0x410AD0 | POPCYLINDER, TRAPDOOR, N:BUMPER, VAC-IN, LIFTER, SPEEDCYLINDER, TIMEBUTTON |
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

### Board Creation — Mode Determination

The game checks `App+0x237` (byte) to determine whether to use Race Board or Arena Board:

```asm
; At ~0x4270AB
CMP [EAX+0x237], BL    ; check App+0x237 (BL=0)
JZ  skip_race           ; if zero, skip race board creation
CALL 0x00426780        ; Race Board switch (jump table at 0x426AB0)
```

- **App+0x237 = 1 (non-zero)** → Race mode → Race Board constructors (0x422xxx)
- **App+0x237 = 0** → Arena mode → Arena Board constructors (0x41Cxxx), called from `0x427080`

The Race Board function (`0x426780`) uses a jump table at `0x426AB0` with 15 entries (indexed by `level_number - 1`). The Arena Board function (`0x427080`) uses a sequential switch at `0x427140` with sequential level IDs 1–14.

### Board Constructor → Sub-Mesh File Mapping (Arena)

| Board | Constructor Area | Sub-Meshes Loaded (verified from binary) |
|-------|-----------------|------------------------------------------|
| Arena L1 | 0x4200E0 | LevelUp-Lifter, LevelUp-SpeedCylinder, LevelUp-Button, Level2-Bridge, Level10-2PBridge |
| Arena L2 | 0x41CB20 | Level2-Bridge, Level3-Tipper, Level3-WaterWheel, Level3-Swirl |
| Arena L3 | 0x41D060 | Level3-Tipper, Level3-WaterWheel, Level3-Swirl, Level3-Gluebie |
| Arena L4 | 0x41E340 | Level4-Catapult, Level4-Drawbridge, Level4-Mace, Level4-Windmill, Level4-Turret |
| Arena L5 | 0x420390 | LevelUp-Lifter, LevelUp-SpeedCylinder, LevelUp-Button, Level2-Bridge, Level10-2PBridge, Level3-Tipper, Level10-Bridge1-2, Level9-PopCylinder1-2, Level8-Blockdawg1-2, Level4-Catapult, Level3-Gluebie |
| Arena L6 | 0x424440 | LevelDark-NeonPlatform, LevelDark-DFloor1-4, LevelDark-Trode, LevelDark-Flickring |
| Arena L7 | 0x41EA40 | Level5-Bridge, Level7-Wobbly1 |
| Arena L8 | 0x41ED80 | Level7-Wobbly1-7 |
| Arena L9 | 0x41F4B0 | Level8-Spinny, Level8-Saw, Level8-Fallout, Level8-Blockdawg1-2, Level9-PopCylinder1-2, Level9-Trapdoor |
| Arena L10 | 0x41F110 | Level7-Wobbly1-7, Level8-Spinny, Level8-Saw, Level8-Fallout, Level8-Blockdawg1-2 |
| Arena L11 | 0x424A90 | LevelImpossible-Looper, Gear, BigGear, Rotator, Pendulum |
| Arena L12 | 0x41F930 | Level9-PopCylinder1-2, Level9-Trapdoor |
| Arena L13 | 0x4206D0 | Level2-Bridge, Level10-2PBridge, Level3-Tipper, Level10-Bridge1-2, Level9-PopCylinder1-2, Level8-Blockdawg1-2, Level4-Catapult, Level3-Gluebie |
| Arena L14 | 0x424C20 | LevelImpossible-Looper, Gear, BigGear, Rotator, Pendulum |

### Factory → Board Sub-Mesh Slot Dependencies

Each Arena factory accesses specific board struct offsets (0x4344–0x4398) to retrieve pre-loaded sub-mesh data. If a slot is null (because the Board constructor didn't load that mesh), the factory will skip that ref type or crash.

| Factory | Board Slots Accessed | Refs Dependent on Slots |
|---------|---------------------|------------------------|
| Beginner (0x40A550) | +0x436C, +0x4370, +0x4374 | BRIDGE, TIPPER, WATERWHEEL/SWIRL |
| Intermediate (0x40A5F0) | +0x436C, +0x4370, +0x4374 | TIPPER, WATERWHEEL, SWIRL, GLUEBIE |
| Dizzy (0x40D7C0) | +0x436C, +0x4370, +0x4378, +0x437C, +0x4390, +0x43A4, +0x43B0, +0x43B4 | CATAPULT, MACE, DRAWBRIDGE, WINDMILL, TRAPDOOR, TURRET (8 slots) |
| Tower (0x4117B0) | +0x436C, +0x4370, +0x4374, +0x4378, +0x4394, +0x4398 | LIFTER, SPEEDCYLINDER, TIMEBUTTON, BRIDGE, TIPPER, BONK |
| Neon (0x416910) | +0x4374, +0x4378, +0x437C, +0x4380, +0x4384, +0x4388, +0x438C, +0x4390 | NEONPLATFORM, DFLOOR1-4, TRODE, FLICKNING (8 slots) |
| Expert (0x40E250) | +0x436C, +0x4370, +0x4374 | BONK, SAWBLADE, BRIDGE, JUDGE, BELL |
| Odd (0x40EC40) | +0x436C, +0x4370, +0x4374, +0x4378, +0x437C | LIFTER, WOBBLY1-5, WAVY1 |
| Toob (0x40FB30) | +0x436C, +0x4370, +0x4374, +0x4378, +0x437C, +0x4380, +0x4384 | SPINNY, SAW, FALLOUT, BLOCKDAWG (7 slots) |
| Glass (0x40F420) | +0x436C, +0x4370, +0x4374, +0x4378, +0x437C, +0x4380, +0x4384 | WOBBLY1-7, WAVY1, SPINNY, FALLOUT, BLOCKDAWG (7 slots) |
| Wobbly (0x40AD80) | +0x4344 | SMASHER1-2 (1 slot) |
| Sky (0x410AD0) | +0x436C, +0x4374, +0x4378, +0x437C, +0x4380, +0x438C, +0x4390 | POPCYLINDER, TRAPDOOR (7 slots) |
| Master (0x4121D0) | +0x436C, +0x4370, +0x4394, +0x4398 | BRIDGE, TIPPER, BONK, BBRIDGE, POPCYLINDER, BLOCKDAWG, CATAPULT, GLUEBIE |
| Impossible (0x417FE0) | +0x436C, +0x4370, +0x4374, +0x4378, +0x437C | LOOPER, GEAR, BIGGEAR, ROTATOR, PENDULUM |

**Critical implication for the universal ref loader**: When the DLL mod tries a factory from another level, that factory will access board slots that weren't loaded by the current level's constructor. **Factories do NOT null-check their sub-mesh slots** — they dereference the slot pointer directly after checking only that `operator_new` succeeded. If the slot is NULL, the factory will crash with an access violation.

The DLL mod must check sub-mesh slots BEFORE calling each factory, or pre-load the required sub-meshes. The slot dependency table above provides the exact offsets each factory accesses.

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

## Complete Ref Name → Factory Reverse Index

This table shows every ref name and which factory(ies) can create it. Use this to determine which sub-meshes need to be loaded to support a given ref in a given level.

| Ref Name | Arena Factories | Race Factories | Required Slots |
|----------|----------------|----------------|----------------|
| BADBALL | Wobbly | — | +0x4344 |
| BBRIDGE1-2 | Tower, Master | — | +0x436C, +0x4370, +0x4394, +0x4398 |
| BELL | Dizzy, Expert | — | +0x436C, +0x4370, +0x4374 |
| BIGGEAR | Impossible | — | +0x436C, +0x4370, +0x4374, +0x4378, +0x437C |
| BLOCKDAWG1-2 | Tower, Toob, Glass, Master | — | varies |
| BLOCKDAWG3 | Toob, Glass | — | +0x436C to +0x4384 |
| BONK | Dizzy, Tower, Expert, Master | — | varies |
| BRIDGE | Beginner, Dizzy, Tower, Expert, Master | — | +0x436C, +0x4370, +0x4374 |
| CATAPULT | Dizzy, Tower, Master | Race Dizzy, Race Tower | +0x436C, +0x4378 |
| CHOMPER | Dizzy | — | +0x436C, +0x4378, +0x43B0 |
| DFLOOR1-4 | Neon | — | +0x4374 to +0x4390 |
| DRAWBRIDGE | Dizzy | — | +0x436C, +0x4378, +0x4390 |
| E:ALERTSAW2 | Toob, Glass | — | +0x436C to +0x4384 |
| E:BELL | Expert | — | +0x436C, +0x4370, +0x4374 |
| E:BRANCH | Toob, Glass | — | +0x436C to +0x4384 |
| E:GRAVITY | Expert, Odd | Race Dizzy, Tower, Up, Expert | varies |
| E:LAUNCH | Master | Race Sky | +0x436C, +0x4394 |
| E:LIGHTSOFF/ON | Neon | — | +0x4374 to +0x4390 |
| E:SCORE | Expert | — | +0x436C, +0x4370, +0x4374 |
| E:ZOOP | Neon | — | +0x4374 to +0x4390 |
| EDGECYLINDER | — | Race Wobbly, Race Sky | (race, no sub-mesh slots) |
| FALLOUT1 | Toob, Glass | — | +0x436C to +0x4384 |
| FAN | Dizzy, Expert | Race Dizzy, Tower, Up, Expert | varies |
| FLICKNING | Neon | Race Neon | +0x4374 to +0x4390 |
| GEAR | Impossible | Race Impossible | +0x436C, +0x4370, +0x4374, +0x4378, +0x437C |
| GLUEBIE | Beginner, Intermediate, Master | — | +0x436C, +0x4370, +0x4374 |
| JUDGE | Dizzy, Expert | — | +0x436C, +0x4370, +0x4374 |
| LIFTER | Tower, Expert, Odd, Sky | Race Dizzy, Tower, Up | varies |
| LOOPER | Impossible | — | +0x436C, +0x4370, +0x4374, +0x4378, +0x437C |
| MACE | Dizzy | Race Dizzy, Race Tower | +0x436C, +0x4378 |
| MAGNIFYER | Toob | — | +0x436C to +0x4384 |
| N:BOUNCE | Impossible | — | +0x436C to +0x437C |
| N:BUMP | Neon | Race Neon | +0x4374 to +0x4390 |
| N:BUMPER | Toob, Sky, Master | Race Base, Tower, Up, Expert | varies |
| N:GLASS | Neon | Race Neon | +0x4374 to +0x4390 |
| N:NEONPLATFORM | Neon | — | +0x4374 to +0x4390 |
| N:ONGEAR | Impossible | — | +0x436C to +0x437C |
| N:ONROTATOR | Impossible | — | +0x436C to +0x437C |
| N:SAWTEETH | Toob | — | +0x436C to +0x4384 |
| N:SPINNER | Master | — | +0x436C, +0x4394 |
| N:SPINNY | Toob, Glass | — | +0x436C to +0x4384 |
| N:SQUAREWOBBLY | Odd, Glass | Race Up, Expert, Wobbly | varies |
| N:TENBONUS1 | Neon | Race Neon | +0x4374 to +0x4390 |
| N:TENBONUS2 | — | Race Neon | (race) |
| N:WAVY | Odd, Glass | — | +0x436C to +0x4384 |
| NEONPLATFORM | Neon | — | +0x4374 to +0x4390 |
| PENDULUM | Impossible | — | +0x436C to +0x437C |
| PILLAR | Toob | Race Wobbly | +0x436C to +0x4384 |
| PLATFORM | — | Race Base | (base, no sub-mesh) |
| POPCYLINDER | Tower, Sky, Master | Race Wobbly, Race Sky | varies |
| ROTATOR | Impossible | — | +0x436C to +0x437C |
| SAWBLADE | Dizzy, Expert | — | +0x436C, +0x4370, +0x4374 |
| SECRET | Wobbly | — | +0x4344 |
| SECRETUNLOCK | Wobbly | — | +0x4344 |
| SMASHER1-2 | Beginner, Intermediate, Wobbly | — | +0x4344 or +0x436C |
| SPEEDCYLINDER | Tower, Sky | — | +0x436C, +0x4394, +0x4398 |
| SPINNY | Odd, Toob, Glass | Race Dizzy | varies |
| STANDS | — | Race Base | (base, no sub-mesh) |
| SWIRL | Beginner, Intermediate | — | +0x436C, +0x4370, +0x4374 |
| TarBubble | Tower | — | +0x436C, +0x4394 |
| TIMEBUTTON | Tower, Sky | — | +0x436C, +0x4394, +0x4398 |
| TIPPER | Beginner, Intermediate, Tower, Master | — | +0x436C, +0x4370, +0x4374 |
| TRAPDOOR | Dizzy, Sky | — | +0x436C, +0x4378, +0x4390 |
| TRODE | Neon | — | +0x4374 to +0x4390 |
| TURRET | — | Race Dizzy, Race Tower | (race) |
| VAC-IN | Sky | — | +0x436C, +0x4390 |
| WATERWHEEL | Beginner, Intermediate | — | +0x436C, +0x4370, +0x4374 |
| WAVY1 | Odd, Glass | — | +0x436C to +0x4384 |
| WINDMILL | Dizzy | — | +0x436C, +0x4378, +0x43A4 |
| WOBBLY1-7 | Odd, Glass | Race Up, Expert, Wobbly | varies |

1. **Two independent Board systems**: RACE Boards (constructors at 0x422xxx, vtables 0x4D1428–0x4D2298) and ARENA Boards (constructors at 0x41Cxxx, vtables 0x4D05A0–0x4D21C0). Race mode uses App+0x237=0; Arena mode uses App+0x237=1. Each has its own vtable[33] factory.
2. **vtable[33] dispatch**: `Scene_CreateDynamicObjects` (0x0040C430) iterates MESHWORLD Section 1 ref points, and for each ref, calls `board->vtable[33](board, refName, &outObj, &outCol, refEntry)` at instruction `0x0040C4BA`. If the factory returns NULL, the ref is silently ignored.
3. **Factory chain (Race)**: Race factories form an inheritance chain — each level-specific factory checks its own refs, then falls through to the base factory `0x4133E0` which handles `PLATFORM`, `STANDS`, `N:BUMPER`.
4. **Factory chain (Arena)**: Arena factories are standalone — each handles its complete ref set without falling through. The most inclusive Arena factory is Master (13 ref types), followed by Dizzy (12 ref types) and Tower (13 ref types).
5. **Sub-mesh preloading**: Each Board constructor preloads specific MESHWORLD files into board struct slots (+0x4344 through +0x43B4). Factories access these slots to get mesh data for object creation. **Factories do NOT null-check these slots** — calling a factory with unloaded slots causes an access violation crash.
6. **N:/E: handler (0x0040C5D0)**: A separate system that processes entity names from Section 3 objects to set behavioral flags (gravity modifiers, launch pads, bumpers, etc.). Called from 25 sites within the Arena factories.
7. **Universal ref loader mod**: A bass.dll proxy that hooks the vtable[33] dispatch at 0x0040C4BA and tries all 13 Arena factories in sequence, with per-factory sub-mesh slot safety checks. Allows loading any ref type into any level (limited by sub-mesh availability).

### How to Load Any Ref Into Any Level

**Option A — MESHWORLD mod only (simple refs)**:
For refs handled by the Race base factory (PLATFORM, STANDS, N:BUMPER), simply add the ref name to the MESHWORLD Section 1. No code changes needed.

**Option B — MESHWORLD mod + DLL hook (any ref)**:
1. Add the desired ref name to the level's MESHWORLD Section 1.
2. Install the universal-ref-loader bass.dll proxy.
3. The DLL mod will try all Arena factories when the level's own factory doesn't recognize the ref.
4. **Limitation**: The ref's sub-mesh data must already be loaded in a board slot. If the ref requires a sub-mesh that wasn't loaded by the current Board constructor, the factory will be skipped (safety check prevents crash, but ref won't be created).

**Option C — MESHWORLD mod + pre-loaded sub-meshes (full support)**:
1. Add the ref name to the level's MESHWORLD Section 1.
2. Pre-load the required sub-mesh by modifying the Board constructor to load additional MESHWORLD files (binary patch the constructor to add MeshWorld loading calls).
3. Install the universal-ref-loader bass.dll proxy.
4. All 75 ref types can now be loaded into any level.
