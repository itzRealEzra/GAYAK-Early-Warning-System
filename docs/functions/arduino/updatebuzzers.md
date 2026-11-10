---
tags: [gayak, function, arduino]
source: "arduino/GAYAK.ino"
---

# `updateBuzzers()`

Runs non-blocking buzzer patterns using `millis()`. Smoke is one short D8 pulse each second. Fire is three short D10 pulses followed by a pause. Shaking alternates D9 and D8 with a brief silent gap. When both flame and MQ-2 alerts are active, a distinct combined pattern plays: two fast D10 pulses followed by a longer D8 pulse. Priority is `FIRE + SMOKE > FIRE > SMOKE > SHAKING`; only one buzzer is energized at a time. Wokwi uses different piezo frequencies on each pin, while physical active buzzers use their fixed hardware tone. The MPU startup fix keeps `quakeAlert` false during its five-second calibration window.

See [[allbuzzersoff]], [[selectedbuzzerpin]], [[updatempu]].
