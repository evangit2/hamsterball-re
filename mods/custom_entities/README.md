# Custom Entities — Lua Scripting for Hamsterball

Embeds a Lua 5.1.5 interpreter into the game via the bass.dll proxy pattern, allowing custom scene objects in `.meshworld` files to have scripted behavior.

## How It Works

1. **On launch**: Creates a `Lua/` folder in the game directory (if it doesn't exist).
2. **Hook**: Detours `Scene_Update` (0x419C00) to run a per-frame Lua tick.
3. **Entity scan**: Scans the SpatialTree for objects whose name starts with `L:`.
4. **Script loading**: For each `L:xxx` object, loads `Lua/xxx.lua` (once per level load).
5. **Per-frame update**: Calls the Lua `update(entity_id, dt)` function every frame.
6. **Static fallback**: If `Lua/xxx.lua` doesn't exist, the object stays static with no behavior.

## Usage

### 1. Install the mod

- Rename your original `bass.dll` to `bass_real.dll`
- Copy this mod's `bass.dll` into the game directory

### 2. Create custom entities in MESHWORLD

In your `.meshworld` file, name a reference point with the `L:` prefix:

```
L:move
L:rotate
L:scale
```

### 3. Write or use Lua scripts

Create scripts in the `Lua/` folder matching the entity names (without the `L:` prefix). See the example scripts below.

### 4. Play

Launch the game. Objects with `L:` names will have their `update()` called every frame.

## Lua API

All functions are in the `hamsterball` table:

| Function | Returns | Description |
|----------|---------|-------------|
| `hamsterball.get_position(id)` | x, y, z | Get entity position (floats) |
| `hamsterball.set_position(id, x, y, z)` | — | Set entity position |
| `hamsterball.get_rotation(id)` | x, y, z | Get entity rotation (radians) |
| `hamsterball.set_rotation(id, x, y, z)` | — | Set entity rotation |
| `hamsterball.get_scale(id)` | sx, sy, sz | Get entity scale (default 1.0) |
| `hamsterball.set_scale(id, sx, sy, sz)` | — | Set entity scale |
| `hamsterball.get_ball_pos(index)` | x, y, z | Get ball position (0=player 1) |
| `hamsterball.get_delta_time()` | float | Seconds since last frame |
| `hamsterball.get_frame_count()` | int | Current scene frame number |
| `hamsterball.get_entity_count()` | int | Number of L: entities found |
| `hamsterball.get_entity_name(id)` | string | Name of entity (without L: prefix) |
| `hamsterball.find_entity(name)` | id or 0 | Find entity by name |
| `hamsterball.log(msg)` | — | Write to `Lua/log.txt` |

### Entity IDs

Entity IDs are 1-indexed integers assigned during the SpatialTree scan. They persist for the duration of a level and may change on level transition.

## Example Scripts

### `Lua/move.lua` — Dynamic Position
Moves an object along a configurable path. Supports four movement modes:
- `"x"` — back and forth along X axis
- `"y"` — up and down along Y axis
- `"z"` — back and forth along Z axis
- `"circle"` — circular movement in the XZ plane

Configurable distance and speed. Default: circle pattern, 200 unit radius, 0.5 Hz.

### `Lua/rotate.lua` — Dynamic Rotation
Continuously rotates an object around configurable axes (X=pitch, Y=yaw, Z=roll).
Spins faster (3x) when the ball is within 300 units. Configurable per-axis speeds.

### `Lua/scale.lua` — Dynamic Scaling
Pulses the object's scale in a sine wave pattern. Grows larger when the ball
approaches (proximity-based scaling up to 2x). Configurable base scale, pulse
amplitude, pulse speed, and proximity range.

### `Lua/myobj.lua` — Floating Object (basic example)
Simple sine-wave floating up and down.

### `Lua/spinner.lua` — Rotating Object (basic example)
Rotates at 90°/sec, spinning 3x faster when the ball is within 200 units.

## Technical Details

- **Lua version**: 5.1.5 (compiled into bass.dll, ~624KB total)
- **Hook target**: `Scene_Update` at 0x419C00 (7-byte entry detour)
- **Entity scan**: Reads SpatialTree object list at `root+0xCB0` (count) / `root+0x10B8` (data)
- **Object name**: At `obj+0x50` (char pointer)
- **Object position**: At `obj+0x454/0x458/0x45C` (Vec3 floats, verified from SceneObject_BaseInit)
- **Object rotation**: At `obj+0x468/0x46C/0x470` (Vec3 floats)
- **Object scale**: At `obj+0x460` (position w-component) and `obj+0x474` (rotation w-component)
- **Rescan interval**: Every 60 frames (~1 second at 60fps) to catch level changes
- **Max entities**: 64 concurrent Lua-scripted objects

### Build

```bash
bash build.sh
```

Requires `i686-w64-mingw32-gcc` (MinGW cross-compiler). Lua source is included in `lua/src/`.

## Limitations

- The SpatialTree scan finds named reference points, not full SceneObjects with collision. Objects are visual/positional only — they don't have collision meshes or physics by default.
- Position writes may be overridden by the game's own update logic if the object has a vtable update function that repositions it.
- The Lua state is shared across all entities — global variables are visible to all scripts.
- File I/O in Lua (io library) writes to the game's working directory, not necessarily the game install directory.

## Crash Test

Tested via hbtestd: 18.7s runtime, no crash. ✓
