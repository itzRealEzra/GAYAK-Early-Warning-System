---
tags: [gayak, function, arduino]
source: "arduino/GAYAK/GAYAK.ino"
---

# `beginBuzzerStep()`

Turns off all buzzer outputs, applies the current step's physical output mask (including D10 when the SFM alarm channel should sound), and records the start time. The Wokwi version instead starts the one selected piezo tone or leaves it silent for a pause. This keeps alarm timing non-blocking so sensor sampling continues.

See [[getbuzzerpattern]], [[allbuzzersoff]], [[updatebuzzers]].
