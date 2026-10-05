#include <device.h>
#include <ESP8266WiFi.h>

namespace device {

String id() {
    String mac = WiFi.macAddress();
    mac.replace(":", "");
    String idStr = mac;
    idStr.toUpperCase();
    return "PK-" + idStr.substring(idStr.length() - 6);
}

String name() {
    return "PortaChiave";
}

String firmware() {
    return "0.1.0";
}

String chipId() {
    String c = String(ESP.getChipId(), HEX);
    c.toUpperCase();
    return c;
}

String macSuffix() {
    String mac = WiFi.macAddress();
    mac.replace(":", "");
    mac.toUpperCase();
    return mac.substring(mac.length() - 4);
}

} // namespace device