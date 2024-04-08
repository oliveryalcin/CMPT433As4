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
#include "hal/shutdown.h"
#include "hal/pwmBuzzer.h"

#include "gameLogic.h"

/* Variables */
static double gameX;
static double gameY;
static int globalScore;

// Thread stuff
static pthread_t gameThread;
static void* gameLoop();
static bool* isRunning; // keep track of shutdown state, initial value passed from MAIN thread during initialization
static ledColors mapXtoLEDColor(double cur_x);
static double generateNum();
static void refreshCoords();
static void adjustLEDsBasedOnTargetDistance(double accelY, neoPixelState pixels, uint32_t col);

void game_init(bool *running) {

    // Pass in the flag to later signal shutdown
    isRunning = running;

    printf("Initializing game...\n");
    pthread_create(&gameThread, NULL, &gameLoop, NULL);

}

// Function to map X to the appropriate color
static ledColors mapXtoLEDColor(double cur_x) {

    // From assignment PDF:
    // If the point is to the left (i.e., need to rotate it left), use a red colour.
    // If the point is to the right (i.e., need to rotate it right), use a green colour.
    // If the point is centred left-right, use a blue colour.
    
    if (fabs(gameX - cur_x) <= TARGET_RANGE) {
        return BLUE;
    } else if ((gameX - cur_x) > 0.0) {
        return GREEN;
    } else {
        return RED;
    }
}

int getScore() {
    return globalScore;
}

static void* gameLoop() {

    printf("Running game thread...\n");

    // Create the LED object
    neoPixelState pixels = {0}; // Initialize all elements to 0 (false) by default
    globalScore = 0;

    while (*isRunning) { 
        // Main game loop here
        
        bool gameOver = false; //unless user doesn't explicitly exit using J_Right, generate new game
        
        // Generate target coordinates
        refreshCoords();

        while (!gameOver) { // updated when user shoots correctly
            double accelX = accel_getXNorm();
            double accelY = accel_getYNorm();
            uint32_t col = mapXtoLEDColor(accelX);
            bool onTargetY = (fabs(gameY - accelY) <= TARGET_RANGE);
            // bool onTargetX = (fabs(gameX - accelX) <= TARGET_RANGE);
            directions joystickDirection = getJoystickDirection(); 
      
            /*
                Joystick Input
            */
            if(joystickDirection == RIGHT) {
                shutdown_signal();
                break;
            }
            if(joystickDirection == DOWN){
                if(onTargetY && (col == BLUE)){
                    buzzerSetTone(HIT);
                    gameOver = true;
                    globalScore += 1;
                    setSegDisplay(globalScore);
                } else {
                    buzzerSetTone(MISS);
                }
                //sleepForMs(150);
                continue;
            }

            // Set all LEDs to 0 initially
            for (int ledNum = 0; ledNum < STR_LEN; ledNum++) { //if on target set all on, otherwise set to default value and overwrite necessary indexes later
                if(onTargetY){
                    pixels[ledNum].color = brightenColor(col);
                }
                else{
                    pixels[ledNum].color = 0x00000000;
                }
            }
            if(!onTargetY){

                // So we are assuming we're NOT on target here
                // LEDs will be lit up based on distance from target
                adjustLEDsBasedOnTargetDistance(accelY, pixels, col);
            }

            setLedSimple(pixels); //update values
           // sleepForMs(0);
        }
    }

    return NULL;

}

static void adjustLEDsBasedOnTargetDistance(double accelY,
                                            neoPixelState pixels,
                                            uint32_t col) {
  
  // Distance from target
  double dist = fabs(gameY - accelY);
  bool isMovingTowardsNegative = accelY < gameY;

  // Extend by one LED in both directions
  double extendedStripLength = STR_LEN + 2;

  // Determine the conceptual main LED index based on distance and direction
  double conceptualMainLedIndex;
  if (isMovingTowardsNegative) {
    conceptualMainLedIndex = ((dist/0.94) * (extendedStripLength / 2.0)) + (STR_LEN / 2.0);
  } else {
    conceptualMainLedIndex = (STR_LEN / 2.0) - ((dist / 0.94) * (extendedStripLength / 2.0)) + 1;
  }

  // Consider extra LEDs
  int mainLedIndex = (int)round(conceptualMainLedIndex)-1;
  mainLedIndex = mainLedIndex < 0 ? -1 : mainLedIndex;
  mainLedIndex = mainLedIndex >= STR_LEN ? STR_LEN : mainLedIndex;

  // LED logic
  for (int i = 0; i < STR_LEN; i++) {
    if (i == mainLedIndex) {
      pixels[i].color = col;  // main ON
      pixels[i].color = brightenColor(col);
    } else {
      pixels[i].color = NEO_LED_BASE;  // all others OFF
    }

    // Making sure we don't change a non-existent LED
    if (mainLedIndex-1 >= 0) {
      pixels[mainLedIndex-1].color = col;  // One LED before main LED on
    }
    if (mainLedIndex+1 <= STR_LEN-1) {
      pixels[mainLedIndex+1].color = col;  // One LED after main LED on
    }
  }
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

double getGameX() {
    return gameX;
}

double getGameY() {
    return gameY;
}

bool getOnTarget() {
    return (fabs(gameY - accel_getYNorm()) < TARGET_RANGE) && (fabs(gameX - accel_getXNorm()) < TARGET_RANGE);
}

/* This is generating our random num [-0.5, 0.5] for target */
static double generateNum() {
    double num = (double)rand() / (double)RAND_MAX;
    return num -= 0.5;
}