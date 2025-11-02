#include "button_manager.h"

// --- Constantes locales ---
const unsigned long HOLD_TIME = 5000; // temps maintien pour changement de mode

// --- Variables internes (privées) ---
static bool redButtonHeld = false;
static unsigned long redButtonStart = 0;

static bool greenButtonHeld = false;
static unsigned long greenButtonStart = 0;

// --- Mémoire du mode précédent (utile pour maintenance) ---
static uint8_t previousMode = 0;

// --- Initialisation des boutons ---
void button_init() {
    pinMode(BUTTON1_PIN, INPUT_PULLUP); // Bouton Vert
    pinMode(BUTTON2_PIN, INPUT_PULLUP); // Bouton Rouge
}

// --- Fonctions simples ---
bool button_red_pressed() {
    return digitalRead(BUTTON2_PIN) == LOW;
}

bool button_blue_pressed() {
    return digitalRead(BUTTON1_PIN) == LOW;
}

// --- Fonction principale de gestion des boutons ---
void handle_buttons(uint8_t *currentMode) {
    unsigned long now = millis();

    // === GESTION DU BOUTON ROUGE ===
    if (button_red_pressed()) {
        if (!redButtonHeld) {
            redButtonHeld = true;
            redButtonStart = now;
        }
        else if (now - redButtonStart >= HOLD_TIME) {
            // ----- APPUI LONG (>=5s) -----
            switch (*currentMode) {

                case 0: // STANDARD → MAINTENANCE
                    previousMode = *currentMode;
                    *currentMode = 2;
                    break;

                case 3: // ECO → STANDARD
                    *currentMode = 0;
                    break;

                case 2: // MAINTENANCE → retour au mode précédent
                    *currentMode = previousMode;
                    break;

                // CONFIG : rien ici (retour géré ailleurs)
                default:
                    break;
            }

            redButtonHeld = false; // éviter répétitions
        }
    }
    else {
        // ----- RELÂCHEMENT DU BOUTON -----
        if (redButtonHeld) {
            unsigned long pressDuration = now - redButtonStart;

            // ----- APPUI COURT (<5s) -----
            if (pressDuration < HOLD_TIME) {
                // STANDARD → CONFIGURATION
                if (*currentMode == 0) {
                    *currentMode = 1;
                }
            }

            redButtonHeld = false;
        }
    }

    // === GESTION DU BOUTON VERT ===
    if (button_blue_pressed()) {
        if (!greenButtonHeld) {
            greenButtonHeld = true;
            greenButtonStart = now;
        }
        else if (now - greenButtonStart >= HOLD_TIME) {
            // ----- APPUI LONG (>=5s) -----
            switch (*currentMode) {

                case 0: // STANDARD → ECO
                    previousMode = *currentMode;
                    *currentMode = 3;
                    break;

                // autres modes : pas d'effet
                default:
                    break;
            }

            greenButtonHeld = false;
        }
    }
    else {
        greenButtonHeld = false;
    }
}

