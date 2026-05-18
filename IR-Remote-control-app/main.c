#include "arayuz.h"
#include "ir_remote.h"

int main(void)
{
    /* Donanim baslatma */
    LEDs_Init();   /* PA1, PA2, PA3 çikis olarak ayarla */
    IR_Init();     /* PA6 floating input olarak ayarla  */

    uint8_t cmd;   /* IR kumandadan gelen komut kodu    */

    while (1)
    {
        /* Kumandadan komut bekle */
        cmd = IR_GetCommand();

        /* Gelen komuta göre ilgili LED'i toggle et */
        if      (cmd == IR_BTN_1) LED1_Toggle();  /* 1 tusu ? LED1 */
        else if (cmd == IR_BTN_2) LED2_Toggle();  /* 2 tusu ? LED2 */
        else if (cmd == IR_BTN_3) LED3_Toggle();  /* 3 tusu ? LED3 */
    }
}
