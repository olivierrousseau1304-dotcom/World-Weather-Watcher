#include "rtc_manager.h"
#include <Wire.h>

#define DS3231_ADDR 0x68

static bool rtcOk = false;

// ------------------------------
// BCD conversion
// ------------------------------
static uint8_t bcdToDec(uint8_t v)
{
    return (v >> 4) * 10 + (v & 0x0F);
}

// ------------------------------
// Init
// ------------------------------
void rtc_init()
{
    Wire.begin();
    Wire.beginTransmission(DS3231_ADDR);
    rtcOk = (Wire.endTransmission() == 0);
}

bool rtc_is_available()
{
    return rtcOk;
}

// ------------------------------
// Core read (internal)
// ------------------------------
static bool readAll(
    uint8_t &sec,
    uint8_t &minu,
    uint8_t &hour,
    uint8_t &day,
    uint8_t &month,
    uint8_t &year
)
{
    if (!rtcOk) return false;

    Wire.beginTransmission(DS3231_ADDR);
    Wire.write(0x00);
    Wire.endTransmission();

    Wire.requestFrom(DS3231_ADDR, 7);
    if (Wire.available() < 7) return false;

    sec   = bcdToDec(Wire.read() & 0x7F);
    minu  = bcdToDec(Wire.read());
    hour  = bcdToDec(Wire.read());
    Wire.read(); // ignore day of week
    day   = bcdToDec(Wire.read());
    month = bcdToDec(Wire.read());
    year  = bcdToDec(Wire.read()); // 0–99 => 20xx

    return true;
}

// ------------------------------
// YYYY-MM-DD (String)
// ------------------------------
String rtc_get_date_str()
{
    uint8_t s,m,h,day,month,year;
    if (!readAll(s,m,h,day,month,year))
        return String("0000-00-00");

    char buf[11];
    snprintf(buf, sizeof(buf), "20%02u-%02u-%02u", year, month, day);
    return String(buf);
}

// ------------------------------
// HH:MM:SS (String)
// ------------------------------
String rtc_get_time_str()
{
    uint8_t s,m,h,day,month,year;
    if (!readAll(s,m,h,day,month,year))
        return String("00:00:00");

    char buf[9];
    snprintf(buf, sizeof(buf), "%02u:%02u:%02u", h, m, s);
    return String(buf);
}

// ------------------------------
// YYMMDD (C-string)
// ------------------------------
void rtc_get_date_YYMMDD(char* out6)
{
    uint8_t s,m,h,day,month,year;
    if (!readAll(s,m,h,day,month,year)) {
        strcpy(out6, "000000");
        return;
    }

    // year is 0–99 → keep only last 2 digits
    snprintf(out6, 7, "%02u%02u%02u", year, month, day);
}

// ------------------------------
// HHMMSS (C-string)
// ------------------------------
void rtc_get_time_HHMMSS(char* out6)
{
    uint8_t s,m,h,day,month,year;
    if (!readAll(s,m,h,day,month,year)) {
        strcpy(out6, "000000");
        return;
    }

    snprintf(out6, 7, "%02u%02u%02u", h, m, s);
}
