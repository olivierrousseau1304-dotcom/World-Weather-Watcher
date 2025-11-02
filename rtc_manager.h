#ifndef RTC_MANAGER_H
#define RTC_MANAGER_H

#include <Arduino.h>

// ============================
// API
// ============================
void rtc_init();
bool rtc_is_available();

// "YYYY-MM-DD"
String rtc_get_date_str();

// "HH:MM:SS"
String rtc_get_time_str();

// === Optimised (no String) ===
// Fill buffer: "YYMMDD"   => must be >= 7 bytes
void rtc_get_date_YYMMDD(char* out6);

// Fill buffer: "HHMMSS"   => must be >= 7 bytes
void rtc_get_time_HHMMSS(char* out6);

#endif
