#include <Arduino.h>

#include "config.h"
#include "commands.h"
#include "gps_manager.h"
#include "bme_manager.h"
#include "rtc_manager.h"
#include "sd_logger.h"
#include "led_manager.h"
#include "button_manager.h"


// ================================
// Modes
// ================================
#define MODE_STANDARD     0
#define MODE_CONFIG       1
#define MODE_MAINT        2
#define MODE_ECO          3

uint8_t currentMode = MODE_STANDARD;


// ================================
// Light wrapper
// ================================
static LightData readLight()
{
    LightData l;
    l.value = analogRead(A0);
    l.ok = true;
    return l;
}


// ================================
// SETUP
// ================================
void setup()
{
    Serial.begin(9600);

    // LED MUST BE FIRST → avoid white boot
    led_init();
    led_off();

    // Buttons
    button_init();

    // Configuration
    config_init();

    // Choose startup mode (red = CONFIG)
    if (button_red_pressed())
    {
        currentMode = MODE_CONFIG;
        led_color(255,255,0);   // Yellow
    }
    else
    {
        currentMode = MODE_STANDARD;
        led_color(0,255,0);     // Green
    }

    // RTC
    rtc_init();

    // GPS
    gps_init();
    gps_set_enabled(config.GPS == 1);

    // SD
    if (!sd_init(4))
    {
        led_pattern_sd_write_error();
    }
    else
    {
        sd_set_max_file_size((uint32_t)config.FILE_MAX_SIZE);
    }

    // BME
    bme_init();
}



// ================================
// LOOP
// ================================
void loop()
{
    // Serial commands (CONFIG mode)
    commands_poll();

    bool red  = button_red_pressed();
    bool blue = button_blue_pressed();


    // ---------------------------------
    // Transitions
    // ---------------------------------
    if (currentMode == MODE_STANDARD)
    {
        if (red)     // → MAINT
        {
            currentMode = MODE_MAINT;
            led_color(255,128,0);    // orange
            sd_close();
        }
        if (blue)    // → ECO
        {
            currentMode = MODE_ECO;
            led_color(0,0,255);      // blue
        }
    }
    else if (currentMode == MODE_MAINT)
    {
        if (red)     // back → STANDARD
        {
            currentMode = MODE_STANDARD;
            led_color(0,255,0);
            sd_init(4);
        }
    }
    else if (currentMode == MODE_ECO)
    {
        if (red)     // back → STANDARD
        {
            currentMode = MODE_STANDARD;
            led_color(0,255,0);
        }
    }



    // ---------------------------------
    // Logging
    // ---------------------------------
    static unsigned long lastLog = 0;

    uint32_t intervalMin = config.LOG_INTERVALL;
    if (currentMode == MODE_ECO)
        intervalMin *= 2;

    uint32_t intervalMs = intervalMin * 60000UL;

    if (millis() - lastLog >= intervalMs)
    {
        lastLog = millis();

        // GPS
        GpsData gps;
        bool gps_ok = gps_is_enabled()
                    ? gps_read(gps, config.GPS_TIMEOUT * 1000UL)
                    : false;

        // BME
        BmeData bme;
        bool bme_ok = bme_read(bme);

        // LIGHT
        LightData l = readLight();

        // RTC strings
        String dStr = rtc_get_date_str();
        String tStr = rtc_get_time_str();


        // SD logging only in STANDARD
        if (currentMode == MODE_STANDARD && sd_is_ready())
        {
            sd_append_csv(
                gps,
                bme,
                l,
                dStr.c_str(),
                tStr.c_str()
            );
        }
    }
}
