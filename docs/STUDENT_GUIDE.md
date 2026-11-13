# GAYAK: A SMART HEAT INDEX MONITORING AND EARLY WARNING SYSTEM WITH DUAL EARTHQUAKE AND FIRE ALARM DETECTION SYSTEM FOR CLASSROOMS

## Purpose
GAYAK is an educational Arduino Uno R3 trainer demonstrating multi-sensor acquisition, sensor calibration, I2C, LCD output, digital alarms, USB serial telemetry, and safe prototype assembly. Its MPU-6050 detects **intense shaking and acceleration** and reports STABLE, MOTION DETECTED, STRONG SHAKING, or INTENSE SHAKING based on filtered acceleration; it cannot measure earthquake magnitude or verify what caused shaking.

**Learning outcomes:** Students should be able to (1) identify each module and power level; (2) interpret a wiring diagram and common ground; (3) share the A4/A5 I2C bus; (4) set and evaluate relative detection thresholds; (5) distinguish a simulated event from measured hardware data; (6) record repeated trial results; and (7) explain battery/charging limitations.

## Student safety briefing
- **This is not an emergency device.** Never rely on it to detect real fires, gas leaks, earthquakes, or to issue evacuation orders.
- For first lessons, use **USB-powered Arduino only**. Keep the 18650 battery pack and charger disconnected.
- Never touch bare battery wiring or bridge battery contacts, and do not connect a 7.4V 2-cell series pack to a TP4056. Battery assembly is instructor-supervised.
- No open flame, spraying lighter gas, intentional smoke exposure, or destructive vibration testing is required. Use Wokwi controls instead.
- Wokwi's direct-connected piezo buzzers are **not** a wiring model for the real two-terminal active buzzers; use driver transistors/MOSFETs.
- Power down before moving wires, observe polarity, check for shorts, and use a properly rated regulated supply.

## Know the components

| Component | Measured/controlled variable | Key limitation |
| --- | --- | --- |
| Arduino Uno R3 | Coordinates input/output, serial and timers | Limited I/O current; D0/D1 reserved for USB Serial |
| DHT22 | Temperature and relative humidity | Slow updates; must allow ~2 s per reading |
| MQ-2 | Relative gas/smoke analog response | Needs warm-up and baseline; not a calibrated ppm or certified smoke detector |
| Flame sensor module (physical) | Digital threshold output (DO) | Depends on sensitivity/polarity, line of sight, and interference |
| Photoresistor (Wokwi only) | Simulated bright-light DO as flame proxy | Not an IR flame detector |
| MPU-6050 / GY-521 | Local acceleration and intense shaking | Filtered motion levels do not reveal Richter magnitude, official seismic intensity, or earthquake origin |
| LCD 16×2 (I2C) | Status and button-selected readings | Address can vary by backpack |
| Pushbutton | Acknowledges active alerts; otherwise advances LCD panel on D11 | Connect between D11 and GND; internal pull-up is enabled |
| Status LEDs | Green/Yellow/Red visual indication | **One 220Ω current-limiting resistor on each LED** |
| Three buzzers | Independent audible patterns | Real active buzzers need properly rated drivers |
| PCB perfboard | Physical interconnect | Isolated-hole perfboard does not have automatic power rails |
| Battery/TP4056/boost | Optional portable supply | 1S protection, safe charge/load design, 5V regulation required |

## Lab workflow

### Activity 1 — Identify and trace wires (15–20 min)
Read [wiring guide](WIRING_AND_POWER.md). Label each pin in the [student pin map](#check-your-pin-map). Explain which sensors send analog data (MQ-2), digital data (flame, DHT22), and I2C data (LCD, MPU-6050).

### Activity 2 — Run the simulator (20–30 min)
Use the [manual Wokwi wiring tutorial](TUTORIALS.md#1-wokwi-simulation) to connect the circuit yourself (no premade diagram file). Each LED uses its own **220Ω** series resistor. Modify one input at a time. Record the LCD, LED, buzzer, and serial output. Observe that the sensor value may change before the confirmed alarm does, because the program filters or confirms inputs.

### Activity 3 — Understand alert priority (15–20 min)
Trigger smoke, then trigger simulated fire without clearing smoke. Expected: fire's red LED/LCD and combined fire + smoke buzzer pattern take priority. Restore both inputs and note the delay before reset. Next, trigger shaking and identify its alternating buzzer pattern.

### Activity 4 — Calibrate carefully (20–30 min)
Read [adjustments](ADJUSTMENTS.md). Record the normal MQ-2 analog baseline, then adjust one threshold **in simulation**. Explain false positives and false negatives. Do not claim an acceleration threshold equals Richter magnitude.

### Activity 5 — Connect local dashboard (optional; 20 min)
Follow [Windows dashboard setup](TUTORIALS.md#3-windows-local-dashboard). Identify the `DATA` and `QUAKE` records. Make sure Arduino IDE Serial Monitor is closed while the Flask dashboard uses the COM port.

### Activity 6 — Evaluate performance (20 min)
Perform three controlled trials of each simulated condition using [TEST_PLAN.md](TEST_PLAN.md). Record observed values and pass/fail instead of assuming that simulation proves the physical safety device works.

## Check your pin map

```text
Arduino Uno R3
D2  DHT22 DATA            A0  MQ-2 AO
D3  Flame digital DO      A4  LCD SDA + MPU SDA
D5  GREEN LED             A5  LCD SCL + MPU SCL
D6  YELLOW LED            D0/D1 Reserved USB Serial
D7  RED LED
D8  5V ACTIVE BUZZER #1 (SMOKE) via driver
D9  5V ACTIVE BUZZER #2 (SHAKING) via driver
D10 SFM-27 3–24V (FIRE) via driver
D11 LCD panel button → GND
```

## Sample observation sheet

| Trial | Input changed | Sensor reading/level | LED | Audible sequence | LCD | Pass? |
| --- | --- | --- | --- | --- | --- | --- |
| 1 | | | | | | |
| 2 | | | | | | |
| 3 | | | | | | |

**Discussion questions:** Why do LCD and MPU share A4/A5? Why are buzzers driven through transistors in real hardware? What is the difference between a filtered motion RMS reading in g and an earthquake magnitude? Why does the gas module need a normal-air baseline? Why must a TP4056 not charge a 7.4V series pack?

## Assessment suggestion
Evaluate **wiring accuracy (20%)**, **correct sensor interpretation (20%)**, **controlled testing and data recording (25%)**, **safety practices (20%)**, and **explanation/documentation (15%)**. These weights are a suggested classroom rubric, not a validated reliability rating.

## Completion
Submit: Wokwi screenshot, pin map, adjusted parameter table (if any), three-trial records, serial-log excerpts, and a brief discussion of prototype limitations.
