# ⛽ Wemos D1 Mini — Tankpreis & Digitaluhr

[![Platform: ESP8266](https://img.shields.io/badge/Platform-ESP8266-orange.svg)](https://en.wikipedia.org/wiki/ESP8266)
[![Framework: Arduino](https://img.shields.io/badge/Framework-Arduino-blue.svg)](https://www.arduino.cc/)
[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
[![API: Tankerkoenig](https://img.shields.io/badge/API-Tankerk%C3%B6nig-yellow.svg)](https://creativecommons.tankerkoenig.de/)

Smarte Schreibtisch-Tankpreis-Anzeige und Digitaluhr auf Basis des **Wemos D1 Mini (ESP8266)** und eines 4-stelligen 7-Segment-LED-Displays (**TM1637**).

Ruft aktuelle Kraftstoffpreise (Super E5, Super E10, Diesel) in Deutschland in Echtzeit über die offizielle **Tankerkönig-API** (MTS-K) ab, wechselt sanft mit einer präzisen NTP-Netzwerkuhr ab, dimmt die Helligkeit mit weichen Überblendungen, animiert Kraftstoffnamen und bietet eine moderne mobile Weboberfläche zur bequemen Konfiguration.

---

[🌐 English](README_EN.md) | [🌐 Русский](README.md)

---

## 🌟 Hauptfunktionen

### 🕒 Uhr & Display
- **NTP-Zeitsynchronisation**: Automatischer Zeitabgleich (`pool.ntp.org` / `time.google.com`) mit automatischer Sommer-/Winterzeit-Umstellung (Zeitzone Berlin `CET/CEST`).
- **Blinkender Doppelpunkt**: 1-Hz-Doppelpunkt in der Displaymitte (`:`).
- **Mehrere Favoriten gleichzeitig im Wechsel**: Wähle mehrere Stationen oder Kraftstoffarten (z.B. Diesel und Super E5). Das Display wechselt nacheinander:
  - Genaue Uhrzeit mit blinkendem Doppelpunkt (z.B. 20s)
  - Lauftext der ersten Kraftstoffsorte (`dIESEL`) → Preis (z.B. 10s)
  - Lauftext der zweiten Kraftstoffsorte (`bEnZIn E5`) → Preis (z.B. 10s)
  - Sanftes Ausblenden und Rückkehr zur Uhr.
- **Sanfte Helligkeitsübergänge (Fade In / Fade Out)**: Weicher Helligkeitswechsel beim Umschalten der Modi.
- **Preistrend-Anzeige**: Zeigt bei Preisänderungen kurzzeitig `UP` (gestiegen) oder `dn` (gesunken).
- **Geschlossen-Anzeige**: Zeigt `CLOS`, falls die Tankstelle gerade geschlossen ist.
- **Nachtmodus (Night Mode)**: Einstellbarer Zeitraum (z.B. 23:00 bis 07:00 Uhr) mit reduzierter Helligkeit oder komplett abgeschaltetem Display (`0` = Aus).

### 🏷️ 4 Anzeige-Formate für Preise
1. `4-stellig exakt`: Volle 4 Ziffern ohne Trennzeichen (z.B. `2379` für 2,379 €).
2. `Gerundet mit Unterstrich`: Auf Cent gerundet mit Unterstrich (z.B. `2_38`).
3. `Gerundet mit Mittelstrich`: Auf Cent gerundet mit Mittelstrich (z.B. `2-38`).
4. `Gerundet ohne Strich`: Auf Cent gerundet mit Leerzeichen (z.B. `2 38`).

### 📶 Multi-WiFi Manager
- **Bis zu 5 WLAN-Netzwerke speichern**: Speichert Heimnetz, Büro oder Smartphone-Hotspot und verbindet sich automatisch mit dem stärksten Netz.
- **Manuelles Hinzufügen**: SSIDs und Passwörter können vorab hinterlegt werden, auch wenn das Netz gerade nicht in Reichweite ist.
- **IP-Adresse beim Start**: Nach dem Verbinden zeigt das Display `IP  ` und nacheinander alle 4 Oktette (`192 ` → `168 ` → `178 ` → ` 45 `).
- **Captive Portal**: Öffnet den Hotspot `Tankpreis-Setup`, wenn kein bekanntes WLAN erreichbar ist.

### 📱 Responsive Web-Interface
- Perfekt optimiert für Smartphone-Browser (iOS / Android) und Desktop.
- **Tankstellensuche**:
  - Nach deutscher Postleitzahl (**PLZ**) mit Umkreis in km.
  - Nach GPS-Koordinaten (**Breitengrad / Längengrad**) inkl. Standort-Taste «📍 Mein Standort».
  - Nach Tankstellen-**UUID**.
- **Multi-Favoriten-Verwaltung**: Beliebige Favoriten per Klick aktivieren/deaktivieren (`[✅ Aktiv (im Wechsel)]` / `[▶ Aktivieren]`). Übersichtskarten mit Live-Preisen in der Kopfzeile.
- **System-Tools**: Schneller **Neustart** und vollständiger **Reset** (formatiert LittleFS).

---

## 🛠️ Hardware-Verdrahtung (Pinbelegung)

| TM1637 Display | Wemos D1 Mini Pin | ESP8266 GPIO | Funktion |
| :--- | :--- | :--- | :--- |
| **CLK** | **D3** | GPIO0 | Taktsignal Display |
| **DIO** | **D4** | GPIO2 | Datensignal Display |
| **VCC** | **5V** (oder 3V3) | 5V / 3.3V | Stromversorgung |
| **GND** | **GND** | GND | Masse |

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

## 💻 Kompilieren & Flashen

### Arduino IDE
1. **ESP8266 Board-Paket** (v3.1.2 oder neuer) im Boardverwalter installieren.
2. Bibliotheken über den Bibliotheksverwalter installieren:
   - `TM1637Display` von Avishay Orpaz
   - `ArduinoJson` von Benoît Blanchon (Version 7.x)
3. Board-Einstellungen:
   - **Board**: `LOLIN(WEMOS) D1 R2 & mini`
   - **Flash Size**: `4MB (FS:2MB OTA:~1019KB)`
   - **Upload Speed**: `460800`

### PlatformIO
```bash
pio run -t upload
```

---

## 🚀 Erste Schritte

1. Wemos mit Strom versorgen. Wenn kein WLAN bekannt ist, startet das Netzwerk **`Tankpreis-Setup`**.
2. Verbinde dich mit dem WLAN und öffne `http://192.168.4.1/`.
3. Im Reiter **WiFi** dein Heim-WLAN auswählen, Passwort eingeben und auf **Verbinden** tippen.
4. Nach dem Neustart zeigt das Display die IP-Adresse. Diese IP im Browser öffnen.
5. Im Reiter **API** den kostenlosen API-Key von [creativecommons.tankerkoenig.de](https://creativecommons.tankerkoenig.de) eintragen.
6. Im Reiter **Suche** nach PLZ suchen und gewünschte Kraftstoffe zu Favoriten hinzufügen.
7. Im Reiter **Favoriten** gewünschte Kraftstoffe aktivieren.
8. Im Reiter **Setup** Wechselzeiten (z.B. 20s Uhr, 10s Preis), Nachtmodus und Preisformat einstellen.

---

## 📂 Projektstruktur

```
Wemos-Tankpreis/
├── .github/
│   └── ISSUE_TEMPLATE/
│       ├── bug_report.md       # GitHub Fehlerbericht-Vorlage
│       └── feature_request.md  # GitHub Feature-Vorschlag-Vorlage
├── .gitignore                  # Git-Ausschlussregeln (Binaries, Build-Dateien, IDEs)
├── LICENSE                     # MIT-Lizenz
├── platformio.ini              # PlatformIO-Projektkonfiguration (VS Code)
├── Wemos-Tankpreis.ino         # Haupt-Sketch: Zustandsautomat, NTP- und API-Loop
├── config.h                    # AppConfig Datenstrukturen, Pin-Belegung, Grenzwerte
├── display_manager.h / .cpp    # TM1637 Treiber: Uhrzeit, Preise, Laufschrift, Trends, IP, Fade
├── wifi_manager.h / .cpp       # Multi-WLAN Manager, Captive Portal, Scan-Animation
├── api_client.h / .cpp         # Tankerkönig HTTPS API Client (PLZ/GPS/UUID & Preise)
├── storage.h / .cpp            # LittleFS Persistente Speicherung (JSON)
├── web_server.h / .cpp         # REST API Endpunkte für Konfigurations-Webinterface
├── web_ui.h                    # Responsives Web-Interface (HTML/CSS/JS in PROGMEM)
├── README.md                   # Dokumentation (Russisch)
├── README_DE.md                # Dokumentation (Deutsch)
└── README_EN.md                # Dokumentation (Englisch)
```

---

## 📜 Lizenz & Datenquellen

- Preisdaten bereitgestellt von [Tankerkönig](https://creativecommons.tankerkoenig.de) unter [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/).
- Quellcode lizenziert unter der [MIT License](LICENSE).

