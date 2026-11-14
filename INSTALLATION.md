---
tags: [gayak, windows, installation, dashboard]
---

# GAYAK Windows Installation

This guide installs the local Flask dashboard and its Python dependencies. The dashboard reads telemetry from the Arduino over USB and listens on this computer only at `127.0.0.1`.

## Requirements

- Windows 10 or 11
- Python 3 for Windows with the `py` launcher, or `python` available on PATH
- This repository extracted to a folder
- Arduino Uno uploaded with `arduino/GAYAK.ino`
- USB cable, and an internet connection for the initial package download

## Install

1. Download the [GAYAK project ZIP](https://github.com/itzRealEzra/GAYAK-Early-Warning-System/archive/refs/heads/main.zip) and extract it, or clone the repository.
2. If Python is not installed, get it from [Python.org for Windows](https://www.python.org/downloads/windows/).
3. Open the extracted project folder.
4. Double-click `install_gayak.bat`.
5. Wait for Flask and PySerial to install. The script creates a `.venv` folder inside the project; it does not install these packages system-wide.

Run the installer again whenever you need to reinstall or update the dependencies listed in `requirements.txt`.

For Arduino firmware builds, use the Arduino IDE Library Manager to install **DHT sensor library (Adafruit)**, **Adafruit Unified Sensor**, and a compatible **LiquidCrystal_I2C** library. `Wire` is included with the Arduino Uno core. The batch installer handles the web dashboard's Python packages; install the Arduino libraries through the IDE.

## Start The Dashboard

1. Upload `arduino/GAYAK.ino` to the Arduino Uno.
2. Connect the Arduino to the PC over USB.
3. Close Arduino IDE Serial Monitor and Serial Plotter. Only one program can open the Arduino's COM port at a time.
4. Double-click `start_gayak.bat`.
5. Open [http://127.0.0.1:5000](http://127.0.0.1:5000) if it does not open automatically.

Keep the command window open while using the dashboard. Press **Ctrl+C** in that window to stop the server. The dashboard is local to the PC and does not require Wi-Fi or a database.

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
| Arduino disconnected | Check the USB cable and selected COM port. Close Serial Monitor and Serial Plotter. |
| Port 5000 is busy | Set `GAYAK_PORT=5001` in the same Command Prompt before running `start_gayak.bat`. |
| Browser did not open | Open `http://127.0.0.1:5000` manually, or use the port set in `GAYAK_PORT`. |

The hardware prototype is not a certified smoke, gas, fire, heat, or earthquake alarm.
