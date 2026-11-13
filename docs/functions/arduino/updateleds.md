---
tags: [gayak, function, arduino]
source: "arduino/GAYAK.ino"
---

# `updateLeds()`

> [!info] Source
> `arduino/GAYAK.ino` — checked-out workspace revision.

## Purpose

Maintains a continuous status flow: steady red during MQ-2 warm-up, steady yellow during calibration, then maps valid heat index to LEDs after calibration: below 27°C green; 27–<33°C yellow; 33–<41°C yellow + red; 41–<52°C red; and ≥52°C yellow + red. Fire, smoke, and shaking indications keep priority. If HI or MPU data is unavailable while otherwise clear, yellow prevents a false all-clear.

## Related notes

- [[../../README|Documentation index]]
- [[../../GITHUB-VS-WORKSPACE|GitHub and workspace revision comparison]]
