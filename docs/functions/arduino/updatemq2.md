---
tags: [gayak, function, arduino]
source: "arduino/GAYAK.ino"
---

# `updateMQ2()`

> [!info] Source
> `arduino/GAYAK.ino` — checked-out workspace revision.

## Purpose

Handles the 2-minute MQ-2 warm-up, 20-second baseline, ADC filtering, delta/slope checks, alert confirmation and clearing. The high-sensitivity physical trigger is a filtered +8 ADC-count change for 0.4 seconds, with clearing at +4. Wokwi uses +12 for 0.25 seconds. These are relative response settings, not smoke concentration values; the MQ-2 can react to other gases. Inspect `MQ2 RAW`, `FILTERED`, `BASE`, and `DELTA` serial diagnostics and calibrate in clean air if it alerts on noise.

## Related notes

- [[../../README|Documentation index]]
- [[../../GITHUB-VS-WORKSPACE|GitHub and workspace revision comparison]]
