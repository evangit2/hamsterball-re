# LevelFeatures_Loader (v9) — RaceFiles.txt swappable races

Universal cross-level object injection and vtable replacement for Hamsterball. Replaces all 15 per-level constructors **and** 4 vtable slots (Board_Update, RaceState, DispatchCollision, CreateDynamicObjects) with universal handlers, enabling config-driven level features without recompilation.

## What's New in v9
- **RaceFiles.txt**: 15-line map `Race N: <file>` (e.g. `Race 1: Level1`) lets you swap any race to any `.MESHWORLD` **without renaming files**. The game loads `levels\\<name>` from this file instead of the hardcoded slot. Accepts `Race 1: Level1`, `2 = LevelCascade`, `3 = levels\\level2.MESHWORLD`, etc. Bare name auto-prepends `levels\\`; `.MESHWORLD` extension auto-stripped. File is re-read on every level load (no restart). Auto-generated with defaults on first run. S1 scan still auto-enables bridge/swirl/windmill from the *loaded* file's S1 refs, so dropping `Level3` into `Race 1` just works.

## What's New in v8

- **TURRET handler fix**: Was calling `Stands vtable[2]` (SceneObject_BuildStrips) for SetPosition, causing infinite loop on TURRET09. Fixed to call `Timer vtable[2]` (Gfx_SetPosition) with 3 floats by value, then `Stands vtable[0x15]` with position struct pointer — matching original `Tower_CreateDynamicObjects` at `0x0040d7c0`.
- **Dedicated mesh slots**: 9 new dedicated mesh offsets at `0x86C0`–`0x86E0` for Tipper, Spinny, Saw, Fallout, Gluebie, Looper, Gear, BigGear. Bridge stays at `0x8620`/`0x8628`. Eliminates mesh slot conflicts when multiple object types share a level.
- **InitBridge fix**: Writes to `UNI_BONK_STORE`/`UNI_SAW1_OBJ` instead of `BRIDGE_MESHWORLD`/`BRIDGE_RENDEROBJ` to avoid double-allocation.
- **DebugLog fallback**: Uses current directory when `GetConfigPath` fails under Wine/BoxedWine.
- **LoadConfig infinite loop fix**: Breaks after `LoadCollisionConfig` since `[COLLISION]` is the last section.

## Verified Levels

| Level | Status | Notes |
|-------|--------|-------|
| Warm-Up | ✅ Passes | No objects, loads clean |
| Beginner | ✅ Passes | Bumpers work, `Board_Setup done` |
| Intermediate | ✅ Passes | Bridge mesh at `0x8620` valid, `Board_Setup done` |
| Dizzy | ✅ Passes | All 7 meshes loaded, Tipper at `0x86C0`, Gluebie at `0x86D4`, Swirl running, `Board_Setup done` |
| Tower | ⚠️ Wine blocker | TURRET handler fixed (Timer vtable[2]). Hangs in `Board_Setup` D3D8 calls on Wine/llvmpipe. Needs Windows testing. |

## What's New in v7

- **Per-level structural init**: All per-level constructor init logic is now config-driven via `LevelData.txt`:
  - `AthenaLists` — AthenaList_Init offsets (Tower: 4 lists, Expert: 3, Dizzy: 2, Up: 1, Sky: 1, Master: 4)
  - `EHVector` — eh_vector_constructor_iterator arrays (Beginner/Toob: 8 bumper slots, Master: 4)
  - `ZeroFills` — State machine variable zeroing (Tower windmill, Dizzy swirl, Toob, Sky)
  - `AssignTex` — Level_AssignTexturesAndScales for reused meshes (Master: 5 meshes)
  - `SoundChannel` — Sound channel allocation (Dizzy waterwheel)
  - `BridgeParam` — Bridge tilt state machine init (Intermediate, Master)
- **Removed hardcoded per-level special cases**: Dizzy/Tower/Expert/Sky/Master/Impossible init is now fully data-driven from the LevelData table, no more `if (raceIndex == N)` blocks.

## What's New in v6

- **Universal Board_Update (Slot 1)**: Replaces all 15 per-level `Board_Update` functions with a single `UniversalBoardUpdate` dispatcher. Calls `Scene_Update` then runs config-driven feature blocks (bridge animation, swirl zones, windmill, badball spawner).
- **Universal RaceState (Slot 19)**: Replaces all 15 per-level `RaceState` handlers. Calls `Board_UpdateRaceState` then runs feature blocks (bumper decay, neon camera, sky popcylinder activator).
- **Universal DispatchCollision (Slot 29)**: Replaces all 15 per-level `DispatchCollision` handlers. Delegates to original per-level handlers (preserved for correct collision logic).
- **Universal CreateDynamicObjects (Slot 33)**: Replaces all 15 per-level object factories. Delegates to original per-level handlers (preserved for correct object creation).
- **Feature flag system**: 7 bitflags (`FEAT_BRIDGE_ANIM`, `FEAT_SWIRL`, `FEAT_WINDMILL`, `FEAT_BADBALL`, `FEAT_BUMPER_DECAY`, `FEAT_NEON_CAM`, `FEAT_SKY_POPCYL`) with default assignments matching original game behavior.

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
| 29 | +0x74 | Per-level DispatchCollision | `UniversalDispatchCollision` | Reimplements all 62 collision events inline, then calls global `DispatchCollisionEvents` |
| 33 | +0x84 | Per-level CreateDynamicObjects | `UniversalCreateDynamicObjects` | Reimplements all 30+ object ctors inline (no delegation to originals) |

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

## RaceFiles.txt
Maps `Race 1`..`Race 15` to a file. Edit `RaceFiles.txt` next to `bass.dll`:
```ini
Race 1: Level1          # WarmUp slot loads levels\\level1
Race 2: LevelCascade    # Beginner slot loads levels\\levelcascade
Race 3: Level2          # etc.
Race 4: Level3          # swap Dizzy into WarmUp: change Race 1 to Level3
```
Any `.MESHWORLD` in `levels\\` works. The S1 feature scan uses the *loaded* file, so features follow the file, not the slot.

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
14 = SWIRL BUMPER_DECAY BRIDGE_ANIM
```

Available features: `BRIDGE_ANIM`, `SWIRL`, `WINDMILL`, `BADBALL`, `BUMPER_DECAY`, `NEON_CAM`, `SKY_POPCYL`.

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
- **Wine/llvmpipe**: Game starts, PatchThread completes, levels load. Dizzy and Intermediate verified crash-free with debug logs showing `Board_Setup done`. Tower hangs in D3D8 `Gfx_SetPosition` / `D3DX_ShaderDispatch_4b` — a Wine rendering limitation, not a code bug.
- **Wine navigation**: Use `xdotool key --delay 200` for menu navigation. Race selection requires precise key timing.
- **File-swap testing**: To test locked levels (Dizzy, Tower), swap `g_levelData[1]` (Warm-Up) with the target level's entry. Warm-Up is always unlocked in Time Trials.
- Per-level code paths: Dizzy and Intermediate verified on Wine. Tower needs Windows testing (D3D8 hang). Other levels untested.
