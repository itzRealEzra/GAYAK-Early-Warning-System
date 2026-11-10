---
tags: [gayak, function, arduino]
source: "arduino/GAYAK.ino"
---

# `updateBuzzers()`

Runs non-blocking active-buzzer patterns using `millis()`. Priority is fire, then smoke, then shaking. Fire cycles all three buzzers; smoke repeats on D8; shaking alternates D8 and D9 every 300 ms for a siren-like pattern. Only one output is energized at a time. The MPU startup fix keeps `quakeAlert` false during its five-second calibration window.

See [[allbuzzersoff]], [[selectedbuzzerpin]], [[updatempu]].
