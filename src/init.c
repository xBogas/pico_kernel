#include "init.h"
#include <stdio.h>
#include "pico/stdlib.h"

extern int main();

void kernel_entry(void) {
	stdio_init_all();
	while (!stdio_usb_connected())
		;
	sleep_ms(500);
	multicore_launch_core1(main);
	printf("exit kernel_entry\n");
}