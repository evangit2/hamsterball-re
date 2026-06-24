# Custom N: Event Objects

Documentation of how N: (and E:, DN:) event objects work inside Hamsterball's collision system, and how to create new ones for mods.

---

## Table of Contents

1. [Overview](#1-overview)
2. [Event Name Prefixes](#2-event-name-prefixes)
3. [How Event Names Are Stored](#3-how-event-names-are-stored)
4. [The Collision Dispatch Chain](#4-the-collision-dispatch-chain)
5. [Complete Event Catalog](#5-complete-event-catalog)
6. [Collision Entry Struct Layout](#6-collision-entry-struct-layout)
7. [How to Create a Custom N: Object](#7-how-to-create-a-custom-n-object)
8. [Reference: Key Addresses](#8-reference-key-addresses)

---

## 1. Overview

Hamsterball's MESHWORLD level files contain geometry (triangles) organized in an octree. Each piece of geometry can carry a **string name** — if that name starts with `N:`, `E:`, or `DN:`, the engine treats it as an event trigger. When the ball's physics body collides with that geometry, the engine reads the string and dispatches to the appropriate handler function.

This means **N: events are not objects** in the traditional sense (they have no update loop, no vtable, no spawning). They are **collision geometry with a tagged name**. The "object" is purely the string name attached to a mesh strip in Section 6 of the MESHWORLD file.

---

## 2. Event Name Prefixes

| Prefix | Meaning | Dispatch Path |
|--------|---------|---------------|
| `N:` | Natural/environmental event | DispatchCollisionEvents (base) |
| `E:` | Effect/interaction event | Level/Arena handler → DispatchCollisionEvents |
| `DN:` | Delayed natural event | Custom vtable override (e.g. SinkPlatform_OnCollision) → DispatchCollisionEvents |
| *(none)* | Bare-name event | DispatchCollisionEvents (checked at name+2 offset) |
| `(NOCOLLIDE)` | Suffix in name | Skips collision geometry creation entirely — visual only |

The `(NOCOLLIDE)` suffix can be appended to any name (e.g. `T:NEONARROW(NOCOLLIDE)`) to tell the engine to skip building collision triangles for that mesh. The geometry renders but the ball passes through.

---

## 3. How Event Names Are Stored

### 3.1 MESHWORLD File (Section 6 — Octree Geometry)

In the MESHWORLD binary format, Section 6 is a recursive octree. Each leaf node contains geometry entries, and each geometry entry has a **string name**. When the engine loads a level, it reads these strings and stores them with the collision mesh buffers.

### 3.2 In Memory — Level_LoadMeshes (0x465860)

When the level loads, `Level_LoadMeshes` iterates every collision geometry entry from the source MeshWorld:

```c
// Simplified from decompiled Level_LoadMeshes (0x465860)
for each geometry entry (iVar14) in sourceMesh:
    // Check for (NOCOLLIDE) — skip if present
    if (entry->name != NULL && strstr(entry->name, "(NOCOLLIDE)") != NULL)
        continue;  // skip — visual only, no collision

    // Allocate a new collision mesh buffer (0x874 bytes)
    meshBuf = CreateMeshBuffer(operator_new(0x874));

    // Copy the event name string
    if (entry->name != NULL):
        meshBuf[0x219] = _strdup(entry->name);  // +0x864 = event name string ptr

        // Set flags based on prefix
        if (strnicmp(entry->name, "N:", 2) == 0)
            meshBuf[0x85D] = 1;  // natural event flag

        if (strnicmp(entry->name, "E:", 2) == 0):
            meshBuf[0x85D] = 1;  // event flag
            meshBuf[0x863] = 1;  // effect flag (triggers Level/Arena handler)
```

### 3.3 Collision Mesh Buffer Layout (0x874 bytes)

| Offset (hex) | Offset (dec) | Type | Description |
|-------------|-------------|------|-------------|
| +0x00 | 0 | vtable* | Mesh buffer vtable (0x4D8E70) |
| +0x04 | 4 | void* | Unused / zero |
| +0x0C | 12 | AthenaList | Triangle list (each 0x60 bytes) |
| +0x21C | 540 | int | Counter |
| +0x85D | 2141 | byte | Is event flag (N: or E: prefix) |
| +0x85E | 2142 | byte | Flag (set to 0) |
| +0x85F | 2143 | byte | Flag (set to 0) |
| +0x861 | 2145 | byte | Flag (set to 0) |
| +0x862 | 2146 | byte | Flag (set to 0) |
| +0x863 | 2147 | byte | Is effect flag (E: prefix only) |
| +0x864 | 2148 | char* | **Event name string** (the N:/E: name) |

The `+0x864` field is the critical one — this is the string pointer that all collision dispatch functions read to determine which event to fire.

---

## 4. The Collision Dispatch Chain

### 4.1 Physics → Collision → Event Dispatch

```
Ball_FallUpdate (0x408830)
  → Ball_AdvancePositionOrCollision (0x4564C0)
    → Collision_TraverseSpatialTree (0x465EF0)  — finds colliding triangles
      → Ball_FallUpdate builds collision entry list at physics+0x848
        → Board vtable[0x1D] (+0x74) = collision dispatch callback
```

### 4.2 Per-Board Collision Handlers (NOT "Level vs Arena")

**IMPORTANT:** There is no single "Level_HandleCollision" or "Arena_HandleCollision" — these were incorrect Ghidra labels. The truth is that **almost every board type overrides vtable[0x1D] with its own unique collision handler**. Each handler processes board-specific events, then falls through to `DispatchCollisionEvents` (0x40C5D0) as the shared base.

The base Scene vtable (0x4D0260) sets vtable[0x1D] = DispatchCollisionEvents directly. Only the WarmUp board uses this default. Every other board overrides it:

| Board | vtable[0x1D] Address | Handler | Events Handled Before Base |
|-------|---------------------|---------|---------------------------|
| **WarmUp** | 0x0040C5D0 | DispatchCollisionEvents (no override) | *(none — uses base directly)* |
| **Intermediate** | 0x0040D340 | custom | N:BRIDGE |
| **Dizzy** | 0x0040D500 | custom | N:WATERWHEEL, N:WHEELEMBED, N:SWIRL |
| **Tower** | 0x0040DCD0 | custom | E:CATAPULTBOTTOM, E:OPENSESAME, N:TRAPDOOR, E:BITE, E:MACETRIGGER, N:MACE |
| **Expert** | 0x0040E6A0 | custom | E:CALLHAMMER, E:HAMMERCHASE, E:ALERTSAW1, E:ALERTSAW2, E:ACTIVATESAW1, E:ACTIVATESAW2, E:ALERTJUDGES, E:SCORE, E:JUMP, E:BELL |
| **Odd** | 0x0040ED30 | custom | E:GRAVITY, N:JUMPFIRST, N:JUMPSECOND, E:SHRINK, E:GROWSOUND, E:GROW, E:DROPLIFT, E:PIPERANDOM, E:LIMIT, E:LIMITX, E:LIMITZ, E:LIMITPIPE1, E:LIMITPIPE2, E:SWALLOW |
| **Beginner** | 0x004111E0 | custom | N:BUMPER |
| **Master (Arena)** | 0x00412850 | custom (CreateSpinner) | N:SPINNER, N:BUMPER, E:LAUNCH, E:CALLHAMMER, E:HAMMERCHASE, E:CATAPULTBOTTOM |
| **Sky/Neon** | 0x00410D00 | custom (CreateLimit) | E:PEGS, E:TRAPPOP, E:NOPEGS, E:HEATON, E:HEATOFF, E:LIMIT |
| **Toob** | 0x00410020 | custom | E:ALERTSAW2, E:BRANCH(A/B), N:SPINNY, N:SAWTEETH, N:BUMPER |
| **Up** | 0x004119B0 | custom | E:HELPINERTIA, E:UNHELPINERTIA, E:VACPOPOUT, N:SPEEDCYLINDER, N:EXTRATIME |
| **Wobbly** | 0x0040F9A0 | custom | N:SQUAREWOBBLY, N:WAVY |
| **Glass** | 0x00417EB0 | custom | N:GLASS, DN:SINKPLATFORM |

Dispatch chain for any collision event:

```
Board vtable[0x1D] (board-specific handler)
  ├─ Process board-specific events (if event name matches)
  │   └─ Return early (for some events) OR fall through
  └─ DispatchCollisionEvents (0x40C5D0) — shared base handler
      └─ Process universal events (N:GOAL, N:TARPIT, N:WATER, E:JUMP, etc.)
```

Note: Some board handlers **return early** without calling DispatchCollisionEvents for certain events (e.g., Intermediate returns early on N:BRIDGE, Dizzy returns early on N:WATERWHEEL/N:WHEELEMBED/N:SWIRL). Most fall through to the base.

### 4.3 How the Dispatch Reads the Event Name

All handlers read the event name the same way:

```c
// param_1 = ball (int*)
// param_2 = collision entry (int*), where:
//   param_2[0] = type/source reference
//   param_2[1] = collision object pointer
//
// Event name string = *(char**)(param_2[1] + 0x864)

char *eventName = *(char **)(param_2[1] + 0x864);

// Then compared via __stricmp / __strnicmp:
if (__stricmp(eventName, "N:GOAL") == 0) { ... }
if (__strnicmp(eventName, "N:SECRET", 8) == 0) { ... }
```

---

## 5. Complete Event Catalog

Events are split into two categories:
1. **Universal events** — handled by `DispatchCollisionEvents` (0x40C5D0), the shared base that ALL boards call
2. **Board-specific events** — handled by each board's own vtable[0x1D] override BEFORE calling DispatchCollisionEvents

### 5.1 Universal Events — DispatchCollisionEvents (0x40C5D0)

These events fire on ALL board types because every board handler calls DispatchCollisionEvents at the end.

#### N: Events (Universal)

| Event Name | What It Does | Ball Fields Affected |
|-----------|-------------|---------------------|
| `N:SECRET` | Calls `Rotator_MarkTriggered(rotator_ptr)` — marks a secret as found | Reads `*param_2 + 0x47C` (rotator) |
| `N:UNLOCKSECRET` | Calls `CheckArenaUnlock(this)` — unlocks arena content | Uses board (this) |
| `N:NOCONTROL` | Sets impact timer = 10 (ball loses control briefly) | `ball[0x202] = 10` |
| `N:WATER` | Sets water flag + timer = 10 (water physics effect) | `ball+0x2D5 = 1`, `ball[0xB6] = 10` |
| `N:TARPIT` | Plays tar sound, sets tar flag, disables dizzy | `ball+0x2CC = 1`, `ball[0x1DA] = 0`, `ball[0xB4] = pos`, plays sound at board+0x484 |
| `N:GOAL` | Finishes the race for the current player | Sets `App+0x5D6` (finished flag), plays "Goal!" music, copies camera angles, sets `App+0x5F0` |
| `N:MOUSETRAP` | Deflects ball trajectory, plays rotator collision sound, adds to rotator list | Scales trajectory vector by `_DAT_004CF370`, sets Y=15.0, iterates rotator list |

#### E: Events (Universal)

| Event Name | What It Does | Key Details |
|-----------|-------------|-------------|
| `E:NODIZZY<TIME>N</TIME>` | Records best time N via `Ball_RecordBest` | Parses XML tags with `MWParser_ReadTag` |
| `E:SAFESWITCH` | Copies parenthesized data to `ball+0xC2C` | `strchr` for `(` char, copies string |
| `E:LIMIT` | Tracks arena completions per player | Sets `ball+0x2E9=1`, increments counters at `board+0x47B4/B8/BC/C0` |
| `E:BREAK` | Calls ball vtable[0x20] callback | `(**(code**)(*ball + 0x20))()` |
| `E:JUMP` | Jump pad: 3D sound + force + impact=10 + score 200 | `ball[0x1F7]=10`, `ball[0xA7]=0x3B03126F`, `ball[0xA8]=1`, `ball[0x202]=10` |
| `E:ACTION(ONCE)(SCORE)` | Score event with optional once-only tracking | Parses XML: `ONCE=TRUE` → `AthenaList_Append(ball+0xCB, obj)`, `SCORE=N` → adds to `App+0x5E4 + pIdx*0xA0` |
| `E:TRAJECTORY(X,Y,Z)` | Sets ball trajectory vector | Writes to `physics+0xCA4/CA8/CAC` |

#### Bare-Name Events (Universal — checked at name+2)

These events are checked by skipping the first 2 characters of the name (comparing at `name + 2`), meaning they work with or without a prefix:

| Event Name | What It Does | Cooldown Field |
|-----------|-------------|----------------|
| `DROPIN` | Sound + score +200 | `ball[0x1F2]` (0x32 = 50 frames) |
| `PIPEBONK` | Random sound + score +100 | `ball[500]` (0x32) |
| `POPOUT` | Sound + score +100 | `ball[499]` (0x32) |
| *(4th, at PTR_DAT_004cf80c)* | Sound | `ball[0x1F5]` (0x32) |

### 5.2 Board-Specific Events — Per-Board vtable[0x1D] Handlers

These events are ONLY processed by specific board types. If the ball touches geometry with one of these names on a different board, it will be ignored (the board handler won't match it, and DispatchCollisionEvents doesn't know about it either).

#### Tower (0x40DCD0)

| Event Name | What It Does |
|-----------|-------------|
| `E:CATAPULTBOTTOM` | Sets impact=1000, iterates catapult list at board+0x43B8, calls `Catapult_Launch` |
| `E:OPENSESAM` | Opens trapdoor — iterates trapdoor list at board+0x4BE8, calls `Trapdoor_Open` |
| `N:TRAPDOOR` | Activates trapdoor — iterates trapdoor list, calls `Trapdoor_Activate` |
| `E:BITE` | Sets damage = 25.0 at `board+0x43A0`, clears `board+0x43A8` |
| `E:MACETRIGGER` | Activates all maces — sets `mace+0x10F0 = 1` |
| `N:MACE` | Ball bounce callback — calls ball vtable[0x20] if mace conditions met |

#### Expert (0x40E6A0)

| Event Name | What It Does |
|-----------|-------------|
| `E:CALLHAMMER` | Creates BONK popup (difficulty-gated: App+0x23C != 0) |
| `E:HAMMERCHASE` | Starts hammer chase (difficulty-gated) |
| `E:ALERTSAW1` / `E:ALERTSAW2` | Alerts saw blade (difficulty-gated) |
| `E:ACTIVATESAW1` / `E:ACTIVATESAW2` | Activates saw blade (difficulty-gated) |
| `E:ALERTJUDGES` | Resets all judges via `Judge_Reset` |
| `E:SCORE` (prefix match) | Sets score display time via `ScoreDisplay_SetTime` |
| `E:JUMP` | Duplicate of base E:JUMP (Expert has its own copy) |
| `E:BELL` (prefix match) | Activates bell, awards 500 bonus time + creates ScoreObject |

#### Master / Arena (0x00412850 — CreateSpinner)

| Event Name | What It Does |
|-----------|-------------|
| `N:SPINNER` | Calls `Rotator_AddObject` — attaches ball to spinner rotator |
| `N:BUMPER` | Bumper physics: plays sound, scales/reverses velocity, sets board flag at +0x53FC + bumperIndex*4 |
| `E:LAUNCH` | Launch pad: looks up "LAUNCHPOINT" position, sets upward trajectory (Y=16.0), impact=50, creates RumbleScore explosion particles |
| `E:CALLHAMMER` | Creates BONK popup (difficulty-gated) |
| `E:HAMMERCHASE` | Starts hammer chase (difficulty-gated) |
| `E:CATAPULTBOTTOM` | Same as Tower — iterates catapult list at board+0x584C, calls `Catapult_Launch` |

#### Dizzy (0x0040D500)

| Event Name | What It Does |
|-----------|-------------|
| `N:WATERWHEEL` | Sets `ball+0x1DE = 1` (water wheel flag), returns early — does NOT call DispatchCollisionEvents |
| `N:WHEELEMBED` | Embeds ball in wheel: computes relative position to wheel center (board+0x4BB0), applies rotation transform, sets `ball+0x30F = 1` (teleport flag), writes new position to `ball+0x310/311/312`, sets impact=50. Returns early. |
| `N:SWIRL` | Sets `ball+0x779 = 1` (swirl flag), returns early |

#### Intermediate (0x0040D340)

| Event Name | What It Does |
|-----------|-------------|
| `N:BRIDGE` | Checks if `board+0x4384 == 3` (bridge state). If so, sets `ball+0x1DE = 1` and returns early. Otherwise falls through to DispatchCollisionEvents. |

#### Odd (0x0040ED30)

| Event Name | What It Does |
|-----------|-------------|
| `E:GRAVITY(TYPE)` | Changes gravity type: `NORMAL` → `Ball_ResetCollisionMesh`, `X` → `Ball_SetTiltedGravity`, `Z` → `Ball_SetFlatGravity`. Parses XML tags. |
| `N:JUMPFIRST` | Teleports ball to "JUMPPIPE1" position, sets upward trajectory (Y=16.0) |
| `N:JUMPSECOND` | Teleports ball to "JUMPPIPE2" position, sets upward trajectory (Y=16.0) |
| `E:SHRINK` | Calls `Ball_StartFall` (shrinks ball), teleports to "SHRINKCENTER", sets downward trajectory (Y=-1.0) |
| `E:GROWSOUND` | Plays grow sound (cooldown: `ball[0x1FE]`, 100 frames) |
| `E:GROW` | Calls `Ball_EndFall` (restores ball size) |
| `E:DROPLIFT` | Calls `Stands_PlayBreakSound` on mesh at board+0x436C |
| `E:PIPERANDOM` | Randomly teleports ball to "PIPERANDOM1" or "PIPERANDOM2" position, zeroes velocity, plays sound |
| `E:LIMIT` | If `ball[0x1D2] == 0`: sets `ball+0x1DA = 0`, `ball+0x2E9 = 1` (qualifies for this limit gate) |
| `E:LIMITX` | Same as LIMIT but checks `ball[0x1D2] == 1` |
| `E:LIMITZ` | Same as LIMIT but checks `ball[0x1D2] == 2` |
| `E:LIMITPIPE1` | If `ball[0x1] != 0` (pipe 1 flag): qualifies for limit |
| `E:LIMITPIPE2` | If `ball[0x5] != 0` (pipe 2 flag): qualifies for limit |
| `E:SWALLOW` | Sets `ball+0xBA = 1` (swallow/pipe entry flag) |

#### Beginner (0x004111E0)

| Event Name | What It Does |
|-----------|-------------|
| `N:BUMPER` | Bumper physics: plays sound, scales/reverses velocity (similar to Master's N:BUMPER), sets board flag at +0x6428 + bumperIndex*4 |

#### Sky / Neon Race (0x00410D00 — CreateLimit)

| Event Name | What It Does |
|-----------|-------------|
| `E:PEGS` | Increments peg counter at `board+0x47F4`, sets `ball[0x1E2] = 1` (peg hit flag) |
| `E:TRAPPOP` | Plays rotator sound (difficulty-gated: App+0x23C != 0) |
| `E:NOPEGS` | Decrements peg counter at `board+0x47F4`, sets `ball[0x1E3] = 1` |
| `E:HEATON` | Adds ball to pendulum list via `Pendulum_AddIndex` (difficulty-gated) |
| `E:HEATOFF` | Removes ball from pendulum list (difficulty-gated) |
| `E:LIMIT` | Removes ball from pendulum list (difficulty-gated, same as HEATOFF) |

#### Toob (0x00410020)

| Event Name | What It Does |
|-----------|-------------|
| `E:ALERTSAW2` | Alerts saw at board+0x4384 (difficulty-gated) |
| `E:BRANCH(A)` / `E:BRANCH(B)` | Branching pipe: looks up numbered POS/VECTOR pairs from hash table, randomly selects one, teleports ball with scaled trajectory. (A) = normal, (B) = double velocity. |
| `N:SPINNY` | Calls `ScoreObject_SetScore` — awards score from rotator |
| `N:SAWTEETH` | Deflects ball: reads position from rotator+0x1100, normalizes and scales by 3.0, calls `Ball_ApplyTrajectory`. Cooldown: `ball[0x1F7]`. |
| `N:BUMPER` | Bumper physics: plays sound, scales/reverses velocity, sets board flag at +0x6448 + bumperIndex*4 |

#### Up (0x004119B0)

| Event Name | What It Does |
|-----------|-------------|
| `E:HELPINERTIA` | Sets `ball[0xA9] = 2.5` (reduces inertia — easier to control) |
| `E:UNHELPINERTIA` | Sets `ball[0xA9] = 5.0` (restores normal inertia) |
| `E:VACPOPOUT` | Sets `ball[0xA1] = 20.0` (vacuum popout force), plays 3D sound |
| `N:SPEEDCYLINDER` | Calls `Pendulum_PlayCollisionSound` — speed boost sound from rotator |
| `N:EXTRATIME` | Awards 500 bonus time: creates ScoreObject "EXTRA TIME:", calls `Timer_Decrement`, adds to board's score list at +0x8B8. Checks rotator+0x10E4 (once-only flag). |

#### Wobbly (0x0040F9A0)

| Event Name | What It Does |
|-----------|-------------|
| `N:SQUAREWOBBLY` | Calls `Stands_AddObject` — adds ball to wobbly platform physics |
| `N:WAVY` | Calls `Blockdawg_AddObject` — adds ball to wavy/blockdawg physics |

#### Glass (0x00417EB0)

| Event Name | What It Does |
|-----------|-------------|
| `N:GLASS` | Sets `ball[0x317] = 0xF` (glass break counter — 15 hits to break) |
| `DN:SINKPLATFORM` | Calls `Scene_StartCountdown` to sink the platform the ball is standing on |

---

## 6. Collision Entry Struct Layout

When `Ball_FallUpdate` processes collisions, it builds a collision entry struct that gets passed to the dispatch functions.

### 6.1 Collision Entry (8 bytes — on stack)

```
struct CollisionEntry {
    int type_or_source_ref;    // +0x00: reference to source (e.g. rotator ptr)
    void* collision_obj;        // +0x04: pointer to collision mesh buffer (0x874 bytes)
};
```

The dispatch functions access the event name as:
```c
char *eventName = *(char **)(collision_entry.collision_obj + 0x864);
```

### 6.2 Collision Mesh Buffer (0x874 bytes — heap allocated)

```
struct CollisionMeshBuffer {
    void* vtable;               // +0x00: 0x4D8E70
    void* unused;               // +0x04
    AthenaList triangles;       // +0x0C: list of CollisionFace (0x60 bytes each)
    // ... (collision geometry data) ...
    byte is_event;              // +0x85D: 1 if name starts with N: or E:
    byte is_effect;             // +0x863: 1 if name starts with E:
    char* event_name;           // +0x864: the event string (e.g. "N:TARPIT")
};
```

---

## 7. How to Create a Custom N: Object

There are four approaches, ranging from simplest to most complex.

### Approach A: MESHWORLD Level Editing (No Code)

**Best for:** Adding existing event types to new locations in a level.

Edit a `.MESHWORLD` file's Section 6 octree to add geometry with an event name. The engine automatically processes it — no code changes needed.

1. Create a mesh with triangles at the desired location
2. Name the mesh entry `N:TARPIT`, `E:JUMP`, `N:GOAL`, etc.
3. Delete all `.cached` files so the game re-reads the `.MESHWORLD`
4. The engine will build collision geometry for the named mesh and dispatch the event when the ball touches it

**Limitation:** You can only use event names that already exist in the engine. The engine ignores unrecognized names (they get collision geometry but no event fires).

**See:** `reference/raptisoft-exporter/` for the MESHWORLD binary format spec.

### Approach B: Fake Collision Entry (CEA Script or DLL)

**Best for:** Triggering existing event logic at arbitrary positions/times without level geometry.

Construct a fake collision entry struct in memory, fill in the event name, and call `DispatchCollisionEvents` directly. This is the **GluebieSpawn pattern** — used to trigger `N:TARPIT` effects when the ball is near Gluebie objects that have no collision geometry of their own.

#### CEA Script Pattern (from GluebieSpawn.CEA):

```asm
// Allocate fake structures
alloc(fake_coll_obj, 0x868)       // collision mesh buffer (only +0x864 matters)
alloc(fake_coll_entry, 8)          // collision entry struct
alloc(tarpit_str, 16)

// Write event name string
tarpit_str:
  db 'N:TARPIT',00

// Set up fake collision object — only +0x864 is read
fake_coll_obj + 0x864:
  dd tarpit_str

// Set up fake collision entry: [source_ref, collision_obj_ptr]
fake_coll_entry:
  dd 0                    // type/source (0 = generic)
  dd fake_coll_obj        // pointer to fake collision object

// Call DispatchCollisionEvents (thiscall: ECX=board, push entry, push ball)
// DispatchCollisionEvents(board, ball, entry):
//   ECX = board (from ball+0x14)
//   push fake_coll_entry
//   push ball
//   call 0040C5D0        // __thiscall, callee cleans 8 bytes (RET 0x8)
```

#### DLL (C) Pattern:

```c
// Build a fake collision entry to trigger N:TARPIT
typedef struct {
    int source_ref;
    void* collision_obj;
} CollisionEntry;

// Minimal collision object — only +0x864 is read
static char fake_coll_obj[0x868];
static const char* tarpit_name = "N:TARPIT";
*(const char**)(fake_coll_obj + 0x864) = tarpit_name;

CollisionEntry entry;
entry.source_ref = 0;
entry.collision_obj = fake_coll_obj;

// Call DispatchCollisionEvents (thiscall)
// ECX = board, params = (ball, &entry)
typedef void (__thiscall *DispatchFn)(void* board, int* ball, CollisionEntry* entry);
DispatchFn DispatchCollisionEvents = (DispatchFn)(0x400000 + 0xC5D0);

void* board = *(void**)(ball + 0x14);   // ball+0x14 = board
DispatchCollisionEvents(board, ball, &entry);
// Note: __thiscall — callee cleans 8 bytes from stack (RET 0x8)
```

**Important calling convention note:** `DispatchCollisionEvents` is `__thiscall` with `RET 0x8` — the callee cleans up 8 bytes (the two pushed parameters). When calling from inline assembly, do NOT add `esp, 8` after the call. When calling from C with a `__thiscall` typedef, the compiler handles this automatically.

### Approach C: Hook DispatchCollisionEvents (DLL)

**Best for:** Adding entirely new event names with custom logic.

Hook `DispatchCollisionEvents` (0x40C5D0) to intercept the event name string before the original function processes it. If the name matches your custom event, execute your logic. Otherwise, fall through to the original.

```c
// Pseudo-code for a DLL hook
void __fastcall hooked_DispatchCollisionEvents(void* board, void* edx,
                                                int* ball, int* entry) {
    char* eventName = *(char**)(entry[1] + 0x864);

    if (eventName) {
        // Custom event handling
        if (_stricmp(eventName, "N:SPEEDBOOST") == 0) {
            // Your custom logic: add velocity to ball
            *(float*)(ball + 0x170) += 50.0f;  // velocity X
            return;  // don't call original
        }
        if (_stricmp(eventName, "N:GRAVITYFLIP") == 0) {
            // Reverse gravity
            *(float*)(ball + 0xC94) *= -1.0f;
            return;
        }
    }

    // Fall through to original DispatchCollisionEvents
    original_DispatchCollisionEvents(board, ball, entry);
}
```

The event name would be placed in a custom MESHWORLD file's Section 6 geometry (Approach A), or triggered via a fake collision entry (Approach B).

**Key advantage:** You can create completely new event names that don't exist in the original engine. The engine will build collision geometry for any `N:`/`E:` prefixed name (it sets the flags but doesn't validate the name), and your hook can intercept it.

### Approach D: Vtable Override (DLL)

**Best for:** Adding per-object custom collision handling (like SinkPlatform's `DN:` pattern).

Override the scene's vtable[0x1D] (+0x74) to point to your own collision handler. This is how `SinkPlatform_OnCollision` works — it checks for `DN:SINKPLATFORM` before calling `DispatchCollisionEvents`.

```c
// Custom collision handler (same signature as DispatchCollisionEvents)
void __fastcall MyCollisionHandler(void* board, void* edx,
                                    int* ball, int* entry) {
    char* eventName = *(char**)(entry[1] + 0x864);

    if (eventName && _stricmp(eventName, "N:MYCUSTOM") == 0) {
        // Custom logic here
        DoSomething(ball, board);
    }

    // Always fall through to base dispatch
    DispatchCollisionEvents(board, ball, entry);
}

// Install: patch board vtable[0x1D] to point to MyCollisionHandler
DWORD oldProtect;
void** vtable = *(void***)board;  // board's vtable
VirtualProtect(&vtable[0x1D], 4, PAGE_EXECUTE_READWRITE, &oldProtect);
vtable[0x1D] = (void*)MyCollisionHandler;
VirtualProtect(&vtable[0x1D], 4, oldProtect, &oldProtect);
```

---

## 8. Reference: Key Addresses

### Functions

| Address | Name | Description |
|---------|------|-------------|
| 0x0040C5D0 | DispatchCollisionEvents | Base collision event handler (universal N:/E:/bare events) |
| 0x0040D340 | Intermediate_HandleCollision | Intermediate board handler (N:BRIDGE) |
| 0x0040D500 | Dizzy_HandleCollision | Dizzy board handler (N:WATERWHEEL, N:WHEELEMBED, N:SWIRL) |
| 0x0040DCD0 | Tower_HandleCollision | Tower board handler (catapult, trapdoor, mace, bite) |
| 0x0040E6A0 | Expert_HandleCollision | Expert board handler (hammer, saw, judge, bell, score) |
| 0x0040ED30 | Odd_HandleCollision | Odd board handler (gravity, pipes, shrink/grow, limits) |
| 0x0040F9A0 | Wobbly_HandleCollision | Wobbly board handler (N:SQUAREWOBBLY, N:WAVY) |
| 0x00410020 | Toob_HandleCollision | Toob board handler (branch pipes, spinny, sawteeth, bumper) |
| 0x00410D00 | SkyNeon_HandleCollision | Sky/Neon board handler (pegs, heat, limit) |
| 0x004111E0 | Beginner_HandleCollision | Beginner board handler (N:BUMPER) |
| 0x004119B0 | Up_HandleCollision | Up board handler (inertia, vacpopout, speed cylinder, extra time) |
| 0x00412850 | Master_HandleCollision | Master/Arena board handler (spinner, bumper, launch, catapult) |
| 0x00417EB0 | Glass_HandleCollision | Glass board handler (N:GLASS, DN:SINKPLATFORM) |
| 0x00465860 | Level_LoadMeshes | Loads collision geometry + copies event names to mesh buffers |
| 0x00458970 | CreateMeshBuffer | Allocates 0x874-byte collision mesh buffer |
| 0x00408830 | Ball_FallUpdate | Physics update — triggers collision dispatch |
| 0x00465EF0 | Collision_TraverseSpatialTree | Octree traversal for collision detection |
| 0x00453810 | AthenaList_Append | Append to AthenaList (used for collision entry lists) |

### Key Offsets

| Offset | On Object | Type | Description |
|--------|-----------|------|-------------|
| +0x864 | Collision Mesh Buffer | char* | Event name string (the N:/E: name) |
| +0x85D | Collision Mesh Buffer | byte | Is event flag (N: or E: prefix) |
| +0x863 | Collision Mesh Buffer | byte | Is effect flag (E: prefix only) |
| +0x74 | Board/Scene vtable | func* | vtable[0x1D] — collision dispatch callback |
| +0x14 | Ball | void* | Board pointer |
| +0x878 | Board | void* | Scene/App pointer |
| +0x164 | Ball | float | Position X |
| +0x168 | Ball | float | Position Y |
| +0x16C | Ball | float | Position Z |
| +0x170 | Ball | float | Velocity X (accumulated, cleared each frame) |
| +0x174 | Ball | float | Velocity Y |
| +0x178 | Ball | float | Velocity Z |
| +0x2CC | Ball | byte | Tar flag (set by N:TARPIT) |
| +0x2D5 | Ball | byte | Water flag (set by N:WATER) |
| +0x202 | Ball | int | Impact timer (set by N:NOCONTROL, E:JUMP, etc.) |
| +0x23C | App | int | Difficulty enum (0=Pipsqueak, 1=Normal, 2=Frenzied) |
| +0x5D6 | App | byte | Player finished flag (set by N:GOAL) |
| +0x5E4 | App | float | Score (per player, +pIdx*0xA0) |

### Calling Convention

`DispatchCollisionEvents` is `__thiscall`:
- **ECX** = board/scene pointer (this)
- Stack param 1 = ball pointer
- Stack param 2 = collision entry pointer
- Returns with `RET 0x8` (callee cleans 8 bytes)

```
; Assembly call pattern:
push fake_coll_entry    ; param 2: collision entry
push ball               ; param 1: ball
mov ecx, board           ; this: board
call 0040C5D0           ; DispatchCollisionEvents
; No "add esp, 8" needed — callee cleans stack (RET 0x8)
```

---

## Appendix: Existing Mod Examples

### GluebieSpawn (tools/gluebie_spawn/GluebieSpawn.CEA)

Uses **Approach B** (fake collision entry) to trigger `N:TARPIT` when the ball is near Gluebie objects. The Gluebie mesh has no collision geometry, so the mod constructs a fake collision entry with `N:TARPIT` as the event name and calls `DispatchCollisionEvents` directly.

### Collision Hook (tools/collision_hook/)

A read-only DLL that hooks all three dispatch functions to log every collision event to a CSV file. Useful for discovering which events fire during gameplay and verifying custom N: objects are working.
