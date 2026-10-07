#pragma once

#include <Arduino.h>

// ============================================================================
// PortaChiave - Authentication / Session
//
// L'ESP NON fa riconoscimento facciale: riceve SOLO il risultato logico
// proveniente dall'autenticazione Face ID eseguita dal dispositivo (iOS).
//   authenticated = true/false
//
// MODALITA' REALE (futura):
//   POST /api/auth/login  con token firmato generato dal client iOS.
//   L'ESP verifica il token ed imposta authenticated.
//
// MODALITA' DEV/TEST (oltre, separata):
//   Compilata con AUTH_DEV_MODE=0 (default). POST /api/auth/login con
//   {mode:"dev"} non autentica (modalità reale). Chiaramente separata: non
//   confondere con il Face ID reale.
//
// Sessione con timeout: quando scade authenticated torna a false.
// Le API protette devono chiamare isAuthenticated() prima di agire.
// ============================================================================

#define AUTH_DEV_MODE 0        // 0 = reale, 1 = solo build/test (off by default)
#define AUTH_TIMEOUT_MS 300000 // 5 minuti
#define AUTH_SETUP_TOKEN_LENGTH 8

namespace auth {

void begin(); // stampa la modalita' attuale

// Imposta lo stato autenticazione e avvia/aggiorna la sessione.
void setAuthenticated(bool value, const String& clientId = "");

// True se la sessione e' valida (aggiorna il timeout all'uso attivo).
bool isAuthenticated();

// True se stiamo operando in modalita' DEV/TEST.
bool isDevMode();

// Duration of the session timeout in seconds.
uint32_t sessionTimeoutSec();

// Secondi rimanenti di sessione.
uint32_t sessionSecondsLeft();

// Ruolo della sessione corrente (enum access::AccessRole, 0 = nessuno).
void setRole(uint8_t r);
uint8_t role();

// Identificativo del client autenticato.
void setClientId(const String& c);
String clientId();

// MAC del proprietario: usata per classificare le stazioni del softAP.
void setOwnerMac(const String& mac);
String ownerMac();

String statusJson();

} // namespace auth