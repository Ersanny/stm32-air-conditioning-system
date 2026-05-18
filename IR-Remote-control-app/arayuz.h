#ifndef ARAYUZ_H
#define ARAYUZ_H

#include "stm32f10x.h"

/* PA1, PA2, PA3 ? push-pull output olarak baslat */
void LEDs_Init(void);

/* PA1 ? LED1 toggle */
void LED1_Toggle(void);

/* PA2 ? LED2 toggle */
void LED2_Toggle(void);

/* PA3 ? LED3 toggle */
void LED3_Toggle(void);

#endif
