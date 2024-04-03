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

static uint32_t brightenColor(ledColors color);
/*
    Initializes pin for PRUOUT and initializes shared struct 
*/
void initNeoPixel(){
    runCommand("config-pin P8.11 pruout");
    //runCommand("cd /sys/class/remoteproc/remoteproc1/ && echo 'start' | sudo tee ./state");
    //runCommand("cd /mnt/remote/pru/pru-as4 && make && sudo make install_PRU0");
    pPruBase = getPruMmapAddr();
    pSharedPru0 = PRU0_MEM_FROM_BASE(pPruBase);
}

// Returns the bright version of the color based on the enum
uint32_t brightenColor(ledColors color) {
    
    switch (color) {
        case RED:
            return 0x00ff0000; // Red Bright
        case GREEN:
            return 0xff000000; // Green Bright
        case BLUE:
            return 0x0000ff00; // Blue Bright
        default:
            return 0x00000000;
    }
}

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
        printf("\nShared Mem after calling setLed %d: 0x%08x\n", i, pSharedPru0->color[i]);
    }
    freePruMmapAddr(pPruBase); //move to shutdown function
}