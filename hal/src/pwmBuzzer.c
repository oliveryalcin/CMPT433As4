#include "hal/pwmBuzzer.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


static const char* command = "config-pin p9_22 pwm";
//static bool* isRunning; // keep track of shutdown state, initial value passed from MAIN thread during initialization

#define PWM0B_DUTY_CYCLE "/dev/bone/pwm/0/a/duty_cycle"
#define PWM0B_PERIOD "/dev/bone/pwm/0/a/period"
#define PWM0B_DUTY_ENABLE "/dev/bone/pwm/0/a/enable"
static void run_command();

void initBuzzer(){
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
