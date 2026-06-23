# Hamsterball Reference Node (Ref) Loading System

## Overview

Hamsterball levels load "reference nodes" (refs) from MESHWORLD files. Each ref has a name (e.g. `N:SPEEDCYLINDER`, `N:BONK`, `N:GEAR`) that determines what game object gets created. The game uses a **code-gated vtable dispatch** system — not a file-driven registry — to decide which refs to instantiate. Each level's Board subclass overrides a virtual method (vtable slot 33, offset +0x84) that contains the factory logic for that level's objects.

**Key finding:** Adding a ref name to a MESHWORLD file is **necessary but not sufficient**. The ref must ALSO be handled by the Board's vtable[33] factory function, or the game will silently skip it. To load any ref into any level, you must either:
1. Patch the target Board's vtable[33] to point to a factory that handles the desired ref names, OR
2. Use a DLL mod that hooks `Scene_CreateDynamicObjects` and intercepts the ref dispatch.

---

## Architecture

### 1. MESHWORLD Ref Points (Section 1)

Each MESHWORLD file contains a Section 1 with "reference points" — named position markers stored as:
- `name` (string, e.g. `N:SPEEDCYLINDER`)
- `position` (3 floats: x, y, z)
- `rotation` (3 floats)
- `scale` (3 floats)
- Additional flags and color data

These are the "refs" that the factory dispatch reads. The `N:` prefix is part of the name string in the binary.

### 2. Scene_CreateDynamicObjects (0x0040C4BA) — The Central Dispatch

```
void __fastcall Scene_CreateDynamicObjects(int *board)
```

This function iterates over all ref points in the loaded MESHWORLD level data. For each ref:
1. Gets the ref name string (`*puVar4` — the first DWORD of the ref entry)
2. Calls `board->vtable[33](refName, &out_obj, &out_col, refEntry)` — i.e. `(**(code**)(*board + 0x84))(name, &out1, &out2, refPtr)`
3. If the factory returns a non-null object, it:
   - Initializes a Timer
   - Sets graphics position/scale from the ref entry
   - Appends the object to `board+0x2578` (the general object list)
   - Appends to the scene's SceneObject list
   - Calls the object's vtable[0x58] (Update) and vtable[0x54] (Render)

The ref entry structure (param_5 / puVar4) is an array of DWORDs:
- `[0]` = name string pointer
- `[1],[2],[3]` = position (x, y, z as floats)
- `[4],[5],[6]` = rotation/scale data
- `[5]` = used for ScaleX calculation
- `[0x14]` = additional float parameter (used by some factories)

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

The handler is called from 25 sites within the vtable[33] factory functions. Each factory calls the handler after creating the object, passing the mesh entity so it can check the entity name field.

### 5. The `Scene+0x23C` Quality Gate

Many (but not all) factory branches check `*(int *)(*(int *)(board + 0x878) + 0x23c) != 0` before creating objects. This field is the **graphics quality setting** — objects like TIPPER, BONK, MACE, SAW, SAW2, BLOCKDAWG, GEARS, etc. are only created when quality is non-zero (high quality). Objects like BRIDGE, SPEEDCYLINDER, LIFTER, POPCYLINDER, TRAPDOOR are created regardless of quality.

---

## Complete Factory Dispatch Table

### Level → Factory Mapping

| # | Level | Board Vtable | Factory Addr | Factory Name | Refs Handled |
|---|-------|-------------|-------------|-------------|--------------|
| 1 | Warm-up Race | 0x4D04A8 | 0x419750 | NoOp | (none — returns null) |
| 2 | Beginner Race | 0x4D1098 | 0x419750 | NoOp | (none) |
| 3 | Intermediate Race | 0x4D05A0 | 0x40A550 | BridgeFactory | BRIDGE |
| 4 | Dizzy Race | 0x4D0890 | 0x40A5F0 | DizzyFactory | TIPPER, WATERWHEEL, SWIRL, GLUEBIE |
| 5 | Tower Race | 0x4D0A08 | 0x40D7C0 | TowerFactory | CATAPULT, MACE, DRAWBRIDGE, WINDMILL, TRAPDOOR, CHOMPER, TURRET |
| 6 | Up Race | 0x4D11A0 | 0x4117B0 | CreateSpeedCylinder | LIFTER, SPEEDCYLINDER, TIMEBUTTON |
| 7 | Neon Race | 0x4D1DF0 | 0x416910 | NeonFactory | NEONPLATFORM, DFLOOR1-4, TRODE |
| 8 | Expert Race | 0x4D0B00 | 0x40E250 | CreateSawblade | BONK, [UP/TOW], SAWBLADE, BRIDGE, JUDGE, BELL |
| 9 | Odd Race | 0x4D0BC0 | 0x40EC40 | LifterFactory | LIFTER |
| 10 | Toob Race | 0x4D0E78 | 0x40FB30 | ToobFactory | SPINNY, SAW, SAW2, FALLOUT1, BLOCKDAWG1-3 |
| 11 | Wobbly Race | 0x4D0D38 | 0x40F420 | WobblyFactory | WOBBLY1-7, WAVY1 |
| 12 | Glass Race | 0x4D1F90 | 0x40AD80 | SmasherFactory | SMASHER1, SMASHER2 |
| 13 | Sky Race | 0x4D0FC8 | 0x410AD0 | SkyFactory | POPCYLINDER, TRAPDOOR |
| 14 | Master Race | 0x4D12B0 | 0x4121D0 | CreateLevelObjects | BRIDGE, TIPPER, BONK, BBRIDGE1-2, POPCYLINDER, BLOCKDAWG1-2, CATAPULT, GLUEBIE |
| 15 | Impossible Race | 0x4D21C0 | 0x417FE0 | CreateMechanicalObjects | LOOPER, GEAR, BIGGEAR, ROTATOR, PENDULUM |

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
    CreateSpeedCylinder(board, refName, outObj, outCol, refEntry);
    if (*outObj != NULL) return;
    
    CreateSawblade(board, refName, outObj, outCol, refEntry);
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

Each Board constructor preloads specific MeshWorld files into board struct slots:

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

For the DLL mod approach, you can load additional MeshWorld files at runtime:
```c
// Load a mesh world into a free board slot
void* mesh = operator_new(0x10d0);
MeshWorld_ctor(mesh, graphicsDevice, "Levels\\Level3-Tipper");
*(void**)(board + 0x436C) = mesh;
```

---

## Scene_CreateSigns (0x0040C400)

A separate dispatch function handles `SIGN` refs. This function is **not** vtable-gated — it runs for all levels and creates `StandsTipper` objects for any ref starting with "SIGN". Special case: `SIGN-TARPIT` gets additional tar-pit data from `scene+0x27C`.

---

## Summary

1. **Two independent dispatch systems**: (A) vtable[33] factory creates objects from bare-name ref points in MESHWORLD Section 1; (B) the N:/E: handler at 0x0040C5D0 processes entity names from Section 3 meshes to set behavioral flags.
2. **The `N:` prefix is NOT stripped** — it's part of the entity name in Section 3, not the ref point name in Section 1. Ref points use bare names.
3. **Each level has its own vtable[33] factory** with different supported ref names.
4. **Master Race has the most inclusive factory** — it handles 9 different ref types borrowed from other levels.
5. **To load any ref into any level**, you need a DLL mod that either patches the vtable or hooks the dispatch, AND ensures the required sub-meshes are preloaded.
6. **Quality gating** (`scene+0x23C`) blocks creation of complex visual objects on low quality settings.
7. **The N:/E: handler is called from within the factories** — it runs as a sub-step of object creation, not as a separate top-level dispatch.
