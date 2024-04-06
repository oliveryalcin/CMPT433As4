/*
    PRU Code
*/

#include <stdint.h>
#include <stdbool.h>
#include <pru_cfg.h>
#include "resource_table_empty.h"
#include "sharedDataStruct.h"

// Reference for shared RAM:
// https://markayoder.github.io/PRUCookbook/05blocks/blocks.html#_controlling_the_pwm_frequency

// GPIO Configuration
// ----------------------------------------------------------
#define STR_LEN 8           // # LEDs in our string
#define oneCyclesOn 700 / 5 // Stay on 700ns
#define oneCyclesOff 800 / 5
#define zeroCyclesOn 350 / 5
#define zeroCyclesOff 600 / 5
#define resetCycles 60000 / 5 // Must be at least 50u, use 60u

// P8_11 for output (on R30), PRU0
#define DATA_PIN 15 // Bit number to output on

volatile register uint32_t __R30;
volatile register uint32_t __R31;

// Shared Memory Configuration
// Don't forget config-pin P8.11 pruout
// -----------------------------------------------------------
#define THIS_PRU_DRAM 0x00000 // Address of DRAM
#define OFFSET 0x200          // Skip 0x100 for Stack, 0x100 for Heap (from makefile)
#define THIS_PRU_DRAM_USABLE (THIS_PRU_DRAM + OFFSET)

// Joystick info
// PRU pins: P8_15 -- pru0_pru_r31_15 (down/fire), P8_14 (right/exit)
// Reference: PRU guide p8
#define JOY_D_MASK (1 << 14)
#define JOY_R_MASK (1 << 15)

// This works for both PRU0 and PRU1 as both map their own memory to 0x0000000
volatile sharedColorStruct_t *pSharedMemStruct = (volatile void *)THIS_PRU_DRAM_USABLE;

// Functions
static void startUpSequence();
static void ledLoop();

void main(void) {

  // Clear SYSCFG[STANDBY_INIT] to enable OCP master port
  CT_CFG.SYSCFG_bit.STANDBY_INIT = 0;

  // Initialize joystick
  pSharedMemStruct->joyDown = false;
  pSharedMemStruct->joyRight = false;

  // Initialize LEDs
  for (int i = 0; i < 8; i++) {
    pSharedMemStruct->color[i] = 0x00000000;
  }

  // Start-up sequencing showing all LEDs in dim white
  startUpSequence();

  while (1) {

    // Main loop to set LEDs based on shared memory
    ledLoop();

    // Read joystick info into struct
    pSharedMemStruct->joyDown = (__R31 & JOY_D_MASK) == 0;
    pSharedMemStruct->joyRight = (__R31 & JOY_R_MASK) == 0;
  }

  // End of program
  __halt();
}

static void ledLoop() {

  // Main loop
  __delay_cycles(resetCycles);
  for (int j = 0; j < STR_LEN; j++) {
    for (int i = 31; i >= 0; i--) {
      if (pSharedMemStruct->color[j] & ((uint32_t)0x1 << i)) {
        __R30 |= 0x1 << DATA_PIN;  // Set the GPIO pin to 1
        __delay_cycles(oneCyclesOn - 1);
        __R30 &= ~(0x1 << DATA_PIN);  // Clear the GPIO pin
        __delay_cycles(oneCyclesOff - 2);
      } else {
        __R30 |= 0x1 << DATA_PIN;  // Set the GPIO pin to 1
        __delay_cycles(zeroCyclesOn - 1);
        __R30 &= ~(0x1 << DATA_PIN);  // Clear the GPIO pin
        __delay_cycles(zeroCyclesOff - 2);
      }
    }
  }

  // Send Reset
  __R30 &= ~(0x1 << DATA_PIN);  // Clear the GPIO pin
  __delay_cycles(resetCycles);
}

#define STARTUP_DELAY 20000000
static void startUpSequence() {

  // Turn all LEDs off
  for (int i = 0; i < STR_LEN; i++) {
    pSharedMemStruct->color[i] = 0x00000000;
    __delay_cycles(resetCycles);
    for (int j = 0; j < STR_LEN; j++) {
      for (int i = 31; i >= 0; i--) {
        if (pSharedMemStruct->color[j] & ((uint32_t)0x1 << i)) {
          __R30 |= 0x1 << DATA_PIN;  // Set the GPIO pin to 1
          __delay_cycles(oneCyclesOn - 1);
          __R30 &= ~(0x1 << DATA_PIN);  // Clear the GPIO pin
          __delay_cycles(oneCyclesOff - 2);
        } else {
          __R30 |= 0x1 << DATA_PIN;  // Set the GPIO pin to 1
          __delay_cycles(zeroCyclesOn - 1);
          __R30 &= ~(0x1 << DATA_PIN);  // Clear the GPIO pin
          __delay_cycles(zeroCyclesOff - 2);
        }
      }
    }
  }

  __delay_cycles(STARTUP_DELAY*2);

  // Dim white startup loop
  for (int i = 0; i < STR_LEN; i++) {
    pSharedMemStruct->color[i] = 0x01010100;
    __delay_cycles(resetCycles);
    for (int j = 0; j < STR_LEN; j++) {
      for (int i = 31; i >= 0; i--) {
        if (pSharedMemStruct->color[j] & ((uint32_t)0x1 << i)) {
          __R30 |= 0x1 << DATA_PIN;  // Set the GPIO pin to 1
          __delay_cycles(oneCyclesOn - 1);
          __R30 &= ~(0x1 << DATA_PIN);  // Clear the GPIO pin
          __delay_cycles(oneCyclesOff - 2);
        } else {
          __R30 |= 0x1 << DATA_PIN;  // Set the GPIO pin to 1
          __delay_cycles(zeroCyclesOn - 1);
          __R30 &= ~(0x1 << DATA_PIN);  // Clear the GPIO pin
          __delay_cycles(zeroCyclesOff - 2);
        }
      }
    }
    __delay_cycles(STARTUP_DELAY);
  }

  // Brightness test
  /*
    for (int i = 0; i < STR_LEN; i++) {
    pSharedMemStruct->color[i] = 0xffffff00;
    __delay_cycles(resetCycles);
    for (int j = 0; j < STR_LEN; j++) {
      for (int i = 31; i >= 0; i--) {
        if (pSharedMemStruct->color[j] & ((uint32_t)0x1 << i)) {
          __R30 |= 0x1 << DATA_PIN;  // Set the GPIO pin to 1
          __delay_cycles(oneCyclesOn - 1);
          __R30 &= ~(0x1 << DATA_PIN);  // Clear the GPIO pin
          __delay_cycles(oneCyclesOff - 2);
        } else {
          __R30 |= 0x1 << DATA_PIN;  // Set the GPIO pin to 1
          __delay_cycles(zeroCyclesOn - 1);
          __R30 &= ~(0x1 << DATA_PIN);  // Clear the GPIO pin
          __delay_cycles(zeroCyclesOff - 2);
        }
      }
    }
  }*/

  // Wait
  __delay_cycles(STARTUP_DELAY*3);

  // Reset
  for (int i = 0; i < 8; i++) {
    pSharedMemStruct->color[i] = 0x00000000;
  }

  // Reset loop
    for (int i = 0; i < STR_LEN; i++) {
    __delay_cycles(resetCycles);
    for (int j = 0; j < STR_LEN; j++) {
      for (int i = 31; i >= 0; i--) {
        if (pSharedMemStruct->color[j] & ((uint32_t)0x1 << i)) {
          __R30 |= 0x1 << DATA_PIN;  // Set the GPIO pin to 1
          __delay_cycles(oneCyclesOn - 1);
          __R30 &= ~(0x1 << DATA_PIN);  // Clear the GPIO pin
          __delay_cycles(oneCyclesOff - 2);
        } else {
          __R30 |= 0x1 << DATA_PIN;  // Set the GPIO pin to 1
          __delay_cycles(zeroCyclesOn - 1);
          __R30 &= ~(0x1 << DATA_PIN);  // Clear the GPIO pin
          __delay_cycles(zeroCyclesOff - 2);
        }
      }
    }
  }
}