#pragma once

#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/usbstd.h>
#include <libopencm3/usb/cdc.h>


/* Called from main.c after usbd_init() */
void usb_set_config(usbd_device *usbd_dev, uint16_t wValue);

void usb_cdc_setup(void);

void usb_cdc_poll(void);


