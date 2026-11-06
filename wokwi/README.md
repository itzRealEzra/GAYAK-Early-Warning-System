# GAYAK: A SMART HEAT INDEX MONITORING AND EARLY WARNING SYSTEM WITH DUAL EARTHQUAKE AND FIRE ALARM DETECTION SYSTEM FOR CLASSROOMS

Original hosted project: https://wokwi.com/projects/478926664159746049

The updated **`diagram.json` + `sketch.ino` + `libraries.txt`** are in this folder. To use them, open the linked Wokwi project, **Save a Copy**, then replace the contents of the three corresponding tabs. GitHub changes do **not** automatically alter that hosted project.

## What changed

- **Green D5; Yellow D6; Red D7**, each through **1kΩ** (LED only).
- Three **piezo buzzer proxies connected directly**: D8 smoke, D9 shaking, D10 fire. **No buzzer series resistors in Wokwi.**
- Removed artificial **FLAME TEST** pushbutton, **ACK/MUTE** button, slide power switch, and analog flame test potentiometer.
- A **photoresistor sensor module on D3** stands in for the physical flame sensor **digital output**. Its adjustable **lux** level changes DO: dark/low lux = no fire, high lux = fire test. This **is not an actual IR flame module**.
- DHT22 D2, MQ-2 A0, MPU-6050 SDA/SCL A4/A5, and I2C LCD at `0x27` remain connected.
- Wokwi breadboard is a **power/ground bus proxy for the future perfboard**; a real PCB perfboard does not automatically have connected rails.
- Cleaner component zones and fewer diagram annotations, with no unrelated controls.

## Testing the simulator

1. Start simulation. Wait for DHT22/LCD and MPU startup. The simulator MQ-2 uses a brief **5 s warm-up plus 3 s baseline calibration**; the physical MQ-2 firmware uses longer real-hardware intervals.
2. **Temperature/humidity**: click DHT22, change temperature or humidity, then observe the temperature, humidity, or heat-index LCD panel when it rotates. DHT22 polls at 2.2 s intervals.
3. **Smoke**: keep initial gas at 400 ppm until calibration is finished, then select MQ-2 and raise the displayed simulated gas concentration. After the ADC-delta threshold and confirmation period, the yellow LED and **D8** smoke buzzer should activate. Return it to baseline to test clearing.
4. **Fire input**: click the photoresistor proxy and increase **lux** above its ~100 lux digital threshold. It pulls D3 LOW, triggering the simulated fire alarm: red LED and **D10 → D8 → D9**. Reduce lux below threshold to clear after the programmed delay. This tests logic only; light sensing does not prove flame detection.
5. **Shaking / acceleration**: repeatedly change MPU-6050 X/Y acceleration. Compare STABLE, MOTION, STRONG and INTENSE levels; the levels reflect filtered motion, not raw input values. Keep shaking sustained long enough to meet the confirmation interval. The red LED should blink fast and **D9** should beep rapidly. Return to steady acceleration to allow the alert to clear. A single static change may be absorbed by the gravity baseline.
6. **Normal**: after all hazards clear and sensors are ready, green LED should be steady and all buzzers silent.
7. Read Serial Monitor at **115200 baud** for `DATA,...` and `QUAKE,...` lines.

## Alerts and durations

| Condition | Priority | Audio pattern |
| --- | --- | --- |
| Fire (D3 proxy) | 1 | D10 → D8 → D9, 450 ms on + 150 ms gap |
| Gas/smoke (A0) | 2 | D8 550 ms on + 450 ms gap |
| Vibration (MPU-6050) | 3 | D9 200 ms on + 150 ms gap |

**No switch or ACK button remains**. Use Wokwi's Start/Stop Simulation to control the program. Alarm clearing follows sensor input and filter/hold timing, not an artificial acknowledge button.

## Simulation is not electrical proof

The Wokwi simulator's buzzers are *piezo* devices driven by `tone()`. The actual 5V 2-pin active buzzers and SFM-27 must be powered through **correctly rated switching drivers**; do not connect them directly at full rated power to Arduino I/O pins. The breadboard is not a physical soldered perfboard.

Wokwi does not simulate your real 2×18650 battery holder, TP4056 charger, power switch, fuse, regulated 5V boost or safe source isolation. See [Physical wiring](../docs/WIRING_AND_POWER.md).

## Limitation

**The layout has been structurally checked but not tested running in a live Wokwi session.** When testing, record any simulator compilation error or unexpected state in [TEST_PLAN.md](../docs/TEST_PLAN.md).