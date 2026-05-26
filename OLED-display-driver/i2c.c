#ifndef SSD1306_H
#define SSD1306_H

#include "stm32f10x.h"
#include "i2c.h"

/* Klima calisma modu */
typedef enum {
    KLIMA_MOD_BEKLEME  = 0,
    KLIMA_MOD_ISITMA   = 1,
    KLIMA_MOD_SOGUTMA  = 2
} KlimaMod;

/* Ekrani baslat */
void SSD1306_Init(void);

/* Frame buffer'i temizle */
void SSD1306_Clear(void);

/* Yazma konumunu ayarla */
void SSD1306_SetCursor(uint8_t page, uint8_t col);

/* Tek karakter yaz (5x8) ? buffer'a */
void SSD1306_WriteChar(char c);

/* String yaz ? buffer'a */
void SSD1306_WriteString(const char *str);

/* Tam ekran bitmap'i buffer'a kopyala */
void SSD1306_DrawBitmap(const uint8_t *bitmap);

/* 18x32 buyuk rakam ciz ? buffer'a
 * digit : 0-9
 * x     : sutun baslangici (0-127)
 * page  : sayfa baslangici (0-4, 4 sayfa kaplar) */
void SSD1306_DrawBigDigit(uint8_t digit, uint8_t x, uint8_t page);

/* Buffer'i tek I2C transaction'da ekrana gonder
 * Donanim hatasi veya timeout durumunda I2C_TIMEOUT doner */
I2C_Status SSD1306_Flush(void);

/* Tum arayuzu guncelle ve ekrana gonder
 * Mod otomatik hesaplanir:
 *   room_temp > set_temp  ->  SOGUTMA
 *   room_temp < set_temp  ->  ISITMA
 *   room_temp == set_temp ->  BEKLEME
 *
 * set_temp  : SET sicaklik (buyuk font, ortada)
 * room_temp : Oda sicakligi (kucuk font, sag alt)
 * fan_pct   : Fan hizi %   (kucuk font, sol alt) */
I2C_Status SSD1306_DrawUI(uint8_t set_temp, uint8_t room_temp, uint8_t fan_pct);

I2C_Status SSD1306_Splash(void);

#endif
