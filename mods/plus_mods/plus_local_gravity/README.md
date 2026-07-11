# Local Gravity — Hamsterball Plus Mod

**Author:** BookwormKevin  
**Contributors:** Hamsterbot  
**API:** Hamsterball Plus v1  
**Files:** `LocalGravity.cpp` (VS), `LocalGravity_MinGW.cpp` (MinGW)

## What It Does

Sets gravity **per level or arena** by reading values from `local_gravity_set.txt`. Each of the 30 lines corresponds to a specific level (15 races + 15 arenas). The file is re-read on every level start, so changes take effect without restarting the game.

Unlike `plus_low_gravity` which uses a single global slider for all levels, this mod lets you customize gravity individually — e.g. low gravity on Warm-Up but heavy gravity on Dizzy.

## Config File

The mod auto-creates `local_gravity_set.txt` next to `Hamsterball.exe` on first launch if it doesn't exist. The file has 30 lines:

```
# --- Races (lines 1-15) ---
0.5    # Warm-Up Race
0.5    # Beginner Race
0.5    # Intermediate Race
0.5    # Dizzy Race
0.5    # Tower Race
0.5    # Up Race
0.5    # Neon Race (Dark)
0.5    # Expert Race
0.5    # Odd Race
0.5    # Toob Race
0.5    # Wobbly Race
0.5    # Glass Race
0.5    # Sky Race
0.5    # Master Race
0.5    # Impossible Race
# --- Arenas (lines 16-30) ---
0.5    # Warm-Up Arena
0.5    # Beginner Arena
0.5    # Intermediate Arena
0.5    # Dizzy Arena
0.5    # Tower Arena
0.5    # Up Arena
0.5    # Neon Arena
0.5    # Expert Arena
0.5    # Odd Arena
0.5    # Toob Arena
0.5    # Wobbly Arena
0.5    # Sky Arena
0.5    # Master Arena
0.5    # Glass Arena
0.5    # Impossible Arena
```

### Values

| Value | Effect |
|-------|--------|
| 0.5 | Normal gravity (game default) |
| 0.125 | Low gravity (25% of normal, floaty) |
| 1.0 | Double gravity |
| 2.0 | Heavy gravity |
| -0.5 | Reversed gravity (upside down) |

Note: The value is used as `ball->spin_rate` (gravity scale). The game's default spin_rate is 5.0. Negative values reverse the gravity direction vector.

## How It Works

1. On `onLevelStart`, re-reads `local_gravity_set.txt`
2. On `onBallUpdate`, identifies the current level by matching `scene->name` against the 30 known scene names
3. Reads the per-level gravity value from the array
4. Writes gravity direction vectors on `physics_object` (same as `plus_low_gravity`)
5. Sets `ball->spin_rate` to `|gravityValue|` as gravity scale

## Key Offsets

| Offset | Field | Purpose |
|--------|-------|---------|
| `ball+0x1A4` | physics_object ptr | Dereferenced for gravity vectors |
| `physics+0x8C` | gravity_x | X-axis gravity (Odd Race walls) |
| `physics+0x90` | gravity_y | Y-axis gravity (normal levels) |
| `physics+0x94` | gravity_z | Z-axis gravity (Odd Race flat) |
| `ball+0x2A4` | spin_rate | Gravity scale (default 5.0) |
| `scene->name` | scene name | Level identification |

## Level Identification

Scene names matched (case-sensitive):

**Races:** `Board (Warm-Up)`, `Board (Beginner)`, `Board (Intermediate)`, `Board (Dizzy)`, `Board (Tower)`, `Board (Up)`, `Board (Dark)`, `Board (Expert)`, `Board (Odd)`, `Board (Toob)`, `Board (Wobbly)`, `Board (Glass)`, `Board (Sky)`, `Board (Master)`, `Board (Impossible)`

**Arenas:** `RumbleBoard (Warmup Arena)`, `RumbleBoard (Beginner Arena)`, `RumbleBoard (Intermediate Arena)`, `RumbleBoard (Dizzy Arena)`, `RumbleBoard (Tower Arena)`, `RumbleBoard (Up Arena)`, `RumbleBoard (Neon Arena)`, `RumbleBoard (Expert Arena)`, `RumbleBoard (Odd Arena)`, `RumbleBoard (Toob Arena)`, `RumbleBoard (Wobbly Arena)`, `RumbleBoard (Sky Arena)`, `RumbleBoard (Master Arena)`, `RumbleBoard (Glass Arena)`, `RumbleBoard (Impossible Arena)`

## Building

### Visual Studio (primary)
1. Open Visual Studio with the Hamsterball Plus mod template
2. Replace `MainModFile.cpp` with `LocalGravity.cpp`
3. Build as 32-bit DLL (output name doesn't matter, the API loads by export)
4. Place the compiled `.dll` in the `Mods\` folder

### MinGW cross-compile (alternative)
```bash
i686-w64-mingw32-g++ -shared -o plus_local_gravity.dll LocalGravity_MinGW.cpp nocrt.cpp \
  -I. -O2 -msse2 -mfpmath=sse -mwindows \
  -fno-exceptions -fno-rtti -fno-threadsafe-statics \
  -fno-asynchronous-unwind-tables -fno-unwind-tables \
  -nostdlib -nostartfiles \
  -lkernel32 -luser32 \
  -Wl,-e,_DllMain@12 -Wl,--enable-stdcall-fixup \
  -Wl,--image-base,0x10000000 -Wl,--gc-sections \
  -ffunction-sections -fdata-sections \
  -fpermissive -fno-builtin \
  -Wl,--exclude-symbols,_strcmp -Wl,--exclude-symbols,_strlen \
  -Wl,--exclude-symbols,_memcpy -Wl,--exclude-symbols,_memset \
  -Wl,--exclude-symbols,_malloc -Wl,--exclude-symbols,_free
```

The MinGW build requires three fixes (all included):
1. **nocrt** (`nocrt.h`/`nocrt.cpp`) — eliminates msvcrt.dll dependency
2. **Manual 16-entry vtable** — fixes MinGW/MSVC ABI mismatch
3. **`hbplus_api.h` wrapper** — fixes IModAPI vtable dispatch

## Controls

- **Toggle:** Options menu → "Local Gravity" → YES/NO

## Difference from `plus_low_gravity`

| Feature | `plus_low_gravity` | `plus_local_gravity` |
|---------|--------------------|-----------------------|
| Scope | Global (all levels) | Per-level (30 entries) |
| Config | In-game slider | `local_gravity_set.txt` file |
| Reload | N/A | Every level start |
| Toggle | N/A (slider) | YES/NO button |
