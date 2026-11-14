---
tags: [gayak, dashboard, flask]
---

# Local dashboard and API

| Route | Behavior |
|---|---|
| `/` | Renders dashboard HTML |
| `/setup` | Renders the Windows installation quick-start guide |
| `/api/status` | Returns readings, freshness, connection, history and alert events as JSON |
| `/api/stream` | Streams JSON Server-Sent Events; history is sent in the first frame |

Browser EventSource provides live updates; polling requests `/api/status` once per second as fallback. Server memory retains at most 600 history points and 100 alert onsets; an initial status sends at most 240 points and 20 events.

The page renders sensor cards, alerts, diagnostics, history charts, events, and mirrored status lamps. The temperature/humidity readings and PAGASA-method HI arrive in each `DATA` record; the dashboard displays the received HI without recalculating it. The card and history chart label the value `HI`, and its severity styling uses the firmware's PAGASA bands: 27–32°C Caution, 33–41°C Extreme Caution, 42–51°C Danger, and 52°C or higher Extreme Danger. It is informational and not a certified hazard detector.

The footer links to the Windows dashboard setup guide. For a client using a pre-programmed Arduino, it covers Python installation, the project installer, USB/COM port connection, dashboard launch, and first-run checks. Arduino IDE is only needed separately to upload or change firmware.
