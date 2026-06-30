-- mousetrap.lua — Recreated Mousetrap (N:MOUSETRAP) Behavior
--
-- Any object named "L:mousetrap" in a .meshworld file becomes a mousetrap
-- hazard that mimics the original N:MOUSETRAP collision behavior from
-- DispatchCollisionEvents (0x40C5D0) and CreateMouseTrap (0x40BF50).
--
-- ═══════════════════════════════════════════════════════════════════════
-- ORIGINAL BEHAVIOR (reverse-engineered from the binary)
-- ═══════════════════════════════════════════════════════════════════════
--
-- The Mousetrap is created by CreateMouseTrap (0x40BF50), which scans the
-- SpatialTree for objects named "MOUSETRAP". Each one is allocated as a
-- TipperVisual struct (0x10F8 bytes) via TipperVisual_Level_Ctor. The
-- constructor:
--   • Copies position from MESHWORLD data (x, z, y — note Y/Z swap)
--     to offsets +0x10DC/+0x10E0/+0x10E4
--   • Sets a height offset at +0x10F4: _DAT_004CF44C - meshworld_param
--   • Appends to 4 lists: render (board+0xCD4), update (board+0x1930),
--     spatial tree (scene+0x480+0x1C), collision (board+0x8B0+0x18)
--   • Calls vtable[0x2C] (method 11) to finalize initialization
--
-- When the ball collides with a mousetrap (event string "N:MOUSETRAP"),
-- DispatchCollisionEvents (0x40C5D0) handles it as follows:
--
--   1. RNG_Rand(50) × 2  — two random numbers for trajectory variation
--   2. Read ball velocity from CollisionMesh+0xCA4 (vx), +0xCA8 (vy), +0xCAC (vz)
--   3. Normalize the velocity vector (preserve direction, magnitude → 1.0)
--   4. Set velocity Y to 15.0 (0x41700000)  — strong upward launch
--   5. Scale entire vector by 20.0 (0x4CF370)  — massive speed boost
--   6. Write modified velocity back to CollisionMesh+0xCA4/CA8/CAC
--   7. Search rotator list at scene+0x1D3C for matching rotator by ID
--      (rotator+0x10D4 == collision_obj+0x47C)
--   8. On match: play 3D collision sound, add rotator to scene+0x2578
--
-- Net effect: the ball is launched at ~20× normal speed, mostly upward,
-- in a randomized direction. This typically sends the ball flying off
-- the level — effectively a kill hazard.
--
-- ═══════════════════════════════════════════════════════════════════════
-- LUA API LIMITATIONS
-- ═══════════════════════════════════════════════════════════════════════
--
-- The Lua API does not expose ball velocity read/write. The original
-- manipulates CollisionMesh+0xCA4/CA8/CAC directly, which requires
-- pointer chasing from ball→physics→collisionmesh. This script simulates
-- the launch effect using available API functions:
--
--   • kill_ball()  — simulates the ball being launched off the level
--   • add_score()  — optional score award (original gives 0 points)
--   • set_rotation() — drives the TipperVisual snap animation
--
-- ═══════════════════════════════════════════════════════════════════════
-- CONFIGURATION
-- ═══════════════════════════════════════════════════════════════════════

-- How close the ball must be to trigger the mousetrap (game units).
-- The original uses the collision mesh overlap system; we approximate
-- with a spherical proximity check.
local TRIGGER_RADIUS = 80.0

-- Cooldown after triggering (seconds). Prevents rapid re-triggering
-- while the ball is still near the mousetrap.
local COOLDOWN = 2.0

-- Which ball to check (0 = player 1)
local BALL_INDEX = 0

-- ══ Launch effect ══════════════════════════════════════════════════════

-- What happens when the mousetrap triggers:
--   "kill"   — kill the ball (simulates launch off the level edge)
--   "score"  — award points only, ball survives
--   "both"   — kill ball AND award points
--   "none"   — visual snap only, no gameplay effect
local LAUNCH_MODE = "kill"

-- Score to award when triggered (original N:MOUSETRAP = 0 points)
local SCORE_AWARD = 0

-- ══ Snap animation (TipperVisual recreation) ══════════════════════════

-- The original mousetrap is a TipperVisual — it tilts/snaps when triggered.
-- These parameters control the visual snap animation.

-- Maximum tilt angle in degrees (how far it snaps forward)
local SNAP_ANGLE = 45.0

-- Speed of the snap forward (degrees per second)
local SNAP_SPEED = 720.0

-- Speed of the return to idle (degrees per second)
local SNAP_RETURN_SPEED = 180.0

-- Rotation axis for the snap: "x" = forward tilt, "z" = sideways tilt
local SNAP_AXIS = "x"

-- ══ Debug ═════════════════════════════════════════════════════════════

-- Set to true for diagnostic output to Lua/log.txt
local DEBUG = false

-- ═══════════════════════════════════════════════════════════════════════
-- INTERNAL STATE
-- ═══════════════════════════════════════════════════════════════════════

local cooldown_timer = 0     -- seconds remaining before next trigger
local snap_state = 0         -- 0 = idle, 1 = snapping forward, 2 = returning
local current_tilt = 0.0     -- current tilt angle (degrees)
local trigger_count = 0      -- how many times triggered (for logging)

-- ═══════════════════════════════════════════════════════════════════════
-- HELPERS
-- ═══════════════════════════════════════════════════════════════════════

local function log(msg)
    if DEBUG then
        hamsterball.log("[mousetrap] " .. msg)
    end
end

-- Simulate the two RNG_Rand(50) calls from the original.
-- These randomize the trajectory direction. We use them to add
-- a random spin to the snap animation for visual variety.
local function random_spin()
    -- Equivalent to two RNG_Rand(50) calls — each returns 0..49
    local r1 = math.random(0, 49)
    local r2 = math.random(0, 49)
    -- Use r1 to pick a random Y rotation offset (0-360°)
    return (r1 / 49.0) * 360.0, r2
end

-- Apply tilt to the entity along the configured axis
local function apply_tilt(entity_id, angle_deg)
    local rad = angle_deg * (math.pi / 180.0)
    local rx, ry, rz = hamsterball.get_rotation(entity_id)

    if SNAP_AXIS == "x" then
        hamsterball.set_rotation(entity_id, rad, ry, rz)
    elseif SNAP_AXIS == "z" then
        hamsterball.set_rotation(entity_id, rx, ry, rad)
    else
        hamsterball.set_rotation(entity_id, rad, ry, rad)
    end
end

-- Trigger the mousetrap: snap animation + gameplay effect
local function trigger(entity_id, dist)
    trigger_count = trigger_count + 1
    cooldown_timer = COOLDOWN

    -- Random spin for visual variety (mimics RNG_Rand(50) × 2)
    local spin_y, r2 = random_spin()
    log(string.format("TRIGGERED #%d (dist=%.1f, spin=%.0f°, r2=%d)",
            trigger_count, dist, spin_y, r2))

    -- Start snap animation
    snap_state = 1
    current_tilt = 0.0

    -- Apply random Y rotation to the entity (visual trajectory randomization)
    local rx, ry, rz = hamsterball.get_rotation(entity_id)
    hamsterball.set_rotation(entity_id, rx, spin_y * (math.pi / 180.0), rz)

    -- Apply gameplay effect based on LAUNCH_MODE
    -- The original launches the ball at 20× speed with Y=15.0 (upward).
    -- Since we can't set ball velocity, we simulate the outcome:
    if LAUNCH_MODE == "kill" or LAUNCH_MODE == "both" then
        -- The 20× velocity + 15.0 Y boost typically sends the ball
        -- flying off the level edge → kill/respawn
        hamsterball.kill_ball(BALL_INDEX)
        log("  → kill_ball (simulating launch off edge)")
    end

    if LAUNCH_MODE == "score" or LAUNCH_MODE == "both" then
        if SCORE_AWARD > 0 then
            hamsterball.add_score(SCORE_AWARD, BALL_INDEX)
            log("  → add_score(" .. SCORE_AWARD .. ")")
        end
    end
end

-- ═══════════════════════════════════════════════════════════════════════
-- UPDATE — called every frame by the lua_support mod
-- ═══════════════════════════════════════════════════════════════════════

function update(entity_id, dt)
    -- ── Cooldown countdown ─────────────────────────────────────────────
    if cooldown_timer > 0 then
        cooldown_timer = cooldown_timer - dt
    end

    -- ── Collision detection ────────────────────────────────────────────
    -- Only check when not on cooldown
    if cooldown_timer <= 0 then
        local dist = hamsterball.distance_to_ball(entity_id, BALL_INDEX)
        if dist >= 0 and dist < TRIGGER_RADIUS then
            trigger(entity_id, dist)
        end
    end

    -- ── Snap animation state machine ───────────────────────────────────
    -- Runs AFTER collision check so a fresh trigger snaps on the same frame.
    -- Phase 1: Snap forward (fast)
    if snap_state == 1 then
        current_tilt = current_tilt + SNAP_SPEED * dt
        if current_tilt >= SNAP_ANGLE then
            current_tilt = SNAP_ANGLE
            snap_state = 2  -- transition to return
        end
        apply_tilt(entity_id, current_tilt)

    -- Phase 2: Return to idle (slow)
    elseif snap_state == 2 then
        current_tilt = current_tilt - SNAP_RETURN_SPEED * dt
        if current_tilt <= 0.0 then
            current_tilt = 0.0
            snap_state = 0  -- back to idle
        end
        apply_tilt(entity_id, current_tilt)
    end
end
