# Custom Entities Mod v17

Spawns testcube meshes at S1 ref point positions marked with `(GRIDxx)` in the level's MESHWORLD file.

## v17 New: `<MESH>` tag for 8-ball mesh selection

Added a new `<MESH>` argument tag (similar to `<CHASE>`, `<HOME>`, `<SIZE>`) that lets you pick which mesh model an 8-ball (BADBALL) uses:

```
BADBALL<CHASE>100</CHASE><HOME>400</HOME><MESH>funball</MESH>
```

- If the `<MESH>` value contains `"funball"`, the ball uses the **FunBall** mesh + texture (mesh index 10)
- If no `<MESH>` tag is present, or the value doesn't contain "funball", the ball uses the default **8Ball** mesh (Sphere + 8ball texture, mesh index 9)

The mod processes `<MESH>` tags after the game's native `CreateBadBall` has spawned the 8-balls during level load. It matches BADBALL objects to spawned balls by their home position (which `CreateBadBall` copies directly from the MESHWORLD object's X/Y/Z).

## v9 Fix: MeshWorld Pointer Location

**Bug:** v8 looked for the MeshWorld pointer at `sceneobj+0x08` — wrong! The log showed `sceneobj+0x08=NULL, scanning for MeshWorld...` and then `no meshworld found`.

**Root cause:** Verified via Ghidra decompilation of `Scene_LoadMeshWorld` (0x461890):
```c
// "this" is the LEVEL object (stored at board+0x8AC)
*(undefined4 **)((int)this + 8) = puVar2;  // MeshWorld ptr at LEVEL+0x08
```

And `Level_MeshWorldCtor` (0x461510):
```c
// this+0x480 = SceneObject (separate allocation)
// LoadMeshWorld(this, param_2) — called with "this" = Level, NOT sceneobj
```

The MeshWorld pointer is at **level+0x08**, not sceneobj+0x08. The SceneObject (at level+0x480) contains the S1 ref point list at +0x894, but NOT the MeshWorld.

**Fix:** `get_meshworld()` now reads from `level+0x08` instead of `sceneobj+0x08`.

## How It Works

1. On level load, the mod reads S1 ref points from `sceneobj+0x894` (AthenaList)
2. For each S1 entry whose name contains `(GRIDxx)`, it reads the position (X, Y, Z)
3. It spawns a testcube mesh at that position by:
   - Allocating a MeshBuffer (0x874 bytes) via `CreateMeshBuffer`
   - Appending it to the MeshWorld's MeshBuffer list (MeshWorld+0x2C)
   - Writing position into the RenderContext array (MeshWorld+0x28 + index × 0x50)
   - Setting diffuse color based on GRID number (Red, Orange, Yellow, Green, Blue)

## Installation

1. Rename original `bass.dll` to `bass_real.dll`
2. Copy mod `bass.dll` to game root
3. Copy `testcube.MESHWORLD` to game root (or `Levels/` subfolder)
4. Copy `custom_entities.txt` to game root (auto-generated if missing)

## Configuration

`custom_entities.txt`:
```ini
# Ticks between grid advances (1 tick = ~16ms)
grid_speed = 10.0
```

## Building

```bash
cd source/
i686-w64-mingw32-gcc -shared -o bass.dll custom_entities.c bass.def \
  -lwinmm -Wl,--enable-stdcall-fixup -O2 -static-libgcc \
  -Wl,--add-stdcall-alias -msse2 -mfpmath=sse -lshlwapi
```

## Key Offsets (Verified via Ghidra)

| Offset | Description |
|--------|-------------|
| board+0x8AC | Level pointer |
| level+0x08 | **MeshWorld pointer** (v9 fix — was sceneobj+0x08) |
| level+0x480 | SceneObject pointer |
| sceneobj+0x894 | S1 ref point AthenaList |
| MeshWorld+0x24 | MeshBuffer count |
| MeshWorld+0x28 | RenderContext array |
| MeshWorld+0x2C | MeshBuffer AthenaList |
| MeshBuffer+0x864 | Name string pointer |
