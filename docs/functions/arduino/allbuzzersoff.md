---
tags: [gayak, function, arduino]
source: "arduino/GAYAK/GAYAK.ino"
---

# `allBuzzersOff()`

Drives the smoke, shaking, and SFM outputs LOW. The SFM output is D10 and is used by every alarm pattern. Called at startup and during alert-pattern transitions so only outputs selected by the current step sound.

See [[updatebuzzers]], [[selectedbuzzerpin]].
