#pragma once

#include "ringbuf.h"

void usb_cdc_init(ringbuf_t* tx_rb, ringbuf_t* rx_rb);
