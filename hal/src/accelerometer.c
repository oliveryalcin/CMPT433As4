#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <pthread.h>
#include <stdint.h>

#include "hal/accelerometer.h"
#include "hal/utils.h"
#include "hal/i2c.h"
#include "hal/neoPixel.h"

#define I2C_DEVICE_ADDRESS 0x18

#define CFG_P9_17 "config-pin P9_17 i2c"
#define CFG_P9_18 "config-pin P9_18 i2c"

// From Zen Red
// For LIS331DLH Accelerometer
#define CTRL_REG1 0x20
#define OUT_X_L 0x28
#define OUT_X_H 0x29
#define OUT_Y_L 0x2A
#define OUT_Y_H 0x2B
#define OUT_Z_L 0x2C
#define OUT_Z_H 0x2D

static _Atomic int i2cFileDesc;
static struct accel_val global_acc;

void accel_init() {
    runCommand(CFG_P9_17);
    runCommand(CFG_P9_18);
    sleepForMs(330); // recommended in assignment pdf
    i2cFileDesc = initI2cBus(I2CDRV_LINUX_BUS1, I2C_DEVICE_ADDRESS);

    // accel datasheet p23/38
    writeI2cReg(i2cFileDesc, CTRL_REG1, 0x27);
}


/* Helper to convert raw accel data into int16 */
static int16_t convertAccelData(unsigned char msb, unsigned char lsb) {
    int16_t value = (msb << 8) | lsb;
    if (value & 0x8000) {
        value = -((value ^ 0xFFFF) + 1);
    }
    return value;
}


/* Return accelerometer data in struct */
struct accel_val accel_get() {
  unsigned char buff[6];

  // Create temp struct
  struct accel_val temp_acc;

  readI2cRegPtr(i2cFileDesc, OUT_X_L + 0x80, buff, 6);

  // Read info into temp struct
  temp_acc.x = convertAccelData(buff[1], buff[0]);   // MSB 1, LSB 0
  temp_acc.y = -convertAccelData(buff[3], buff[2]);  // MSB 3, LSB 2
  temp_acc.z = convertAccelData(buff[5], buff[4]);   // MSB 5, LSB 4

  // Check if the read data is all zeros and if global_acc has any non-zero
  // value
  if (!((temp_acc.x == 0 && temp_acc.y == 0 && temp_acc.z == 0) &&
        (global_acc.x != 0 || global_acc.y != 0 || global_acc.z != 0))) {
    // If not all zeros or global_acc is all zeros, update the global_acc with
    // new data
    global_acc.x = temp_acc.x;
    global_acc.y = temp_acc.y;
    global_acc.z = temp_acc.z;
  }

  //   printf("Buffer contents: 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\n",
  //  buff[0], buff[1], buff[2], buff[3], buff[4], buff[5]);
  //   printf("\tAccel values: %d %d %d\n", acc.x, acc.y, acc.z);

  return global_acc;
}

/* Get X value normalized to [-1.0,1.0] */
double accel_getXNorm() {
    return ((double)accel_get().x) / ACCEL_MAX;
}

/* Get Y value normalized to [-1.0,1.0] */
double accel_getYNorm() {
    return ((double)accel_get().y) / ACCEL_MAX;
}

void accel_cleanup() {
    close(i2cFileDesc);
}