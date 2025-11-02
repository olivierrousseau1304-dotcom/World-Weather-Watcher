#include <Arduino.h>
#include "config.h"
#include "led_manager.h"

// Initialisation du module LED
ChainableLED leds(LED_PIN, LED_PIN + 1, 1);

// --- Allumer une certaine couleur ---
void led_color(int red, int green, int blue) {
    leds.setColorRGB(0, red, green, blue);
}

// --- Éteindre la LED ---
void led_off() {
    leds.setColorRGB(0, 0, 0, 0);
}

// --- Fonction de gestion LED selon le mode ---
// (aucune variable globale utilisée)
void handle_leds(uint8_t currentMode) {
    switch (currentMode) {
        case 0: // STANDARD
            led_color(0, 255, 0);   // Vert
            break;
        case 1: // CONFIG
            led_color(255, 255, 0); // Jaune
            break;
        case 2: // MAINTENANCE
            led_color(255, 128, 0); // Orange
            break;
        case 3: // ECO
            led_color(0, 0, 255);   // Bleu
            break;
        default:
            led_off(); // Sécurité : éteindre si mode inconnu
            break;
    }
}
void led_pattern_sd_full(){
  for(uint8_t i=0;i<6;i++){
    led_color(200,0,0); delay(500);
    led_color(255,255,255); delay(500);
  }
  led_off();
}

void led_pattern_sd_write_error(){
  for(uint8_t i=0;i<4;i++){
    led_color(200,0,0); delay(500);
    led_color(255,255,255); delay(1000);
  }
  led_off();
}
