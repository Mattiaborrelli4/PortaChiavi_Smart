#ifndef RobotEyes_h
#define RobotEyes_h

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

enum EyeEmotion : uint8_t {
    EMOTION_NEUTRAL = 0,
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
    EMOTION_WINK_RIGHT,
    EMOTION_SUSPICIOUS,
    EMOTION_FOCUSED,
    EMOTION_ALERT,
    EMOTION_CONFUSED,
    EMOTION_SCANNING,
    EMOTION_LAG
};

enum EyeBehavior : uint8_t {
    BEHAVIOR_IDLE = 0,
    BEHAVIOR_CENTER,
    BEHAVIOR_LOOK_LEFT,
    BEHAVIOR_LOOK_RIGHT,
    BEHAVIOR_LOOK_UP,
    BEHAVIOR_LOOK_DOWN,
    BEHAVIOR_LOOK_UP_LEFT,
    BEHAVIOR_LOOK_UP_RIGHT,
    BEHAVIOR_LOOK_DOWN_LEFT,
    BEHAVIOR_LOOK_DOWN_RIGHT,
    BEHAVIOR_LOOK_AROUND,
    BEHAVIOR_SCAN_LEFT_RIGHT,
    BEHAVIOR_SLOW_DRIFT,
    BEHAVIOR_FAST_LOOK,
    BEHAVIOR_BLINK,
    BEHAVIOR_DOUBLE_BLINK,
    BEHAVIOR_MICRO_SACCADE,
    BEHAVIOR_ALERT_VISUAL
};

enum EyeEvent : uint8_t {
    EYE_EVENT_MENU_OPEN = 0,
    EYE_EVENT_INFO_OPEN,
    EYE_EVENT_NETWORK_OPEN,
    EYE_EVENT_WIFI_SCAN_START,
    EYE_EVENT_CAMERA_OPEN,
    EYE_EVENT_QR_GENERATED,
    EYE_EVENT_QR_SCANNED,
    EYE_EVENT_CAMERA_CONNECTED,
    EYE_EVENT_CAMERA_DISCONNECTED,
    EYE_EVENT_SECURITY_OPEN,
    EYE_EVENT_SETTING_CHANGED,
    EYE_EVENT_ERROR
};

enum EyePosition : uint8_t {
    POS_CENTER = 0,
    POS_N,
    POS_NE,
    POS_E,
    POS_SE,
    POS_S,
    POS_SW,
    POS_W,
    POS_NW
};

class RobotEyes {
public:
    RobotEyes();
    ~RobotEyes();

    void begin(Adafruit_SSD1306* display, uint16_t screenWidth, uint16_t screenHeight);
    void begin(Adafruit_SSD1306* display, uint16_t screenWidth, uint16_t screenHeight, uint8_t maxFps);
    
    void update();
    void refresh();
    void setFramerate(uint8_t fps);
    uint8_t getFramerate() const;
    
    void setEyeSize(uint8_t width, uint8_t height);
    void setBorderRadius(uint8_t radius);
    void setSpaceBetween(int16_t space);
    
    void setPosition(EyePosition pos);
    void setPupilOffset(int8_t x, int8_t y);
    
    void setEyelidOpen(uint8_t amount);
    void openEyes();
    void closeEyes();
    void setEyelids(uint8_t left, uint8_t right);
    void openLeftEye();
    void openRightEye();
    void closeLeftEye();
    void closeRightEye();
    
    void setPupilSize(uint8_t size);
    void setPupilPosition(int8_t x, int8_t y);
    void setPupils(int8_t lx, int8_t ly, int8_t rx, int8_t ry);
    
    void setEmotion(EyeEmotion emotion);
    void setEmotionWithTransition(EyeEmotion emotion, uint16_t transitionMs);
    EyeEmotion getEmotion() const;
    
    void setAutoblinker(bool enable);
    void setBreathing(bool enable);
    void setSleepMode(bool enable);
    void setIdleMovement(bool enable);
    void setRandomBehavior(bool enable);
    void setMovementAmplitude(uint8_t amplitude);
    void setMovementSpeed(uint8_t speed);
    uint8_t getMovementAmplitude() const;
    uint8_t getMovementSpeed() const;
    EyeBehavior getBehavior() const;
    void setBehavior(EyeBehavior behavior);
    void playBehavior(EyeBehavior behavior);
    void dispatchEvent(EyeEvent event);
    
    void playBlink();
    void playDoubleBlink();
    void playWinkLeft();
    void playWinkRight();
    void playStartup();
    void playShutdown();
    
    void playLag();
    bool isLagPlaying() const;
    
    bool isAnimating() const;
    bool isSleeping() const;
    uint16_t getFPS() const;
    
    void setEyeColor(uint16_t color);
    void setPupilColor(uint16_t color);
    void setInverted(bool inverted);
    
private:
    struct EyeState {
        uint8_t width;
        uint8_t height;
        uint8_t radius;
        int8_t pupilX;
        int8_t pupilY;
        uint8_t pupilSize;
        uint8_t eyelidOpen;
        uint8_t targetEyelid;
        bool isOpen;
        int16_t centerX;
        int16_t centerY;
        int8_t tilt;
    };
    
    struct EmotionConfig {
        uint8_t eyeWidth;
        uint8_t eyeHeight;
        uint8_t borderRadius;
        uint8_t pupilSize;
        int8_t pupilX;
        int8_t pupilY;
        uint8_t eyelidOpen;
        int8_t positionXOffset;
        int8_t positionYOffset;
        int8_t leftTilt;
        int8_t rightTilt;
    };
    
    struct AnimState {
        uint32_t startTime;
        uint32_t duration;
        float progress;
        bool active;
        bool loop;
        uint8_t phase;
    };
    
    void drawEyes();
    void drawEye(uint8_t index);
    void updateBehaviors();
    void updateAnimation();
    void updateEmotionTransition();
    void applyEmotion(EyeEmotion emotion);
    void calculatePositions();
    void applyTargetSize();
    void drawSleepIndicator();
    
    float easeInOut(float t);
    float easeOutBack(float t);
    
    void setupNeutral();
    void setupHappy();
    void setupAngry();
    void setupSad();
    void setupSleepy();
    void setupSurprised();
    void setupCurious();
    void setupScared();
    void setupLove();
    void setupLaughing();
    void setupThinking();
    void setupWinkLeft();
    void setupWinkRight();
    void setupSuspicious();
    void setupFocused();
    void setupAlert();
    void setupConfused();
    void setupScanning();
    void setupLag();
    void drawTiltedEye(EyeState* eye, int16_t x, int16_t y, uint8_t height);
    void drawTiltedArea(EyeState* eye, int16_t x, int16_t y, uint8_t w, uint8_t h, uint16_t color);
    void drawFilledEye(EyeState* eye, int16_t x, int16_t y, uint8_t fullH, uint8_t effH);
    void drawEyelidMasks(EyeState* eye, int16_t x, int16_t y, uint8_t fullH, uint8_t effH);
    void drawLagTears(EyeState* eye, int16_t x, int16_t y, uint8_t fullH, uint8_t effH);
    void tearRowRange(int16_t ax, int16_t ay, uint16_t w, uint16_t h, int8_t shift);
    void updateLag();
    void startLook(int8_t x, int8_t y, uint16_t duration);
    
    Adafruit_SSD1306* _display;
    
    uint16_t _screenW;
    uint16_t _screenH;
    uint8_t _maxFps;
    uint32_t _frameInterval;
    uint32_t _lastUpdate;
    uint16_t _currentFps;
    uint32_t _fpsCounter;
    uint32_t _fpsTimer;
    
    EyeState _left;
    EyeState _right;
    EyeState _leftTarget;
    EyeState _rightTarget;
    
    EmotionConfig _emotionConfigs[19];
    
    int16_t _spaceBetween;
    int8_t _posX;
    int8_t _posY;
    EyePosition _position;
    
    EyeEmotion _emotion;
    EyeEmotion _targetEmotion;
    bool _transitioning;
    uint32_t _transitionStart;
    uint32_t _transitionDuration;
    
    bool _autoBlink;
    uint32_t _blinkTimer;
    uint32_t _blinkCooldown;
    bool _breathing;
    uint32_t _breathTimer;
    float _breathPhase;
    bool _sleepMode;
    bool _idleMove;
    bool _randomBehavior;
    uint32_t _idleTimer;
    uint8_t _movementAmplitude;
    uint8_t _movementSpeed;
    EyeBehavior _behavior;
    uint32_t _behaviorEnd;
    int8_t _behaviorStartX;
    int8_t _behaviorStartY;
    int8_t _behaviorTargetX;
    int8_t _behaviorTargetY;
    
    // Sleep animation
    float _sleepPhase;
    uint32_t _sleepTwitchTimer;
    
    // Horizontal gaze (look-around)
    int8_t _gazeX;
    int8_t _gazeTargetX;
    bool _gazeActive;
    uint32_t _gazeTimer;
    
    AnimState _anim;
    bool _animPlaying;
    
    // Sequenza "lag/glitch" (solo resa, mai bloccante)
    bool _lagPlaying;
    uint32_t _lagStart;
    int8_t _lagOffsetX;
    int8_t _lagOffsetY;
    int8_t _lagPupilX;
    int8_t _lagPupilY;
    uint8_t _lagGlitchAmp;
    uint16_t _lagSquint;
    
    uint16_t _eyeColor;
    uint16_t _pupilColor;
    uint16_t _bgColor;
    
    bool _dirty;
    uint8_t _lastLeftEyelid;
    uint8_t _lastRightEyelid;
    int8_t _lastLeftPupilX;
    int8_t _lastLeftPupilY;
    int8_t _lastRightPupilX;
    int8_t _lastRightPupilY;
};

#endif