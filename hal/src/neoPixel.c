#include "hal/neoPixel.h"
#include "hal/utils.h"
#include <stdbool.h>
#include "../pru-as4/sharedDataStruct.h"

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "hal/utils.h"

// add init which configures pins.
// add function that sets leds

/*
    Configures GPIO pins for PRU 
*/ 

static volatile void *pPruBase;
static volatile sharedColorStruct_t *pSharedPru0;

/*
    Initializes pin for PRUOUT and initializes shared struct 
*/
void initNeoPixel(){
    runCommand("config-pin P8.11 pruout");
    runCommand("cd /sys/class/remoteproc/remoteproc1/ && echo 'start' | sudo tee ./state");
    runCommand("cd /mnt/remote/pru/pru-as4 && make && sudo make install_PRU0");
    pPruBase = getPruMmapAddr();
    pSharedPru0 = PRU0_MEM_FROM_BASE(pPruBase);
}

// Returns the bright version of the color based on the enum
uint32_t brightenColor(ledColors color) {
    
    switch (color) {
        case RED:
            return 0x000f0000; // Red Bright
        case GREEN:
            return 0x0f000000; // Green Bright
        case BLUE:
            return 0x00000f00; // Blue Bright
        default:
            return 0x00000000;
    }
}

/* Generate a bit string for the appropriate color set to a specific value */
uint32_t setSingleColorBrightness(uint32_t current, ledColors color, int brightness) {

    if (brightness < 0 || brightness > 255) {
        printf("ERROR: Brightness not in range 0-255\n");
        return current;
    }

    // Get current values
    uint32_t curRed = (current >> 16) & 0xFF;
    uint32_t curGreen = (current >> 24) & 0xFF;
    uint32_t curBlue = (current >> 8) & 0xFF;
    
    // Update brightness
    switch (color) {
      case RED:
        curRed = brightness;
        break;
      case GREEN:
        curGreen = brightness;
        break;
      case BLUE:
        curBlue = brightness;
        break;
      default:
        printf("ERROR setSingleColorBrightness: Invalid color\n");
        return current;
    }

    // Combine colours and return
    uint32_t newCol = (curGreen << 24) | (curRed << 16) | (curBlue << 8);
    return newCol;
}

/* Get the brightness level (0-255) for the specified color */
int getSingleColorBrightness(uint32_t current, ledColors color) {
    switch (color) {
        case RED:
            return (current >> 16) & 0xFF;
        case GREEN:
            return (current >> 24) & 0xFF;
        case BLUE:
            return (current >> 8) & 0xFF;
        default:
            printf("ERROR getSingleColorBrightness: invalid color\n");
            return 0;
    }
}

/* Increment/decrement brightness of one colour by a specific amount */
//TODO needs to be tested
uint32_t crementSingleColorBrightness(uint32_t current, ledColors color, int amount) {
    int brightness = getSingleColorBrightness(current, color);
    int newBrightness = brightness + amount;

    // Map to 0-255
    if (newBrightness < 0) {
        newBrightness = 0;
    } else if (newBrightness > 255) {
        newBrightness = 255;
    }

    // Set new brightness
    return setSingleColorBrightness(current, color, newBrightness);
}

/* Set all color brightnesses at the same time */
//TODO I just realized this is kind of useless since its just white lol
//TODO needs to be tested
uint32_t setAllColorBrightness(uint32_t current, int brightness) {
    if (brightness < 0 || brightness > 255) {
        printf("ERROR: Brightness not in range 0-255\n");
        return current;
    }

    // Set all colours to the same brightness
    uint32_t newCol = (brightness << 24) | (brightness << 16) | (brightness << 8);
    return newCol;
}

/* Get an LED bit string that represents the given (R,G,B) value */
uint32_t setLedColor(int red, int green, int blue) {

    // Input validation
    if (red < 0 || red > 255 || green < 0 || green > 255 || blue < 0 || blue > 255) {
        printf("ERROR setLedColor: RGB values not in range 0-255\n");
        return 0x00000000;
    }

    // Construct appropriate color
    uint32_t col = ((uint32_t)green << 24) | ((uint32_t)red << 16) | ((uint32_t)blue << 8);
    return col;

}

/* Original function to set LEDs */
void setLeds(neoPixelState positions) {
        

    for (int i = 0; i < STR_LEN; i++) {
        if (positions[i].isOn) {
            if (positions[i].isBright) {
                // Make LED Brighter
                pSharedPru0->color[i] = brightenColor(positions[i].color);
            } else {
                // Set the LED
                pSharedPru0->color[i] = positions[i].color;
            }
        } else {
            // Turn off
            pSharedPru0->color[i] = 0x00000000;
        }
    }
    for (int i = 0; i < STR_LEN; i++) {
        // printf("\nShared Mem after calling setLed %d: 0x%08x\n", i, pSharedPru0->color[i]); //TODO delete
    }
    // freePruMmapAddr(pPruBase); //move to shutdown function //TODO
}

/* Simpler function that only changes color value directly */
void setLedSimple(neoPixelState positions) {
    for (int i = 0; i < STR_LEN; i++) {
        pSharedPru0->color[i] = positions[i].color;
    }
}

void neopixel_cleanup() {
    for (int i = 0; i < STR_LEN; i++) {
        pSharedPru0->color[i] = NEO_LED_BASE;
    }
}