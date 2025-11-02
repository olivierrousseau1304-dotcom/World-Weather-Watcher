#ifndef GPS_MANAGER_H
#define GPS_MANAGER_H

#include <Arduino.h>

struct GpsData {
  bool  fix;
  float lat;
  float lon;
  float alt;
  uint8_t sats;
  float speed;
};

void gps_init();
bool gps_read(GpsData &out, unsigned long timeout_ms);
void gps_set_enabled(bool en);
bool gps_is_enabled();

#endif
