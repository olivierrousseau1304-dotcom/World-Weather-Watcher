#ifndef BUTTON_MANAGER_H
#define BUTTON_MANAGER_H

#include <Arduino.h>
#include "config.h"

// === Initialisation ===
void button_init();

// === Fonctions basiques ===
bool button_red_pressed();
bool button_blue_pressed();

// === Gestion transversale des boutons ===
// currentMode est passé par pointeur pour être modifié directement
// Modes : 0 = STANDARD, 1 = CONFIG, 2 = MAINTENANCE, 3 = ECO
void handle_buttons(uint8_t *currentMode);

#endif
