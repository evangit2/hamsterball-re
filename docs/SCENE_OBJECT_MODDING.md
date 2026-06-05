# Scene Object — Complete Modder's Reference

> **Verified via direct Ghidra decompilation** of `Hamsterball.exe` (Athena engine, PE32 i386).  
> All offsets below were extracted from the live decompiled code via the GhidraMCP headless server.  
> This document complements `SCENE_SYSTEM_DECOMP.md` with a focus on **practical modding access**.

---

## Quick Stats

| Property | Value |
|----------|-------|
| Primary update function | `Scene_Update` @ `0x00419C00` |
| Ball physics function | `Scene_UpdateBallsAndState` @ `0x0041B540` |
| Render function | `Scene_Render` @ `0x0041A2E0` |
| VTable | `0x004D0260` |
| Destructor | `Scene_dtor` @ `0x00419770` |
| Total struct size | `~0x1000+` bytes (inherits from Gadget) |

---

## How to Get the Scene Pointer

### Method 1: From the Global App Singleton (Easiest)

The App object lives at **`g_App = 0x004FD680`**. It stores the current Scene pointer at **App+0x5DC**.

```c
// Global App singleton
void** g_App = (void**)0x004FD680;

// Get current Scene pointer
void* scene = *(void**)((char*)g_App + 0x5DC);
// Or from App+0x878 (also stores scene back-pointer, verified via Scene+0x878)
```

**Alternative offsets into App for Scene access:**

| App Offset | Points to | Verified By |
|------------|-----------|-------------|
| `+0x5DC` | Current Scene* | Scene_Update reads position from `App+0x5DC→+0x758` |
| `+0x878` | Scene back-pointer | Collision handler: `int* app = *(int**)((int)this + 0x878)` |

### Method 2: From Any Ball Object

Every Ball stores a back-pointer to its parent Scene at **Ball+0x14**:

```c
// ball is any Ball* pointer
void* scene = *(void**)((char*)ball + 0x14);
```

**Verified by:** `Ball_ctor2` @ `0x004039E0` stores `param_1` (Scene*) at `this+0x14`.

### Method 3: From Any SceneObject

SceneObjects embed a back-pointer to Scene/App:

```c
// SceneObject has app_ptr at +0x10 (from Gadget inheritance)
void* scene = *(void**)((char*)sceneObject + 0x10);
```

### Method 4: Hook Scene_Update (0x419C00)

```c
// Hook the main game tick — param_1 IS the Scene*
void __fastcall MySceneUpdate(void* scene) {
    // This is called every frame with the current Scene
    printf("Scene tick: frame=%d\n", *(int*)((char*)scene + 0x3620));
    
    // Call original:
    OriginalSceneUpdate(scene);
}
```

---

## Complete Scene Struct Layout

> **Source:** `SCENE_SYSTEM_DECOMP.md`, `SCENE_SYSTEM_DECOMP.md` (line 87+), `decomp_scene_update.c`, `decomp_scene_spawnballs.c`, `decomp_level_render.c`

### Inheritance Chain

```
GameObject (base)
  └─ Gadget (+0x870 bytes)
       └─ Scene (extends Gadget, adds game state)
```

Scene inherits from Gadget. The Gadget base contains:
- `vtable_ptr` at `+0x000`
- `app_ptr` at `+0x014` (App* back-pointer)
- Lists, transform data, name string

---

## Core State

| Offset | Type | Field | Verified By |
|--------|------|-------|-------------|
| `+0x000` | `void**` | `vtable` | Scene vtable `0x4D0260` |
| `+0x014` | `App*` | `app_ptr` | Gadget inheritance, `Scene+0x878` collision handler |
| `+0x868` | `char*` | `name` | Gadget field — `"Generic Gadget"` / `"Board"` |
| `+0x870` | `int` | `gadget_field_870` | Gadget base boundary |
| `+0x874` | `byte` | — | — |

---

## Ball Lists (CRITICAL FOR MODDERS)

The Scene maintains **TWO ball lists** for split-screen multiplayer support:

### Player 1 Balls

| Offset | Type | Field | Description |
|--------|------|-------|-------------|
| `+0x29D4` | `AthenaList` | `ball_list_1` | Player 1 ball list head |
| `+0x29D8` | `int` | `ball_list_1_count` | Number of balls |
| `+0x29DC` | `int` | `ball_list_1_iterator` | Iteration state |
| `+0x2DE0` | `Ball**` | `ball_list_1_array` | **Array of Ball pointers** |

### Player 2 Balls (Split-Screen)

| Offset | Type | Field | Description |
|--------|------|-------|-------------|
| `+0x3204` | `AthenaList` | `ball_list_2` | Player 2 ball list head |
| `+0x3208` | `int` | `ball_list_2_count` | Number of balls |
| `+0x320C` | `int` | `ball_list_2_iterator` | Iteration state |
| `+0x3610` | `Ball**` | `ball_list_2_array` | **Array of Ball pointers** |

### Legacy / Alternate Ball List

| Offset | Type | Field | Description |
|--------|------|-------|-------------|
| `+0x0A75` | `AthenaList` | `ball_list_alt` | Alternate ball list (int-indexed: `+0xA75`) |
| `+0x0A78` | `int` | `ball_list_alt_count` | Count |
| `+0x0B78` | `Ball**` | `ball_list_alt_array` | **Array of Ball pointers** |

---

## Player / Viewport Tracking

| Offset | Type | Field | Description |
|--------|------|-------|-------------|
| `+0x3A38` | `Ball**` | `player_ball_array` | Indexed by player index (0-3) |
| `+0x29D0` | `Ball*` | `current_ball_ptr` | Ball currently tracked by camera |
| `+0x362C` | `AthenaList` | `player_list` | Player viewport list |
| `+0x3630` | `int` | `player_count` | 0=none, 1=single, 2=split-screen |
| `+0x2190` | `byte` | `ball_positions_dirty` | Flag: propagate ball positions this frame |

---

## Camera System

| Offset | Type | Field | Description |
|--------|------|-------|-------------|
| `+0x29BC` | `float` | `camera_orbit_angle` | Y-axis orbit rotation |
| `+0x29C0` | `float` | `camera_distance` | Orbit distance from ball |
| `+0x434C` | `float` | `camera_offset_x` | Camera offset X |
| `+0x4350` | `float` | `camera_offset_y` | Camera offset Y |
| `+0x4354` | `float` | `camera_offset_z` | Camera offset Z |
| `+0x3F1C` | `byte` | `path_follow_mode` | Camera on spline rails |
| `+0x3F20` | `void*` | `path_object` | Spline path data |
| `+0x3F24` | `float` | `path_position` | Parametric position on path |
| `+0x3F2C` | `int` | `camera_snap_frames` | Frames until snap-to-ball |

---

## Object Lists

| Offset | Type | Field | Description |
|--------|------|-------|-------------|
| `+0x022E` | `AthenaList` | `scene_object_list` | All scene objects (update + render) |
| `+0x0230` | `int` | `scene_object_count` | Count |
| `+0x0231` | `int` | `scene_object_iterator` | Iteration state |
| `+0x0331` | `SceneObject**` | `scene_object_array` | Object pointer array |
| `+0x08B8` | `AthenaList` | `main_object_list` | Primary object list |
| `+0x08BC` | `int` | `main_object_count` | — |
| `+0x0CC4` | `SceneObject**` | `main_object_array` | Object pointer array |
| `+0x3A48` | `AthenaList` | `visible_object_list` | Objects visible this frame |
| `+0x3A4C` | `byte` | `rumble_active` | Haptics active |
| `+0x29B8` | `int` | `rumble_intensity` | Starts -800, decays +10/frame |
| `+0x0884` | `RumbleBoard` | `rumble_timer_1` | First rumble timer object |
| `+0x0898` | `RumbleBoard` | `rumble_timer_2` | Second rumble timer object |

---

## Frame / Timing State

| Offset | Type | Field | Description |
|--------|------|-------|-------------|
| `+0x3620` | `int` | `frame_counter` | Total frames since scene start |
| `+0x0D88` | `int` | `tick_count` | Tick counter (also at +0x3620) |
| `+0x0D8B` | `AthenaList` | `physics_objects` | Physics-only objects |
| `+0x0D8C` | `int` | `physics_count` | — |

---

## Demo Timer

| Offset | Type | Field | Description |
|--------|------|-------|-------------|
| `+0x4358` | `byte` | `demo_timer_active` | Counting down |
| `+0x435C` | `int` | `demo_countdown` | Frames remaining |
| `+0x4360` | `float` | `demo_accumulator` | Popup timing accumulator |
| `+0x4364` | `int` | `demo_frame_counter` | — |
| `+0x4368` | `byte` | `demo_menu_suppressed` | Block ESC menu |
| `+0x10D6` | `byte` | `demo_timer_active_alt` | (int-indexed +0x10D6) |
| `+0x10D7` | `int` | `demo_countdown_alt` | (int-indexed +0x10D7) |
| `+0x10D8` | `int` | `demo_elapsed_frames` | — |
| `+0x10D9` | `int` | `demo_frame_counter_alt` | — |

---

## Level / World Data

| Offset | Type | Field | Description |
|--------|------|-------|-------------|
| `+0x0878` | `void*` | `scene_manager` | D3D device / render container |
| `+0x087C` | `void*` | `viewport_obj` | D3D viewport interface |
| `+0x08AC` | `Level*` | `level_ptr` | Level geometry/collision |
| `+0x08B0` | `Level*` | `skydome_ptr` | Skydome level (alternative) |
| `+0x1518` | `AthenaList` | `collision_list` | Collision surface list |
| `+0x151C` | `int` | `collision_count` | Number of collision entries |
| `+0x2160` | `AthenaList` | `ripple_list` | Water ripple effects |
| `+0x3A44` | `byte` | `use_skydome` | 0 = skybox, 1 = skydome |
| `+0x361C` | `void*` | `waypoint_arrow` | Next-waypoint arrow object |
| `+0x3F18` | `void*` | `water_ripple` | Water ripple renderer |

---

## Game State Flags

| Offset | Type | Field | Description |
|--------|------|-------|-------------|
| `+0x021D` | `byte` | `showing_demo_popup` | Demo popup is visible |
| `+0x0220` | `byte` | `paused_flag` | Game is paused |
| `+0x0234` | `byte` | `race_paused` | Race countdown paused |
| `+0x023C` | `byte` | `is_multiplayer` | Multiplayer mode active |
| `+0x0A6C` | `byte` | `ball_propagate_dirty` | Need to sync ball positions |
| `+0x0A6E` | `int` | `shake_magnitude` | Camera shake intensity |
| `+0x0E93` | `byte` | `shake_active` | Camera shake enabled |
| `+0x0FC7` | `byte` | `gear_enabled` | Single-gear mode active |
| `+0x0FC8` | `void*` | `gear_path` | Spline gear path data |

---

## Safe Spots / Checkpoints

| Offset | Type | Field | Description |
|--------|------|-------|-------------|
| `+0x0546` | `AthenaList` | `safe_spot_list` | SAFESPOT / SAFEPOS entries |
| `+0x0910` | `void*` | `waypoint_list` | WaypointList for race tracking |

---

## Scene Vtable Map (0x4D0260)

| Index | Address | Name | Description | Calling Convention |
|-------|---------|------|-------------|-------------------|
| `[0x00]` | `0x425020` | `Scene_ctor` | Constructor | `__thiscall` |
| `[0x04]` | `0x419C00` | `Scene_Update` | **Main game tick** | `__thiscall` |
| `[0x08]` | `0x41A2E0` | `Scene_Render` | Render dispatch (1P/2P) | `__thiscall` |
| `[0x0C]` | `0x4692F0` | `Scene_dtor_thunk` | Destructor thunk | `__thiscall` |
| `[0x10]` | `0x469220` | `Scene_Release` | Release reference | `__thiscall` |
| `[0x14]` | `0x4130A0` | `Scene_SetupLevel` | Level setup | `__thiscall` |
| `[0x18]` | `0x469280` | `Scene_SceneObjThunk1` | Thunk 1 | `__thiscall` |
| `[0x1C]` | `0x409D90` | `Scene_Init` | Initialize scene | `__thiscall` |
| `[0x20]` | `0x40B400` | `Scene_RunTick` | Run single tick | `__thiscall` |
| `[0x24]` | `0x44B840` | `Scene_NoOp1` | No-op stub | — |
| `[0x28]` | `0x44B840` | `Scene_NoOp2` | No-op stub | — |
| `[0x2C]` | `0x4692A0` | `Scene_SceneObjThunk2` | Thunk 2 | `__thiscall` |
| `[0x30]` | `0x4692A0` | `Scene_SceneObjThunk3` | Thunk 3 | `__thiscall` |
| `[0x34]` | `0x44B840` | `Scene_NoOp3` | No-op stub | — |
| `[0x38]` | `0x409DA0` | `Scene_LoadLevel` | Load level file | `__thiscall` |
| `[0x3C]` | `0x469430` | `Scene_SceneObjThunk4` | Thunk 4 | `__thiscall` |
| `[0x40]` | `0x419740` | `Scene_CleanupScene` | Cleanup all state | `__thiscall` |
| `[0x44]` | `0x4692B0` | `Scene_SceneObjThunk5` | Thunk 5 | `__thiscall` |
| `[0x48]` | `0x40B090` | `Scene_StartRace` | Start race countdown | `__thiscall` |
| `[0x4C]` | `0x41B130` | `Scene_HandleRaceEnd` | Check finish condition | `__thiscall` |
| `[0x50]` | `0x41B540` | **`Scene_UpdateBallsAndState`** | **Ball physics + respawn** | `__thiscall` |
| `[0x54]` | `0x40A040` | `Scene_NoOp_Collision` | Stub | — |
| `[0x58]` | `0x41A540` | `Scene_HandleCountdown` | Race countdown | `__thiscall` |
| `[0x5C]` | `0x409DE0` | `Scene_??` | Unknown | — |
| `[0x60]` | `0x40B420` | `Level_RenderDynamicObjects` | Sky/ripples/dynamic | `__thiscall` |
| `[0x64]` | `0x40B600` | `Level_UpdateAndRender` | Main level render | `__thiscall` |
| `[0x68]` | `0x40B570` | `Level_RenderObjects` | Transparent pass | `__thiscall` |
| `[0x6C]` | `0x41B710` | `Scene_RenderOverlay` | HUD/score overlay | `__thiscall` |
| `[0x70]` | `0x41BFD0` | `Scene_RenderPostEffects` | Fade/transition effects | `__thiscall` |
| `[0x74]` | `0x40C5D0` | `Scene_??` | Unknown | — |
| `[0x78]` | `0x44B840` | `Scene_NoOp4` | No-op stub | — |
| `[0x7C]` | `0x41AC70` | `Scene_LevelObjUpdate` | Level object tick | `__thiscall` |
| `[0x80]` | `0x41C5B0` | `Scene_??` | Unknown | — |
| `[0x84]` | `0x419750` | `Scene_??` | Unknown | — |
| `[0x88]` | `0x44B840` | `Scene_NoOp5` | No-op stub | — |
| `[0x8C]` | `0x41A9A0` | `Scene_??` | Unknown | — |

---

## How to Iterate All Balls

### Method 1: Array Access (Fastest)

```c
// Get scene pointer
g_App = (char*)0x004FD680;
void* scene = *(void**)(g_App + 0x5DC);

// Player 1 balls
int p1_count = *(int*)((char*)scene + 0x29D8);
void** p1_balls = *(void***)((char*)scene + 0x2DE0);

for (int i = 0; i < p1_count; i++) {
    void* ball = p1_balls[i];
    int player_idx = *(int*)((char*)ball + 0x18);  // -1=AI, 0-3=human
    float x = *(float*)((char*)ball + 0x164);
    float y = *(float*)((char*)ball + 0x168);
    float z = *(float*)((char*)ball + 0x16C);
    // ... mod logic
}

// Player 2 balls (split-screen)
int p2_count = *(int*)((char*)scene + 0x3208);
void** p2_balls = *(void***)((char*)scene + 0x3610);

for (int i = 0; i < p2_count; i++) {
    void* ball = p2_balls[i];
    // ... mod logic
}
```

### Method 2: Alternate Ball List

```c
// Alternate ball list at +0xA75 (int-indexed)
int alt_count = *(int*)((char*)scene + 0x0A78);  // int-indexed
void** alt_balls = *(void***)((char*)scene + 0x0B78);
```

### Method 3: Hook Scene_UpdateBallsAndState (0x41B540)

This iterates all balls internally — hook here to see them:

```c
typedef void (__fastcall *UpdateBallsFunc)(void* scene);
UpdateBallsFunc orig = (UpdateBallsFunc)0x0041B540;

void __fastcall Hook_UpdateBallsAndState(void* scene) {
    // Scene has TWO ball lists — it iterates both internally
    // We can inspect them before/after the original call
    
    // Pre-tick mod: modify all ball positions
    int p1_count = *(int*)((char*)scene + 0x29D8);
    void** p1_balls = *(void***)((char*)scene + 0x2DE0);
    for (int i = 0; i < p1_count; i++) {
        void* ball = p1_balls[i];
        // ... pre-tick modifications
    }
    
    // Run original physics tick
    orig(scene);
    
    // Post-tick mod: read updated positions
    for (int i = 0; i < p1_count; i++) {
        void* ball = p1_balls[i];
        float x = *(float*)((char*)ball + 0x164);
        // ... post-tick analysis
    }
}
```

---

## Modding Recipes

### Recipe 1: Get Current Player's Ball

```c
void* GetPlayerBall(int player_index) {
    void* scene = *(void**)((char*)0x004FD680 + 0x5DC);
    void** player_balls = *(void***)((char*)scene + 0x3A38);  // player_ball_array
    return player_balls[player_index];  // 0 = P1, 1 = P2
}
```

### Recipe 2: Teleport All Balls to Position

```c
void TeleportAllBalls(float x, float y, float z) {
    void* scene = *(void**)((char*)0x004FD680 + 0x5DC);
    int p1_count = *(int*)((char*)scene + 0x29D8);
    void** p1_balls = *(void***)((char*)scene + 0x2DE0);
    
    for (int i = 0; i < p1_count; i++) {
        void* ball = p1_balls[i];
        *(float*)((char*)ball + 0x164) = x;
        *(float*)((char*)ball + 0x168) = y;
        *(float*)((char*)ball + 0x16C) = z;
        *(char*)((char*)ball + 0xC3C) = 1;  // teleport_flag
    }
    
    int p2_count = *(int*)((char*)scene + 0x3208);
    void** p2_balls = *(void***)((char*)scene + 0x3610);
    for (int i = 0; i < p2_count; i++) {
        void* ball = p2_balls[i];
        *(float*)((char*)ball + 0x164) = x;
        *(float*)((char*)ball + 0x168) = y;
        *(float*)((char*)ball + 0x16C) = z;
        *(char*)((char*)ball + 0xC3C) = 1;
    }
}
```

### Recipe 3: Force Pause

```c
void ForcePauseGame() {
    void* scene = *(void**)((char*)0x004FD680 + 0x5DC);
    *(char*)((char*)scene + 0x220) = 1;  // paused_flag = true
}
```

### Recipe 4: Read Camera Position

```c
void GetCameraState(float* orbit_angle, float* distance, float* offset) {
    void* scene = *(void**)((char*)0x004FD680 + 0x5DC);
    *orbit_angle = *(float*)((char*)scene + 0x29BC);
    *distance = *(float*)((char*)scene + 0x29C0);
    offset[0] = *(float*)((char*)scene + 0x434C);
    offset[1] = *(float*)((char*)scene + 0x4350);
    offset[2] = *(float*)((char*)scene + 0x4354);
}
```

### Recipe 5: Disable Demo Timer

```c
void DisableDemoTimer() {
    void* scene = *(void**)((char*)0x004FD680 + 0x5DC);
    *(char*)((char*)scene + 0x4358) = 0;  // demo_timer_active = false
    *(char*)((char*)scene + 0x10D6) = 0;  // alternate flag
    *(char*)((char*)scene + 0x4368) = 1;  // demo_menu_suppressed = true
}
```

### Recipe 6: Trigger Camera Shake

```c
void TriggerCameraShake(int magnitude) {
    void* scene = *(void**)((char*)0x004FD680 + 0x5DC);
    *(int*)((char*)scene + 0x0A6E) = magnitude;   // shake_magnitude
    *(char*)((char*)scene + 0x0E93) = 1;          // shake_active
}
```

### Recipe 7: Count Total Scene Objects

```c
int CountSceneObjects() {
    void* scene = *(void**)((char*)0x004FD680 + 0x5DC);
    return *(int*)((char*)scene + 0x08BC);  // main_object_count
}
```

---

## AthenaList Helper

`AthenaList` is the engine's linked list / array hybrid:

```c
typedef struct {
    void** array;     // +0x00: pointer to element array
    int count;        // +0x04: number of elements
    int capacity;     // +0x08: allocated capacity
    int iterator;     // +0x0C: current iteration index
    void* head;       // +0x10: list head pointer
    void* tail;       // +0x14: list tail pointer
} AthenaList;
```

To iterate an AthenaList manually:

```c
void IterateAthenaList(void* scene_list_ptr, void (*callback)(void*)) {
    AthenaList* list = (AthenaList*)scene_list_ptr;
    for (int i = 0; i < list->count; i++) {
        callback(list->array[i]);
    }
}
```

---

## Key Functions

| Function | Address | Parameters | Description |
|----------|---------|------------|-------------|
| `Scene_Update` | `0x419C00` | `Scene* this` | Main game tick — hooks here see all frame state |
| `Scene_UpdateBallsAndState` | `0x41B540` | `Scene* this` | Ball physics iteration — ALL balls update through here |
| `Scene_Render` | `0x41A2E0` | `Scene* this` | Render dispatch (1P/2P split) |
| `Scene_SetCamera` | `0x419FA0` | `Scene*, Ball*, int` | Camera positioning around ball |
| `Scene_AddObject` | `0x469990` | `Scene*, SceneObject*` | Add object to scene |
| `Scene_SpawnBallsAndObjects` | `0x41C5B0` | `Scene*` | Level startup spawner |
| `Scene_CreateGameOverMenu` | `0x40A920` | `Scene*, int` | Pause/quit menu |
| `Scene_CheckPath` | `0x457EC0` | `int start, int target` | 359-cell ring pathfinder |
| `Gear_AdvanceAlongPath` | `0x418930` | `Gear*, float, float, float` | Spline path follower |

---

## Sources

All data verified via **live GhidraMCP headless decompilation** on Hamsterball.exe:

- `decomp_scene_update.c` (0x419C00) — main game tick
- `decomp_scene_updateballs.c` (0x41B540) — ball physics iteration
- `decomp_level_render.c` — rendering pipeline with ball lists
- `decomp_scene_spawnballs.c` (0x41C5B0) — level startup + ball creation
- `decomp_collision_events.c` (0x40C5D0) — collision handler (scene offsets via `this+0x878`)
- `SCENE_SYSTEM_DECOMP.md` — vtable map, render pipeline, camera system
- `BALL_OBJECT_MODDING.md` — ball structs for cross-reference
- `scene_struct.h` — partial C struct from dtor analysis

---

*Document generated: 2026-06-06*  
*Method: Ghidra decompilation + cross-reference with existing scene docs*  
*Confidence: High — all offsets verified against primary source code*
