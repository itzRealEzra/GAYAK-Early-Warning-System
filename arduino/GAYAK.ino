#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <avr/pgmspace.h>

// ================ PIN CONFIGURATION ================

#define DHT_PIN 2
#define DHT_TYPE DHT22
#define FLAME_PIN 3
#define MQ2_PIN A0

// Status LEDs (each through a 220-330 ohm resistor to GND)
#define LED_GREEN_PIN  5
#define LED_YELLOW_PIN 6
#define LED_RED_PIN    7

// ================ COMPONENTS ================

DHT dht(DHT_PIN, DHT_TYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ================ SETTINGS ================

const unsigned long PANEL_INTERVAL  = 3000UL;
const unsigned long SENSOR_INTERVAL = 2500UL;

// Change only after comparing with a reliable thermometer.
const float TEMP_OFFSET = 0.0;

// LED behaviour
const unsigned long LED_BLINK_MS = 500UL;   // blink half-period (warm-up / calibrating)
const bool GREEN_BLINK_WHEN_READY = false;  // true = green blinks, false = green solid

// MQ-2 smoke detector timing
const unsigned long MQ2_WARMUP_MS       = 180000UL;  // was 120000UL (longer settle = steadier baseline)
const unsigned long MQ2_BASELINE_MS     = 20000UL;   // was 10000UL (more samples averaged)
const unsigned long MQ2_SAMPLE_INTERVAL = 100UL;
const unsigned long MQ2_CLEAR_TIME      = 1500UL;
const unsigned long MQ2_DEBUG_INTERVAL  = 1000UL;

// False-alarm protection
const int MQ2_OVERSAMPLE = 16;                   // ADC reads averaged per sample
const unsigned long MQ2_CONFIRM_MS = 2000UL;     // delta must stay high this long (was 3 samples = 0.3 s)

// Provisional ADC-difference thresholds (NOT calibrated concentrations).
const int MQ2_TRIGGER_DELTA   = 40;   // raised back from 30: 30 sat inside the noise
const int MQ2_CLEAR_DELTA     = 25;   // raised from 20, keeps hysteresis below the trigger

// Asymmetric filter: slow to rise, fast to fall
const float MQ2_RISE_ALPHA = 0.15f;   // was 0.3f: rejects short upward spikes
const float MQ2_FALL_ALPHA = 0.6f;

// Baseline tracking (only applied while delta < MQ2_TRIGGER_DELTA)
const float MQ2_BASELINE_RISE_IDLE   = 0.001f;  // was 0.0005f: follows slow thermal/humidity drift
const float MQ2_BASELINE_FALL        = 0.02f;   // fast downward drift
const float MQ2_BASELINE_RISE_ALERT  = 0.05f;   // releases a stuck offset

// Fast-recovery: lets the baseline chase a decaying sensor tail while an
// alert is active, without absorbing real (persistent) smoke.
const float MQ2_BASELINE_RISE_RECOVERY = 0.05f;  // chase speed during decay
const float MQ2_RECOVERY_FRACTION      = 0.6f;   // chase only below 60% of peak
const float MQ2_SLOPE_TOLERANCE        = 1.0f;   // ADC counts per second
const unsigned long MQ2_SLOPE_WINDOW   = 1000UL;

// Serial telemetry interval (GAYAK dashboard)
const unsigned long SERIAL_INTERVAL_MS = 200UL;    // DATA record, 5 per second
const long SERIAL_BAUD = 115200;                   // must match GAYAK_BAUD in app.py
unsigned long lastSerialSend = 0;
bool lastSentSmoke = false;   // alert state in the last DATA record
bool lastSentFlame = false;

// Flame sensor settings
// Auto-learn reads the pin at boot (no flame in view) and treats that
// level as "no fire". Fire = the pin leaving that level.
const bool FLAME_AUTO_LEARN = true;

// Used only when FLAME_AUTO_LEARN is false.
// true = OUT goes LOW on fire, false = OUT goes HIGH on fire.
const bool FLAME_ACTIVE_LOW = true;

const unsigned long FLAME_SAMPLE_INTERVAL = 50UL;
const unsigned long FLAME_CLEAR_TIME      = 1500UL;
const int FLAME_CONFIRM_SAMPLES           = 3;

// ================ DISPLAY MODES ================

const byte MODE_NORMAL = 0;
const byte MODE_SMOKE  = 1;
const byte MODE_FIRE   = 2;

byte lastDisplayMode = 255;   // forces a clean first draw

// ================ DHT22 VARIABLES ================

unsigned long lastPanelChange = 0;
unsigned long lastSensorRead = 0;

byte currentPanel = 0;

bool sensorOK = false;
bool firstReading = true;

const byte DHT_MAX_FAILS = 3;   // consecutive failed reads before "Sensor Error"
byte dhtFailCount = 0;

float temperature = 0.0;
float humidity = 0.0;
float heatIndex = 0.0;
int heatIndexTenths = 0;

// LCD line cache prevents unnecessary flicker.
char previousLine0[17] = "";
char previousLine1[17] = "";

// ================ MQ-2 VARIABLES ================

enum MQ2State {
  MQ2_WARMING_UP,
  MQ2_CALIBRATING,
  MQ2_READY
};

MQ2State mq2State = MQ2_WARMING_UP;

unsigned long mq2StartTime = 0;
unsigned long lastMQ2Read = 0;
unsigned long lastMQ2Debug = 0;
unsigned long baselineStartTime = 0;
unsigned long baselineSamples = 0;
unsigned long mq2ClearStart = 0;
unsigned long mq2ConfirmStart = 0;   // 0 = delta not currently above trigger

float baselineSum = 0.0;
float mq2Baseline = 0.0;
float mq2Filtered = 0.0;

// Fast-recovery tracking
float mq2PeakDelta = 0.0f;       // highest delta seen during the current alert
float mq2Slope = 0.0f;           // ADC counts per MQ2_SLOPE_WINDOW (+ rising, - falling)
float mq2SlopeRef = 0.0f;        // filtered value at the start of the slope window
unsigned long mq2SlopeRefTime = 0;

int mq2Raw = 0;

bool mq2FilterInitialized = false;
bool mq2Alert = false;      // true = smoke alert active

// ================ FLAME VARIABLES ================

unsigned long lastFlameRead = 0;
unsigned long flameClearStart = 0;

int flameConfirmCount = 0;

bool flameAlert = false;    // true = fire alert active
int flameIdleLevel = HIGH;  // pin level that means "no fire"

// ================ HEAT STATUS ================

const __FlashStringHelper* getHeatStatus(int hiTenths) {
  if (hiTenths > 540) return F("EXT DANG");
  if (hiTenths >= 410) return F("DANGER");
  if (hiTenths >= 320) return F("EXT CAUT");
  if (hiTenths >= 270) return F("CAUTION");
  return F("NORMAL");
}

// ================ STATUS LEDS ================

void setLeds(bool green, bool yellow, bool red) {
  digitalWrite(LED_GREEN_PIN,  green  ? HIGH : LOW);
  digitalWrite(LED_YELLOW_PIN, yellow ? HIGH : LOW);
  digitalWrite(LED_RED_PIN,    red    ? HIGH : LOW);
}

// Priority: fire > smoke > MQ-2 sensor state.
//   FIRE            = RED solid
//   SMOKE           = YELLOW solid
//   WARMING UP      = RED blinking
//   CALIBRATING     = YELLOW blinking
//   WORKING/NORMAL  = GREEN (solid, or blinking if GREEN_BLINK_WHEN_READY)
void updateLeds(unsigned long now) {
  bool blinkOn = ((now / LED_BLINK_MS) % 2UL) == 0UL;

  if (flameAlert) {
    setLeds(false, false, true);
  } else if (mq2Alert) {
    setLeds(false, true, false);
  } else if (mq2State == MQ2_WARMING_UP) {
    setLeds(false, false, blinkOn);
  } else if (mq2State == MQ2_CALIBRATING) {
    setLeds(false, blinkOn, false);
  } else {
    setLeds(GREEN_BLINK_WHEN_READY ? blinkOn : true, false, false);
  }
}

// ================ LCD LINE CONTROL ================

void printLine(byte row, const char* message) {
  char line[17];
  snprintf_P(line, sizeof(line), PSTR("%-16.16s"), message);

  char* previousLine = (row == 0) ? previousLine0 : previousLine1;

  if (strcmp(line, previousLine) != 0) {
    lcd.setCursor(0, row);
    lcd.print(line);
    strcpy(previousLine, line);
  }
}

// Same as printLine() but the text lives in flash (use with F("...")).
void printLineP(byte row, const __FlashStringHelper* message) {
  char line[17];
  snprintf_P(line, sizeof(line), PSTR("%-16.16S"), (PGM_P)message);

  char* previousLine = (row == 0) ? previousLine0 : previousLine1;

  if (strcmp(line, previousLine) != 0) {
    lcd.setCursor(0, row);
    lcd.print(line);
    strcpy(previousLine, line);
  }
}

// Wipes the LCD and the line cache so the next frame is drawn clean.
void forceRedraw() {
  lcd.clear();
  previousLine0[0] = '\0';
  previousLine1[0] = '\0';
}

// ================ DHT22 ERROR DISPLAY ================

void showError() {
  printLineP(0, F("Sensor Error"));
  printLineP(1, F("Check DHT22"));
}

// ================ READ DHT22 ================

void readSensor() {
  float newHumidity = dht.readHumidity();
  float newTemperature = dht.readTemperature();

  bool bad = isnan(newHumidity) || isnan(newTemperature) ||
             newHumidity < 0.0 || newHumidity > 100.0 ||
             newTemperature < -40.0 || newTemperature > 80.0;

  if (bad) {
    // One missed read is normal for a DHT22 (timing glitch). Keep the last
    // good values and only flag an error after DHT_MAX_FAILS in a row.
    if (dhtFailCount < 255) dhtFailCount++;
    Serial.print(F("DHT22 read failed ("));
    Serial.print(dhtFailCount);
    Serial.print(F("/"));
    Serial.print(DHT_MAX_FAILS);
    Serial.println(F(")"));
    if (dhtFailCount >= DHT_MAX_FAILS) {
      sensorOK = false;
    }
    return;
  }

  dhtFailCount = 0;

  temperature = newTemperature + TEMP_OFFSET;
  humidity = newHumidity;

  heatIndex = dht.computeHeatIndex(temperature, humidity, false);
  heatIndexTenths = (int)roundf(heatIndex * 10.0f);

  sensorOK = true;

  Serial.print(F("TEMP: "));
  Serial.print(temperature, 1);
  Serial.print(F(" C | HUMID: "));
  Serial.print(humidity, 0);
  Serial.print(F("% | HI: "));
  Serial.print(heatIndex, 1);
  Serial.print(F(" C | STATUS: "));
  Serial.println(getHeatStatus(heatIndexTenths));
}

// ================ MQ-2 STATE NAME ================

const __FlashStringHelper* getMQ2StateName() {
  switch (mq2State) {
    case MQ2_WARMING_UP: return F("WARMING UP");
    case MQ2_CALIBRATING: return F("CALIBRATING");
    case MQ2_READY: return F("MONITORING");
  }
  return F("UNKNOWN");
}

// ================ MQ-2 ADC (OVERSAMPLED) ================

// Averages MQ2_OVERSAMPLE back-to-back reads to cut Uno ADC noise.
int readMQ2Averaged() {
  long sum = 0;
  for (int i = 0; i < MQ2_OVERSAMPLE; i++) {
    sum += analogRead(MQ2_PIN);
  }
  return (int)((sum + (MQ2_OVERSAMPLE / 2)) / MQ2_OVERSAMPLE);
}

// ================ SERIAL TELEMETRY (GAYAK dashboard) ================

// Sends one machine-readable DATA record per call, throttled by
// SERIAL_INTERVAL_MS. Diagnostic LOG output is untouched.
void sendSerialData(unsigned long now) {
  // An alert change goes out immediately; everything else every interval.
  bool alertChanged = (mq2Alert != lastSentSmoke) ||
                      (flameAlert != lastSentFlame);
  if (!alertChanged && now - lastSerialSend < SERIAL_INTERVAL_MS) {
    return;
  }
  lastSerialSend = now;
  lastSentSmoke = mq2Alert;
  lastSentFlame = flameAlert;

  Serial.print(F("DATA,"));

  // Temperature
  if (sensorOK) {
    Serial.print(temperature, 1);
  } else {
    Serial.print(F("null"));
  }
  Serial.print(F(","));

  // Humidity
  if (sensorOK) {
    Serial.print(humidity, 0);
  } else {
    Serial.print(F("null"));
  }
  Serial.print(F(","));

  // Heat index (°C)
  if (sensorOK) {
    Serial.print(heatIndex, 1);
  } else {
    Serial.print(F("null"));
  }
  Serial.print(F(","));

  // Heat status (null when the DHT22 has no valid reading)
  if (sensorOK) {
    Serial.print(getHeatStatus(heatIndexTenths));
  } else {
    Serial.print(F("null"));
  }
  Serial.print(F(","));

  // MQ-2 raw ADC
  Serial.print(mq2Raw);
  Serial.print(F(","));

  // MQ-2 baseline + delta only mean something once calibration is done.
  // Before that baseline is 0 and delta would be the whole ADC value.
  if (mq2State == MQ2_READY) {
    Serial.print(mq2Baseline, 1);
    Serial.print(F(","));
    Serial.print((int)roundf(mq2Filtered - mq2Baseline));
  } else {
    Serial.print(F("null,null"));
  }
  Serial.print(F(","));

  // MQ-2 state (underscored for machine parsing)
  switch (mq2State) {
    case MQ2_WARMING_UP: Serial.print(F("WARMING_UP")); break;
    case MQ2_CALIBRATING: Serial.print(F("CALIBRATING")); break;
    case MQ2_READY: Serial.print(F("MONITORING")); break;
    default: Serial.print(F("ERROR"));
  }
  Serial.print(F(","));

  // MQ-2 alert (0 or 1)
  Serial.print(mq2Alert ? 1 : 0);
  Serial.print(F(","));

  // Fire alert (0 or 1)
  Serial.print(flameAlert ? 1 : 0);
  Serial.print(F(","));

  // Live flame pin level + learned idle level (for wiring diagnostics)
  Serial.print(digitalRead(FLAME_PIN));
  Serial.print(F(","));
  Serial.println(flameIdleLevel);
}

// ================ MQ-2 SMOKE DETECTION ================

void updateMQ2(unsigned long now) {
  if (now - lastMQ2Read < MQ2_SAMPLE_INTERVAL) {
    return;
  }

  lastMQ2Read = now;

  mq2Raw = readMQ2Averaged();

  // Asymmetric smoothing: rise slowly, fall fast.
  if (!mq2FilterInitialized) {
    mq2Filtered = mq2Raw;
    mq2FilterInitialized = true;
  } else {
    float alpha = ((float)mq2Raw < mq2Filtered)
                    ? MQ2_FALL_ALPHA
                    : MQ2_RISE_ALPHA;
    mq2Filtered += alpha * ((float)mq2Raw - mq2Filtered);
  }

  // -------- WARM-UP --------

  if (mq2State == MQ2_WARMING_UP) {
    if (now - mq2StartTime >= MQ2_WARMUP_MS) {
      mq2State = MQ2_CALIBRATING;
      baselineStartTime = now;
      baselineSum = 0.0;
      baselineSamples = 0;

      Serial.println();
      Serial.println(F("MQ-2 warm-up complete."));
      Serial.println(F("Keep sensor in smoke-free air."));
      Serial.println(F("Starting baseline calibration..."));
    }
    return;
  }

  // -------- BASELINE CALIBRATION --------

  if (mq2State == MQ2_CALIBRATING) {
    baselineSum += mq2Filtered;
    baselineSamples++;

    if (now - baselineStartTime >= MQ2_BASELINE_MS &&
        baselineSamples > 0) {
      mq2Baseline = baselineSum / (float)baselineSamples;
      mq2State = MQ2_READY;
      mq2ConfirmStart = 0;
      mq2Alert = false;

      // Seed the slope tracker and peak for fast recovery.
      mq2SlopeRef = mq2Filtered;
      mq2SlopeRefTime = now;
      mq2Slope = 0.0f;
      mq2PeakDelta = 0.0f;

      Serial.print(F("MQ-2 baseline: "));
      Serial.println(mq2Baseline, 1);
      Serial.println(F("Smoke monitoring enabled."));
    }
    return;
  }

  // -------- NORMAL MONITORING --------

  float diff = mq2Filtered - mq2Baseline;
  int delta = (int)roundf(diff);

  // 1-second slope: positive = rising, negative = falling.
  if (now - mq2SlopeRefTime >= MQ2_SLOPE_WINDOW) {
    mq2Slope = mq2Filtered - mq2SlopeRef;
    mq2SlopeRef = mq2Filtered;
    mq2SlopeRefTime = now;
  }

  // Track the alert's peak so recovery can be judged relative to it.
  if (mq2Alert && diff > mq2PeakDelta) {
    mq2PeakDelta = diff;
  }

  // Baseline tracking below the trigger level only, so real smoke is never
  // absorbed. Falls fast (post warm-up sinking), rises slowly.
  if (delta < MQ2_TRIGGER_DELTA) {
    float a;
    if (diff < 0.0f) {
      a = MQ2_BASELINE_FALL;
    } else {
      a = mq2Alert ? MQ2_BASELINE_RISE_ALERT : MQ2_BASELINE_RISE_IDLE;
    }
    mq2Baseline += a * diff;

  } else if (mq2Alert &&
             mq2Slope <= MQ2_SLOPE_TOLERANCE &&
             diff < mq2PeakDelta * MQ2_RECOVERY_FRACTION) {
    // Smoke is gone but the sensor tail is still high:
    // pull the baseline up so the alert releases quickly.
    mq2Baseline += MQ2_BASELINE_RISE_RECOVERY * diff;
  }

  if (!mq2Alert) {

    // Smoke = delta held above the trigger for MQ2_CONFIRM_MS without a dip.
    if (delta >= MQ2_TRIGGER_DELTA) {

      if (mq2ConfirmStart == 0) {
        mq2ConfirmStart = now;
      }

      if (now - mq2ConfirmStart >= MQ2_CONFIRM_MS) {
        mq2Alert = true;
        mq2ClearStart = 0;
        mq2ConfirmStart = 0;
        mq2PeakDelta = diff;

        Serial.println();
        Serial.println(F("!!! SMOKE ALERT !!!"));
        Serial.print(F("  RAW="));
        Serial.print(mq2Raw);
        Serial.print(F(" FILTERED="));
        Serial.print(mq2Filtered, 1);
        Serial.print(F(" BASE="));
        Serial.print(mq2Baseline, 1);
        Serial.print(F(" DELTA="));
        Serial.println(delta);
      }
    } else {
      mq2ConfirmStart = 0;
    }

  } else {

    // Clear once readings stay low for MQ2_CLEAR_TIME.
    if (delta <= MQ2_CLEAR_DELTA) {

      if (mq2ClearStart == 0) {
        mq2ClearStart = now;
      }

      if (now - mq2ClearStart >= MQ2_CLEAR_TIME) {
        mq2Alert = false;
        mq2ConfirmStart = 0;
        mq2ClearStart = 0;
        mq2PeakDelta = 0.0f;

        Serial.println(F("Smoke alert cleared. Back to normal."));
      }

    } else {
      mq2ClearStart = 0;
    }
  }
}

// ================ FLAME DETECTION ================

void learnFlameIdle() {
  if (!FLAME_AUTO_LEARN) {
    flameIdleLevel = FLAME_ACTIVE_LOW ? HIGH : LOW;
  } else {
    int lowCount = 0;

    // 60 samples x 50 ms = 3 s of sampling, majority vote.
    for (int i = 0; i < 60; i++) {
      if (digitalRead(FLAME_PIN) == LOW) lowCount++;
      delay(50);
    }

    if (lowCount >= 10 && lowCount <= 50) {
      // Pin was flickering during learning (IR light in view, or a loose
      // wire): do not trust it, use the configured polarity instead.
      flameIdleLevel = FLAME_ACTIVE_LOW ? HIGH : LOW;
      Serial.println(F("WARNING: flame pin unstable while learning."));
      Serial.println(F("Using FLAME_ACTIVE_LOW polarity. Shield sensor from sun/lamps."));
    } else {
      flameIdleLevel = (lowCount > 30) ? LOW : HIGH;
    }
  }

  Serial.print(F("Flame idle level: "));
  Serial.println(flameIdleLevel == LOW ? F("LOW") : F("HIGH"));
  Serial.println(flameIdleLevel == LOW
                   ? F("Module idles LOW. Fire = HIGH.")
                   : F("Module idles HIGH. Fire = LOW."));
}

bool flameDetectedNow() {
  return digitalRead(FLAME_PIN) != flameIdleLevel;
}

void updateFlame(unsigned long now) {
  if (now - lastFlameRead < FLAME_SAMPLE_INTERVAL) {
    return;
  }

  lastFlameRead = now;

  bool detected = flameDetectedNow();

  if (!flameAlert) {

    // Several consecutive hits required, so one stray pulse is ignored.
    if (detected) {
      flameConfirmCount++;

      if (flameConfirmCount >= FLAME_CONFIRM_SAMPLES) {
        flameAlert = true;
        flameClearStart = 0;

        Serial.println();
        Serial.println(F("!!! FIRE ALERT !!!"));
      }
    } else {
      flameConfirmCount = 0;
    }

  } else {

    // Clear once the flame stays gone for FLAME_CLEAR_TIME.
    if (!detected) {

      if (flameClearStart == 0) {
        flameClearStart = now;
      }

      if (now - flameClearStart >= FLAME_CLEAR_TIME) {
        flameAlert = false;
        flameConfirmCount = 0;
        flameClearStart = 0;

        Serial.println(F("Fire alert cleared. Back to normal."));
      }

    } else {
      flameClearStart = 0;
    }
  }
}

// ================ FREE SRAM ================

// Bytes between heap and stack. If this drops below ~200 the sketch is
// close to corrupting memory (random DHT22 / LCD faults).
int freeRam() {
  extern int __heap_start, *__brkval;
  int v;
  return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}

// ================ SERIAL DIAGNOSTICS ================

void printDiagnostics(unsigned long now) {
  if (now - lastMQ2Debug < MQ2_DEBUG_INTERVAL) {
    return;
  }

  lastMQ2Debug = now;

  Serial.print(F("MQ2 RAW="));
  Serial.print(mq2Raw);
  Serial.print(F(" FILTERED="));
  Serial.print(mq2Filtered, 1);
  Serial.print(F(" STATE="));
  Serial.print(getMQ2StateName());

  if (mq2State == MQ2_READY) {
    int delta = (int)roundf(mq2Filtered - mq2Baseline);

    Serial.print(F(" BASE="));
    Serial.print(mq2Baseline, 1);
    Serial.print(F(" DELTA="));
    Serial.print(delta);
    Serial.print(F(" SLOPE="));
    Serial.print(mq2Slope, 1);
    Serial.print(F(" PEAK="));
    Serial.print(mq2PeakDelta, 1);
    Serial.print(F(" SMOKE="));
    Serial.print(mq2Alert ? F("YES") : F("NO"));
  }

  Serial.print(F(" | FLAME_PIN="));
  Serial.print(digitalRead(FLAME_PIN));
  Serial.print(F(" IDLE="));
  Serial.print(flameIdleLevel);
  Serial.print(F(" FIRE="));
  Serial.print(flameAlert ? F("YES") : F("NO"));
  Serial.print(F(" FREE_RAM="));
  Serial.print(freeRam());

  Serial.println();
}

// ================ LCD DISPLAY ================

void showPanel() {
  // Fire outranks smoke, smoke outranks normal.
  byte mode = MODE_NORMAL;
  if (flameAlert)      mode = MODE_FIRE;
  else if (mq2Alert)   mode = MODE_SMOKE;

  // Clean slate whenever the display mode changes.
  if (mode != lastDisplayMode) {
    forceRedraw();
    lastDisplayMode = mode;
  }

  if (mode == MODE_FIRE) {
    printLineP(0, F("FIRE ALERT!"));
    printLineP(1, mq2Alert ? F("FIRE+SMOKE! RUN!") : F("EVACUATE NOW!"));
    return;
  }

  if (mode == MODE_SMOKE) {
    printLineP(0, F("SMOKE ALERT!"));
    printLineP(1, F("EVACUATE NOW!"));
    return;
  }

  // Smoke sensor status panel (only while the MQ-2 is not ready).
  if (currentPanel == 2 && mq2State != MQ2_READY) {
    char line1[17];

    printLineP(0, F("SMOKE SENSOR"));

    if (mq2State == MQ2_WARMING_UP) {
      unsigned long elapsed = millis() - mq2StartTime;
      unsigned long left = (elapsed >= MQ2_WARMUP_MS)
                             ? 0UL
                             : (MQ2_WARMUP_MS - elapsed) / 1000UL;
      snprintf_P(line1, sizeof(line1), PSTR("WARMUP %us"), (unsigned int)left);
    } else {
      snprintf_P(line1, sizeof(line1), PSTR("CALIBRATING..."));
    }

    printLine(1, line1);
    return;
  }

  if (!sensorOK) {
    showError();
    return;
  }

  char line0[17];
  char line1[17];
  char tempText[12];
  char humidText[8];
  char hiText[12];

  // Convert decimals to strings for Arduino Uno.
  dtostrf(temperature, 4, 1, tempText);
  dtostrf(heatIndex, 4, 1, hiText);

  snprintf_P(humidText, sizeof(humidText), PSTR("%u"),
           (unsigned int)roundf(humidity));

  if (currentPanel == 0) {
    // PANEL 1: TEMPERATURE + HUMIDITY
    snprintf_P(line0, sizeof(line0), PSTR("Temp: %sC"), tempText);
    snprintf_P(line1, sizeof(line1), PSTR("Humid: %s%%"), humidText);
  } else {
    // PANEL 2: HEAT INDEX + STATUS
    snprintf_P(line0, sizeof(line0), PSTR("HI: %sC"), hiText);
    snprintf_P(line1, sizeof(line1), PSTR("STATUS: %S"),
               (PGM_P)getHeatStatus(heatIndexTenths));
  }

  printLine(0, line0);
  printLine(1, line1);
}

// ================ SETUP ================

void setup() {
  Serial.begin(SERIAL_BAUD);

  pinMode(MQ2_PIN, INPUT);
  pinMode(FLAME_PIN, INPUT);

  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_YELLOW_PIN, OUTPUT);
  pinMode(LED_RED_PIN, OUTPUT);

  // Lamp test: all three LEDs on during the splash screen.
  setLeds(true, true, true);

  dht.begin();

  lcd.init();
  lcd.backlight();
  lcd.clear();

  printLineP(0, F("GAYAK MONITOR"));
  printLineP(1, F("INITIALIZING..."));

  delay(2000);

  setLeds(false, false, false);

  // Learn the flame module's resting level (no flame in view).
  printLineP(0, F("FLAME LEARNING"));
  printLineP(1, F("KEEP FLAME AWAY"));
  learnFlameIdle();

  currentPanel = 0;

  unsigned long now = millis();

  lastPanelChange = now;
  lastSensorRead = 0;

  mq2StartTime = now;
  lastMQ2Read = now;
  lastMQ2Debug = now;
  lastFlameRead = now;
  lastSerialSend = now;
}

// ================ MAIN LOOP ================

void loop() {
  unsigned long now = millis();

  // Poll both alarm sensors first so neither is ever starved.
  updateFlame(now);
  updateMQ2(now);

  // Read DHT22 immediately, then every 2.5 seconds.
  if (firstReading || now - lastSensorRead >= SENSOR_INTERVAL) {
    firstReading = false;
    lastSensorRead = now;
    readSensor();            // blocks ~250 ms worst case
  }

  now = millis();
  updateFlame(now);          // catch up right after the DHT22 block
  updateMQ2(now);
  printDiagnostics(now);
  updateLeds(now);

  // Rotate LCD panels every 3 seconds.
  // 3 panels while the MQ-2 is warming up, 2 once it is ready.
  if (now - lastPanelChange >= PANEL_INTERVAL) {
    lastPanelChange = now;

    byte panelCount = (mq2State == MQ2_READY) ? 2 : 3;
    currentPanel++;
    if (currentPanel >= panelCount) {
      currentPanel = 0;
    }
  }

  showPanel();

  // Serial telemetry for the GAYAK dashboard.
  sendSerialData(now);
}
