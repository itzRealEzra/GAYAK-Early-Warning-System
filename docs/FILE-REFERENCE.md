---
tags: [gayak, files, reference]
---

# Repository file reference

| File | Responsibility |
|---|---|
| `app.py` | Flask routes, RAM history/events, SSE, process lifecycle |
| `serial_reader.py` | Port selection, serial thread, DATA parsing, snapshots |
| `arduino/GAYAK.ino` | Local Arduino firmware |
| `templates/index.html` | Dashboard markup and SVG icon sprite |
| `templates/setup.html` | Local web installation guide |
| `static/js/dashboard.js` | Live updates, rendering, charts, SSE/poll fallback |
| `static/css/style.css` | Layout, state and responsive styles |
| `requirements.txt` | Flask and PySerial |
| `install_gayak.bat` | Creates `.venv` and installs Python dashboard dependencies |
| `start_gayak.bat` | Checks dependencies, opens browser and starts Flask |
| `INSTALLATION.md` | Windows dashboard and Arduino setup instructions |
| `README.md` | Original setup and behavior overview |

GitHub `main` additionally has guides, tests, and Wokwi files as of the revision in [[GITHUB-VS-WORKSPACE]].
