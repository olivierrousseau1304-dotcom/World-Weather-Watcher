#include "gps_manager.h"
#include <SoftwareSerial.h>

static const uint8_t GPS_RX_PIN = 6;   // Air530 → TX
static const uint8_t GPS_TX_PIN = 7;   // Air530 ← RX

static SoftwareSerial gpsSerial(GPS_RX_PIN, GPS_TX_PIN);
static bool gpsEnabled = true;

void gps_set_enabled(bool en) { gpsEnabled = en; }
bool gps_is_enabled() { return gpsEnabled; }

// ------------------------------------------
static float parseCoord(const char *s, char h)
{
    if (!s || !*s) return 0;

    float raw = atof(s);     // ddmm.mmmm
    int deg = (int)(raw / 100);
    float minutes = raw - deg * 100.0f;
    float dec = deg + minutes / 60.0f;

    if (h == 'S' || h == 'W') dec = -dec;
    return dec;
}

// ------------------------------------------
bool gps_read(GpsData &o, unsigned long timeout_ms)
{
    if (!gpsEnabled) return false;

    unsigned long start = millis();
    o.fix = false;

    char buf[100];
    uint8_t i = 0;

    while (millis() - start < timeout_ms)
    {
        if (!gpsSerial.available()) continue;

        char c = gpsSerial.read();

        if (c == '\n')
        {
            buf[i] = 0;
            i = 0;

            // Only GGA + RMC needed
            if (strncmp(buf, "$GNGGA", 6) == 0 || strncmp(buf, "$GPGGA", 6) == 0)
            {
                // 1=time,2=lat,3=N,4=lon,5=E,6=quality,7=sats,8=HDOP,9=alt
                char *p = buf;
                char *t[12];
                int k=0;

                while (k<12 && (t[k] = strtok(p, ","))) {
                    p = NULL;
                    k++;
                }
                if (k >= 10)
                {
                    int fixq    = atoi(t[6]);
                    uint8_t sats= atoi(t[7]);
                    float alt   = atof(t[9]);

                    if (fixq > 0) {
                        o.fix  = true;
                        o.lat  = parseCoord(t[2], t[3][0]);
                        o.lon  = parseCoord(t[4], t[5][0]);
                        o.sats = sats;
                        o.alt  = alt;
                    }
                }
                continue;
            }

            if (strncmp(buf, "$GNRMC", 6) == 0 || strncmp(buf, "$GPRMC", 6) == 0)
            {
                // 1=time,2=valid,3=lat,4=N,5=lon,6=E,7=speed(kn)
                char *p = buf;
                char *t[10];
                int k=0;

                while (k<10 && (t[k] = strtok(p, ","))) {
                    p = NULL;
                    k++;
                }
                if (k >= 8)
                {
                    if (t[2][0] == 'A')
                    {
                        o.fix  = true;
                        o.lat  = parseCoord(t[3], t[4][0]);
                        o.lon  = parseCoord(t[5], t[6][0]);

                        float kn = atof(t[7]);
                        o.speed = kn * 1.852f;   // km/h
                    }
                }
                // If RMC gave fix, return now
                if (o.fix) return true;
            }
        }
        else
        {
            if (i < sizeof(buf)-1)
                buf[i++] = c;
        }
    }
    return o.fix;
}

// ------------------------------------------
void gps_init()
{
    gpsSerial.begin(9600);
}
