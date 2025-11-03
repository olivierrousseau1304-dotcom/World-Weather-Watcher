#include "rtc_manager.h"
#include <RTClib.h>
#include <string.h>

static RTC_DS3231 rtc;
static bool rtc_ok = false;

// Stock DOW (3 chars + '\0')
static char dowStore[4] = "MON";

bool rtc_init()
{
    if (!rtc.begin()) {
        rtc_ok = false;
        return false;
    }

    if (rtc.lostPower()) {
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }

    rtc_ok = true;
    return true;
}

bool rtc_is_available()
{
    return rtc_ok;
}

// ---------------------------------------------------------
// GET DATE → "YYMMDD"
// ---------------------------------------------------------
void rtc_get_date_YYMMDD(char* out6)
{
    if (!rtc_ok) {
        strcpy(out6, "000000");
        return;
    }

    DateTime now = rtc.now();
    snprintf(out6, 7, "%02d%02d%02d",
        now.year() % 100,
        now.month(),
        now.day());
}

// ---------------------------------------------------------
// GET TIME → "HHMMSS"
// ---------------------------------------------------------
void rtc_get_time_HHMMSS(char* out6)
{
    if (!rtc_ok) {
        strcpy(out6, "000000");
        return;
    }

    DateTime now = rtc.now();
    snprintf(out6, 7, "%02d%02d%02d",
        now.hour(),
        now.minute(),
        now.second());
}

// ---------------------------------------------------------
// SET TIME → CLOCK HH:MM:SS
// ---------------------------------------------------------
void rtc_set_time(uint8_t hh, uint8_t mm, uint8_t ss)
{
    if (!rtc_ok) return;

    DateTime now = rtc.now();
    rtc.adjust(DateTime(
        now.year(),
        now.month(),
        now.day(),
        hh, mm, ss
    ));
}

// ---------------------------------------------------------
// SET DATE → DATE MM,DD,YYYY
// ---------------------------------------------------------
void rtc_set_date(uint8_t month, uint8_t day, uint16_t year)
{
    if (!rtc_ok) return;

    DateTime now = rtc.now();
    rtc.adjust(DateTime(
        year,
        month,
        day,
        now.hour(),
        now.minute(),
        now.second()
    ));
}

// ---------------------------------------------------------
// SET DAY → DAY MON / TUE / ...
// ---------------------------------------------------------
void rtc_set_day(const char* dow3)
{
    if (!dow3) return;

    dowStore[0] = dow3[0];
    dowStore[1] = dow3[1];
    dowStore[2] = dow3[2];
    dowStore[3] = 0;
}
