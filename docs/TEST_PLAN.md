# GAYAK: A SMART HEAT INDEX MONITORING AND EARLY WARNING SYSTEM WITH DUAL EARTHQUAKE AND FIRE ALARM DETECTION SYSTEM FOR CLASSROOMS

**Status:** These are proposed acceptance tests, **not recorded results**. The project has not been certified or independently validated as a life-safety warning device.

## Setup and baseline

- Build and connect the Wokwi circuit yourself using [manual Wokwi wiring](../wokwi/README.md); load current `wokwi/sketch.ino` and `wokwi/libraries.txt`. Use **three 220Ω LED resistors**, one per indicator.
- Set DHT22 to comfortable baseline values, MQ-2 to initial 400 ppm simulation value, photoresistor to **low lux**, and MPU-6050 to X=0, Y=0, Z=1 g.
- Start Serial Monitor at 115200 baud. Record initial LCD and LED behavior.
- Allow the shorter Wokwi MQ-2 warm-up and baseline to complete; for a real device, allow the physical intervals.
- Change one input at a time, restore baseline, then deliberately combine simultaneous alerts.

> For the distinction between motion-level labels and confirmed alarms, and an MPU-specific repeatable testing procedure, read the [MPU-6050 guide](MPU6050_SHAKING.md).

## Functional trials (record each three times)

| Test | Stimulus | Expected after confirmation | Clear condition |
| --- | --- | --- | --- |
| DHT22 temperature | Raise simulated temperature | Updated temperature and heat index at next sensor read/LCD panel | Return value |
| DHT22 humidity | Change simulated humidity | Updated humidity and recalculated heat index | Return value |
| Normal LEDs | Normal sensors after calibration | D5 green ON, D6/D7 OFF | Recalibrate or alarm |
| MQ-2 smoke | Raise gas control after baseline | D6 yellow + D8 repeating, LCD smoke alert | Lower gas/delta; clear delay |
| Flame proxy | Increase photoresistor lux above DO threshold | D7 red + D10→D8→D9, LCD fire alert | Lower lux; clear delay |
| MPU-6050 strong shaking | Repeatedly change X/Y acceleration until filtered reading reaches at least 0.12 g | After sustained confirmation: D7 blinking, D9 rapid buzzer, STRONG SHAKING, `QUAKE,...,STRONG` | Return to steady reading; allow clear/hold delay |
| MPU-6050 intense shaking | Repeatedly change X/Y acceleration until filtered reading reaches at least 0.30 g | After sustained confirmation: D7 blinking, D9 rapid buzzer, INTENSE SHAKING, `QUAKE,...,INTENSE` | Return to steady reading; allow clear/hold delay |
| Alert priority | Trigger smoke, then fire | Fire takes priority over smoke sounds | Remove fire; smoke should resume if still active |
| Sensor unavailable | Carefully simulate missing MPU only if supported | Not falsely green all-clear | Restore sensor |
| Serial telemetry | Change input while monitoring | `DATA` and `QUAKE` records continue without corrupting fields | N/A |
| Manual Wokwi wiring | Inspect your own circuit | D5 green, D6 yellow, D7 red, each with 220Ω; no extra buttons or piezo buzzer resistors | Recheck all wires |

**Important nuance:** “Earthquake test” in this table means an **artificial motion input** to the MPU-6050. Neither a static force setting nor a simulated wobble reproduces real seismic propagation.

## Trial recording form

| Test | Trial | Input value / ADC / g | Detection time (s) | Clear time (s) | LCD | LED | Buzzer | Pass/Fail | Remarks |
| --- | ---: | --- | ---: | ---: | --- | --- | --- | --- | --- |
| Smoke | 1 | | | | | | | | |
| Smoke | 2 | | | | | | | | |
| Smoke | 3 | | | | | | | | |
| Fire-input proxy | 1 | | | | | | | | |
| Fire-input proxy | 2 | | | | | | | | |
| Fire-input proxy | 3 | | | | | | | | |
| Vibration | 1 | | | | | | | | |
| Vibration | 2 | | | | | | | | |
| Vibration | 3 | | | | | | | | |

## Suggested reliability exercises

1. **Startup repeatability:** restart three times; no false alert during normal conditions.
2. **Simultaneous events:** apply fire and shaking, then smoke, and verify sound priorities.
3. **Input clearing:** restore normal conditions, measure delay, and check repeated alarm suppression.
4. **Serial stability:** watch for malformed records or dashboard disconnections during changes.
5. **Electrical review:** before real-power experiments, verify no 5V-GND short, safe buzzer driver current, compatible I2C logic and correct power source; do not attempt unsafe failure injection.
6. **Power continuity:** use a current-limited bench supply and measure regulated 5V stability; do not use an unprotected lithium battery to simulate failures.

## Pass/fail criteria

A trial passes only if the observed LCD, LED, buzzer, and serial behavior match the documented expected state **after** the specified filter/confirmation delays. Record actual detection and clearing times and anomalous events; do not claim “100% accuracy” from three simulator trials. Simulation results validate **program logic**, not physical sensor accuracy or field suitability.

## Classroom report

Include purpose, module list, pin table, wiring screenshot, test environment, measured readings, three trials, limitations, parameter changes, observed bugs, and corrective actions. Evaluator categories may include functionality, accuracy of displayed **prototype data**, quality, workability and electrical safety practices.