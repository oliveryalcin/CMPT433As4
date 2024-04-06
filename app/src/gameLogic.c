#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdlib.h> // rand()

#include "hal/segDisplay.h"
#include "hal/joystick.h"
#include "hal/neoPixel.h"
#include "hal/accelerometer.h"
#include "hal/utils.h" // sleepForMs()

#include "gameLogic.h"

/* Variables */
//TODO I don't think they need to be Atomic, but maybe?
static double gameX;
static double gameY;

// Thread stuff
static pthread_t gameThread;
static void* gameLoop();
static bool* isRunning; // keep track of shutdown state, initial value passed from MAIN thread during initialization
static int mapYtoLEDIndex(double cur_y);
static ledColors mapXtoLEDColor(double cur_x);
static double generateNum();
static void refreshCoords();

void game_init(bool *flag) {

    // Pass in the flag to later signal shutdown
    isRunning = flag;

    printf("Initializing game...\n");
    pthread_create(&gameThread, NULL, &gameLoop, NULL);

}

// Function to map Y accelerometer value to an LED index
static int mapYtoLEDIndex(double cur_y) {

    // Normalize Y to [0.0, 1.0]
    double normalized = (cur_y + 1.0) / 2.0;

    // Scale to [0, 7]
    double scaled = normalized * 7.0;

    // Get LED index
    int ledIndex = (int)scaled;

    // Ensure index within [0, 7]
    if (ledIndex < 0) ledIndex = 0;
    if (ledIndex > 7) ledIndex = 7;

    //TODO we will have to convert this to work with gameY value
    //TODO we have to add cases for the flashing when too far (see https://youtu.be/-4HGtYqb4II&t=2m)
    
    //TODO I just noticed, the mapping is off. Top LED is getting illuminated with Y at -0.71 instead of -0.1

    // Reverse it to match Brian's video
    return 7-ledIndex;
}

// Function to map X to the appropriate color
static ledColors mapXtoLEDColor(double cur_x) {

    // From assignment PDF:
    // If the point is to the left (i.e., need to rotate it left), use a red colour.
    // If the point is to the right (i.e., need to rotate it right), use a green colour.
    // If the point is centred left-right, use a blue colour.

    // We will start by mapping to the normal range
    //TODO we will have to convert this to work with gameX
    
    if (cur_x <= -0.1) {
        return RED;
    } else if (cur_x >= 0.1) {
        return GREEN;
    } else {
        return BLUE;
    }
}

static void* gameLoop() {

    printf("Running game thread...\n");

    // Create the LED object
    neoPixelState pixels = {0}; // Initialize all elements to 0 (false) by default

    // Initialize LEDs // TODO ???
    for (int i = 0; i < STR_LEN; i++) {
        // pixels[i].isOn = true;
        pixels[i].isBright = true;
        // pixels[i].color = RED;
    }

    while (isRunning) {
        // Main game loop here

        // Generate target coordinates
        refreshCoords();

        // X-AXIS
        // RED -> rotate LEFT
        // GREEN -> rotate RIGHT
        // BLUE -> on target

        // Y-AXIS
        // LEDs map to Y-AXIS value EXCEPT when close to target (ALL LEDs ON)
        // Flashing if user goes too far to the limit (-1.0 or 1.0)

        for (int i = 0; i < 500; i++) {
            uint32_t col = mapXtoLEDColor(accel_getXNorm());
            int ledToChange = mapYtoLEDIndex(accel_getYNorm());
            for (int ledNum = 0; ledNum < STR_LEN; ledNum++) {
                if (ledNum == ledToChange) {
                    pixels[ledNum].color = col;
                } else {
                    pixels[ledNum].color = 0x00000000;
                }
            }
            // double cur_x = accel_getXNorm();
            // double cur_y = accel_getYNorm();
            // uint32_t col = setAllColorBrightness(NEO_LED_BASE, i);
            // uint32_t col = setLedColor(0, i, i);
            // printf("Setting to 0x%08X\n", col);
            // pixels[0].color = col;
            // pixels[1].color = col;
            // pixels[2].color = col;
            // pixels[3].color = col;
            // pixels[4].color = col;
            // pixels[5].color = col;
            // pixels[6].color = col;
            // pixels[7].color = col;
            setLedSimple(pixels);
            sleepForMs(0);
        }

        // Original loop
        /*if (cur_x > 0) {
            pixels[0].color = RED;
            pixels[0].isOn = true;
            pixels[1].color = GREEN;
            pixels[1].isOn = false;
        } else {
            pixels[0].isOn = false;
            pixels[1].isOn = true;
        }*/

        // setLeds(pixels);

        // printf("LED0 %d\t%d\t%d", getSingleColorBrightness(pixels[0].color, RED), getSingleColorBrightness(pixels[0].color, GREEN), getSingleColorBrightness(pixels[0].color, BLUE));
        // printf("LED1 %d\t%d\t%d", getSingleColorBrightness(pixels[1].color, RED), getSingleColorBrightness(pixels[1].color, GREEN), getSingleColorBrightness(pixels[1].color, BLUE));

        // struct accel_val acc = accel_get();
        // printf("Checking accel %.1f, %.1f\n", cur_x, cur_y);
        // printf("Generated %.1f, %.1f\n", gameX, gameY);
    }

    return NULL;

}

/* Cleanup function */
void game_cleanup() {

    printf("Cleaning up game...\n");
    pthread_join(gameThread, NULL);

}

/* This is just generating two targets (as global vars) */
static void refreshCoords() {
    gameX = generateNum();
    gameY = generateNum();
}

/* This is generating our random num [-0.5, 0.5] for target */
static double generateNum() {
    double num = (double)rand() / (double)RAND_MAX;
    return num -= 0.5;
}