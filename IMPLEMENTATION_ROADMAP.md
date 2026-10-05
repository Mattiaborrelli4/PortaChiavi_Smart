# IMPLEMENTATION ROADMAP

Piano ordinato per portare PortaChiave allo stato completo, costruito
sull'aUdito di `PROJECT_STATUS.md`.  Priorità: P0 (bloccante/sicurezza
critica), P1 (funzionalità principale rotta), P2 (bug importante),
P3 (miglioramento), P4 (feature futura).

Regola generale: NON rompere i comandi esistenti
(`emotion:happy`, `anim:blink`, `sleep:on`, ...) e NON rifattorizzare in
blocco; ogni fase è indipendente e fa build + upload di verifica.

---

## PHASE 0 — Stabilizzazione (prima cosa da fare)

Obbiettivo: nessun crash, nessuna funzionalità "morta" e build pulita.

| # | Problema | File | Soluzione | Priorità |
| - | -------- | ---- | --------- | -------- |
| 0.1 | Sessione globale: chiunque diventa OWNER | `src/auth.cpp`, `src/pk_api.cpp` | Stato per-client (chiave = MAC/IP del requester) o token di sessione per-client; l'ownerGate valuta sull'identità di quel client | P0 |
| 0.2 | `AUTH_DEV_MODE=1` di default: login dev senza segreto | `include/auth.h` / `src/pk_api.cpp` | Disponre dev-mode a `0` di default (solo build di test) oppure usare un segreto condiviso | P0 |
| 0.3 | Stack overflow in sendProbeReq | `src/pk_net.cpp:381` | Allargare `uint8_t p[64]` -> `p[80]` oppure troncare SSID a 24 char | P1 |
| 0.4 | Scan async senza timeout; `WIFI_SCAN_RUNNING` ignorato da stopAll/attack | `src/pk_net.cpp` | Timeout su isBusy/MODE_SCAN; gestire ritorno -2 (restart/annulla) prima di attaccare | P1 |
| 0.5 | Risposta evil start/stop inviata DOPO il softAP change | `src/pk_api.cpp` (`handleApiEvilStart/Stop`) | Inviare la risposta JSON PRIMA di `softAPdisconnect`/`softAP`, oppure usare un task deferrito | P1 |
| 0.6 | Camera dead: `begin/update/render` mai chiamati | `src/main.cpp` | Chiamare `pk_camera::update()` nel loop (e `render()` solo quando `dirty`) | P1 |
| 0.7 | Dashboard funzioni vuote: `jget()` su risposta già parsata | `src/pk_dashboard.cpp` (loadFuncs/loadStats/loadSec) | Parsare una volta sola e usare l'oggetto `r` | P1 |
| 0.8 | `while(true)` se OLED assente | `src/main.cpp` | Retry con timeout + fallback a loop senza occhi + dettaglio su Serial | P2 |
| 0.9 | Timeout timer poll (loadNet/netPoll/loadEvil/loadMon) mai cancellati | `src/pk_dashboard.cpp` | WeakSet/cancel prima di nuovo avvio; un solo polling attivo per screen | P2 |

Criterio di uscita: avvio pulito anche senza OLED; nessuna response JSON
persa; scan/sniff/attacchi tornano sempre a IDLE; camera gestita dal
loop; dashboard senza schermate vuote.  Build + upload COM6.

---

## PHASE 1 — RobotEyes (espressioni e behaviors realmente visibili)

Cosa sistemare (dettagli nel PROJECT_STATUS sez. 3) nell'ordine logico:
render → eyelid → pupil → blink/wink → behavior/event.

1. **Non far sovrascrivere nulla**: `calculatePositions()` non deve
   azzerare `pupilX/Y`; `applyEmotion()` non deve toccare `targetEyelid`
   se un'animazione dedicata è attiva.
2. **Pupille**: farle muovere davvero (position/offset + gaze).
3. **Eyelids**: propagare `targetEyelid` al render e rispettare
   `setEyelidOpen/setEyelids`.
4. **Blink/double blink/wink**: correggere wink (una sola palpebra),
   rendere il double blink realmente doppio.
5. **Tilt**: interpolare `leftTilt/rightTilt` tra stati.
6. **Easing**: usare `easeInOut`/`easeOutBack` nelle transizioni.
7. **Idle**: aumentare l'ampiezza dell'idle movement (visibile su 64px).
8. **Event system**: collegare `dispatchEvent(EYE_EVENT_*)` a menu/scan/
   camera/error; ogni evento → reazione occhi + log (`value_store`).
9. **SLEEPY**: mantenere la EMOZIONE sleepy distinta dal sleep MODE.
10. **src/eyes/**: decisione su vuoti file (riempirli o rimuoverli con
    nota). NON eliminarli prima di aver deciso.

Test: per ogni emozione → render, eyelid, pupil, transition, idle, event.
Matrice di test in `TEST_MATRIX.md` (sezione 11 host.txt).

---

## PHASE 2 — Dashboard complete (UI coerente + fix JS)

1. Fix bug P1/P2 di PHASE 0 applicati alla JS.
2. Struttura predefinita da host.txt: HOME → INFO DEVICE → NETWORK →
   SECURITY → CAMERA → SCAN/TOOLS → SETTINGS (già quasi tutta).
3. Uniformare stati loading/success/error (messaggi pannello).
4. Integrare reazione occhi: quando un evento UI/API parte, mandare
   l'evento RobotEyes (es. WIFI_SCAN_START → gli occhi "scansionano").
5. Sostituire emoji generiche con icone SVG/geometrie/terminal-style
   (regola host.txt sezione 3).
6. Verificare responsive (mobile = il "telefono" che aprirà il portal).

---

## PHASE 3 — Camera Pairing reale

1. Collegare `update()` e `render()` (fatto in PHASE 0) → QR visibile.
2. Handler server `/camera/<sessione>`: pagina smartphone (richiesta
   permesso camera, senza caviardare WebRTC).
3. Machine a stati completa: WAITING → SCANNED → REQUESTED →
   CONNECTED / DISCONNECTED → EXPIRED/STOPPED.
4. Expiration reale: già in `update()`, basta collegarla; render "EXPIRED"
   quando scaduta; `stop()` invalida la sessione (non riusabile).
5. Token più imprevedibile: seed da `ESP.getADC`/`analogRead` o `random32`
   WiFi; considerare formato 6-6 char.
6. QR: verificare che l'URL venga scansionato e la pagina si apra.

---

## PHASE 4 — WebRTC (design; implementazione su richiesta)

NON implementare subito (host.txt).  In questa fase:
* Definire signaling (WebSocket) + offer/answer + ICE; TURN fallback.
* Suddivisione di responsabilità ESP/browser/backend (docs/architecture).
* Necessità HTTPS (getUserMedia su origine sicura) → reverse proxy/HTTPS
  sul backend oppure page servita con token.
* Dashboard "WebRTC Viewer", smartphone "WebRTC Publisher".
* disconnect/expiration handling.

Implementazione backend SOLO quando esplicitamente richiesta.

---

## PHASE 5 — Security Hardening

1. Sessione per-client con token di sessione e header Cookie/Authorization
   (P0 del PHASE 0, qui il completo).
2. Autenticazione robusta: prevedere hash password/config su FS;
   rimuovere/sostituire il fallback dev-mode con modalità "primo setup".
3. Rate limiting su /api/login e /submit.
4. Validazione input su tutte le route (SSID, channel, file download).
5. Download wardrive: autenticazione obbligatoria + Content-Disposition.
6. /submit (phishing): validare lunghezza, ripulire da iniezioni CSV.
7. Log accessi su LittleFS con rotazione (anziché solo RAM).
8. Evil log: valutare cifratura o cancellazione automatica.

---

## PHASE 6 — Polish / performance

* Memoria: tenere RAM sotto il 60% per headroom; IRAM vicino a 88% →
  eventualmente spostare funzioni in flash (PROGMEM).
* Logging distribuito (livelli, timestamp).
* Error states su display (mensole OCR/sistema).
* Documentazione aggiornata (incl. questo README/roadmap).

---

## Priorità e dipendenze

- 0.1/0.2 PRIMA di pubblicare l'AP in rete aperta (P0 security).
- 0.3 PRIMA di usare probe flood (crash).
- 0.4 PRIMA di mixare scan e attacchi dalla UI (stuck).
- 0.6 PRIMA di PHASE 3 (camera).
- PHASE 1 indipendente da PHASE 3/4 (robot non dipende da camera).
- PHASE 4 richiede PHASE 3 (stato camera) per il pairing.
- PHASE 5 può partire in parallelo a PHASE 1-4 (security layer).

---

## Primo batch consigliato (PHASE 0)

1. [P0] Sessione per-client + dev-mode off (auth/pk_api).
2. [P1] Fix `sendProbeReq` buffer.
3. [P1] Scan timeout + gestione `WIFI_SCAN_RUNNING`.
4. [P1] Risposta evil inviata prima del cambio AP.
5. [P1] Wiring camera `update()`/`render()`.
6. [P1] Fix `jget` dashboard.
7. [P2] OLED fallback + timeout.

Dopo il batch: build, upload, test manuale su COM6, aggiornare questi
documenti.