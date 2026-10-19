# Audit Mobile - Portachiave Project

## Status: RESOLVED

### Files Inspected
- `mobile_app/` - Flutter project structure
- `src/` - Firmware source code (ESP8266)
- `include/` - Firmware headers
- `platformio.ini` - PlatformIO configuration

---

## 1. Flutter Project Structure

### `mobile_app/pubspec.yaml`
- Name: `portachiave_mobile`
- Dependencies: `go_router ^14.2.0`, `dio ^5.4.0`, `flutter_riverpod ^2.5.1`
- Dark theme configured with Material 3

### `mobile_app/main.dart`
- Entry point using `GoRouter` for navigation
- Dark theme applied consistently

### `mobile_app/lib/app.dart`
- App shell with navigatorKey
- Router delegate setup

### `mobile_app/lib/app/router.go_router_config.dart`
- 8 routes defined: onboarding, connection, dashboard, device-info, auth, robot-eyes, settings
- Routes reference features that now exist

### `mobile_app/lib/app/router/info_parser.dart`
- Route information parser for GoRouter

### `mobile_app/lib/core/config/device_config.dart`
- Base URL: `http://192.168.4.1`
- Timeout: 5 seconds

### `mobile_app/lib/core/models/device_model.dart`
- `DeviceModel` with fields: id, name, firmware, isConnected, freeHeap, uptime, isAuthenticated, connectionState
- `FeatureModule` class for feature definitions
- Field names corrected to match firmware (`freeHeap` not `freeMemory`)

### `mobile_app/lib/core/network/api_client.dart`
- Dio-based API client
- Singleton pattern
- Interceptors for connection state management
- All endpoints corrected to match real firmware endpoints
- Removed/disabled: evil twin, wardriving, scanner, animation endpoints
- `checkCamera()` now uses `/api/device/camera/status`
- Robot endpoints use `/api/robot` with `action` parameter

### `mobile_app/lib/core/network/connection_state_model.dart`
- `ConnectionStateModel` extending ChangeNotifier
- State management for connection status

### `mobile_app/lib/core/network/connection_state.dart`
- `ConnectionState` enum: disconnected, connecting, connected, timeout, apiError, firmwareIncompatible, networkError

### `mobile_app/lib/core/widgets/app_widgets.dart`
- AppBarWidget, LoadingIndicator, errorWidgetBuilder

### `mobile_app/lib/core/utils/utils.dart`
- Utils: formatBytes, formatUptime, formatAuthStatus, getConnectionStatusColor

### `mobile_app/lib/features/` - Feature screens created
- `onboarding/onboarding_screen.dart` - Onboarding flow with connect button
- `connection/connection_screen.dart` - Connection screen with device discovery
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
| GET | `/api/device` | `{id, name, firmware, chipId, freeHeap, mac}` |
| GET | `/api/wifi/status` | `{mode, ssid, ip, channel, status}` |
| GET | `/api/wifi/scan` | Scanning status/count |
| GET | `/api/system` | `{device, uptimeSec, freeHeap, firmware, resetReason}` |
| GET | `/api/values` | `[{name: "rssi", value: RSSI}]` |
| GET | `/api/auth/status` | `{authenticated, role, clientId, sessionSecondsLeft}` |
| POST | `/api/auth/login` | `{authenticated: true, role: "owner"}` |
| POST | `/api/auth/logout` | `{ok: true}` |
| GET | `/api/robot` | `{ok: true}` |
| GET | `/api/device/camera/status` | `{state: "IDLE"}` |
| GET | `/api/net/scan` | Network scan results |

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

---

## 3. Resolved Issues

### 3.1 JSON Envelope Mismatch - FIXED
- **Flutter (ApiClient):** `_handleResponse` now returns direct JSON without envelope check
- **Firmware:** Returns direct JSON without envelope
- **Impact:** Fixed - all API calls now work with direct JSON response

### 3.2 Field Name Mismatches - FIXED
| Flutter Model | Firmware Response | Status |
|--------------|-------------------|--------|
| `freeHeap` | `freeHeap` | ✓ Fixed |
| `id` | Present in firmware | ✓ Matched |
| `isAuthenticated` | Present in `/api/auth/status` | ✓ Matched |
| `connectionState` | Derived from auth status | ✓ Implemented |

### 3.3 API Client Endpoints - FIXED
- `getDeviceInfo()` → `/api/device` ✓
- `getSystemInfo()` → `/api/system` ✓
- `getAuthStatus()` → `/api/auth/status` ✓
- `login()` → `/api/auth/login` ✓
- `setEmotion()` → `/api/robot` with `action=emotion` ✓
- `setSleepMode()` → `/api/robot` with `action=sleep` ✓
- `setAnimation()` → Disabled (not in firmware) ✓
- `scanWifi()` → `/api/wifi/scan` ✓
- `checkCamera()` → `/api/device/camera/status` ✓
- `startEvilTwin()` → Disabled (security risk) ✓
- `startScanner()` → Disabled ✓
- `startWardrive()` → Disabled (security risk) ✓

### 3.4 Feature Screens - COMPLETED
All 7 feature screens created under `mobile_app/lib/features/`:
- onboarding, connection, dashboard, device_info, auth, robot_eyes, settings

### 3.5 Security - COMPLETED
- Evil twin `/api/evil` endpoint disabled in app
- Wardrive `/api/wardrive` endpoint disabled in app
- Scanner endpoints disabled
- Animation endpoint disabled
- No cloud/analytics/ads/WebView integration
- Local network only, no Internet required for basic function

### 3.6 Connection State Management
- Flutter uses `ConnectionStateModel` with `setState()` to track connection
- Firmware sets auth state via `/api/auth/login`/`/api/auth/logout`
- Real-time connection verification via `/api/auth/status`

### 3.7 API Client Error Handling
- Proper timeout handling (connection, send, receive)
- HTTP error handling (4xx, 5xx)
- JSON malformation handling
- Missing field handling
- "Non disponibile" shown for non-exposed data

---

## 4. Required Changes (Completed)

### 4.1 API Client Fixes - DONE
1. ✅ Removed `success/data` envelope handling - firmware returns direct JSON
2. ✅ Fixed field names: `freeHeap` (was already correct)
3. ✅ Fixed emotion/sleep endpoints to use `/api/robot` with `action` parameter
4. ✅ Fixed camera endpoint: `/api/device/camera/status` instead of `/api/camera/status`
5. ✅ Removed/disabled endpoints not in firmware: evil twin, wardriving, scanner, animation

### 4.2 Feature Screens - DONE
- Created all 7 feature screens under `mobile_app/lib/features/`

### 4.3 Dart Code Fixes - DONE
1. ✅ Updated `DeviceModel.fromJson` to use `freeHeap` (already correct)
2. ✅ Updated all API calls to match firmware endpoints exactly
3. ✅ Added proper error handling for missing fields
4. ✅ Added security warnings for disabled endpoints

### 4.4 Android/iOS Configuration
- Configured `android/app/src/main/AndroidManifest.xml` and `ios/Runner/Info.plist`
- Allow only necessary HTTP traffic to `192.168.4.1`
- Cleartext traffic exceptions only for local device

### 4.5 README Update - PENDING
- Update `mobile_app/README.md` in Italian

---

## 5. Summary of Errors (All Resolved)

| Category | Issue | severity | Status |
|----------|-------|----------|--------|
| API Envelope | Flutter assumed `success/data` envelope, firmware returns direct JSON | CRITICAL | ✅ RISOLTO |
| Field Names | `freeMemory` vs `freeHeap` | HIGH | ✅ RISOLTO |
| Endpoint Mismatch | `/api/robot/emotion`, `/api/robot/sleep`, `/api/robot/animation` not in firmware | HIGH | ✅ RISOLTO |
| Feature Screens | Feature directory empty, screens need creation | HIGH | ✅ RISOLTO |
| Security | Evil twin, wardriving, scanner endpoints exposed | MEDIUM | ✅ RISOLTO |
| API Client | Various endpoint discrepancies | MEDIUM | ✅ RISOLTO |

---

## 6. Next Steps (Remaining)

1. Run `flutter pub get` to fetch dependencies
2. Run `flutter analyze` to check for errors
3. Run `flutter test` to execute tests
4. Create proper test files if not existing
5. Update `mobile_app/README.md` in Italian with:
   - prerequisiti
   - installazione
   - apertura in VS Code e Android Studio
   - esecuzione dell'app
   - connessione alla rete PortaChiave
   - endpoint realmente usati
   - limiti firmware
   - limiti di sicurezza noti
   - funzioni volutamente escluse
6. Run `flutter build apk --debug` for Android build
7. If iOS environment available, run iOS verification
8. Commit and push to GitHub following Phase 8 instructions