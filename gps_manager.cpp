#include "gps_manager.h"
#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>

// -----------------------------
// GPS Air530Z → Grove
// TX GPS → D6 (RX µC)
// RX GPS ← D7 (TX µC)
// Baud = 9600
// -----------------------------

static const uint8_t GPS_RX = 6;  // Arduino lit ici (GPS → µC)
static const uint8_t GPS_TX = 7;  // Arduino parle ici (µC → GPS)

static TinyGPSPlus     gps;
static SoftwareSerial  gpsSerial(GPS_RX, GPS_TX);

static bool gpsEnabled = true;


// ----------------------------------------------------
void gps_set_enabled(bool en)
{
    gpsEnabled = en;
}

bool gps_is_enabled()
{
    return gpsEnabled;
}


// ----------------------------------------------------
// Init module GPS
// ----------------------------------------------------
void gps_init()
{
    gpsSerial.begin(9600);
    gpsEnabled = true;
}


// ----------------------------------------------------
// Lecture GPS avec timeout
// Remplit GpsData
// Retourne true si FIX valide
// ----------------------------------------------------
bool gps_read(GpsData& out, unsigned long timeout_ms)
{
    if (!gpsEnabled) {
        out.fix  = false;
        return false;
    }

    unsigned long start = millis();

    while (millis() - start < timeout_ms)
    {
        while (gpsSerial.available() > 0)
        {
            gps.encode(gpsSerial.read());
        }

        if (gps.location.isUpdated())
        {
            out.fix   = gps.location.isValid();
            out.lat   = gps.location.lat();
            out.lon   = gps.location.lng();
            out.alt   = gps.altitude.meters();
            out.sats  = gps.satellites.value();
            out.speed = gps.speed.kmph();

            // UTC
            if (gps.time.isValid())
            {
                char bufT[9];
                snprintf(bufT, sizeof(bufT), "%02d:%02d:%02d",
                         gps.time.hour(),
                         gps.time.minute(),
                         gps.time.second());
                out.timeUTC = bufT;
            } else {
                out.timeUTC = "NA";
            }

            // Date
            if (gps.date.isValid())
            {
                char bufD[11];
                snprintf(bufD, sizeof(bufD), "%04d-%02d-%02d",
                         gps.date.year(),
                         gps.date.month(),
                         gps.date.day());
                out.date = bufD;
            } else {
                out.date = "NA";
            }

            return out.fix;   // OK
        }
    }

    // Pas d’update
    out.fix = false;
    return false;
}
