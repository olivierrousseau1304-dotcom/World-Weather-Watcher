#ifndef GPS_MANAGER_H
#define GPS_MANAGER_H

#include <Arduino.h>

// Structure publique utilisée partout
struct GpsData {
    bool    fix;
    double  lat;
    double  lon;
    double  alt;
    uint8_t sats;
    double  speed;
    String  timeUTC;
    String  date;
};

// API
void gps_init();
bool gps_read(GpsData& out, unsigned long timeout_ms);
void gps_set_enabled(bool en);
bool gps_is_enabled();

#endif
