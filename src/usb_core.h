#pragma once

#include <libopencm3/usb/usbd.h>


void usb_core_init(void);
usbd_device* usb_core_get_handle(void);
void usb_core_poll(void);

