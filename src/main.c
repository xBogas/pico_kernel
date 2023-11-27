#include "stdio.h"
#include "memory.h"
#include "init.h"
#include "hardware/irq.h"
#include "terminal.h"
#include "tusb.h"

int main(void)
{
	if (cpuid() == 0){
		printk("core 0 entered main\n");
		k_mem_init();

		uint32_t start = time_us_32();
		while (time_us_32() - start < 1000000) {
			printk("hello\n");
			sleep_ms(10);
		}
	}
	else {
		printk("core 1 running ...\n");
		while (1)
			usb_task();

	}

	return 0;
}