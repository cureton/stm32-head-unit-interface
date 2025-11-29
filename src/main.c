#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/stm32/usart.h>


#include <stddef.h> /* for NULL */

#include "usb_descriptors.h"  /* dev_descriptor, config_descriptor, usb_strings, usb_set_config */
#include "usb_cdc.h"




/* --------------------------------------------------------------------------
 * Clock Setup
 * -------------------------------------------------------------------------- */
static void clock_setup(void)
{
    rcc_clock_setup_pll(&rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_96MHZ]);

    /* GPIO */
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOC);
 
    /* USB OTG */
    rcc_periph_clock_enable(RCC_OTGFS);
    rcc_periph_reset_pulse(RST_OTGFS);

    /* USART 2 */
    rcc_periph_clock_enable(RCC_USART2);
}


/* --------------------------------------------------------------------------
 * GPIO Setup: PA11/PA12 USB, PA9 fake VBUS, PC13 LED
 * -------------------------------------------------------------------------- */
static void gpio_setup(void)
{
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



void hard_fault_handler(void)
{
    while (1) {
        gpio_toggle(GPIOC, GPIO13);
        for (volatile int i = 0; i < 500000; i++);
    }
}


/* --------------------------------------------------------------------------
 * USART Setup
 * -------------------------------------------------------------------------- */
static void usart_setup(void)
{

    	/* Setup USART2 parameters. */
	usart_set_baudrate(USART2, 19200);
	usart_set_databits(USART2, 8);
	usart_set_stopbits(USART2, USART_STOPBITS_1);
	usart_set_mode(USART2, USART_MODE_TX);
	usart_set_parity(USART2, USART_PARITY_NONE);
	usart_set_flow_control(USART2, USART_FLOWCONTROL_NONE);

	/* Finally enable the USART. */
	usart_enable(USART2);
}


/* --------------------------------------------------------------------------
 * main()
 * -------------------------------------------------------------------------- */
int main(void)
{
    clock_setup();
    gpio_setup();

    usb_cdc_setup();
    usart_setup();

    while (1) {
        usb_cdc_poll();

        /* Read the USSRT if data is ready */
        if (USART_SR(USART1) & USART_SR_RXNE) {
            uint8_t b = USART_DR(USART1);
        }
    }

    return 0;
}
