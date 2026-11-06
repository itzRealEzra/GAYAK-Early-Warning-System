# GAYAK: A SMART HEAT INDEX MONITORING AND EARLY WARNING SYSTEM WITH DUAL EARTHQUAKE AND FIRE ALARM DETECTION SYSTEM FOR CLASSROOMS

Work from the simplest fault first: **power → ground → pins → libraries → sensor warm-up → firmware settings → dashboard**. Never troubleshoot by shorting a lithium cell or removing its protection.

| Symptom | Likely cause | Check |
| --- | --- | --- |
| Wokwi wiring looks cluttered | Old or unorganized connections | Rearrange wires manually according to [Wokwi wiring guide](../wokwi/README.md); no imported diagram is necessary |
| Old extra buttons still appear | Legacy controls remain in your Wokwi copy | Delete those parts and their wires manually; use updated `wokwi/sketch.ino` with no button controls |
| Green/red LEDs swapped | Firmware or wires still have previous pin assignment | Current order is **D5 Green, D6 Yellow, D7 Red** |
| LEDs dark or wrong status | Reversed LED, wrong pin, MQ-2 calibrating or MPU offline | Check **D5 Green, D6 Yellow, D7 Red**, each with **220Ω series resistor** toward the LED anode; cathode→GND. Read status output |
| Wokwi piezo makes no noise | Running physical active-buzzer sketch, browser muted or disconnected pin | Use **wokwi/sketch.ino** with `tone()` and verify D8/D9/D10→piezo pin 2, pin 1→GND |
| Physical active buzzer weak or silent | Voltage starved by 1kΩ in series, insufficient GPIO current or bad driver | Use regulated supply and correctly rated MOSFET driver; measure current |
| LCD backlight on but no text | Wrong I2C address/contrast/library/wiring | Confirm 0x27 or scan I2C, SDA=A4 SCL=A5, adjust contrast on real backpack |
| MPU-6050 offline | I2C pin conflict, VCC/GND issue, AD0, wrong address, level issue | Check A4/A5, default address 0x68, 3.3V logic and compatible pullups |
| Earthquake alert stays active | Repeated acceleration changes, filtered-motion hold time, setup disturbance | Stop moving MPU, leave acceleration steady; wait for programmed hold and clear intervals |
| Shaking never triggers | One static adjustment absorbed by gravity filter, insufficient sustained motion | Repeatedly change X/Y acceleration in Wokwi across confirmation period |
| “Magnitude” not displayed | MPU-6050 does not measure seismic magnitude | Report filtered acceleration in `g` and STABLE/MOTION/STRONG/INTENSE local states, never Richter values |
| STRONG or INTENSE appears before buzzer | Instantaneous filtered-motion state changes before sustained-alarm confirmation | Keep controlled shaking above the trigger for `QUAKE_CONFIRM_MS` and review clear/hold timings |
| Fire proxy doesn't trigger in Wokwi | Photoresistor lux too low, wrong D3, old pushbutton sketch | Increase **lux** on the photoresistor until its DO changes LOW; use current sketch |
| Physical flame module false alarms | Sensitivity too high, sunlight, mislearned idle polarity | Shade module and calibrate comparator; check `FLAME_ACTIVE_LOW` |
| MQ-2 reports warming up | Expected startup state | Wokwi ~5 s + 3 s baseline; real firmware ~180 s + 20 s |
| MQ-2 never clears | High gas input, slow baseline recovery, wrong threshold | Return simulator gas value to normal, inspect RAW/FILTERED/BASE/DELTA |
| Sensor data updates slowly | DHT22 reads at ≥2s, LCD panel rotation, alert confirmation | See [adjustments](ADJUSTMENTS.md), avoid unsafe reduction of minimum sensor intervals |
| Arduino upload fails (`resp=0x00`) | Wrong COM port, Serial Monitor/dash app holds port, loose USB, short | Stop Flask/Serial Monitor; check board/port, cable and D0/D1 wiring |
| Dashboard shows DISCONNECTED | COM port unavailable or Python app not running | Check Device Manager, `GAYAK_COM_PORT`, USB cable |
| Dashboard is stale or readings absent | Bad serial baud/data, MPU offline or no DATA | Confirm 115200 baud, `DATA` records and matching firmware |
| No earthquake card in dashboard | Old browser cache/files | Update `templates/index.html`, `static/js/dashboard.js`, `serial_reader.py`; hard-refresh the browser |
| Battery supply resets system | Undersized boost, unprotected/weak pack, wiring/ground fault | Disconnect battery; test stable regulated 5V on bench, review [power safety](WIRING_AND_POWER.md) |
| Battery charger becomes hot | Wrong cells/pack, charge current, wiring, lack of protection | Disconnect immediately; do not charge until qualified inspection |

## Structured debugging procedure

1. **Photograph/log the exact error** and record sketch revision and wiring version.
2. Confirm Wokwi is using the correct **simulation sketch**, and physical Arduino uses **arduino/GAYAK.ino**.
3. Start with all simulated sensor inputs at normal defaults.
4. Open the Serial Monitor and copy the newest `DATA,` and `QUAKE,` lines.
5. Make **one** controlled change; record the time and raw sensor value.
6. Watch for the associated status change, LED and buzzer; expect deliberate confirmation/clear delays.
7. For I2C, separately check LCD and MPU addresses and voltage compatibility.
8. For physical batteries/buzzers, stop and ask the instructor to inspect driver and power wiring instead of bypassing limits.

When reporting a bug, include: screenshot of your **manually wired** circuit, sketch revision/commit, expected and actual behavior, timing, and relevant serial lines.