#pragma once

#include <libopencm3/stm32/usart.h>

#include <stdint.h>
#include <stdbool.h>
#include "ringbuf.h"

/* You choose the size */
#define USART_TX_BUF_SIZE 256

void usart_setup(void);

/* RX forwarding: register ringbuffer to insert receved characters into*/
void usart_set_usb_rx_ringbuf(ringbuf_t *rb);

/* get TX ringbuffer reference to external software to write into */
ringbuf_t *usart_get_tx_ringbuf(void);

/* Send data out the UART */
int usart_write(const uint8_t *data, int len);

