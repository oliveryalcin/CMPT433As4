// Reads from the accelerometer using I2C

#ifndef _ACCELEROMETER_H_
#define _ACCELEROMETER_H_

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

#include "hal/utils.h"

struct accel_val {
    int x;
    int y;
    int z;
};

void accel_init();
struct accel_val accel_get();
void accel_cleanup();

#endif // _ACCELEROMETER_H_