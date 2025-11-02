#pragma once
#include <Arduino.h>
#include "gps_manager.h"

// Initialisation de la carte SD
bool sd_init(uint8_t csPin = 4);

// État de la carte SD
bool sd_is_ready();

// Fermeture du fichier courant
void sd_close();

// Écrit une ligne CSV (date, heure, données GPS)
bool sd_append_csv(const GpsData& g, const String& dateStr, const String& timeStr);

// Paramètres divers
void sd_set_max_file_size(uint32_t bytes);
uint32_t sd_get_current_size();
const char* sd_get_current_filename();
