#include "ir_remote.h"

/* ================================================
 * Basit döngü gecikmesi — SysTick yok
 * 8MHz'de yaklasik 1µs = 8 cycle
 * Her döngü ~4 cycle ? 2 iterasyon ˜ 1µs
 * ================================================ */
static void delay_us(uint32_t us)
{
    /* volatile: derleyici optimize etmesin */
    volatile uint32_t count = us * 2;
    while (count--);
}

/* PA6 pinini oku */
static inline uint8_t ir_pin(void)
{
    return (GPIOA->IDR >> 6) & 1;
}

/* Pinin 'level' seviyesinde kaç "birim" kaldigini ölç
 * Her birim ~1µs (yaklasik)
 * timeout asilirsa o anki degeri döner */
static uint32_t measure_pulse(uint8_t level, uint32_t timeout)
{
    uint32_t count = 0;

    /* Beklenen seviyeye geçisi bekle */
    uint32_t wait = 0;
    while (ir_pin() != level)
    {
        delay_us(1);
        if (++wait > 20000) return 0;
    }

    /* Bu seviyede ne kadar kaldigini say */
    while (ir_pin() == level)
    {
        delay_us(1);
        if (++count >= timeout) break;
    }

    return count;
}

/* ================================================
 * IR_Init — PA6 floating input
 * VS1838B kendi içinde pull-up tasir
 * ================================================ */
void IR_Init(void)
{
    RCC->APB2ENR |= (1 << 2);       /* GPIOA clock aç                */

    GPIOA->CRL &= ~(0xF << 24);     /* PA6 temizle [27:24]           */
    GPIOA->CRL |=  (0x4 << 24);     /* CNF=01(floating), MODE=00     */
}

/* ================================================
 * IR_GetCommand — NEC Decode
 *
 * Bit karari threshold tabanli:
 * HIGH < 1000µs  ? Bit 0
 * HIGH > 1000µs  ? Bit 1
 *
 * Bu yaklasim timing hassasiyetine daha toleransli.
 * ================================================ */
uint8_t IR_GetCommand(void)
{
    uint32_t dur;
    uint32_t raw = 0;

    /* Pin bosta HIGH olmali */
    if (!ir_pin()) return 0;

    /* Falling edge bekle */
    uint32_t wait = 0;
    while (ir_pin())
    {
        delay_us(1);
        if (++wait > 100000) return 0;
    }

    /* Leader LOW: ~9000µs — 6000 ile 12000 arasi olmali */
    dur = measure_pulse(0, 15000);
    if (dur < 6000 || dur > 12000) return 0;

    /* Leader HIGH: ~4500µs — 3000 ile 6000 arasi olmali */
    dur = measure_pulse(1, 8000);
    if (dur < 3000 || dur > 6000) return 0;

    /* 32 bit veri */
    for (int i = 0; i < 32; i++)
    {
        /* LOW kismi: ~562µs — her bit LOW ile baslar */
        dur = measure_pulse(0, 2000);
        if (dur < 200 || dur > 1500) return 0;

        /* HIGH kismi: bit degerini belirler
         * Bit 0 ? ~562µs  HIGH
         * Bit 1 ? ~1687µs HIGH
         * Threshold: 1000µs */
        dur = measure_pulse(1, 3000);

        if (dur < 200)
        {
            return 0;           /* Çok kisa — hata */
        }
        else if (dur < 1000)
        {
            /* Bit 0 — raw'da degisiklik yok */
        }
        else
        {
            /* Bit 1 */
            raw |= (1U << i);
        }
    }

    /* Dogrulama: Komut ^ ~Komut = 0xFF */
    uint8_t cmd     = (raw >> 16) & 0xFF;
    uint8_t cmd_inv = (raw >> 24) & 0xFF;

    if ((uint8_t)(cmd ^ cmd_inv) != 0xFF) return 0;

    return cmd;
}
