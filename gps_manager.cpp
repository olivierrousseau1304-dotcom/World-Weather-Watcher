#include "gps_manager.h"
#include "config.h"
#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>

// === Définition des broches ===
// RX Arduino ← TX GPS  |  TX Arduino → RX GPS
static const uint8_t GPS_RX = 6;
static const uint8_t GPS_TX = 7;

// === Instances des objets ===
static SoftwareSerial gpsSerial(GPS_RX, GPS_TX);
static TinyGPSPlus gps;
static bool gpsEnabled = true;

// === Fonctions publiques ===
void gps_set_enabled(bool en) { gpsEnabled = en; }
bool gps_is_enabled() { return gpsEnabled; }

// === Initialisation ===
void gps_init() {
    gpsSerial.begin(9600); // conforme au wiki Seeed (9600 baud par défaut)
    Serial.println(F("GPS Air530 initialisé sur D6(DRX)/D7(DTX) à 9600 bauds."));
}

// === Lecture du GPS ===
bool gps_read(GpsData &out, unsigned long timeout_ms) {
    if (!gpsEnabled) return false;

    unsigned long start = millis();
    bool received = false;

    while (millis() - start < timeout_ms) {
        while (gpsSerial.available()) {
            char c = gpsSerial.read();
            if (gps.encode(c)) received = true;
        }
    }

    // Remplissage des données
    if (gps.location.isValid()) {
        out.fix = true;
        out.lat = gps.location.lat();
        out.lon = gps.location.lng();
    } else out.fix = false;

    if (gps.altitude.isValid())   out.alt = gps.altitude.meters();
    if (gps.satellites.isValid()) out.sats = gps.satellites.value();
    if (gps.speed.isValid())      out.speed = gps.speed.kmph();

    if (gps.time.isValid()) {
        char buf[9];
        snprintf(buf, sizeof(buf), "%02d:%02d:%02d", gps.time.hour(), gps.time.minute(), gps.time.second());
        out.timeUTC = String(buf);
    }
    if (gps.date.isValid()) {
        char buf[11];
        snprintf(buf, sizeof(buf), "20%02d-%02d-%02d", gps.date.year()%100, gps.date.month(), gps.date.day());
        out.date = String(buf);
    }

    // Retour : vrai si au moins une trame valide reçue
    return out.fix && received;
}
// Retourne vrai si le GPS a une position valide
