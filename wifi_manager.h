#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <IPAddress.h>
#include "config.h"

void wifiInit();
void wifiStartAP();
bool wifiConnect(const String& ssid, const String& pass, int timeoutSec = WIFI_CONNECT_TIMEOUT);
bool wifiConnectMulti(const WifiCredential* networks, int count, int timeoutSec = WIFI_CONNECT_TIMEOUT);
bool wifiIsConnected();
String wifiGetSSID();
String wifiGetIP();
IPAddress wifiGetIPAddress();
void wifiStartScan();
int wifiScanStatus();
String wifiGetScanResultsJson();
String wifiScanNetworks();  // Legacy blocking scan
void wifiLoop();            // Handle DNS for captive portal

#endif // WIFI_MANAGER_H
