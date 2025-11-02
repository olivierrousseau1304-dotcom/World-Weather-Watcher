#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Broches matérielles
#define LED_PIN 4
#define BUTTON2_PIN 9
#define BUTTON1_PIN 8

// === Structure principale ===
struct Config {
    int LOG_INTERVALL;   // minutes
    int TIMEOUT;         // secondes
    int FILE_MAX_SIZE;   // octets
    int GPS;             // 0/1 activation du GPS
    int GPS_TIMEOUT;     // secondes timeout de lecture

};

// === Métadonnées ===
struct ConfigParam {
    const char* name;   // Nom du paramètre
    int* value;         // Pointeur vers la variable dans Config
    int minVal;         // Valeur minimale
    int maxVal;         // Valeur maximale
    int defaultVal;     // Valeur par défaut
};

// === Fonctions principales ===
void config_init();              // Charger depuis EEPROM + validation
void config_save();              // Sauvegarder dans EEPROM
void config_reset_defaults();    // Réinitialiser tous les paramètres par défaut
void config_print_from_eeprom();
// === Fonctions d’accès dynamiques ===
bool config_set(const char* key, int value);  // Modifier une clé
int  config_get(const char* key);             // Lire une clé
void config_print();                          // Affiche toutes les valeurs (debug)

// === Variables globales exposées ===
extern Config config;
extern ConfigParam configParams[];
extern const int NUM_CONFIG_PARAMS;

#endif
