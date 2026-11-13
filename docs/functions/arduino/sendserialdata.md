---
tags: [gayak, function, arduino]
source: "arduino/GAYAK.ino"
---

# `sendSerialData()`

> [!info] Source
> `arduino/GAYAK.ino` — checked-out workspace revision.

## Purpose

Emits backward-compatible DATA CSV telemetry (10- and 13-field records remain accepted). Current firmware appends a 14th field for HI-warning acknowledgement so the local dashboard can dismiss its popup when D11 is pressed.

## Related notes

- [[../../README|Documentation index]]
- [[../../GITHUB-VS-WORKSPACE|GitHub and workspace revision comparison]]
