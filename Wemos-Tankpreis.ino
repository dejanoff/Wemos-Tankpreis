// ============================================================
// Wemos D1 Mini — Tankpreis Display
// Displays German fuel prices on TM1637 7-segment display
// Uses Tankerkoenig API (creativecommons.tankerkoenig.de)
// Features: NTP Clock, Alternating cycle, Smooth Fade, Trend, Closed, Night mode
// ============================================================

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <DNSServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <TM1637Display.h>
#include <time.h>

// Project modules
#include "config.h"
#include "storage.h"
#include "display_manager.h"
#include "wifi_manager.h"
#include "api_client.h"
#include "web_server.h"

// ---- Global State ----
AppConfig config;
unsigned long lastPriceUpdate = 0;
bool needPriceUpdate = true;
unsigned long lastScrollTime = 0;
const unsigned long SCROLL_INTERVAL_MS = 60000UL; // 1 minute

// Price, Trend & Station Status per Favorite
struct FavState {
  float price = -1.0f;
  float prevPrice = -1.0f;
  int trend = 0; // +1 = UP, -1 = dn, 0 = none
  bool isOpen = true;
  bool valid = false;
};

FavState favStates[MAX_FAVORITES];
int currentDisplayFavIndex = -1;
int activeFavsShownInCycle = 0;
float lastPrice = -1.0f;

int getActiveFavCount() {
  int count = 0;
  for (int i = 0; i < config.favCount; i++) {
    if (config.favorites[i].active) count++;
  }
  return count;
}

int getNextActiveFav(int fromIndex) {
  if (config.favCount == 0) return -1;
  for (int step = 1; step <= config.favCount; step++) {
    int idx = (fromIndex + step) % config.favCount;
    if (config.favorites[idx].active) return idx;
  }
  return -1;
}

// Display Alternation State Machine
enum DisplayMode {
  MODE_CLOCK,
  MODE_PRICE
};

DisplayMode currentMode = MODE_CLOCK;
unsigned long modeStartTime = 0;

// Brightness & Night mode
uint8_t currentAppliedBrightness = 255;
bool displayIsOff = false;

// ---- Time Helpers ----

bool getCurrentTime(int& hour, int& min, int& sec) {
  time_t now = time(nullptr);
  if (now < 100000) return false; // NTP not synced yet
  struct tm ti;
  localtime_r(&now, &ti);
  hour = ti.tm_hour;
  min  = ti.tm_min;
  sec  = ti.tm_sec;
  return true;
}

bool isNightTime(int hour, int min) {
  if (!config.nightMode) return false;
  int cur = hour * 60 + min;
  int st  = config.nightStartHour * 60 + config.nightStartMin;
  int en  = config.nightEndHour * 60 + config.nightEndMin;
  if (st <= en) {
    return (cur >= st && cur < en);
  } else {
    return (cur >= st || cur < en); // Wrap midnight (e.g. 23:00 to 07:00)
  }
}

uint8_t getTargetBrightness(int hour, int min) {
  if (isNightTime(hour, min)) {
    return (uint8_t)config.nightBrightness;
  }
  return (uint8_t)config.brightness;
}

// ---- Callbacks ----

void onConfigChanged() {
  Serial.println(F("[Main] Config changed"));

  // Apply daytime brightness immediately
  currentAppliedBrightness = config.brightness;
  displaySetBrightness(config.brightness);
  displayIsOff = false;

  // Trigger price refresh
  needPriceUpdate = true;
}

// ---- Price Update ----

void updatePrices() {
  if (!wifiIsConnected()) return;
  if (config.apiKey.isEmpty()) return;
  if (config.favCount == 0) return;

  PriceData batchResults[MAX_FAVORITES];
  if (apiFetchAllPrices(config.apiKey, config.favorites, config.favCount, batchResults)) {
    float pricesForWeb[MAX_FAVORITES];
    for (int i = 0; i < config.favCount; i++) {
      if (batchResults[i].ok) {
        favStates[i].valid = true;
        favStates[i].isOpen = batchResults[i].isOpen;
        if (batchResults[i].isOpen && batchResults[i].price > 0) {
          if (favStates[i].price > 0) {
            if (batchResults[i].price > favStates[i].price + 0.0005f) {
              favStates[i].trend = 1; // UP
            } else if (batchResults[i].price < favStates[i].price - 0.0005f) {
              favStates[i].trend = -1; // dn
            }
          }
          favStates[i].prevPrice = favStates[i].price;
          favStates[i].price = batchResults[i].price;
        }
      }
      pricesForWeb[i] = favStates[i].price;
    }
    webServerSetFavoritePrices(pricesForWeb, config.favCount);

    if (config.activeFav >= 0 && config.activeFav < config.favCount) {
      lastPrice = favStates[config.activeFav].price;
      webServerSetPrice(lastPrice);
    }
  }

  lastPriceUpdate = millis();
  needPriceUpdate = false;
}

void showFavoritePrice(int favIdx, bool forceFuelScroll) {
  if (favIdx < 0 || favIdx >= config.favCount) return;
  Favorite& fav = config.favorites[favIdx];
  FavState& st = favStates[favIdx];

  displaySetBrightness(currentAppliedBrightness);

  if (forceFuelScroll) {
    displayScrollFuel(fav.fuel, 160);
  }

  if (st.trend != 0) {
    displayShowTrend(st.trend);
    displayFadeIn(currentAppliedBrightness, 20);
    delay(1200);
    displayFadeOut(currentAppliedBrightness, 20);
    st.trend = 0;
  }

  if (st.valid && !st.isOpen) {
    displayShowClosed();
  } else if (st.price > 0) {
    displayShowPrice(st.price, config.priceFormat);
  } else {
    displayShowLoading();
  }
  displayFadeIn(currentAppliedBrightness, 20);
}

// ---- Display State Machine ----

void updateDisplay() {
  int hour = 0, min = 0, sec = 0;
  bool haveTime = getCurrentTime(hour, min, sec);

  // Check night mode / brightness
  uint8_t targetBr = haveTime ? getTargetBrightness(hour, min) : (uint8_t)config.brightness;
  if (targetBr == 0) {
    if (!displayIsOff) {
      displayFadeOut(currentAppliedBrightness, 20);
      displayOff();
      displayIsOff = true;
    }
    return; // Night mode: display off
  } else {
    if (displayIsOff) {
      displayIsOff = false;
      displaySetBrightness(targetBr);
      currentAppliedBrightness = targetBr;
      displayFadeIn(targetBr, 20);
    } else if (currentAppliedBrightness != targetBr) {
      currentAppliedBrightness = targetBr;
      displaySetBrightness(targetBr);
    }
  }

  int activeCount = getActiveFavCount();

  // If time not synced yet, display active price if available
  if (!haveTime) {
    if (currentDisplayFavIndex < 0) currentDisplayFavIndex = getNextActiveFav(-1);
    if (currentDisplayFavIndex >= 0 && favStates[currentDisplayFavIndex].price > 0) {
      displayShowPrice(favStates[currentDisplayFavIndex].price, config.priceFormat);
    }
    return;
  }

  // If no active favorite selected, simply display clock
  if (activeCount == 0) {
    static int lastClockSec = -1;
    if (sec != lastClockSec) {
      lastClockSec = sec;
      bool colon = (sec % 2 == 0);
      displayShowTime(hour, min, colon);
    }
    return;
  }

  unsigned long now = millis();
  unsigned long elapsed = now - modeStartTime;

  switch (currentMode) {
    case MODE_CLOCK: {
      static int lastClockSec = -1;
      if (sec != lastClockSec) {
        lastClockSec = sec;
        bool colon = (sec % 2 == 0);
        displayShowTime(hour, min, colon);
      }

      unsigned long clockMs = (unsigned long)config.clockDuration * 1000UL;
      if (elapsed >= clockMs) {
        displayFadeOut(currentAppliedBrightness, 20);
        activeFavsShownInCycle = 0;

        currentDisplayFavIndex = getNextActiveFav(currentDisplayFavIndex);
        if (currentDisplayFavIndex >= 0) {
          bool needScroll = (activeCount > 1) || (now - lastScrollTime >= SCROLL_INTERVAL_MS);
          if (now - lastScrollTime >= SCROLL_INTERVAL_MS) {
            lastScrollTime = now;
          }
          showFavoritePrice(currentDisplayFavIndex, needScroll);
          activeFavsShownInCycle = 1;
          currentMode = MODE_PRICE;
          modeStartTime = millis();
        }
      }
      break;
    }

    case MODE_PRICE: {
      unsigned long priceMs = (unsigned long)config.priceDuration * 1000UL;
      if (elapsed >= priceMs) {
        displayFadeOut(currentAppliedBrightness, 20);

        if (activeFavsShownInCycle < activeCount) {
          // Show next active favorite in rotation
          int nextFav = getNextActiveFav(currentDisplayFavIndex);
          if (nextFav >= 0) {
            currentDisplayFavIndex = nextFav;
            showFavoritePrice(currentDisplayFavIndex, true); // Always scroll fuel name when multiple
            activeFavsShownInCycle++;
            modeStartTime = millis();
            break;
          }
        }

        // All active favorites shown -> return to CLOCK
        currentMode = MODE_CLOCK;
        bool colon = (sec % 2 == 0);
        displayShowTime(hour, min, colon);
        displayFadeIn(currentAppliedBrightness, 20);
        activeFavsShownInCycle = 0;
        modeStartTime = millis();
      }
      break;
    }
  }
}

// ---- Setup ----

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println(F("\n\n============================="));
  Serial.println(F("  Tankpreis & Clock Display"));
  Serial.println(F("============================="));

  // Initialize display
  displayInit(DEFAULT_BRIGHTNESS);
  currentAppliedBrightness = DEFAULT_BRIGHTNESS;

  // Hardware segment & dot test on boot (1 second)
  displayLampTest();
  delay(1000);

  displayShowLoading();

  // Initialize filesystem
  if (!storageInit()) {
    Serial.println(F("[Main] LittleFS init FAILED!"));
    displayShowError();
    delay(3000);
    ESP.restart();
  }
  Serial.println(F("[Main] LittleFS initialized"));

  // Load saved configuration
  if (loadConfig(config)) {
    Serial.println(F("[Main] Configuration loaded"));
    displaySetBrightness(config.brightness);
    currentAppliedBrightness = config.brightness;
  } else {
    Serial.println(F("[Main] No config found, using defaults"));
  }

  // Initialize WiFi
  wifiInit();

  // Try connecting to saved WiFi
  bool connected = false;
  if (config.wifiCount > 0) {
    Serial.printf("[Main] Connecting to multi WiFi (%d saved)...\n", config.wifiCount);
    connected = wifiConnectMulti(config.wifiNetworks, config.wifiCount, WIFI_CONNECT_TIMEOUT);
  } else if (!config.wifiSsid.isEmpty()) {
    Serial.printf("[Main] Connecting to saved WiFi: %s\n", config.wifiSsid.c_str());
    connected = wifiConnect(config.wifiSsid, config.wifiPass, WIFI_CONNECT_TIMEOUT);
  }

  // Start Access Point if not connected
  if (!connected) {
    Serial.println(F("[Main] Starting Access Point mode"));
    wifiStartAP();
    displayShowAP();
  } else {
    // Show IP address on TM1637 display
    displayShowIP(wifiGetIPAddress());
    // Keep active SSID synced with actually connected WiFi
    String curSsid = wifiGetSSID();
    if (!curSsid.isEmpty() && config.wifiSsid != curSsid) {
      config.wifiSsid = curSsid;
      saveConfig(config);
    }
    // Configure NTP time for Germany (Berlin timezone with auto DST)
    configTime("CET-1CEST,M3.5.0,M10.5.0/3", "pool.ntp.org", "time.google.com");
    Serial.println(F("[Main] NTP configured for CET/CEST"));
  }

  // Start web server
  webServerInit(&config);
  webServerSetConfigCallback(onConfigChanged);

  modeStartTime = millis();
  lastScrollTime = millis();

  Serial.print(F("[Main] Ready! IP: "));
  Serial.println(wifiGetIP());
  Serial.println(F("[Main] Open http://"));
  Serial.print(wifiGetIP());
  Serial.println(F("/ in your browser"));
}

// ---- Main Loop ----

void loop() {
  // Handle web server requests
  webServerLoop();

  // Handle captive portal DNS
  wifiLoop();

  // Handle pending WiFi connection requested from Web UI
  if (webServerHasPendingConnect()) {
    String s, p;
    webServerGetPendingConnect(s, p);
    Serial.printf("[Main] Connecting to %s from web request...\n", s.c_str());
    bool ok = wifiConnect(s, p, WIFI_CONNECT_TIMEOUT);
    if (ok) {
      displayShowIP(wifiGetIPAddress());
      configTime("CET-1CEST,M3.5.0,M10.5.0/3", "pool.ntp.org", "time.google.com");
      needPriceUpdate = true;
    } else {
      Serial.println(F("[Main] Connection failed, restarting AP mode"));
      wifiStartAP();
    }
  }

  // Ensure NTP sync once WiFi is connected
  if (wifiIsConnected() && time(nullptr) < 100000) {
    static unsigned long lastNtpAttempt = 0;
    if (millis() - lastNtpAttempt > 10000) {
      lastNtpAttempt = millis();
      configTime("CET-1CEST,M3.5.0,M10.5.0/3", "pool.ntp.org", "time.google.com");
    }
  }

  // Periodic price update
  if (wifiIsConnected() && getActiveFavCount() > 0 && !config.apiKey.isEmpty()) {
    unsigned long now = millis();
    unsigned long intervalMs = (unsigned long)config.interval * 1000UL;

    if (needPriceUpdate || (now - lastPriceUpdate >= intervalMs)) {
      updatePrices();
    }
  }

  // Update display state machine (Clock / Trend / Price / Closed with smooth fade)
  updateDisplay();

  // Let the ESP8266 handle background tasks
  yield();
}
