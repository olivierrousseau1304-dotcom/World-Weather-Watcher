#include <EEPROM.h>
#include "config.h"

// === Instance unique ===
Config config = {
    10,     // LOG_INTERVALL
    30,     // TIMEOUT
    4096,    // FILE_MAX_SIZE
    1,      // GPS activé par défaut
    5,       // GPS_TIMEOUT = 5s
};

// === Table des paramètres ===
ConfigParam configParams[] = {
    {"LOG_INTERVALL", &config.LOG_INTERVALL, 1, 60, 10},
    {"TIMEOUT",       &config.TIMEOUT,       1, 300, 30},
    {"FILE_MAX_SIZE", &config.FILE_MAX_SIZE, 512, 32767, 4096},
    {"GPS",           &config.GPS,           0, 1, 1},
    {"GPS_TIMEOUT",   &config.GPS_TIMEOUT,   1, 60, 5}
};
const int NUM_CONFIG_PARAMS = sizeof(configParams) / sizeof(ConfigParam);

// === Fonctions principales ===

void config_reset_defaults() {
    for (int i = 0; i < NUM_CONFIG_PARAMS; i++) {
        *configParams[i].value = configParams[i].defaultVal;
    }
    config_save();
}

void config_save() {
    EEPROM.put(0, config);
}

void config_init() {
    EEPROM.get(0, config);

    // Validation des bornes
    for (int i = 0; i < NUM_CONFIG_PARAMS; i++) {
        if (*configParams[i].value < configParams[i].minVal ||
            *configParams[i].value > configParams[i].maxVal) {
            *configParams[i].value = configParams[i].defaultVal;
        }
    }
}

// === Accès dynamiques ===

bool config_set(const char* key, int value) {
    for (int i = 0; i < NUM_CONFIG_PARAMS; i++) {
        if (strcmp(key, configParams[i].name) == 0) {
            if (value >= configParams[i].minVal && value <= configParams[i].maxVal) {
                *configParams[i].value = value;
                config_save();
                return true;
            } else {
                return false; // Hors bornes
            }
        }
    }
    return false; // Clé non trouvée
}

int config_get(const char* key) {
    for (int i = 0; i < NUM_CONFIG_PARAMS; i++) {
        if (strcmp(key, configParams[i].name) == 0)
            return *configParams[i].value;
    }
    return -1; // Non trouvé
}

void config_print() {
    Serial.println("=== Configuration actuelle ===");
    for (int i = 0; i < NUM_CONFIG_PARAMS; i++) {
        Serial.print(configParams[i].name);
        Serial.print(" = ");
        Serial.println(*configParams[i].value);
    }
}

void config_print_from_eeprom() {
    Config temp;
    EEPROM.get(0, temp);
    Serial.println("=== Données réelles EEPROM ===");
    Serial.print("LOG_INTERVALL = "); Serial.println(temp.LOG_INTERVALL);
    Serial.print("TIMEOUT       = "); Serial.println(temp.TIMEOUT);
    Serial.print("FILE_MAX_SIZE = "); Serial.println(temp.FILE_MAX_SIZE);
}
