#include <pk_captive.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>

namespace captive {

static DNSServer dnsServer;

void begin() {
    // Risponde a qualsiasi dominio risolvendolo all'IP del softAP (192.168.4.1)
    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    bool ok = dnsServer.start(53, "*", WiFi.softAPIP());
    if (ok) {
        Serial.println(F("[Captive] DNS attivo: qualsiasi hostname -> 192.168.4.1"));
    } else {
        Serial.println(F("[Captive] ERRORE: DNS non avviato"));
    }
}

void process() {
    dnsServer.processNextRequest();
}

} // namespace captive