#include <wifi_ap.h>
#include <device.h>
#include <ESP8266WiFi.h>
#include <sys/queue.h>
#include <cstdio>

// API SDK del softAP: scegliamo wifi_softap_get_station_info() perche'
// wifi_softap_get_station_num() e' inaffidabile su Arduino core 3.1.x.
extern "C" {
struct station_info* wifi_softap_get_station_info(void);
void wifi_softap_free_station_info(void);
}

namespace wifi_ap {

static String _ssid;

void begin() {
    _ssid = "PortaChiave-" + ::device::macSuffix();

    WiFi.mode(WIFI_AP);
    const uint8_t channel   = 1;
    const bool    hidden    = false;

    bool ok = WiFi.softAP(_ssid.c_str(), AP_PASSWORD, channel, hidden);

    Serial.println();
    Serial.println(F("[WiFi] PortaChiave ACCESS POINT"));
    Serial.print(F("[WiFi] SSID: "));
    Serial.println(_ssid);
    Serial.print(F("[WiFi] Password: "));
    Serial.println(AP_PASSWORD);
    if (ok) {
        Serial.print(F("[WiFi] IP: "));
        Serial.println(WiFi.softAPIP());
    } else {
        Serial.println(F("[WiFi] ERRORE: softAP non avviato"));
    }
}

String ssid() {
    return _ssid;
}

String password() {
    return AP_PASSWORD;
}

bool hasClient() {
    struct station_info* info = wifi_softap_get_station_info();
    if (info) {
        wifi_softap_free_station_info();
        return true;
    }
    return false;
}

uint8_t clientCount() {
    uint8_t n = 0;
    for (struct station_info* info = wifi_softap_get_station_info();
         info != nullptr; info = STAILQ_NEXT(info, next)) {
        n++;
    }
    wifi_softap_free_station_info();
    return n;
}

String clientsDebug() {
    String out = "Client associati: ";
    struct station_info* info = wifi_softap_get_station_info();
    if (!info) {
        out += "0";
        out += "\r\n";
        return out;
    }
    uint8_t n = 0;
    for (; info != nullptr; info = STAILQ_NEXT(info, next)) {
        n++;
        out += "\r\n - ";
        for (int i = 0; i < 6; i++) {
            out += (info->bssid[i] < 0x10 ? "0" : "");
            out += String(info->bssid[i], HEX);
            if (i < 5) out += ":";
        }
        if (info->ip.addr != 0) {
            out += " IP=";
            out += IPAddress(info->ip.addr).toString();
        }
    }
    wifi_softap_free_station_info();
    return out;
}

String firstClientMac() {
    struct station_info* info = wifi_softap_get_station_info();
    if (!info) {
        return "";
    }
    char mac[18];
    snprintf(mac, sizeof(mac), "%02x:%02x:%02x:%02x:%02x:%02x",
             info->bssid[0], info->bssid[1], info->bssid[2],
             info->bssid[3], info->bssid[4], info->bssid[5]);
    wifi_softap_free_station_info();
    return String(mac);
}

String clientsJson() {
    String j = "[";
    struct station_info* info = wifi_softap_get_station_info();
    if (info) {
        bool first = true;
        for (; info != nullptr; info = STAILQ_NEXT(info, next)) {
            if (!first) j += ",";
            first = false;
            char mac[18];
            snprintf(mac, sizeof(mac), "%02x:%02x:%02x:%02x:%02x:%02x",
                     info->bssid[0], info->bssid[1], info->bssid[2],
                     info->bssid[3], info->bssid[4], info->bssid[5]);
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

String statusJson() {
    String j = "{";
    j += "\"auth\":\"public\",";
    j += "\"mode\":\"AP\",";
    j += "\"ssid\":\"" + ssid() + "\",";
    j += "\"ip\":\"" + WiFi.softAPIP().toString() + "\",";
    j += "\"password\":\"" + String(AP_PASSWORD) + "\",";
    j += "\"clients\":" + String((int)clientCount());
    j += "}";
    return j;
}

} // namespace wifi_ap