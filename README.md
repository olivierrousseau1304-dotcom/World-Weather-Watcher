# 🌍 World Weather Watcher – 3W_V5.2

> **Système embarqué modulaire — GPS + RTC + SD + Modes intelligents**
> Arduino UNO – Air530 GPS – DS3231 RTC – SD Card – LED P9813 – Boutons

<p align="center">
  <img src="https://via.placeholder.com/900x320?text=World+Weather+Watcher+3W_V5.2" />
</p>

---

## 📘 Description

**World Weather Watcher – 3W_V5.2** est un système embarqué permettant :

* d’acquérir périodiquement des données GPS,
* d’horodater les mesures avec un RTC précis,
* d’enregistrer toutes les mesures sur carte SD (CSV),
* tout en gérant intelligemment la consommation via plusieurs modes.

💡 Le système utilise **4 modes** (Standard, Config, Maintenance, Eco) sélectionnables via des boutons et représentés par une LED RGB chainable (**P9813**).

Il est conçu pour être :
✅ **Robuste**
✅ **Économe en énergie**
✅ **Facile à exploiter**
✅ **Extensible**

---

# ✅ Fonctionnalités principales

| Fonction                           | Support |
| ---------------------------------- | ------- |
| Acquisition GPS Air530             | ✅       |
| Timestamp RTC DS3231               | ✅       |
| Sauvegarde SD en CSV               | ✅       |
| Rotation automatique des fichiers  | ✅       |
| 4 modes de fonctionnement          | ✅       |
| LED RGB → état système             | ✅       |
| EEPROM → configuration persistante | ✅       |
| Boutons avec appui long (5s)       | ✅       |
| Mode ÉCO → consomme moins          | ✅       |
| Maintenance → SD sécurisée         | ✅       |

---

# 🔧 Architecture matérielle

<p align="center">
  <img src="https://via.placeholder.com/750x360?text=Hardware+Diagram" />
</p>

## ✅ Composants

| Composant              | Rôle                    |
| ---------------------- | ----------------------- |
| Arduino UNO            | Microcontrôleur central |
| Grove Air530 GPS       | Acquisition coordonnées |
| DS3231 RTC             | Horodatage              |
| SD Card Module         | Stockage CSV            |
| P9813 Chainable LED    | Signaux lumineux        |
| Boutons poussoirs (x2) | Navigation des modes    |

---

# 🔌 Connexions

## 🛰 GPS Air530 (UART)

| Air530 | Arduino UNO | Rôle     |
| ------ | ----------- | -------- |
| VCC    | 5V          |          |
| GND    | GND         |          |
| TX     | D6          | GPS → µC |
| RX     | D7          | µC → GPS |

> **Librairies** : `TinyGPSPlus`, `SoftwareSerial`

---

## ⏱ RTC DS3231 (I²C)

| DS3231 | Arduino UNO |
| ------ | ----------- |
| SDA    | A4          |
| SCL    | A5          |
| VCC    | 5V          |
| GND    | GND         |

> Utilise `Wire.h`

---

## 💾 SD Card

| SD module | Arduino UNO |
| --------- | ----------- |
| CS        | D4          |
| MOSI      | D11         |
| MISO      | D12         |
| SCK       | D13         |

> **D10 doit être en OUTPUT** pour maintenir le mode SPI master ✔

---

## 🔵 LED RGB P9813

| LED | Arduino UNO |
| --- | ----------- |
| DI  | D8          |
| CI  | D9          |

---

## 🔘 Boutons

| Bouton | Arduino UNO | Rôle         |
| ------ | ----------- | ------------ |
| Vert   | D2          | Mode ÉCO     |
| Rouge  | D3          | CONFIG/MAINT |

> Pull-up interne activée → `INPUT_PULLUP`

---

# 🌈 Modes de fonctionnement

<p align="center">
  <img src="https://via.placeholder.com/750x350?text=Mode+State+Diagram" />
</p>

| Mode        | Couleur LED | Description                                  |
| ----------- | ----------- | -------------------------------------------- |
| Standard    | 🟢 Vert     | Acquisition GPS + log SD                     |
| Config      | 🟡 Jaune    | Lecture uniquement, pas d’écriture           |
| Maintenance | 🟠 Orange   | SD fermée → extraction safe                  |
| Éco         | 🔵 Bleu     | Consommation réduite (GPS 1/2, intervalle×2) |

## 🔀 Transitions

| Depuis      | Action       | Vers        |
| ----------- | ------------ | ----------- |
| Standard    | Long Rouge   | Maintenance |
| Standard    | Long Vert    | Eco         |
| Eco         | Long Rouge   | Standard    |
| Maintenance | Long Rouge   | Standard    |
| Boot        | Rouge appuyé | Config      |

> Appui long = **5s**

---

# 🗂 Gestion des fichiers

## 📄 Nom fichier

```
YYMMDD_0.LOG
YYMMDD_1.LOG
YYMMDD_2.LOG
…
```

## 📈 Rotation

Si taille > FILE_MAX_SIZE →
→ `_0.LOG` → `_1.LOG`
→ `_1.LOG` → `_2.LOG`
→ etc.

Puis recrée un nouveau `_0.LOG` avec header CSV.

---

# 📋 Format CSV

```csv
date,utc,lat,lon,alt,sats,speed
2025-02-10,14:20:31,43.70012,7.25046,92.3,8,0.5
2025-02-10,14:21:31,NA,NA,NA,NA,NA
```

> `NA` = pas de FIX valide

---

# 🧠 EEPROM — Paramètres

| Paramètre     | Rôle            | Type |
| ------------- | --------------- | ---- |
| LOG_INTERVALL | période (min)   | int  |
| TIMEOUT       | timeout général | int  |
| FILE_MAX_SIZE | rotation seuil  | int  |
| GPS           | ON/OFF          | int  |
| GPS_TIMEOUT   | timeout GPS     | int  |

---

# 📁 Arborescence du projet

```
3W_V5_2/
 ├── 3W_V5_2.ino
 ├── button_manager.cpp/h
 ├── led_manager.cpp/h
 ├── gps_manager.cpp/h
 ├── rtc_manager.cpp/h
 ├── sd_logger.cpp/h
 ├── config.cpp/h
 ├── README.md
 └── Makefile
```

---

# ✅ Tests

## ✅ Test 1 — Boot

1. Alimenter
2. LED =

   * Bouton ROUGE tenu → Config
   * Sinon → Standard

## ✅ Test 2 — SD Logging

1. Mode Standard
2. Attendre intervalle
3. Check CSV → valeurs présentes

## ✅ Test 3 — Maintenance (SD closed)

1. Appui long rouge → LED orange
2. Retirer SD
3. Pas de corruption

## ✅ Test 4 — Mode ÉCO

1. Appui long vert → LED bleue
2. Vérifier intervalle ×2
3. GPS → mis à jour 1/2

## ✅ Test 5 — Rotation

1. Réduire FILE_MAX_SIZE
2. Constater création `_1.LOG`, `_2.LOG`, etc.

---

# 🚨 Limitations

* Pas de filtrage Kalman GPS
* Pas de compression des données
* Pas d’interface série utilisateur (volontaire)
* Delay dans patterns LED d’erreur (comportement volontaire)
* GPS dépend qualité réception
* Pas de configuration dynamique via USB

---

# 🚀 Améliorations possibles

* Portage STM32 ou ESP32
* Ajout capteurs environnemental (BME280…)
* Compression/encodage binaire des logs
* Sync cloud (MQTT / LoRa)
* Webserver local (ESP32)
* Horodatage fusion GPS/RTC automatique
* Calibration automatique TIMEOUT

---

# 🏆 Auteurs

* **Corentin CARTALLIER – Olivier Rousseau**

Mentions :

* Arduino
* SeeedStudio
* TinyGPSPlus
* DS3231
* FAT SD

---

# ✅ Conclusion

✅ Robuste
✅ Modulaire
✅ Efficace
✅ Conforme cahier des charges

> **World Weather Watcher – 3W_V5.2**
