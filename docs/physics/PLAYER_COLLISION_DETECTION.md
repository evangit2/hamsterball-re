# Player Collision Detection — Detecting When a Player Gets Bumped

## Overview

This document covers **every viable approach** for detecting when the player
ball collides with a badball (NPC 8-ball) or another player ball in Hamsterball.
Each approach is traced from Ghidra decompilation of the original binary,
with exact addresses, struct offsets, trade-offs, and working code patterns.

## Table of Contents

1. [How the Original Game Detects Ball-Ball Collisions](#1-how-the-original-game-detects-ball-ball-collisions)
2. [Ball Struct Reference](#2-ball-struct-reference)
3. [Collision Entry Struct Layout](#3-collision-entry-struct-layout)
4. [Vtable Layout (Player Ball vs 8-Ball)](#4-vtable-layout-player-ball-vs-8-ball)
5. [Approach 1: Code Cave Hook (Proven — Existing Mod)](#approach-1-code-cave-hook)
6. [Approach 2: Detour on Ball_Update Entry](#approach-2-detour-on-ball_update-entry)
7. [Approach 3: Vtable Patching (Swap vtable[4])](#approach-3-vtable-patching)
8. [Approach 4: Background Polling Thread](#approach-4-background-polling-thread)
9. [Approach 5: Hook the Impulse Function (vtable[6])](#approach-5-hook-the-impulse-function)
10. [Approach 6: Hook Collision Event Dispatchers](#approach-6-hook-collision-event-dispatchers)
11. [Approach 7: Custom Distance Check (No Hooks)](#approach-7-custom-distance-check)
12. [Approach 8: Reimplementation](#approach-8-reimplementation)
13. [Decision Matrix: Which Approach to Use](#decision-matrix)
14. [Pitfalls and Lessons Learned](#pitfalls-and-lessons-learned)

---

## 1. How the Original Game Detects Ball-Ball Collisions

### Call Chain

```
Scene_UpdateBallsAndState (0x41B540)
  │  Iterates Scene+0x29D4 (Ball AthenaList)
  │  For each ball: calls vtable[4] (Ball_Update for player, AI tick for 8-ball)
  │
  └─ Ball_Update (0x405E00) — player ball per-frame physics tick
       │  23-phase physics pipeline
       │
       ├─ Phase: Build collision tree
       │    CollisionMesh vtable[6] (0x456140) → builds collision result list
       │    at PhysicsBody+0x18 (AthenaList of collision entries)
       │
       ├─ Phase: Iterate collision results (0x406B77–0x407178)
       │    Loop: walk collision entry array at PhysicsBody+0x424
       │    Each entry (EBP/piVar16):
       │      [EBP+0x00] == 2 → wall collision (type 2)
       │      [EBP+0x00] == 1 → ball-ball collision (type 1) ← TARGET
       │
       └─ For type==1 (ball-ball):
            0x406C90: MOV EDI, [EBP+0x0C]     ; EDI = OTHER BALL pointer
            0x406C93: FLD [ESI+0x16C]         ; this ball Z
            0x406C99: FLD [ESI+0x168]         ; this ball Y
            0x406CA5: FLD [ESI+0x164]         ; this ball X
            0x406CB1: FLD [EDI+0x164]         ; other ball X
            ... direction = (this - other) × 0.025, normalize ...
            0x406DA6: CALL [EDX+0x18]         ; ApplyForce to OTHER ball (push away)
            0x406DBD: CALL [EDX+0x18]         ; ApplyForce to THIS ball (push away)
            0x406FD1: FLD [EDI+0x284]         ; speed comparison begins
            0x406FF7: CMP [ESI+0x18], -1     ; is THIS ball a player?
            0x407009: CALL Difficulty_GetTimeModifier(scene, 500.0)
            ... add 500 to player score ...
            0x407094: FLD [EDI+0x284]         ; reverse speed comparison
            0x4070BA: CMP [EDI+0x18], -1      ; is OTHER ball a player?
            ... mirror scoring for reverse case ...
```

### Key Insight: Symmetric Double-Fire

`Ball_Update` runs for **both balls** in a collision pair. When Player 1 bumps
an 8-ball, the collision is detected from both perspectives:
- During Player 1's `Ball_Update`: ESI=Player1, EDI=8ball
- During 8-ball's `Ball_Update`: ESI=8ball, EDI=Player1

This means any hook at the collision point fires **twice per collision event**.
You must deduplicate — either by pointer comparison (`ESI < EDI` to count once)
or by tracking collision IDs (`[EBP+0x64]`).

---

## 2. Ball Struct Reference

### Key Offsets for Collision Detection

| Offset | Type | Field | Description |
|--------|------|-------|-------------|
| `+0x10` | `void*` | scene_ptr (sound) | Used for Sound_Play3D (Scene+0x43C) |
| `+0x14` | `void*` | scene_ptr (collision) | Used for Mesh_FindClosestCollision (Scene+0x8B0) |
| `+0x18` | `int32` | player_index | -1 = NPC/badball, 0-3 = Player 1-4 |
| `+0x164` | `float` | pos_x | Current world position X |
| `+0x168` | `float` | pos_y | Current world position Y |
| `+0x16C` | `float` | pos_z | Current world position Z |
| `+0x170` | `float` | vel_x | Velocity accumulator X (force) |
| `+0x174` | `float` | vel_y | Velocity accumulator Y (force) |
| `+0x178` | `float` | vel_z | Velocity accumulator Z (force) |
| `+0x1A4` | `void*` | physics_body | CollisionMesh/PhysicsBody pointer |
| `+0x284` | `float` | radius | Collision radius (default ~27.0) |
| `+0x2E8` | `byte` | respawn_flag | Set during respawn handling |
| `+0x2E9` | `byte` | dizzy_lock | Sticky flag preventing Ball_ApplyTrajectory re-firing (E:LIMIT collision, speed>1.0) — NEVER use for ground check |
| `+0x2EC` | `int32` | bounce_count | Dizzy system bounce counter (double-incremented 0→1→2 when collision speed exceeds 0.03 and 0.1; triggers Ball_ApplyTrajectory when >1 AND dizzy_lock==0) |
| `+0x2F0` | `int32` | force_count | Number of forces applied this frame |
| `+0x2F4` | `int32` | best_streak | Per-ball best streak (int32) |
| `+0x2F9` | `byte` | frozen | Ball is frozen/stuck |
| `+0x2FC` | `int32` | alpha | Countdown when falling |
| `+0x324` | `byte` | in_tube | Skip collision processing when set |
| `+0x748` | `int32` | gravity_axis | 0=Y, 1=X, 2=Z (gravity plane) |
| `+0x808` | `int32` | teleport_active | Non-zero = teleport in progress |
| `+0x810` | `AthenaList` | path_list | Ball path tracking |
| `+0xC28` | `char*` | popup_text | Score popup string (freed each frame) |
| `+0xC2C` | `char[]` | section_filter | Current collision section name |

### Scene Offsets for Ball Access

| Offset | Type | Field | Description |
|--------|------|-------|-------------|
| `Scene+0x178` | `void*` | — | (App→Scene pointer is at App+0x178) |
| `Scene+0x29D4` | `AthenaList` | balls | Player balls list |
| `Scene+0x29D8` | `int32` | ball_count | Number of player balls |
| `Scene+0x2DE0` | `void**` | ball_array | Array of ball pointers |
| `Scene+0x3204` | `AthenaList` | eight_balls | NPC 8-ball list |
| `Scene+0x3208` | `int32` | eight_ball_count | Number of 8-balls |
| `Scene+0x3610` | `void**` | eight_ball_array | Array of 8-ball pointers |
| `Scene+0x8B0` | `void*` | collision_level | CollisionLevel for raycasts |
| `Scene+0x43C` | `void*` | sound_manager | Sound system |

### Global Access

```c
void* app   = *(void**)0x005341E0;           // App global
void* scene = *(void**)((char*)app + 0x178); // App+0x178 = Scene
```

---

## 3. Collision Entry Struct Layout

Each collision entry is accessed via `EBP` (or `piVar16` in decompiled C).
The struct is allocated as 32 bytes (8 DWORDs) via `operator_new(0x20)`.

```
CollisionEntry {
    +0x00 (int32)  type           // 1 = ball-ball, 2 = wall, 5 = floor
    +0x04 (???)    padding
    +0x08 (???)    padding
    +0x0C (void*)  other_ball     // When type==1: pointer to the other ball
    +0x10–0x1C     ???            // Unknown fields
    +0x20 (float)  normal_x       // Surface normal X (collision response)
    +0x24 (float)  normal_y       // Surface normal Y
    +0x28 (float)  normal_z       // Surface normal Z
    +0x2C (float)  collision_pt   // Collision point or distance
    +0x30 (float)  normal2_x      // Secondary normal components
    +0x34 (float)  normal2_y
    +0x38 (float)  normal2_z
    +0x3C–0x60     ???            // Additional collision data
    +0x64 (int32)  collision_id   // Dedup token (compared against EBP register)
}
```

### Decoded from Decompiled Code

```c
// From Ball_Update decompilation (lines 486–624):
while (piVar16 != NULL) {
    if (*piVar16 == 2) {                    // type == 2: wall collision
        if (piVar16[0x19] == unaff_EBP) {   // collision_id matches
            // Store ball position as "last wall contact" (ball+0x2DC)
        }
    }
    if (*piVar16 == 1) {                   // type == 1: ball-ball collision
        if (piVar16[0x19] == unaff_EBP) {  // collision_id matches
            // Apply trajectory boost (catapult-like effect)
            // Check gravity plane and call vtable[8] (bounce)
        }
        if (*piVar16 == 1) {               // Still type 1 (redundant check)
            piVar2 = (int*)piVar16[3];     // other_ball = entry[3] = +0x0C

            // Calculate direction between balls
            dir_y = (ball[0x5A] - other[0x5A]) * 0.025;  // × position_scale
            dir_x = (ball[0x59] - other[0x59]) * 0.025;
            dir_z = (ball[0x5B] - other[0x5B]) * 0.025;

            // Normalize direction, clamp to minimum 3.0
            magnitude = sqrt(dir_x² + dir_y² + dir_z²);
            if (magnitude > 0 && magnitude < 3.0)
                scale = 3.0 / magnitude;
            else
                scale = 1.0;

            // Apply impulse to OTHER ball (push away)
            (*other->vtable[6])(other, -dir_x*scale, -dir_z*scale, -dir_y*scale, 1.0);

            // Apply impulse to THIS ball (push away)
            (*this->vtable[6])(this, dir_x*scale, dir_z*scale, dir_y*scale, dir_x*scale);

            // Sound effect (3D positioned)
            Sound_Play3D(scene->sound_mgr, ball_x, ball_y, ball_z);

            // Scoring: if THIS ball is faster, OTHER ball gets bumped
            if (other->radius < this->radius * 0.7) {
                (*other->vtable[8])(other);  // bounce callback
                if (this->player_index != -1) {
                    // Add 500 score × difficulty modifier
                    score = Difficulty_GetTimeModifier(scene, 500.0);
                    app_score[player_index] += score;
                    this->popup_text = format("+%d", score);
                    this->freeze_counter = 200;
                }
            }

            // Scoring: reverse — if OTHER ball is faster, THIS ball gets bumped
            if (this->radius < other->radius * 0.7) {
                (*this->vtable[8])(this);   // bounce callback
                if (other->player_index != -1) {
                    score = Difficulty_GetTimeModifier(scene, 500.0);
                    app_score[other->player_index] += score;
                    other->popup_text = format("+%d", score);
                    other->freeze_counter = 200;
                }
            }
        }
    }
    // Advance to next collision entry
    ...
}
```

### Collision Constants

| Address | Value | Description |
|---------|-------|-------------|
| `0x4CF520` | `0.025` | Position scale factor for direction calculation |
| `0x4CF508` | `0.7` (double) | Speed comparison multiplier (radius ratio threshold) |
| `0x4CF418` | `3.0` | Minimum collision direction magnitude |
| `0x4CF518` | `-0.5` (double) | Direction dot-product threshold for sound vs silent hit |
| `0x4CF510` | `0.04` | Surface angle threshold |
| `0x4CF48C` | `2.0` | Surface speed threshold (for sound playback) |
| `0x4CF3E0` | `0.5` (double) | Speed factor |
| `0x4CF3C8` | `1.0` (double) | Comparison threshold |
| `0x4CF308` | `0.1` (double) | Impact multiplier |
| `0x4CF380` | `0.25` | First-frame force damping |
| `0x4CF378` | `0.0` | In-tube force damping (zeroes all force) |
| `0x4CF374` | `0.2` | On-ice force damping |
| `0x4CF36C` | `0.75` | Dizzy force damping |

---

## 4. Vtable Layout (Player Ball vs 8-Ball)

### Player Ball Vtable at `0x4CF314`

| Index | Offset | Address | Function |
|-------|--------|---------|----------|
| 0 | +0x00 | `0x402A50` | Destructor |
| 1 | +0x04 | `0x4015B0` | One-time init (SetupCollisionRender) |
| 2 | +0x08 | `0x403DC0` | Unknown |
| 3 | +0x0C | `0x402A70` | Unknown |
| 4 | +0x10 | `0x405E00` | **Ball_Update** (per-frame physics tick) |
| 5 | +0x14 | `0x401590` | Unknown |
| 6 | +0x18 | `0x4016F0` | **Ball_ApplyForceV2** (gravity-plane-aware force) |
| 7 | +0x1C | `0x402C10` | Unknown |
| 8 | +0x20 | `0x409050` | **Bounce callback** (called on significant hit) |

### 8-Ball Vtable at `0x4CF3A0`

| Index | Offset | Address | Function |
|-------|--------|---------|----------|
| 0 | +0x00 | `0x4027F0` | Destructor |
| 1 | +0x04 | `0x405100` | One-time init (InitPhysicsDefaults) |
| 2 | +0x08 | `0x402DE0` | Unknown |
| 3 | +0x0C | `0x402A70` | Unknown (shared with player) |
| 4 | +0x10 | `0x408390` | **AI tick** (per-frame, calls Ball_Update internally) |
| 5 | +0x14 | `0x401590` | Unknown (shared with player) |
| 6 | +0x18 | `0x402650` | **Ball_ApplyForceWithMultipliers** (simpler force) |
| 7 | +0x1C | `0x402C10` | Unknown (shared with player) |
| 8 | +0x20 | `0x409480` | **Bounce callback** (8-ball version) |

### Important Notes

- **vtable[4]** is the per-frame update. For player balls it's `Ball_Update`
  (0x405E00). For 8-balls it's the AI tick (0x408390), which internally calls
  `Ball_Update` (at 0x4083BD: `CALL 0x405E00`). Both run the collision loop.

- **vtable[6]** is the impulse/force function. Both player and 8-ball versions
  accumulate velocity at `+0x170/+0x174/+0x178` and apply multipliers based on
  ball state (frozen, dizzy, in-tube, on-ice). The player version
  (`Ball_ApplyForceV2`, 0x4016F0) has gravity-plane-awareness for facing angle
  computation. The 8-ball version (`Ball_ApplyForceWithMultipliers`, 0x402650)
  uses different scale constants but the same core logic.

- **vtable[8]** is the bounce callback, called when a collision is "significant"
  (the other ball's radius × 0.7 is smaller than this ball's radius). This is
  the "you got bumped hard" indicator.

---

## Approach 1: Code Cave Hook

**Status: PROVEN — Working mod exists (`mods/8ball_hit_detect/`)**

### Concept

Patch a JMP instruction at a strategic point inside `Ball_Update`'s collision
loop to redirect execution to a hand-assembled code cave. The cave checks ball
types, records the hit, then executes the original instruction and jumps back.

### Hook Point: `0x406FD1`

```
Original instruction: FLD DWORD [EDI+0x284]  (6 bytes: D9 87 84 02 00 00)
```

At this point in the code:
- `ESI` = this ball (the one running Ball_Update)
- `EDI` = other ball (collision partner, loaded at 0x406C90)
- Both balls are confirmed colliding (type==1 check passed)
- Impulse forces have already been applied (0x406DA6, 0x406DBD)
- Speed comparison is about to begin

This is the **scoring section** — the game is about to check which ball was
moving faster and award score. Perfect for detection.

### Code Cave Logic

```asm
; Entry: ESI = this ball, EDI = other ball
PUSHAD

MOV EAX, [ESI+0x18]      ; this ball's player_index
MOV EBX, [EDI+0x18]      ; other ball's player_index

; Check: is this a player→8-ball collision?
CMP EAX, 0xFFFFFFFF       ; is this ball an 8-ball?
JE  .check_case2          ; yes → check if other is player
CMP EBX, 0xFFFFFFFF       ; is other ball an 8-ball?
JNE .done                 ; both are players → skip (use different handler)
JMP .hit_detected         ; this=player, other=8-ball → HIT

.check_case2:
CMP EBX, 0xFFFFFFFF       ; is other ball also 8-ball?
JE  .done                 ; both 8-balls → skip
; this=8-ball, other=player → HIT (player got bumped)

.hit_detected:
INC DWORD [g_hit_count]   ; increment counter
; Store player_index for polling thread
CMP EAX, 0xFFFFFFFF
JNE .esi_is_player
MOV ECX, EBX              ; EDI is the player
JMP .set_flag
.esi_is_player:
MOV ECX, EAX             ; ESI is the player
.set_flag:
INC ECX                   ; convert to 1-based (0 = no pending)
MOV DWORD [g_hit_pending], ECX

.done:
POPAD
; Execute original instruction
FLD DWORD [EDI+0x284]
; Jump back to 0x406FD1 + 6
JMP 0x00406FD7
```

### Implementation Pattern

```c
// Code cave is assembled at runtime into VirtualAlloc'd memory.
// The hook site is patched with: JMP <cave> + NOP
// A background thread polls g_hit_pending and writes to hitlog.txt.
//
// CRITICAL: Never call C functions from the code cave itself.
// FPU/stack corruption crashes the game. Use the volatile flag + polling
// thread pattern (see Pitfalls section).

static volatile DWORD g_hit_count = 0;
static volatile DWORD g_hit_pending = 0;  // 0 = none, 1-4 = player index + 1
```

### Alternative Hook Points

| Address | Instruction | Context | Best For |
|---------|-------------|---------|----------|
| `0x406FD1` | `FLD [EDI+0x284]` | After impulse, before scoring | Hit detection + scoring |
| `0x406C90` | `MOV EDI, [EBP+0x0C]` | Other ball just identified | Modify impulse before it applies |
| `0x406BD3` | `CMP [EBP], 0x1` | Type check (earliest) | Filter collision pairs before physics |
| `0x406DA6` | `CALL [EDX+0x18]` | First impulse call (to other ball) | Intercept/modify force direction |
| `0x406DBD` | `CALL [EDX+0x18]` | Second impulse call (to this ball) | Intercept/modify force direction |

### Pros

- Direct access to both ball pointers at collision time
- Minimal overhead (a few comparisons + one memory write)
- Can read/modify any ball field (position, velocity, radius, player_index)
- Can modify the collision response (knockback, score, sound)
- Proven working in production mod

### Cons

- Requires hand-assembled x86 machine code (error-prone)
- Must not call C functions from the cave (FPU/stack corruption)
- Symmetric double-fire: fires for both balls in a pair (need dedup)
- Hook site is hardcoded to specific game version (byte signature check required)
- Cannot easily add complex logic (use volatile flag + polling thread instead)

### Working Reference

- **Source**: `mods/8ball_hit_detect/8ball_hit_detect.c`
- **Build**: `i686-w64-mingw32-gcc -shared -o bass.dll 8ball_hit_detect.c -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc -Wl,--add-stdcall-alias`
- **Install**: Rename original `bass.dll` → `bass_real.dll`, copy proxy `bass.dll`

---

## Approach 2: Detour on Ball_Update Entry

**Status: Theoretical (pattern proven in `tools/collision_hook/`)**

### Concept

Hook the **entry point** of `Ball_Update` (0x405E00) with a standard 5-byte JMP
detour. Your C function runs before the physics tick, giving you a chance to
inspect ball state. After your function returns, the original `Ball_Update`
runs normally via a trampoline.

### Implementation

```c
typedef void (__fastcall *BallUpdate_t)(void *ball);
static BallUpdate_t g_orig_Ball_Update = NULL;
static unsigned char g_trampoline[16];

// Previous frame positions for delta detection
static float g_prev_x[4] = {0};  // per player
static float g_prev_y[4] = {0};
static float g_prev_z[4] = {0};

void __fastcall hook_Ball_Update(void *ball, void *edx_dummy) {
    // Read ball state BEFORE update
    int player_idx = *(int*)((char*)ball + 0x18);
    float x = *(float*)((char*)ball + 0x164);
    float y = *(float*)((char*)ball + 0x168);
    float z = *(float*)((char*)ball + 0x16C);

    if (player_idx >= 0 && player_idx < 4) {
        // After update, the collision list at ball+0x1A4 will have results
        // We can check them in a POST-update hook
    }

    // Call original
    g_orig_Ball_Update(ball, NULL);

    // POST-update: check collision results
    if (player_idx >= 0) {
        void *physics = *(void**)((char*)ball + 0x1A4);
        if (physics) {
            // Read collision list at physics+0x18
            int count = *(int*)((char*)physics + 0x1C);
            void **entries = *(void**)((char*)physics + 0x424);
            for (int i = 0; i < count && entries; i++) {
                int *entry = (int*)entries[i];
                if (entry && entry[0] == 1) {  // type == 1 (ball-ball)
                    void *other = (void*)entry[3];  // other ball
                    if (other) {
                        int other_idx = *(int*)((char*)other + 0x18);
                        printf("Player %d hit by ball %d (idx=%d)\n",
                               player_idx, other_idx != -1 ? other_idx+1 : -1);
                    }
                }
            }
        }
    }
}
```

### Hook Installation (5-byte JMP detour)

```c
static int install_detour(void *target, void *hook, unsigned char *trampoline) {
    DWORD oldProtect;
    unsigned char *t = (unsigned char *)target;

    VirtualProtect(t, 16, PAGE_EXECUTE_READWRITE, &oldProtect);

    // Copy original bytes to trampoline
    memcpy(trampoline, t, 5);  // copy first 5 bytes
    // Append JMP back to target+5
    trampoline[5] = 0xE9;
    *(unsigned long*)(trampoline + 6) =
        (unsigned long)((char*)target + 5 - (char*)(trampoline + 5) - 5);

    // Make trampoline executable
    DWORD tp;
    VirtualProtect(trampoline, 16, PAGE_EXECUTE_READWRITE, &tp);

    // Overwrite target: JMP rel32
    unsigned long rel = (unsigned long)((char*)hook - (char*)target - 5);
    t[0] = 0xE9;
    *(unsigned long*)(t + 1) = rel;

    FlushInstructionCache(GetCurrentProcess(), target, 5);
    return 1;
}
```

### Pros

- Can inspect state both BEFORE and AFTER the physics tick
- Written in C (no hand-assembled machine code)
- Can call C library functions (printf, file I/O) safely
- Can access the full collision result list after Ball_Update processes it
- Clean uninstall (restore original bytes)

### Cons

- The collision list at `physics+0x18` may be in an inconsistent state
  post-update (some entries may have been processed and cleared)
- The AthenaList iteration uses an internal index at `physics+0x408` that
  wraps at 256 — reading it externally may miss entries or get stale data
- Trampoline requires at least 5 bytes of safe overwrite space at the
  function entry (must verify no relative jumps in first 5 bytes)
- Post-update hook fires for ALL balls, not just collision pairs — you must
  scan the collision list yourself to find ball-ball hits
- Thread safety: the hook runs on the game thread, so blocking operations
  (file I/O, sleeps) will stutter the game

### Working Reference

- **Pattern**: `tools/collision_hook/collision_hook.c` (hooks event dispatchers
  using the same detour technique, but on `DispatchCollisionEvents`/`TowerCollisionEvents`/
  `ExpertCollisionEvents` instead of `Ball_Update`)

---

## Approach 3: Vtable Patching

**Status: Theoretical (pattern proven in ball-ai-clone-system)**

### Concept

The Ball vtable is stored in `.rdata` at a fixed address. By patching the
vtable entry for `vtable[4]` (the per-frame update), you can redirect all
ball updates to your custom function. Your function can wrap the original
and add collision detection logic.

### Vtable Addresses

```
Player Ball vtable: 0x4CF314  (vtable[4] at 0x4CF324 = 0x405E00)
8-Ball vtable:      0x4CF3A0  (vtable[4] at 0x4CF3B0 = 0x408390)
```

### Implementation

```c
// Save original vtable[4] values
static DWORD g_orig_player_v4 = 0;
static DWORD g_orig_8ball_v4 = 0;

// Custom update wrapper
void __fastcall custom_ball_update(void *ball, void *edx_dummy) {
    // Pre-update: save state
    int player_idx = *(int*)((char*)ball + 0x18);
    float prev_vel[3] = {
        *(float*)((char*)ball + 0x170),
        *(float*)((char*)ball + 0x174),
        *(float*)((char*)ball + 0x178)
    };

    // Call original update
    // NOTE: Must use the original function pointer, not vtable lookup
    // (because we patched the vtable)
    ((void (__fastcall *)(void*))g_orig_player_v4)(ball);

    // Post-update: check for velocity changes (collision signature)
    float new_vel[3] = {
        *(float*)((char*)ball + 0x170),
        *(float*)((char*)ball + 0x174),
        *(float*)((char*)ball + 0x178)
    };

    // Large velocity delta = collision impulse was applied
    float dx = new_vel[0] - prev_vel[0];
    float dy = new_vel[1] - prev_vel[1];
    float dz = new_vel[2] - prev_vel[2];
    float delta = sqrtf(dx*dx + dy*dy + dz*dz);

    if (delta > 5.0f && player_idx >= 0) {
        // Player was bumped!
        // Scan collision list for details
        void *physics = *(void**)((char*)ball + 0x1A4);
        if (physics) {
            int count = *(int*)((char*)physics + 0x1C);
            // ... iterate collision entries ...
        }
    }
}

void install_vtable_patch(void) {
    DWORD oldProtect;

    // Patch player ball vtable[4]
    DWORD *player_vt = (DWORD*)0x4CF324;  // vtable+0x10
    VirtualProtect(player_vt, 4, PAGE_READWRITE, &oldProtect);
    g_orig_player_v4 = *player_vt;
    *player_vt = (DWORD)custom_ball_update;
    VirtualProtect(player_vt, 4, oldProtect, &oldProtect);

    // Patch 8-ball vtable[4] (if needed)
    DWORD *ball8_vt = (DWORD*)0x4CF3B0;  // 8-ball vtable+0x10
    VirtualProtect(ball8_vt, 4, PAGE_READWRITE, &oldProtect);
    g_orig_8ball_v4 = *ball8_vt;
    *ball8_vt = (DWORD)custom_ball_update;  // or separate 8-ball handler
    VirtualProtect(ball8_vt, 4, oldProtect, &oldProtect);
}
```

### Pros

- Cleanest hook: no code modification, just data (vtable pointer swap)
- Can intercept ALL ball updates with a single patch
- Fully written in C
- Easy to uninstall (restore original vtable values)
- Can add pre/post processing around the original update
- Does not need to know the calling convention of the original function
  (it's always `__thiscall` via vtable dispatch)

### Cons

- The vtable is in `.rdata` — VirtualProtect is needed to make it writable
- If the game creates balls AFTER your patch, the new balls will use the
  patched vtable automatically (good) but you must ensure your wrapper
  handles all ball types correctly
- The wrapper must not call C library functions that disturb the FPU state
  (the original `Ball_Update` uses heavy x87 FPU operations)
- Velocity-delta detection is an indirect signal — it can't distinguish
  between a collision impulse and a catapult/trajectory boost
- The vtable may be copied per-ball-instance at construction time — need
  to verify whether all balls share the same vtable pointer or have copies
- **8-ball AI tick (0x408390) calls Ball_Update internally** — if you patch
  both vtables, the 8-ball's call to Ball_Update will go through your wrapper
  TWICE (once for the AI tick wrapper, once for the internal Ball_Update call).
  Must handle this with a re-entrancy guard.

### Working Reference

- **Pattern**: `ball-ai-clone-system` (skill `hamsterball-re` reference
  `ball-ai-clone-system.md`) — creates custom vtables for AI clones by
  copying the player vtable and replacing specific slots

---

## Approach 4: Background Polling Thread

**Status: Theoretical**

### Concept

Run a background thread that periodically reads the ball positions from the
Scene's ball lists and performs distance-based collision detection. This
requires no code hooks at all — just memory reads.

### Implementation

```c
static DWORD WINAPI collision_poll_thread(LPVOID param) {
    (void)param;
    Sleep(5000);  // Wait for game to fully load

    void *app = *(void**)0x005341E0;
    if (!app) return 1;

    void *scene = *(void**)((char*)app + 0x178);
    if (!scene) return 1;

    // Previous positions for all balls
    float prev_pos[8][3] = {0};
    int prev_count = 0;

    while (1) {
        // Read player ball list
        int p_count = *(int*)((char*)scene + 0x29D8);
        void **p_balls = *(void***)((char*)scene + 0x2DE0);

        // Read 8-ball list
        int b_count = *(int*)((char*)scene + 0x3208);
        void **b_balls = *(void***)((char*)scene + 0x3610);

        // Check all player vs 8-ball pairs
        for (int p = 0; p < p_count && p_balls; p++) {
            void *pball = p_balls[p];
            if (!pball || IsBadReadPtr(pball, 0x200)) continue;

            float px = *(float*)((char*)pball + 0x164);
            float py = *(float*)((char*)pball + 0x168);
            float pz = *(float*)((char*)pball + 0x16C);
            float pr = *(float*)((char*)pball + 0x284);

            for (int b = 0; b < b_count && b_balls; b++) {
                void *bball = b_balls[b];
                if (!bball || IsBadReadPtr(bball, 0x200)) continue;

                float bx = *(float*)((char*)bball + 0x164);
                float by = *(float*)((char*)bball + 0x168);
                float bz = *(float*)((char*)bball + 0x16C);
                float br = *(float*)((char*)bball + 0x284);

                float dx = px - bx, dy = py - by, dz = pz - bz;
                float dist = sqrtf(dx*dx + dy*dy + dz*dz);

                if (dist < pr + br) {
                    // Collision detected!
                    printf("Player %d hit 8-ball %d (dist=%.1f)\n",
                           p, b, dist);
                }
            }

            // Also check player vs player
            for (int p2 = p+1; p2 < p_count && p_balls; p2++) {
                void *pball2 = p_balls[p2];
                if (!pball2 || IsBadReadPtr(pball2, 0x200)) continue;

                float p2x = *(float*)((char*)pball2 + 0x164);
                float p2y = *(float*)((char*)pball2 + 0x168);
                float p2z = *(float*)((char*)pball2 + 0x16C);
                float p2r = *(float*)((char*)pball2 + 0x284);

                float dx = px - p2x, dy = py - p2y, dz = pz - p2z;
                float dist = sqrtf(dx*dx + dy*dy + dz*dz);

                if (dist < pr + p2r) {
                    printf("Player %d hit Player %d (dist=%.1f)\n",
                           p, p2, dist);
                }
            }
        }

        Sleep(16);  // ~60Hz polling
    }
    return 0;
}
```

### Pros

- **No code hooks whatsoever** — pure memory reads
- Cannot crash the game (read-only access)
- Can detect ALL collision types (player-ball, player-player, ball-ball)
- Works with any game version (only depends on struct offsets)
- Can run at any frequency (60Hz, 30Hz, etc.)
- Can read additional ball state (velocity, radius) for richer detection

### Cons

- **Race condition risk**: reading ball positions while the game thread is
  writing them can give inconsistent data (ball moved between X and Y reads)
- **No collision event data**: you only get proximity, not the actual
  collision normal, impulse direction, or speed comparison
- **No pre/post state**: you see the result after the physics tick, not the
  collision itself — you may miss fast collisions that resolve in one frame
- **Radius-based detection is approximate**: the game's actual collision uses
  sphere-vs-triangle intersection with the collision mesh, not simple
  sphere-vs-sphere distance
- **Thread safety**: `IsBadReadPtr` is technically deprecated and can cause
  issues; better to use SEH or VirtualQuery for safe reads
- **Memory overhead**: polling at 60Hz with multiple balls creates CPU load
- **False positives**: two balls being close doesn't mean they "collided" —
  they might be resting against each other. Need velocity or position-delta
  to distinguish a new collision from continuous contact

### Best Use Case

When you need a **quick, safe detection** without modifying game code at all.
Good for statistics gathering, overlay displays, or simple trigger systems.

---

## Approach 5: Hook the Impulse Function (vtable[6])

**Status: Theoretical**

### Concept

Instead of hooking inside `Ball_Update`, hook the **ApplyForce** function
(vtable[6]) that is called during collision response. Every time a collision
impulse is applied, your hook runs and can inspect the direction and magnitude.

### Addresses

| Ball Type | vtable[6] Address | Function Name |
|-----------|-------------------|---------------|
| Player | `0x4016F0` | Ball_ApplyForceV2 |
| 8-Ball | `0x402650` | Ball_ApplyForceWithMultipliers |

### Implementation

```c
// Hook signature: void __thiscall ApplyForce(void *this, float dx, float dy, float dz, float multiplier)
typedef void (__fastcall *ApplyForce_t)(void *ball, void *edx, float dx, float dy, float dz, float mult);
static ApplyForce_t g_orig_ApplyForce = NULL;
static unsigned char g_trampoline[16];

void __fastcall hook_ApplyForce(void *ball, void *edx_dummy,
                                 float dx, float dy, float dz, float mult) {
    // This is called for EVERY force application, not just collisions.
    // Need to filter for collision impulses.
    //
    // Collision impulses have multiplier == 1.0 and are called in pairs
    // (one for each ball in the collision). The direction is normalized.
    //
    // Input forces (from player control) have different multipliers
    // (0.25 for first frame, 0.0 in tube, etc.)

    if (mult == 1.0f) {
        // Likely a collision impulse
        int player_idx = *(int*)((char*)ball + 0x18);
        float magnitude = sqrtf(dx*dx + dy*dy + dz*dz);

        if (magnitude > 1.0f && player_idx >= 0) {
            printf("Player %d received impulse (%.1f, %.1f, %.1f) mag=%.1f\n",
                   player_idx, dx, dy, dz, magnitude);
        }
    }

    // Call original
    g_orig_ApplyForce(ball, NULL, dx, dy, dz, mult);
}
```

### Pros

- Called at the exact moment of impulse application
- Can **modify** the impulse direction/magnitude (custom knockback)
- Can cancel the impulse entirely (no-clip mode)
- Can distinguish between collision impulses (mult=1.0) and control forces
- Written in C with standard detour technique

### Cons

- **Very high call frequency**: `ApplyForce` is called for EVERY force, not
  just collisions — player input, catapult boosts, trajectory launches,
  gravity adjustments all go through this function
- **Hard to distinguish collision impulses**: the multiplier is 1.0 for
  collisions, but also for other forces. The direction is normalized for
  collisions but may be arbitrary for other forces
- **No other-ball context**: the function only receives `(this, dx, dy, dz, mult)`.
  You don't know WHICH ball caused the collision. To find the other ball,
  you'd need to scan the collision list separately
- **Asymmetric**: only the ball receiving the force calls this function. You
  can't tell if it was a player-player or player-8ball collision from the
  call alone
- **May be called from non-collision paths**: the 8-ball AI may call ApplyForce
  for movement, not just collisions

### Best Use Case

When you need to **modify collision response** (e.g., custom knockback, immunity
frames, or amplified bump force) rather than just detect it.

---

## Approach 6: Hook Collision Event Dispatchers

**Status: PROVEN — Working tool exists (`tools/collision_hook/`)**

### Concept

Hook the three collision dispatch functions that the game calls when the ball
hits named collision objects (walls, events, triggers). These are NOT for
ball-ball collisions, but for **level geometry** and **event trigger** collisions.

### Addresses

| Address | Function | Domain |
|---------|----------|--------|
| `0x40C5D0` | `DispatchCollisionEvents` | Shared base handler (all events) |
| `0x40DCD0` | `TowerCollisionEvents` | Race level events |
| `0x40E6A0` | `ExpertCollisionEvents` | Arena events |

### Important Distinction

These dispatchers handle **level object collisions** (walls, catapults, jump
pads, trapdoors, etc.), NOT ball-ball collisions. The event name string at
`collObj+0x864` identifies what was hit.

### When to Use

- Detect when a player hits a specific level object (e.g., "E:JUMP")
- Log all collision events for debugging
- Filter by event name (e.g., only "E:CATAPULTBOTTOM")

### When NOT to Use

- Ball-ball collision detection (these dispatchers are not called for that)
- Detecting "player got bumped by badball" (use Approach 1 or 2 instead)

### Working Reference

- **Source**: `tools/collision_hook/collision_hook.c`
- **Build**: `i686-w64-mingw32-gcc -shared -o collision_hook.dll collision_hook.c -Wl,--enable-stdcall-fixup`
- **Inject**: `tools/collision_hook/injector.exe`

---

## Approach 7: Custom Distance Check (No Hooks)

**Status: Theoretical (simplest possible approach)**

### Concept

In your existing per-frame render hook (slot 11), read all ball positions and
perform a simple sphere-vs-sphere distance check. This requires NO additional
hooks — just memory reads in your existing render callback.

### Implementation (in your mod API framework)

```cpp
void OnRender() {
    void* app = api->GetApp();
    if (!app) return;
    void* scene = *(void**)((char*)app + 0x178);
    if (!scene) return;

    int p_count = *(int*)((char*)scene + 0x29D8);
    void** p_balls = *(void***)((char*)scene + 0x2DE0);
    int b_count = *(int*)((char*)scene + 0x3208);
    void** b_balls = *(void***)((char*)scene + 0x3610);

    static float last_dist[4][16] = {0};  // [player][ball] previous distance
    static bool in_contact[4][16] = {false};

    for (int p = 0; p < p_count && p_balls; p++) {
        void* pball = p_balls[p];
        if (!pball) continue;

        float px = *(float*)((char*)pball + 0x164);
        float py = *(float*)((char*)pball + 0x168);
        float pz = *(float*)((char*)pball + 0x16C);
        float pr = *(float*)((char*)pball + 0x284);

        for (int b = 0; b < b_count && b_balls; b++) {
            void* bball = b_balls[b];
            if (!bball) continue;

            float bx = *(float*)((char*)bball + 0x164);
            float by = *(float*)((char*)bball + 0x168);
            float bz = *(float*)((char*)bball + 0x16C);
            float br = *(float*)((char*)bball + 0x284);

            float dx = px - bx, dy = py - by, dz = pz - bz;
            float dist = sqrtf(dx*dx + dy*dy + dz*dz);
            float threshold = pr + br;

            bool touching = (dist < threshold);

            // Only report NEW contacts (rising edge)
            if (touching && !in_contact[p][b]) {
                // Player p just bumped 8-ball b
                DrawTextOnScreen(font, "BUMP!", 300, 50, ...);
            }
            in_contact[p][b] = touching;
        }
    }
}
```

### Pros

- Simplest implementation — no hooks, no code caves, no vtable patching
- Runs in your existing render hook — no new threads
- Can draw visual feedback immediately (screen text, overlays)
- Safe: read-only memory access, no game state modification
- Can detect player-player and player-8ball collisions equally

### Cons

- **Rising-edge detection only**: you detect "just started touching", but you
  can't get the impulse direction, force magnitude, or collision normal
- **Radius is approximate**: the game's actual collision uses the collision mesh
  (triangles), not simple sphere-sphere. Two balls may appear to be touching
  by distance but not actually collide in the game's physics
- **Frame-rate dependent**: if two balls pass through each other in one frame
  (high speed), you may miss the collision entirely
- **No dedup with game's collision system**: the game may have already resolved
  the collision (separated the balls) by the time your render hook runs

### Best Use Case

Quick prototyping, visual feedback, or simple "bump counter" HUD overlays.

---

## Approach 8: Reimplementation

**Status: Partial — reimpl has basic ground collision but no ball-ball**

### Concept

In the Hamsterball reimplementation (`reimpl/src/physics/physics.c`), implement
ball-ball collision detection from scratch using sphere-sphere intersection.

### Current State

The reimpl currently has:
- A single `g_ball` struct (no multi-ball support)
- Basic ground collision via `get_ground_height()` heightmap lookup
- No ball-ball collision detection at all
- No 8-ball AI or NPC balls

### What Would Be Needed

```c
typedef struct {
    Vec3 position;
    Vec3 velocity;
    float radius;
    int player_index;  // -1 = NPC, 0+ = player
    bool active;
} Ball;

#define MAX_BALLS 32
static Ball balls[MAX_BALLS];
static int ball_count = 0;

void check_ball_ball_collisions(void) {
    for (int i = 0; i < ball_count; i++) {
        for (int j = i + 1; j < ball_count; j++) {
            Ball *a = &balls[i];
            Ball *b = &balls[j];
            if (!a->active || !b->active) continue;

            float dx = b->position.x - a->position.x;
            float dy = b->position.y - a->position.y;
            float dz = b->position.z - a->position.z;
            float dist = sqrtf(dx*dx + dy*dy + dz*dz);
            float min_dist = a->radius + b->radius;

            if (dist < min_dist && dist > 0.0001f) {
                // Collision detected!
                float scale = (min_dist - dist) / dist * 0.5f;

                // Separate balls
                a->position.x -= dx * scale;
                a->position.y -= dy * scale;
                a->position.z -= dz * scale;
                b->position.x += dx * scale;
                b->position.y += dy * scale;
                b->position.z += dz * scale;

                // Calculate impulse direction (matching original game's 0.025 scale)
                float dir_x = (a->position.x - b->position.x) * 0.025f;
                float dir_y = (a->position.y - b->position.y) * 0.025f;
                float dir_z = (a->position.z - b->position.z) * 0.025f;
                float mag = sqrtf(dir_x*dir_x + dir_y*dir_y + dir_z*dir_z);
                if (mag > 0 && mag < 3.0f) {
                    float s = 3.0f / mag;
                    dir_x *= s; dir_y *= s; dir_z *= s;
                }

                // Apply impulses
                a->velocity.x += dir_x;
                a->velocity.y += dir_y;
                a->velocity.z += dir_z;
                b->velocity.x -= dir_x;
                b->velocity.y -= dir_y;
                b->velocity.z -= dir_z;

                // Fire collision callback
                on_ball_collision(a, b);
            }
        }
    }
}
```

### Pros

- Full control over collision detection and response
- Can implement custom collision rules (e.g., team-based, power-up effects)
- No binary patching required
- Can add detailed collision events (direction, force, contact point)

### Cons

- Requires multi-ball support in the reimpl first (currently single-ball)
- Requires NPC 8-ball AI to exist in the reimpl (doesn't yet)
- Must match the original game's collision feel (0.025 scale, 3.0 minimum, 0.7
  speed ratio for "significant hit")
- Significant development effort

---

## Decision Matrix

| Approach | Difficulty | Reliability | Can Modify Response | Gets Other Ball | Best For |
|----------|-----------|------------|---------------------|-----------------|----------|
| 1. Code Cave | Hard | High | Yes (at hook point) | Yes (EDI) | Production mods, scoring |
| 2. Detour Ball_Update | Medium | Medium | Pre/post only | Via collision list | Analysis, logging |
| 3. Vtable Patch | Medium | High | Yes (wrapper) | Via collision list | AI mods, custom physics |
| 4. Poll Thread | Easy | Low | No | Yes (position) | Safe detection, stats |
| 5. Hook ApplyForce | Medium | Low | Yes (impulse) | No | Custom knockback |
| 6. Event Dispatchers | Easy | High | No | No (level objects) | Level event logging |
| 7. Render Hook Dist | Easy | Medium | No | Yes (position) | Quick prototyping |
| 8. Reimpl | Hard | High | Full | Full | Long-term, custom game |

### Recommendations

- **Just need to know when a bump happens**: Approach 7 (render hook distance check)
- **Need reliable detection with logging**: Approach 1 (code cave, proven in 8ball_hit_detect)
- **Need to modify collision response (knockback, immunity)**: Approach 5 (hook ApplyForce)
- **Need to intercept before physics runs**: Approach 2 (detour Ball_Update)
- **Building a full mod with custom AI**: Approach 3 (vtable patching)
- **Can't modify game code at all**: Approach 4 (polling thread)

---

## Pitfalls and Lessons Learned

### 1. Never Call C Functions from Hand-Assembled Code Caves

**Problem**: Calling `log_hit()`, `snprintf()`, or any C function from inside a
code cave in `Ball_Update` corrupts the FPU/stack/SEH state and crashes the game.

**Root Cause**: `Ball_Update` uses heavy x87 FPU operations. A `CALL` to a C
function disrupts the FPU stack alignment and may modify control words. The
`PUSHAD`/`POPAD` saves general registers but NOT the FPU state. Even
`FNSAVE`/`FNRSTOR` doesn't fully protect against SEH frame corruption.

**Solution**: Use the **volatile flag + polling thread pattern**:
- Code cave sets a `volatile DWORD g_hit_pending = player_index + 1`
- A background `CreateThread` loop checks `g_hit_pending` every 50ms
- All file I/O and string formatting happens on the polling thread

**Source**: This lesson was learned the hard way during 8ball_hit_detect v4
development. See `mods/8ball_hit_detect/8ball_hit_detect.c` comments.

### 2. Symmetric Double-Fire

**Problem**: `Ball_Update` runs for both balls in a collision pair. Your hook
fires twice per collision event.

**Solution**: Use pointer comparison (`ESI < EDI`) to count each collision once,
or track the collision ID (`[EBP+0x64]`) and deduplicate.

### 3. Calling Convention: `__thiscall` via `__fastcall`

**Problem**: MinGW GCC doesn't support `__thiscall` directly on x86.

**Solution**: Use `__fastcall` with a dummy EDX parameter:
```c
// __thiscall(this, arg1) == __fastcall(this, dummy_edx, arg1)
void __fastcall hook_func(void *this_, void *edx_dummy, void *arg1);
```
Both use ECX for `this`, and the callee cleans the stack.

### 4. AthenaList Iteration is Not Thread-Safe

**Problem**: The collision list at `PhysicsBody+0x18` uses `AthenaList_NextIndex`
which increments an internal counter at `+0x408` that wraps at 256. Reading this
list from a background thread may return stale or inconsistent data.

**Solution**: Either (a) hook inside `Ball_Update` where the list is valid, or
(b) use position-based detection (Approach 4/7) which doesn't depend on the
collision list.

### 5. `ball+0x2E9` is NOT a Ground/On-Surface Flag

**Problem**: The `dizzy_lock` at `ball+0x2E9` looks like it indicates
surface contact, but it's actually a sticky flag set by `E:LIMIT` (arena
boundary) collisions. It's never cleared within `Ball_Update`.

**Solution**: For ground detection, use `Mesh_FindClosestCollision`
(0x465D90) raycasting as documented in `docs/RAYCASTING_FOR_DLL_MODS.md`.

### 6. Verify Original Bytes Before Patching

**Problem**: Different game versions may have different bytes at the hook site.

**Solution**: Always verify the original bytes match before installing a hook:
```c
if (memcmp(hook_addr, expected_bytes, byte_count) != 0) {
    log("ERROR: Hook site bytes mismatch — wrong game version?");
    return;
}
```

### 7. Velocity Fields are Force Accumulators, Not Direct Velocity

**Problem**: Writing directly to `ball+0x170/174/178` (velocity) REPLACES the
accumulated force, killing horizontal momentum.

**Solution**: Use `FLD`/`FADD`/`FSTP` to ADD to the accumulator:
```asm
FLD DWORD [ESI+0x174]    ; load current Y velocity
FADD <impulse>           ; add impulse
FSTP DWORD [ESI+0x174]  ; store result
```

### 8. The 8-Ball AI Tick Calls Ball_Update

The 8-ball AI tick at `0x408390` calls `Ball_Update` (0x405E00) at offset
`0x4083BD` (`CALL 0x405E00`). This means:
- If you hook `Ball_Update`, your hook runs for both player balls and 8-balls
  (because the 8-ball AI internally calls through to Ball_Update)
- If you patch both vtable[4] entries, the 8-ball's internal call to Ball_Update
  goes through your wrapper TWICE

Use a re-entrancy guard:
```c
static __thread int in_update = 0;
void __fastcall hook_Ball_Update(void *ball, void *edx) {
    if (in_update) {
        g_orig_Ball_Update(ball, NULL);
        return;
    }
    in_update = 1;
    // ... your logic ...
    g_orig_Ball_Update(ball, NULL);
    // ... post-update logic ...
    in_update = 0;
}
```

### 9. Font Pointer Not Available at DLL Load

**Problem**: If you cache `App+0x318` (font pointer) at DLL load time, it will
be NULL because the font hasn't been created yet.

**Solution**: Read `App+0x318` fresh every frame in your render hook. The
font is valid once any scene (menu or level) has loaded.

### 10. `UI_DrawTextCentered` Does NOT Auto-Center

Despite its name, `0x409C60` does not automatically center text on screen.
The "centered" refers to vertical centering within the glyph cell. You must
calculate the text width and compute the X position yourself. See
`docs/UI_TEXT_ELEMENTS.md` for the `GetTextWidth()` implementation.

---

## References

- `mods/8ball_hit_detect/` — Working code cave hook mod (proven)
- `tools/collision_hook/` — Working detour hook tool for event dispatchers
- `docs/COLLISION_SYSTEM.md` — Collision mesh, octree, and physics pipeline
- `docs/COLLISION_SYSTEM_DEEP.md` — Deep dive on collision structs
- `docs/COLLISION_EVENT_SYSTEM.md` — Event dispatcher chain (level/arena events)
- `docs/BALL_OBJECT.md` — Ball struct field reference
- `docs/BALL_UPDATE_DECOMP.md` — Ball_Update decompilation annotations
- `docs/RAYCASTING_FOR_DLL_MODS.md` — Raycast pattern for ground/wall detection
- `docs/ARENA_SCORING.md` — Arena scoring system (RumbleBoard, score overflow)
- Skill `hamsterball-re` reference `ball-ai-clone-system.md` — Vtable patching for AI clones
- Skill `hamsterball-re` reference `ball-ball-collision-hook.md` — Collision hook addresses
- Skill `hamsterball-re` reference `hamsterball-dll-modding.md` — DLL mod build patterns
