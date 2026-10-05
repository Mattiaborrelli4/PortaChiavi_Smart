#pragma once

#include <Arduino.h>
#include <Adafruit_SSD1306.h>

namespace pk_camera {

enum CameraState : uint8_t {
    CAMERA_IDLE = 0,
    CAMERA_WAITING,
    CAMERA_SCANNED,
    CAMERA_REQUESTED,
    CAMERA_CONNECTED,
    CAMERA_DISCONNECTED,
    CAMERA_EXPIRED,
    CAMERA_STOPPED
};

void begin(Adafruit_SSD1306* display, uint16_t screenWidth, uint16_t screenHeight);
void update();
void render();
bool start();
void stop();
void setState(CameraState state);
bool active();
CameraState state();
String stateName();
String sessionId();
String sessionUrl();
uint32_t secondsLeft();
String statusJson();
bool signalState(const String& state);
void setLocation(const String& lat, const String& lon, const String& accuracy);
void clearLocation();
bool hasLocation();
String locationLat();
String locationLon();
String locationAccuracy();

} // namespace pk_camera