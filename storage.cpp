#include "storage.h"
#include <LittleFS.h>
#include <ArduinoJson.h>

bool storageInit() {
  return LittleFS.begin();
}

bool loadConfig(AppConfig& config) {
  File file = LittleFS.open(CONFIG_FILE, "r");
  if (!file) {
    Serial.println(F("[Storage] No config file found"));
    return false;
  }

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, file);
  file.close();

  if (err) {
    Serial.print(F("[Storage] JSON parse error: "));
    Serial.println(err.c_str());
    return false;
  }

  config.wifiSsid   = doc["wifi_ssid"] | "";
  config.wifiPass   = doc["wifi_pass"] | "";
  config.apiKey     = doc["api_key"]   | "";
  config.interval   = doc["interval"]  | DEFAULT_INTERVAL;
  config.brightness = doc["brightness"]| DEFAULT_BRIGHTNESS;
  config.activeFav  = doc["active_fav"]| -1;

  config.clockDuration   = doc["clock_duration"]   | 20;
  config.priceDuration   = doc["price_duration"]   | 10;
  config.priceFormat     = doc["price_format"]     | 0;
  config.nightMode       = doc["night_mode"]       | false;
  config.nightStartHour  = doc["night_start_hour"] | 23;
  config.nightStartMin   = doc["night_start_min"]  | 0;
  config.nightEndHour    = doc["night_end_hour"]   | 7;
  config.nightEndMin     = doc["night_end_min"]    | 0;
  config.nightBrightness = doc["night_brightness"] | 1;

  // Load WiFi networks array
  config.wifiCount = 0;
  JsonArray nets = doc["wifi_networks"].as<JsonArray>();
  if (!nets.isNull()) {
    for (JsonObject net : nets) {
      if (config.wifiCount >= MAX_WIFI_NETWORKS) break;
      config.wifiNetworks[config.wifiCount].ssid = net["ssid"] | "";
      config.wifiNetworks[config.wifiCount].pass = net["pass"] | "";
      if (!config.wifiNetworks[config.wifiCount].ssid.isEmpty()) {
        config.wifiCount++;
      }
    }
  }

  // Migration: if wifi_networks was empty but wifi_ssid was saved, store it as network #0
  if (config.wifiCount == 0 && !config.wifiSsid.isEmpty()) {
    config.wifiNetworks[0].ssid = config.wifiSsid;
    config.wifiNetworks[0].pass = config.wifiPass;
    config.wifiCount = 1;
  }

  config.favCount = 0;
  bool anyActive = false;
  JsonArray favs = doc["favorites"].as<JsonArray>();
  if (!favs.isNull()) {
    for (JsonObject fav : favs) {
      if (config.favCount >= MAX_FAVORITES) break;
      config.favorites[config.favCount].id     = fav["id"]   | "";
      config.favorites[config.favCount].name   = fav["name"] | "";
      config.favorites[config.favCount].fuel   = fav["fuel"] | "e5";
      config.favorites[config.favCount].active = fav["active"] | false;
      if (config.favorites[config.favCount].active) anyActive = true;
      config.favCount++;
    }
  }

  // Migration: if no favorite was marked active, activate activeFav
  if (!anyActive && config.activeFav >= 0 && config.activeFav < config.favCount) {
    config.favorites[config.activeFav].active = true;
  }

  Serial.printf("[Storage] Loaded: SSIDs=%d, favs=%d, active=%d, night=%d, format=%d\n",
                config.wifiCount, config.favCount, config.activeFav, config.nightMode, config.priceFormat);
  return true;
}

bool saveConfig(const AppConfig& config) {
  JsonDocument doc;

  doc["wifi_ssid"]  = config.wifiSsid;
  doc["wifi_pass"]  = config.wifiPass;
  doc["api_key"]    = config.apiKey;
  doc["interval"]   = config.interval;
  doc["brightness"] = config.brightness;
  doc["active_fav"] = config.activeFav;

  doc["clock_duration"]   = config.clockDuration;
  doc["price_duration"]   = config.priceDuration;
  doc["price_format"]     = config.priceFormat;
  doc["night_mode"]       = config.nightMode;
  doc["night_start_hour"] = config.nightStartHour;
  doc["night_start_min"]  = config.nightStartMin;
  doc["night_end_hour"]   = config.nightEndHour;
  doc["night_end_min"]    = config.nightEndMin;
  doc["night_brightness"] = config.nightBrightness;

  // Save WiFi networks array
  JsonArray nets = doc["wifi_networks"].to<JsonArray>();
  for (int i = 0; i < config.wifiCount; i++) {
    JsonObject net = nets.add<JsonObject>();
    net["ssid"] = config.wifiNetworks[i].ssid;
    net["pass"] = config.wifiNetworks[i].pass;
  }

  JsonArray favs = doc["favorites"].to<JsonArray>();
  for (int i = 0; i < config.favCount; i++) {
    JsonObject fav = favs.add<JsonObject>();
    fav["id"]     = config.favorites[i].id;
    fav["name"]   = config.favorites[i].name;
    fav["fuel"]   = config.favorites[i].fuel;
    fav["active"] = config.favorites[i].active;
  }

  File file = LittleFS.open(CONFIG_FILE, "w");
  if (!file) {
    Serial.println(F("[Storage] Failed to open config file for writing"));
    return false;
  }

  serializeJson(doc, file);
  file.close();
  Serial.println(F("[Storage] Config saved"));
  return true;
}
