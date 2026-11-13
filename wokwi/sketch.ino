// Wokwi GAYAK: sensor controls plus D11-to-GND LCD panel button; no faux power switch.
// Photoresistor module on D3 is a visible-light proxy, NOT a real flame sensor.
// LEDs: D5 GREEN, D6 YELLOW, D7 RED; buzzers: D8 SMOKE, D9 QUAKE, D10 FIRE.
// Wokwi piezos use tone(); the real two-terminal active buzzers use DC drivers.
#define GAYAK_WOKWI_SIM 1

/*
 GAYAK - Complete Arduino Uno R3 firmware and three active buzzers

 WOKWI BUZZER PROXY: direct D8/D9/D10 -> piezo(+) and piezo(-) -> GND.
 Wokwi piezo buzzers are simulated, with no series 1k ohm resistors.
 Physical active buzzers need rated switching drivers for safe full-volume operation.

 Pin assignments for the Wokwi simulation:
 D2 DHT22, D3 flame DO, D5 green LED, D6 yellow LED, D7 red LED,
 D11 LCD panel button, A0 MQ-2 analog, A4 SDA (LCD + GY-521), A5 SCL (LCD + GY-521).
 Serial baud: 115200.

 Smoke: one short D8 pulse each second.
 Fire: three short D10 pulses followed by a pause.
 Shaking: D9 and D8 alternate, followed by a short pause.
 Fire + smoke: two fast D10 pulses then a longer D8 pulse.
 Alarm priority: FIRE + SMOKE > FIRE > SMOKE > SHAKING.

 NOTE: This is a laboratory prototype, not a certified life-safety or
 earthquake warning system. MPU6050 detects motion, not earthquake origin.
*/

// GAYAK Early Warning System - MPU-6050 + alternating active buzzers
// Arduino Uno R3. LED pins corrected: D5=GREEN, D6=YELLOW, D7=RED.
// Prototype intense shaking/acceleration detection, not a certified seismic alarm.
// LCD (0x27) + GY-521 (0x68) share SDA=A4, SCL=A5.
// Validate voltage levels: MPU6050 chip uses 3.3V I2C logic.
// Keep DATA CSV intact; supplemental QUAKE CSV at 1 Hz.

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
#define PANEL_BUTTON_PIN 11

// GY-521 MPU-6050; shared I2C with LCD on A4/A5.
#define MPU_ADDR 0x68

// LEDs: D5 Green, D6 Yellow, D7 Red; each via 220-ohm series resistor to LED anode, cathode to GND.
#define LED_RED_PIN    7
#define LED_YELLOW_PIN 6
#define LED_GREEN_PIN  5

// Wokwi three piezo buzzer proxies: no series resistors; use tone()/noTone().
// Physical active buzzers must use a properly rated driver circuit.
#define BUZZER_SMOKE_PIN 8    // 5V active buzzer #1
#define BUZZER_QUAKE_PIN 9    // 5V active buzzer #2
#define BUZZER_FIRE_PIN  10   // SFM-27 (3-24V) active buzzer

// ================ COMPONENTS ================

DHT dht(DHT_PIN, DHT_TYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ================ SETTINGS ================

const unsigned long SENSOR_INTERVAL = 2200UL; // DHT22 minimum 2 seconds
const unsigned long PANEL_BUTTON_DEBOUNCE_MS = 40UL;

// ================ MPU-6050 ACCELERATION / SHAKING SETTINGS ================
// Experimental local motion levels; NOT earthquake magnitude or official intensity.
// Secure the MPU firmly to the structure being monitored.
const unsigned long MPU_SAMPLE_MS       = 20UL;    // target 50 Hz
const unsigned long MPU_RETRY_MS        = 5000UL;
const unsigned long MPU_SETTLE_MS       = 2000UL;  // keep still after startup
const unsigned long QUAKE_CONFIRM_MS    = 500UL;   // sustained vibration
const unsigned long QUAKE_CLEAR_MS      = 1200UL;  // must be quiet to clear
const unsigned long QUAKE_MIN_HOLD_MS   = 5000UL;  // minimum alert duration
const unsigned long QUAKE_SERIAL_MS     = 400UL;

const float MOTION_NOTICE_RMS_G = 0.04f;   // unconfirmed movement indication
const float QUAKE_TRIGGER_RMS_G = 0.12f;  // sustained STRONG shaking threshold
const float INTENSE_SHAKING_RMS_G = 0.30f; // intense filtered acceleration (not Richter)
const float QUAKE_CLEAR_RMS_G   = 0.06f;
const float MPU_GRAVITY_ALPHA   = 0.005f; // slow-moving gravity baseline
const float MPU_ENERGY_ALPHA    = 0.08f;  // RMS-like vibration smoother

// Change only after comparing with a reliable thermometer.
const float TEMP_OFFSET = 0.0;

// LED behaviour
const unsigned long LED_BLINK_MS = 500UL;   // blink half-period (warm-up / calibrating)
const bool GREEN_BLINK_WHEN_READY = false;  // true = green blinks, false = green solid

// ================ DISTINCT BUZZER ALARMS ================
// Rhythm patterns work with the Wokwi piezo proxies and physical active buzzers.
enum BuzzerAlarm { BUZZER_IDLE, BUZZER_SMOKE, BUZZER_QUAKE, BUZZER_FIRE, BUZZER_FIRE_SMOKE, BUZZER_HEAT };
struct BuzzerStep { byte pin; unsigned int durationMs; }; // pin 0 = silence

// Smoke: one short pulse each second on D8.
const BuzzerStep SMOKE_PATTERN[] = {
  { BUZZER_SMOKE_PIN, 180 }, { 0, 820 }
};
// Fire: three short pulses on the fire buzzer, then a clear pause.
const BuzzerStep FIRE_PATTERN[] = {
  { BUZZER_FIRE_PIN, 180 }, { 0, 120 },
  { BUZZER_FIRE_PIN, 180 }, { 0, 120 },
  { BUZZER_FIRE_PIN, 180 }, { 0, 900 }
};
// Shaking: alternate buzzers with a short break between each pair.
const BuzzerStep QUAKE_PATTERN[] = {
  { BUZZER_QUAKE_PIN, 250 }, { BUZZER_SMOKE_PIN, 250 }, { 0, 100 }
};
// Fire + smoke: two fast fire pulses followed by one longer smoke-buzzer pulse.
const BuzzerStep FIRE_SMOKE_PATTERN[] = {
  { BUZZER_FIRE_PIN, 120 }, { 0, 100 },
  { BUZZER_FIRE_PIN, 120 }, { 0, 100 },
  { BUZZER_SMOKE_PIN, 500 }, { 0, 800 }
};
const BuzzerStep HEAT_PATTERN[] = {
  { BUZZER_QUAKE_PIN, 500 }, { 0, 250 },
  { BUZZER_QUAKE_PIN, 500 }, { 0, 1000 }
};

BuzzerAlarm buzzerMode = BUZZER_IDLE;
byte buzzerStep = 0;
unsigned long buzzerPhaseStarted = 0;

// MQ-2 smoke detector timing
const unsigned long MQ2_WARMUP_MS       = 5000UL;  // was 120000UL (longer settle = steadier baseline)
const unsigned long MQ2_BASELINE_MS     = 3000UL;   // was 10000UL (more samples averaged)
const unsigned long MQ2_SAMPLE_INTERVAL = 100UL;
const unsigned long MQ2_CLEAR_TIME      = 800UL;
const unsigned long MQ2_DEBUG_INTERVAL  = 1000UL;

// False-alarm protection
const int MQ2_OVERSAMPLE = 16;                   // ADC reads averaged per sample
const unsigned long MQ2_CONFIRM_MS = 750UL;     // delta must stay high this long (was 3 samples = 0.3 s)

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
bool lastSentHeatAcknowledged = false;

// ================ MPU-6050 STATE ================
// Local filtered acceleration in g; not a seismic intensity classification.
enum MotionLevel { MOTION_STABLE, MOTION_NOTICE, MOTION_STRONG, MOTION_INTENSE };

bool mpuReady = false;
bool quakeAlert = false;
bool quakeCandidate = false;
byte mpuConsecutiveFails = 0;
unsigned long lastMPURead = 0;
unsigned long lastMPURetry = 0;
unsigned long mpuStartTime = 0;
unsigned long quakeCandidateSince = 0;
unsigned long quakeActivatedAt = 0;
unsigned long quakeClearSince = 0;
unsigned long lastQuakeSerial = 0;
bool lastSentQuake = false;

float gravityX = 0.0f, gravityY = 0.0f, gravityZ = 1.0f;
float quakeEnergy = 0.0f;
float quakeRmsG = 0.0f;
float quakePeakG = 0.0f;

// Flame sensor settings
// Auto-learn reads the pin at boot (no flame in view) and treats that
// level as "no fire". Fire = the pin leaving that level.
const bool FLAME_AUTO_LEARN = false; // Wokwi photoresistor DO LOW = bright/flame proxy

// Used only when FLAME_AUTO_LEARN is false.
// true = OUT goes LOW on fire, false = OUT goes HIGH on fire.
const bool FLAME_ACTIVE_LOW = true;

const unsigned long FLAME_SAMPLE_INTERVAL = 50UL;
const unsigned long FLAME_CLEAR_TIME      = 500UL;
const int FLAME_CONFIRM_SAMPLES           = 2;

// ================ DISPLAY MODES ================

const byte MODE_NORMAL = 0;
const byte MODE_SMOKE  = 1;
const byte MODE_FIRE   = 2;
const byte MODE_QUAKE  = 3;
const byte MODE_HEAT   = 4;

byte lastDisplayMode = 255;   // forces a clean first draw

// ================ DHT22 VARIABLES ================

unsigned long lastSensorRead = 0;

byte currentPanel = 0;
bool panelButtonLastReading = HIGH;
bool panelButtonStableState = HIGH;
unsigned long panelButtonLastChange = 0;

bool sensorOK = false;
bool firstReading = true;

const byte DHT_MAX_FAILS = 3;   // consecutive failed reads before "Sensor Error"
byte dhtFailCount = 0;

float temperature = 0.0;
float humidity = 0.0;
float heatIndex = 0.0;
int heatIndexTenths = 0;
bool heatWarningActive = false;
bool heatAlertAcknowledged = false;

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

// ================ MPU-6050 ACCELERATION / INTENSE SHAKING ================

bool writeMPURegister(byte reg, byte value) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  Wire.write(value);
  return Wire.endTransmission() == 0;
}

bool readMPUAcceleration(float &x, float &y, float &z) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);                         // ACCEL_XOUT_H
  if (Wire.endTransmission(false) != 0) return false;

  // Only read accelerometer bytes: faster than fetching gyro + temperature.
  if (Wire.requestFrom((uint8_t)MPU_ADDR, (uint8_t)6,
                       (uint8_t)true) != 6) return false;

  int16_t rawX = (int16_t)((Wire.read() << 8) | Wire.read());
  int16_t rawY = (int16_t)((Wire.read() << 8) | Wire.read());
  int16_t rawZ = (int16_t)((Wire.read() << 8) | Wire.read());

  // ACCEL_CONFIG = 0 means +/-2 g, sensitivity 16384 LSB/g.
  x = rawX / 16384.0f;
  y = rawY / 16384.0f;
  z = rawZ / 16384.0f;
  return true;
}

void initializeMPU(unsigned long now) {
  lastMPURetry = now;
  mpuReady = false;

  // Verify that a device is present at 0x68 and is an MPU-6050.
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x75);                         // WHO_AM_I
  if (Wire.endTransmission(false) != 0 ||
      Wire.requestFrom((uint8_t)MPU_ADDR, (uint8_t)1,
                       (uint8_t)true) != 1) {
    Serial.println(F("MPU6050 OFFLINE: check I2C wires"));
    return;
  }
  byte who = Wire.read();
  if ((who & 0x7E) != 0x68) {
    Serial.print(F("Unexpected MPU WHO_AM_I: 0x"));
    Serial.println(who, HEX);
    return;
  }

  // Wake sensor, 44 Hz low-pass, 50 Hz sample-rate, +/-2 g range.
  if (!writeMPURegister(0x6B, 0x01) ||   // PWR_MGMT_1
      !writeMPURegister(0x1A, 0x03) ||   // CONFIG DLPF
      !writeMPURegister(0x19, 19) ||     // SMPLRT_DIV
      !writeMPURegister(0x1C, 0x00)) {  // ACCEL_CONFIG
    Serial.println(F("MPU6050 setup failed"));
    return;
  }

  float ax, ay, az;
  if (!readMPUAcceleration(ax, ay, az)) {
    Serial.println(F("MPU6050 initial read failed"));
    return;
  }
  gravityX = ax;
  gravityY = ay;
  gravityZ = az;
  quakeEnergy = 0.0f;
  quakeRmsG = 0.0f;
  quakePeakG = 0.0f;
  quakeAlert = false;
  quakeCandidate = false;
  quakeClearSince = 0;
  mpuConsecutiveFails = 0;
  mpuStartTime = now;
  lastMPURead = now;
  mpuReady = true;
  Serial.println(F("MPU6050 READY - keep sensor stationary for 2 s"));
}

void updateMPU(unsigned long now) {
  if (!mpuReady) {
    if (now - lastMPURetry >= MPU_RETRY_MS) initializeMPU(now);
    return;                              // Other GAYAK sensors keep working
  }
  if (now - lastMPURead < MPU_SAMPLE_MS) return;
  lastMPURead = now;

  float ax, ay, az;
  if (!readMPUAcceleration(ax, ay, az)) {
    if (++mpuConsecutiveFails >= 3) {
      mpuReady = false;
      quakeAlert = false;                // no false "safe" indication on LCD
      Serial.println(F("MPU6050 LOST - acceleration monitor OFFLINE"));
      lastMPURetry = now;
    }
    return;
  }
  mpuConsecutiveFails = 0;

  // Remove slowly varying gravitational acceleration on each axis.
  float dx = ax - gravityX;
  float dy = ay - gravityY;
  float dz = az - gravityZ;
  gravityX += MPU_GRAVITY_ALPHA * dx;
  gravityY += MPU_GRAVITY_ALPHA * dy;
  gravityZ += MPU_GRAVITY_ALPHA * dz;

  float motionSquared = dx * dx + dy * dy + dz * dz;
  quakeEnergy += MPU_ENERGY_ALPHA * (motionSquared - quakeEnergy);
  if (quakeEnergy < 0.0f) quakeEnergy = 0.0f;
  quakeRmsG = sqrtf(quakeEnergy);
  if (quakeRmsG > quakePeakG) quakePeakG = quakeRmsG;

  // Let the baseline settle before enabling alerts.
  if (now - mpuStartTime < MPU_SETTLE_MS) return;

  if (!quakeAlert) {
    if (quakeRmsG >= QUAKE_TRIGGER_RMS_G) {
      if (!quakeCandidate) {
        quakeCandidate = true;
        quakeCandidateSince = now;
      } else if (now - quakeCandidateSince >= QUAKE_CONFIRM_MS) {
        quakeAlert = true;
        quakeCandidate = false;
        quakeActivatedAt = now;
        quakeClearSince = 0;
        Serial.println(F("!!! SUSTAINED SHAKING DETECTED !!!"));
      }
    } else {
      quakeCandidate = false;
    }
  } else {
    if (quakeRmsG <= QUAKE_CLEAR_RMS_G) {
      if (quakeClearSince == 0) quakeClearSince = now;
      if (now - quakeClearSince >= QUAKE_CLEAR_MS &&
          now - quakeActivatedAt >= QUAKE_MIN_HOLD_MS) {
        quakeAlert = false;
        quakeClearSince = 0;
        Serial.println(F("Shaking stopped - returning to monitoring"));
      }
    } else {
      quakeClearSince = 0;
    }
  }
}

// Derived local motion levels; alerts still require sustained shaking.
MotionLevel classifyMotion() {
  if (!mpuReady || millis() - mpuStartTime < MPU_SETTLE_MS) return MOTION_STABLE;
  if (quakeRmsG >= INTENSE_SHAKING_RMS_G) return MOTION_INTENSE;
  if (quakeRmsG >= QUAKE_TRIGGER_RMS_G) return MOTION_STRONG;
  if (quakeRmsG >= MOTION_NOTICE_RMS_G) return MOTION_NOTICE;
  return MOTION_STABLE;
}

const char* motionLevelName(MotionLevel level) {
  switch (level) {
    case MOTION_INTENSE: return "INTENSE";
    case MOTION_STRONG:  return "STRONG";
    case MOTION_NOTICE:  return "MOTION";
    default:             return "STABLE";
  }
}

// A separate record preserves the original dashboard DATA CSV field count.
// QUAKE,<online:0/1>,<alert:0/1>,<filtered acceleration RMS g>,<level>
// The optional fifth field is ignored by older readers.
void sendQuakeData(unsigned long now) {
  bool changed = quakeAlert != lastSentQuake;
  if (!changed && now - lastQuakeSerial < QUAKE_SERIAL_MS) return;
  lastQuakeSerial = now;
  lastSentQuake = quakeAlert;
  Serial.print(F("QUAKE,"));
  Serial.print(mpuReady ? 1 : 0);
  Serial.print(F(","));
  Serial.print(quakeAlert ? 1 : 0);
  Serial.print(F(","));
  if (mpuReady) {
    Serial.print(quakeRmsG, 3);
    Serial.print(F(","));
    Serial.println(motionLevelName(classifyMotion()));
  } else {
    Serial.println(F("null,null"));
  }
}

// ================ HEAT STATUS ================

const __FlashStringHelper* getHeatStatus(int hiTenths) {
  if (hiTenths >= 520) return F("EXT DANG");
  if (hiTenths >= 410) return F("DANGER");
  if (hiTenths >= 330) return F("EXT CAUT");
  if (hiTenths >= 270) return F("CAUTION");
  return F("NORMAL");
}

// ================ STATUS LEDS ================

void setLeds(bool green, bool yellow, bool red) {
  digitalWrite(LED_GREEN_PIN,  green  ? HIGH : LOW);
  digitalWrite(LED_YELLOW_PIN, yellow ? HIGH : LOW);
  digitalWrite(LED_RED_PIN,    red    ? HIGH : LOW);
}

// Calibration keeps red/yellow steady; after calibration, valid HI drives LEDs.
//   FIRE            = RED solid
//   SMOKE           = YELLOW solid
//   EXTREME DANGER / EXTREME CAUTION = YELLOW + RED
void updateLeds(unsigned long now) {
  if (flameAlert) {
    setLeds(false, false, true);
  } else if (mq2Alert) {
    setLeds(false, true, false);
  } else if (quakeAlert) {
    // Distinctive fast red blink for vibration/shaking.
    setLeds(false, false, ((now / 250UL) % 2UL) == 0UL);
  } else if (mq2State == MQ2_WARMING_UP) {
    setLeds(false, false, true);
  } else if (mq2State == MQ2_CALIBRATING) {
    setLeds(false, true, false);
  } else if (sensorOK && heatIndexTenths >= 520) {
    setLeds(false, true, true);
  } else if (sensorOK && heatIndexTenths >= 410) {
    setLeds(false, false, true);
  } else if (sensorOK && heatIndexTenths >= 330) {
    setLeds(false, true, true);
  } else if (sensorOK && heatIndexTenths >= 270) {
    setLeds(false, true, false);
  } else if (!mpuReady || !sensorOK) {
    // Do not show a green all-clear while the quake sensor is unavailable.
    setLeds(false, true, false);
  } else {
    setLeds(GREEN_BLINK_WHEN_READY ? blinkOn : true, false, false);
  }
}

// ================ ACTIVE BUZZER ALTERNATION ================

void allBuzzersOff() {
#if GAYAK_WOKWI_SIM
  noTone(BUZZER_SMOKE_PIN);
  noTone(BUZZER_QUAKE_PIN);
  noTone(BUZZER_FIRE_PIN);
#endif
  digitalWrite(BUZZER_SMOKE_PIN, LOW);
  digitalWrite(BUZZER_QUAKE_PIN, LOW);
  digitalWrite(BUZZER_FIRE_PIN, LOW);
}

void soundBuzzer(byte pin) {
#if GAYAK_WOKWI_SIM
  // Wokwi's piezo proxy needs a square wave. Only one buzzer active at once.
  unsigned int hz = pin == BUZZER_FIRE_PIN ? 950 :
                    pin == BUZZER_SMOKE_PIN ? 750 : 1550;
  tone(pin, hz);
#else
  // Actual 2-pin ACTIVE buzzers use DC HIGH through a properly rated driver.
  digitalWrite(pin, HIGH);
#endif
}

const BuzzerStep* getBuzzerPattern(BuzzerAlarm mode, byte &length) {
  switch (mode) {
    case BUZZER_SMOKE:
      length = sizeof(SMOKE_PATTERN) / sizeof(SMOKE_PATTERN[0]);
      return SMOKE_PATTERN;
    case BUZZER_QUAKE:
      length = sizeof(QUAKE_PATTERN) / sizeof(QUAKE_PATTERN[0]);
      return QUAKE_PATTERN;
    case BUZZER_FIRE:
      length = sizeof(FIRE_PATTERN) / sizeof(FIRE_PATTERN[0]);
      return FIRE_PATTERN;
    case BUZZER_FIRE_SMOKE:
      length = sizeof(FIRE_SMOKE_PATTERN) / sizeof(FIRE_SMOKE_PATTERN[0]);
      return FIRE_SMOKE_PATTERN;
    case BUZZER_HEAT:
      length = sizeof(HEAT_PATTERN) / sizeof(HEAT_PATTERN[0]);
      return HEAT_PATTERN;
    default:
      length = 0;
      return NULL;
  }
}

void beginBuzzerStep(unsigned long now) {
  byte length = 0;
  const BuzzerStep* pattern = getBuzzerPattern(buzzerMode, length);
  if (length == 0 || pattern == NULL) {
    allBuzzersOff();
    return;
  }

  buzzerStep %= length;
  allBuzzersOff();
  if (pattern[buzzerStep].pin != 0) {
    soundBuzzer(pattern[buzzerStep].pin);
  }
  buzzerPhaseStarted = now;
}

// Call every loop; never use delay(), so all sensors keep updating.
// Priority: FIRE + SMOKE > FIRE > SMOKE > EXTREME HEAT > SHAKING > QUIET.
void updateBuzzers(unsigned long now) {
  BuzzerAlarm requested = BUZZER_IDLE;
  if (flameAlert && mq2Alert) requested = BUZZER_FIRE_SMOKE;
  else if (flameAlert) requested = BUZZER_FIRE;
  else if (mq2Alert)   requested = BUZZER_SMOKE;
  else if (heatWarningActive && heatIndexTenths >= 520 && !heatAlertAcknowledged) requested = BUZZER_HEAT;
  else if (quakeAlert) requested = BUZZER_QUAKE;

  // Start the appropriate pattern immediately when alerts change.
  if (requested != buzzerMode) {
    allBuzzersOff();
    buzzerMode = requested;
    buzzerStep = 0;
    if (requested != BUZZER_IDLE) {
      beginBuzzerStep(now);
    }
    return;
  }
  if (buzzerMode == BUZZER_IDLE) return;

  byte length = 0;
  const BuzzerStep* pattern = getBuzzerPattern(buzzerMode, length);
  if (length == 0 || pattern == NULL) return;
  if (now - buzzerPhaseStarted < pattern[buzzerStep].durationMs) return;

  buzzerStep = (buzzerStep + 1) % length;
  beginBuzzerStep(now);
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
  heatWarningActive = heatIndexTenths >= 410;
  if (!heatWarningActive) heatAlertAcknowledged = false;

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
                      (flameAlert != lastSentFlame) ||
                      (heatAlertAcknowledged != lastSentHeatAcknowledged);
  if (!alertChanged && now - lastSerialSend < SERIAL_INTERVAL_MS) {
    return;
  }
  lastSerialSend = now;
  lastSentSmoke = mq2Alert;
  lastSentFlame = flameAlert;
  lastSentHeatAcknowledged = heatAlertAcknowledged;

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
  Serial.print(flameIdleLevel);
  Serial.print(F(","));
  Serial.print(heatAlertAcknowledged ? 1 : 0);
  Serial.println();
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

  // Digital output from Wokwi adjustable-light proxy at D3 (active LOW).
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
  Serial.print(F(" MPU="));
  Serial.print(mpuReady ? F("READY") : F("OFFLINE"));
  Serial.print(F(" VIB_G="));
  if (mpuReady) Serial.print(quakeRmsG, 3);
  else Serial.print(F("null"));
  Serial.print(F(" PEAK_G="));
  if (mpuReady) Serial.print(quakePeakG, 3);
  else Serial.print(F("null"));
  quakePeakG = 0.0f; // peak since the previous 1-second diagnostic update
  Serial.print(F(" SHAKE="));
  Serial.print(quakeAlert ? F("YES") : F("NO"));
  Serial.print(F(" FREE_RAM="));
  Serial.print(freeRam());

  Serial.println();
}

// ================ LCD DISPLAY ================

void updatePanelButton(unsigned long now) {
  byte panelCount = (mq2State == MQ2_READY) ? 3 : 4;
  if (currentPanel >= panelCount) currentPanel = 0;

  bool reading = digitalRead(PANEL_BUTTON_PIN);
  if (reading != panelButtonLastReading) {
    panelButtonLastReading = reading;
    panelButtonLastChange = now;
  }

  if (now - panelButtonLastChange >= PANEL_BUTTON_DEBOUNCE_MS &&
      reading != panelButtonStableState) {
    panelButtonStableState = reading;
    if (panelButtonStableState == LOW) {
      if (heatWarningActive && !heatAlertAcknowledged) {
        heatAlertAcknowledged = true;
      } else {
        currentPanel = (currentPanel + 1) % panelCount;
      }
    }
  }
}

void showPanel() {
  // Fire and smoke outrank heat; an unacknowledged HI warning outranks shaking.
  byte mode = MODE_NORMAL;
  if (flameAlert)      mode = MODE_FIRE;
  else if (mq2Alert)   mode = MODE_SMOKE;
  else if (heatWarningActive && !heatAlertAcknowledged) mode = MODE_HEAT;
  else if (quakeAlert) mode = MODE_QUAKE;

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

  if (mode == MODE_HEAT) {
    printLineP(0, heatIndexTenths >= 520 ? F("HI EXT DANGER") : F("HI DANGER"));
    char heatValue[17];
    char hiText[8];
    dtostrf(heatIndex, 1, 1, hiText);
    snprintf_P(heatValue, sizeof(heatValue), PSTR("%s C  D11=Ack"), hiText);
    printLine(1, heatValue);
    return;
  }

  if (mode == MODE_QUAKE) {
    printLineP(0, classifyMotion() == MOTION_INTENSE
                  ? F("INTENSE SHAKING") : F("STRONG SHAKING"));
    printLineP(1, F("DROP COVER HOLD"));
    return;
  }

  // MPU monitor panel, shown even when the DHT22 has a read error.
  if (currentPanel == 2) {
    if (!mpuReady) {
      printLineP(0, F("MPU6050 OFFLINE"));
      printLineP(1, F("CHECK I2C WIRES"));
    } else if (millis() - mpuStartTime < MPU_SETTLE_MS) {
      printLineP(0, F("MPU CALIBRATING"));
      printLineP(1, F("KEEP SENSOR STILL"));
    } else {
      char vibration[10];
      char line[17];
      dtostrf(quakeRmsG, 4, 2, vibration);
      printLineP(0, F("MPU ACCELERATION"));
      snprintf_P(line, sizeof(line), PSTR("%s %sg"),
                 motionLevelName(classifyMotion()), vibration);
      printLine(1, line);
    }
    return;
  }

  // Smoke sensor status panel (only while the MQ-2 is not ready).
  if (currentPanel == 3 && mq2State != MQ2_READY) {
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
  Wire.begin();                       // I2C shared by LCD + GY-521

  pinMode(MQ2_PIN, INPUT);
  pinMode(FLAME_PIN, INPUT); // Wokwi light-sensor DO is an actively driven digital output
  pinMode(PANEL_BUTTON_PIN, INPUT_PULLUP);
  panelButtonLastReading = digitalRead(PANEL_BUTTON_PIN);
  panelButtonStableState = panelButtonLastReading;
  panelButtonLastChange = millis();

  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_YELLOW_PIN, OUTPUT);
  pinMode(LED_RED_PIN, OUTPUT);

  // Initialize all buzzer pins OFF before any alarm detection begins.
  allBuzzersOff();
  pinMode(BUZZER_SMOKE_PIN, OUTPUT);
  pinMode(BUZZER_QUAKE_PIN, OUTPUT);
  pinMode(BUZZER_FIRE_PIN, OUTPUT);
  allBuzzersOff();

  // Lamp test: all three LEDs on during the splash screen.
  setLeds(true, true, true);

  dht.begin();

  lcd.init();
  lcd.backlight();
  lcd.clear();

  printLineP(0, F("GAYAK MONITOR"));
  printLineP(1, F("INITIALIZING..."));

  delay(600);  // shorten Wokwi boot splash; continue sensor warm-up asynchronously

  setLeds(false, false, false);

  // Initialize MPU while leaving original fire/smoke logic unchanged.
  initializeMPU(millis());

  // Learn the flame module's resting level (no flame in view).
  printLineP(0, F("FLAME LEARNING"));
  printLineP(1, F("KEEP FLAME AWAY"));
  learnFlameIdle();

  currentPanel = 0;

  unsigned long now = millis();

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

  // Poll alarms early: fire, smoke and vibration.
  updateFlame(now);
  updateMQ2(now);
  updateMPU(now);

  // Read DHT22 immediately, then every 2.5 seconds.
  if (firstReading || now - lastSensorRead >= SENSOR_INTERVAL) {
    firstReading = false;
    lastSensorRead = now;
    readSensor();            // blocks ~250 ms worst case
  }

  now = millis();
  updateFlame(now);          // catch up right after the DHT22 block
  updateMQ2(now);
  updateMPU(now);
  printDiagnostics(now);
  updateLeds(now);
  updateBuzzers(now);
  updatePanelButton(now);

  showPanel();

  // Unchanged DATA CSV for existing desktop dashboard + separate QUAKE CSV.
  sendSerialData(now);
  sendQuakeData(now);
}
