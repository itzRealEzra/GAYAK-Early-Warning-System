---
tags: [gayak, function, arduino]
source: "arduino/GAYAK/GAYAK.ino"
---

# `getBuzzerPattern()`

Returns the step array and number of steps for the active alarm mode: smoke, shaking, fire, combined fire + smoke, HI Danger, or HI Extreme Danger. A physical step uses a buzzer output mask plus duration; the SFM mask controls D10, the shared output used by every alarm. The Wokwi version identifies one piezo pin per step because its `tone()` proxy plays one channel at a time; pin `0` represents silence.

See [[updatebuzzers]], [[beginbuzzerstep]].
