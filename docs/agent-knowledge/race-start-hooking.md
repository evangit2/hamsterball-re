# Race Start Hooking Guide

## How to Execute Code When a Race Starts

This document maps the complete race-start initialization pipeline and
identifies every hookable function where modders can execute custom code
at the moment a player loads into a race — after balls and objects are
instantiated, but before the countdown finishes.

All addresses, calling conventions, and field offsets were verified via
GhidraMCP decompilation and disassembly of Hamsterball.exe (V3.6.c).

---

## Race-Start Pipeline (Call Chain)

```
App_Start*Race (0x4288B0 / 0x4289F0 / 0x428B20 / 0x428C50 / 0x429230)
  ├─ App_StartRace (0x4287C0)                    ← Reset old scene, free old objects
  │    └─ Scene_UpdateChildren (0x466AC0)        ← Destroy children of old scene
  ├─ Create PlayerProfile (operator_new + ctor)
  └─ Tournament_AdvanceRace (0x427080)            ← param_1='\x01' = show menu only
       │
       └─ [Player waits at TourneyMenu, timer counts up]
       │
       └─ TourneyMenu_Tick (0x450860)              ← Timer expires
            ├─ (*menu->vtable[16])()               ← Destroy menu
            └─ Tournament_AdvanceRace (0x427080)   ← param_1='\x00' = CREATE BOARD
                 ├─ switch(race_index):
                 │    case 1: operator_new(0x436C) → BoardLevel1_WarmUp_ctor
                 │    case 2: operator_new(0x644C) → BoardLevel_Beginner_Ctor
                 │    case 3: operator_new(0x438C) → BoardLevel2_Intermediate_ctor
                 │    ... (15 cases total)
                 │
                 │  Each ctor:
                 │    └─ Board_ctor (0x419030)
                 │         └─ Scene_CtorBase (0x457FE0)
                 │
                 ├─ (*board->vtable[18])(board)    ← Level-specific load (OVERRIDDEN per level)
                 │    │
                 │    │  Example: Scene_LoadLevel2 (0x40D280) — loads "levels\\level2":
                 │    │    ├─ MeshWorld_ctor (0x461510)         ← Load .MESHWORLD geometry
                 │    │    ├─ CollisionLevel_ctorWithLevel (0x465080) ← Load collision data
                 │    │    ├─ Level_InitScene (0x40B090)        ← Sound, camera, graphics setup
                 │    │    │    ├─ SoundChannel_Ctor
                 │    │    │    ├─ Level_SelectCameraProfile
                 │    │    │    ├─ Graphics_SetCullMode, SetProjection, SetViewportZ
                 │    │    │    └─ Audio_PlayMusicAtSpeed (start level music)
                 │    │    └─ (*board->vtable[32])(board)      ← Scene_SpawnBallsAndObjects
                 │    │         ├─ For each player entry:
                 │    │         │    ├─ Look up "START%d-%d" position from hash table
                 │    │         │    ├─ Ball_ctor2 (0x4039E0) — create ball (0xC60 bytes)
                 │    │         │    │    └─ Sets ball+0x14C = 0 (controls ENABLED)
                 │    │         │    │    └─ Sets ball+0x284 = 27.0 (initial radius)
                 │    │         │    │    └─ Sets ball+0x188 = 5000.0 (placeholder max_speed)
                 │    │         │    │    └─ Sets ball+0x2FC = 1.0 (gravity_scale)
                 │    │         │    ├─ Ball_SetTrajectory (0x403850) — set spawn position
                 │    │         │    ├─ Override fields:
                 │    │         │    │    └─ ball+0x284 = 26.0 (radius)
                 │    │         │    │    └─ ball+0x188 = 5.0 (max_speed)
                 │    │         │    │    └─ ball+0x2FC = 0.5 (gravity_scale)
                 │    │         │    │    └─ ball+0x1A0 = 1.05 (speed multiplier)
                 │    │         │    └─ AthenaList_Append(Scene+0x29D4, ball)
                 │    │         ├─ Scan SAFESPOT/SAFEPOS entries
                 │    │         ├─ CreateBadBall (if demo/tournament mode)
                 │    │         ├─ CreateMouseTrap (if demo/tournament mode)
                 │    │         ├─ CreateSecretObjects
                 │    │         ├─ Scene_CreateFlags (0x40C0F0)
                 │    │         ├─ Scene_CreateSigns (0x40C270)
                 │    │         └─ Scene_CreateDynamicObjects (0x40C430)
                 │
                 ├─ Scene_AddObject (0x469990) — add board to scene tree
                 └─ Set up player scores (App+0x5E8, +0x688, +0x728, +0x7C8)

[Next frame: game loop begins]

Scene_Update (0x419C00) / RumbleBoard_Update (0x421FE0)  ← vtable[1]
  ├─ Frame counter increment (Scene+0xD88)
  ├─ Demo timer check
  ├─ ESC / pause check
  ├─ Ball position propagation (Ball_SetTargetPos)
  ├─ Gear path follow
  ├─ RumbleBoard_TickTimer
  ├─ Camera shake decay
  ├─ Scene object update + render (vtable[4] / vtable[0])
  └─ Physics pipeline (gated by ball+0x14C):
       ball_count = AthenaList_GetSize(Scene+0x362C)
       if (ball_count == 1):
           ball = *(App + 0x5DC)     ← current physics ball pointer
           if (ball+0x14C != 0):      ← controls disabled
               SKIP vtable[19-22]    ← skip ALL physics + countdown
       [else: always run vtable[19-22]]
       ├─ vtable[19] (0x4C) = Scene_HandleRaceEnd (0x41B130) ← 3-2-1 countdown display
       │    ├─ If Scene+0x3A4C != 0: skip (countdown already done)
       │    ├─ If Scene+0x8B4 != 0: skip
       │    ├─ If Scene+0x3620 <= 25: skip (wait 25 frames before starting)
       │    ├─ Phase 0: display "3" texture (App+0x394), increment timer
       │    ├─ Phase 1: display "2" texture (App+0x398), increment timer
       │    ├─ Phase 2: display "1" texture (App+0x39C), play sound
       │    └─ Phase 3: set Scene+0x3A4C = 1 (countdown done)
       │         └─ Start race timers, iterate balls
       ├─ vtable[20] (0x50) = Scene_UpdateBallsAndState (0x41B540)
       │    └─ For each ball: Ball_SetCamera + Ball_Update (vtable[4])
       ├─ vtable[21] (0x54) = NoOp (0x40A040)
       ├─ vtable[22] (0x58) = Scene_HandleCountdown (0x41A540) ← race-end handler
       └─ For each object: vtable[31] (0x7C) = Scene_vmethod31 (camera setup)
```

---

## Two Separate Countdown Systems

### 1. Pre-Race 3-2-1 Countdown (Scene_HandleRaceEnd, vtable[19])

This is the visual "3, 2, 1" countdown displayed at the start of each race.

- **phase**: Scene+0x3A50 (int) — 0="3", 1="2", 2="1", 3=done
- **phase_timer**: Scene+0x3A54 (float) — accumulates time per phase
- **countdown_done**: Scene+0x3A4C (byte) — 1 = countdown finished
- **sound_played**: Scene+0x3A58 (byte) — prevents double-sound in phase 2
- **frame_counter**: Scene+0x3620 (int) — must exceed 25 (0x19) before countdown starts
- **race_timer**: Scene+0x3624 (float) — decrements during race
- **skip_flag**: Scene+0x8B4 (int) — non-zero = skip countdown entirely

**Key behavior**: The 3-2-1 countdown does NOT freeze the ball. Ball+0x14C is
0 (enabled) from Ball_ctor2. The ball can move during the countdown display.
The countdown is purely cosmetic — it displays numbers and plays sounds,
then sets Scene+0x3A4C=1 and starts race timers.

The 25-frame delay (Scene+0x3620 > 25) before the countdown starts gives the
game ~0.4 seconds (at 60fps) to settle after level load before showing numbers.

### 2. SinkPlatform Countdown (Scene_StartCountdown, 0x437130)

Called when a ball touches a "DN:SINKPLATFORM" object. This is a
level-specific mechanism, NOT the universal pre-race countdown.

- **active**: Scene+0x10F1 (byte) — 1 = countdown active
- **flag2**: Scene+0x10F2 (byte) — second gate flag
- **timer**: Scene+0x10F4 (int) — 400 (0x190) normal, 50 (0x32) for AI/demo
- **ball**: Scene+0x10F8 (ptr) — ball being frozen

**Key behavior**: Sets ball+0x14C=1 (freezes ball — disables input and
physics). Restores ball+0x2FC=1.0 (gravity_scale back to 1.0 from 0.5).
The timer (Scene+0x10F4) is set to 400 but no function in the analyzed
codebase decrements it — it may be processed by a function Ghidra hasn't
identified, or it may be a dead feature that was replaced by the
Scene_HandleRaceEnd phase system.

**Callers of Scene_StartCountdown**: SinkPlatform_OnCollision (0x413BD0),
and various level-specific collision handlers at 0x413FA0, 0x4143AE,
0x414660, 0x414ED9, 0x4151C0, 0x4155B6, 0x41624F, 0x417620, 0x417F08,
0x41873E.

---

## Ball+0x14C (Controls Disabled Flag)

This byte flag gates the entire physics pipeline in Scene_Update and the
player input path in Ball_Update.

- **Ball_ctor2** (0x403D1D): Sets to 0 (enabled) at ball creation
- **Scene_StartCountdown** (0x43717B): Sets to 1 (disabled) on SinkPlatform touch
- **Scene_HandleRaceEnd** (0x41B40D): Sets to 1 (disabled) when race timer expires
- **RumbleBoard_Update** (0x421FE0): Sets to 1 (disabled) for all balls at arena end

**Nobody clears ball+0x14C back to 0 after it's set to 1.** The ball is
frozen permanently until respawn (which creates a new ball via Ball_ctor2).

### Scene_Update Physics Gate

```asm
; At 0x419EE4-0x419F04 in Scene_Update (0x419C00)
00419ee4: MOV ECX, EBX                ; EBX = Scene+0x362C (start entries list)
00419ee6: CALL 0x004536a0             ; AthenaList_GetSize(Scene+0x362C)
00419eeb: CMP EAX, 0x1                ; ball_count == 1?
00419eee: JNZ 0x00419f06              ; If != 1: skip gate, run physics unconditionally
00419ef0: MOV EAX, [ESI + 0x878]      ; EAX = App (board+0x878)
00419ef6: MOV ECX, [EAX + 0x5dc]     ; ECX = App+0x5DC (current physics ball POINTER)
00419efc: MOV AL, [ECX + 0x14c]       ; AL = controls_disabled
00419f02: TEST AL, AL
00419f04: JNZ 0x00419f84              ; If disabled: SKIP vtable[19-22] (all physics)
```

The gate only applies when there is exactly one ball (single-player). In
multiplayer, physics always runs. App+0x5DC is a ball pointer (not a
count), set to the current physics ball.

### Ball_Update Input Gate

```asm
; At 0x4060A1 in Ball_Update (0x405E00)
004060a1: MOV AL, byte ptr [ESI + 0x14c]   ; AL = controls_disabled
004060a7: TEST AL, AL
004060a9: JNZ 0x004082C9                   ; If disabled: jump to RETURN (skip input)
```

---

## Hook Points (MinHook / DLL Detour)

### Hook 1: Scene_SpawnBallsAndObjects — RECOMMENDED

**Address**: 0x41C5B0 (vtable[32], shared across ALL levels)

**Calling convention**: `__fastcall` (ECX = board). Plain `RET` (no stack
cleanup). No additional parameters.

**Timing**: After all balls and objects are created. This is the earliest
point where balls exist. Level geometry, collision, camera, and sound are
already initialized (done by vtable[18] which calls this internally at the
end).

**Why this is the best hook**:
- Universal: vtable[32] = 0x41C5B0 in every level subclass vtable (verified)
- Complete: everything is initialized (balls, objects, camera, sound)
- Pre-update: no physics tick has run yet
- Pre-countdown: the 3-2-1 countdown hasn't started (needs 25 frames first)
- Board pointer is in ECX (thiscall this) — directly accessible
- Clean function boundary: detour entry, call original, run your code, return

```c
typedef void (__fastcall *SpawnBallsFn)(void* board);
SpawnBallsFn Orig_SpawnBalls = NULL;

void __fastcall Hooked_SpawnBallsAndObjects(void* board) {
    Orig_SpawnBalls(board);

    // At this point:
    // - All balls are spawned (ball_list at board+0x29D4)
    // - All objects are created (flags, signs, dynamic objects)
    // - Camera and sound are configured
    // - No physics tick has run yet
    // - Ball+0x14C = 0 (controls enabled)
    // - 3-2-1 countdown hasn't started (needs 25 frames)

    int ball_count = *(int*)((char*)board + 0x29D8);
    if (ball_count < 1) return;

    int** ball_array = *(int***)((char*)board + 0x2DE0);
    if (!ball_array || !*ball_array) return;

    for (int i = 0; i < ball_count; i++) {
        int* ball = (*ball_array)[i];
        if (!ball || IsBadReadPtr(ball, 0x20)) continue;

        float x = *(float*)((char*)ball + 0x164);
        float y = *(float*)((char*)ball + 0x168);
        float z = *(float*)((char*)ball + 0x16C);

        // Modify ball state here
        // *(float*)((char*)ball + 0x284) = 30.0f;  // radius
        // *(float*)((char*)ball + 0x188) = 7.0f;   // max_speed
    }
}

// Install:
// MH_CreateHook((LPVOID)0x41C5B0, &Hooked_SpawnBallsAndObjects, (LPVOID*)&Orig_SpawnBalls);
```

### Hook 2: Tournament_AdvanceRace — UNIVERSAL ENTRY

**Address**: 0x427080

**Calling convention**: `__thiscall` (ECX = tournament profile,
`[ESP+4]` = char param_1). `RET 0x4` (callee cleans 1 stack param).

**Timing**: The board is created inside this function (when param_1=0). To
access the board, call the original first, then read `[this+0xC]`.

**Parameter**:
- `param_1 = '\0'` (0): Create board — loads level, spawns balls
- `param_1 = '\x01'` (1): Show tourney menu — stores scores, no board created

Only hook fires on the `param_1=0` path. Check param_1 before running your code.

```c
typedef void (__thiscall *AdvanceRaceFn)(void* profile, char param_1);
AdvanceRaceFn Orig_AdvanceRace = NULL;

void __thiscall Hooked_AdvanceRace(void* profile, char param_1) {
    Orig_AdvanceRace(profile, param_1);

    // Only run when a board was actually created
    if (param_1 != 0) return;

    // Board pointer is stored at profile+0x0C after creation
    int* board = *(int**)((char*)profile + 0x0C);
    if (!board || IsBadReadPtr(board, 0x100)) return;

    // Ball list
    int ball_count = *(int*)((char*)board + 0x29D8);
    if (ball_count < 1) return;

    int** ball_array = *(int***)((char*)board + 0x2DE0);
    if (!ball_array || !*ball_array) return;

    int* ball = (*ball_array)[0];
    if (!ball || IsBadReadPtr(ball, 0x20)) return;

    // Your onRaceStart code here
}

// Install:
// MH_CreateHook((LPVOID)0x427080, &Hooked_AdvanceRace, (LPVOID*)&Orig_AdvanceRace);
```

**Important**: The doc's earlier version claimed that EAX = board pointer at
address 0x4273E3 (after the vtable[18] call). This is WRONG — EAX is
clobbered by the vtable[18] call at 0x4273E0. The board pointer is stored
at `[ESI+0x0C]` (profile+0x0C) and loaded into EAX at 0x4273E6. When using
MinHook, you don't need to worry about register state — just detour the
function entry and read profile+0x0C after calling the original.

### Hook 3: App_StartRace — EARLIEST

**Address**: 0x4287C0

**Calling convention**: `__thiscall` (ECX = App, `[ESP+4]` = int flag).
`RET 0x4` (callee cleans 1 stack param). The flag is always 1 in all
observed callers (App_StartTournamentRace, App_StartPracticeRace,
Scene_HandleCountdown).

**Timing**: Before scene reset. Old objects are still alive. The new level
hasn't been loaded yet.

**Use case**: When you need to run code BEFORE the old scene is destroyed
(e.g., save data from the previous race). Too early for accessing new balls.

```c
typedef void (__thiscall *StartRaceFn)(void* app, int flag);
StartRaceFn Orig_StartRace = NULL;

void __thiscall Hooked_StartRace(void* app, int flag) {
    // Old scene is still alive at App+0x178
    int* old_scene = *(int**)((char*)app + 0x178);
    // Save data from previous race here...

    Orig_StartRace(app, flag);
    // After this: old scene destroyed, but new board not yet created
}

// Install:
// MH_CreateHook((LPVOID)0x4287C0, &Hooked_StartRace, (LPVOID*)&Orig_StartRace);
```

### Hook 4: Level_InitScene — AFTER GEOMETRY, BEFORE BALLS

**Address**: 0x40B090 (base vtable[18])

**Calling convention**: `__fastcall` (ECX = board). Plain `RET`.

**Timing**: After MeshWorld_ctor and CollisionLevel_ctorWithLevel have
loaded geometry and collision data. Sound, camera, and graphics are set up
here. Balls have NOT been spawned yet (vtable[32] hasn't run).

**Note**: This is the BASE vtable[18]. Each level subclass overrides
vtable[18] with its own loader (e.g., Scene_LoadLevel2 at 0x40D280) which
internally calls MeshWorld_ctor → CollisionLevel_ctorWithLevel →
Level_InitScene → vtable[32]. Hooking Level_InitScene directly catches
the moment after camera/sound setup but before balls exist.

```c
typedef void (__fastcall *InitSceneFn)(int* board);
InitSceneFn Orig_InitScene = NULL;

void __fastcall Hooked_InitScene(int* board) {
    Orig_InitScene(board);
    // Geometry loaded, collision loaded, camera/sound set up
    // Balls NOT yet spawned (vtable[32] hasn't run)
    // Good for: modifying camera, changing level music, etc.
}
```

### Hook 5: Ball_ctor2 — AT BALL CREATION

**Address**: 0x4039E0

**Calling convention**: `__thiscall` (ECX = allocated memory,
`[ESP+4]` = scene pointer). `RET 0x4`.

**Timing**: During Scene_SpawnBallsAndObjects, for each player entry. The
ball is being constructed — fields get default values here, then
SpawnBalls overrides some of them (radius, max_speed, gravity_scale).

**Use case**: When you need to modify ball fields at the earliest possible
point. Note that SpawnBalls will override radius (27→26), max_speed
(5000→5), and gravity_scale (1.0→0.5) AFTER ctor returns.

```c
typedef void* (__thiscall *BallCtorFn)(void* alloc, int scene);
BallCtorFn Orig_BallCtor = NULL;

void* __thiscall Hooked_BallCtor(void* alloc, int scene) {
    void* ball = Orig_BallCtor(alloc, scene);
    // Ball is constructed with default values
    // Fields will be overridden by SpawnBalls after this returns
    return ball;
}
```

### Hook 6: Ball_Update — FIRST PHYSICS TICK

**Address**: 0x405E00 (vtable[4])

**Calling convention**: `__thiscall` (ECX = ball). Plain `RET`.

**Timing**: First physics tick after level load. The 3-2-1 countdown has
started (but ball+0x14C=0, so input is enabled). This runs EVERY FRAME for
EVERY BALL, so use a one-shot flag.

```c
typedef void (__thiscall *BallUpdateFn)(void* ball);
BallUpdateFn Orig_BallUpdate = NULL;

static bool race_started = false;

void __thiscall Hooked_BallUpdate(void* ball) {
    if (!race_started) {
        race_started = true;
        // First ball update after level load
        // ball+0x14C = 0 (controls enabled)
        // Countdown is displaying but ball can move
    }
    Orig_BallUpdate(ball);
}
```

### Hook 7: Scene_HandleRaceEnd — COUNTDOWN START

**Address**: 0x41B130 (vtable[19])

**Calling convention**: `__fastcall` (ECX = board). Plain `RET`.

**Timing**: First call is 25 frames after level load. The 3-2-1 countdown
display begins here. Also called every frame during the race (handles
race-end when timer expires).

**Use case**: When you need to run code exactly when the countdown starts
displaying (not before, not after). Check Scene+0x3A4C to distinguish
countdown-start from race-end.

### Hook 8: Scene_AddObject — BOARD ADDED TO SCENE TREE

**Address**: 0x469990

**Calling convention**: `__thiscall` (ECX = scene object list,
`[ESP+4]` = object to add). `RET 0x4`.

**Timing**: Called by Tournament_AdvanceRace right after vtable[18] returns.
The board has been fully initialized (geometry, balls, objects) and is
being added to the scene tree for rendering.

**Use case**: When you need to intercept the moment the board becomes
visible to the scene tree. The added object is the board itself.

### Hook 9: Scene_SetRaceActive — RACE ACTIVE FLAG

**Address**: 0x4366E0

**Calling convention**: `__fastcall` (ECX = board). Plain `RET`.

**Timing**: Sets App+0x10EC = 1. Called from many places (62 xrefs) —
NOT just at race start. Called whenever the game transitions to "race
active" state.

**Use case**: Not recommended for race-start detection (too many callers,
fires in many contexts). Included here for completeness.

---

## Complete Race-Start Timeline

```
Frame 0:   App_Start*Race → App_StartRace → reset old scene
           Tournament_AdvanceRace (show menu, param_1=1)
           [OR] Tournament_AdvanceRace (create board, param_1=0)

Frame 0+:  Tournament_AdvanceRace (param_1=0) creates board:
             switch(race_index) → level-specific Board ctor → Board_ctor
             (*board->vtable[18])(board):
               [Level-specific override, e.g. Scene_LoadLevel2]:
                 MeshWorld_ctor → CollisionLevel_ctorWithLevel
                 → Level_InitScene (sound/camera/graphics)
                 → (*board->vtable[32])(board) = Scene_SpawnBallsAndObjects:
                   Ball_ctor2 (0x14C=0, radius=27, max_speed=5000, gravity=1.0)
                   Ball_SetTrajectory (spawn position)
                   Override: radius=26, max_speed=5, gravity=0.5
                   AthenaList_Append(Scene+0x29D4, ball)
                   CreateBadBall, CreateMouseTrap, CreateFlags, CreateSigns
                   CreateDynamicObjects
             Scene_AddObject (board → scene tree)
             Set up player scores

           ↑ HOOK 1 (Scene_SpawnBallsAndObjects): balls just spawned ← BEST
           ↑ HOOK 2 (Tournament_AdvanceRace): board at profile+0x0C
           ↑ HOOK 4 (Level_InitScene): geometry loaded, no balls yet
           ↑ HOOK 5 (Ball_ctor2): ball being constructed
           ↑ HOOK 8 (Scene_AddObject): board added to scene tree

Frame 1:   Scene_Update (vtable[1]):
             AthenaList_GetSize(Scene+0x362C) → ball_count
             if (ball_count==1 && ball+0x14C!=0): skip physics
             else:
               vtable[19] = Scene_HandleRaceEnd:
                 Scene+0x3620 = 1 (frame counter, < 25 → skip countdown)
               vtable[20] = Scene_UpdateBallsAndState:
                 Ball_Update (vtable[4]) — first physics tick

           ↑ HOOK 6 (Ball_Update entry): first physics tick
           ↑ HOOK 7 (Scene_HandleRaceEnd): called but countdown skipped

Frame 2-25: Scene_Update repeats. Countdown not yet displayed.
             Ball can move freely (ball+0x14C=0).

Frame 26:  Scene_HandleRaceEnd:
             Scene+0x3620 > 25 → start countdown
             Phase 0: display "3" (App+0x394 texture)
             Scene+0x3A50 = 0

           ↑ HOOK 7 (Scene_HandleRaceEnd): countdown display starts

Frame ~150: Phase 1: display "2" (App+0x398 texture)

Frame ~275: Phase 2: display "1" (App+0x39C texture), play sound

Frame ~400: Phase 3: Scene+0x3A4C = 1 (countdown done)
             Start race timers, iterate balls
```

---

## Scene Vtable Map (vtable base at 0x4D0260)

All entries verified by reading raw vtable bytes from memory.

- **[0]** (0x00): 0x425020 — Board_dtor (destructor)
- **[1]** (0x04): 0x419C00 — Scene_Update (main game tick)
- **[2]** (0x08): 0x41A2E0 — Scene_SetCamera (camera setup per ball)
- **[18]** (0x48): 0x40B090 — Level_InitScene (BASE: sound, camera, graphics)
- **[19]** (0x4C): 0x41B130 — Scene_HandleRaceEnd (3-2-1 countdown + race-end)
- **[20]** (0x50): 0x41B540 — Scene_UpdateBallsAndState (ball physics update)
- **[21]** (0x54): 0x40A040 — NoOp (unused slot)
- **[22]** (0x58): 0x41A540 — Scene_HandleCountdown (race-end menu/results)
- **[27]** (0x6C): 0x41B710 — Scene_RenderScoreHUD (render countdown numbers + HUD)
- **[28]** (0x70): 0x41BFD0 — Scene_RenderTimerHUD (render race timer)
- **[31]** (0x7C): 0x41AC70 — Scene_vmethod31 (per-ball camera + render setup)
- **[32]** (0x80): 0x41C5B0 — Scene_SpawnBallsAndObjects (spawn balls + all objects)
- **[33]** (0x84): 0x419750 — Scene_method33

**vtable[18] is OVERRIDDEN by each level subclass.** The base vtable[18]
(0x40B090 = Level_InitScene) is what the BASE class uses. Each level
subclass assigns its own vtable with a different vtable[18] that loads
the level-specific .MESHWORLD file. These overrides internally call
MeshWorld_ctor → CollisionLevel_ctorWithLevel → Level_InitScene →
vtable[32].

**vtable[32] is shared across ALL levels** (0x41C5B0 in every subclass
vtable, verified by reading 4 different level vtables).

Example level vtable[18] overrides (verified from vtable memory reads):

- L1 WarmUp (0x4D04A8): vtable[18] = 0x40D1C0 — loads "levels\\level1"
- L2 Cascade (0x4D1098): vtable[18] = 0x4110D0 — loads "levels\\levelcascade"
- L3 Intermediate (0x4D05A0): vtable[18] = 0x40D280 — loads "levels\\level2"
- L4 Dizzy (0x4D0890): vtable[18] = 0x40D390 — loads "levels\\level3"

---

## Key Offsets Summary

### Ball (0xC60 bytes, allocated by operator_new in Scene_SpawnBallsAndObjects)

| Offset | Type | Name | Set By | Value |
|--------|------|------|--------|-------|
| +0x14C | byte | controls_disabled | Ball_ctor2 (0) | 0 |
| +0x164 | float | pos_x | Scene_SpawnBallsAndObjects | START position |
| +0x168 | float | pos_y | Scene_SpawnBallsAndObjects | START position |
| +0x16C | float | pos_z | Scene_SpawnBallsAndObjects | START position |
| +0x18 | int | player_id | Ball_ctor2 | -1 = AI/demo, 0+ = player |
| +0x188 | float | max_speed | Ball_ctor2 → SpawnBalls | ctor=5000.0, SpawnBalls=5.0 |
| +0x1A0 | float | speed_multiplier | Scene_SpawnBallsAndObjects | 1.05 |
| +0x27C | float | friction_scale | Scene_SpawnBallsAndObjects | 0.1 |
| +0x284 | float | radius | Ball_ctor2 → SpawnBalls | ctor=27.0, SpawnBalls=26.0 |
| +0x2F8 | byte | is_falling | Ball_ctor2 / SpawnBalls | 0 |
| +0x2FC | float | gravity_scale | Ball_ctor2 → SpawnBalls | ctor=1.0, SpawnBalls=0.5 |
| +0x310 | byte | has_target_pos | Ball_ctor2 | 1 |
| +0x769 | byte | active_flag | Scene_SpawnBallsAndObjects | 1 |

### Scene/Board (variable size, 0x436C–0x6498 depending on level)

| Offset | Type | Name | Description |
|--------|------|------|-------------|
| +0x878 | ptr | app | App object pointer |
| +0x184 | ptr | scene_object_list | AthenaList of scene objects |
| +0x29D4 | AthenaList | ball_list | Main ball list (player balls) |
| +0x29D8 | int | ball_count | Number of balls in ball_list |
| +0x2DE0 | ptr | ball_array | Pointer to ball pointer array |
| +0x3620 | int | frame_counter | Incremented each Scene_Update |
| +0x3624 | float | race_timer | Decrements during race |
| +0x362C | AthenaList | start_entries_list | Start position entries (used for ball_count gate) |
| +0x3A4C | byte | countdown_done | 1 = 3-2-1 countdown finished |
| +0x3A50 | int | countdown_phase | 0="3", 1="2", 2="1" |
| +0x3A54 | float | countdown_phase_timer | Time in current phase |
| +0x3A58 | byte | countdown_sound_played | Prevents double-sound |
| +0x8B4 | int | countdown_skip_flag | Non-zero = skip countdown |
| +0x10F1 | byte | sink_countdown_active | SinkPlatform countdown |
| +0x10F4 | int | sink_countdown_timer | 400 normal, 50 AI/demo |
| +0x10F8 | ptr | sink_countdown_ball | Ball being frozen by SinkPlatform |

### App (global at 0x5341E0)

| Offset | Type | Name | Description |
|--------|------|------|-------------|
| +0x174 | ptr | gfx_device | Graphics device (D3D) |
| +0x178 | ptr | sound_device | SoundDevice (SoundDevice_UpdateChannels) — NOT scene! |
| +0x17C | ptr | music_device | MusicDevice (MusicDevice_FadeAll) |
| +0x180 | ptr | input_device | InputDevice (InputDevice_PollAndRelease) |
| +0x184 | ptr | meshworld | MeshWorld / scene manager (contains board in its list) |
| +0x220 | ptr | player_profile | Current PlayerProfile (profile+0xC = board) |
| +0x237 | byte | is_arena | 1 = arena mode (2P), 0 = tournament/practice |
| +0x23C | int | difficulty | 0=Pipsqueak, 1=Normal, 2=Frenzied |
| +0x5DC | ptr | physics_ball | Current physics ball pointer (read-only, never written in code) |
| +0x10EC | int | race_active | Set to 1 by Scene_SetRaceActive |

**WARNING**: Previous versions of this doc claimed App+0x178 = "scene". This is
WRONG. App+0x178 is the SoundDevice, verified via App_FrameUpdate (0x46C170)
which calls `SoundDevice_UpdateChannels(*(App+0x178))`. The scene/board is
NOT stored at any single App field — see "Getting the Scene Pointer" below.

---

## Complete Calling Convention Reference

All verified via disassembly (RET instruction inspection).

| Function | Address | Convention | RET | Params |
|----------|---------|------------|-----|--------|
| App_StartRace | 0x4287C0 | __thiscall | RET 0x4 | ECX=App, [ESP+4]=int flag (always 1) |
| App_StartTournamentRace | 0x4288B0 | __fastcall | RET | ECX=App |
| App_StartTourneyRace | 0x4289F0 | __fastcall | RET | ECX=App |
| App_StartMPRace | 0x428B20 | __thiscall | RET 0x4 | ECX=App, [ESP+4]=? |
| App_StartPracticeRace | 0x428C50 | __thiscall | RET 0x4 | ECX=App, [ESP+4]=? |
| App_Start2PRace | 0x429230 | __thiscall | RET 0x4 | ECX=App, [ESP+4]=? |
| Tournament_AdvanceRace | 0x427080 | __thiscall | RET 0x4 | ECX=profile, [ESP+4]=char param_1 |
| Scene_SpawnBallsAndObjects | 0x41C5B0 | __fastcall | RET | ECX=board |
| Scene_Update | 0x419C00 | __fastcall | RET | ECX=board |
| Scene_HandleRaceEnd | 0x41B130 | __fastcall | RET | ECX=board |
| Scene_UpdateBallsAndState | 0x41B540 | __fastcall | RET | ECX=board |
| Scene_HandleCountdown | 0x41A540 | __fastcall | RET | ECX=board |
| Scene_StartCountdown | 0x437130 | __thiscall | RET 0x4 | ECX=scene, [ESP+4]=ball |
| Ball_Update | 0x405E00 | __thiscall | RET | ECX=ball |
| Ball_ctor2 | 0x4039E0 | __thiscall | RET 0x4 | ECX=alloc, [ESP+4]=scene |
| Ball_SetTrajectory | 0x403850 | __thiscall | RET 0x14 | ECX=ball, 5 stack floats |
| Level_InitScene | 0x40B090 | __fastcall | RET | ECX=board |
| Scene_AddObject | 0x469990 | __thiscall | RET 0x4 | ECX=list, [ESP+4]=obj |
| MeshWorld_ctor | 0x461510 | __thiscall | RET 0x8 | ECX=alloc, [ESP+4]=d3dDevice, [ESP+8]=path |
| CollisionLevel_ctorWithLevel | 0x465080 | __thiscall | RET 0x4 | ECX=alloc, [ESP+4]=sourceMesh |
| Board_ctor | 0x419030 | __thiscall | RET 0x4 | ECX=alloc, [ESP+4]=App |
| Scene_SetRaceActive | 0x4366E0 | __fastcall | RET | ECX=board |

## Getting the Scene Pointer

There is **NO global variable** that directly holds the current board/scene
pointer. The board is only accessible through indirect chains or by hooking
a function that receives it as a parameter. Verified by searching the entire
binary for writes to any dedicated "current scene" field — none exist.

### Method 1: Hook Scene_Update + Global (RECOMMENDED for MinHook)

Hook Scene_Update (0x419C00), save ECX to a global, and compare with the
previous frame to detect level changes.

**Why Scene_Update (0x419C00) and NOT Ball_Update (0x405E00):**
- Scene_Update is called for ALL levels, ALL modes, EVERY frame
- vtable[1] is overridden by some levels (Intermediate=0x41CC90, Dizzy=0x41D510,
  RumbleBoard=0x421FE0), but ALL overrides `CALL 0x419C00` directly — so the
  MinHook at 0x419C00 fires for every level
- Ball_Update (0x405E00) is NOT called for player balls in race mode!
  vtable[4] (Ball_AI_ChaseNearest, 0x408390) checks `ball+0xC74` (AI flag)
  and `App+0x237` (is_arena). If neither is set: skips Ball_Update and calls
  vtable[5] instead. Ball_Update only fires for AI balls or arena mode.
- Scene_Update is `__fastcall` (ECX = board), plain `RET` — clean hooking

**Why Scene_SpawnBallsAndObjects (0x41C5B0) may not have fired:**
The function IS called for every level (verified via vtable analysis + disasm
of 4 level loaders). ASLR is disabled, address is correct. If the hook doesn't
fire, likely causes are: MinHook initialization failure (check `MH_CreateHook`
return code), hook not enabled (`MH_EnableHook` not called), or output
mechanism not working (use `MessageBoxA` for debugging, not `fopen`).

```c
static int* g_scene = NULL;
static int* g_prev_scene = NULL;
static bool g_level_just_started = false;

typedef void (__fastcall *SceneUpdateFn)(int* board);
SceneUpdateFn Orig_SceneUpdate;

void __fastcall Hooked_SceneUpdate(int* board) {
    g_scene = board;

    if (g_scene != g_prev_scene) {
        // Level just changed (or first frame)
        g_level_just_started = true;
        g_prev_scene = g_scene;
    } else {
        g_level_just_started = false;
    }

    Orig_SceneUpdate(board);
}

// Install: MH_CreateHook((LPVOID)0x419C00, &Hooked_SceneUpdate, (LPVOID*)&Orig_SceneUpdate);
// Then anywhere: if (g_level_just_started) { ... onLevelStart code ... }
```

This gives you both a persistent scene pointer AND a level-change callback.

### Method 2: App Global → PlayerProfile → Board

Read the App global at 0x5341E0, then follow the profile chain.

```c
int* app = *(int**)0x5341E0;
if (!app || IsBadReadPtr(app, 0x300)) return NULL;

int* profile = *(int**)((char*)app + 0x220);
if (!profile || IsBadReadPtr(profile, 0x20)) return NULL;

int* board = *(int**)((char*)profile + 0x0C);
if (!board || IsBadReadPtr(board, 0x100)) return NULL;

// board is now the scene/board pointer
```

**Caveat**: profile+0xC is set to NULL at the START of Tournament_AdvanceRace
(before creating the new board). It's only valid during active gameplay.

### Method 3: App Global → MeshWorld → Items Array → Board

Read the scene manager (MeshWorld) and iterate its object list.

```c
int* app = *(int**)0x5341E0;
if (!app || IsBadReadPtr(app, 0x200)) return NULL;

int* meshworld = *(int**)((char*)app + 0x184);
if (!meshworld || IsBadReadPtr(meshworld, 0x500)) return NULL;

// MeshWorld+0x40C = pointer to items array (AthenaList internal)
int* items = *(int**)((char*)meshworld + 0x40C);
if (!items || IsBadReadPtr(items, 0x10)) return NULL;

// First item = board (usually the only object in the list)
int* board = *(int**)items;
if (!board || IsBadReadPtr(board, 0x100)) return NULL;

// board is now the scene/board pointer
```

**Verified**: GameUpdate (0x469CF0) uses this exact path:
`MOV EAX, [EBP+0x40C]; MOV ECX, [EAX]` to get the first board from the list.

### Why App+0x178 Does NOT Work

Previous docs claimed App+0x178 = "scene pointer". This is **wrong**.
App_FrameUpdate (0x46C170) proves it:

```c
// From App_FrameUpdate decompilation:
if (*(int*)(param_1 + 0x178) != 0)
    SoundDevice_UpdateChannels(*(int*)(param_1 + 0x178));  // App+0x178 = SOUND DEVICE
GameUpdate(*(int*)(param_1 + 0x184));                       // App+0x184 = scene manager
```

App+0x178 is the SoundDevice, not the scene. The confusion arose because
App_StartRace calls `Scene_UpdateChildren(*(App+0x178))`, and Scene_UpdateChildren
was misidentified as a scene function — it's actually a sound channel cleanup call.

---

## Pitfalls

### 1. Ghidra Function Name Confusion

- **Scene_HandleRaceEnd** (0x41B130, vtable[19]) = Pre-race **3-2-1 countdown**
  display, NOT just race-end. It also handles post-race results.
- **Scene_HandleCountdown** (0x41A540, vtable[22]) = **Race-end** menu/results
  handler, NOT the pre-race countdown. Name is misleading.
- **Scene_StartCountdown** (0x437130) = **SinkPlatform** freeze countdown,
  NOT the universal pre-race 3-2-1 countdown.

### 2. Ball+0x14C Is NOT the Countdown Gate

The 3-2-1 countdown does NOT set ball+0x14C. The ball is free to move during
the countdown. Ball+0x14C is only set by:
- SinkPlatform collision (Scene_StartCountdown)
- Race timer expiry (Scene_HandleRaceEnd post-countdown)
- Arena end (RumbleBoard_Update)

### 3. Scene+0x10F4 Timer Is Dead Code

The SinkPlatform countdown timer (Scene+0x10F4=400) is set by
Scene_StartCountdown but no function in the analyzed codebase decrements it.
Verified by searching for 0x10F4 references in Scene_Update,
Scene_HandleRaceEnd, Scene_UpdateBallsAndState, and Scene_HandleCountdown —
none reference it. It may be a legacy feature replaced by the
Scene_HandleRaceEnd phase system.

### 4. Multiple Scene Vtables

There are 15+ scene subclass vtables (one per level type), each at a different
address. vtable[18] is OVERRIDDEN per level (each loads a different
.MESHWORLD file), but vtable[32] (Scene_SpawnBallsAndObjects) is shared.
For universal hooks, target shared functions (0x41C5B0, 0x427080) rather
than per-level functions.

### 5. Arena vs Race Initialization

Arena levels use `RumbleBoard_*_Init` functions (e.g.
RumbleBoard_WarmUp_Init at 0x413C20) instead of `Scene_LoadLevel*`.
These also call vtable[32] = Scene_SpawnBallsAndObjects, so the same
hook works for both arenas and races.

### 6. Tournament_AdvanceRace param_1

`Tournament_AdvanceRace(profile, param_1)`:
- `param_1 = '\0'` (0): **Create board** — loads level, spawns balls, adds to scene
- `param_1 = '\x01'` (1): **Show tourney menu** — stores scores, creates TourneyMenu
- `param_1 = '\x01'` with `App+0x237` set: Creates `TourneyMenu_CreateBoard` (arena)

Only the `param_1 = '\0'` path creates the board. Hook 1 fires on this path.

### 7. Ball Field Overrides

Ball_ctor2 sets initial values, but Scene_SpawnBallsAndObjects overrides
several fields AFTER Ball_ctor2 returns. If you hook Ball_ctor2 to modify
a field, SpawnBalls may overwrite your change. Hook
Scene_SpawnBallsAndObjects instead to modify fields after all overrides.

### 8. App_StartRace Has a Stack Parameter

App_StartRace (0x4287C0) has `RET 0x4`, meaning it cleans 1 stack
parameter. All callers push 1 before calling. Ghidra labels it
`__fastcall` but it's effectively `__thiscall` (ECX=App). When hooking
with MinHook, declare it as `__thiscall` with 2 params.

---

*Document verified via GhidraMCP decompilation and disassembly of
Hamsterball.exe (V3.6.c, md5=7d25019366b8d7f55906325bd630d7fe). All
function addresses, vtable layouts, calling conventions (via RET
instruction inspection), and field offsets cross-referenced against
raw decompiled C code and x86 disassembly. Scene vtable at 0x4D0260
verified by reading raw memory bytes. Level subclass vtables verified
for WarmUp (0x4D04A8), Cascade (0x4D1098), Intermediate (0x4D05A0),
and Dizzy (0x4D0890).*
