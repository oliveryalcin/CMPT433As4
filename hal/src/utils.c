#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/wait.h> // WEXITSTATUS
#include <time.h>


/* Runs a Linux command */
void runCommand(char *command)
{
    // Execute the shell command (output into pipe)
    FILE *pipe = popen(command, "r");
    // Ignore output of the command; but consume it
    // so we don't get an error when closing the pipe.
    char buffer[1024];
    while (!feof(pipe) && !ferror(pipe))
    {
        if (fgets(buffer, sizeof(buffer), pipe) == NULL)
            break;
        // printf("--> %s", buffer); // Uncomment for debugging
    }
    // Get the exit code from the pipe; non-zero is an error:
    int exitCode = WEXITSTATUS(pclose(pipe));
    if (exitCode != 0)
    {
        perror("Unable to execute command:");
        printf(" command: %s\n", command);
        printf(" exit code: %d\n", exitCode);
    }
}


/* Gets the time in milliseconds */
long long getTimeInMs(void)
{
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    long long seconds = spec.tv_sec;
    long long nanoSeconds = spec.tv_nsec;
    long long milliSeconds = seconds * 1000 + nanoSeconds / 1000000;
    return milliSeconds;
}


/* Sleep for a specific amount of time */
void sleepForMs(long long delayInMs)
{
    const long long NS_PER_MS = 1000 * 1000;
    const long long NS_PER_SECOND = 1000000000;
    long long delayNs = delayInMs * NS_PER_MS;
    int seconds = delayNs / NS_PER_SECOND;
    int nanoseconds = delayNs % NS_PER_SECOND;
    struct timespec reqDelay = {seconds, nanoseconds};
    nanosleep(&reqDelay, (struct timespec *)NULL);
}


/* Mimics the 'cat' command */
void readFromFileToScreen(char *fileName)
{
    FILE *pFile = fopen(fileName, "r");
    if (pFile == NULL)
    {
        printf("ERROR: Unable to open file (%s) for read\n", fileName);
        exit(-1);
    }

    // Read string (line)
    const int MAX_LENGTH = 1024;
    char buff[MAX_LENGTH];
    fgets(buff, MAX_LENGTH, pFile);

    // Close
    fclose(pFile);
    printf("Read: '%s'\n", buff);
}


/* Get a bool val from file (-1 if neither 0 nor 1 )
    Modification of readFromFileToScreen */
int readBoolFromFile(char *fileName)
{
    FILE *pFile = fopen(fileName, "r");
    if (pFile == NULL)
    {
        printf("ERROR: Unable to open file (%s) for read\n", fileName);
        return -1;
    }
    
    const int MAX_LENGTH = 1024;
    char buff[MAX_LENGTH];
    if (fgets(buff, MAX_LENGTH, pFile) == NULL)
    {
        fclose(pFile);
        return -1;
    }

    fclose(pFile);
    
    // Trim newline
    buff[strcspn(buff, "\n")] = 0;

    // Check for values
    if (strcmp(buff, "1") == 0) { // Found a 1
        return 1;
    } else if (strcmp(buff, "0") == 0) { // Found a 0
        return 0;
    } else { // Found neither
        return -1;
    }
}
