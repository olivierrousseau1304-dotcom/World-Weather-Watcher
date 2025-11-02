#include "rtc_manager.h"
#include <Wire.h>

// ===============================
// DS3231 I2C
// ===============================
#define DS3231_ADDRESS 0x68

static bool rtcOk = false;

// -----------------------------------------------------
// Convertisseur BCD
// -----------------------------------------------------
static byte bcdToDec(byte val)
{
    return ( (val / 16 * 10) + (val % 16) );
}

// -----------------------------------------------------
// Init RTC
// -----------------------------------------------------
void rtc_init()
{
    Wire.begin();

    Wire.beginTransmission(DS3231_ADDRESS);
    rtcOk = (Wire.endTransmission() == 0);
}

// -----------------------------------------------------
bool rtc_is_available()
{
    return rtcOk;
}

// -----------------------------------------------------
// Lecture brute registre DS3231
// renvoie YYYY-MM-DD
// -----------------------------------------------------
String rtc_get_date_str()
{
    if (!rtcOk) return String("0000-00-00");

    Wire.beginTransmission(DS3231_ADDRESS);
    Wire.write(0x00);
    Wire.endTransmission();

    Wire.requestFrom(DS3231_ADDRESS, 7);

    byte second = bcdToDec(Wire.read() & 0x7F);
    byte minute = bcdToDec(Wire.read());
    byte hour   = bcdToDec(Wire.read());
    Wire.read(); // jour semaine (on ignore)
    byte day    = bcdToDec(Wire.read());
    byte month  = bcdToDec(Wire.read());
    byte year   = bcdToDec(Wire.read()); // 0–99

    char buf[11];
    // "YYYY-MM-DD"
    snprintf(buf, sizeof(buf), "20%02d-%02d-%02d", year, month, day);

    return String(buf);
}

// -----------------------------------------------------
// Lecture HH:MM:SS
// -----------------------------------------------------
String rtc_get_time_str()
{
    if (!rtcOk) return String("00:00:00");

    Wire.beginTransmission(DS3231_ADDRESS);
    Wire.write(0x00);
    Wire.endTransmission();

    Wire.requestFrom(DS3231_ADDRESS, 7);

    byte second = bcdToDec(Wire.read() & 0x7F);
    byte minute = bcdToDec(Wire.read());
    byte hour   = bcdToDec(Wire.read());

    char buf[9];
    // "HH:MM:SS"
    snprintf(buf, sizeof(buf), "%02d:%02d:%02d", hour, minute, second);

    return String(buf);
}
