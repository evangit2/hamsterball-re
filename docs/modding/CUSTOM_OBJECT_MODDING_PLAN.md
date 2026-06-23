# Custom Object Types — Modding Plan

**Goal**: Add custom object types to Hamsterball levels that interact with the game engine
without modifying the original EXE.

## Architecture Overview

There are two injection points for custom objects:

1. **Ref Points (Section 1)** — Game logic objects looked up by name during scene construction
2. **Octree Geoms (Section 6)** — Collision/render geometry with named event triggers

Both are purely data-driven — the game loads them from the .MESHWORLD binary file.
No EXE modification is needed to **add** named objects to levels. The constraint is
that the EXE only **recognizes** specific name prefixes for factory dispatch and
collision events.

## Strategy: Three Approaches (Escalating Complexity)

### Approach A: Binary MW File Editing (Data-Only Modding)

**What you can do WITHOUT touching the EXE:**

1. **Add new E: event zones** with existing event names to any level
   - Insert named collision geometry (e.g., `E:JUMP`, `E:NODIZZY<TIME>300</TIME>`)
   - The collision dispatcher (`DispatchCollisionEvents`) will process any `E:`/`N:` name it recognizes
   - Unrecognized names are silently ignored (no crash, no effect)

2. **Add new N: interactive collision objects** with existing names
   - E.g., add `N:GOAL` to a new location, add `N:TARPIT` zones, `N:WATER` zones
   - These trigger existing game logic via the collision dispatch chain

3. **Reposition existing ref points** (START, FLAG, SAFESPOT, etc.)
   - Edit Section 1 coordinates to change spawn points, checkpoint locations

4. **Add BADBALL enemy balls** with custom parameters
   - `<CHASE>`, `<HOME>`, `<SIZE>`, `<SPINDISTANCE>` tags are parsed from the name string

5. **Add factory objects** (BRIDGE, TIPPER, BONK, CATAPULT, etc.)
   - The factory dispatcher (`CreateLevelObjects`) matches by prefix
   - Adding a `CATAPULT` ref point + associated mesh data creates a working catapult

6. **Change materials/textures** on existing geometry
   - Per-geom material data in Section 6 (ambient/diffuse/specular/emissive/power/texture)

**Tool needed**: A .MESHWORLD binary editor that can parse and re-serialize the format.
The parser in this project's `docs/MESHWORLD_OBJECT_SYSTEM.md` documents the full format.

**Limitations**:
- Cannot create genuinely NEW behavior (new event names are ignored)
- Cannot add new factory types (the prefix list is hardcoded in the EXE)
- Cannot change physics parameters (gravity, friction, etc. are in the EXE)

### Approach B: DLL Proxy Hooking (Runtime Behavior Extension)

**What you can do with a proxy DLL (bass.dll or d3d8.dll):**

1. **Hook the collision dispatch** to intercept `DispatchCollisionEvents` (0x40C5D0)
   - Intercept `collider->name` before the EXE processes it
   - Recognize custom `E:CUSTOM_*` names and execute custom logic
   - Then call the original `DispatchCollisionEvents` for standard events

2. **Hook the factory dispatcher** `CreateLevelObjects` (0x4121D0)
   - Intercept ref point names before the EXE matches them
   - Recognize custom prefixes (e.g., `CUSTOM_BOSS`, `CUSTOM_TELEPORTER`)
   - Allocate custom game objects, register them in the scene's active object list

3. **Hook `Mesh_FindClosestCollision`** (0x465D90)
   - Add custom collision response for custom named objects
   - Could implement teleporters, speed pads, gravity wells, etc.

4. **Add per-frame update hooks** via the scene update loop
   - Hook `Scene_UpdateBallsAndState` to run custom object logic each frame
   - Access game state through known struct offsets (Scene, Ball, App)

**Implementation pattern** (bass.dll proxy, proven to work for FPS mods):
```c
// Hook DispatchCollisionEvents to intercept custom event names
typedef void (__thiscall *DispatchCollisionEvents_t)(void* this, int* ball, int* collObj);
DispatchCollisionEvents_t original_DispatchCollisionEvents = NULL;

void __thiscall Hooked_DispatchCollisionEvents(void* this, int* ball, int* collObj) {
    char* eventName = *(char**)(collObj[1] + 0x864);
    
    // Check for custom event names
    if (strnicmp(eventName, "E:CUSTOM_TELEPORT", 17) == 0) {
        // Custom teleport logic: move ball to a target ref point
        Vec3 target = HashTable_Lookup(scene_hashtable, "TELEPORT_TARGET");
        ball->pos_x = target.x;
        ball->pos_y = target.y;
        ball->pos_z = target.z;
        return; // Skip original handler
    }
    
    // Fall through to original handler for standard events
    original_DispatchCollisionEvents(this, ball, collObj);
}
```

**Key addresses for hooking**:
- `DispatchCollisionEvents` = 0x40C5D0 (base collision event handler)
- `Level_HandleCollision` = 0x40DCD0 (race-specific events)
- `Arena_HandleCollision` = 0x40E6A0 (arena-specific events)
- `CreateLevelObjects` = 0x4121D0 (factory dispatcher)
- `CreateExpertLevelObjects` = 0x40E250 (arena factory sub-dispatcher)
- `Mesh_FindClosestCollision` = 0x465D90 (collision raycast)
- `Scene_SpawnBallsAndObjects` = 0x41C5B0 (scene initialization)
- `Scene_UpdateBallsAndState` — per-frame update (call via vtable)
- Image base: 0x400000 (use RVA = addr - 0x400000 for ASLR safety)

**Limitations**:
- Requires C/C++ compilation (MinGW cross-compile, proven workflow)
- Must handle VS2003 CRT heap isolation (use `HeapAlloc` with game heap, not `malloc`)
- All struct offsets must be verified against the specific EXE version
- DLL proxy pattern is well-established (bass.dll proxy works, documented in skill)

### Approach C: Full EXE Patching (Binary Modification)

**What you can do by patching the EXE directly:**

1. **Add new factory dispatch entries** in `CreateLevelObjects`
   - Extend the `__strnicmp` chain to recognize new prefixes
   - Allocate and initialize custom game object structs

2. **Add new collision event handlers** in `DispatchCollisionEvents`
   - Extend the `__stricmp` chain to handle new `E:`/`N:` names

3. **Modify physics constants** (gravity, friction, max speed, etc.)

4. **Add new vtable entries** for custom object update/render/dtor

5. **Patch the octree loader** to handle custom data sections

**Implementation**: Direct binary patching with hex editor or patcher script
(similar to the 8ball AI patches at 0x4083D3, 0x4085D1, 0x408598).

**Limitations**:
- Most invasive approach — changes the original EXE
- Must preserve code alignment and instruction boundaries
- Limited by available code cave space for new logic
- Each patch must be version-specific

## Recommended Implementation Plan

### Phase 1: MW File Editor Tool (Approach A)
Build a Python tool that can:
1. Parse any .MESHWORLD binary file into structured data
2. Edit ref points (add/remove/reposition)
3. Add named collision geometry to the octree
4. Modify materials and textures
5. Re-serialize to valid .MESHWORLD format

This enables immediate modding with all existing object types.

### Phase 2: Custom Event DLL (Approach B)
Build a bass.dll proxy that:
1. Hooks `DispatchCollisionEvents` at 0x40C5D0
2. Recognizes custom `E:CUSTOM_*` event names
3. Implements a registry of custom event handlers (configurable via INI)
4. Provides custom events: teleport, speed pad, gravity flip, ball size change, etc.

### Phase 3: Custom Factory Objects (Approach B+)
Extend the proxy DLL to:
1. Hook `CreateLevelObjects` at 0x4121D0
2. Recognize custom ref point prefixes
3. Allocate custom game objects with update/render/collision vtables
4. Register in scene's active object list (+0x2578)
5. Implement custom object types: moving platforms, collectibles, bosses

## Custom Object Type Design Space

Given the engine architecture, feasible custom object types include:

| Type | Implementation | Data Required |
|------|---------------|---------------|
| **Teleporter** | Hook DispatchCollisionEvents, `E:CUSTOM_TELEPORT` → move ball | Target ref point in Section 1 |
| **Speed Pad** | Hook DispatchCollisionEvents, `E:CUSTOM_SPEED` → add velocity | Direction + magnitude in name |
| **Gravity Zone** | Hook DispatchCollisionEvents, `E:CUSTOM_GRAVITY` → change gravity | Direction in XML tags |
| **Size Changer** | Hook DispatchCollisionEvents, `E:CUSTOM_GROW/SHRINK` → change ball radius | New radius in name |
| **Moving Platform** | Hook factory, custom prefix → animate mesh | Waypoints in Section 1 |
| **Collectible** | Hook DispatchCollisionEvents, `N:CUSTOM_COIN` → score + disappear | Score value in name |
| **Custom Hazard** | Hook DispatchCollisionEvents, `N:CUSTOM_HAZARD` → damage/respawn | Damage value in name |
| **Checkpoints++** | Hook DispatchCollisionEvents, `E:CUSTOM_CHECKPOINT` → track progress | Order in name |
| **Wind Zone** | Hook per-frame, apply force in region | Direction + strength |
| **Ice Surface** | Hook per-frame, reduce friction in zone | Friction value |

## Key Technical Constraints

1. **Image base**: 0x400000. Use RVA = addr - 0x400000, then `GetModuleHandle(NULL) + RVA` for ASLR safety.
2. **CRT heap**: operator_new/malloc crash cross-module (VS2003 CRT critical sections). Use `HeapAlloc(GetProcessHeap(), 0, size)` directly.
3. **Struct sizes**: Ball=0xC60, Scene=varies (0x6000+), App=0x2328, CollisionLevel=0x10D0, MeshBuffer=0x874.
4. **16384 triangle cap**: Game crashes if a single level exceeds 2^14 triangles.
5. **String format**: Length-prefixed (int32 + data with NUL terminator).
6. **Axis swap**: All positions in MW files are x,z,y (Max Z-up → engine Y-up). NOT in-engine — only in file I/O.
7. **Name writing rule**: Exporter only writes geom names where `name[1] == ':'` or name contains `"NOCOLLIDE"`. Custom objects must use a `X:` prefix to have their name preserved.
8. **CollisionLevel**: Each level's collision geometry is a separate MeshWorld loaded via `Level_LoadCollision`. Named collision faces are stored as MeshBuffer objects in the MeshWorld's object list.
