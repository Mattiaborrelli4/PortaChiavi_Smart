# PortaChiave Project - Detailed Documentation

## Overview
PortaChiave is an ESP8266-based device that creates a WiFi Access Point, serves a web dashboard, and displays animations on an SSD1306 OLED display. The device can show the currently playing song from a connected client (like Spotify).

## Hardware
- **ESP8266** (NodeMCU or similar)
- **SSD1306 128x64 I2C OLED display**
- WiFi Access Point mode

## Core Modules

### 1. main.cpp
Main entry point that handles:
- OLED initialization
- WiFi Access Point setup
- API server startup
- RobotEyes (display) initialization
- Main loop managing display surfaces and animations

**Key functions:**
- `setup()`: Initializes hardware, WiFi AP, API server, and displays
- `loop()`: Core processing loop that:
  - Updates camera pairing state
  - Updates animations manager
  - Manages display surface priorities (QR > WIFI_SCAN > LAG > REACTION > EYES)
  - Handles song display when EYES surface is active
  - Handles client API requests
  - Periodic heap status logging

### 2. pk_api.cpp
Web API server that handles all HTTP endpoints:

**Main endpoints:**
- `/api/device` - Device info (ID, name, firmware, chip ID, auth status)
- `/api/wifi/status` - WiFi AP status
- `/api/system` - System uptime, free heap, robot FPS
- `/api/values` - Value store data
- `/api/auth/status` - Authentication status
- `/api/auth/login` - Login (dev mode or real token)
- `/api/auth/logout` - Logout
- `/api/wifi/scan` - WiFi scan (owner only)
- `/api/net/scan|sniff|attack` - Network tools (owner only)
- `/api/evil` - Evil twin functionality (owner only)
- `/api/wardrive` - Wardriving (owner only)
- `/api/modules` - Module permissions
- `/api/security/status|audit|devices` - Security center (owner only)
- `/api/robot` - Robot eyes control (auth required)
- `/api/device/camera/start|stop|status` - Camera pairing
- `/api/device/camera/signal` - Camera state signaling from smartphone
- `/api/song` - **NEW**: Receive song name from client (POST with JSON `{"song": "Song Title"}`)
- `/api/control` - Send commands to robot eyes
- `/api/settings` - Device settings

**Authentication:**
- Global `_authenticated` flag + per-client role system
- `ownerGate()` checks role before allowing actions
- Roles: OWNER, GUEST, none
- Dev mode (`AUTH_DEV_MODE`) allows `{mode:'dev'}` login without verification

### 3. RobotEyes (src/RobotEyes.cpp + include/RobotEyes.h)
Library for displaying animated eyes on the OLED:

**Emotions/States:**
- EMOTION_NEUTRAL, HAPPY, ANGRY, SAD, SLEEPY, SURPRISED, CURIOUS, SCARED
- LOVE, LAUGHING, THINKING, WINK_LEFT, WINK_RIGHT
- SUSPICIOUS, FOCUSED, ALERT, CONFUSED, SCANNING, LAG

**Key functions:**
- `setEmotion()` - Change emotion immediately
- `setEmotionWithTransition()` - Smooth transition between emotions
- `playBlink()` - Natural blink animation
- `playDoubleBlink()` - Double blink
- `playWinkLeft()/playWinkRight()` - Wink one eye
- `playLag()` - Glitch/lag effect
- `setSleepMode()` - Sleep mode with Z indicator
- `setEmotion()` dispatches events via `dispatchEvent()`

**Events:**
- `EYE_EVENT_WIFI_SCAN_START` → scanning emotion
- `EYE_EVENT_QR_SCANNED` → surprised
- `EYE_EVENT_CAMERA_CONNECTED` → happy
- `EYE_EVENT_CAMERA_DISCONNECTED`/`EYE_EVENT_ERROR` → confused

### 4. Animations (src/animations.cpp + include/animations.h)
Manages fullscreen animations with priority system:

**Surfaces (priority order):**
- QR (camera pairing - highest priority)
- WIFI_SCAN - WiFi scanning animation
- LAG - Glitch/glitch effect
- REACTION - Quick reaction to phone commands
- EYES - Default RobotEyes display

**Key functions:**
- `begin()` - Initialize with display pointer
- `requestWifiScan()` - Start scan animation
- `completeWifiScan(n)` - Complete scan with n networks found
- `requestLag()` - Start lag effect
- `requestReaction()` - Start reaction animation
- `currentSurface()` - Return current surface
- `isSpecial()` - Is something other than EYES displayed?
- `update()` - Advance animation state
- `render()` - Draw current surface

### 5. pk_camera.cpp
Handles QR code pairing for camera:

**Key functions:**
- `begin()` - Initialize with display
- `start()` - Start pairing session, generate QR
- `stop()` - Stop pairing session
- `update()` - Update session state
- `render()` - Render QR code on display
- `active()` - Is a session active?
- `statusJson()` - Status JSON
- `signalState()` - Signal state from smartphone

### 6. wifi_ap.cpp
WiFi Access Point management:

**Key functions:**
- `begin()` - Start AP with SSID "PortaChiave-XXXX"
- `hasClient()` - Check if any client connected
- `clientCount()` - Number of connected clients
- `clientsJson()` - JSON of connected clients
- `firstClientMac()` - MAC of first connected client
- `statusJson()` - Full status JSON

### 7. Device info (src/device.cpp + include/device.h)
- `id()` - "PK-" + last 6 chars of MAC address
- `name()` - "PortaChiave"
- `firmware()` - Version string
- `chipId()` - ESP chip ID in hex
- `macSuffix()` - Last 4 chars of MAC

## Communication Flow

### 1. Device Powers On
1. ESP8266 boots, initializes I2C
2. Scans for OLED at 0x3C
3. If OLED found, initializes display
4. Starts WiFi Access Point: "PortaChiave-XXXX"
5. Starts API web server
6. Initializes RobotEyes animation library
7. Starts animations manager

### 2. Client Connects to AP
1. Client (phone/laptop) connects to "PortaChiave-XXXX"
2. Client accesses http://192.168.4.1
3. Dashboard page loads

### 3. Sending Song Name from Client (Spotify)
1. Client app sends POST request to `http://192.168.4.1/api/song`
2. Body: JSON `{"song": "Song Title - Artist"}`
3. API endpoint `handleApiSong()` receives the request
4. Stores song name in `currentSong` buffer (max 31 chars)
5. Updates `songLastUpdate` timestamp
6. Returns robot state JSON

### 4. Displaying Song on OLED
1. Main loop checks `animations::currentSurface()`
2. If surface is `EYES` (no special animation active):
   - Draws "Now playing: [song]" at bottom of screen
   - Uses text size 1, cursor at (0, 48)
3. If other surfaces are active (QR, scan, lag, reaction):
   - Song display is suppressed (animation has priority)
   - Display shows the animation instead

### 5. API Endpoint Details: /api/song
**Method:** POST  
**Body:** `{"song": "Song Title"}`  
**Max length:** 31 characters  
**Response:** JSON with current robot state  
**Effect:** Updates the song displayed on the OLED when EYES surface is active

## Priority System
The display surface priority determines what's shown on the OLED:
1. **QR** - Camera pairing (highest, takes full screen)
2. **WIFI_SCAN** - WiFi scanning animation
3. **LAG** - Glitch/lag effect
4. **REACTION** - Quick reaction animation
5. **EYES** - Default RobotEyes (song displayed here)

When a higher-priority surface is active, the song display is suppressed.

## Adding New Features

### Adding a New API Endpoint
1. Add function in `pk_api.cpp` following the pattern of `handleApiSong()`
2. Register route in `begin()` function
3. Return appropriate JSON response

### Adding a New Animation Surface
1. Add new `Surface` enum value in `animations.h`
2. Add request function (e.g., `requestNewSurface()`)
3. Handle in `update()` and `render()` functions
4. Update `currentSurface()` logic

### Changing Display Priority
1. Modify `currentSurface()` in `animations.cpp`
2. Update the `if/else if` chain in `main.cpp` loop()
3. Adjust priority order in comments and logic

## Troubleshooting

### Song Not Displaying
1. Check if client is sending to `/api/song` endpoint
2. Verify song name is ≤ 31 characters
3. Ensure no higher-priority animation is active (QR, scan, etc.)
4. Check OLED is initialized (`displayOk` flag)

### API Not Responding
1. Verify client is connected to AP
2. Check correct endpoint URL
3. Ensure proper JSON format
4. Check authentication requirements (some endpoints need owner role)

### Display Crashes
1. Ensure `displayOk` is true before touching OLED
2. Check OLED address (default 0x3C)
3. Verify display initialization in setup()
4. Watch for buffer conflicts between animations and song display