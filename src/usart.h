#pragma once

#pragma once
#include <libopencm3/stm32/usart.h>
#include "ringbuf.h"

typedef struct {
    uint32_t usart;
    ringbuf_t *tx_rb_ptr;
    ringbuf_t *rx_rb_ptr;
    volatile int tx_idle;
} usart_ctx_t;

void usart_init(usart_ctx_t *ctx, uint32_t usart,
                ringbuf_t *tx_rb_ptr, ringbuf_t *rx_rb_ptr);

void usart_irq_handler(usart_ctx_t *ctx);

