#ifndef LED_MANAGER_H
#define LED_MANAGER_H

#include <Arduino.h>
#include <ChainableLED.h>

// === LED chainable P9813 ===
extern ChainableLED leds;

// Couleur fixe
void led_color(uint8_t r, uint8_t g, uint8_t b);
void led_off();

// Patterns d'erreurs
// SD pleine → Rouge <-> Blanc (1 Hz)
void led_pattern_sd_full();

// Erreur R/W → Rouge (0.5s) Blanc (1s)
void led_pattern_sd_write_error();

// Gère couleur selon mode
// 0=STANDARD,1=CONFIG,2=MAINTENANCE,3=ECO
void handle_leds(uint8_t currentMode);

#endif
