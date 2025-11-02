#include "mode_config.h"
#include "led_manager.h"
#include "button_manager.h"
#include "config.h"
#include "commands.h"

void mode_config_start() {
    led_color(255, 255, 0); // jaune

    unsigned long start = millis();
    unsigned long timeout_mlls = (unsigned long)config.TIMEOUT * 60000UL;


    while (true) {
        // === Lecture série ===
        if (Serial.available() > 0) {
            String input = Serial.readStringUntil('\n');
            input.trim();

            start = millis(); // Reset du timer à chaque commande

            if (input.equalsIgnoreCase("EXIT")) {
                Serial.println("Sortie du mode configuration...");
                break;
            }
            if (input.equalsIgnoreCase("VERSION")) {
                Serial.println("V5.2");
                continue;
            }

            processCommand(input);
            config_print_from_eeprom();
        }

        // === Vérification du timeout ===
        if (millis() - start > timeout_mlls) {
            Serial.println("Timeout atteint-> retour en mode standard");
            break;
        }

        delay(100);
    }
}

