---
tags: [gayak, function, arduino, lcd]
source: "arduino/GAYAK.ino"
---

# `updatePanelButton()`

Debounces the D11 pushbutton configured with `INPUT_PULLUP`. While an HI Danger/Extreme Danger warning is unacknowledged, the first press acknowledges it and silences the extreme-HI buzzer. Otherwise, a press (LOW) advances to the next LCD panel and wraps at the last currently available panel. While MQ-2 warms or calibrates, its status panel is included; after monitoring begins, the panel count drops to three. Active alerts temporarily override the chosen panel without resetting it. The heat warning clears automatically when a valid HI reading drops below 41°C.
