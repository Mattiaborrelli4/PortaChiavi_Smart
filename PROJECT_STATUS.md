# PROJECT STATUS

Fotografia tecnica reale di PortaChiave dopo il lavoro di OpenCode e
l'AUDIT completo (7 sotto-agenti specializzati).  Documento fratello:
`IMPLEMENTATION_ROADMAP.md` (il piano ordinato) e `FUNZIONI.txt`
(glossario delle funzioni).

## 1. Hardware

| Elemento   | Stato                                   | Note |
| ---------- | --------------------------------------- | ---- |
| ESP8266    | DIMOSTRATO (NodeMCU, nodemcuv2)         | Upload OK su COM6, hash verificato |
| Display    | SSD1306 128x64 I2C monocromatico        | Probit connessione in main() |
| Pin        | I2C default (SDA/SCL) + pin di default  | Niente pin personalizzati nel codice |
| Periferiche| OLED + ESP8266 radio (AP + promiscuo)   | Niente camera/USB host reali |
| Stato HW   | Funzionante                            | Firmware 456416 byte su COM6 |

## 2. Firmware

Piattaforma: PlatformIO, framework Arduino ESP8266 core **3.1.2**,
SDK NONOSDK22x_190703. AP di servizio "PK-xxxxxx" + captive portal.

Moduli e stato:

| Modulo        | Stato                      | Problemi |
| ------------- | -------------------------- | -------- |
| main.cpp      | Cablato (nuovo)            | `while(true)` se OLED assente |
| wifi_ap       | OK                         | - |
| pk_net        | OK in scan/sniff/attacchi  | buffer probe req [P1], scan senza timeout [P1] |
| pk_wardrive   | Nuovo (LittleFS)           | dedup solo per sessione |
| pk_evil       | Nuovo                      | risposta API prima del softAP [P1] |
| pk_scanner    | OK                         | - |
| pk_captive    | OK                         | - |
| pk_api        | OK                         | route Evil/wardrive nuove |
| pk_access     | OK                         | lista funzioni hardcoded |
| auth          | OK (ma P0 di sicurezza)    | vedi sez. 5 |
| device        | OK                         | - |
| value_store   | OK                         | hash collisioni possibili (P4) |
| pk_robot      | OK                         | dipende da bug RobotEyes |
| RobotEyes     | Parziale (molte parti rotte) | vedi sez. 3 |
| pk_camera     | Parziale                   | mai cablato [P1] |
| pk_dashboard  | OK                         | bug JS [P1/P2] |

### API principali (agente 4)
`/api/device`, `/api/system`, `/api/modules`, `/api/wifi/status`,
`/api/wifi/scan`, `/api/net/scan|sniff|attack|attack-state|attack-stop`,
`/api/auth/status|login|logout`, `/api/values`, `/api/control`,
`/api/settings`, `/api/robot`, `/api/security/status|audit|devices`,
`/api/camera/start|stop|status`, `/api/evil*` (nuovi), `/api/wardrive`
(nuovo), `/download`, `/submit` (phishing, pubblico).
Compatibili comandi legacy: `emotion:happy`, `anim:blink`, `sleep:on`,
`breath:on`, `fps`, protocollo `chiave:valore`.

## 3. RobotEyes

Tabella espressioni (verifica visuale/logica):

| Funzione    | Stato        | Problemi                                       | Priorità |
| ----------- | ------------ | ---------------------------------------------- | -------- |
| NORMAL      | Parziale     | pupille sempre al centro; configurazione base  | P1       |
| HAPPY       | Parziale     | visibile ma sovrascritto da eyelid/pupille     | P1       |
| SAD         | Parziale     | idem                                           | P1       |
| ANGRY       | Parziale     | idem                                           | P1       |
| SLEEPY      | Parziale     | mood vs emotion conflitto (`_sleepMode`)      | P1       |
| SURPRISED   | Parziale     | idem                                           | P1       |
| CURIOUS     | Parziale     | solo nome associato a parametri sovrascritti   | P1       |
| SCARED      | Parziale     | idem                                           | P1       |
| SUSPICIOUS  | Non visibile | presente in enum ma non in `applyEmotion`      | P2       |
| FOCUSED     | Non visibile | idem                                           | P2       |
| ALERT       | Non visibile | idem                                           | P2       |
| CONFUSED    | Non visibile | idem                                           | P2       |
| SCANNING    | Non visibile | idem                                           | P2       |

Bug principali (aggiornati dal controllo specifico sezione 7 host.txt):

* `calculatePositions()` sovrascrive le coordinate della pupilla
  (`pupilX/pupilY` sempre 0) -> occhi "fissi".
* `applyEmotion()` sovrascrive gli eyelids (`targetEyelid` non propagato,
  `setEyelidOpen` non usato).
* Wink: spegne ENTRAMBI gli occhi invece di uno (`_left`/`_right`).
* `playDoubleBlink()` non esegue un doppio blink reale.
* `isOpen` dichiarato ma mai letto (inanimato).
* Tilt: `drawTiltedEye` esiste ma valore non interpolato -> poco visibile.
* Easing `easeInOut`/`easeOutBack` definiti ma non usati nelle transizioni.
* Idle movement `_movementAmplitude` troppo piccolo per essere percepito.
* `uint8_t` arithmetic in eyelid potrebbe underfloware.
* Event system: `dispatchEvent(EYE_EVENT_*)` esiste ma nessuno chiama gli
  eventi (Menu/Info/Network/...). Architettura lo "permette" (c'e'
  l'enum) ma non e' collegato dalla dashboard/API.
* `src/eyes/*` e `include/eyes/*` sono file vuoti (0 byte).

Separation Expression/Behavior/Event: esistono i tre enum; il sistema sta
in piedi architettonicamente, ma manca il wiring Behavior/Event->Render.

## 4. Dashboard

Esiste una SPA completa (HTML/CSS/JS inline in `pk_dashboard.cpp`) con:
HOME, INFO DEVICE, NETWORK/WIFI (scan, sniff, attacchi, wardriving card,
evil twin), SECURITY (permessi, audit, alert, decoy), CAMERA (stato+QR),
TOOLS (flood/random), SETTINGS (settings, robot eyes), menu navigabile,
tema cyber.

Bug trovati (agente 3):
* `loadFuncs`/`loadStats`/`loadSec` (audit) usano `jget()` su una risposta
  GIÀ parsata -> schermata FUNCTIONS/metriche SEMPRE vuote.          [P1]
* Timer ricorsivi mai cancellati: `loadNet`, `loadAtt`, `loadEvil`,
  `loadMon` -> doppia poll / leak.                                  [P2]
* `netPoll` chiama poll in parallelo con `loadNet`.                  [P2]

Manca (rispetto a sezione HOME->INFO/NETWORK/SECURITY/CAMERA/SCAN/
SETTINGS): image degli occhi in live nella dashboard (se non previsto),
niente reazione eventi robot, caricamento da stato "loading" uniforme.

## 5. Authentication

Stato attuale:
* Single global flag `_authenticated` + ruolo globale (`auth.cpp`).
* Però: **qualunque client connesso all'AP diventa OWNER** appena l'owner
  fa il login.                                      [P0 - sicurezza]
* `AUTH_DEV_MODE = 1` di default: `/api/login` accetta `{mode:'dev'}` e
  autentica senza alcuna verifica.                    [P0 - sicurezza]
* Timeout sessione: definito (es. 5 min), check in `isAuthenticated`,
  rinnovo ad ogni uso. Non c'e' logout per-client.
* `ownerGate`/`canUse`: funziona, ma sullo stato globale.

Impatto: mentre l'owner usa la dashboard, un qualunque visitatore del
captive portal vede TUTTO (funzioni evil/wardrive incluse, se i permessi
sono ruolo-based). Corregge con stato per-client (IP/MAC del requester).

## 6. Camera Pairing

* QR: `pk_camera::render()` esiste e disegna il QR (qrcode lib), ma NON
  viene mai chiamato -> niente QR sul display.                       [P1]
* Session: `start()` genera sessione `XXXXXX-XXXX`, URL
  `http://<softAPIP>/camera/<sessione>`; `stop()` -> STOPPED.
* Token: `random()` (nessun seed dedicato) - prevedibile.            [P3/P4]
* Expiration: TTL 5 min in `expiresAt`, ma `update()` mai chiamato ->
  NON scade in pratica.                                               [P1]
* Smartphone page: non esiste `/camera/...` handler -> 404 quando si
  naviga il QR.                                                      [P1]
* Camera permission: nessuna (nessuna pagina).
* WebRTC: assente (previsto solo in docs/architecture.md).
* Stati SCANNED/REQUESTED/CONNECTED/DISCONNECTED: definiti ma
  irraggiungibili (manca signaling).

## 7. Security

* Autorizzazione: `pk_access` con ruoli (owner/preview/slave/none) e
  "decoy mode" per estranei. OK di principio.
* Problemi critici: sessione globale + dev-mode (`sez. 5`).           [P0]
* `handleDownload` (CSV wardrive) e `<a>` del download: nessuna
  autenticazione di fatto sullo stream (dipende da flag).            [P1]
* Input: usati check `bodyHas`/`extractJsonString`; niente rate limiting
  sul login; niente HTTPS (AP locale).
* `value_store` hash: somma di byte -> collisioni possibili su chiavi
  simili (P4, non urgente).
* File system LittleFS auto-formattato (nessun danno dati sensibili).
* Password nell'evil log CSV: salvate in chiaro su LittleFS (P3:
  valutare se cifrare / cancellare).

## 8. Build

* Compilazione: **SUCCESS** (pio run).
* RAM: 46624/81920 B (56.9%).
* Flash: 452267/1044464 B (43.3%).
* firmware.bin: 456416 byte, upload SUCCESS su COM6 (hash verificato).
* Warning: 1 warning di codice; `SyntaxWarning` in `elf2bin.py`
  (Python SCons env: NIENTE DI ROTTO, solo avviso).
* IRAM `.text1+text`: ~28939 B su 0x8000 (~88%) - ATTENZIONE: aggiungere
  troppe funzioni in IRAM puo' arrivare al limite.   [P3]
* Heap free a boot: ~35.3 KB; realistico runtime 20-27 KB.
* Dipendenze: Arduino core 3.1.2 (SDK NONOSSDK22x_190703), librerie
  Adafruit GFX/SSD1306, QRCode.            Doppio PIO core presente
  (6.1.18 obsoleto) -> costo di compilazione, non errore.
* `platformio.ini` NON fissa la versione piattaforma (`platform =
  espressif8266`) -> build non "riproducibile".     [P3]
* FS partition: `spiffs 4m1m` + LittleFS auto-format pronta da usare.

Va bene: doppio core del platformio = ambiente locale, non progetto.

## 9. OpenCode Integration

Aggiunte/modifiche che sembrano opera di OpenCode (differenze da
timestamp/contenuto, non solo git):
* `src/pk_net.cpp` / `include/pk_net.h`:
  - nuovi `MODE_BEACON_RND`, `MODE_PROBE`;
  - `probe` flood e `beaconrnd` beacon con SSID random;
  - probe tracking per client (`probes` in sniffState);
  - `stopAll()`, `handleScanProgress`, `syncMood()`, `randomSeed`.
* `src/pk_wardrive.cpp` / `include/pk_wardrive.h`: intero modulo nuovo
  (CSV WiGLE su LittleFS).
* `src/pk_evil.cpp` / `include/pk_evil.h`: intero modulo nuovo
  (AP clone + phishing + log CSV). Route relative in `pk_api`.
* `src/pk_api.cpp`: route `evil`, `wardrive`, `download`, `submit`,
  root-switch evil/dashboard, `ownerGate` esteso.
* `src/pk_dashboard.cpp`: scr-evil, card wardriving, card flood/random,
  box probe nel monitor, JS (loadWD, loadEvil, evilrow, wddl, ...).
* `src/main.cpp`: **wiring completo** setup/loop (prima era solo OLED).
* `src/pk_camera.cpp`: fix `ESP.random` -> `random()`.
* `FUNZIONI.txt`, `PROJECT_STATUS.md`, `IMPLEMENTATION_ROADMAP.md`
  (questi 3 documenti, questa sessione).

## Prossimità rispetto all'architettura definita

Da `docs/architecture.md` l'intesa prevede ESP8266 + RobotEyes +
Dashboard + API + Security + Camera Pairing + WebRTC (futuro).  Stato
generale: SOLIDO per WiFi/API/dashboard/security base; INCOMPLETO per
RobotEyes (bug), Camera Pairing e WebRTC (quasi assenti) - vedi
IMPLEMENTATION_ROADMAP.md per l'ordine di lavoro.