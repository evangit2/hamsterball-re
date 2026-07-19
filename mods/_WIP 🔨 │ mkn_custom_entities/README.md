# Custom Entities Mod v23

A bass.dll proxy mod for Hamsterball that spawns custom meshes and entities from MESHWORLD level files.

## Features

### 1. Testcube Mesh Spawning (GRID)

Spawns testcube meshes at S1 ref point positions marked with `(GRIDxx)` in the level's MESHWORLD file. Each GRID gets a different diffuse color (Red, Orange, Yellow, Green, Blue).

### 2. 8-Ball Customization (`<MESH>` / `<SPEEDMULT>`)

Customize BADBALL 8-balls from the MESHWORLD file using tags:

#### `<MESH>` — pick 8-ball mesh model

```
BADBALL<CHASE>100</CHASE><HOME>400</HOME><MESH>funball</MESH>
```

- If the value contains `"funball"`, the ball uses the **FunBall** mesh + texture (mesh index 10)
- Otherwise, the ball uses the default **8Ball** mesh (mesh index 9)

> **Note:** The `<MESH>` tag for 8-balls is currently **disabled** (has no effect). It will be re-enabled in a future version.

#### `<SPEEDMULT>` — multiply 8-ball speed

```
BADBALL<CHASE>100</CHASE><HOME>400</HOME><SPEEDMULT>2.0</SPEEDMULT>
```

- Multiplies the ball's `max_speed` (ball+0x188, default 6.0) by the given float value
- `<SPEEDMULT>2.0</SPEEDMULT>` → max_speed = 12.0 (twice as fast)
- `<SPEEDMULT>0.5</SPEEDMULT>` → max_speed = 3.0 (half speed)
- Clamped to range 0.01–100.0

### 3. Rotater Entity Spawning (REF:Rotater)

Spawns rotating platform objects (like the Dizzy Race SWIRL) at positions specified in the MESHWORLD file. The mod scans section-3 objects for entries named `Rotater` or `REF:Rotater` and spawns a `Rotator_ctor_Impossible` object at each position.

#### Rotater Arguments

Parse custom tags from the object name to control the spawned Rotater:

| Tag | Type | Description | Default |
|---|---|---|---|
| `<MESH>path</MESH>` | String | Custom mesh file path (relative to game dir) | `levels\Level3-Swirl` |
| `<rotX>0.0</rotX>` | Float | X-axis rotation speed (radians/frame) | `0.0` |
| `<rotY>0.004</rotY>` | Float | Y-axis rotation speed (radians/frame) | `0.004` (native SWIRL) |
| `<rotZ>0.0</rotZ>` | Float | Z-axis rotation speed (radians/frame) | `0.0` |

#### Example MESHWORLD Entry

```
Rotater<MESH>levels\Level3-Swirl</MESH><rotY>0.004</rotY>
```

#### How Rotation Works

- **`<rotY>`** — overrides the Y-axis rotation speed. The angle is written to `obj+0x10E8` each frame (~60fps) by the mod's entity thread.
- **`<rotX>` / `<rotZ>`** — stored in the config struct for future render hook support. Currently only Y-axis rotation is applied by the native render function.
- The native SWIRL rotates at `0.004` radians/frame (~5.73°/sec at 25fps, ~62.8s per full rotation).
- Rotation axis is **Y** (vertical/up-down axis).

#### SWIRL Rotation Technical Details (via Ghidra)

- **Angle field:** `obj+0x10E8` (float, init `0.0` practice / `-0.2` non-practice)
- **Direction field:** `obj+0x10EC` (float, init `1.0` practice / `0.0` non-practice)
- **Speed constant:** `0x004D5C88` = `0.004` (radians per frame)
- **Render function:** vtable[0] at `0x0043B310` — increments angle each frame: `angle += 0.004`
- **Vertex deformation:** vtable[1] `Rotator_Update` at `0x004606D0` — copies mesh vertices into a dynamic buffer and re-uploads
- **Constructor:** `Rotator_ctor_Impossible` at `0x00435940` — allocates 0x1508 bytes, stores position at `+0x10D8/D8/DC`

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
  -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
  -Wl,--add-stdcall-alias -msse2 -mfpmath=sse -lshlwapi
```

## Key Offsets (Verified via Ghidra)

| Offset | Description |
|--------|-------------|
| board+0x8AC | Level pointer |
| level+0x08 | MeshWorld pointer |
| level+0x480 | SceneObject pointer |
| sceneobj+0x894 | S1 ref point AthenaList |
| sceneobj+0xCA0 | Section-3 object array |
| sceneobj+0x898 | Section-3 object count |
| MeshWorld+0x24 | MeshBuffer count |
| MeshWorld+0x28 | RenderContext array |
| MeshWorld+0x2C | MeshBuffer AthenaList |
| MeshBuffer+0x864 | Name string pointer |
| obj+0x10E8 | Rotater Y rotation angle (float) |
| obj+0x10EC | Rotater rotation direction (float) |
| ball+0x188 | Ball max_speed (default 6.0) |
| ball+0x754 | Ball mesh index (9=8Ball, 10=FunBall) |

## Changelog

- **v23** — Added `<MESH>`, `<rotX>`, `<rotY>`, `<rotZ>` tag parsing for REF:Rotater entries. Per-frame Y rotation updates (~60fps). Custom mesh path support.
- **v22** — Fixed Rotater name matching: accept plain `Rotater` without `REF:` prefix.
- **v21** — Added REF:Rotater entity spawning (Dizzy SWIRL platform).
- **v20** — Fixed startup crash: added `load_real_bass()` to DllMain.
- **v19** — Added `<MESH>` and `<SPEEDMULT>` tags for 8-ball customization.
- **v17** — Initial testcube mesh spawning at GRID reference points.
