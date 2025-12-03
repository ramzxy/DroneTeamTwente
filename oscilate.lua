---@diagnostic disable: undefined-global

local CH_TEST    = 1      -- Channel to oscillate: 1=Roll, 2=Pitch, 4=Yaw
local CH_SWITCH  = 8      -- RC channel used as ON/OFF switch
local AMP        = 200    -- Amplitude [us] around mid stick
local FREQ_START = 0.5    -- Starting frequency [Hz]
local FREQ_END   = 15.0   -- Ending frequency [Hz]
local SWEEP_TIME = 30.0   -- Time to sweep from start to end [seconds]
local MID        = 1500   -- Neutral PWM value
local PERIOD_MS  = 20     -- Update every 20 ms (~50 Hz)
local TIMEOUT_MS = 100    -- Safety timeout for servo override (ms)

local active = false
local start_time = 0
local sweep_complete = false

function update()
  local sw = rc:get_pwm(CH_SWITCH) or 1500 -- default to 1500 if no switch is found
  if sw > 1700 then
    if not active then
      gcs:send_text(0, string.format("Frequency sweep starting: %.1f Hz to %.1f Hz over %.0f sec", FREQ_START, FREQ_END, SWEEP_TIME))
      active = true
      start_time = millis() / 1000.0
      sweep_complete = false
    end
    
    if not sweep_complete then
      local current_time = millis()
      if not current_time then
        return update, PERIOD_MS
      end
      
      local elapsed = (current_time / 1000.0) - start_time
      local freq = FREQ_START + (FREQ_END - FREQ_START) * (elapsed / SWEEP_TIME)
      
      if freq >= FREQ_END then
        freq = FREQ_END
        sweep_complete = true
        gcs:send_text(0, "Frequency sweep complete - stopping")
        SRV_Channels:set_output_pwm_chan_timeout(CH_TEST, MID, TIMEOUT_MS)
        active = false
      else
        local t = current_time / 1000.0
        local angle = 2.0 * math.pi * freq * t
        local pwm = MID + AMP * math.sin(angle)
        SRV_Channels:set_output_pwm_chan_timeout(CH_TEST, math.floor(pwm + 0.5), TIMEOUT_MS)
      end
    end
  else
    if active then
      gcs:send_text(0, "Sweep stopped by switch")
      active = false
      sweep_complete = false
    end
    SRV_Channels:set_output_pwm_chan_timeout(CH_TEST, MID, TIMEOUT_MS)
  end
  return update, PERIOD_MS
end

gcs:send_text(0, "==============Lua Oscillation script loaded===============")
return update()
