#include "terminal.h"
#include "memory.h"
#include "pico/platform.h"

int main(void)
{
	if (get_core_num() == 0){
		printk("core 0 entered main\n");
		k_mem_init();
	}
	else {
		printk("core 1 running ...\n");
		while (1)
			;//usb_task();

	}
	return 0;
}