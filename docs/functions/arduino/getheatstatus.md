---
tags: [gayak, function, arduino, heat-index]
source: "arduino/GAYAK.ino"
---

# `getHeatStatus()`

Maps the heat index in tenths of a degree Celsius to status text: `NORMAL` below 27°C, `CAUTION` from 27°C, `EXT CAUT` from 33°C, `DANGER` from 41°C, and `EXT DANG` from 52°C. The 26.x°C interval is treated as Safe/Normal to avoid an unclassified gap in the provided ranges.

See [[updateleds]], [[showpanel]].
