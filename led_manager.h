#ifndef LED_MANAGER_H
#define LED_MANAGER_H

#include <Arduino.h>
#include <ChainableLED.h>

// === P9813 LEDs ===
extern ChainableLED leds;

// === Init ===
void led_init();

// === Fixed colors ===
void led_color(uint8_t r, uint8_t g, uint8_t b);
void led_off();

// === Patterns ===
void led_pattern_sd_full();           // Rouge <-> Blanc (1Hz)
void led_pattern_sd_write_error();    // Rouge <-> Blanc (Blanc x2)

void led_pattern_rtc_error();         // Rouge <-> Bleu
void led_pattern_gps_error();         // Rouge <-> Jaune
void led_pattern_sensor_error();      // Rouge <-> Vert
void led_pattern_sensor_incoherent(); // Rouge <-> Vert (Vert x2)

#endif
