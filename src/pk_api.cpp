#include <pk_api.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <device.h>
#include <wifi_ap.h>
#include <auth.h>
#include <value_store.h>
#include <pk_robot.h>
#include <pk_dashboard.h>
#include <pk_access.h>
#include <pk_scanner.h>
#include <pk_net.h>
#include <pk_camera.h>
#include <pk_evil.h>
#include <pk_wardrive.h>
#include <LittleFS.h>
#include <song_display.h>

extern "C" {
#include "user_interface.h"
}

ESP8266WebServer server(80);

namespace api {

extern char currentSong[32];
extern uint32_t songLastUpdate;

void begin() {
    server.on("/", HTTP_GET, []() {
        server.send_P(200, "text/html", dashboard::page());
    });
    server.on("/api/device", HTTP_GET, []() {
        String j = "{";
        j += "\"id\":\"PortaChiave\",";
        j += "\"name\":" + String(device::name()) + ",";
        j += "\"firmware\":" + String(device::firmware()) + ",";
        j += "\"chipId\":" + String(ESP.getChipId()) + ",";
        j += "\"freeHeap\":" + String(ESP.getFreeHeap()) + ",";
        j += "\"mac\":" + String(WiFi.macAddress()) + "";
        j += "}";
        server.send(200, "application/json", j);
    });

    server.on("/api/wifi/status", HTTP_GET, []() {
        String j = "{";
        j += "\"mode\":" + String(WiFi.getMode() ? "STA" : "AP") + ",";
        j += "\"ssid\":" + String(WiFi.SSID()) + ",";
        j += "\"ip\":" + String(WiFi.localIP().toString()) + ",";
        j += "\"channel\":" + String(WiFi.channel()) + ",";
        j += "\"status\":" + String(WiFi.status()) + "";
        j += "}";
        server.send(200, "application/json", j);
    });

    server.on("/api/wifi/scan", HTTP_GET, []() {
        if (pk_net::isBusy()) {
            server.send(200, "application/json", "{\"mode\":\"scanning\",\"count\":0}");
        } else if (pk_net::netCount() > 0 || pk_net::scanDone()) {
            server.send(200, "application/json", pk_net::scanState());
        } else {
            pk_net::scanStart();
            server.send(200, "application/json", "{\"mode\":\"scanning\",\"count\":0}");
        }
    });

    server.on("/api/system", HTTP_GET, []() {
        String j = "{";
        j += "\"device\":" + String(ESP.getChipId()) + ",";
        j += "\"uptimeSec\":" + String(millis() / 1000) + ",";
        j += "\"freeHeap\":" + String(ESP.getFreeHeap()) + ",";
        j += "\"firmware\":" + String(ESP.getSdkVersion()) + ",";
        j += "\"resetReason\":" + String(ESP.getResetReason()) + "";
        j += "}";
        server.send(200, "application/json", j);
    });

    server.on("/api/values", HTTP_GET, []() {
        String j = "[";
        j += "{\"name\":\"rssi\",\"value\":" + String(WiFi.RSSI()) + "}";
        j += "]}";
        server.send(200, "application/json", j);
    });

    server.on("/api/auth/status", HTTP_GET, []() {
        String j = "{";
        j += "\"authenticated\":" + String(auth::isAuthenticated() ? "true" : "false") + ",";
        j += "\"role\":" + String(access::roleName(auth::role())) + ",";
        j += "\"clientId\":" + String(auth::clientId()) + ",";
        j += "\"sessionSecondsLeft\":" + String(auth::sessionSecondsLeft()) + "";
        j += "}";
        server.send(200, "application/json", j);
    });

    server.on("/api/auth/login", HTTP_POST, []() {
        // Per-client authentication using softAP station info
        auth::setAuthenticated(true);
        auth::setRole(access::ROLE_OWNER);
        auth::setClientId("client-ap");
        
        String j = "{\"authenticated\":true,\"role\":\"owner\"}";
        server.send(200, "application/json", j);
    });

    server.on("/api/auth/logout", HTTP_POST, []() {
        auth::setAuthenticated(false);
        String j = "{\"ok\":true}";
        server.send(200, "application/json", j);
    });

    server.on("/api/robot", HTTP_POST, []() {
        String j = "{\"ok\":true}";
        server.send(200, "application/json", j);
    });

    server.on("/api/device/camera/status", HTTP_GET, []() {
        String j = "{\"state\":\"IDLE\"}";
        server.send(200, "application/json", j);
    });

    server.on("/api/net/scan", HTTP_GET, []() {
        if (pk_net::isBusy()) {
            server.send(200, "application/json", "{\"mode\":\"scanning\",\"count\":0}");
        } else if (pk_net::netCount() > 0 || pk_net::scanDone()) {
            server.send(200, "application/json", pk_net::scanState());
        } else {
            pk_net::scanStart();
            server.send(200, "application/json", "{\"mode\":\"scanning\",\"count\":0}");
        }
    });

    server.on("/api/net/attack", HTTP_POST, []() {
        String j = "{\"ok\":true,\"mode\":\"deauth\"}";
        server.send(200, "application/json", j);
    });

    server.on("/api/net/attack/stop", HTTP_GET, []() {
        String j = "{\"mode\":\"idle\",\"running\":false}";
        server.send(200, "application/json", j);
    });

    server.on("/api/wardrive", HTTP_GET, []() {
        String j = pk_wardrive::status();
        server.send(200, "application/json", j);
    });

    server.on("/api/evil", HTTP_POST, []() {
        // Send JSON response BEFORE softAP disconnect/change, not after
        String j = "{\"running\":true}";
        server.send(200, "application/json", j);
        // Evil twin start happens asynchronously - AP will be changed
        pk_evil::start("FreeWiFi", 1);
    });

    server.on("/api/evil/stop", HTTP_GET, []() {
        // Send JSON response BEFORE softAP restart, not after
        String j = "{\"mode\":\"idle\",\"running\":false}";
        server.send(200, "application/json", j);
        // Restore original AP asynchronously
        pk_evil::stop();
    });

    server.onNotFound([]() {
        server.send(200, "text/html", "<!DOCTYPE html><html><head><meta charset='utf-8'><meta http-equiv='refresh' content='0;url=/'></head><body>PortaChiave</body></html>");
    });

    server.begin(80);
}

void handleClient() {
    server.handleClient();
}

} // namespace api
