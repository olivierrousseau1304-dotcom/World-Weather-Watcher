#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
// #include <SD.h>   // ❌ NE PAS UTILISER : on a un logger SD custom, cette lib fait exploser la FLASH

#include "config.h"
#include "commands.h"
#include "rtc_manager.h"
#include "sd_logger.h"
#include "gps_manager.h"
#include "bme_manager.h"
#include "led_manager.h"
#include "button_manager.h"

// =====================
// MODES
// =====================
enum Mode : uint8_t {
  MODE_STANDARD = 0,
  MODE_ECO,
  MODE_MAINT,
  MODE_CONFIG
};

static Mode currentMode   = MODE_STANDARD;
static Mode previousMode  = MODE_STANDARD;

// =====================
// PINS
// =====================
static const uint8_t PIN_SD_CS = 4;
static const uint8_t PIN_LIGHT = A0;

// =====================
// TIMING
// =====================
static unsigned long lastMeasureMs = 0;
static unsigned long measureIntervalMs = 10UL * 60UL * 1000UL;

static unsigned long configLastActivityMs = 0;
static const unsigned long CONFIG_IDLE_BACK_MS = 30UL * 60UL * 1000UL;

// ECO → GPS 1/2
static bool ecoGpsToggle = false;

// FAIL counters
static uint8_t failGps = 0;
static uint8_t failBme = 0;

// =====================
// HELPERS
// =====================
static void restoreModeLed() {
  switch (currentMode) {
    case MODE_STANDARD:  led_mode_standard();    break;
    case MODE_ECO:       led_mode_economy();     break;
    case MODE_MAINT:     led_mode_maintenance(); break;
    case MODE_CONFIG:    led_mode_config();      break;
  }
}

static void enterMode(Mode m) {
  currentMode = m;

  switch (m) {
    case MODE_STANDARD:
      measureIntervalMs = (uint32_t)config.LOG_INTERVALL * 60000UL;
      led_mode_standard();
      break;

    case MODE_ECO:
      measureIntervalMs = (uint32_t)config.LOG_INTERVALL * 120000UL;
      ecoGpsToggle = false;
      led_mode_economy();
      break;

    case MODE_MAINT:
      led_mode_maintenance();
      sd_close(); // sécurité carte
      break;

    case MODE_CONFIG:
      led_mode_config();
      configLastActivityMs = millis();
      break;
  }
}

static void readLight(struct LightData &L) {
  L.value = analogRead(PIN_LIGHT);
  L.ok    = true;
}

// =====================
// ACQUISITION + LOG
// =====================
static void doAcquisitionAndLog() {
  char d6[7], t6[7];
  rtc_get_date_YYMMDD(d6);
  rtc_get_time_HHMMSS(t6);

  // ---- GPS ----
  GpsData gps = {};
  bool wantGps = (config.GPS != 0);

  if (currentMode == MODE_ECO) {
    ecoGpsToggle = !ecoGpsToggle;
    wantGps = wantGps && ecoGpsToggle;
  }

  if (wantGps) {
    bool ok = gps_read(gps, (unsigned long)config.GPS_TIMEOUT * 1000UL);
    if (!ok || !gps.fix) { if (failGps < 255) failGps++; }
    else failGps = 0;
  } else {
    gps.fix = false;
  }

  // ---- BME ----
  BmeData bme = {};
  if (!bme_read(bme, (unsigned long)config.TIMEOUT * 1000UL) || !bme.ok) {
    if (failBme < 255) failBme++;
  } else {
    failBme = 0;
  }

  // ---- LIGHT ----
  LightData L = {};
  readLight(L);

  // ---- LED ERROR after 2 failures ----
  if (failGps >= 2)  { led_err_gps();    restoreModeLed(); }
  if (failBme >= 2)  { led_err_sensor(); restoreModeLed(); }

  // ============================
  // MODE MAINT → print compact SANS float
  // ============================
  if (currentMode == MODE_MAINT) {
    // On évite Serial.print(float, n) pour ne pas linker le formateur float.
    // On scale en entiers :
    // lat/lon : micro-degrés (deg * 1e6)
    // alt : dm (m*10)
    // speed : dizièmes (noeuds*10) si tu sors en noeuds, ou km/h*10 selon ton gps_manager
    // temp/hum/press : ×10
    long lat_u = 0, lon_u = 0, alt_d = 0, spd_d = 0;
    int16_t t10 = 0, h10 = 0, p10 = 0;

    if (gps.fix) {
      lat_u = (long)(gps.lat * 1000000.0f);
      lon_u = (long)(gps.lon * 1000000.0f);
      alt_d = (long)(gps.alt * 10.0f);
      spd_d = (long)(gps.speed * 10.0f);
    }

    if (bme.ok) {
      t10 = (int16_t)(bme.temp * 10.0f);
      h10 = (int16_t)(bme.hum  * 10.0f);
      p10 = (int16_t)(bme.pres * 10.0f);
    }

    // CSV : YYMMDD,HHMMSS,lat_u,lon_u,alt_d,sats,spd_d,t10,h10,p10,lum
    Serial.print(d6);  Serial.print(',');
    Serial.print(t6);  Serial.print(',');

    if (gps.fix) {
      Serial.print(lat_u); Serial.print(',');
      Serial.print(lon_u); Serial.print(',');
      Serial.print(alt_d); Serial.print(',');
      Serial.print(gps.sats); Serial.print(',');
      Serial.print(spd_d);
    } else {
      Serial.print(F("NA,NA,NA,NA,NA"));
    }

    Serial.print(',');

    if (bme.ok) {
      Serial.print(t10); Serial.print(',');
      Serial.print(h10); Serial.print(',');
      Serial.print(p10);
    } else {
      Serial.print(F("NA,NA,NA"));
    }

    Serial.print(',');
    Serial.println(L.value);

    return;
  }

  // ============================
  // STANDARD / ECO → SD logging
  // ============================
  sd_append_csv(gps, bme, L, d6, t6);
}

// =====================
// SETUP
// =====================
void setup() {
  Serial.begin(9600);
  Wire.begin();

  buttons_init();
  config_load();
  rtc_init();

  led_off();

  sd_set_max_file_size((uint32_t)config.FILE_MAX_SIZE);
  sd_init(PIN_SD_CS);

  bme_init();
  gps_init();

  // Bootstrap mode
  if (digitalRead(BUTTON_RED_PIN) == LOW)
       enterMode(MODE_CONFIG);
  else enterMode(MODE_STANDARD);

  if (!rtc_is_available()) {
    led_err_rtc();
    restoreModeLed();
  }

  lastMeasureMs = millis();
}

// =====================
// LOOP
// =====================
void loop() {
  // CONFIG
  if (currentMode == MODE_CONFIG) {
    if (Serial.available())
      configLastActivityMs = millis();

    commands_poll();

    if (millis() - configLastActivityMs >= CONFIG_IDLE_BACK_MS)
      enterMode(MODE_STANDARD);

    delay(10);
    return;
  }

  // BUTTONS
  uint8_t req = buttons_update((uint8_t)currentMode);
  if (req == 1 && currentMode == MODE_STANDARD) enterMode(MODE_ECO);
  else if (req == 2) { previousMode = currentMode; enterMode(MODE_MAINT); }
  else if (req == 3) { if (!sd_is_ready()) sd_init(PIN_SD_CS); enterMode(previousMode); }
  else if (req == 4) enterMode(MODE_STANDARD);

  // MEASURE
  unsigned long now = millis();
  if (now - lastMeasureMs >= measureIntervalMs) {
    lastMeasureMs = now;
    doAcquisitionAndLog();
  }
}
