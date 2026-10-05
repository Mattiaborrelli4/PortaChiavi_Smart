#pragma once

// ============================================================================
// PortaChiave - Wardriving: log delle reti osservate in CSV WiGLE su LittleFS
//
// Ispirato ad AlexLynd/ESP8266-Wardriving, adattato al nostro hardware:
// niente scheda SD niente GPS: le righe sono scritte "on the fly" a ogni scan
// completato (hook in pk_net) e scaricabili dal web tramite /download. Il file
// wardrive.csv usa il formato WiGLE v1.4, quindi e' importabile su wigle.net.
//
// Non usa un RTC: il timestamp e' derivato dal tempo NTP se disponibile,
// altrimenti parte da un'epoca fittizia (anni '80) + uptime di questa sessione.
// ============================================================================

#include <Arduino.h>

namespace pk_wardrive {

// Monta LittleFS e conta le righe gia' presenti. Ritorna true se il FS e' ok.
bool begin();

bool fsOk();

// Abilita/disabilita la registrazione delle reti scansionate.
String setEnabled(bool on);
bool enabled();

// Stato: {"enabled":bool,"entries":n,"size":n,"fs":bool}
String status();

// Svuota il file e il dedup di sessione.
String clear();

bool hasData();
uint32_t fileSize();

// Chiamata da pk_net a ogni scan completato (se disabilitata non fa nulla).
// Fa dedup per BSSID nella sessione corrente per non gonfiare il file.
void record(const char* bssid, const char* ssid, int8_t rssi,
            uint8_t channel, uint8_t enc);

} // namespace pk_wardrive