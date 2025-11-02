#include <Arduino.h>
#include <string.h>
#include <avr/pgmspace.h>

#include "commands.h"
#include "config.h"
#include "rtc_manager.h"

static const char VERSION_STR[] PROGMEM = "3W_V5.2";

static char cmdBuf[40];
static uint8_t cmdIdx = 0;

static void trimNewline()
{
    for (uint8_t i = 0; i < cmdIdx; i++)
        if (cmdBuf[i] == '\r' || cmdBuf[i] == '\n')
            cmdBuf[i] = 0;
}

// ============================
//  Set RTC CLOCK HH:MM:SS
// ============================
static void cmdClock(const char* s)
{
    int hh, mm, ss;
    if (sscanf(s, "%d:%d:%d", &hh, &mm, &ss) != 3) {
        Serial.println(F("ERR CLOCK"));
        return;
    }

    // FUTURE WORK: rtc_set_time(hh,mm,ss)
    Serial.println(F("OK"));
}

// ============================
//  Set RTC DATE MM:DD:YYYY
// ============================
static void cmdDate(const char* s)
{
    int mm, dd, yyyy;
    if (sscanf(s, "%d:%d:%d", &mm, &dd, &yyyy) != 3) {
        Serial.println(F("ERR DATE"));
        return;
    }

    // FUTURE WORK: rtc_set_date(mm,dd,yyyy)
    Serial.println(F("OK"));
}

// ============================
//  Set DAY
// ============================
static void cmdDay(const char* s)
{
    // MON/TUE/WED/THU/FRI/SAT/SUN
    // FUTURE WORK: rtc_set_day()

    Serial.println(F("OK"));
}


// ============================
//   MAIN PARSER
// ============================
void commands_poll()
{
    if (!Serial.available())
        return;

    char c = Serial.read();
    if (c != '\r' && c != '\n')
    {
        if (cmdIdx < sizeof(cmdBuf) - 1)
            cmdBuf[cmdIdx++] = c;
        return;
    }

    // fin de ligne
    cmdBuf[cmdIdx] = 0;
    cmdIdx = 0;
    trimNewline();

    if (!cmdBuf[0]) return;    // empty

    // =====================================
    // RESET
    // =====================================
    if (!strcmp(cmdBuf, "RESET"))
    {
        config_reset_defaults();
        Serial.println(F("OK RESET"));
        return;
    }

    // =====================================
    // VERSION
    // =====================================
    if (!strcmp(cmdBuf, "VERSION"))
    {
        Serial.print(F("VERSION="));
        Serial.println(VERSION_STR);
        return;
    }

    // =====================================
    // CLOCK HH:MM:SS
    // =====================================
    if (!strncmp(cmdBuf, "CLOCK ", 6))
    {
        cmdClock(cmdBuf + 6);
        return;
    }

    // =====================================
    // DATE MM:DD:YYYY
    // =====================================
    if (!strncmp(cmdBuf, "DATE ", 5))
    {
        cmdDate(cmdBuf + 5);
        return;
    }

    // =====================================
    // DAY XXX
    // =====================================
    if (!strncmp(cmdBuf, "DAY ", 4))
    {
        cmdDay(cmdBuf + 4);
        return;
    }

    // =====================================
    // KEY=VALUE
    // =====================================
    char* eq = strchr(cmdBuf, '=');
    if (eq)
    {
        *eq = 0;
        const char* key = cmdBuf;
        int val = atoi(eq + 1);

        if (config_set_param(String(key), val))
        {
            config_save();
            Serial.println(F("OK"));
        }
        else
        {
            Serial.println(F("ERR"));
        }
        return;
    }

    Serial.println(F("ERR"));
}
