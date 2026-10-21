# Audit Mobile - Portachiave Project

## Status: CODE CORRECTIONS COMPLETED, VERIFICATION PENDING

### Files Inspected
- `mobile_app/` - Flutter project structure
- `src/` - Firmware source code (ESP8266 NodeMCU)
- `include/` - Firmware headers
- `platformio.ini` - PlatformIO configuration with `board = nodemcuv2`

---

## 1. Flutter Project Structure

### `mobile_app/pubspec.yaml`
- Name: `portachiave_mobile`
- Dependencies: `go_router ^14.2.0`, `dio ^5.4.0`, `flutter_riverpod ^2.5.1`
- Dark theme configured with Material 3

### `mobile_app/lib/main.dart`
- Entry point using `GoRouter` for navigation
- Dark theme applied consistently
- Imports router via `package:portachiave_mobile/app/router.dart as router`

### `mobile_app/lib/app/router/go_router_config.dart`
- 7 routes defined: onboarding, connection, dashboard, device-info, auth, robot-eyes, settings
- All routes reference features that exist under `mobile_app/lib/features/`

### `mobile_app/lib/app/router/info_parser.dart`
- Route information parser for GoParser
- `GoRouteInfo.tryParse` maps paths to routes: /onboarding, /connection, /dashboard, /device-info, /auth, /robot-eyes, /settings

### `mobile_app/lib/core/network/api_client.dart`
- Dio-based API client with all endpoints inline
- `DeviceConfig` and `ApiException` defined locally (no separate files)
- Base URL: `http://192.168.4.1`, Timeout: 5 seconds
- All endpoints use direct JSON (no `success/data` envelope)
- Interceptors manage connection state (connecting, connected, networkError)

### `mobile_app/lib/core/models/device_model.dart`
- `DeviceModel` with fields: id, name, firmware, isConnected, freeHeap, uptime, isAuthenticated, connectionState
- `FeatureModule` class for feature definitions
- Field names match firmware (`freeHeap` not `freeMemory`)

### `mobile_app/lib/core/network/connection_state_model.dart`
- `ConnectionStateModel` extending ChangeNotifier
- State management for connection status with setState/reset

### `mobile_app/lib/core/widgets/app_widgets.dart`
- AppBarWidget, LoadingIndicator, errorWidgetBuilder

### `mobile_app/lib/core/utils/utils.dart`
- Utils: formatBytes, formatUptime, formatAuthStatus, getConnectionStatusColor

### `mobile_app/lib/features/` - Feature screens created under `lib/features/`
- `onboarding/onboarding_screen.dart` - Onboarding flow with connect button, warns "this network might not have Internet"
- `connection/connection_screen.dart` - Connection screen with device discovery, IP `192.168.4.1`
- `dashboard/dashboard_screen.dart` - Dashboard with device info, firmware, uptime, memory, authentication
- `device_info/device_info_screen.dart` - Detailed device information
- `auth/auth_screen.dart` - Authentication status and login/logout
- `robot_eyes/robot_eyes_screen.dart` - Robot eyes with emotion and sleep controls (disabled where not supported)
- `settings/settings_screen.dart` - Local preferences only, no firmware persistence

---

## 2. Firmware API Endpoints (from `src/pk_api.cpp` + `include/pk_api.h`)

### Registered Endpoints (valid for app)

| Method | Endpoint | Response |
|--------|----------|----------|
| GET | `/api/device` | `{id, name, firmware, freeHeap, mac}` |
| GET | `/api/system` | `{device, uptimeSec, freeHeap, firmware, resetReason}` |
| GET | `/api/wifi/scan` | Scanning status/count |
| GET | `/api/auth/status` | `{authenticated, role, clientId, sessionSecondsLeft}` |
| POST | `/api/auth/login` | `{authenticated: true, role: "owner"}` |
| POST | `/api/auth/logout` | `{ok: true}` |
| POST | `/api/robot` | `{ok: true}` (with `action` parameter) |
| GET | `/api/device/camera/status` | `{state: "IDLE"}` |

### Endpoints NOT in firmware (removed/disabled from app)
- `/api/robot/emotion` - Use `/api/robot` with `action=emotion` POST
- `/api/robot/sleep` - Use `/api/robot` with `action=sleep` POST
- `/api/robot/animation` - Not supported by firmware - disabled in app
- `/api/device/camera/start|stop` - Only status available via `/api/device/camera/status`
- `/api/security/status` - Not in firmware
- `/api/security/audit` - Not in firmware
- `/api/security/devices` - Not in firmware
- `/api/settings` - Not in firmware
- `/api/control` - Not in firmware
- `/api/net/attack` - Attack endpoints (deauth, sniff, etc.) - disabled in app
- `/api/net/attack/start` - Not supported
- `/api/net/sniff` - Not in firmware
- `/api/evil` - Evil twin - disabled in app (security risk)
- `/api/wardrive` - Wardriving - disabled in app (security risk)

### API Contract Verification
- ✅ `GET /api/device` matches firmware
- ✅ `GET /api/system` supported by firmware
- ✅ `GET /api/auth/status` matches firmware
- ✅ `POST /api/auth/login` matches firmware
- ✅ `POST /api/auth/logout` matches firmware
- ✅ `POST /api/robot` with `action` parameter (NOT `GET /api/robot`)
- ✅ `GET /api/device/camera/status` matches firmware
- ✅ No `GET /api/robot` declared (firmware uses POST)
- ✅ Local traffic only: `http://192.168.4.1`

---

## 3. Resolved Issues

### 3.1 JSON Envelope Mismatch - CODE FIXED
- **Flutter (ApiClient):** `_handleResponse` now returns direct JSON without envelope check
- **Firmware:** Returns direct JSON without envelope
- **Status:** Code corrected in `api_client.dart`

### 3.2 Field Name Mismatches - CODE FIXED
- `freeHeap` field name corrected to match firmware (was already `freeHeap`, removed `freeMemory` confusion)
- All `DeviceModel` field mappings match firmware response fields

### 3.3 API Client Endpoints - CODE FIXED
- All API calls corrected to match firmware endpoints exactly
- Robot endpoints use `/api/robot` with `action` parameter via POST
- Camera endpoint: `/api/device/camera/status` instead of `/api/camera/status`
- Evil twin, wardriving, scanner endpoints disabled (return error messages)
- Animation endpoint disabled (not in firmware)

### 3.4 Feature Screens - CODE COMPLETED
- All 7 feature screens created under `mobile_app/lib/features/`:
  - onboarding, connection, dashboard, device_info, auth, robot_eyes, settings

### 3.5 Security - CODE COMPLETED
- Evil twin `/api/evil` endpoint disabled in app (returns error)
- Wardrive `/api/wardrive` endpoint disabled in app (returns error)
- Scanner endpoints disabled (return error messages)
- Animation endpoint disabled (returns error)
- No cloud/analytics/ads/WebView integration
- Local network only, no Internet required for basic function

### 3.6 Connection State Management
- Flutter uses `ConnectionStateModel` with `setState()` to track connection
- Firmware sets auth state via `/api/auth/login`/`/api/auth/logout`
- Real-time connection verification via `/api/auth/status`

### 3.7 API Client Error Handling - CODE FIXED
- Proper timeout handling (connection, send, receive)
- HTTP error handling (4xx, 5xx)
- JSON malformation handling
- Missing field handling - "Non disponibile" shown for non-exposed data

---

## 4. Required Changes (Code Completed)

### 4.1 API Client Fixes - CODE COMPLETED
1. ✅ Removed `success/data` envelope handling - firmware returns direct JSON
2. ✅ Fixed field names: `freeHeap` (already correct, clarified `freeMemory` confusion)
3. ✅ Fixed emotion/sleep endpoints to use `/api/robot` with `action` parameter via POST
4. ✅ Fixed camera endpoint: `/api/device/camera/status` instead of `/api/camera/status`
5. ✅ Removed/disabled endpoints not in firmware: evil twin, wardriving, scanner, animation

### 4.2 Feature Screens - CODE COMPLETED
- Created all 7 feature screens under `mobile_app/lib/features/`

### 4.3 Dart Code Fixes - CODE COMPLETED
1. ✅ Updated `DeviceModel.fromJson` to use `freeHeap` (already correct)
2. ✅ Updated all API calls to match firmware endpoints exactly
3. ✅ Added proper error handling for missing fields
4. ✅ Removed duplicate `features/` directory outside `lib/features/`
5. ✅ Consolidated `DeviceConfig` and `ApiException` inside `api_client.dart`

### 4.4 Security Hardening - CODE COMPLETED
- Evil twin, wardriving, scanner endpoints disabled with error messages
- No cloud/analytics, no external WebView
- Local traffic only: `http://192.168.4.1`
- Removed all insecure method references from public client

### 4.5 iOS/Android Platform Configuration - PENDING
- Android: platforms need `flutter pub get` and `flutter build apk`
- iOS: requires macOS/Xcode - not available in this environment
- Cleartext traffic exceptions configured only for `192.168.4.1`

### 4.6 README Update - CODE COMPLETED
- Updated `mobile_app/README.md` with real endpoints and limits

---

## 5. Summary of Code Changes (All Completed)

| Category | Change | Status |
|----------|--------|--------|
| API Envelope | `_handleResponse` returns direct JSON | ✅ Code fixed |
| Field Names | `freeHeap` confirmed matching firmware | ✅ Code fixed |
| Endpoint Mismatch | All endpoints corrected to POST where firmware uses POST | ✅ Code fixed |
| Feature Screens | All 7 under `lib/features/`, duplicate `features/` removed | ✅ Code fixed |
| Security | Insecure endpoints disabled with error messages | ✅ Code fixed |
| API Client | Dio init consolidated, DeviceConfig/ApiException inline | ✅ Code fixed |
| Router | GoRouter config with 7 valid routes | ✅ Code fixed |
| Tests | Fixed to import only real `lib/` files, no network dependencies | ✅ Code fixed |

---

## 6. Remaining Verification (Requires Flutter Environment)

1. Run `flutter pub get` to fetch dependencies
2. Run `flutter analyze` to check for errors
3. Run `flutter test` to execute tests
4. Run `flutter build apk --debug` for Android build
5. iOS build requires macOS/Xcode - not verifiable in this environment

### Known Issues Without Flutter Environment
- `flutter analyze` cannot be executed
- `flutter test` cannot be executed  
- `flutter build apk --debug` cannot be executed
- iOS build cannot be verified without macOS/Xcode

---

## 7. Endpoints Realmente Usati (Real Endpoints Used)

L'app comunica con il firmware PortaChiave tramite questi endpoint HTTP locali:

- `GET /api/device` - Informazioni dispositivo (nome, ID, firmware, memoria, uptime)
- `GET /api/system` - Stato sistema (uptime, heap gratuito, motivo reset)
- `GET /api/wifi/scan` - Scanning reti Wi-Fi
- `GET /api/auth/status` - Stato autenticazione
- `POST /api/auth/login` - Accesso con credenziali
- `POST /api/auth/logout` - Uscita
- `POST /api/robot` - Controllo occhi robotici (con parametro `action`)
- `GET /api/device/camera/status` - Stato fotocamera

Limite firmware: tutti i comandi `robot` usano `POST /api/robot` con parametro `action`, NON `GET /api/robot`.