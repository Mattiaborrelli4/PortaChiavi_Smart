#pragma once

#include <Arduino.h>

// ============================================================================
// PortaChiave - Web Server + API Core
//
// Endpoint strutturati:
//   GET  /                   -> dashboard
//   GET  /api/device
//   GET  /api/wifi/status
//   GET  /api/system
//   GET  /api/values
//   GET  /api/auth/status
//   POST /api/auth/login      (DEV mode simula il risultato Face ID)
//   POST /api/auth/logout
//   GET  /api/wifi/scan       (futuro: non implementato)
//   POST /api/robot           (AUTH_REQUIRED)
//   POST /api/device/camera/start|stop
//   GET  /api/device/camera/status
//   GET  /values              (legacy)
//   GET  /button              (legacy)
// ============================================================================

namespace api {

void begin();
void handleClient();

} // namespace api