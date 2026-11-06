# GAYAK: A SMART HEAT INDEX MONITORING AND EARLY WARNING SYSTEM WITH DUAL EARTHQUAKE AND FIRE ALARM DETECTION SYSTEM FOR CLASSROOMS

## 1. Wokwi simulation

1. Open https://wokwi.com/projects/478926664159746049 and **Save a Copy** first.
2. Open the copied project's `diagram.json` tab and paste the content of [../wokwi/diagram.json](../wokwi/diagram.json).
3. Replace `sketch.ino` with [../wokwi/sketch.ino](../wokwi/sketch.ino). Do **not** paste the physical Arduino sketch, since Wokwi piezo buzzers need `tone()`.
4. Replace `libraries.txt` with [../wokwi/libraries.txt](../wokwi/libraries.txt).
5. Click **Start Simulation**. Check that the LCD initializes, green LED eventually illuminates and no buzzer sounds during normal operation.
6. Click **DHT22** to change temperature and humidity. DHT22 is polled about every **2.2 seconds**, and LCD panels rotate about every **1.8 seconds**.
7. Click **MQ-2** and change the simulated ppm after ~5 seconds warming + ~3 seconds calibration. A sustained increase should eventually trigger **D8** and the yellow LED. A change isn't always instant because noise filtering and confirmation are intentional.
8. Click **photoresistor** and increase lux above its digital switching threshold (nominal ~100 lux) to simulate a *fire input*. It drives **D3 LOW** and triggers **D10 → D8 → D9**. Lower lux to clear. **This is not real fire simulation.**
9. Click **MPU-6050** and repeatedly change acceleration on X or Y to simulate **intense shaking and acceleration**; the red LED should blink and **D9** pulse. Leave readings steady for the alert to clear. The module filters gravity, so one static acceleration step is not the same as sustained vibration.
10. In Serial Monitor at **115200 baud**, look for separate `DATA,...` and `QUAKE,...` records.
11. Stop/Start Wokwi to reset sensors and calibration. No extra test buttons or power switch are in this revision.

### What if changes seem delayed?
- **DHT22** normally updates only once every ≥2 s.
- **LCD** rotates panels; active alerts override panel rotation immediately.
- **MQ-2** must finish warm-up and calibration and then exceed a filtered analog *delta* for a confirmation period.
- **MPU-6050** has a startup settling interval and a sustained-vibration confirmation threshold.
- **Fire proxy** uses the digital output threshold and a short confirmation/clear interval.
- Check the Serial Monitor: sensor values can update earlier than a confirmed alert.

## 2. Physical Arduino build

**Before connecting power:** read [WIRING_AND_POWER.md](WIRING_AND_POWER.md). For first testing, leave all lithium batteries/charger modules disconnected. Use USB only and do not drive real active buzzers directly at unverified currents.

1. Connect DHT22 DATA→D2 and MQ-2 AO→A0, with compatible power/GND.
2. Connect flame sensor DO→D3, making sure the module's idle/firing polarity is understood.
3. Connect **Green LED D5**, **Yellow LED D6**, **Red LED D7** through individual **1kΩ** current-limiting resistors; LEDs' negative/cathode sides go to GND.
4. Connect LCD SDA and MPU SDA to **A4**; connect both SCLs to **A5**. Respect 3.3V I2C logic limits using a level shifter when the LCD backpack pulls SDA/SCL to 5V.
5. Wire your active buzzers through independently rated transistor/MOSFET low-side drivers, not directly to the Arduino; see [WIRING_AND_POWER.md](WIRING_AND_POWER.md).
6. In Arduino IDE, install **DHT sensor library (Adafruit)**, **Adafruit Unified Sensor**, and **LiquidCrystal_I2C**; `Wire` is built in.
7. Select **Board → Arduino Uno**, then your COM port, and open `arduino/GAYAK.ino`.
8. Upload while powered via USB. If uploads fail, inspect wiring around D0/D1, the serial port and electrical shorts; no sensor should normally require disconnecting its analog wire.
9. Open Serial Monitor at **115200 baud**, let the MQ-2 finish physical warm-up/baseline, and confirm MPU-6050 READY.
10. Test visual states safely. Use simulated inputs or benign nonhazardous bench exercises; do not ignite materials or release fuel gas.

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