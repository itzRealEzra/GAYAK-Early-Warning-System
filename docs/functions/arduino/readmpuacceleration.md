---
tags: [gayak, function, arduino]
source: "arduino/GAYAK.ino"
---

# `readMPUAcceleration()`

Reads the six accelerometer bytes beginning at `ACCEL_XOUT_H`, converts signed X/Y/Z samples to g at the configured ±2 g range, and reports read success.

See [[updatempu]], [[initializempu]].
