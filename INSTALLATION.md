---
tags: [gayak, windows, installation, dashboard]
---

# GAYAK Dashboard Setup (Windows)

Each normal dashboard start checks the connected Arduino and uploads the current firmware from `arduino/GAYAK/GAYAK.ino` before Flask opens the serial port. The dashboard does not need Arduino IDE. It installs the command-line uploader and firmware dependencies automatically on first start.

## What You Need

- Windows 10 or 11 PC
- The assembled GAYAK Arduino Uno R3
- A USB data cable connected between the Arduino and PC
- Python 3 for Windows (the `py` launcher or `python` on PATH)
- This repository downloaded and extracted to a folder
- Internet access for the first Python and Arduino tool installation
- Arduino Serial Monitor, Serial Plotter, and other serial terminal apps closed while starting GAYAK

You do **not** need Arduino IDE. `start_gayak.bat` downloads the official [Arduino CLI](https://docs.arduino.cc/arduino-cli/), the Uno board core, and any missing sketch libraries as needed. The Arduino must remain connected and powered while you want live readings.

## 1. Download The Dashboard

Download and extract the [GAYAK project ZIP](https://github.com/itzRealEzra/GAYAK-Early-Warning-System/archive/refs/heads/main.zip). Keep the files together in the extracted project folder.

## 2. Install Python

If Python 3 is not installed, download it from [Python.org for Windows](https://www.python.org/downloads/windows/). During installation, enable the option to add Python to PATH if offered.

## 3. Install Dashboard Packages

1. Open the extracted project folder.
2. Double-click `install_gayak.bat`.
3. Allow it to download Flask and PySerial. This needs internet access the first time.

The installer creates a project-local `.venv` and does not install these packages system-wide. Run it again to reinstall or update packages from `requirements.txt`. Arduino CLI and the firmware compile dependencies are prepared automatically by the dashboard launcher.

## 4. Connect The Arduino And Start

1. Connect the Arduino to the PC with a USB **data** cable and power it on.
2. Close Arduino Serial Monitor, Serial Plotter, and other apps using the Arduino COM port.
3. Double-click `start_gayak.bat`. It detects the Arduino, compiles the sketch in `arduino/GAYAK/`, uploads it to the Uno, and then starts the dashboard. The Uno reboots once as part of uploading. The first start can take a few minutes while Arduino CLI and the board libraries download.
4. If the browser does not open automatically, visit [http://127.0.0.1:5000](http://127.0.0.1:5000).

Keep the command window open while using the dashboard. Press **Ctrl+C** in that window to stop the server. The dashboard runs on this PC and does not need Wi-Fi, a database, or Arduino IDE.

## First Run

- Confirm the dashboard's **System**, **Arduino USB**, and **Live Feed** indicators connect.
- After a power-up, allow the MQ-2 to complete its firmware warm-up and clean-air baseline (about 2 minutes 20 seconds). This calibration runs on the Arduino; the dashboard simply displays its data.
- Confirm temperature, humidity, HI, gas/smoke, flame, and MPU-6050 readings update.

## Choose A COM Port

The uploader and dashboard use the same port setting. If more than one serial device is connected or automatic detection chooses the wrong one, open Command Prompt in the project folder and run:

```bat
set GAYAK_COM_PORT=COM4
start_gayak.bat
```

Replace `COM4` with the Arduino's port shown in Windows Device Manager. If port 5000 is busy, you can use another local port:

```bat
set GAYAK_PORT=5001
start_gayak.bat
```

Then visit `http://127.0.0.1:5001`.

If you need to run the dashboard without rewriting firmware, set `GAYAK_SKIP_UPLOAD=1` in the same Command Prompt before running `start_gayak.bat`. This is useful when the board is already programmed and you do not want the launcher to compile or upload.

## Troubleshooting

| Problem | What to do |
| --- | --- |
| Python not found | Install Python for Windows, reopen Command Prompt, then run `install_gayak.bat` again. |
| Package installation fails | Check internet access and retry `install_gayak.bat`. |
| Arduino CLI or board libraries fail to download | Check internet access and run `start_gayak.bat` again. Arduino CLI is downloaded from the official Arduino server. |
| Firmware compile or upload fails | Confirm the connected board is an Uno, close Serial Monitor/Plotter, check `GAYAK_COM_PORT`, and start again. The dashboard waits for a successful upload by default. |
| Arduino disconnected | Check that the USB cable supports data, the board is powered, and the correct COM port is selected. Install the USB-serial driver if Windows does not show a port. |
| Another program is using the COM port | Close Arduino Serial Monitor, Serial Plotter, or other serial terminal applications, then restart `start_gayak.bat`. |
| Port 5000 is busy | Set `GAYAK_PORT=5001` in the same Command Prompt before running `start_gayak.bat`. |
| Browser did not open | Open `http://127.0.0.1:5000` manually, or use the port set in `GAYAK_PORT`. |

The dashboard uploader uses `arduino/GAYAK/GAYAK.ino` as its firmware source. Editing that sketch changes the firmware uploaded on the next normal start. Arduino IDE can still be used for manual development, but it is not needed to install or run the dashboard.

The hardware prototype is not a certified smoke, gas, fire, heat, or earthquake alarm.
