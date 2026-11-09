---
tags: [gayak, function, arduino]
source: "arduino/GAYAK.ino"
---

# `updateMPU()`

Samples acceleration at the target interval, retries an unavailable sensor, tracks gravity, smooths residual acceleration, and confirms/clears sustained shaking alerts.

## Startup suppression

For the first five seconds after MPU initialization, the function follows the gravity baseline faster, zeros vibration energy/RMS/peak, clears any candidate/alert state, and returns before alert confirmation. This prevents sensor wake-up and initial baseline movement from reaching `updateBuzzers()`. Sustained shaking after calibration is still eligible to alert.

See [[initializempu]], [[classifymotion]], [[updatebuzzers]].
