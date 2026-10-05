#pragma once

#include <Arduino.h>

// ============================================================================
// PortaChiave - Value Store + ponte Serial
//
// Mantiene il vecchio sistema key:value letto dalla Serial:
//   key:value\n
// e aggiorna keys / valueMap. Espone /values (legacy) e /api/values.
// Le linee seriali che iniziano con prefissi robot (emotion:/anim:/...)
// vengono smistate al modulo RobotEyes.
// ============================================================================

#define VALUE_MAP_SIZE 200

namespace value_store {

void setup();

// Processa una riga seriale completa (comandi robot + key:value)
void handleLine(const String& line);

// Debug: testo con chiavi e valori correnti
String dump();

// JSON array legacy usato da /values e /api/values:
//   [{"name":"k","value":"v"},...]
String toJson();

// Imposta/aggiorna un valore in modo programmatico (es. da API)
void setValue(const String& key, const String& value);

// Persiste una piccola impostazione su LittleFS (file "/settings/<key>").
// Ritorna false se il file non è scrivibile/montabile.
bool settingsSave(const String& key, const String& value);

// Legge una impostazione persistita; ritorna String vuota se assente/errore.
String settingsLoad(const String& key);

uint32_t hashString(const String& s);

} // namespace value_store