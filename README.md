# GAYAK: A SMART HEAT INDEX MONITORING AND EARLY WARNING SYSTEM WITH DUAL EARTHQUAKE AND FIRE ALARM DETECTION SYSTEM FOR CLASSROOMS

**Arduino Uno R3 | Temperature · Humidity · HI · Gas/Smoke · Flame · Vibration**

GAYAK is a **student-built hazard-monitoring prototype**. It displays live measurements on a 16×2 I2C LCD and sends USB serial telemetry to an optional **offline Flask dashboard** running locally on a Windows PC. It also uses three status LEDs and three alternating buzzer channels.

> **Educational prototype, not a certified life-safety product.** The MPU-6050 detects local shaking, **not earthquake magnitude, epicenter, or proof that an earthquake occurred**. MQ-2 readings are relative analog measurements, not certified gas concentrations. Do not depend on GAYAK for evacuation or personal safety decisions.

## Start here

| I want to… | Open |
| --- | --- |
| Learn what the project does | [Student guide](docs/STUDENT_GUIDE.md) |
| Build and wire the Wokwi circuit yourself | [Manual Wokwi tutorial](docs/TUTORIALS.md#1-wokwi-simulation) |
| Build a physical circuit | [Wiring and power guide](docs/WIRING_AND_POWER.md) |
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
- **MQ-2:** relative gas/smoke response, with a 2-minute warm-up, 20-second baseline calibration, and alert confirmation.
- **Flame module:** digital flame indication on the **physical hardware**.
- **GY-521 / MPU-6050:** measures three-axis acceleration and detects intense shaking using filtered acceleration; it does not determine earthquake magnitude.
- **LCD:** rotating sensor panels with priority alert messages.
- **Status LEDs:** Green / Yellow / Red and condition-specific blinking.
- **Three alarms:** fire alternates SFM-27 → smoke buzzer → shaking buzzer; smoke and shaking have separate patterns.
- **Offline dashboard:** Flask + PySerial + Server-Sent Events (SSE); no database, Wi-Fi, cloud, or internet needed after setup.

## Current Arduino Uno pinout

| Arduino pin | Physical function |
| --- | --- |
| D2 | DHT22 DATA |
| D3 | Flame sensor DO |
| D5 | **GREEN LED** via 220Ω resistor |
| D6 | **YELLOW LED** via 220Ω resistor |
| D7 | **RED LED** via 220Ω resistor |
| D8 | 5V active buzzer #1, SMOKE (use rated driver) |
| D9 | 5V active buzzer #2, SHAKING (use rated driver) |
| D10 | SFM-27 3–24V active buzzer, FIRE (use rated driver) |
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
| Gas/smoke | Yellow solid | D8 repeating |
| Sustained shaking | Red fast blink | D9 rapid pulses |
| Flame/fire | Red solid | D10 → D8 → D9 alternating |

### Shaking and acceleration detection (experimental)

| Level | Filtered local acceleration |
| --- | --- |
| STABLE | Below 0.04 g |
| MOTION DETECTED | 0.04 to below 0.12 g |
| STRONG SHAKING | 0.12 to below 0.30 g |
| INTENSE SHAKING | 0.30 g and higher |

These thresholds are **prototype starting values**, not validated seismic intensity values or a prediction of earthquakes. Alarm activation requires sustained filtered acceleration, so a displayed motion level may change before a buzzer sounds. Tilting or handling the MPU-6050 can also affect the reading. Validate against normal classroom vibrations and mount the sensor securely.

**Priority: FIRE > SMOKE > SHAKING** for LCD and sound. Fire has the highest priority; all channels are not energized continuously.

## Simulated vs. physical hardware

The simulator uses a **Wokwi photoresistor module as an adjustable-light proxy** for the flame sensor, because it can expose a controllable DO signal. Changing *lux* is an **input demonstration, not a real flame detection test**. It has **no extra pushbuttons, acknowledgment switch, or software power switch**. Use the built-in DHT22, MQ-2, MPU-6050, and light-proxy properties to test readings.

Wokwi models **piezo** buzzers that require `tone()`. Your real **two-terminal active buzzers** use DC on/off through appropriately rated MOSFET/transistor driver circuits. **Do not copy the direct-connected Wokwi buzzer wiring to the real build.** Wokwi has **no real TP4056/18650 charging circuit** in this project.

- `arduino/GAYAK.ino` — **physical-hardware** firmware (active-buzzer DC control).
- `wokwi/sketch.ino` — **simulator-only** firmware (piezo `tone()`, faster simulation timings).
- **Manual Wokwi wiring:** use [`wokwi/README.md`](wokwi/README.md) to place and connect the components yourself, with 220Ω per LED. A premade `diagram.json` is not required.
- `wokwi/libraries.txt` — simulator dependencies.

Changing GitHub files **does not automatically update the hosted Wokwi URL**. Manually wire your own Wokwi project; copy only the simulator program `wokwi/sketch.ino` and dependencies `wokwi/libraries.txt`. Wokwi saves your custom circuit automatically.

## Arduino software

Arduino IDE libraries: **DHT sensor library (Adafruit)**, **Adafruit Unified Sensor**, and a compatible **LiquidCrystal_I2C** library. `Wire` is included with the Arduino AVR core; no extra MPU library is required by the supplied firmware. Open `arduino/GAYAK/GAYAK.ino` in Arduino IDE and upload to an Uno. Keep it synchronized with the repository firmware at `arduino/GAYAK.ino`.

### PAGASA heat index

The firmware recalculates HI from each valid DHT22 temperature and relative-humidity reading. It uses the simple low-range estimate and Rothfusz regression with humidity adjustments described in PAGASA heat-index guidance. HI categories follow PAGASA thresholds: Caution (27–32°C), Extreme Caution (33–41°C), Danger (42–51°C), and Extreme Danger (52°C and above). The HI is an estimated apparent temperature, not a medical measurement. See [PAGASA heat-index guidance](https://pubfiles.pagasa.dost.gov.ph/climps/climateforum/speciallecture.pdf).

## Local desktop dashboard (Windows)

1. Install Python 3 and run `py -m pip install -r requirements.txt` from the repository root.
2. Connect the Arduino via USB; upload the physical `.ino` file.
3. **Close the Arduino IDE Serial Monitor** (only one application can own the COM port).
4. Run `start_gayak.bat`, or `py app.py`.
5. Open **http://127.0.0.1:5000**. If auto-detection uses the wrong COM port, run `set GAYAK_COM_PORT=COM4` in that CMD window before launching, replacing COM4 with your own port.

USB Serial runs at **115200 baud**. Existing `DATA` CSV telemetry remains compatible with the desktop app; additional `QUAKE,online,alert,rms_g,level` records carry MPU status. The parser also supports the earlier four-field format. The dashboard displays vibration, sensor health, and shaking events without changing the existing temperature/fire/smoke record fields.

## Portable battery system — safety warning

A standard TP4056 USB-C charger is a **one-cell / 1S Li-ion charger**, not a 5V regulator or automatic load-sharing module. **Never connect a two-cell 7.4V series holder to a TP4056**. A compatible protected 1S battery or properly engineered, matched parallel 1S pack needs the **correct protection, fuse, SPST switch, suitably sized regulated 5V boost converter, and USB/battery power-source isolation**. Do not charge while running unless a suitable power-path design is implemented. **For initial classroom tests, use USB power and leave the battery disconnected.**

See [Physical wiring and power](docs/WIRING_AND_POWER.md) for an explicit connection guide and limitations.

## Project status

The files are statically checked for key pin and component references. **A live Wokwi run, a physical circuit validation, battery safety testing, and alarm-volume testing are still required.** Instructors should use the [test plan](docs/TEST_PLAN.md) to record actual results.
