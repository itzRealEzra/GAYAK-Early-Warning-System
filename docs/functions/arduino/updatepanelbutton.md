---
tags: [gayak, function, arduino, lcd]
source: "arduino/GAYAK.ino"
---

# `updatePanelButton()`

Debounces the D11 pushbutton configured with `INPUT_PULLUP`. A press (LOW) advances to the next LCD panel and wraps at the last currently available panel. While the MQ-2 is warming or calibrating, its status panel is included; after MQ-2 monitoring begins, the panel count drops to three. Active alerts temporarily override the chosen panel without resetting it.
