# GAYAK: A SMART HEAT INDEX MONITORING AND EARLY WARNING SYSTEM WITH DUAL EARTHQUAKE AND FIRE ALARM DETECTION SYSTEM FOR CLASSROOMS

**Edit the relevant `.ino` source** in Arduino IDE for the physical device, or `wokwi/sketch.ino` for the simulator. They intentionally have different timing and buzzer handling. After changing constants, recompile and upload/restart. Make only one parameter change per experiment and log both the before/after values.

## Quick lookup

| Parameter | Function | Physical prototype default | Wokwi default |
| --- | --- | --- | --- |
| `SENSOR_INTERVAL` | DHT22 polling | 2500 ms | 2200 ms |
| `MQ2_WARMUP_MS` | Initial sensor warm-up | 120000 ms | 5000 ms |
| `MQ2_BASELINE_MS` | Normal-air averaging | 20000 ms | 3000 ms |
| `MQ2_CONFIRM_MS` | Extra time the delta must remain above threshold | 0 ms | 0 ms |
| `MQ2_TRIGGER_DELTA` | Filtered ADC rise above clean-air baseline that activates smoke | 4 | 4 |
| `MQ2_CLEAR_DELTA` | Delta below which an active smoke alert can clear | 2 | 2 |
| `MQ2_BASELINE_RISE_IDLE` | Idle baseline tracking of rising readings (0 disables it) | 0.0 | 0.0 |
| `MPU_SAMPLE_MS` | MPU read target interval | 20 ms (~50 Hz) | 20 ms |
| `MOTION_NOTICE_RMS_G` | Start displaying unconfirmed movement | 0.04 g | 0.04 g |
| `QUAKE_TRIGGER_RMS_G` | STRONG SHAKING / alert confirmation threshold | 0.12 g | 0.12 g |
| `INTENSE_SHAKING_RMS_G` | INTENSE SHAKING display threshold | 0.30 g | 0.30 g |
| `QUAKE_CLEAR_RMS_G` | Motion/vibration clear threshold | 0.06 g | 0.06 g |
| `QUAKE_CONFIRM_MS` | Required sustained motion | 800 ms | 500 ms |
| `FLAME_CLEAR_TIME` | Required quiet interval to clear | 1500 ms | 500 ms |
| `ALERT_ACK_REMINDER_MS` | Silence duration before a still-active alert is announced again | 30000 ms | 30000 ms |

These are **initial experimental defaults**, not approved alarm thresholds. If a source constant changes, treat the actual code as authoritative.

## 1. Adjust MQ-2 baseline and sensitivity

**Physical procedure**
1. Allow warm-up with the sensor in clean ambient air. The firmware waits 120 s, then collects a 20 s clean-air baseline on each startup. First-use or long-stored sensing elements may require additional manufacturer-recommended conditioning beyond this prototype startup delay.
2. Read Serial Monitor at **115200 baud**. Observe `RAW`, `FILTERED`, `BASE`, `DELTA` and `STATE`.
3. Record ten baseline/delta readings in normal ambient air at similar humidity and temperature.
4. The physical default is intentionally very sensitive: a filtered rise of 4 ADC counts above the clean-air baseline triggers on the first qualifying sample, with no added confirmation delay. Samples are taken about every 100 ms and a quick-rise filter is still applied, so this is not literally instantaneous. Idle baseline upward tracking is disabled so a gradual increase is not learned away as normal air; this can also cause nuisance alerts from environmental drift. If it false-triggers, investigate wiring, power-supply noise, warm-up, and baseline drift; then raise `MQ2_TRIGGER_DELTA` or add a small `MQ2_CONFIRM_MS` in controlled steps.
5. Keep `MQ2_CLEAR_DELTA` below `MQ2_TRIGGER_DELTA` for hysteresis. The current trigger/clear values are 4/2 counts. These are prototype response settings, not an absolute smoke concentration threshold.
6. Re-test three trials and record both detection and clearing times. The output is an **ADC delta, not calibrated smoke ppm**.

**Wokwi**: set initial MQ-2 gas level to its default during startup and only increase the simulated ppm after baseline calibration. The simulator's ppm control does **not** mean the physical sketch measures absolute ppm.

## 2. Adjust digital flame detection

**Physical flame module:** The small potentiometer typically controls a comparator threshold on DO. Turn in small increments while watching `FLAME_PIN` at 115200 baud; DO polarity may vary. The sketch can auto-learn idle DO during boot (`FLAME_AUTO_LEARN`) if the module is stable and no flame-like source is present. If unreliable, disable auto-learn and set `FLAME_ACTIVE_LOW` to match measured behavior. Avoid sun, strong reflections and lamps during setup.

**Wokwi:** There is **no simulated IR flame module** in this project. The photoresistor's `DO` goes LOW when simulated light exceeds its threshold (initial threshold 2.5V, roughly 100 lux for the stock module). The Wokwi sketch fixes `FLAME_ACTIVE_LOW=true` and `FLAME_AUTO_LEARN=false`. Click the photoresistor and change `lux` to see the input transition; this **does not calibrate real flame detection**.

## 3. Adjust MPU-6050 intense shaking and acceleration detection

For the acceleration processing method, four-level classifier, confirmation behavior, and classroom lab procedure, see [MPU-6050 shaking and acceleration guide](MPU6050_SHAKING.md).


GAYAK classifies **filtered, gravity-compensated local acceleration** as STABLE (<0.04 g), MOTION DETECTED (0.04–0.119 g), STRONG SHAKING (0.12–0.299 g), or INTENSE SHAKING (≥0.30 g). These are device labels, **not** earthquake magnitude or official intensity. The visual state responds to filtered acceleration, but the shaking alarm waits for the `QUAKE_CONFIRM_MS` confirmation interval. Avoid interpreting one tilt or brief change as a confirmed earthquake.

- Secure the MPU module to a stable surface. Hold it still for the `MPU_SETTLE_MS` start interval.
- Record `QUAKE,1,0,<rms_g>` values over at least several minutes of ordinary ambient movement and observe the normal noise level.
- Choose `QUAKE_TRIGGER_RMS_G` above that noise, and `QUAKE_CLEAR_RMS_G` sufficiently lower for hysteresis. Higher trigger = fewer nuisance alerts but lower sensitivity; lower trigger = more nuisance alerts.
- Increase `QUAKE_CONFIRM_MS` to require longer sustained motion; reduce only if controlled tests demonstrate acceptable false-positive rates.
- The device filters gravity and computes a smoothed motion metric, **not a seismograph waveform calibrated to Richter or moment magnitude**. Changing it to `7.0` does not mean the detector can identify a magnitude-7 earthquake.
- In Wokwi, changing a constant acceleration is a *step input*. Repeated X/Y changes better resemble a brief vibration. Simulated acceleration units are g, not Richter magnitude.
- Never represent GAYAK's displayed filtered acceleration as a measurement of actual earthquake magnitude or an early earthquake forecast.

## 4. Adjust DHT22 and heat index

- `TEMP_OFFSET` is a software temperature offset; adjust **only after comparing against a suitable calibrated thermometer** at the same measurement location. Keep default `0.0` without comparison.
- Keep DHT22 reads at least about **2 seconds apart**.
- Heat index is computed from temperature and relative humidity and is an estimated comfort/risk indicator. It does not replace PAGASA or other official advisories.
- A debounced D11 press acknowledges all active fire, smoke, shaking, and HI alerts; when none need acknowledgment, it advances the LCD panel. Panels no longer rotate automatically.

## 5. Adjust buzzer patterns

Edit the `*_PATTERN` step arrays. The physical Arduino sketch uses `{ buzzer mask, duration in ms }`; combine `BUZZER_MASK_*` values to sound multiple buzzers together, and use `BUZZER_MASK_NONE` for silence. Wokwi keeps one piezo pin per step. Both patterns are non-blocking and advance with `millis()`.

- **Smoke:** physical hardware pulses D8 + D10 together once per second; Wokwi sequences a short D8 pulse followed by a short D10 pulse because `tone()` supports one channel.
- **Fire:** three short D10 pulses, then a pause.
- **Shaking (physical hardware):** D8 + D9 together for 500 ms, D10 SFM for 500 ms, then pause for 150 ms. Wokwi approximates the first phase with its D9 piezo, then plays D10.
- **Fire + smoke:** two fast D10 pulses followed by a longer D8 + D10 pulse. Wokwi sequences D8 then D10 for that final phase.
- **HI Danger (41–<52°C):** repeating siren-style on/off cadence on D10. The active SFM-27 has a fixed tone, so it cannot perform a pitch sweep.
- **HI Extreme Danger (≥52°C):** yellow and red LEDs alternate every 500 ms; the SFM-27 on D10 plays a repeating triple-pulse warning.

When changing durations, preserve a distinct rhythm for each condition. Wokwi `tone()` drives piezo elements; physical active buzzers use DC drivers and cannot be made louder by changing a `tone()` frequency.

## Parameter change record

| Parameter | Old value | New value | Reason | Trial result | Restore? |
| --- | --- | --- | --- | --- | --- |
| | | | | | |

**Calibration is incomplete without observed measurements.** Do not write “accurate” or “safe” solely because code compiles or an LED lights.
