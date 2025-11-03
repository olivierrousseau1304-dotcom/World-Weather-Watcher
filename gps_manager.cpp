#include "gps_manager.h"
#include <Arduino.h>
#include <SoftwareSerial.h>

// GPS pins (Grove UART)
static SoftwareSerial gpsSerial(6, 7); // RX,TX

// Convert ddmm.mmmm + dir -> decimal
static float nmeaToDecimal(float v, char dir)
{
  int deg = (int)(v / 100);
  float mins = v - deg * 100;
  float d = deg + mins / 60.0f;
  if (dir == 'S' || dir == 'W') d = -d;
  return d;
}

void gps_init()
{
  gpsSerial.begin(9600);
  delay(50);
}

static bool parseGGA(const char* s, GpsData &d)
{
  // Ex: $GPGGA,123519,4807.038,N,01131.000,E,1,08,...,545.4,M,...
  // idx:  0     1      2      3    4      5 6  7         9
  char copy[100];
  strncpy(copy, s, sizeof(copy));
  copy[sizeof(copy)-1] = 0;

  char* tok = strtok(copy, ",");
  uint8_t idx = 0;
  float lat=0, lon=0, alt=0;
  char ns='N', ew='E';
  int fix=0, sats=0;

  while (tok)
  {
    switch (idx)
    {
      case 2: lat = atof(tok); break;
      case 3: ns  = tok[0];    break;
      case 4: lon = atof(tok); break;
      case 5: ew  = tok[0];    break;
      case 6: fix = atoi(tok); break;
      case 7: sats = atoi(tok); break;
      case 9: alt = atof(tok); break;
    }
    tok = strtok(nullptr, ",");
    idx++;
  }

  if (fix > 0)
  {
    d.fix  = true;
    d.lat  = nmeaToDecimal(lat, ns);
    d.lon  = nmeaToDecimal(lon, ew);
    d.alt  = alt;
    d.sats = (uint8_t)sats;
    return true;
  }

  return false;
}

static bool parseRMC(const char* s, GpsData &d)
{
  // Ex: $GPRMC,hhmmss,A,lat,NS,lon,EW,speed...
  char copy[100];
  strncpy(copy, s, sizeof(copy));
  copy[sizeof(copy)-1] = 0;

  char* tok = strtok(copy, ",");
  uint8_t idx = 0;
  char A='V';
  float lat=0, lon=0, sp=0;
  char ns='N', ew='E';

  while (tok)
  {
    switch (idx)
    {
      case 2: A   = tok[0]; break;
      case 3: lat = atof(tok); break;
      case 4: ns  = tok[0]; break;
      case 5: lon = atof(tok); break;
      case 6: ew  = tok[0]; break;
      case 7: sp  = atof(tok); break;   // knots
    }
    tok = strtok(nullptr, ",");
    idx++;
  }

  if (A != 'A') return false; // active fix

  d.fix   = true;
  d.lat   = nmeaToDecimal(lat, ns);
  d.lon   = nmeaToDecimal(lon, ew);
  d.speed = sp * 1.852f;  // knots -> km/h
  return true;
}

bool gps_read(GpsData &out, unsigned long timeoutMs)
{
  out.fix = false;
  out.lat = out.lon = 0;
  out.alt = 0;
  out.sats = 0;
  out.speed = 0;

  unsigned long start = millis();
  char buf[100];
  uint8_t pos = 0;

  while (millis() - start < timeoutMs)
  {
    if (gpsSerial.available())
    {
      char c = gpsSerial.read();
      if (c == '\n' || c == '\r')
      {
        buf[pos] = 0;
        if (pos > 6)
        {
          if (!out.fix && strncmp(buf, "$GPGGA", 6)==0)
            parseGGA(buf, out);

          if (!out.fix && strncmp(buf, "$GPRMC", 6)==0)
            parseRMC(buf, out);
        }
        pos = 0;

        if (out.fix) return true;
      }
      else
      {
        if (pos < sizeof(buf)-1)
          buf[pos++] = c;
      }
    }
  }
  return false;
}
