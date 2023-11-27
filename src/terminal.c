#include "tusb.h"
#include "pico/stdio_usb.h"

#include "pico/types.h"
#include "locks.h"
#include "terminal.h"

static struct spinlock usb;
static struct spinlock _usb_task;

int term_connected(void)
{
	return tud_cdc_connected();
}

void term_init(void)
{
	//tusb_init();
	stdio_usb_init();
	init_lock(&usb, "usb_lock");
	init_lock(&_usb_task, "usb_task_lock");
}

void usb_task(void)
{
	acquire_lock(&_usb_task);
	tud_task();
	release_lock(&_usb_task);
}

void term_flush(void)
{
	tud_cdc_write_flush();
}

#include "hardware/irq.h"
#include "RP2040.h"

void printk(const char *str)
{
	if (!stdio_usb_connected())
		return;

	acquire_lock(&usb);

	int length = strlen(str);
	int written = 0;
	while (written < length)
	{
		int avail = tud_cdc_write_available();
		if( avail > length )
			avail = length;

		if (avail == 0)
			continue;

		// else
		// {
		// }
		written += tud_cdc_write(str, avail);
		tud_cdc_write_flush();
		str += written;
	}
	release_lock(&usb);

	if((__get_current_exception() != 0) && NVIC_GetPendingIRQ(USBCTRL_IRQ)) {
		irq_handler_t irq = irq_get_vtable_handler(USBCTRL_IRQ);
		irq();
	}
}

