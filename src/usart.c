#include <libopencm3/stm32/usart.h>

#include "usart.h"

/* --------------------------------------------------------------------------
 * USART Setup
 * -------------------------------------------------------------------------- */
void usart_setup(void)
{

        /* Setup USART2 parameters. */
        usart_set_baudrate(USART2, 19200);
        usart_set_databits(USART2, 8);
        usart_set_stopbits(USART2, USART_STOPBITS_1);
        usart_set_mode(USART2, USART_MODE_TX);
        usart_set_parity(USART2, USART_PARITY_NONE);
        usart_set_flow_control(USART2, USART_FLOWCONTROL_NONE);

        /* Finally enable the USART. */
        usart_enable(USART2);
}

