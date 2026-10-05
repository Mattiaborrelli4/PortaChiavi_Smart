#include <pk_camera.h>
#include <ESP8266WiFi.h>
#include <QRCode.h>
#include <device.h>

namespace pk_camera {

static Adafruit_SSD1306* displayRef = nullptr;
static uint16_t screenW = 128;
static uint16_t screenH = 64;
static CameraState currentState = CAMERA_IDLE;
static String currentSession;
static String currentUrl;
static uint32_t expiresAt = 0;
static bool dirty = false;
static const uint32_t SESSION_TTL_MS = 5UL * 60UL * 1000UL;
static bool _hasLoc = false;
static String _locLat;
static String _locLon;
static String _locAcc;

static String stateText(CameraState value) {
    switch (value) {
        case CAMERA_WAITING: return "WAITING";
        case CAMERA_SCANNED: return "SCANNED";
        case CAMERA_REQUESTED: return "REQUESTED";
        case CAMERA_CONNECTED: return "CONNECTED";
        case CAMERA_DISCONNECTED: return "DISCONNECTED";
        case CAMERA_EXPIRED: return "EXPIRED";
        case CAMERA_STOPPED: return "STOPPED";
        default: return "IDLE";
    }
}

static String randomToken(uint8_t length) {
    static const char alphabet[] = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789";
    String token;
    token.reserve(length);
    for (uint8_t i = 0; i < length; i++) {
        token += alphabet[random(0, (int)sizeof(alphabet) - 1)];
    }
    return token;
}

void begin(Adafruit_SSD1306* display, uint16_t width, uint16_t height) {
    displayRef = display;
    screenW = width;
    screenH = height;
}

void update() {
    if (currentState == CAMERA_WAITING && expiresAt != 0 &&
        (int32_t)(millis() - expiresAt) >= 0) {
        currentState = CAMERA_EXPIRED;
        expiresAt = 0;
        dirty = true;
    }
}

bool start() {
    clearLocation();
    currentSession = randomToken(8);
    currentUrl = String("http://") + WiFi.softAPIP().toString() +
                 "/c/" + currentSession;
    currentState = CAMERA_WAITING;
    expiresAt = millis() + SESSION_TTL_MS;
    dirty = true;
    return currentSession.length() > 0;
}

void stop() {
    if (currentState == CAMERA_IDLE) return;
    clearLocation();
    currentState = CAMERA_STOPPED;
    expiresAt = 0;
    dirty = true;
}

void setState(CameraState next) {
    if (currentState == CAMERA_EXPIRED || currentState == CAMERA_STOPPED) return;
    currentState = next;
    if (next == CAMERA_CONNECTED || next == CAMERA_DISCONNECTED) expiresAt = 0;
    dirty = true;
}

bool signalState(const String& state) {
    if (state.equals("scanned")) { setState(CAMERA_SCANNED); return true; }
    if (state.equals("requested")) { setState(CAMERA_REQUESTED); return true; }
    if (state.equals("connected")) { setState(CAMERA_CONNECTED); return true; }
    if (state.equals("disconnected")) { setState(CAMERA_DISCONNECTED); return true; }
    if (state.equals("expired")) { setState(CAMERA_EXPIRED); return true; }
    return false;
}

bool active() {
    return currentState == CAMERA_WAITING || currentState == CAMERA_SCANNED ||
           currentState == CAMERA_REQUESTED || currentState == CAMERA_CONNECTED;
}

CameraState state() { return currentState; }
String stateName() { return stateText(currentState); }
String sessionId() { return currentSession; }
String sessionUrl() { return currentUrl; }

void setLocation(const String& lat, const String& lon, const String& accuracy) {
    _hasLoc = true;
    _locLat = lat;
    _locLon = lon;
    _locAcc = accuracy;
}

void clearLocation() {
    _hasLoc = false;
    _locLat = String();
    _locLon = String();
    _locAcc = String();
}

bool hasLocation() { return _hasLoc; }
String locationLat() { return _locLat; }
String locationLon() { return _locLon; }
String locationAccuracy() { return _locAcc; }

uint32_t secondsLeft() {
    if (expiresAt == 0 || (int32_t)(millis() - expiresAt) >= 0) return 0;
    return (expiresAt - millis()) / 1000UL;
}

static String jsonEscape(const String& s) {
    static const char hexDigits[] = "0123456789abcdef";
    String out;
    out.reserve(s.length());
    for (unsigned int i = 0; i < s.length(); i++) {
        const char c = s[i];
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if ((uint8_t)c < 0x20) {
                    out += "\\u00";
                    out += hexDigits[((uint8_t)c >> 4) & 0x0F];
                    out += hexDigits[(uint8_t)c & 0x0F];
                } else {
                    out += c;
                }
        }
    }
    return out;
}

String statusJson() {
    String json;
    json.reserve(192);
    json += "{\"ok\":true,\"deviceId\":\"" + jsonEscape(device::id()) + "\"";
    json += ",\"state\":\"" + jsonEscape(stateName()) + "\"";
    json += ",\"sessionId\":\"" + jsonEscape(currentSession) + "\"";
    json += ",\"url\":\"" + jsonEscape(currentUrl) + "\"";
    json += ",\"expiresInSec\":" + String(secondsLeft());
    json += ",\"hasLocation\":";
    json += _hasLoc ? "true" : "false";
    if (_hasLoc) {
        json += ",\"location\":{\"lat\":\"" + jsonEscape(_locLat) + "\"";
        json += ",\"lon\":\"" + jsonEscape(_locLon) + "\"";
        json += ",\"accuracy\":\"" + jsonEscape(_locAcc) + "\"}";
    }
    json += "}";
    return json;
}

static void drawQr(QRCode& qr, int16_t x, int16_t y, uint8_t scale) {
    if (!displayRef || qr.size == 0 || scale < 1) return;
    const uint8_t qrSize = qr.size;
    for (uint8_t row = 0; row < qrSize; row++) {
        for (uint8_t col = 0; col < qrSize; col++) {
            if (qrcode_getModule(&qr, col, row)) {
                displayRef->fillRect(x + (int16_t)col * (int16_t)scale,
                                     y + (int16_t)row * (int16_t)scale,
                                     (int16_t)scale,
                                     (int16_t)scale,
                                     SSD1306_WHITE);
            }
        }
    }
}

void render() {
    if (!displayRef || !dirty) return;

    displayRef->clearDisplay();
    displayRef->setTextSize(1);
    displayRef->setTextColor(SSD1306_WHITE);

    if (!active()) {
        displayRef->setCursor(18, 22);
        displayRef->print("SESSION CLOSED");
        displayRef->setCursor(36, 38);
        displayRef->print(stateText(currentState));
        displayRef->display();
        dirty = false;
        return;
    }

    // Safe area superiore: le prime righe del pannello sono gialle a basso
    // contrasto per lo scanner; si posiziona il QR nella meta' blu spingendolo
    // verso il basso, senza tagliare mai la quiet zone laterale.
    static const uint8_t kMarginH = 4;        // quiet zone laterale
    static const uint8_t kBottomMargin = 2;   // margine inferiore

    char url[96];
    currentUrl.toCharArray(url, sizeof(url));
    uint8_t qrData[qrcode_getBufferSize(3)];
    QRCode qr;
    qr.size = 0;
    // ECC_LOW + URL corto (/c/<8>) => versione 2 (25 moduli): moduli da 2 px,
    // molto piu' leggibili da iPhone rispetto ai 29 moduli a 1 px di prima.
    if (qrcode_initText(&qr, qrData, 3, ECC_LOW, url) < 0) {
        qr.size = 0;
    }

    if (qr.size > 0) {
        const int16_t availW = (int16_t)screenW - (int16_t)(2 * kMarginH);
        const int16_t availH = (int16_t)screenH - (int16_t)kBottomMargin;
        uint8_t scaleW = 1;
        uint8_t scaleH = 1;
        if (qr.size > 0 && availW > 0) {
            scaleW = (uint8_t)(availW / (int16_t)qr.size);
        }
        if (qr.size > 0 && availH > 0) {
            scaleH = (uint8_t)(availH / (int16_t)qr.size);
        }
        uint8_t scale = scaleW < scaleH ? scaleW : scaleH;
        if (scale < 1) scale = 1;
        const int16_t qrPixels = (int16_t)qr.size * (int16_t)scale;
        const int16_t x = ((int16_t)screenW - qrPixels) / 2;
        // Banda superiore del pannello = gialla (16 righe) a contrasto ridotto:
        // se il QR sta interamente sotto (zona blu) lo si centra li', altrimenti
        // lo si allinea in basso esponendo al giallo solo le prime righe.
        const int16_t blueTop = 16;
        const int16_t availBlue = (int16_t)screenH - blueTop - (int16_t)kBottomMargin;
        int16_t y;
        if (qrPixels <= availBlue) {
            y = blueTop + (availBlue - qrPixels) / 2;
        } else {
            y = (int16_t)screenH - qrPixels - (int16_t)kBottomMargin;
        }
        if (y < 2) y = 2;
        drawQr(qr, x, y, scale);
    }

    displayRef->display();
    dirty = false;
}

} // namespace pk_camera