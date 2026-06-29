-- myobj.lua — Example custom entity script
-- 
-- This script controls any object named "L:myobj" in a .meshworld file.
-- It demonstrates the basic API: making an object float up and down.

local time = 0

function update(entity_id, dt)
    time = time + dt
    
    -- Get current position
    local x, y, z = hamsterball.get_position(entity_id)
    
    -- Make the object float up and down in a sine wave
    local base_y = y
    if not _base_y then
        _base_y = y
    end
    local new_y = _base_y + math.sin(time * 2.0) * 50.0
    
    -- Apply the new position
    hamsterball.set_position(entity_id, x, new_y, z)
end
