// Main program to build the application
// Has main(); does initialization and cleanup and perhaps some basic logic.

#include <stdio.h>
#include "time.h"
#include "stdbool.h"
#include <stdlib.h>
#include <pthread.h>
#include "hal/segDisplay.h"
#include "hal/joystick.h"
#include "hal/neoPixel.h"

//static bool isRunning = true; // used to keep track of all busy-wait thread loops


int main()
{

    /*
        Initialize HAL
    */
    //initSegDisplay(&isRunning);
    //joystick_init(&isRunning); // Instead of using GPIO we need to get it using PRU for faster access
    initNeoPixel();

    neoPixelState pixels = {0}; // Initialize all elements to 0 (false) by default

    // Set the 5th index to be bright blue
    pixels[4].isOn = true;
    pixels[4].isBright = true;
    pixels[4].color = BLUE;

    // Set the 4th and 6th indices to be blue but not bright
    pixels[3].isOn = true;
    pixels[3].isBright = false;
    pixels[3].color = BLUE;

    pixels[5].isOn = true;
    pixels[5].isBright = false;
    pixels[5].color = BLUE;

    // Print out the state of the neoPixelState array
    for (int i = 0; i < 8; ++i) {
        printf("Index %d: isOn=%s, isBright=%s, 0x%08x\n", 
               i, 
               pixels[i].isOn ? "true" : "false",
               pixels[i].isBright ? "true" : "false",
               pixels[i].color);
    }
    setLeds(pixels);
    printf("anan");
    // demonstration of 
    
    /*
        Cleanup HAL
    */
    //cleanupSegDisplay();
    //joystick_shutdown();

    return 0;

}