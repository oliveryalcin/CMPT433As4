#ifndef _GAME_LOGIC_H_
#define _GAME_LOGIC_H_

#include <stdbool.h>

#define TARGET_RANGE 0.1

void game_init(bool *running);
double getGameX();
double getGameY();
double getMappedY();
bool getOnTarget();
int getScore();
void game_cleanup(void);

#endif // _GAME_LOGIC_H_