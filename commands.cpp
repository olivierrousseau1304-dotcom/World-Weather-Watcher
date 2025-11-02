#include "commands.h"
#include "config.h"

static int find_param_index(const String& key) {
  for (int i = 0; i < NUM_CONFIG_PARAMS; i++) {
    if (key.equalsIgnoreCase(configParams[i].name)) return i;
  }
  return -1;
}

void processCommand(String input) {
  input.trim();
  if (input.equalsIgnoreCase("SHOW")) {
    config_print(); // RAM
    return;
  }
  if (input.equalsIgnoreCase("EEPROM_SHOW")) {
    config_print_from_eeprom(); // EEPROM
    return;
  }
  if (input.equalsIgnoreCase("RESET")) {
    config_reset_defaults();
    return;
  }

  int sep = input.indexOf('=');
  if (sep == -1) {
    for (int i = 0; i < NUM_CONFIG_PARAMS; i++) {
      Serial.print(configParams[i].name);
      if (i < NUM_CONFIG_PARAMS - 1) Serial.print(", ");
    }
    Serial.println();
    return;
  }

  String key = input.substring(0, sep); key.trim();
  String valStr = input.substring(sep + 1); valStr.trim();

  int idx = find_param_index(key);
  if (idx == -1) {
    for (int i = 0; i < NUM_CONFIG_PARAMS; i++) {
      Serial.print(configParams[i].name);
      if (i < NUM_CONFIG_PARAMS - 1) Serial.print(", ");
    }
    Serial.println();
    return;
  }

  // parse int en sécurité
  long valLong = valStr.toInt();
  int val = (int) valLong;

  // bornes
  if (val < configParams[idx].minVal || val > configParams[idx].maxVal) {
    Serial.print("Valeur hors limites pour ");
    Serial.print(configParams[idx].name);
    Serial.print(" (");
    Serial.print(configParams[idx].minVal);
    Serial.print("..");
    Serial.print(configParams[idx].maxVal);
    Serial.println(")");
    return;
  }

  // application + sauvegarde
  *configParams[idx].value = val;
  config_save();
  
  Serial.print(configParams[idx].name);
  Serial.print(" mis à jour à ");
  Serial.println(val);

  // Optionnel: afficher la EEPROM réelle juste après
  config_print_from_eeprom();
}
