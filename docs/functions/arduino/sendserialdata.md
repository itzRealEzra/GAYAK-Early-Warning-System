---
tags: [gayak, function, arduino]
source: "arduino/GAYAK.ino"
---

# `sendSerialData()`

> [!info] Source
> `arduino/GAYAK.ino` — checked-out workspace revision.

## Purpose

Emits backward-compatible DATA CSV telemetry (10-, 13-, and 14-field records remain accepted). Current firmware appends a 14th field for HI-warning acknowledgment and a 15th four-bit alert-acknowledgment mask (`0x01` fire, `0x02` smoke, `0x04` shaking, `0x08` HI) so the dashboard can dismiss alert banners when D11 is pressed.

## Related notes

- [[../../README|Documentation index]]
- [[../../GITHUB-VS-WORKSPACE|GitHub and workspace revision comparison]]
