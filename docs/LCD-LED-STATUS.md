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

Temperature/humidity and heat-index/status rotate with the MPU and MQ-2 pages. Warm-up and calibration pages show the remaining time at the right edge. Hazard messages use compact title-case instructions. HI Danger (≥41°C) and Extreme Danger (≥52°C) temporarily replace the normal display with a warning and the current HI value. Pressing D11 acknowledges the warning, silences the Extreme Danger buzzer, and clears the matching dashboard popup; the next press resumes panel cycling. The warning also clears automatically when a valid HI reading drops below 41°C.

## LED sequence

| Phase | LED indication | Meaning |
|---|---|---|
| MQ-2 warm-up | Red, steady | Sensor warming |
| MQ-2 baseline calibration | Yellow, steady | Baseline being learned |
| Ready, HI Safe (<27°C) | Green, steady | Heat-index safe band |
| HI Caution (27–<33°C) | Yellow, steady | Heat-index caution band |
| HI Extreme Caution (33–<41°C) | Yellow + red, steady | Heat-index extreme caution band |
| HI Danger (41–<52°C) | Red, steady | LCD warning until D11 acknowledgement or HI <41°C |
| HI Extreme Danger (≥52°C) | Yellow + red, steady | LCD warning and repeating two-pulse D9 buzzer until acknowledgement or HI <41°C |
| MPU unavailable after MQ-2 is ready | Yellow, steady | No green all-clear while shaking monitor is offline |
| Shaking alert | Red, fast blink | Sustained shaking detected |
| Fire / smoke alert | Red / yellow, steady | Alert priority overrides startup status |

The startup lamp test still lights all three LEDs briefly, then switches directly to steady red. Warm-up and calibration remain red/yellow until MQ-2 is ready; after calibration, the LEDs automatically follow the HI bands when no higher-priority sensor alarm is active. Fire, smoke, and shaking indications continue to take priority over the HI LEDs.
