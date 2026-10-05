#pragma once

// ============================================================================
// PortaChiave - Rete Wi-Fi avanzata (scan profondo + monitor + attack)
//
// Modulo ispirato a esp8266_deauther (Spacehuhn) e al blocco "wlan_app" del
// Flipper-Zero-ESP32-Port, limitato a quello che un ESP8266 puo' fare davvero:
//   - scan AP asincrono non bloccante  (WiFi.scanNetworks async)
//   - sniffer passivo (promiscuous)    (packet monitor + rilevamento client)
//   - deauthentication / disasso ciation payload injection (wifi_send_pkt_freedom)
//   - beacon spam                      (stesso meccanismo del deauther)
//   - probe request flood             (come Probe_Download/Probe_Flood del deauther)
//   - beacon random                   (SSID random generato a runtime)
//   - probe tracking                  (quali SSID i dispositivi stanno cercando)
//
// La radio e' una sola: un'"operazione rete" alla volta. Durante sniff/attack
// l'hardware salta di canale: il softAP resta attivo ma i client possono
// dover riconnettersi; a fine operazione si torna sul canale 1.
//
// Con core Arduino 3.1.2 + NONOSSDK22x_190703 le funzioni freedom SDK sono
// presenti (wifi_send_pkt_freedom / wifi_set_promiscuous_rx_cb), quindi
// injection e monitoraggio funzionano davvero (stessa SDK della fork deauther).
// ============================================================================

#include <Arduino.h>

namespace pk_net {

void begin();

// Chiamata a ogni giro del loop: scheduler cooperativo non bloccante.
void tick();

// Stato: "idle","scan","sniff","deauth","deauthall","beacon","beaconrnd","probe"
const char* modeName();
bool isBusy();          // true se un'operazione rete e' in corso
bool scanDone();        // true se l'ultimo scan e' stato completato
uint16_t netCount();    // numero di reti dell'ultimo scan

// Ferma qualunque operazione in corso (usata dagli altri moduli, es. evil twin).
void stopAll();

// --- AP scan asincrono ----------------------------------------------------
String scanStart();     // avvia { "mode":"scanning" }   || restart
String scanState();     // stato + reti + stazioni JSON

// --- Sniffer passivo ------------------------------------------------------
String sniffToggle(bool on);
String sniffState();    // contatori pacchetti + stazioni JSON

// --- Attack ---------------------------------------------------------------
// type: "deauthall" | "deauth" (bssid) | "beacon" (ssid) |
//       "beaconrnd" | "probe" (ssid, usa la lista nomi degli SSID scansionati)
String attackStart(const String& type, const String& bssid,
                   uint8_t channel, const String& ssid,
                   uint32_t timeoutSec);
String attackStop();
String attackState();   // stato + contatori JSON

} // namespace pk_net