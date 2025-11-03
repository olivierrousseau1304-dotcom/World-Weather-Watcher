#include "config.h"

Config config;

#define EE_ADDR   0

void config_reset_defaults() {
    config.LOG_INTERVALL = 10;
    config.FILE_MAX_SIZE = 2048;
    config.TIMEOUT       = 30;
    config.GPS_TIMEOUT   = 5;
    config.GPS           = 1;

    // Luminosity
    config.LUMIN      = 1;
    config.LUMIN_LOW  = 255;
    config.LUMIN_HIGH = 768;

    // Temp (°C)
    config.TEMP_AIR     = 1;
    config.MIN_TEMP_AIR = -10;
    config.MAX_TEMP_AIR = 60;

    // Hygrometry
    config.HYGR     = 1;
    config.HYGR_MINT = 0;
    config.HYGR_MAXT = 50;

    // Pressure (hPa)
    config.PRESSURE     = 1;
    config.PRESSURE_MIN = 850;
    config.PRESSURE_MAX = 1080;
}

void config_save() {
    EEPROM.put(EE_ADDR, config);
}

void config_load() {
    EEPROM.get(EE_ADDR, config);

    // minimal validation → if EEPROM empty → reset
    if (config.LOG_INTERVALL > 50000) {
        config_reset_defaults();
        config_save();
    }
}
