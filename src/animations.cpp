#include <animations.h>
#include <pk_camera.h>

// ============================================================================
// PortaChiave - AnimationManager (implementazione)
//
// Macchina a stati interna:
//     EYES <--> WIFI_SCAN --(completeWifiScan / timeout)--> WIFI_DONE -> EYES
//     EYES <--> LAG      (si auto-termina)
//     EYES <--> REACTION (si auto-termina)
//
// currentSurface() risolve la priorita':
//     pk_camera::active()  -> QR            (camera possiede lo schermo)
//     WIFI_SCAN/WIFI_DONE  -> WIFI_SCAN
//     LAG                  -> LAG
//     REACTION             -> REACTION
//     altrimenti           -> EYES
// WIFI_SCAN e' il piu' "impaziente": una richiesta di scan durante una LAG
// o una REACTION parte subito (sovrascrive); al contrario requestLag()/
// requestReaction() vengono ignorate se uno scan e' in corso, perche' QR >
// WIFI_SCAN > LAG > REACTION > EYES.
//
// Timing scelti:
//     scan   : sweep continuo (aperto, ~3-5s reali; al massimo 12s di
//              timeout di sicurezza, uguale al timeout di pk_net), poi lock
//              + esito 1.5s
//     lag    : 1.7s su 5 slot (glitch -> freeze -> recupero) -> EYES
//     reaction: 0.7s (pop cerchio + check) -> EYES
// Nessuna allocazione dinamica, nessun delay(), solo uint32_t timestamp.
// ============================================================================

namespace animations {

// ------------------------------------------------------------------ statica
enum class SubState : uint8_t {
    EYES = 0,
    WIFI_SCAN,
    WIFI_DONE,
    LAG,
    REACTION
};

static Adafruit_SSD1306* displayRef = nullptr;

static SubState _sub         = SubState::EYES;
static bool     _dirty       = false;
static uint32_t _startedAt   = 0;      // inizio della SubState corrente
static uint16_t _found       = 0;      // reti trovate (esito scan)
static uint32_t _lastRender  = 0;      // throttle del rendering
static bool     _resultDrawn = false;  // il frame "N reti" e' gia' stato disegnato
static bool     _reactionHeld = false; // il frame statico di REACTION e' gia' stato disegnato
static uint8_t  _lastSlot    = 0xFF;   // ultimo slot LAG disegnato
static uint32_t _lagSeed     = 0;      // LCG deterministico per il glitch

// ------------------------------------------------------------------ tempi
static const uint32_t SCAN_TIMEOUT_MS = 12000UL; // = timeout scan di pk_net
static const uint32_t WIFI_DONE_MS    = 1500UL;  // pulsa aggancio + "N reti"
static const uint32_t WIFI_PULSE_MS   = 560UL;   // durata lampeggi di lock
static const uint32_t LAG_MS          = 1700UL;
static const uint32_t REACTION_MS     = 700UL;
static const uint32_t RENDER_MS       = 45UL;    // cadenza dei frame animati

// ------------------------------------------------------------------ geometria wifi (128x64)
// Il simbolo vive basso e centrato: la fascia "attiva" non tocca le prime
// ~8 righe in alto (zona gialla del modulo). Raggi degli archi: 7/12/17,
// il primo piccolo cerchio/arco nasce al centro e cresce durante la carica.
static const int16_t  WIFI_CX      = 64;
static const int16_t  WIFI_CY      = 40;
static const int16_t  ARC_R[3]     = { 10, 15, 20 };
static const int16_t  WIFI_DOT_R   = 3;
static const int16_t  LOCK_R       = 30;     // alone di "aggancio" (ARC_R[2]+3)
static const int16_t  WAVE_R_MIN   = 8;
static const int16_t  WAVE_R_MAX   = 30;     // < CY-8 per restare sotto la fascia

// ------------------------------------------------------------------ helper

// Hash deterministico per-pixel: simula il contrasto/vicinanza senza aver
// una scala di grigi (dithering stabile, zero flicker).
static uint32_t pixelHash(int16_t x, int16_t y) {
    uint32_t h = (uint32_t)x * 374761393u + (uint32_t)y * 668265263u + 0x9E3779B9u;
    h = (h ^ (h >> 13)) * 0x5BD1E995u;
    return h ^ (h >> 15);
}

static void plotGray(int16_t x, int16_t y, uint32_t gray) {
    if (x < 0 || x >= 128 || y < 0 || y >= 64) return;
    if (gray == 0) return;
    if (gray >= 252u) { displayRef->drawPixel(x, y, SSD1306_WHITE); return; }
    if ((pixelHash(x, y) & 0xFFu) <= gray) {
        displayRef->drawPixel(x, y, SSD1306_WHITE);
    }
}

// Cammina la circonferenza di raggio r (midpoint/Bresenham, niente sin/cos)
// e plotta i punti del settore "wifi": meta' superiore, cono ~±58° dal
// vertice, come i due corni del simbolo standard.
static void wifiArcRing(int16_t r, uint32_t gray) {
    if (r < 1) r = 1;
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0, y = r;
    int16_t limit = (int16_t)((r * 85) / 100); // |px| <= 0.85r (cono)
    while (x <= y) {
        if (f >= 0) { y--; ddF_y += 2; f += ddF_y; }
        x++;
        ddF_x += 2;
        f += ddF_x;
        const int16_t px[4] = { x, y, (int16_t)-x, (int16_t)-y };
        const int16_t py[4] = { (int16_t)-y, (int16_t)-x, (int16_t)-y, (int16_t)-x };
        for (int k = 0; k < 4; k++) {
            if (py[k] >= 0) continue;                 // solo sopra il centro
            int16_t ax = px[k] < 0 ? -px[k] : px[k];
            if (ax > limit) continue;                 // fuori dal cono
            plotGray(WIFI_CX + px[k], WIFI_CY + py[k], gray);
        }
    }
}

// Dente di sega triangolare 0..255..0 di periodo `period` ms.
static uint32_t triPulse(uint32_t t, uint32_t period) {
    uint32_t ph = t % period;
    uint32_t half = period / 2u;
    return (ph < half) ? (ph * 255u) / half : ((period - ph) * 255u) / half;
}

// ==========================================================================
// RENDERING WI-FI SCAN (sweep di "caricamento")
// ==========================================================================
// Simbolo grande e centrale, niente occhi. Un piccolo anello nasce al centro
// e cresce; i tre archi si accendono con un fronte di luminosita' che viaggia
// dall'arco interno verso l'esterno (carica progressiva, sfumata via
// dithering), mentre onde/archi eco si espandono e svaniscono lentamente.
static void drawWifiSweep(uint32_t now) {
    Adafruit_SSD1306* d = displayRef;
    const uint32_t t = now - _startedAt;
    d->clearDisplay();

    // opacita' iniziale: il simbolo "emerge" nel primo secondo
    const uint32_t charge = (t < 1000UL) ? (t * 255u) / 1000UL : 255u;

    // piccolo cerchio centrale che si espande all'inizio (fino al primo arco)
    if (t < 700UL) {
        int16_t rr = 1 + (int16_t)((t * 3u) / 700UL); // 1..4
        wifiArcRing(rr, charge);
    }

    // fronte di carica: la luminosita' massima viaggia arco-interno -> esterno
    const uint32_t period = 1400UL;
    const uint32_t win = period / 3u;
    const uint32_t cyc = t % period;
    const uint32_t base = (charge * 70u) / 255u;
    for (int i = 0; i < 3; i++) {
        uint32_t local = (cyc + (uint32_t)i * win) % period;
        uint32_t front = 0;
        if (local <= win) {
            uint32_t a = win / 2u; // picco a meta' finestra: front 0 -> max -> 0
            front = (local <= a) ? (local * 255u) / a : ((win - local) * 255u) / (win - a);
        }
        uint32_t gray = base + (front * charge) / 255u;
        if (gray > 255u) gray = 255u;
        wifiArcRing(ARC_R[i], gray);
        wifiArcRing(ARC_R[i] + 1, gray);
    }

    // onde/archi eco: nascono al centro, si espandono fino a WAVE_R_MAX e
    // svaniscono (triangolo di intensita'), una ogni 400ms
    const uint32_t spawn = 400UL;
    const uint32_t wPeriod = 1000UL;
    const uint32_t n = t / spawn;
    uint32_t count = n + 1u;
    if (count > 3u) count = 3u;
    for (uint32_t j = 0; j < count; j++) {
        uint32_t age = t - (n - (count - 1u - j)) * spawn;
        if (age >= wPeriod) continue;
        uint32_t p = (age * 255u) / wPeriod;
        uint32_t env = (p < 128u) ? (p * 2u) : (255u - p) * 2u;
        uint32_t gray = 22u + (env * 80u) / 255u; // eco debole, non disturba il simbolo
        int16_t R = WAVE_R_MIN + (int16_t)((p * (uint32_t)(WAVE_R_MAX - WAVE_R_MIN)) / 255u);
        wifiArcRing(R, gray);
        wifiArcRing(R + 1, gray);
    }

    // nodo centrale: emerge col charge poi pulsa lentamente (triPulse)
    if (charge > 0u) {
        const uint32_t dp = triPulse(t, 900u);
        uint32_t dotGray = (charge * (170u + dp / 2u)) / 255u;
        if (dotGray > 255u) dotGray = 255u;
        if (dotGray >= 252u) {
            d->fillCircle(WIFI_CX, WIFI_CY, WIFI_DOT_R, SSD1306_WHITE);
        } else {
            plotGray(WIFI_CX, WIFI_CY, dotGray);
            plotGray(WIFI_CX - 1, WIFI_CY, dotGray);
            plotGray(WIFI_CX + 1, WIFI_CY, dotGray);
            plotGray(WIFI_CX, WIFI_CY - 1, dotGray);
            plotGray(WIFI_CX, WIFI_CY + 1, dotGray);
        }
    }
}

// ==========================================================================
// RENDERING ESITO SCAN (lock + "N reti")
// ==========================================================================
// Fase pulsa: 3 lampeggi pieni, poi si stabilizza con il testo dell'esito.
static void drawWifiDonePulse(bool pulseOn) {
    Adafruit_SSD1306* d = displayRef;
    d->clearDisplay();
    const uint32_t gray = pulseOn ? 255u : 28u;
    for (int i = 0; i < 3; i++) {
        wifiArcRing(ARC_R[i], gray);
        wifiArcRing(ARC_R[i] + 1, gray);
    }
    wifiArcRing(LOCK_R, gray);         // alone di aggancio
    wifiArcRing(LOCK_R + 1, gray);
    if (pulseOn) d->fillCircle(WIFI_CX, WIFI_CY, WIFI_DOT_R, SSD1306_WHITE);
}

static void drawWifiDoneResult() {
    Adafruit_SSD1306* d = displayRef;
    d->clearDisplay();
    for (int i = 0; i < 3; i++) {
        wifiArcRing(ARC_R[i], 255u);
        wifiArcRing(ARC_R[i] + 1, 255u);
    }
    wifiArcRing(LOCK_R, 255u);         // alone stabile = aggancio completato
    wifiArcRing(LOCK_R + 1, 255u);
    d->fillCircle(WIFI_CX, WIFI_CY, WIFI_DOT_R, SSD1306_WHITE);

    char buf[14];
    snprintf(buf, sizeof(buf), "%u reti", (unsigned)_found);
    int16_t x1, y1;
    uint16_t tw, th;
    d->setTextSize(1);
    d->setTextColor(SSD1306_WHITE);
    d->getTextBounds(buf, 0, 0, &x1, &y1, &tw, &th);
    d->setCursor((128 - (int16_t)tw) / 2, 42);
    d->print(buf);
}

// ==========================================================================
// RENDERING LAG / GLITCH (fullscreen, ~1.7s, si auto-termina)
// ==========================================================================
// Slot che "si congelano" (micro-freeze), barre e blocchi in posizioni
// pseudo-casuali deterministiche, strisce orizzontali spostate di 2-3px,
// recupero progressivo. Ogni slot viene disegnato UNA volta sul buffer: il
// frame resta fermo tra uno slot e l'altro (effetto freeze senza batter
// I2C inutilmente).
static uint32_t lagRand() {
    _lagSeed = _lagSeed * 1664525u + 1013904223u;
    return _lagSeed >> 16;
}

static void drawLagFrame(uint8_t frame, uint32_t now) {
    Adafruit_SSD1306* d = displayRef;
    d->clearDisplay();
    _lagSeed = (now * 0x9E3779B9u) ^ ((uint32_t)frame * 0xC2B2AE35u) ^ 0x12345678u;

    switch (frame) {
        case 0: { // comparsa: barre sparse + un blocco
            uint32_t bars = 3u + (lagRand() % 3u);
            for (uint32_t i = 0; i < bars && i < 6; i++) {
                int16_t y = 8 + (int16_t)(lagRand() % 48u);
                int16_t x = (int16_t)(lagRand() % 60u);
                int16_t len = 20 + (int16_t)(lagRand() % 70u);
                d->drawFastHLine(x, y, len, SSD1306_WHITE);
            }
            d->fillRect((int16_t)8 + (int16_t)(lagRand() % 80u),
                        (int16_t)10 + (int16_t)(lagRand() % 36u),
                        8 + (int16_t)(lagRand() % 12u),
                        2 + (int16_t)(lagRand() % 4u), SSD1306_WHITE);
            break;
        }
        case 1: { // strisce orizzontali spostate di 2-3px (tearing)
            int16_t y1 = 14 + (int16_t)(lagRand() % 30u);
            int16_t sh = 2 + (int16_t)(lagRand() % 2u);
            d->fillRect(0, y1, 128, 4, SSD1306_BLACK);
            d->drawFastHLine(sh, y1, 42, SSD1306_WHITE);
            d->drawFastHLine(86 - sh, y1, 42, SSD1306_WHITE);
            d->drawFastHLine(0, y1 + 1, 60, SSD1306_WHITE);
            d->drawFastHLine(70, y1 + 1, 58, SSD1306_WHITE);
            d->drawFastHLine(20, y1 + 2, 40, SSD1306_WHITE);
            d->drawFastHLine(30, y1 + 3, 50, SSD1306_WHITE);
            break;
        }
        case 2: { // danno pesante: blocchi + riga a scacchi corrotta + linea
            d->fillRect(20, 22, 40, 14, SSD1306_WHITE);
            d->fillRect(72, 12, 18, 8, SSD1306_WHITE);
            for (int16_t x = 0; x < 128; x += 8) {
                d->fillRect(x, 32, 4, 2,
                            ((x / 8) & 1) ? SSD1306_WHITE : SSD1306_BLACK);
            }
            d->drawFastVLine(60, 6, 40, SSD1306_WHITE);
            for (int i = 0; i < 4; i++) {
                d->drawFastHLine(10 + (int16_t)(lagRand() % 90u),
                                 44 + (int16_t)(lagRand() % 12u),
                                 14 + (int16_t)(lagRand() % 50u), SSD1306_WHITE);
            }
            break;
        }
        case 3: { // recupero: solo due barre corte
            d->drawFastHLine(20 + (int16_t)(lagRand() % 70u),
                             22 + (int16_t)(lagRand() % 8u),
                             30 + (int16_t)(lagRand() % 30u), SSD1306_WHITE);
            d->drawFastHLine(10 + (int16_t)(lagRand() % 80u),
                             40 + (int16_t)(lagRand() % 10u),
                             24 + (int16_t)(lagRand() % 36u), SSD1306_WHITE);
            break;
        }
        default: { // quasi pulito: ultimo piccolo blocco destinato a sparire
            d->fillRect(60, 30, 8, 4, SSD1306_WHITE);
            break;
        }
    }
}

static uint8_t lagSlot(uint32_t t) {
    static const uint16_t bound[6] = { 0, 150, 330, 620, 1150, 1700 };
    for (uint8_t i = 0; i < 5; i++) {
        if (t < bound[i + 1]) return i;
    }
    return 4;
}

// ==========================================================================
// RENDERING REACTION (check tondo, ~0.7s)
// ==========================================================================
static void drawReactionFrame(uint32_t t) {
    Adafruit_SSD1306* d = displayRef;
    d->clearDisplay();
    const int16_t cx = 64, cy = 32;
    if (t < 170UL) { // cerchio che "pop" (da piccolo a pieno)
        int16_t rr = (int16_t)((t * 9u) / 170u);
        if (rr < 1) rr = 1;
        d->drawCircle(cx, cy, rr, SSD1306_WHITE);
    } else if (t < 450UL) { // cerchio pieno + check che si disegna progressivamente
        d->drawCircle(cx, cy, 9, SSD1306_WHITE);
        const int16_t x1 = 58, y1 = 31, xm = 63, ym = 36, x2 = 70, y2 = 27;
        int32_t p = (int32_t)((t - 170UL) * 255u) / 280u;
        int16_t ex1 = x1 + (int16_t)(((int32_t)(xm - x1) * p) / 255L);
        int16_t ey1 = y1 + (int16_t)(((int32_t)(ym - y1) * p) / 255L);
        int16_t sx2 = xm, sy2 = ym;
        if (p >= 128L) {
            int32_t p2 = p - 128L;
            sx2 = xm + (int16_t)(((int32_t)(x2 - xm) * p2) / 127L);
            sy2 = ym + (int16_t)(((int32_t)(y2 - ym) * p2) / 127L);
        }
        d->drawLine(x1, y1, ex1, ey1, SSD1306_WHITE);
        d->drawLine(xm, ym, sx2, sy2, SSD1306_WHITE);
    } else { // hold del check compiuto
        d->drawCircle(cx, cy, 9, SSD1306_WHITE);
        d->drawLine(58, 31, 63, 36, SSD1306_WHITE);
        d->drawLine(63, 36, 70, 27, SSD1306_WHITE);
    }
}

// ==========================================================================
// API PUBBLICA
// ==========================================================================

void begin(Adafruit_SSD1306* display) {
    displayRef = display;
    _dirty = true;
}

void requestWifiScan() {
    _sub = SubState::WIFI_SCAN;
    _startedAt = millis();
    _found = 0;
    _resultDrawn = false;
    _lastRender = 0;
    _dirty = true;
}

void completeWifiScan(uint16_t found) {
    // Priorita': l'esito dello scan ha sempre priorita' su LAG/REACTION.
    // Se chiamato dopo l'auto-timeout dello sweep, mostra comunque l'esito.
    _sub = SubState::WIFI_DONE;
    _startedAt = millis();
    _found = found;
    _resultDrawn = false;
    _lastRender = 0;
    _dirty = true;
}

void requestLag() {
    if (_sub == SubState::WIFI_SCAN || _sub == SubState::WIFI_DONE) return;
    _sub = SubState::LAG;
    _startedAt = millis();
    _lagSeed = _startedAt ^ 0x9E3779B9u;
    _lastSlot = 0xFF;
    _lastRender = 0;
    _dirty = true;
}

void requestReaction() {
    if (_sub == SubState::WIFI_SCAN || _sub == SubState::WIFI_DONE ||
        _sub == SubState::LAG) {
        return; // priorita' piu' alte attive
    }
    _sub = SubState::REACTION;
    _startedAt = millis();
    _reactionHeld = false;
    _lastRender = 0;
    _dirty = true;
}

void clearSpecial() {
    _sub = SubState::EYES;
    _resultDrawn = false;
    _reactionHeld = false;
    _lastSlot = 0xFF;
    _dirty = true;
}

Surface currentSurface() {
    if (pk_camera::active()) return Surface::QR; // priorita' massima
    switch (_sub) {
        case SubState::WIFI_SCAN:
        case SubState::WIFI_DONE: return Surface::WIFI_SCAN;
        case SubState::LAG:       return Surface::LAG;
        case SubState::REACTION:  return Surface::REACTION;
        default:                  return Surface::EYES;
    }
}

bool isSpecial() {
    Surface s = currentSurface();
    return s == Surface::WIFI_SCAN || s == Surface::LAG || s == Surface::REACTION;
}

bool dirty() {
    return _dirty;
}

void update() {
    const uint32_t now = millis();
    switch (_sub) {
        case SubState::WIFI_SCAN:
            // Durante uno scan profondo (modalità probe/sniffer), lo stato WIFI_SCAN
            // rimane attivo fino a quando completeWifiScan() non viene chiamata dal
            // driver di rete al termine dello scan. Non viene impostato un timeout
            // fisso: lo scan procede come una rete probe in corso.
            break;
        case SubState::WIFI_DONE:
            if (now - _startedAt >= WIFI_DONE_MS) {
                _sub = SubState::EYES;
                _startedAt = now;
                _resultDrawn = false;
                _dirty = true; // segnale: EYES deve ridisegnare lo schermo
            }
            break;
        case SubState::LAG:
            if (now - _startedAt >= LAG_MS) {
                _sub = SubState::EYES;
                _startedAt = now;
                _lastSlot = 0xFF;
                _dirty = true;
            }
            break;
        case SubState::REACTION:
            if (now - _startedAt >= REACTION_MS) {
                _sub = SubState::EYES;
                _startedAt = now;
                _reactionHeld = false;
                _dirty = true;
            }
            break;
        default:
            break;
    }
}

void render() {
    if (!displayRef) { _dirty = false; return; }
    const uint32_t now = millis();
    const uint32_t t = now - _startedAt;

    switch (_sub) {
        case SubState::EYES:
            _dirty = false;
            return;

        case SubState::WIFI_SCAN: { // sweep continuo, throttlato
            if (now - _lastRender < RENDER_MS) return; // _dirty resta true
            _lastRender = now;
            drawWifiSweep(now);
            displayRef->display();
            return;
        }

        case SubState::WIFI_DONE: {
            if (t >= WIFI_PULSE_MS) { // esito stabile: disegna una volta sola
                if (_resultDrawn) { _dirty = false; return; }
                if (now - _lastRender < RENDER_MS) return;
                _lastRender = now;
                drawWifiDoneResult();
                _resultDrawn = true;
                _dirty = false;
                displayRef->display();
                return;
            }
            if (now - _lastRender < (RENDER_MS / 2u)) return; // pulse piu' rapide
            _lastRender = now;
            drawWifiDonePulse(((t / 90u) & 1u) == 0u); // 3 lampeggi (~90ms)
            displayRef->display();
            return;
        }

        case SubState::LAG: { // disegna solo al cambio slot -> micro-freeze
            uint8_t slot = lagSlot(t);
            if (slot == _lastSlot) {
                if (slot == 4) _dirty = false; // ultimo slot: frame statico di recupero
                return;
            }
            _lastSlot = slot;
            drawLagFrame(slot, now);
            displayRef->display();
            return;
        }

        case SubState::REACTION: {
            if (t >= 450UL) { // fase statica finale: disegna una volta sola
                if (_reactionHeld) { _dirty = false; return; }
                if (now - _lastRender < RENDER_MS) return;
                _lastRender = now;
                drawReactionFrame(now - _startedAt);
                _reactionHeld = true;
                _dirty = false;
                displayRef->display();
                return;
            }
            if (now - _lastRender < RENDER_MS) return;
            _lastRender = now;
            drawReactionFrame(now - _startedAt);
            displayRef->display();
            return;
        }
    }
}

} // namespace animations