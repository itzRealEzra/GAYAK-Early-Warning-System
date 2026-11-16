---
tags: [gayak, function, arduino]
source: "arduino/GAYAK.ino"
---

# `updateMQ2()`

> [!info] Source
> `arduino/GAYAK.ino` — checked-out workspace revision.

## Purpose

Handles the 2-minute MQ-2 warm-up, 20-second baseline, ADC filtering, delta/slope checks, alert triggering, and clearing. Physical and Wokwi settings trigger on the first sampled filtered rise of 4 ADC counts above baseline, with no added confirmation delay; an active alert clears after the delta remains at or below 2 for the configured clear interval. The ADC sampling interval is about 100 ms, and sensor response and filtering still affect latency. These are relative response settings, not smoke concentration values; the MQ-2 can react to other gases. Inspect `MQ2 RAW`, `FILTERED`, `BASE`, and `DELTA` serial diagnostics and calibrate in clean air if it alerts on noise.

## Related notes

- [[../../README|Documentation index]]
- [[../../GITHUB-VS-WORKSPACE|GitHub and workspace revision comparison]]
