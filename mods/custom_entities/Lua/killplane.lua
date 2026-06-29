-- killplane.lua — Kill the player on contact
--
-- Any object named "L:killplane" in a .meshworld file becomes a deadly zone.
-- When the ball gets within the kill radius of this object, the player is
-- killed and respawns at the last checkpoint.
--
-- This works like a "floor is lava" hazard — place it anywhere in the
-- level and the ball will die when it touches the area.

-- ═══════════════════════════════════════════════════════════════════════
-- CONFIGURATION
-- ═══════════════════════════════════════════════════════════════════════

-- How close the ball needs to be to trigger the kill (in game units)
local kill_radius = 100.0

-- Which ball to kill (0 = player 1, 1 = player 2, etc.)
local ball_index = 0

-- Cooldown after a kill (in seconds) to prevent repeated triggers
local cooldown = 2.0

-- ═══════════════════════════════════════════════════════════════════════
-- INTERNAL STATE
-- ═══════════════════════════════════════════════════════════════════════

local timer = 0

function update(entity_id, dt)
    -- Count down cooldown timer
    if timer > 0 then
        timer = timer - dt
        return
    end

    -- Check distance to ball
    local dist = hamsterball.distance_to_ball(entity_id, ball_index)

    if dist >= 0 and dist < kill_radius then
        -- Ball is within kill radius — kill it!
        hamsterball.kill_ball(ball_index)
        timer = cooldown
    end
end
