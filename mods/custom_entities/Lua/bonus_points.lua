-- bonus_points.lua — Give the player bonus points on contact
--
-- Any object named "L:bonus_points" in a .meshworld file becomes a
-- bonus point pickup. When the ball touches it, the specified amount
-- of points is added to the player's score.
--
-- This works similar to the E:ACTION(SCORE) event in the original game.

-- ═══════════════════════════════════════════════════════════════════════
-- CONFIGURATION
-- ═══════════════════════════════════════════════════════════════════════

-- Points to award when the ball touches this object
local points = 500

-- How close the ball needs to be to trigger (in game units)
local touch_radius = 80.0

-- Which ball to check (0 = player 1)
local ball_index = 0

-- Whether this is a one-time pickup (true) or repeatable (false)
local one_time = true

-- ═══════════════════════════════════════════════════════════════════════
-- INTERNAL STATE
-- ═══════════════════════════════════════════════════════════════════════

local collected = false
local cooldown = 0

function update(entity_id, dt)
    -- Cooldown for repeatable pickups
    if cooldown > 0 then
        cooldown = cooldown - dt
        return
    end

    -- Already collected (one-time mode)
    if collected and one_time then
        return
    end

    -- Check distance to ball
    local dist = hamsterball.distance_to_ball(entity_id, ball_index)

    if dist >= 0 and dist < touch_radius then
        -- Ball is touching — award points!
        hamsterball.add_score(points, ball_index)

        if one_time then
            collected = true
            -- Move the object far away so it visually "disappears"
            local x, y, z = hamsterball.get_position(entity_id)
            hamsterball.set_position(entity_id, 0, -99999, 0)
        else
            -- Repeatable: set cooldown
            cooldown = 2.0
        end
    end
end
