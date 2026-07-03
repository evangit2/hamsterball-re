# Custom Entities Mod

Adds new customizable `E:` and `N:` entity types to Hamsterball, with behaviours loaded from external DLL files.

## How It Works

1. Place behavior DLLs in a `Behaviours/` folder in the game root directory
2. Add entities with names like `E:Rotator` or `N:Rotator` to any `.MESHWORLD` file
3. The mod scans all loaded MeshBuffers for unknown `E:`/`N:` prefixed names
4. When a match is found, it loads `Behaviours/<EntityName>.dll` and calls its update function every frame

### Example

An entity named `E:Rotator` in a MESHWORLD file → loads `Behaviours/Rotator.dll` → the DLL's `Behavior_Update` function rotates the entity's transform every frame.

## Architecture

```
Game Root/
├── Hamsterball.exe
├── bass.dll              ← This mod (proxy DLL)
├── bass_real.dll         ← Original BASS audio library (renamed)
├── Behaviours/
│   └── Rotator.dll       ← Behavior DLL for E:Rotator entities
└── Levels/
    └── Level1.MESHWORLD   ← Contains E:Rotator entity
```

### Main Mod (`bass.dll`)

- **Non-invasive:** No code patches, no function hooks. Uses a background polling thread (`Sleep(16)` ≈ 60Hz).
- **Safe:** All pointer access guarded by `IsBadReadPtr`. Validates vtable pointers before use.
- **Automatic:** Detects level changes, scans for entities, loads DLLs, calls init/update/shutdown.

### Behavior DLL Interface

Each behavior DLL must export these three functions (C calling convention):

```c
// Called once when entity is first detected (level load)
__declspec(dllexport) void __cdecl Behavior_Init(EntityTransform* transform, void* board);

// Called every frame (~60fps) while the level is active
__declspec(dllexport) void __cdecl Behavior_Update(EntityTransform* transform, void* board);

// Called when the level ends (board destroyed)
__declspec(dllexport) void __cdecl Behavior_Shutdown(void);
```

### EntityTransform Structure (0x50 bytes)

Located at `MeshWorld+0x28 + meshBufferIndex * 0x50`. Verified from `Scene_LoadMeshWorld` (0x461890) decompilation.

| Offset | Type   | Field      | Description                     |
|--------|--------|------------|---------------------------------|
| +0x00  | DWORD  | vtable     | Internal vtable pointer          |
| +0x04  | float  | rotX       | Rotation X (radians)            |
| +0x08  | float  | rotY       | Rotation Y (radians)            |
| +0x0C  | float  | rotZ       | Rotation Z (radians)            |
| +0x10  | float  | rotScale   | Rotation scale (default 1.0)    |
| +0x14  | float  | posX       | Position X                      |
| +0x18  | float  | posY       | Position Y                      |
| +0x1C  | float  | posZ       | Position Z                      |
| +0x20  | float  | posScale   | Position scale (default 1.0)    |
| +0x24  | float  | extra[12]  | Additional transform data        |
| +0x4C  | BYTE   | hasRot     | 1 if rotation != default         |
| +0x4D  | BYTE   | hasExtra   | Extra flag                      |

The game's `MeshWorld_CollectRenderLists` (0x460FCC0) reads these values every frame to build render lists — so modifying `rotX`, `rotY`, `rotZ`, `posX`, `posY`, or `posZ` from a behavior DLL will immediately affect rendering.

### Known Game Entities (Not Intercepted)

The mod skips these entity names to avoid conflicts with the game's native event/collision dispatch:

**E: prefix events:** JUMP, NODIZZY, LIMIT, TELEPORT, WATER, DROPIN, PIPEBONK, POPOUT, VACPOPOUT, TARPIT, GOAL (1-6), BUMPER (1-8), FAN, SAWBLADE, JUDGE, BELL, LAUNCH, MOUSETRAP, TARBUBBLE, FALL, SWIRL, SPEEDPAD

**N: prefix entities:** GLASS

Any entity name NOT in this list will be treated as custom and loaded from `Behaviours/`.

**Note:** Mechanical objects like ROTATOR, GEAR, LOOPER, PENDULUM are only matched by the game's `CreateMechanicalObjects` (0x417FE0) when their name has NO `E:`/`N:` prefix. A mesh named `E:Rotator` does NOT trigger the native `Rotator_ctor` — so it's safe for our mod to handle it.

## Included Behaviors

### Rotator (`Behaviours/Rotator.dll`)

- **Entity name:** `E:Rotator` (or `N:Rotator`)
- **Effect:** Constantly rotates the entity on its X axis at ~0.02 radians/frame (~69°/sec at 60fps)
- **Source:** `behaviours/rotator.c`

## Installation

1. Rename original `bass.dll` to `bass_real.dll`
2. Copy mod `bass.dll` to game root
3. Create `Behaviours/` folder in game root
4. Copy behavior DLLs (e.g. `Rotator.dll`) into `Behaviours/`

## Building

### Main Mod (bass.dll)

```bash
i686-w64-mingw32-gcc -shared -o bass.dll custom_entities.c \
  -I../shared -lwinmm -Wl,--enable-stdcall-fixup -O2 -static \
  -static-libgcc -Wl,--add-stdcall-alias -msse2 -mfpmath=sse
```

### Behavior DLLs

```bash
i686-w64-mingw32-gcc -shared -o Rotator.dll rotator.c \
  -O2 -static -static-libgcc -msse2 -mfpmath=sse
```

## Writing Custom Behaviors

1. Create a new `.c` file (e.g. `my_behavior.c`)
2. Include `entity_api.h` (or define the `EntityTransform` struct manually)
3. Implement `Behavior_Init`, `Behavior_Update`, `Behavior_Shutdown`
4. Compile to `Behaviours/MyBehavior.dll`
5. Add `E:MyBehavior` to a `.MESHWORLD` file

### Example: Simple Oscillator

```c
#include <windows.h>

typedef struct {
    DWORD vtable; float rotX, rotY, rotZ, rotScale;
    float posX, posY, posZ, posScale;
    float extra[12];
} EntityTransform;

static float g_time = 0.0f;

__declspec(dllexport) void __cdecl Behavior_Init(EntityTransform* t, void* board) {
    g_time = 0.0f;
}

__declspec(dllexport) void __cdecl Behavior_Update(EntityTransform* t, void* board) {
    g_time += 0.02f;
    t->posY = t->posY + sinf(g_time) * 0.5f;  // Bob up and down
}

__declspec(dllexport) void __cdecl Behavior_Shutdown(void) {}
```

## Diagnostic Log

The mod writes a `custom_entities.log` file in the game root directory. This log shows:
- Every MeshBuffer name found during scanning
- Which entities were identified as custom
- Which behavior DLLs were loaded successfully
- Transform pointer addresses and initial values

## Files

| File | Description |
|------|-------------|
| `entity_api.h` | Shared header defining EntityTransform and behavior interface |
| `custom_entities.c` | Main mod source (bass.dll proxy) |
| `behaviours/rotator.c` | Rotator behavior DLL source |
| `behaviours/Rotator.dll` | Compiled Rotator behavior |
| `test/Level1-TestRotator.MESHWORLD` | Test level with E:Rotator entity |
| `bass.dll` | Compiled main mod |

## Technical Details

### Pointer Chain

```
App (0x5341E0)
  → +0x220 (PlayerProfile*)
    → +0x0C (Board*)
      → +0x878 (Scene*)
        → +0x08 (MeshWorld*)
          → +0x28 (EntityTransform[] array)
          → +0x2C (AthenaList<MeshBuffer>)
```

### MeshBuffer Structure (0x874 bytes)

| Offset | Type  | Field           | Description                     |
|--------|-------|-----------------|---------------------------------|
| +0x00  | DWORD | vtable          | MeshBuffer vtable               |
| +0x04  | DWORD | ctx_index       | Index into EntityTransform[]    |
| +0x860 | DWORD | name_length     | Length of name string           |
| +0x862 | BYTE  | is_object       | 1 if name starts with "O:"     |
| +0x863 | BYTE  | is_event        | 1 if name starts with "E:"      |
| +0x864 | char* | name            | Entity name string              |

### Safety Features

- `IsBadReadPtr` before every pointer dereference
- Pointer range validation (reject addresses < 0x10000)
- MeshBuffer count bounds check (1-10000)
- EntityTransform pointer validated before each `Behavior_Update` call
- Behavior DLL load failure is non-fatal (entity is simply skipped)
- Level change detection: entities are shut down and re-scanned on board change
