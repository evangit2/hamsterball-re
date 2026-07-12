-- tarpit.lua — Recreated Dizzy Race TARPIT Behavior
--
-- Any object named "L:tarpit" in a .meshworld file becomes a sticky tar
-- pit that mimics the original N:TARPIT collision behavior from
-- DispatchCollisionEvents (0x40C5D0).
--
-- ═══════════════════════════════════════════════════════════════════════
-- ORIGINAL BEHAVIOR (reverse-engineered from the binary)
-- ═══════════════════════════════════════════════════════════════════════
--
-- Tar pits appear in the Dizzy Race (Level3). The level geometry contains
-- collision meshes tagged with the event name "N:TARPIT". When the ball
-- collides with one, DispatchCollisionEvents (0x40C5D0) does:
--
--   1. Check if ball+0x2CC (tar_flag) is already set (byte)
--   2. If NOT already tarred (first contact):
--      a. Play 3D tar squelch sound from board+0x484 sound buffer
--         at the ball's current position (ball+0x164/0x168/0x16C)
--      b. Record ball's Y position: ball+0x2D0 = ball+0x168 (pos.y)
--         This is the "sticky height" — the ball is held at this Y
--   3. Set ball+0x2CC = 1  (tar_flag — disables force application)
--   4. Set ball+0x768 = 0  (clears ball "active" state flag)
--
-- In Ball_FallUpdate (0x408830), the tar flag is checked every frame:
--
--   if (ball+0x2CC != 0 || ball+0x2F9 != 0):
--       target_pos = current_pos   -- OVERRIDE physics target with current pos
--
-- This means when tarred, the physics engine computes a movement target
-- (from input + gravity + forces), but that target is REPLACED with the
-- ball's current position. The ball effectively cannot move — it is
-- frozen in place at the position where it touched the tar.
--
-- The tar flag is ONLY cleared by Ball_InitPhysicsDefaults (0x405100),
-- which is called on respawn/level reset. There is no escape — the ball
-- stays stuck until the player respawns or another ball knocks it out
-- (ball-ball collision pushes position directly, bypassing the force
-- system).
--
-- Key constants from the binary:
--   _DAT_004cf3c8 = 0.0       (near-zero velocity threshold)
--   _DAT_004cf418 = 3.0       (minimum speed — if slower, boosted to 3.0)
--   _DAT_004cf448 = 0.02      (gravity decrement per frame)
--   Sound buffer: board+0x484 (tar squelch SFX)
--
-- ═══════════════════════════════════════════════════════════════════════
-- LUA API LIMITATIONS
-- ═══════════════════════════════════════════════════════════════════════
--
-- The Lua API does not expose:
--   • Ball velocity read/write (original manipulates CollisionMesh+0xCA4)
--   • Ball position write (original freezes ball by overriding physics target)
--   • Ball flag write (original sets ball+0x2CC=1, ball+0x768=0)
--   • Sound playback (original plays 3D sound from board+0x484)
--
-- This script simulates the tar pit effect using available API:
--
--   • Proximity detection — when ball enters the tar zone
--   • Visual feedback — entity pulses and darkens to simulate bubbling tar
--   • Stuck state — tracks whether the ball is "caught" in the tar
--   • Escape detection — if the ball moves out of the tar zone, it's free
--   • Sink timeout — if stuck too long, kill_ball (simulates sinking)
--
-- The visual scale-pulse mimics the original's sticky resistance: the
-- entity appears to "grip" the ball, expanding and contracting like
-- viscous tar bubbling around it.
--
-- ═══════════════════════════════════════════════════════════════════════
-- CONFIGURATION
-- ═══════════════════════════════════════════════════════════════════════

-- How close the ball must be to trigger the tar pit (game units).
-- The original uses collision mesh overlap; we approximate with
-- a spherical proximity check. Ball radius is ~26 units, tar pits
-- in Dizzy Race are wide flat areas, so 90 gives good coverage.
local TRIGGER_RADIUS = 90.0

-- How close the ball must be to COUNT AS ESCAPED (game units).
-- Once the ball moves beyond this distance from the tar pit center,
-- the stuck state is cleared. This simulates the ball struggling
-- free from the tar's grip. Set equal to TRIGGER_RADIUS for a
-- clean enter/exit boundary.
local ESCAPE_RADIUS = 90.0

-- Which ball to check (0 = player 1)
local BALL_INDEX = 0

-- ══ Stuck behavior ════════════════════════════════════════════════════

-- How long the ball stays "stuck" before sinking (seconds).
-- The original has NO timeout — the ball is permanently stuck until
-- respawn. Set to math.huge for faithful behavior, or use a finite
-- timeout for a more forgiving gameplay experience.
--
-- With a finite timeout:
--   2.0 = quick sink (arcade-style, ball dies after 2s in tar)
--   5.0 = moderate (gives time to be pushed out by another ball)
--   math.huge = faithful (ball stuck forever until respawn)
local SINK_TIMEOUT = 5.0

-- Whether to kill the ball when the sink timeout expires.
-- true  = kill_ball (ball respawns — simulates sinking into tar)
-- false = just clear the stuck flag (ball escapes after timeout)
local KILL_ON_SINK = true

-- Score awarded when the ball first enters the tar (original = 0)
local SCORE_AWARD = 0

-- ══ Visual: Tar bubbling effect ═══════════════════════════════════════

-- The original tar pit is static geometry — no visual animation.
-- This script adds a bubbling/pulsing scale effect to give visual
-- feedback that the tar is "active" (gripping the ball).

-- Enable the visual pulse when ball is stuck
local PULSE_ENABLED = true

-- Scale multiplier at peak pulse (1.0 = no visible pulse)
local PULSE_SCALE = 1.15

-- Pulse speed (cycles per second)
local PULSE_SPEED = 6.0

-- Scale when idle (ball not in tar — subtle breathing effect)
local IDLE_PULSE_SCALE = 1.03
local IDLE_PULSE_SPEED = 0.5

-- ══ Debug ═════════════════════════════════════════════════════════════

-- Set to true for diagnostic output to Lua/log.txt
local DEBUG = false

-- ═══════════════════════════════════════════════════════════════════════
-- INTERNAL STATE
-- ═══════════════════════════════════════════════════════════════════════

-- Tar state machine:
--   0 = IDLE      (ball not in zone, tar is dormant)
--   1 = STUCK     (ball is in zone, tar is gripping)
--   2 = SINKING   (timeout expired, ball is being killed)
local tar_state = 0

-- Timer counting how long the ball has been stuck (seconds)
local stuck_timer = 0.0

-- Total time elapsed (for visual pulse animation)
local elapsed_time = 0.0

-- Whether this is the first contact (for sound/score one-shot)
local first_contact = true

-- Base scale captured on first frame (for restoring after pulse)
local base_scale_x = 1.0
local base_scale_y = 1.0
local initialized = false

-- Trigger counter (for logging)
local trigger_count = 0

-- ═══════════════════════════════════════════════════════════════════════
-- HELPERS
-- ═══════════════════════════════════════════════════════════════════════

local function log(msg)
    if DEBUG then
        hamsterball.log("[tarpit] " .. msg)
    end
end

-- 3D distance
local function dist3d(x1, y1, z1, x2, y2, z2)
    local dx = x2 - x1
    local dy = y2 - y1
    local dz = z2 - z1
    return math.sqrt(dx * dx + dy * dy + dz * dz)
end

-- ═══════════════════════════════════════════════════════════════════════
-- STATE TRANSITIONS
-- ═══════════════════════════════════════════════════════════════════════

-- Enter the STUCK state (ball just touched the tar)
-- Mimics: DispatchCollisionEvents setting ball+0x2CC=1, playing sound,
-- recording sticky Y position.
local function enter_stuck(entity_id, dist)
    tar_state = 1
    stuck_timer = 0.0
    trigger_count = trigger_count + 1

    -- First contact: play "sound" (visual only — no audio API)
    -- and award score if configured
    if first_contact then
        first_contact = false
        if SCORE_AWARD > 0 then
            hamsterball.add_score(SCORE_AWARD, BALL_INDEX)
        end
        log(string.format("FIRST CONTACT #%d (dist=%.1f) — ball caught in tar",
                trigger_count, dist))
    else
        log(string.format("RE-CAUGHT #%d (dist=%.1f)", trigger_count, dist))
    end
end

-- Enter the SINKING state (timeout expired)
-- Mimics: the ball has been stuck too long and sinks into the tar.
local function enter_sinking(entity_id)
    tar_state = 2
    log(string.format("SINKING — ball stuck for %.1fs, killing", stuck_timer))

    if KILL_ON_SINK then
        hamsterball.kill_ball(BALL_INDEX)
    end

    -- Reset to idle after the sink
    tar_state = 0
    stuck_timer = 0.0
    first_contact = true  -- allow re-triggering after respawn
end

-- Exit to IDLE state (ball escaped the tar zone)
-- Mimics: another ball pushed this ball out, or the player wiggled free.
local function enter_idle(entity_id)
    log(string.format("ESCAPED — ball free after %.1fs stuck", stuck_timer))
    tar_state = 0
    stuck_timer = 0.0
    -- Note: first_contact stays false — if the ball re-enters, it won't
    -- play the first-contact effects again until after a sink/respawn
end

-- ═══════════════════════════════════════════════════════════════════════
-- UPDATE — called every frame by the lua_support mod
-- ═══════════════════════════════════════════════════════════════════════

function update(entity_id, dt)
    -- ── Initialize: capture base scale ────────────────────────────────
    if not initialized then
        base_scale_x, base_scale_y = hamsterball.get_scale(entity_id)
        if base_scale_x == 0 then base_scale_x = 1.0 end
        if base_scale_y == 0 then base_scale_y = 1.0 end
        initialized = true
    end

    elapsed_time = elapsed_time + dt

    -- ── State machine ──────────────────────────────────────────────────

    if tar_state == 0 then
        -- ══ IDLE: check if ball enters tar zone ══════════════════════
        local dist = hamsterball.distance_to_ball(entity_id, BALL_INDEX)
        if dist >= 0 and dist < TRIGGER_RADIUS then
            enter_stuck(entity_id, dist)
        end

    elseif tar_state == 1 then
        -- ══ STUCK: ball is caught in tar ═════════════════════════════
        -- Count up the stuck timer
        stuck_timer = stuck_timer + dt

        -- Check if ball has escaped the tar zone
        local dist = hamsterball.distance_to_ball(entity_id, BALL_INDEX)
        if dist < 0 or dist >= ESCAPE_RADIUS then
            -- Ball moved out of the tar zone — escaped!
            enter_idle(entity_id)
        elseif SINK_TIMEOUT ~= math.huge and stuck_timer >= SINK_TIMEOUT then
            -- Ball has been stuck too long — sink!
            enter_sinking(entity_id)
        end

    elseif tar_state == 2 then
        -- ══ SINKING: brief transition state (handled in enter_sinking) ═
        -- enter_sinking() already resets to idle, so we shouldn't be here.
        -- Safety fallback:
        tar_state = 0
        stuck_timer = 0.0
    end

    -- ── Visual effects ────────────────────────────────────────────────

    if PULSE_ENABLED then
        if tar_state == 1 then
            -- Active tar: fast aggressive pulsing (tar is gripping!)
            local pulse = math.sin(elapsed_time * PULSE_SPEED * math.pi * 2)
            local scale_mult = 1.0 + (PULSE_SCALE - 1.0) * pulse

            hamsterball.set_scale(entity_id,
                base_scale_x * scale_mult,
                base_scale_y * scale_mult,
                1.0)
        else
            -- Idle tar: slow gentle breathing (dormant tar)
            local pulse = math.sin(elapsed_time * IDLE_PULSE_SPEED * math.pi * 2)
            local scale_mult = 1.0 + (IDLE_PULSE_SCALE - 1.0) * pulse

            hamsterball.set_scale(entity_id,
                base_scale_x * scale_mult,
                base_scale_y * scale_mult,
                1.0)
        end
    end
end
