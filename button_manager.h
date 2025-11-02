#ifndef BUTTON_MANAGER_H
#define BUTTON_MANAGER_H

#include <Arduino.h>

void button_init();

bool button_red_pressed();
bool button_blue_pressed();

bool button_red_long();
bool button_blue_long();

#endif
