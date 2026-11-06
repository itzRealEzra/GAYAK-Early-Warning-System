# GAYAK Wokwi Wiring — Revised Layout

Source Wokwi project: https://wokwi.com/projects/478926664159746049

Copy `diagram.json`, `sketch.ino` and `libraries.txt` from this folder into a **duplicate** of the Wokwi project. GitHub does not update the hosted Wokwi simulation directly. Diagram connectivity was checked programmatically, but operation in Wokwi remains untested.

## Simulated Arduino Uno pins

| Pin | Component |
| --- | --- |
| D2 | DHT22 DATA |
| D3 | Flame-test button, F |
| D4 | Software power switch (not a true disconnect) |
| D5 | RED LED via 1kΩ |
| D6 | YELLOW LED via 1kΩ |
| D7 | GREEN LED via 1kΩ |
| D8 | Smoke piezo buzzer via 1kΩ |
| D9 | Shaking piezo buzzer via 1kΩ |
| D10 | Fire piezo buzzer via 1kΩ |
| D12 | ACK/MUTE 5s button, A |
| A0 | MQ-2 gas sensor analog output |
| A1 | Simulated flame analog slider |
| A4 | SDA — LCD + MPU-6050 |
| A5 | SCL — LCD + MPU-6050 |

## Functional tests

Press **F** for a fire alert (D10 → D8 → D9); increase MQ-2 ppm to trigger smoke (D8), vary MPU accelerations for sustained shaking (D9), and press **A** to mute for five seconds. This circuit represents the PCB perfboard using Wokwi's breadboard, which has connected rails unlike ordinary isolated-hole perfboard.

**Physical circuit caution:** Wokwi's piezo buzzers use `tone()`; your two-terminal real active buzzers need DC on/off via properly rated driver stages. A series 1kΩ resistor cannot guarantee adequate volume. The TP4056, two 18650 cells, switch, fuse and regulated 5V boost converter are NOT simulated. A TP4056 only supports a suitable protected 1S pack and cannot be connected directly to the Arduino 5V pin. Check MPU-6050 3.3V logic voltage and LCD I2C pullups. This is an educational prototype, not certified hazard/seismic monitoring.
