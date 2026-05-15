/*#ifndef ARAYUZ_H
#define ARAYUZ_H

#include "stm32f10x.h"

// LED — PC13 (Blue Pill built-in, Active LOW)
void LED_Init(void);
void LED_On(void);
void LED_Off(void);
void LED_Toggle(void);

// BUTON — PA0 (Pull-up, Active LOW) 
void Button_Init(void);
uint8_t Button_IsPressed(void);

#endif
*/
#ifndef ARAYUZ_H
#define ARAYUZ_H

#include "stm32f10x.h"

/* Mevcut LED (PC13 built-in) */
void LED_Init(void);
void LED_On(void);
void LED_Off(void);
void LED_Toggle(void);

/* Buton */
void Button_Init(void);
uint8_t Button_IsPressed(void);

/* Yeni: 3 harici LED — PA1, PA2, PA3 */
void LEDs_Init(void);
void LED1_Toggle(void);
void LED2_Toggle(void);
void LED3_Toggle(void);

#endif
