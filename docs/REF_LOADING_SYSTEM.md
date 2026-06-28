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

**Complete list of entity names processed by the N:/E: handler** (verified from binary):

| Entity Name | Behavior |
|------------|----------|
| N:SECRET | Secret/unlock mechanics |
| N:UNLOCKSECRET | Unlock secret areas |
| E:NODIZZY | Clear TIME checkpoints (NOT dizzy-related) |
| TIME | Timer object |
| E:SAFESWITCH | Safe switch trigger |
| E:LIMIT | Boundary kill plane |
| E:BREAK | Breakable surface |
| E:JUMP | Jump pad |
| E:ACTION | Action trigger |
| ONCE, TRUE, YES | Boolean modifiers |
| SCORE | Scoring object |
| E:TRAJECTORY | Trajectory modifier |
| N:NOCONTROL | Disable control |
| N:WATER | Water effect |
| N:TARPIT | Tar pit effect |
| DROPIN | Pipe drop-in |
| PIPEBONK | Pipe collision |
| POPOUT | Pipe pop-out |
| ZIP | Zip/boost effect |
| Goal! | Goal text popup |

**Call site example** (at 0x0040D380, Dizzy factory):
```asm
MOV EAX, [ESP+0x0C]    ; load ref entry from stack
PUSH EDI               ; push board pointer (param_3)
PUSH EAX               ; push ref entry (param_2)
MOV ECX, ESI           ; ECX = mesh entity (this for __thiscall)
CALL 0x0040C5D0        ; N: handler(mesh_entity, ref_entry, board)
```

### 5. The `Scene+0x23C` Quality Gate

Many (but not all) factory branches check `*(int *)(*(int *)(board + 0x878) + 0x23c) != 0` before creating objects. This field is the **graphics quality setting**.

**Quality-gated refs** (only created when quality is non-zero/high):
- TIPPER, GLUEBIE, MACE, FAN, SAWBLADE, SPINNY, BONK, POPCYLINDER, MAGNIFYER
- BLOCKDAWG1, BLOCKDAWG2, BLOCKDAWG3
- E:ALERTSAW2, E:BRANCH

**Always-created refs** (created regardless of quality setting):
- BRIDGE, WATERWHEEL, SWIRL, SMASHER1-2, CATAPULT, DRAWBRIDGE, WINDMILL, TRAPDOOR, CHOMPER, TURRET
- LIFTER, SPEEDCYLINDER, TIMEBUTTON, NEONPLATFORM, DFLOOR1-4, TRODE, BELL, JUDGE
- SAW, SAW2, FALLOUT1, WOBBLY1-7, WAVY1, LOOPER, GEAR, BIGGEAR, ROTATOR, PENDULUM
- BBRIDGE1-2, SECRET, SECRETUNLOCK, BADBALL, PILLAR, TARBUBBLE, MOUSETRAP, LAUNCH

40 quality gate checks found across the factory function range (0x40A000–0x419000), verified by binary pattern matching: `MOV EAX, [reg+0x23C]; TEST EAX, EAX; JZ skip_creation`.

---

## Race vs Arena — Two Separate Board Systems

Hamsterball has **TWO independent Board systems**: one for Race mode and one for Arena mode. Each uses different Board constructors, different vtables, and different vtable[33] factories.

### RACE Board System (0x41Cxxx constructors)

**CORRECTED (June 2026):** Previous version of this section swapped race and arena constructors. The 0x41Cxxx constructors create RACE boards (verified by decompiling all 30 constructors and reading board name strings: "Board (X)" + "X RACE"). The 0x422xxx constructors create ARENA boards ("RumbleBoard (X)" + "X ARENA").

Race Board constructors are at `0x41CA40`–`0x424C20`. Each is called from a jump table at `0x42761C` (15 entries, indexed by `level_number - 1`). The jump table is reached via `JMP [EAX*4 + 0x42761C]` at `0x427080`.

Race Board vtables are at `0x4D1428`–`0x4D2298`. The Race factories form an **inheritance chain**: each level-specific factory checks its own refs, then falls through to call the base factory `0x4133E0` (which handles `PLATFORM`, `STANDS`).

| # | Race Level | Constructor | Board Vtable | Factory Addr | Refs Handled (from decompilation) |
|---|-----------|------------|-------------|-------------|-------------------------------------|
| 1 | Warm-up | 0x4224A0 | 0x4D1428 | 0x4133E0 | PLATFORM, STANDS (base only) |
| 2 | Beginner | 0x422550 | 0x4D14F0 | 0x4133E0 | PLATFORM, STANDS (base only) |
| 3 | Intermediate | 0x4226E0 | 0x4D15C0 | 0x4133E0 | PLATFORM, STANDS (base only) |
| 4 | Dizzy | 0x422790 | 0x4D1680 | 0x4143D0 | SPINNY, MACE, CATAPULT, TURRET, LIFTER, FAN + base |
| 5 | Tower | 0x4228C0 | 0x4D1740 | 0x414680 | MACE, CATAPULT, TURRET, LIFTER, FAN + base |
| 6 | Up | 0x422B10 | 0x4D17F8 | 0x414A20 | LIFTER, FAN, WOBBLY + base |
| 7 | Neon | 0x424860 | 0x4D1EC8 | 0x4173B0 | NEONPLATFORM, DFLOOR, TRODE + base |
| 8 | Expert | 0x423060 | 0x4D18C8 | 0x414BD0 | FAN, WOBBLY + base |
| 9 | Odd | 0x423220 | 0x4D1980 | 0x4133E0 | PLATFORM, STANDS (base only) |
| 10 | Toob | 0x4234E0 | 0x4D1A40 | 0x4133E0 | PLATFORM, STANDS (base only) |
| 11 | Wobbly | 0x423690 | 0x4D1B18 | 0x415460 | WOBBLY, PILLAR, POPCYLINDER + base |
| 12 | Glass | 0x424B60 | 0x4D2048 | 0x4133E0 | PLATFORM, STANDS (base only) |
| 13 | Sky | 0x423BF0 | 0x4D1BD8 | 0x415A30 | POPCYLINDER + base |
| 14 | Master | 0x424380 | 0x4D1C80 | 0x4133E0 | PLATFORM, STANDS (base only) |
| 15 | Impossible | 0x424EC0 | 0x4D2298 | 0x418760 | GEAR + base |

**Note**: The "Refs Handled" column above lists what the factory's decompiled code can match via `__strnicmp`. This is NOT the same as what refs actually exist in the MESHWORLD files — see `docs/VERIFIED_REFS_BY_LEVEL.md` for the ground-truth list of refs that are actually placed in each level's MESHWORLD file. Several refs the factories can handle (like PLATFORM, STANDS) are utility/system refs, not game objects.

**Key insight**: Levels 1, 2, 3, 9, 10, 12, 14 use ONLY the base factory — they handle only `PLATFORM`, `STANDS`, `N:BUMPER` in race mode. Their level-specific objects (tippers, bridges, etc.) are NOT created by vtable[33] in race mode — they are either part of the static level geometry or loaded via the Arena Board system.

**Race constructor chain**: Like the Arena constructors, Race Board constructors also form a chained function. Race L7 (Neon) loads Impossible sub-meshes, L12 (Glass) loads Impossible sub-meshes, and L14 (Master) loads Neon sub-meshes — all via conditional level-name checks (GLASSRACE, IMPOSSIBLERACE, NEONRACE).

### ARENA Board System (0x422xxx constructors)

Arena Board constructors are at `0x4224A0`–`0x424EC0`. Each is called from the Arena switch function at `0x426AB0` (jump table, 15 entries).

Arena Board vtables are at `0x4D04A8`–`0x4D21C0`. The Arena factories handle **many more ref types** than Race factories — they create ALL interactive objects for Arena mode.

| # | Arena Level | Constructor | Board Vtable | Factory Addr | Refs Handled (verified from binary) |
|---|-----------|------------|-------------|-------------|-------------------------------------|
| 1 | Warm-up Arena | 0x41CA40 | 0x4D04A8 | 0x419750 | (none — NoOp) |
| 2 | Beginner Arena | 0x4200E0 | 0x4D1098 | 0x419750 | (none — NoOp) |
| 3 | Intermediate Arena | 0x41CB20 | 0x4D05A0 | 0x40A550 | BRIDGE, TIPPER, WATERWHEEL, SWIRL, GLUEBIE, SMASHER1, SMASHER2 |
| 4 | Dizzy Arena | 0x41D060 | 0x4D0890 | 0x40A5F0 | TIPPER, WATERWHEEL, SWIRL, GLUEBIE, SMASHER1, SMASHER2 |
| 5 | Tower Arena | 0x41E340 | 0x4D0A08 | 0x40D7C0 | CATAPULT, MACE, DRAWBRIDGE, WINDMILL, TRAPDOOR, CHOMPER, BONK, FAN, SAWBLADE, BRIDGE, JUDGE, BELL |
| 6 | Up Arena | 0x420390 | 0x4D11A0 | 0x4117B0 | LIFTER, SPEEDCYLINDER, TIMEBUTTON, TarBubble, BRIDGE, TIPPER, BONK, BBRIDGE1-2, POPCYLINDER, BLOCKDAWG1-2, CATAPULT |
| 7 | Neon Arena | 0x424440 | 0x4D1DF0 | 0x416910 | NEONPLATFORM, DFLOOR1-4, TRODE, N:NEONPLATFORM, E:ZOOP, E:LIGHTSOFF, E:LIGHTSON, FLICKNING, N:BUMP, N:GLASS, N:TENBONUS1 |
| 8 | Expert Arena | 0x41EA40 | 0x4D0B00 | 0x40E250 | BONK, FAN, SAWBLADE, BRIDGE, JUDGE, BELL, E:SCORE, E:BELL, LIFTER, E:GRAVITY |
| 9 | Odd Arena | 0x41ED80 | 0x4D0BC0 | 0x40EC40 | LIFTER, E:GRAVITY, WOBBLY1-7, WAVY1, N:SQUAREWOBBLY, N:WAVY, SPINNY |
| 10 | Toob Arena | 0x41F4B0 | 0x4D0E78 | 0x40FB30 | SPINNY, FALLOUT1, BLOCKDAWG1-3, E:ALERTSAW2, E:BRANCH, N:SPINNY, N:SAWTEETH, N:BUMPER, PILLAR, MAGNIFYER |
| 11 | Glass Arena | 0x41F110 | 0x4D0D38 | 0x40F420 | WOBBLY1-7, WAVY1, N:SQUAREWOBBLY, N:WAVY, SPINNY, FALLOUT1, BLOCKDAWG1-3, E:ALERTSAW2, E:BRANCH, N:SPINNY |
| 12 | Wobbly Arena | 0x424A90 | 0x4D1F90 | 0x40AD80 | SMASHER1, SMASHER2, SECRETUNLOCK, SECRET, BADBALL |
| 13 | Sky Arena | 0x41F930 | 0x4D0FC8 | 0x410AD0 | POPCYLINDER, TRAPDOOR, N:BUMPER, VAC-IN, LIFTER, SPEEDCYLINDER, TIMEBUTTON |
| 14 | Master Arena | 0x4206D0 | 0x4D12B0 | 0x4121D0 | BRIDGE, TIPPER, BONK, BBRIDGE1-2, POPCYLINDER, BLOCKDAWG1-2, CATAPULT, GLUEBIE, N:SPINNER, N:BUMPER, E:LAUNCH |
| 15 | Impossible Arena | 0x424C20 | 0x4D21C0 | 0x417FE0 | LOOPER, GEAR, BIGGEAR, ROTATOR, PENDULUM, N:BOUNCE, N:ONROTATOR, N:ONGEAR |

### Level File → Board Mapping

The game's internal level numbers map to file paths differently for race vs arena. **Race file numbers do NOT match level order** — e.g. `level5` = Expert Race (L8), `level6` = Odd Race (L9).

| Race Level | File Path | Arena Level | File Path |
|-----------|----------|-----------|----------|
| L1 (Warm-up) | levels\level1 | L1 (Warm-up Arena) | levels\arena-WarmUp |
| L2 (Beginner) | levels\levelcascade | L2 (Beginner Arena) | levels\arena-beginner |
| L3 (Intermediate) | levels\level2 | L3 (Intermediate Arena) | levels\arena-intermediate |
| L4 (Dizzy) | levels\level3 | L4 (Dizzy Arena) | levels\arena-dizzy |
| L5 (Tower) | levels\level4 | L5 (Tower Arena) | levels\arena-tower |
| L6 (Up) | levels\levelup | L6 (Up Arena) | levels\arena-up |
| L7 (Neon) | levels\leveldark | L7 (Neon Arena) | levels\arena-neon |
| L8 (Expert) | levels\level5 | L8 (Expert Arena) | levels\arena-expert |
| L9 (Odd) | levels\level6 | L9 (Odd Arena) | levels\arena-Odd |
| L10 (Toob) | levels\level8 | L10 (Toob Arena) | levels\arena-Toob |
| L11 (Wobbly) | levels\level7 | L11 (Wobbly Arena) | levels\arena-Wobbly |
| L12 (Glass) | levels\levelglass | L12 (Glass Arena) | levels\arena-glass |
| L13 (Sky) | levels\level9 | L13 (Sky Arena) | levels\arena-Sky |
| L14 (Master) | levels\level10 | L14 (Master Arena) | levels\arena-Master |
| L15 (Impossible) | levels\levelimpossible | L15 (Impossible Arena) | levels\arena-impossible |

### Complete Ref Name → Constructor Mapping

Verified against the actual MESHWORLD binary data (46 unique object types). Numbered suffixes (e.g. TIPPER01, GEAR02) are collapsed to their base type. See `docs/VERIFIED_REFS_BY_LEVEL.md` for the full verified data.

| Ref Name | Constructor | Alloc Size | Quality-Gated? | Race Levels Found In |
|----------|------------|-----------|----------------|---------------------|
| BBRIDGE | BreakBridge_ctor | 0x1100 | No | Master |
| BELL | Tipper_Level_Ctor | 0x10E8 | No | Expert |
| BIGGEAR | Gear_ctor | 0x1514 | No | Impossible |
| BLOCKDAWG | Blockdawg_ctor | 0x1154 | Yes | Toob, Master |
| BONK | Bonk_ctor | 0x1200 | Yes | Expert, Master |
| BRIDGE | (returns pre-loaded mesh) | 0 | No | Intermediate, Expert, Master |
| CATAPULT | Catapult_ctor | 0x1108 | No | Tower, Master |
| CHOMPER | (position update only) | 0 | No | Tower |
| DFLOOR | RumbleBoard_Stands_ctor | 0x1104 | No | Neon |
| DRAWBRIDGE | Glass_Level_ctor | 0x113C | No | Tower |
| FALLOUT1 | Stands_CtorCollisionV2 | 0x10E8 | No | Toob |
| FAN | (FAN + FANSLOW + FAN(SUPER)(UP) variants) | varies | varies | Expert |
| GEAR | Gear_ctor | 0x1514 | No | Impossible |
| GLUEBIE | Gluebie_ctor | 0x110C | Yes | Dizzy, Master |
| JUDGE | Gear_Level_ctor | 0x1100 | No | Expert |
| LAUNCH | (launch pad marker) | 0 | No | Odd |
| LIFTER | Stands_CtorWithCollision / Rotator_ctor_sound | 0x10FC / 0x10F4 | No | Up, Odd |
| LOOPER | Looper_ctor | 0x1500 | No | Impossible |
| MACE | CascadeStands_Ctor | 0x110C | Yes | Tower |
| MAGNIFYER | (magnifying glass object) | 0 | No | Sky |
| MOUSETRAP | GlassStands_Ctor | 0x10F8 | No | Intermediate, Master |
| NEONPLATFORM | Stands_CtorRotator | 0x10EC | No | Neon |
| PENDULUM | Pendulum_ctor | 0x1504 | No | Impossible |
| PILLAR | (static mesh + collision) | 0 | No | Sky |
| POPCYLINDER | PopCylinder_ctor / Platform_ctor | 0x10E8 / 0x10F4 | Conditional | Sky, Master |
| ROTATOR | Rotator_ctor | 0x1508 | No | Impossible |
| SAW | Stands_CtorCollision | 0x1110 | Yes | Toob |
| SAW-BREAK | (breakable saw marker) | 0 | No | Expert |
| SAW2 | Stands_CtorSpeedCylinder | 0x1118 | Yes | Toob |
| SAWBLADE | Sawblade_Level_Ctor | 0x111C | Yes | Expert |
| SIGN-TARPIT | (sign handler + tar pit data) | 0 | No | Dizzy |
| SMASHER | (position update only) | 0 | No | Glass |
| SPEEDCYLINDER | Pendulum_ctor | 0x150C | No | Up |
| SPINNY | Rotator_ctor | 0x1508 | No | Toob |
| SWIRL | (returns pre-loaded mesh) | 0 | No | Dizzy |
| TARBUBBLE | (tar bubble animation) | 0 | No | Dizzy, Master |
| TIMEBUTTON | Rotator_ctor_nosound | 0x10E8 | No | Up |
| TIPPER | Tipper_ctor | 0x1104 | Yes | Dizzy, Master |
| TRAPDOOR | GlassStands_Ctor / Rotator_ctor | 0x10F8 / 0x10F4 | No | Tower, Sky |
| TRODE | RumbleBoard_Stands_ctor | 0x1104 | No | Neon |
| TURRET | Stands_ctor + CollisionLevel | 0x10D0 | No | Tower |
| WATERWHEEL | (returns pre-loaded mesh) | 0 | No | Dizzy |
| WAVY | Stands_CtorWithCollisionLevel | 0x1AE7C | No | Wobbly |
| WINDMILL | (returns pre-loaded mesh + collision) | 0 | No | Tower |
| WOBBLY | GameLevel_ctor | 0x1524 | No | Wobbly |

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

Confirmed by parsing the original MESHWORLD binary files with a spec-compliant parser. See `docs/VERIFIED_REFS_BY_LEVEL.md` for the full ground-truth data.

**46 unique object types** across 15 race levels. Refs listed below are Section 1 ref points only (object spawn markers). Utility refs (START, SAFESPOT, FLAG, BADBALL, SECRET, etc.) are omitted — see the standalone doc for the full list.

| # | Race Level | Object Refs (Section 1) |
|---|-----------|------------------------|
| L1 | Warm-up | (none) |
| L2 | Beginner | (none) |
| L3 | Intermediate | BRIDGE, MOUSETRAP |
| L4 | Dizzy | SIGN-TARPIT, TARBUBBLE, GLUEBIE, TIPPER, WATERWHEEL, SWIRL |
| L5 | Tower | CATAPULT, TRAPDOOR, DRAWBRIDGE, MACE, WINDMILL, CHOMPER, TURRET |
| L6 | Up | SPEEDCYLINDER, LIFTER, TIMEBUTTON |
| L7 | Neon | DFLOOR, TRODE, NEONPLATFORM |
| L8 | Expert | BONK, FAN, FANSLOW, SAWBLADE, BRIDGE, SAW-BREAK, JUDGE, BELL |
| L9 | Odd | LIFTER, LAUNCH |
| L10 | Toob | SPINNY, SAW, FALLOUT1, SAW2, BLOCKDAWG |
| L11 | Wobbly | WOBBLY, WAVY |
| L12 | Glass | SMASHER |
| L13 | Sky | PILLAR, MAGNIFYER, POPCYLINDER, TRAPDOOR |
| L14 | Master | BBRIDGE, BLOCKDAWG, BONK, BRIDGE, CATAPULT, GLUEBIE, MOUSETRAP, POPCYLINDER, TIPPER, TARBUBBLE |
| L15 | Impossible | LOOPER, GEAR, BIGGEAR, ROTATOR, PENDULUM |

Common utility refs in ALL levels: START (spawn point), SAFESPOT (respawn point), FLAG (checkpoint), BADBALL (AI ball).

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

### Arena Board Constructor Chain Architecture

The Arena Board constructors are **NOT separate functions** — they are **chained sections of one large function** starting at `0x41CA40`. Each section:

1. Compares the level name string (e.g. `"BEGINNERRACE"`, `"INTERMEDIATERACE"`, `"DIZZYRACE"`, `"UPRACE"`, etc.)
2. If the name matches, loads the appropriate sub-meshes for that level
3. Falls through to the next section for the next level

The jump table at `0x42761C` calls `operator_new` with different allocation sizes for each level (ranging from 17,260 to 25,752 bytes), then calls the constructor at `0x41CA40` which chains through all level sections.

**Key implication**: Since all Arena constructors share one function, ALL sub-meshes can be loaded by bypassing the level name comparison. The level name check (not separate function calls) is what prevents loading unrelated sub-meshes.

### Complete Verified Sub-Mesh Loading Table

| Arena Level | Level Name String | Sub-Meshes Loaded |
|-----------|------------------|-------------------|
| L1 Warm-up | (none — first section) | (no sub-meshes) |
| L2 Beginner | CASCADERACE | (no sub-meshes) |
| L3 Intermediate | INTERMEDIATERACE | Level2-Bridge, MOUSETRAP |
| L4 Dizzy | DIZZYRACE | Level3-Tipper, Level3-WaterWheel, Level3-Swirl, Level3-Gluebie |
| L5 Tower | (checks TOWERRACE) | Level4-Catapult, Level4-Drawbridge, Level4-Mace, Level4-Windmill, Level4-Turret |
| L6 Up | UPRACE | LevelUp-Lifter, LevelUp-SpeedCylinder, LevelUp-Button, Level2-Bridge, Level10-2PBridge, Level3-Tipper, Level10-Bridge1, Level10-Bridge2, Level9-PopCylinder1-2, Level8-Blockdawg1-2, Level4-Catapult, Level3-Gluebie |
| L7 Neon | (checks NEONRACE) | LevelDark-NeonPlatform, LevelDark-DFloor1-4, LevelDark-Trode, LevelDark-Flickning |
| L8 Expert | EXPERTRACE | Level5-Bridge |
| L9 Odd | ODDRACE | Level7-Wobbly1-7 |
| L10 Toob | TOOBRACE | Level8-Spinny, Level8-Saw, Level8-Fallout, Level8-Blockdawg1-2 |
| L11 Glass | GLASSRACE | Level7-Wobbly1-7 |
| L12 Wobbly | (no section found) | (no sub-meshes) |
| L13 Sky | SKYRACE | meshes/skypillar, meshes/magnifyingglass, Level9-PopCylinder1-2, Level9-Trapdoor, textures/clouds.png |
| L14 Master | MASTERRACE | Level2-Bridge, Level10-2PBridge, Level3-Tipper, Level10-Bridge1-2, Level8-Blockdawg1-2, Level4-Catapult, Level3-Gluebie, Level4-Mace, Level4-Catapult, Level4-Turret, Level7-Wobbly8 |
| L15 Impossible | IMPOSSIBLERACE | LevelImpossible-Looper, LevelImpossible-Gear, LevelImpossible-BigGear, LevelImpossible-Rotator, LevelImpossible-Pendulum, LevelImpossible-Gear |

All 46 sub-mesh files referenced by Arena Board constructors:

| Source Level | Sub-Mesh Files |
|-------------|---------------|
| Level2 | Level2-Bridge |
| Level3 | Level3-Tipper, Level3-WaterWheel, Level3-Gluebie, Level3-Swirl |
| Level4 | Level4-Catapult, Level4-Drawbridge, Level4-Mace, Level4-Windmill, Level4-Turret, Level4-Trapdoor1, Level4-Trapdoor2 |
| Level5 | Level5-Bridge, Level5-Bonk |
| Level6 | Level6-Lifter |
| Level7 (Neon) | Level7-Wobbly1-8, Level7-Wavy1 |
| Level8 | Level8-Spinny, Level8-Saw, Level8-Fallout, Level8-Blockdawg1-2 |
| Level9 (Sky) | Level9-Trapdoor, Level9-PopCylinder1-2 |
| Level10 | Level10-Bridge1-2, Level10-2PBridge |
| LevelDark (Neon) | LevelDark-NeonPlatform, LevelDark-DFloor1-4, LevelDark-Trode, LevelDark-Flickning |
| LevelUp | LevelUp-Lifter, LevelUp-SpeedCylinder, LevelUp-Button |
| LevelImpossible | LevelImpossible-Looper, LevelImpossible-Gear, LevelImpossible-BigGear, LevelImpossible-Rotator, LevelImpossible-Pendulum |

**Note**: "Level7" in the sub-mesh names = Neon Race (internal level 7), while "LevelDark" = the dark/neon-themed mesh set. Both are used by the Neon Arena constructor.

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

## Scene_CreateSigns (0x0040C270)

A separate dispatch function handles `SIGN` refs. This function is **not** vtable-gated — it runs for all levels and creates `StandsTipper` objects for any ref starting with "SIGN". Special case: `SIGN-TARPIT` gets additional tar-pit data from `scene+0x27C`.

---

## Complete Ref Name → Factory Reverse Index

This table shows every verified Section 1 ref name and which factory(ies) can create it. Only 46 verified object types are listed — refs that were previously listed here but do not exist in any original MESHWORLD file have been removed. See `docs/VERIFIED_REFS_BY_LEVEL.md` for the ground-truth source data.

**Note**: The "Arena Factories" column lists which Arena Board vtable[33] factory can create the object. The "Race Factories" column lists which Race Board factory handles it. Many refs are Arena-only (no Race factory handles them).

| Ref Name | Arena Factories | Race Factories | Required Slots |
|----------|----------------|----------------|----------------|
| BBRIDGE | Master | — | +0x436C, +0x4370, +0x4394, +0x4398 |
| BELL | Expert | — | +0x436C, +0x4370, +0x4374 |
| BIGGEAR | Impossible | — | +0x436C, +0x4370, +0x4374, +0x4378, +0x437C |
| BLOCKDAWG | Toob, Master | — | +0x436C to +0x4384 |
| BONK | Dizzy, Expert, Master | — | varies |
| BRIDGE | Beginner, Dizzy, Tower, Expert, Master | — | +0x436C, +0x4370, +0x4374 |
| CATAPULT | Tower, Master | Race Tower | +0x436C, +0x4378 |
| CHOMPER | Tower | — | +0x436C, +0x4378, +0x43B0 |
| DFLOOR | Neon | — | +0x4374 to +0x4390 |
| DRAWBRIDGE | Tower | — | +0x436C, +0x4378, +0x4390 |
| FALLOUT1 | Toob, Glass | — | +0x436C to +0x4384 |
| FAN | Expert | Race Expert | varies |
| FANSLOW | Expert | — | +0x436C, +0x4370, +0x4374 |
| GEAR | Impossible | Race Impossible | +0x436C, +0x4370, +0x4374, +0x4378, +0x437C |
| GLUEBIE | Beginner, Intermediate, Master | — | +0x436C, +0x4370, +0x4374 |
| JUDGE | Expert | — | +0x436C, +0x4370, +0x4374 |
| LAUNCH | — | Race Odd | (race) |
| LIFTER | Tower, Expert, Odd | Race Up, Odd | varies |
| LOOPER | Impossible | — | +0x436C, +0x4370, +0x4374, +0x4378, +0x437C |
| MACE | Tower | Race Tower | +0x436C, +0x4378 |
| MAGNIFYER | Sky | — | +0x436C to +0x4384 |
| MOUSETRAP | Master | — | +0x436C |
| NEONPLATFORM | Neon | — | +0x4374 to +0x4390 |
| PENDULUM | Impossible | — | +0x436C to +0x437C |
| PILLAR | Sky | — | +0x436C |
| POPCYLINDER | Sky, Master | Race Sky | varies |
| ROTATOR | Impossible | — | +0x436C to +0x437C |
| SAW | Toob | — | +0x436C to +0x4384 |
| SAW-BREAK | Expert | — | +0x436C, +0x4370, +0x4374 |
| SAW2 | Toob | — | +0x436C to +0x4384 |
| SAWBLADE | Expert | — | +0x436C, +0x4370, +0x4374 |
| SIGN-TARPIT | — | (separate Scene_CreateSigns dispatch) | N/A |
| SMASHER | Glass | — | +0x4344 |
| SPEEDCYLINDER | Tower, Sky | Race Up | +0x436C, +0x4394, +0x4398 |
| SPINNY | Toob, Glass | — | +0x436C to +0x4384 |
| SWIRL | Beginner, Intermediate | — | +0x436C, +0x4370, +0x4374 |
| TARBUBBLE | Tower | — | +0x436C, +0x4394 |
| TIMEBUTTON | Tower, Sky | Race Up | +0x436C, +0x4394, +0x4398 |
| TIPPER | Beginner, Intermediate, Master | — | +0x436C, +0x4370, +0x4374 |
| TRAPDOOR | Tower, Sky | — | +0x436C, +0x4378, +0x4390 |
| TRODE | Neon | — | +0x4374 to +0x4390 |
| TURRET | — | Race Tower | (race) |
| WATERWHEEL | Beginner, Intermediate | — | +0x436C, +0x4370, +0x4374 |
| WAVY | Odd, Glass | Race Wobbly | +0x436C to +0x4384 |
| WINDMILL | Tower | — | +0x436C, +0x4378, +0x43A4 |
| WOBBLY | Odd, Glass | Race Wobbly | varies |

1. **Two independent Board systems**: RACE Boards (constructors at 0x422xxx, vtables 0x4D1428–0x4D2298) and ARENA Boards (constructors at 0x41Cxxx, vtables 0x4D05A0–0x4D21C0). Race mode uses App+0x237=0; Arena mode uses App+0x237=1. Each has its own vtable[33] factory.
2. **vtable[33] dispatch**: `Scene_CreateDynamicObjects` (0x0040C430) iterates MESHWORLD Section 1 ref points, and for each ref, calls `board->vtable[33](board, refName, &outObj, &outCol, refEntry)` at instruction `0x0040C4BA`. If the factory returns NULL, the ref is silently ignored.
3. **Factory chain (Race)**: Race factories form an inheritance chain — each level-specific factory checks its own refs, then falls through to the base factory `0x4133E0` which handles `PLATFORM`, `STANDS`, `N:BUMPER`.
4. **Factory chain (Arena)**: Arena factories are standalone — each handles its complete ref set without falling through. The most inclusive Arena factory is Master (13 ref types), followed by Dizzy (12 ref types) and Tower (13 ref types).
5. **Sub-mesh preloading**: Each Board constructor preloads specific MESHWORLD files into board struct slots (+0x4344 through +0x43B4). Factories access these slots to get mesh data for object creation. **Factories do NOT null-check these slots** — calling a factory with unloaded slots causes an access violation crash.
6. **N:/E: handler (0x0040C5D0)**: A separate system that processes entity names from Section 3 objects to set behavioral flags (gravity modifiers, launch pads, bumpers, etc.). Called from 25 sites within the Arena factories.
7. **Universal ref loader mod**: A bass.dll proxy that hooks the vtable[33] dispatch at 0x0040C4BA and tries all 13 Arena factories in sequence, with per-factory sub-mesh slot safety checks. Allows loading any ref type into any level (limited by sub-mesh availability).

### How to Load Any Ref Into Any Level

**Option A — MESHWORLD mod only (limited)**:
For refs handled by the Race base factory (PLATFORM, STANDS), simply add the ref name to the MESHWORLD Section 1. No code changes needed. Note: the base factory handles very few object types — most objects require level-specific factory support.

**Option B — MESHWORLD mod + DLL hook (any ref)**:
1. Add the desired ref name to the level's MESHWORLD Section 1.
2. Install the universal-ref-loader bass.dll proxy.
3. The DLL mod will try all Arena factories when the level's own factory doesn't recognize the ref.
4. **Limitation**: The ref's sub-mesh data must already be loaded in a board slot. If the ref requires a sub-mesh that wasn't loaded by the current Board constructor, the factory will be skipped (safety check prevents crash, but ref won't be created).

**Option C — MESHWORLD mod + pre-loaded sub-meshes (full support)**:
1. Add the ref name to the level's MESHWORLD Section 1.
2. Pre-load the required sub-mesh by modifying the Board constructor to load additional MESHWORLD files (binary patch the constructor to add MeshWorld loading calls).
3. Install the universal-ref-loader bass.dll proxy.
4. All 46 verified ref types can now be loaded into any level.
