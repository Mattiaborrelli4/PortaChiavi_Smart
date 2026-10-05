#include <value_store.h>
#include <pk_robot.h>

#include <vector>
#include <algorithm>
#include <LittleFS.h>

namespace value_store {

static std::vector<String> keys;
static String valueMap[VALUE_MAP_SIZE];

static String escapeJson(const String& s) {
    String out = s;
    out.replace("\\", "\\\\");
    out.replace("\"", "\\\"");
    out.replace("\r", " ");
    out.replace("\n", " ");
    return out;
}

uint32_t hashString(const String& s) {
    uint32_t hashcode = 0;
    for (unsigned int i = 0; i < s.length(); i++) {
        hashcode += (uint8_t)s.charAt(i);
    }
    return hashcode;
}

void setup() {
    keys.clear();
}

void setValue(const String& key, const String& value) {
    if (key.length() == 0) return;
    if (std::find(keys.begin(), keys.end(), key) == keys.end()) {
        if (keys.size() < VALUE_MAP_SIZE) {
            keys.push_back(key);
        }
    }
    valueMap[hashString(key) % VALUE_MAP_SIZE] = value;
}

String toJson() {
    String json = "[";
    for (size_t i = 0; i < keys.size(); i++) {
        const String& k = keys[i];
        String v = valueMap[hashString(k) % VALUE_MAP_SIZE];
        json += "{\"name\":\"" + escapeJson(k) + "\",\"value\":\"" + escapeJson(v) + "\"}";
        if (i + 1 < keys.size()) json += ",";
    }
    json += "]";
    return json;
}

static void handleLineInternal(const String& line) {
    if (line.length() == 0) return;

    // Comandi robot (emotion:..., anim:..., sleep:.., breath:..) -> RobotEyes
    if (line.startsWith("emotion:") ||
        line.startsWith("anim:")    ||
        line.startsWith("sleep:")   ||
        line.startsWith("breath:")  ||
        line == "fps") {
        if (pk_robot::handleCommand(line)) {
            return;
        }
    }

    // Protocollo key:value legacy
    int idx = line.indexOf(':');
    if (idx > 0) {
        String key = line.substring(0, idx);
        key.trim();
        String value = line.substring(idx + 1);
        value.trim();
        setValue(key, value);
    }
}

void handleLine(const String& line) {
    String trimmed = line;
    trimmed.trim();
    handleLineInternal(trimmed);
}

String dump() {
    String out = "keys=" + String((int)keys.size()) + "\r\n";
    for (size_t i = 0; i < keys.size(); i++) {
        const String& k = keys[i];
        String v = valueMap[hashString(k) % VALUE_MAP_SIZE];
        out += k + " = " + v + "\r\n";
    }
    if (keys.size() == 0) out += "(vuoto)\r\n";
    return out;
}

// Chiave valida solo se non vuota e composta da caratteri "sicuri"
// (alphanumerici + _ - .). Evita "" "/" "\" e qualsiasi carattere strano
// che potrebbe creare percorsi/valori imprevisti su LittleFS.
static bool keyIsValid(const String& key) {
    if (key.length() == 0 || key.length() > 32) return false;
    for (unsigned int i = 0; i < key.length(); i++) {
        char c = key.charAt(i);
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.')) {
            return false;
        }
    }
    return true;
}

bool settingsSave(const String& key, const String& value) {
    if (!keyIsValid(key)) return false;
    if (!LittleFS.begin()) return false;
    if (!LittleFS.exists("/settings")) {
        if (!LittleFS.mkdir("/settings")) return false;
    }
    File f = LittleFS.open("/settings/" + key, "w");
    if (!f) return false;
    size_t written = f.print(value);
    f.close();
    return written == (size_t)value.length();
}

String settingsLoad(const String& key) {
    if (!keyIsValid(key)) return "";
    if (!LittleFS.begin()) return "";
    if (!LittleFS.exists("/settings/" + key)) return "";
    File f = LittleFS.open("/settings/" + key, "r");
    if (!f) return "";
    String out = f.readString();
    f.close();
    return out;
}

} // namespace value_store