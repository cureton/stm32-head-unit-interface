#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/usb/usbd.h>


#include <stddef.h> /* for NULL */

#include "usb_cdc.h"
#include "usart.h"




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
    rcc_periph_clock_enable(RCC_USART1);
    rcc_periph_clock_enable(RCC_GPIOB);
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
 * main()
 * -------------------------------------------------------------------------- */
int main(void)
{

    uint8_t usart_tx_buf[256];            // backing store
    ringbuf_t usart_tx_rb;

    uint8_t usb_cdc_tx_buf[256];            // backing store
    ringbuf_t usb_cdc_tx_rb;

    clock_setup();
    gpio_setup();

    usb_cdc_setup();

    /* Initialise ring buffer structures */
    ringbuf_init(&usart_tx_rb, usart_tx_buf, sizeof(usart_tx_rb));
    ringbuf_init(&usb_cdc_tx_rb, usb_cdc_tx_buf, sizeof(usb_cdc_tx_rb));

    usb_cdc_set_tx_rb_ptr(&usb_cdc_tx_rb);   
    usb_cdc_set_rx_rb_ptr(&usart_tx_rb);   

    usart_setup();

	
   int count=0;

    while (1) {
        usb_cdc_poll();

        /* Read the USSRT if data is ready */
        if (USART_SR(USART1) & USART_SR_RXNE) {
            uint8_t b = USART_DR(USART1);
            usb_cdc_write(&b, 1);

//            gpio_toggle(GPIOC, GPIO13);
        }
        if ( count > 500000 )
        {
 //           uint8_t buf[]="hello\n";
//            usb_cdc_write(buf, sizeof(buf));
        //    gpio_toggle(GPIOC, GPIO13);
	    count = 0;
        } else {
            count++;
        }
    }

    return 0;
}
