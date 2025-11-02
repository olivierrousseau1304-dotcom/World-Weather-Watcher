#include <Arduino.h>

// === Modules internes ===
#include "sd_logger.h"
#include "rtc_manager.h"
#include "config.h"
#include "led_manager.h"
#include "gps_manager.h"
#include "button_manager.h"

// === Variables globales ===
// Mode : 0 = STANDARD, 1 = CONFIG, 2 = MAINTENANCE, 3 = ECO
uint8_t currentMode = 0;

void setup()
{
    Serial.begin(9600);
    Serial.println();
    Serial.println(F("=== SYSTEME 3W_V5_2 - INITIALISATION ==="));

    button_init();
    config_init();

    // Détection configuration au boot
    if (button_red_pressed()) {
        currentMode = 1; // CONFIG
        Serial.println(F("[MODE] Configuration (bouton rouge détecté)"));
    } else {
        currentMode = 0; // STANDARD
        Serial.println(F("[MODE] Standard"));
    }
    handle_leds(currentMode);

    // RTC
    rtc_init();
    if (!rtc_is_available()) {
        Serial.println(F("[RTC] Non détecté"));
    } else {
        Serial.print(F("[RTC] "));
        Serial.print(rtc_get_date_str());
        Serial.print(' ');
        Serial.println(rtc_get_time_str());
    }

    // GPS
    gps_init();
    gps_set_enabled(config.GPS == 1);
    Serial.println(gps_is_enabled() ? F("[GPS] Module actif") : F("[GPS] Désactivé"));

    // SD
    if (!sd_init(4)) {
        Serial.println(F("[SD] INIT FAIL"));
        led_pattern_sd_write_error();
    } else {
        sd_set_max_file_size((uint32_t)config.FILE_MAX_SIZE);
        Serial.print(F("[SD] OK, fichier: "));
        Serial.println(sd_get_current_filename());
    }

    Serial.println(F("[SETUP] Terminé."));
}


void loop()
{
    static unsigned long lastLogMs = 0;
    static bool ecoToggle = false;
    static uint8_t prevMode = 255;

    handle_buttons(&currentMode);

    // Transitions de modes
    if (currentMode != prevMode)
    {
        if (currentMode == 2) {          // MAINTENANCE → SD close
            sd_close();
            Serial.println(F("[SD] Fermeture (maintenance)"));
        }
        else if (prevMode == 2 && currentMode == 0) // retour standard
        {
            if (sd_init(4)) {
                sd_set_max_file_size((uint32_t)config.FILE_MAX_SIZE);
                Serial.println(F("[SD] Réouverture OK"));
            } else {
                Serial.println(F("[SD] Réouverture échouée"));
                led_pattern_sd_write_error();
            }
        }

        handle_leds(currentMode);
        prevMode = currentMode;
    }

    // Fréquence LOG
    uint32_t intervalMin = config.LOG_INTERVALL;
    if (currentMode == 3) intervalMin *= 2;   // ECO
    uint32_t intervalMs = intervalMin * 60UL * 1000UL;

    if (millis() - lastLogMs >= intervalMs)
    {
        lastLogMs = millis();

        bool doGps = true;
        if (currentMode == 3) { ecoToggle = !ecoToggle; doGps = ecoToggle; }

        GpsData data;
        bool ok = false;
        if (gps_is_enabled() && doGps) {
            ok = gps_read(data, (unsigned long)config.GPS_TIMEOUT * 1000UL);
        }

        String dStr = rtc_get_date_str();
        String tStr = rtc_get_time_str();

        if (currentMode == 0 && sd_is_ready())
        {
            if (!sd_append_csv(data, dStr, tStr))
            {
                Serial.println(F("[SD] Erreur ecriture/rotation"));
            }
        }

        Serial.println(F("-----"));
        Serial.print(F("Date: ")); Serial.print(dStr);
        Serial.print(F("  Time: ")); Serial.println(tStr);

        if (ok) {
            Serial.print(F("Lat: "));  Serial.print(data.lat, 6);
            Serial.print(F("  Lon: "));Serial.print(data.lon, 6);
            Serial.print(F("  Sats: "));Serial.println(data.sats);
        } else {
            Serial.println(F("GPS: Pas de fix ou non lu"));
        }
    }
}
