#include "api_client.h"
#include "config.h"
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <ArduinoJson.h>

// Perform HTTPS GET request, return response body
static String httpGet(const String& url) {
  std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
  client->setInsecure();  // Skip certificate verification (saves RAM)
  client->setBufferSizes(16384, 512);

  HTTPClient http;
  http.setTimeout(10000);  // 10 second timeout
  http.setUserAgent("ESP8266-Tankpreis/1.0");
  String payload = "";

  Serial.print(F("[API] GET "));
  Serial.println(url);

  if (http.begin(*client, url)) {
    int code = http.GET();
    if (code == HTTP_CODE_OK) {
      payload = http.getString();
    } else {
      Serial.printf("[API] HTTP error: %d\n", code);
    }
    http.end();
  } else {
    Serial.println(F("[API] Connection failed"));
  }

  return payload;
}

class WebServerChunkPrint : public Print {
  ESP8266WebServer& _srv;
  char _buf[512];
  size_t _idx = 0;
public:
  WebServerChunkPrint(ESP8266WebServer& srv) : _srv(srv) {}

  ~WebServerChunkPrint() {
    flush();
  }

  void flush() override {
    if (_idx > 0) {
      _srv.sendContent(_buf, _idx);
      _idx = 0;
    }
  }

  size_t write(uint8_t c) override {
    _buf[_idx++] = (char)c;
    if (_idx >= sizeof(_buf)) {
      flush();
    }
    return 1;
  }

  size_t write(const uint8_t *buffer, size_t size) override {
    for (size_t i = 0; i < size; i++) {
      write(buffer[i]);
    }
    return size;
  }

  int availableForWrite() override {
    return 512;
  }
};

static bool apiStreamHttpGet(const String& url, ESP8266WebServer& srv) {
  std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
  client->setInsecure();
  client->setBufferSizes(16384, 512);

  HTTPClient http;
  http.setTimeout(15000);
  http.setUserAgent("ESP8266-Tankpreis/1.0");

  Serial.print(F("[API] Stream GET "));
  Serial.println(url);

  if (!http.begin(*client, url)) {
    Serial.println(F("[API] Connection failed"));
    return false;
  }

  int code = http.GET();
  if (code != HTTP_CODE_OK) {
    Serial.printf("[API] HTTP error: %d\n", code);
    http.end();
    return false;
  }

  srv.setContentLength(CONTENT_LENGTH_UNKNOWN);
  srv.send(200, "application/json", "");

  int bytesWritten = 0;
  {
    WebServerChunkPrint chunkPrinter(srv);
    bytesWritten = http.writeToPrint(&chunkPrinter);
    chunkPrinter.flush();
  }
  Serial.printf("[API] Stream finished, bytes sent: %d\n", bytesWritten);

  srv.sendContent(""); // Ends chunked stream
  http.end();
  return (bytesWritten > 0);
}

bool apiStreamSearchStations(const String& apiKey, float lat, float lng, float radius, ESP8266WebServer& srv) {
  if (radius > 25.0f) radius = 25.0f;  // API limit
  if (radius < 1.0f)  radius = 1.0f;

  String url = String(API_BASE_URL) + "list.php?lat="
    + String(lat, 6) + "&lng=" + String(lng, 6)
    + "&rad=" + String(radius, 1)
    + "&sort=dist&type=all&apikey=" + apiKey;

  return apiStreamHttpGet(url, srv);
}

String apiSearchStations(const String& apiKey, float lat, float lng, float radius) {
  if (radius > 25.0f) radius = 25.0f;  // API limit
  if (radius < 1.0f)  radius = 1.0f;

  String url = String(API_BASE_URL) + "list.php?lat="
    + String(lat, 6) + "&lng=" + String(lng, 6)
    + "&rad=" + String(radius, 1)
    + "&sort=dist&type=all&apikey=" + apiKey;

  return httpGet(url);
}

PriceData apiFetchPriceData(const String& apiKey, const String& stationId, const String& fuelType) {
  PriceData res = { false, false, -1.0f };

  String url = String(API_BASE_URL) + "prices.php?ids="
    + stationId + "&apikey=" + apiKey;

  String payload = httpGet(url);
  if (payload.isEmpty()) return res;

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, payload);
  if (err) {
    Serial.print(F("[API] JSON parse error: "));
    Serial.println(err.c_str());
    return res;
  }

  if (!(doc["ok"].as<bool>())) {
    Serial.print(F("[API] API error: "));
    Serial.println(doc["message"].as<const char*>());
    return res;
  }

  JsonObject station = doc["prices"][stationId];
  if (station.isNull()) {
    Serial.println(F("[API] Station not found in response"));
    return res;
  }

  res.ok = true;
  String status = station["status"] | "closed";
  res.isOpen = (status == "open");

  if (!res.isOpen) {
    Serial.printf("[API] Station is closed (status: %s)\n", status.c_str());
    return res;
  }

  // Get price for requested fuel type
  float price = station[fuelType] | -1.0f;
  if (price > 0) {
    res.price = price;
    Serial.printf("[API] %s price: %.3f\n", fuelType.c_str(), price);
  } else {
    Serial.printf("[API] No %s price available\n", fuelType.c_str());
  }

  return res;
}

float apiFetchPrice(const String& apiKey, const String& stationId, const String& fuelType) {
  PriceData data = apiFetchPriceData(apiKey, stationId, fuelType);
  if (data.ok && data.isOpen) {
    return data.price;
  }
  return -1.0f;
}

bool apiFetchAllPrices(const String& apiKey, const Favorite* favs, int favCount, PriceData* outPrices) {
  for (int i = 0; i < favCount; i++) {
    outPrices[i] = { false, false, -1.0f };
  }

  String idList = "";
  int activeCount = 0;
  for (int i = 0; i < favCount; i++) {
    if (favs[i].active && !favs[i].id.isEmpty()) {
      activeCount++;
      if (idList.indexOf(favs[i].id) == -1) {
        if (!idList.isEmpty()) idList += ",";
        idList += favs[i].id;
      }
    }
  }

  if (activeCount == 0 || idList.isEmpty()) {
    Serial.println(F("[API] No active favorites to fetch"));
    return false;
  }

  String url = String(API_BASE_URL) + "prices.php?ids=" + idList + "&apikey=" + apiKey;
  String payload = httpGet(url);
  if (payload.isEmpty()) return false;

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, payload);
  if (err) {
    Serial.printf("[API] Batch prices JSON error: %s\n", err.c_str());
    return false;
  }

  if (!(doc["ok"].as<bool>())) {
    Serial.printf("[API] Batch API error: %s\n", doc["message"].as<const char*>());
    return false;
  }

  JsonObject pricesObj = doc["prices"].as<JsonObject>();
  for (int i = 0; i < favCount; i++) {
    if (!favs[i].active || favs[i].id.isEmpty()) continue;
    JsonObject station = pricesObj[favs[i].id];
    if (!station.isNull()) {
      outPrices[i].ok = true;
      String status = station["status"] | "closed";
      outPrices[i].isOpen = (status == "open");
      if (outPrices[i].isOpen) {
        outPrices[i].price = station[favs[i].fuel] | -1.0f;
        Serial.printf("[API] Fav #%d (%s, %s): %.3f (open)\n",
                      i + 1, favs[i].name.c_str(), favs[i].fuel.c_str(), outPrices[i].price);
      } else {
        outPrices[i].price = -1.0f;
        Serial.printf("[API] Fav #%d (%s): CLOSED\n", i + 1, favs[i].name.c_str());
      }
    }
  }

  return true;
}

bool geocodePLZ(const String& plz, float& outLat, float& outLng) {
  std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
  client->setInsecure();
  client->setBufferSizes(16384, 512);

  HTTPClient http;
  http.setTimeout(8000);
  http.setUserAgent("Mozilla/5.0 (ESP8266-Tankpreis)");

  String url = "https://photon.komoot.io/api/?q=" + plz + "+Germany&limit=1";
  Serial.printf("[API] Geocoding PLZ via Photon: %s\n", plz.c_str());

  if (!http.begin(*client, url)) {
    Serial.println(F("[API] Geocoding connect failed"));
    return false;
  }

  int code = http.GET();
  if (code != HTTP_CODE_OK) {
    Serial.printf("[API] Geocoding HTTP error: %d\n", code);
    http.end();
    return false;
  }

  String payload = http.getString();
  http.end();

  if (payload.isEmpty()) return false;

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, payload);
  if (err) {
    Serial.printf("[API] Geocode JSON error: %s\n", err.c_str());
    return false;
  }

  JsonArray features = doc["features"].as<JsonArray>();
  if (features.size() == 0) {
    Serial.println(F("[API] Geocode: PLZ not found"));
    return false;
  }

  JsonArray coords = features[0]["geometry"]["coordinates"].as<JsonArray>();
  if (coords.size() < 2) return false;

  // Photon format: coordinates = [longitude, latitude]
  outLng = coords[0].as<float>();
  outLat = coords[1].as<float>();

  Serial.printf("[API] Geocoded PLZ %s -> lat=%.4f, lng=%.4f\n", plz.c_str(), outLat, outLng);
  return true;
}

String geocodePLZ(const String& plz) {
  float lat = 0, lng = 0;
  if (geocodePLZ(plz, lat, lng)) {
    return "[{\"lat\":\"" + String(lat, 6) + "\",\"lon\":\"" + String(lng, 6) + "\"}]";
  }
  return "[]";
}

bool apiStreamStationDetail(const String& apiKey, const String& stationId, ESP8266WebServer& srv) {
  String url = String(API_BASE_URL) + "detail.php?id="
    + stationId + "&apikey=" + apiKey;
  return apiStreamHttpGet(url, srv);
}

String apiGetStationDetail(const String& apiKey, const String& stationId) {
  String url = String(API_BASE_URL) + "detail.php?id="
    + stationId + "&apikey=" + apiKey;
  return httpGet(url);
}
