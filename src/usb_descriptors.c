#include <stddef.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/usbstd.h>
#include <libopencm3/usb/cdc.h>

#include <libopencm3/stm32/desig.h> // For getting device uniq id -> usb serial

#include "usb_descriptors.h"

/* --------------------------------------------------------------------------
 * Device descriptor
 * -------------------------------------------------------------------------- */
const struct usb_device_descriptor dev_descriptor = {
    .bLength = USB_DT_DEVICE_SIZE,
    .bDescriptorType = USB_DT_DEVICE,
    .bcdUSB = 0x0200,

    .bDeviceClass = USB_CLASS_CDC,
    .bDeviceSubClass = 0,
    .bDeviceProtocol = 0,
    .bMaxPacketSize0 = 64,
    .idVendor = 0x1209,
    .idProduct = 0x0001,
    .bcdDevice = 0x0100,
    .iManufacturer = 1,
    .iProduct      = 2,
    .iSerialNumber = 3,
    .bNumConfigurations = 1,
};

/* --------------------------------------------------------------------------
 * Endpoint descriptors
 * -------------------------------------------------------------------------- */

/* CDC0 notification (interrupt IN) */
static const struct usb_endpoint_descriptor cdc0_comm_endp[] = {{
    .bLength = USB_DT_ENDPOINT_SIZE,
    .bDescriptorType = USB_DT_ENDPOINT,
    .bEndpointAddress = EP_CDC0_NOTIFY,
    .bmAttributes = USB_ENDPOINT_ATTR_INTERRUPT,
    .wMaxPacketSize = 16,
    .bInterval = 255,
}};



/* CDC0 data (bulk IN/OUT) */
static const struct usb_endpoint_descriptor cdc0_data_endp[] = {{
    .bLength = USB_DT_ENDPOINT_SIZE,
    .bDescriptorType = USB_DT_ENDPOINT,
    .bEndpointAddress = EP_CDC0_OUT,
    .bmAttributes = USB_ENDPOINT_ATTR_BULK,
    .wMaxPacketSize = 64,
    .bInterval = 1,
}, {
    .bLength = USB_DT_ENDPOINT_SIZE,
    .bDescriptorType = USB_DT_ENDPOINT,
    .bEndpointAddress = EP_CDC0_IN,
    .bmAttributes = USB_ENDPOINT_ATTR_BULK,
    .wMaxPacketSize = 64,
    .bInterval = 1,
}};

/* --------------------------------------------------------------------------
 * CDC functional descriptors (one block per function)
 * NOTE: bDescriptorType = CS_INTERFACE (from libopencm3/usb/cdc.h)
 * -------------------------------------------------------------------------- */

static const struct {
    struct usb_cdc_header_descriptor header;
    struct usb_cdc_call_management_descriptor call_mgmt;
    struct usb_cdc_acm_descriptor acm;
    struct usb_cdc_union_descriptor cdc_union;
} __attribute__((packed)) cdc0_func_desc = {
    .header = {
        .bFunctionLength = sizeof(struct usb_cdc_header_descriptor),
        .bDescriptorType = CS_INTERFACE,
        .bDescriptorSubtype = USB_CDC_TYPE_HEADER,
        .bcdCDC = 0x0110,
    },
    .call_mgmt = {
        .bFunctionLength = sizeof(struct usb_cdc_call_management_descriptor),
        .bDescriptorType = CS_INTERFACE,
        .bDescriptorSubtype = USB_CDC_TYPE_CALL_MANAGEMENT,
        .bmCapabilities = 0,
        .bDataInterface = IFACE_CDC0_DATA,
    },
    .acm = {
        .bFunctionLength = sizeof(struct usb_cdc_acm_descriptor),
        .bDescriptorType = CS_INTERFACE,
        .bDescriptorSubtype = USB_CDC_TYPE_ACM,
        .bmCapabilities = 0x02, /* line coding + serial state */
    },
    .cdc_union = {
        .bFunctionLength = sizeof(struct usb_cdc_union_descriptor),
        .bDescriptorType = CS_INTERFACE,
        .bDescriptorSubtype = USB_CDC_TYPE_UNION,
        .bControlInterface = IFACE_CDC0_COMM,
        .bSubordinateInterface0 = IFACE_CDC0_DATA,
    },
};


/* --------------------------------------------------------------------------
 * Interface descriptors
 * -------------------------------------------------------------------------- */

/* CDC0 COMM interface */
static const struct usb_interface_descriptor cdc0_comm_iface[] = {{
    .bLength = USB_DT_INTERFACE_SIZE,
    .bDescriptorType = USB_DT_INTERFACE,
    .bInterfaceNumber = IFACE_CDC0_COMM,
    .bAlternateSetting = 0,
    .bNumEndpoints = 1,
    .bInterfaceClass = USB_CLASS_CDC,
    .bInterfaceSubClass = USB_CDC_SUBCLASS_ACM,
    .bInterfaceProtocol = USB_CDC_PROTOCOL_AT,
    .iInterface = 0,

    .endpoint = cdc0_comm_endp,
    .extra   = &cdc0_func_desc,
    .extralen = sizeof(cdc0_func_desc),
}};

/* CDC0 DATA interface */
static const struct usb_interface_descriptor cdc0_data_iface[] = {{
    .bLength = USB_DT_INTERFACE_SIZE,
    .bDescriptorType = USB_DT_INTERFACE,
    .bInterfaceNumber = IFACE_CDC0_DATA,
    .bAlternateSetting = 0,
    .bNumEndpoints = 2,
    .bInterfaceClass = USB_CLASS_DATA,
    .bInterfaceSubClass = 0,
    .bInterfaceProtocol = 0,
    .iInterface = 0,

    .endpoint = cdc0_data_endp,
    .extra = NULL,
    .extralen = 0,
}};

/* Interface table */
static const struct usb_interface interfaces[] = {
    {
        .num_altsetting = 1,
        .altsetting     = cdc0_comm_iface,
    },
    {
        .num_altsetting = 1,
        .altsetting     = cdc0_data_iface,
    },
};

/* --------------------------------------------------------------------------
 * Configuration descriptor
 * -------------------------------------------------------------------------- */
const struct usb_config_descriptor config_descriptor = {
    .bLength = USB_DT_CONFIGURATION_SIZE,
    .bDescriptorType = USB_DT_CONFIGURATION,
    .wTotalLength = 0,  /* filled in by libopencm3 */
    .bNumInterfaces = 2,
    .bConfigurationValue = 1,
    .iConfiguration = 0,
    .bmAttributes = 0x80, /* bus-powered */
    .bMaxPower = 50,      /* 100 mA (units of 2 mA) */

    .interface = interfaces,
};

/* --------------------------------------------------------------------------
 * Strings
 * -------------------------------------------------------------------------- */

/*  Space to populate STM32 uniw ID into as hex */
#define SERIAL_BUFFER_LEN = 25  /* Max 126 chars */

static char usb_serial[25] = "STM32 Unique ID";



const char *usb_strings[] = {
    "VK3DCU - Code Hax",        /* 1 */
    "FT7900 Radio head adapter",       /* 2 */
    usb_serial,                /* 3 */  // The hexidecimal equivilent of the device unique ID registers
};


static void word_to_hex(uint32_t w, char *dst)
{
    for (int i = 0; i < 8; i++) {
        uint8_t nib = (w >> 28) & 0xF;
        w <<= 4;
        dst[i] = (nib < 10) ? ('0' + nib) : ('A' + (nib - 10));
    }
}

/* Copy STM32 uniqie id into serial localtion  */
void usb_descriptors_set_unique_serial() 
{
    uint32_t uid[3];

    /* Get pointer to uniq id rehister  from libopencm3*/
    desig_get_unique_id(uid); 
    
    word_to_hex(uid[2], &usb_serial[0]);
    word_to_hex(uid[1], &usb_serial[8]);
    word_to_hex(uid[0], &usb_serial[16]);

    usb_serial[24] = '\0';
}
