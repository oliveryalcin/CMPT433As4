
#ifndef _NEO_PIXEL_H_
#define _NEO_PIXEL_H_

#include "stdbool.h"
typedef enum {
    RED = 0x000f0000, // Red
    GREEN = 0x0f000000, // Green
    BLUE = 0x00000f00, // Blue
} ledColors;

typedef struct {
    bool isOn; // either on or off
    bool isBright;
    ledColors color;
} ledIndexer;

// this is now a special type of led positions of length 8 in total.
typedef ledIndexer neoPixelState[8];
void initNeoPixel();
void setLeds(neoPixelState positions);


#endif