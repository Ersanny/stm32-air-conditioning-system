#include "arayuz.h"

/* ================================================
 * Built-in LED — PC13
 * Active LOW: ODR=0 yanar, ODR=1 söner
 * CRH bit[23:20] ? PC13
 * ================================================ */
void LED_Init(void)
{
    RCC->APB2ENR |= (1 << 4);       /* GPIOC clock aç (IOPCEN)       */
    GPIOC->CRH   &= ~(0xF << 20);   /* PC13 temizle                  */
    GPIOC->CRH   |=  (0x2 << 20);   /* MODE=10(2MHz), CNF=00(PP)     */
    GPIOC->ODR   |=  (1 << 13);     /* Baslangiç: sönsün             */
}

void LED_On(void)     { GPIOC->ODR &= ~(1 << 13); }
void LED_Off(void)    { GPIOC->ODR |=  (1 << 13); }
void LED_Toggle(void) { GPIOC->ODR ^=  (1 << 13); }

/* ================================================
 * Buton — PA0
 * Pull-up, Active LOW
 * CRL bit[3:0] ? PA0
 * ================================================ */
void Button_Init(void)
{
    RCC->APB2ENR |= (1 << 2);       /* GPIOA clock aç (IOPAEN)       */
    GPIOA->CRL   &= ~(0xF << 0);    /* PA0 temizle                   */
    GPIOA->CRL   |=  (0x8 << 0);    /* MODE=00(input), CNF=10(pull)  */
    GPIOA->ODR   |=  (1 << 0);      /* Pull-UP seç (F103: ODR ile)   */
}

uint8_t Button_IsPressed(void)
{
    return ((GPIOA->IDR & (1 << 0)) == 0) ? 1 : 0;
}

/* ================================================
 * 3 Harici LED — PA1, PA2, PA3
 * Push-pull output, 2MHz
 * CRL bit[7:4]=PA1, bit[11:8]=PA2, bit[15:12]=PA3
 * ================================================ */
void LEDs_Init(void)
{
    RCC->APB2ENR |= (1 << 2);       /* GPIOA clock aç                */

    /* PA1, PA2, PA3 ? MODE=10(2MHz), CNF=00(push-pull) = 0x2 */
    GPIOA->CRL &= ~(0xFFF << 4);    /* PA1-PA2-PA3 temizle           */
    GPIOA->CRL |=  (0x222 << 4);    /* Üçü de 2MHz push-pull         */

    /* Baslangiçta hepsi sönsün */
    GPIOA->ODR &= ~((1 << 1) | (1 << 2) | (1 << 3));
}

void LED1_Toggle(void) { GPIOA->ODR ^= (1 << 1); }
void LED2_Toggle(void) { GPIOA->ODR ^= (1 << 2); }
void LED3_Toggle(void) { GPIOA->ODR ^= (1 << 3); }
