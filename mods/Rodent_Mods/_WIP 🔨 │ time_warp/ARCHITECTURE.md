# Time Warp — Architecture Document

## Overview

Time Warp is a bass.dll proxy mod that combines four systems into a unified
"special custom race" framework:

1. **Ghost Saver** — Records and persists Time Trial ghost data to `.ghost` files
2. **Ghost Event** — Loads and plays back `.ghost` files as ghost balls in any game mode
3. **Ghost Triggers** — S1 ref point proximity triggers for ghost balls
4. **Warp** — Level transition system with visual effects (color lerp, flash, fade)

The core concept: a custom race where the player races against **two ghost balls
simultaneously**, each loaded from a different `.ghost` file, with warp-style
visual effects and trigger-based events.

---

## System Architecture

### Shared Infrastructure

All four subsystems share:

- **BASS proxy layer** — forwards to `bass_real.dll`
- **Frame epilogue hook** at 0x46C1F1 (single hook, shared dispatch)
- **Diagnostic logging** — `time_warp_log.txt` next to bass.dll
- **Game function wrappers** — `call_btt_ctor`, `call_btt_dtor`, `call_alist_append`, `game_operator_new`, `game_free`
- **Board resolution** — `App(0x5341E0) → +0x220 → +0x0C → board`

### Subsystem: Ghost Saver (Recording + Persistence)

**Purpose:** Record the player's run and save it as a `.ghost` file.

**Mechanism:**
- Detour hook on `App_StartPracticeRace` (0x428C50)
- Pre-inject saved ghost into `App+0x910` before the trampoline call
- Create dummy recording BTT at `App+0x90C` (NO_TIME) to protect playback
- Background monitor thread (60Hz) watches `App+0x5D6` (goal flag)
- On goal: read game's own BTT recording from `App+0x90C`'s AthenaList
- Save to `Ghosts/<RaceName>.ghost` (if time beats existing best)
- Always save to `Ghosts/Previous_Run.ghost`
- Atomic writes via temp file + `MoveFileEx`

**Ghost file format (binary, GHSG):**
```
[4] magic = 0x47485347 ("GHSG")
[4] version = 1
[4] time (game ticks, lower = better)
[4] frame_count
[frame_count × 40] snapshots (10 DWORDs each)
```

**Snapshot layout (10 DWORDs = 40 bytes):**
```
 0: pos_x        (ball+0x164)
 1: pos_y        (ball+0x168)
 2: pos_z        (ball+0x16C)
 3: facing_x     (ball+0x190)
 4: facing_z     (ball+0x194)
 5: roll_angle   (ball+0x150)
 6: gravity_plane(ball+0x748)
 7: surface_a    (ball+0x74C)
 8: surface_b    (ball+0x750)
 9: radius       (ball+0x284)
```

### Subsystem: Dual Ghost Playback

**Purpose:** Play back two ghost balls simultaneously from two `.ghost` files.

**Ghost ball 1 (primary):**
- Injected into `App+0x910` (game's native playback slot)
- Ghost ball created at `board+0x361C` by Board_ctor (native path)
- Playback advanced by game's `Scene_UpdateBallsAndState` in TT mode
- In non-TT mode: frame epilogue hook advances playback index manually

**Ghost ball 2 (secondary):**
- Standalone BTT allocated via `operator_new(0x528)` + `BTT_Ctor`
- NOT stored in App+0x90C or App+0x910 — invisible to game's BTT management
- Standalone Ball allocated via `operator_new(0xC60)` + `Ball_ctor`
- Added to ball AthenaList at `board+0x29D4` via `AthenaList_Append`
- Each frame: read next snapshot from secondary BTT, write to secondary ball
- Ghost ball color: customizable (default: distinct color to differentiate
  from ghost 1, e.g. heliotrope purple #db03fc)

**Per-frame playback (secondary ghost):**
```
frame_epilogue:
  if (g_ghost2.active && g_ghost2.btt && g_ghost2.ball):
    idx = g_ghost2.playbackIdx
    if idx < g_ghost2.frameCount:
      snap = g_ghost2.snapshots[idx]
      ghost2_ball->pos = snap.pos
      ghost2_ball->facing = snap.facing
      ghost2_ball->roll = snap.roll
      ghost2_ball->radius = snap.radius
      ghost2_ball->gravity_plane = snap.gravity_plane
      ghost2_ball->surface_a = snap.surface_a
      ghost2_ball->surface_b = snap.surface_b
      g_ghost2.playbackIdx++
    else:
      // Playback finished — clamp to last frame
      g_ghost2.playbackIdx = frameCount - 1
```

**Cleanup:**
- On level transition (board pointer changes): destroy ghost 2 ball + BTT
- Ball: remove from AthenaList, call `Ball_DTOR` (vtable[0], flags=1)
- BTT: call `BTT_DTOR` (0x4278C0, flags=1)
- Track allocations in a struct for safe cleanup

### Subsystem: Ghost Triggers

**Purpose:** S1 ref point proximity triggers that fire events when a ghost ball
enters a trigger zone.

**S1 ref point format:** `GT:EventName`
- Position (x, y, z) = trigger center
- `rot_y` field = trigger radius (default 50.0 if 0)

**Trigger scanning:**
- On level load (board pointer change): scan S1 AthenaList for `GT:` entries
- Store up to 64 triggers in array
- Each frame: check both ghost balls against all triggers

**Trigger events:**
- `GT:COLOR(hexcolor)` — changes ghost ball color (e.g. `GT:COLOR(#db03fc)`)
- `GT:RESET` — resets ghost playback to frame 0
- `GT:SPEED(float)` — adjusts ghost playback speed multiplier
- `GT:STOP` — stops ghost playback (freezes at current frame)
- `GT:START` — resumes ghost playback
- Custom event names can be added via a handler table

**Note:** Warp triggers are NOT ghost triggers. Warps use `WARP(levelname)`
S1 ref points and are triggered by the **player ball only**, same as the
standalone warp mod. Ghost balls cannot trigger warps.

**Trigger semantics:**
- Fires on **entry** (was outside, now inside)
- 60-frame (1 second) cooldown per trigger to prevent rapid re-fire
- Checks both ghost balls independently

### Subsystem: Warp (Level Transition)

**Purpose:** Smooth level transition with visual effects, triggered by
`GT:WARP(levelname)` ghost triggers or player proximity.

**Phase timeline:**
```
RUMBLE: 2.0 sec — Ghost ball color lerps to heliotrope purple (#db03fc)
                   Ball alpha fades 1.0 → 0.5
                   Steering disabled, CPUID jitter on
                   Music fade starts (3.0 sec)
FLASH:  0.15 sec — Instant white flash, fades out over 150ms
                   Ball goes invisible (alpha=0)
                   Physics freeze (in_tar=1)
                   Timer freeze (code cave, g_freezeTimer=1)
HOLD:   1.0 sec  — Screen clear, ball stays invisible
FADE:   2.0 sec  — Screen fades to solid white
LOAD:   instant  — Load target level while screen stays white
REVEAL: 1.0 sec  — Fade from white to reveal new level
                   Restore ball state (color, alpha, physics, timer)
```

**Timer freeze (code caves):**
- Patch 1: 0x41B3E5 (JNZ + DEC, 9 bytes) → cave checks `g_freezeTimer`
- Patch 2: 0x41B50C (JNZ + INC, 5 bytes) → cave checks `g_freezeTimer`
- Same passive mechanism as N:GOAL — just adds a second condition

**Tournament timeout abort:**
- If tournament timer expires during RUMBLE/early FLASH (before freeze):
  abort warp, restore physics/music/timer/pause
- Keep ghost ball color + partial alpha during natural timeout popup

**Warp trigger sources:**
1. `GT:WARP(levelname)` — ghost ball enters trigger zone
2. Player ball proximity (optional, via separate trigger prefix `WARP(levelname)`)

---

## Data Structures

### Ghost2 (secondary ghost ball state)
```c
typedef struct {
    DWORD    ball;           // Ball* — secondary ghost ball struct
    DWORD    btt;            // BTT* — standalone BestTimeTracker
    DWORD    *snapshots;     // malloc'd snapshot array (frame_count × 10 DWORDs)
    int      frameCount;     // total frames in recording
    int      playbackIdx;    // current playback frame
    float    colorR, colorG, colorB;  // custom color multiplier
    BOOL     active;         // is this ghost currently playing?
    BOOL     paused;         // temporarily stopped?
    float    speedMult;      // playback speed multiplier (1.0 = normal)
    float    speedAccum;     // fractional frame accumulator for speed mult
} Ghost2;
```

### Trigger
```c
typedef struct {
    float pos_x, pos_y, pos_z;   // trigger center (world space)
    float radius;
    float radius_sq;
    char  name[64];              // event name after "GT:" prefix
    int   inside_ghost1;         // ghost 1 currently inside?
    int   inside_ghost2;         // ghost 2 currently inside?
    int   cooldown1;             // frames before ghost1 can retrigger
    int   cooldown2;             // frames before ghost2 can retrigger
} Trigger;
```

### WarpState
```c
typedef struct {
    int      phase;              // RUMBLE / FLASH / HOLD / FADE / LOAD / REVEAL / IDLE
    DWORD    phaseStartTime;    // GetTickCount() at phase start
    char     targetLevel[128];   // level name to warp to
    float    whiteAlpha;        // white flash alpha (0.0–1.0)
    // Saved state for restore
    float    origBallR, origBallG, origBallB;
    float    origBallAlpha;
    int      colorSaved;
    int      freezeTimer;
} WarpState;
```

---

## Hook Summary

| Hook | Address | Type | Shared By |
|------|---------|------|----------|
| Frame epilogue | 0x46C1F1 | 5-byte JMP to stub | All subsystems |
| App_StartPracticeRace | 0x428C50 | 7-byte detour + trampoline | Ghost saver |
| DispatchCollisionEvents | 0x40C5D0 | 8-byte JMP to stub | Ghost event (E:GHOST) |
| Level_UpdateAndRender | 0x40B7F5 + 0x40B7FF | 2× NOP (2 bytes each) | Ghost event (mode check removal) |
| Timer DEC cave | 0x41B3E5 | 9-byte JMP to cave | Warp (timer freeze) |
| Timer INC cave | 0x41B50C | 5-byte JMP to cave | Warp (timer freeze) |
| Pause block | 0x19D5B, 0x130B5, 0x0B405 | 3-byte patches | Warp (pause prevention) |
| TT recording check | 0x41B690 | 7-byte NOP | Ghost recording in Tournament |

**Frame epilogue dispatch order:**
1. Ghost saver: check goal flag, save ghost on finish
2. Ghost 2 playback: advance frame, write position to secondary ball
3. Ghost triggers: check both ghosts against all triggers
4. Warp state machine: advance phases, apply visual effects

---

## File Layout

```
mods/_WIP 🔨 │ time_warp/
├── time_warp.c           — Main source (single compilation unit)
├── bass_proxy.h          — Shared BASS proxy declarations
├── README.md             — User documentation
├── ARCHITECTURE.md       — This file
└── time_warp_log.txt     — Runtime log (auto-generated)
```

---

## Configuration

### MESHWORLD S1 Ref Points

| Prefix | Example | Purpose |
|--------|---------|---------|
| `WARP(level)` | `WARP(3)` | Warp to level — player ball only (not ghost) |
| `GT:COLOR(#hex)` | `GT:COLOR(#db03fc)` | Change ghost ball color |
| `GT:RESET` | `GT:RESET` | Reset ghost playback to frame 0 |
| `GT:SPEED(float)` | `GT:SPEED(2.0)` | Set ghost playback speed |
| `GT:STOP` | `GT:STOP` | Pause ghost playback |
| `GT:START` | `GT:START` | Resume ghost playback |

### Ghost Files

- `Ghosts/<RaceName>.ghost` — Best time per race
- `Ghosts/Previous_Run.ghost` — Always the most recent run
- `Ghosts/<CustomName>.ghost` — Custom ghosts for E:GHOST triggers

---

## Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll time_warp.c \
  -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
  -Wl,--add-stdcall-alias -msse2 -mfpmath=sse
```

---

## Safety Considerations

1. **Thread safety** — CRITICAL_SECTION around shared state between the
   detour hook (main thread) and background monitor thread
2. **Cleanup on level transition** — destroy ghost 2 ball + BTT when board
   pointer changes
3. **Cleanup on game exit** — destroy all allocated resources in DllMain
   DETACH
4. **IsBadReadPtr guards** — all memory access to game structs guarded
5. **Double-free prevention** — track all allocations, only destroy if
   pointer still matches what we allocated
6. **Timer cave restore** — restore original bytes on unload
7. **Pause patch restore** — restore original bytes when warp completes

---

## Design Decisions (resolved)

### Ghost 2 trigger source
Ghost 2 is loaded **exclusively** when the player warps to the same level
they just warped from. This is the only way Ghost 2 appears. It is not
triggered by collision events or S1 ref points — it's a natural consequence
of the warp system.

When the warp targets the same level:
1. **Before warp completes**: Save the player's current run as a ghost
   (the route they just took to reach the warp trigger)
2. **After warp loads the same level**: Inject the saved ghost as Ghost 2
   with heliotrope purple color (#db03fc)

This creates the effect of seeing your previous route as a ghost after
warping — you can race against your own past run.

Works in **Time Trial and Tournament mode**. Does NOT work in Party mode.

### Ghost 2 color
Ghost 2 is **always** heliotrope purple (#db03fc) — the same color the
player ball lerps to during the warp RUMBLE phase. This visually connects
the warp effect to the ghost that appears after.

### Warp trigger source
Warp is triggered by the **player ball only** — same as the current warp
mod. The ghost ball(s) cannot trigger warps. This uses the existing
`WARP(levelname)` S1 ref point proximity system.

### Ghost 2 game mode support
- **Time Trial**: Ghost 2 appears after warping to same level. Ghost 1
  (normal color) also active via game's native ghost system.
- **Tournament**: Ghost 2 appears after warping to same level. No Ghost 1
  (game doesn't create ghosts in Tournament).
- **Party mode**: Ghost 2 does NOT appear. Warps to same level still work
  (visual effect + level reload) but no ghost is saved or loaded.

### Playback sync
Both ghosts start at **frame 0** when the level loads after the warp.
- **Ghost 1** (normal color): Only active in Time Trial mode. Plays back
  the best-time ghost from `Ghosts/<RaceName>.ghost`.
- **Ghost 2** (heliotrope purple): Active in TT and Tournament after a
  same-level warp. Plays back the route the player just took to reach
  the warp trigger.

Both ghosts are mutually independent — Ghost 1's presence/absence does
not affect Ghost 2, and vice versa.

### Multi-Segment Ghost System (Time Warp Levels)

A Time Warp level consists of multiple runs (warp → reload → warp → goal).
Each run segment must be saved separately for proper ghost playback.

**File naming convention:**
- `LevelName[N].ghost` — temporary (brackets) — current attempt in progress
- `LevelName(N).ghost` — confirmed best (parentheses) — from a completed best attempt

**During an attempt:**
- Run 1 (first load) → warp → save `LevelName[1].ghost`
- Run 2 (Ghost 2 active) → warp → save `LevelName[2].ghost`
- Run 3 (Ghost 2 active) → touches goal → save `LevelName[3].ghost`

**On goal touch — compare total clock time:**
- If no previous best exists: rename all `[N]` → `(N)` — this IS the first best
- If new total time < previous best: delete old `(N)` files, rename `[N]` → `(N)`
- If new total time >= previous best: delete all `[N]` files — discard attempt

**Ghost 1 (normal colored) — seamless chaining:**
- Plays segment `(1)`, then `(2)`, then `(3)`, etc. sequentially
- On warp to same level: Ghost 1 must resume from the exact frame it was at
  (not restart from 0). Save playback index before warp, restore after.
- When Ghost 1 reaches end of segment N, load segment (N+1) and continue
  from frame 0
- If only one segment exists, Ghost 1 replays it normally

**Ghost 2 (heliotrope purple):**
- Always the most recent run segment (the one that led to the current warp)
- Resets to frame 0 on each warp (it's the fresh replay of your previous route)

**Total clock time calculation:**
- Sum of all segment times across the full attempt
- Compare against previous best total time (stored in a separate metadata file
  or inferred from the `(N)` ghost file headers)

### Recording in Tournament mode
The game's recording code in `Scene_UpdateBallsAndState` (0x41B540) is gated
by a Time Trial flag check at 0x41B690:

```asm
0041b690: MOV CL,[EDX+0x11]     ; profile->isTimeTrial
0041b693: TEST CL,CL
0041b695: JZ 0x0041b708         ; skip if not TT
0041b697: MOV CL,[EAX+0x234]   ; App->partyMode (KEEP THIS CHECK)
0041b69d: TEST CL,CL
0041b69f: JNZ 0x0041b708        ; skip if party
```

**Approach: NOP the TT check, keep the party check.**
- Patch 7 bytes at 0x41B690: `8A 4A 11 84 C9 74 71` → `90 ×7` (NOP)
- Party mode check at 0x41B697 stays intact
- Create a BTT at `App+0x90C` in Tournament mode (via `operator_new(0x528)` + `BTT_Ctor`)
- The game's recording code then runs naturally every frame
- On same-level warp: read the BTT's AthenaList for Ghost 2 data
- Do NOT set `profile+0x11=1` — avoids confusing other game systems

**Ghost save rules:**
- Time Trial: Save to `Ghosts/<RaceName>.ghost` + `Previous_Run.ghost` at goal
- Tournament: Only save in-memory for Ghost 2 on same-level warp
- Party mode: Never save
