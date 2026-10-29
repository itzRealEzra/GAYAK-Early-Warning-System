/*
 * GAYAK — Hazard Monitoring System
 * Dashboard frontend (vanilla JS, no external dependencies, offline-safe).
 *
 * Data is PUSHED over Server-Sent Events the moment the Arduino reports a
 * record (about 5 per second). Nothing on the page "refreshes": the DOM only
 * changes when a value actually changes, numbers glide to their new value and
 * the charts scroll continuously on every animation frame.
 */

(function () {
  "use strict";

  const STREAM_URL = "/api/stream";
  const POLL_URL = "/api/status";
  const DASH = "—";
  const DEG = "°";
  const CHART_WINDOW_S = 120;
  const HISTORY_STEP_S = 0.5;
  const HISTORY_KEEP_S = 150;

  const $ = (id) => document.getElementById(id);

  /* ---------- Cheap DOM writers (touch the DOM only on change) ---------- */
  function setText(el, text) { if (el.textContent !== text) el.textContent = text; }
  function setClass(el, name, on) { if (el.classList.contains(name) !== on) el.classList.toggle(name, on); }
  function setCardState(card, state) {
    ["ok", "warn", "bad"].forEach((s) => setClass(card, "state-" + s, s === state));
  }
  function setPill(el, text, kind) {
    setText(el, text);
    const cls = "pill" + (kind ? " pill-" + kind : "");
    if (el.className !== cls) el.className = cls;
  }
  function setDot(el, kind) {
    const cls = "dot dot-" + kind;
    if (el.className !== cls) el.className = cls;
  }
  function fmtTime(ts) {
    if (ts == null) return DASH;
    return new Date(ts * 1000).toLocaleTimeString(undefined, {
      hour: "2-digit", minute: "2-digit", second: "2-digit",
    });
  }
  function levelName(v) { return v == null ? DASH : (v ? "HIGH" : "LOW"); }

  // Icon node for rows built in JS; references the sprite in index.html.
  const SVG_NS = "http://www.w3.org/2000/svg";
  function makeIcon(name) {
    const svg = document.createElementNS(SVG_NS, "svg");
    svg.setAttribute("class", "icon");
    svg.setAttribute("aria-hidden", "true");
    const use = document.createElementNS(SVG_NS, "use");
    use.setAttribute("href", "#i-" + name);
    svg.appendChild(use);
    return svg;
  }

  /* ---------- Smoothly animated numbers ---------- */
  // Each tween eases the shown number toward the latest target every frame.
  const tweens = [];
  function makeTween(id, decimals, unit, withUnitSpan) {
    const el = $(id);
    let numEl = el, unitEl = null;
    if (withUnitSpan) {
      el.textContent = "";
      numEl = document.createElement("span");
      unitEl = document.createElement("span");
      unitEl.className = "unit";
      el.append(numEl, unitEl);
    }
    const t = { el, numEl, unitEl, decimals, unit, target: null, shown: null, text: null };
    tweens.push(t);
    return t;
  }
  const tTemp  = makeTween("valTemp", 1, DEG + "C", true);
  const tHum   = makeTween("valHum", 0, "%", true);
  const tHi    = makeTween("valHi", 1, DEG + "C", true);
  const tRaw   = makeTween("diagRaw", 0, "", false);
  const tBase  = makeTween("diagBaseline", 1, "", false);
  const tDelta = makeTween("diagDelta", 0, "", false);

  function stepTweens() {
    const eps = 0.5 * Math.pow(10, -2); // settle once visually identical
    for (const t of tweens) {
      let text;
      if (t.target == null) {
        t.shown = null;
        text = DASH;
      } else {
        if (t.shown == null) t.shown = t.target;               // first value: no animation
        else {
          const d = t.target - t.shown;
          t.shown = Math.abs(d) < eps ? t.target : t.shown + d * 0.22;
        }
        text = t.shown.toFixed(t.decimals);
      }
      if (text !== t.text) {
        t.text = text;
        t.numEl.textContent = text;
        if (t.unitEl) t.unitEl.textContent = t.target == null ? "" : t.unit;
        setClass(t.el, "unavailable", t.target == null);
      }
    }
  }

  /* ---------- Charts: continuous, time-based ---------- */
  const charts = {
    chartTemp:  { key: "temperature", color: "#ff9500", dec: 1, unit: DEG + "C", cur: "curTemp",  lo: null, hi: null },
    chartHum:   { key: "humidity",    color: "#007aff", dec: 0, unit: "%",        cur: "curHum",   lo: null, hi: null },
    chartHi:    { key: "heat_index",  color: "#ff3b30", dec: 1, unit: DEG + "C", cur: "curHi",    lo: null, hi: null },
    chartDelta: { key: "mq2_delta",   color: "#af52de", dec: 0, unit: "",         cur: "curDelta", lo: null, hi: null },
  };

  let history = [];          // [{t, temperature, humidity, heat_index, mq2_delta}] in SERVER time
  let clockOffset = 0;       // serverTime - clientTime, smoothed
  let haveOffset = false;
  const nowServer = () => Date.now() / 1000 + clockOffset;

  function drawChart(id, now) {
    const cfg = charts[id];
    const canvas = $(id);
    const dpr = window.devicePixelRatio || 1;
    const w = canvas.clientWidth, h = canvas.clientHeight;
    if (!w || !h) return;
    const bw = Math.round(w * dpr), bh = Math.round(h * dpr);
    if (canvas.width !== bw || canvas.height !== bh) { canvas.width = bw; canvas.height = bh; }
    const ctx = canvas.getContext("2d");
    ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
    ctx.clearRect(0, 0, w, h);

    const padL = 40, padR = 10, padT = 10, padB = 18;
    const plotW = w - padL - padR, plotH = h - padT - padB;
    const t0 = now - CHART_WINDOW_S;

    const pts = [];
    for (const p of history) {
      const v = p[cfg.key];
      if (v != null && p.t >= t0 - 2) pts.push({ t: p.t, v: v });
    }

    // grid + x labels
    ctx.strokeStyle = "rgba(0,0,0,0.06)";
    ctx.lineWidth = 1;
    for (let i = 0; i <= 3; i++) {
      const y = padT + (plotH * i) / 3;
      ctx.beginPath(); ctx.moveTo(padL, y); ctx.lineTo(w - padR, y); ctx.stroke();
    }
    ctx.fillStyle = "#a1a1a6";
    ctx.font = "10px -apple-system, 'Segoe UI', sans-serif";
    ctx.textAlign = "left";  ctx.fillText("-2 min", padL, h - 4);
    ctx.textAlign = "right"; ctx.fillText("now", w - padR, h - 4);

    const curEl = $(cfg.cur);
    if (!pts.length) {
      setText(curEl, "");
      ctx.textAlign = "center";
      ctx.font = "12px -apple-system, 'Segoe UI', sans-serif";
      ctx.fillText("Waiting for data…", w / 2, h / 2);
      return;
    }

    // Target Y range; the displayed range eases toward it (no jumps).
    let min = Infinity, max = -Infinity;
    for (const p of pts) { if (p.v < min) min = p.v; if (p.v > max) max = p.v; }
    if (max - min < 1) { const mid = (max + min) / 2; min = mid - 0.5; max = mid + 0.5; }
    const span = max - min;
    min -= span * 0.15; max += span * 0.15;
    if (cfg.lo == null) { cfg.lo = min; cfg.hi = max; }
    else { cfg.lo += (min - cfg.lo) * 0.12; cfg.hi += (max - cfg.hi) * 0.12; }
    const lo = cfg.lo, hi = cfg.hi;

    const X = (t) => padL + ((t - t0) / CHART_WINDOW_S) * plotW;
    const Y = (v) => padT + (1 - (v - lo) / (hi - lo)) * plotH;

    ctx.fillStyle = "#a1a1a6";
    ctx.textAlign = "right";
    ctx.fillText(hi.toFixed(cfg.dec), padL - 6, padT + 8);
    ctx.fillText(lo.toFixed(cfg.dec), padL - 6, padT + plotH);

    // clip so the curve slides under the plot's left edge
    ctx.save();
    ctx.beginPath(); ctx.rect(padL, 0, plotW + padR, h); ctx.clip();

    // The line runs on to "now" at the latest value (no gap on the right).
    const xy = pts.map((p) => [X(p.t), Y(p.v)]);
    const lastP = pts[pts.length - 1];
    xy.push([X(now), Y(lastP.v)]);

    const trace = () => {
      ctx.beginPath();
      ctx.moveTo(xy[0][0], xy[0][1]);
      for (let i = 1; i < xy.length - 1; i++) {
        const mx = (xy[i][0] + xy[i + 1][0]) / 2;
        const my = (xy[i][1] + xy[i + 1][1]) / 2;
        ctx.quadraticCurveTo(xy[i][0], xy[i][1], mx, my);
      }
      ctx.lineTo(xy[xy.length - 1][0], xy[xy.length - 1][1]);
    };

    trace();
    ctx.lineTo(xy[xy.length - 1][0], padT + plotH);
    ctx.lineTo(xy[0][0], padT + plotH);
    ctx.closePath();
    const grad = ctx.createLinearGradient(0, padT, 0, padT + plotH);
    grad.addColorStop(0, cfg.color + "33");
    grad.addColorStop(1, cfg.color + "00");
    ctx.fillStyle = grad;
    ctx.fill();

    trace();
    ctx.strokeStyle = cfg.color;
    ctx.lineWidth = 2;
    ctx.lineJoin = "round";
    ctx.lineCap = "round";
    ctx.stroke();

    const end = xy[xy.length - 1];
    ctx.beginPath();
    ctx.arc(end[0], end[1], 3.5, 0, Math.PI * 2);
    ctx.fillStyle = "#fff";
    ctx.fill();
    ctx.strokeStyle = cfg.color;
    ctx.stroke();
    ctx.restore();

    setText(curEl, lastP.v.toFixed(cfg.dec) + (cfg.unit ? " " + cfg.unit : ""));
  }

  /* ---------- State text ---------- */
  function gasStateText(state) {
    switch (state) {
      case "WARMING_UP": return "Warming up";
      case "CALIBRATING": return "Calibrating";
      case "MONITORING": return "Monitoring";
      default: return state ? "Sensor unavailable" : DASH;
    }
  }
  function heatKind(status) {
    switch (status) {
      case "NORMAL": return "ok";
      case "CAUTION":
      case "EXT CAUT": return "warn";
      case "DANGER":
      case "EXT DANG": return "bad";
      default: return "";
    }
  }
  function latestEventTime(events, kind) {
    for (let i = events.length - 1; i >= 0; i--) if (events[i].kind === kind) return events[i].t;
    return null;
  }


  /* ---------- Status LED: mirrors the Arduino, in the tab title + top bar ---------- */
  // Same rules as the sketch: FIRE red solid, SMOKE yellow solid,
  // warming up red blinking, calibrating yellow blinking, working green solid.
  const LED_EMOJI = { red: "\uD83D\uDD34", yellow: "\uD83D\uDFE1", green: "\uD83D\uDFE2", off: "\u26AA" };
  const LED_HEX = { red: "#ff3b30", yellow: "#ffcc00", green: "#34c759", off: "#c7c7cc" };
  const LED_BLINK_MS = 500;

  let led = { color: "off", blink: false, label: "No signal", title: "Offline" };
  let ledKey = "";

  function ledFor(conn, stale, state, smoke, fire) {
    if (conn !== "CONNECTED" || stale) return { color: "off", blink: false, label: "No signal", title: "Offline" };
    if (fire)  return { color: "red",    blink: false, label: "Fire alert",   title: "FIRE ALERT" };
    if (smoke) return { color: "yellow", blink: false, label: "Smoke alert",  title: "SMOKE ALERT" };
    if (state === "WARMING_UP")  return { color: "red",    blink: true,  label: "Warming up",  title: "Warming up" };
    if (state === "CALIBRATING") return { color: "yellow", blink: true,  label: "Calibrating", title: "Calibrating" };
    if (state === "MONITORING")  return { color: "green",  blink: false, label: "Working",     title: "Monitoring" };
    return { color: "off", blink: false, label: "No signal", title: "Offline" };
  }

  function drawFavicon(color) {
    const c = document.createElement("canvas");
    c.width = c.height = 32;
    const g = c.getContext("2d");
    g.beginPath(); g.arc(16, 16, 13, 0, Math.PI * 2);
    g.fillStyle = color; g.fill();
    g.lineWidth = 2; g.strokeStyle = "rgba(0,0,0,0.18)"; g.stroke();
    $("favicon").href = c.toDataURL("image/png");
  }

  // Called on every stream message (so it keeps blinking in a background tab,
  // where timers are throttled but the push stream is not) and on a timer.
  function applyLed() {
    const lit = !led.blink || Math.floor(Date.now() / LED_BLINK_MS) % 2 === 0;
    const shown = lit ? led.color : "off";
    const key = led.title + "|" + shown;
    if (key === ledKey) return;
    ledKey = key;

    const t = " GAYAK \u2014 " + led.title;
    if (document.title !== t) document.title = t;
    drawFavicon(LED_HEX[shown]);

    setClass($("lampRed"), "on", shown === "red");
    setClass($("lampYellow"), "on", shown === "yellow");
    setClass($("lampGreen"), "on", shown === "green");
    setText($("ledLabel"), led.label);
  }

  /* ---------- Apply one payload ---------- */
  let eventsSig = "";
  let lastPointT = 0;
  let lastStamp = null;

  function ingestHistory(data) {
    if (data.history) {
      history = data.history.slice();
      lastPointT = history.length ? history[history.length - 1].t : 0;
    } else if (data.last_update && data.last_update !== lastStamp && !data.stale &&
               data.last_update - lastPointT >= HISTORY_STEP_S) {
      const r = data.readings || {};
      history.push({
        t: data.last_update,
        temperature: r.temperature, humidity: r.humidity,
        heat_index: r.heat_index, mq2_delta: r.mq2_delta,
      });
      lastPointT = data.last_update;
      const cut = data.last_update - HISTORY_KEEP_S;
      while (history.length && history[0].t < cut) history.shift();
    }
    lastStamp = data.last_update;
  }

  function render(data) {
    // Keep a smoothed server<->client clock offset so the charts glide.
    if (data.server_time) {
      const off = data.server_time - Date.now() / 1000;
      clockOffset = haveOffset ? clockOffset * 0.9 + off * 0.1 : off;
      haveOffset = true;
    }
    ingestHistory(data);

    const conn = data.connection_state || "DISCONNECTED";
    const stale = !!data.stale;
    const r = data.readings || {};
    const events = data.alert_events || [];

    setClass(document.body, "stale", stale || conn !== "CONNECTED");

    if (conn === "CONNECTED" && !stale) {
      setText($("sysStatus"), "Online");            setDot($("sysDot"), "on");
      setText($("arduinoStatus"), "Connected");     setDot($("usbDot"), "on");
    } else if (conn === "CONNECTED") {
      setText($("sysStatus"), "Stale data");        setDot($("sysDot"), "warn");
      setText($("arduinoStatus"), "No data");       setDot($("usbDot"), "warn");
    } else if (conn === "ERROR") {
      setText($("sysStatus"), "Error");             setDot($("sysDot"), "warn");
      setText($("arduinoStatus"), "Port error");    setDot($("usbDot"), "bad");
    } else {
      setText($("sysStatus"), "Offline");           setDot($("sysDot"), "off");
      setText($("arduinoStatus"), "Disconnected");  setDot($("usbDot"), "off");
    }
    setText($("lastUpdate"), fmtTime(data.last_update));

    /* Numeric targets (animated by stepTweens) */
    tTemp.target = r.temperature;
    tHum.target = r.humidity;
    tHi.target = r.heat_index;
    tRaw.target = r.mq2_raw;
    tBase.target = r.mq2_baseline;
    tDelta.target = r.mq2_delta;

    setText($("metaTemp"), r.temperature == null ? "Sensor unavailable" : "");
    setText($("metaHum"), r.humidity == null ? "Sensor unavailable" : "");
    setText($("metaHi"), r.heat_status || DASH);
    setCardState($("cardHi"), heatKind(r.heat_status));

    /* Gas / smoke */
    const smoke = !!r.mq2_alert;
    const state = r.mq2_state || null;
    if (smoke) {
      setText($("valGas"), "ALERT");
      setText($("metaGas"), "Gas / smoke detected");
      setCardState($("cardGas"), "warn");
      setPill($("diagStatePill"), "Alert", "warn");
    } else {
      setText($("valGas"), gasStateText(state));
      setText($("metaGas"), state === "MONITORING" ? "Air normal" : (state ? "Please wait" : DASH));
      setCardState($("cardGas"), state === "MONITORING" ? "ok" : "");
      setPill($("diagStatePill"), gasStateText(state),
        state === "MONITORING" ? "ok" : (state ? "warn" : ""));
    }
    setClass($("valGas"), "unavailable", !state);

    /* Flame / fire */
    const fire = !!r.fire_alert;
    const haveFlame = r.flame_pin != null;
    if (fire) {
      setText($("valFire"), "FIRE");
      setText($("metaFire"), "");
      setCardState($("cardFire"), "bad");
      setPill($("flamePill"), "Fire", "bad");
    } else if (haveFlame || state) {
      setText($("valFire"), "Clear");
      setText($("metaFire"), "");
      setCardState($("cardFire"), "ok");
      setPill($("flamePill"), "Clear", "ok");
    } else {
      setText($("valFire"), DASH);
      setText($("metaFire"), "");
      setCardState($("cardFire"), "");
      setPill($("flamePill"), DASH, "");
    }
    setClass($("valFire"), "unavailable", !fire && !haveFlame && !state);

    /* Banners */
    setClass($("fireBanner"), "hidden", !fire);
    setClass($("smokeBanner"), "hidden", !smoke);
    if (fire) { const t = latestEventTime(events, "fire"); setText($("fireStarted"), t ? "Active since " + fmtTime(t) : ""); }
    if (smoke) { const t = latestEventTime(events, "smoke"); setText($("smokeStarted"), t ? "Active since " + fmtTime(t) : ""); }
    led = ledFor(conn, stale, state, smoke, fire);
    applyLed();

    /* Diagnostics */
    setText($("flamePin"), levelName(r.flame_pin));
    setText($("flameIdle"), levelName(r.flame_idle));
    setText($("flameFire"), r.flame_idle == null ? DASH : levelName(r.flame_idle ? 0 : 1));

    /* Alert events: rebuilt only when the list really changed */
    const sig = events.map((e) => e.t + e.kind).join("|");
    if (sig !== eventsSig) {
      eventsSig = sig;
      const list = $("eventsList");
      list.textContent = "";
      if (events.length) {
        events.slice().reverse().forEach((e) => {
          const row = document.createElement("div");
          row.className = "event-row";
          const time = document.createElement("span");
          time.className = "event-time";
          time.textContent = fmtTime(e.t);
          const isFire = e.kind === "fire";
          const tag = document.createElement("span");
          tag.className = "event-tag " + (isFire ? "fire" : "smoke");
          tag.append(makeIcon(isFire ? "flame" : "smoke"),
                     document.createTextNode(isFire ? "Fire alert" : "Gas / smoke alert"));
          row.append(time, tag);
          list.appendChild(row);
        });
      } else {
        const empty = document.createElement("div");
        empty.className = "empty";
        empty.textContent = "No alert events recorded.";
        list.appendChild(empty);
      }
    }
  }

  function renderOffline() {
    setText($("sysStatus"), "Offline");           setDot($("sysDot"), "off");
    setText($("arduinoStatus"), "Unreachable");   setDot($("usbDot"), "off");
    setClass(document.body, "stale", true);
    led = ledFor("DISCONNECTED", true, null, false, false);
    applyLed();
  }

  /* ---------- Live feed: SSE, with polling only as a fallback ---------- */
  let es = null;
  let pollTimer = null;
  function setFeed(kind, text) { setDot($("feedDot"), kind); setText($("feedStatus"), text); }

  async function pollOnce() {
    try {
      const resp = await fetch(POLL_URL, { cache: "no-store", credentials: "omit" });
      if (!resp.ok) throw new Error("HTTP " + resp.status);
      render(await resp.json());
      setFeed("warn", "Polling");
    } catch (err) {
      renderOffline();
      setFeed("off", "Offline");
    }
  }
  function startPolling() { if (!pollTimer) { pollOnce(); pollTimer = setInterval(pollOnce, 1000); } }
  function stopPolling() { if (pollTimer) { clearInterval(pollTimer); pollTimer = null; } }

  function startStream() {
    if (!window.EventSource) { startPolling(); return; }
    es = new EventSource(STREAM_URL);
    es.onopen = () => { stopPolling(); setFeed("on", "Live"); };
    es.onmessage = (ev) => {
      stopPolling();
      try { render(JSON.parse(ev.data)); setFeed("on", "Live"); } catch (err) { /* skip bad frame */ }
    };
    es.onerror = () => { setFeed("warn", "Reconnecting"); startPolling(); };
  }

  /* ---------- One animation loop for everything that moves ---------- */
  let lastClock = "";
  function frame() {
    stepTweens();
    const now = nowServer();
    for (const id in charts) drawChart(id, now);
    const clock = new Date().toLocaleTimeString(undefined, { hour: "2-digit", minute: "2-digit", second: "2-digit" });
    if (clock !== lastClock) { lastClock = clock; setText($("pcTime"), clock); }
    requestAnimationFrame(frame);
  }

  /* ---------- Full screen: press F (or double-click the status bar) ---------- */
  function toggleFullscreen() {
    if (document.fullscreenElement) document.exitFullscreen();
    else if (document.documentElement.requestFullscreen) document.documentElement.requestFullscreen().catch(() => {});
  }
  document.addEventListener("keydown", (e) => {
    if ((e.key === "f" || e.key === "F") && !e.ctrlKey && !e.metaKey && !e.altKey) toggleFullscreen();
  });
  document.querySelector(".toolbar").addEventListener("dblclick", toggleFullscreen);

  setInterval(applyLed, 250);
  applyLed();
  startStream();
  requestAnimationFrame(frame);
})();