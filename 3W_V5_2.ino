// === Librairies ===
#include <Arduino.h>
#include <ChainableLED.h>



// === Fichiers internes ===
#include "led_manager.h"
#include "gps_manager.h"
#include "button_manager.h"
#include "mode_config.h"
#include "mode_standard.h"
#include "sd_logger.h"
#include "rtc_manager.h"
#include "config.h"


// === Variables globales ===
// Mode : 0 = STANDARD, 1 = CONFIG, 2 = MAINTENANCE, 3 = ECO
uint8_t currentMode = 0;

// Variables pour lecture capteurs (exemple)
unsigned long lastSensorRead = 0;
const unsigned long SENSOR_INTERVAL = 1000; // toutes les 1 secondes

// === SETUP ===
void setup() {
  Serial.begin(9600);
  Serial.println();
  Serial.println(F("=== SYSTEME 3W_V5.2 - INITIALISATION ==="));

  // --- Initialisations de base (utilise TON API) ---
  button_init();
  config_init();

  // Sélection du mode au démarrage (bouton rouge pressé = CONFIG)
  if (button_red_pressed()) {
    currentMode = 1; // CONFIG
    Serial.println(F("[MODE] Configuration (bouton rouge détecté)"));
  } else {
    currentMode = 0; // STANDARD
    Serial.println(F("[MODE] Standard"));
  }
  // LED via ton gestionnaire centralisé (ne pas mettre led_init ici)
  handle_leds(currentMode);

  // --- RTC ---
  rtc_init();
  if (!rtc_is_available()) {
    Serial.println(F("[RTC] Non détecté"));
  } else {
    Serial.print(F("[RTC] "));
    Serial.print(rtc_get_date_str());
    Serial.print(' ');
    Serial.println(rtc_get_time_str());
  }

  // --- GPS ---
  gps_init();
  gps_set_enabled(config.GPS == 1);
  if (gps_is_enabled()) Serial.println(F("[GPS] Module actif"));
  else                  Serial.println(F("[GPS] Désactivé"));

  // --- Carte SD (CS = D4) ---
  if (!sd_init(4)) {
    Serial.println(F("[SD] INIT FAIL"));
    // Motif LED erreur écriture (obligatoire dans ton led_manager)
    led_pattern_sd_write_error();
  } else {
    sd_set_max_file_size((uint32_t)config.FILE_MAX_SIZE);
    Serial.print(F("[SD] OK, fichier: "));
    Serial.println(sd_get_current_filename());
  }

  Serial.println(F("[SETUP] Terminé."));
}


// === LOOP PRINCIPALE ===
void loop() {
  static unsigned long lastLogMs = 0;
  static bool ecoToggle = false;
  static uint8_t prevMode = 255;

  // 1️⃣ Gestion des transitions de mode (ton système de boutons gère tout)
  handle_buttons(&currentMode);

  // 2️⃣ Actions automatiques lors d’un changement de mode
  if (currentMode != prevMode) {
    if (currentMode == 2) { // MAINTENANCE
      sd_close();           // Éjection sûre de la carte SD
      Serial.println(F("[SD] Fermeture (mode Maintenance)"));
    }
    else if (prevMode == 2 && currentMode == 0) { // Retour en STANDARD
      if (sd_init(4)) {
        sd_set_max_file_size((uint32_t)config.FILE_MAX_SIZE);
        Serial.println(F("[SD] Réouverture OK (retour Standard)"));
      } else {
        Serial.println(F("[SD] Réouverture échouée !"));
        led_pattern_sd_write_error();
      }
    }

    // Mets à jour la LED selon le mode courant
    handle_leds(currentMode);
    prevMode = currentMode;
  }

  // 3️⃣ Calcul de l’intervalle de log
  uint32_t intervalMin = (uint32_t)config.LOG_INTERVALL;
  if (currentMode == 3) intervalMin *= 2;  // Mode Éco double la période
  uint32_t intervalMs = intervalMin * 60UL * 1000UL;

  // 4️⃣ Exécution périodique du cycle de log
  if (millis() - lastLogMs >= intervalMs) {
    lastLogMs = millis();

    // GPS lu une fois sur deux en mode Éco
    bool doGps = true;
    if (currentMode == 3) {
      ecoToggle = !ecoToggle;
      doGps = ecoToggle;
    }

    GpsData data;
    bool ok = false;
    if (gps_is_enabled() && doGps) {
      ok = gps_read(data, (unsigned long)config.GPS_TIMEOUT * 1000UL);
    }

    // Récupération date/heure du RTC
    String dStr = rtc_get_date_str();
    String tStr = rtc_get_time_str();

    // Écriture sur SD uniquement en mode STANDARD
    if (currentMode == 0 && sd_is_ready()) {
      if (!sd_append_csv(data, dStr, tStr)) {
        Serial.println(F("[SD] Erreur écriture ou rotation"));
      }
    }

    // 5️⃣ Sortie console (debug)
    Serial.println(F("-----"));
    Serial.print(F("Date: ")); Serial.print(dStr);
    Serial.print(F("  Time: ")); Serial.println(tStr);
    if (ok) {
      Serial.print(F("Lat: ")); Serial.print(data.lat, 6);
      Serial.print(F("  Lon: ")); Serial.print(data.lon, 6);
      Serial.print(F("  Sats: ")); Serial.println(data.sats);
    } else {
      Serial.println(F("GPS: Pas de fix ou non lu"));
    }
  }
}



// Gestion des capteurs (exemple) ===
void handle_sensors() {
    unsigned long now = millis();
    if (now - lastSensorRead >= SENSOR_INTERVAL) {
        lastSensorRead = now;

        if (gps_is_enabled()) {
            GpsData data;
            bool ok = gps_read(data, config.GPS_TIMEOUT * 1000UL);
            if (ok) {
                Serial.println(F("---- Données GPS ----"));
                Serial.print(F("Fix: ")); Serial.println(data.fix);
                Serial.print(F("Lat: ")); Serial.println(data.lat, 6);
                Serial.print(F("Lon: ")); Serial.println(data.lon, 6);
                Serial.print(F("Alt: ")); Serial.println(data.alt);
                Serial.print(F("Sats: ")); Serial.println(data.sats);
                Serial.print(F("Vitesse: ")); Serial.print(data.speed); Serial.println(F(" km/h"));
                Serial.print(F("Heure: ")); Serial.println(data.timeUTC);
                Serial.print(F("Date: ")); Serial.println(data.date);
            } else {
                Serial.println(F("Aucune donnée GPS valide."));
            }
        }
    }
}


// Gestion des modes ===
void handle_modes() {
    if (currentMode == 0) {
        // MODE STANDARD
        // logique standard
    } 
    else if (currentMode == 1) {
        // MODE CONFIG
        mode_config_start();
        currentMode = 0; // retour au mode standard
    } 
    else if (currentMode == 2) {
        // MODE MAINTENANCE
        // logique future
    } 
    else if (currentMode == 3) {
        // MODE ECO
        // logique future
    }
}

