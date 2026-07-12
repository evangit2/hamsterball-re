# Hamsterball Rotating Objects Comparison

## Overview

Hamsterball has three rotating platform systems across different races. Despite
similar visual behavior, they use fundamentally different implementation
strategies.

## 1. Dizzy Swirl (Board-Update Rotation)

### Architecture

The Dizzy swirl is **NOT a separate game object** — it's rotation logic baked
directly into the Dizzy board's per-frame update function.

### Key Functions

- `DizzyBoard_Update` (0x0041D510) — vtable[1] for Dizzy board
- `Dizzy_CreateDynamicObjects` (0x0040A5F0) — vtable[33], stores SWIRL positions

### How It Works

1. `Dizzy_CreateDynamicObjects` stores SWIRL positions from level data into
   `board+0x4BCC/0x4BD0/0x4BD4` (X/Y/Z) and mesh refs at `board+0x4BC4/0x4BC8`
2. `DizzyBoard_Update` calls `Scene_Update` first
3. Iterates all balls via AthenaList at `board+0xA75`
4. For each ball NOT in tar (`ball+0x2CC == 0`):
   - Iterates swirl zones from AthenaList at `board+0x10DE`
   - Checks 3D distance from ball to swirl center against `radius * _DAT_004D0930`
   - If within range: applies force to `ball+0xCA4/0xCA8/0xCAC`
   - Plays `sounds\whoosh`, creates dust particles
   - Sets `ball+0x2BC = 1` (being swirled flag)
5. Rotation via **matrix transforms**:
   - `board+0x12EA` = SceneObject (primary swirl mesh)
   - `board+0x12EC/0x12ED/0x12EE` = position (X/Y/Z)
   - `board+0x12EF` = scale
   - `board+0x12F0` = 0x3F000000 (0.5 — rotation increment)
   - `board+0x12F1` = SceneObject (secondary swirl mesh)
   - `board+0x12F3/0x12F4/0x12F5` = secondary position
   - `board+0x12F6` = accumulated rotation angle (0.5/frame, 0.25 non-tournament)
   - `board+0x12F7` = sound channel
   - Uses `Timer_Init`, `Gfx_ScaleY`, `Gfx_SetPosition`, then calls
     `vtable[0x58]` (SetPosition) and `vtable[0x54]` (SetDirection)
6. After transform, iterates balls again and applies rotation matrix to their
   positions (only for balls with `ball+0x778 != 0`)

### Key Insight

This is a **render-only transform** using matrix operations. The mesh itself
is not modified — only its world-space transform changes. This is simpler and
more performant than vertex deformation.

## 2. Toob Spinny (Rotator Object — Vertex Deformation)

### Architecture

The Spinny is a separate `Stands`-derived game object with its own vtable and
update function.

### Key Functions

- `Rotator_ctor_Toob` (0x00435940) — constructor
- `Rotator_Update` (0x004606D0) — vtable[1], shared update function

### Constructor Details

- `__thiscall(this, board, x, y, z, mesh_level)`
- Size: **0x1508 bytes** (Stands-derived)
- Vtable: **0x004D5518**
- Calls `Stands_ctor(this, mesh_level)` first
- Stores: board@+0x10D0, pos@+0x10D8/DC/E0
- `this+0x10E4` = CPUID speed factor (float)
- `this+0x10E8` = 0 (initial rotation angle) or -0.2 (0xBE4CCCCD) if non-multi
- `this+0x10EC` = 1.0 (rotation direction) or 0.0 if non-multi
- Creates collision mesh via `FUN_00465080` at `this+0x10D4`
- AthenaList initialized at `this+0x10F0`

### Update Function (Shared)

`Rotator_Update` (0x004606D0) is shared by BOTH Toob Spinny and Impossible
Rotator. It performs **vertex buffer deformation**:

1. Checks `this+0x47C == this` (self-reference)
2. First frame: allocates vertex buffer at `sceneObject+0x10C8`
   (size = vertCount * 0x60), sets flag at `+0x10C4`
3. Updates child objects via AthenaList at `this+0x18`
4. Iterates MeshBuffers in the mesh
5. For each MeshBuffer: counts total vertices, creates a temporary `MeshWorld`
6. Copies vertex data into the dynamic vertex buffer (3 vertices at a time =
   triangle strips), alternating vertex order for strip continuity
7. Calls `Font_RenderToTextureComplex` to upload the result
8. Calls `Mesh_SaveAndFree` to clean up

### Mesh Source

Pre-loaded `Level8-Spinny.MESHWORLD` (loaded during board construction).

## 3. Impossible Rotator (Rotator Object — Vertex Deformation)

### Architecture

Same architecture as Toob Spinny but with a different constructor variant and
different struct layout.

### Key Functions

- `Rotator_ctor_Impossible` (0x004366F0) — constructor
- `Rotator_Update` (0x004606D0) — vtable[1], **same shared function**

### Constructor Details

- `__thiscall(this, board, x, y, z, extra_float, mesh_level)`
- **Extra float parameter** (scale offset from level data)
- Size: **0x1508 bytes** (same as Toob)
- Vtable: **0x4D5708** (different dtor, same update)
- Calls `Stands_ctor(this, mesh_level)` first
- Stores: board@+0x10D0, pos@+0x10D4/D8/DC (**different offsets from Toob!**)
- `this+0x10E0` = extra_float (scale offset)
- Creates collision mesh at `this+0x10F0` (**different offset from Toob!**)
- `this+0x10ED` = 0 (byte flag)
- `this+0x10EC` = 1 (byte — rotation enabled)

### Post-Construction Setup

`Impossible_CreateDynamicObjects` sets:
- `this+0x43A` = 0x3F800000 (1.0 — rotation direction)
- CPUID check: if result == 0, sets `this+0x43A = 0xBF800000` (-1.0 — reverse)

### Mesh Source

Pre-loaded `LevelImpossible-Rotator.MESHWORLD`.

## Comparison Table

| Feature | Dizzy Swirl | Toob Spinny | Impossible Rotator |
|---|---|---|---|
| **Object type** | Board update logic (not separate object) | Stands-derived object | Stands-derived object |
| **Constructor** | N/A | Rotator_ctor_Toob (0x435940) | Rotator_ctor_Impossible (0x4366F0) |
| **Object size** | N/A | 0x1508 | 0x1508 |
| **Vtable** | N/A | 0x4D5518 | 0x4D5708 |
| **Update function** | DizzyBoard_Update (0x41D510) | Rotator_Update (0x4606D0) | Rotator_Update (0x4606D0) |
| **Rotation method** | Matrix transform (Gfx_ScaleY/SetPosition + vtable calls) | Vertex buffer deformation | Vertex buffer deformation |
| **Mesh source** | Level3-Swirl.MESHWORLD | Level8-Spinny.MESHWORLD | LevelImpossible-Rotator.MESHWORLD |
| **Collision** | Proximity zones (no mesh collision) | Mesh-based (FUN_00465080) | Mesh-based (FUN_00465080) |
| **Ball interaction** | Force applied to velocity (board logic) | Collision events (N:SPINNER) | Collision events (N:ONROTATOR) |
| **Rotation speed** | 0.5/frame (0.25 non-tournament) | this+0x10E8 + this+0x10EC | this+0x43A (1.0 or -1.0 via CPUID) |
| **Performance** | Lightweight (matrix only) | Heavy (vertex buffer copy per frame) | Heavy (same vertex buffer copy) |

## 4. Up Race "Rotator" Variants (NOT Rotators)

These were formerly misnamed as Rotator/Pendulum variants by Ghidra auto-analysis.
They are completely different object types.

### Lifter_ctor (0x00436920)

- Formerly: `Rotator_ctor_sound`
- Used for: `"LIFTER"` in Up race
- Size: 0x10F4, Vtable: 0x4D5770
- Extra param: `lifter_id` (int from `_atol` on level string)
- `this+0x10EC` = lifter_id (default 0x96 = 150)
- `this+0x10E8` = 0x3C (60 — timer/speed)
- `this+0x10F0` = 1 (byte flag)
- `this+0x10E4` = 0

### TimeButton_ctor (0x00436C10)

- Formerly: `Rotator_ctor_nosound`
- Used for: `"TIMEBUTTON"` in Up race
- Size: 0x10E8, Vtable: 0x4D5830
- `this+0x10E4` = 0 (byte flag)
- `this+0x10E5` = 1 (byte flag)

### SpeedCylinder_ctor (0x00436A20)

- Formerly: `Pendulum_ctor` (first variant)
- Used for: `"SPEEDCYLINDER"` in Up race
- Size: 0x150C, Vtable: 0x4D57D0
- Extra param: int → float at `this+0x10E4` (speed value)
- `this+0x10E8` = 0
- `this+0x1508` = 0
- `this+0x10EC` = 0x3E800000 (0.25 — default scale)
- AthenaList at `this+0x10F0`

### Pendulum_ctor (0x00437700) — Actual Pendulum

- Used for: `"PENDULUM"` in Impossible race (this IS a real Pendulum)
- Size: 0x1504, Vtable: 0x4D5B30
- `this+0x14FC` = CPUID result (float)
- `this+0x1500` = `CPUID_result * _DAT_004CF3D0 + _DAT_004CF3F0` (swing speed)
- AthenaList at `this+0x10E4`

## All 15 Level Dynamic Object Creation Functions (vtable[33])

| Level | Function Name | Address | Objects |
|---|---|---|---|
| WarmUp | WarmUp_Beginner_CreateDynamicObjects | 0x419750 | *(none — no-op)* |
| Beginner | WarmUp_Beginner_CreateDynamicObjects | 0x419750 | *(none — same function)* |
| Intermediate | Intermediate_CreateDynamicObjects | 0x40A550 | BRIDGE (position only) |
| Dizzy | Dizzy_CreateDynamicObjects | 0x40A5F0 | TIPPER, WATERWHEEL, SWIRL, GLUEBIE |
| Tower | Tower_CreateDynamicObjects | 0x40D7C0 | CATAPULT, MACE, DRAWBRIDGE, WINDMILL, TRAPDOOR, CHOMPER, TURRET |
| Expert | Expert_CreateDynamicObjects | 0x40E250 | BONK, FAN, SAWBLADE, BRIDGE, JUDGE, BELL |
| Odd | Odd_CreateDynamicObjects | 0x40EC40 | LIFTER |
| Up | Up_CreateDynamicObjects | 0x4117B0 | LIFTER, SPEEDCYLINDER, TIMEBUTTON |
| Wobbly | Wobbly_CreateDynamicObjects | 0x40F420 | WOBBLY1-7, WAVY1 |
| Sky | Sky_CreateDynamicObjects | 0x410AD0 | POPCYLINDER, TRAPDOOR |
| Toob | Toob_CreateDynamicObjects | 0x40FB30 | SPINNY, SAW, SAW2, FALLOUT1, BLOCKDAWG1-3 |
| Glass | Glass_CreateDynamicObjects | 0x40AD80 | SMASHER1, SMASHER2 (position only) |
| Neon | Neon_CreateDynamicObjects | 0x416910 | NEONPLATFORM, DFLOOR1-4, TRODE |
| Master | Master_CreateDynamicObjects | 0x4121D0 | BRIDGE, TIPPER, BONK, BBRIDGE1/2, POPCYLINDER, BLOCKDAWG1/2, CATAPULT, GLUEBIE |
| Impossible | Impossible_CreateDynamicObjects | 0x417FE0 | LOOPER, GEAR, BIGGEAR, ROTATOR, PENDULUM |

## Key Lessons

1. **Every level has its own unique vtable[33]** — no sharing between levels
2. **Ghidra auto-names are often wrong** — always verify via xrefs and decompilation
3. **The Dizzy Swirl uses matrix transforms** (render-only), while Toob Spinny and
   Impossible Rotator use **vertex buffer deformation** (physically modifies mesh)
4. **Matrix transform vs vertex deformation — the deciding factor is object shape:**
   - **Cylindrical/symmetric objects** (Dizzy Swirl): matrix transform is sufficient
     because the collision surface is identical at every rotation angle. The game
     cheats: rotates the visual mesh but never touches the collision geometry.
   - **Non-cylindrical/asymmetric objects**: vertex buffer deformation is REQUIRED
     because the collision surface changes as the object rotates. Without it, the
     visual spins while the ball stands on an invisible flat surface.
5. **For a global rotating ref mod**: use the vertex buffer deformation approach
   (Rotator_Update 0x4606D0) for a general-purpose solution that works for ANY
   shape. The matrix transform shortcut only works for symmetric objects.

## Global Pre-Load System

`TimerDisplay` (0x00429FFF) pre-loads these MESHWORLD files globally for all
levels via vtable[0x50]:
- `Levels\MouseTrap`
- `Levels\Secret`
- `Levels\Secret-Unlock`
- `Levels\PopupSign`
- `Level4-Trapdoor1`, `Level4-Trapdoor2`
- `Level6-Lifter`

Additionally, `FUN_0041C5B0` calls these global object creators for every level:
- `CreateSecretObjects` (0x0040BAA0)
- `CreateMouseTrap` (0x0040BF50)
- `Scene_CreateFlags` (0x0040C0F0)
- `Scene_CreateSigns` (0x0040C270)

## Ghidra Renames Applied

| Old Name | New Name | Address |
|---|---|---|
| CreateExpertLevelObjects | Expert_CreateDynamicObjects | 0x40E250 |
| CreateLevelObjects | Master_CreateDynamicObjects | 0x4121D0 |
| CreateMechanicalObjects | Impossible_CreateDynamicObjects | 0x417FE0 |
| CreateUpLevelObjects | Up_CreateDynamicObjects | 0x4117B0 |
| FUN_00419750 | WarmUp_Beginner_CreateDynamicObjects | 0x419750 |
| FUN_0040A5F0 | Dizzy_CreateDynamicObjects | 0x40A5F0 |
| FUN_0040D7C0 | Tower_CreateDynamicObjects | 0x40D7C0 |
| FUN_0040A550 | Intermediate_CreateDynamicObjects | 0x40A550 |
| OddBoard_LifterUpdate | Odd_CreateDynamicObjects | 0x40EC40 |
| FUN_0040F420 | Wobbly_CreateDynamicObjects | 0x40F420 |
| FUN_00410AD0 | Sky_CreateDynamicObjects | 0x410AD0 |
| FUN_0040FB30 | Toob_CreateDynamicObjects | 0x40FB30 |
| FUN_0040AD80 | Glass_CreateDynamicObjects | 0x40AD80 |
| FUN_00416910 | Neon_CreateDynamicObjects | 0x416910 |
| Rotator_ctor (Toob) | Rotator_ctor_Toob | 0x435940 |
| Rotator_ctor (Impossible) | Rotator_ctor_Impossible | 0x4366F0 |
| Rotator_ctor_sound | Lifter_ctor | 0x436920 |
| Rotator_ctor_nosound | TimeButton_ctor | 0x436C10 |
| Pendulum_ctor (Up) | SpeedCylinder_ctor | 0x436A20 |
| FUN_0041D510 | DizzyBoard_Update | 0x41D510 |
