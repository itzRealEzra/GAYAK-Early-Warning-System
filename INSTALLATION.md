---
tags: [gayak, windows, installation, dashboard]
---

# GAYAK Windows Installation

This guide walks through the complete Windows setup: install Arduino IDE and libraries, wire and upload the hardware firmware, then install and start the local Flask dashboard. The dashboard reads the Arduino over USB and listens on this computer only at `127.0.0.1`.

## Requirements

- Windows 10 or 11
- Arduino Uno or compatible ATmega328P board and USB data cable
- Python 3 for Windows with the `py` launcher, or `python` available on PATH
- This repository extracted to a folder
- DHT22, MQ-2 with AO, flame sensor with DO, GY-521/MPU-6050, and 16×2 I2C LCD
- Three LEDs with individual 220Ω resistors, pushbutton, three active buzzers, suitable buzzer driver transistors/MOSFETs, jumper wires and breadboard/perfboard
- Internet access for initial downloads

## 1. Download The Project And Software

1. Download the [GAYAK project ZIP](https://github.com/itzRealEzra/GAYAK-Early-Warning-System/archive/refs/heads/main.zip) and extract it, or clone the repository.
2. Install [Arduino IDE for Windows](https://www.arduino.cc/en/software/).
3. Install [Python 3 for Windows](https://www.python.org/downloads/windows/) if it is not already installed.

Arduino's official [IDE setup guide](https://support.arduino.cc/hc/en-us/articles/360019833020-Download-and-install-Arduino-IDE) covers the Windows installer. If Windows does not show your board's COM port, inspect Device Manager and install the driver for the USB-serial chip fitted to that board.

## 2. Install Arduino Board And Libraries

1. Open Arduino IDE.
2. If **Arduino Uno** is missing from **Tools → Board**, install the Arduino AVR Boards package in **Tools → Board → Boards Manager**. See Arduino's [board package guide](https://support.arduino.cc/hc/en-us/articles/360016119519-Add-boards-to-Arduino-IDE).
3. In **Tools → Manage Libraries**, install:
   - **DHT sensor library** by Adafruit
   - **Adafruit Unified Sensor**
   - A compatible **LiquidCrystal_I2C** library
4. `Wire` is included with the Uno board package.

See Arduino's [Board Manager instructions](https://support.arduino.cc/hc/en-us/articles/360016119519-Add-boards-to-Arduino-IDE) and [Library Manager instructions](https://support.arduino.cc/hc/en-us/articles/5145457742236-Install-libraries-in-the-Arduino-IDE) if needed.

## 3. Wire The Hardware

Use the complete [hardware pin and power guide](https://github.com/itzRealEzra/GAYAK-Early-Warning-System/blob/main/docs/WIRING_AND_POWER.md) before assembly. Quick pin map:

| Uno pin | Connect to |
| --- | --- |
| D2 | DHT22 DATA |
| D3 | Flame module DO |
| A0 | MQ-2 AO |
| A4 / A5 | LCD and GY-521 shared SDA / SCL bus |
| D5 / D6 / D7 | Green / yellow / red LEDs, each with its own 220Ω series resistor |
| D8 / D9 / D10 | Smoke / shaking / fire buzzer driver inputs |
| D11 | Pushbutton to GND (`INPUT_PULLUP`) |

Join grounds, check each module's voltage and output polarity, and keep D0/D1 free for USB serial. A bare DHT22 may need a data pull-up; some modules include one. Check I2C voltage compatibility between the LCD backpack and GY-521; use the level shifting described in the wiring guide when needed. Drive physical buzzers through appropriately rated transistor/MOSFET circuits; never power them directly from Arduino GPIO. **For first tests, power the Uno by USB and leave the battery/TP4056 disconnected.** Battery operation is optional and requires the separate power guide.

## 4. Upload The Physical Firmware

1. In Arduino IDE, open `arduino/GAYAK/GAYAK.ino`.
2. Select **Tools → Board → Arduino Uno** and select the Arduino's COM port under **Tools → Port**.
3. Make sure the flame sensor is aimed at a normal indoor scene during startup so firmware can learn its idle DO level. Keep MQ-2 in clean air during its baseline calibration.
4. Click **Upload** and wait for the success message.
5. Open Serial Monitor at **115200 baud** and confirm sensor/status output appears. Close Serial Monitor before starting the dashboard.

The MQ-2 has a 2-minute warm-up followed by a 20-second baseline calibration. Its readings are relative gas response, not certified smoke concentration. Use Wokwi or benign bench inputs for functional trials; don't create smoke or flame indoors.

## 5. Install The Local Dashboard

1. Open the extracted project folder.
2. Double-click `install_gayak.bat`.
3. Allow it to download Flask and PySerial. An internet connection is needed for the initial install.
4. The installer creates `.venv` in the project folder and installs packages there rather than system-wide.

Run `install_gayak.bat` again to update/reinstall packages from `requirements.txt`.

## 6. Launch And Connect

1. Connect the uploaded Arduino to the PC with a USB data cable.
2. Close Arduino IDE Serial Monitor and Serial Plotter. Only one program can open the Arduino's COM port at a time.
3. Double-click `start_gayak.bat`.
4. Open [http://127.0.0.1:5000](http://127.0.0.1:5000) if it does not open automatically.

Keep the command window open while using the dashboard. Press **Ctrl+C** in that window to stop the server. The dashboard is local to the PC and does not require Wi-Fi or a database.

## First-Run Check

- The dashboard's **System**, **Arduino USB**, and **Live Feed** indicators should connect and update.
- Allow the physical MQ-2 warm-up and clean-air baseline to finish; on this firmware that is about 2 minutes 20 seconds.
- Confirm temperature, humidity, HI, MQ-2 values, and shaking data appear. If the dashboard reports Arduino disconnected, select the correct COM port below.
- Use the Wokwi simulator or benign bench inputs for alarm checks. Do not make smoke or a flame to test the classroom prototype.

## Choose A COM Port

The application tries to detect the Arduino automatically. To choose a port manually, open Command Prompt in the project folder and run:

```bat
set GAYAK_COM_PORT=COM4
start_gayak.bat
```

Replace `COM4` with the Arduino's port from Windows Device Manager. `GAYAK_PORT` can change the web address port if 5000 is already occupied, for example:

```bat
set GAYAK_PORT=5001
start_gayak.bat
```

## Troubleshooting

| Problem | What to do |
| --- | --- |
| Python not found | Install Python for Windows, then reopen Command Prompt and run `install_gayak.bat` again. |
| Package installation fails | Check internet access and retry `install_gayak.bat`. |
| Arduino IDE cannot find Uno | Check USB/data cable and Device Manager; select/install the Uno board package in Boards Manager. For clone boards, install the driver matching the USB-serial chip. |
| Sketch cannot find a library | Reopen Library Manager, confirm the library is installed, then restart Arduino IDE and compile again. |
| Arduino disconnected | Check the USB cable and selected COM port. Close Serial Monitor and Serial Plotter. |
| Port 5000 is busy | Set `GAYAK_PORT=5001` in the same Command Prompt before running `start_gayak.bat`. |
| Browser did not open | Open `http://127.0.0.1:5000` manually, or use the port set in `GAYAK_PORT`. |

The hardware prototype is not a certified smoke, gas, fire, heat, or earthquake alarm.
