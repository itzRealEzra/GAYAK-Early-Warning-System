# GAYAK: A SMART HEAT INDEX MONITORING AND EARLY WARNING SYSTEM WITH DUAL EARTHQUAKE AND FIRE ALARM DETECTION SYSTEM FOR CLASSROOMS

## Build and wire the Wokwi circuit yourself

**No premade wiring diagram is needed.** This repository provides `wokwi/sketch.ino` and `wokwi/libraries.txt`. You can create, move, and connect parts directly in Wokwi; it maintains the diagram in your own project.

Open [the existing Wokwi project](https://wokwi.com/projects/478926664159746049), **Save a Copy**, remove obsolete controls and duplicated wires as needed, and keep/add one pushbutton for LCD paging. You may also start from a blank Arduino Uno project.

### Add these Wokwi parts

Arduino Uno R3; one DHT22; one MQ-2/Gas Sensor; one Photoresistor Sensor module (**fire-input proxy, not a real flame sensor**); one 16x2 LCD set to **I2C** mode (address `0x27`); one MPU6050; one pushbutton; green/yellow/red LEDs (one each); **three 220Ω resistors (LEDs only)**; three Wokwi piezo buzzers; optional breadboard for 5V/GND wiring.

### Manual wiring reference

| Part | Connection to Arduino Uno | Other terminal(s) |
| --- | --- | --- |
| DHT22 | Data/SDA → **D2** | VCC → 5V; GND → GND |
| MQ-2 gas sensor | AOUT → **A0** | VCC → 5V; GND → GND |
| Photoresistor module | DO → **D3** | VCC → 5V; GND → GND |
| LCD next-panel pushbutton | One switch side → **D11**; opposite side → **GND** | `INPUT_PULLUP` is enabled; no external resistor |
| Green LED | **D5 → 220Ω → anode A** | Cathode C → GND |
| Yellow LED | **D6 → 220Ω → anode A** | Cathode C → GND |
| Red LED | **D7 → 220Ω → anode A** | Cathode C → GND |
| Smoke piezo buzzer | **D8 → buzzer pin 2 (+)** | Buzzer pin 1 (−) → GND |
| Shaking piezo buzzer | **D9 → buzzer pin 2 (+)** | Buzzer pin 1 (−) → GND |
| Fire piezo buzzer | **D10 → buzzer pin 2 (+)** | Buzzer pin 1 (−) → GND |
| I2C LCD | SDA → **A4**, SCL → **A5** | VCC → 5V; GND → GND |
| MPU6050 | SDA → **A4**, SCL → **A5** | VCC/GND in simulation; AD0 → GND (`0x68`) |

**Important:** Put a separate **220Ω** current-limiting resistor in series with **each LED only**. The Wokwi piezo buzzers use **no series resistor**; this direct-pin scheme is **simulation-only** and must not be copied to real active buzzers. All GND terminals must share a common ground. The LCD and MPU6050 share I2C A4/A5; their distinct addresses are 0x27 and 0x68.

### Setup and functional tests

1. Arrange components in logical groups. Connect one wire at a time according to the table, then inspect each wire and common GND. An optional Wokwi breadboard can organize power connections.
2. Replace your Wokwi `sketch.ino` with [the simulation program](sketch.ino), and `libraries.txt` with [the library list](libraries.txt). **You do not need to paste a `diagram.json`.**
3. Start Simulation. The MQ-2 simulator uses approximately **5 seconds warm-up plus 3 seconds baseline calibration**, versus longer real-hardware intervals.
4. Change DHT22 temperature and humidity; readings update about every **2.2 seconds**. Press and release the D11 button to advance the LCD panel; alerts temporarily override the selected panel.
5. After baseline calibration, increase MQ-2 simulated gas concentration. Smoke alone uses **D6 yellow LED + one short D8 pulse each second**.
6. Increase the photoresistor's **lux** until DO switches LOW (roughly >100 lux with the default simulated threshold). This tests a **digital fire-input signal** and activates **D7 red LED + three short D10 pulses followed by a pause**. With both flame and MQ-2 alerts active, the combined sound is **two fast D10 pulses followed by one longer D8 pulse**. Decrease lux to clear. This is **not an actual fire sensor test**.
7. Repeatedly vary the MPU6050 X/Y accelerations to simulate strong/intense shaking. The filtered motion classification can change before the confirmed alarm; shaking activates **D7 blinking red + alternating D9/D8 pulses** when sustained.
8. After all conditions clear, verify **D5 green LED ON** and buzzers OFF. Open Serial Monitor at **115200 baud** to see `DATA` and `QUAKE` telemetry.

### Wokwi versus physical hardware

The Wokwi piezo parts require `tone()`; the physical system uses **two two-terminal 5V active buzzers and one SFM-27 3–24V buzzer**. Those require correctly rated transistor/MOSFET drivers for reliable operation. A 220Ω resistor specified for an LED is not the correct buzzer driver. The Wokwi power supply does **not** simulate the 18650 pack, TP4056, boost converter, battery protection or safe charge/load switching.

The physical MPU6050 IC uses 3.3V I2C logic; a real 5V LCD backpack can require level shifting. See [physical wiring and battery safety](../docs/WIRING_AND_POWER.md).

The simulation is an educational logic demonstration, not a certified earthquake, gas, or fire alarm. This manual-wiring tutorial and pin list require a real Wokwi run after you finish wiring to validate your specific diagram.
