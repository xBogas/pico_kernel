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

static int cpuid() {
	return sio_hw->cpuid;
}

// this cannot be constructor
void /* __attribute__((constructor))  */ kernel_entry(void);

extern char __StackLimit; /* Set by linker.  */

extern uint32_t __attribute__((section(".ram_vector_table"))) ram_vector_table[48];


#endif