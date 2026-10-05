#pragma once

// ============================================================================
// PortaChiave - AnimationManager (animazioni fullscreen + priorita' schermo)
//
// Decide QUALE superficie possiede lo schermo secondo la priorita':
//     QR > WIFI_SCAN > LAG > REACTION > EYES (default)
// e disegna le animazioni fullscreen (scan Wi-Fi "caricamento", effetto
// lag/glitch, reazione breve a comandi telefono). Nessun delay() bloccante:
// tutto e' temporizzato con millis() e disegnato sul buffer SSD1306, che
// viene pushato via display() solo quando il frame e' cambiato (niente
// flickering).
//
// Integrazione (a cura di main.cpp / pk_net.cpp):
//   - animations::begin(&display)     dopo display.begin() (come gli occhi)
//   - animations::requestWifiScan()   quando pk_net avvia uno scan
//   - animations::completeWifiScan(n) a scan completato
//   - animations::requestLag()        su errore operazione (futuro)
//   - nel loop: animations::update(); se isSpecial() -> animations::render()
//   - currentSurface() ritorna QR quando pk_camera::active(): in quel caso
//     il rendering resta di competenza di pk_camera::render().
// ============================================================================

#include <Arduino.h>
#include <Adafruit_SSD1306.h>

namespace animations {

// Superficie che possiede lo schermo in questo momento.
enum class Surface : uint8_t {
    EYES = 0,      // occhi RobotEyes (default)
    QR,            // QR pairing di pk_camera (priorita' massima)
    WIFI_SCAN,     // animazione scan Wi-Fi fullscreen
    LAG,           // effetto lag/glitch fullscreen (si auto-termina)
    REACTION       // reazione breve a comandi del telefono (opzionale)
};

// Salva il puntatore al display usato per il rendering. Da chiamare
// dopo display.begin(), esattamente come per gli occhi / pk_camera.
void begin(Adafruit_SSD1306* display);

// --- richieste (priorita' gestita internamente) ---

// Avvia l'animazione "caricamento scan Wi-Fi" (nessun occhio durante il
// simbolo). Se una LAG o una REACTION era attiva, parte comunque (WIFI_SCAN
// ha priorita' piu' alta). Chiamata da pk_net a inizio scan.
void requestWifiScan();

// Fine dello scan: breve esito "aggancio" + testo "<n> reti", poi ritorno
// automatico a EYES. Chiamata da pk_net al termine dello scan.
void completeWifiScan(uint16_t found);

// Riproduce l'effetto lag/glitch fullscreen (~1.7s) e torna da solo a EYES.
void requestLag();

// Reazione breve a un comando del telefono (~0.7s): check tondo "pop".
void requestReaction();

// Forza il ritorno a EYES (chiude qualunque animazione in corso).
void clearSpecial();

// --- interrogazione per main ---
Surface currentSurface();
bool isSpecial();          // true se animations sta disegnando qualcosa
                           // (WIFI_SCAN/LAG/REACTION; false per QR e EYES)

// --- poll del loop ---
void update();             // avanza la macchina a stati (da chiamare ogni loop)
void render();             // disegna la superficie attiva; display() solo se frame nuovo
bool dirty();              // true se serve un render / screen refresh

} // namespace animations