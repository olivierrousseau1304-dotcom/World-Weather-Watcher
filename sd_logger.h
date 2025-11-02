#ifndef SD_LOGGER_H
#define SD_LOGGER_H

#include <Arduino.h>
#include "gps_manager.h"

// API publique – version définitive
bool     sd_init(uint8_t csPin = 4);
bool     sd_is_ready();
void     sd_close();

bool     sd_append_csv(const GpsData& g,
                       const String& dateStr,
                       const String& timeStr);

void     sd_set_max_file_size(uint32_t bytes);
uint32_t sd_get_current_size();
const char* sd_get_current_filename();

#endif
