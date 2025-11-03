#ifndef RTC_MANAGER_H
#define RTC_MANAGER_H

#include <stdint.h>

bool rtc_init();
bool rtc_is_available();

void rtc_get_date_YYMMDD(char* out6);   // "YYMMDD" + '\0'
void rtc_get_time_HHMMSS(char* out6);   // "HHMMSS" + '\0'

void rtc_set_time(uint8_t hh, uint8_t mm, uint8_t ss);
void rtc_set_date(uint8_t month, uint8_t day, uint16_t year);
void rtc_set_day(const char* dow3);

#endif
