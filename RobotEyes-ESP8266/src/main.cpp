// ============================================================================
// ESP8266 NodeMCU + SSD1306 OLED - ULTRA OPTIMIZED
// ============================================================================

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "RobotEyes.h"
#include "wifi/wifi_manager.h"

void handleCommand(String cmd);
void goToSleep();
void wakeUp();

// ============================================================================
// HARDWARE CONFIG
// ============================================================================

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define OLED_ADDRESS  0x3C
#define SDA_PIN       4
#define SCL_PIN       5

// ============================================================================
// PERFORMANCE
// ============================================================================

#define TARGET_FPS    60
#define MIN_FRAME_MS  16

// ============================================================================
// WIFI ACCESS POINT - only devices with SSID + password can join
// ============================================================================

#define AP_SSID      "RobotEyes"
#define AP_PASSWORD  "RobotEyes2026"

// ============================================================================
// INSTANCES
// ============================================================================

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
RobotEyes eyes;

// ============================================================================
// STATE
// ============================================================================

uint32_t lastUpdate = 0;
bool robotAwake = false;
bool lastClientState = false;

// ============================================================================
// SLEEP / WAKE
// ============================================================================

void goToSleep() {
    robotAwake = false;
    eyes.setSleepMode(true);
}

void wakeUp() {
    robotAwake = true;
    eyes.setSleepMode(false);
    eyes.setEmotionWithTransition(EMOTION_NEUTRAL, 600);
    eyes.setAutoblinker(true);
    eyes.setIdleMovement(true);
}

// ============================================================================
// SETUP
// ============================================================================

void setup() {
    Serial.begin(115200);
    wifiManager.begin(AP_SSID, AP_PASSWORD);

    Serial.println(F("\n=== ROBOT EYES ULTRA OPTIMIZED ==="));

    // Fast I2C
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
    display.println(F("v3.0 Optimized"));
    display.display();
    delay(400);

    // Init eyes
    eyes.begin(&display, SCREEN_WIDTH, SCREEN_HEIGHT, TARGET_FPS);
    eyes.setEyeColor(SSD1306_WHITE);
    eyes.setPupilColor(SSD1306_BLACK);

    // Startup animation
    eyes.playStartup();
    delay(300);

    // Start asleep (half-closed eyes, waiting for the phone)
    goToSleep();

    lastUpdate = millis();
    lastClientState = false;

    Serial.println(F("=== READY (SLEEPING) ==="));
    Serial.print(F("Rete WiFi '"));
    Serial.print(wifiManager.getSSID());
    Serial.print(F("', password '"));
    Serial.print(AP_PASSWORD);
    Serial.println(F("' (solo dispositivi autorizzati)"));
    Serial.println(F("Commands: emotion:neutral|happy|angry|sad|sleepy|surprised|curious|scared|love|laughing|thinking|wink_left|wink_right"));
    Serial.println(F("          anim:blink|double|wink_left|wink_right"));
    Serial.println(F("          sleep:on|off  breath:on|off  fps"));
}

// ============================================================================
// LOOP - ULTRA FAST
// ============================================================================

void loop() {
    uint32_t now = millis();

    // Frame rate control
    if (now - lastUpdate < MIN_FRAME_MS) {
        return;
    }
    lastUpdate = now;

    wifiManager.update();

    // ========================================================================
    // WIFI PRESENCE -> SLEEP / WAKE
    // ========================================================================
    bool client = wifiManager.hasClient();
    if (client != lastClientState) {
        lastClientState = client;
        if (client) {
            Serial.println(F("Phone connesso -> WAKE"));
            wakeUp();
        } else {
            Serial.println(F("Phone disconnesso -> SLEEP"));
            goToSleep();
        }
    }

    // ========================================================================
    // UPDATE EYES - THIS IS THE MAIN RENDER LOOP
    // ========================================================================
    eyes.update();

    // ========================================================================
    // SERIAL COMMANDS
    // ========================================================================
    if (Serial.available()) {
        String cmd = Serial.readStringUntil('\n');
        cmd.trim();
        handleCommand(cmd);
    }
}

// ============================================================================
// COMMAND HANDLER
// ============================================================================

void handleCommand(String cmd) {
    if (cmd == "help") {
        Serial.println(F("emotion:neutral|happy|angry|sad|sleepy|surprised|curious|scared|love|laughing|thinking|wink_left|wink_right"));
        Serial.println(F("anim:blink|double|wink_left|wink_right"));
        Serial.println(F("sleep:on|off  breath:on|off  fps"));
        return;
    }

    if (cmd.startsWith("emotion:")) {
        String v = cmd.substring(8);
        v.toLowerCase();
        if (v == "neutral") eyes.setEmotionWithTransition(EMOTION_NEUTRAL, 400);
        else if (v == "happy") eyes.setEmotionWithTransition(EMOTION_HAPPY, 400);
        else if (v == "angry") eyes.setEmotionWithTransition(EMOTION_ANGRY, 400);
        else if (v == "sad") eyes.setEmotionWithTransition(EMOTION_SAD, 400);
        else if (v == "sleepy") eyes.setEmotionWithTransition(EMOTION_SLEEPY, 500);
        else if (v == "surprised") eyes.setEmotionWithTransition(EMOTION_SURPRISED, 300);
        else if (v == "curious") eyes.setEmotionWithTransition(EMOTION_CURIOUS, 400);
        else if (v == "scared") eyes.setEmotionWithTransition(EMOTION_SCARED, 300);
        else if (v == "love") eyes.setEmotionWithTransition(EMOTION_LOVE, 500);
        else if (v == "laughing") eyes.setEmotionWithTransition(EMOTION_LAUGHING, 300);
        else if (v == "thinking") eyes.setEmotionWithTransition(EMOTION_THINKING, 400);
        else if (v == "wink_left") eyes.setEmotionWithTransition(EMOTION_WINK_LEFT, 200);
        else if (v == "wink_right") eyes.setEmotionWithTransition(EMOTION_WINK_RIGHT, 200);
        Serial.println(F("OK"));
        return;
    }

    if (cmd.startsWith("anim:")) {
        String v = cmd.substring(5);
        v.toLowerCase();
        if (v == "blink") eyes.playBlink();
        else if (v == "double") eyes.playDoubleBlink();
        else if (v == "wink_left") eyes.playWinkLeft();
        else if (v == "wink_right") eyes.playWinkRight();
        Serial.println(F("OK"));
        return;
    }

    if (cmd == "sleep:on") { goToSleep(); Serial.println(F("Sleep ON")); return; }
    if (cmd == "sleep:off") { wakeUp(); Serial.println(F("Sleep OFF")); return; }
    if (cmd == "breath:on") { eyes.setBreathing(true); Serial.println(F("Breath ON")); return; }
    if (cmd == "breath:off") { eyes.setBreathing(false); Serial.println(F("Breath OFF")); return; }
    if (cmd == "fps") { Serial.print(F("FPS: ")); Serial.println(eyes.getFPS()); return; }
    if (cmd == "clients") { Serial.print(wifiManager.getClientsDebug()); return; }

    Serial.println(F("Unknown command"));
}
