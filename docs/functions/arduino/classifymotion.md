---
tags: [gayak, function, arduino]
source: "arduino/GAYAK.ino"
---

# `classifyMotion()`

Maps filtered RMS acceleration into stable, motion-notice, strong, or intense local motion categories. Returns stable while the sensor is offline or in its startup calibration window. These categories are experimental and do not represent earthquake magnitude.

See [[motionlevelname]], [[updatempu]].
