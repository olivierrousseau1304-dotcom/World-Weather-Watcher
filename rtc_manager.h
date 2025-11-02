#ifndef RTC_MANAGER_H
#define RTC_MANAGER_H

#include <Arduino.h>

// =====================================================
// RTC – DS3231
// API publique
// =====================================================

void   rtc_init();
bool   rtc_is_available();

// Renvoie "YYYY-MM-DD"
String rtc_get_date_str();

// Renvoie "HH:MM:SS"
String rtc_get_time_str();

#endif
