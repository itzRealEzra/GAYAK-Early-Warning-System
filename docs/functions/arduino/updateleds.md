---
tags: [gayak, function, arduino]
source: "arduino/GAYAK.ino"
---

# `updateLeds()`

> [!info] Source
> `arduino/GAYAK.ino` — checked-out workspace revision.

## Purpose

Maintains a continuous status flow: steady red during MQ-2 warm-up, steady yellow during calibration (or MPU offline), and steady green when ready. Fire/smoke status overrides remain solid; active shaking uses a fast red blink. Solid warm-up/calibration indications avoid dark gaps between phases.

## Related notes

- [[../../README|Documentation index]]
- [[../../GITHUB-VS-WORKSPACE|GitHub and workspace revision comparison]]
