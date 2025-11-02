#ifndef BME_MANAGER_H
#define BME_MANAGER_H

#include <Arduino.h>
#include <Wire.h>

struct BmeData {
  float temperature;
  float humidity;
  float pressure;
  bool ok;
};

bool bme_init();
bool bme_read(BmeData &out);

#endif

