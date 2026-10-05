#pragma once

#include <Arduino.h>

class WiFiManager {
public:
    void begin(const char* ssid = "RobotEyes", const char* password = nullptr);
    void update();

    bool isConnected() const;
    bool hasClient() const;
    uint8_t clientCount() const;

    String getSSID() const;
    String getIP() const;

private:
    String _ssid;
};

extern WiFiManager wifiManager;
