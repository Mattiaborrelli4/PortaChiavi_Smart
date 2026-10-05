#pragma once

#include <Arduino.h>

// ============================================================================
// PortaChiave - Wi-Fi Access Point autonomo
//
// L'ESP crea la propria rete, con SSID derivato dal dispositivo:
//   PortaChiave-XXXX  (ultimi 4 esadecimali della MAC)
// IP prevedibile: 192.168.4.1
// Password WPA2: AP_PASSWORD
// ============================================================================

#define AP_PASSWORD "PortaChiave2026"

namespace wifi_ap {

void begin();

String ssid();
String password();

// Presenza stazioni: usa la lista reale del softAP (affidabile sul core 3.1.x)
bool hasClient();
uint8_t clientCount();
String clientsDebug();
String clientsJson();   // JSON array [{"mac":"..","ip":".."}] delle stazioni associate
String firstClientMac();// MAC della prima stazione associata (o "")

String statusJson();

} // namespace wifi_ap