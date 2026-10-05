#pragma once

#include <Arduino.h>

// ============================================================================
// PortaChiave - Captive Portal
//
// Alla connessione il telefono (iOS/Android) rileva una rete senza internet
// e apre automaticamente il browser. Per farlo deve risolvere le sonde
// captive.apple.com / connectivitycheck.gstatic.com verso l'ESP.
//
// Soluzione: un mini server DNS su 192.168.4.1 che risponde a QUALSIASI
// dominio con l'IP dell'ESP; il web server risponde alle richieste non
// riconosciute con un redirect a http://192.168.4.1/ (la dashboard).
// ============================================================================

namespace captive {

void begin();

// Da chiamare nel loop
void process();

} // namespace captive