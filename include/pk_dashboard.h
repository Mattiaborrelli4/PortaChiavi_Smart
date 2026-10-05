#pragma once

#include <Arduino.h>

// ============================================================================
// PortaChiave - Dashboard web (file separato dal Core)
// ============================================================================

namespace dashboard {
// Pagina completa conservata in PROGMEM (nessuna copia in RAM).
// page() puo' essere mandata direttamente con server.send_P().
const char* page();
String html(); // compatibilita': NON usare per pagine grandi (copia in RAM)
}