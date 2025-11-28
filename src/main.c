#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/usb/usbd.h>

#include <stddef.h> /* for NULL */

#include "usb_descriptors.h"  /* dev_descriptor, config_descriptor, usb_strings, usb_set_config */

/* Global USB device handle */
static usbd_device *usbdev;

/* Control request buffer (required by usbd_init) */
static uint8_t usbd_control_buffer[128];

/* --------------------------------------------------------------------------
 * Clock Setup
 * -------------------------------------------------------------------------- */
static void clock_setup(void)
{
    rcc_clock_setup_pll(&rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_96MHZ]);
}

/* --------------------------------------------------------------------------
 * GPIO Setup: PA11/PA12 USB, PA9 fake VBUS, PC13 LED
 * -------------------------------------------------------------------------- */
static void gpio_setup(void)
{
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOC);

    /* USB pins: PA11 = DM, PA12 = DP (AF10) */
    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO11 | GPIO12);
    gpio_set_af(GPIOA, GPIO_AF10, GPIO11 | GPIO12);

    /* Fake VBUS: PA9 high so OTG FS thinks VBUS is present */
    gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO9);
    gpio_set(GPIOA, GPIO9);

    /* Optional: PC13 as LED output */
    gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO13);
    gpio_set(GPIOC, GPIO13); /* LED off (board-dependent) */
}

/* --------------------------------------------------------------------------
 * USB Setup
 * -------------------------------------------------------------------------- */
static void usb_setup(void)
{
    rcc_periph_clock_enable(RCC_OTGFS);
    rcc_periph_reset_pulse(RST_OTGFS);

    usbdev = usbd_init(&otgfs_usb_driver,
                       &dev_descriptor,
                       &config_descriptor,
                       usb_strings,
                       3,
                       usbd_control_buffer,
                       sizeof(usbd_control_buffer));

    usbd_register_set_config_callback(usbdev, usb_set_config);
}


void hard_fault_handler(void)
{
    while (1) {
        gpio_toggle(GPIOC, GPIO13);
        for (volatile int i = 0; i < 500000; i++);
    }
}

/* --------------------------------------------------------------------------
 * main()
 * -------------------------------------------------------------------------- */
int main(void)
{
    clock_setup();
    gpio_setup();

    usb_set_unique_serial();
    usb_setup();

    while (1) {
        usbd_poll(usbdev);
    }

    return 0;
}
