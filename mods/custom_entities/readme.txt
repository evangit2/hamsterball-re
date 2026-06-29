===============================================================
 CUSTOM ENTITIES — Lua Scripting for Hamsterball
 readme.txt
===============================================================

WHAT IS THIS?
-------------
This mod embeds a Lua 5.1.5 interpreter into Hamsterball via the
bass.dll proxy. It lets you write custom Lua scripts that control
the behavior of objects placed in .meshworld level files.

Any object whose name starts with "L:" in a .meshworld file will
be detected and its corresponding Lua script will be loaded and
run every frame.


HOW TO INSTALL
--------------
1. Rename your original bass.dll to bass_real.dll
2. Copy the mod's bass.dll into the game directory
3. A "Lua" folder will be created automatically on first launch


HOW TO USE
----------
1. Open your .meshworld file in a hex editor or level editor
2. Add a reference point (object) with a name starting with "L:"
   For example:  L:move
3. Create a Lua script in the Lua/ folder with the same name:
   Lua/move.lua
4. Write an update function in that script:

   function update(entity_id, dt)
       -- your code here
   end

5. The update function is called every frame.
   - entity_id: a number identifying this object (1-indexed)
   - dt: delta time in seconds since last frame

If no Lua/xxx.lua file exists for an "L:xxx" object, the object
stays static with no behavior.


LUA API FUNCTIONS
----------------
All functions are accessed through the "hamsterball" table.

--- POSITION ---
hamsterball.get_position(entity_id)
  Returns: x, y, z (three floats)
  Gets the current position of the entity.

hamsterball.set_position(entity_id, x, y, z)
  Sets the position of the entity.
  x, y, z: floats (new position)

--- ROTATION ---
hamsterball.get_rotation(entity_id)
  Returns: x, y, z (three floats, in radians)
  Gets the current rotation of the entity.

hamsterball.set_rotation(entity_id, x, y, z)
  Sets the rotation of the entity.
  x, y, z: floats (radians)

--- SCALE ---
hamsterball.get_scale(entity_id)
  Returns: sx, sy, sz (three floats, default 1.0)
  Gets the current scale of the entity.

hamsterball.set_scale(entity_id, sx, sy, sz)
  Sets the scale of the entity.
  sx, sy, sz: floats (1.0 = original size)

--- BALL ---
hamsterball.get_ball_pos(ball_index)
  Returns: x, y, z (three floats)
  Gets the position of a ball.
  ball_index: 0 = player 1, 1 = player 2, etc.
  (default: 0 if omitted)

--- TIME ---
hamsterball.get_delta_time()
  Returns: float (seconds since last frame)

hamsterball.get_frame_count()
  Returns: integer (current scene frame number)

--- ENTITIES ---
hamsterball.get_entity_count()
  Returns: integer (total L: entities found in this level)

hamsterball.get_entity_name(entity_id)
  Returns: string (entity name without "L:" prefix)

hamsterball.find_entity(name)
  Returns: entity_id (number) or 0 if not found
  Find an entity by its name (without "L:" prefix)

--- LOGGING ---
hamsterball.log(message)
  Writes a line to Lua/log.txt
  message: string


EXAMPLE SCRIPTS
---------------
The following example scripts are included in the Lua/ folder:

move.lua    - Moves "L:move" objects along a configurable path
              Supports 4 modes: x, y, z, or circle
              Configurable distance and speed

rotate.lua  - Rotates "L:rotate" objects around configurable axes
              Spins faster when the ball is nearby
              Configurable per-axis rotation speeds

scale.lua   - Pulses "L:scale" objects in a sine wave
              Grows larger when the ball approaches
              Configurable base scale, pulse amplitude, speed

myobj.lua   - Basic example: floating up and down in a sine wave

spinner.lua - Basic example: rotates at 90 deg/sec,
              3x faster when ball is within 200 units


EXAMPLE CODE
------------

-- Make an object float up and down
local time = 0
local base_y = nil

function update(entity_id, dt)
    time = time + dt
    local x, y, z = hamsterball.get_position(entity_id)
    if not base_y then base_y = y end
    hamsterball.set_position(entity_id, x, base_y + math.sin(time * 2) * 50, z)
end

-- Rotate an object based on ball proximity
local angle = 0

function update(entity_id, dt)
    angle = angle + dt * 90
    angle = angle % 360
    local rad = angle * (math.pi / 180)
    hamsterball.set_rotation(entity_id, 0, rad, 0)
end

-- Scale an object in a pulse
local t = 0

function update(entity_id, dt)
    t = t + dt
    local s = 1.0 + math.sin(t * 3) * 0.3
    hamsterball.set_scale(entity_id, s, s, 1.0)
end


LIMITATIONS
-----------
- Objects are visual/positional only. They do not have collision
  meshes or physics by default.
- Position writes may be overridden by the game's own update logic
  if the object has a vtable update function.
- The Lua state is shared across all entities. Global variables
  are visible to all scripts.
- Maximum 64 concurrent Lua-scripted objects per level.
- Entity scanning happens every 60 frames (~1 second). New objects
  added to the scene will be picked up within 1 second.


TECHNICAL DETAILS
-----------------
Lua version:    5.1.5 (compiled into bass.dll)
DLL size:       ~624 KB
Hook target:    Scene_Update at 0x419C00
Object name:    stored at obj+0x50 (char pointer)
Object position: stored at obj+0x454/0x458/0x45C (Vec3 floats)
Object rotation: stored at obj+0x468/0x46C/0x470 (Vec3 floats)
Object scale:   stored at obj+0x460 and obj+0x474 (Vec3 w-components)
Build requires: i686-w64-mingw32-gcc (MinGW cross-compiler)
