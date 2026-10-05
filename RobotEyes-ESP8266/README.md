# 👁️ ESP8266 Robot Eyes

[![Platform: ESP8266](https://img.shields.io/badge/Platform-ESP8266-blue.svg)](https://www.espressif.com/en/products/socs/esp8266)
[![Display: SSD1306](https://img.shields.io/badge/Display-SSD1306-important.svg)](https://www.adafruit.com/product/326)
[![FPS: 60](https://img.shields.io/badge/FPS-60-ff69b4.svg)]()
[![Emotions: 13](https://img.shields.io/badge/Emotions-13-ff69b4.svg)]()
[![Arduino Compatible](https://img.shields.io/badge/Arduino-Compatible-success.svg)]()
[![Made with ❤️](https://img.shields.io/badge/Made%20with-❤️-red.svg)]()
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)]()
[![Stars](https://img.shields.io/github/stars/yourusername/ESP8266-Robot-Eyes.svg?style=social)]()
[![Forks](https://img.shields.io/github/forks/yourusername/ESP8266-Robot-Eyes.svg?style=social)]()

---

## 🤯 Mind-blowing robot eyes for ESP8266! 13 emotions, 60 FPS smoothness, independent pupil/eyelid control, natural blinking, breathing, idle gaze & sleep/wake on SSD1306 OLED. 🔥

---

<p align="center">
  <img src="docs/demo.gif" alt="Robot Eyes Demo" width="600">
  <br>
  <em>Commercial-grade robot eye animations running at 60 FPS on ESP8266!</em>
</p>

---

## ✨ **What Makes This Special?**

Transform your ESP8266 robot from a simple machine into an expressive companion with this high-performance eye animation system. Perfect for humanoid robots, animatronics, desktop companions, interactive art, and STEM projects.

### 🎯 **Key Highlights**

| Feature | Description |
|---------|-------------|
| ⚡ **60 FPS** | Buttery-smooth animations on ESP8266 |
| 🎭 **13 Emotions** | Expressive range from Happy to Curious |
| 👁️ **Independent Control** | Each pupil & eyelid moves separately |
| 💨 **Natural Blinking** | 4-8 second intervals with double-blink |
| 🌬️ **Breathing Effect** | Subtle eye size variation |
| 👀 **Idle Gaze** | Random eye movements for lifelike feel |
| 😴 **Sleep/Wake Modes** | Power-saving with gentle animations |
| 🚀 **Ultra-Optimized** | Runs on 80KB RAM ESP8266 |

---

## 🎭 **13 Emotions**

| # | Emotion | Visual Description |
|---|---------|-------------------|
| 1 | 😐 **Neutral** | Calm, centered, balanced gaze |
| 2 | 😊 **Happy** | Bright, squinted, upward curves |
| 3 | 😠 **Angry** | Focused, narrowed, downward brows |
| 4 | 😢 **Sad** | Drooping eyelids, downward gaze |
| 5 | 😴 **Sleepy** | Heavy-lidded, half-closed eyes |
| 6 | 😮 **Surprised** | Wide open, large eyes, small pupils |
| 7 | 🤔 **Thinking** | Contemplative, looking up/away |
| 8 | 😏 **Curious** | Inquisitive, side gaze with tilt |
| 9 | 😨 **Scared** | Wide eyes, tiny pupils, tense |
| 10 | ❤️ **Love** | Large pupils, soft, warm gaze |
| 11 | 😂 **Laughing** | Joyful squint, upward curves |
| 12 | 😉 **Wink Left** | Playful left eye wink |
| 13 | 😉 **Wink Right** | Playful right eye wink |

---

## 🎬 **Animation Features**

### Core Animations
- ✅ **60 FPS** rendering for cinematic smoothness
- ✅ **Easing functions** (cubic, elastic, bounce, back)
- ✅ **Independent pupil movement** (cross-eyed, divergent)
- ✅ **Independent eyelid control** (wink, squint, blink)

### Automatic Behaviors
- ✅ **Random blinking** (4-8 second intervals)
- ✅ **Double blink** for expressive emphasis
- ✅ **Breathing effect** (subtle size variation)
- ✅ **Idle gaze shifts** (random eye movement)
- ✅ **Sleep mode** (with gentle twitching)
- ✅ **Wake-up animation** (gradual opening)

### Professional Sequences
- ✅ **Startup animation** (cinematic boot sequence)
- ✅ **Shutdown animation** (graceful power-down)
- ✅ **Smooth emotion transitions** (300ms crossfade)

---

## 🛠️ **Hardware Requirements**
- ✅ **𝐄𝐒𝐏𝟖𝟐𝟔𝟔 𝐍𝐨𝐝𝐞𝐌𝐂𝐔**
- ✅ **𝟎.𝟗𝟔" 𝐎𝐋𝐄𝐃 𝐝𝐢𝐬𝐩𝐥𝐚𝐲**

### Pin Connections

| Component | NodeMCU Pin | GPIO | Function |
|-----------|-------------|------|----------|
| OLED VCC  | 3V3 | - | Power (3.3V) |
| OLED GND  | GND | - | Ground |
| OLED SDA  | D6 | GPIO12 | I2C Data |
| OLED SCL  | D5 | GPIO14 | I2C Clock |

---

## 📦 **Installation**

### 1️⃣ Install Required Libraries
```bash
# In Arduino IDE → Sketch → Include Library → Manage Libraries
# Search and install:
✅ Adafruit GFX Library
✅ Adafruit SSD1306
```
