---
tags: [gayak, hardware, wiring]
---

# Hardware, pins, and interfaces

| Uno pin | Current local sketch purpose |
|---|---|
| D2 | DHT22 data |
| D3 | Flame module digital output |
| D5 / D6 / D7 | Green / yellow / red LEDs; each needs a current-limiting resistor |
| A0 | MQ-2 analog output |
| A4 / A5 | I2C SDA / SCL for LCD (commonly address `0x27`) |
| A4 / A5 | Shared I2C SDA / SCL for GY-521 / MPU-6050 (address `0x68`) |
| D8 | Active buzzer for smoke | 
| D9 | Active buzzer for shaking | 
| D10 | Active fire buzzer (SFM-27) |
| D11 | Alert acknowledgment / LCD panel button; connect between D11 and GND, using `INPUT_PULLUP` |
| D0 / D1 | Reserved for USB serial |

Press D11 to acknowledge all currently active alerts, silence their buzzer patterns, and clear their LCD/LED indications. Sensor readings remain live. Alerts return after 30 seconds if a source remains active, and also re-arm after clearing and triggering again. When no new alert needs acknowledgment, the button advances the LCD panel. No external pull-up resistor is needed.

Share ground and confirm module voltage and signal polarity. The sketch's prototype buzzer wiring uses series resistors; properly rated transistor/MOSFET drivers are recommended. Interpret MQ-2 as relative response. Do not treat simulated piezo connections as physical active-buzzer driver circuits. Use USB power for initial tests; battery/charger wiring requires correctly protected, regulated design. See [[GITHUB-VS-WORKSPACE]].
