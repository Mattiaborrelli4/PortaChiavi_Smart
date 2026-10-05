#include <pk_access.h>
#include <auth.h>
#include <ESP8266WiFi.h>
#include <sys/queue.h>

extern "C" {
struct station_info* wifi_softap_get_station_info(void);
void wifi_softap_free_station_info(void);
}

namespace access {

static const uint8_t AUDIT_MAX   = 20;
static const uint8_t ALERT_MAX   = 5;
static const uint8_t ALERT_LEN   = 24;

struct AuditEntry {
    uint32_t t;
    char client[24];
    char func[16];
    bool allowed;
    bool decoy;
};

static AuditEntry _audit[AUDIT_MAX];
static uint8_t _auditHead = 0;
static uint8_t _auditCount = 0;

static char _alerts[ALERT_MAX][ALERT_LEN];
static uint8_t _alertCount = 0;

// ---------------------------------------------------------------- ruolo

const char* roleName(uint8_t r) {
    static const char* table[] = { "none", "guest", "owner", "unknown" };
    if (r < 4) return table[r];
    return "unknown";
}

const char* roleName() {
    return roleName(auth::role());
}

// ---------------------------------------------------------------- permessi

bool canUse(const char* func) {
    if (!func) return false;
    uint8_t r = auth::role();
    switch (r) {
        case ROLE_OWNER:
            return strcmp(func, "wifi_scan") == 0 ||
                   strcmp(func, "security")  == 0 ||
                   strcmp(func, "settings")  == 0 ||
                   strcmp(func, "camera")    == 0 ||
                   strcmp(func, "robot")     == 0 ||
                   strcmp(func, "controls")  == 0 ||
                   strcmp(func, "net")       == 0 ||
                   strcmp(func, "attack")    == 0 ||
                   strcmp(func, "monitor")   == 0 ||
                   strcmp(func, "evil")      == 0 ||
                   strcmp(func, "wardrive")  == 0;
        case ROLE_GUEST:
            return strcmp(func, "robot")    == 0 ||
                   strcmp(func, "controls") == 0;
        default:
            return false;
    }
}

bool require(const char* func) {
    bool allowed = canUse(func);
    logAction(String(func), allowed, false);
    return allowed;
}

// ---------------------------------------------------------------- audit

static void copyStr(char* dst, size_t cap, const String& src) {
    size_t len = src.length();
    if (len >= cap) len = cap - 1;
    for (size_t i = 0; i < len; i++) dst[i] = src.charAt(i);
    dst[len] = '\0';
}

void logAction(const String& func, bool allowed, bool decoy) {
    AuditEntry* e = &_audit[_auditHead];
    e->t = millis() / 1000UL;
    String client = auth::clientId();
    if (client.length() == 0) {
        String owner = auth::ownerMac();
        if (owner.length() != 0) {
            client = "mac:" + owner;
        } else {
            client = "sconosciuto";
        }
    }
    copyStr(e->client, sizeof(e->client), client);
    copyStr(e->func, sizeof(e->func), func);
    e->allowed = allowed;
    e->decoy = decoy;
    _auditHead = (_auditHead + 1) % AUDIT_MAX;
    if (_auditCount < AUDIT_MAX) _auditCount++;
}

uint8_t auditCount() {
    return _auditCount;
}

static String escapeJson(const char* s) {
    String out;
    for (size_t i = 0; s[i] != '\0'; i++) {
        char c = s[i];
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

String auditJson() {
    String j = "[";
    for (uint8_t k = 0; k < _auditCount; k++) {
        uint8_t idx = (_auditHead + AUDIT_MAX - 1 - k) % AUDIT_MAX;
        const AuditEntry& e = _audit[idx];
        if (k > 0) j += ",";
        j += "{\"t\":" + String(e.t) + ",";
        j += "\"client\":\"" + escapeJson(e.client) + "\",";
        j += "\"func\":\"" + escapeJson(e.func) + "\",";
        j += "\"allowed\":" + String(e.allowed ? "true" : "false") + ",";
        j += "\"decoy\":" + String(e.decoy ? "true" : "false") + "}";
    }
    j += "]";
    return j;
}

// ---------------------------------------------------------------- alert

uint8_t alertsCount() {
    return _alertCount;
}

void alert(const char* label) {
    for (uint8_t i = ALERT_MAX - 1; i > 0; i--) {
        memcpy(_alerts[i], _alerts[i - 1], ALERT_LEN);
    }
    copyStr(_alerts[0], ALERT_LEN, String(label));
    if (_alertCount < ALERT_MAX) _alertCount++;
}

String alertsJson() {
    String j = "[";
    for (uint8_t i = 0; i < _alertCount; i++) {
        if (i > 0) j += ",";
        j += "\"" + escapeJson(_alerts[i]) + "\"";
    }
    j += "]";
    return j;
}

// ---------------------------------------------------------------- stazioni sconosciute

String unknownDevicesJson() {
    String j = "[";
    struct station_info* info = wifi_softap_get_station_info();
    if (info) {
        String owner = auth::ownerMac();
        owner.toLowerCase();
        bool first = true;
        for (; info != nullptr; info = STAILQ_NEXT(info, next)) {
            char mac[18];
            snprintf(mac, sizeof(mac), "%02X:%02X:%02X:%02X:%02X:%02X",
                     info->bssid[0], info->bssid[1], info->bssid[2],
                     info->bssid[3], info->bssid[4], info->bssid[5]);
            String macLower = String(mac);
            macLower.toLowerCase();
            if (owner.length() != 0 && macLower == owner) continue;
            if (!first) j += ",";
            first = false;
            j += "{\"mac\":\"" + String(mac) + "\",\"ip\":\"";
            if (info->ip.addr != 0) {
                j += IPAddress(info->ip.addr).toString();
            }
            j += "\"}";
        }
        wifi_softap_free_station_info();
    }
    j += "]";
    return j;
}

} // namespace access