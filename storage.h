#ifndef STORAGE_H
#define STORAGE_H

#include "config.h"

bool storageInit();
bool loadConfig(AppConfig& config);
bool saveConfig(const AppConfig& config);

#endif // STORAGE_H
