#ifndef BUTTON_MANAGER_H
#define BUTTON_MANAGER_H

#include <Arduino.h>

// Pins
#define BUTTON_GREEN_PIN   3
#define BUTTON_RED_PIN     2

// À appeler dans loop()
// Retourne :
//  0 = aucun changement
//  1 = demande passage ÉCO
//  2 = demande passage MAINTENANCE
//  3 = demande sortie MAINTENANCE
//  4 = demande passage STANDARD
uint8_t buttons_update(uint8_t currentMode);

// Pour init
void buttons_init();

#endif
