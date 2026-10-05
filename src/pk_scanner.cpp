#include <pk_scanner.h>
#include <ESP8266WiFi.h>

namespace pk_scanner {

static const int SCAN_LIMIT = 12;

static String sanitizeSsid(const String& raw) {
    String out;
    for (unsigned int i = 0; i < raw.length(); i++) {
        char c = raw.charAt(i);
        if (c == '"' || c == '\\' || (uint8_t)c < 0x20) continue;
        out += c;
        if (out.length() >= 32) break;
    }
    if (out.length() == 0) return "<nascosta>";
    return out;
}

static void securityInfo(uint8_t encType, String& name, String& risk, String& note) {
    switch (encType) {
        case ENC_TYPE_NONE:
            name = "OPEN";
            risk = "high";
            note = "rete senza cifratura: traffico in chiaro";
            break;
        case ENC_TYPE_WEP:
            name = "WEP";
            risk = "critical";
            note = "WEP deprecato e vulnerabile";
            break;
        case ENC_TYPE_TKIP:
            name = "WPA-TKIP";
            risk = "high";
            note = "cifratura legacy TKIP debole";
            break;
        case ENC_TYPE_CCMP:
            name = "WPA2-AES";
            risk = "good";
            note = "WPA2 con cifratura moderna";
            break;
        case ENC_TYPE_AUTO:
            name = "WPA/WPA2";
            risk = "normal";
            note = "modalita' di compatibilita'";
            break;
        default:
            name = "UNKNOWN";
            risk = "unknown";
            note = "tipo di protezione non rilevabile";
            break;
    }
}

String scanJson() {
    int n = WiFi.scanNetworks(false, true);
    if (n < 0) {
        return "{\"status\":\"failed\",\"networks\":[],\"mode\":\"real\"}";
    }
    int count = n > SCAN_LIMIT ? SCAN_LIMIT : n;

    String j = "{\"status\":\"completed\",\"mode\":\"real\",\"count\":";
    j += String(count);
    j += ",\"networks\":[";
    for (int i = 0; i < count; i++) {
        if (i > 0) j += ",";
        String secName, secRisk, secNote;
        securityInfo(WiFi.encryptionType(i), secName, secRisk, secNote);

        j += "{\"ssid\":\"" + sanitizeSsid(WiFi.SSID(i)) + "\",";
        j += "\"bssid\":\"" + WiFi.BSSIDstr(i) + "\",";
        j += "\"rssi\":" + String(WiFi.RSSI(i)) + ",";
        j += "\"channel\":" + String(WiFi.channel(i)) + ",";
        j += "\"security\":\"" + secName + "\",";
        j += "\"risk\":\"" + secRisk + "\",";
        j += "\"note\":\"" + secNote + "\"}";
    }
    j += "]}";
    WiFi.scanDelete();
    return j;
}

String decoyJson() {
    String j = "{\"status\":\"completed\",\"mode\":\"decoy\",\"count\":3,\"networks\":[";
    j += "{\"ssid\":\"Rete_01\",\"rssi\":-45,\"channel\":1,\"risk\":\"good\",\"note\":\"rete fittizia di test\"},";
    j += "{\"ssid\":\"Rete_02\",\"rssi\":-60,\"channel\":6,\"risk\":\"normal\",\"note\":\"rete fittizia di test\"},";
    j += "{\"ssid\":\"Rete_03\",\"rssi\":-72,\"channel\":11,\"risk\":\"normal\",\"note\":\"rete fittizia di test\"}";
    j += "]}";
    return j;
}

} // namespace pk_scanner