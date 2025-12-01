#include <libopencm3/stm32/usart.h>

#include <libopencm3/stm32/gpio.h>

#include "ringbuf.h"
#include "usart.h"

/* --------------------------------------------------------------------------
 * Transmission Ring buffer 
 * -------------------------------------------------------------------------- */
static ringbuf_t tx_rb;

 /* pointer to ring buffer for receved characters. i.e USB CDC tx ring buffer */

static ringbuf_t *rx_rb = NULL;

/* Track current transmission state */
static volatile bool tx_idle = true;

/* --------------------------------------------------------------------------
 * USART Setup
 * -------------------------------------------------------------------------- */
void usart_setup(void)
{


    gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO6 | GPIO7);
    gpio_set_af(GPIOB, GPIO_AF7, GPIO6 | GPIO7);
    gpio_set_output_options(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO6 | GPIO7);


    /* Setup USART1 parameters. */
    usart_set_baudrate(USART1, 19200);
    usart_set_databits(USART1, 8);
    usart_set_stopbits(USART1, USART_STOPBITS_1);
    usart_set_mode(USART1, USART_MODE_TX_RX);
    usart_set_parity(USART1, USART_PARITY_NONE);
    usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);

    /* Finally enable the USART. */
    usart_enable(USART1);
}


/* --------------------------------------------------------------------------
 * Register USB CDC RX ringbuffer (UART RX → USB TX)
 * -------------------------------------------------------------------------- */
void usart_set_usb_rx_ringbuf(ringbuf_t *rb)
{
    rx_rb = rb;
}

/* --------------------------------------------------------------------------
 * Register USB CDC RX ringbuffer (UART RX → USB TX)
 * -------------------------------------------------------------------------- */
ringbuf_t *usart_get_tx_ringbuf(void)
{
    return &tx_rb;
}


/* --------------------------------------------------------------------------
 * Notify callback: registed  to ringbuffer to notifiy usart of writes that may 
 * required us to start the tranmistter 
 * -------------------------------------------------------------------------- */
static void usart_tx_notify_cb(void *arg)
{
    (void)arg;

    /* Only start TX when USART is idle */
    if (tx_idle && !ringbuf_empty(&tx_rb)) {
        tx_idle = false;
        usart_enable_tx_interrupt(USART1);
    }
}

/* --------------------------------------------------------------------------
 * Non-blocking write into TX ringbuffer
 * -------------------------------------------------------------------------- */
int usart_write(const uint8_t *data, int len)
{
    return ringbuf_write(&tx_rb, data, len);
}

