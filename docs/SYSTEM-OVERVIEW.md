---
tags: [gayak, system, overview]
---

# GAYAK system overview

GAYAK is an Arduino classroom prototype monitoring temperature, relative humidity, estimated heat index, relative MQ-2 response, and a digital flame input. Firmware drives a 16×2 I2C LCD and status LEDs. A Windows PC can run a local Flask dashboard that reads telemetry over USB serial.

## This checkout includes

- DHT22 readings and derived heat index.
- MQ-2 warm-up, baseline, filtering, and alert confirmation.
- Flame input handling and alert confirmation.
- LCD/status outputs and CSV telemetry.
- Local dashboard with charts, alerts, SSE, and polling fallback.

This educational prototype is not certified life-safety equipment. MQ-2 readings are relative ADC values, not gas concentration. The local firmware includes an experimental MPU-6050 shaking monitor, but the older local Python/dashboard code ignores its supplemental QUAKE records.

See [[ARCHITECTURE]], [[HARDWARE-AND-PINS]], and [[GITHUB-VS-WORKSPACE]].
