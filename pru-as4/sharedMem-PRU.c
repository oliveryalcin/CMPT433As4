/*
    PRU Code
*/

#include <stdint.h>
#include <stdbool.h>
#include <pru_cfg.h>
#include "resource_table_empty.h"
#include "../hal/sharedDataStruct.h"

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

// This works for both PRU0 and PRU1 as both map their own memory to 0x0000000
volatile sharedColorStruct_t *pSharedMemStruct = (volatile void *)THIS_PRU_DRAM_USABLE;

void main(void)
{
    // Clear SYSCFG[STANDBY_INIT] to enable OCP master port
    CT_CFG.SYSCFG_bit.STANDBY_INIT = 0;

    // Initialize:
    for (int i = 0; i < 8; i++)
    {
        pSharedMemStruct->color[i] = 0x00000000;
    }

    while (1)
    {
        __delay_cycles(resetCycles);
        for (int j = 0; j < STR_LEN; j++)
        {
            for (int i = 31; i >= 0; i--)
            {
                if (pSharedMemStruct->color[j] & ((uint32_t)0x1 << i))
                {
                    __R30 |= 0x1 << DATA_PIN; // Set the GPIO pin to 1
                    __delay_cycles(oneCyclesOn - 1);
                    __R30 &= ~(0x1 << DATA_PIN); // Clear the GPIO pin
                    __delay_cycles(oneCyclesOff - 2);
                }
                else
                {
                    __R30 |= 0x1 << DATA_PIN; // Set the GPIO pin to 1
                    __delay_cycles(zeroCyclesOn - 1);
                    __R30 &= ~(0x1 << DATA_PIN); // Clear the GPIO pin
                    __delay_cycles(zeroCyclesOff - 2);
                }
            }
        }
    }

    // Send Reset
    __R30 &= ~(0x1 << DATA_PIN); // Clear the GPIO pin
    __delay_cycles(resetCycles);
    __halt();
}