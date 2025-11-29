#pragma once

#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/usbstd.h>
#include <libopencm3/usb/cdc.h>

/* Interface numbers */
enum {
    IFACE_CDC0_COMM = 0,
    IFACE_CDC0_DATA = 1,
};

/* Endpoints for first CDC function */
#define EP_CDC0_OUT     0x01  /* Bulk OUT  */
#define EP_CDC0_IN      0x81  /* Bulk IN   */
#define EP_CDC0_NOTIFY  0x82  /* Interrupt IN */

/* Descriptors exposed to main.c */
extern const struct usb_device_descriptor dev_descriptor;
extern const struct usb_config_descriptor config_descriptor;
extern const char *usb_strings[];

void usb_descriptors_set_unique_serial(); /* Set serial number to processor id */

