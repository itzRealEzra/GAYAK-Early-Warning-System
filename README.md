GAYAK — Hazard Monitoring System
Local Web Dashboard for Arduino Uno R3 (USB Serial)
====================================================

OVERVIEW
--------
GAYAK is a local-only environmental hazard monitoring dashboard. It reads live
sensor data from an Arduino Uno R3 over USB serial (PySerial) and serves a web
dashboard through Flask on http://127.0.0.1:5000.

No Wi-Fi, no internet, no cloud services, no database and no external network
module are required. Everything runs on your PC.

HARDWARE (current Arduino Uno R3)
---------------------------------
DHT22 DATA D2, Flame DO D3, MQ-2 AO A0
RED LED D5, YELLOW LED D6, GREEN LED D7
Smoke active buzzer D8, Quake active buzzer D9, SFM-27 fire buzzer D10
LCD 0x27 and GY-521 MPU-6050 0x68: SDA=A4, SCL=A5 (shared I2C)
D4, D12 and A1 in the Wokwi project are simulator-only controls.

SOFTWARE DEPENDENCIES (PC)
--------------------------
* Python 3
* Flask
* PySerial

Install with: pip install -r requirements.txt

ARDUINO LIBRARIES (install via Arduino Library Manager)
-------------------------------------------------------
* DHT sensor library by Adafruit
* Adafruit Unified Sensor (required by the DHT library)
* LiquidCrystal I2C by Frank de Brabander

SETUP
-----
1. Wire the Arduino exactly as listed above. Do not change the wiring.
2. Open arduino/GAYAK.ino in the Arduino IDE.
3. Select Tools -> Board -> Arduino Uno and the correct COM port.
4. Upload the sketch.
5. CLOSE the Arduino Serial Monitor. The Python dashboard needs exclusive
   access to the COM port; having the Serial Monitor open will block it.
6. Determine the Arduino COM port (Device Manager -> Ports, e.g. COM3).
7. Run start_gayak.bat. To fix a specific port, run:
      set GAYAK_COM_PORT=COM3
      start_gayak.bat
   or set it permanently in your system environment variables.
8. Open http://127.0.0.1:5000 in your browser (start_gayak.bat does this
   automatically after a short delay).

HOW IT WORKS
------------
* The Arduino samples the DHT22 every 2 s, reads the MQ-2 continuously,
  rotates its LCD panels every 3 s, and sends one DATA record per line over
  serial 5 times per second (and immediately when a smoke or fire alert changes).
* The Python serial reader owns the COM port in a background thread, parses
  DATA records, ignores LOG diagnostics, keeps the latest readings in memory
  and reconnects automatically if the USB cable is unplugged or the Arduino
  resets.
* The browser receives live updates pushed over Server-Sent Events
  (/api/stream) the moment a record arrives. If the stream drops it falls back
  to polling /api/status once per second.
* Serial speed is 115200 baud (sketch + dashboard). Set the Arduino Serial
  Monitor to 115200 too. Override the dashboard with GAYAK_BAUD if needed.
* History and alert events live in RAM only and reset when the application
  restarts.

DASHBOARD STATES
----------------
System status: ONLINE / DISCONNECTED / ERROR (reflects the serial link).
MQ-2 states: WARMING UP / CALIBRATING / MONITORING / SENSOR UNAVAILABLE.
Readings unavailable from the sensor are shown as "—", never fabricated.
Stale data (no update for >5 s) is clearly marked.

MQ-2 ALERT
----------
When the Arduino reports an active gas/smoke alert, the dashboard shows:
  GAS/SMOKE ALERT!
  EVACUATE NOW!
The banner stays visible for as long as the Arduino reports the alert and
records the time the alert began. The dashboard is an additional monitoring
interface only — not a certified smoke alarm or gas detector.

TROUBLESHOOTING
---------------
* "Arduino USB: DISCONNECTED" — check the cable, the COM port, and that no
  other program (Serial Monitor, Putty, another GAYAK instance) holds the port.
* "Wrong COM port" — set GAYAK_COM_PORT to the port shown in Device Manager.
* Values show "—" / "Sensor unavailable" — the DHT22 may be failing or
  warming up; MQ-2 shows WARMING UP / CALIBRATING during its first minutes.
* "Stale data" — no serial record received for more than 5 s.
* "Port 5000 is already in use" — start_gayak.bat names the PID holding it.
  Close it, or run: set GAYAK_PORT=5001 then start_gayak.bat.

FILES
-----
app.py                 Flask web application
serial_reader.py       USB serial reader background thread
templates/index.html   Dashboard HTML
static/css/style.css   Dashboard styles
static/js/dashboard.js Dashboard logic + charts
requirements.txt       Python dependencies
start_gayak.bat        Windows startup script
arduino/GAYAK.ino      Arduino firmware

FIRE ALERT NOT DETECTING?
-------------------------
The "Flame Sensor" panel shows the live D3 level, the idle level learned at
boot and the level that means fire. Hold a lighter ~20 cm from the sensor:
  * Pin does not change   -> turn the module's sensitivity pot, check DO wiring
                             (it must go to D3, not AO) and the 5V/GND pins.
  * Pin changes but no alert -> the idle level was learned wrongly (sun or a
                             lamp was in view at boot). Reboot with the sensor
                             shaded, or set FLAME_AUTO_LEARN = false in the
                             sketch and choose FLAME_ACTIVE_LOW for your module.

FULL SCREEN
-----------
start_gayak.bat opens the dashboard as a full-screen Edge/Chrome app window.
Press F11 (or F on the page) to toggle full screen; double-click the top bar
works too. Set the browser zoom (Ctrl +/-) to taste.

STATUS LIGHT (TAB TITLE + TOP BAR)
----------------------------------
The page mirrors the Arduino LEDs so you can watch it from another tab:
  Warming up   red blinking      Calibrating  yellow blinking
  Working      green solid       Smoke        yellow solid
  Fire         red solid         No signal    grey
The tab title and tab icon change colour (e.g. "green GAYAK - Monitoring"),
and the lamps in the top bar light the same way.


NOVEMBER 2026: MPU-6050 AND 3-BUZZER UPDATE
--------------------------------------------
* Physical firmware: arduino/GAYAK.ino; calibrated vibration thresholds must
  be tested in place. This is motion detection, NOT Richter magnitude.
* Wokwi simulation: wokwi/diagram.json, sketch.ino, libraries.txt and README.md.
  The hosted Wokwi circuit is not auto-updated by this GitHub repository.
* LED pins: D5 RED / D6 YELLOW / D7 GREEN.
* Alarms: FIRE D10 -> D8 -> D9 alternating; SMOKE D8; SHAKING D9.
* The original DATA serial format is preserved for the existing Flask web
  dashboard; MPU data is emitted separately as QUAKE,online,alert,rms_g.
  The present dashboard does not yet show QUAKE telemetry.
* The Wokwi sketch uses tone() for simulated piezo buzzers; do NOT substitute
  it for the real active-buzzer sketch which uses HIGH/LOW outputs.
* Prototype 1kΩ buzzer test wiring limits current but may be too quiet for an
  actual alarm. Verify currents and use rated transistor/MOSFET drivers.
* Use proper 3.3V I2C level shifting as necessary with the physical GY-521.
* The TP4056 is a single-cell/1S charger, not a 5V regulator. Do not connect
  a two-cell series 18650 holder to it. Use a compatible protected 1S battery
  system, a suitable boost regulator, fuse, isolation and safe power path.
* GAYAK is not a certified fire, smoke, gas, or earthquake warning system.
