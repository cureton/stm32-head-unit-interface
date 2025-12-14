#include <stddef.h>
#include <stdint.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/usbstd.h>


#include "usb_core.h"
#include "usb_descriptors.h"

/* Global USB device handle */
static usbd_device *usbdev;

uint8_t control_request_buffer[128]; // Buffer for control requests - ensure this is big enough for the entire descriptor

void usb_core_init()
{
    /* Replace placeholder with processor serial number to allow unique udev rules */
    usb_descriptors_set_unique_serial();

    usbdev = usbd_init(&otgfs_usb_driver,
                       &dev_descriptor,
                       &config_descriptor,
                       usb_strings,
                       3,
                       control_request_buffer,
                       sizeof(control_request_buffer));

}

usbd_device* usb_core_get_handle() 
{
  return usbdev;
}

void usb_core_poll() 
{
    usbd_poll(usbdev);
}
