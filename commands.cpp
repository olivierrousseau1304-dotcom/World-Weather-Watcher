#include "commands.h"
#include "config.h"
#include "rtc_manager.h"
#include <Arduino.h>
#include <string.h>
#include <stdlib.h>

static char buf[32];

static void trim(char* s){
  int n = strlen(s);
  while (n>0 && (s[n-1]=='\r' || s[n-1]=='\n' || s[n-1]==' ' || s[n-1]=='\t')) s[--n]=0;
  while (*s==' ' || *s=='\t') { memmove(s, s+1, strlen(s)); }
}

static bool read_line(){
  if(!Serial.available()) return false;
  int n = Serial.readBytesUntil('\n', buf, sizeof(buf)-1);
  buf[n]=0;
  trim(buf);
  return buf[0]!=0;
}

// GET helpers (imprime valeur)
static bool get_key(const char* k){
  if (!strcmp(k,"LOG"))   { Serial.println(config.LOG_INTERVALL); return true; }
  if (!strcmp(k,"FILE"))  { Serial.println(config.FILE_MAX_SIZE); return true; }
  if (!strcmp(k,"TO"))    { Serial.println(config.TIMEOUT); return true; }
  if (!strcmp(k,"GTO"))   { Serial.println(config.GPS_TIMEOUT); return true; }
  if (!strcmp(k,"GPS"))   { Serial.println(config.GPS); return true; }

  if (!strcmp(k,"LUM"))   { Serial.println(config.LUMIN); return true; }
  if (!strcmp(k,"L_LO"))  { Serial.println(config.LUMIN_LOW); return true; }
  if (!strcmp(k,"L_HI"))  { Serial.println(config.LUMIN_HIGH); return true; }

  if (!strcmp(k,"TMP"))   { Serial.println(config.TEMP_AIR); return true; }
  if (!strcmp(k,"T_MIN")) { Serial.println(config.MIN_TEMP_AIR); return true; }
  if (!strcmp(k,"T_MAX")) { Serial.println(config.MAX_TEMP_AIR); return true; }

  if (!strcmp(k,"HYG"))   { Serial.println(config.HYGR); return true; }
  if (!strcmp(k,"H_MIN")) { Serial.println(config.HYGR_MINT); return true; }
  if (!strcmp(k,"H_MAX")) { Serial.println(config.HYGR_MAXT); return true; }

  if (!strcmp(k,"PRE"))   { Serial.println(config.PRESSURE); return true; }
  if (!strcmp(k,"P_MIN")) { Serial.println(config.PRESSURE_MIN); return true; }
  if (!strcmp(k,"P_MAX")) { Serial.println(config.PRESSURE_MAX); return true; }

  return false;
}

// SET helpers (écrit valeur)
static bool set_key(const char* k, long v){
  if (!strcmp(k,"LOG"))   { config.LOG_INTERVALL = (uint16_t)v; return true; }
  if (!strcmp(k,"FILE"))  { config.FILE_MAX_SIZE = (uint16_t)v; return true; }
  if (!strcmp(k,"TO"))    { config.TIMEOUT = (uint8_t)v; return true; }
  if (!strcmp(k,"GTO"))   { config.GPS_TIMEOUT = (uint8_t)v; return true; }
  if (!strcmp(k,"GPS"))   { config.GPS = (uint8_t)v; return true; }

  if (!strcmp(k,"LUM"))   { config.LUMIN = (uint8_t)v; return true; }
  if (!strcmp(k,"L_LO"))  { config.LUMIN_LOW = (uint16_t)v; return true; }
  if (!strcmp(k,"L_HI"))  { config.LUMIN_HIGH = (uint16_t)v; return true; }

  if (!strcmp(k,"TMP"))   { config.TEMP_AIR = (uint8_t)v; return true; }
  if (!strcmp(k,"T_MIN")) { config.MIN_TEMP_AIR = (int16_t)v; return true; }
  if (!strcmp(k,"T_MAX")) { config.MAX_TEMP_AIR = (int16_t)v; return true; }

  if (!strcmp(k,"HYG"))   { config.HYGR = (uint8_t)v; return true; }
  if (!strcmp(k,"H_MIN")) { config.HYGR_MINT = (int16_t)v; return true; }
  if (!strcmp(k,"H_MAX")) { config.HYGR_MAXT = (int16_t)v; return true; }

  if (!strcmp(k,"PRE"))   { config.PRESSURE = (uint8_t)v; return true; }
  if (!strcmp(k,"P_MIN")) { config.PRESSURE_MIN = (uint16_t)v; return true; }
  if (!strcmp(k,"P_MAX")) { config.PRESSURE_MAX = (uint16_t)v; return true; }

  return false;
}

static void print_config_compact(){
  // Sortie compacte (mode B)
  Serial.print(F("LOG="));  Serial.println(config.LOG_INTERVALL);
  Serial.print(F("FILE=")); Serial.println(config.FILE_MAX_SIZE);
  Serial.print(F("TO="));   Serial.println(config.TIMEOUT);
  Serial.print(F("GTO="));  Serial.println(config.GPS_TIMEOUT);
  Serial.print(F("GPS="));  Serial.println(config.GPS);

  Serial.print(F("LUM="));  Serial.println(config.LUMIN);
  Serial.print(F("L_LO=")); Serial.println(config.LUMIN_LOW);
  Serial.print(F("L_HI=")); Serial.println(config.LUMIN_HIGH);

  Serial.print(F("TMP="));  Serial.println(config.TEMP_AIR);
  Serial.print(F("T_MIN="));Serial.println(config.MIN_TEMP_AIR);
  Serial.print(F("T_MAX="));Serial.println(config.MAX_TEMP_AIR);

  Serial.print(F("HYG="));  Serial.println(config.HYGR);
  Serial.print(F("H_MIN="));Serial.println(config.HYGR_MINT);
  Serial.print(F("H_MAX="));Serial.println(config.HYGR_MAXT);

  Serial.print(F("PRE="));  Serial.println(config.PRESSURE);
  Serial.print(F("P_MIN="));Serial.println(config.PRESSURE_MIN);
  Serial.print(F("P_MAX="));Serial.println(config.PRESSURE_MAX);
}

void commands_poll(){
  if (!read_line()) return;

  // COMMANDES SANS '='
  if (!strcmp(buf, "RESET")) {
    config_reset_defaults();
    config_save();
    Serial.println(F("OK"));
    return;
  }

  if (!strcmp(buf, "CONFIG")) {
    print_config_compact();
    return;
  }

  if (!strcmp(buf, "VERSION")) {
    Serial.println(F("V=5.2 L=A1"));
    return;
  }

  if (!strncmp(buf,"CLOCK ",6)) {
    int hh,mm,ss;
    if (sscanf(buf+6, "%d:%d:%d", &hh,&mm,&ss)==3) rtc_set_time(hh,mm,ss);
    Serial.println(F("OK"));
    return;
  }
  if (!strcmp(buf,"CLOCK")) {
    char t6[7]; rtc_get_time_HHMMSS(t6); Serial.println(t6); return;
  }

  if (!strncmp(buf,"DATE ",5)) {
    int M,D,Y;
    if (sscanf(buf+5, "%d,%d,%d", &M,&D,&Y)==3) rtc_set_date(M,D,Y);
    Serial.println(F("OK"));
    return;
  }
  if (!strcmp(buf,"DATE")) {
    char d6[7]; rtc_get_date_YYMMDD(d6); Serial.println(d6); return;
  }

  if (!strncmp(buf,"DAY ",4)) {
    rtc_set_day(buf+4);
    Serial.println(F("OK"));
    return;
  }

  // KEY=VALUE
  char* eq = strchr(buf,'=');
  if (eq) {
    *eq = 0;
    const char* key = buf;
    const char* sval = eq+1;
    long v = strtol(sval, NULL, 10);
    if (set_key(key, v)) {
      config_save();
      Serial.println(F("OK"));
    } else {
      Serial.println(F("ERR"));
    }
    return;
  }

  // KEY (GET)
  if (get_key(buf)) return;

  Serial.println(F("ERR"));
}
