#include "hal/infoPrinter.h"

#include <stdbool.h>
#include "../pru-as4/sharedDataStruct.h"
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h> // fabs()

#include "hal/accelerometer.h"
#include "hal/segDisplay.h"
#include "hal/pwmBuzzer.h"
#include "hal/neoPixel.h"
#include "hal/utils.h"
#include "hal/joystick.h"
#include "../app/src/gameLogic.h"

#define POLLING_RATE 300

/* For PRU shared memory */
static volatile void *pPruBase;
static volatile sharedColorStruct_t *pSharedPru0;

const char* JOY_DIRECTIONS[6] = {"UP", "PUSH", "RIGHT", "DOWN", "LEFT", "NA"};

static pthread_t printerThread;
static bool* isRunning; // keep track of shutdown state, initial value passed from MAIN thread during initialization
static void* printerLoop();
static void printLedDetails();


void printer_init(bool *running) {

    // Set running var
    isRunning = running;

    // Start thread
    pthread_create(&printerThread, NULL, printerLoop, NULL);
}

void printer_cleanup() {
    // Join the thread
    pthread_join(printerThread, NULL);
}

static void* printerLoop() {

    // Config PRU shared mem
    pPruBase = getPruMmapAddr();
    pSharedPru0 = PRU0_MEM_FROM_BASE(pPruBase);

    // Main loop
    while (*isRunning) {

        printf("===============  SCORE: %d ========================\n", getScore());

        // Print accelerometer data
        printf("Accelero\t\t%.2f\t%.2f\n", accel_getXNorm(), accel_getYNorm());

        // Get target points
        printf("Target\t\t\t%.2f\t%.2f\n", getGameX(), getGameY());

        // Get score

        // Check on target
        // printf("On target:\t\t%s\n", getOnTarget() ? "TRUE" : "FALSE");
        // printf("On target:\t\t\033[%sm%s\033[0m\n", getOnTarget() ? "32" : "31", getOnTarget() ? "TRUE" : "FALSE");
        printf("On target X:\t\t\033[%sm%s\033[0m\n", fabs(accel_getXNorm() - getGameX()) <= TARGET_RANGE ? "32" : "31", fabs(accel_getXNorm() - getGameX()) <= TARGET_RANGE ? "TRUE" : "FALSE");
        printf("On target Y:\t\t\033[%sm%s\033[0m\n", fabs(accel_getYNorm() - getGameY()) <= TARGET_RANGE ? "32" : "31", fabs(accel_getYNorm() - getGameY()) <= TARGET_RANGE ? "TRUE" : "FALSE");

        // Raw accelero data
        struct accel_val acc = accel_get();
        printf("Accelero-raw\t\t%d\t%d\t%d\n", acc.x, acc.y, acc.z);

        // Print joystick data
        printf("Joystick\t\t%s\n", JOY_DIRECTIONS[getJoystickDirection()]);

        // Print SegDisplay data
        printf("SegDisplay\t\t%d %d\n", getSegDisplayFirst(), getSegDisplaySecond());

        // Print NeoPixel data
        printLedDetails();

        // Splitter
        printf("\n");

        // Polling rate
        sleepForMs(POLLING_RATE);
    }
    return NULL;
}

static void printLedDetails() {

    return; // Comment out if you want LED data shown

    // Loop through each LED index
    for (int i = 0; i < STR_LEN; i++) {
        uint32_t col = pSharedPru0->color[i];
        // Get brightness for each color component
        int redBrightness = getSingleColorBrightness(col, RED);
        int greenBrightness = getSingleColorBrightness(col, GREEN);
        int blueBrightness = getSingleColorBrightness(col, BLUE);

        // Print the details
        // printf("0x%08X\t%3d\t%3d\t%3d\n",(pSharedPru0->color[i]), redBrightness, greenBrightness, blueBrightness);
                // Print the hex color value
        // printf("0x%08X\t", col);

        printf("\033[31m%3d\033[0m\t\033[32m%3d\033[0m\t\033[34m%3d\033[0m\n", redBrightness, greenBrightness, blueBrightness);
    }
}