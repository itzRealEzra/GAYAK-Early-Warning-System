"""GAYAK Hazard Monitoring System - local Flask web dashboard.

Binds to 127.0.0.1:5000 (or GAYAK_PORT) only. Reads live sensor data from the
Arduino over USB serial via serial_reader.SerialReader. The browser receives
updates pushed over Server-Sent Events (/api/stream) the moment a record
arrives; /api/status remains as a polling fallback.

Ctrl+C / SIGTERM shuts down the Flask server, stops the serial reader thread,
and releases the Arduino COM port cleanly.
"""

from __future__ import annotations

import json
import os
import signal
import sys
import time
import threading
from collections import deque
from typing import Deque

from flask import Flask, Response, jsonify, render_template, stream_with_context

from serial_reader import SerialReader

app = Flask(__name__)

# Configuration (override via environment)
COM_PORT = os.environ.get("GAYAK_COM_PORT")  # None = auto-select first port
BAUDRATE = int(os.environ.get("GAYAK_BAUD", "115200"))  # must match the sketch
HOST = "127.0.0.1"
PORT = int(os.environ.get("GAYAK_PORT", "5000"))

reader = SerialReader(port=COM_PORT, baudrate=BAUDRATE)

# Shutdown coordination
_shutdown = threading.Event()


def _handle_signal(signum, frame) -> None:
    """Graceful shutdown on Ctrl+C (SIGINT) or termination (SIGTERM)."""
    _shutdown.set()
    reader.stop(timeout=3.0)
    sys.exit(0)


# In-memory history (RAM only, reset on restart)
HISTORY_MAX = 600
HISTORY_SEND = 240          # 2 min at one point per 0.5 s
HISTORY_STEP = 0.5          # seconds between stored history points
ALERT_EVENTS_MAX = 100
ALERT_EVENTS_SEND = 20

history: Deque[dict] = deque(maxlen=HISTORY_MAX)
alert_events: Deque[dict] = deque(maxlen=ALERT_EVENTS_MAX)
_history_lock = threading.Lock()
_last_smoke_state = False
_last_fire_state = False
_last_history_stamp = None   # last_update of the record already in history
_last_history_t = 0.0        # time of the last stored history point


def build_payload(include_history: bool = True) -> dict:
    """Snapshot + events (+ history), shared by the stream and the poll route.

    The stream sends history only in its first frame; the browser appends the
    new points itself, so each live frame stays tiny."""
    global _last_smoke_state, _last_fire_state, _last_history_stamp, _last_history_t

    snapshot = reader.get_snapshot()
    readings = snapshot["readings"]
    now = time.time()

    with _history_lock:
        # One history point per NEW, fresh record.
        stamp = snapshot["last_update"]
        if (stamp is not None
                and stamp != _last_history_stamp
                and stamp - _last_history_t >= HISTORY_STEP
                and not snapshot["stale"]
                and (readings.get("temperature") is not None
                     or readings.get("humidity") is not None
                     or readings.get("mq2_delta") is not None)):
            _last_history_stamp = stamp
            _last_history_t = stamp
            history.append({
                "t": stamp,
                "temperature": readings.get("temperature"),
                "humidity": readings.get("humidity"),
                "heat_index": readings.get("heat_index"),
                "mq2_delta": readings.get("mq2_delta"),
            })

        # Record alert onset events (transition false -> true).
        smoke_now = bool(readings.get("mq2_alert"))
        if smoke_now and not _last_smoke_state:
            alert_events.append({"t": now, "kind": "smoke",
                                 "message": "GAS/SMOKE ALERT"})
        _last_smoke_state = smoke_now

        fire_now = bool(readings.get("fire_alert"))
        if fire_now and not _last_fire_state:
            alert_events.append({"t": now, "kind": "fire",
                                 "message": "FIRE ALERT"})
        _last_fire_state = fire_now

        hist = list(history)[-HISTORY_SEND:] if include_history else None
        events = list(alert_events)[-ALERT_EVENTS_SEND:]

    payload = {
        "connection_state": snapshot["connection_state"],
        "stale": snapshot["stale"],
        "last_update": snapshot["last_update"],
        "readings": readings,
        "alert_events": events,
        "server_time": now,
    }
    if include_history:
        payload["history"] = hist
    return payload


@app.route("/")
def index():
    return render_template("index.html")


@app.route("/api/status")
def api_status():
    return jsonify(build_payload())


@app.route("/api/stream")
def api_stream():
    """Server-Sent Events: pushes a payload on every new serial record, and at
    least once a second (so stale/offline detection and clocks stay live)."""

    @stream_with_context
    def generate():
        version = -1
        first = True
        yield "retry: 500\n\n"
        while not _shutdown.is_set():
            version = reader.wait_for_update(version, timeout=1.0)
            yield "data: " + json.dumps(build_payload(include_history=first)) + "\n\n"
            first = False

    resp = Response(generate(), mimetype="text/event-stream")
    resp.headers["Cache-Control"] = "no-cache, no-transform"
    resp.headers["X-Accel-Buffering"] = "no"
    return resp


def main() -> None:
    signal.signal(signal.SIGINT, _handle_signal)
    if hasattr(signal, "SIGTERM"):
        signal.signal(signal.SIGTERM, _handle_signal)

    reader.start()
    try:
        app.run(host=HOST, port=PORT, threaded=True, use_reloader=False)
    except (KeyboardInterrupt, SystemExit):
        pass
    finally:
        _shutdown.set()
        reader.stop(timeout=5.0)


if __name__ == "__main__":
    main()
