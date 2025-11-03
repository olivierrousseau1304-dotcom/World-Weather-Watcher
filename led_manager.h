#ifndef LED_MANAGER_H
#define LED_MANAGER_H

#include <Arduino.h>

// === Couleurs fixes (modes) ===
void led_mode_standard();     // Vert
void led_mode_config();       // Jaune
void led_mode_economy();      // Bleu
void led_mode_maintenance();  // Orange profond

// Éteindre
void led_off();

// === Motifs d'erreur (clignotants) ===
// 1 Hz = 500/500 ms
void led_err_rtc();           // Rouge ↔ Bleu
void led_err_gps();           // Rouge ↔ Jaune
void led_err_sensor();        // Rouge ↔ Vert
void led_err_sensor_inco();   // Rouge ↔ Vert (vert 2x)
void led_err_sd_full();       // Rouge ↔ Blanc
void led_err_sd_write();      // Rouge ↔ Blanc (blanc 2x)

#endif
