#ifndef GPS_MANAGER_H
#define GPS_MANAGER_H

#include <Arduino.h>

// Structure des données GPS
struct GpsData {
    bool fix = false;
    double lat = 0.0;
    double lon = 0.0;
    double alt = 0.0;
    uint8_t sats = 0;
    double speed = 0.0;
    String timeUTC = "";
    String date = "";
};

// === Fonctions principales ===
void gps_init();                                // Initialisation du module GPS
bool gps_read(GpsData &out, unsigned long timeout_ms); // Lecture avec timeout
void gps_set_enabled(bool en);                  // Active/désactive le GPS
bool gps_is_enabled();                          // Renvoie l’état actuel

#endif
