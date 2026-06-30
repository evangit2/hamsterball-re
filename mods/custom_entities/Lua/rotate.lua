-- rotate.lua — Dynamic rotation control for "L:rotate" objects
--
-- This script continuously rotates an object around configurable axes.
-- You can set different rotation speeds for X, Y, and Z.
--
-- The rotation also reacts to the ball's proximity — spinning faster
-- when the ball is near, creating an interactive element.

-- ═══════════════════════════════════════════════════════════════════════
-- CONFIGURATION
-- ═══════════════════════════════════════════════════════════════════════

-- Rotation speed for each axis (degrees per second)
local speed_x = 0.0     -- pitch
local speed_y = 90.0    -- yaw (spin around vertical axis)
local speed_z = 0.0     -- roll

-- Proximity boost: when the ball is within this range, multiply speed
local proximity_range = 300.0
local proximity_boost = 3.0  -- 3x faster when ball is close

-- ═══════════════════════════════════════════════════════════════════════
-- INTERNAL STATE
-- ═══════════════════════════════════════════════════════════════════════

local angle_x = 0
local angle_y = 0
local angle_z = 0

local function to_rad(deg)
    return deg * (math.pi / 180.0)
end

function update(entity_id, dt)
    -- Calculate proximity to ball
    local boost = 1.0
    local bx, by, bz = hamsterball.get_ball_pos(0)
    if bx ~= 0 or by ~= 0 or bz ~= 0 then
        local ex, ey, ez = hamsterball.get_position(entity_id)
        local dx = bx - ex
        local dy = by - ey
        local dz = bz - ez
        local dist = math.sqrt(dx*dx + dy*dy + dz*dz)
        if dist < proximity_range then
            boost = proximity_boost
        end
    end
    
    -- Accumulate angles
    angle_x = angle_x + speed_x * boost * dt
    angle_y = angle_y + speed_y * boost * dt
    angle_z = angle_z + speed_z * boost * dt
    
    -- Keep angles in [0, 360) to avoid float precision issues over time
    angle_x = angle_x % 360
    angle_y = angle_y % 360
    angle_z = angle_z % 360
    
    -- Apply rotation (convert degrees to radians)
    hamsterball.set_rotation(
        entity_id,
        to_rad(angle_x),
        to_rad(angle_y),
        to_rad(angle_z)
    )
end
