#include "ir_remote.h"

/* Mikrosaniye bazli gecikme fonksiyonu
 * volatile: derleyicinin döngüyü optimize edip silmesini engeller
 * 8MHz clock ? her iterasyon ~0.5µs ? us*2 iterasyon = us µs */
static void delay_us(uint32_t us)
{
    volatile uint32_t count = us * 2;
    while (count--);
}

/* PA6 pin durumunu oku (VS1838B çikisi)
 * Bosta: HIGH (1), Sinyal gelince: LOW (0)
 * inline: döngü içinde sik çagrildigi için kodu buraya göm */
static inline uint8_t ir_pin(void)
{
    return (GPIOA->IDR >> 6) & 1;
}

/* Belirtilen seviyede geçirilen süreyi µs cinsinden ölç
 * level  : ölçülecek seviye (0=LOW, 1=HIGH)
 * timeout: maksimum bekleme süresi (µs)
 * return : geçirilen süre (µs), timeout asilirsa o anki deger */
static uint32_t measure_pulse(uint8_t level, uint32_t timeout)
{
    uint32_t count = 0;
    uint32_t wait  = 0;

    /* Beklenen seviyeye geçisi bekle, max 20ms */
    while (ir_pin() != level)
    {
        delay_us(1);
        if (++wait > 20000) return 0;
    }

    /* Bu seviyede kaç µs kaldigini ölç */
    while (ir_pin() == level)
    {
        delay_us(1);
        if (++count >= timeout) break;
    }

    return count;
}

/* PA6'yi floating input olarak ayarla
 * VS1838B kendi içinde pull-up tasir, bizim pull-up gerekmez
 * CNF=01 (floating), MODE=00 (input) = 0x4 */
void IR_Init(void)
{
    /* GPIOA clock'unu aktif et */
    RCC->APB2ENR |= (1 << 2);

    /* PA6 için CRL bitlerini temizle [27:24] */
    GPIOA->CRL &= ~(0xF << 24);

    /* PA6 ? floating input */
    GPIOA->CRL |=  (0x4 << 24);
}

/* NEC protokolü çözümleyici
 * Kumandadan gelen sinyali decode eder
 * return: komut byte'i (0x45 gibi), sinyal yoksa 0
 *
 * NEC çerçeve yapisi:
 * [9ms LOW][4.5ms HIGH][Adres 8bit][~Adres 8bit][Komut 8bit][~Komut 8bit]
 * Bit 0 ? 562µs LOW + 562µs  HIGH
 * Bit 1 ? 562µs LOW + 1687µs HIGH */
uint8_t IR_GetCommand(void)
{
    uint32_t duration;   /* Ölçülen darbe süresi (µs) */
    uint32_t raw = 0;    /* 32 bitlik ham NEC verisi   */

    /* Pin bosta HIGH olmali, LOW ise sinyal ortasindayiz */
    if (!ir_pin()) return 0;

    /* Falling edge bekle — sinyal baslangici */
    uint32_t wait = 0;
    while (ir_pin())
    {
        delay_us(1);
        if (++wait > 100000) return 0;  /* 100ms sinyal gelmedi */
    }

    /* Leader LOW dogrula: ~9000µs olmali */
    duration = measure_pulse(0, 15000);
    if (duration < 6000 || duration > 12000) return 0;

    /* Leader HIGH dogrula: ~4500µs olmali */
    duration = measure_pulse(1, 8000);
    if (duration < 3000 || duration > 6000) return 0;

    /* 32 biti sirayla oku */
    for (int i = 0; i < 32; i++)
    {
        /* Her bit LOW ile baslar: ~562µs */
        duration = measure_pulse(0, 2000);
        if (duration < 200 || duration > 1500) return 0;

        /* HIGH süresi bit degerini belirler
         * Kisa (~562µs)  ? Bit 0
         * Uzun (~1687µs) ? Bit 1 */
        duration = measure_pulse(1, 3000);

        if (duration < 200)
        {
            return 0;              /* Çok kisa — gürültü */
        }
        else if (duration < 1000)
        {
            /* Bit 0 — raw degismez, zaten 0 */
        }
        else
        {
            /* Bit 1 — raw'in i. bitine yaz */
            raw |= (1U << i);
        }
    }

    /* Komut byte'ini çikar (bit 16-23) */
    uint8_t cmd     = (raw >> 16) & 0xFF;

    /* Ters komut byte'ini çikar (bit 24-31) */
    uint8_t cmd_inv = (raw >> 24) & 0xFF;

    /* Dogrulama: cmd XOR ~cmd = 0xFF olmali */
    if ((uint8_t)(cmd ^ cmd_inv) != 0xFF) return 0;

    return cmd;
}
