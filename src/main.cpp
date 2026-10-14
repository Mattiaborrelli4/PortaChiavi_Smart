#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "RobotEyes.h"
#include <pk_robot.h>
#include <wifi_ap.h>
#include <auth.h>
#include <pk_captive.h>
#include <pk_api.h>
#include <pk_net.h>
#include <pk_wardrive.h>
#include <pk_evil.h>
#include <pk_camera.h>
#include <animations.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDRESS 0x3C
#define SDA_PIN 4
#define SCL_PIN 5

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
RobotEyes eyes;

char currentSong[32] = "";
uint32_t songLastUpdate = 0;

// ---- setup / loop (required by Arduino framework) --------------------------

void setup() {
    Serial.begin(115200);

    // Fast I2C for OLED
    Wire.begin(SDA_PIN, SCL_PIN);
    Wire.setClock(400000);

    // Init display
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
        Serial.println(F("Display failed!"));
        for (;;) delay(1000);
    }

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(25, 20);
    display.println(F("RoboEyes"));
    display.setCursor(20, 35);
    display.println(F("v3.0"));
    display.display();
    delay(400);

    // Init eyes with display
    eyes.begin(&display, SCREEN_WIDTH, SCREEN_HEIGHT, 50);
    eyes.setEyeColor(SSD1306_WHITE);
    eyes.setPupilColor(SSD1306_BLACK);

    // Startup animation
    eyes.playStartup();
    delay(300);

    // Eyes start awake (not asleep)
    eyes.setSleepMode(false);

    // Wi-Fi AP initialization - MUST be called for SoftAP to be visible
    wifi_ap::begin();

    // API server
    api::begin();

    // Captive portal initialization - MUST be called after WiFi AP is running
    captive::begin();

    Serial.println(F("=== READY ==="));
}

void loop() {
    // Cooperative network scheduler
    pk_net::tick();
    // Update camera state
    pk_camera::update();

    // Run animations
    animations::update();

    // Handle HTTP requests
    api::handleClient();

    // Process captive portal DNS queries (CRITICAL for iOS captive detection)
    captive::process();

    // Auto-authenticate first connected client
    static bool authSent = false;
    if (!authSent && wifi_ap::hasClient() && !auth::isAuthenticated()) {
        String clientMac = wifi_ap::firstClientMac();
        if (clientMac.length() > 0) {
            auth::setAuthenticated(true, clientMac);
            authSent = true;
            Serial.println(F("[Auth] Auto-authenticated first client"));
        }
    }

    // Sleep/wake eyes based on client connection
    static bool sleepSent = false;
    if (wifi_ap::hasClient()) {
        if (eyes.isSleeping()) {
            eyes.setSleepMode(false);
            sleepSent = false;
            Serial.println(F("[Eyes] Client connected - waking up"));
        }
    } else {
        if (!sleepSent) {
            eyes.setSleepMode(true);
            sleepSent = true;
            Serial.println(F("[Eyes] No clients - sleeping mode"));
        }
    }

    // Update eyes - render to display
    eyes.update();

    // Refresh display
    display.display();
}