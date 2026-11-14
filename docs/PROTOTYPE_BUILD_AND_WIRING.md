---
title: GAYAK Prototype Build And Wiring
tags:
  - gayak
  - hardware
  - wiring
  - prototype
aliases:
  - Physical Prototype Build Guide
  - GAYAK Assembly Guide
---

# GAYAK Physical Prototype: Step-By-Step Build And Wiring

This guide walks through assembling the physical GAYAK Arduino prototype, connecting its sensors and indicators, checking the circuit, and uploading the physical firmware. It follows the current pin assignments in `arduino/GAYAK/GAYAK.ino`.

> [!warning] Prototype safety
> GAYAK is an educational prototype, not a certified fire, smoke, gas, heat, or earthquake alarm. Start with USB power and keep the battery and TP4056 disconnected while assembling and checking the circuit. Never test by starting a fire or releasing gas or smoke.

> [!info] Read before wiring
> This guide uses a proper transistor/MOSFET driver for each real active buzzer. Do not connect physical two-wire buzzers directly to Arduino pins. See [[WIRING_AND_POWER]] for the detailed driver and optional battery power diagrams.

## 1. Gather The Parts And Tools

### Main components

| Part | Quantity | Purpose |
| --- | ---: | --- |
| Arduino Uno R3 or compatible 5V ATmega328P board | 1 | Reads sensors and controls indicators |
| DHT22 temperature/humidity sensor | 1 | Temperature, humidity, and calculated HI |
| MQ-2 gas sensor module with AO pin | 1 | Relative gas/smoke response on analog input |
| Digital flame sensor module with DO pin | 1 | Digital flame/IR indication |
| GY-521 module with MPU-6050 | 1 | Local shaking/vibration measurements |
| 16×2 LCD with I2C backpack | 1 | Local status and readings |
| Green, yellow, and red LEDs | 1 each | Status and warning indications |
| 220Ω LED resistors | 3 | One current-limiting resistor per LED |
| Momentary normally-open pushbutton | 1 | D11 alert acknowledgment / LCD panel control |
| Two-wire active buzzers | 3 | Smoke, shaking, and fire alarms |
| Suitable logic-level N-channel MOSFETs | 3 | One low-side driver for each active buzzer |
| Gate resistor and gate pull-down per MOSFET | 3 each | Gate protection and defined OFF state |
| Breadboard or isolated-pad perfboard, jumper wires | As needed | Temporary assembly or final mounting |
| USB data cable | 1 | Initial power, firmware upload, and dashboard serial link |

For the buzzer drivers, use parts rated for the buzzer's actual voltage and current. Typical starting values in [[WIRING_AND_POWER]] are 100Ω–1kΩ in series with each MOSFET gate and a 10kΩ–100kΩ gate-to-ground pull-down. Check the exact MOSFET pinout and buzzer datasheet. An inductive buzzer may also need a flyback diode.

### Tools

- Small wire cutters/strippers and insulated jumper wires
- Multimeter with continuity and DC-voltage modes
- Soldering tools if building on perfboard
- Arduino IDE for the one-time firmware upload or later firmware changes

## 2. Identify The Board Pins Before Connecting Anything

Unplug USB and disconnect the battery. Read the labels on each sensor board rather than assuming the pin order; module layouts differ between manufacturers. Identify each module's VCC, GND, signal, and output pins. Confirm its allowed supply voltage from its markings or documentation.

The current physical firmware uses this pin map:

| Arduino pin | Connect to | Notes |
| --- | --- | --- |
| D2 | DHT22 DATA | Bare sensor may need a 4.7kΩ–10kΩ DATA-to-VCC pull-up; many modules include one |
| D3 | Flame sensor DO | Digital output only; confirm board polarity |
| A0 | MQ-2 AO | Do not exceed the Uno analog input voltage range |
| A4 | LCD SDA and GY-521 SDA | Shared I2C data line |
| A5 | LCD SCL and GY-521 SCL | Shared I2C clock line |
| D5 | Green LED through its own 220Ω resistor | LED cathode returns to GND |
| D6 | Yellow LED through its own 220Ω resistor | LED cathode returns to GND |
| D7 | Red LED through its own 220Ω resistor | LED cathode returns to GND |
| D8 | Smoke buzzer MOSFET gate/driver input | Active HIGH in the sketch |
| D9 | Shaking buzzer MOSFET gate/driver input | Active HIGH in the sketch |
| D10 | Fire buzzer MOSFET gate/driver input | Active HIGH in the sketch |
| D11 | Momentary button to GND | Uses `INPUT_PULLUP`; no external pull-up is needed |
| D0 / D1 | USB serial | Leave free for upload and dashboard communication |
| 5V / GND | Regulated supply and common ground | Begin on USB power only |

The firmware uses a 16×2 LCD I2C address commonly set to `0x27` and an MPU-6050 address of `0x68`. Check the actual boards if either device is not detected.

## 3. Make A Ground And Power Plan

1. Use the Uno's USB connection as the only supply for the first sensor checks.
2. Connect every sensor ground to the Arduino ground. The LCD, GY-521, DHT22, MQ-2, and flame module all need a shared reference ground.
3. Do not power the MQ-2 heater from an Arduino GPIO. Use the sensor module's specified supply rail and ensure its AO signal stays within the Uno's analog input range.
4. Check I2C logic compatibility before joining the LCD and GY-521. Some LCD backpacks pull SDA/SCL up to 5V while the MPU-6050 uses 3.3V logic. Use a suitable bidirectional I2C level shifter unless the exact modules are verified compatible.
5. Leave all 18650 cells, TP4056 charger boards, and boost converters disconnected during the initial build. Optional battery power is a separate design; follow [[WIRING_AND_POWER]].

## 4. Wire The DHT22, MQ-2, And Flame Module

Make these connections with power removed:

### DHT22

- Sensor VCC → a compatible supply pin for that DHT22 board
- Sensor GND → common GND
- Sensor DATA → Arduino D2
- If using a bare DHT22 with no breakout pull-up, add 4.7kΩ–10kΩ between DATA and the sensor's compatible VCC.

### MQ-2

- Module VCC → the supply specified for that module (MQ-2 heater boards commonly need a 5V rail with adequate current capacity)
- Module GND → common GND
- Module AO → Arduino A0
- Leave DO unconnected for this firmware's analog smoke/gas measurement.

The MQ-2 requires a warm-up and clean-air baseline after power-up. Current firmware waits about 2 minutes, then calibrates for 20 seconds. Readings are relative response, not certified concentration or proof of smoke.

### Digital flame module

- Module VCC → compatible supply
- Module GND → common GND
- Module DO → Arduino D3

The current module has digital output only; it cannot report a numeric flame distance. At startup, aim it at a normal indoor scene so the firmware can learn its idle level. Keep it away from windows, sky, and direct sunlight, and adjust its sensitivity potentiometer in its final mounted position. Its response still depends on the module's IR threshold and field of view.

## 5. Wire The Shared I2C LCD And MPU-6050

Both devices share the Uno I2C bus:

```text
Arduino Uno A4 (SDA) ──┬── LCD backpack SDA
                       └── GY-521 SDA

Arduino Uno A5 (SCL) ──┬── LCD backpack SCL
                       └── GY-521 SCL

Arduino GND ────────────┬── LCD GND
                        └── GY-521 GND
```

Connect each module's VCC only after checking its board's voltage requirements. Resolve the LCD-to-MPU I2C voltage issue described in Step 3 before powering the bus. Mount the GY-521 firmly to the structure being monitored; a loose module or dangling wires can create misleading motion readings.

## 6. Add The Three Status LEDs

Wire each LED as its own series circuit. Do not share one resistor between LEDs:

```text
Arduino D5 ── 220Ω ── Green LED anode (+)
Green LED cathode (−) ── GND

Arduino D6 ── 220Ω ── Yellow LED anode (+)
Yellow LED cathode (−) ── GND

Arduino D7 ── 220Ω ── Red LED anode (+)
Red LED cathode (−) ── GND
```

The long LED leg is commonly the anode, but verify the LED's polarity. The sketch briefly lights all LEDs during its splash screen, then uses red/yellow status during MQ-2 warm-up/calibration before normal status indications.

## 7. Connect The D11 Pushbutton

Use a **momentary normally-open** pushbutton:

- One button terminal → Arduino D11
- The other button terminal → GND

The firmware uses `INPUT_PULLUP`. The input is normally HIGH and becomes LOW when pressed, so no external pull-up resistor is required. A latching on/off switch is not a substitute for a momentary button. Pressing D11 acknowledges active alerts; with no alert awaiting acknowledgment, it advances the LCD panel.

## 8. Build The Three Buzzer Driver Channels

The firmware assigns D8 to smoke, D9 to shaking, and D10 to fire. Each pin should control one separate MOSFET driver; the Arduino pin is a control signal, not the buzzer's power source.

Repeat this low-side driver circuit for each channel:

```text
Rated buzzer supply (+) ─────────────── Buzzer (+)
                                         Buzzer (−) ── MOSFET DRAIN
Arduino D8, D9, or D10 ──[100Ω–1kΩ]── MOSFET GATE
                                          │
                                      10kΩ–100kΩ
                                          │
Common GND ───────────────────────── MOSFET SOURCE
       └───────────────────────────── supply (−)
```

- Smoke buzzer driver input: D8
- Shaking buzzer driver input: D9
- Fire buzzer driver input: D10
- Join the buzzer supply negative, MOSFET source, and Arduino GND.
- Use the buzzer's specified operating voltage and a supply rated for its measured current. The SFM-27 label range alone does not establish that a particular unit will be loud enough or draw a safe current at 5V or 12V.
- If the buzzer is inductive, install a suitable flyback diode across it as directed by its manufacturer (usually stripe/cathode toward positive).
- Never connect a 12V buzzer supply to the Arduino 5V pin. Never drive a physical active buzzer directly from a GPIO.

The simulated Wokwi piezo wiring is different from this physical driver circuit. See [[WIRING_AND_POWER]] before choosing driver parts or a separate buzzer rail.

## 9. Arrange The Prototype On Breadboard Or Perfboard

1. Place the Uno so USB remains accessible and the board cannot touch conductive surfaces.
2. Place modules so their labels and pin order remain visible during bring-up.
3. Keep power distribution and buzzer-current paths short and secure. Keep signal wiring neat and away from loose battery leads.
4. Use a breadboard for the first signal checks. Move to isolated-pad perfboard only after the wiring works; perfboard pads are not automatically connected into rails.
5. If soldering, label the 5V, GND, I2C, LED, sensor, button, and buzzer-driver connections. Add strain relief and insulation over exposed joints.
6. Do not mount the flame sensor where it points through a window. Mount the MPU-6050 firmly to the test structure rather than on a loose wire.

## 10. Inspect Before Applying Power

With USB and battery disconnected, check each item:

- Every module's VCC/GND polarity matches its board labels.
- DHT22 DATA, MQ-2 AO, flame DO, and I2C wires reach the pins in the tables above.
- Each LED has its own 220Ω series resistor and correct polarity.
- D11 connects only through the momentary switch to GND.
- Each physical buzzer has its own correctly oriented, current-rated driver.
- I2C voltage compatibility has been addressed.
- D0/D1 are clear for USB serial.
- A multimeter shows no short between the 5V rail and GND.
- Battery/TP4056 wiring is still disconnected for the first power-up.

## 11. Upload The Physical Firmware

1. Install Arduino IDE on the developer's computer.
2. Install **DHT sensor library** by Adafruit, **Adafruit Unified Sensor**, and a compatible **LiquidCrystal_I2C** library through Library Manager. `Wire` is included with the Uno core.
3. Open `arduino/GAYAK/GAYAK.ino`.
4. Select **Arduino Uno** and the correct COM port. Keep D0/D1 free; close other programs using the serial port.
5. Power the Uno by USB, then click **Upload**.
6. Open Serial Monitor at **115200 baud** and look for sensor diagnostics. Close Serial Monitor before using the web dashboard.

The dashboard-only client does not need Arduino IDE after the firmware is uploaded; see [[../INSTALLATION|Windows Dashboard Setup]].

## 12. Power Up And Check One Subsystem At A Time

1. Power the Uno over USB with the buzzer driver supply and battery still disconnected.
2. Confirm the LCD starts and the LEDs perform their startup sequence.
3. Let the flame module learn its normal indoor idle state at startup.
4. Keep the MQ-2 in clean air and wait through its warm-up and baseline calibration. Do not judge smoke response before calibration finishes.
5. Confirm DHT22, MQ-2, flame DO, LCD, and MPU-6050 readings/diagnostics over Serial Monitor.
6. Press D11 once. It should acknowledge an active alert, or advance the LCD panel when no alert is active.
7. Only after signals work, connect one buzzer-driver supply at a time and verify wiring/current against the component datasheets.
8. For a safe functional demonstration, use Wokwi or benign nonhazardous inputs. Do not create smoke, release fuel gas, or ignite a flame inside the classroom.

For dashboard setup after firmware is uploaded, connect the Arduino by USB, close Serial Monitor, and run `start_gayak.bat` as described in [[../INSTALLATION|Windows Dashboard Setup]].

## Quick Troubleshooting

| Symptom | First checks |
| --- | --- |
| LCD is blank or garbled | Check I2C power/ground, SDA=A4, SCL=A5, address, and logic-level compatibility |
| DHT22 shows no reading | Check D2, VCC/GND, DATA pin order, and bare-sensor pull-up if needed |
| MQ-2 reading stays at zero or alerts immediately | Check AO→A0, module supply, common GND, output voltage, and allow the full warm-up/baseline |
| Flame shows active at startup | Reboot aimed at a normal indoor scene, shield it from outdoor IR, and adjust the module threshold; a DO-only module cannot measure distance |
| MPU-6050 is offline | Check shared A4/A5 wiring, address 0x68, supply, and I2C voltage compatibility |
| Pushbutton does not respond | Use a momentary normally-open button between D11 and GND; test continuity only while unpowered |
| Buzzer is silent or Arduino resets | Check MOSFET pinout, common ground, buzzer supply/current, polarity, and driver ratings; do not power the buzzer from a GPIO |
| Dashboard cannot connect | Close Serial Monitor/Plotter, check the USB data cable and COM port, and keep D0/D1 free |

## Related Notes

- [[HARDWARE-AND-PINS]] — concise pin reference
- [[WIRING_AND_POWER]] — detailed buzzer, I2C, perfboard, and optional battery safety notes
- [[TUTORIALS]] — firmware, Wokwi, and dashboard workflows
- [[TROUBLESHOOTING]] — additional sensor and dashboard troubleshooting
