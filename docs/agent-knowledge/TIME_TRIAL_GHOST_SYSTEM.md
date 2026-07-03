# Time Trial Ghost System

> **Verified via direct Ghidra decompilation** of `Hamsterball.exe` (Athena engine, PE32 i386).  
> All offsets were extracted from live decompiled code via GhidraMCP.  
> **Confidence:** ✅ = Verified in raw decompiled C (2+ functions), ⚠️ = Verified in 1 function only.

## Overview

The Time Trial ghost is a **frame-by-frame position recording** system. It captures the player ball's state every frame during a race, then replays those snapshots as a semi-transparent ghost ball on subsequent runs of the same level. The system is entirely in-memory — ghost data is never persisted to disk or the registry.

No "ghost" or "replay" strings appear anywhere in the binary. The system reuses two generic engine mechanisms: **AthenaList** (dynamic arrays) and a **"BestTimeTracker"** object that wraps an AthenaList of ball snapshots.

## Key Offsets

### App Struct Offsets

| Offset | Size | Name | Description | Confidence |
|--------|------|------|-------------|------------|---|
| `+0x90C` | 4 | `recording_buffer_ptr` | `BestTimeTracker*` — stores per-frame snapshots of the current race | ✅ |
| `+0x910` | 4 | `playback_buffer_ptr` | `BestTimeTracker*` — stores best run snapshots, played back as the ghost | ✅ |
| `+0x220` | 4 | `player_profile_ptr` | `PlayerProfile*` — `+0x11` (byte) = time-trial flag (non-zero = practice/TT) | ✅ |
| `+0x234` | 1 | `is_party_mode` | 0 = Time Trial, non-zero = Party mode (disables ghost) | ✅ |
| `+0x5DC` | 4 | `current_ball_ptr` | `Ball*` — player's ball, used as source for recording | ✅ |
| `+0x5E8` | 4 | `race_timer` | Current race time in ticks — written to `recording+0x524` on goal | ✅ |

### Scene/Board Struct Offsets

| Offset | Size | Name | Description | Confidence |
|--------|------|------|-------------|------------|---|
| `+0x361C` | 4 | `ghost_ball_ptr` | `Ball*` — the ghost ball object, created by `Board_ctor` when playback buffer exists | ✅ |
| `+0x29D4` | — | `ball_list_1` | AthenaList of P1 balls (ghost is appended here for rendering) | ✅ |
| `+0x3204` | — | `ball_list_2` | AthenaList of P2 balls | ✅ |

> **⚠️ CORRECTION:** Previous docs labeled `scene+0x361C` as `waypoint_arrow` / `waypoint_display_obj`. This is **incorrect** — it is the **ghost ball pointer**, created and used exclusively by the Time Trial ghost system. See "Corrections to Prior Documentation" below.

### BestTimeTracker Struct (0x528 bytes)

Allocated by `BestTimeTracker_ctor` (0x427660). Two instances exist on the App:
- `App+0x90C` = recording buffer (always created when a new race starts)
- `App+0x910` = playback buffer (promoted from recording when a better time is achieved)

| Offset | Size | Name | Description | Confidence |
|--------|------|------|-------------|------------|---|
| `+0x000` | 4 | `vtable` | Virtual function table pointer (`PTR_FUN_004d262c`) | ✅ |
| `+0x004` | — | `athena_list` | Embedded AthenaList (see below) | ✅ |
| `+0x149` (dword) | 4 | `best_time` | Race time in ticks. `9999999` = sentinel (never finished / no recording) | ✅ |
| `+0x408` | 4 | `iterator_counter` | AthenaList iterator counter (wraps 1–255) | ✅ |
| `+0x410` | 4 | `list_array` | `BallSnapshot**` — pointer to array of snapshot pointers | ✅ |
| `+0x41C` | 4 | `playback_index` | Current playback frame index (incremented each render frame) | ✅ |
| `+0x420` | 4 | `race_time` | Race time at recording (copied to ghost ball `+0x154` during playback) | ✅ |
| `+0x524` | 4 | `finish_time` | Written by `DispatchCollisionEvents` N:GOAL handler — the race time at goal crossing | ✅ |

> **Note on `+0x149`:** In the decompiled C, `param_1[0x149]` accesses the dword at byte offset `0x149 * 4 = 0x524`. This is the `finish_time` field, used as the "best time" for comparison.

### AthenaList Layout (embedded in BestTimeTracker at +0x004)

The AthenaList is initialized by `AthenaList_Init` (0x453210):

| Offset (from list start) | Size | Name | Description |
|--------------------------|------|------|-------------|
| `+0x000` (from struct) | 4 | `vtable` | `PTR_FUN_004d875c` (AthenaList vtable) |
| `+0x004` | 4 | `list_count` | Number of items in the list |
| `+0x008` | 0x400 | `items[256]` | Fixed array of 256 item pointers (4 bytes each) |
| `+0x408` | 4 | `iterator_counter` | Iterator state (wraps 1–255 via `AthenaList_GetIterator`) |
| `+0x40C` | 4 | `pad` | |
| `+0x410` | 4 | `list_array` | Dynamic array pointer (when count > 256, items spill here) |
| `+0x414` | 4 | `capacity` | Dynamic array capacity |

### BallSnapshot Struct (0x28 / 40 bytes)

Each frame's snapshot, allocated by `BestTimeTracker_RecordSnapshot` (0x427810):

| Offset | Size | Name | Source (Ball offset) | Description |
|--------|------|------|----------------------|-------------|
| `+0x00` | 4 | `pos_x` | `ball+0x164` | Ball position X |
| `+0x04` | 4 | `pos_y` | `ball+0x168` | Ball position Y |
| `+0x08` | 4 | `pos_z` | `ball+0x16C` | Ball position Z |
| `+0x0C` | 4 | `vel_x` | `ball+0x190` | Velocity X |
| `+0x10` | 4 | `vel_y` | `ball+0x194` | Velocity Y |
| `+0x14` | 4 | `rotation` | `ball+0x150` | Rotation/orientation |
| `+0x18` | 1 | `state_flag` | `ball+0x748` | Ball state flag (byte) |
| `+0x19` | 3 | `pad` | — | Alignment padding |
| `+0x1C` | 4 | `rot_x` | `ball+0x74C` | Rotation X |
| `+0x20` | 4 | `rot_y` | `ball+0x750` | Rotation Y |
| `+0x24` | 4 | `radius` | `ball+0x284` | Ball radius (26.0 = player, 35.0 = 8-ball) |

## Function Reference

### Core Ghost Functions

| Address | Name | Description |
|---------|------|-------------|
| `0x427660` | `BestTimeTracker_ctor` | Constructs a 0x528-byte BestTimeTracker, initializes AthenaList, sets `best_time = 9999999` |
| `0x427760` | `BestTimeTracker_dtor` | Frees all BallSnapshot entries, then frees the AthenaList array |
| `0x427810` | `BestTimeTracker_RecordSnapshot` | Allocates a 0x28-byte BallSnapshot from the current ball state, appends to AthenaList |
| `0x427690` | `BestTimeTracker_PlaybackSnapshot` | Reads the next BallSnapshot from the list, copies it onto the ghost ball |
| `0x428C50` | `App_StartPracticeRace` | Race start: manages recording/playback buffers, promotes recording to playback if better |
| `0x419030` | `Board_ctor` | Level setup: creates ghost ball at `scene+0x361C` if `App+0x910` exists |
| `0x41B540` | `Scene_UpdateBallsAndState` | Per-frame: calls `RecordSnapshot` on `App+0x90C`, advances playback index on `App+0x910` |
| `0x40B600` | `Level_UpdateAndRender` | Per-frame render: calls `PlaybackSnapshot` on `App+0x910`, sets ghost alpha, renders ghost |
| `0x40C5D0` | `DispatchCollisionEvents` | N:GOAL handler writes `App+0x5E8` (race time) to `recording+0x524` (finish_time) |

### Supporting Functions

| Address | Name | Description |
|---------|------|-------------|
| `0x453210` | `AthenaList_Init` | Initializes an AthenaList struct, zeroes 256 entries, sets vtable |
| `0x4532B0` | `AthenaList_GetIterator` | Returns next iterator index (wraps 1–255) for safe reentrant iteration |
| `0x453280` | `AthenaList_Clear` | Frees dynamic array and resets count — does NOT free individual items |
| `0x453250` | `Vec3List_Free` | Frees the list array pointer and resets vtable |

### Not the Ghost System (Common Confusion)

| Address | Name | Why It's Confusing |
|---------|------|--------------------|
| `0x440840` | `BounceBall_Update` | References the `"BallPath"` and `"FOLLOWBALLSPOT"` strings — these are **mesh names** for the catapult/ball-launcher system, NOT the ghost recording. |
| `0x43EBC0` | `FollowBall_Ctor` | Constructs a `FollowBall` — a ball that follows a **predefined mesh path** for level mechanics (e.g., Tower Race catapult). NOT the ghost ball. |
| `0x4D0237` | `"@FollowPos: %f"` | Debug format string, unrelated to the ghost system. |

## System Flow

### 1. Race Start (`App_StartPracticeRace`, 0x428C50)

```
1. App+0x237 = 0 (clear arena flag)
2. App_StartRace(app)  // initialize level
3. Manage recording/playback buffers:
   - If App+0x90C (recording) and App+0x910 (playback) both exist:
     - Compare their best_time fields (+0x524)
     - If recording's time < playback's time: promote recording → playback
       (free old playback, move recording pointer to App+0x910)
     - If recording's time == 9999999 (DNF): keep playback, free recording
   - Create fresh BestTimeTracker at App+0x90C for new race
4. Set player active flags
5. Create PlayerProfile, set profile+0x11 = 1 (time-trial flag)
6. Tournament_AdvanceRace(profile, 0)  // creates the level board
```

### 2. Ghost Ball Creation (`Board_ctor`, 0x419030)

During level construction, if **all three** conditions are met:
- `PlayerProfile+0x11 != 0` (Time Trial mode)
- `App+0x234 == 0` (not Party mode)
- `App+0x910 != NULL` (playback buffer exists from a previous run)

Then a **ghost ball** is created at `scene+0x361C`:
```
ghost = operator_new(0xC60)     // Ball-sized allocation
Ball_ctor2(ghost, scene)         // Initialize as a Ball
ghost->vtable[1]()               // SetPosition/Setup vtable call
Matrix_Scale4x4(..., 0.3f)       // Scale matrix with 0.3 alpha (semi-transparent)
Ball_SetTrajectory(ghost, ...)    // Set initial trajectory
ghost->playerID = -1             // Not a real player (ball+0x18 = 0xFFFFFFFF)
ghost->gravity_scale = 0.5       // ball+0x278 = 0x3F000000
ghost->unknown_27c = 0.1         // ball+0x27C = 0x3DCCCCCD
ghost->radius = 26.0             // ball+0x284 = 0x41D00000 (player ball size)
ghost->max_speed = 1000.0        // ball+0x188 = 0x4479C000 (very high, never clips)
ghost->is_8ball = false          // ball+0x281 = 0
```

### 3. Recording — Every Frame (`Scene_UpdateBallsAndState`, 0x41B540)

Called once per frame during gameplay. At the end, if Time Trial mode is active:
```
if (profile+0x11 != 0 && app+0x234 == 0) {
    BestTimeTracker_RecordSnapshot(app+0x90C, app+0x5DC);  // record current ball
    
    // Also advance playback index on the playback buffer
    if (app+0x910 != NULL) {
        playback->playback_index++;
        if (playback_index >= AthenaList_GetSize(playback))
            playback_index = size - 1;  // clamp to last frame
    }
}
```

`BestTimeTracker_RecordSnapshot` (0x427810) allocates a 40-byte `BallSnapshot`:
```
snapshot = operator_new(0x28);
snapshot->pos_x = ball[0x164];     // position
snapshot->pos_y = ball[0x168];
snapshot->pos_z = ball[0x16C];
snapshot->vel_x = ball[0x190];     // velocity
snapshot->vel_y = ball[0x194];
snapshot->rotation = ball[0x150];
snapshot->state_flag = ball[0x748];
snapshot->rot_x = ball[0x74C];
snapshot->rot_y = ball[0x750];
snapshot->radius = ball[0x284];
AthenaList_Append(tracker->list, snapshot);
```

### 4. Goal Crossing (`DispatchCollisionEvents`, 0x40C5D0)

When the ball hits an `N:GOAL` collision event:
```
if (profile+0x11 != 0 && app+0x234 == 0 && app+0x90C != NULL) {
    recording->finish_time = app->race_timer;  // recording+0x524 = App+0x5E8
}
```

This stores the finish time on the recording buffer so it can be compared on the next race.

### 5. Playback — Every Render Frame (`Level_UpdateAndRender`, 0x40B600)

During rendering, if Time Trial mode and playback buffer exists:
```
if (profile+0x11 != 0 && app+0x234 == 0 && app+0x910 != NULL) {
    BestTimeTracker_PlaybackSnapshot(app+0x910, scene+0x361C);
    ghost_ball->field_2FC = 0x3EE66666;  // 0.45f — render alpha/visibility
    ghost_ball->vtable[2]();             // Render the ghost ball
}
```

`BestTimeTracker_PlaybackSnapshot` (0x427690):
```
index = tracker->playback_index;
snapshot = tracker->list_array[index];

// Call ball->vtable[13] (SetRenderPosition) twice — once with current pos, once with snapshot pos
ghost->vtable[13](ghost->pos, 1);      // interpolate from current
ghost->vtable[13](snapshot->pos, 0);  // set to snapshot

// Copy entire snapshot onto ghost ball
ghost->pos_x = snapshot->pos_x;       // ball+0x164
ghost->pos_y = snapshot->pos_y;       // ball+0x168
ghost->pos_z = snapshot->pos_z;       // ball+0x16C
ghost->vel_x = snapshot->vel_x;       // ball+0x190
ghost->vel_y = snapshot->vel_y;       // ball+0x194
ghost->rotation = snapshot->rotation; // ball+0x150
ghost->state = snapshot->state_flag;  // ball+0x748
ghost->rot_x = snapshot->rot_x;       // ball+0x74C
ghost->rot_y = snapshot->rot_y;       // ball+0x750
ghost->radius = snapshot->radius;     // ball+0x284
ghost->race_time = tracker->race_time; // ball+0x154
```

### 6. Why the Ghost Disappears

**Quit the game:** Both `App+0x90C` and `App+0x910` are in-memory only. The `BestTime` registry value (at `App+0x86C`, 0x50 bytes) stores **only finish times** as a binary blob — never the position recordings. When the process exits, `BestTimeTracker_dtor` (0x427760) frees all snapshot entries and the list array.

**Switch levels:** `App+0x910` persists across level changes within the same session, but the ghost ball at `scene+0x361C` is only created by `Board_ctor` when a new level loads. If you start a different level, the old recording from level A would produce a ghost with positions from level A's coordinate space — the ball would appear to fly through walls. The game doesn't explicitly guard against this; the recording at `App+0x910` is overwritten when you finish a race on the new level with a better time.

## Memory Budget

A typical 60-second race at 25 FPS produces ~1,500 snapshots:
- 1,500 frames × 40 bytes per snapshot = **~60 KB** per recording
- Two recordings (recording + playback) = **~120 KB** total
- All in RAM, never persisted to disk

## Corrections to Prior Documentation

### scene+0x361C — Was "waypoint_arrow", Actually "ghost_ball_ptr"

Multiple docs previously labeled `scene+0x361C` as `waypoint_arrow` or `waypoint_display_obj`:

- `docs/rendering/LEVEL_RENDER_DECOMP.md` line 46: "waypoint_display_obj (arrow/marker for next checkpoint)"
- `docs/decompilation/SCENE_SYSTEM_DECOMP.md` line 104: "+0x361C=waypoint"
- `docs/objects/SCENE_OBJECT_MODDING.md` line 138: "waypoint_arrow | Next-waypoint arrow | ✅"
- `docs/objects/SCENE_OBJECT_MODDING.md` line 468: "waypoint_arrow | Level_UpdateAndRender, Scene_dtor | 2"
- `docs/modding/MODDING_FUNCTION_REFERENCE.md` line 831: "waypoint_arrow | Next checkpoint arrow"
- `docs/objects/OBJECT_CATALOG.md` line 230: "Ball pointer (first player ball)"

**Correct meaning:** `scene+0x361C` is the **ghost ball pointer** — a `Ball*` created by `Board_ctor` (0x419030) exclusively for Time Trial ghost playback. It is only non-NULL when:
1. `PlayerProfile+0x11 != 0` (Time Trial mode)
2. `App+0x234 == 0` (not Party mode)
3. `App+0x910 != NULL` (playback buffer exists)

### App+0x90C / App+0x910 — Were "unknown object pointer", Actually "BestTimeTracker*"

- `docs/tournament/TOURNAMENT_RACE_ADVANCEMENT.md` lines 312-313: "unknown object pointer (freed on tournament start)"
- `docs/agent-knowledge/LEVEL_COMPLETION_FLOW.md` line 63: "Manages linked list of previously-played race entries"
- `docs/agent-knowledge/LEVEL_COMPLETION_FLOW.md` line 82: "Copies the race name from board+0x29B4 into the race history entry"

**Correct meaning:** These are `BestTimeTracker*` pointers — the recording and playback buffers for the Time Trial ghost system. They are NOT linked lists of race history entries. The "race name comparison" at `App+0x90C+0x424` is actually comparing the current level name against the name stored in the score-tracking object (which has a different purpose — it tracks which level the best time was recorded on).
