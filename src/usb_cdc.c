#include <stddef.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/usbstd.h>
#include <libopencm3/usb/cdc.h>

#include <libopencm3/stm32/desig.h> // For getting device uniq id -> usb serial

#include "usb_descriptors.h"
#include "usb_cdc.h"

/* Global USB device handle */
static usbd_device *usbdev;

/* Control request buffer (required by usbd_init) */
static uint8_t usbd_control_buffer[128];


/* --------------------------------------------------------------------------
 * Class hooks
 * -------------------------------------------------------------------------- */

static enum usbd_request_return_codes
cdc_control_request(usbd_device *dev,
                    struct usb_setup_data *req,
                    uint8_t **buf,
                    uint16_t *len,
                    usbd_control_complete_callback *complete)
{
    (void)dev;
    (void)complete;

    switch (req->bRequest) {
    case USB_CDC_REQ_SET_CONTROL_LINE_STATE:
        /* You can watch req->wValue bits here if you care */
        return USBD_REQ_HANDLED;

    case USB_CDC_REQ_SET_LINE_CODING:
        /* Host sends 7 bytes; we just accept them */
        if (*len == 7) {
            return USBD_REQ_HANDLED;
        }
        return USBD_REQ_NOTSUPP;

    case USB_CDC_REQ_GET_LINE_CODING:
        if (*len < 7) {
            return USBD_REQ_NOTSUPP;
        }
        /* Fake 115200 8N1 for both functions */
        (*buf)[0] = 0x00; /* 115200 = 0x0001C200 (little endian) */
        (*buf)[1] = 0xC2;
        (*buf)[2] = 0x01;
        (*buf)[3] = 0x00;
        (*buf)[4] = 0;    /* stop bits: 1 */
        (*buf)[5] = 0;    /* parity: none */
        (*buf)[6] = 8;    /* data bits: 8 */
        *len = 7;
        return USBD_REQ_HANDLED;

    default:
        return USBD_REQ_NEXT_CALLBACK;
    }
}



/* Simple echo callbacks for each CDC data OUT EP */

static void cdc0_data_rx_cb(usbd_device *dev, uint8_t ep)
{
    (void)ep;
    uint8_t buf[64];
    int len = usbd_ep_read_packet(dev, EP_CDC0_OUT, buf, sizeof(buf));
    if (len > 0) {
        usbd_ep_write_packet(dev, EP_CDC0_IN, buf, len);
    }
}

/* Called from main.c */
void usb_set_config(usbd_device *usbd_dev, uint16_t wValue)
{
    (void)wValue;

    /* CDC0 endpoints */
    usbd_ep_setup(usbd_dev, EP_CDC0_OUT,
                  USB_ENDPOINT_ATTR_BULK, 64, cdc0_data_rx_cb);
    usbd_ep_setup(usbd_dev, EP_CDC0_IN,
                  USB_ENDPOINT_ATTR_BULK, 64, NULL);
    usbd_ep_setup(usbd_dev, EP_CDC0_NOTIFY,
                  USB_ENDPOINT_ATTR_INTERRUPT, 16, NULL);

    /* One control callback shared for both CDC functions */
    usbd_register_control_callback(
        usbd_dev,
        USB_REQ_TYPE_CLASS | USB_REQ_TYPE_INTERFACE,
        USB_REQ_TYPE_TYPE  | USB_REQ_TYPE_RECIPIENT,
        cdc_control_request);
}


/* --------------------------------------------------------------------------
 * USB Setup
 * -------------------------------------------------------------------------- */
void usb_cdc_setup(void)
{

    /* Replace placeholder with processor serial number to allow unique udev rules */
    usb_descriptors_set_unique_serial();


    usbdev = usbd_init(&otgfs_usb_driver,
                       &dev_descriptor,
                       &config_descriptor,
                       usb_strings,
                       3,
                       usbd_control_buffer,
                       sizeof(usbd_control_buffer));

    usbd_register_set_config_callback(usbdev, usb_set_config);
}

void usb_cdc_poll() 
{
    usbd_poll(usbdev);
}
