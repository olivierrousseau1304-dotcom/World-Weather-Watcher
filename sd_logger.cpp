#include "sd_logger.h"
#include <SPI.h>
#include <SD.h>
#include "config.h"
#include "rtc_manager.h"
#include "led_manager.h"

static File   logFile;
static bool   sdReady = false;
static uint8_t sdCS = 4;
static uint32_t maxSize = 4096;
static char   baseName[9];     // "YYMMDD"
static char   currentName[13]; // "YYMMDD_0.LOG"

static void ensureSpiMasterPin(){ pinMode(10, OUTPUT); }

static void makeBaseName(){
  String d = rtc_get_date_str(); // "YYYY-MM-DD"
  if(d.length() >= 10){
    String yy = d.substring(2,4), mm = d.substring(5,7), dd = d.substring(8,10);
    snprintf(baseName, sizeof(baseName), "%s%s%s", yy.c_str(), mm.c_str(), dd.c_str());
  } else snprintf(baseName, sizeof(baseName), "000000");
}

static void makeRevisionName(uint8_t rev){
  snprintf(currentName, sizeof(currentName), "%s_%u.LOG", baseName, rev);
}

static bool openZero(){
  makeRevisionName(0);
  bool exists = SD.exists(currentName);
  logFile = SD.open(currentName, FILE_WRITE);
  if(!logFile) return false;
  if(!exists){
    logFile.println(F("date,utc,lat,lon,alt,sats,speed"));
    logFile.flush();
  }
  return true;
}

static bool copyFile(const char* src, const char* dst){
  File s = SD.open(src, FILE_READ); if(!s) return false;
  File d = SD.open(dst, FILE_WRITE); if(!d){ s.close(); return false; }
  uint8_t buf[64];
  while(s.available()){
    int n = s.read(buf, sizeof(buf)); if(n <= 0) break;
    d.write(buf, n);
  }
  d.flush(); d.close(); s.close();
  return true;
}

static bool rotateCopy(){
  uint16_t r = 1; char dst[13];
  while(true){
    snprintf(dst, sizeof(dst), "%s_%u.LOG", baseName, r);
    if(!SD.exists(dst)) break;
    if(++r > 999) return false;
  }
  logFile.flush(); logFile.close();
  char zero[13]; snprintf(zero, sizeof(zero), "%s_0.LOG", baseName);
  if(!copyFile(zero, dst)) return false;

  logFile = SD.open(zero, FILE_WRITE);
  if(!logFile) return false;
  logFile.println(F("date,utc,lat,lon,alt,sats,speed"));
  logFile.flush();
  return true;
}

bool sd_init(uint8_t csPin){
  sdCS = csPin; ensureSpiMasterPin();
  if(!SD.begin(sdCS)){ sdReady = false; return false; }
  maxSize = (config.FILE_MAX_SIZE > 0) ? (uint32_t)config.FILE_MAX_SIZE : 4096;
  makeBaseName();
  if(!openZero()){ sdReady = false; return false; }
  sdReady = true; return true;
}

bool sd_is_ready(){ return sdReady && logFile; }

void sd_close(){ if(logFile){ logFile.flush(); logFile.close(); } sdReady = false; }

void sd_set_max_file_size(uint32_t bytes){ maxSize = bytes; }
uint32_t sd_get_current_size(){ return logFile ? (uint32_t)logFile.size() : 0; }
const char* sd_get_current_filename(){ return currentName; }

static bool rotateIfNeeded(){
  if(!logFile) return false;
  if((uint32_t)logFile.size() < maxSize) return true;
  if(!rotateCopy()){ led_pattern_sd_full(); return false; }
  return true;
}

bool sd_append_csv(const GpsData& g, const String& dateStr, const String& timeStr){
  if(!sd_is_ready()) return false;
  if(!rotateIfNeeded()) return false;
  if(!logFile){ led_pattern_sd_write_error(); return false; }

  // date,utc,lat,lon,alt,sats,speed
  if(dateStr.length()) logFile.print(dateStr); else logFile.print(F("NA")); logFile.print(',');
  if(timeStr.length()) logFile.print(timeStr); else logFile.print(F("NA")); logFile.print(',');
  if(g.fix) logFile.print(g.lat, 6); else logFile.print(F("NA")); logFile.print(',');
  if(g.fix) logFile.print(g.lon, 6); else logFile.print(F("NA")); logFile.print(',');
  if(g.fix) logFile.print(g.alt, 1); else logFile.print(F("NA")); logFile.print(',');
  logFile.print(g.sats); logFile.print(',');
  if(g.fix) logFile.println(g.speed, 1); else logFile.println(F("NA"));

  logFile.flush();
  return true;
}
