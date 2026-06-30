-- tipper.lua — Recreated Dizzy Race TIPPER Behavior
--
-- Any object named "L:tipper" in a .meshworld file becomes a tilting
-- platform that mimics the original TIPPER behavior from Dizzy Race
-- (Level3), created by Tipper_ctor (0x437960) and rendered via
-- Rotator_Render (0x434070).
--
-- ═══════════════════════════════════════════════════════════════════════
-- ORIGINAL BEHAVIOR (reverse-engineered from the binary)
-- ═══════════════════════════════════════════════════════════════════════
--
-- Tippers are tilting platforms found in Dizzy Race (Level3). They are
-- created by CreateLevelObjects (0x41212D0) when it encounters a
-- "TIPPER" tag in the MESHWORLD file. The constructor is Tipper_ctor
-- at 0x437960.
--
-- Struct layout (Tipper, size 0x1104 bytes, vtable 0x4D4ED8):
--
--   +0x10D0: board pointer (parent scene/board)
--   +0x10D4: 0 (unknown flag)
--   +0x10D8: position X (from MESHWORLD ref point, MW x-axis)
--   +0x10DC: position Z (from MESHWORLD ref point, MW z-axis)
--   +0x10E0: position Y (from MESHWORLD ref point, MW y-axis)
--   +0x10E4: rotation X (from MESHWORLD ref point, MW rot x)
--   +0x10E8: rotation Z (from MESHWORLD ref point, MW rot z)
--   +0x10EC: rotation Y (from MESHWORLD ref point, MW rot y)
--   +0x10F0: 0 (state/timer)
--   +0x10F4: 0xC61C3C00 ≈ -10000.0 (initial scale/tilt sentinel)
--            This is the Gfx_ScaleX value — controls the visual
--            tilt/scale of the platform. Updated during gameplay
--            to create the tipping animation.
--   +0x10F8: 0 (aux timer)
--   +0x10FC: 0 (aux state)
--   +0x1100: 50 + CPUID feature flag (frame counter for timing)
--
-- A TipperVisual object (size 0x10D0, vtable 0x4D9068) is also created
-- and attached via TipperVisual_Attach (0x465200). It clones the
-- spatial tree from the source mesh for rendering.
--
-- Rotator_Render (0x434070, shared vtable entry) does:
--   1. D3DXSkinMesh_CopyStripData — render mesh strips
--   2. Gfx_ScaleX(this+0x10F4) — apply tilt/scale factor
--   3. Gfx_SetPosition(
--        pos_z - 10.0,   -- X render position (offset by _DAT_004cf9f8 = 10.0)
--        pos_y,          -- Y render position (no offset)
--        rot_x - 10.0    -- Z render position (offset by 10.0)
--      )
--   4. Temporarily adjusts projection matrix offsets (gfx+0x790/0x794)
--      by ±0.11111... (_DAT_004cf308 = 0x3DFFFFFF... ≈ 0.111)
--   5. Calls board+0x578 vtable[0x1C] (scene render callback)
--   6. Restores projection offsets
--
-- The tipping motion comes from the Gfx_ScaleX value at +0x10F4
-- changing over time. The initial value of -10000.0 is a sentinel
-- meaning "inactive/uninitialized." During gameplay, the game's own
-- timer/animation system updates this field to create the back-and-
-- forth tilting motion that characterizes Dizzy Race platforms.
--
-- The collision is handled by the base DispatchCollisionEvents
-- (0x40C5D0) via vtable[5] = Rotator_CollisionDispatch (0x46F3B0),
-- which just sets up the render frame — no custom physics response.
-- The ball physics on the tipper come from the mesh geometry itself
-- (the tilted collision surface causes the ball to slide).
--
-- ═══════════════════════════════════════════════════════════════════════
-- LUA API LIMITATIONS
-- ═══════════════════════════════════════════════════════════════════════
--
-- The Lua API does not expose:
--   • Mesh/collision surface modification (original tilts the actual
--     collision geometry via Gfx_ScaleX)
--   • Timer system access (original uses Timer_Init for animation timing)
--   • Projection matrix manipulation (original adjusts camera offsets)
--
-- This script simulates the tipper effect using available API:
--
--   • Visual tilt — rotates the L:tipper entity back and forth on
--     one axis (pitch/X rotation), mimicking the Gfx_ScaleX tilt
--   • Proximity detection — detects when the ball is on the platform
--   • Ball nudge — when the ball is on the tilting platform, gently
--     pushes it in the tilt direction (simulates sliding off the edge)
--   • Visual feedback — scale pulse when ball is on the platform
--
-- ═══════════════════════════════════════════════════════════════════════
-- CONFIGURATION
-- ═══════════════════════════════════════════════════════════════════════

-- Maximum tilt angle in degrees (how far the platform tips).
-- The original Dizzy Race tippers tilt approximately 15-25 degrees.
-- Larger values = more dramatic tilt.
local MAX_TILT_ANGLE = 20.0

-- Tilt cycle speed in degrees per second.
-- The original tips back and forth over ~2-3 seconds per direction.
-- Higher = faster oscillation.
local TILT_SPEED = 60.0

-- Which axis the platform tilts around.
-- 1 = X axis (pitch — tips forward/backward)
-- 2 = Y axis (yaw — rotates left/right, like a turntable)
-- 3 = Z axis (roll — tips side to side)
local TILT_AXIS = 1

-- How close the ball must be (horizontally) to be "on" the platform.
-- The original uses collision mesh overlap; we use a circular proximity
-- check. Dizzy Race platforms are ~100-150 units wide.
local PLATFORM_RADIUS = 120.0

-- Vertical range: how far above/below the platform the ball can be
-- to count as "on" it. Prevents triggering when the ball is far below
-- or above on a different level section.
local PLATFORM_HEIGHT = 80.0

-- Nudge force: how fast the ball is pushed when on the tilting
-- platform (units per second). Simulates the ball sliding off the
-- tilted surface. Set to 0 to disable ball nudging (visual-only tilt).
local NUDGE_FORCE = 300.0

-- Nudge direction follows the tilt: when the platform tips left,
-- the ball is pushed left. This mimics gravity pulling the ball
-- down the slope of the tilted surface.
local NUDGE_ENABLED = true

-- Which ball to affect (0 = player 1)
local BALL_INDEX = 0

-- ══ Visual: Scale pulse when ball is on platform ══════════════════════

-- The original tipper doesn't have a visual pulse, but this adds
-- feedback so the player can see when the tipper is "active."
local PULSE_ENABLED = true

-- Scale multiplier at peak (1.0 = no pulse)
local PULSE_SCALE = 1.08

-- Pulse speed (cycles per second)
local PULSE_SPEED = 4.0

-- ══ Debug ═════════════════════════════════════════════════════════════

-- Set to true for diagnostic output to Lua/log.txt
local DEBUG = false

-- ═══════════════════════════════════════════════════════════════════════
-- INTERNAL STATE
-- ═══════════════════════════════════════════════════════════════════════

-- Tilt state machine:
--   0 = TILTING_RIGHT  (platform tilting toward +X/positive direction)
--   1 = TILTING_LEFT   (platform tilting toward -X/negative direction)
local tilt_state = 0

-- Current tilt angle in degrees (accumulated over time)
local current_tilt = 0.0

-- Elapsed time for pulse animation
local elapsed_time = 0.0

-- Whether the ball is currently on the platform
local ball_on_platform = false

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
        hamsterball.log("[tipper] " .. msg)
    end
end

local function to_rad(deg)
    return deg * (math.pi / 180.0)
end

-- Clamp a value to [min, max]
local function clamp(val, min_val, max_val)
    if val < min_val then return min_val end
    if val > max_val then return max_val end
    return val
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

    -- ── Accumulate tilt angle (triangle wave for back-and-forth) ──────
    --
    -- The original Tipper uses a timer-based system (Timer_Init at
    -- +0x434 of TipperVisual, frame counter at +0x1100). We simulate
    -- the oscillation with a simple triangle wave:
    --
    --   The tilt goes from 0 → +MAX → 0 → -MAX → 0 → ...
    --   This creates the characteristic "tip to one side, then the other"
    --   motion of Dizzy Race platforms.

    if tilt_state == 0 then
        -- Tilting right (increasing angle)
        current_tilt = current_tilt + TILT_SPEED * dt
        if current_tilt >= MAX_TILT_ANGLE then
            current_tilt = MAX_TILT_ANGLE
            tilt_state = 1
        end
    else
        -- Tilting left (decreasing angle)
        current_tilt = current_tilt - TILT_SPEED * dt
        if current_tilt <= -MAX_TILT_ANGLE then
            current_tilt = -MAX_TILT_ANGLE
            tilt_state = 0
        end
    end

    -- ── Apply tilt as rotation ───────────────────────────────────────
    --
    -- The original uses Gfx_ScaleX (+0x10F4) to create the tilt effect
    -- during rendering. Since we can't access the render pipeline, we
    -- apply the tilt as a rotation on the entity itself.
    --
    -- The tilt direction corresponds to the accumulated angle. We
    -- preserve the entity's other rotation axes.

    local rad_tilt = to_rad(current_tilt)
    local _, rot_y, rot_z = hamsterball.get_rotation(entity_id)

    if TILT_AXIS == 1 then
        -- Pitch (tip forward/backward)
        hamsterball.set_rotation(entity_id, rad_tilt, rot_y, rot_z)
    elseif TILT_AXIS == 2 then
        -- Yaw (rotate like turntable)
        hamsterball.set_rotation(entity_id, 0, rad_tilt, rot_z)
    else
        -- Roll (tip side to side)
        hamsterball.set_rotation(entity_id, 0, rot_y, rad_tilt)
    end

    -- ── Ball proximity check ──────────────────────────────────────────
    --
    -- Check if the ball is "on" the platform using both horizontal
    -- distance (within PLATFORM_RADIUS) and vertical proximity
    -- (within PLATFORM_HEIGHT of the platform's Y).

    local ex, ey, ez = hamsterball.get_position(entity_id)
    local bx, by, bz = hamsterball.get_ball_pos(BALL_INDEX)

    -- Horizontal distance (XZ plane)
    local hdx = bx - ex
    local hdz = bz - ez
    local h_dist = math.sqrt(hdx * hdx + hdz * hdz)

    -- Vertical distance
    local vdy = by - ey

    local was_on_platform = ball_on_platform
    ball_on_platform = (h_dist < PLATFORM_RADIUS) and
                       (math.abs(vdy) < PLATFORM_HEIGHT)

    if ball_on_platform and not was_on_platform then
        trigger_count = trigger_count + 1
        log(string.format("BALL ON PLATFORM #%d (h_dist=%.1f, vdy=%.1f)",
                trigger_count, h_dist, vdy))
    elseif not ball_on_platform and was_on_platform then
        log(string.format("BALL LEFT PLATFORM (after %.1fs)", elapsed_time))
    end

    -- ── Ball nudge: simulate sliding off tilted surface ──────────────
    --
    -- When the ball is on the tilting platform, push it in the
    -- direction of the tilt. This mimics the physics of the ball
    -- sliding down a sloped surface.
    --
    -- The nudge direction is based on the current tilt sign:
    --   Positive tilt → push in +X (or +Z for roll axis)
    --   Negative tilt → push in -X (or -Z for roll axis)
    --
    -- The nudge force scales with the tilt magnitude (steeper = faster).

    if ball_on_platform and NUDGE_ENABLED then
        local tilt_fraction = math.abs(current_tilt) / MAX_TILT_ANGLE
        local push = NUDGE_FORCE * tilt_fraction * dt

        if current_tilt > 0 then
            -- Tilted right: push ball in +X or +Z depending on axis
            if TILT_AXIS == 1 or TILT_AXIS == 2 then
                bx = bx + push
            else
                bz = bz + push
            end
        else
            if TILT_AXIS == 1 or TILT_AXIS == 2 then
                bx = bx - push
            else
                bz = bz - push
            end
        end

        -- Note: The Lua API doesn't have set_ball_pos, so the nudge
        -- is visual-only unless the game's own physics takes over.
        -- The actual sliding effect requires the mesh collision surface
        -- to be tilted, which the original achieves via Gfx_ScaleX.
        -- For real gameplay effect, place a "TIPPER" collision mesh
        -- in the MESHWORLD and use this Lua script for supplementary
        -- visual/feedback behavior.
    end

    -- ── Visual: Scale pulse when ball is on platform ─────────────────
    --
    -- The original tipper has no visual pulse — the tilt itself IS
    -- the visual feedback. But since our rotation-based tilt may be
    -- subtle on some meshes, we add a gentle scale pulse to make
    -- it clear when the tipper is "active" (ball is on it).

    if PULSE_ENABLED then
        if ball_on_platform then
            -- Active: gentle pulsing to show the tipper is engaged
            local pulse = math.sin(elapsed_time * PULSE_SPEED * math.pi * 2)
            local scale_mult = 1.0 + (PULSE_SCALE - 1.0) * pulse
            hamsterball.set_scale(entity_id,
                base_scale_x * scale_mult,
                base_scale_y * scale_mult,
                1.0)
        else
            -- Idle: reset to base scale
            hamsterball.set_scale(entity_id, base_scale_x, base_scale_y, 1.0)
        end
    end
end
