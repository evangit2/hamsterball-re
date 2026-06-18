# Hamsterball Event Plane System

## Overview

Event planes are **invisible collision meshes** in Hamsterball levels. They use the `E:` name prefix in the `.COL` (collision) binary file. When the ball enters an event plane's collision volume, the game dispatches behavior based on the name string (e.g., `E:JUMP`, `E:LIMIT`, `E:NODIZZY<TIME>300</TIME>`).

This document covers:
1. How event planes are loaded from MESHWORLD/.COL files
2. How they're stored in memory (MeshBuffer struct)
3. How collision detection finds them
4. How the 3-tier dispatch system routes events by name
5. Every known event type with exact addresses, parameters, and Ball/Scene offsets

---

## 1. Loading Pipeline

### Source: `Level_LoadCollision` (0x00465260)

The `.COL` file is loaded alongside the `.MESHWORLD` visual mesh. The collision loader reads:

```
Header: 24 bytes → MeshWorld+0x45C (transform/flags)
int32:   sublevel_count
  if < 1 → single-mesh mode (read objects below)
  else   → multi-level mode (recursive sub-load)
```

**Per collision object (single-mesh mode):**
1. `CreateMeshBuffer(0x874)` — allocate a MeshBuffer (2148 bytes)
2. Append to `MeshWorld->object_list` (+0x2C)
3. Read `int32 name_length` then `char[name_length]` — the name string
4. **Prefix detection** (case-insensitive `strnicmp`):
   - `"N:"` prefix → `buf->interactive = 1` (offset +0x85D) — visible + collidable
   - `"E:"` prefix → `buf->interactive = 1` (+0x85D) **AND** `buf->no_render = 1` (+0x863) — invisible event trigger
5. Read `int32 face_count`
6. For each face: allocate 0x60 bytes, read 9 floats (v0 xyz, v1 xyz, v2 xyz), compute face normal via cross product, store per-vertex normals (flat shading), append to object's face list

### Key Point: E: objects are NEVER rendered

The `no_render` flag (+0x863) causes `Scene_RenderAllObjects` (0x45E0E0) to skip them entirely. They exist only in the collision mesh for trigger detection.

---

## 2. MeshBuffer Struct (Collision Object) — 0x874 bytes

| Offset | Size | Field | Description |
|--------|------|-------|-------------|
| +0x000 | 4 | vtable* | Virtual table pointer |
| +0x00C | 0x418 | AthenaList | Face list (triangle faces) |
| +0x217 | 1 | byte | render_flag (= 0 for collision objects) |
| +0x85D | 1 | byte | **interactive_flag** (1 for N: and E: prefixes) |
| +0x863 | 1 | byte | **no_render_flag** (1 for E: prefix only) |
| +0x864 | 4 | char** | **Name string pointer** (e.g., "E:JUMP") |

### CollisionFace Layout (0x60 bytes per triangle)

| Offset | Field |
|--------|-------|
| +0x00 | float v0.x, v0.y, v0.z |
| +0x0C | float normal.x, normal.y, normal.z (flat shading) |
| +0x18 | float v1.x, v1.y, v1.z |
| +0x24 | float normal.x, normal.y, normal.z |
| +0x30 | float v2.x, v2.y, v2.z |
| +0x3C | float normal.x, normal.y, normal.z |

---

## 3. Collision Detection → Event Name Retrieval

When a collision is detected, the handler receives a `collObj` pointer (2-element array of pointers). The event name is retrieved as:

```c
char *eventName = *(char **)(collObj[1] + 0x864);  // name string from MeshBuffer
```

This is the exact same string that was loaded from the `.COL` file. The dispatch code then does `stricmp`/`strnicmp` comparisons on this string to determine which behavior to trigger.

---

## 4. Two-Tier Dispatch System

Collision events are dispatched through a **2-tier handler chain**. Level and Arena handlers are **parallel, not chained** — they never call each other. Both delegate to the shared base handler (`CreateNoDizzy`) as the final step. The scene's vtable determines which top-level handler runs.

Note: `Ball_AdvancePositionOrCollision` (0x4564C0) handles only geometric collision detection (velocity integration, mesh intersection via `CollisionLevel->vtable[0x1C]`, max-speed clamping). It does NOT dispatch event-name-based collision events. Event dispatch is triggered from the ball update chain (`ball->vtable[0x10]`, called by `Scene_UpdateBallsAndState`).

### Tier 1a: `Level_HandleCollision` (0x0040DCD0) — Level Events
**Signature:** `void __thiscall Level_HandleCollision(Scene *scene, Ball *ball, Collider *collider)`

Handles level-specific events (catapults, trapdoors, maces, bite damage), then calls `CreateNoDizzy`.

### Tier 1b: `Arena_HandleCollision` (0x0040E6A0) — Arena Events
**Signature:** `void __thiscall Arena_HandleCollision(Scene *scene, Ball *ball, Collider *collider)`

Handles arena/rumble events (hammers, saw blades, judges, bells), then calls `CreateNoDizzy`.

### Base Tier: `CreateNoDizzy` (0x0040C5D0) — Shared Base Handler
**Signature:** `void __thiscall CreateNoDizzy(void *this, int *ball, int *collObj)`

Handles ALL common events. This is always called last regardless of level type.

---

## 5. Complete Event Reference

### 5a. Base Handler Events — `CreateNoDizzy` (0x0040C5D0)

| Event String | Match Type | Condition | Effect | Ball/Scene Offsets |
|---|---|---|---|---|
| `N:SECRET` | `__strnicmp` 8 chars | — | Mark rotator as triggered | `*collObj + 0x47C` |
| `N:UNLOCKSECRET` | `__strnicmp` 14 chars | — | Check arena unlock | — |
| `E:NODIZZY` | `__strnicmp` 9 chars | — | Anti-dizzy zone. Parses `<TIME>value</TIME>` tags via MWParser_ReadTag. Duration passed to Ball_RecordBest | — |
| `E:SAFESWITCH` | `__strnicmp` 12 chars (0xC) | — | Switch state. If no `(` in name: clear `ball+0x30B=0`. If `(` found: `strcpy(ball+0xC2C, paren_content)` | ball+0x30B, ball+0xC2C |
| `E:LIMIT` | `__stricmp` exact | — | Arena finish line. Clear velocity flag, set limit-hit | ball+0x1DA=0, ball+0x2E9=1, board+0x47B4-47C0 |
| `E:BREAK` | `__stricmp` exact | — | Call ball bounce callback | `ball->vtable[0x20]()` |
| **`E:JUMP`** | `__stricmp` exact | `impactCounter < 1` | Bounce pad. Play 3D jump sound, set cooldown=10, upward force=0.025, freeze=10 frames, +200 score | ball+0x1F7=10, ball+0xA7=0.025f, ball+0xA8=1, ball+0x202=10 |
| `E:ACTION` | `__strnicmp` 8 chars | — | Parse XML tags: `<ONCE>TRUE</ONCE>` = one-time trigger (track in ball+0xCB list), `<SCORE>500</SCORE>` = award points with difficulty modifier | ball+0xCB (once list) |
| `E:TRAJECTORY` | `__strnicmp` 12 chars | — | Parse `<X>`, `<Y>`, `<Z>` tags. Set ball collision mesh direction | mesh+0xCA4/CA8/CAC |
| `N:NOCONTROL` | `__stricmp` exact | — | Disable ball input for 10 frames | ball+0x202=10 |
| `N:WATER` | `__stricmp` exact | — | Set water flag, start 10-frame timer | ball+0x2D5=1, ball+0xB6=10 |
| `N:TARPIT` | `__stricmp` exact | first entry only | Play tar sound. Set in-tar flag, clear velocity | ball+0xB3=1, ball+0x1DA=0 |
| `E:DROPIN` | `__stricmp(eventName+2, "DROPIN")` | `dist > threshold && dropinCounter < 1` | Play dropin sound, set cooldown 50 frames, +200 score | ball dropinCounter=50 |
| **`E:PIPEBONK`** | `__stricmp(eventName+2, "PIPEBONK")` | `pipebonkCounter < 1` | Play random pipe sound (3 variants), set cooldown 10 frames, +100 score | ball pipebonkCounter=10 |
| `E:POPOUT` | `__stricmp(eventName+2, "POPOUT")` | `popoutCounter < 1` | Play popout sound, set cooldown 50 frames, +100 score | ball popoutCounter=50 |
| `N:GOAL` | `strncmp` 7 chars | `!ball.finished && ball.active` | Set goalReached=1, play "Goal!" music, mark player finished | — |
| `N:MOUSETRAP` | `strncmp` 12 chars | — | RNG seed, deflect ball direction × trap speed | — |

**Note on `E:DROPIN`, `E:PIPEBONK`, `E:POPOUT`:** These three events use `eventName+2` comparison, skipping the "E:" prefix. The name in the `.COL` file is still `"E:PIPEBONK"` etc.

### 5b. Level Handler Events — `Level_HandleCollision` (0x0040DCD0)

| Event String | Match | Condition | Effect | Scene Offsets |
|---|---|---|---|---|
| `E:CATAPULTBOTTOM` | `stricmp` exact | `ball.cooldown < 1` | Set cooldown=1000, find matching catapult in list, store ball pointer, launch catapult, play sound | scene+0x43B8 (catapult list), catapult+0x10EC (ball ptr) |
| `E:OPENSESAME` | `stricmp` exact | — | Open first trapdoor | scene+0x4BEC |
| `N:TRAPDOOR` | `stricmp` exact | — | Activate matching trapdoor(s) | scene+0x47D0 |
| `E:BITE` | `stricmp` exact | — | Set damage timer=0, damage amount=25.0 | scene+0x43A8=0, scene+0x43A0=25.0 |
| `E:MACETRIGGER` | `stricmp` exact | — | Set all maces active | mace list+0x5000, mace+0x10F0=1 |
| `N:MACE` | `stricmp` exact | mace active & speed ≠ 80.0 | Call BounceForce on ball | `ball->vtable[0x20]()` |

### 5c. Arena Handler Events — `Arena_HandleCollision` (0x0040E6A0)

| Event String | Match | Condition | Effect | Scene Offsets |
|---|---|---|---|---|
| `E:CALLHAMMER` | `stricmp` exact | tournament only | Create hammer popup | scene+0x436C |
| `E:HAMMERCHASE` | `stricmp` exact | tournament only | Start hammer chase | scene+0x436C |
| `E:ALERTSAW1/2` | `stricmp` exact | tournament only | Pre-activate saw blade | scene+0x4370/0x4374 |
| `E:ACTIVATESAW1/2` | `stricmp` exact | tournament only | Full activate saw blade | scene+0x4370/0x4374 |
| `E:ALERTJUDGES` | `stricmp` exact | — | Reset all judge objects | scene+0x4FC8 |
| `E:SCORE<n>` | `strnicmp` 7 chars | — | Parse number suffix, set time on score displays | — |
| `E:JUMP` | `stricmp` exact | cooldown < 1 | **Duplicate of base handler**: play sound, bounce, +200 score | ball+0x1F7=10, ball+0xA7, ball+0xA8 |
| `E:BELL` | `strnicmp` 6 chars | — | Activate bell, +500 bonus time if not racing, show "EXTRA TIME:" popup | scene+0x4FD4 |

---

## 6. Tag Parsing System (MWParser)

Several events embed XML-style parameters in the name string. These are parsed by `MWParser_ReadTag()`:

| Event | Tag Format | Parameters |
|---|---|---|
| `E:NODIZZY` | `<TIME>value</TIME>` | duration in frames |
| `E:ACTION` | `<ONCE>TRUE</ONCE>` `<SCORE>500</SCORE>` | once-only trigger, score points |
| `E:TRAJECTORY` | `<X>value</X>` `<Y>value</Y>` `<Z>value</Z>` | direction vector components |
| `BADBALL` | `<CHASE>value</CHASE>` `<HOME>` `<SIZE>value</SIZE>` `<SPINDISTANCE>value</SPINDISTANCE>` | AI ball behavior tags |

**MWParser_ReadTag returns:**
```c
struct MWTag {
    void*  release;   // +0x00: vtable with dtor
    char*  tag_name;   // +0x04: tag key (e.g., "TIME", "CHASE")
    char*  tag_value;  // +0x08: tag value as string (e.g., "300", "25.0")
};
```

**SAFESWITCH** uses a different pattern — parenthesized data: `E:SAFESWITCH(data)` → `strchr(eventName, '(')` extracts the content.

---

## 7. N: vs E: Prefix Distinction

| Prefix | `interactive` (+0x85D) | `no_render` (+0x863) | Rendered | Collidable | Examples |
|--------|----------------------|---------------------|----------|-------------|----------|
| **N:** | 1 | 0 | **Yes** | Yes | N:WALL, N:WATER, N:GOAL, N:TRAPDOOR |
| **E:** | 1 | 1 | **No** | Yes | E:JUMP, E:LIMIT, E:NODIZZY, E:PIPEBONK |
| (none) | 0 | 0 | Yes | No | Regular visual geometry |

- **N:** objects are visible geometry that also trigger gameplay events (walls, water zones, goal lines)
- **E:** objects are invisible trigger volumes (jump pads, boundaries, switches)
- Unprefixed objects are visual-only, non-interactive geometry

---

## 8. MESHWORLD Section 3 Object System (Visual Spawners)

Section 3 of the MESHWORLD format contains named reference objects used for spawning game objects (not collision events). These use different prefixes:

- **Access path:** `Scene+0x8AC → MeshWorld* → MeshWorld+0x480 → section_root`
- **Object array:** `section_root+0xCA0`
- **Object count:** `section_root+0x898`
- **Per-object layout:**
  - +0x00: `char* name` (e.g., "BADBALL<CHASE>25</CHASE>", "START0-0", "E:JUMP")
  - +0x04: `float x, y, z` (position)
  - +0x14: `float rot_y` (rotation)

The `CreateLevelObjects` factory (0x4121D0) scans these names with `strnicmp` to instantiate game objects:

| Name Prefix | Object | Ctor Size | Scene Offset |
|---|---|---|---|
| `BRIDGE` | Bridge mesh config | — | +0x436C/4370 |
| `TIPPER` | Tipper | 0x1104 | +0x2578 |
| `BONK` | Bonk | 0x1200 | +0x540C |
| `BBRIDGE1/2` | BreakBridge | 0x1100 | +0x5418/541C |
| `POPCYLINDER` | PopCylinder | 0x10E8 | +0x5428 |
| `BLOCKDAWG1/2` | Blockdawg | 0x1154 | +0x2578 |
| `CATAPULT` | Catapult | 0x1108 | +0x584C |
| `GLUEBIE` | Gluebie | 0x110C | +0x6080 |
| `BADBALL<tags>` | BadBall (AI) | 0xC98 | — |

---

## 9. Adding Custom Event Plane Types

To add a new event type to the game (e.g., `E:MYNEWEVENT`):

### In the .COL file:
1. Name your collision object `"E:MYNEWEVENT"` — the `E:` prefix automatically sets `interactive=1, no_render=1`
2. Append tag parameters if needed: `"E:MYNEWEVENT<DURATION>100</DURATION>"`
3. The name string is stored verbatim at MeshBuffer+0x864 and passed to the collision dispatcher

### In the collision handler:
1. Add a `stricmp`/`strnicmp` check in the appropriate handler:
   - `Level_HandleCollision` (0x40DCD0) for level-specific events
   - `Arena_HandleCollision` (0x40E6A0) for arena-specific events
   - `CreateNoDizzy` (0x40C5D0) for universal events
2. The event name is read from `*(char **)(collObj[1] + 0x864)`
3. Use `strnicmp` for prefix matching (allows suffix data like tags)
4. Use `stricmp` for exact matching (no suffix data needed)
5. For tag parameters, use `MWParser_ReadTag()` to parse XML-style `<TAG>value</TAG>` syntax
6. For parenthesized data, use `strchr(eventName, '(')` to extract `(data)`

### Key ball offsets for writing effects:

| Offset | Type | Name | Description |
|--------|------|------|-------------|
| +0xA7 | float | vert_velocity | Vertical force (E:JUMP uses 0.025) |
| +0xA8 | byte | vert_velocity_on | Vertical velocity enable flag |
| +0xB3 | byte | in_tar | In tar zone flag |
| +0xB6 | int | water_timer | Water/zone effect timer (frames) |
| +0xCB | AthenaList | once_list | One-time trigger records (E:ACTION) |
| +0x1DA | byte | velocity_flag | Velocity/direction clear |
| +0x1F7 | byte | impact_counter | Jump/bounce cooldown timer |
| +0x202 | int | freeze_counter | Input freeze timer |
| +0x2D5 | byte | in_water | In water zone flag |
| +0x2E9 | byte | limit_hit | Finish line reached flag |
| +0x30B | byte | safe_switch | Switch state (0 = clear) |
| +0xC2C | char[?] | safe_switch_data | Switch data buffer |
| +0xCA4 | Vec3 | collision_direction | Trajectory direction vector |

### Key scene offsets:

| Offset | Type | Name | Description |
|--------|------|------|-------------|
| +0x43A0 | float | damage_amount | Bite damage value (25.0) |
| +0x43A8 | int | damage_timer | Damage effect timer |
| +0x43B8 | AthenaList | catapult_list | Catapult objects |
| +0x47D0 | AthenaList | door_list | Trapdoor objects |
| +0x4BEC | AthenaList | first_door | First trapdoor reference |
| +0x4FD4 | Bell* | bell_obj | Bell object |
| +0x5000 | AthenaList | mace_list | Mace objects |
| +0x540C | Bonk* | bonk_ref | Bonk object reference |

---

## 10. Data Flow Summary

```
.COL Binary File
  │
  ├─ Level_LoadCollision (0x465260)
  │   ├─ Read object name string (e.g., "E:JUMP")
  │   ├─ Detect "E:" prefix → set interactive=1, no_render=1
  │   ├─ Store name at MeshBuffer+0x864
  │   └─ Build collision face list (triangles + normals)
  │
  ├─ Ball Physics Update
  │   └─ Mesh_FindClosestCollision — DDA ray traversal
  │       └─ Returns collObj pointing to MeshBuffer
  │
  ├─ Collision Handler Dispatch (2-tier, vtable-driven)
  │   ├─ Level_HandleCollision (0x40DCD0) — race levels
  │   │   └─ Level-specific events first (CATAPULTBOTTOM, OPENSESAME, etc.)
  │   │   └─ delegates to CreateNoDizzy
  │   ├─ Arena_HandleCollision (0x40E6A0) — arenas
  │   │   └─ Arena events first (CALLHAMMER, SAW, BELL, etc.)
  │   │   └─ delegates to CreateNoDizzy
  │   └─ CreateNoDizzy (0x40C5D0) — shared base, always called last
  │       └─ eventName = *(collObj[1] + 0x864)
  │       └─ stricmp/strnicmp dispatch to specific handlers
  │
  └─ Scene_RenderAllObjects (0x45E0E0)
      └─ Skip objects with no_render flag (+0x863)
```

---

## 11. E:BLACKOUT — Not Found

`E:BLACKOUT` does **not** exist in any decompiled code. No string matching "blackout", "BLACKOUT", "BlackOut", or "black_out" was found. This event type either:
- Does not exist in the original game (possibly a mod/community addition)
- Uses a different internal name
- Is handled in an undiscovered code path

If you want to implement `E:BLACKOUT` for custom levels, add it as a new `stricmp` case in `CreateNoDizzy` and handle it with whatever effect you want (e.g., fade screen to black, toggle visibility).