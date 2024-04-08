#include "hal/pwmBuzzer.h"
#include "hal/utils.h" // sleepForMs()
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdatomic.h>


static const char* command = "config-pin p9_22 pwm";

#define PWM0B_DUTY_CYCLE "/dev/bone/pwm/0/a/duty_cycle"
#define PWM0B_PERIOD "/dev/bone/pwm/0/a/period"
#define PWM0B_DUTY_ENABLE "/dev/bone/pwm/0/a/enable"

static void run_command();
static pthread_t buzzerThread;
static pthread_mutex_t buzzerMutex = PTHREAD_MUTEX_INITIALIZER;
static void* buzzerLoop(); // for pthread_create
static bool* isRunning; // keep track of shutdown state, initial value passed from MAIN thread during initialization
static _Atomic buzzerTones currentTone;
void buzzerSetTone(buzzerTones tone);
static void* playHitSoundThread(void* arg);
static void* playMissSoundThread(void* arg);

void initBuzzer(bool* running) {

    isRunning = running;
    currentTone = NONE;

    run_command();
    FILE *fDutyCycle = fopen(PWM0B_DUTY_CYCLE, "w");
    FILE *fPeriod = fopen(PWM0B_PERIOD, "w");
    FILE *fEnable = fopen(PWM0B_DUTY_ENABLE, "w");

    if (!fEnable || !fDutyCycle || !fPeriod){
        exit(-1);
    }
    int fDutyCycleWritten = fprintf(fDutyCycle, "0");
    int fPeriodWritten = fprintf(fPeriod, "0");
    int fEnableWritten = fprintf(fEnable, "1");

    if (fEnableWritten <= 0 || fDutyCycleWritten <= 0|| fPeriodWritten <= 0) {
        perror("Unable to write to pin");
    }
    // Close file
    fclose(fEnable);

    // Begin buzzer loop thread
    pthread_create(&buzzerThread, NULL, &buzzerLoop, NULL);
}

void setBuzzer(int dutyCycle, int period){
    FILE *fDutyCycle = fopen(PWM0B_DUTY_CYCLE, "w");
    FILE *fPeriod = fopen(PWM0B_PERIOD, "w");
     if (!fDutyCycle || !fPeriod){
        exit(-1);
    }
    int fDutyCycleWritten = fprintf(fDutyCycle, "%d", dutyCycle);
    int fPeriodWritten = fprintf(fPeriod, "%d", period);

    if (fDutyCycleWritten <= 0|| fPeriodWritten <= 0) {
        perror("Unable to write to pin");
        exit(1);
    }
    // Close file
    fclose(fDutyCycle);
    fclose(fPeriod);

}

void buzzerSetTone(buzzerTones tone) {
    currentTone = tone;
}

static void* playHitSoundThread(void* arg) {

    (void)arg;

    if (pthread_mutex_trylock(&buzzerMutex) != 0) {
        return NULL;
    }

    setBuzzer(600000, 1000000);
    sleepForMs(100);
    setBuzzer(700000, 1000000);
    sleepForMs(120);
    setBuzzer(800000, 1000000);
    sleepForMs(140);
    setBuzzer(900000, 1000000);
    sleepForMs(160);
    setBuzzer(1000000, 1000000);
    sleepForMs(180);
    setBuzzer(110000, 1000000);
    sleepForMs(200);
    setBuzzer(0, 1000000);

    pthread_mutex_unlock(&buzzerMutex);

    return NULL;
}

static void* playMissSoundThread(void* arg) {

    (void)arg;

    if (pthread_mutex_trylock(&buzzerMutex) != 0) {
        return NULL;
    }

    setBuzzer(100000, 1000000); 
    sleepForMs(400);            
    setBuzzer(60000, 1000000); 
    sleepForMs(150);            
    setBuzzer(40000, 1000000); 
    sleepForMs(150);            
    setBuzzer(20000, 1000000); 
    sleepForMs(400);            
    setBuzzer(0, 1000000);      

    pthread_mutex_unlock(&buzzerMutex);

    return NULL;
}

static void playMissSound() {
    pthread_t missSoundThread;
    pthread_create(&missSoundThread, NULL, &playMissSoundThread, NULL);
    pthread_detach(missSoundThread);
}

static void playHitSound() {
    pthread_t hitSoundThread;
    pthread_create(&hitSoundThread, NULL, &playHitSoundThread, NULL);
    pthread_detach(hitSoundThread);
}

static void* buzzerLoop() {

    while (*isRunning) {

        // Check for buzzer tone
        switch (currentTone) {
            case NONE:
                break;
            case HIT:
                printf("Playing HIT\n");
                playHitSound();
                break;
            case MISS:
                printf("Playing MISS\n");
                playMissSound();
                break;
            default:
                break;
        }
        currentTone = NONE;
    }
    return NULL;
}

void buzzerCleanup() {
    // Join the thread
    pthread_join(buzzerThread, NULL);
}

static void run_command(){
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
}

