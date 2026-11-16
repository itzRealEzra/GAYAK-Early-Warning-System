---
tags: [gayak, function, python]
source: "serial_reader.py"
---

# `_open()`

> [!info] Source
> `serial_reader.py` — checked-out workspace revision.

## Purpose

Opens the configured or auto-detected serial port, waits for board reset, clears startup bytes, and records failures. If the configured COM port has disappeared, it tries to find an Arduino-compatible port under its new name before the next retry.

## Related notes

- [[../../README|Documentation index]]
- [[../../GITHUB-VS-WORKSPACE|GitHub and workspace revision comparison]]
