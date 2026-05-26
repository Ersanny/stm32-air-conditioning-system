#include "i2c.h"

void I2C_Init(void)
{
    /* GPIOB ve I2C1 clocklarini ac */
    RCC->APB2ENR |= (1 << 3);    /* IOPBEN - GPIOB clock */
    RCC->APB1ENR |= (1 << 21);   /* I2C1EN - I2C1 clock */

    /* PB6 ve PB7 - Alternate Function Open-Drain, 50MHz
     * CRL bit[27:24] = PB6, bit[31:28] = PB7
     * MODE=11 (50MHz), CNF=11 (AF Open-Drain) = 0xFF */
    GPIOB->CRL &= ~(0xFF << 24);
    GPIOB->CRL |=  (0xFF << 24);

    /* I2C1 ayarlari */
    I2C1->CR1  &= ~(1 << 0);     /* I2C'yi devre disi birak (ayar icin) */

    /* CR2: cevre birimi clock frekansi = 8MHz */
    I2C1->CR2   =  8;

    /* CCR: 100kHz icin clock ayari
     * Standard mode: CCR = fPCLK1 / (2 * fI2C)
     * CCR = 8MHz / (2 * 100kHz) = 40 */
    I2C1->CCR   =  40;

    /* TRISE: maksimum rise time
     * Standard mode: TRISE = fPCLK1(MHz) + 1 = 9 */
    I2C1->TRISE =  9;

    /* I2C'yi aktif et */
    I2C1->CR1  |=  (1 << 0);
}

I2C_Status I2C_Start(void)
{
    uint32_t timeout = I2C_TIMEOUT_VAL;

    I2C1->CR1 |= (1 << 8);   /* START bit'i set et */

    /* SB (Start Bit) flagini bekle — SR1 bit 0 */
    while (!(I2C1->SR1 & (1 << 0)))
    {
        if (--timeout == 0) return I2C_TIMEOUT;
    }

    return I2C_OK;
}

void I2C_Stop(void)
{
    I2C1->CR1 |= (1 << 9);   /* STOP bit'i set et */
}

I2C_Status I2C_WriteAddress(uint8_t address)
{
    uint32_t timeout = I2C_TIMEOUT_VAL;

    /* Adresi DR registera yaz (yazma modu: adres << 1) */
    I2C1->DR = (address << 1);

    /* ADDR flagini bekle — SR1 bit 1 */
    while (!(I2C1->SR1 & (1 << 1)))
    {
        if (--timeout == 0) return I2C_TIMEOUT;
    }

    /* ADDR flagini temizle - SR1 ve SR2 sirayla okunarak temizlenir */
    (void)I2C1->SR1;
    (void)I2C1->SR2;

    return I2C_OK;
}

I2C_Status I2C_WriteByte(uint8_t data)
{
    uint32_t timeout = I2C_TIMEOUT_VAL;

    /* TXE flagini bekle — SR1 bit 7 */
    while (!(I2C1->SR1 & (1 << 7)))
    {
        if (--timeout == 0) return I2C_TIMEOUT;
    }

    I2C1->DR = data;

    /* BTF flagini bekle — SR1 bit 2 */
    timeout = I2C_TIMEOUT_VAL;
    while (!(I2C1->SR1 & (1 << 2)))
    {
        if (--timeout == 0) return I2C_TIMEOUT;
    }

    return I2C_OK;
}
