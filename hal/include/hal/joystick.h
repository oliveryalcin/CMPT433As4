#ifndef _JOYSTICK_H_
#define _JOYSTICK_H_
#include <stdbool.h>

typedef enum {
    UP,
    PUSH,
    RIGHT,
    DOWN,
    LEFT,
    NA
} directions;

void joystick_init(bool *flag);
void joystick_shutdown(void);
directions getJoystickDirection();

#endif