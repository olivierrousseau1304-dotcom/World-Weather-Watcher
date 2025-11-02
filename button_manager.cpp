#include "button_manager.h"
#include "led_manager.h"
#include "sd_logger.h"

// ========================================
// Init : input + pull-ups
// ========================================
void button_init()
{
    pinMode(BTN_VERT,  INPUT_PULLUP);
    pinMode(BTN_ROUGE, INPUT_PULLUP);
}

// ========================================
// Pression simple (LOW = appuyé)
// ========================================
bool button_red_pressed()
{
    return (digitalRead(BTN_ROUGE) == LOW);
}

bool button_green_pressed()
{
    return (digitalRead(BTN_VERT) == LOW);
}

// ========================================
// Détection appui long (5 s)
// Non bloquant
// ========================================
bool button_is_long_press(uint8_t pin)
{
    static unsigned long tStart = 0;
    static uint8_t lastPin = 255;

    int state = digitalRead(pin);

    if (state == LOW)
    {
        if (lastPin != pin)
        {
            tStart = millis();
            lastPin = pin;
        }
        if (millis() - tStart >= LONG_PRESS_MS)
        {
            lastPin = 255;
            return true;
        }
    }
    else
    {
        if (pin == lastPin)
            lastPin = 255;
    }
    return false;
}

// ========================================
// Gère transitions de mode
// currentMode : 0=STD,1=CONFIG,2=MAINT,3=ECO
// ========================================
void handle_buttons(uint8_t* currentMode)
{
    if (!currentMode) return;

    bool rougeLong = button_is_long_press(BTN_ROUGE);
    bool vertLong  = button_is_long_press(BTN_VERT);

    switch (*currentMode)
    {
        // =================================
        // STANDARD → Maintenance / Eco
        // =================================
        case 0: // Standard
            if (rougeLong)
            {
                *currentMode = 2;        // Maintenance
                led_color(255,128,0);    // orange
                sd_close();
            }
            else if (vertLong)
            {
                *currentMode = 3;        // ECO
                led_color(0,0,255);      // bleu
            }
            break;

        // =================================
        // MAINTENANCE → Standard
        // =================================
        case 2:
            if (rougeLong)
            {
                *currentMode = 0;        // Standard
                led_color(0,255,0);      // vert
                sd_init(4);              // ré-ouverture SD
            }
            break;

        // =================================
        // ECO → Standard
        // =================================
        case 3:
            if (rougeLong)
            {
                *currentMode = 0;
                led_color(0,255,0);      // vert
            }
            break;

        // =================================
        // CONFIG : transitions gérées via boot
        // =================================
        case 1:
        default:
            break;
    }
}
