---
tags: [gayak, serial, protocol]
---

# USB serial protocol

Newline-delimited CSV is read over USB serial. The parser accepts 10-field legacy and 13-field extended DATA records. Other lines, including LOG diagnostics, are ignored.

| Field | Meaning |
|---:|---|
| 0 | `DATA` marker |
| 1–3 | Temperature, humidity, heat index |
| 4 | Heat status |
| 5–8 | MQ-2 raw, baseline, delta, state |
| 9 | MQ-2 alert (`1` = active) |
| 10–12 (extended form) | Flame alert, flame pin, learned idle level |

Blank/`null` readings become unavailable values. `last_update` advances only for valid DATA rows; readings become stale after five seconds without one. The updated firmware also sends a separate `QUAKE,online,alert,rms_g,level` line about once per second and immediately when its shaking alert changes. The current Python reader ignores `QUAKE` rows, so the MPU values are not shown in this checkout's dashboard. See [[functions/python/parse-data]], [[functions/python/handle-line]], [[functions/arduino/sendserialdata]], and [[functions/arduino/sendquakedata]].
