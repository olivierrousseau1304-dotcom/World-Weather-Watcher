#include "config.h"
#include <EEPROM.h>

// ========================================================
// EEPROM layout
// On stocke la struct telle quelle
// ========================================================

Config config = {
    10,     // LOG_INTERVALL (min)
    30,     // TIMEOUT (s)
    4096,   // FILE_MAX_SIZE (octets)
    1,      // GPS actif
    5       // GPS_TIMEOUT (s)
};

// --------------------------------------------------------
static void eeprom_read(Config& c)
{
    EEPROM.get(0, c);

    // sécurité si EEPROM vide / incohérente
    if (c.LOG_INTERVALL <= 0 || c.LOG_INTERVALL > 1440)
        c.LOG_INTERVALL = 10;

    if (c.TIMEOUT <= 0 || c.TIMEOUT > 3600)
        c.TIMEOUT = 30;

    if (c.FILE_MAX_SIZE < 512 || c.FILE_MAX_SIZE > 32767)
        c.FILE_MAX_SIZE = 4096;

    if (c.GPS < 0 || c.GPS > 1)
        c.GPS = 1;

    if (c.GPS_TIMEOUT <= 0 || c.GPS_TIMEOUT > 60)
        c.GPS_TIMEOUT = 5;
}

// --------------------------------------------------------
static void eeprom_write(const Config& c)
{
    EEPROM.put(0, c);
}

// --------------------------------------------------------
void config_init()
{
    eeprom_read(config);
}

// --------------------------------------------------------
void config_save()
{
    eeprom_write(config);
}
