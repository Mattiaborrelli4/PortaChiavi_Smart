#include "wifi/wifi_manager.h"
#include <ESP8266WiFi.h>
#include <sys/queue.h>

// API SDK dal softAP: l'eventuale bug del core 3.1.x rende
// wifi_softap_get_station_num() inaffidabile; la lista stazioni no.
extern "C" {
struct station_info* wifi_softap_get_station_info(void);
void wifi_softap_free_station_info(void);
}

WiFiManager wifiManager;

void WiFiManager::begin(const char* ssid, const char* password) {
    _ssid = ssid;

    WiFi.mode(WIFI_AP);

    // SSID visibile (compare nella lista Wi-Fi) + password WPA2:
    // solo chi conosce la password puo' connettersi.
    const uint8_t channel   = 1;
    const bool    ssidHidden = false;
    bool ok = password ? WiFi.softAP(ssid, password, channel, ssidHidden)
                       : WiFi.softAP(ssid, nullptr, channel, ssidHidden);

    Serial.println();
    Serial.println(F("[WiFi] Manager inizializzato"));
    Serial.println(F("[WiFi] Modalita: ACCESS POINT"));
    Serial.print(F("[WiFi] SSID: "));
    Serial.println(_ssid);
    Serial.print(F("[WiFi] Password: "));
    if (password) {
        Serial.println(password);
    } else {
        Serial.println(F("(nessuna)"));
    }

    if (ok) {
        Serial.print(F("[WiFi] IP: "));
        Serial.println(WiFi.softAPIP());
    } else {
        Serial.println(F("[WiFi] ERRORE: softAP non avviato"));
    }
}

void WiFiManager::update() {
    // Il rilevamento dei client avviene on-demand tramite hasClient().
}

bool WiFiManager::hasClient() const {
    struct station_info* info = wifi_softap_get_station_info();
    if (info) {
        wifi_softap_free_station_info();
        return true;
    }
    return false;
}

uint8_t WiFiManager::clientCount() const {
    uint8_t n = 0;
    for (struct station_info* info = wifi_softap_get_station_info();
         info != nullptr; info = STAILQ_NEXT(info, next)) {
        n++;
    }
    wifi_softap_free_station_info();
    return n;
}

bool WiFiManager::isConnected() const {
    return hasClient();
}

String WiFiManager::getSSID() const {
    return _ssid;
}

String WiFiManager::getIP() const {
    return WiFi.softAPIP().toString();
}

String WiFiManager::getClientsDebug() const {
    String out;
    struct station_info* info = wifi_softap_get_station_info();
    if (!info) {
        return F("nessun client associato");
    }
    uint8_t n = 0;
    for (; info != nullptr; info = STAILQ_NEXT(info, next)) {
        n++;
        out += " - ";
        for (int i = 0; i < 6; i++) {
            out += (info->bssid[i] < 0x10 ? "0" : "");
            out += String(info->bssid[i], HEX);
            if (i < 5) out += ":";
        }
        if (info->ip.addr != 0) {
            out += " IP=";
            out += IPAddress(info->ip.addr).toString();
        }
        out += "\r\n";
    }
    wifi_softap_free_station_info();
    String res = "Client associati: ";
    res += String(n);
    res += "\r\n";
    res += out;
    return res;
}
