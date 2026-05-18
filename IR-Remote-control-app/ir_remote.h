#ifndef IR_REMOTE_H
#define IR_REMOTE_H

#include "stm32f10x.h"

/* NEC protokolü komut kodlari
 * Not: Farkli kumandalar farkli kodlar gönderebilir
 * Bilinmeyen kumandalar için ir_sniffer_main.c kullan */
#define IR_BTN_1    0x45
#define IR_BTN_2    0x46
#define IR_BTN_3    0x47

/* PA6'yi VS1838B için floating input olarak baslat */
void IR_Init(void);

/* NEC protokolü çözümleyici
 * return: komut byte'i, sinyal yoksa 0 */
uint8_t IR_GetCommand(void);

#endif
