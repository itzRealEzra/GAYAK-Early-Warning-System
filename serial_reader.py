"""Serial reader for GAYAK hazard monitoring system.

Runs a background thread that opens the Arduino COM port, reads newline
delimited records over USB serial, parses DATA records and maintains the
latest valid sensor readings in memory. Diagnostic LOG lines are ignored.

If no port is configured, the reader auto-detects an Arduino-compatible
USB-serial port so data flows to the dashboard without manual setup.
"""

from __future__ import annotations

import threading
import time
from typing import Optional

import serial
import serial.tools.list_ports


# Keywords that indicate a port is likely an Arduino / USB-serial adapter.
ARDUINO_PORT_HINTS = (
    "arduino",
    "ch340",
    "ch341",
    "ftdi",
    "usb serial",
    "usb-serial",
    "silabs",
    "cp210",
    "mega",
    "uno",
)


def _looks_like_arduino(port_info) -> bool:
    text = " ".join(
        str(part) for part in (port_info.device, port_info.description, port_info.hwid or "")
    ).lower()
    return any(hint in text for hint in ARDUINO_PORT_HINTS)


class SerialReader:
    """Thread-safe USB serial reader for the Arduino sensor node."""

    def __init__(
        self,
        port: Optional[str] = None,
        baudrate: int = 9600,
        timeout: float = 1.0,
        stale_threshold: float = 5.0,
        reconnect_delay: float = 3.0,
    ) -> None:
        self.configured_port = port
        self.baudrate = baudrate
        self.timeout = timeout
        self.stale_threshold = stale_threshold
        self.reconnect_delay = reconnect_delay

        self._serial: Optional[serial.Serial] = None
        self._port_lock = threading.Lock()
        self._stop = threading.Event()
        self._thread: Optional[threading.Thread] = None

        self._data_lock = threading.Lock()
        self.latest = {
            "temperature": None,
            "humidity": None,
            "heat_index": None,
            "heat_status": None,
            "mq2_raw": None,
            "mq2_baseline": None,
            "mq2_delta": None,
            "mq2_state": None,
            "mq2_alert": False,
            "fire_alert": False,
            "flame_pin": None,
            "flame_idle": None,
        }
        # Bumped on every new record so streaming clients wake up at once.
        self._cond = threading.Condition()
        self.version = 0
        self.last_update: Optional[float] = None
        self.connection_state = "DISCONNECTED"  # DISCONNECTED | CONNECTED | ERROR

    # ------------------------------------------------------------------ #
    # Lifecycle
    # ------------------------------------------------------------------ #
    def start(self) -> None:
        if self._thread is not None and self._thread.is_alive():
            return
        self._stop.clear()
        self._thread = threading.Thread(target=self._run, name="gayak-serial", daemon=True)
        self._thread.start()

    def stop(self, timeout: float = 5.0) -> None:
        self._stop.set()
        if self._thread is not None:
            self._thread.join(timeout=timeout)
        self._close()

    def list_ports(self) -> list[str]:
        return [p.device for p in serial.tools.list_ports.comports()]

    # ------------------------------------------------------------------ #
    # Port handling (single owner)
    # ------------------------------------------------------------------ #
    def _pick_port(self) -> Optional[str]:
        """Return the configured port, or auto-detect an Arduino port."""
        if self.configured_port:
            return self.configured_port
        for p in serial.tools.list_ports.comports():
            if _looks_like_arduino(p):
                print(f"[serial] Auto-detected Arduino on {p.device}")
                return p.device
        # Fall back to the first available serial port.
        ports = serial.tools.list_ports.comports()
        if ports:
            print(f"[serial] No Arduino hint found, using first port: {ports[0].device}")
            return ports[0].device
        return None

    def _open(self) -> bool:
        port = self._pick_port()
        if not port:
            return False
        with self._port_lock:
            if self._serial is not None:
                return True
            try:
                self._serial = serial.Serial(port, self.baudrate, timeout=self.timeout)
                # Allow the Arduino to reset after the port is opened.
                time.sleep(2.0)
                self._serial.reset_input_buffer()
                self.port = port
                return True
            except (serial.SerialException, OSError) as exc:
                self._serial = None
                self.connection_state = "ERROR"
                print(f"[serial] Cannot open {port}: {exc}")
                return False

    def _reset_readings(self) -> None:
        with self._data_lock:
            self.latest = {
                "temperature": None,
                "humidity": None,
                "heat_index": None,
                "heat_status": None,
                "mq2_raw": None,
                "mq2_baseline": None,
                "mq2_delta": None,
                "mq2_state": None,
                "mq2_alert": False,
                "fire_alert": False,
                "flame_pin": None,
                "flame_idle": None,
            }
        self._notify()

    def _notify(self) -> None:
        with self._cond:
            self.version += 1
            self._cond.notify_all()

    def wait_for_update(self, last_version: int, timeout: float = 1.0) -> int:
        """Block until a new record arrives (or timeout); return the version."""
        with self._cond:
            if self.version == last_version:
                self._cond.wait(timeout)
            return self.version

    def _close(self) -> None:
        self._reset_readings()
        with self._port_lock:
            if self._serial is not None:
                try:
                    self._serial.close()
                except Exception:
                    pass
                self._serial = None
            self.port = None

    # ------------------------------------------------------------------ #
    # Main read loop
    # ------------------------------------------------------------------ #
    def _run(self) -> None:
        while not self._stop.is_set():
            if self._serial is None:
                if not self._open():
                    self.connection_state = "DISCONNECTED"
                    self._stop.wait(self.reconnect_delay)
                    continue

            if self.connection_state != "CONNECTED":
                self.connection_state = "CONNECTED"
                self._notify()
            try:
                line = self._serial.readline()
            except (serial.SerialException, OSError) as exc:
                print(f"[serial] Read error: {exc}")
                self._close()
                self.connection_state = "DISCONNECTED"
                continue

            if not line:
                continue
            try:
                text = line.decode("utf-8", errors="replace").strip()
            except Exception:
                continue
            if not text:
                continue
            # last_update is stamped inside _parse_data, only for a valid
            # DATA record. LOG lines must not keep the link looking "fresh".
            self._handle_line(text)

    # ------------------------------------------------------------------ #
    # Parsing
    # ------------------------------------------------------------------ #
    def _handle_line(self, text: str) -> None:
        if text.startswith("DATA,"):
            self._parse_data(text)
        # LOG lines and any other diagnostics are intentionally ignored.

    def _parse_data(self, text: str) -> None:
        parts = text.split(",")
        if len(parts) not in (10, 13) or parts[0] != "DATA":
            return
        try:
            readings = {
                "temperature": self._to_float(parts[1]),
                "humidity": self._to_float(parts[2]),
                "heat_index": self._to_float(parts[3]),
                "heat_status": self._to_text(parts[4]),
                "mq2_raw": self._to_int(parts[5]),
                # The Arduino sends the baseline with one decimal (e.g.
                # "512.3"); int() rejected it, so it was always None.
                "mq2_baseline": self._to_float(parts[6]),
                "mq2_delta": self._to_int(parts[7]),
                "mq2_state": self._to_text(parts[8]),
                "mq2_alert": parts[9].strip() == "1",
                "fire_alert": False,
                "flame_pin": None,
                "flame_idle": None,
            }
            if len(parts) == 13:
                readings["fire_alert"] = parts[10].strip() == "1"
                readings["flame_pin"] = self._to_int(parts[11])
                readings["flame_idle"] = self._to_int(parts[12])
        except (IndexError, ValueError):
            return

        with self._data_lock:
            self.latest = readings
            self.last_update = time.time()
        self._notify()

    @staticmethod
    def _to_text(value: str):
        value = value.strip()
        if value == "" or value == "null":
            return None
        return value

    @staticmethod
    def _to_float(value: str):
        value = value.strip()
        if value == "" or value == "null":
            return None
        try:
            return float(value)
        except ValueError:
            return None

    @staticmethod
    def _to_int(value: str):
        value = value.strip()
        if value == "" or value == "null":
            return None
        try:
            return int(value)
        except ValueError:
            return None

    # ------------------------------------------------------------------ #
    # Snapshot for Flask
    # ------------------------------------------------------------------ #
    def get_snapshot(self) -> dict:
        with self._data_lock:
            latest = dict(self.latest)
            last_update = self.last_update

        now = time.time()
        stale = last_update is None or (now - last_update) > self.stale_threshold
        return {
            "connection_state": self.connection_state,
            "stale": stale,
            "last_update": last_update,
            "readings": latest,
            "version": self.version,
        }
