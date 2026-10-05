#include "wifi/wifi_manager.h"
#include <ESP8266WiFi.h>

WiFiManager wifiManager;

void WiFiManager::begin(const char* ssid, const char* password) {
    _ssid = ssid;

    WiFi.mode(WIFI_AP);

    // SSID nascosto (ultimo parametro = hidden) + password WPA2:
    // solo chi conosce SSID e password puo' connettersi.
    const uint8_t channel   = 1;
    const bool    ssidHidden = true;
    bool ok = password ? WiFi.softAP(ssid, password, channel, ssidHidden)
                       : WiFi.softAP(ssid, nullptr, channel, ssidHidden);

    Serial.println();
    Serial.println(F("[WiFi] Manager inizializzato"));
    Serial.println(F("[WiFi] Modalita: ACCESS POINT (SSID nascosto)"));
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
    return WiFi.softAPgetStationNum() > 0;
}

uint8_t WiFiManager::clientCount() const {
    return WiFi.softAPgetStationNum();
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
