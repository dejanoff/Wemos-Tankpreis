#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <IPAddress.h>
#include "config.h"

void displayInit(uint8_t brightness = DEFAULT_BRIGHTNESS);
void displayShowPrice(float price, int formatMode = 0);
void displayShowLoading();
void displayShowError();
void displayShowFavIndex(int idx);
void displaySetBrightness(uint8_t level);
void displayClear();
void displayShowAP();
void displayShowIP(IPAddress ip);
void displayPerimeterStep(uint8_t step);
void displayScrollFuel(const String& fuelType, uint16_t stepDelayMs = 260);
void displayLampTest();

// Clock, Trend, Closed & Smooth Fade
void displayShowTime(int hours, int minutes, bool showColon);
void displayShowClosed();
void displayShowTrend(int trend); // +1 = UP, -1 = dn
void displayFadeOut(uint8_t currentLevel, uint8_t stepDelayMs = 25);
void displayFadeIn(uint8_t targetLevel, uint8_t stepDelayMs = 25);
void displayOff();

#endif // DISPLAY_MANAGER_H
