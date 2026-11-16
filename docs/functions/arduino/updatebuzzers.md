---
tags: [gayak, function, arduino]
source: "arduino/GAYAK.ino"
---

# `updateBuzzers()`

Runs non-blocking buzzer patterns using `millis()`. Smoke is one short D8 pulse each second. Fire is three short D10 pulses followed by a pause. Physical shaking alarm drives D8 and D9 together for 500 ms, then D10 SFM for 500 ms, followed by 150 ms of silence. Wokwi approximates the simultaneous first phase with its D9 piezo, then plays D10. When both flame and MQ-2 alerts are active and unacknowledged, a distinct combined pattern plays: two fast D10 pulses followed by a longer D8 pulse. Extreme HI (≥52°C) uses a repeating two-pulse D9 alarm. D11 acknowledges all currently active alert sources and silences their buzzer patterns; still-active sources re-alert after 30 seconds, and cleared sources can trigger again later. Priority is `FIRE + SMOKE > FIRE > SMOKE > EXTREME HI > SHAKING`; only one alert category runs at a time. The MPU startup fix keeps `quakeAlert` false during its five-second calibration window.

See [[allbuzzersoff]], [[selectedbuzzerpin]], [[updatempu]].
