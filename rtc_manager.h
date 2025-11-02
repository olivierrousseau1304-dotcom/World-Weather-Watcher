#pragma once
#include <Arduino.h>

struct RtcDateTime {
  uint8_t day, month, year, hour, minute, second; // year = 00..99
};

void rtc_init();
bool rtc_is_available();
void rtc_set_datetime(uint8_t day, uint8_t month, uint8_t year, uint8_t hour, uint8_t minute, uint8_t second);
bool rtc_get_datetime(RtcDateTime &out);
String rtc_get_date_str();  // "YYYY-MM-DD"
String rtc_get_time_str();  // "HH:MM:SS"
