---
tags: [gayak, function, arduino]
source: "arduino/GAYAK.ino"
---

# `calculatePAGASAHeatIndexC()`

Recalculates heat index from DHT22 dry-bulb temperature in °C and relative humidity. It follows the PAGASA-published Rothfusz flow: return air temperature at ≤40°F, use the simple apparent-temperature expression when its result is below 79°F, otherwise apply the Rothfusz regression and humidity adjustments. The result is converted back to °C.

PAGASA source: [Heat Index lecture](https://pubfiles.pagasa.dost.gov.ph/climps/climateforum/speciallecture.pdf).
