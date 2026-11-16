"""Prepare Arduino CLI and upload the canonical GAYAK Uno firmware."""

from __future__ import annotations

import json
import os
import shutil
import struct
import subprocess
import sys
import tempfile
import urllib.error
import urllib.request
import zipfile
from pathlib import Path, PurePosixPath

import serial.tools.list_ports


ROOT = Path(__file__).resolve().parent
SKETCH_DIR = ROOT / "arduino" / "GAYAK"
SKETCH_FILE = SKETCH_DIR / "GAYAK.ino"
LOCAL_CLI = ROOT / ".tools" / "arduino-cli" / "arduino-cli.exe"
CLI_WINDOWS_64 = "https://downloads.arduino.cc/arduino-cli/arduino-cli_latest_Windows_64bit.zip"
CLI_WINDOWS_32 = "https://downloads.arduino.cc/arduino-cli/arduino-cli_latest_Windows_32bit.zip"
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
REQUIRED_LIBRARIES = (
    "DHT sensor library",
    "Adafruit Unified Sensor",
    "LiquidCrystal I2C",
)


class UploadError(RuntimeError):
    """An expected setup, connection, compilation, or upload error."""


def _port_description(port_info) -> str:
    return " ".join(
        str(part) for part in (port_info.device, port_info.description, port_info.hwid or "")
    ).lower()


def _looks_like_arduino(port_info) -> bool:
    description = _port_description(port_info)
    return any(hint in description for hint in ARDUINO_PORT_HINTS)


def find_arduino_port() -> str:
    """Find the configured or most likely Arduino port without guessing ambiguously."""
    ports = list(serial.tools.list_ports.comports())
    configured = os.environ.get("GAYAK_COM_PORT", "").strip()

    if configured:
        match = next((port for port in ports if port.device.lower() == configured.lower()), None)
        if match is not None:
            print(f"[INFO] Using configured Arduino port {match.device}.", flush=True)
            return match.device

    hinted = [port for port in ports if _looks_like_arduino(port)]
    if len(hinted) == 1:
        if configured:
            print(
                f"[WARN] {configured} is unavailable; using detected Arduino port {hinted[0].device}.",
                flush=True,
            )
        else:
            print(f"[INFO] Detected Arduino on {hinted[0].device}.", flush=True)
        return hinted[0].device

    if not hinted and len(ports) == 1:
        print(
            f"[WARN] Windows did not identify the board; using the only serial port {ports[0].device}.",
            flush=True,
        )
        return ports[0].device

    if len(hinted) > 1 or len(ports) > 1:
        choices = ", ".join(
            f"{port.device} ({port.description})" for port in (hinted if hinted else ports)
        )
        raise UploadError(
            f"More than one possible Arduino port is connected: {choices}. "
            "Set GAYAK_COM_PORT to the Uno's COM number and run start_gayak.bat again."
        )

    if configured:
        detail = f"Configured port {configured} was not found. "
    else:
        detail = "No Arduino serial port was found. "
    raise UploadError(
        detail
        + "Connect the Uno with a USB data cable, close Serial Monitor/Plotter, "
        "and run start_gayak.bat again."
    )


def _download_cli() -> Path:
    """Install the official Arduino CLI ZIP into the project-local tools folder."""
    if os.name != "nt":
        raise UploadError("Automatic Arduino CLI setup currently supports Windows only.")

    url = CLI_WINDOWS_64 if struct.calcsize("P") == 8 else CLI_WINDOWS_32
    LOCAL_CLI.parent.mkdir(parents=True, exist_ok=True)
    print("[INFO] Downloading Arduino CLI from the official Arduino server...", flush=True)

    request = urllib.request.Request(url, headers={"User-Agent": "GAYAK-Firmware-Uploader"})
    with tempfile.NamedTemporaryFile(suffix=".zip", delete=False) as temporary:
        archive_path = Path(temporary.name)

    try:
        with urllib.request.urlopen(request, timeout=120) as response, archive_path.open(
            "wb"
        ) as archive:
            shutil.copyfileobj(response, archive)

        with zipfile.ZipFile(archive_path) as archive:
            executable = next(
                (
                    item
                    for item in archive.infolist()
                    if PurePosixPath(item.filename.replace("\\", "/")).name.lower()
                    == "arduino-cli.exe"
                ),
                None,
            )
            if executable is None:
                raise UploadError("The Arduino CLI download did not contain arduino-cli.exe.")

            temporary_executable = LOCAL_CLI.with_suffix(".exe.tmp")
            temporary_executable.write_bytes(archive.read(executable))
            temporary_executable.replace(LOCAL_CLI)
    except (OSError, urllib.error.URLError, zipfile.BadZipFile) as exc:
        raise UploadError(f"Could not install Arduino CLI: {exc}") from exc
    finally:
        archive_path.unlink(missing_ok=True)

    return LOCAL_CLI


def get_cli() -> Path:
    """Use a configured/system CLI, or install a local copy on first launch."""
    configured = os.environ.get("GAYAK_ARDUINO_CLI", "").strip()
    if configured:
        configured_path = Path(configured).expanduser()
        if configured_path.is_dir():
            configured_path /= "arduino-cli.exe"
        if configured_path.is_file():
            return configured_path
        resolved = shutil.which(configured)
        if resolved:
            return Path(resolved)
        raise UploadError(
            "GAYAK_ARDUINO_CLI does not point to an Arduino CLI executable: "
            f"{configured}"
        )

    system_cli = shutil.which("arduino-cli")
    if system_cli:
        return Path(system_cli)
    if LOCAL_CLI.is_file():
        return LOCAL_CLI
    return _download_cli()


def run_cli(
    cli: Path, arguments: list[str], *, capture: bool = False
) -> subprocess.CompletedProcess[str]:
    command = [str(cli), "--no-color", *arguments]
    try:
        result = subprocess.run(
            command,
            cwd=ROOT,
            check=False,
            text=True,
            capture_output=capture,
        )
    except OSError as exc:
        raise UploadError(f"Could not start Arduino CLI: {exc}") from exc

    if result.returncode != 0:
        raise UploadError(
            f"Arduino CLI failed while running '{' '.join(arguments)}' "
            f"(exit code {result.returncode})."
        )
    return result


def _ensure_board_core(cli: Path, fqbn: str) -> None:
    parts = fqbn.split(":")
    if len(parts) < 3 or not all(parts[:3]):
        raise UploadError(f"Invalid Arduino board identifier: {fqbn}")
    core = ":".join(parts[:2])
    installed = run_cli(cli, ["core", "list"], capture=True).stdout
    if core.lower() in installed.lower():
        return

    print("[INFO] Installing the Arduino AVR board core (first run only)...", flush=True)
    run_cli(cli, ["core", "update-index"])
    run_cli(cli, ["core", "install", core])


def _installed_libraries(cli: Path) -> set[str]:
    result = run_cli(cli, ["lib", "list", "--json"], capture=True)
    try:
        data = json.loads(result.stdout)
    except ValueError as exc:
        raise UploadError("Arduino CLI returned an unreadable installed-library list.") from exc

    libraries = data.get("installed_libraries", [])
    return {
        entry.get("library", {}).get("name", "").casefold()
        for entry in libraries
        if isinstance(entry, dict) and isinstance(entry.get("library"), dict)
    }


def _ensure_libraries(cli: Path) -> None:
    installed = _installed_libraries(cli)
    missing = [name for name in REQUIRED_LIBRARIES if name.casefold() not in installed]
    if not missing:
        return

    print("[INFO] Installing the GAYAK Arduino libraries (first run only)...", flush=True)
    run_cli(cli, ["lib", "update-index"])
    for name in missing:
        if name.casefold() in _installed_libraries(cli):
            continue
        run_cli(cli, ["lib", "install", name])


def upload_firmware() -> None:
    if not SKETCH_FILE.is_file():
        raise UploadError(f"The GAYAK firmware sketch was not found: {SKETCH_FILE}")

    port = find_arduino_port()
    fqbn = os.environ.get("GAYAK_FQBN", "arduino:avr:uno").strip() or "arduino:avr:uno"
    cli = get_cli()

    print(f"[INFO] Preparing firmware for {fqbn} from arduino/GAYAK/...", flush=True)
    _ensure_board_core(cli, fqbn)
    _ensure_libraries(cli)

    print("[INFO] Compiling the current firmware...", flush=True)
    run_cli(cli, ["compile", "--fqbn", fqbn, str(SKETCH_DIR)])

    print(f"[INFO] Uploading firmware to {port}. The Arduino will reboot once.", flush=True)
    run_cli(cli, ["upload", "--fqbn", fqbn, "-p", port, str(SKETCH_DIR)])
    print("[READY] Firmware upload completed.", flush=True)


def main() -> int:
    try:
        upload_firmware()
    except UploadError as exc:
        print(f"[ERROR] {exc}", file=sys.stderr, flush=True)
        print(
            "[INFO] The dashboard was not started so it cannot run with an old firmware image.",
            file=sys.stderr,
            flush=True,
        )
        return 1
    except (OSError, ValueError) as exc:
        print(f"[ERROR] Firmware setup failed: {exc}", file=sys.stderr, flush=True)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
