#include "hal/joystick.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

#include "hal/utils.h"
#include "hal/neoPixel.h"

static const char* gpio_value_file[] = {
    "/sys/class/gpio/gpio26/value", //up
    "/sys/class/gpio/gpio27/value", //push
    "/sys/class/gpio/gpio47/value", //right
    "/sys/class/gpio/gpio46/value", //down
    "/sys/class/gpio/gpio65/value" //left
};
/*
    Private Static variables for managing state and configurations
*/
static char* command = "config-pin p8.14 gpio && config-pin p8.15 gpio && config-pin p8.16 gpio && config-pin p8.18 gpio";
static char* command_pru = "config-pin p8.16 pruin && config-pin p8.15 pruin"; // for PRU shared mem
static const int size = 2; // size of joystickInput buffer
static directions joystickDirection = NA;
static bool* isRunning; // keep track of shutdown state, initial value passed from MAIN thread during initialization

/* For PRU shared memory */
static volatile void *pPruBase;
static volatile sharedColorStruct_t *pSharedPru0;

/*
    pThread variables
*/
static pthread_mutex_t pollingMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t joystickThreadPID;

/*
    Static Function Declarations
*/
// static void run_command();
static directions joystick_getInput();
static void* joystick_pollingThread();
static void* joystick_pollingThreadPRU();
// static void sleepForMs(long long delayInMs);

void joystick_init(bool *flag){
    isRunning = flag;
    // run_command();
    runCommand(command);
    pthread_create(&joystickThreadPID, NULL, joystick_pollingThread, NULL);

}

void joystick_initPRU(bool *flag) {
    isRunning = flag;
    runCommand(command_pru);
    pthread_create(&joystickThreadPID, NULL, joystick_pollingThreadPRU, NULL);
}

void joystick_shutdown(){
    // Join the polling thread to ensure it's terminated
    pthread_join(joystickThreadPID, NULL);
    // Destroy the polling mutex
    pthread_mutex_destroy(&pollingMutex);
}

static void* joystick_pollingThread(){

    while (isRunning)
    {
        // Read joystick input
        pthread_mutex_lock(&pollingMutex);
        {
            joystickDirection = joystick_getInput();
	        
        }
        pthread_mutex_unlock(&pollingMutex);
        sleepForMs(10); 
    }

    return NULL;
}

static void* joystick_pollingThreadPRU() {

    // Config PRU shared mem
    pPruBase = getPruMmapAddr();
    pSharedPru0 = PRU0_MEM_FROM_BASE(pPruBase);

    while (isRunning) {
        // Read joystick input from PRU shared memory
        pthread_mutex_lock(&pollingMutex);
        {
            //printf("joystick %d %d\n", pSharedPru0->joyRight, pSharedPru0->joyDown); //TODO delete
            joystickDirection = pSharedPru0->joyRight ? RIGHT : pSharedPru0->joyDown ? DOWN : NA;
        }
        pthread_mutex_unlock(&pollingMutex);
        sleepForMs(10);
    }
    
    return NULL;
}


/*
    Threadsafe - getter function for currently pressed joystick value
*/
directions getJoystickDirection() {
    directions input;
    pthread_mutex_lock(&pollingMutex);
    {
        input = joystickDirection;
        pthread_mutex_unlock(&pollingMutex);
    }
    return input;
}

/*
    Returns the direction of joystick that is being currently pressed.
*/
static directions joystick_getInput(){
    char joystickInput[2];
    // checks which pin is active and returns corresponding value
    for(int i = 0; i < 5; i++){
        FILE *pJoystickFile = fopen(gpio_value_file[i], "r");
        if (pJoystickFile == NULL){
            printf("ERROR: Unable to open GPIO file for read\n");
            exit(1);
        }
        
        fgets(joystickInput, size, pJoystickFile);
        fclose(pJoystickFile);
        //printf("Read: %s", buff);

        if(atoi(joystickInput) == 0){
            return (directions)i;
        }
    }
    
    return NA;
}


//TODO we can remove this since I changed it to use the one in utils
/*static void run_command(){
    // Execute the shell command (output into pipe)
    FILE *pipe = popen(command, "r");
    // Ignore output of the command; but consume it
    // so we don't get an error when closing the pipe.
    char buffer[1000];
    while (!feof(pipe) && !ferror(pipe)) {
        if (fgets(buffer, sizeof(buffer), pipe) == NULL)
            break;
        //printf("--> %s", buffer); // Uncomment for debugging
    }
    // Get the exit code from the pipe; non-zero is an error:
    int exitCode = WEXITSTATUS(pclose(pipe));
    if (exitCode != 0) {
        perror("Unable to execute command:");
        printf(" command: %s\n", command);
        printf(" exit code: %d\n", exitCode);
    }
}*/

