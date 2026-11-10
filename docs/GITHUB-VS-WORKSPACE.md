---
tags: [gayak, github, versions]
---

# GitHub vs. local workspace

## Revisions checked

- Local: branch `main`, commit `bf8d4c6` (`Fix dashboard rendering: use Unicode em-dash and degree symbols instead of HTML entities`).
- GitHub `main`: commit `b70ced2` (`Remove obsolete premade Wokwi diagram; users wire circuits manually`).
- The local tree was clean when inspected. GitHub was fetched to update the tracking reference; project files were not changed by the fetch.

## Upstream additions and changes

GitHub has newer README, Flask, Arduino firmware, serial parsing, browser JS/CSS, and HTML. It adds guides for adjustments, MPU-6050 shaking, student activities, tests, troubleshooting, tutorials, and wiring/power; a `wokwi/` simulator; and `tests/test_serial_reader.py`. The newer firmware adds MPU-6050 local shaking monitoring, buzzer behavior, and extended telemetry.

The local Arduino sketch has now been updated from GitHub's firmware and includes MPU-6050/QUAKE telemetry, buzzer outputs, and local LCD/LED refinements. The rest of the local checkout remains at the older revision: its Python reader ignores QUAKE records and the dashboard does not display MPU readings. GitHub's tests and Wokwi files are not present locally.

Use local function notes for the checked-out code. The sketch is based on current upstream firmware with a local five-second MPU baseline-calibration fix. Other upstream guides may describe software behavior absent locally; review hardware changes before applying them to a physical device.

- [GitHub repository](https://github.com/itzRealEzra/GAYAK-Early-Warning-System)
- [Upstream guides](https://github.com/itzRealEzra/GAYAK-Early-Warning-System/tree/main/docs)
- [Upstream README](https://github.com/itzRealEzra/GAYAK-Early-Warning-System/blob/main/README.md)
