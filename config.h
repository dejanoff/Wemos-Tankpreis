#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ---- TM1637 Display Pins ----
#define TM1637_CLK D3  // GPIO0
#define TM1637_DIO D4  // GPIO2

// ---- WiFi AP Settings ----
#define AP_SSID "Tankpreis-Setup"
#define AP_PASS ""  // Open network for easy setup

// ---- LittleFS Config File ----
#define CONFIG_FILE "/config.json"

// ---- Limits ----
#define MAX_FAVORITES 10
#define MAX_WIFI_NETWORKS 5
#define DEFAULT_INTERVAL 600   // 10 minutes in seconds
#define DEFAULT_BRIGHTNESS 5   // 0-7
#define WIFI_CONNECT_TIMEOUT 20 // seconds

// ---- Tankerkoenig API ----
#define API_HOST "creativecommons.tankerkoenig.de"
#define API_BASE_URL "https://creativecommons.tankerkoenig.de/json/"

// ---- Data Structures ----

struct Favorite {
  String id;    // Station UUID
  String name;  // Display name (brand + name)
  String fuel;  // "e5", "e10", or "diesel"
  bool active;  // Whether this favorite is active in rotation

  Favorite() : active(false) {}
};

struct WifiCredential {
  String ssid;
  String pass;
};

struct AppConfig {
  WifiCredential wifiNetworks[MAX_WIFI_NETWORKS];
  int wifiCount;
  String wifiSsid;     // Currently connected/active SSID
  String wifiPass;
  String apiKey;
  int interval;        // Tankerkoenig update interval in seconds (default 600)
  int brightness;      // Daytime display brightness 0-7 (default 5)
  int activeFav;       // Index of active favorite (-1 = none)
  Favorite favorites[MAX_FAVORITES];
  int favCount;        // Number of saved favorites

  // Clock & Display alternation
  int clockDuration;   // Seconds to show clock (default 20)
  int priceDuration;   // Seconds to show price (default 10)

  // Price Display Format (0 = 4 digits "2379", 1 = "2_38", 2 = "2-38", 3 = "2 38")
  int priceFormat;

  // Night mode
  bool nightMode;      // Enable night mode
  int nightStartHour;  // 0-23 (default 23)
  int nightStartMin;   // 0-59 (default 0)
  int nightEndHour;    // 0-23 (default 7)
  int nightEndMin;     // 0-59 (default 0)
  int nightBrightness; // 0 = display completely off, 1-7 = dim (default 1)

  AppConfig() : wifiCount(0),
                interval(DEFAULT_INTERVAL), brightness(DEFAULT_BRIGHTNESS),
                activeFav(-1), favCount(0),
                clockDuration(20), priceDuration(10),
                priceFormat(0),
                nightMode(false), nightStartHour(23), nightStartMin(0),
                nightEndHour(7), nightEndMin(0), nightBrightness(1) {}
};

#endif // CONFIG_H
