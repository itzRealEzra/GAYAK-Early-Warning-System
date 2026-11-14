---
tags: [gayak, windows, setup]
---

# Running on Windows

1. Follow the full [Windows installation guide](../INSTALLATION.md), or open **Setup & Installation** from the local dashboard footer.
2. Install Python 3 and Arduino IDE libraries (Adafruit DHT, Adafruit Unified Sensor, LiquidCrystal I2C).
3. Run `install_gayak.bat` once to create `.venv` and install the Python packages from `requirements.txt`.
4. Upload `arduino/GAYAK.ino` to an Arduino Uno, connect USB, and close Arduino Serial Monitor/Plotter.
5. Run `start_gayak.bat`; open `http://127.0.0.1:5000` if it does not open automatically.

| Environment variable | Default | Purpose |
|---|---:|---|
| `GAYAK_COM_PORT` | Auto-detect | Select a COM port, e.g. `COM3` |
| `GAYAK_BAUD` | `115200` | Must match firmware serial speed |
| `GAYAK_PORT` | `5000` | Local Flask port |

In Command Prompt, set a variable with `set GAYAK_COM_PORT=COM3` before launching in that window. The start script uses the project-local Python virtual environment and runs the installer if the environment is missing or its packages cannot be imported.
