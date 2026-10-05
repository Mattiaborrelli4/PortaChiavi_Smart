#include <pk_robot.h>
#include <value_store.h>

namespace pk_robot {

static RobotEyes* _eyes = nullptr;

// L'emozione scelta dall'utente viene persistita su LittleFS ("emotion"):
// al riavvio si riapplica per non far perdere la maschera selezionata.
// Gli stati transienti (lag/scanning) vengono ignorati: sono effetti
// guidati dagli eventi, non scelte dell'utente.
static void persistEmotion(const String& name) {
    if (name == "lag" || name == "scanning") return;
    value_store::settingsSave("emotion", name);
}

void begin(RobotEyes* eyes) {
    _eyes = eyes;
}

static bool emotionByName(const String& v, EyeEmotion& out) {
    if (v == "neutral")    { out = EMOTION_NEUTRAL;    return true; }
    if (v == "happy")      { out = EMOTION_HAPPY;      return true; }
    if (v == "angry")      { out = EMOTION_ANGRY;      return true; }
    if (v == "sad")        { out = EMOTION_SAD;        return true; }
    if (v == "sleepy")     { out = EMOTION_SLEEPY;     return true; }
    if (v == "surprised")  { out = EMOTION_SURPRISED;  return true; }
    if (v == "curious")    { out = EMOTION_CURIOUS;    return true; }
    if (v == "scared")     { out = EMOTION_SCARED;     return true; }
    if (v == "love")       { out = EMOTION_LOVE;       return true; }
    if (v == "laughing")   { out = EMOTION_LAUGHING;   return true; }
    if (v == "thinking")   { out = EMOTION_THINKING;   return true; }
    if (v == "wink_left")  { out = EMOTION_WINK_LEFT;  return true; }
    if (v == "wink_right") { out = EMOTION_WINK_RIGHT; return true; }
    if (v == "scanning")   { out = EMOTION_SCANNING;   return true; }
    if (v == "lag")        { out = EMOTION_LAG;        return true; }
    return false;
}

static String emotionName(EyeEmotion e) {
    switch (e) {
        case EMOTION_HAPPY:     return "happy";
        case EMOTION_ANGRY:     return "angry";
        case EMOTION_SAD:       return "sad";
        case EMOTION_SLEEPY:    return "sleepy";
        case EMOTION_SURPRISED: return "surprised";
        case EMOTION_CURIOUS:   return "curious";
        case EMOTION_SCARED:    return "scared";
        case EMOTION_LOVE:      return "love";
        case EMOTION_LAUGHING:  return "laughing";
        case EMOTION_THINKING:  return "thinking";
        case EMOTION_WINK_LEFT: return "wink_left";
        case EMOTION_WINK_RIGHT:return "wink_right";
        case EMOTION_SCANNING:  return "scanning";
        case EMOTION_LAG:       return "lag";
        default:                return "neutral";
    }
}

String emotions() {
    return "neutral|happy|angry|sad|sleepy|surprised|curious|scared|love|laughing|thinking|wink_left|wink_right|lag|scanning";
}

String currentEmotionName() {
    if (!_eyes) return "none";
    return emotionName(_eyes->getEmotion());
}

void restoreEmotion() {
    if (!_eyes) return;
    String saved = value_store::settingsLoad("emotion");
    if (saved.length() == 0) return;
    saved.toLowerCase();
    EyeEmotion e;
    if (emotionByName(saved, e)) {
        _eyes->setEmotionWithTransition(e, 400);
        Serial.print(F("[Robot] Emozione ripristinata: "));
        Serial.println(saved);
    }
}

uint16_t currentFps() {
	return _eyes ? _eyes->getFPS() : 0;
}

void reactEvent(EyeEvent event) {
    if (_eyes) _eyes->dispatchEvent(event);
}

bool isSleeping() {
    return _eyes ? _eyes->isSleeping() : false;
}

bool handleCommand(const String& cmd) {
    if (!_eyes) return false;

    if (cmd.startsWith("emotion:")) {
        String v = cmd.substring(8);
        v.toLowerCase();
        EyeEmotion e;
        if (emotionByName(v, e)) {
            _eyes->setEmotionWithTransition(e, 400);
            persistEmotion(v);
            return true;
        }
        return true; // riconosciuto come comando robot anche se nome errato
    }

    if (cmd.startsWith("anim:")) {
        String v = cmd.substring(5);
        v.toLowerCase();
        if (v == "blink")      { _eyes->playBlink();      return true; }
        if (v == "double")     { _eyes->playDoubleBlink();return true; }
        if (v == "wink_left")  { _eyes->playWinkLeft();   return true; }
        if (v == "wink_right") { _eyes->playWinkRight();  return true; }
        return true;
    }

    if (cmd.startsWith("sleep:")) {
        String v = cmd.substring(6);
        v.toLowerCase();
        if (v == "on")  { _eyes->setSleepMode(true);  return true; }
        if (v == "off") { _eyes->setSleepMode(false); return true; }
        return true;
    }

    if (cmd.startsWith("breath:")) {
        String v = cmd.substring(7);
        v.toLowerCase();
        if (v == "on")  { _eyes->setBreathing(true);  return true; }
        if (v == "off") { _eyes->setBreathing(false); return true; }
        return true;
    }

    if (cmd == "fps") {
        if (_eyes) Serial.print(F("FPS: "));
        if (_eyes) Serial.println(_eyes->getFPS());
        return true;
    }

    return false;
}

String apiApply(const String& action, const String& value) {
    if (!_eyes) return "{\"ok\":false,\"error\":\"robot non inizializzato\"}";

    if (action == "emotion") {
        EyeEmotion e;
        if (!emotionByName(value, e)) {
            return "{\"ok\":false,\"error\":\"emotion sconosciuta\"}";
        }
        _eyes->setEmotionWithTransition(e, 400);
        persistEmotion(value);
        return "{\"ok\":true,\"emotion\":\"" + value + "\"}";
    }

    if (action == "anim") {
        if (value == "blink")       { _eyes->playBlink();       return "{\"ok\":true}"; }
        if (value == "double")      { _eyes->playDoubleBlink(); return "{\"ok\":true}"; }
        if (value == "wink_left")   { _eyes->playWinkLeft();    return "{\"ok\":true}"; }
        if (value == "wink_right")  { _eyes->playWinkRight();   return "{\"ok\":true}"; }
        return "{\"ok\":false,\"error\":\"anim sconosciuta\"}";
    }

    if (action == "sleep") {
        String v = value;
        v.toLowerCase();
        if (v == "on")  { _eyes->setSleepMode(true);  return "{\"ok\":true,\"sleep\":true}"; }
        if (v == "off") { _eyes->setSleepMode(false); return "{\"ok\":true,\"sleep\":false}"; }
        return "{\"ok\":false,\"error\":\"valore non valido\"}";
    }

    if (action == "breath") {
        String v = value;
        v.toLowerCase();
        if (v == "on")  { _eyes->setBreathing(true);  return "{\"ok\":true,\"breath\":true}"; }
        if (v == "off") { _eyes->setBreathing(false); return "{\"ok\":true,\"breath\":false}"; }
        return "{\"ok\":false,\"error\":\"valore non valido\"}";
    }

    if (action == "lag") {
        _eyes->playLag();
        return "{\"ok\":true,\"lag\":true}";
    }

    if (action == "state") {
        return "{\"ok\":true,\"emotion\":\"" + currentEmotionName() +
               "\",\"sleep\":" + String(_eyes->isSleeping() ? "true" : "false") +
               ",\"fps\":" + String(_eyes->getFPS()) + "}";
    }

    return "{\"ok\":false,\"error\":\"azione sconosciuta\"}";
}

} // namespace pk_robot