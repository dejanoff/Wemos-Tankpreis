#ifndef API_CLIENT_H
#define API_CLIENT_H

#include <Arduino.h>
#include <ESP8266WebServer.h>

// Stream gas stations search response in chunks directly to client (prevents RAM exhaustion)
bool apiStreamSearchStations(const String& apiKey, float lat, float lng, float radius, ESP8266WebServer& srv);
String apiSearchStations(const String& apiKey, float lat, float lng, float radius);

struct PriceData {
  bool ok;
  bool isOpen;
  float price;
};

#include "config.h"

// Fetch price and open status for a specific station and fuel type
PriceData apiFetchPriceData(const String& apiKey, const String& stationId, const String& fuelType);

// Fetch prices for all active favorites in a single request
bool apiFetchAllPrices(const String& apiKey, const Favorite* favs, int favCount, PriceData* outPrices);

// Backward compatible helper returning price, or -1.0 on error/closed
float apiFetchPrice(const String& apiKey, const String& stationId, const String& fuelType);

// Geocode a German postal code (PLZ) via Photon (OpenStreetMap)
bool geocodePLZ(const String& plz, float& outLat, float& outLng);
String geocodePLZ(const String& plz); // Legacy fallback

// Get detailed station info by ID
bool apiStreamStationDetail(const String& apiKey, const String& stationId, ESP8266WebServer& srv);
String apiGetStationDetail(const String& apiKey, const String& stationId);

#endif // API_CLIENT_H
