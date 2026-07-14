# Custom Entities Mod v5

Adds the **GRID system** to Hamsterball — cycle visibility of mesh objects in sequence, like an animated texture but for 3D geometry.

## GRID System

### How It Works

MeshBuffer names in `.MESHWORLD` files can include a `(GRIDxx)` suffix (GRID01 through GRID99). The mod cycles through these groups in order, showing only the meshes matching the current grid counter and hiding all others.

### Example

A level with 3 mesh objects:
```
N:PlatformA(GRID01)
N:PlatformB(GRID02)
N:PlatformC(GRID03)
```

The mod will:
1. Show PlatformA, hide B and C (grid_counter = 1)
2. After `grid_speed` ticks, show PlatformB, hide A and C (grid_counter = 2)
3. After `grid_speed` ticks, show PlatformC, hide A and B (grid_counter = 3)
4. After `grid_speed` ticks, loop back to step 1 (grid_counter = 1)

### Configuration

Create `custom_entities.txt` next to `bass.dll`:

```ini
# Ticks between grid advances (1 tick = ~16ms)
grid_speed = 10.0
```

- Default: `10.0` (≈160ms per step, ~6 steps/second)
- Minimum: `1.0` (one tick per step, fastest)
- The config file is auto-generated with defaults if it doesn't exist

### How Visibility Works

The mod sets `EntityTransform.posScale` to `1.0` (visible) or `0.0` (invisible) for each mesh. This is the same field the game uses for mesh scaling — setting it to 0 makes the mesh have zero size, effectively hiding it without removing collision data or affecting other systems.

### GRID Number Parsing

- `(GRID01)` through `(GRID09)` — single and double digit both work
- `(GRID1)` and `(GRID01)` are equivalent
- The suffix can appear anywhere in the name: `N:Platform(GRID01)`, `(GRID01)N:Platform`, etc.
- Multiple suffixes can be combined: `N:Platform(GRID01)(NOSHADOW)` — the game's native NOSHADOW still works alongside GRID
- The highest GRID number found in the level determines the loop point

### Combining with Native Suffixes

GRID works alongside the game's built-in suffixes:
- `(NOSHADOW)` — mesh casts no shadow (game-native)
- `(NOCOLLIDE)` — mesh has no collision (game-native)
- `(WANTZ)` — force Z-buffer rendering (game-native)
- `(GRIDxx)` — grid visibility cycling (this mod)

Example: `N:Platform(GRID01)(NOSHADOW)` = visible during grid step 1, no shadow when visible.

## Legacy CE Entity System

The mod also preserves the v4 custom entity system. See the v4 README for details on `CE:` prefixed entities and behavior DLLs.

## Installation

1. Rename original `bass.dll` to `bass_real.dll`
2. Copy mod `bass.dll` to game root
3. Copy `custom_entities.txt` to game root (or let the mod auto-generate it)

## Building

```bash
i686-w64-mingw32-gcc -shared -o bass.dll custom_entities.c \
  -I../shared -lwinmm -Wl,--enable-stdcall-fixup -O2 -static \
  -static-libgcc -Wl,--add-stdcall-alias -msse2 -mfpmath=sse -lshlwapi
```

## Files

| File | Description |
|------|-------------|
| `bass.dll` | Compiled mod (proxy DLL) |
| `custom_entities.txt` | Config file (grid_speed) |
| `source/custom_entities.c` | Main mod source |
| `source/entity_api.h` | Shared header for behavior DLLs |
| `source/meshworld_merger.c` | Runtime file merger (legacy, unused in v5) |
