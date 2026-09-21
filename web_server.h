#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include "config.h"

// Callback types
typedef void (*ConfigChangeCallback)();

void webServerInit(AppConfig* config);
void webServerLoop();
void webServerSetConfigCallback(ConfigChangeCallback cb);
void webServerSetPrice(float price);
void webServerSetFavoritePrices(const float* prices, int count);
void triggerWifiConnect(const String& ssid, const String& pass);
bool webServerHasPendingConnect();
void webServerGetPendingConnect(String& ssid, String& pass);

#endif // WEB_SERVER_H
