# LevelSpecials_Loader (v5)

Universal cross-level object injection for Hamsterball. Replaces all 15 per-level constructors with a single universal constructor, then injects config-driven objects (bumpers, bridges, etc.) into any level.

## How It Works

The mod intercepts `Tournament_AdvanceRace` (0x00427080) at the point where it calls the per-level `Scene_LoadLevel*` function via `CALL [EDX+0x48]` (vtable[18]). Instead of letting the original per-level constructor run, a **universal constructor** takes over and performs the same steps every level does:

1. `operator_new(0x10D0)` → `Level_MeshWorldCtor(mem, gfx, meshPath)` → `board+0x8AC`
2. `operator_new(0x10D0)` → `Level_RenderCtor(mem, meshWorld)` → `board+0x8B0`
3. `Level_InitScene(board)`
4. `UniversalPostSetup(board)` — config-driven features (bumpers, bridge, etc.) — **before** Board_Setup so that sub-meshes are available when `vtable[33]` (CreateDynamicObjects) reads them
5. `board->vtable[0x80]()` = `Board_Setup`

The per-level `Scene_LoadLevel*` functions **never run**. The universal constructor reads the race index from `[ESI+0x8]` and looks up the correct mesh path from a table.

### Hooks

| Hook | Address | Description |
|------|---------|-------------|
| **Alloc size patch** | 15 sites in `Tournament_AdvanceRace` (0x27109–0x273A5) | Patches all 15 `PUSH imm32` allocation sizes to `0xA2F8` (union of all board struct sizes) |
| **Universal constructor** | 0x4273E0 (`CALL [EDX+0x48]`) | 6-byte JMP detour. Naked thunk reads race index from `[ESI+0x8]`, calls `UniversalConstructor(board, raceIndex)` |
| **Board constructor hooks** | 15 `CALL LevelBoard_*_ctor` sites (0x2712C–0x273C8) | Patches to per-level naked thunks → `UniversalBoardCtorLogic` |
| **Collision hook** | 0x40C5D0 (`DispatchCollisionEvents`) | 8-byte trampoline. Intercepts all collision events for bumper physics |

### Object system

Features are implemented as **objects** with per-level toggles. Each object type has:
- An `Init` function that replicates the exact steps from the original per-level constructor
- A config entry in `LevelSpecials.txt` listing which levels it's active on

#### Implemented objects

| Object | Init function | Source (Ghidra) | Description |
|--------|---------------|-----------------|-------------|
| `BUMPERS` | `UniversalPostSetup` | Beginner ctor (0x4111E0) | Collects N:BUMPER1-8 via `Scene_CollectByNameFilter`, applies bounce physics on collision |
| `BRIDGE` | `InitBridge` | Intermediate ctor (0x411CB20) | Loads `Levels\Level2-Bridge` mesh+render, calls `TipperVisual_Attach`, sets bridge params (45.0, 0, 50) |

#### Adding new objects

1. Add entry to `ObjectType` enum and `g_objectNames[]` array
2. Write an `Init<ObjectName>` function that replicates the original constructor steps
3. Call it from `UniversalPostSetup` guarded by `g_objectEnabled[OBJ_XXX][level]`
4. Add the object name to `LevelSpecials.txt`

## Config file

`LevelSpecials.txt` (next to `bass.dll`) controls which objects are active per level:

```ini
# Level numbers: 1=WarmUp 2=Beginner 3=Intermediate 4=Dizzy 5=Tower
#   6=Up 7=Neon 8=Expert 9=Odd 10=Toob 11=Wobbly 12=Glass 13=Sky
#   14=Master 15=Impossible

[OBJECTS]
BUMPERS = 2 5 8
BRIDGE = 3 14
```

List level numbers after `=` to enable objects for those levels. Empty `()` means disabled everywhere. Config is re-read on every level load.

## LevelData.txt

Auto-generated on first run with Ghidra-extracted per-level defaults. Editable to customize:
- Level names, vtable addresses, board names, race titles
- Race data, music names, colors
- Mesh paths and extra mesh loads (for non-object meshes)

## Installation

1. Rename original `bass.dll` to `bass_real.dll`
2. Copy `bass.dll` and `LevelSpecials.txt` from this mod into the game folder

## Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll LevelSpecials.c \
  -lwinmm -static-libgcc -Wl,--enable-stdcall-fixup -O2
```

## Files

- `bass.dll` — compiled mod (rename original bass.dll to bass_real.dll)
- `LevelSpecials.c` — source code
- `LevelSpecials.txt` — config file (object toggles per level)
- `LevelData.txt` — auto-generated per-level data (created on first run)
- `LevelSpecials.xml` — reference catalog of all injectable objects (documentation only)
- `bass.def` — export definitions for bass.dll proxy

## Compatibility

- Game version: V3.6.c
- Load mechanism: bass.dll proxy
- The mod patches 15 allocation sites, 15 board constructor calls, 1 scene constructor call, and 1 collision handler. All patches verify original bytes before applying.
