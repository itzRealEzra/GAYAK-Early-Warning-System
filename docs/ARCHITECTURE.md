---
tags: [gayak, architecture]
---

# Software architecture and data flow

```text
Sensors → Arduino firmware → USB serial CSV → SerialReader thread
                                              ↓
Browser ← Flask routes (/ and /api/*) ← in-memory snapshot/history
   ├─ Server-Sent Events: /api/stream
   └─ Polling fallback: /api/status
```

`arduino/GAYAK.ino` samples sensors and emits rows. `serial_reader.py` reads on a daemon thread, parses DATA rows, and exposes a locked snapshot. `app.py` serves the page and API on loopback. History and alert-onset events stay in RAM and reset on restart. `templates/index.html`, `static/css/style.css`, and `static/js/dashboard.js` provide the browser UI.

There is no database or cloud service in this checkout. Flask binds to `127.0.0.1`.
