---@diagnostic disable: undefined-global

local AXIS = 1        -- 1=Roll (Change to 2 or 3 to test others)
local START_DELAY_MS = 75000  -- Wait 75 Seconds before trying to start

-- Configuration
local AMP_START = 5.0
local AMP_END   = 20.0
local FREQ_START = 0.5
local FREQ_END   = 2.0
local SWEEP_TIME = 30.0
local PERIOD_MS  = 20

local active = false
local sweep_complete = false
local start_time = 0
local last_print = 0

function update()
  local now = millis():tofloat()
  
  -- PHASE 1: Waiting for the timer (0 to 75 seconds)
  if now < START_DELAY_MS then
    -- Print a countdown every 10 seconds so you know it's alive
    if now - last_print > 10000 then
       local remaining = math.floor((START_DELAY_MS - now) / 1000)
       gcs:send_text(0, string.format("Auto-Sweep starting in %d seconds...", remaining))
       last_print = now
    end
    return update, PERIOD_MS
  end

  if not active and not sweep_complete then
      -- Safety Checks
      local safe = true
      
      if not arming:is_armed() then
         if now - last_print > 2000 then
            gcs:send_text(0, "Waiting: ARM the drone to start sweep")
            last_print = now
         end
         safe = false
      end

      local mode = vehicle:get_mode()
      if mode ~= 4 and mode ~= 15 then -- 4=Guided
         if now - last_print > 2000 then
             gcs:send_text(0, "Waiting: Switch to GUIDED to start sweep")
             last_print = now
         end
         safe = false
      end

      if safe then
         gcs:send_text(0, "Timer Reached & Safety Passed -> Sweep STARTED")
         start_time = millis():tofloat()
         active = true
      end
      return update, PERIOD_MS
  end
    
  if active then
    local current_time = millis():tofloat()
    local elapsed = (current_time - start_time) / 1000.0
    local progress = elapsed / SWEEP_TIME
      
    if progress >= 1.0 then
      gcs:send_text(0, "Sweep COMPLETE")
      active = false
      sweep_complete = true
      vehicle:set_target_angle_and_climbrate(0, 0, 0, 0, 0, 0)
    else
      local freq = FREQ_START + (FREQ_END - FREQ_START) * progress
      local amp = AMP_START + (AMP_END - AMP_START) * progress
      local angle_rad = 2.0 * math.pi * freq * elapsed
      local cmd_val = amp * math.sin(angle_rad)
      local cmd_rad = math.rad(cmd_val)
      
      if AXIS == 1 then
         vehicle:set_target_angle_and_climbrate(cmd_rad, 0, 0, 0, 0, 0)
      elseif AXIS == 2 then
         vehicle:set_target_angle_and_climbrate(0, cmd_rad, 0, 0, 0, 0)
      elseif AXIS == 3 then
         vehicle:set_target_angle_and_climbrate(0, 0, cmd_rad, 0, 0, 0)
      end
    end
  end

  return update, PERIOD_MS
end

return update()