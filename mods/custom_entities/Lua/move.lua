-- move.lua — Dynamic position control for "L:move" objects
--
-- This script moves an object along a configurable path.
-- The object oscillates between its starting position and a target offset.
--
-- You can customize the movement by editing the variables below.

-- ═══════════════════════════════════════════════════════════════════════
-- CONFIGURATION
-- ═══════════════════════════════════════════════════════════════════════

-- Movement axis: "x", "y", "z", or "circle"
local axis = "circle"

-- How far to move (in game units)
local distance = 200.0

-- Speed of oscillation (cycles per second)
local speed = 0.5

-- ═══════════════════════════════════════════════════════════════════════
-- INTERNAL STATE (don't edit below)
-- ═══════════════════════════════════════════════════════════════════════

local time = 0
local start_x, start_y, start_z = nil, nil, nil

function update(entity_id, dt)
    time = time + dt
    
    -- Store starting position on first frame
    if not start_x then
        start_x, start_y, start_z = hamsterball.get_position(entity_id)
    end
    
    local t = time * speed * math.pi * 2  -- full cycle per (1/speed) seconds
    
    if axis == "x" then
        -- Back and forth along X axis
        local offset = math.sin(t) * distance
        hamsterball.set_position(entity_id, start_x + offset, start_y, start_z)
        
    elseif axis == "y" then
        -- Up and down along Y axis
        local offset = math.sin(t) * distance
        hamsterball.set_position(entity_id, start_x, start_y + offset, start_z)
        
    elseif axis == "z" then
        -- Back and forth along Z axis
        local offset = math.sin(t) * distance
        hamsterball.set_position(entity_id, start_x, start_y, start_z + offset)
        
    elseif axis == "circle" then
        -- Circular movement in the XZ plane
        local offset_x = math.cos(t) * distance
        local offset_z = math.sin(t) * distance
        hamsterball.set_position(entity_id, start_x + offset_x, start_y, start_z + offset_z)
    end
end
