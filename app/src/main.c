// Main program to build the application
// Has main(); does initialization and cleanup and perhaps some basic logic.

#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>

#include "hal/segDisplay.h"
#include "hal/joystick.h"
#include "hal/neoPixel.h"
#include "hal/accelerometer.h"
#include "hal/utils.h" // TODO remove ??

#include "gameLogic.h"

static bool isRunning = true; // used to keep track of all busy-wait thread loops


int main()
{

    /*
        Initialize HAL
    */
    
    initNeoPixel();
    joystick_initPRU(&isRunning);

    initSegDisplay(&isRunning);

    accel_init();

    //TODO buzzer init
    //TODO PRU isRunning variable

    // Initialize game logic
    game_init(&isRunning);



    /*
        Cleanup HAL
    */

    game_cleanup();
    accel_cleanup();
    cleanupSegDisplay();
    joystick_shutdown();

    return 0;

}