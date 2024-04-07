#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include "hal/segDisplay.h"
#include <pthread.h>
#include <stdbool.h>
#include "hal/utils.h"


#define I2CDRV_LINUX_BUS1 "/dev/i2c-1" //only one that is necessary for LED
#define I2C_DEVICE_ADDRESS 0x20
// ZenCape Red values:
#define REG_DIRA 0x02
#define REG_DIRB 0x03
#define REG_OUTA 0x01
#define REG_OUTB 0x00

#define MAX_DISPLAYABLE_VALUE 99

#define GPIO61_DIRECTION "/sys/class/gpio/gpio61/direction"
#define GPIO44_DIRECTION "/sys/class/gpio/gpio44/direction"
#define GPIO_EXPORT "/sys/class/gpio/export"
#define GPIO61_VALUE "/sys/class/gpio/gpio61/value"
#define GPIO44_VALUE "/sys/class/gpio/gpio44/value"
// Display Thread
static pthread_t tSegDisplayPID;
static bool* isRunning; // keep track of shutdown state, initial value passed from MAIN thread during initialization

static const char* command = "config-pin P9_18 i2c & config-pin P9_17 i2c"; // Initializes pins for I2C

//static function declatarions
//static unsigned char readI2cReg(int i2cFileDesc, unsigned char regAddr);
static int initI2cBus(char* bus, int address);
static void writeI2cReg(int i2cFileDesc, unsigned char regAddr, unsigned char value);
static int toggleGPIOPin(const char* gpioPinDir, int state);
static int firstDigit, secondDigit;
static void* backgroundSegDisplayThread(); 
static void run_command();
static pthread_mutex_t pollingMutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    const int outA; // upper triangle
    const int outB; // lower triangle
} HexPairDigit;

// Index corresponds to actual digit 0 to 9 inclusive
static const HexPairDigit digit_hex_lookup_table[10] = {
    {0xE1, 0xD0}, // 0
    {0x04, 0xC0}, // 1
    {0xC3, 0x98}, // 2
    {0x03, 0xD8}, // 3
    {0x22, 0xC8}, // 4
    {0x23, 0x58}, // 5
    {0xE3, 0x58}, // 6
    {0x05, 0x01}, // 7
    {0xE3, 0xD8}, // 8
    {0x23, 0xC8}  // 9
};

static int toggleGPIOPin(const char* gpioPinDir, int state) {
    FILE *fPin = fopen(gpioPinDir, "w");
    int fPinWritten;
    
    if (!fPin) {
        perror("Error accessing GPIO pin");
        return -1;
    }
    
    fPinWritten = fprintf(fPin, "%d", state);

    if (fPinWritten <= 0) {
        perror("Error writing GPIO output");
        fclose(fPin);
        return -1;
    }
    
    // Close file
    fclose(fPin);
    return 0;
}

static int exportGPIOPins(){

    FILE *fPinExport = fopen(GPIO_EXPORT, "w");
    if (fPinExport == NULL) {
        return -1;
    }
    const char* pins[] = {"61", "44"};
    int i;
    for (i = 0; i < 2; i++) {
        int fPinExportWritten = fprintf(fPinExport, pins[i]);
        if (fPinExportWritten <= 0) {
            perror("Error writing to file");
            fclose(fPinExport);
            return -1;
        }
    }
    fclose(fPinExport);
    return 0;
}

static int configureGPIODirection(){

    FILE *fPin61 = fopen(GPIO61_DIRECTION, "w");
    FILE *fPin44 = fopen(GPIO44_DIRECTION, "w");

    int fPin61Written;
    int fPin44Written;
    if (!fPin61 || !fPin44){
        fclose(fPin61);
        fclose(fPin44);
        perror("Error opening GPIO direction files");
        return -1;
    }
    fPin61Written = fprintf(fPin61, "out");
    fPin44Written = fprintf(fPin44, "out");

     if (fPin61Written <= 0 || fPin44Written <= 0) {
        fclose(fPin61);
        fclose(fPin44);
        perror("Error writing GPIO direction files");
        return -1;
    }
    
    // Close file
    fclose(fPin61);
    fclose(fPin44);
    return 0;
}
void initSegDisplay(bool *flag){
	isRunning = flag;
    run_command(); 
//• Configure both pins on the microprocessor for output through GPIO (see other guide).
// If GPIO pins not yet exported, then export them (avoid re-exporting pins):
	if(exportGPIOPins() < 0){
        perror("Error exporting pins");
    }
//• Set direction to output:
	if(configureGPIODirection() < 0){
        perror("Error configuring GPIO direction");
    }
//• Drive a 1 to the GPIO pin to turn on the digit. The following turns on both digits.
	if (toggleGPIOPin(GPIO61_VALUE, 0) < 0 || toggleGPIOPin(GPIO44_VALUE, 0) < 0){
        perror("Error turning on SegDisplay GPIO pins");
    }
    if (pthread_create(&tSegDisplayPID, NULL, backgroundSegDisplayThread, NULL) != 0){
        perror("Error creating UDP Thread\n");
    }
	
}
static void turnOnFirstDigit() {
	toggleGPIOPin(GPIO61_VALUE, 1);
	toggleGPIOPin(GPIO44_VALUE, 0);
}

static void turnOnSecondDigit() {
    toggleGPIOPin(GPIO61_VALUE, 0);
	toggleGPIOPin(GPIO44_VALUE, 1);
}
void turnOffBothDigits(){
    toggleGPIOPin(GPIO61_VALUE, 0);
	toggleGPIOPin(GPIO44_VALUE, 0);
}
/*
    TODO: If necessary add mutex here, necessary if we call setSegDisplay from more than 1 thread.
*/
void setSegDisplay(int number){

    if (number > 99) {
        number = 99;
    }
    pthread_mutex_lock(&pollingMutex);
    {      
        secondDigit = number % 10;
        number = number / 10;
        firstDigit = number % 10;
    }
    pthread_mutex_unlock(&pollingMutex);


}
static void* backgroundSegDisplayThread(){
    int i2cFileDesc = initI2cBus(I2CDRV_LINUX_BUS1, I2C_DEVICE_ADDRESS);
    writeI2cReg(i2cFileDesc, REG_DIRA, 0x00);
    writeI2cReg(i2cFileDesc, REG_DIRB, 0x00);

    while (*isRunning) {
        
        turnOffBothDigits();
        pthread_mutex_unlock(&pollingMutex);
        {
            writeI2cReg(i2cFileDesc, REG_OUTA, digit_hex_lookup_table[secondDigit].outA);
            writeI2cReg(i2cFileDesc, REG_OUTB, digit_hex_lookup_table[secondDigit].outB);
        }
        pthread_mutex_unlock(&pollingMutex);
        turnOnSecondDigit();
        sleepForMs(5);

        turnOffBothDigits();
        pthread_mutex_unlock(&pollingMutex);
        {
            writeI2cReg(i2cFileDesc, REG_OUTA, digit_hex_lookup_table[firstDigit].outA);
            writeI2cReg(i2cFileDesc, REG_OUTB, digit_hex_lookup_table[firstDigit].outB);
        }
        pthread_mutex_unlock(&pollingMutex);
        turnOnFirstDigit();

        sleepForMs(5);

    }
    close(i2cFileDesc);
    return NULL;

}
int cleanupSegDisplay(){

    if (pthread_join(tSegDisplayPID, NULL) != 0) {
        perror("Error joining receiver\n");
        return -1;
    }
    
    if (toggleGPIOPin(GPIO61_VALUE, 0) < 0 || toggleGPIOPin(GPIO44_VALUE, 0) < 0){
        perror("Error turning off SegDisplay GPIO pins");
        return -1;
    }

	return 0;
}
static int initI2cBus(char* bus, int address)
{
	int i2cFileDesc = open(bus, O_RDWR);
	if (i2cFileDesc < 0) {
		printf("I2C DRV: Unable to open bus for read/write (%s)\n", bus);
		perror("Error is:");
		exit(-1);
	}

	int result = ioctl(i2cFileDesc, I2C_SLAVE, address);
	if (result < 0) {
		perror("Unable to set I2C device to slave address.");
		exit(-1);
	}
	return i2cFileDesc;
}

static void writeI2cReg(int i2cFileDesc, unsigned char regAddr, unsigned char value)
{
	unsigned char buff[2];
	buff[0] = regAddr;
	buff[1] = value;
	int res = write(i2cFileDesc, buff, 2);
	if (res != 2) {
		perror("Unable to write i2c register");
		exit(-1);
	}
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