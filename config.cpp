#include "config.h"
#include <EEPROM.h>
#include <avr/pgmspace.h>

Config config;

// =====================================================
// PARAM DEFINITIONS (PROGMEM)
// =====================================================

static const char N_LOG_INTERVALL[] PROGMEM = "LOG_INTERVALL";
static const char N_TIMEOUT[]       PROGMEM = "TIMEOUT";
static const char N_FILE_MAX_SIZE[] PROGMEM = "FILE_MAX_SIZE";
static const char N_GPS[]           PROGMEM = "GPS";
static const char N_GPS_TIMEOUT[]   PROGMEM = "GPS_TIMEOUT";

static const char N_LUMIN[]         PROGMEM = "LUMIN";
static const char N_TEMP_AIR[]      PROGMEM = "TEMP_AIR";
static const char N_HYGR[]          PROGMEM = "HYGR";
static const char N_PRESSURE[]      PROGMEM = "PRESSURE";

static const char N_LUMIN_LOW[]     PROGMEM = "LUMIN_LOW";
static const char N_LUMIN_HIGH[]    PROGMEM = "LUMIN_HIGH";

static const char N_MIN_TEMP_AIR[]  PROGMEM = "MIN_TEMP_AIR";
static const char N_MAX_TEMP_AIR[]  PROGMEM = "MAX_TEMP_AIR";

static const char N_HYGR_MINT[]     PROGMEM = "HYGR_MINT";
static const char N_HYGR_MAXT[]     PROGMEM = "HYGR_MAXT";

static const char N_PRESSURE_MIN[]  PROGMEM = "PRESSURE_MIN";
static const char N_PRESSURE_MAX[]  PROGMEM = "PRESSURE_MAX";

enum ParamId : uint8_t {
  PID_LOG_INTERVALL = 0,
  PID_TIMEOUT,
  PID_FILE_MAX_SIZE,
  PID_GPS,
  PID_GPS_TIMEOUT,
  PID_LUMIN,
  PID_TEMP_AIR,
  PID_HYGR,
  PID_PRESSURE,
  PID_LUMIN_LOW,
  PID_LUMIN_HIGH,
  PID_MIN_TEMP_AIR,
  PID_MAX_TEMP_AIR,
  PID_HYGR_MINT,
  PID_HYGR_MAXT,
  PID_PRESSURE_MIN,
  PID_PRESSURE_MAX,
  PID_COUNT
};

struct ParamMetaPGM {
  PGM_P name;
  int16_t minV;
  int16_t maxV;
  int16_t defV;
  uint8_t id;
};

static const ParamMetaPGM PARAMS[] PROGMEM = {
  { N_LOG_INTERVALL,   1,   60,    10,  PID_LOG_INTERVALL },
  { N_TIMEOUT,         1,  300,    30,  PID_TIMEOUT },
  { N_FILE_MAX_SIZE, 512, 32767, 4096,  PID_FILE_MAX_SIZE },
  { N_GPS,             0,    1,     1,  PID_GPS },
  { N_GPS_TIMEOUT,     1,   60,     5,  PID_GPS_TIMEOUT },

  { N_LUMIN,           0,    1,     1,  PID_LUMIN },
  { N_TEMP_AIR,        0,    1,     1,  PID_TEMP_AIR },
  { N_HYGR,            0,    1,     1,  PID_HYGR },
  { N_PRESSURE,        0,    1,     1,  PID_PRESSURE },

  { N_LUMIN_LOW,       0, 1023,   10,  PID_LUMIN_LOW },
  { N_LUMIN_HIGH,      0, 1023,  900,  PID_LUMIN_HIGH },

  { N_MIN_TEMP_AIR,  -50,   80,   -10,  PID_MIN_TEMP_AIR },
  { N_MAX_TEMP_AIR,  -50,   80,    60,  PID_MAX_TEMP_AIR },

  { N_HYGR_MINT,       0,  100,    20,  PID_HYGR_MINT },
  { N_HYGR_MAXT,       0,  100,    90,  PID_HYGR_MAXT },

  { N_PRESSURE_MIN,  800, 1200,   900,  PID_PRESSURE_MIN },
  { N_PRESSURE_MAX,  800, 1200,  1100,  PID_PRESSURE_MAX },
};

static char nameBuf[22];

static int* ptrForId(uint8_t id)
{
  switch (id)
  {
    case PID_LOG_INTERVALL:  return &config.LOG_INTERVALL;
    case PID_TIMEOUT:        return &config.TIMEOUT;
    case PID_FILE_MAX_SIZE:  return &config.FILE_MAX_SIZE;
    case PID_GPS:            return &config.GPS;
    case PID_GPS_TIMEOUT:    return &config.GPS_TIMEOUT;

    case PID_LUMIN:          return &config.LUMIN;
    case PID_TEMP_AIR:       return &config.TEMP_AIR;
    case PID_HYGR:           return &config.HYGR;
    case PID_PRESSURE:       return &config.PRESSURE;

    case PID_LUMIN_LOW:      return &config.LUMIN_LOW;
    case PID_LUMIN_HIGH:     return &config.LUMIN_HIGH;

    case PID_MIN_TEMP_AIR:   return &config.MIN_TEMP_AIR;
    case PID_MAX_TEMP_AIR:   return &config.MAX_TEMP_AIR;

    case PID_HYGR_MINT:      return &config.HYGR_MINT;
    case PID_HYGR_MAXT:      return &config.HYGR_MAXT;

    case PID_PRESSURE_MIN:   return &config.PRESSURE_MIN;
    case PID_PRESSURE_MAX:   return &config.PRESSURE_MAX;
  }
  return nullptr;
}

static void readMeta(uint8_t idx, char* outName, int16_t &minV, int16_t &maxV, int16_t &defV, uint8_t &id)
{
  ParamMetaPGM m;
  memcpy_P(&m, &PARAMS[idx], sizeof(ParamMetaPGM));

  strcpy_P(outName, m.name);
  minV = m.minV;
  maxV = m.maxV;
  defV = m.defV;
  id   = m.id;
}

static void setDefaultsInternal()
{
  for (uint8_t i=0; i<PID_COUNT; i++)
  {
    int16_t minV, maxV, defV; uint8_t id;
    readMeta(i, nameBuf, minV, maxV, defV, id);
    int* p = ptrForId(id);
    if (p) *p = defV;
  }
}

void config_save()
{
  uint32_t signature = 0xCAFEBABE;
  EEPROM.put(0, signature);

  int addr = sizeof(uint32_t);
  for (uint8_t i=0; i<PID_COUNT; i++)
  {
    int* p = ptrForId(i);
    int v = p ? *p : 0;
    EEPROM.put(addr, v);
    addr += sizeof(int);
  }
}

static void loadFromEEPROM()
{
  uint32_t signature = 0;
  EEPROM.get(0, signature);

  if (signature != 0xCAFEBABE)
  {
    setDefaultsInternal();
    config_save();
    return;
  }

  int addr = sizeof(uint32_t);
  for (uint8_t i=0; i<PID_COUNT; i++)
  {
    int v = 0;
    EEPROM.get(addr, v);
    addr += sizeof(int);
    int* p = ptrForId(i);
    if (p) *p = v;
  }
}

void config_init()
{
  loadFromEEPROM();
}

void config_reset_defaults()
{
  setDefaultsInternal();
  config_save();
}

void config_print_all()
{
  Serial.println(F("\n=== CONFIG ==="));
  for (uint8_t i=0; i<PID_COUNT; i++)
  {
    int16_t minV, maxV, defV; uint8_t id;
    readMeta(i, nameBuf, minV, maxV, defV, id);
    int* p = ptrForId(id);
    Serial.print(nameBuf);
    Serial.print(F(" = "));
    Serial.println(p ? *p : 0);
  }
  Serial.println(F("==============\n"));
}

bool config_set_param(const String &key, int value)
{
  for (uint8_t i=0; i<PID_COUNT; i++)
  {
    int16_t minV, maxV, defV; uint8_t id;
    readMeta(i, nameBuf, minV, maxV, defV, id);

    if (key.equalsIgnoreCase(nameBuf))
    {
      if (value < minV) value = minV;
      if (value > maxV) value = maxV;

      int* p = ptrForId(id);
      if (p) *p = value;

      Serial.print(F("SET "));
      Serial.print(nameBuf);
      Serial.print(F(" = "));
      Serial.println(value);

      return true;
    }
  }
  return false;
}
