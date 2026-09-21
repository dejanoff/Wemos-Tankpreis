#include "wifi_manager.h"
#include "config.h"
#include "display_manager.h"
#include <ESP8266WiFi.h>
#include <DNSServer.h>

static DNSServer dnsServer;
static bool apMode = false;
static const byte DNS_PORT = 53;

void wifiInit() {
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
}

void wifiStartAP() {
  WiFi.disconnect();
  delay(100);
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(AP_SSID, AP_PASS);

  // Captive portal: redirect all DNS queries to our IP
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
  apMode = true;

  Serial.print(F("[WiFi] AP started, IP: "));
  Serial.println(WiFi.softAPIP());
}

bool wifiConnect(const String& ssid, const String& pass, int timeoutSec) {
  Serial.printf("[WiFi] Connecting to %s", ssid.c_str());

  WiFi.begin(ssid.c_str(), pass.c_str());

  unsigned long start = millis();
  unsigned long timeoutMs = (unsigned long)timeoutSec * 1000UL;
  uint8_t animStep = 0;

  while (WiFi.status() != WL_CONNECTED && (millis() - start < timeoutMs)) {
    displayPerimeterStep(animStep++);
    delay(75);
    yield();
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print(F("[WiFi] Connected! IP: "));
    Serial.println(WiFi.localIP());
    return true;
  }

  Serial.println(F("[WiFi] Connection failed"));
  return false;
}

bool wifiConnectMulti(const WifiCredential* networks, int count, int timeoutSec) {
  if (count <= 0) return false;

  Serial.printf("[WiFi] Scanning for %d saved networks...\n", count);

  // Scan available networks to find best match
  int n = WiFi.scanNetworks();
  int bestMatch = -1;
  int bestRSSI = -1000;

  for (int i = 0; i < n; i++) {
    String foundSsid = WiFi.SSID(i);
    for (int j = 0; j < count; j++) {
      if (foundSsid == networks[j].ssid) {
        int rssi = WiFi.RSSI(i);
        if (rssi > bestRSSI) {
          bestRSSI = rssi;
          bestMatch = j;
        }
      }
    }
  }
  WiFi.scanDelete();

  // If a matching network was found nearby, try that first
  if (bestMatch >= 0) {
    Serial.printf("[WiFi] Best nearby match: %s (RSSI: %d)\n",
                  networks[bestMatch].ssid.c_str(), bestRSSI);
    if (wifiConnect(networks[bestMatch].ssid, networks[bestMatch].pass, timeoutSec)) {
      return true;
    }
  }

  // Fallback: try all saved networks in sequence
  for (int j = 0; j < count; j++) {
    if (j == bestMatch) continue;
    if (wifiConnect(networks[j].ssid, networks[j].pass, 8)) {
      return true;
    }
  }

  return false;
}

bool wifiIsConnected() {
  return WiFi.status() == WL_CONNECTED;
}

String wifiGetSSID() {
  if (WiFi.status() == WL_CONNECTED) {
    return WiFi.SSID();
  }
  return "";
}

String wifiGetIP() {
  if (WiFi.status() == WL_CONNECTED) {
    return WiFi.localIP().toString();
  }
  return WiFi.softAPIP().toString();
}

IPAddress wifiGetIPAddress() {
  if (WiFi.status() == WL_CONNECTED) {
    return WiFi.localIP();
  }
  return WiFi.softAPIP();
}

void wifiStartScan() {
  int status = WiFi.scanComplete();
  if (status == WIFI_SCAN_RUNNING) return;
  WiFi.scanDelete();
  WiFi.scanNetworks(true); // Asynchronous scan!
}

int wifiScanStatus() {
  return WiFi.scanComplete();
}

String wifiGetScanResultsJson() {
  int n = WiFi.scanComplete();
  if (n < 0) return "[]";

  String json = "[";
  for (int i = 0; i < n; i++) {
    if (i > 0) json += ",";
    json += "{\"ssid\":\"";
    String ssid = WiFi.SSID(i);
    ssid.replace("\\", "\\\\");
    ssid.replace("\"", "\\\"");
    json += ssid;
    json += "\",\"rssi\":";
    json += String(WiFi.RSSI(i));
    json += ",\"enc\":";
    json += (WiFi.encryptionType(i) != ENC_TYPE_NONE) ? "1" : "0";
    json += "}";
  }
  json += "]";
  WiFi.scanDelete();
  return json;
}

String wifiScanNetworks() {
  wifiStartScan();
  unsigned long st = millis();
  while (WiFi.scanComplete() == WIFI_SCAN_RUNNING && (millis() - st < 4000)) {
    delay(50);
    yield();
  }
  return wifiGetScanResultsJson();
}

void wifiLoop() {
  if (apMode) {
    dnsServer.processNextRequest();
  }
}
