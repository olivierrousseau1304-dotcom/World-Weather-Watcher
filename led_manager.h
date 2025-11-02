#ifndef LED_MANAGER_H
#define LED_MANAGER_H

#include <Arduino.h>
#include <ChainableLED.h>

// Déclaration du contrôleur de LEDs (défini dans le .cpp)
extern ChainableLED leds;

// Fonctions de contrôle basique
void led_color(int red, int green, int blue);
void led_off();

void led_pattern_sd_full();         // Rouge <-> Blanc (1 Hz)   (SD pleine)
void led_pattern_sd_write_error();  // Rouge <-> Blanc (Blanc x2) (Erreur R/W)


// Fonction transversale de gestion de LEDs selon le mode
// Mode : 0=STANDARD, 1=CONFIG, 2=MAINTENANCE, 3=ECO
void handle_leds(uint8_t currentMode);

#endif
