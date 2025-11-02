#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// =====================================================
// STRUCTURE DE CONFIG
// =====================================================
struct Config {

  // Base
  int LOG_INTERVALL;     // minutes
  int TIMEOUT;           // sec read sensors
  int FILE_MAX_SIZE;     // bytes
  int GPS;               // enable GPS
  int GPS_TIMEOUT;       // sec read GPS

  // Sensors enable
  int LUMIN;             // enable luminosity
  int TEMP_AIR;          // enable temperature
  int HYGR;              // enable humidity
  int PRESSURE;          // enable pressure

  // Lumi bounds
  int LUMIN_LOW;
  int LUMIN_HIGH;

  // temp bounds
  int MIN_TEMP_AIR;
  int MAX_TEMP_AIR;

  // humidity bounds
  int HYGR_MINT;
  int HYGR_MAXT;

  // pressure bounds
  int PRESSURE_MIN;
  int PRESSURE_MAX;
};

// =====================================================
// API
// =====================================================
extern Config config;

// initialisation → load EEPROM
void config_init();

// impression des paramètres
void config_print_all();

// sauve en EEPROM
void config_save();

// remet valeurs défaut
void config_reset_defaults();

// setter SET <key> <value>
bool config_set_param(const String &key, int value);

#endif
