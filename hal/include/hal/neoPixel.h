
#ifndef _NEO_PIXEL_H_
#define _NEO_PIXEL_H_

#include <stdbool.h>
#include "../../pru-as4/sharedDataStruct.h"

#define NEO_LED_BASE 0x00000000

typedef enum {
    RED = 0x00010000, // Red
    GREEN = 0x01000000, // Green
    BLUE = 0x00000100, // Blue
} ledColors;

typedef struct {
    bool isOn; // either on or off
    bool isBright;
    ledColors color;
} ledIndexer;

// this is now a special type of led positions of length 8 in total.
typedef ledIndexer neoPixelState[8];
void initNeoPixel();
uint32_t brightenColor(ledColors color);

uint32_t setSingleColorBrightness(uint32_t current, ledColors color, int brightness);
int getSingleColorBrightness(uint32_t current, ledColors color);
uint32_t crementSingleColorBrightness(uint32_t current, ledColors color, int amount);
uint32_t setAllColorBrightness(uint32_t current, int brightness);
uint32_t setLedColor(int red, int green, int blue);

void setLeds(neoPixelState positions);
void setLedSimple(neoPixelState positions);

void neopixel_cleanup();


#endif