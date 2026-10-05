// ============================================================================
// ESP8266 NodeMCU + SSD1306 OLED - ULTRA OPTIMIZED
// ============================================================================

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "RobotEyes.h"
#include <pk_api.h>
#include <pk_net.h>

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

// Replace the FPS settings
#define TARGET_FPS    60
#define MIN_FRAME_MS  16



// ============================================================================
// INSTANCES
// ============================================================================

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
RobotEyes eyes;

// ============================================================================
// STATE
// ============================================================================

uint32_t lastUpdate = 0;
uint32_t emotionTimer = 0;
uint8_t emotionIndex = 0;
uint32_t animTimer = 0;
uint8_t animIndex = 0;

// ============================================================================
// EMOTION LIST
// ============================================================================

const EyeEmotion emotions[] = {
    EMOTION_NEUTRAL,
    EMOTION_HAPPY,
    EMOTION_ANGRY,
    EMOTION_SAD,
    EMOTION_SLEEPY,
    EMOTION_SURPRISED,
    EMOTION_CURIOUS,
    EMOTION_SCARED,
    EMOTION_LOVE,
    EMOTION_LAUGHING,
    EMOTION_THINKING,
    EMOTION_WINK_LEFT,
    EMOTION_WINK_RIGHT
};
const uint8_t numEmotions = 13;

// ============================================================================
// SETUP
// ============================================================================

void setup() {
    Serial.begin(115200);
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
    
    // Enable features
    eyes.setAutoblinker(true);
    eyes.setBreathing(true);
    eyes.setIdleMovement(true);
    
    // Startup animation
    eyes.playStartup();
    delay(300);
    
    // Start with neutral
    eyes.setEmotion(EMOTION_NEUTRAL);
    
    // Initialize timers
    lastUpdate = millis();
    emotionTimer = millis();
    animTimer = millis();
    
    // PK Net / Wi‑Fi init
    pk_net::begin();

    // API server
    api::begin();

    Serial.println(F("=== READY ==="));
    Serial.println(F("Commands: emotion:neutral|happy|angry|sad|sleepy|surprised|curious|scared|love|laughing|thinking|wink_left|wink_right"));
    Serial.println(F("          anim:blink|double|wink_left|wink_right"));
    Serial.println(F("          sleep:on|off  breath:on|off"));
}

// ============================================================================
// LOOP - ULTRA FAST
// ============================================================================

void loop() {
    uint32_t now = millis();

    // Add FPS monitoring in loop()
if (eyes.getFPS() < 50) {
    Serial.print("Warning: FPS dropped to ");
    Serial.println(eyes.getFPS());
}
    
    // Frame rate control
    if (now - lastUpdate < MIN_FRAME_MS) {
        return;
    }
    lastUpdate = now;
    
    // ========================================================================
    // UPDATE EYES - THIS IS THE MAIN RENDER LOOP
    // ========================================================================
    eyes.update();
    
    // ========================================================================
    // CONTINUOUS EMOTION CYCLE
    // ========================================================================
    if (!eyes.isAnimating() && now - emotionTimer > 6000) {
        emotionTimer = now;
        emotionIndex = (emotionIndex + 1) % numEmotions;
        eyes.setEmotionWithTransition(emotions[emotionIndex], 400);
        Serial.print(F("Emotion: "));
        Serial.println(emotionIndex);
    }
    
    // ========================================================================
    // CONTINUOUS ANIMATIONS (every 30 seconds)
    // ========================================================================
    if (!eyes.isAnimating() && now - animTimer > 30000) {
        animTimer = now;
        uint8_t type = random(0, 4);
        switch (type) {
            case 0: eyes.playBlink(); break;
            case 1: eyes.playDoubleBlink(); break;
            case 2: eyes.playWinkLeft(); break;
            case 3: eyes.playWinkRight(); break;
        }
        Serial.print(F("Animation: "));
        Serial.println(type);
    }
    
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
    
    if (cmd == "sleep:on") { eyes.setSleepMode(true); Serial.println(F("Sleep ON")); return; }
    if (cmd == "sleep:off") { eyes.setSleepMode(false); Serial.println(F("Sleep OFF")); return; }
    if (cmd == "breath:on") { eyes.setBreathing(true); Serial.println(F("Breath ON")); return; }
    if (cmd == "breath:off") { eyes.setBreathing(false); Serial.println(F("Breath OFF")); return; }
    if (cmd == "fps") { Serial.print(F("FPS: ")); Serial.println(eyes.getFPS()); return; }
    
    Serial.println(F("Unknown command"));
}
