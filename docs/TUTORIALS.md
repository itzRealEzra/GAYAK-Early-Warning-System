# GAYAK: A SMART HEAT INDEX MONITORING AND EARLY WARNING SYSTEM WITH DUAL EARTHQUAKE AND FIRE ALARM DETECTION SYSTEM FOR CLASSROOMS

## 1. Wokwi simulation — manual wiring

**Build your own wiring. No premade diagram or `diagram.json` import is needed.** Use [the complete pin-by-pin Wokwi guide](../wokwi/README.md).

1. Open https://wokwi.com/projects/478926664159746049 and **Save a Copy**, or create a new Wokwi Arduino Uno R3 project. Remove outdated extra pushbuttons and the simulated switch, if present; add one pushbutton for LCD paging.
2. Add **DHT22, MQ-2 Gas Sensor, Photoresistor Sensor (fire-input proxy), MPU6050, 16x2 I2C LCD, one pushbutton, three LEDs (green/yellow/red), three 220Ω resistors, and three Wokwi piezo buzzers**.
3. Wire DHT22 data → **D2**, MQ-2 AOUT → **A0**, and photoresistor DO → **D3**. Power their VCC/GND from the simulated 5V/GND rails.
4. Wire **D5 → 220Ω → Green LED anode**, **D6 → 220Ω → Yellow LED anode**, and **D7 → 220Ω → Red LED anode**. Connect each cathode to GND.
5. Wire piezo buzzer pin 2 to **D8 (smoke)**, **D9 (shaking)**, and **D10 (fire)**; wire each piezo pin 1 to GND. **No buzzer resistors in Wokwi.** Real two-terminal active buzzers need drivers.
6. Set the LCD to **I2C** at address **0x27**. Connect LCD SDA + MPU SDA to **A4**, and LCD SCL + MPU SCL to **A5**. Power them in simulation, share GND, and connect MPU AD0 to GND for **0x68**.
7. Connect the pushbutton between **D11 and GND**. Firmware enables `INPUT_PULLUP`, so pressing the button pulls D11 LOW; no external resistor is needed.
8. Copy [`wokwi/sketch.ino`](../wokwi/sketch.ino) into your Wokwi sketch tab and [`wokwi/libraries.txt`](../wokwi/libraries.txt) into its libraries tab. Wokwi saves your **own** diagram automatically as you wire.
9. Start simulation and allow initialization (~5s MQ-2 warm-up + ~3s baseline). Normal operation ends with **green D5** and no audible alarm.
10. Change DHT22 values: sensor reads approximately every **2.2s**. Press D11 to acknowledge active alerts; when none need acknowledgment, it advances the LCD panels.
11. Raise MQ-2 gas after calibration to test **yellow D6 + D8**. Adjust photoresistor lux above its threshold (about 100 lux) to test **red D7 + fire buzzer**; this is a simulated light-input proxy, not real flame sensing.
12. Repeatedly change MPU6050 X/Y acceleration to test sustained intense shaking, **blinking red D7 + shaking buzzer**. Restore steady acceleration to clear the alarm after its hold/clear intervals.
13. Use Serial Monitor at **115200 baud** to inspect `DATA` and `QUAKE` lines. Stop/Start simulation to restart. D11 acknowledges active alerts; otherwise it advances the LCD panels. There is no fake power switch.

### What if changes seem delayed?
- **DHT22** normally updates only once every ≥2 s.
- **LCD** uses a debounced D11 press to acknowledge all active alerts; when none need acknowledgment, the press advances panels. Unacknowledged alerts temporarily override the selected panel.
- **MQ-2** must finish warm-up and calibration and then exceed a filtered analog *delta* for a confirmation period.
- **MPU-6050** has a startup settling interval and a sustained-vibration confirmation threshold.
- **Fire proxy** uses the digital output threshold and a short confirmation/clear interval.
- Check the Serial Monitor: sensor values can update earlier than a confirmed alert.

## 2. Physical Arduino build

**Before connecting power:** read [WIRING_AND_POWER.md](WIRING_AND_POWER.md). For first testing, leave all lithium batteries/charger modules disconnected. Use USB only and do not drive real active buzzers directly at unverified currents.

1. Connect DHT22 DATA→D2 and MQ-2 AO→A0, with compatible power/GND.
2. Connect flame sensor DO→D3, making sure the module's idle/firing polarity is understood.
3. Connect the LCD panel pushbutton between **D11 and GND**; the firmware uses `INPUT_PULLUP`, so no external pull-up resistor is required.
4. Connect **Green LED D5**, **Yellow LED D6**, **Red LED D7** through individual **220Ω** current-limiting resistors; LEDs' negative/cathode sides go to GND.
5. Connect LCD SDA and MPU SDA to **A4**; connect both SCLs to **A5**. Respect 3.3V I2C logic limits using a level shifter when the LCD backpack pulls SDA/SCL to 5V.
6. Wire your active buzzers through independently rated transistor/MOSFET low-side drivers, not directly to the Arduino; see [WIRING_AND_POWER.md](WIRING_AND_POWER.md).
7. In Arduino IDE, install **DHT sensor library (Adafruit)**, **Adafruit Unified Sensor**, and **LiquidCrystal_I2C**; `Wire` is built in.
8. Select **Board → Arduino Uno**, then your COM port, and open `arduino/GAYAK.ino`.
9. Upload while powered via USB. If uploads fail, inspect wiring around D0/D1, the serial port and electrical shorts; no sensor should normally require disconnecting its analog wire.
10. Open Serial Monitor at **115200 baud**, let the MQ-2 finish physical warm-up/baseline, and confirm MPU-6050 READY.
11. Test visual states safely. Use simulated inputs or benign nonhazardous bench exercises; do not ignite materials or release fuel gas.

## 3. Windows local dashboard

You will need Python 3, a USB cable, the physical Arduino sketch, and the files in this repository. The dashboard works locally—**no Wi-Fi or database**.

```bat
cd C:\path\to\GAYAK-Early-Warning-System
py -m pip install -r requirements.txt
set GAYAK_COM_PORT=COM4
start_gayak.bat
```

Replace `COM4` with the actual port or leave the variable unset to try automatic detection. Navigate to `http://127.0.0.1:5000`. Another option is `py app.py`.

The firmware emits two machine-readable records:

```text
DATA,temp,humidity,heat_index,heat_status,mq2_raw,mq2_baseline,mq2_delta,mq2_state,mq2_alert,fire_alert,flame_pin,flame_idle
QUAKE,online,alert,filtered_acceleration_rms_g,motion_level
```

Do not assume the sample labels above are an actual measurement. The motion level is `STABLE`, `MOTION`, `STRONG`, `INTENSE` or `null`; the desktop parser also accepts older four-field `QUAKE` messages. Levels describe filtered local acceleration, **not earthquake magnitude**. Fields may be `null` when a sensor is not ready. The dashboard displays both records without changing the existing `DATA` layout.

**Only one program can own the COM port.** Close Arduino Serial Monitor and Serial Plotter before launching the Flask app. If Flask port 5000 is busy, set `GAYAK_PORT=5001`. Use **Ctrl+C** in the terminal to stop the server.

## 4. Classroom testing and reset

- Begin every test from normal conditions and note elapsed warm-up time.
- Change only one input initially; record an observed threshold, LED state and buzzer pattern.
- Restore the input and measure clearing time.
- For conflicting alerts, check priority: fire overrides smoke, smoke overrides shaking.
- Disconnect USB or stop Wokwi only **after saving data**.
- Complete [TEST_PLAN.md](TEST_PLAN.md), including all failures and untested scenarios.
