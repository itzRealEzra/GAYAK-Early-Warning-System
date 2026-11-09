---
tags: [gayak, function, arduino]
source: "arduino/GAYAK.ino"
---

# `initializeMPU()`

Verifies the device identity, wakes/configures the MPU-6050, seeds the gravity estimate from an initial sample, resets alert/filter state, and starts the five-second baseline calibration window. If setup fails, the monitor stays offline and retries later.

See [[readmpuacceleration]], [[updatempu]].
