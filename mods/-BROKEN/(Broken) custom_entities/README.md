# Custom Entities Mod

Adds new customizable entities to Hamsterball using `CE:`, `E:`, and `N:` prefixed entity names in MESHWORLD files, with meshes and behaviours loaded from external files.

## How It Works

### CE: Prefix — Custom Entity Reference System

When an object named `CE:Rotator` is placed in a level `.MESHWORLD` file (from the `Levels/` folder), it acts as a **reference pointer**:

1. The object's **position, rotation, and scale** in the level define the initial transform
2. The **mesh geometry** is loaded from `CustomEntities/Rotator.MESHWORLD`
3. The **behavior** is loaded from `CustomEntities/Rotator.dll`
4. `Behavior_Update` is called every frame to animate the entity

This means level designers can place `CE:` objects in their levels to summon custom entities with full geometry and behavior, without modifying the level's own mesh data.

### E:/N: Prefix — Legacy Custom Entities

Entities named `E:Rotator` or `N:Rotator` work the same as before — behavior DLLs are loaded from `CustomEntities/<name>.dll`, but no separate mesh file is loaded. These are useful for simpler entities that only need transform animation.

### Example

```
Levels/Level1.MESHWORLD       → contains "CE:Rotator" at position (100, 50, 200)
CustomEntities/Rotator.MESHWORLD → mesh geometry for the Rotator object
CustomEntities/Rotator.dll    → behavior DLL that rotates on X axis
```

When the level loads, the mod:
1. Detects `CE:Rotator` in the **S1 ref point scan** (same pointer chain as `CreateMouseTrap`)
2. Verifies `CustomEntities/Rotator.MESHWORLD` exists
3. Loads `CustomEntities/Rotator.dll`
4. Calls `Behavior_Init` with the entity's transform (position/rotation from the S1 ref point)
5. Calls `Behavior_Update` every frame (~60Hz) — the DLL rotates the entity on X axis

### REF: Prefix Support

Some MESHWORLD files use `REF:CE:Rotator` instead of `CE:Rotator`. The mod automatically strips the `REF:` prefix before checking for `CE:`/`E:`/`N:` prefixes. This is handled in both the runtime S1 scanner and the DllMain-time meshworld merger.

## Architecture

```
Game Root/
├── Hamsterball.exe
├── bass.dll                    ← This mod (proxy DLL)
├── bass_real.dll               ← Original BASS audio library (renamed)
├── CustomEntities/
│   ├── Rotator.MESHWORLD       ← Mesh geometry for CE:Rotator
│   └── Rotator.dll             ← Behavior DLL for CE:Rotator
├── Levels/
│   └── Level1.MESHWORLD        ← Level file containing CE:Rotator reference
```

### Main Mod (`bass.dll`)

- **File-level mesh merging:** At DllMain time, scans `Levels/*.MESHWORLD` for `CE:` ref points in S1, loads matching `CustomEntities/<name>.MESHWORLD` files, and merges their vertices + octree geometry into the level file before the game loads it.
- **Non-invasive:** No code patches, no function hooks. Uses a background polling thread (`Sleep(16)` ≈ 60Hz) for behavior animation only.
- **Safe:** All pointer access guarded by `IsBadReadPtr`. Validates vtable pointers before use.
- **Automatic:** Detects level changes, scans for entities, loads DLLs, calls init/update/shutdown.
- **CE: support:** Checks for `CustomEntities/<name>.MESHWORLD` file existence before loading behavior.

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

### Entity Prefix Types

| Prefix | Type | Behavior | MESHWORLD file required? |
|--------|------|----------|--------------------------|
| `CE:`  | Custom Entity Reference | Loads mesh from `CustomEntities/<name>.MESHWORLD` + behavior DLL | Yes |
| `E:`   | Legacy custom entity | Loads behavior DLL only | No |
| `N:`   | Legacy custom entity | Loads behavior DLL only | No |

### Known Game Entities (Not Intercepted)

The mod skips these `E:`/`N:` entity names to avoid conflicts with the game's native event/collision dispatch:

**E: prefix events:** JUMP, NODIZZY, LIMIT, TELEPORT, WATER, DROPIN, PIPEBONK, POPOUT, VACPOPOUT, TARPIT, GOAL (1-6), BUMPER (1-8), FAN, SAWBLADE, JUDGE, BELL, LAUNCH, MOUSETRAP, TARBUBBLE, FALL, SWIRL, SPEEDPAD

**N: prefix entities:** GLASS

Any entity name NOT in this list will be treated as custom. **Note:** `CE:` prefix entities are NEVER checked against this list — the game doesn't recognize the `CE:` prefix at all, so all `CE:` entities are custom.

**Note:** Mechanical objects like ROTATOR, GEAR, LOOPER, PENDULUM are only matched by the game's `CreateMechanicalObjects` (0x417FE0) when their name has NO `E:`/`N:`/`CE:` prefix. A mesh named `CE:Rotator` does NOT trigger the native `Rotator_ctor` — so it's safe for our mod to handle it.

## Included Behaviors

### Rotator (`CustomEntities/Rotator.dll`)

- **Entity name:** `CE:Rotator` (or `E:Rotator`, `N:Rotator`)
- **Effect:** Constantly rotates the entity on its X axis at ~0.02 radians/frame (~69°/sec at 60fps)
- **Source:** `CustomEntities/rotator.c`
- **MESHWORLD:** `CustomEntities/Rotator.MESHWORLD` (mesh geometry)

## Installation

1. Rename original `bass.dll` to `bass_real.dll`
2. Copy mod `bass.dll` to game root
3. Create `CustomEntities/` folder in game root
4. Copy behavior DLLs (e.g. `Rotator.dll`) and `.MESHWORLD` files into `CustomEntities/`
5. Create `Levels/` folder in game root (if using custom level files)
6. Copy level `.MESHWORLD` files (e.g. `Level1.MESHWORLD`) into `Levels/`

## Building

### Main Mod (bass.dll)

```bash
i686-w64-mingw32-gcc -shared -o bass.dll custom_entities.c meshworld_merger.c \
  -I../shared -lwinmm -Wl,--enable-stdcall-fixup -O2 -static \
  -static-libgcc -Wl,--add-stdcall-alias -msse2 -mfpmath=sse -lshlwapi
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
4. Compile to `CustomEntities/MyBehavior.dll`
5. Create `CustomEntities/MyBehavior.MESHWORLD` (mesh geometry)
6. Add `CE:MyBehavior` to a `.MESHWORLD` file in `Levels/`

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
- Which entities were identified as custom (with prefix type: CE/E/N)
- Which .MESHWORLD files were found in CustomEntities/
- Which behavior DLLs were loaded successfully
- Transform pointer addresses and initial values (pos/rot/scale)

## Files

| File | Description |
|------|-------------|
| `entity_api.h` | Shared header defining EntityTransform and behavior interface |
| `custom_entities.c` | Main mod source (bass.dll proxy) |
| `meshworld_merger.c` | Runtime file merger — injects CE: entity meshes into level files |
| `CustomEntities/rotator.c` | Rotator behavior DLL source |
| `CustomEntities/Rotator.dll` | Compiled Rotator behavior |
| `CustomEntities/Rotator.MESHWORLD` | Rotator mesh geometry |
| `Levels/Level1.MESHWORLD` | Test level with CE:Rotator S1 ref point |
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
| +0x860 | DWORD | name_length     | Length of name string               |
| +0x862 | BYTE  | is_object       | 1 if name starts with "O:"     |
| +0x863 | BYTE  | is_event        | 1 if name starts with "E:"      |
| +0x864 | char* | name            | Entity name string              |

### Safety Features

- `IsBadReadPtr` before every pointer dereference
- Pointer range validation (reject addresses < 0x10000)
- MeshBuffer count bounds check (1-10000)
- EntityTransform pointer validated before each `Behavior_Update` call
- Behavior DLL load failure is non-fatal (entity is simply skipped)
- CE: entities require `CustomEntities/<name>.MESHWORLD` to exist
- Level change detection: entities are shut down and re-scanned on board change
