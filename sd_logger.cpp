#include "sd_logger.h"
#include <SdFat.h>
#include "led_manager.h"
#include "rtc_manager.h"

// ===================================================
//   SD STATE
// ===================================================
static SdFat   sd;
static File    logFile;
static bool    sdReady = false;
static uint32_t maxSize = 2048UL;

static char currentFile[20] = {0};

// ===================================================
// Print float WITHOUT float.print()
// ===================================================
static void printFixed(File &f, float v, uint8_t decimals)
{
    if (v < 0) { f.write('-'); v = -v; }

    uint32_t mult = 1;
    for (uint8_t i=0; i<decimals; i++) mult *= 10;

    uint32_t iv = (uint32_t)(v * mult + 0.5f);
    uint32_t ip = iv / mult;
    uint32_t fp = iv % mult;

    f.print(ip);
    if (!decimals) return;

    f.write('.');
    uint32_t div = mult / 10;
    while (div > 0) {
        if (fp < div) f.write('0');
        div /= 10;
    }
    f.print(fp);
}


// ===================================================
//  HEADER
// ===================================================
static void writeHeader(File &f)
{
    f.println(F("date,time,lat,lon,alt,sats,speed,temp,hum,press,lum"));
}


// ===================================================
//  CREATE _0 FILE OF THE DAY
// ===================================================
static bool openTodayFile()
{
    char date[9];
    rtc_get_date_YYMMDD(date);

    snprintf(currentFile, sizeof(currentFile),
             "%s_0.LOG", date);

    logFile = sd.open(currentFile, FILE_WRITE);
    if (!logFile) return false;

    writeHeader(logFile);
    logFile.flush();
    return true;
}


// ===================================================
//  ROTATION
// ===================================================
static bool rotateFile()
{
    if (logFile) logFile.close();

    char date[9];
    rtc_get_date_YYMMDD(date);

    // search LOG_1.LOG ... LOG_n.LOG
    for (uint8_t i = 1; i < 50; i++)
    {
        char newName[20];
        snprintf(newName, sizeof(newName), "%s_%u.LOG", date, i);

        // if free → rename
        if (!sd.exists(newName))
        {
            sd.rename(currentFile, newName);
            return openTodayFile();
        }
    }

    // Too many rotations
    led_pattern_sd_full();
    return false;
}


// ===================================================
//  API IMPLEMENTATION
// ===================================================
bool sd_init(uint8_t csPin)
{
    if (!sd.begin(csPin))
    {
        sdReady = false;
        return false;
    }

    sdReady = openTodayFile();
    return sdReady;
}

bool sd_is_ready()
{
    return sdReady;
}


void sd_close()
{
    if (logFile) logFile.close();
    sdReady = false;
}

void sd_set_max_file_size(uint32_t bytes)
{
    maxSize = bytes;
}

uint32_t sd_get_current_size()
{
    return logFile ? logFile.size() : 0;
}

const char* sd_get_current_filename()
{
    return currentFile;
}


// ===================================================
//  WRITE CSV LINE
// ===================================================
bool sd_append_csv(const GpsData &gps,
                   const BmeData &bme,
                   const LightData &l,
                   const char* dStr,
                   const char* tStr)
{
    if (!sdReady) return false;

    // Rotate if full
    if (logFile.size() >= maxSize)
    {
        if (!rotateFile())
        {
            led_pattern_sd_write_error();
            return false;
        }
    }

    // date,time
    logFile.print(dStr);
    logFile.write(',');
    logFile.print(tStr);
    logFile.write(',');

    // GPS
    if (gps.fix) {
        printFixed(logFile, gps.lat,   6); logFile.write(',');
        printFixed(logFile, gps.lon,   6); logFile.write(',');
        printFixed(logFile, gps.alt,   2); logFile.write(',');
        logFile.print(gps.sats);        logFile.write(',');
        printFixed(logFile, gps.speed, 2); logFile.write(',');
    } else {
        logFile.print(F("NA,NA,NA,NA,NA,"));
    }

    // BME
    if (bme.ok) {
        printFixed(logFile, bme.temperature, 2); logFile.write(',');
        printFixed(logFile, bme.humidity,    2); logFile.write(',');
        printFixed(logFile, bme.pressure,    2); logFile.write(',');
    } else {
        logFile.print(F("NA,NA,NA,"));
    }

    // Luminosité
    if (l.ok) {
        logFile.print(l.value);
        logFile.write('\n');
    } else {
        logFile.print(F("NA\n"));
    }

    logFile.flush();
    return true;
}
