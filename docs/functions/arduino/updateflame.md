---
tags: [gayak, function, arduino]
source: "arduino/GAYAK.ino"
---

# `updateFlame()`

> [!info] Source
> `arduino/GAYAK.ino` — checked-out workspace revision.

## Purpose

Samples the flame module every 50 ms and requires 20 consecutive active samples (about one second) before confirming fire; it clears after 1.5 seconds inactive. This rejects brief flashes, but cannot distinguish a steady infrared light source from flame using the digital output alone. Adjust the module's sensitivity potentiometer so ordinary lamps/sunlight do not switch its DO output.

## Related notes

- [[../../README|Documentation index]]
- [[../../GITHUB-VS-WORKSPACE|GitHub and workspace revision comparison]]
