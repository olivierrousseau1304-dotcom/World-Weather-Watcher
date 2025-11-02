#include "rtc_manager.h"
#include <Wire.h>
#define DS3231_ADDRESS 0x68

static bool rtcOk = false;
static byte decToBcd(byte v){ return ( (v/10*16) + (v%10) ); }
static byte bcdToDec(byte v){ return ( (v/16*10) + (v%16) ); }

void rtc_init(){
  Wire.begin();
  Wire.beginTransmission(DS3231_ADDRESS);
  rtcOk = (Wire.endTransmission() == 0);
}

bool rtc_is_available(){ return rtcOk; }

void rtc_set_datetime(uint8_t day, uint8_t month, uint8_t year, uint8_t hour, uint8_t minute, uint8_t second){
  Wire.beginTransmission(DS3231_ADDRESS);
  Wire.write(0x00);
  Wire.write(decToBcd(second));
  Wire.write(decToBcd(minute));
  Wire.write(decToBcd(hour));
  Wire.write(0);
  Wire.write(decToBcd(day));
  Wire.write(decToBcd(month));
  Wire.write(decToBcd(year));
  Wire.endTransmission();
}

bool rtc_get_datetime(RtcDateTime &out){
  if(!rtcOk) return false;
  Wire.beginTransmission(DS3231_ADDRESS);
  Wire.write(0x00);
  Wire.endTransmission();
  Wire.requestFrom(DS3231_ADDRESS, 7);
  if(Wire.available() < 7) return false;
  out.second = bcdToDec(Wire.read() & 0x7F);
  out.minute = bcdToDec(Wire.read());
  out.hour   = bcdToDec(Wire.read());
  Wire.read();
  out.day    = bcdToDec(Wire.read());
  out.month  = bcdToDec(Wire.read());
  out.year   = bcdToDec(Wire.read());
  return true;
}

String rtc_get_date_str(){
  RtcDateTime t; if(!rtc_get_datetime(t)) return "0000-00-00";
  char buf[11]; snprintf(buf, sizeof(buf), "20%02u-%02u-%02u", t.year, t.month, t.day);
  return String(buf);
}

String rtc_get_time_str(){
  RtcDateTime t; if(!rtc_get_datetime(t)) return "00:00:00";
  char buf[9]; snprintf(buf, sizeof(buf), "%02u:%02u:%02u", t.hour, t.minute, t.second);
  return String(buf);
}
