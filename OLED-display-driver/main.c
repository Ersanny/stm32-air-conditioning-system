#include "ssd1306.h"

int main(void)
{
    SSD1306_Init();
		SSD1306_Splash();   /* <-- Acilis animasyonu */

    while (1)
    {
        /* Ileride gercek sensor verileri buraya gelecek */
        if (SSD1306_DrawUI(27, 19, 45) == I2C_TIMEOUT)
        {
            /* TODO: hata isle — watchdog, LED, yeniden baslat vs. */
        }
    }
}
