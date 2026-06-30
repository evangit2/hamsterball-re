-- 8ball.lua — Recreated BadBall (8-Ball) AI Behavior
--
-- Any object named "L:8ball" in a .meshworld file becomes an AI-controlled
-- enemy ball that mimics the original BadBall behavior from
-- Ball_AI_ChaseNearest (vtable[4], 0x408390).
--
-- ═══════════════════════════════════════════════════════════════════════════════
-- ORIGINAL AI LOGIC (reverse-engineered from the binary)
-- ═══════════════════════════════════════════════════════════════════════════════
--
-- The BadBall has three behavioral states, checked every frame:
--
--   1. ORBIT — When far from the player (beyond CHASE + HOME range), the ball
--      circles around its "home position" (where it was placed in the MESHWORLD):
--          target.x = sin(spin_angle) * SPINDISTANCE + home.x
--          target.z = cos(spin_angle) * SPINDISTANCE + home.z
--      spin_angle increments by 2.0 radians every frame (constant).
--      This only activates when distance from home < 220.0 (orbit threshold).
--
--   2. CHASE — When a player ball is found and:
--        dist_from_home < HOME  AND  nearest_player_dist < CHASE
--      The target switches to the player's position. The ball drives toward them.
--
--   3. FLEE — Same as CHASE, but if the player's radius is much bigger
--      (own_radius < player_radius * 0.667), the target is inverted — the ball
--      runs away from the player instead.
--
--   The ball then moves toward the computed target by normalizing the direction
--   vector and applying force. We simulate this by directly setting position
--   toward the target at a speed proportional to max_speed.
--
--   On contact with the player, the original 8-ball kills them (knockoff).
--
-- ═══════════════════════════════════════════════════════════════════════════════
-- CONFIGURATION — Match these to the MESHWORLD BADBALL tags you'd use
-- ═══════════════════════════════════════════════════════════════════════════════

-- CHASE distance: how close the player must be (in game units) before the
-- 8-ball abandons orbiting and starts chasing. Original Beginner = 300.
local CHASE = 300.0

-- HOME distance: the maximum range from the home position where chase behavior
-- is allowed. If the 8-ball wandered too far from home, it returns to orbit.
-- Original Beginner = 375.
local HOME = 375.0

-- SPINDISTANCE: radius of the circular orbit pattern around home position.
-- Original Beginner default = 50.0 (no MESHWORLD tag). Dizzy Race = 45.0.
local SPINDISTANCE = 50.0

-- Orbit threshold: only orbit when within this distance from home.
-- Original hardcoded constant = 220.0 (at 0x4CF550).
local ORBIT_THRESHOLD = 220.0

-- Spin speed: radians per frame. Original = 2.0 (at 0x4CF48C).
-- At 25fps this is ~50 rad/s ≈ 8 revolutions per second.
local SPIN_SPEED = 2.0

-- Flee ratio: flee if own_radius < player_radius * this value.
-- Original = 0.667 (at 0x4CF508). Since we don't have ball radius info
-- in Lua, we use a "size ratio" approximation (see FLEE_SIZE_RATIO below).
local FLEE_RATIO = 0.667

-- Movement speed: how fast the 8-ball moves toward its target (units/sec).
-- The original uses Ball_ApplyForceWithMultipliers internally, which applies
-- a force proportional to max_speed (default 6.0). We approximate by moving
-- directly toward the target at this speed.
local MOVE_SPEED = 180.0  -- tuned to feel like original (~6.0 * 30 units/frame at 30fps)

-- Kill radius: how close the player must be to get knocked off by the 8-ball.
local KILL_RADIUS = 70.0

-- Kill cooldown: seconds between kills to prevent rapid multi-trigger.
local KILL_COOLDOWN = 2.0

-- Which player ball to target (0 = player 1).
local TARGET_BALL_INDEX = 0

-- Score awarded on kill (original = 2000 * Difficulty_GetTimeModifier).
local KILL_SCORE = 1000

-- Whether to log debug info to Lua/log.txt
local DEBUG = false

-- ═══════════════════════════════════════════════════════════════════════════════
-- INTERNAL STATE
-- ═══════════════════════════════════════════════════════════════════════════════

local spin_angle = 0.0        -- accumulated spin angle (radians), like ball+0xC78
local home_x, home_y, home_z  -- home position (captured on first frame)
local initialized = false
local kill_timer = 0          -- cooldown timer for kills

-- ═══════════════════════════════════════════════════════════════════════════════
-- HELPERS
-- ═══════════════════════════════════════════════════════════════════════════════

local function log(msg)
    if DEBUG then
        hamsterball.log("[8ball] " .. msg)
    end
end

-- 2D distance (XZ plane), matching the original Math_FastDistance2D
local function dist2d(x1, z1, x2, z2)
    local dx = x2 - x1
    local dz = z2 - z1
    return math.sqrt(dx * dx + dz * dz)
end

-- Normalize a 2D vector (XZ), return x, z components
local function normalize2d(x, z)
    local len = math.sqrt(x * x + z * z)
    if len > 0.0001 then
        return x / len, z / len
    end
    return 0, 0
end

-- ═══════════════════════════════════════════════════════════════════════════════
-- AI UPDATE — Faithful recreation of Ball_AI_ChaseNearest pipeline
-- ═══════════════════════════════════════════════════════════════════════════════

function update(entity_id, dt)
    -- ── Step 0: Initialize home position on first frame ──────────────────
    -- In the original, CreateBadBall stores the MESHWORLD spawn position as
    -- the "home" position (ball+0xC60/0xC64/0xC68). We capture it here.
    if not initialized then
        home_x, home_y, home_z = hamsterball.get_position(entity_id)
        if home_x == 0 and home_y == 0 and home_z == 0 then
            -- Position not ready yet, skip this frame
            return
        end
        initialized = true
        log("Initialized at home (" .. home_x .. ", " .. home_y .. ", " .. home_z .. ")")
    end

    -- ── Kill cooldown ─────────────────────────────────────────────────────
    if kill_timer > 0 then
        kill_timer = kill_timer - dt
    end

    -- ── Step 6: Spin angle increment (always, every frame) ───────────────
    -- Original: param_1[0x31e] += _DAT_004cf48c  (+= 2.0 rad/frame)
    -- This is frame-rate independent in the original (constant per frame,
    -- not per second). We scale by dt to remain consistent at any FPS.
    spin_angle = spin_angle + SPIN_SPEED * (dt * 60.0)  -- normalize to 60fps frames
    -- Keep angle in a reasonable range to avoid float precision loss
    if spin_angle > 100000.0 then
        spin_angle = spin_angle % (2.0 * math.pi)
    end

    -- ── Get current position ────────────────────────────────────────────
    local cur_x, cur_y, cur_z = hamsterball.get_position(entity_id)
    if cur_x == 0 and cur_y == 0 and cur_z == 0 then
        return  -- not ready
    end

    -- ── Step 3: Compute distance from home (2D, XZ plane) ────────────────
    local dist_from_home = dist2d(cur_x, cur_z, home_x, home_z)

    -- ── Step 4: Find nearest player ball ─────────────────────────────────
    -- Original filters: ball active (+0x768), not falling (+0x2F9=0),
    -- not special (+0x300=0), not flagged (+0x324=0), race active
    -- (scene+0x3A4C≠0), has player index (+0x18≠-1).
    -- We simplify: just get the target player's position.
    local ball_x, ball_y, ball_z = hamsterball.get_ball_pos(TARGET_BALL_INDEX)

    local has_target = false
    local nearest_dist = 999999.0

    if ball_x ~= 0 or ball_y ~= 0 or ball_z ~= 0 then
        nearest_dist = dist2d(cur_x, cur_z, ball_x, ball_z)
        has_target = true
    end

    -- ── Step 5/7: Compute target position ────────────────────────────────
    -- Default: orbit around home position
    --   target_x = sin(spin_angle) * SPINDISTANCE + home_x
    --   target_z = cos(spin_angle) * SPINDISTANCE + home_z
    local target_x, target_z

    local in_chase_range = has_target
        and dist_from_home < HOME
        and nearest_dist < CHASE

    if in_chase_range then
        -- ── Step 7: Chase/flee override ──────────────────────────────────
        -- Original: if own_radius < player_radius * 0.667, FLEE (invert target)
        -- We don't have radius in Lua, so we approximate: flee is disabled by
        -- default. To enable flee, set FLEE_ENABLED = true below and the ball
        -- will flee when the player is very close (< KILL_RADIUS * 2).
        -- In the original, this makes smaller 8-balls run from bigger players.
        local FLEE_ENABLED = false

        if FLEE_ENABLED and nearest_dist < KILL_RADIUS * 2.0 then
            -- FLEE: move away from player
            target_x = cur_x - (ball_x - cur_x)
            target_z = cur_z - (ball_z - cur_z)
            log("FLEEING from player (dist=" .. nearest_dist .. ")")
        else
            -- CHASE: target = player position
            target_x = ball_x
            target_z = ball_z
            log("Chasing player (dist=" .. nearest_dist .. ")")
        end
    else
        -- ── Step 5: Orbit computation ─────────────────────────────────────
        -- Original: if dist_from_home < 220.0, orbit around home
        -- If beyond 220, the ball just heads toward home (target = home)
        if dist_from_home < ORBIT_THRESHOLD then
            target_x = math.sin(spin_angle) * SPINDISTANCE + home_x
            target_z = math.cos(spin_angle) * SPINDISTANCE + home_z
            log("Orbiting home (dist=" .. dist_from_home .. ", angle=" .. spin_angle .. ")")
        else
            -- Too far from home — return to home position
            target_x = home_x
            target_z = home_z
            log("Returning home (dist=" .. dist_from_home .. ")")
        end
    end

    -- ── Step 8: Apply force toward target ────────────────────────────────
    -- Original: normalize (target - pos), call vtable[5](dx, 0, dz)
    -- We move directly toward target at MOVE_SPEED.
    local dx = target_x - cur_x
    local dz = target_z - cur_z
    local nx, nz = normalize2d(dx, dz)

    local new_x = cur_x + nx * MOVE_SPEED * dt
    local new_z = cur_z + nz * MOVE_SPEED * dt

    -- Keep Y at original height (the 8-ball stays on its ground plane)
    hamsterball.set_position(entity_id, new_x, cur_y, new_z)

    -- ── Kill check: if player is within KILL_RADIUS, knock them off ──────
    -- Original: the 8-ball kills via collision dispatch + physics overlap.
    -- We simulate by checking distance and calling kill_ball.
    if has_target and kill_timer <= 0 then
        -- Use full 3D distance for kill check (ball can be above/below)
        local dxk = ball_x - cur_x
        local dyk = ball_y - cur_y
        local dzk = ball_z - cur_z
        local kill_dist = math.sqrt(dxk * dxk + dyk * dyk + dzk * dzk)

        if kill_dist < KILL_RADIUS then
            log("KILL! Player hit by 8-ball (dist=" .. kill_dist .. ")")
            hamsterball.kill_ball(TARGET_BALL_INDEX)
            hamsterball.add_score(KILL_SCORE, TARGET_BALL_INDEX)
            kill_timer = KILL_COOLDOWN
        end
    end
end
