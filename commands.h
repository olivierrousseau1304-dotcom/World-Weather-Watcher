#ifndef COMMANDS_H
#define COMMANDS_H

#include <Arduino.h>

// Doit être appelé régulièrement pour lire les commandes
void commands_poll();

// Pour afficher VERSION + LOT
void print_version();

// Enregistrement date/heure
void set_clock(const char* s);
void set_date(const char* s);
void set_day(const char* s);

#endif
