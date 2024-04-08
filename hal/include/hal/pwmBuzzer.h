#ifndef _PWM_BUZZER_H_
#define _PWM_BUZZER_H_


typedef enum {
    NONE,
    HIT,
    MISS,
} buzzerTones;


void initBuzzer();
void buzzerSetTone(buzzerTones tone);
void buzzerCleanup();


#endif