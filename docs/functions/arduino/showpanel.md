---
tags: [gayak, function, arduino]
source: "arduino/GAYAK.ino"
---

# `showPanel()`

> [!info] Source
> `arduino/GAYAK.ino` — checked-out workspace revision.

## Purpose

Renders the user-selected LCD panel, prioritizing fire/smoke, then any unacknowledged HI Danger (≥41°C) or Extreme Danger (≥52°C) warning, then shaking and sensor pages. The HI overlay shows the current value and asks the user to press D11 to acknowledge; it automatically clears below 41°C. Normal readings use title-case labels and right-aligned values, while calibration screens show compact remaining time.

## Related notes

- [[../../README|Documentation index]]
- [[../../GITHUB-VS-WORKSPACE|GitHub and workspace revision comparison]]
