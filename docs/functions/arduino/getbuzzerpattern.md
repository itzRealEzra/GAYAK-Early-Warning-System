---
tags: [gayak, function, arduino]
source: "arduino/GAYAK.ino"
---

# `getBuzzerPattern()`

Returns the step array and number of steps for the active alarm mode: smoke, shaking, fire, or combined fire + smoke. A step identifies one buzzer pin and its duration; pin `0` represents silence.

See [[updatebuzzers]], [[beginbuzzerstep]].
