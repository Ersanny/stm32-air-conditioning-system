while (1)
{
    cmd = IR_GetCommand();
    if (cmd != 0)
    {
        /* Komut kodunu LED blink sayısıyla göster
         * Üst nibble için LED1, alt nibble için LED2 */
        uint8_t upper = (cmd >> 4) & 0x0F;
        uint8_t lower = cmd & 0x0F;

        /* Üst nibble — LED1 */
        for (uint8_t i = 0; i < upper; i++)
        {
            LED1_Toggle();
            delay_ms(200);
            LED1_Toggle();
            delay_ms(200);
        }

        delay_ms(1000);  /* Ayıraç */

        /* Alt nibble — LED2 */
        for (uint8_t i = 0; i < lower; i++)
        {
            LED2_Toggle();
            delay_ms(200);
            LED2_Toggle();
            delay_ms(200);
        }

        delay_ms(2000);  /* Sonraki tuşa hazırlan */
    }
}
