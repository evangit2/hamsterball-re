-- sfxtest.lua — Sound Effects API demonstration
--
-- Attach this to any object named "L:sfxtest" in a .meshworld file.
-- It cycles through all available sound effects, playing each one with
-- different audio manipulations (volume, pitch, pan, looping, 3D position).
--
-- This script demonstrates the full hamsterball sound API:
--   hamsterball.play_sound(name [, opts])
--   hamsterball.stop_sound(name)
--   hamsterball.set_sound_volume(name, volume)
--   hamsterball.set_sound_pitch(name, pitch)
--   hamsterball.set_sound_pan(name, pan)
--   hamsterball.list_sounds()
--
-- ═══════════════════════════════════════════════════════════════════════
-- CONFIGURATION
-- ═══════════════════════════════════════════════════════════════════════

-- Time between each test phase (seconds)
local phase_delay = 2.5

-- Time between individual sound plays within a phase
local sound_delay = 0.6

-- ═══════════════════════════════════════════════════════════════════════
-- INTERNAL STATE
-- ═══════════════════════════════════════════════════════════════════════

local timer = 0
local phase = 0
local sound_idx = 1
local sound_list = {}
local current_loop = nil

-- ═══════════════════════════════════════════════════════════════════════
-- INITIALIZATION — run once when the script loads
-- ═══════════════════════════════════════════════════════════════════════

-- Get the full list of available sounds
sound_list = hamsterball.list_sounds()
hamsterball.log("[sfxtest] Found " .. #sound_list .. " sounds available")
hamsterball.log("[sfxtest] Test phases:")
hamsterball.log("  Phase 1: Play each sound at normal volume")
hamsterball.log("  Phase 2: Volume sweep (loud to quiet)")
hamsterball.log("  Phase 3: Pitch sweep (low to high)")
hamsterball.log("  Phase 4: Stereo pan sweep (left to right)")
hamsterball.log("  Phase 5: 3D positional playback")
hamsterball.log("  Phase 6: Looping test (start/stop)")

-- ═══════════════════════════════════════════════════════════════════════
-- HELPER FUNCTIONS
-- ═══════════════════════════════════════════════════════════════════════

local function pick_sound(idx)
    if #sound_list == 0 then return "bell" end
    idx = ((idx - 1) % #sound_list) + 1
    return sound_list[idx]
end

-- ═══════════════════════════════════════════════════════════════════════
-- UPDATE — called every frame
-- ═══════════════════════════════════════════════════════════════════════

function update(entity_id, dt)
    timer = timer + dt

    -- Stop any looping sound before switching phases
    if current_loop then
        hamsterball.stop_sound(current_loop)
        current_loop = nil
    end

    -- Phase 1: Play each sound at normal volume
    if phase == 0 then
        if timer >= sound_delay then
            timer = 0
            local name = pick_sound(sound_idx)
            hamsterball.log("[sfxtest] Phase 1: Playing '" .. name .. "'")
            hamsterball.play_sound(name)
            sound_idx = sound_idx + 1
            if sound_idx > #sound_list then
                phase = 1
                timer = 0
                hamsterball.log("[sfxtest] → Phase 2: Volume sweep")
            end
        end

    -- Phase 2: Volume sweep (play "bell" from loud to silent)
    elseif phase == 1 then
        if timer >= sound_delay then
            timer = 0
            local step = sound_idx - 1
            if step < 5 then
                local vol = 1.0 - (step * 0.2)
                hamsterball.log("[sfxtest] Phase 2: bell at volume " .. vol)
                hamsterball.play_sound("bell", {volume = vol})
                sound_idx = sound_idx + 1
            else
                phase = 2
                timer = 0
                sound_idx = 1
                hamsterball.log("[sfxtest] → Phase 3: Pitch sweep")
            end
        end

    -- Phase 3: Pitch sweep (play "whistle" from low to high pitch)
    elseif phase == 2 then
        if timer >= sound_delay then
            timer = 0
            local step = sound_idx - 1
            if step < 6 then
                local pitch = 0.5 + (step * 0.3)
                hamsterball.log("[sfxtest] Phase 3: whistle at pitch " .. pitch)
                hamsterball.play_sound("whistle", {pitch = pitch})
                sound_idx = sound_idx + 1
            else
                phase = 3
                timer = 0
                sound_idx = 1
                hamsterball.log("[sfxtest] → Phase 4: Pan sweep")
            end
        end

    -- Phase 4: Pan sweep (play "ting" from left to right)
    elseif phase == 3 then
        if timer >= sound_delay then
            timer = 0
            local step = sound_idx - 1
            if step < 5 then
                local pan = -1.0 + (step * 0.5)
                hamsterball.log("[sfxtest] Phase 4: ting at pan " .. pan)
                hamsterball.play_sound("ting", {pan = pan})
                sound_idx = sound_idx + 1
            else
                phase = 4
                timer = 0
                sound_idx = 1
                hamsterball.log("[sfxtest] → Phase 5: 3D positional")
            end
        end

    -- Phase 5: 3D positional playback (play sounds at entity position)
    elseif phase == 4 then
        if timer >= sound_delay then
            timer = 0
            local name = pick_sound(sound_idx)
            local x, y, z = hamsterball.get_position(entity_id)
            hamsterball.log("[sfxtest] Phase 5: '" .. name .. "' at entity pos (" .. x .. ", " .. y .. ", " .. z .. ")")
            hamsterball.play_sound(name, {x = x, y = y, z = z, volume = 0.8})
            sound_idx = sound_idx + 1
            if sound_idx > 6 then
                phase = 5
                timer = 0
                sound_idx = 1
                hamsterball.log("[sfxtest] → Phase 6: Looping test")
            end
        end

    -- Phase 6: Looping test (start a loop, then stop it)
    elseif phase == 5 then
        if timer >= phase_delay then
            timer = 0
            local step = sound_idx
            if step == 1 then
                -- Start a looping sound
                hamsterball.log("[sfxtest] Phase 6: Starting 'saw' loop")
                hamsterball.play_sound("saw", {loop = true, volume = 0.5})
                current_loop = "saw"
                sound_idx = 2
            elseif step == 2 then
                -- Still looping, wait a bit
                if timer >= phase_delay then
                    hamsterball.log("[sfxtest] Phase 6: Stopping 'saw' loop")
                    hamsterball.stop_sound("saw")
                    current_loop = nil
                    sound_idx = 3
                end
            else
                -- Try another looping sound with pitch
                hamsterball.log("[sfxtest] Phase 6: Starting 'fan-blow' loop at pitch 1.5")
                hamsterball.play_sound("fan-blow", {loop = true, pitch = 1.5, volume = 0.4})
                current_loop = "fan-blow"
                -- Stop it after a moment (next cycle)
                if timer >= phase_delay then
                    hamsterball.stop_sound("fan-blow")
                    current_loop = nil
                    sound_idx = 1
                end
            end
        end

        -- Safety: stop loop if we're done
        if sound_idx > 4 then
            if current_loop then
                hamsterball.stop_sound(current_loop)
                current_loop = nil
            end
            phase = 0
            timer = 0
            sound_idx = 1
            hamsterball.log("[sfxtest] → Restarting from Phase 1")
        end
    end
end
