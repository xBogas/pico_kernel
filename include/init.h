#ifndef W_KERNEL_UTILS_H
#define W_KERNEL_UTILS_H

#include <stdint.h>
#include "pico/stdio.h"
#include "pico/multicore.h"
#include "hardware/resets.h"
#include "hardware/clocks.h"

// hardware registers
#include "hardware/structs/sio.h"
#include "hardware/structs/scb.h"
#include "hardware/regs/resets.h"

extern int main();

static int cpuid() {
	return sio_hw->cpuid;
}

static void main_wrapper(void)
{
	main();
}

static void kernel_entry(void) {
	stdio_init_all();
	while (!stdio_usb_connected())
		;
	multicore_launch_core1(main_wrapper);
	printf("exit kernel_entry\n");
}

extern char __StackLimit; /* Set by linker.  */

extern uint32_t __attribute__((section(".ram_vector_table"))) ram_vector_table[48];


#endif