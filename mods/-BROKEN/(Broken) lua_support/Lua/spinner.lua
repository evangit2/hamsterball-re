-- spinner.lua — Rotating entity script
--
-- Any object named "L:spinner" in a .meshworld file will slowly rotate.

local angle = 0

function update(entity_id, dt)
    angle = angle + dt * 90.0  -- 90 degrees per second
    
    if angle > 360 then
        angle = angle - 360
    end
    
    -- Convert to radians and set rotation
    local rad = angle * (math.pi / 180.0)
    local x, y, z = hamsterball.get_rotation(entity_id)
    hamsterball.set_rotation(entity_id, rad, y, z)
    
    -- Also track the ball position
    local bx, by, bz = hamsterball.get_ball_pos(0)
    local ex, ey, ez = hamsterball.get_position(entity_id)
    
    -- If ball is within 200 units, spin faster
    local dx = bx - ex
    local dy = by - ey
    local dz = bz - ez
    local dist = math.sqrt(dx*dx + dy*dy + dz*dz)
    
    if dist < 200 then
        angle = angle + dt * 270.0  -- 3x speed when ball is near
    end
end
