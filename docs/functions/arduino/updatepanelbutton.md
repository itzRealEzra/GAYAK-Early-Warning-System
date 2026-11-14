---
tags: [gayak, function, arduino, lcd]
source: "arduino/GAYAK.ino"
---

# `updatePanelButton()`

Debounces the D11 pushbutton configured with `INPUT_PULLUP`. If any fire, smoke, shaking, or HI alert is active and not yet acknowledged, one press acknowledges all currently active sources. This clears LCD overlays and dashboard alert banners, silences their buzzer patterns, and sends the HI acknowledgment to dismiss the popup. Fire/smoke/shaking LED alerts clear; HI LEDs continue showing the live heat band. If a source remains active, its alert reappears after `ALERT_ACK_REMINDER_MS` (30 seconds); sources that clear can also trigger again later. If there is no unacknowledged alert, a press (LOW) advances to the next LCD panel and wraps at the last currently available panel. While MQ-2 warms or calibrates, its status panel is included; after monitoring begins, the panel count drops to three.
