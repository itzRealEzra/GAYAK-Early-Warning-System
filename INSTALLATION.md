---
tags: [gayak, windows, installation, dashboard]
---

# GAYAK Dashboard Setup (Windows)

Use this guide when the Arduino has already been programmed with the GAYAK firmware. The dashboard does not need Arduino IDE; it reads live sensor data from the programmed Arduino over USB.

## What You Need

- Windows 10 or 11 PC
- The assembled GAYAK Arduino with firmware already uploaded
- A USB data cable connected between the Arduino and PC
- Python 3 for Windows (the `py` launcher or `python` on PATH)
- This repository downloaded and extracted to a folder
- Internet access for the first Python package installation

You do **not** need Arduino IDE, Arduino libraries, or to upload firmware again for dashboard use. The Arduino itself must stay connected and powered while you want live readings.

## 1. Download The Dashboard

Download and extract the [GAYAK project ZIP](https://github.com/itzRealEzra/GAYAK-Early-Warning-System/archive/refs/heads/main.zip). Keep the files together in the extracted project folder.

## 2. Install Python

If Python 3 is not installed, download it from [Python.org for Windows](https://www.python.org/downloads/windows/). During installation, enable the option to add Python to PATH if offered.

## 3. Install Dashboard Packages

1. Open the extracted project folder.
2. Double-click `install_gayak.bat`.
3. Allow it to download Flask and PySerial. This needs internet access the first time.

The installer creates a project-local `.venv` and does not install these packages system-wide. Run it again to reinstall or update packages from `requirements.txt`.

## 4. Connect The Arduino And Start

1. Connect the already-programmed Arduino to the PC with a USB **data** cable and power it on.
2. Close Arduino Serial Monitor or Serial Plotter if either is open; only one application can use the Arduino COM port at a time.
3. Double-click `start_gayak.bat`.
4. If the browser does not open automatically, visit [http://127.0.0.1:5000](http://127.0.0.1:5000).

Keep the command window open while using the dashboard. Press **Ctrl+C** in that window to stop the server. The dashboard runs on this PC and does not need Wi-Fi, a database, or Arduino IDE.

## First Run

- Confirm the dashboard's **System**, **Arduino USB**, and **Live Feed** indicators connect.
- After a power-up, allow the MQ-2 to complete its firmware warm-up and clean-air baseline (about 2 minutes 20 seconds). This calibration runs on the Arduino; the dashboard simply displays its data.
- Confirm temperature, humidity, HI, gas/smoke, flame, and MPU-6050 readings update.

## Choose A COM Port

The application tries to detect the Arduino automatically. If it selects the wrong port, open Command Prompt in the project folder and run:

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

## Troubleshooting

| Problem | What to do |
| --- | --- |
| Python not found | Install Python for Windows, reopen Command Prompt, then run `install_gayak.bat` again. |
| Package installation fails | Check internet access and retry `install_gayak.bat`. |
| Arduino disconnected | Check that the USB cable supports data, the board is powered, and the correct COM port is selected. Install the USB-serial driver if Windows does not show a port. |
| Another program is using the COM port | Close Arduino Serial Monitor, Serial Plotter, or other serial terminal applications, then restart `start_gayak.bat`. |
| Port 5000 is busy | Set `GAYAK_PORT=5001` in the same Command Prompt before running `start_gayak.bat`. |
| Browser did not open | Open `http://127.0.0.1:5000` manually, or use the port set in `GAYAK_PORT`. |

To change or re-upload the Arduino firmware, a developer will need Arduino IDE and the firmware setup instructions. This is separate from installing or running the dashboard.

The hardware prototype is not a certified smoke, gas, fire, heat, or earthquake alarm.
