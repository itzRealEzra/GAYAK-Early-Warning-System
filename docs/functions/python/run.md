---
tags: [gayak, function, python]
source: "serial_reader.py"
---

# `_run()`

> [!info] Source
> `serial_reader.py` — checked-out workspace revision.

## Purpose

Reads lines on a background thread and retries serial connections without restarting Flask. It reopens a port after read errors or when valid Arduino `DATA` records stop arriving, and rediscovers the Arduino if Windows assigns another COM name.

## Related notes

- [[../../README|Documentation index]]
- [[../../GITHUB-VS-WORKSPACE|GitHub and workspace revision comparison]]
