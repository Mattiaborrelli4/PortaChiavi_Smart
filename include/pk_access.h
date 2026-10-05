#pragma once

// ============================================================================
// PortaChiave - Access Control
//
// Matrice dei permessi per ruolo + audit (log azioni) + alert + stazioni
// sconosciute. NON include auth.h: usa auth:: solo dal .cpp (niente cicli).
// ============================================================================

#include <Arduino.h>

namespace access {

enum AccessRole : uint8_t { ROLE_NONE = 0, ROLE_GUEST = 1, ROLE_OWNER = 2, ROLE_UNKNOWN = 3 };

const char* roleName();              // nome del ruolo corrente (legge auth::role())
const char* roleName(uint8_t r);
bool canUse(const char* func);       // permesso per il ruolo corrente, senza log
bool require(const char* func);      // come canUse + logAction(func, risultato, false)
void logAction(const String& func, bool allowed, bool decoy);
uint8_t auditCount();
String auditJson();                  // array JSON ultime 20 voci
uint8_t alertsCount();
void alert(const char* label);       // accoda (max 5, elimina le piu' vecchie)
String alertsJson();
String unknownDevicesJson();         // stazioni connesse con MAC diversa da auth::ownerMac()

} // namespace access