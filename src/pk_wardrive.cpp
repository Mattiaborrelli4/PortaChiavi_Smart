#include <pk_wardrive.h>
#include <LittleFS.h>

#include <string.h>
#include <time.h>

#define WR_FILE   "/wardrive.csv"
#define DEDUP_MAX 48

namespace pk_wardrive {

static bool     _fsOk = false;
static bool     _enabled = false;
static uint32_t _entries = 0;

static uint8_t  _dupMac[DEDUP_MAX][6];
static uint8_t  _dupCount = 0;

// ---------------------------------------------------------------- helper mac

static uint8_t hx(char c) {
    if (c >= '0' && c <= '9') return (uint8_t)(c - '0');
    if (c >= 'a' && c <= 'f') return (uint8_t)(c - 'a' + 10);
    if (c >= 'A' && c <= 'F') return (uint8_t)(c - 'A' + 10);
    return 0;
}

static bool parseMacB(const char* s, uint8_t* out) {
    int pos = 0, idx = 0;
    while (idx < 6) {
        char a = s[pos], b = s[pos + 1];
        if (a == '\0' || b == '\0') break;
        out[idx++] = (uint8_t)((hx(a) << 4) | hx(b));
        pos += 2;
        if (s[pos] == ':') pos++;
    }
    return idx == 6;
}

static bool macInDedup(const uint8_t* m) {
    for (int i = 0; i < _dupCount; i++) {
        if (memcmp(_dupMac[i], m, 6) == 0) return true;
    }
    return false;
}

static void addToDedup(const uint8_t* m) {
    if (_dupCount >= DEDUP_MAX) memmove(&_dupMac[0], &_dupMac[1], sizeof(_dupMac) - sizeof(_dupMac[0]));
    if (_dupCount >= DEDUP_MAX) _dupCount = DEDUP_MAX - 1;
    memcpy(_dupMac[_dupCount], m, 6);
    _dupCount++;
}

// ---------------------------------------------------------------- timestamp

static void tsNow(char* out, size_t n) {
    time_t t = time(nullptr);
    if (t < 500000000) t = 500000000 + (time_t)(millis() / 1000); // epoca fittizia + uptime
    struct tm* g = gmtime(&t);
    strftime(out, n, "%Y-%m-%d %H:%M:%S", g);
}

// ---------------------------------------------------------------- csv row

static String csvField(const char* s) {
    String v = String(s);
    if (v.indexOf(',') >= 0 || v.indexOf('"') >= 0) {
        v.replace("\"", "\"\"");
        return "\"" + v + "\"";
    }
    return v;
}

static const char* wifiAuth(uint8_t enc) {
    switch (enc) {
        case 0x01: return "[WEP][ESS]";                // WEP
        case 0x02: return "[WPA-PSK-CCMP+TKIP][ESS]"; // WPA
        case 0x04: return "[WPA2-PSK-CCMP+TKIP][ESS]";// WPA2
        default:   return "[ESS]";                     // OPEN / altri
    }
}

// ---------------------------------------------------------------- api

bool begin() {
    if (!LittleFS.begin()) {
        _fsOk = false;
        return false;
    }
    _fsOk = true;
    if (LittleFS.exists(WR_FILE)) {
        File f = LittleFS.open(WR_FILE, "r");
        if (f) {
            while (f.available()) {
                if (f.read() == '\n') _entries++;
            }
            f.close();
        }
    }
    return true;
}

bool fsOk() {
    return _fsOk;
}

String setEnabled(bool on) {
    _enabled = on;
    return status();
}

bool enabled() {
    return _enabled;
}

String status() {
    String j = "{";
    j += "\"enabled\":" + String(_enabled ? "true" : "false") + ",";
    j += "\"entries\":" + String((uint32_t)_entries) + ",";
    j += "\"size\":" + String(fileSize()) + ",";
    j += "\"fs\":" + String(_fsOk ? "true" : "false");
    j += "}";
    return j;
}

String clear() {
    if (_fsOk) LittleFS.remove(WR_FILE);
    _entries = 0;
    _dupCount = 0;
    return status();
}

bool hasData() {
    return _entries > 0;
}

uint32_t fileSize() {
    if (!_fsOk || !LittleFS.exists(WR_FILE)) return 0;
    File f = LittleFS.open(WR_FILE, "r");
    uint32_t sz = f ? f.size() : 0;
    if (f) f.close();
    return sz;
}

void record(const char* bssid, const char* ssid, int8_t rssi,
            uint8_t channel, uint8_t enc) {
    if (!_fsOk || !_enabled) return;

    uint8_t mac[6];
    if (!parseMacB(bssid, mac)) return;
    if (macInDedup(mac)) return;
    addToDedup(mac);

    char ts[24];
    tsNow(ts, sizeof(ts));

    File f = LittleFS.open(WR_FILE, "a");
    if (!f) return;
    bool fresh = LittleFS.exists(WR_FILE) && f.size() == 0;
    if (fresh) {
        f.println("WigleWifi-1.4,appRelease=1.0,model=ESP8266,release=0.1.0,"
                  "device=PortaChiave,display=web,board=NodeMCU");
        f.println("MAC,SSID,AuthMode,FirstSeen,Channel,RSSI,CurrentLatitude,"
                  "CurrentLongitude,AltitudeMeters,AccuracyMeters,Type");
    }
    f.print(csvField(bssid)); f.print(",");
    f.print(csvField(ssid));  f.print(",");
    f.print(wifiAuth(enc));   f.print(",");
    f.print(ts);              f.print(",");
    f.print(channel);         f.print(",");
    f.print(rssi);            f.print(",");
    f.print("0.000000,0.000000,0,0,WIFI\n");
    f.close();

    _entries++;
}

} // namespace pk_wardrive