#ifndef BUTTON_MANAGER_H
#define BUTTON_MANAGER_H

#include <Arduino.h>

// =============================
// Broches boutons
// =============================
#define BTN_VERT   2     // passer en ECO / gérer transitions
#define BTN_ROUGE  3     // gérer CONFIG au boot / Maintenance

// Durée appui long (ms)
#define LONG_PRESS_MS 5000UL

// =============================
// API
// =============================

void button_init();

bool button_red_pressed();
bool button_green_pressed();

bool button_is_long_press(uint8_t pin);

void handle_buttons(uint8_t* currentMode);

#endif
