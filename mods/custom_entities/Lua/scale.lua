-- scale.lua — Dynamic scaling control for "L:scale" objects
--
-- This script dynamically scales an object using a pulsing effect.
-- The object grows and shrinks in a sine wave pattern.
-- It can also react to the ball's distance — growing larger when
-- the ball approaches, like a threat display.

-- ═══════════════════════════════════════════════════════════════════════
-- CONFIGURATION
-- ═══════════════════════════════════════════════════════════════════════

-- Base scale (1.0 = original size)
local base_scale = 1.0

-- Pulse amplitude (how much it grows/shrinks)
local pulse_amplitude = 0.3

-- Pulse speed (cycles per second)
local pulse_speed = 1.0

-- Proximity mode: grow when ball is near
local proximity_range = 250.0
local proximity_max_growth = 2.0  -- max scale when ball is touching

-- ═══════════════════════════════════════════════════════════════════════
-- INTERNAL STATE
-- ═══════════════════════════════════════════════════════════════════════

local time = 0

function update(entity_id, dt)
    time = time + dt
    
    -- Calculate pulse
    local pulse = math.sin(time * pulse_speed * math.pi * 2) * pulse_amplitude
    local sx = base_scale + pulse
    local sy = base_scale + pulse
    
    -- Clamp to positive values (negative scale flips geometry)
    if sx < 0.1 then sx = 0.1 end
    if sy < 0.1 then sy = 0.1 end
    
    -- Proximity growth
    local bx, by, bz = hamsterball.get_ball_pos(0)
    if bx ~= 0 or by ~= 0 or bz ~= 0 then
        local ex, ey, ez = hamsterball.get_position(entity_id)
        local dx = bx - ex
        local dy = by - ey
        local dz = bz - ez
        local dist = math.sqrt(dx*dx + dy*dy + dz*dz)
        
        if dist < proximity_range then
            -- Scale grows as ball gets closer (linear interpolation)
            local closeness = 1.0 - (dist / proximity_range)
            local proximity_scale = 1.0 + (proximity_max_growth - 1.0) * closeness
            sx = sx * proximity_scale
            sy = sy * proximity_scale
        end
    end
    
    hamsterball.set_scale(entity_id, sx, sy, 1.0)
end
