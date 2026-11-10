---
tags: [gayak, lcd, leds, interface]
---

# LCD and LED status flow

## LCD layout

The 16×2 LCD uses title-case text, short labels, and readings aligned to the right edge. Example rows:

```text
Temp:       58.0 °C
Humid:          45 %
```

Temperature/humidity and heat-index/status rotate with the MPU and MQ-2 pages. Warm-up and calibration pages show the remaining time at the right edge. Hazard messages use compact title-case instructions.

## LED sequence

| Phase | LED indication | Meaning |
|---|---|---|
| MQ-2 warm-up | Red, steady | Sensor warming |
| MQ-2 baseline calibration | Yellow, steady | Baseline being learned |
| Ready | Green, steady | Sensors ready and no active alert |
| MPU unavailable after MQ-2 is ready | Yellow, steady | No green all-clear while shaking monitor is offline |
| Shaking alert | Red, fast blink | Sustained shaking detected |
| Fire / smoke alert | Red / yellow, steady | Alert priority overrides startup status |

The startup lamp test still lights all three LEDs briefly, then switches directly to steady red. Warm-up and calibration no longer blink off between phases.
