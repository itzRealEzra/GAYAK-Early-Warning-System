---
tags: [gayak, function, python]
source: "serial_reader.py"
---

# `_run()`

> [!info] Source
> `serial_reader.py` — checked-out workspace revision.

## Purpose

Reads lines on a background thread and retries serial connections without restarting Flask. It reopens a port after read errors or when the stale timeout passes and Windows no longer lists the open COM port. A present but silent port remains open and is reported as stale, avoiding repeated Arduino resets. Port discovery handles a changed COM name.

## Related notes

- [[../../README|Documentation index]]
- [[../../GITHUB-VS-WORKSPACE|GitHub and workspace revision comparison]]
