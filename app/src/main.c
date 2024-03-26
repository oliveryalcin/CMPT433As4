// Main program to build the application
// Has main(); does initialization and cleanup and perhaps some basic logic.

#include <stdio.h>
#include "time.h"
#include "stdbool.h"
#include <stdlib.h>
#include <pthread.h>
#include "hal/segDisplay.h"
#include "hal/joystick.h"


static bool isRunning = true; // used to keep track of all busy-wait thread loops


int main()
{

    /*
        Initialize HAL
    */
    initSegDisplay(&isRunning);
    joystick_init(&isRunning); // Instead of using GPIO we need to get it using PRU for faster access

 
    
    /*
        Cleanup HAL
    */
    cleanupSegDisplay();
    joystick_shutdown();

    return 0;

}