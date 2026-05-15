#ifndef IR_REMOTE_H
#define IR_REMOTE_H

#include "stm32f10x.h"

/* NEC Protokolü — Yaygin mini kumanda kodlari
 * Çalismazsa asagidaki sniffer adimina bakacagiz */
#define IR_BTN_1    0x45
#define IR_BTN_2    0x46
#define IR_BTN_3    0x47

void    IR_Init(void);
uint8_t IR_GetCommand(void);

#endif
