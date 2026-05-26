#ifndef I2C_H
#define I2C_H

#include "stm32f10x.h"

/* I2C islem sonuc kodu */
typedef enum {
    I2C_OK      = 0,
    I2C_TIMEOUT = 1
} I2C_Status;

/* Her flag bekleme dongusu icin maksimum deneme sayisi
 * 8MHz PCLk1, 100kHz I2C -> bir bit ~10us
 * 10000 iterasyon ~ 10ms timeout */
#define I2C_TIMEOUT_VAL 10000

void       I2C_Init(void);
I2C_Status I2C_Start(void);
void       I2C_Stop(void);
I2C_Status I2C_WriteAddress(uint8_t address);
I2C_Status I2C_WriteByte(uint8_t data);

#endif
