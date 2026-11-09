---
tags: [gayak, dashboard, flask]
---

# Local dashboard and API

| Route | Behavior |
|---|---|
| `/` | Renders dashboard HTML |
| `/api/status` | Returns readings, freshness, connection, history and alert events as JSON |
| `/api/stream` | Streams JSON Server-Sent Events; history is sent in the first frame |

Browser EventSource provides live updates; polling requests `/api/status` once per second as fallback. Server memory retains at most 600 history points and 100 alert onsets; an initial status sends at most 240 points and 20 events.

The page renders sensor cards, alerts, diagnostics, history charts, events, and mirrored status lamps. It is informational and not a certified hazard detector.
