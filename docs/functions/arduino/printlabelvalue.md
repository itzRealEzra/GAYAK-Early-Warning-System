---
tags: [gayak, function, arduino]
source: "arduino/GAYAK.ino"
---

# `printLabelValue()`

Builds a fixed 16-character LCD row with a short label on the left and its value flush to the right edge. Temperature and heat values include the HD44780 degree glyph. It delegates drawing to `printLine()` so unchanged rows are not rewritten.
