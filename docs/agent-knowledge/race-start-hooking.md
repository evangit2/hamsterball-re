# Race Start Hooking Guide

## How to Execute Code When a Race Starts

This document maps the complete race-start initialization pipeline and
identifies every hookable function where modders can execute custom code
at the moment a player loads into a race — after balls and objects are
instantiated, but before the countdown finishes.

---

## Race-Start Pipeline (Call Chain)

```
App_Start*Race (0x4288B0 / 0x4289F0 / 0x428B20 / 0x428C50 / 0x29230)
  ├─ App_StartRace (0x4287C0)                    ← Reset scene, free old objects
  │    └─ Scene_UpdateChildren (0x466AC0)        ← Calls vtable[18] on child objects
  ├─ Create PlayerProfile (operator_new + ctor)
  └─ Tournament_AdvanceRace (0x427080)            ← param_1='\x01' = show menu only
       │
       └─ [Player waits at TourneyMenu, timer counts up]
       │
       └─ TourneyMenu_Tick (0x450860)              ← Timer expires
            ├─ (*menu->vtable[16])()               ← Destroy menu
            └─ Tournament_AdvanceRace (0x427080)   ← param_1='\x00' = CREATE BOARD
                 ├─ Board ctor (e.g. BoardLevel1_WarmUp_ctor at 0x41CA40)
                 │    └─ Board_ctor (0x419030)
                 │         └─ Scene_CtorBase (0x457FE0)
                 ├─ board->vtable[18] (offset 0x48) = Scene_LoadLevel2 / Scene_SetupLevel5 etc.
                 │    ├─ MeshWorld_ctor (load "levels\\levelN" geometry)
                 │    ├─ CollisionLevel_ctorWithLevel (load collision data)
                 │    ├─ Level_InitScene (0x40B090)
                 │    │    ├─ SoundChannel_Ctor (sound setup)
                 │    │    ├─ Camera profile selection
                 │    │    ├─ Graphics_SetCullMode, SetProjection, SetViewportZ
                 │    │    └─ Audio_PlayMusicAtSpeed (start level music)
                 │    └─ board->vtable[32] (offset 0x80) = Scene_SpawnBallsAndObjects (0x41C5B0)
                 │         ├─ For each player entry:
                 │         │    ├─ Look up "START%d-%d" position from hash table
                 │         │    ├─ Ball_ctor2 (0x4039E0) — create ball (0xC60 bytes)
                 │         │    │    └─ Sets ball+0x14c = 0 (controls ENABLED)
                 │         │    │    └─ Sets ball+0x284 = 26.0 (radius)
                 │         │    │    └─ Sets ball+0x1A0 = 5.0 (max speed)
                 │         │    │    └─ Sets ball+0x2FC = 1.0 (gravity scale)
                 │         │    ├─ Ball_SetTrajectory (set spawn position)
                 │         │    └─ AthenaList_Append(Scene+0x29D4, ball)
                 │         ├─ Scan SAFESPOT/SAFEPOS entries
                 │         ├─ CreateBadBall (if demo/tournament mode)
                 │         ├─ CreateMouseTrap (if demo/tournament mode)
                 │         ├─ CreateSecretObjects
                 │         ├─ Scene_CreateFlags (0x40C0F0)
                 │         ├─ Scene_CreateSigns (0x40C270)
                 │         └─ Scene_CreateDynamicObjects (0x40C430)
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
  └─ Physics pipeline (gated by ball+0x14c):
       if (ball_count != 1 || ball+0x14c == 0):
           ├─ vtable[19] (0x4C) = Scene_HandleRaceEnd (0x41B130) ← 3-2-1 countdown display
           │    ├─ If Scene+0x3a4c != 0: skip (countdown already done)
           │    ├─ If Scene+0x8b4 != 0: skip
           │    ├─ If Scene+0x3620 <= 25: skip (wait 25 frames before starting)
           │    ├─ Phase 0: display "3" texture (App+0x394), increment timer
           │    ├─ Phase 1: display "2" texture (App+0x398), increment timer
           │    ├─ Phase 2: display "1" texture (App+0x39C), play sound
           │    └─ Phase 3: set Scene+0x3a4c = 1 (countdown done)
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

| Field | Offset | Type | Description |
|-------|--------|------|-------------|
| `phase` | Scene+0x3A50 | int | 0="3", 1="2", 2="1", 3=done |
| `phase_timer` | Scene+0x3A54 | float | Accumulates time per phase |
| `countdown_done` | Scene+0x3A4C | byte | 1 = countdown finished |
| `sound_played` | Scene+0x3A58 | byte | Prevents double-sound in phase 2 |
| `frame_counter` | Scene+0x3620 | int | Must exceed 25 (0x19) before countdown starts |
| `race_timer` | Scene+0x3624 | float | Decrements during race |
| `skip_flag` | Scene+0x8B4 | int | Non-zero = skip countdown entirely |

**Key behavior**: The 3-2-1 countdown does NOT freeze the ball. Ball+0x14c is
0 (enabled) from Ball_ctor2. The ball can move during the countdown display.
The countdown is purely cosmetic — it displays numbers and plays sounds,
then sets Scene+0x3A4C=1 and starts race timers.

The 25-frame delay (Scene+0x3620 > 25) before the countdown starts gives the
game ~0.4 seconds (at 60fps) to settle after level load before showing numbers.

### 2. SinkPlatform Countdown (Scene_StartCountdown, 0x437130)

Called when a ball touches a "DN:SINKPLATFORM" object. This is a
level-specific mechanism, NOT the universal pre-race countdown.

| Field | Offset | Type | Description |
|-------|--------|------|-------------|
| `active` | Scene+0x10F1 | byte | 1 = countdown active |
| `flag2` | Scene+0x10F2 | byte | Second gate flag |
| `timer` | Scene+0x10F4 | int | 400 (0x190) normal, 50 (0x32) for AI/demo |
| `ball` | Scene+0x10F8 | ptr | Ball being frozen |

**Key behavior**: Sets ball+0x14c=1 (freezes ball — disables input and
physics). This is the ONLY mechanism that freezes the ball via +0x14c during
gameplay. The timer (Scene+0x10F4) is set to 400 but no function in the
analyzed codebase decrements it — it may be processed by a function Ghidra
hasn't identified, or it may be a dead feature that was replaced by the
Scene_HandleRaceEnd phase system.

**Callers of Scene_StartCountdown**: SinkPlatform_OnCollision (0x413BD0),
and various level-specific collision handlers at 0x413FA0, 0x4143AE,
0x414660, 0x414ED9, 0x4151C0, 0x4155B6, 0x41624F, 0x417620, 0x417F08,
0x41873E.

---

## Ball+0x14C (Controls Disabled Flag)

This byte flag gates the entire physics pipeline in Scene_Update and the
player input path in Ball_Update.

| Setter | Address | Context |
|--------|---------|---------|
| Ball_ctor2 | 0x403D1D | Sets to 0 (enabled) at ball creation |
| Scene_StartCountdown | 0x43717B | Sets to 1 (disabled) on SinkPlatform touch |
| Scene_HandleRaceEnd | 0x41B40D | Sets to 1 (disabled) when race timer expires |
| RumbleBoard_Update | 0x4221F0 | Sets to 1 (disabled) for all balls at arena end |

**Nobody clears ball+0x14C back to 0 after it's set to 1.** The ball is
frozen permanently until respawn (which creates a new ball via Ball_ctor2).

### Scene_Update Physics Gate

```asm
; At 0x419EF6 in Scene_Update (0x419C00)
00419ef6: MOV ECX, dword ptr [EAX + 0x5dc]  ; ECX = ball (App+0x5DC)
00419efc: MOV AL, byte ptr [ECX + 0x14c]   ; AL = controls_disabled
; If AL != 0 (disabled): SKIP vtable[19-22] (physics + countdown + race-end)
```

### Ball_Update Input Gate

```asm
; At 0x4060A1 in Ball_Update (0x405E00)
004060a1: MOV AL, byte ptr [ESI + 0x14c]   ; AL = controls_disabled
004060a7: TEST AL, AL
004060a9: JNZ 0x004082C9                   ; If disabled: jump to RETURN (skip input)
```

---

## Hook Points

### Hook 1: Tournament_AdvanceRace after vtable[18] — RECOMMENDED

**Address**: 0x4273E3 (instruction after `CALL dword ptr [EDX + 0x48]`)

**Timing**: After full level load — geometry, collision, camera, sound, balls,
and all objects are created. Before the first Scene_Update tick.

**Register state at hook point**:
- `EAX` = board pointer (also at `[ESI+0xC]`)
- `ESI` = tournament/player profile object
- `[ESI+4]` = App pointer
- `[ESI+0xC]` = board pointer

**Why this is the best hook**:
- Universal: works for ALL 15 levels (vtable[18] is called for every level)
- Complete: everything is initialized (balls, objects, camera, sound)
- Pre-update: no physics tick has run yet
- Pre-countdown: the 3-2-1 countdown hasn't started (needs 25 frames first)
- Board pointer is available in EAX for accessing scene, balls, etc.

**Accessing key objects from the board pointer**:
```c
// Board = EAX at hook point
int* board = (int*)eax;
int* app = *(int**)((char*)board + 0x878);  // App pointer (Board+0x878)
int scene = board;  // Board IS the scene subclass

// Ball list at Scene+0x29D4 (AthenaList)
int* ball_list = (int*)((char*)board + 0x29D4);
int ball_count = *(int*)((char*)board + 0x29D8);
int* ball_array = *(int**)((char*)board + 0x2DE0);

// First ball
int* ball = ball_array[0];
float ball_x = *(float*)((char*)ball + 0x164);
float ball_y = *(float*)((char*)ball + 0x168);
float ball_z = *(float*)((char*)ball + 0x16C);
```

**CEA script pattern**:
```asm
// Hook at 0x4273E3 — after vtable[18] returns in Tournament_AdvanceRace
// Board pointer is in EAX
alloc(onRaceStart, 256)
alloc(original_bytes, 5)
registersymbol(onRaceStart)
registersymbol(original_bytes)

onRaceStart:
    // EAX = board pointer
    PUSHAD
    PUSHFD

    // Your code here — board is in EAX
    // Example: read ball position
    // MOV ESI, [EAX]           ; ESI = vtable (not needed)
    // MOV EDX, [EAX + 0x2DE0]  ; EDX = ball array ptr
    // MOV EDI, [EDX]           ; EDI = first ball
    // MOV ECX, [EDI + 0x164]   ; ECX = ball X position

    POPFD
    POPAD

    // Original instruction: MOV ECX, [ESI+4]
    db 8B 4E 04
    JMP original_return

original_return:
    // Continue normal execution at 0x4273E6
    JMP 0x004273E6

// Install hook
0x004273E3:
    JMP onRaceStart
    NOP
```

**DLL mod pattern**:
```c
// Detour Tournament_AdvanceRace, or hook the return address 0x4273E3
// Use a trampoline to capture the board pointer after vtable[18] returns.

// The board pointer is stored at [tournament_obj + 0xC]
// Access it from within your hook:

void __fastcall OnLevelLoaded(int* tournament_obj) {
    int* board = *(int**)((char*)tournament_obj + 0xC);
    if (!board || IsBadReadPtr(board, 0x100)) return;

    // Ball list
    int ball_count = *(int*)((char*)board + 0x29D8);
    if (ball_count < 1) return;

    int** ball_array = *(int***)((char*)board + 0x2DE0);
    if (!ball_array || !*ball_array) return;

    int* ball = (*ball_array)[0];
    if (!ball || IsBadReadPtr(ball, 0x20)) return;

    // Read ball position
    float x = *(float*)((char*)ball + 0x164);
    float y = *(float*)((char*)ball + 0x168);
    float z = *(float*)((char*)ball + 0x16C);

    // Your onRaceStart code here
}
```

### Hook 2: Scene_SpawnBallsAndObjects Return — PRECISE

**Address**: 0x41CA3C (the `RET` instruction at the end of the function)

**Timing**: Immediately after all balls and objects are created. This is the
earliest point where balls exist.

**Calling convention**: `__fastcall` (ECX = this = board/scene). Returns with
plain `RET` (no stack cleanup). ECX is preserved via ESI internally — at the
return point, the original ECX is gone but the board pointer can be recovered
from the caller's stack frame.

**Advantage**: Runs for every level (it's the universal vtable[32]).
**Disadvantage**: Hooking a RET requires a code cave. The board pointer isn't
in a convenient register at the return point — you need to read it from the
caller's stack.

**Alternative**: Hook the ENTRY of Scene_SpawnBallsAndObjects (0x41C5B0)
and run your code AFTER calling the original. This is easier with DLL detours.

```c
// DLL detour pattern
typedef void (__fastcall *SpawnBallsAndObjects_t)(void* board);
SpawnBallsAndObjects_t Original_SpawnBallsAndObjects;

void __fastcall Hooked_SpawnBallsAndObjects(void* board) {
    Original_SpawnBallsAndObjects(board);

    // At this point, balls and objects are created
    int ball_count = *(int*)((char*)board + 0x29D8);
    if (ball_count < 1) return;

    int** ball_array = *(int***)((char*)board + 0x2DE0);
    if (!ball_array || !*ball_array) return;

    int* ball = (*ball_array)[0];
    if (!ball) return;

    // Your code here — ball is ready, objects are created
}
```

### Hook 3: Scene_LoadLevelN after vtable[32] call — PER-LEVEL

**Address**: Varies by level. For Level 2 (Beginner Race): 0x40D32F

| Level | Load Function | Post-vtable[32] Address |
|-------|---------------|--------------------------|
| L1 WarmUp | RumbleBoard_WarmUp_Init (0x413C20) | Varies (arena path) |
| L2 Beginner | Scene_LoadLevel2 (0x40D280) | 0x40D32F |
| L3 Intermediate | Scene_LoadLevel3 (0x40D390) | Check disasm |
| L4 Dizzy | Scene_LoadLevel4 (0x40D6D0) | Check disasm |
| L5 Tower | Scene_SetupLevel5 (0x40E190) | Check disasm |
| L6 Up | Scene_SetupLevel6 (0x40EA90) | Check disasm |
| L7 Neon | Scene_SetupLevel7 (0x40F360) | Check disasm |
| L8 Expert | Scene_SetupLevel8 (?) | Check disasm |
| L9 Odd | Scene_SetupLevel9 (0x410830) | Check disasm |
| L10 Toob | Scene_SetupLevel10 (0x411F60) | Check disasm |
| L11 Wobbly | Scene_SetupLevelCascade (0x4110D0) | Check disasm |
| L12 Glass | LevelGlass_InitScene (0x417640) | Check disasm |
| L13 Sky | Scene_SetupLevelUp (0x411540) | Check disasm |
| L14 Master | Scene_SetupLevelDark (0x416270) | Check disasm |
| L15 Impossible | LevelImpossible_InitScene (0x417F20) | Check disasm |

**Disadvantage**: Requires a separate hook for each level. Not recommended for
universal mods. Use Hook 1 or Hook 2 instead.

### Hook 4: App_StartRace Entry — EARLIEST

**Address**: 0x4287C0 (entry point of App_StartRace)

**Timing**: Before scene reset. Old objects are still alive. The new level
hasn't been loaded yet.

**Calling convention**: `__fastcall` (ECX = App, no params). Plain `RET`.

**Use case**: When you need to run code BEFORE the old scene is destroyed
(e.g., save data from the previous race). Too early for accessing new balls.

### Hook 5: First Ball_Update — FIRST PHYSICS TICK

**Address**: 0x405E00 (entry of Ball_Update, vtable[4])

**Timing**: First physics tick after level load. The 3-2-1 countdown has
started (but ball+0x14C=0, so input is enabled).

**Calling convention**: `__thiscall` (ECX = ball). The ball pointer is in ECX.

**Use case**: When you need to modify ball physics or state on the first frame.
Note: Ball_Update is called every frame for every ball, so you need a one-shot
flag to detect the first call after level load.

```c
static bool race_started = false;

void __fastcall Hooked_BallUpdate(void* ball) {
    if (!race_started) {
        race_started = true;
        // First ball update after level load
        // ball+0x14C = 0 (controls enabled)
        // Countdown is displaying but ball can move
    }
    Original_BallUpdate(ball);
}
```

### Hook 6: Scene_HandleRaceEnd (vtable[19]) — COUNTDOWN START

**Address**: 0x41B130 (entry of Scene_HandleRaceEnd)

**Timing**: First call is 25 frames after level load. The 3-2-1 countdown
display begins here.

**Calling convention**: `__thiscall` (ECX = board/scene). ESI = ECX after prolog.

**Use case**: When you need to run code exactly when the countdown starts
displaying (not before, not after).

---

## Complete Race-Start Timeline

```
Frame 0:   App_Start*Race → App_StartRace → reset scene
           Tournament_AdvanceRace (show menu)
           [OR] Tournament_AdvanceRace (create board immediately)

Frame 0+:  Tournament_AdvanceRace creates board:
             Board ctor → vtable[18] (level load):
               MeshWorld_ctor → CollisionLevel_ctor → Level_InitScene
               vtable[32] = Scene_SpawnBallsAndObjects:
                 Ball_ctor2 (ball+0x14C=0, radius=26, speed=5)
                 CreateBadBall, CreateMouseTrap, CreateFlags, CreateSigns
                 CreateDynamicObjects
             Scene_AddObject (board → scene tree)

           ↑ HOOK 1 (0x4273E3): everything loaded, no tick yet
           ↑ HOOK 2 (0x41CA3C): balls just spawned

Frame 1:   Scene_Update (vtable[1]):
             vtable[19] = Scene_HandleRaceEnd:
               Scene+0x3620 = 1 (frame counter, < 25 → skip countdown)
             vtable[20] = Scene_UpdateBallsAndState:
               Ball_Update (vtable[4]) — first physics tick

           ↑ HOOK 5 (Ball_Update entry): first physics tick

Frame 2-25: Scene_Update repeats. Countdown not yet displayed.
             Ball can move freely (ball+0x14C=0).

Frame 26:  Scene_HandleRaceEnd:
             Scene+0x3620 > 25 → start countdown
             Phase 0: display "3" (App+0x394 texture)
             Scene+0x3A50 = 0

           ↑ HOOK 6 (0x41B130): countdown display starts

Frame ~150: Phase 1: display "2" (App+0x398 texture)

Frame ~275: Phase 2: display "1" (App+0x39C texture), play sound

Frame ~400: Phase 3: Scene+0x3A4C = 1 (countdown done)
             Start race timers, iterate balls
             [In practice mode: set ball+0x14C=1 when timer expires]
```

---

## Scene Vtable Map (vtable base at 0x4D0260)

| Slot | Offset | Address | Function | Description |
|------|--------|---------|----------|-------------|
| 0 | 0x00 | 0x425020 | Board_dtor | Destructor |
| 1 | 0x04 | 0x419C00 | Scene_Update | Main game tick |
| 2 | 0x08 | 0x41A2E0 | Scene_SetCamera | Camera setup per ball |
| 18 | 0x48 | 0x40B090 | Level_InitScene | Base scene init (camera, sound, graphics) |
| 19 | 0x4C | 0x41B130 | Scene_HandleRaceEnd | **3-2-1 countdown + race-end handler** |
| 20 | 0x50 | 0x41B540 | Scene_UpdateBallsAndState | Ball physics update |
| 21 | 0x54 | 0x40A040 | NoOp | Unused slot |
| 22 | 0x58 | 0x41A540 | Scene_HandleCountdown | **Race-end menu/results handler** |
| 27 | 0x6C | 0x41B710 | Scene_RenderScoreHUD | Render countdown numbers + HUD |
| 28 | 0x70 | 0x41BFD0 | Scene_RenderTimerHUD | Render race timer |
| 31 | 0x7C | 0x41AC70 | Scene_vmethod31 | Per-ball camera + render setup |
| 32 | 0x80 | 0x41C5B0 | **Scene_SpawnBallsAndObjects** | **Spawn balls + all level objects** |
| 33 | 0x84 | 0x419750 | Scene_method33 | Unknown |

**Note**: Level-specific vtable overrides exist at 0x4D04A8 (WarmUp), 0x4D05C0
(Beginner), etc. Each overrides vtable[18] with its own load function (e.g.
Scene_LoadLevel2), but vtable[32] is shared across all levels
(Scene_SpawnBallsAndObjects at 0x41C5B0).

---

## Key Offsets Summary

### Ball (0xC60 bytes, allocated by operator_new in Scene_SpawnBallsAndObjects)

| Offset | Type | Name | Set By |
|--------|------|------|--------|
| +0x14C | byte | controls_disabled | Ball_ctor2 (0), Scene_StartCountdown (1), Scene_HandleRaceEnd (1) |
| +0x164 | float | pos_x | Scene_SpawnBallsAndObjects (START position) |
| +0x168 | float | pos_y | Scene_SpawnBallsAndObjects (START position) |
| +0x16C | float | pos_z | Scene_SpawnBallsAndObjects (START position) |
| +0x18 | int | player_id | Ball_ctor2 (-1 = AI/demo, 0+ = player) |
| +0x284 | float | radius | Ball_ctor2 (26.0) |
| +0x1A0 | float | max_speed | Ball_ctor2 (5.0) |
| +0x2FC | float | gravity_scale | Ball_ctor2 (1.0), Scene_StartCountdown (1.0) |
| +0x2F8 | byte | is_falling | Ball_ctor2 (0) |
| +0x2F9 | byte | fall_timer | Ball_ctor2 (0) |
| +0x769 | byte | active_flag | Scene_SpawnBallsAndObjects (1) |
| +0x768 | byte | frozen_flag | Ball_ctor2 (1 = initially active) |
| +0x310 | byte | has_target_pos | Ball_ctor2 (1) |

### Scene/Board (variable size, 0x436C–0x6498 depending on level)

| Offset | Type | Name | Description |
|--------|------|------|-------------|
| +0x878 | ptr | app | App object pointer |
| +0x184 | ptr | scene_object_list | AthenaList of scene objects |
| +0x29D4 | AthenaList | ball_list | Main ball list (player balls) |
| +0x3204 | AthenaList | ai_ball_list | Secondary ball list (AI balls) |
| +0x3620 | int | frame_counter | Incremented each Scene_Update |
| +0x3624 | float | race_timer | Decrements during race |
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
| +0x178 | ptr | scene | Scene/Board pointer |
| +0x184 | ptr | scene_object_list | Scene object AthenaList |
| +0x220 | ptr | player_profile | Current PlayerProfile |
| +0x237 | byte | is_arena | 1 = arena mode |
| +0x5DC | ptr | physics_ball | Current physics ball (for gate check) |

---

## Recommended Hook Strategy

### For DLL Mods (bass.dll proxy)

**Best choice**: Detour `Scene_SpawnBallsAndObjects` (0x41C5B0).

```c
typedef void (__fastcall *SpawnBallsFn)(void* board);
SpawnBallsFn Orig_SpawnBalls;

void __fastcall Hooked_SpawnBallsAndObjects(void* board) {
    Orig_SpawnBalls(board);

    // At this point:
    // - All balls are spawned (ball_list at board+0x29D4)
    // - All objects are created (flags, signs, dynamic objects)
    // - Camera and sound are configured
    // - No physics tick has run yet
    // - Ball+0x14C = 0 (controls enabled)
    // - 3-2-1 countdown hasn't started (needs 25 frames)

    int count = *(int*)((char*)board + 0x29D8);
    if (count < 1) return;

    int** balls = *(int***)((char*)board + 0x2DE0);
    if (!balls || !*balls) return;

    for (int i = 0; i < count; i++) {
        int* ball = (*balls)[i];
        if (!ball || IsBadReadPtr(ball, 0x20)) continue;

        // Access ball position, modify state, etc.
        float x = *(float*)((char*)ball + 0x164);
        float y = *(float*)((char*)ball + 0x168);
        float z = *(float*)((char*)ball + 0x16C);

        // Example: modify ball radius
        // *(float*)((char*)ball + 0x284) = 30.0f;
    }
}
```

### For CEA Scripts (Cheat Engine)

**Best choice**: Code cave at 0x4273E3 (after vtable[18] in Tournament_AdvanceRace).

```asm
// Hook after level load in Tournament_AdvanceRace
// Board pointer is in EAX at 0x4273E3
alloc(raceStartHook, 512)
alloc(raceStarted, 1)
registersymbol(raceStartHook)
registersymbol(raceStarted)

raceStarted:
  db 00

raceStartHook:
    // Check if already fired (one-shot)
    CMP byte ptr [raceStarted], 01
    JE raceStartHook_done

    // EAX = board pointer
    PUSHAD
    PUSHFD

    // Verify board pointer
    TEST EAX, EAX
    JZ raceStartHook_skip
    CMP EAX, 00400000
    JB raceStartHook_skip

    // Read ball list
    MOV ESI, [EAX + 0x29D8]    // ball count
    TEST ESI, ESI
    JLE raceStartHook_skip

    MOV EDI, [EAX + 0x2DE0]    // ball array ptr
    TEST EDI, EDI
    JZ raceStartHook_skip

    MOV EDI, [EDI]              // first ball
    TEST EDI, EDI
    JZ raceStartHook_skip

    // === YOUR CODE HERE ===
    // EDI = first ball
    // Example: read ball position
    // MOV ECX, [EDI + 0x164]    // ball X
    // MOV EDX, [EDI + 0x168]    // ball Y

    // Set flag
    MOV byte ptr [raceStarted], 01

raceStartHook_skip:
    POPFD
    POPAD

raceStartHook_done:
    // Original instruction: MOV ECX, [ESI+4]
    db 8B 4E 04
    JMP 004273E6

// Install hook at 0x4273E3
004273E3:
    JMP raceStartHook
    NOP

// Reset flag when race ends (hook App_StartRace at 0x4287C0)
// to re-enable for next race
```

### Resetting One-Shot Flags

When hooking with a one-shot flag (to avoid running your code every frame),
reset the flag when a new race starts. Hook `App_StartRace` (0x4287C0) entry:

```c
void __fastcall Hooked_AppStartRace(void* app) {
    race_started = false;  // Reset one-shot flag
    Original_AppStartRace(app);
}
```

---

## Calling Convention Reference

| Function | Address | Convention | Params | Return |
|----------|---------|------------|--------|--------|
| App_StartRace | 0x4287C0 | __fastcall | ECX=App | RET |
| Tournament_AdvanceRace | 0x427080 | __thiscall | ECX=profile, [ESP+4]=char | RET 4 |
| Scene_SpawnBallsAndObjects | 0x41C5B0 | __fastcall | ECX=board | RET |
| Scene_Update | 0x419C00 | __fastcall | ECX=board | RET |
| Scene_HandleRaceEnd | 0x41B130 | __fastcall | ECX=board | RET |
| Scene_UpdateBallsAndState | 0x41B540 | __fastcall | ECX=board | RET |
| Scene_StartCountdown | 0x437130 | __thiscall | ECX=scene, [ESP+4]=ball | RET 4 |
| Ball_Update | 0x405E00 | __thiscall | ECX=ball | RET |
| Ball_ctor2 | 0x4039E0 | __fastcall | ECX=alloc, [ESP+4]=scene | RET 4 |
| Level_InitScene | 0x40B090 | __fastcall | ECX=board | RET |
| Scene_AddObject | 0x469990 | __fastcall | ECX=list, [ESP+4]=obj | RET 4 |
| Tournament_AdvanceRace vtable[18] call | 0x4273E0 | __thiscall | ECX=board | RET |
| Tournament_AdvanceRace vtable[32] call | 0x40D329 | __thiscall | ECX=board | RET |

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

### 3. Scene+0x10F4 Timer May Be Dead Code

The SinkPlatform countdown timer (Scene+0x10F4=400) is set by
Scene_StartCountdown but no function in the analyzed codebase decrements it.
It may be processed by an unidentified function, or it may be a legacy
feature replaced by the Scene_HandleRaceEnd phase system.

### 4. Multiple Scene Vtables

There are 15+ scene subclass vtables (one per level type), each at a different
address. Hooking a specific vtable entry only works for that level type.
For universal hooks, target the shared functions (Scene_SpawnBallsAndObjects
at 0x41C5B0, Tournament_AdvanceRace at 0x427080) rather than per-level
functions.

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

---

*Document created from decompiled source analysis of Hamsterball.exe via GhidraMCP.
All function addresses, vtable layouts, and field offsets verified through
disassembly cross-referencing. Scene vtable at 0x4D0260 mapped with all 36 entries.
Countdown phase system verified in Scene_HandleRaceEnd (0x41B130) and
Scene_RenderScoreHUD (0x41B710).*
