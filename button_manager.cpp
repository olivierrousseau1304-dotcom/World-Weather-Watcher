#include "button_manager.h"

// Rouge = D6, Bleu = D7
static const uint8_t PIN_RED  = 6;
static const uint8_t PIN_BLUE = 7;

static unsigned long tRed = 0;
static unsigned long tBlue = 0;

static bool redWasDown  = false;
static bool blueWasDown = false;

static const unsigned long LONG_MS = 5000;

void button_init()
{
    pinMode(PIN_RED, INPUT_PULLUP);
    pinMode(PIN_BLUE, INPUT_PULLUP);
}

bool button_red_pressed()
{
    bool down = (digitalRead(PIN_RED) == LOW);

    if (down && !redWasDown)
    {
        redWasDown = true;
        tRed = millis();
        return true;        // appui court détecté immédiatement
    }
    if (!down)
        redWasDown = false;

    return false;
}

bool button_blue_pressed()
{
    bool down = (digitalRead(PIN_BLUE) == LOW);

    if (down && !blueWasDown)
    {
        blueWasDown = true;
        tBlue = millis();
        return true;        // appui court
    }
    if (!down)
        blueWasDown = false;

    return false;
}

bool button_red_long()
{
    bool down = (digitalRead(PIN_RED) == LOW);

    if (down && (millis() - tRed >= LONG_MS) && redWasDown)
    {
        redWasDown = false;
        return true;
    }
    return false;
}

bool button_blue_long()
{
    bool down = (digitalRead(PIN_BLUE) == LOW);

    if (down && (millis() - tBlue >= LONG_MS) && blueWasDown)
    {
        blueWasDown = false;
        return true;
    }
    return false;
}
