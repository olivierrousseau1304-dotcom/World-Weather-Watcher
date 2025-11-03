#include "button_manager.h"
#include <Arduino.h>

// Long press (ms)
#define LONG_PRESS_MS   5000

static uint8_t  prevMode = 0;
static unsigned long redPressStart = 0;
static unsigned long greenPressStart = 0;
static bool redWasPressed = false;
static bool greenWasPressed = false;

void buttons_init()
{
    pinMode(BUTTON_GREEN_PIN, INPUT_PULLUP);
    pinMode(BUTTON_RED_PIN,   INPUT_PULLUP);
}

// return code:
// 0 = rien
// 1 = demande passage ÉCO
// 2 = demande passage MAINTENANCE
// 3 = demande sortie MAINTENANCE
// 4 = demande passage STANDARD
uint8_t buttons_update(uint8_t currentMode)
{
    bool redPressed   = (digitalRead(BUTTON_RED_PIN)   == LOW);
    bool greenPressed = (digitalRead(BUTTON_GREEN_PIN) == LOW);
    unsigned long now = millis();

    // =========== GREEN long press -> MODE ÉCO ===========
    if (greenPressed)
    {
        if (!greenWasPressed)
        {
            greenPressStart = now;
            greenWasPressed = true;
        }
        else
        {
            if ((now - greenPressStart) >= LONG_PRESS_MS)
            {
                // Pour éviter répétitions
                greenWasPressed = false;

                // Depuis STANDARD → ECO
                if (currentMode == 0) return 1;   // ECO
            }
        }
    }
    else
    {
        greenWasPressed = false;
    }

    // =========== RED long press -> Maintenance / Quit ===========
    if (redPressed)
    {
        if (!redWasPressed)
        {
            redPressStart = now;
            redWasPressed = true;
        }
        else
        {
            if ((now - redPressStart) >= LONG_PRESS_MS)
            {
                redWasPressed = false;

                // STANDARD → MAINT
                if (currentMode == 0)
                {
                    prevMode = currentMode;
                    return 2;   // request MAINT
                }

                // ECO → STANDARD
                if (currentMode == 1)
                {
                    return 4;   // request STANDARD
                }

                // MAINT → previous
                if (currentMode == 2)
                {
                    return 3;   // exit MAINT
                }
            }
        }
    }
    else
    {
        redWasPressed = false;
    }

    return 0;
}
