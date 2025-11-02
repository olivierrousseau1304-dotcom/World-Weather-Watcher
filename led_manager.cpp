#include "led_manager.h"

// === P9813 LED setup ===
// DI = 8, CI = 9 — 1 LED
ChainableLED leds(8, 9, 1);


// ============================================================
// BASE
// ============================================================
void led_init()
{
  // Force frame reset
  leds.setColorRGB(0, 0, 0, 0);   // r,g,b = 0
  delay(5);

  // Envoie une seconde fois pour être sûr
  leds.setColorRGB(0, 0, 0, 0);
  delay(5);
}

void led_color(uint8_t r, uint8_t g, uint8_t b)
{
  leds.setColorRGB(0, r, g, b);
}

void led_off()
{
  led_color(0,0,0);
}


// ============================================================
// HELPER
// ============================================================
static void patternBlink(uint8_t r1,uint8_t g1,uint8_t b1,
                         uint8_t r2,uint8_t g2,uint8_t b2,
                         uint16_t delayMs)
{
  led_color(r1,g1,b1);
  delay(delayMs);
  led_color(r2,g2,b2);
  delay(delayMs);
}


// ============================================================
// SD patterns
// ============================================================

void led_pattern_sd_full()
{
  patternBlink(255,0,0, 255,255,255, 500);
}

void led_pattern_sd_write_error()
{
  led_color(255,0,0);     delay(300);
  led_color(255,255,255); delay(200);
  led_color(255,255,255); delay(200);
}


// ============================================================
// RTC error — Red/Blue
// ============================================================
void led_pattern_rtc_error()
{
  patternBlink(255,0,0, 0,0,255, 500);
}


// ============================================================
// GPS error — Red/Yellow
// ============================================================
void led_pattern_gps_error()
{
  patternBlink(255,0,0, 255,255,0, 500);
}


// ============================================================
// Sensor error — Red/Green
// ============================================================
void led_pattern_sensor_error()
{
  patternBlink(255,0,0, 0,255,0, 500);
}


// ============================================================
// Sensor incoherent — Red / Green ×2
// ============================================================
void led_pattern_sensor_incoherent()
{
  led_color(255,0,0); delay(200);
  led_color(0,255,0); delay(200);
  led_color(0,255,0); delay(200);
}
