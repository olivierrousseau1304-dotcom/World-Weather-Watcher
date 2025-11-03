#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <EEPROM.h>

// ==========================
// CONFIG STRUCT
// ==========================
struct Config {
    // Core
    uint16_t LOG_INTERVALL;   // minutes
    uint16_t FILE_MAX_SIZE;   // bytes
    uint8_t  TIMEOUT;         // s
    uint8_t  GPS_TIMEOUT;     // s
    uint8_t  GPS;             // 0/1

    // ---- Light / Luminosity ----
    uint8_t  LUMIN;           // 0/1 activation
    uint16_t LUMIN_LOW;       // 0–1023
    uint16_t LUMIN_HIGH;      // 0–1023

    // ---- Temperature ----
    uint8_t  TEMP_AIR;        // 0/1
    int16_t  MIN_TEMP_AIR;    // -40 to 85
    int16_t  MAX_TEMP_AIR;    // -40 to 85

    // ---- Hygrometry ----
    uint8_t  HYGR;            // 0/1
    int16_t  HYGR_MINT;       // -40 to 85
    int16_t  HYGR_MAXT;       // -40 to 85

    // ---- Pressure ----
    uint8_t  PRESSURE;        // 0/1
    uint16_t PRESSURE_MIN;    // 300–1100
    uint16_t PRESSURE_MAX;    // 300–1100
};

extern Config config;

// Load/Save
void config_load();
void config_save();

// Reset all to defaults
void config_reset_defaults();

#endif
