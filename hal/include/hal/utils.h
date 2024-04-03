#ifndef _UTILS_H_
#define _UTILS_H_

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/wait.h> // WEXITSTATUS
#include <time.h>
#include <sys/mman.h>


// General PRU Memomry Sharing Routine
// ----------------------------------------------------------------
#define PRU_ADDR      0x4A300000   // Start of PRU memory Page 184 am335x TRM
#define PRU_LEN       0x80000      // Length of PRU memory
#define PRU0_DRAM     0x00000      // Offset to DRAM
#define PRU1_DRAM     0x02000
#define PRU_SHAREDMEM 0x10000      // Offset to shared memory
#define PRU_MEM_RESERVED 0x200     // Amount used by stack and heap

// Convert base address to each memory section
#define PRU0_MEM_FROM_BASE(base) ( (base) + PRU0_DRAM + PRU_MEM_RESERVED)
#define PRU1_MEM_FROM_BASE(base) ( (base) + PRU1_DRAM + PRU_MEM_RESERVED)
#define PRUSHARED_MEM_FROM_BASE(base) ( (base) + PRU_SHAREDMEM)

/* Runs a Linux command (given in as1) */
void runCommand(char *command);

/* Gets the current time in milliseconds */
long long getTimeInMs(void);

/* Sleep for specified ms */
void sleepForMs(long long delayInMs);

/* Mimics the 'cat' command (read file, print content)) */
void readFromFileToScreen(char *fileName);

/* Reads a boolean value from a file; 
returns -1 if the content is neither 0 nor 1 */
int readBoolFromFile(char *fileName);

// Return the address of the PRU's base memory
volatile void* getPruMmapAddr(void);

void freePruMmapAddr(volatile void* pPruBase);

#endif // _UTILS_H_
