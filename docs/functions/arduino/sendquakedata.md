---
tags: [gayak, function, arduino]
source: "arduino/GAYAK.ino"
---

# `sendQuakeData()`

Emits a supplemental `QUAKE,online,alert,rms_g,level` serial row at one-second intervals and immediately when the alert state changes. This preserves the existing `DATA` row shape.

See [[motionlevelname]], [[SERIAL-PROTOCOL]].
