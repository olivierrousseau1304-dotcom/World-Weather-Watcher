#include "bme_manager.h"
#include "config.h"
#include "led_manager.h"
#include <Wire.h>

// Capteur BME280 I2C 0x76
static const uint8_t BME_ADDR = 0x76;
static bool inited = false;
static uint8_t failCount = 0;


// -------------------------------------------------------------------
// Lecture d’un registre
// -------------------------------------------------------------------
static bool readReg(uint8_t reg, uint8_t &v)
{
    Wire.beginTransmission(BME_ADDR);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0) return false;

    if (Wire.requestFrom(BME_ADDR, (uint8_t)1) != 1) return false;
    v = Wire.read();
    return true;
}


// -------------------------------------------------------------------
// Lecture brute compensation + conversion simplifiée
// (On simplifie → précision suffisante + FLASH faible)
// -------------------------------------------------------------------
bool bme_init()
{
    Wire.begin();

    uint8_t id = 0;
    if (!readReg(0xD0, id)) {
        inited = false;
        return false;
    }

    // BME280 ID officiel = 0x60
    if (id != 0x60) {
        inited = false;
        return false;
    }

    // Forced mode basic
    Wire.beginTransmission(BME_ADDR);
    Wire.write(0xF4);
    Wire.write(0x25);   // oversampling light + forced
    Wire.endTransmission();

    inited = true;
    failCount = 0;
    return true;
}


// -------------------------------------------------------------------
// Forced measurement
// -------------------------------------------------------------------
static bool bme_take_forced()
{
    Wire.beginTransmission(BME_ADDR);
    Wire.write(0xF4);
    Wire.write(0x25); // forced
    return Wire.endTransmission() == 0;
}


// -------------------------------------------------------------------
// read24 helper
// -------------------------------------------------------------------
static bool read24(uint8_t reg, int32_t &out)
{
    Wire.beginTransmission(BME_ADDR);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0) return false;

    if (Wire.requestFrom(BME_ADDR, (uint8_t)3) != 3) return false;

    uint32_t v = Wire.read();
    v = (v << 8) | Wire.read();
    v = (v << 8) | Wire.read();

    out = (int32_t)v;
    return true;
}


// -------------------------------------------------------------------
// Lecture simplifiée
// -------------------------------------------------------------------
bool bme_read(BmeData &o, unsigned long timeoutMs)
{
    o.ok  = false;
    o.temp = 0;
    o.hum  = 0;
    o.pres = 0;

    if (!inited)
    {
        failCount++;
        return false;
    }

    unsigned long t0 = millis();
    if (!bme_take_forced()) {
        failCount++;
        return false;
    }

    delay(10);

    int32_t rawT, rawH, rawP;
    if (!read24(0xFA, rawT)) return false;
    if (!read24(0xFD, rawH)) return false;
    if (!read24(0xF7, rawP)) return false;

    float T = rawT / 16384.0f;
    float H = rawH / 16384.0f;
    float P = rawP / 25600.0f;

    bool inco = false;
    if (H < config.HYGR_MINT || H > config.HYGR_MAXT) inco = true;
    if (P < config.PRESSURE_MIN || P > config.PRESSURE_MAX) inco = true;

    if (inco)
    {
        led_err_sensor_inco();
        o.ok = false;
        return false;
    }

    o.ok   = true;
    o.temp = T;
    o.hum  = H;
    o.pres = P;

    failCount = 0;
    return true;
}
