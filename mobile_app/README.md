# PortaChiave Mobile App

PortaChiave Professional Trading App - Flutter Native iOS/Android

## Installation

1. Open in VS Code or Android Studio
2. Run `flutter pub get` to install dependencies
3. Ensure device is on same network as PortaChiave-XXXX Wi-Fi

## Getting Started

1. Connect phone to `PortaChiave-XXXX` Wi-Fi network
2. Open the app - it will detect the device automatically
3. Press "Connect to PortaChiave" to establish connection

## Structure

- `lib/main.dart` - Entry point
- `lib/app/app.dart` - App shell
- `lib/app/router.dart` - Navigation with GoRouter
- `lib/app/theme.dart` - Dark theme config

## Connection Flow

1. Connect phone to `PortaChiave-XXXX` Wi-Fi
2. Open app - shows onboarding
3. Press "Connect to PortaChiave"
4. App connects via `http://192.168.4.1`
5. Dashboard shows device info

## Features

- Connection status display
- Device info (name, ID, firmware, memory, uptime)
- Authentication (Guest/Owner)
- Dashboard with connection status
- RobotEyes controls

## Endpoints Used

- `GET /api/device` - Device info
- `GET /api/system` - System status
- `GET /api/auth/status` - Auth status
- `GET /api/auth/login` - Login
- `GET /api/auth/logout` - Logout
- `GET /api/robot` - Robot eyes

## Test Results

- `flutter analyze` - No errors
- `flutter test` - All tests pass
- Compilation verified on available platform

## Endpoints Used

- `GET /api/device` - Device info
- `GET /api/system` - System status
- `GET /api/auth/status` - Auth status
- `GET /api/auth/login` - Login
- `GET /api/auth/logout` - Logout
- `GET /api/robot` - Robot eyes
```

## Test Results

- `flutter analyze` - No errors
- `flutter test` - All tests pass
- Compilation verified on available platform

## Endpoints Used

- `GET /api/device` - Device info
- `GET /api/system` - System status
- `GET /api/auth/status` - Auth status
- `GET /api/auth/login` - Login
- `GET /api/auth/logout` - Logout
- `GET /api/robot` - Robot eyes

## Limits

- No Internet required / expected offline operation
- No WebView - native Flutter dashboard
- No external data sending
- HTTP access restricted to local network only
- No attack functions (deauth, evil twin, sniffing)
- No external data transmission