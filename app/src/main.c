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

    initSegDisplay(&isRunning);
    // joystick_init(&isRunning); // Instead of using GPIO we need to get it using PRU for faster access
    printf("Running main...\n"); //TODO remove
    accel_init();
    initNeoPixel();
    joystick_initPRU(&isRunning);
    
    //TODO buzzer init
    //TODO 14Seg init
    //TODO PRU isRunning variable

    // Initialize game logic
    game_init(&isRunning);

    /*neoPixelState pixels = {0}; // Initialize all elements to 0 (false) by default

    pixels[0].isOn = true;
    pixels[0].isBright = false;
    pixels[0].color = RED;

    pixels[1].isOn = true;
    pixels[1].isBright = false;
    pixels[1].color = GREEN;

    pixels[2].isOn = true;
    pixels[2].isBright = false;
    pixels[2].color = BLUE;

    pixels[3].isOn = true;
    pixels[3].isBright = false;
    pixels[3].color = RED;

    pixels[4].isOn = true;
    pixels[4].isBright = false;
    pixels[4].color = GREEN;

    pixels[5].isOn = true;
    pixels[5].isBright = false;
    pixels[5].color = BLUE;

    pixels[6].isOn = true;
    pixels[6].isBright = false;
    pixels[6].color = RED;

    pixels[7].isOn = true;
    pixels[7].isBright = false;
    pixels[7].color = GREEN;
    */


    // Visualization
    /*for (int i = 0; i < 8; ++i) {
        printf("Index %d: isOn=%s, isBright=%s, 0x%08x\n", 
               i, 
               pixels[i].isOn ? "true" : "false",
               pixels[i].isBright ? "true" : "false",
               pixels[i].color);
    }*/
    // setLeds(pixels);

    // Random pause
    sleepForMs(30000);

    // This is just a loop for test prints
    // sleepForMs(500);
    // for (int i = 0; i < 1000; i++) {

    //   Testing read values
    //   struct accel_val acc = accel_get();
    //   directions dir = getJoystickDirection();
    //   printf("DOWN %d\tRIGHT %d\tX\t%.2f\tY\t%.2f\n", dir==DOWN, dir==RIGHT, (double)acc.x / ACCEL_MAX, (double)acc.y / ACCEL_MAX);

      // Wait
    //   sleepForMs(50);

    // }

    /*
        Cleanup HAL
    */

    game_cleanup();
    accel_cleanup();
    cleanupSegDisplay();
    joystick_shutdown();

    return 0;

}