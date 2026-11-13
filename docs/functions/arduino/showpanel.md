---
tags: [gayak, function, arduino]
source: "arduino/GAYAK.ino"
---

# `showPanel()`

> [!info] Source
> `arduino/GAYAK.ino` — checked-out workspace revision.

## Purpose

Renders the user-selected LCD panel, prioritizing unacknowledged fire/smoke, HI Danger (≥41°C), and shaking alerts before sensor pages. The HI overlay shows a compact `HI: <value> C` reading without a button instruction. Pressing D11 acknowledges all currently active alert sources and clears the LCD overlays while sensor readings remain live. HI popup acknowledgment is synchronized to the dashboard; fire/smoke/shaking banners also clear. The HI warning clears automatically below 41°C. Normal readings use title-case labels and right-aligned values, while calibration screens show compact remaining time.

## Related notes

- [[../../README|Documentation index]]
- [[../../GITHUB-VS-WORKSPACE|GitHub and workspace revision comparison]]
