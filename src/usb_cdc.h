#pragma once

#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/usbstd.h>
#include <libopencm3/usb/cdc.h>

#include "ringbuf.h"


void usb_cdc_init(ringbuf_t* tx_rb, ringbuf_t* rx_rb);
void usb_cdc_poll(void);

