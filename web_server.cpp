#include "web_server.h"
#include "web_ui.h"
#include "api_client.h"
#include "storage.h"
#include "wifi_manager.h"
#include "display_manager.h"
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

static ESP8266WebServer server(80);
static AppConfig* cfg = nullptr;
static float currentPrice = -1.0f;
static ConfigChangeCallback onConfigChange = nullptr;

static bool pendingConnect = false;
static unsigned long pendingConnectTime = 0;
static String connectSsid = "";
static String connectPass = "";

void triggerWifiConnect(const String& ssid, const String& pass) {
  connectSsid = ssid;
  connectPass = pass;
  pendingConnect = true;
  pendingConnectTime = millis() + 500;
}

bool webServerHasPendingConnect() {
  return pendingConnect && (millis() >= pendingConnectTime);
}

void webServerGetPendingConnect(String& ssid, String& pass) {
  ssid = connectSsid;
  pass = connectPass;
  pendingConnect = false;
}

// ---- Helpers ----

static String maskApiKey(const String& key) {
  if (key.length() > 8) {
    return key.substring(0, 4) + "..." + key.substring(key.length() - 4);
  } else if (!key.isEmpty()) {
    return "****";
  }
  return "";
}

// ---- Handlers ----

static void handleRoot() {
  server.send_P(200, "text/html", INDEX_HTML);
}

static void handleStatus() {
  bool connected = wifiIsConnected();
  String connectedSsid = "";
  if (connected) {
    connectedSsid = wifiGetSSID();
    if (connectedSsid.isEmpty()) {
      connectedSsid = cfg->wifiSsid;
    }
  }

  String json = "{\"wifi_connected\":";
  json += connected ? "true" : "false";
  json += ",\"wifi_ip\":\"";
  json += wifiGetIP();
  json += "\",\"wifi_ssid\":\"";
  json += connectedSsid;
  json += "\",\"price\":";
  json += String(currentPrice, 3);
  json += ",\"active_fav\":";
  json += String(cfg->activeFav);
  json += "}";
  server.send(200, "application/json", json);
}

static void handleWifiScan() {
  int status = wifiScanStatus();
  if (status == WIFI_SCAN_RUNNING) {
    server.send(200, "application/json", "{\"status\":\"scanning\"}");
    return;
  }
  if (status >= 0) {
    String json = wifiGetScanResultsJson();
    server.send(200, "application/json", json);
    return;
  }
  // Start scan asynchronously
  wifiStartScan();
  server.send(200, "application/json", "{\"status\":\"scanning\"}");
}

static void handleWifiConnect() {
  JsonDocument doc;
  deserializeJson(doc, server.arg("plain"));

  String ssid = doc["ssid"] | "";
  String pass = doc["password"] | "";

  if (ssid.isEmpty()) {
    server.send(200, "application/json", "{\"success\":false,\"message\":\"SSID darf nicht leer sein\"}");
    return;
  }

  cfg->wifiSsid = ssid;
  if (!pass.isEmpty()) cfg->wifiPass = pass;
  saveConfig(*cfg);

  triggerWifiConnect(ssid, pass);
  server.send(200, "application/json", "{\"success\":true,\"connecting\":true}");
}

static void handleWifiSaved() {
  JsonDocument doc;
  JsonArray arr = doc.to<JsonArray>();
  for (int i = 0; i < cfg->wifiCount; i++) {
    JsonObject obj = arr.add<JsonObject>();
    obj["index"] = i;
    obj["ssid"] = cfg->wifiNetworks[i].ssid;
  }
  String json;
  serializeJson(doc, json);
  server.send(200, "application/json", json);
}

static void handleWifiAdd() {
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, server.arg("plain"));
  if (err) {
    server.send(400, "application/json", "{\"success\":false,\"message\":\"Ungueltige Daten\"}");
    return;
  }

  String ssid = doc["ssid"] | "";
  String pass = doc["password"] | "";
  bool connectNow = doc["connect_now"] | false;

  if (ssid.isEmpty()) {
    server.send(200, "application/json", "{\"success\":false,\"message\":\"SSID darf nicht leer sein\"}");
    return;
  }

  int targetIdx = -1;
  for (int i = 0; i < cfg->wifiCount; i++) {
    if (cfg->wifiNetworks[i].ssid == ssid) {
      targetIdx = i;
      break;
    }
  }

  if (targetIdx >= 0) {
    if (!pass.isEmpty()) cfg->wifiNetworks[targetIdx].pass = pass;
  } else {
    if (cfg->wifiCount < MAX_WIFI_NETWORKS) {
      targetIdx = cfg->wifiCount;
      cfg->wifiNetworks[targetIdx].ssid = ssid;
      cfg->wifiNetworks[targetIdx].pass = pass;
      cfg->wifiCount++;
    } else {
      server.send(200, "application/json", "{\"success\":false,\"message\":\"Maximum 5 Netzwerke gespeichert\"}");
      return;
    }
  }

  if (connectNow) {
    cfg->wifiSsid = ssid;
    if (!pass.isEmpty()) cfg->wifiPass = pass;
    saveConfig(*cfg);
    triggerWifiConnect(ssid, cfg->wifiNetworks[targetIdx].pass);
  } else {
    saveConfig(*cfg);
  }

  String json = "{\"success\":true,\"connecting\":";
  json += connectNow ? "true" : "false";
  json += "}";
  server.send(200, "application/json", json);
}

static void handleWifiRemove() {
  int idx = -1;
  if (server.hasArg("index")) {
    idx = server.arg("index").toInt();
  } else if (server.hasArg("ssid")) {
    String s = server.arg("ssid");
    for (int i = 0; i < cfg->wifiCount; i++) {
      if (cfg->wifiNetworks[i].ssid == s) {
        idx = i;
        break;
      }
    }
  }

  if (idx < 0 || idx >= cfg->wifiCount) {
    server.send(200, "application/json", "{\"success\":false,\"message\":\"Netzwerk nicht gefunden\"}");
    return;
  }

  for (int i = idx; i < cfg->wifiCount - 1; i++) {
    cfg->wifiNetworks[i] = cfg->wifiNetworks[i + 1];
  }
  cfg->wifiCount--;

  if (cfg->wifiCount == 0) {
    cfg->wifiSsid = "";
    cfg->wifiPass = "";
  }

  saveConfig(*cfg);
  server.send(200, "application/json", "{\"success\":true}");
}

static void handleWifiDelete() {
  cfg->wifiSsid = "";
  cfg->wifiPass = "";
  cfg->wifiCount = 0;
  saveConfig(*cfg);
  server.send(200, "application/json", "{\"success\":true}");
  Serial.println(F("[Web] All WiFi credentials deleted"));
}

static void handleSystemReboot() {
  server.send(200, "application/json", "{\"success\":true,\"message\":\"Wemos startet neu...\"}");
  delay(500);
  ESP.restart();
}

static void handleSystemReset() {
  server.send(200, "application/json", "{\"success\":true,\"message\":\"Speicher geloescht. Neustart...\"}");
  LittleFS.remove(CONFIG_FILE);
  LittleFS.format();
  delay(500);
  ESP.restart();
}

static void handleGetConfig() {
  JsonDocument doc;
  doc["api_key"] = cfg->apiKey;
  doc["api_key_masked"] = maskApiKey(cfg->apiKey);
  doc["api_key_set"] = !cfg->apiKey.isEmpty();
  doc["interval"] = cfg->interval;
  doc["brightness"] = cfg->brightness;

  doc["clock_duration"]   = cfg->clockDuration;
  doc["price_duration"]   = cfg->priceDuration;
  doc["price_format"]     = cfg->priceFormat;
  doc["night_mode"]       = cfg->nightMode;
  doc["night_start_hour"] = cfg->nightStartHour;
  doc["night_start_min"]  = cfg->nightStartMin;
  doc["night_end_hour"]   = cfg->nightEndHour;
  doc["night_end_min"]    = cfg->nightEndMin;
  doc["night_brightness"] = cfg->nightBrightness;

  String json;
  serializeJson(doc, json);
  server.send(200, "application/json", json);
}

static void handlePostConfig() {
  JsonDocument doc;
  deserializeJson(doc, server.arg("plain"));

  // Only update API key if a non-empty value is provided
  if (!doc["api_key"].isNull()) {
    String newKey = doc["api_key"].as<String>();
    if (!newKey.isEmpty()) {
      cfg->apiKey = newKey;
    }
  }
  if (!doc["interval"].isNull()) {
    cfg->interval = doc["interval"].as<int>();
    if (cfg->interval < 300) cfg->interval = 300;  // min 5 minutes
  }
  if (!doc["brightness"].isNull()) {
    cfg->brightness = constrain(doc["brightness"].as<int>(), 0, 7);
  }

  if (!doc["clock_duration"].isNull()) {
    cfg->clockDuration = constrain(doc["clock_duration"].as<int>(), 5, 120);
  }
  if (!doc["price_duration"].isNull()) {
    cfg->priceDuration = constrain(doc["price_duration"].as<int>(), 5, 120);
  }
  if (!doc["price_format"].isNull()) {
    cfg->priceFormat = constrain(doc["price_format"].as<int>(), 0, 3);
  }
  if (!doc["night_mode"].isNull()) {
    cfg->nightMode = doc["night_mode"].as<bool>();
  }
  if (!doc["night_start_hour"].isNull()) {
    cfg->nightStartHour = constrain(doc["night_start_hour"].as<int>(), 0, 23);
  }
  if (!doc["night_start_min"].isNull()) {
    cfg->nightStartMin = constrain(doc["night_start_min"].as<int>(), 0, 59);
  }
  if (!doc["night_end_hour"].isNull()) {
    cfg->nightEndHour = constrain(doc["night_end_hour"].as<int>(), 0, 23);
  }
  if (!doc["night_end_min"].isNull()) {
    cfg->nightEndMin = constrain(doc["night_end_min"].as<int>(), 0, 59);
  }
  if (!doc["night_brightness"].isNull()) {
    cfg->nightBrightness = constrain(doc["night_brightness"].as<int>(), 0, 7);
  }

  saveConfig(*cfg);
  if (onConfigChange) onConfigChange();

  server.send(200, "application/json", "{\"success\":true}");
}

static void handleStationSearch() {
  if (cfg->apiKey.isEmpty()) {
    server.send(200, "application/json", "{\"ok\":false,\"message\":\"API-Key nicht gesetzt\"}");
    return;
  }

  String lat = server.arg("lat");
  String lng = server.arg("lng");
  String rad = server.arg("rad");

  lat.trim();
  lng.trim();
  rad.trim();

  // Support German comma decimals
  lat.replace(',', '.');
  lng.replace(',', '.');
  rad.replace(',', '.');

  if (lat.isEmpty() || lng.isEmpty()) {
    server.send(200, "application/json", "{\"ok\":false,\"message\":\"Koordinaten fehlen\"}");
    return;
  }

  float latVal = lat.toFloat();
  float lngVal = lng.toFloat();

  // Auto-correct swapped coordinates for Germany (lat ~ 47-55, lng ~ 6-15)
  if (latVal < 20.0f && lngVal > 45.0f) {
    float tmp = latVal;
    latVal = lngVal;
    lngVal = tmp;
  }

  float radius = rad.isEmpty() ? 5.0f : rad.toFloat();
  if (!apiStreamSearchStations(cfg->apiKey, latVal, lngVal, radius, server)) {
    server.send(200, "application/json", "{\"ok\":false,\"message\":\"Tankerkoenig-API nicht erreichbar\"}");
  }
}

static void handleStationSearchPLZ() {
  if (cfg->apiKey.isEmpty()) {
    server.send(200, "application/json", "{\"ok\":false,\"message\":\"API-Key nicht gesetzt\"}");
    return;
  }

  String plz = server.arg("plz");
  String rad = server.arg("rad");
  plz.trim();
  rad.trim();
  rad.replace(',', '.');

  if (plz.isEmpty() || plz.length() != 5) {
    server.send(200, "application/json", "{\"ok\":false,\"message\":\"Bitte gueltige PLZ eingeben (5 Ziffern)\"}");
    return;
  }

  // Geocode PLZ to coordinates via Photon
  float lat = 0.0f, lng = 0.0f;
  if (!geocodePLZ(plz, lat, lng)) {
    server.send(200, "application/json", "{\"ok\":false,\"message\":\"PLZ nicht gefunden oder Geocoding fehlgeschlagen\"}");
    return;
  }

  Serial.printf("[Web] PLZ %s -> lat=%.4f lng=%.4f\n", plz.c_str(), lat, lng);

  float radius = rad.isEmpty() ? 5.0f : rad.toFloat();
  if (!apiStreamSearchStations(cfg->apiKey, lat, lng, radius, server)) {
    server.send(200, "application/json", "{\"ok\":false,\"message\":\"Tankerkoenig-API nicht erreichbar\"}");
  }
}

static void handleStationDetail() {
  if (cfg->apiKey.isEmpty()) {
    server.send(200, "application/json", "{\"ok\":false,\"message\":\"API-Key nicht gesetzt\"}");
    return;
  }

  String id = server.arg("id");
  id.trim();
  if (id.isEmpty()) {
    server.send(200, "application/json", "{\"ok\":false,\"message\":\"Station-ID fehlt\"}");
    return;
  }

  if (!apiStreamStationDetail(cfg->apiKey, id, server)) {
    server.send(200, "application/json", "{\"ok\":false,\"message\":\"API nicht erreichbar\"}");
  }
}

static float favPricesArray[MAX_FAVORITES] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };

static void handleGetFavorites() {
  JsonDocument doc;
  JsonArray favs = doc["favorites"].to<JsonArray>();

  for (int i = 0; i < cfg->favCount; i++) {
    JsonObject fav = favs.add<JsonObject>();
    fav["id"]     = cfg->favorites[i].id;
    fav["name"]   = cfg->favorites[i].name;
    fav["fuel"]   = cfg->favorites[i].fuel;
    fav["active"] = cfg->favorites[i].active;
    fav["price"]  = favPricesArray[i];
  }

  doc["active"] = cfg->activeFav;
  doc["price"]  = currentPrice;

  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

static void handleAddFavorite() {
  if (cfg->favCount >= MAX_FAVORITES) {
    server.send(200, "application/json",
                "{\"success\":false,\"message\":\"Maximum " + String(MAX_FAVORITES) + " Favoriten\"}");
    return;
  }

  JsonDocument doc;
  deserializeJson(doc, server.arg("plain"));

  String id   = doc["id"]   | "";
  String name = doc["name"] | "";
  String fuel = doc["fuel"] | "e5";

  if (id.isEmpty()) {
    server.send(200, "application/json", "{\"success\":false,\"message\":\"Station-ID fehlt\"}");
    return;
  }

  // Check for duplicate
  for (int i = 0; i < cfg->favCount; i++) {
    if (cfg->favorites[i].id == id && cfg->favorites[i].fuel == fuel) {
      server.send(200, "application/json", "{\"success\":false,\"message\":\"Bereits in Favoriten\"}");
      return;
    }
  }

  int idx = cfg->favCount;
  cfg->favorites[idx].id     = id;
  cfg->favorites[idx].name   = name;
  cfg->favorites[idx].fuel   = fuel;
  cfg->favorites[idx].active = true; // Auto-activate newly added favorite
  cfg->favCount++;

  if (cfg->activeFav < 0) {
    cfg->activeFav = idx;
  }

  saveConfig(*cfg);
  if (onConfigChange) onConfigChange();

  server.send(200, "application/json", "{\"success\":true}");
}

static void handleDeleteFavorite() {
  int index = server.arg("index").toInt();
  if (index < 0 || index >= cfg->favCount) {
    server.send(200, "application/json", "{\"success\":false,\"message\":\"Ungueltiger Index\"}");
    return;
  }

  // Shift remaining favorites down
  for (int i = index; i < cfg->favCount - 1; i++) {
    cfg->favorites[i] = cfg->favorites[i + 1];
    favPricesArray[i] = favPricesArray[i + 1];
  }
  favPricesArray[cfg->favCount - 1] = -1.0f;
  cfg->favCount--;

  // Adjust active favorite index
  if (cfg->favCount == 0) {
    cfg->activeFav = -1;
  } else if (cfg->activeFav == index) {
    cfg->activeFav = 0;
  } else if (cfg->activeFav > index) {
    cfg->activeFav--;
  }

  saveConfig(*cfg);
  if (onConfigChange) onConfigChange();

  server.send(200, "application/json", "{\"success\":true}");
}

static void handleToggleFavorite() {
  int index = server.arg("index").toInt();
  if (index < 0 || index >= cfg->favCount) {
    server.send(200, "application/json", "{\"success\":false}");
    return;
  }

  cfg->favorites[index].active = !cfg->favorites[index].active;

  cfg->activeFav = -1;
  for (int i = 0; i < cfg->favCount; i++) {
    if (cfg->favorites[i].active) {
      cfg->activeFav = i;
      break;
    }
  }

  saveConfig(*cfg);
  if (onConfigChange) onConfigChange();

  server.send(200, "application/json", "{\"success\":true}");
}

static void handleActivateFavorite() {
  int index = server.arg("index").toInt();
  if (index < 0 || index >= cfg->favCount) {
    server.send(200, "application/json", "{\"success\":false}");
    return;
  }

  cfg->favorites[index].active = true;
  cfg->activeFav = index;
  saveConfig(*cfg);
  if (onConfigChange) onConfigChange();

  server.send(200, "application/json", "{\"success\":true}");
}

static void handleNotFound() {
  // Captive portal: redirect all unknown URLs to root
  server.sendHeader("Location", "http://" + wifiGetIP(), true);
  server.send(302, "text/plain", "");
}

// ---- Public API ----

void webServerInit(AppConfig* config) {
  cfg = config;

  server.on("/",                        HTTP_GET,    handleRoot);
  server.on("/api/status",              HTTP_GET,    handleStatus);
  server.on("/api/wifi/scan",           HTTP_GET,    handleWifiScan);
  server.on("/api/wifi/connect",        HTTP_POST,   handleWifiConnect);
  server.on("/api/wifi/saved",          HTTP_GET,    handleWifiSaved);
  server.on("/api/wifi/saved",          HTTP_DELETE, handleWifiDelete);
  server.on("/api/wifi/add",            HTTP_POST,   handleWifiAdd);
  server.on("/api/wifi/remove",         HTTP_DELETE, handleWifiRemove);
  server.on("/api/system/reboot",       HTTP_POST,   handleSystemReboot);
  server.on("/api/system/reset",        HTTP_POST,   handleSystemReset);
  server.on("/api/config",              HTTP_GET,    handleGetConfig);
  server.on("/api/config",              HTTP_POST,   handlePostConfig);
  server.on("/api/stations/search",     HTTP_GET,    handleStationSearch);
  server.on("/api/stations/searchplz",  HTTP_GET,    handleStationSearchPLZ);
  server.on("/api/stations/detail",     HTTP_GET,    handleStationDetail);
  server.on("/api/favorites",           HTTP_GET,    handleGetFavorites);
  server.on("/api/favorites",           HTTP_POST,   handleAddFavorite);
  server.on("/api/favorites",           HTTP_DELETE, handleDeleteFavorite);
  server.on("/api/favorites/activate",  HTTP_POST,   handleActivateFavorite);
  server.on("/api/favorites/toggle",    HTTP_POST,   handleToggleFavorite);

  server.onNotFound(handleNotFound);
  server.begin();

  Serial.println(F("[Web] Server started on port 80"));
}

void webServerLoop() {
  server.handleClient();
}

void webServerSetConfigCallback(ConfigChangeCallback cb) {
  onConfigChange = cb;
}

void webServerSetPrice(float price) {
  currentPrice = price;
}

void webServerSetFavoritePrices(const float* prices, int count) {
  for (int i = 0; i < count && i < MAX_FAVORITES; i++) {
    favPricesArray[i] = prices[i];
  }
}
