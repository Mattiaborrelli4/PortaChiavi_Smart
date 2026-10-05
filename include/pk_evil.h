#pragma once

// ============================================================================
// PortaChiave - Evil Twin: clone di una rete scelta + pagina di "verifica"
//
// Ispirato a brighteyekid/ESP8266-WiFi-Pentesting-Tool e a DX4GREY
// (esp8266_wifitools/EvilTwin): l'ESP copia SSID (+ eventuale canale) di una
// rete scansionata, pubblica un AP APERTO con lo stesso nome e serve una
// pagina che chiede la password ("aggiornamento/verifica"). Le credenziali
// sottoposte vengono accumulate (RAM + /evil_log.csv su LittleFS) e la pagina
// continua a rispondere "password non valida" per raccogliere piu' tentativi.
//
// NOTA LEGALE: funziona solo su reti di tua proprieta' o autorizzate. Ferma
// qualunque operazione radio in corso (pk_net) prima di partire; /stop ripristina
// il normale access point della PortaChiave.
// ============================================================================

#include <Arduino.h>

namespace pk_evil {

bool begin();
bool isRunning();
const char* targetSsid();
uint8_t currentChannel();
int capturedCount();

// Pubblica l'AP falso con lo stesso SSID (senza password). Ritorna status JSON.
String start(const String& ssid, uint8_t channel);

// Disattiva l'AP falso e ripristina l'AP standard (wifi_ap::begin()).
String stop();

// {"running":bool,"ssid":"..","channel":n,"captured":n,"creds":[{ssid,pass}]}
String status();

// Chiamata dalla POST /submit: archivia la password tentata.
void onSubmitted(const String& pass);

// Pagine servite durante l'attacco ("" se non attivo).
String loginPage();
String failPage();

} // namespace pk_evil