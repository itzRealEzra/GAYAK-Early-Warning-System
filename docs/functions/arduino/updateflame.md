---
tags: [gayak, function, arduino]
source: "arduino/GAYAK.ino"
---

# `updateFlame()`

> [!info] Source
> `arduino/GAYAK.ino` — checked-out workspace revision.

## Purpose

Samples the flame module every 50 ms and requires 60 consecutive active samples (about three seconds) before confirming fire; it clears after 1.5 seconds inactive. This rejects brief flashes, but cannot distinguish a steady infrared light source from flame using the digital output alone. Aim the module into the room away from windows/sky, adjust its sensitivity potentiometer so ordinary lighting does not switch its DO output, and consider an opaque hood to narrow its field of view.

## Related notes

- [[../../README|Documentation index]]
- [[../../GITHUB-VS-WORKSPACE|GitHub and workspace revision comparison]]
