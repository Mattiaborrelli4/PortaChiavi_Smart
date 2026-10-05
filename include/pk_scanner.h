#pragma once

// ============================================================================
// PortaChiave - Scanner Wi-Fi
//
// Scansione passiva difensiva delle reti visibili + risposta decoy innocua
// per chi non ha i permessi.
// ============================================================================

#include <Arduino.h>

namespace pk_scanner {

String scanJson();    // scansione passiva difensiva delle reti visibili
String decoyJson();   // risultato simulato innocuo per chi non ha permesso

} // namespace pk_scanner