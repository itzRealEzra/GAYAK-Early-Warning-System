# MPU-6050 Intense Shaking and Acceleration Detection

**Project:** GAYAK: A SMART HEAT INDEX MONITORING AND EARLY WARNING SYSTEM WITH DUAL EARTHQUAKE AND FIRE ALARM DETECTION SYSTEM FOR CLASSROOMS

## Purpose and limits

In GAYAK, the **GY-521 MPU-6050 is an accelerometer-based local shaking sensor**. It measures acceleration along the X, Y and Z axes and is used to identify **sustained strong and intense shaking** of the surface on which the module is mounted. It does **not** measure earthquake magnitude, predict earthquakes, confirm that a vibration was seismic, or provide a certified earthquake intensity rating.

Although the MPU-6050 contains a gyroscope, the current GAYAK firmware uses its **three-axis accelerometer only** for the shaking classifier. Gyroscope readings are not used to make the alarm decision.

## Signals and wiring

| Arduino Uno pin | GY-521 MPU-6050 | Shared with |
| --- | --- | --- |
| A4 | SDA | LCD SDA |
| A5 | SCL | LCD SCL |
| GND | GND | Common ground |
| Voltage-qualified supply | VCC | Check the particular GY-521 module |

The MPU chip is 3.3V logic; a 5V-powered LCD backpack may pull SDA and SCL to 5V. Use a verified compatible level-shifted I2C arrangement. The MPU address is normally `0x68` (AD0 LOW), while the LCD is generally `0x27`.

## How the firmware evaluates shaking

1. Configure the accelerometer for a **±2g full-scale range**.
2. Target **50 samples per second** (20ms interval).
3. Calculate X/Y/Z acceleration in gravitational units (`g`; 1g approximately 9.81 m/s²).
4. Estimate slowly changing gravity on each axis and subtract it from each sample. This helps reject tilt and stationary gravity but does **not** guarantee rejection of all handling artifacts.
5. Smooth the **squared vector residual** and take its square root to obtain a gravity-compensated RMS-like acceleration estimate in `g`.
6. Classify the reading immediately into one of four **experimental, device-specific motion levels**. A visual level transition may occur before an alarm.
7. Activate the shaking alarm only when the signal exceeds its alert threshold for a **continuous confirmation interval**. Clear after sustained quiet plus a minimum hold interval.

The smoothing gives a **filtered acceleration score**, not calibrated ground-acceleration intensity, earthquake magnitude, or a seismograph waveform. The raw accelerometer can also saturate above its selected ±2g range.

## Motion classifications

| Display level | Filtered RMS-like acceleration | Alarm |
| --- | --- | --- |
| STABLE | Below 0.04g | No shaking alarm |
| MOTION DETECTED | 0.04g to below 0.12g | Monitor only |
| STRONG SHAKING | 0.12g to below 0.30g | Alarm **after confirmation** |
| INTENSE SHAKING | At least 0.30g | Alarm **after confirmation** |

These labels and thresholds are **software definitions for this prototype**, not PHIVOLCS/PAGASA/Mercalli classifications. A brief sharp motion can temporarily show a high level without triggering the sustained-shaking buzzer.

## Physical vs. Wokwi timing

| Setting | Physical sketch | Wokwi sketch |
| --- | ---: | ---: |
| `MPU_SAMPLE_MS` | 20ms | 20ms |
| `MPU_SETTLE_MS` | 2,000ms | 2,000ms |
| `QUAKE_TRIGGER_RMS_G` | 0.12g | 0.12g |
| `INTENSE_SHAKING_RMS_G` | 0.30g | 0.30g |
| `QUAKE_CLEAR_RMS_G` | 0.06g | 0.06g |
| `QUAKE_CONFIRM_MS` | 800ms | 500ms |
| `QUAKE_CLEAR_MS` | 2,500ms | 1,200ms |
| `QUAKE_MIN_HOLD_MS` | 5,000ms | 5,000ms |

Wokwi is accelerated for classroom demonstrations; **these are defaults, not calibrated field settings**.

## Student testing tutorial

1. Start the Wokwi simulation using `wokwi/diagram.json` and `wokwi/sketch.ino`, or power the real board over USB with safe buzzer drivers.
2. Hold the MPU-6050 stationary for at least the startup settling period; allow the LCD to initialize.
3. In the Serial Monitor at **115200 baud**, inspect messages beginning with `QUAKE,`. The record fields are `QUAKE,online,alert,filtered_rms_g,level`.
4. In Wokwi, **repeatedly** change the X or Y acceleration back and forth, rather than leaving one nonzero value fixed. Record when the displayed filtered reading passes 0.04g, 0.12g, and 0.30g.
5. Hold a sustained simulated shaking pattern long enough to trigger the D9 shaking buzzer and flashing red D7 LED. Note the confirmation interval.
6. Stop adjusting acceleration and wait for the quiet/hold intervals. Record the time until the alarm clears.
7. Repeat a **minimum of three trials** at each test condition and log unexpected triggering, missing readings, or I2C failures.

Do not violently shake a live battery assembly, soldered battery wiring, or exposed prototype. Fix the board mechanically and use a controlled safe fixture for physical bench tests.

## Adjusting sensitivity responsibly

- Record the normal resting filtered acceleration in the intended classroom mounting location.
- Set `QUAKE_TRIGGER_RMS_G` above observed movement noise, then test meaningful mechanical shaking using a repeatable and safe method.
- Keep `QUAKE_CLEAR_RMS_G` lower than the trigger threshold for hysteresis.
- Increase `QUAKE_CONFIRM_MS` to suppress short bumps; decreasing it can increase nuisance alarms.
- Use `INTENSE_SHAKING_RMS_G` for the **display classification**, not for mapping to Richter magnitude.
- Do not call the alarm accurate until detection, clearance, and false-alarm behavior have been measured under controlled conditions.

## Safety and limitations

The device responds to local movement where installed. Classroom desk tapping, hand movement, footsteps, machine vibration, vehicle movement, and temperature-related sensor drift may cause readings. An earthquake elsewhere may not register. A single MPU-6050 is not a replacement for certified seismic monitoring or official earthquake advisories.

See [adjustments](ADJUSTMENTS.md), [test plan](TEST_PLAN.md), and [physical power wiring](WIRING_AND_POWER.md) for the complete educational prototype procedures.
