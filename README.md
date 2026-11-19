# GAYAK: A SMART HEAT INDEX MONITORING AND EARLY WARNING SYSTEM WITH DUAL EARTHQUAKE AND FIRE ALARM DETECTION SYSTEM FOR CLASSROOMS

**Arduino Uno R3 | Temperature · Humidity · HI · Gas/Smoke · Flame · Vibration**

GAYAK is a **student-built hazard-monitoring prototype**. It displays live measurements on a 16×2 I2C LCD and sends USB serial telemetry to an optional **offline Flask dashboard** running locally on a Windows PC. It also uses three status LEDs and three active-buzzer outputs.

> **Educational prototype, not a certified life-safety product.** The MPU-6050 detects local shaking, **not earthquake magnitude, epicenter, or proof that an earthquake occurred**. MQ-2 readings are relative analog measurements, not certified gas concentrations. Do not depend on GAYAK for evacuation or personal safety decisions.

## Start here

| I want to… | Open |
| --- | --- |
| Learn what the project does | [Student guide](docs/STUDENT_GUIDE.md) |
| Write the research methodology step by step | [Research procedure](docs/RESEARCH_PROCEDURE.md) |
| Build and wire the Wokwi circuit yourself | [Manual Wokwi tutorial](docs/TUTORIALS.md#1-wokwi-simulation) |
| Build and wire the physical prototype step by step | [Prototype build and wiring guide](docs/PROTOTYPE_BUILD_AND_WIRING.md) |
| Review buzzer drivers and power safety | [Wiring and power guide](docs/WIRING_AND_POWER.md) |
| Change sensitivity and timing | [Adjustments and calibration](docs/ADJUSTMENTS.md) |
| Understand intense shaking and acceleration | [MPU-6050 detection guide](docs/MPU6050_SHAKING.md) |
| Run the local web dashboard | [Dashboard tutorial](docs/TUTORIALS.md#3-windows-local-dashboard) |
| Find a wiring/programming problem | [Troubleshooting](docs/TROUBLESHOOTING.md) |
| Evaluate the trainer in a laboratory | [Test plan and checklists](docs/TEST_PLAN.md) |
| Edit the latest simulator | [Wokwi project files](wokwi/README.md) |

**Wokwi starter:** https://wokwi.com/projects/478926664159746049  
**GitHub:** https://github.com/itzRealEzra/GAYAK-Early-Warning-System

## Features

- **DHT22:** temperature and relative humidity feed the PAGASA heat-index method; the LCD and dashboard label this calculated value **HI**.
- **MQ-2:** very sensitive relative gas/smoke response, with a 2-minute warm-up and 20-second clean-air baseline. After calibration, a filtered rise of 4 ADC counts above baseline triggers on the first qualifying sample with no extra confirmation delay; idle baseline tracking is disabled so gradual rises are not absorbed. Environmental drift can cause nuisance alerts. It can also react to other gases and is not a calibrated or certified smoke alarm.
- **Flame module:** digital flame indication on the **physical hardware**, confirmed over 3 seconds to reject brief light changes. Aim it into the room away from windows/sky, adjust its sensitivity potentiometer in place, and use an opaque hood to narrow its field of view. Its digital output can still respond to a steady infrared source that crosses its threshold.
- **GY-521 / MPU-6050:** measures three-axis acceleration and detects intense shaking using filtered acceleration; it does not determine earthquake magnitude.
- **LCD / D11 button:** one press acknowledges all currently active fire, smoke, shaking, and HI alerts, clears their LCD overlays and dashboard alert banners, silences their buzzer patterns, and dismisses an active HI popup. Fire/smoke/shaking LED alerts clear; HI LEDs continue to show the current heat band. Sensor readings remain active; if a hazard persists, its alert returns after a 30-second silence. When there is no new alert to acknowledge, D11 advances the LCD panel.
- **Heat-index LEDs after MQ-2 calibration:** Safe (<27°C) is green; Caution (27–<33°C) yellow; Extreme Caution (33–<41°C) yellow + red; Danger (41–<52°C) red; Extreme Danger (≥52°C) alternates yellow, then red, every 500 ms.
  - **Heat-index popup:** the local dashboard opens a warning at HI ≥41°C. Its dismiss button or a valid HI reading below 41°C clears it; pressing D11 acknowledges the active firmware alerts and syncs HI acknowledgment to clear the popup.
- **Distinct buzzer alarms:** D10 drives the SFM-27 in every alarm pattern. Smoke adds a D8 pulse alongside it; shaking sounds D8 + D9 together, then D10. Fire gives three short D10 pulses, and combined fire + smoke gives two fast D10 pulses followed by a longer D8 + D10 pulse. HI Danger (41–<52°C) uses a siren-style D10 pulse cadence; HI Extreme Danger (≥52°C) uses faster triple D10 pulses. The active SFM-27 has a fixed tone, so the siren effect comes from its on/off rhythm rather than a pitch sweep. D11 temporarily silences all currently active sources; alarms repeat after 30 seconds if a source remains active, and also re-arm after clearing and triggering again. Priority is FIRE + SMOKE > FIRE > SMOKE > HI > SHAKING.
- **Offline dashboard:** Flask + PySerial + Server-Sent Events (SSE); no database, Wi-Fi, cloud, or internet needed after setup.

## Current Arduino Uno pinout

| Arduino pin | Physical function |
| --- | --- |
| D2 | DHT22 DATA |
| D3 | Flame sensor DO |
| D5 | **GREEN LED** via 220Ω resistor |
| D6 | **YELLOW LED** via 220Ω resistor |
| D7 | **RED LED** via 220Ω resistor |
| D8 | 5V active buzzer #1, adds sound for SMOKE and first SHAKING phase (use rated driver) |
| D9 | 5V active buzzer #2, first SHAKING phase (use rated driver) |
| D10 | SFM-27 3–24V active buzzer, common output for all alarms; second SHAKING phase (use rated driver) |
| D11 | Panel button; connect button between D11 and GND (`INPUT_PULLUP`) |
| A0 | MQ-2 AO |
| A4 | LCD + MPU-6050 SDA |
| A5 | LCD + MPU-6050 SCL |
| 5V / GND | Regulated supply and common ground |

Digital pins **D0 and D1 are reserved for USB Serial**. The real device does not require D4, D12, or A1. The LCD usually uses I2C address `0x27` and the MPU-6050 `0x68`; verify with an I2C scanner if needed.

## Alert priority

| State | LED response | Buzzer response |
| --- | --- | --- |
| Normal after calibration | Green solid | Silent |
| MQ-2 warming up | Red solid | Silent |
| MQ-2 baseline calibration | Yellow solid | Silent |
| MPU offline | Yellow solid (when otherwise clear) | Silent |
| HI Safe (<27°C) | Green solid | Silent |
| HI Caution (27–<33°C) | Yellow solid | Silent |
| HI Extreme Caution (33–<41°C) | Yellow + red solid | Silent |
| HI Danger (41–<52°C) | Red solid; warning returns after 30-second D11 silence if HI stays high | Siren-style D10 pulse cadence |
| HI Extreme Danger (≥52°C) | Yellow, then red, alternating every 500 ms; warning returns after 30-second D11 silence if HI stays high | Repeating triple-pulse D10 pattern |
| Unacknowledged gas/smoke | Yellow solid | D8 + D10 repeating |
| Unacknowledged fire + smoke | Red solid | Two fast D10 pulses, then a longer D8 + D10 pulse |
| Unacknowledged sustained shaking | Red fast blink | D8 + D9 together, then D10 SFM |
| Unacknowledged flame/fire | Red solid | Three short pulses on D10 |

### Shaking and acceleration detection (experimental)

| Level | Filtered local acceleration |
| --- | --- |
| STABLE | Below 0.04 g |
| MOTION DETECTED | 0.04 to below 0.12 g |
| STRONG SHAKING | 0.12 to below 0.30 g |
| INTENSE SHAKING | 0.30 g and higher |

These thresholds are **prototype starting values**, not validated seismic intensity values or a prediction of earthquakes. Alarm activation requires sustained filtered acceleration, so a displayed motion level may change before a buzzer sounds. Tilting or handling the MPU-6050 can also affect the reading. Validate against normal classroom vibrations and mount the sensor securely.

**Sound priority: FIRE + SMOKE > FIRE > SMOKE > HI > SHAKING.** When both smoke and flame are detected, the combined pattern plays. Only one alarm category runs at a time; every pattern uses D10, and the shaking pattern keeps D8 and D9 together before D10. LCD fire/smoke status remains fire-first.

## Simulated vs. physical hardware

The simulator uses a **Wokwi photoresistor module as an adjustable-light proxy** for the flame sensor, because it can expose a controllable DO signal. Changing *lux* is an **input demonstration, not a real flame detection test**. A pushbutton between **D11 and GND** acknowledges all active alerts; when no alert needs acknowledgment, it advances the LCD panel. Use the built-in DHT22, MQ-2, MPU-6050, and light-proxy properties to test readings.

Wokwi models **piezo** buzzers that require `tone()`. Your real **two-terminal active buzzers** use DC on/off through appropriately rated MOSFET/transistor driver circuits. **Do not copy the direct-connected Wokwi buzzer wiring to the real build.** Wokwi has **no real TP4056/18650 charging circuit** in this project.

- `arduino/GAYAK.ino` — **physical-hardware** firmware (active-buzzer DC control).
- `wokwi/sketch.ino` — **simulator-only** firmware (piezo `tone()`, faster simulation timings).
- **Manual Wokwi wiring:** use [`wokwi/README.md`](wokwi/README.md) to place and connect the components yourself, with 220Ω per LED. A premade `diagram.json` is not required.
- `wokwi/libraries.txt` — simulator dependencies.

Changing GitHub files **does not automatically update the hosted Wokwi URL**. Manually wire your own Wokwi project; copy only the simulator program `wokwi/sketch.ino` and dependencies `wokwi/libraries.txt`. Wokwi saves your custom circuit automatically.

## Arduino software

Arduino IDE libraries: **DHT sensor library (Adafruit)**, **Adafruit Unified Sensor**, and a compatible **LiquidCrystal_I2C** library. `Wire` is included with the Arduino AVR core; no extra MPU library is required by the supplied firmware. `arduino/GAYAK/GAYAK.ino` is the canonical physical sketch; open it in Arduino IDE for manual uploads. The dashboard uploader uses this same file. `arduino/GAYAK.ino` is a legacy copy and is not used for automatic uploads.

### PAGASA heat index

The firmware recalculates HI from each valid DHT22 temperature and relative-humidity reading. It uses the simple low-range estimate and Rothfusz regression with humidity adjustments described in PAGASA heat-index guidance. HI categories follow PAGASA thresholds: Caution (27–32°C), Extreme Caution (33–41°C), Danger (42–51°C), and Extreme Danger (52°C and above). The HI is an estimated apparent temperature, not a medical measurement. See [PAGASA heat-index guidance](https://pubfiles.pagasa.dost.gov.ph/climps/climateforum/speciallecture.pdf).

## Local desktop dashboard (Windows)

Read the [Windows dashboard setup guide](INSTALLATION.md), or use the **Setup & Installation** link at the bottom of the running dashboard. Install Python 3, extract the repository, then run `install_gayak.bat` once to download Flask and PySerial into the project-local `.venv`. Arduino IDE is not required; the dashboard launcher uses Arduino CLI to build and upload the firmware.

Connect the Uno over USB, close Arduino Serial Monitor/Plotter if open, then run `start_gayak.bat`. Before Flask starts, it detects the Arduino, compiles and uploads `arduino/GAYAK/GAYAK.ino`, then opens **http://127.0.0.1:5000**. The launcher downloads Arduino CLI, the AVR board core, and missing libraries on first use. Each normal launch uploads the current repository firmware and reboots the Uno once. To use the already-flashed firmware without uploading, set `GAYAK_SKIP_UPLOAD=1`. If port detection is ambiguous, set `GAYAK_COM_PORT=COM4` in Command Prompt before launching, replacing COM4 with the Arduino's actual port.

USB Serial runs at **115200 baud**. `DATA` CSV telemetry carries temperature, humidity, heat status, MQ-2 and flame readings, plus HI acknowledgment and a four-bit alert acknowledgment mask. The local parser accepts the current 15-field record and older 10-/13-/14-field records. Separate `QUAKE,online,alert,rms_g,level` records carry MPU status; the parser also supports the earlier four-field format. The dashboard displays vibration, sensor health, and shaking events.

## Portable battery system — safety warning

A standard TP4056 USB-C charger is a **one-cell / 1S Li-ion charger**, not a 5V regulator or automatic load-sharing module. **Never connect a two-cell 7.4V series holder to a TP4056**. A compatible protected 1S battery or properly engineered, matched parallel 1S pack needs the **correct protection, fuse, SPST switch, suitably sized regulated 5V boost converter, and USB/battery power-source isolation**. Do not charge while running unless a suitable power-path design is implemented. **For initial classroom tests, use USB power and leave the battery disconnected.**

See [Physical wiring and power](docs/WIRING_AND_POWER.md) for an explicit connection guide and limitations.

## Project status

The files are statically checked for key pin and component references. **A live Wokwi run, a physical circuit validation, battery safety testing, and alarm-volume testing are still required.** Instructors should use the [test plan](docs/TEST_PLAN.md) to record actual results.
