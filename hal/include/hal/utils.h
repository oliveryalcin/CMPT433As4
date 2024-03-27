
#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/wait.h> // WEXITSTATUS
#include <time.h>

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

#endif // _UTILS_H_
