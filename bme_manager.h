#ifndef BME_MANAGER_H
#define BME_MANAGER_H

#include <Arduino.h>

struct BmeData {
  bool   ok;
  float  temp;
  float  hum;
  float  pres;
};

// Init BME
bool bme_init();

// Lecture (avec TIMEOUT ms)
bool bme_read(BmeData &out, unsigned long timeoutMs);

#endif
