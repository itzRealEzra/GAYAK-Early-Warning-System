---
tags: [gayak, function, arduino]
source: "arduino/GAYAK.ino"
---

# `updateMQ2()`

> [!info] Source
> `arduino/GAYAK.ino` — checked-out workspace revision.

## Purpose

Handles the 2-minute MQ-2 warm-up, 20-second baseline, ADC filtering, delta/slope checks, alert confirmation and clearing. The current provisional trigger is a +20 ADC-count change for 1.5 seconds, with clearing at +12. These are relative starting values; inspect `MQ2 RAW`, `FILTERED`, `BASE`, and `DELTA` serial diagnostics and calibrate in clean air if the sensor still misses smoke or alerts on noise.

## Related notes

- [[../../README|Documentation index]]
- [[../../GITHUB-VS-WORKSPACE|GitHub and workspace revision comparison]]
