---
tags: [gayak, function, arduino]
source: "arduino/GAYAK.ino"
---

# `updateBuzzers()`

Runs non-blocking active-buzzer patterns using `millis()`. Priority is fire, then smoke, then shaking. It selects the corresponding pin, alternates ON and silent phases, and turns every buzzer off when no alarm is active. The MPU startup fix keeps `quakeAlert` false during its five-second calibration window.

See [[allbuzzersoff]], [[selectedbuzzerpin]], [[updatempu]].
