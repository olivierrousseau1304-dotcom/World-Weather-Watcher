#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ========================================================
// Structure principale des paramètres système
// ========================================================

struct Config {
    int LOG_INTERVALL;   // minutes
    int TIMEOUT;         // secondes (général)
    int FILE_MAX_SIZE;   // octets
    int GPS;             // 0/1
    int GPS_TIMEOUT;     // secondes
};

// Param global accessible partout
extern Config config;

// ========================================================
void config_init();
void config_save();

#endif
