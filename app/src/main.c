// Main program to build the application
// Has main(); does initialization and cleanup and perhaps some basic logic.

#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>

#include "hal/infoPrinter.h"
#include "hal/segDisplay.h"
#include "hal/joystick.h"
#include "hal/neoPixel.h"
#include "hal/accelerometer.h"
#include "hal/utils.h"
#include "hal/pwmBuzzer.h"
#include "hal/shutdown.h"

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
    initBuzzer(&isRunning);
    accel_init();

    /* 
    Initialize game logic
    */
    printer_init(&isRunning);
    game_init(&isRunning);
    shutdown_init(&isRunning);

    /* Wait for user to signal shutdown */
    shutdown_wait();

    /*
        Cleanup HAL
    */

    shutdown_cleanup();
    printer_cleanup();
    neopixel_cleanup();
    game_cleanup();
    buzzerCleanup();
    accel_cleanup();
    cleanupSegDisplay();
    joystick_shutdown();

    return 0;

}