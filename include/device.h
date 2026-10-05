#pragma once

#include <Arduino.h>

// ============================================================================
// PortaChiave - Device Info
// ============================================================================

namespace device {

// Identificativo stabile derivato dalla MAC: "PK-A1B2C3D4"
String id();

// Nome prodotto
String name();

// Versione firmware
String firmware();

// Chip ID (hex)
String chipId();

// Ultimi 4 esadecimali della MAC, per gli SSID derivati
String macSuffix();

} // namespace device