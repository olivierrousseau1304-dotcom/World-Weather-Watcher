#ifndef GPS_MANAGER_H
#define GPS_MANAGER_H

#include <Arduino.h>

// Données GPS utiles CDC
struct GpsData {
  bool   fix;
  float  lat;
  float  lon;
  float  alt;
  uint8_t sats;
  float  speed;
};

// Init module
void gps_init();

// Lecture avec timeout (ms)
bool gps_read(GpsData &out, unsigned long timeoutMs);

#endif
