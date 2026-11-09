---
tags: [gayak, windows, setup]
---

# Running on Windows

1. Install Python 3 and required Arduino libraries (Adafruit DHT, Adafruit Unified Sensor, LiquidCrystal I2C).
2. Upload `arduino/GAYAK.ino` to an Arduino Uno and connect USB.
3. Close Arduino Serial Monitor so Python can own the COM port.
4. Install dependencies: `py -m pip install -r requirements.txt`.
5. Run `start_gayak.bat` or `py app.py`; open `http://127.0.0.1:5000`.

| Environment variable | Default | Purpose |
|---|---:|---|
| `GAYAK_COM_PORT` | Auto-detect | Select a COM port, e.g. `COM3` |
| `GAYAK_BAUD` | `115200` | Must match firmware serial speed |
| `GAYAK_PORT` | `5000` | Local Flask port |

In Command Prompt, set a variable with `set GAYAK_COM_PORT=COM3` before launching in that window.
