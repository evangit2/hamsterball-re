# LevelFeatures_Loader (v6)

Universal cross-level object injection and vtable replacement for Hamsterball. Replaces all 15 per-level constructors **and** 4 vtable slots (Board_Update, RaceState, DispatchCollision, CreateDynamicObjects) with universal handlers, enabling config-driven level features without recompilation.

## What's New in v6

- **Universal Board_Update (Slot 1)**: Replaces all 15 per-level `Board_Update` functions with a single `UniversalBoardUpdate` dispatcher. Calls `Scene_Update` then runs config-driven feature blocks (bridge animation, swirl zones, windmill, badball spawner).
- **Universal RaceState (Slot 19)**: Replaces all 15 per-level `RaceState` handlers. Calls `Board_UpdateRaceState` then runs feature blocks (bumper decay, neon camera, sky popcylinder activator).
- **Universal DispatchCollision (Slot 29)**: Replaces all 15 per-level `DispatchCollision` handlers. Delegates to original per-level handlers (preserved for correct collision logic).
- **Universal CreateDynamicObjects (Slot 33)**: Replaces all 15 per-level object factories. Delegates to original per-level handlers (preserved for correct object creation).
- **Feature flag system**: 8 bitflags (`FEAT_BRIDGE_ANIM`, `FEAT_SWIRL`, `FEAT_WINDMILL`, `FEAT_BADBALL`, `FEAT_BUMPER_DECAY`, `FEAT_NEON_CAM`, `FEAT_SKY_POPCYL`, `FEAT_MASTER_EXTRA`) with default assignments matching original game behavior.

## How It Works

### Constructor Replacement

The mod intercepts `Tournament_AdvanceRace` (0x00427080) at the point where it calls the per-level `Scene_LoadLevel*` function via `CALL [EDX+0x48]` (vtable[18]). Instead of letting the original per-level constructor run, a **universal constructor** takes over:

1. `operator_new(0x10D0)` → `Level_MeshWorldCtor(mem, gfx, meshPath)` → `board+0x8AC`
2. `operator_new(0x10D0)` → `Level_RenderCtor(mem, meshWorld)` → `board+0x8B0`
3. `Level_InitScene(board)`
4. `UniversalPostSetup(board)` — config-driven features (bumpers, bridge, etc.) — **before** Board_Setup
5. `board->vtable[0x80]()` = `Board_Setup`

### Vtable Slot Patching

After construction, `InstallVtablePatches` overwrites 4 vtable slots in all 15 level vtables:

| Slot | Offset | Original | Universal Handler | Strategy |
|------|--------|----------|-------------------|----------|
| 1 | +0x04 | Per-level Board_Update | `UniversalBoardUpdate` | Feature blocks (bridge, swirl, windmill, badball) |
| 19 | +0x4C | Per-level RaceState | `UniversalRaceState` | Base `Board_UpdateRaceState` + feature blocks (bumper decay, neon cam, sky popcyl) |
| 29 | +0x74 | Per-level DispatchCollision | `UniversalDispatchCollision` | Delegates to saved original pointer |
| 33 | +0x84 | Per-level CreateDynamicObjects | `UniversalCreateDynamicObjects` | Delegates to saved original pointer |

### Feature Blocks

Each feature block replicates the exact logic from the original decompiled per-level handlers:

| Feature | Levels | Source Function | Description |
|---------|--------|-----------------|-------------|
| `FEAT_BRIDGE_ANIM` | 3, 14 | Intermediate (0x41CC90), Master (0x421400) | 4-state bridge tilt machine (wait→tilt down→wait→tilt back) |
| `FEAT_SWIRL` | 4, 14 | Dizzy (0x41D510), Master (0x420DA0) | Proximity check, velocity scale, tar bubbles, mesh rotation |
| `FEAT_WINDMILL` | 5 | Tower (0x41E760) | Rotation + 4-state speed control (spin up→creak→spin down→pause) |
| `FEAT_BADBALL` | 9 | Odd (0x41EE80) | Timer-based BadBall spawning with random position selection |
| `FEAT_BUMPER_DECAY` | 2, 10, 14 | Beginner/Toob/Master | Decays 4-8 bumper "lit" float values by 0.05/frame |
| `FEAT_NEON_CAM` | 7 | Neon (0x424790) | Positions render objects relative to ball position |
| `FEAT_SKY_POPCYL` | 13 | Sky (0x41FC90) | Random popcylinder activation via 6-case switch |

### Hooks

| Hook | Address | Description |
|------|---------|-------------|
| **Alloc size patch** | 15 sites in `Tournament_AdvanceRace` (0x27109–0x273A5) | Patches all 15 `PUSH imm32` allocation sizes to `0xA2F8` (union of all board struct sizes) |
| **Universal constructor** | 0x4273E0 (`CALL [EDX+0x48]`) | 6-byte JMP detour. Naked thunk reads race index from `[ESI+0x8]`, calls `UniversalConstructor(board, raceIndex)` |
| **Board constructor hooks** | 15 `CALL LevelBoard_*_ctor` sites (0x2712C–0x273C8) | Patches to per-level naked thunks → `UniversalBoardCtorLogic` |
| **Collision hook** | 0x40C5D0 (`DispatchCollisionEvents`) | 8-byte trampoline. Intercepts all collision events for bumper physics |
| **Vtable patches** | 15 vtables × 4 slots | Overwrites Board_Update, RaceState, DispatchCollision, CreateDynamicObjects slots |

### Object system

Features are implemented as **objects** with per-level toggles. Each object type has:
- An `Init` function that replicates the exact steps from the original per-level constructor
- A config entry in `LevelFeatures.txt` listing which levels it's active on

#### Implemented objects

| Object | Init function | Source (Ghidra) | Description |
|--------|---------------|-----------------|-------------|
| `BUMPERS` | `UniversalPostSetup` | Beginner ctor (0x4111E0) | Collects N:BUMPER1-8 via `Scene_CollectByNameFilter`, applies bounce physics on collision |
| `BRIDGE` | `InitBridge` | Intermediate ctor (0x411CB20) | Loads `Levels\Level2-Bridge` mesh+render, calls `TipperVisual_Attach`, sets bridge params (45.0, 0, 50) |

#### Adding new objects

1. Add entry to `ObjectType` enum and `g_objectNames[]` array
2. Write an `Init<ObjectName>` function that replicates the original constructor steps
3. Call it from `UniversalPostSetup` guarded by `g_objectEnabled[OBJ_XXX][level]`
4. Add the object name to `LevelFeatures.txt`

## Config file

`LevelFeatures.txt` (next to `bass.dll`) controls which objects and features are active per level:

### [OBJECTS] — Object toggles

```ini
[OBJECTS]
BUMPERS = 2 5 8
BRIDGE = 3 14
```

List level numbers after `=` to enable objects for those levels. Empty `()` means disabled everywhere.

### [FEATURES] — Update feature overrides

```ini
[FEATURES]
# Levels not listed use built-in defaults.
# Use () to clear all features for a level.
2 = SWIRL BUMPER_DECAY   # Add swirl to Beginner!
14 = SWIRL BUMPER_DECAY BRIDGE_ANIM MASTER_EXTRA
```

Available features: `BRIDGE_ANIM`, `SWIRL`, `WINDMILL`, `BADBALL`, `BUMPER_DECAY`, `NEON_CAM`, `SKY_POPCYL`, `MASTER_EXTRA`.

Config is re-read on every level load.

## LevelData.txt

Auto-generated on first run with Ghidra-extracted per-level defaults. Editable to customize:
- Level names, vtable addresses, board names, race titles
- Race data, music names, colors
- Mesh paths and extra mesh loads (for non-object meshes)

## Installation

1. Rename original `bass.dll` to `bass_real.dll`
2. Copy `bass.dll` and `LevelFeatures.txt` from this mod into the game folder

## Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll LevelFeatures.c \
  bass.def -lwinmm -static-libgcc -Wl,--enable-stdcall-fixup -O2
```

## Files

- `bass.dll` — compiled mod (rename original bass.dll to bass_real.dll)
- `LevelFeatures.c` — source code
- `LevelFeatures.txt` — config file (object toggles per level)
- `LevelData.txt` — auto-generated per-level data (created on first run)
- `LevelFeatures.xml` — reference catalog of all injectable objects (documentation only)
- `bass.def` — export definitions for bass.dll proxy (111 forwarded exports)

## Compatibility

- Game version: V3.6.c
- Load mechanism: bass.dll proxy
- The mod patches 15 allocation sites, 15 board constructor calls, 1 scene constructor call, 1 collision handler, and 60 vtable slots (15 vtables × 4 slots). All patches verify original bytes before applying.
- **Wine/llvmpipe crash test: PASSED** — game survived 35+ seconds with offset fixes applied.
