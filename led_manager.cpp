#include "led_manager.h"

// DATA=D8, CLK=D9 (adapte si besoin)
ChainableLED leds(8, 9, 1);

void led_color(uint8_t r, uint8_t g, uint8_t b) { leds.setColorRGB(0, r, g, b); }
void led_off() { leds.setColorRGB(0, 0, 0, 0); }

// --------- PATTERN SD PLEINE / ROTATION KO (3 cycles, ~3 s) ----------
void led_pattern_sd_full() {
    for (uint8_t i = 0; i < 3; ++i) {
        led_color(255, 0, 0);     delay(500);  // rouge
        led_color(255, 255, 255); delay(500);  // blanc
    }
    // Laisse en rouge fixe pour signal fort
    led_color(255, 0, 0);
}

// --------- PATTERN ERREUR ECRITURE (3 cycles, ~4.5 s) ----------
void led_pattern_sd_write_error() {
    for (uint8_t i = 0; i < 3; ++i) {
        led_color(255, 0, 0);     delay(500);   // rouge court
        led_color(255, 255, 255); delay(1000);  // blanc long
    }
    // Laisse en rouge fixe
    led_color(255, 0, 0);
}

// 0=STANDARD (vert), 1=CONFIG (jaune), 2=MAINT (orange), 3=ECO (bleu)
void handle_leds(uint8_t currentMode) {
    switch (currentMode) {
        case 0: led_color(0, 255, 0);    break;
        case 1: led_color(255, 255, 0);  break;
        case 2: led_color(255, 128, 0);  break;
        case 3: led_color(0, 0, 255);    break;
        default: led_off();              break;
    }
}
