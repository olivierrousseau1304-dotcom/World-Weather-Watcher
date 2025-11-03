#ifndef SD_LOGGER_H
#define SD_LOGGER_H

#include <Arduino.h>
#include "gps_manager.h"
#include "bme_manager.h"

// --- Light struct ---
struct LightData {
    int  value;
    bool ok;
};

// API
bool     sd_init(uint8_t csPin = 4);    // Grove SD on UNO
bool     sd_is_ready();
void     sd_close();

// Ajoute une ligne CSV
bool sd_append_csv(const GpsData& gps,
                   const BmeData& bme,
                   const LightData& l,
                   const char* dateStr,    // YYMMDD
                   const char* timeStr);   // HHMMSS

void     sd_set_max_file_size(uint32_t bytes);
uint32_t sd_get_current_size();
const char* sd_get_current_filename();

#endif
