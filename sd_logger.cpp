#include "sd_logger.h"
#include <SD.h>
#include "rtc_manager.h"
#include "led_manager.h"

static File     logFile;
static bool     sd_ok = false;
static uint8_t  sd_cs = 4;
static uint32_t maxFileSize = 4096;
static char     currentName[20];   // "YYMMDD_0.LOG"

// -------------------------------------------------------------
// Helpers : noms basés sur la date RTC
// -------------------------------------------------------------
static void makeDailyBaseName(char* out) {
    String ds = rtc_get_date_str(); // "YYYY-MM-DD" ou "0000-00-00"
    const char* y = ds.c_str();

    char YY[3] = "00";
    char MM[3] = "00";
    char DD[3] = "00";

    if (ds.length() >= 10) {
        YY[0] = y[2]; YY[1] = y[3];
        MM[0] = y[5]; MM[1] = y[6];
        DD[0] = y[8]; DD[1] = y[9];
    }
    sprintf(out, "%s%s%s_0.LOG", YY, MM, DD);
}

static void makePrefix(char* out) {
    String ds = rtc_get_date_str();
    const char* y = ds.c_str();
    char YY[3] = "00", MM[3] = "00", DD[3] = "00";
    if (ds.length() >= 10) {
        YY[0] = y[2]; YY[1] = y[3];
        MM[0] = y[5]; MM[1] = y[6];
        DD[0] = y[8]; DD[1] = y[9];
    }
    sprintf(out, "%s%s%s", YY, MM, DD);
}

// -------------------------------------------------------------
void sd_set_max_file_size(uint32_t bytes) { maxFileSize = bytes; }

// -------------------------------------------------------------
bool sd_init(uint8_t cs) {
    sd_cs = cs;

    // IMPORTANT UNO: rester en SPI master
    pinMode(10, OUTPUT);
    digitalWrite(10, HIGH);

    pinMode(sd_cs, OUTPUT);
    if (!SD.begin(sd_cs)) {
        sd_ok = false;
        return false;
    }
    sd_ok = true;

    makeDailyBaseName(currentName);

    // Crée _0.LOG du jour s'il n'existe pas (avec en-tête CSV)
    if (!SD.exists(currentName)) {
        logFile = SD.open(currentName, FILE_WRITE);
        if (!logFile) { sd_ok = false; return false; }
        logFile.println(F("date,utc,lat,lon,alt,sats,speed"));
        logFile.flush();
        logFile.close();
    }
    return true;
}

// -------------------------------------------------------------
bool sd_is_ready() { return sd_ok; }

// -------------------------------------------------------------
void sd_close() {
    if (logFile) { logFile.flush(); logFile.close(); }
}

// -------------------------------------------------------------
uint32_t sd_get_current_size() {
    if (!sd_ok) return 0;
    File f = SD.open(currentName, FILE_READ);
    if (!f) return 0;
    uint32_t s = f.size();
    f.close();
    return s;
}

// -------------------------------------------------------------
const char* sd_get_current_filename() { return currentName; }

// -------------------------------------------------------------
static bool copySmallFile(const char* src, const char* dst) {
    File in = SD.open(src, FILE_READ);
    if (!in) return false;
    File out = SD.open(dst, FILE_WRITE);
    if (!out) { in.close(); return false; }

    char buf[64];
    int n;
    while ((n = in.read(buf, sizeof(buf))) > 0) {
        if (out.write(buf, n) != n) { in.close(); out.close(); return false; }
    }
    in.close(); out.flush(); out.close();
    return true;
}

// -------------------------------------------------------------
static bool rotateFiles() {
    char prefix[10];
    makePrefix(prefix);

    // Cherche le prochain index libre
    for (int k = 1; k < 100; k++) {
        char newName[20];
        snprintf(newName, sizeof(newName), "%s_%d.LOG", prefix, k);
        if (!SD.exists(newName)) {
            // copie _0 -> _k
            if (!copySmallFile(currentName, newName)) {
                led_pattern_sd_write_error(); // signal fort
                return false;
            }
            // recrée _0 avec en-tête
            logFile = SD.open(currentName, FILE_WRITE);
            if (!logFile) { sd_ok = false; return false; }
            logFile.println(F("date,utc,lat,lon,alt,sats,speed"));
            logFile.flush();
            logFile.close();
            return true;
        }
    }
    // Pas de slot libre : SD pleine / plancher de rotation atteint
    led_pattern_sd_full();
    return false;
}

// -------------------------------------------------------------
bool sd_append_csv(const GpsData& g, const String& dateStr, const String& timeStr) {
    if (!sd_ok) return false;

    // Rotation si taille > max
    if (sd_get_current_size() > maxFileSize) {
        if (!rotateFiles()) return false;
    }

    logFile = SD.open(currentName, FILE_WRITE);
    if (!logFile) { sd_ok = false; led_pattern_sd_write_error(); return false; }

    char latBuf[16]   = "NA";
    char lonBuf[16]   = "NA";
    char altBuf[16]   = "NA";
    char satsBuf[8]   = "NA";
    char speedBuf[16] = "NA";

    if (g.fix) {
        dtostrf(g.lat,   1, 6, latBuf);
        dtostrf(g.lon,   1, 6, lonBuf);
        dtostrf(g.alt,   1, 2, altBuf);
        snprintf(satsBuf, sizeof(satsBuf), "%u", g.sats);
        dtostrf(g.speed, 1, 2, speedBuf);
    }

    // CSV : date,utc,lat,lon,alt,sats,speed
    logFile.print(dateStr);  logFile.print(',');
    logFile.print(timeStr);  logFile.print(',');
    logFile.print(latBuf);   logFile.print(',');
    logFile.print(lonBuf);   logFile.print(',');
    logFile.print(altBuf);   logFile.print(',');
    logFile.print(satsBuf);  logFile.print(',');
    logFile.println(speedBuf);

    logFile.flush();
    logFile.close();
    return true;
}
