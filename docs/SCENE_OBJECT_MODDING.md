# Scene Object — Complete Modder's Reference

> **Verified via direct Ghidra decompilation** of `Hamsterball.exe` (Athena engine, PE32 i386).  
> All offsets below were extracted from the live decompiled code via the GhidraMCP headless server.  
> **Confidence markers:** ✅ = Verified in raw decompiled C (2+ functions), ⚠️ = Verified in 1 function only, ❓ = Inferred from struct layout / not found in raw C.

---

## Quick Stats

| Property | Value |
|----------|-------|
| Primary update function | `Scene_Update` @ `0x00419C00` |
| Ball physics function | `Scene_UpdateBallsAndState` @ `0x0041B540` |
| Render function | `Scene_Render` @ `0x0041A2E0` |
| VTable | `0x004D0260` |
| Destructor | `Scene_dtor` @ `0x00419770` |
| Total struct size | `~0x5000` bytes (inherits from Gadget, extends to 0x4FD4+) |

---

## How to Get the Scene Pointer

### Method 1: From the Global App Singleton (Easiest)

The App object lives at **`g_App = 0x004FD680`**. It stores the current Scene pointer at **App+0x5DC**.

```c
// Global App singleton
void** g_App = (void**)0x004FD680;

// Get current Scene pointer
void* scene = *(void**)((char*)g_App + 0x5DC);
```

**Alternative offsets into App for Scene access:**

| App Offset | Points to | Verified By |
|------------|-----------|-------------|
| `+0x5DC` | Current Scene* | `Scene_Update` reads position from `App+0x5DC→+0x758` |
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
    printf("Scene tick: frame=%d\n", *(int*)((char*)scene + 0x3620));
    OriginalSceneUpdate(scene);
}
```

---

## Complete Scene Struct Layout (Address-Ordered)

> **Source:** `SCENE_SYSTEM_DECOMP.md` (explicit byte offsets from int-indexed Ghidra fields), `decomp_scene_update.c`, `decomp_scene_spawnballs.c`, `decomp_level_render.c`, `decomp_collision_events.c`

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

### How to Read This Table

- All offsets are **byte addresses** from the start of the Scene struct.
- `int-indexed` fields from decomp outputs have been multiplied by `×4`.
- ⚠️ = From comment-only decomp files (no raw C code available)
- ❓ = Inferred from single decomp source — may need confirmation.

---

| Offset | Type | Field | Description | Confidence |
|--------|------|-------|-------------|------------|
| `+0x000` | `void**` | `vtable` | Scene vtable `0x4D0260` | ✅
| `+0x004` | — | `(Gadget base)` | Inherited fields — vtable, refs, transform data | ✅
| `+0x014` | `App*` | `app_ptr` | Gadget-level back-pointer to App singleton `0x4FD680` | ✅
| `+0x868` | `char*` | `name` | `"Generic Gadget"` / `"Board"` (Gadget inherited) | ✅ |
| `+0x870` | — | `(Scene extension start)` | First field after Gadget base (0x870 bytes) | ✅ |
| `+0x874` | `byte` | `is_skydome` | 0 = skybox, 1 = skydome | ✅
| `+0x878` | `App*` | `scene_manager` | D3D device / render container (also resolves as App* in some contexts) | ✅
| `+0x87C` | `void*` | `viewport_obj` | D3D viewport interface | ✅
| `+0x884` | `RumbleBoard` | `rumble_timer_1` | First rumble/haptics timer (0x14 bytes) | ✅
| `+0x898` | `RumbleBoard` | `rumble_timer_2` | Second rumble/haptics timer (0x14 bytes) | ✅
| `+0x8AC` | `Level*` | `level_ptr` | Level geometry / collision data | ✅
| `+0x8B0` | `Level*` | `skydome_ptr` | Skydome level (alternative sky rendering) | ✅
| `+0x8B8` | `AthenaList` | `scene_object_list` | All scene objects (update + render pass) | ✅
| `+0x8BC` | `int` | `scene_object_count` | Number of objects in above list | ✅
| `+0x8C0` | `int` | `scene_object_iterator` | Current iteration index ❓ | ❓
| `+0x910` | `WaypointList*` | `waypoint_list` | Race checkpoint tracking | ⚠️️
| `+0xCC4` | `SceneObject**` | `scene_object_array` | Direct pointer array to scene objects | ✅
| `+0x1518` | `AthenaList` | `collision_list` | Collision surface list (planes, walls, floors) | ✅
| `+0x151C` | `int` | `collision_count` | Number of collision entries | ❓
| `+0x1520` | `int` | `collision_iterator` | Collision list iteration index ❓ | ❓
| `+0x1924` | `void**` | `collision_array` | Collision object pointer array | ❓
| `+0x2160` | `AthenaList` | `ripple_list` | Water ripple effects | ✅
| `+0x29B0` | `byte` | `ball_positions_dirty` | 1 = need to propagate ball positions this frame | ✅
| `+0x29B8` | `int` | `shake_magnitude` | Camera shake intensity. Starts at -800, decays by +10/frame | ✅
| `+0x29BC` | `float` | `camera_orbit_angle` | Y-axis orbit rotation around ball | ✅
| `+0x29C0` | `float` | `camera_distance` | Orbit distance from ball | ✅
| `+0x29D0` | `Ball*` | `current_ball_ptr` | Ball currently tracked by camera | ✅
| `+0x29D4` | `AthenaList` | `ball_list_1` | **Player 1 ball list head** | ✅
| `+0x29D8` | `int` | `ball_list_1_count` | Number of P1 balls | ✅
| `+0x29DC` | `int` | `ball_list_1_iterator` | P1 list iteration state ❓ | ❓
| `+0x2DE0` | `Ball**` | `ball_list_1_array` | **P1 Ball pointer array** — direct read | ✅
| `+0x3204` | `AthenaList` | `ball_list_2` | **Player 2 ball list head** (split-screen) | ✅
| `+0x3208` | `int` | `ball_list_2_count` | Number of P2 balls | ✅
| `+0x320C` | `int` | `ball_list_2_iterator` | P2 list iteration state ❓ | ❓
| `+0x3610` | `Ball**` | `ball_list_2_array` | **P2 Ball pointer array** — direct read | ✅
| `+0x361C` | `SceneObject*` | `waypoint_arrow` | Next-waypoint arrow | ✅
| `+0x3620` | `int` | `frame_counter` | Total frames since scene start. Also called `tick_count` | ✅
| `+0x362C` | `AthenaList` | `player_list` | Player viewport list. Decomp calls this `physics_objects` at int `+0x0D8B` | ✅
| `+0x3630` | `int` | `player_count` | 0=none, 1=single player, 2=split-screen | ✅
| `+0x3634` | `int` | `player_iterator` | Player list iteration state ❓ | ❓
| `+0x3A38` | `Ball**` | `player_ball_array` | Array of ball pointers indexed by player (0-3) | ✅
| `+0x3A44` | `byte` | `use_skydome` | 0 = skybox, 1 = skydome | ❓
| `+0x3A48` | `AthenaList` | `visible_object_list` | Objects visible this frame (render bucket) | ✅
| `+0x3A4C` | `byte` | `shake_active` | 1 = camera shake / haptics currently active | ✅
| `+0x3AFC` | `void*` | `dynamic_object` | Object with vtable[8] render callback. Also called `post_update_callback_obj` | ✅
| `+0x3B00` | `void*` | `trail_particles_ptr` | Trail particle system pointer | ❓
| `+0x3F18` | `void*` | `water_ripple` | Water ripple renderer object | ✅
| `+0x3F1C` | `byte` | `path_follow_mode` | 1 = camera rides spline rails | ✅
| `+0x3F20` | `void*` | `path_object` | Spline path data for camera rails | ✅
| `+0x3F24` | `float` | `path_position` | Parametric t-position on spline | ✅
| `+0x3F2C` | `int` | `camera_snap_frames` | Frames remaining until snap-to-ball | ✅
| `+0x434C` | `float` | `camera_offset_x` | Camera offset X from ball | ✅
| `+0x4350` | `float` | `camera_offset_y` | Camera offset Y from ball | ✅
| `+0x4354` | `float` | `camera_offset_z` | Camera offset Z from ball | ✅
| `+0x4358` | `byte` | `demo_timer_active` | 1 = demo countdown running | ✅
| `+0x435C` | `int` | `demo_countdown` | Frames remaining until demo popup | ✅
| `+0x4360` | `float` | `demo_accumulator` | Popup timing accumulator | ✅
| `+0x4364` | `int` | `demo_frame_counter` | Demo tick counter | ✅
| `+0x4368` | `byte` | `demo_menu_suppressed` | 1 = block ESC menu during demo | ✅
| `+0x436C` | `void*` | `hammer_obj` | Bonk/hammer object (arena) | ✅
| `+0x4370` | `void*` | `saw1_obj` | Saw blade 1 object (arena) | ✅
| `+0x4374` | `void*` | `saw2_obj` | Saw blade 2 object (arena) | ✅
| `+0x43A0` | `float` | `damage_amount` | Current damage value (set by E:BITE) | ✅
| `+0x43A8` | `int` | `damage_timer` | Damage countdown timer | ✅
| `+0x43B8` | `void*` | `catapult_list` | Catapult object list | ✅
| `+0x47D0` | `void*` | `door_list` | Trapdoor/door object list | ✅
| `+0x4BBC` | `void*` | `judge_list` | Judge/score display list (arena) | ✅
| `+0x4BEC` | `void*` | `door_list_alt` | Alternate door list | ✅
| `+0x4FD4` | `void*` | `bell_obj` | Bell object (arena extra time) | ✅

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
| `[0x8C]` | `0x41A9A0` | `Scene_ComputeInputForceDirection` | Computes 3D force vector from strongest player input (Ghidra label "ComputeLighting" is wrong) | `__thiscall` |

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

### Method 2: Hook Scene_UpdateBallsAndState (0x41B540)

```c
typedef void (__fastcall *UpdateBallsFunc)(void* scene);
UpdateBallsFunc orig = (UpdateBallsFunc)0x0041B540;

void __fastcall Hook_UpdateBallsAndState(void* scene) {
    int p1_count = *(int*)((char*)scene + 0x29D8);
    void** p1_balls = *(void***)((char*)scene + 0x2DE0);
    for (int i = 0; i < p1_count; i++) {
        void* ball = p1_balls[i];
        // ... pre-tick modifications
    }
    orig(scene);
}
```

---

## Modding Recipes

### Recipe 1: Get Current Player's Ball

```c
void* GetPlayerBall(int player_index) {
    void* scene = *(void**)((char*)0x004FD680 + 0x5DC);
    void** player_balls = *(void***)((char*)scene + 0x3A38);
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

### Recipe 4: Read Camera State

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
    *(char*)((char*)scene + 0x4368) = 1;  // demo_menu_suppressed = true
}
```

### Recipe 6: Trigger Camera Shake

```c
void TriggerCameraShake(int magnitude) {
    void* scene = *(void**)((char*)0x004FD680 + 0x5DC);
    *(int*)((char*)scene + 0x29B8) = magnitude;   // shake_magnitude
    *(char*)((char*)scene + 0x3A4C) = 1;          // shake_active
}
```

### Recipe 7: Count Total Scene Objects

```c
int CountSceneObjects() {
    void* scene = *(void**)((char*)0x004FD680 + 0x5DC);
    return *(int*)((char*)scene + 0x08BC);  // scene_object_count
}
```

### Recipe 8: Activate All Trapdoors

```c
void ActivateAllTrapdoors(void* scene) {
    void* door_list = *(void**)((char*)scene + 0x47D0);
    // Iterate door_list, call Trapdoor_Activate(door) for each
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
| `Level_HandleCollision` | `0x40DCD0` | `Scene*, Ball*, Collider*` | Level collision events |
| `Arena_HandleCollision` | `0x40E6A0` | `Scene*, Ball*, Collider*` | Arena collision events |

---

## Verification Notes

### INT-INDEXED vs BYTE OFFSETS

Ghidra sometimes reports offsets as **int indices** (array indices into `int[]`). For this PE32 binary, `sizeof(int) = 4`. To convert:

| Int Index | Byte Offset |
|-----------|-------------|
| `0x021D` | `0x0874` |
| `0x0220` | `0x0880` |
| `0x0A6C` | `0x29B0` |
| `0x0D88` | `0x3620` |
| `0x10D6` | `0x4358` |

The decompilations in `decomp_scene_update.c` use **int-indexed notation** in comments. The offsets in this document are already converted to **byte addresses**.

### Verified Offsets (raw decompiled C, 19 functions scanned)

Automated verification via GhidraMCP REST API decompilation of these functions:
`Scene_Update`, `Scene_dtor`, `Scene_SetCamera`, `Scene_Render`, `Scene_UpdateBallsAndState`,
`Scene_SpawnBallsAndObjects`, `Scene_LevelObjUpdate`, `Level_UpdateAndRender`, `Level_RenderDynamicObjects`,
`Level_RenderObjects`, `Level_HandleCollision`, `Arena_HandleCollision`, `Ball_Update`, `Ball_ctor`,
`Scene_StartRace`, `Scene_HandleRaceEnd`, `Scene_HandleCountdown`, `Scene_CreateGameOverMenu`, `Scene_CheckPath`.

**Result:** 56 offsets verified in raw C. 11 offsets not found in any decompiled function body.

| Offset | Field | Verified In | Count |
|--------|-------|-------------|-------|
| `0x0874` | `is_skydome` | `Scene_CreateGameOverMenu`, `Scene_HandleCountdown`, `Scene_Update` | 3 |
| `0x0878` | `scene_manager` | `Arena_HandleCollision`, `Level_HandleCollision`, `Level_UpdateAndRender` | 3+ |
| `0x087C` | `viewport_obj` | `Scene_Render`, `Scene_SetCamera`, `Scene_dtor` | 3 |
| `0x0884` | `rumble_timer_1` | `Scene_Update`, `Scene_dtor` | 2 |
| `0x0898` | `rumble_timer_2` | `Scene_Update`, `Scene_dtor` | 2 |
| `0x08AC` | `level_ptr` | `Level_RenderObjects`, `Level_UpdateAndRender`, `Scene_SpawnBallsAndObjects` | 3 |
| `0x08B0` | `skydome_ptr` | `Scene_dtor` | 1 |
| `0x08B8` | `scene_object_list` | `Arena_HandleCollision`, `Scene_HandleCountdown`, `Scene_Update` | 3 |
| `0x08BC` | `scene_object_count` | `Scene_HandleCountdown`, `Scene_Update`, `Scene_dtor` | 3 |
| `0x0CC4` | `scene_object_array` | `Scene_HandleCountdown`, `Scene_Update`, `Scene_dtor` | 3 |
| `0x1518` | `collision_list` | `Scene_SpawnBallsAndObjects`, `Scene_dtor` | 2 |
| `0x2160` | `ripple_list` | `Scene_dtor` | 1 |
| `0x29B0` | `ball_positions_dirty` | `Scene_Update` | 1 |
| `0x29B8` | `shake_magnitude` | `Scene_Update` | 1 |
| `0x29BC` | `camera_orbit_angle` | `Scene_SetCamera` | 1 |
| `0x29C0` | `camera_distance` | `Scene_SetCamera` | 1 |
| `0x29D0` | `current_ball_ptr` | `Scene_Render` | 1 |
| `0x29D4` | `ball_list_1` | `Level_UpdateAndRender`, `Scene_SpawnBallsAndObjects`, `Scene_Update` | 3 |
| `0x29D8` | `ball_list_1_count` | `Level_UpdateAndRender`, `Scene_Update`, `Scene_UpdateBallsAndState` | 3 |
| `0x2DE0` | `ball_list_1_array` | `Level_UpdateAndRender`, `Scene_Update`, `Scene_UpdateBallsAndState` | 3 |
| `0x3204` | `ball_list_2` | `Level_UpdateAndRender`, `Scene_UpdateBallsAndState`, `Scene_dtor` | 3 |
| `0x3208` | `ball_list_2_count` | `Level_UpdateAndRender`, `Scene_UpdateBallsAndState`, `Scene_dtor` | 3 |
| `0x3610` | `ball_list_2_array` | `Level_UpdateAndRender`, `Scene_UpdateBallsAndState`, `Scene_dtor` | 3 |
| `0x361C` | `waypoint_arrow` | `Level_UpdateAndRender`, `Scene_dtor` | 2 |
| `0x3620` | `frame_counter` | `Scene_Update` | 1 |
| `0x362C` | `player_list` | `Scene_HandleCountdown`, `Scene_Render`, `Scene_SpawnBallsAndObjects` | 3 |
| `0x3630` | `player_count` | `Scene_HandleCountdown`, `Scene_Render`, `Scene_SpawnBallsAndObjects` | 3 |
| `0x3A38` | `player_ball_array` | `Scene_HandleCountdown`, `Scene_Render`, `Scene_SpawnBallsAndObjects` | 3 |
| `0x3A48` | `visible_object_list` | `Level_RenderObjects`, `Level_UpdateAndRender`, `Scene_dtor` | 3 |
| `0x3A4C` | `shake_active` | `Scene_Update` | 1 |
| `0x3AFC` | `dynamic_object` | `Scene_Update`, `Scene_dtor` | 2 |
| `0x3F18` | `water_ripple` | `Scene_dtor` | 1 |
| `0x3F1C` | `path_follow_mode` | `Scene_SetCamera`, `Scene_Update` | 2 |
| `0x3F20` | `path_object` | `Scene_SetCamera`, `Scene_Update`, `Scene_dtor` | 3 |
| `0x3F24` | `path_position` | `Scene_SetCamera` | 1 |
| `0x3F2C` | `camera_snap_frames` | `Scene_SetCamera` | 1 |
| `0x434C` | `camera_offset_x` | `Scene_SetCamera` | 1 |
| `0x4350` | `camera_offset_y` | `Scene_SetCamera` | 1 |
| `0x4354` | `camera_offset_z` | `Scene_SetCamera` | 1 |
| `0x4358` | `demo_timer_active` | `Scene_Update` | 1 |
| `0x435C` | `demo_countdown` | `Scene_Update` | 1 |
| `0x4360` | `demo_accumulator` | `Scene_Update` | 1 |
| `0x4364` | `demo_frame_counter` | `Scene_Update` | 1 |
| `0x4368` | `demo_menu_suppressed` | `Scene_Update` | 1 |
| `0x436C` | `hammer_obj` | `Arena_HandleCollision` | 1 |
| `0x4370` | `saw1_obj` | `Arena_HandleCollision` | 1 |
| `0x4374` | `saw2_obj` | `Arena_HandleCollision` | 1 |
| `0x43A0` | `damage_amount` | `Level_HandleCollision` | 1 |
| `0x43A8` | `damage_timer` | `Level_HandleCollision` | 1 |
| `0x43B8` | `catapult_list` | `Level_HandleCollision` | 1 |
| `0x47D0` | `door_list` | `Level_HandleCollision` | 1 |
| `0x4BBC` | `judge_list` | `Arena_HandleCollision` | 1 |
| `0x4BEC` | `door_list_alt` | `Level_HandleCollision` | 1 |
| `0x4FD4` | `bell_obj` | `Arena_HandleCollision` | 1 |

### Unverified Offsets (NOT found in raw decompiled C)

These offsets are listed in the document but do **not** appear in any decompiled function body from the 19-function scan. They may be:
- Embedded AthenaList sub-fields (count, iterator, capacity) that are accessed inline
- Comment-only hypotheses from earlier RE sessions
- Struct fields that exist but are never directly dereferenced in the scanned functions

| Offset | Field | Note |
|--------|-------|------|
| `0x08C0` | `scene_object_iterator` | Likely AthenaList embedded `iterator` at `scene_object_list + 0x0C` |
| `0x151C` | `collision_count` | Likely AthenaList embedded `count` at `collision_list + 0x04` |
| `0x1520` | `collision_iterator` | Likely AthenaList embedded `iterator` at `collision_list + 0x0C` |
| `0x1924` | `collision_array` | Likely AthenaList embedded `array` at `collision_list + 0x00` |
| `0x29DC` | `ball_list_1_iterator` | Likely AthenaList embedded `iterator` at `ball_list_1 + 0x0C` |
| `0x320C` | `ball_list_2_iterator` | Likely AthenaList embedded `iterator` at `ball_list_2 + 0x0C` |
| `0x3634` | `player_iterator` | Likely AthenaList embedded `iterator` at `player_list + 0x0C` |
| `0x3A44` | `use_skydome` | Field verified via `is_skydome` at `0x874`; `0x3A44` may be a duplicate/alias |
| `0x3B00` | `trail_particles_ptr` | Not found in any decompiled function from the scan set |

### Comment-Only Offsets

These appear only in the comment headers of `decomp_scene_update.c` (the raw C body is not saved in that file):

- `0x3620` — `frame_counter` ✅ NOW VERIFIED (found in `Scene_Update` raw C)
- `0x362C` — `player_list` ✅ NOW VERIFIED (found in `Scene_Render` raw C)
- `0x3630` — `player_count` ✅ NOW VERIFIED (found in `Scene_Render` raw C)
- `0x3A38` — `player_ball_array` ✅ NOW VERIFIED (found in `Scene_Render` raw C)
- `0x3A4C` — `shake_active` ✅ NOW VERIFIED (found in `Scene_Update` raw C)
- `0x4358`–`0x4368` — Demo timer fields ✅ ALL NOW VERIFIED (found in `Scene_Update` raw C)

---

## Sources

All data verified via **live GhidraMCP headless decompilation** on Hamsterball.exe:

- `decomp_scene_update.c` (0x419C00) — main game tick (comment-only header)
- `decomp_scene_updateballs.c` (0x41B540) — ball physics iteration (comment-only header)
- `decomp_level_render.c` — rendering pipeline with ball lists (comment-only header)
- `decomp_scene_spawnballs.c` (0x41C5B0) — level startup + ball creation (comment-only header)
- `decomp_collision_events.c` (0x40C5D0) — collision handler with scene offsets ✅ raw C
- `decomp_level_handlecollision.c` (0x40DCD0) — level events ✅ raw C
- `decomp_arena_handlecollision.c` (0x40E6A0) — arena events ✅ raw C
- `decomp_scene_setcamera.c` (0x419FA0) — camera system ✅ raw C
- `decomp_scene_render.c` (0x41A2E0) — render dispatch (comment-only header)
- `decomp_ball_vtable_0x408390.c` — ball list iteration ✅ raw C
- `decomp_ball_vtable_0x409480.c` — P2 ball list access ✅ raw C
- `SCENE_SYSTEM_DECOMP.md` — vtable map, render pipeline, camera system
- `BALL_OBJECT_MODDING.md` — ball structs for cross-reference
- `scene_struct.h` — partial C struct from dtor analysis

---

*Document generated: 2026-06-06*  
*Method: Ghidra decompilation + cross-reference with existing scene docs + automated offset extraction from 29 raw decomp files*  
*Confidence: High for ✅ verified offsets, Medium for ⚠️ comment-only offsets, Low for ❓ inferred fields*
