#include <auth.h>
#include <pk_access.h>
#include <value_store.h>

namespace auth {

static bool   _authenticated = false;
static uint32_t _sessionStart = 0;
static uint8_t  _role        = 0;      // access::ROLE_NONE
static String   _clientId;
static String   _ownerMac;
static String   _setupToken;

// Per-client authentication state, keyed by client ID.
// Maps clientId -> {authenticated, role, sessionStart}
#define MAX_CLIENTS 8
static struct {
    char     clientId[33];
    bool     authenticated;
    uint32_t sessionStart;
    uint8_t  role;
} _clients[MAX_CLIENTS];
static uint8_t _clientCount = 0;

static uint8_t findClientById(const char* clientId) {
    for (uint8_t i = 0; i < _clientCount; i++) {
        if (strcmp(_clients[i].clientId, clientId) == 0) return i;
    }
    return 0xFF;
}

static uint8_t addClient(const char* clientId) {
    if (_clientCount >= MAX_CLIENTS) return 0xFF;
    strcpy(_clients[_clientCount].clientId, clientId);
    _clients[_clientCount].authenticated = false;
    _clients[_clientCount].sessionStart = 0;
    _clients[_clientCount].role = 0;
    return _clientCount++;
}

static void resetClientState(uint8_t idx) {
    _clients[idx].authenticated = false;
    _clients[idx].sessionStart = 0;
    _clients[idx].role = 0;
    memset(_clients[idx].clientId, 0, 33);
}

// Pulizia stato sessione client (ruolo + identita') dopo logout/timeout.
static void resetSessionState() {
    for (uint8_t i = 0; i < _clientCount; i++) {
        resetClientState(i);
    }
}

bool loadSetupToken() {
    _setupToken = value_store::settingsLoad("auth_token");
    return _setupToken.length() > 0;
}

void saveSetupToken(const String& token) {
    value_store::settingsSave("auth_token", token);
    _setupToken = token;
}

void begin() {
    _authenticated = false;
    _sessionStart = 0;
    loadSetupToken();
    Serial.println();
    Serial.println(F("[Auth] Sistema autenticazione avviato"));
    if (_setupToken.length() > 0) {
        Serial.print(F("[Auth] Token setup presente: "));
        Serial.println(_setupToken.substring(0, 4) + "....");
    } else {
        Serial.println(F("[Auth] Nessun token setup - modalita' primo avvio"));
    }
    Serial.print(F("[Auth] Timeout sessione: "));
    Serial.print(AUTH_TIMEOUT_MS / 1000UL);
    Serial.println(F(" s"));
}

void setAuthenticated(bool value, const String& clientId) {
    uint8_t idx = findClientById(clientId.c_str());
    if (idx == 0xFF) {
        idx = addClient(clientId.c_str());
    }
    _clients[idx].authenticated = value;
    if (value) {
        _clients[idx].sessionStart = millis();
        _clients[idx].role = access::ROLE_OWNER;
        _authenticated = true;
        _sessionStart = millis();
        _clientId = clientId;
        Serial.print(F("[Auth] "));
        Serial.print(clientId);
        Serial.println(F(" authenticated = true"));
    } else {
        resetClientState(idx);
        // Se non ci sono più clienti autenticati, resetta lo stato globale
        bool anyAuthenticated = false;
        for (uint8_t i = 0; i < _clientCount; i++) {
            if (_clients[i].authenticated) anyAuthenticated = true;
        }
        _authenticated = anyAuthenticated;
        _sessionStart = anyAuthenticated ? millis() : 0;
        _clientId = "";
    }
}

bool isAuthenticated() {
    if (!_authenticated) return false;
    uint32_t elapsed = millis() - _sessionStart;
    if (elapsed >= AUTH_TIMEOUT_MS) {
        _authenticated = false;
        _sessionStart = 0;
        resetSessionState();
        return false;
    }
    // Uso attivo: riavvia il timeout della sessione.
    _sessionStart = millis();
    return true;
}

bool isDevMode() {
#if AUTH_DEV_MODE
    return true;
#else
    return false;
#endif
}

uint32_t sessionTimeoutSec() {
    return AUTH_TIMEOUT_MS / 1000UL;
}

uint32_t sessionSecondsLeft() {
    if (!_authenticated) return 0;
    uint32_t elapsed = millis() - _sessionStart;
    if (elapsed >= AUTH_TIMEOUT_MS) return 0;
    return (AUTH_TIMEOUT_MS - elapsed) / 1000UL;
}

void setRole(uint8_t r) {
    _role = r;
}

uint8_t role() {
    return _role;
}

void setClientId(const String& c) {
    _clientId = c;
}

String clientId() {
    return _clientId;
}

void setOwnerMac(const String& mac) {
    _ownerMac = mac;
}

String ownerMac() {
    return _ownerMac;
}

static String escapeJson(const String& s) {
    String out;
    for (unsigned int i = 0; i < s.length(); i++) {
        char c = s.charAt(i);
        if (c == '"' || c == '\\') {
            out += '\\';
            out += c;
        } else if ((uint8_t)c < 0x20) {
            // char di controllo: saltato
        } else {
            out += c;
        }
    }
    return out;
}

String statusJson() {
    String j = "{";
    j += "\"auth\":\"public\",";
    j += "\"authenticated\":" + String(_authenticated ? "true" : "false") + ",";
    j += "\"mode\":\"";
    j += isDevMode() ? "dev" : "real";
    j += "\",";
    j += "\"sessionSecondsLeft\":" + String(isAuthenticated() ? sessionSecondsLeft() : 0) + ",";
    j += "\"role\":\"";
    j += access::roleName(_role);
    j += "\",";
    j += "\"clientId\":\"";
    j += escapeJson(_clientId);
    j += "\"";
    j += "}";
    return j;
}

} // namespace auth