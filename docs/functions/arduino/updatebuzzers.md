---
tags: [gayak, function, arduino]
source: "arduino/GAYAK/GAYAK.ino"
---

# `updateBuzzers()`

Runs non-blocking buzzer patterns using `millis()`. D10 drives the SFM-27 for every alarm. Physical smoke sounds D8 and D10 together once per second; Wokwi sequences D8 then D10 because `tone()` supports one channel. Fire uses three short D10 pulses. Physical shaking sounds D8 + D9 together for 500 ms, then D10 for 500 ms, followed by 150 ms of silence; Wokwi approximates the first phase with D9, then plays D10. Combined fire + smoke uses two fast D10 pulses, followed by a longer D8 + D10 pulse. HI Danger (41–<52°C) uses a repeating siren-style D10 on/off cadence; the active SFM-27 cannot sweep pitch. HI Extreme Danger (≥52°C) uses a faster repeating triple pulse on D10. D11 acknowledges all currently active alert sources and silences their buzzer patterns; still-active sources re-alert after 30 seconds, and cleared sources can trigger again later. Priority is `FIRE + SMOKE > FIRE > SMOKE > HI > SHAKING`; only one alert category runs at a time. The MPU startup fix keeps `quakeAlert` false during its five-second calibration window.

See [[allbuzzersoff]], [[selectedbuzzerpin]], [[updatempu]].
