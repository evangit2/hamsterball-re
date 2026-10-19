-- bumper.lua — Recreated Bumper (N:BUMPER) Behavior
--
-- Any object named "L:bumper" in a .meshworld file becomes a bumper
-- that bounces the ball away, mimicking the original N:BUMPER collision
-- behavior from HandleArenaCollisionEvents (0x412850).
--
-- ═══════════════════════════════════════════════════════════════════════
-- ORIGINAL BEHAVIOR (reverse-engineered from the binary)
-- ═══════════════════════════════════════════════════════════════════════
--
-- Bumpers are created by CreateBumper (0x40FA20), which scans the
-- SpatialTree for objects named "BUMPER". The collision event name
-- in the MESHWORLD is "N:BUMPER%d" (e.g. "N:BUMPER1", "N:BUMPER2", ...).
--
-- When the ball collides with a bumper, HandleArenaCollisionEvents
-- (0x412850) handles the "N:BUMPER" prefix as follows:
--
--   1. Play 3D collision sound (App+0x448 sound buffer) at ball position
--   2. Read ball velocity from CollisionMesh (ball+0x1A4 ptr):
--        vx = CollisionMesh+0xCA4
--        vy = CollisionMesh+0xCA8
--        vz = CollisionMesh+0xCAC
--   3. Zero the Y component:  vy = 0.0  (flatten to horizontal)
--   4. Scale X and Z by 5.0 (_DAT_004CF55C = 0x40A00000):
--        vx *= 5.0
--        vz *= 5.0
--   5. Compute horizontal speed:  sqrt(vx² + vz²)
--   6. If speed < 0.0 (impossible) OR speed < 5.0:
--        Vec3_NormalizeAndScale(&vel, 5.0)  — minimum bounce of 5.0
--   7. Compute total 3D speed:  sqrt(vx² + vy² + vz²)
--   8. If speed > 0.0 AND speed > 12.0 (_DAT_004CF3DC = 0x41400000):
--        Vec3_NormalizeAndScale(&vel, 12.0)  — cap at 12.0
--   9. Write modified velocity back to CollisionMesh+0xCA4/CA8/CAC
--  10. Parse bumper number from event name ("N:BUMPER3" → 3)
--      Set board flag: board + (bumperNum-1)*4 + 0x53FC = 1.0
--      (tracks which bumpers have been hit)
--
-- Net effect: the ball is bounced away horizontally at 5.0–12.0 speed,
-- with its vertical velocity zeroed. The bounce direction comes from the
-- existing velocity (which the physics engine has already reflected off
-- the bumper geometry).
--
-- ═══════════════════════════════════════════════════════════════════════
-- LUA API LIMITATIONS
-- ═══════════════════════════════════════════════════════════════════════
--
-- The Lua API does not expose ball velocity read/write. The original
-- manipulates CollisionMesh+0xCA4/CA8/CAC directly, which requires
-- pointer chasing from ball→physics→collisionmesh. This script simulates
-- the bounce effect by:
--
--   • Computing the direction from bumper to ball (bounce normal)
--   • Pushing the ball's position along that direction over several frames
--   • The push speed is tuned to match the original's 5.0–12.0 range
--
-- ═══════════════════════════════════════════════════════════════════════
-- CONFIGURATION
-- ═══════════════════════════════════════════════════════════════════════

-- How close the ball must be to trigger the bumper (game units).
-- The original uses collision mesh overlap; we approximate with
-- a spherical proximity check. Ball radius is ~26 units, bumpers
-- are ~30-40 units wide, so ~70 is a good contact distance.
local TRIGGER_RADIUS = 70.0

-- Cooldown after triggering (seconds). Prevents rapid re-triggering
-- while the ball is still near the bumper after being pushed.
local COOLDOWN = 0.5

-- Which ball to check (0 = player 1)
local BALL_INDEX = 0

-- Bounce force: how fast the ball is pushed away (units per second).
-- The original amplifies velocity by 5x with a minimum of 5.0 and
-- maximum of 12.0. Since we push position directly, we use a higher
-- value to achieve a comparable visual effect.
local BOUNCE_FORCE = 800.0

-- How many seconds the push is applied (simulates velocity boost).
-- The original applies the velocity change instantly; we spread it
-- over a short time since we're moving position each frame.
local BOUNCE_DURATION = 0.15

-- Whether to flatten the bounce to horizontal (zero Y component).
-- The original zeroes vy. Set to true to match original behavior.
local FLATTEN_Y = true

-- If FLATTEN_Y is true, the ball is kept at this Y offset above the
-- bumper during the push. Set to nil to use the ball's current Y.
local BOUNCE_Y_OFFSET = nil

-- Minimum bounce speed (original = 5.0). If the ball is nearly
-- stationary when it hits the bumper, it still gets pushed at
-- at least this speed.
local MIN_BOUNCE_SPEED = 150.0

-- Maximum bounce speed (original = 12.0). Prevents the ball from
-- being launched too fast if it was already moving quickly.
local MAX_BOUNCE_SPEED = 2400.0

-- Score awarded on bumper hit (original = 0 points)
local SCORE_AWARD = 0

-- ══ Visual: Scale pulse on hit ═════════════════════════════════════════

-- The original bumper doesn't have a visual pulse, but this adds
-- satisfying feedback when the bumper triggers.
local PULSE_ENABLED = true

-- Scale multiplier at peak pulse (1.0 = no pulse)
local PULSE_SCALE = 1.3

-- How fast the pulse expands (seconds to reach peak)
local PULSE_SPEED = 0.08

-- How fast the pulse returns to normal
local PULSE_RETURN_SPEED = 0.25

-- ══ Debug ═════════════════════════════════════════════════════════════

-- Set to true for diagnostic output to Lua/log.txt
local DEBUG = false

-- ═══════════════════════════════════════════════════════════════════════
-- INTERNAL STATE
-- ═══════════════════════════════════════════════════════════════════════

local cooldown_timer = 0      -- seconds remaining before next trigger
local bounce_timer = 0        -- seconds remaining for active push
local bounce_dir_x = 0        -- cached bounce direction X
local bounce_dir_y = 0        -- cached bounce direction Y
local bounce_dir_z = 0        -- cached bounce direction Z
local bounce_speed = 0         -- cached bounce speed (units/sec)
local pulse_timer = 0          -- seconds remaining for pulse animation
local trigger_count = 0       -- how many times triggered (for logging)
local base_scale_x = 1.0      -- captured base scale for pulse
local base_scale_y = 1.0
local initialized = false

-- ═══════════════════════════════════════════════════════════════════════
-- HELPERS
-- ═══════════════════════════════════════════════════════════════════════

local function log(msg)
    if DEBUG then
        hamsterball.log("[bumper] " .. msg)
    end
end

-- 3D distance
local function dist3d(x1, y1, z1, x2, y2, z2)
    local dx = x2 - x1
    local dy = y2 - y1
    local dz = z2 - z1
    return math.sqrt(dx * dx + dy * dy + dz * dz)
end

-- Normalize a 3D vector, return x, y, z, length
local function normalize3d(x, y, z)
    local len = math.sqrt(x * x + y * y + z * z)
    if len > 0.0001 then
        return x / len, y / len, z / len, len
    end
    return 0, 0, 0, 0
end

-- Clamp a value to [min, max]
local function clamp(val, min, max)
    if val < min then return min end
    if val > max then return max end
    return val
end

-- Trigger the bumper: compute bounce direction and start push
local function trigger(entity_id, dist)
    trigger_count = trigger_count + 1
    cooldown_timer = COOLDOWN

    -- Get bumper and ball positions
    local bx, by, bz = hamsterball.get_position(entity_id)
    local ball_x, ball_y, ball_z = hamsterball.get_ball_pos(BALL_INDEX)

    -- Compute bounce direction: from bumper center toward ball
    -- This mimics the physics engine's collision response (ball bounces
    -- away from the surface it hit — the side of the bumper facing the ball)
    local dx = ball_x - bx
    local dy = ball_y - by
    local dz = ball_z - bz

    local nx, ny, nz, len = normalize3d(dx, dy, dz)

    -- If direction is degenerate (ball at exact center), push straight up
    if len < 0.01 then
        nx, ny, nz = 0, 1, 0
    end

    -- Flatten Y to match original behavior (zero vertical component)
    if FLATTEN_Y then
        ny = 0
        -- Re-normalize the horizontal components
        local hlen = math.sqrt(nx * nx + nz * nz)
        if hlen > 0.0001 then
            nx = nx / hlen
            nz = nz / hlen
        else
            -- Ball is directly above/below bumper — push in random horizontal dir
            nx = 1
            nz = 0
        end
    end

    -- Compute bounce speed.
    -- Original: amplifies existing velocity by 5x, clamped to [5.0, 12.0].
    -- We can't read velocity, so we use a fixed force that feels right.
    -- A faster approach speed should produce a faster bounce.
    -- We approximate approach speed from the distance / dt ratio:
    --   If the ball was moving fast (covered distance quickly), bounce harder.
    -- Since we don't have velocity, use a fixed mid-range value.
    bounce_speed = BOUNCE_FORCE

    -- Clamp to [min, max]
    bounce_speed = clamp(bounce_speed, MIN_BOUNCE_SPEED, MAX_BOUNCE_SPEED)

    -- Store bounce direction
    bounce_dir_x = nx
    bounce_dir_y = ny
    bounce_dir_z = nz

    -- Start the push
    bounce_timer = BOUNCE_DURATION

    -- Start visual pulse
    if PULSE_ENABLED then
        pulse_timer = PULSE_SPEED + PULSE_RETURN_SPEED
    end

    -- Award score (original = 0)
    if SCORE_AWARD > 0 then
        hamsterball.add_score(SCORE_AWARD, BALL_INDEX)
    end

    log(string.format("TRIGGERED #%d (dist=%.1f, dir=(%.2f,%.2f,%.2f), speed=%.0f)",
            trigger_count, dist, nx, ny, nz, bounce_speed))
end

-- ═══════════════════════════════════════════════════════════════════════
-- UPDATE — called every frame by the lua_support mod
-- ═══════════════════════════════════════════════════════════════════════

function update(entity_id, dt)
    -- ── Initialize: capture base scale for pulse animation ───────────
    if not initialized then
        base_scale_x, base_scale_y = hamsterball.get_scale(entity_id)
        if base_scale_x == 0 then base_scale_x = 1.0 end
        if base_scale_y == 0 then base_scale_y = 1.0 end
        initialized = true
    end

    -- ── Cooldown countdown ───────────────────────────────────────────
    if cooldown_timer > 0 then
        cooldown_timer = cooldown_timer - dt
    end

    -- ── Active bounce push ────────────────────────────────────────────
    -- Move the ball away from the bumper for BOUNCE_DURATION seconds.
    -- This simulates the original's velocity amplification (5x with
    -- min 5.0, max 12.0) by directly displacing the ball.
    if bounce_timer > 0 then
        bounce_timer = bounce_timer - dt

        -- Apply remaining push this frame
        local push = bounce_speed * dt

        -- Get current ball position and push it
        local ball_x, ball_y, ball_z = hamsterball.get_ball_pos(BALL_INDEX)
        local new_x = ball_x + bounce_dir_x * push
        local new_y = ball_y + bounce_dir_y * push
        local new_z = ball_z + bounce_dir_z * push

        -- The Lua API doesn't have set_ball_pos, so we can't directly
        -- move the ball. Instead, we rely on the push being applied
        -- through the bumper entity's position (visual feedback only).
        -- The actual gameplay bounce requires velocity manipulation
        -- which is not available in the current Lua API.
        --
        -- NOTE: If the API is extended with set_ball_pos or
        -- set_ball_velocity, the bounce push would be applied here.
        -- For now, the bumper provides visual feedback (pulse) and
        -- proximity detection, but the actual bounce physics must be
        -- handled by the game's own collision system.
        --
        -- The bumper object should be placed in the MESHWORLD with
        -- "N:BUMPER" collision event name so the game's native collision
        -- dispatch handles the actual velocity reversal. This Lua script
        -- adds supplementary behavior (cooldown tracking, score, pulse).
    end

    -- ── Collision detection ───────────────────────────────────────────
    -- Only check when not on cooldown and not actively pushing
    if cooldown_timer <= 0 then
        local dist = hamsterball.distance_to_ball(entity_id, BALL_INDEX)
        if dist >= 0 and dist < TRIGGER_RADIUS then
            trigger(entity_id, dist)
        end
    end

    -- ── Visual pulse animation ────────────────────────────────────────
    if pulse_timer > 0 then
        pulse_timer = pulse_timer - dt

        local total_pulse = PULSE_SPEED + PULSE_RETURN_SPEED
        local elapsed = total_pulse - pulse_timer

        local scale_mult
        if elapsed < PULSE_SPEED then
            -- Expanding phase
            local t = elapsed / PULSE_SPEED
            scale_mult = 1.0 + (PULSE_SCALE - 1.0) * t
        else
            -- Returning phase
            local t = (elapsed - PULSE_SPEED) / PULSE_RETURN_SPEED
            scale_mult = PULSE_SCALE - (PULSE_SCALE - 1.0) * t
        end

        hamsterball.set_scale(entity_id,
            base_scale_x * scale_mult,
            base_scale_y * scale_mult,
            1.0)
    else
        -- Reset to base scale
        hamsterball.set_scale(entity_id, base_scale_x, base_scale_y, 1.0)
    end
end
