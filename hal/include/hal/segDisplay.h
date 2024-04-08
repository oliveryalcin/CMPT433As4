#ifndef _SEG_DISPLAY_H_
#define _SEG_DISPLAY_H_
#include <stdbool.h>

int cleanupSegDisplay();
void initSegDisplay(bool* isRunning);
void setSegDisplay(int someNumberVal);
int getSegDisplayFirst();
int getSegDisplaySecond();

#endif