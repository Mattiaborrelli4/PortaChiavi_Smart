#include "RobotEyes.h"
#include <math.h>

// ============================================================================
// CONSTRUCTOR
// ============================================================================

RobotEyes::RobotEyes()
    : _display(nullptr)
    , _screenW(128)
    , _screenH(64)
    , _maxFps(50)
    , _frameInterval(20)
    , _lastUpdate(0)
    , _currentFps(0)
    , _fpsCounter(0)
    , _fpsTimer(0)
    , _spaceBetween(8)
    , _posX(0)
    , _posY(0)
    , _position(POS_CENTER)
    , _emotion(EMOTION_NEUTRAL)
    , _targetEmotion(EMOTION_NEUTRAL)
    , _transitioning(false)
    , _transitionStart(0)
    , _transitionDuration(300)
    , _autoBlink(false)
    , _blinkTimer(0)
    , _blinkCooldown(0)
    , _breathing(false)
    , _breathTimer(0)
    , _breathPhase(0)
    , _sleepMode(false)
    , _idleMove(false)
    , _randomBehavior(true)
    , _idleTimer(0)
    , _movementAmplitude(16)
    , _movementSpeed(100)
    , _behavior(BEHAVIOR_IDLE)
    , _behaviorEnd(0)
    , _behaviorStartX(0)
    , _behaviorStartY(0)
    , _behaviorTargetX(0)
    , _behaviorTargetY(0)
    , _sleepPhase(0)
    , _sleepTwitchTimer(0)
    , _gazeX(0)
    , _gazeTargetX(0)
    , _gazeActive(false)
    , _gazeTimer(0)
    , _animPlaying(false)
    , _lagPlaying(false)
    , _lagStart(0)
    , _lagOffsetX(0)
    , _lagOffsetY(0)
    , _lagPupilX(0)
    , _lagPupilY(0)
    , _lagGlitchAmp(0)
    , _lagSquint(255)
    , _eyeColor(SSD1306_WHITE)
    , _pupilColor(SSD1306_BLACK)
    , _bgColor(SSD1306_BLACK)
    , _dirty(true)
    , _lastLeftEyelid(255)
    , _lastRightEyelid(255)
    , _lastLeftPupilX(0)
    , _lastLeftPupilY(0)
    , _lastRightPupilX(0)
    , _lastRightPupilY(0)
{
    // Initialize eyes
    _left.width = 30;
    _left.height = 30;
    _left.radius = 6;
    _left.pupilX = 0;
    _left.pupilY = 0;
    _left.pupilSize = 5;
    _left.eyelidOpen = 255;
    _left.targetEyelid = 255;
    _left.isOpen = true;
    _left.centerX = 0;
    _left.centerY = 0;
    _left.tilt = 0;
    
    _right = _left;
    _leftTarget = _left;
    _rightTarget = _right;
    
    setupNeutral();
}

RobotEyes::~RobotEyes() {}

// ============================================================================
// INITIALIZATION
// ============================================================================

void RobotEyes::begin(Adafruit_SSD1306* display, uint16_t screenW, uint16_t screenH) {
    begin(display, screenW, screenH, 50);
}

void RobotEyes::begin(Adafruit_SSD1306* display, uint16_t screenW, uint16_t screenH, uint8_t fps) {
    _display = display;
    _screenW = screenW;
    _screenH = screenH;
    _maxFps = fps;
    _frameInterval = 1000 / fps;
    _lastUpdate = millis();
    _dirty = true;
    calculatePositions();
    setAutoblinker(true);
    setIdleMovement(true);
    setBreathing(true);
}

// ============================================================================
// MAIN UPDATE
// ============================================================================

void RobotEyes::update() {
    uint32_t now = millis();
    
    // Frame rate control
    if (now - _lastUpdate < _frameInterval) {
        return;
    }
    _lastUpdate = now;
    
    // FPS counter
    _fpsCounter++;
    if (now - _fpsTimer >= 1000) {
        _currentFps = _fpsCounter;
        _fpsCounter = 0;
        _fpsTimer = now;
    }
    
    // Update behaviors
    updateBehaviors();
    
    // Update animations
    updateAnimation();
    
    // Update emotion transition
    updateEmotionTransition();
    
    // Update lag/glitch sequence
    updateLag();
    
    // Smooth eyelid following
    if (_left.eyelidOpen != _left.targetEyelid) {
        int diff = _left.targetEyelid - _left.eyelidOpen;
        _left.eyelidOpen += (diff > 0) ? max(1, diff / 3) : min(-1, diff / 3);
        if (abs(diff) < 3) _left.eyelidOpen = _left.targetEyelid;
        _dirty = true;
    }
    if (_right.eyelidOpen != _right.targetEyelid) {
        int diff = _right.targetEyelid - _right.eyelidOpen;
        _right.eyelidOpen += (diff > 0) ? max(1, diff / 3) : min(-1, diff / 3);
        if (abs(diff) < 3) _right.eyelidOpen = _right.targetEyelid;
        _dirty = true;
    }
    
    // Eye size always follows target (works even without breathing)
    applyTargetSize();
    
    // ========================================================================
    // HORIZONTAL GAZE - looks left/right with smooth easing
    // ========================================================================
    if (_gazeActive && !_sleepMode && _emotion == EMOTION_NEUTRAL && !_animPlaying) {
        if (_gazeX != _gazeTargetX) {
            _gazeX += (_gazeTargetX > _gazeX) ? 1 : -1;
            if (abs(_gazeTargetX - _gazeX) <= 1) _gazeX = _gazeTargetX;
            _leftTarget.pupilX = _gazeX;
            _rightTarget.pupilX = _gazeX;
            _dirty = true;
        }
    }
    
    // Draw if dirty
    if (_dirty) {
        drawEyes();
        _dirty = false;
    }
}

void RobotEyes::refresh() {
    _dirty = true;
}

// ============================================================================
// DRAW EYES
// ============================================================================

void RobotEyes::drawEyes() {
    if (!_display) return;
    
    _display->clearDisplay();
    if (_bgColor != SSD1306_BLACK) {
        _display->fillRect(0, 0, _screenW, _screenH, _bgColor);
    }
    calculatePositions();
    
    // Sequenza lag: breve deformazione, gli occhi si spostano di qualche pixel
    // e tornano (solo resa, i dati interni restano intatti).
    if (_lagPlaying && (_lagOffsetX != 0 || _lagOffsetY != 0)) {
        _left.centerX += _lagOffsetX;
        _left.centerY += _lagOffsetY;
        _right.centerX += _lagOffsetX;
        _right.centerY += _lagOffsetY;
    }
    
    drawEye(0);
    drawEye(1);
    
    // Sleep indicator: bobbing "Z" at top-left so everyone sees it's sleeping
    if (_sleepMode) {
        drawSleepIndicator();
    }
    
    _display->display();
}

void RobotEyes::drawSleepIndicator() {
    // Z fissa, in alto a sinistra, un po' piu' piccola.
    // Righe 17..28 -> zona "blu" del display bicolore (le prime 16 righe sono gialle).
    const int8_t zy = 17;

    _display->drawFastHLine(4, zy, 11, _eyeColor);
    _display->drawFastHLine(4, zy + 11, 11, _eyeColor);
    _display->drawLine(14, zy, 5, zy + 11, _eyeColor);
}

void RobotEyes::drawEye(uint8_t index) {
    EyeState* eye = (index == 0) ? &_left : &_right;

    int16_t x = eye->centerX;
    int16_t y = eye->centerY;

    // Sleep: striscia bianca per entrambi gli occhi (come stile classico).
    if (_sleepMode) {
        uint8_t stripW = 30;
        uint8_t stripR = 3;
        uint8_t stripH = (uint8_t)(((uint32_t)eye->eyelidOpen * 6) / 102);
        if (stripH < 2) stripH = 2;
        _display->fillRoundRect(x - stripW / 2, y - stripH / 2,
                                stripW, stripH, stripR, _eyeColor);
        return;
    }

    uint8_t fullH = eye->height;
    uint8_t effH = (eye->eyelidOpen * fullH) >> 8;

    // Occhio praticamente chiuso: niente da disegnare (chiuso = nero),
    // così il blink chiude davvero fino in fondo.
    if (eye->eyelidOpen < 40) return;
    if (effH < 2) effH = 2;

    // Il "lag" socchiude leggermente le palpebre (solo in resa, i target restano).
    if (_lagPlaying && _lagSquint < 255) {
        uint16_t sq = ((uint16_t)effH * _lagSquint) / 255;
        if (sq < 2) sq = 2;
        effH = (uint8_t)sq;
    }

    // Stile grafico: un solo renderer (filled), senza pupilla.
    drawFilledEye(eye, x, y, fullH, effH);

    // Blocchi "glitch" sopra l'occhio durante la sequenza lag.
    if (_lagPlaying && _lagGlitchAmp > 0) {
        drawLagTears(eye, x, y, fullH, effH);
    }
}

// ----------------------------------------------------------------------------
// Renderer attivo (unico): occhio pieno e pulito, un'unica forma chiara
// arrotondata, NIENTE pupille né riflessi; le palpebre nere lo coprono con
// le classiche maschere.
// ----------------------------------------------------------------------------
void RobotEyes::drawFilledEye(EyeState* eye, int16_t x, int16_t y, uint8_t fullH, uint8_t effH) {
    int16_t hw = eye->width >> 1;
    int16_t top = y - (fullH >> 1);

    if (eye->tilt == 0) {
        _display->fillRoundRect(x - hw, top, eye->width, fullH, eye->radius, _eyeColor);
    } else {
        drawTiltedArea(eye, x, y, eye->width, fullH, _eyeColor);
    }

    drawEyelidMasks(eye, x, y, fullH, effH);
}

// ----------------------------------------------------------------------------
// Maschere palpebre (sopra/sotto), comuni a tutti gli stili. Restano identiche
// al classico quando non c'è lag; durante il lag forzano lo "sguardo socchiuso"
// anche con eyelidOpen=255 (la palpebra è ridotta solo in resa).
// ----------------------------------------------------------------------------
void RobotEyes::drawEyelidMasks(EyeState* eye, int16_t x, int16_t y, uint8_t fullH, uint8_t effH) {
    if (!_lagPlaying && eye->eyelidOpen >= 240) return;
    if (effH >= fullH) return;

    uint16_t covered = fullH - effH;
    uint16_t halfCover = covered >> 1;
    int16_t hw = eye->width >> 1;
    int16_t top = y - (fullH >> 1);

    if (halfCover > 0) {
        _display->fillRect(x - hw, top, eye->width, halfCover, _bgColor);
        _display->fillRect(x - hw, top + fullH - halfCover, eye->width, halfCover, _bgColor);
    }
}

// ----------------------------------------------------------------------------
// Blocchi glitch: due fasce di righe "strappate" e traslate di 1-3 px a caso,
// dentro la bounding box dell'occhio (effetto schermo/immagine corrotta).
// ----------------------------------------------------------------------------
void RobotEyes::drawLagTears(EyeState* eye, int16_t x, int16_t y, uint8_t fullH, uint8_t effH) {
    if (!_display) return;
    int16_t top = y - (fullH >> 1);
    int16_t hw = eye->width >> 1;
    int16_t bandH = (int16_t)fullH / 7;
    if (bandH < 2) bandH = 2;

    int16_t band1Y = y - (int8_t)(fullH / 8);
    int16_t band2Y = y + (int8_t)(fullH / 10);
    uint8_t amp = _lagGlitchAmp;
    if (amp > 3) amp = 3;

    uint32_t flick = (millis() - _lagStart) / 45;

    for (int b = 0; b < 2; b++) {
        int16_t by = (b == 0) ? band1Y : band2Y;
        if (by + bandH <= top || by >= top + fullH) continue;

        int8_t shift = (b == 0) ? (int8_t)amp : -((int8_t)amp);
        bool on = (b == 0) ? ((flick & 1) != 0) : (((flick >> 1) & 1) != 0);
        if (!on) continue;

        tearRowRange(x - hw + 2, by, (uint16_t)(eye->width) - 4, (uint16_t)bandH, shift);
    }
}

// ----------------------------------------------------------------------------
// Traslazione di fasce di righe nel buffer del display (letto con getPixel e
// riscritto spostato di `shift`, l'origine viene svuotata). Serve solo per il
// glitch del lag, che è puramente visivo.
// ----------------------------------------------------------------------------
void RobotEyes::tearRowRange(int16_t ax, int16_t ay, uint16_t w, uint16_t h, int8_t shift) {
    if (!_display || shift == 0) return;
    int16_t endX = ax + (int16_t)w;
    int16_t screenW = (int16_t)_screenW;
    int16_t screenH = (int16_t)_screenH;

    for (int16_t row = ay; row < ay + (int16_t)h; row++) {
        if (row < 0 || row >= screenH) continue;

        if (shift > 0) {
            for (int16_t col = endX - 1; col >= ax; col--) {
                if (col < 0 || col >= screenW) continue;
                bool c = _display->getPixel(col, row);
                int16_t dest = col + shift;
                if (dest >= 0 && dest < screenW) {
                    _display->drawPixel(dest, row, c ? SSD1306_WHITE : SSD1306_BLACK);
                }
                _display->drawPixel(col, row, SSD1306_BLACK);
            }
        } else {
            for (int16_t col = ax; col < endX; col++) {
                if (col < 0 || col >= screenW) continue;
                bool c = _display->getPixel(col, row);
                int16_t dest = col + shift;
                if (dest >= 0 && dest < screenW) {
                    _display->drawPixel(dest, row, c ? SSD1306_WHITE : SSD1306_BLACK);
                }
                _display->drawPixel(col, row, SSD1306_BLACK);
            }
        }
    }
}


void RobotEyes::drawTiltedArea(EyeState* eye, int16_t x, int16_t y, uint8_t w, uint8_t h, uint16_t color) {
    int16_t halfWidth = w >> 1;
    int16_t halfHeight = h >> 1;
    int16_t tilt = constrain((int16_t)eye->tilt, -8, 8);
    _display->fillRect(x - halfWidth, y - halfHeight + abs(tilt), w, h - abs(tilt), color);
    if (tilt >= 0) {
        _display->fillTriangle(x - halfWidth, y - halfHeight + tilt,
                               x + halfWidth, y - halfHeight,
                               x + halfWidth, y + halfHeight - tilt,
                               color);
    } else {
        _display->fillTriangle(x - halfWidth, y - halfHeight,
                               x + halfWidth, y - halfHeight - tilt,
                               x - halfWidth, y + halfHeight + tilt,
                               color);
    }
}


void RobotEyes::drawTiltedEye(EyeState* eye, int16_t x, int16_t y, uint8_t height) {
    drawTiltedArea(eye, x, y, eye->width, height, _eyeColor);
}


// ============================================================================
// BEHAVIORS - FIXED
// ============================================================================

void RobotEyes::updateBehaviors() {
    uint32_t now = millis();
    
    // ========================================================================
    // AUTOBLINKER - natural blinking (4-8s), only when awake
    // ========================================================================
    if (_autoBlink && !_animPlaying && !_sleepMode) {
        if (now >= _blinkTimer && now >= _blinkCooldown) {
            if (_left.eyelidOpen > 200 && _right.eyelidOpen > 200) {
                playBlink();
                _blinkTimer = now + random(4000, 8000);
                _blinkCooldown = now + 800;
            }
        }
    }
    
    // ========================================================================
    // BREATHING - gentle phase advance (size applied in applyTargetSize)
    // ========================================================================
    if (_breathing && !_sleepMode) {
        if (now - _breathTimer > 20) {
            _breathTimer = now;
            _breathPhase += 0.04f;
            if (_breathPhase > 6.28f) _breathPhase -= 6.28f;
            _dirty = true;
        }
    }
    
    // ========================================================================
    // IDLE GAZE - looks LEFT/RIGHT as if scanning around
    // ========================================================================
    if (_idleMove && !_animPlaying && !_sleepMode && _emotion == EMOTION_NEUTRAL) {
        if (now >= _idleTimer) {
            _gazeTargetX = random(-10, 11);
            _idleTimer = now + random(2500, 5500);
            _dirty = true;
        }
    }
    
    // ========================================================================
    // SLEEP MODE - half-closed heavy eyelids ALWAYS visible + slow breathing
    // ========================================================================
    if (_sleepMode) {
        // Slow breath cycle ~6s: 3s smooth inhale (eyes drift UP), 3s smooth exhale (down).
        uint32_t breathCycle = 6000;
        float p = (float)(now % breathCycle) / (float)breathCycle;   // 0..1
        float s = sinf(p * 6.2832f - 1.5708f);                       // -1 at start, +1 at 3s, -1 at 6s

        // Eyes move UP on inhale, DOWN on exhale, continuously fluid (~6px)
        int8_t driftY = (int8_t)(-s * 6.0f);
        _posX = 0;
        _posY = driftY;

        // Heavy eyelids, optimistic couple: almost-closed on inhale, slightly
        // open on exhale -> the "sleepy" look moves instead of freezing.
        uint8_t base = (uint8_t)(90.0f + s * 12.0f); // 78..102
        uint8_t lid = base;
        _leftTarget.targetEyelid = lid;
        _rightTarget.targetEyelid = lid;

        // Occasional sleepy twitch
        if (now >= _sleepTwitchTimer) {
            _leftTarget.pupilX = random(-3, 4);
            _leftTarget.pupilY = random(-3, 4);
            _rightTarget.pupilX = random(-3, 4);
            _rightTarget.pupilY = random(-3, 4);
            _sleepTwitchTimer = now + random(2500, 5000);
        }
        _dirty = true;
    }
}

// ============================================================================
// ANIMATION SYSTEM - FIXED: BLINK IS FAST, NO LONG CLOSED EYES
// ============================================================================

void RobotEyes::updateAnimation() {
    if (!_animPlaying) return;
    
    uint32_t now = millis();
    uint32_t elapsed = now - _anim.startTime;
    
    if (elapsed >= _anim.duration) {
        _animPlaying = false;
        openEyes();
        return;
    }
    
    _anim.progress = (float)elapsed / _anim.duration;
    
    if (_anim.phase == 3) {
        float p = _anim.progress;
        int lid = 255;
        if (p < 0.25f) {
            lid = (int)(255.0f * (1.0f - p / 0.25f));
        } else if (p < 0.40f) {
            lid = (int)(255.0f * ((p - 0.25f) / 0.15f));
        } else if (p < 0.65f) {
            lid = (int)(255.0f * (1.0f - (p - 0.40f) / 0.25f));
        } else if (p < 0.85f) {
            lid = (int)(255.0f * ((p - 0.65f) / 0.20f));
        } else {
            lid = 255;
        }
        _leftTarget.targetEyelid = (uint8_t)lid;
        _rightTarget.targetEyelid = (uint8_t)lid;
        _left.targetEyelid = (uint8_t)lid;
        _right.targetEyelid = (uint8_t)lid;
        _dirty = true;
        return;
    }
    
    float p = _anim.progress;
    int lid = 255;
    if (p < 0.3f) {
        lid = (int)(255.0f * (1.0f - p / 0.3f));
    } else if (p < 0.5f) {
        lid = 0;
    } else {
        lid = (int)(255.0f * (p - 0.5f) / 0.5f);
    }
    
    if (_anim.phase == 1) {
        _leftTarget.targetEyelid = (uint8_t)lid;
        _rightTarget.targetEyelid = 255;
        _left.targetEyelid = (uint8_t)lid;
        _right.targetEyelid = 255;
    } else if (_anim.phase == 2) {
        _leftTarget.targetEyelid = 255;
        _rightTarget.targetEyelid = (uint8_t)lid;
        _left.targetEyelid = 255;
        _right.targetEyelid = (uint8_t)lid;
    } else {
        _leftTarget.targetEyelid = (uint8_t)lid;
        _rightTarget.targetEyelid = (uint8_t)lid;
        _left.targetEyelid = (uint8_t)lid;
        _right.targetEyelid = (uint8_t)lid;
    }
    _dirty = true;
}

void RobotEyes::updateEmotionTransition() {
    if (!_transitioning) return;
    
    uint32_t now = millis();
    uint32_t elapsed = now - _transitionStart;
    
    if (elapsed >= _transitionDuration) {
        _transitioning = false;
        applyEmotion(_targetEmotion);
        _emotion = _targetEmotion;
        // Ensure eyes are open after transition
        _left.targetEyelid = 255;
        _right.targetEyelid = 255;
        _dirty = true;
        return;
    }
    
    // Smooth transition - gradually change eye parameters
    float t = easeInOut((float)elapsed / _transitionDuration);
    
    // Interpolate eye shape
    uint8_t targetW = _emotionConfigs[_targetEmotion].eyeWidth;
    uint8_t targetH = _emotionConfigs[_targetEmotion].eyeHeight;
    uint8_t targetR = _emotionConfigs[_targetEmotion].borderRadius;
    
    _leftTarget.width = _leftTarget.width + (uint8_t)((targetW - _leftTarget.width) * t * 0.1f);
    _rightTarget.width = _leftTarget.width;
    _leftTarget.height = _leftTarget.height + (uint8_t)((targetH - _leftTarget.height) * t * 0.1f);
    _rightTarget.height = _leftTarget.height;
    _leftTarget.radius = _leftTarget.radius + (uint8_t)((targetR - _leftTarget.radius) * t * 0.1f);
    _rightTarget.radius = _leftTarget.radius;
    
    // Interpolate pupil position
    int8_t targetPX = _emotionConfigs[_targetEmotion].pupilX;
    int8_t targetPY = _emotionConfigs[_targetEmotion].pupilY;
    _leftTarget.pupilX += (int8_t)((targetPX - _leftTarget.pupilX) * t * 0.1f);
    _leftTarget.pupilY += (int8_t)((targetPY - _leftTarget.pupilY) * t * 0.1f);
    _rightTarget.pupilX = _leftTarget.pupilX;
    _rightTarget.pupilY = _leftTarget.pupilY;
    
    // Interpolate pupil size
    uint8_t targetPS = _emotionConfigs[_targetEmotion].pupilSize;
    _leftTarget.pupilSize += (uint8_t)((targetPS - _leftTarget.pupilSize) * t * 0.1f);
    _rightTarget.pupilSize = _leftTarget.pupilSize;
    
    // Keep eyes open during transition
    _left.targetEyelid = 255;
    _right.targetEyelid = 255;
    _dirty = true;
}

// ============================================================================
// LAG / GLITCH SEQUENCE - solo visiva, NON bloccante, ~1.6s totali
// ============================================================================

void RobotEyes::updateLag() {
    if (!_lagPlaying) return;

    uint32_t el = millis() - _lagStart;
    const uint32_t DUR = 1600;

    if (el >= DUR) {
        _lagPlaying = false;
        _lagOffsetX = 0;
        _lagOffsetY = 0;
        _lagPupilX = 0;
        _lagPupilY = 0;
        _lagGlitchAmp = 0;
        _lagSquint = 255;
        _dirty = true;
        return;
    }

    float t = (float)el / (float)DUR;

    // 1) Blocchi glitch: prima leggeri, poi forti, poi recupero progressivo.
    if (el < 250) {
        _lagGlitchAmp = 1;
    } else if (el < 600) {
        _lagGlitchAmp = ((el % 120) < 70) ? 2 : 1;
    } else if (el < 900) {
        _lagGlitchAmp = 3;
    } else if (el < 1150) {
        _lagGlitchAmp = ((el % 100) < 60) ? 2 : 1;
    } else {
        _lagGlitchAmp = 0;
    }

    // 2) Palpebre che si socchiudono ("il volto se ne accorge") con recupero.
    if (el < 250) {
        _lagSquint = 205;
    } else if (el < 600) {
        _lagSquint = 165;
    } else if (el < 900) {
        _lagSquint = 140;
    } else if (el < 1300) {
        _lagSquint = 140 + (uint16_t)((el - 900) * 34u / 100u);
        if (_lagSquint > 255) _lagSquint = 255;
    } else {
        _lagSquint = 255;
    }

    // 3) Pupille che si spostano/sfarfallano.
    if (el >= 200 && el < 500) {
        _lagPupilX = (int8_t)(2.0f * sinf(t * 18.0f));
        _lagPupilY = (int8_t)(1.5f * sinf(t * 22.0f));
    } else if (el >= 500 && el < 900) {
        _lagPupilX = (int8_t)(3.0f * sinf(t * 30.0f));
        _lagPupilY = (int8_t)(2.0f * sinf(t * 42.0f));
    } else {
        _lagPupilX = 0;
        _lagPupilY = 0;
    }

    // 4) Breve deformazione: gli occhi si spostano di qualche pixel e tornano.
    _lagOffsetX = 0;
    _lagOffsetY = 0;
    if (el >= 550 && el < 1050) {
        float u = (float)(el - 550) / 500.0f;
        float bump = sinf(u * 3.14159265f);
        int8_t amp = (el < 820) ? 3 : 2;
        _lagOffsetY = (int8_t)(amp * bump);
        if (el < 820) _lagOffsetX = (int8_t)(2.0f * bump);
    }
    // Piccolo "salto" verticale instabile durante la fase forte (sync persa).
    if (el >= 420 && el < 900) {
        _lagOffsetY += (int8_t)(((el / 90) & 1) ? 1 : 0);
    }

    _dirty = true;
}

// ============================================================================
// EMOTION CONFIGURATIONS - WITH STORED CONFIG FOR TRANSITIONS
// ============================================================================

void RobotEyes::applyEmotion(EyeEmotion emotion) {
    // Store current emotion
    _emotion = emotion;
    
    // Apply emotion configuration directly
    switch (emotion) {
        case EMOTION_NEUTRAL: setupNeutral(); break;
        case EMOTION_HAPPY: setupHappy(); break;
        case EMOTION_ANGRY: setupAngry(); break;
        case EMOTION_SAD: setupSad(); break;
        case EMOTION_SLEEPY: setupSleepy(); break;
        case EMOTION_SURPRISED: setupSurprised(); break;
        case EMOTION_CURIOUS: setupCurious(); break;
        case EMOTION_SCARED: setupScared(); break;
        case EMOTION_LOVE: setupLove(); break;
        case EMOTION_LAUGHING: setupLaughing(); break;
        case EMOTION_THINKING: setupThinking(); break;
        case EMOTION_WINK_LEFT: setupWinkLeft(); break;
        case EMOTION_WINK_RIGHT: setupWinkRight(); break;
        case EMOTION_SUSPICIOUS: setupSuspicious(); break;
        case EMOTION_FOCUSED: setupFocused(); break;
        case EMOTION_ALERT: setupAlert(); break;
        case EMOTION_CONFUSED: setupConfused(); break;
        case EMOTION_SCANNING: setupScanning(); break;
        case EMOTION_LAG: setupLag(); break;
        default: setupNeutral(); break;
    }

    _leftTarget.tilt = _emotionConfigs[emotion].leftTilt;
    _rightTarget.tilt = _emotionConfigs[emotion].rightTilt;
    _dirty = true;
}

void RobotEyes::setupNeutral() {
    _leftTarget.width = 30; _rightTarget.width = 30;
    _leftTarget.height = 30; _rightTarget.height = 30;
    _leftTarget.radius = 6; _rightTarget.radius = 6;
    _leftTarget.pupilSize = 5; _rightTarget.pupilSize = 5;
    _leftTarget.pupilX = 0; _leftTarget.pupilY = 0;
    _rightTarget.pupilX = 0; _rightTarget.pupilY = 0;
    _leftTarget.targetEyelid = 255; _rightTarget.targetEyelid = 255;
    _posX = 0; _posY = 0;
    _spaceBetween = 8;
    
    // Store config for transitions
    _emotionConfigs[EMOTION_NEUTRAL] = {30, 30, 6, 5, 0, 0, 255, 0, 0};
}

void RobotEyes::setupHappy() {
    _leftTarget.width = 28; _rightTarget.width = 28;
    _leftTarget.height = 24; _rightTarget.height = 24;
    _leftTarget.radius = 10; _rightTarget.radius = 10;
    _leftTarget.pupilSize = 6; _rightTarget.pupilSize = 6;
    _leftTarget.pupilX = 0; _leftTarget.pupilY = 3;
    _rightTarget.pupilX = 0; _rightTarget.pupilY = 3;
    _leftTarget.targetEyelid = 255; _rightTarget.targetEyelid = 255;
    _spaceBetween = 6;
    _emotionConfigs[EMOTION_HAPPY] = {28, 24, 10, 6, 0, 3, 255, 0, 0};
}

void RobotEyes::setupAngry() {
    _leftTarget.width = 26; _rightTarget.width = 26;
    _leftTarget.height = 28; _rightTarget.height = 28;
    _leftTarget.radius = 3; _rightTarget.radius = 3;
    _leftTarget.pupilSize = 4; _rightTarget.pupilSize = 4;
    _leftTarget.pupilX = -2; _leftTarget.pupilY = -3;
    _rightTarget.pupilX = 2; _rightTarget.pupilY = -3;
    _leftTarget.targetEyelid = 200; _rightTarget.targetEyelid = 200;
    _spaceBetween = 12;
    _emotionConfigs[EMOTION_ANGRY] = {26, 28, 3, 4, -2, -3, 200, 0, 0};
    _emotionConfigs[EMOTION_ANGRY].leftTilt = -7;
    _emotionConfigs[EMOTION_ANGRY].rightTilt = 7;
}

void RobotEyes::setupSad() {
    _leftTarget.width = 28; _rightTarget.width = 28;
    _leftTarget.height = 26; _rightTarget.height = 26;
    _leftTarget.radius = 8; _rightTarget.radius = 8;
    _leftTarget.pupilSize = 5; _rightTarget.pupilSize = 5;
    _leftTarget.pupilX = 0; _leftTarget.pupilY = 4;
    _rightTarget.pupilX = 0; _rightTarget.pupilY = 4;
    _leftTarget.targetEyelid = 180; _rightTarget.targetEyelid = 180;
    _spaceBetween = 10;
    _emotionConfigs[EMOTION_SAD] = {28, 26, 8, 5, 0, 4, 180, 0, 0};
    _emotionConfigs[EMOTION_SAD].leftTilt = 4;
    _emotionConfigs[EMOTION_SAD].rightTilt = -4;
}

void RobotEyes::setupSleepy() {
    _leftTarget.width = 26; _rightTarget.width = 26;
    _leftTarget.height = 16; _rightTarget.height = 16;
    _leftTarget.radius = 6; _rightTarget.radius = 6;
    _leftTarget.pupilSize = 3; _rightTarget.pupilSize = 3;
    _leftTarget.pupilX = 0; _leftTarget.pupilY = -2;
    _rightTarget.pupilX = 0; _rightTarget.pupilY = -2;
    _leftTarget.targetEyelid = 80; _rightTarget.targetEyelid = 80;
    _spaceBetween = 8;
    _emotionConfigs[EMOTION_SLEEPY] = {26, 16, 6, 3, 0, -2, 80, 0, 0};
}

void RobotEyes::setupSurprised() {
    _leftTarget.width = 36; _rightTarget.width = 36;
    _leftTarget.height = 36; _rightTarget.height = 36;
    _leftTarget.radius = 16; _rightTarget.radius = 16;
    _leftTarget.pupilSize = 3; _rightTarget.pupilSize = 3;
    _leftTarget.pupilX = 0; _leftTarget.pupilY = 0;
    _rightTarget.pupilX = 0; _rightTarget.pupilY = 0;
    _leftTarget.targetEyelid = 255; _rightTarget.targetEyelid = 255;
    _spaceBetween = 6;
    _emotionConfigs[EMOTION_SURPRISED] = {36, 36, 16, 3, 0, 0, 255, 0, 0};
}

void RobotEyes::setupCurious() {
    _leftTarget.width = 30; _rightTarget.width = 30;
    _leftTarget.height = 28; _rightTarget.height = 28;
    _leftTarget.radius = 6; _rightTarget.radius = 6;
    _leftTarget.pupilSize = 6; _rightTarget.pupilSize = 6;
    _leftTarget.pupilX = 4; _leftTarget.pupilY = 2;
    _rightTarget.pupilX = 4; _rightTarget.pupilY = 2;
    _leftTarget.targetEyelid = 255; _rightTarget.targetEyelid = 255;
    _spaceBetween = 6;
    _emotionConfigs[EMOTION_CURIOUS] = {30, 28, 6, 6, 4, 2, 255, 0, 0};
}

void RobotEyes::setupScared() {
    _leftTarget.width = 34; _rightTarget.width = 34;
    _leftTarget.height = 28; _rightTarget.height = 28;
    _leftTarget.radius = 6; _rightTarget.radius = 6;
    _leftTarget.pupilSize = 3; _rightTarget.pupilSize = 3;
    _leftTarget.pupilX = -3; _leftTarget.pupilY = -2;
    _rightTarget.pupilX = 3; _rightTarget.pupilY = -2;
    _leftTarget.targetEyelid = 220; _rightTarget.targetEyelid = 220;
    _spaceBetween = 4;
    _emotionConfigs[EMOTION_SCARED] = {34, 28, 6, 3, -3, -2, 220, 0, 0};
}

void RobotEyes::setupLove() {
    _leftTarget.width = 26; _rightTarget.width = 26;
    _leftTarget.height = 28; _rightTarget.height = 28;
    _leftTarget.radius = 14; _rightTarget.radius = 14;
    _leftTarget.pupilSize = 7; _rightTarget.pupilSize = 7;
    _leftTarget.pupilX = 0; _leftTarget.pupilY = 2;
    _rightTarget.pupilX = 0; _rightTarget.pupilY = 2;
    _leftTarget.targetEyelid = 255; _rightTarget.targetEyelid = 255;
    _spaceBetween = 4;
    _emotionConfigs[EMOTION_LOVE] = {26, 28, 14, 7, 0, 2, 255, 0, 0};
}

void RobotEyes::setupLaughing() {
    _leftTarget.width = 24; _rightTarget.width = 24;
    _leftTarget.height = 18; _rightTarget.height = 18;
    _leftTarget.radius = 12; _rightTarget.radius = 12;
    _leftTarget.pupilSize = 5; _rightTarget.pupilSize = 5;
    _leftTarget.pupilX = 0; _leftTarget.pupilY = 3;
    _rightTarget.pupilX = 0; _rightTarget.pupilY = 3;
    _leftTarget.targetEyelid = 255; _rightTarget.targetEyelid = 255;
    _spaceBetween = 4;
    _emotionConfigs[EMOTION_LAUGHING] = {24, 18, 12, 5, 0, 3, 255, 0, 0};
}

void RobotEyes::setupThinking() {
    _leftTarget.width = 26; _rightTarget.width = 26;
    _leftTarget.height = 26; _rightTarget.height = 26;
    _leftTarget.radius = 6; _rightTarget.radius = 6;
    _leftTarget.pupilSize = 4; _rightTarget.pupilSize = 4;
    _leftTarget.pupilX = 5; _leftTarget.pupilY = -2;
    _rightTarget.pupilX = -5; _rightTarget.pupilY = -2;
    _leftTarget.targetEyelid = 200; _rightTarget.targetEyelid = 200;
    _posX = -4; _posY = -4;
    _spaceBetween = 10;
    _emotionConfigs[EMOTION_THINKING] = {26, 26, 6, 4, 5, -2, 200, -4, -4};
}

void RobotEyes::setupWinkLeft() {
    _leftTarget.width = 30; _rightTarget.width = 30;
    _leftTarget.height = 30; _rightTarget.height = 30;
    _leftTarget.radius = 6; _rightTarget.radius = 6;
    _leftTarget.pupilSize = 5; _rightTarget.pupilSize = 5;
    _leftTarget.pupilX = 0; _leftTarget.pupilY = 0;
    _rightTarget.pupilX = 0; _rightTarget.pupilY = 0;
    _leftTarget.targetEyelid = 0;
    _rightTarget.targetEyelid = 255;
    _posX = 0; _posY = 0;
    _spaceBetween = 8;
    _emotionConfigs[EMOTION_WINK_LEFT] = {30, 30, 6, 5, 0, 0, 0, 0, 0};
}

void RobotEyes::setupWinkRight() {
    _leftTarget.width = 30; _rightTarget.width = 30;
    _leftTarget.height = 30; _rightTarget.height = 30;
    _leftTarget.radius = 6; _rightTarget.radius = 6;
    _leftTarget.pupilSize = 5; _rightTarget.pupilSize = 5;
    _leftTarget.pupilX = 0; _leftTarget.pupilY = 0;
    _rightTarget.pupilX = 0; _rightTarget.pupilY = 0;
    _leftTarget.targetEyelid = 255;
    _rightTarget.targetEyelid = 0;
    _posX = 0; _posY = 0;
    _spaceBetween = 8;
    _emotionConfigs[EMOTION_WINK_RIGHT] = {30, 30, 6, 5, 0, 0, 255, 0, 0};
}

void RobotEyes::setupSuspicious() {
    _leftTarget = _left;
    _rightTarget = _right;
    _leftTarget.width = _rightTarget.width = 29;
    _leftTarget.height = _rightTarget.height = 24;
    _leftTarget.radius = _rightTarget.radius = 5;
    _leftTarget.pupilSize = _rightTarget.pupilSize = 4;
    _leftTarget.pupilX = -7;
    _rightTarget.pupilX = 7;
    _leftTarget.pupilY = _rightTarget.pupilY = -1;
    _leftTarget.targetEyelid = _rightTarget.targetEyelid = 205;
    _emotionConfigs[EMOTION_SUSPICIOUS] = {29, 24, 5, 4, 0, -1, 205, 0, 0, 2, -2};
}

void RobotEyes::setupFocused() {
    _leftTarget.width = _rightTarget.width = 31;
    _leftTarget.height = _rightTarget.height = 29;
    _leftTarget.radius = _rightTarget.radius = 6;
    _leftTarget.pupilSize = _rightTarget.pupilSize = 5;
    _leftTarget.pupilX = _rightTarget.pupilX = 0;
    _leftTarget.pupilY = _rightTarget.pupilY = 0;
    _leftTarget.targetEyelid = _rightTarget.targetEyelid = 245;
    _emotionConfigs[EMOTION_FOCUSED] = {31, 29, 6, 5, 0, 0, 245, 0, 0, 0, 0};
}

void RobotEyes::setupAlert() {
    _leftTarget.width = _rightTarget.width = 34;
    _leftTarget.height = _rightTarget.height = 34;
    _leftTarget.radius = _rightTarget.radius = 10;
    _leftTarget.pupilSize = _rightTarget.pupilSize = 4;
    _leftTarget.pupilX = _rightTarget.pupilX = 0;
    _leftTarget.pupilY = _rightTarget.pupilY = -2;
    _leftTarget.targetEyelid = _rightTarget.targetEyelid = 255;
    _emotionConfigs[EMOTION_ALERT] = {34, 34, 10, 4, 0, -2, 255, 0, 0, 0, 0};
}

void RobotEyes::setupConfused() {
    _leftTarget.width = 30;
    _rightTarget.width = 27;
    _leftTarget.height = 28;
    _rightTarget.height = 24;
    _leftTarget.radius = _rightTarget.radius = 6;
    _leftTarget.pupilSize = _rightTarget.pupilSize = 5;
    _leftTarget.pupilX = -3;
    _rightTarget.pupilX = 4;
    _leftTarget.pupilY = 1;
    _rightTarget.pupilY = -2;
    _leftTarget.targetEyelid = 230;
    _rightTarget.targetEyelid = 190;
    _emotionConfigs[EMOTION_CONFUSED] = {30, 28, 6, 5, 0, 0, 230, 0, 0, 5, -3};
}

void RobotEyes::setupScanning() {
    _leftTarget.width = _rightTarget.width = 31;
    _leftTarget.height = _rightTarget.height = 29;
    _leftTarget.radius = _rightTarget.radius = 5;
    _leftTarget.pupilSize = _rightTarget.pupilSize = 4;
    _leftTarget.pupilX = _rightTarget.pupilX = 0;
    _leftTarget.pupilY = _rightTarget.pupilY = 0;
    _leftTarget.targetEyelid = _rightTarget.targetEyelid = 245;
    _emotionConfigs[EMOTION_SCANNING] = {31, 29, 5, 4, 0, 0, 245, 0, 0, 0, 0};
}

void RobotEyes::setupLag() {
    _leftTarget.width = _rightTarget.width = 31;
    _leftTarget.height = _rightTarget.height = 28;
    _leftTarget.radius = _rightTarget.radius = 6;
    _leftTarget.pupilSize = _rightTarget.pupilSize = 4;
    _leftTarget.pupilX = _rightTarget.pupilX = 0;
    _leftTarget.pupilY = _rightTarget.pupilY = 0;
    _leftTarget.targetEyelid = _rightTarget.targetEyelid = 195;
    _spaceBetween = 8;
    _emotionConfigs[EMOTION_LAG] = {31, 28, 6, 4, 0, 0, 195, 0, 0};
}

// ============================================================================
// POSITION CALCULATION
// ============================================================================

void RobotEyes::calculatePositions() {
    uint16_t cx = _screenW >> 1;
    uint16_t cy = _screenH >> 1;
    
    int16_t halfSpace = (_left.width + _right.width + (_spaceBetween << 1)) >> 2;
    
    _left.centerX = cx - halfSpace + _posX;
    _left.centerY = cy + _posY;
    _right.centerX = cx + halfSpace + _posX;
    _right.centerY = cy + _posY;
}

void RobotEyes::applyTargetSize() {
    float b = _breathing ? (1.0f + sinf(_breathPhase) * 0.006f) : 1.0f;
    
    _left.width  = (uint8_t)(_leftTarget.width  * b);
    _right.width = (uint8_t)(_rightTarget.width * b);
    _left.height = (uint8_t)(_leftTarget.height * b);
    _right.height= (uint8_t)(_rightTarget.height * b);
    _left.radius = _leftTarget.radius;
    _right.radius= _rightTarget.radius;
    _left.tilt = _leftTarget.tilt;
    _right.tilt = _rightTarget.tilt;
    _left.pupilSize = _leftTarget.pupilSize;
    _right.pupilSize= _rightTarget.pupilSize;
    _left.pupilX = _leftTarget.pupilX;
    _left.pupilY = _leftTarget.pupilY;
    _right.pupilX = _rightTarget.pupilX;
    _right.pupilY = _rightTarget.pupilY;
    _left.targetEyelid = _leftTarget.targetEyelid;
    _right.targetEyelid = _rightTarget.targetEyelid;
}

// ============================================================================
// EASING FUNCTIONS
// ============================================================================

float RobotEyes::easeInOut(float t) {
    return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;
}

float RobotEyes::easeOutBack(float t) {
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.0f;
    return 1.0f + c3 * powf(t - 1.0f, 3.0f) + c1 * powf(t - 1.0f, 2.0f);
}

// ============================================================================
// PUBLIC API
// ============================================================================

void RobotEyes::setEyeSize(uint8_t w, uint8_t h) {
    _leftTarget.width = constrain(w, 16, 44);
    _rightTarget.width = _leftTarget.width;
    _leftTarget.height = constrain(h, 12, 44);
    _rightTarget.height = _leftTarget.height;
    _dirty = true;
}

void RobotEyes::setBorderRadius(uint8_t r) {
    _leftTarget.radius = constrain(r, 2, 20);
    _rightTarget.radius = _leftTarget.radius;
    _dirty = true;
}

void RobotEyes::setSpaceBetween(int16_t space) {
    _spaceBetween = constrain(space, 2, 30);
    _dirty = true;
}

void RobotEyes::setPosition(EyePosition pos) {
    _position = pos;
    switch (pos) {
        case POS_N: _posX = 0; _posY = -8; break;
        case POS_NE: _posX = 8; _posY = -8; break;
        case POS_E: _posX = 8; _posY = 0; break;
        case POS_SE: _posX = 8; _posY = 8; break;
        case POS_S: _posX = 0; _posY = 8; break;
        case POS_SW: _posX = -8; _posY = 8; break;
        case POS_W: _posX = -8; _posY = 0; break;
        case POS_NW: _posX = -8; _posY = -8; break;
        default: _posX = 0; _posY = 0; break;
    }
    _dirty = true;
}

void RobotEyes::setPupilOffset(int8_t x, int8_t y) {
    _leftTarget.pupilX = constrain(x, -12, 12);
    _leftTarget.pupilY = constrain(y, -12, 12);
    _rightTarget.pupilX = _leftTarget.pupilX;
    _rightTarget.pupilY = _leftTarget.pupilY;
    _dirty = true;
}

void RobotEyes::setPupilPosition(int8_t x, int8_t y) {
    setPupilOffset(x, y);
}

void RobotEyes::setPupils(int8_t lx, int8_t ly, int8_t rx, int8_t ry) {
    _leftTarget.pupilX = constrain(lx, -12, 12);
    _leftTarget.pupilY = constrain(ly, -12, 12);
    _rightTarget.pupilX = constrain(rx, -12, 12);
    _rightTarget.pupilY = constrain(ry, -12, 12);
    _dirty = true;
}

void RobotEyes::setPupilSize(uint8_t size) {
    _leftTarget.pupilSize = constrain(size, 2, 10);
    _rightTarget.pupilSize = _leftTarget.pupilSize;
    _dirty = true;
}

void RobotEyes::setEyelidOpen(uint8_t amount) {
    _leftTarget.targetEyelid = constrain(amount, 0, 255);
    _rightTarget.targetEyelid = _leftTarget.targetEyelid;
    _dirty = true;
}

void RobotEyes::setEyelids(uint8_t left, uint8_t right) {
    _leftTarget.targetEyelid = constrain(left, 0, 255);
    _rightTarget.targetEyelid = constrain(right, 0, 255);
    _dirty = true;
}

void RobotEyes::openEyes() {
    _leftTarget.targetEyelid = 255;
    _rightTarget.targetEyelid = 255;
    _leftTarget.isOpen = true;
    _rightTarget.isOpen = true;
    _dirty = true;
}

void RobotEyes::closeEyes() {
    _leftTarget.targetEyelid = 0;
    _rightTarget.targetEyelid = 0;
    _leftTarget.isOpen = false;
    _rightTarget.isOpen = false;
    _dirty = true;
}

void RobotEyes::setEmotion(EyeEmotion emotion) {
    _emotion = emotion;
    _targetEmotion = emotion;
    _transitioning = false;
    applyEmotion(emotion);
}

void RobotEyes::setEmotionWithTransition(EyeEmotion emotion, uint16_t ms) {
    _targetEmotion = emotion;
    _transitioning = true;
    _transitionStart = millis();
    _transitionDuration = ms;
    _dirty = true;
}

void RobotEyes::dispatchEvent(EyeEvent event) {
    switch (event) {
        case EYE_EVENT_MENU_OPEN:
        case EYE_EVENT_NETWORK_OPEN:
        case EYE_EVENT_CAMERA_OPEN:
        case EYE_EVENT_SETTING_CHANGED:
            setEmotionWithTransition(EMOTION_CURIOUS, 350);
            break;
        case EYE_EVENT_INFO_OPEN:
        case EYE_EVENT_QR_GENERATED:
            setEmotionWithTransition(EMOTION_FOCUSED, 350);
            break;
        case EYE_EVENT_WIFI_SCAN_START:
            setEmotionWithTransition(EMOTION_SCANNING, 350);
            break;
        case EYE_EVENT_QR_SCANNED:
            setEmotionWithTransition(EMOTION_SURPRISED, 350);
            break;
        case EYE_EVENT_CAMERA_CONNECTED:
            setEmotionWithTransition(EMOTION_HAPPY, 350);
            break;
        case EYE_EVENT_CAMERA_DISCONNECTED:
        case EYE_EVENT_ERROR:
            setEmotionWithTransition(EMOTION_CONFUSED, 350);
            break;
        case EYE_EVENT_SECURITY_OPEN:
            setEmotionWithTransition(EMOTION_ALERT, 350);
            break;
        default:
            setEmotionWithTransition(EMOTION_CURIOUS, 350);
            break;
    }
}

EyeEmotion RobotEyes::getEmotion() const {
    return _emotion;
}

void RobotEyes::setAutoblinker(bool enable) {
    _autoBlink = enable;
    if (enable) {
        _blinkTimer = millis() + random(4000, 6000);
        _blinkCooldown = 0;
    }
}

void RobotEyes::setBreathing(bool enable) {
    _breathing = enable;
    if (enable) {
        _breathTimer = millis();
        _breathPhase = 0;
    }
}

void RobotEyes::setSleepMode(bool enable) {
    if (enable) {
        _sleepMode = true;
        _autoBlink = false;
        _breathing = false;
        _idleMove = false;
        _animPlaying = false;
        _transitioning = false;
        _gazeActive = false;
        _gazeTargetX = 0;
        _gazeX = 0;
        _sleepPhase = 0;
        setupSleepy();
        _sleepTwitchTimer = millis() + random(3000, 6000);
        _left.targetEyelid = 80;
        _right.targetEyelid = 80;
        _dirty = true;
    } else {
        _sleepMode = false;
        _transitioning = false;
        _autoBlink = true;
        _idleMove = true;
        _breathing = true;
        _breathTimer = millis();
        _breathPhase = 0;
        _posX = 0;
        _posY = 0;
        _gazeActive = true;
        _gazeTargetX = 0;
        _gazeX = 0;
        _idleTimer = millis() + random(2000, 4000);
        _blinkTimer = millis() + random(2500, 4500);
        _blinkCooldown = 0;
        applyEmotion(_emotion);
        _dirty = true;
    }
}

void RobotEyes::setIdleMovement(bool enable) {
    _idleMove = enable;
    if (enable) {
        _idleTimer = millis() + random(3000, 5000);
    }
}

// ============================================================================
// ANIMATIONS
// ============================================================================

void RobotEyes::playBlink() {
    if (_animPlaying || _sleepMode) return;
    _animPlaying = true;
    _anim.startTime = millis();
    _anim.duration = 200;
    _anim.phase = 0;
    _anim.progress = 0;
    _anim.loop = false;
    _dirty = true;
}

void RobotEyes::playDoubleBlink() {
    if (_animPlaying || _sleepMode) return;
    _animPlaying = true;
    _anim.startTime = millis();
    _anim.duration = 450;
    _anim.phase = 3;
    _anim.progress = 0;
    _anim.loop = false;
    _dirty = true;
}

void RobotEyes::playWinkLeft() {
    if (_animPlaying || _sleepMode) return;
    _animPlaying = true;
    _anim.startTime = millis();
    _anim.duration = 250;
    _anim.phase = 1;
    _anim.progress = 0;
    _anim.loop = false;
    _leftTarget.targetEyelid = 0;
    _rightTarget.targetEyelid = 255;
    _dirty = true;
}

void RobotEyes::playWinkRight() {
    if (_animPlaying || _sleepMode) return;
    _animPlaying = true;
    _anim.startTime = millis();
    _anim.duration = 250;
    _anim.phase = 2;
    _anim.progress = 0;
    _anim.loop = false;
    _leftTarget.targetEyelid = 255;
    _rightTarget.targetEyelid = 0;
    _dirty = true;
}

void RobotEyes::playStartup() {
    closeEyes();
    delay(100);
    openEyes();
    _dirty = true;
}

void RobotEyes::playShutdown() {
    closeEyes();
    _dirty = true;
}

// ============================================================================
// LAG / GLITCH
// ============================================================================

void RobotEyes::playLag() {
    if (_lagPlaying || _sleepMode) return;
    _lagPlaying = true;
    _lagStart = millis();
    _lagGlitchAmp = 0;
    _lagSquint = 255;
    _lagOffsetX = 0;
    _lagOffsetY = 0;
    _lagPupilX = 0;
    _lagPupilY = 0;
    _dirty = true;
}

bool RobotEyes::isLagPlaying() const {
    return _lagPlaying;
}

bool RobotEyes::isAnimating() const {
    return _animPlaying;
}

bool RobotEyes::isSleeping() const {
    return _sleepMode;
}

uint16_t RobotEyes::getFPS() const {
    return _currentFps;
}

void RobotEyes::setEyeColor(uint16_t color) {
    _eyeColor = color;
    _dirty = true;
}

void RobotEyes::setPupilColor(uint16_t color) {
    _pupilColor = color;
    _dirty = true;
}

void RobotEyes::setInverted(bool inverted) {
    if (inverted) {
        _bgColor = SSD1306_WHITE;
        _eyeColor = SSD1306_BLACK;
        _pupilColor = SSD1306_WHITE;
    } else {
        _bgColor = SSD1306_BLACK;
        _eyeColor = SSD1306_WHITE;
        _pupilColor = SSD1306_BLACK;
    }
    _dirty = true;
}

void RobotEyes::setFramerate(uint8_t fps) {
    _maxFps = constrain(fps, 20, 80);
    _frameInterval = 1000 / _maxFps;
}

uint8_t RobotEyes::getFramerate() const {
    return _maxFps;
}

void RobotEyes::openLeftEye() {
    _leftTarget.targetEyelid = 255;
    _leftTarget.isOpen = true;
    _dirty = true;
}

void RobotEyes::openRightEye() {
    _rightTarget.targetEyelid = 255;
    _rightTarget.isOpen = true;
    _dirty = true;
}

void RobotEyes::closeLeftEye() {
    _leftTarget.targetEyelid = 0;
    _leftTarget.isOpen = false;
    _dirty = true;
}

void RobotEyes::closeRightEye() {
    _rightTarget.targetEyelid = 0;
    _rightTarget.isOpen = false;
    _dirty = true;
}