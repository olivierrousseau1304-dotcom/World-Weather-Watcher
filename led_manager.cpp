#include "led_manager.h"
#include <ChainableLED.h>

// P9813 — DI = 8, CI = 9, 1 LED
static ChainableLED leds(8, 9, 1);

// ========= BAS NIVEAU COULEUR =========
static void setRGB(uint8_t r, uint8_t g, uint8_t b)
{
  leds.setColorRGB(0, r, g, b);
}

// ========= MODES (fixe) =========
void led_mode_standard()     { setRGB(0,   255,   0  ); }   // vert
void led_mode_config()       { setRGB(255, 255,   0  ); }   // jaune
void led_mode_economy()      { setRGB(0,     0, 255 ); }   // bleu
void led_mode_maintenance()  { setRGB(255,  80,   0 ); }   // orange profond

void led_off()               { setRGB(0,0,0); }

// ========= BASE CLIGNOTEMENT =========
// d1 = ms COLOR1, d2 = ms COLOR2
static void blink2(uint8_t r1,uint8_t g1,uint8_t b1,
                   uint8_t r2,uint8_t g2,uint8_t b2,
                   uint16_t d1,uint16_t d2)
{
  setRGB(r1,g1,b1);
  delay(d1);
  setRGB(r2,g2,b2);
  delay(d2);
}

// ========= MOTIFS ERREURS (CDC) =========

// 1 Hz = 500/500 ms
// RTC = Rouge ↔ Bleu
void led_err_rtc()
{
  blink2(255,0,0,   0,0,255,   500,500);
}

// GPS = Rouge ↔ Jaune
void led_err_gps()
{
  blink2(255,0,0,   255,255,0, 500,500);
}

// Capteur = Rouge ↔ Vert
void led_err_sensor()
{
  blink2(255,0,0,   0,255,0,   500,500);
}

// Capteur incohérent = Rouge ↔ Vert (vert plus long)
void led_err_sensor_inco()
{
  blink2(255,0,0,   0,255,0,   500,1000);
}

// SD pleine = Rouge ↔ Blanc
void led_err_sd_full()
{
  blink2(255,0,0,   255,255,255, 500,500);
}

// SD write error = Rouge ↔ Blanc (blanc 2× plus long)
void led_err_sd_write()
{
  blink2(255,0,0,   255,255,255, 500,1000);
}
