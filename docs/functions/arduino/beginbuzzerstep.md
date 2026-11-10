---
tags: [gayak, function, arduino]
source: "arduino/GAYAK.ino"
---

# `beginBuzzerStep()`

Turns off all buzzer outputs, starts the selected buzzer for the current pattern step (or keeps them silent for a pause step), and records the start time. This keeps the alarm timing non-blocking so sensor sampling continues.

See [[getbuzzerpattern]], [[allbuzzersoff]], [[updatebuzzers]].
