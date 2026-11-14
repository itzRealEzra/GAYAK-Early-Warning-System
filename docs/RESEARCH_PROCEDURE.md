---
title: GAYAK Research Procedure
tags:
  - gayak
  - research
  - methodology
  - testing
aliases:
  - Research Methodology Procedure
  - Chapter 3 Procedure
---

# GAYAK Research Procedure: Step By Step

This note provides a reproducible procedure for writing the **Research Methodology / Chapter 3** section for the GAYAK prototype. It covers development, calibration, functional trials, data recording, and analysis. It is a protocol template: enter the team's actual equipment, dates, trial readings, and results after performing the work.

> [!warning] Scope and safety
> GAYAK is an educational prototype, not a certified fire, smoke, gas, heat, or earthquake alarm. Do not create a fire, release smoke or fuel gas, or use the prototype for emergency decisions. Use simulation or safe controlled electrical inputs to test alarm logic. Report simulation and physical sensor observations separately.

> [!note] Human participants
> The proposed bench and simulation procedure does not require student participants. If the study adds surveys or classroom user evaluations, obtain the required adviser/school approval and informed consent first, and avoid collecting unnecessary identifying information.

## 1. Define The Research Scope

Use an engineering development and controlled functional-evaluation design. A suitable research scope is whether the prototype can acquire sensor inputs, calculate/display readings, and produce the programmed indicators under defined test conditions.

Do not describe this protocol as proving real-world fire detection accuracy, smoke concentration accuracy, earthquake detection, or life-safety reliability. The MQ-2 reports relative analog response, the flame module is digital-only, and the MPU-6050 reports local acceleration/shaking rather than earthquake magnitude.

Before testing, write down:

- Research question and objectives
- Which hardware revision and firmware commit will be evaluated
- Whether each trial is **Wokwi simulation**, **safe electrical input**, or **physical sensor observation**
- Required output for each test condition
- Pass/fail criteria and measurement tolerance
- Number of trials, instruments, and data-recording method

The current project [[TEST_PLAN]] proposes **three repeated functional trials per condition** as a pilot check. Three trials are not enough to establish broad reliability; use a larger, justified sample if the paper makes statistical performance claims.

## 2. Identify Variables And Instruments

| Study element | Suggested definition |
| --- | --- |
| Independent variable | Defined sensor input or scenario: temperature/RH, simulated MQ-2 level, flame DO state/proxy, MPU motion level, or alert combination |
| Dependent variables | Displayed readings, HI category, confirmed alert state, LCD text, LED state, buzzer pattern, serial/dashboard output, detection time, and clear time |
| Controlled conditions | Firmware commit, wiring, supply source, sensor placement, warm-up/baseline procedure, test interval, and selected threshold configuration |
| Instruments | Multimeter; reference thermometer/hygrometer for physical environmental readings; stopwatch or timestamped video/log for response timing; Arduino Serial Monitor/log; optional dashboard capture |

Record the manufacturer/model and stated accuracy of each reference instrument. Establish the allowed difference from reference readings **before** reviewing the results; do not choose a tolerance afterward to make trials pass.

## 3. Assemble And Document The Prototype

1. Assemble the physical circuit by following [[PROTOTYPE_BUILD_AND_WIRING]] and [[WIRING_AND_POWER]].
2. Photograph the completed circuit and label the sensors, power source, and Arduino connections.
3. Record the actual board and module models, firmware revision, pin map, buzzer driver parts, and whether the test uses USB or another validated supply.
4. Inspect polarity, shared ground, I2C voltage compatibility, LED resistors, buzzer drivers, and 5V-to-GND continuity before power-up.
5. Keep the battery and charger disconnected during initial work. Use a separate, documented power configuration only if it has been reviewed and validated.

The current physical pin map is D2 DHT22, D3 flame DO, A0 MQ-2 AO, A4/A5 shared LCD and GY-521 I2C, D5/D6/D7 green/yellow/red LEDs, D8/D9/D10 smoke/shaking/fire driver inputs, and D11 button-to-GND. D0/D1 remain reserved for USB serial.

## 4. Prepare The Firmware And Data Log

1. Upload `arduino/GAYAK/GAYAK.ino` to the Uno using the documented libraries and board settings.
2. Open Serial Monitor at **115200 baud** for development trials. Close it before testing through the web dashboard because both programs cannot use the same COM port at once.
3. Create one data sheet per test session. Assign each run a unique test ID, such as `SIM-MQ2-01` or `PHY-DHT-02`.
4. Synchronize the observer's clock and choose a response-time method. For short confirmation delays, timestamped video or a logger is more repeatable than human reaction time alone.
5. Save raw readings and logs before calculating summaries. Do not replace raw data with pass/fail labels.

## 5. Establish A Normal Baseline

1. Place the prototype in its documented test location, away from direct sunlight, drafts from heaters, and unrelated sources of infrared or vibration.
2. For physical trials, record date/time, room conditions, sensor placement, reference temperature/RH if available, supply voltage, and whether the area appears normal and clear.
3. Power the Uno by USB and record its startup LCD and LED sequence.
4. Aim the digital flame module at a normal indoor scene during its idle-level learning at startup.
5. Allow the MQ-2 to complete its **2-minute warm-up** and **20-second clean-air baseline calibration** before evaluating its alarm logic.
6. Record the settled DHT22 values, HI, MQ-2 raw/baseline/delta where available, flame DO state, MPU online/motion state, and dashboard connection state.
7. If an alert appears during baseline, stop, record it as an observation, check the setup, and repeat the baseline only after documenting the cause or correction.

Do not hide false alerts by deleting startup or baseline readings. Keep excluded or invalid runs with a reason in the notes.

## 6. Run Controlled Functional Trials

Change **one input at a time**, restore the baseline between conditions, and use the same wait/record method on every repetition. Run at least three repetitions per condition for the project's proposed pilot protocol. Mark every row as simulated, electrically injected, or physically observed.

| Test condition | Safe procedure | Record / expected behavior to compare |
| --- | --- | --- |
| Startup and calibration | Restart under documented normal conditions; do not introduce smoke or flame | Startup indications, MQ-2 warm-up/baseline duration, false alerts, serial status |
| Temperature and humidity | Use Wokwi controls or a controlled environment; for physical readings, place a reference meter near the DHT22 without touching it | DHT22 temperature/RH, firmware HI, LCD/dashboard value, difference from reference or independent HI calculation |
| HI category boundaries | In simulation or controlled input, set values below and around 27°C, 33°C, 41°C, and 52°C HI thresholds | Displayed HI category and LED/popup behavior; note that audible HI warning is used for Extreme Danger according to firmware |
| MQ-2 alert logic | Prefer Wokwi's MQ-2 control or a documented safe electrical test input with the sensor output safely isolated | Input/ADC or delta, confirmation time, LCD/LED/buzzer/serial alert and clear behavior |
| Flame alert logic | Use Wokwi's photoresistor only as a **digital-input proxy**, or a documented safe DO electrical test with the module isolated | DO/proxy state, confirmation and clear times, outputs; do not report the light proxy as a physical flame-sensitivity test |
| MPU motion logic | Use Wokwi controlled acceleration values or a documented repeatable, low-risk vibration fixture; secure the physical module | Filtered RMS g, motion level, alert confirmation/clear timing, LED/buzzer/serial/dashboard state |
| Combined smoke + flame | In simulation or controlled input only, activate both test signals | Confirm the programmed fire-plus-smoke priority/pattern; label the run simulated or electrical |
| Button acknowledgment | While an alert is active, press and release D11 once; also press it with no alert active | Whether alert is acknowledged, LCD panel advances when appropriate, and a persistent source re-arms as programmed |
| Dashboard telemetry | With the Arduino connected by USB and Serial Monitor closed, change safe test inputs | Arduino connection, live readings, event state, chart update, and disconnect/reconnect behavior |

For an electrical input test, document the exact method and voltage. Disconnect the sensor output before injecting another signal so two outputs are not driven against each other. If a safe, repeatable test signal cannot be provided, omit that physical functional trial and report the simulation result separately.

## 7. Measure Detection And Clear Times Consistently

For every alert trial:

1. Define the stimulus start as the first recorded time the input enters the test condition.
2. Define detection as the first confirmed alert output specified in advance (for example, LCD alert or LED state); do not switch definitions between trials.
3. Define stimulus removal as the recorded time the input returns to baseline.
4. Define clear as the first time the selected alert indication returns to its documented clear state.
5. Calculate detection time as `detection timestamp − stimulus start` and clear time as `clear timestamp − stimulus removal`.
6. Record buzzer timing separately if it has a different confirmation, priority, or acknowledgment delay.

The firmware filters and confirms several inputs. For example, flame confirmation is about 3 seconds; MQ-2 includes warm-up/baseline and a confirmation interval; MPU shaking requires sustained motion. Record those delays rather than labeling them as missed detections.

## 8. Use A Trial Recording Sheet

Record at least one row for every attempt, including failed or invalid runs:

| Test ID | Trial | Mode (simulation / electrical / physical) | Firmware commit | Input and value | Baseline | Detection time (s) | Clear time (s) | LCD / LED / buzzer | Serial / dashboard | Pass? | Notes |
| --- | ---: | --- | --- | --- | --- | ---: | ---: | --- | --- | --- | --- |
|  | 1 |  |  |  |  |  |  |  |  |  |  |
|  | 2 |  |  |  |  |  |  |  |  |  |  |
|  | 3 |  |  |  |  |  |  |  |  |  |  |

Keep screenshots, photographs, serial logs, reference-meter readings, and the completed sheet together. Protect any personal information if people or classroom records appear in images.

## 9. Analyze The Collected Data

1. Separate simulation, safe electrical-input, and physical ambient-observation data into distinct groups.
2. Check every row for missing units, timing method, firmware revision, or test mode. Explain exclusions without deleting the original record.
3. For numeric readings and times, report the trial count, mean, standard deviation, and range when the sample supports those summaries. With only three trials, describe results as preliminary.
4. For pass/fail checks, report `passed trials / valid trials` with the raw counts. Do not label this a field detection accuracy or certified reliability rate.
5. Compare HI against the specified independent calculation/reference method and the tolerance selected before testing.
6. List false alerts, missed expected outputs, stale readings, resets, and unexpected behavior, even if the overall trial passes.
7. Discuss limitations: MQ-2 is not calibrated to ppm, flame DO is threshold-based, the Wokwi photoresistor is not a real flame sensor, and MPU-6050 motion levels do not establish earthquake magnitude or cause.

## 10. Write The Methodology Section

Adapt this paragraph only after the procedure has actually been completed. Replace every bracketed item with the team's real details; remove any test type that was not performed.

> The study used an engineering development and functional-evaluation design to construct and assess a GAYAK classroom monitoring prototype. The prototype used [actual Arduino board], [actual sensor/module models], and firmware revision [commit or version]. Testing was conducted using [Wokwi simulation / safe electrical input / physical ambient observation], with [number] repetitions for each defined condition. Before trials, the MQ-2 completed its programmed warm-up and baseline calibration, and the flame module was started under the documented normal indoor condition. The team recorded [actual dependent variables] using [actual instruments and timing method]. Results were summarized using [actual calculations]. No open flame, intentional smoke, or fuel gas was introduced. Simulation outcomes were analyzed separately from physical sensor observations.

Do not write expected behavior as if it were an observed result. Report actual measured values and limitations in the Results and Discussion sections.

## Related Notes

- [[PROTOTYPE_BUILD_AND_WIRING]] — physical build and pin-by-pin wiring
- [[TEST_PLAN]] — proposed firmware acceptance tests and blank trial table
- [[WIRING_AND_POWER]] — buzzer drivers, I2C voltage, and optional battery notes
- [[ADJUSTMENTS]] — sensor thresholds and timing parameters
- [[TROUBLESHOOTING]] — sensor and dashboard fault checks
