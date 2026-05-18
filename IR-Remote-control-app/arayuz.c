#include "arayuz.h"

void LEDs_Init(void)
{
    /* GPIOA clock'unu aktif et (APB2ENR bit 2 = IOPAEN) */
    RCC->APB2ENR |= (1 << 2);

    /* PA1, PA2, PA3 için CRL bitlerini temizle
     * PA1 = bit[7:4], PA2 = bit[11:8], PA3 = bit[15:12] */
    GPIOA->CRL &= ~(0xFFF << 4);

    /* PA1, PA2, PA3 ? push-pull output, 2MHz
     * MODE=10 (2MHz), CNF=00 (push-pull) = 0x2
     * 3 pin için 0x222 */
    GPIOA->CRL |= (0x222 << 4);
}

/* PA1 ? LED1 toggle */
void LED1_Toggle(void) { GPIOA->ODR ^= (1 << 1); }

/* PA2 ? LED2 toggle */
void LED2_Toggle(void) { GPIOA->ODR ^= (1 << 2); }

/* PA3 ? LED3 toggle */
void LED3_Toggle(void) { GPIOA->ODR ^= (1 << 3); }
