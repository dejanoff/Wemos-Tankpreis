# ⛽ Wemos D1 Mini — Tankpreis & Digital Clock

[![Platform: ESP8266](https://img.shields.io/badge/Platform-ESP8266-orange.svg)](https://en.wikipedia.org/wiki/ESP8266)
[![Framework: Arduino](https://img.shields.io/badge/Framework-Arduino-blue.svg)](https://www.arduino.cc/)
[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
[![API: Tankerkoenig](https://img.shields.io/badge/API-Tankerk%C3%B6nig-yellow.svg)](https://creativecommons.tankerkoenig.de/)

Smart desktop fuel price ticker and digital clock powered by **Wemos D1 Mini (ESP8266)** and a 4-digit 7-segment LED display (**TM1637**).

Fetches real-time fuel prices (Super E5, Super E10, Diesel) in Germany using the official **Tankerkönig** API (MTS-K), alternates them with accurate NTP network time, smoothly fades brightness, animates fuel names, and provides a modern responsive mobile web UI for effortless configuration.

---

[🌐 Deutsch](README_DE.md) | [🌐 Русский](README.md)

---

## 🌟 Key Features

### 🕒 Clock & Display
- **NTP Time Synchronization**: Automatic sync with `pool.ntp.org` / `time.google.com`, automatic Daylight Saving Time (Berlin/Germany `CET/CEST`).
- **Blinking Colon**: 1 Hz central colon (`:`).
- **Multi-Favorite Alternation**: Activate multiple stations or fuel types simultaneously (e.g. Diesel + Super E5). The display smoothly cycles through:
  - Accurate time with blinking colon (e.g. 20s)
  - Scrolling fuel name (`dIESEL`) → Price (e.g. 10s)
  - Scrolling fuel name (`bEnZIn E5`) → Price (e.g. 10s)
  - Smooth fade back to clock.
- **Smooth Brightness Transitions**: Gentle **Fade In / Fade Out** between screens.
- **Price Trend Indicator**: Displays `UP` (price rose) or `dn` (price dropped) when updated.
- **Station Closed Indicator**: Displays `CLOS` if the station is currently closed.
- **Night Mode**: Configurable schedule (e.g. 23:00 to 07:00) with dimmed display or complete display shutoff (`0` = display off).

### 🏷️ Customizable Price Formats
Choose between 4 price rendering formats on the 4-digit display:
1. `4-stellig exakt`: Full 4 digits without separator (e.g. `2379` for 2.379 €).
2. `Gerundet mit Unterstrich`: Rounded to cents with underscore (e.g. `2_38`).
3. `Gerundet mit Mittelstrich`: Rounded to cents with middle dash (e.g. `2-38`).
4. `Gerundet ohne Strich`: Rounded to cents with space (e.g. `2 38`).

### 📶 Multi-WiFi Manager
- **Stores up to 5 WiFi networks**: Remembers home, office, or mobile hotspot; auto-connects to the strongest available network.
- **Manual Network Addition**: Add SSID and password in advance, even if the network is currently out of range.
- **Sequential IP Display on Boot**: Displays `IP  ` followed by each of the 4 octets (`192 ` → `168 ` → `178 ` → ` 45 `).
- **Captive Portal**: Starts `Tankpreis-Setup` AP if no known WiFi is reachable.

### 📱 Responsive Web Interface
- Fully optimized for mobile screens (iOS/Android) and desktop browsers.
- **Station Search**:
  - By German postal code (**PLZ**) with search radius.
  - By GPS coordinates (**Latitude / Longitude**) with "📍 Mein Standort" IP/GPS auto-detection.
  - By station **UUID**.
- **Multi-Favorites Management**: Toggle any favorite in/out of the display rotation (`[✅ Aktiv]` / `[▶ Aktivieren]`). Real-time active price overview cards in the header.
- **System Controls**: One-click **Reboot** and factory **Reset** (formats LittleFS).

---

## 🛠️ Hardware Wiring & Pinout

| TM1637 Display | Wemos D1 Mini Pin | ESP8266 GPIO | Description |
| :--- | :--- | :--- | :--- |
| **CLK** | **D3** | GPIO0 | Clock signal |
| **DIO** | **D4** | GPIO2 | Data signal |
| **VCC** | **5V** (or 3V3) | 5V / 3.3V | Power supply |
| **GND** | **GND** | GND | Ground |

```
    +-------------------+
    |   Wemos D1 Mini   |
    |                   |
    |         D3 (GPIO0)--------> CLK [ TM1637 ]
    |         D4 (GPIO2)--------> DIO [ Display ]
    |                5V --------> VCC
    |               GND --------> GND
    +-------------------+
```

---

## 💻 Build & Flashing

### Arduino IDE
1. Install **ESP8266 board package** (v3.1.2 or newer) in Boards Manager.
2. Install libraries via Library Manager:
   - `TM1637Display` by Avishay Orpaz
   - `ArduinoJson` by Benoît Blanchon (v7.x)
3. Board settings:
   - **Board**: `LOLIN(WEMOS) D1 R2 & mini`
   - **Flash Size**: `4MB (FS:2MB OTA:~1019KB)`
   - **Upload Speed**: `460800` (or `921600`)

### PlatformIO
Clone and build with a single command:
```bash
pio run -t upload
```

---

## 🚀 Quick Setup Guide

1. Power on the Wemos. If not connected, it creates a Wi-Fi hotspot: **`Tankpreis-Setup`**.
2. Connect from your phone or PC and open `http://192.168.4.1/`.
3. In the **WiFi** tab, select or enter your home Wi-Fi credentials and click **Verbinden**.
4. The display will show the assigned IP address. Open this IP in your web browser.
5. In the **API** tab, enter your free API key from [creativecommons.tankerkoenig.de](https://creativecommons.tankerkoenig.de).
6. In the **Suche** tab, find stations by your postal code (PLZ) and add desired fuels to favorites.
7. In the **Favoriten** tab, activate one or multiple fuels to cycle through.
8. In the **Setup** tab, configure cycle duration (e.g. 20s clock, 10s price), night mode, and price format.

---

## 📂 Project Structure

```
Wemos-Tankpreis/
├── .github/
│   └── ISSUE_TEMPLATE/
│       ├── bug_report.md       # GitHub bug report template
│       └── feature_request.md  # GitHub feature proposal template
├── .gitignore                  # Git ignore rules (binaries, build artifacts, IDEs)
├── LICENSE                     # MIT License
├── platformio.ini              # PlatformIO (VS Code) project configuration
├── Wemos-Tankpreis.ino         # Main sketch: state machine, NTP and API loop
├── config.h                    # AppConfig data structures, pin assignments, limits
├── display_manager.h / .cpp    # TM1637 driver: clock, prices, ticker, trends, IP, fade
├── wifi_manager.h / .cpp       # Multi-WiFi management, captive portal, scanning animation
├── api_client.h / .cpp         # Tankerkönig HTTPS API client (PLZ/GPS/UUID search & prices)
├── storage.h / .cpp            # LittleFS persistent storage (JSON)
├── web_server.h / .cpp         # REST API endpoints for configuration web UI
├── web_ui.h                    # Responsive mobile web UI (HTML/CSS/JS in PROGMEM)
├── README.md                   # Documentation (Russian)
├── README_DE.md                # Dokumentation (Deutsch)
└── README_EN.md                # Documentation (English)
```

---

## 📜 License & Credits

- Fuel price data provided by [Tankerkönig](https://creativecommons.tankerkoenig.de) under [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/).
- Source code released under the [MIT License](LICENSE).

