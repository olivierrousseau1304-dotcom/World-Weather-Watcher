#include "bme_manager.h"
#include <Wire.h>

#define BME280_ADDR   0x76
#define REG_ID        0xD0
#define REG_CTRL_HUM  0xF2
#define REG_CTRL_MEAS 0xF4
#define REG_CONFIG    0xF5
#define REG_DATA      0xF7

static bool bmeOK = false;

// -------------------------
// I2C helpers
// -------------------------
static uint8_t read8(uint8_t r) {
  Wire.beginTransmission(BME280_ADDR);
  Wire.write(r);
  Wire.endTransmission();
  Wire.requestFrom(BME280_ADDR, 1);
  return Wire.read();
}

static uint32_t read24(uint8_t r) {
  Wire.beginTransmission(BME280_ADDR);
  Wire.write(r);
  Wire.endTransmission();
  Wire.requestFrom(BME280_ADDR, 3);
  uint32_t v = Wire.read();
  v = (v << 8) | Wire.read();
  v = (v << 8) | Wire.read();
  return v;
}

// -------------------------
// Init
// -------------------------
bool bme_init()
{
  Wire.begin();

  if (read8(REG_ID) != 0x60)
    return false;

  // Humidity oversampling x1
  Wire.beginTransmission(BME280_ADDR);
  Wire.write(REG_CTRL_HUM);
  Wire.write(0x01);
  Wire.endTransmission();

  // temp+press oversampling x1, normal mode
  Wire.beginTransmission(BME280_ADDR);
  Wire.write(REG_CTRL_MEAS);
  Wire.write(0x27);
  Wire.endTransmission();

  // Standby 1000 ms
  Wire.beginTransmission(BME280_ADDR);
  Wire.write(REG_CONFIG);
  Wire.write(0xA0);
  Wire.endTransmission();

  delay(10);
  bmeOK = true;
  return true;
}

// -------------------------
// Read sensors
// Simplified conversion
// -------------------------
bool bme_read(BmeData &o)
{
  if (!bmeOK) {
    o.ok = false;
    return false;
  }

  uint32_t rawP = read24(REG_DATA);
  uint32_t rawT = read24(REG_DATA + 3);
  uint16_t rawH = (read8(REG_DATA + 6) << 8) | read8(REG_DATA + 7);

  rawP >>= 4;
  rawT >>= 4;

  // ---------- Temperature (simplifiée)
  float T = rawT / 5120.0;  
  o.temperature = T;

  // ---------- Humidity (simplifiée)
  o.humidity = rawH / 1024.0;

  // ---------- Pressure (simplifiée)
  o.pressure = rawP / 100.0;

  o.ok = true;
  return true;
}
