#include "arayuz.h"
#include "ir_remote.h"

int main(void)
{
    LEDs_Init();
    IR_Init();

    uint8_t cmd;

    while (1)
    {
        cmd = IR_GetCommand();

        if      (cmd == IR_BTN_1) LED1_Toggle();
        else if (cmd == IR_BTN_2) LED2_Toggle();
        else if (cmd == IR_BTN_3) LED3_Toggle();
    }
}
