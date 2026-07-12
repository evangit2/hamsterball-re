# Global Chrome (BadBall) Spawner

## What This Does

Spawns chrome balls (BadBalls) at the player's position on any level. Set `[SpawnChrome]` to 1 in Cheat Engine to trigger a spawn.

## Key Insight: Chromes ARE BadBalls

**Chromes in Odd Race are not a separate object type — they are BadBalls (enemy balls) rendered with the `chrome.png` texture.** There is no "chrome spawner" object. The Odd Race board's Update function spawns BadBalls at LAUNCH positions, and the game renders them with chrome textures based on the ball's playerID being -1 (enemy).

## How Odd Race Spawns Chromes (Full Trace)

### 1. Scene_SetupLevel6 (0x40EA90) — Board Setup

Loads `levels\level6` MESHWORLD and extracts named object positions:
- `LAUNCH01` → board+0x43DC (3 floats: X, Y, Z)
- `LAUNCH02` → board+0x43E8
- `LAUNCH03` → board+0x43F4
- `CHROMESHADOW` → board+0x43A4
- Timer set to 200 (board+0x4374), active flag = 0 (board+0x4370)

### 2. OddBoard_Update (0x41EE80, vtable[1]) — Per-Frame Spawner

Runs every frame. When `board+0x4370` (active flag) is set AND timer (board+0x4374) reaches zero:

1. Checks: `AthenaList_GetSize(board+0x29D4) < 10` (max 10 balls alive)
2. Checks: `board+0x4378` (spawn counter) `< 100` (max 100 total)
3. Resets timer: `board+0x4374 = CPUID_CheckProcessorFeature(&PTR_PTR, 25, 0) + 25` (25-50 frames)
4. Picks random launch point (0/1/2) — avoids repeating the last one
5. Reads position from `board + launchIndex*12 + 0x43DC`
6. Plays `sounds\ting` (App+0x4D0) at the launch position via `Sound_Play3D`
7. Allocates BadBall: `operator_new(0xC64)` → `BadBall_ctor(ball, board)`
8. Calls `vtable[1]` (0x40A040) to initialize physics
9. Calls `Ball_SetTrajectory` (0x403850) with launch position
10. Sets `ball+0x18 = -1` (playerID = enemy)
11. Sets Y position offset: `ball+0x168 += _DAT_004D0C50`
12. Sets ball properties:
    - `ball+0x278 = 0.5f` (gravity_scale)
    - `ball+0x27C = 0.1f` (friction)
    - `ball+0x284 = 24.0f` (radius)
    - `ball+0x188 = 5.0f` (max_speed)
13. Generates random XZ velocity (magnitude 2.5), Y=12.0 (upward)
14. Calls `Ball_SetVec3AtOffset` (0x402A20) to set velocity on physics struct
15. Calls physics `vtable[4]` to apply velocity
16. Adds ball to `board+0x29D4` (AthenaList)
17. Increments spawn counter

### 3. TimerDisplay (0x4298C0) — Texture Loading

Loads chrome textures globally for all levels:
- `chrome.png` → App+0xDA0 (0x368 in int-indexed)
- `chromeshadow.png` → App+0xDA4 (0x36C)

These are used when rendering BadBalls — the game selects the chrome texture based on `ball+0x18 == -1` (enemy ball).

### 4. PopCylinder vs Chrome — Completely Different Objects

| Property | PopCylinder | Chrome (BadBall) |
|---|---|---|
| Level | Sky Race (Level9) | Odd Race (Level6) |
| Mesh | `levels\level9-popcylinder1/2` | `Meshes\Sphere` |
| Alloc Size | 0x10E8 | 0xC64 |
| Vtable | 0x4D58F0 | 0x4CF494 |
| Purpose | Decorative pop-up cylinder | Enemy ball with chrome texture |
| Created by | `CreateLevelObjects` when name="POPCYLINDER" | `OddBoard_Update` per-frame timer |

## Why v7 Failed

v7 spawned PopCylinders via `CreateLevelObjects("POPCYLINDER")` and called `vtable[0x0B]` (PopCylinder_Update). PopCylinder_Update (0x43DED0) just does:
1. Check `this+0x10E4` flag (byte at int-index 0x439)
2. If set: `Timer_Init` → call `vtable[0x16]` (update) → call `vtable[0x15]` (render) → clear flag

No chrome is created. The PopCylinder is a standalone visual object with no spawning capability.

## Key Functions

| Address | Name | Purpose |
|---|---|---|
| 0x40EA90 | Scene_SetupLevel6 | Odd Race board setup, loads LAUNCH positions |
| 0x41EE80 | OddBoard_Update | Per-frame chrome (BadBall) spawner (vtable[1]) |
| 0x405D90 | BadBall_ctor | Constructs enemy ball (sets vtable, gravity, fall timer) |
| 0x40A040 | BadBall vtable[1] | Initialize physics for badball |
| 0x403850 | Ball_SetTrajectory | Sets ball trajectory target position |
| 0x402A20 | Ball_SetVec3AtOffset | Sets velocity on physics struct |
| 0x45DD60 | CPUID_CheckProcessorFeature | Game's PRNG (Fibonacci LCG, NOT real CPUID) |
| 0x4298C0 | TimerDisplay | Global texture/sound/mesh loading |
| 0x4BA57B | operator_new | C++ malloc wrapper |
| 0x453810 | AthenaList_Append | Add item to linked list |

## Key Struct Offsets

### Ball (BadBall uses same struct as player ball)

| Offset | Type | Field |
|---|---|---|
| +0x00 | ptr | vtable pointer |
| +0x14 | ptr | Board pointer |
| +0x18 | int | playerID (-1 = enemy/badball) |
| +0x164 | float | Position X |
| +0x168 | float | Position Y |
| +0x16C | float | Position Z |
| +0x188 | float | max_speed |
| +0x1A4 | ptr | Physics struct pointer |
| +0x278 | float | gravity_scale |
| +0x27C | float | friction |
| +0x284 | float | radius |
| +0x80C | int | fall timer (badball only, =50) |
| +0xC60 | float | gravity override (badball only, =1.0) |

### Board

| Offset | Type | Field |
|---|---|---|
| +0x878 | ptr | App pointer |
| +0x8AC | ptr | MeshWorld/Scene manager |
| +0x29D4 | AthenaList | Ball list (all balls including badballs) |
| +0x4370 | byte | Chrome spawn active flag |
| +0x4374 | int | Chrome spawn timer (counts down) |
| +0x4378 | int | Chrome spawn counter (max 100) |
| +0x43DC | float[3] | LAUNCH01 position (X, Y, Z) |
| +0x43E8 | float[3] | LAUNCH02 position |
| +0x43F4 | float[3] | LAUNCH03 position |

### BadBall Vtable (0x4CF494)

| Index | Offset | Address | Function |
|---|---|---|---|
| 0 | +0x00 | 0x405DD0 | dtor |
| 1 | +0x04 | 0x40A040 | init physics |
| 4 | +0x10 | 0x408830 | FallUpdate (badball fall timer logic) |

### OddBoard Vtable (0x4D0BC0)

| Index | Offset | Address | Function |
|---|---|---|---|
| 0 | +0x00 | 0x4250E0 | dtor |
| 1 | +0x04 | 0x41EE80 | Update (chrome spawner) |
| 0x12 | +0x48 | 0x40EA90 | SetupLevel6 |
| 0x1D | +0x74 | 0x40ED30 | CollisionHandler |

## Ghidra Misnomer Warning

`CPUID_CheckProcessorFeature` (0x45DD60) is NOT the real CPUID instruction. It is the game's custom PRNG — a Fibonacci-style LCG with a 55-element state table at `PTR_PTR_004F7360`. Ghidra auto-named it based on the import `IsProcessorFeaturePresent`. Used as `rand() % N` throughout the game.

## CEA Usage

1. Enable the script in Cheat Engine
2. Set `[SpawnChrome]` to 1 to spawn a chrome ball at the player's position
3. The ball spawns with random directional velocity (4 directions, cycling) and upward launch (Y=12.0)
4. The flag auto-resets to 0 after each spawn
