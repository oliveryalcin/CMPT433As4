#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdlib.h> // rand()
#include <math.h> // fabs()

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
static bool aimOverflow; //if user is aiming way above or way below, set to true

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
    if (ledIndex < 0) {
        ledIndex = 0;
        aimOverflow = true;
    }
    else if (ledIndex > 7){
        ledIndex = 7;
        aimOverflow = true;
    } 
    else{
        aimOverflow = false;
    }

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
    int score = 0;

    // Initialize LEDs // TODO ???
    for (int i = 0; i < STR_LEN; i++) {
        // pixels[i].isOn = true;
        pixels[i].isBright = true;
        // pixels[i].color = RED;
    }

    while (isRunning) { 
        // Main game loop here
        bool gameOver = false; //unless user doesn't explicitly exit using J_Right, generate new game
        // Generate target coordinates
        refreshCoords();

        // X-AXIS
        // RED -> rotate LEFT
        // GREEN -> rotate RIGHT
        // BLUE -> on target

        // Y-AXIS
        // LEDs map to Y-AXIS value EXCEPT when close to target (ALL LEDs ON)
        // Flashing if user goes too far to the limit (-1.0 or 1.0)

        while (!gameOver) { // updated when user shoots correctly
            double accelX = accel_getXNorm();
            double accelY = accel_getYNorm();
            uint32_t col = mapXtoLEDColor(accelX);
            int ledToChange = mapYtoLEDIndex(accelY);
            bool onTargetY = (fabs(gameY - accelY) < 0.125); // checks if gameY value of target is same as accelY if so true
           // bool onTargetX = (fabs(gameX - accelX) < 0.333); // checks if gameX value of target is same as accelX if so true
            directions joystickDirection = getJoystickDirection(); 
      

            /*
                Joystick Input
            */
            if(joystickDirection == RIGHT){
                isRunning = false;
                gameOver = true; //basically used to iterate game loop
            }
            if(joystickDirection == DOWN){
                if(onTargetY && (col == BLUE)){
                    gameOver = true;
                    score += 1;
                    setSegDisplay(score);
                }
            }

            // Set all LEDs to 0 initially
            for (int ledNum = 0; ledNum < STR_LEN; ledNum++) { //if on target set all on, otherwise set to default value and overwrite necessary indexes later
                if(onTargetY){
                    pixels[ledNum].color = col;
                }
                else{
                    pixels[ledNum].color = 0x00000000;
                    
                }
            }
            if(!onTargetY){ // case where not on target, seperated as doing all edge cases in loop not feasable
                if (ledToChange == 0) { // edge case
                    if(accelY > 0.5){ // edge case of edge case
                        pixels[ledToChange].color = col;
                    }
                    else{
                        pixels[ledToChange].color = brightenColor(col);
                        pixels[ledToChange + 1].color = col;
                    }
                } else if (ledToChange == 7) { // edge case
                    if(accelY < -0.5){ // edge case of edge case
                        pixels[ledToChange].color = col;
                    }
                    else{
                        pixels[ledToChange].color = brightenColor(col);
                        pixels[ledToChange - 1].color = col;
                    }

                } else { // regular case
                    pixels[ledToChange + 1].color = col;
                    pixels[ledToChange].color = brightenColor(col);
                    pixels[ledToChange - 1].color = col;
                }
            }

            setLedSimple(pixels); //update values
            sleepForMs(0);
        }
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