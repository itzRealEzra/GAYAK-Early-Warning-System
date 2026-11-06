# GAYAK: A SMART HEAT INDEX MONITORING AND EARLY WARNING SYSTEM WITH DUAL EARTHQUAKE AND FIRE ALARM DETECTION SYSTEM FOR CLASSROOMS

The GY-521 MPU-6050 is used for **intense shaking and acceleration** measurement. Mount it securely to the observed structure so loose wiring or handling does not dominate the acceleration signal. Its filtered acceleration levels do not identify earthquake magnitude or origin.

**Read this before assembly.** Use a USB-powered Arduino Uno for classroom development. This document describes electrical intent and necessary additional protections; it is **not a certified battery-pack design or a complete manufacturing schematic**.

## 1. Signals and components

| Arduino Uno pin | Module connection | Notes |
| --- | --- | --- |
| D2 | DHT22 DATA | Bare DHT22 typically requires a 4.7k–10k pull-up to its compatible VCC; some modules include it |
| D3 | Flame sensor DO | Real module only, typically active-LOW; confirm polarity |
| A0 | MQ-2 AO | Analog sensor VCC/GND must match board specification |
| A4 | LCD SDA and GY-521 SDA | Shared I2C bus |
| A5 | LCD SCL and GY-521 SCL | Shared I2C bus |
| D5 | GREEN LED + 1kΩ series | LED cathode → GND |
| D6 | YELLOW LED + 1kΩ series | LED cathode → GND |
| D7 | RED LED + 1kΩ series | LED cathode → GND |
| D8 | Smoke buzzer driver input | 5V two-pin active buzzer #1 |
| D9 | Shaking buzzer driver input | 5V two-pin active buzzer #2 |
| D10 | Fire buzzer driver input | SFM-27 3–24V active buzzer |
| D0, D1 | USB Serial | Do not use these pins for alarms while the dashboard runs |

### Common I2C bus

```text
Arduino Uno A4 (SDA) ── LCD SDA
                    └── MPU-6050 SDA
Arduino Uno A5 (SCL) ── LCD SCL
                    └── MPU-6050 SCL
GND ─────────────────── LCD GND + MPU GND
```

I2C devices are distinguished by their addresses: commonly **0x27 for LCD** and **0x68 for MPU-6050**. However, the **MPU-6050 IC uses 3.3V logic**. A GY-521 board may have a VCC regulator but still expose 3.3V-level SDA/SCL. Many LCD backpacks pull SDA and SCL up to **5V**. Use a **suitable bidirectional 3.3V↔5V I2C level shifter** or an independently verified, compatible bus arrangement. Do not assume 5V SDA/SCL are automatically safe for the MPU.

## 2. Real buzzer drivers (required for reliable sound output)

The 2×5V active buzzers and the SFM-27 are **two-terminal DC buzzers**. They are NOT the Wokwi piezo components.

Use a **suitable logic-level N-channel MOSFET (one per buzzer)** rated for the measured buzzer operating current and voltage. Example low-side connection (repeat for D8/D9/D10):

```text
REGULATED BUZZER SUPPLY +5V ─── buzzer (+)
                                   buzzer (-)
                                       |
                                  MOSFET DRAIN
Arduino D8/D9/D10 ──[100Ω–1kΩ]── MOSFET GATE
                                       |
                                  10kΩ–100kΩ pull-down
                                       |
COMMON GND ──────────────────── MOSFET SOURCE + pull-down other end
```

- Choose a **logic-level device** whose on-resistance and current ratings are suitable at **4.5–5V gate drive**; check the actual MOSFET's pinout and data sheet.
- The SFM-27 can use a **regulated 5V supply only if its exact variant supports 5V at the measured operating current**. A stated 3–24V range does not guarantee loudness at the bottom of that range.
- If the buzzer is electromagnetic/inductive, fit a flyback diode across it (cathode/stripe to positive rail, anode to MOSFET drain) per its manufacturer's guidance. Pure piezos may not need it.
- A resistor placed in series with a buzzer may limit current but also **starve it of voltage and sound**. It is not a substitute for a properly rated driver.
- Measure supply current with all relevant modules operating and **allow margin**; three buzzer branches should not depend on Uno pin current limits.
- **Do not directly connect the Wokwi no-resistor buzzer arrangement to real Arduino I/O pins.** Wokwi simulates a piezo load, not your physical active buzzer current.

If you only have 1kΩ resistors and no transistor/MOSFET, keep buzzers in **bench-test-only mode with measured currents** or leave them disconnected until suitable drivers are obtained. Do not use that configuration as a deployable audible warning.

## 3. PCB perfboard layout

A typical perfboard has **isolated solder pads**; no rails are connected unless you add soldered wire/traces.

1. Place modules/terminal blocks so exposed connections cannot touch each other.
2. Create separate **regulated +5V** and **GND** distribution conductors of sufficient size.
3. Keep the 5V supply path separated from signal lines and provide strain relief for all battery and USB leads.
4. Mount three buzzer MOSFETs/drivers with their gate resistors and gate pull-down resistors; join all MOSFET sources to the shared GND.
5. Connect D8→smoke driver, D9→shaking driver and D10→fire driver. Connect each buzzer (+) to its specified regulated voltage, not an Arduino GPIO.
6. Route the DHT22, flame, MQ-2, LCD, and MPU signal wires as in the pinout above.
7. Label **5V / GND / D5 green / D6 yellow / D7 red / D8 smoke / D9 shaking / D10 fire**.
8. Before power-up, use a multimeter to confirm **no continuity from 5V to GND**, no reversed buzzer polarity, and each expected terminal is connected to its intended pad.

## 4. 18650 + TP4056 + SPST switch (optional portable build)

### Critical compatibility

A standard USB-C **TP4056** charging board charges **one Li-ion cell / a properly engineered 1S pack (nominal 3.6–3.7V, up to ~4.2V)**. It **cannot** charge a two-cell series pack (nominal 7.2–7.4V). Many inexpensive 2×18650 holders are wired **in series**: count the terminals and **measure the pack voltage** before considering the charger. If you are unsure how the holder is wired, do not connect it.

For a classroom project, the simplest safer option is a **single high-quality protected 18650 cell and suitable holder**, or a manufacturer-built **protected 1S pack**. Do not solder or casually parallel unmatched lithium cells. The battery/charger protection circuitry must be matched to chemistry, capacity, expected current and pack design. The presence of a board marked "TP4056" does not guarantee over-discharge, over-current or temperature protection.

### Conceptual battery power flow (NOT a charge-while-running circuit)

```text
Compatible PROTECTED 1S Li-ion PACK
       |             |
      B+            B-
       |             |
  TP4056 1S CHARGER / PROTECTION MODULE
       | protected load OUT+ / OUT- (if provided)
       +----- Fuse ----- SPST ON/OFF ----- 5V BOOST converter IN+
       |                                  5V BOOST converter IN-
       +-------------------------------------------|
                                            |
                              regulated +5V OUT / GND
                                            |
                                    Arduino 5V / GND
                                    sensors + buzzer drivers
```

This is a **conceptual** load path only, not complete, build-ready charging circuitry. In a real build, follow your exact protection module's **B+ / B− / OUT+ / OUT−** documentation. Size fuse, wiring, boost converter and charging current from actual component ratings and measured loads, and protect against short-circuits, reverse polarity and brownouts.

**Do not connect 3.0–4.2V raw battery output straight to Arduino 5V.** Use a stable regulated **5.0V boost converter** with adequate load rating. Feeding a regulated 5V rail into the Uno 5V pin bypasses onboard input regulation, so verify the voltage and **prevent USB 5V backfeed / dual-source contention**. Do not simultaneously connect an external 5V rail and a PC USB cable without proper source isolation/selection. Never connect a 7.4V pack to the 5V pin.

**TP4056 is not a power-path/load-sharing controller.** The load should be disconnected during charging unless a specifically designed and validated charger + power-path module is used. For student experiments, use **USB powered bench testing with the battery pack physically disconnected**.

## 5. Pre-power inspection

- All leads labeled and correct polarity; no exposed loose battery terminals.
- Battery disconnected when wiring, soldering, or uploading via USB unless validated source switching is present.
- Correct LED resistor per LED (1kΩ is acceptable; may reduce brightness).
- Buzzer drivers present and correctly oriented; current ratings verified.
- GY-521 I2C bus voltage compatibility checked.
- Common grounds correct, no shorts between positive and GND.
- Start with Arduino and sensors before testing any buzzer driver.
- MQ-2 gets its required warm-up before evaluating gas response.

**Never treat a successful Wokwi animation as evidence that an unprotected battery or a buzzer circuit is electrically safe.**