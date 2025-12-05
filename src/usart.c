#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/f4/nvic.h> /* For interrupts */

#include <libopencm3/stm32/gpio.h>

#include "ringbuf.h"
#include "usart.h"


// Forward declarations
void usart_tx_notify_cb(void *ctx);
static void usart_start_tx(usart_ctx_t *ctx);


void usart_tx_notify_cb(void *ctx)
{
    usart_start_tx((usart_ctx_t *) ctx );
}

void usart_init(usart_ctx_t *ctx, uint32_t usart,
                ringbuf_t *tx_rb_ptr, ringbuf_t *rx_rb_ptr)
{
    ctx->usart = usart;
    ctx->tx_rb_ptr = tx_rb_ptr;
    ctx->rx_rb_ptr = rx_rb_ptr;
    ctx->tx_idle = 1;

    // Allow TX ring buffer to wake the USART driver 
    if (ctx->tx_rb_ptr != NULL) 
    { 
        ringbuf_set_write_notify_fn(ctx->tx_rb_ptr, usart_tx_notify_cb, (void *)ctx);
    } 

    // Configure USART hardware 
    usart_set_baudrate(usart, 19200);
    usart_set_databits(usart, 8);
    usart_set_stopbits(usart, USART_STOPBITS_1);
    usart_set_mode(usart, USART_MODE_TX_RX);
    usart_set_parity(usart, USART_PARITY_NONE);
    usart_set_flow_control(usart, USART_FLOWCONTROL_NONE);


    usart_enable_rx_interrupt(usart);
    nvic_enable_irq(NVIC_USART1_IRQ);

    usart_enable(usart);
}


static void usart_start_tx(usart_ctx_t *ctx)
{

    if (!ctx->tx_idle)
        return;

    uint8_t b;
    if (ringbuf_read(ctx->tx_rb_ptr, &b, 1) == 1) {
        ctx->tx_idle = 0;
        gpio_clear(GPIOC,GPIO13);
        usart_send(ctx->usart, b);
        usart_enable_tx_interrupt(ctx->usart);
    }
}

void usart_irq_handler(usart_ctx_t *ctx)
{
    uint32_t us = ctx->usart;

    /* RX interrupt */
    if (usart_get_flag(us, USART_SR_RXNE)) {
        uint8_t b = usart_recv(us);
 	// could check for overrun here on 0 length returned  - but that is unlikely 
        ringbuf_write(ctx->rx_rb_ptr, &b, 1);
    }

    /* TX interrupt */
    if (usart_get_flag(us, USART_SR_TXE)) {
        uint8_t b;
        if (ringbuf_read(ctx->tx_rb_ptr, &b, 1) == 1) {
            usart_send(us, b);
        } else {
            /* Nothing left → go idle */
            gpio_set(GPIOC,GPIO13);
            ctx->tx_idle = 1;
            usart_disable_tx_interrupt(us);
        }
    }
}
