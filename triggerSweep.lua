---@diagnostic disable: undefined-global

local AXIS = 1         -- 1=Roll, 2=Pitch, 3=Yaw
local CH_SWITCH  = 8   -- RC channel used as ON/OFF switch

-- Configuration
local AMP_START_DEG = 5.0  -- Starting Amplitude [deg or deg/s]
local AMP_END_DEG   = 20.0 -- Ending Amplitude [deg or deg/s]
local FREQ_START    = 0.5  -- Starting frequency [Hz]
local FREQ_END      = 2.0  -- Ending frequency [Hz]
local SWEEP_TIME    = 30.0 -- Time to sweep [seconds]
local PERIOD_MS     = 20   -- Update every 20 ms (~50 Hz)

local active = false
local start_time = 0
local sweep_complete = false


function update()
  local sw = rc:get_pwm(CH_SWITCH) or 1500

  if sw > 1700 then
    if not active then
      local axis_names = {"Roll", "Pitch", "Yaw"}
      gcs:send_text(0, string.format("Sweep %s: %.1f-%.1f Hz, %.1f-%.1f Amp over %.0f s", axis_names[AXIS], FREQ_START, FREQ_END, AMP_START_DEG, AMP_END_DEG, SWEEP_TIME))
      active = true
      start_time = millis():tofloat()
      sweep_complete = false

      -- Ensure we are in a mode that accepts angle targets (like GUIDED)
      if vehicle:get_mode() ~= 4 and vehicle:get_mode() ~= 15 then
         gcs:send_text(0, "WARNING: Switch to GUIDED mode!")
      end
    end

    if not sweep_complete then
      local current_time = millis():tofloat()
      local elapsed = (current_time - start_time) / 1000.0
      local progress = elapsed / SWEEP_TIME

      if progress >= 1.0 then
        progress = 1.0
        sweep_complete = true
        gcs:send_text(0, "Sweep complete")
        active = false
        vehicle:set_target_angle_and_climbrate(0, 0, 0, 0, 0, 0)
      else
        -- Interpolate Frequency and Amplitude
        local freq = FREQ_START + (FREQ_END - FREQ_START) * progress
        local amp = AMP_START_DEG + (AMP_END_DEG - AMP_START_DEG) * progress

        local t = current_time / 1000.0
        local angle_rad = 2.0 * math.pi * freq * t
        local cmd_val = amp * math.sin(angle_rad)
        local cmd_rad = math.rad(cmd_val)

        -- Send Target
        local r, p, y = 0, 0, 0

        if AXIS == 1 then     -- Roll Angle
          r = cmd_rad
        elseif AXIS == 2 then -- Pitch Angle
          p = cmd_rad
        elseif AXIS == 3 then -- Yaw (Rate?)
          y = cmd_rad -- User note: Yaw param often interpreted as Rate
        end

        vehicle:set_target_angle_and_climbrate(r, p, y, 0, 0, 0)
      end
    end
  else
    if active then
      gcs:send_text(0, "Sweep stopped")
      active = false
      sweep_complete = false
      vehicle:set_target_angle_and_climbrate(0, 0, 0, 0, 0, 0)
    end
  end

  return update, PERIOD_MS
end

gcs:send_text(0, "==============Lua Oscillation script loaded===============")
return update() 