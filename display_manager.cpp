#include "display_manager.h"
#include "config.h"
#include <TM1637Display.h>

static TM1637Display display(TM1637_CLK, TM1637_DIO);
static uint8_t currentSegments[4] = { 0, 0, 0, 0 };

static void setSegmentsInternal(const uint8_t segs[4]) {
  memcpy(currentSegments, segs, 4);
  display.setSegments(segs);
}

// Letter segment definitions using library constants SEG_A..SEG_G
#define LETTER_E (SEG_A | SEG_D | SEG_E | SEG_F | SEG_G)
#define LETTER_r (SEG_E | SEG_G)
#define LETTER_A (SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G)
#define LETTER_P (SEG_A | SEG_B | SEG_E | SEG_F | SEG_G)
#define LETTER_F (SEG_A | SEG_E | SEG_F | SEG_G)
#define LETTER_DASH SEG_G

void displayInit(uint8_t brightness) {
  display.setBrightness(brightness & 0x07, true);
  display.clear();
  memset(currentSegments, 0, 4);
}

void displayShowPrice(float price, int formatMode) {
  if (price < 0.0f) {
    displayShowError();
    return;
  }

  uint8_t segs[4] = { 0, 0, 0, 0 };

  if (formatMode >= 1 && formatMode <= 3) {
    // Mode 1: Rounded with underscore "2_38" (SEG_D)
    // Mode 2: Rounded with middle dash "2-38" (SEG_G)
    // Mode 3: Rounded without separator "2 38" (blank)
    int rounded = (int)(price * 100.0f + 0.5f);
    int d0 = (rounded / 100) % 10;
    int d2 = (rounded / 10) % 10;
    int d3 = rounded % 10;

    segs[0] = display.encodeDigit(d0);
    if (formatMode == 1) {
      segs[1] = SEG_D;
    } else if (formatMode == 2) {
      segs[1] = SEG_G;
    } else {
      segs[1] = 0x00;
    }
    segs[2] = display.encodeDigit(d2);
    segs[3] = display.encodeDigit(d3);
  } else {
    // Mode 0: Exact 4 digits, e.g. 2.379 -> "2379"
    if (price < 10.0f) {
      int intPart = (int)price;
      int fracPart = (int)((price - (float)intPart) * 1000.0f + 0.5f);
      if (fracPart >= 1000) {
        intPart++;
        fracPart -= 1000;
      }

      int d0 = intPart % 10;
      int d1 = (fracPart / 100) % 10;
      int d2 = (fracPart / 10) % 10;
      int d3 = fracPart % 10;

      segs[0] = display.encodeDigit(d0) | 0x80;
      segs[1] = display.encodeDigit(d1);
      segs[2] = display.encodeDigit(d2);
      segs[3] = display.encodeDigit(d3);
    } else {
      int total = (int)(price * 100.0f + 0.5f);
      int d0 = (total / 1000) % 10;
      int d1 = (total / 100) % 10;
      int d2 = (total / 10) % 10;
      int d3 = total % 10;

      segs[0] = display.encodeDigit(d0);
      segs[1] = display.encodeDigit(d1) | 0x80;
      segs[2] = display.encodeDigit(d2);
      segs[3] = display.encodeDigit(d3);
    }
  }

  setSegmentsInternal(segs);
}

void displayShowIP(IPAddress ip) {
  // Show "IP  "
  uint8_t ipHeader[] = {
    (SEG_B | SEG_C),                         // I
    (SEG_A | SEG_B | SEG_E | SEG_F | SEG_G), // P
    0x00,
    0x00
  };
  setSegmentsInternal(ipHeader);
  delay(600);
  yield();

  // Show each of the 4 octets sequentially
  for (int o = 0; o < 4; o++) {
    int val = ip[o];
    uint8_t segs[4] = { 0x00, 0x00, 0x00, 0x00 };
    if (val >= 100) {
      segs[0] = display.encodeDigit(val / 100);
      segs[1] = display.encodeDigit((val / 10) % 10);
      segs[2] = display.encodeDigit(val % 10);
    } else if (val >= 10) {
      segs[1] = display.encodeDigit(val / 10);
      segs[2] = display.encodeDigit(val % 10);
    } else {
      segs[2] = display.encodeDigit(val);
    }
    setSegmentsInternal(segs);
    delay(o == 3 ? 1200 : 700);
    yield();
  }
}

void displayShowLoading() {
  uint8_t segs[] = { LETTER_DASH, LETTER_DASH, LETTER_DASH, LETTER_DASH };
  setSegmentsInternal(segs);
}

void displayShowError() {
  uint8_t segs[] = { LETTER_E, LETTER_r, LETTER_r, 0x00 };
  setSegmentsInternal(segs);
}

void displayShowFavIndex(int idx) {
  int num = idx + 1;
  uint8_t segs[4] = { LETTER_F, 0x00, 0x00, 0x00 };

  if (num < 10) {
    segs[3] = display.encodeDigit(num);
  } else {
    segs[2] = display.encodeDigit(num / 10);
    segs[3] = display.encodeDigit(num % 10);
  }
  setSegmentsInternal(segs);
}

void displaySetBrightness(uint8_t level) {
  display.setBrightness(level & 0x07, level > 0);
  display.setSegments(currentSegments);
}

void displayClear() {
  display.clear();
  memset(currentSegments, 0, 4);
}

void displayShowAP() {
  uint8_t segs[] = { LETTER_A, LETTER_P, 0x00, 0x00 };
  setSegmentsInternal(segs);
}

static uint8_t charToSegment(char c) {
  switch (c) {
    case ' ': return 0x00;
    case 'd': case 'D': return (SEG_B | SEG_C | SEG_D | SEG_E | SEG_G); // 0x5E 'd'
    case 'I': case 'i': case '1': return (SEG_B | SEG_C);                // 0x06 'I'/'1'
    case 'E': case 'e': return (SEG_A | SEG_D | SEG_E | SEG_F | SEG_G); // 0x79 'E'
    case 'S': case 's': case '5': return (SEG_A | SEG_C | SEG_D | SEG_F | SEG_G); // 0x6D '5'/'S'
    case 'L': case 'l': return (SEG_D | SEG_E | SEG_F);                 // 0x38 'L'
    case 'b': case 'B': return (SEG_C | SEG_D | SEG_E | SEG_F | SEG_G); // 0x7C 'b'
    case 'n': case 'N': return (SEG_C | SEG_E | SEG_G);                 // 0x54 'n'
    case 'Z': case 'z': case '2': return (SEG_A | SEG_B | SEG_D | SEG_E | SEG_G); // 0x5B '2'/'Z'
    case '0': return (SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F);   // 0x3F '0'
    default: return 0x00;
  }
}

struct PerimeterStep {
  uint8_t digit;
  uint8_t segment;
};

static const PerimeterStep PERIMETER_STEPS[12] = {
  { 0, SEG_A }, // Top edge (left to right)
  { 1, SEG_A },
  { 2, SEG_A },
  { 3, SEG_A },
  { 3, SEG_B }, // Right edge (top to bottom)
  { 3, SEG_C },
  { 3, SEG_D }, // Bottom edge (right to left)
  { 2, SEG_D },
  { 1, SEG_D },
  { 0, SEG_D },
  { 0, SEG_E }, // Left edge (bottom to top)
  { 0, SEG_F }
};

void displayPerimeterStep(uint8_t step) {
  uint8_t segs[4] = { 0, 0, 0, 0 };
  uint8_t head = step % 12;
  uint8_t tail = (step + 11) % 12;
  segs[PERIMETER_STEPS[head].digit] |= PERIMETER_STEPS[head].segment;
  segs[PERIMETER_STEPS[tail].digit] |= PERIMETER_STEPS[tail].segment;
  setSegmentsInternal(segs);
}

void displayLampTest() {
  uint8_t allOn[] = { 0xFF, 0xFF, 0xFF, 0xFF };
  setSegmentsInternal(allOn);
}

void displayScrollFuel(const String& fuelType, uint16_t stepDelayMs) {
  String text;
  if (fuelType.equalsIgnoreCase("diesel")) {
    text = "   dIE5EL   ";
  } else if (fuelType.equalsIgnoreCase("e10")) {
    text = "bEn21n E10   ";
  } else {
    text = "bEn21n E5   ";
  }

  int len = text.length();
  for (int i = 0; i <= len - 4; i++) {
    uint8_t segs[4];
    for (int j = 0; j < 4; j++) {
      segs[j] = charToSegment(text.charAt(i + j));
    }
    setSegmentsInternal(segs);

    unsigned long start = millis();
    while (millis() - start < stepDelayMs) {
      yield();
      delay(10);
    }
  }
}

void displayShowTime(int hours, int minutes, bool showColon) {
  uint8_t segs[4];
  segs[0] = (hours >= 10) ? display.encodeDigit(hours / 10) : 0x00;
  segs[1] = display.encodeDigit(hours % 10);
  if (showColon) {
    segs[1] |= 0x80; // Center colon on digit 1
  }
  segs[2] = display.encodeDigit(minutes / 10);
  segs[3] = display.encodeDigit(minutes % 10);
  setSegmentsInternal(segs);
}

void displayShowClosed() {
  uint8_t segs[4] = {
    (SEG_A | SEG_D | SEG_E | SEG_F),                  // C (0x39)
    (SEG_D | SEG_E | SEG_F),                          // L (0x38)
    (SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F),  // O (0x3F)
    (SEG_A | SEG_C | SEG_D | SEG_F | SEG_G)           // S (0x6D)
  };
  setSegmentsInternal(segs);
}

void displayShowTrend(int trend) {
  uint8_t segs[4] = { 0, 0, 0, 0 };
  if (trend > 0) {
    // "  UP"
    segs[2] = (SEG_B | SEG_C | SEG_D | SEG_E | SEG_F); // U
    segs[3] = (SEG_A | SEG_B | SEG_E | SEG_F | SEG_G); // P
  } else if (trend < 0) {
    // "  dn"
    segs[2] = (SEG_B | SEG_C | SEG_D | SEG_E | SEG_G); // d
    segs[3] = (SEG_C | SEG_E | SEG_G);                 // n
  }
  setSegmentsInternal(segs);
}

void displayFadeOut(uint8_t currentLevel, uint8_t stepDelayMs) {
  if (currentLevel > 7) currentLevel = 7;
  for (int b = (int)currentLevel; b >= 0; b--) {
    display.setBrightness(b, true);
    display.setSegments(currentSegments);
    delay(stepDelayMs);
  }
  display.setBrightness(0, false);
  display.setSegments(currentSegments);
}

void displayFadeIn(uint8_t targetLevel, uint8_t stepDelayMs) {
  if (targetLevel > 7) targetLevel = 7;
  if (targetLevel == 0) {
    display.setBrightness(0, false);
    display.setSegments(currentSegments);
    return;
  }
  for (int b = 0; b <= (int)targetLevel; b++) {
    display.setBrightness(b, true);
    display.setSegments(currentSegments);
    delay(stepDelayMs);
  }
}

void displayOff() {
  display.setBrightness(0, false);
  display.setSegments(currentSegments);
}
