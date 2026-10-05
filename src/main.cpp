#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "RobotEyes.h"
#include <pk_robot.h>
#include <wifi_ap.h>
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

    // Start asleep (half-closed eyes, waiting for the phone)
    eyes.setSleepMode(true);

    // PK Net / Wi‑Fi init
    pk_net::begin();

    // API server
    api::begin();

    Serial.println(F("=== READY ==="));
}

void loop() {
    // Cooperative network scheduler
    pk_net::tick();

    // Run animations
    animations::update();

    // Handle HTTP requests
    api::handleClient();

    // Update eyes - render to display
    eyes.update();

    // Refresh display
    display.display();
}